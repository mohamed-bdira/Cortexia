#include "SynthVoice.h"
#include <cmath>

SynthVoice::SynthVoice() {}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    currentMidiNote = midiNoteNumber;
    targetHz = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    const bool glide = portaSec > 1.0e-4f && alwaysGlide && lastGlideHz > 1.0;
    playingHz = glide ? lastGlideHz : targetHz;
    currentPitch = playingHz;
    osc1.resetPhases();
    osc2.resetPhases();
    noteVelocity = velocity;
    polyAftertouch01 = 0.0f;
    filterL.reset();
    filterR.reset();
    adsr.noteOn();
    env2.noteOn();
    retriggerLfos();
}

void SynthVoice::retriggerLfos()
{
    for (int i = 0; i < kNumLfos; ++i)
        if (lfoRetrig[i])
            lfos[i].reset();
}

void SynthVoice::setVoicing (float portaSeconds, bool always, double lastHz)
{
    portaSec = juce::jmax (0.0f, portaSeconds);
    alwaysGlide = always;
    lastGlideHz = lastHz;
}

void SynthVoice::retriggerPitch (int midiNote, bool retriggerEnv)
{
    currentMidiNote = midiNote;
    targetHz = juce::MidiMessage::getMidiNoteInHertz (midiNote);
    const bool glide = portaSec > 1.0e-4f;
    if (! glide)
        playingHz = targetHz;

    if (retriggerEnv)
    {
        osc1.resetPhases();
        osc2.resetPhases();
        filterL.reset();
        filterR.reset();
        adsr.noteOn();
        env2.noteOn();
        retriggerLfos();
    }
}

