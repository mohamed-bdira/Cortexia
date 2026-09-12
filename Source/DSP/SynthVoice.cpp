#include "SynthVoice.h"
#include <cmath>

SynthVoice::SynthVoice() {}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    targetHz = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    const bool glide = portaSec > 1.0e-4f && alwaysGlide && lastGlideHz > 1.0;
    playingHz = glide ? lastGlideHz : targetHz;
    currentPitch = playingHz;
    osc1.resetPhases();
    osc2.resetPhases();
    noteVelocity = velocity;
    filterL.reset();
    filterR.reset();
    adsr.noteOn();
}

void SynthVoice::setVoicing (float portaSeconds, bool always, double lastHz)
{
    portaSec = juce::jmax (0.0f, portaSeconds);
    alwaysGlide = always;
    lastGlideHz = lastHz;
}

void SynthVoice::retriggerPitch (double newHz, bool retriggerEnv)
{
    targetHz = newHz;
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
    }
}

void SynthVoice::stopNote (float, bool allowTailOff)
{
    adsr.noteOff();
    if (! allowTailOff || ! adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    pitchWheel = juce::jlimit (0, 16383, newPitchWheelValue);
}

void SynthVoice::controllerMoved (int, int) {}

void SynthVoice::prepareToPlay (double sampleRate, int)
{
    adsr.setSampleRate (sampleRate);
    currentSampleRate = sampleRate;
    osc1.prepareToPlay (sampleRate);
    osc2.prepareToPlay (sampleRate);
    filterL.reset();
    filterR.reset();
}

void SynthVoice::updateParameters (WaveType wave1, float gain1, float tune1, float detune1, int uni1, float uDet1, float uBlnd1, float wtPos1,
                                   WaveType wave2, float gain2, float tune2, float detune2, int uni2, float uDet2, float uBlnd2, float wtPos2,
                                   const juce::ADSR::Parameters& params, float cutoff, float resonance,
                                   float lfoRate, float lfoDepth, int target, float mVol,
                                   float bendRange, int pitchWheelValue)
{
    osc1.setParameters (wave1, gain1, tune1, detune1, uni1, uDet1, uBlnd1, wtPos1);
    osc2.setParameters (wave2, gain2, tune2, detune2, uni2, uDet2, uBlnd2, wtPos2);

    adsr.setParameters (params);
    baseCutoff = cutoff;
    filterResonance = resonance;
    filterLFO.setParameters (lfoRate, currentSampleRate);
    modDepth = lfoDepth;
    currentLfoTarget = target;
    masterVol = mVol;
    bendRangeSemitones = bendRange;
    pitchWheel = juce::jlimit (0, 16383, pitchWheelValue);
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive()) return;

    while (--numSamples >= 0)
    {
        if (currentSampleRate <= 0.0) break;

        float lfoVal = filterLFO.getNextSample();
        float modulationAmount = lfoVal * modDepth;

        float pitchExtra = 0.0f;
        float modulatedCutoff = baseCutoff;

        switch (currentLfoTarget)
        {
            case 1:
                modulatedCutoff = baseCutoff + (modulationAmount * baseCutoff);
                break;
            case 2:
                pitchExtra = modulationAmount * 12.0f;
                break;
        }

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
        osc1.processNextSample (bentPitch, pitchExtra, osc1L, osc1R);
        osc2.processNextSample (bentPitch, 0.0f, osc2L, osc2R);

        float sampleL = osc1L + osc2L;
        float sampleR = osc1R + osc2R;

        float adsrValue = adsr.getNextSample();
        sampleL *= noteVelocity * adsrValue * masterVol;
        sampleR *= noteVelocity * adsrValue * masterVol;

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
