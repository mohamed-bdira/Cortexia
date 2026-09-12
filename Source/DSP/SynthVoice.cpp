#include "SynthVoice.h"

SynthVoice::SynthVoice() {}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    currentPitch = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    osc1.resetPhases();
    osc2.resetPhases();
    noteVelocity = velocity;
    filterL.reset();
    filterR.reset();
    adsr.noteOn();
}

void SynthVoice::stopNote (float, bool allowTailOff)
{
    adsr.noteOff();
    if (! allowTailOff || ! adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::pitchWheelMoved (int) {}
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
                                   float lfoRate, float lfoDepth, int target, float mVol)
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

        float osc1L = 0.0f, osc1R = 0.0f;
        float osc2L = 0.0f, osc2R = 0.0f;
        osc1.processNextSample (currentPitch, pitchExtra, osc1L, osc1R);
        osc2.processNextSample (currentPitch, 0.0f, osc2L, osc2R);

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