void SynthVoice::stopNote (float, bool allowTailOff)
{
    adsr.noteOff();
    env2.noteOff();
    if (! allowTailOff)
        env2.reset();
    if (! allowTailOff || ! adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    pitchWheel = juce::jlimit (0, 16383, newPitchWheelValue);
}

void SynthVoice::controllerMoved (int controllerNumber, int controllerValue)
{
    const float n = juce::jlimit (0.0f, 1.0f, (float) controllerValue / 127.0f);
    if (controllerNumber == 1)
        modWheel01 = n;
    else if (controllerNumber == 11)
        expression01 = n;
}

void SynthVoice::channelPressureChanged (int newChannelPressureValue)
{
    channelPressure01 = juce::jlimit (0.0f, 1.0f, (float) newChannelPressureValue / 127.0f);
}

void SynthVoice::aftertouchChanged (int newAftertouchValue)
{
    polyAftertouch01 = juce::jlimit (0.0f, 1.0f, (float) newAftertouchValue / 127.0f);
}

void SynthVoice::prepareToPlay (double sampleRate, int)
{
    adsr.setSampleRate (sampleRate);
    env2.setSampleRate (sampleRate);
    currentSampleRate = sampleRate;
    osc1.prepareToPlay (sampleRate);
    osc2.prepareToPlay (sampleRate);
    filterL.reset();
    filterR.reset();
}

void SynthVoice::updateParameters (WaveType wave1, float gain1, float tune1, float detune1, int uni1, float uDet1, float uBlnd1, float wtPos1,
                                   WaveType wave2, float gain2, float tune2, float detune2, int uni2, float uDet2, float uBlnd2, float wtPos2,
                                   const juce::ADSR::Parameters& params, const juce::ADSR::Parameters& env2Params,
                                   float cutoff, float resonance,
                                   const LfoSettings (&lfoSettings)[kNumLfos], float mVol,
                                   float bendRange, int pitchWheelValue, float modWheel,
                                   float expression, float channelPressure,
                                   const float (&macros)[4],
                                   const ModSlotPack& slots)
{
    osc1.setParameters (wave1, gain1, tune1, detune1, uni1, uDet1, uBlnd1, wtPos1);
    osc2.setParameters (wave2, gain2, tune2, detune2, uni2, uDet2, uBlnd2, wtPos2);

    adsr.setParameters (params);
    env2.setParameters (env2Params);
    baseCutoff = cutoff;
    filterResonance = resonance;
    for (int i = 0; i < kNumLfos; ++i)
    {
        lfos[i].setParameters (lfoSettings[i].rateHz, currentSampleRate);
        lfos[i].setShape (lfoSettings[i].shape);
        lfoDepths[i] = lfoSettings[i].depth;
        lfoRetrig[i] = lfoSettings[i].retrigger;
    }
    masterVol = mVol;
    baseWtPos1 = wtPos1;
    baseWtPos2 = wtPos2;
    bendRangeSemitones = bendRange;
    pitchWheel = juce::jlimit (0, 16383, pitchWheelValue);
    modWheel01 = juce::jlimit (0.0f, 1.0f, modWheel);
    expression01 = juce::jlimit (0.0f, 1.0f, expression);
    channelPressure01 = juce::jlimit (0.0f, 1.0f, channelPressure);
    for (int i = 0; i < 4; ++i)
        macro01[i] = juce::jlimit (0.0f, 1.0f, macros[i]);
    matrix = slots;
}

float SynthVoice::sourceValue (ModSource source, const float (&lfoValues)[kNumLfos], float env1, float env2Value, bool bipolar) const
{
    float uni = 0.0f;

    int lfoIndex = -1;
    if (source == ModSource::Lfo1)      lfoIndex = 0;
    else if (source == ModSource::Lfo2) lfoIndex = 1;
    else if (source == ModSource::Lfo3) lfoIndex = 2;

    if (lfoIndex >= 0)
    {
        const float v = lfoValues[lfoIndex] * lfoDepths[lfoIndex];
        return bipolar ? v : (lfoValues[lfoIndex] + 1.0f) * 0.5f * lfoDepths[lfoIndex];
    }

    switch (source)
    {
        case ModSource::Env1:
            uni = env1;
            break;
        case ModSource::Env2:
            uni = env2Value;
            break;
        case ModSource::Vel:
            uni = noteVelocity;
            break;
        case ModSource::ModWheel:
            uni = modWheel01;
            break;
        case ModSource::Keytrack:
            uni = (float) juce::jlimit (0, 127, currentMidiNote) / 127.0f;
            break;
        case ModSource::Aftertouch:
            uni = juce::jmax (channelPressure01, polyAftertouch01);
            break;
        case ModSource::Expression:
            uni = expression01;
            break;
        case ModSource::Macro1: uni = macro01[0]; break;
        case ModSource::Macro2: uni = macro01[1]; break;
        case ModSource::Macro3: uni = macro01[2]; break;
        case ModSource::Macro4: uni = macro01[3]; break;
        default:
            return 0.0f;
    }

    return bipolar ? (uni * 2.0f - 1.0f) : uni;
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive()) return;

    while (--numSamples >= 0)
    {
        if (currentSampleRate <= 0.0) break;

        float lfoValues[kNumLfos];
        for (int i = 0; i < kNumLfos; ++i)
            lfoValues[i] = lfos[i].getNextSample();
        const float adsrValue = adsr.getNextSample();
        const float env2Value = env2.getNextSample();

        float cutoffContrib = 0.0f;
        float osc1Pitch = 0.0f;
        float osc2Pitch = 0.0f;
        float osc1Vol = 1.0f;
        float osc2Vol = 1.0f;
        float wt1 = baseWtPos1;
        float wt2 = baseWtPos2;
        float masterMul = 1.0f;

        for (int i = 0; i < ModSlotPack::kNumSlots; ++i)
        {
            const auto& slot = matrix.slots[i];
            if (slot.source == ModSource::None || slot.dest == ModDest::None)
                continue;
            if (std::abs (slot.amount) < 1.0e-5f)
                continue;

            const float contrib = sourceValue (slot.source, lfoValues, adsrValue, env2Value, slot.bipolar) * slot.amount;

            switch (slot.dest)
            {
                case ModDest::Cutoff:    cutoffContrib += contrib; break;
                case ModDest::Osc1Pitch: osc1Pitch += contrib * 12.0f; break;
                case ModDest::Osc2Pitch: osc2Pitch += contrib * 12.0f; break;
                case ModDest::Osc1Vol:   osc1Vol *= juce::jmax (0.0f, 1.0f + contrib); break;
                case ModDest::Osc2Vol:   osc2Vol *= juce::jmax (0.0f, 1.0f + contrib); break;
                case ModDest::Osc1Wt:    wt1 += contrib; break;
                case ModDest::Osc2Wt:    wt2 += contrib; break;
                case ModDest::Master:    masterMul *= juce::jmax (0.0f, 1.0f + contrib); break;
                default: break;
            }
        }

        const float modulatedCutoff = juce::jlimit (20.0f, 20000.0f, baseCutoff * (1.0f + cutoffContrib));
        osc1.setWtPosition (wt1);
        osc2.setWtPosition (wt2);

        const float bendNorm = (float) (pitchWheel - 8192) / 8192.0f;

        if (portaSec <= 1.0e-4f)
        {
            playingHz = targetHz;
        }
        else
        {
            const double curMidi = 69.0 + 12.0 * std::log2 (juce::jmax (playingHz, 1.0) / 440.0);
            const double tgtMidi = 69.0 + 12.0 * std::log2 (juce::jmax (targetHz, 1.0) / 440.0);
            const double coeff = 1.0 - std::exp (-1.0 / (portaSec * currentSampleRate));
            playingHz = 440.0 * std::pow (2.0, ((curMidi + (tgtMidi - curMidi) * coeff) - 69.0) / 12.0);
        }

        currentPitch = playingHz;
        const double bentPitch = playingHz * std::pow (2.0, (bendNorm * bendRangeSemitones) / 12.0);

        float osc1L = 0.0f, osc1R = 0.0f;
        float osc2L = 0.0f, osc2R = 0.0f;
        osc1.processNextSample (bentPitch, osc1Pitch, osc1L, osc1R);
        osc2.processNextSample (bentPitch, osc2Pitch, osc2L, osc2R);

        osc1L *= osc1Vol;
        osc1R *= osc1Vol;
        osc2L *= osc2Vol;
        osc2R *= osc2Vol;

        float sampleL = (osc1L + osc2L) * noteVelocity * adsrValue * masterVol * masterMul;
        float sampleR = (osc1R + osc2R) * noteVelocity * adsrValue * masterVol * masterMul;

        filterL.setParams (modulatedCutoff, filterResonance, currentSampleRate);
        filterR.setParams (modulatedCutoff, filterResonance, currentSampleRate);

        float finalSampleL = filterL.processLowPass (sampleL);
        float finalSampleR = filterR.processLowPass (sampleR);

        if (outputBuffer.getNumChannels() > 0) outputBuffer.addSample (0, startSample, finalSampleL);
        if (outputBuffer.getNumChannels() > 1) outputBuffer.addSample (1, startSample, finalSampleR);

        startSample++;

        if (! adsr.isActive()) { clearCurrentNote(); break; }
    }
}
