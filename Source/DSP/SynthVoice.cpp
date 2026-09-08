#include "SynthVoice.h"

SynthVoice::SynthVoice() {}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound) 
{ 
    return dynamic_cast<SynthSound*> (sound) != nullptr; 
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    currentPitch = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    
    for (int i = 0; i < 7; ++i)
    {
        phase1[i] = random.nextDouble() * juce::MathConstants<double>::twoPi;
        phase2[i] = random.nextDouble() * juce::MathConstants<double>::twoPi;
    }

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
    filterL.reset(); 
    filterR.reset();
}

void SynthVoice::updateParameters (WaveType wave1, float gain1, float tune1, float detune1, int uni1, float uDet1, float uBlnd1,
                                   WaveType wave2, float gain2, float tune2, float detune2, int uni2, float uDet2, float uBlnd2,
                                   const juce::ADSR::Parameters& params, float cutoff, float resonance, 
                                   float lfoRate, float lfoDepth, int target, float mVol)
{
    currentWave1 = wave1; 
    masterGain1 = gain1; 
    osc1Tune = tune1; 
    osc1Detune = detune1;
    unisonCount1 = juce::jlimit (1, 7, uni1); 
    unisonDetune1 = uDet1; 
    unisonBlend1 = uBlnd1;
    
    currentWave2 = wave2; 
    masterGain2 = gain2; 
    osc2Tune = tune2; 
    osc2Detune = detune2;
    unisonCount2 = juce::jlimit (1, 7, uni2); 
    unisonDetune2 = uDet2; 
    unisonBlend2 = uBlnd2;

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

        float currentOsc1Tune = osc1Tune;
        float modulatedCutoff = baseCutoff;

        switch (currentLfoTarget)
        {
            case 1: 
                modulatedCutoff = baseCutoff + (modulationAmount * baseCutoff); 
                break;
            case 2: 
                currentOsc1Tune = osc1Tune + (modulationAmount * 12.0f); 
                break;
        }

        float sampleL = 0.0f;
        float sampleR = 0.0f;

        // RENDER OSCILLATOR 1
        for (int v = 0; v < unisonCount1; ++v)
        {
            float vDetune = 0.0f; 
            float vPan = 0.5f;
            float vGain = (v == 0) ? 1.0f : unisonBlend1 * (1.0f / std::sqrt ((float) unisonCount1 - 1.0f));

            if (unisonCount1 > 1 && v > 0)
            {
                float spread = 1.0f / (unisonCount1 / 2);
                int pair = (v + 1) / 2;
                float side = (v % 2 == 1) ? -1.0f : 1.0f;
                vPan = 0.5f + (side * 0.5f * (pair * spread));
                vDetune = side * pair * spread * unisonDetune1 * 50.0f;
            }

            double pitch1 = currentPitch * std::pow (2.0, (currentOsc1Tune + (osc1Detune / 100.0) + (vDetune / 100.0)) / 12.0);
            double phaseInc1 = (pitch1 / currentSampleRate) * juce::MathConstants<double>::twoPi;
            phase1[v] += phaseInc1;
            if (phase1[v] >= juce::MathConstants<double>::twoPi) 
                phase1[v] -= juce::MathConstants<double>::twoPi;

            float waveSample = 0.0f;
            switch (currentWave1)
            {
                case WaveType::Sine:     waveSample = (float) std::sin (phase1[v]); break;
                case WaveType::Saw:      waveSample = (float) (1.0 - (phase1[v] / juce::MathConstants<double>::pi)); break;
                case WaveType::Square:   waveSample = (phase1[v] < juce::MathConstants<double>::pi) ? 1.0f : -1.0f; break;
                case WaveType::Triangle: waveSample = (float) (2.0 * std::abs (2.0 * (phase1[v] / juce::MathConstants<double>::twoPi) - 1.0) - 1.0); break;
            }
            sampleL += waveSample * masterGain1 * vGain * (1.0f - vPan);
            sampleR += waveSample * masterGain1 * vGain * vPan;
        }

        // RENDER OSCILLATOR 2
        for (int v = 0; v < unisonCount2; ++v)
        {
            float vDetune = 0.0f; 
            float vPan = 0.5f;
            float vGain = (v == 0) ? 1.0f : unisonBlend2 * (1.0f / std::sqrt ((float) unisonCount2 - 1.0f));

            if (unisonCount2 > 1 && v > 0)
            {
                float spread = 1.0f / (unisonCount2 / 2);
                int pair = (v + 1) / 2;
                float side = (v % 2 == 1) ? -1.0f : 1.0f;
                vPan = 0.5f + (side * 0.5f * (pair * spread));
                vDetune = side * pair * spread * unisonDetune2 * 50.0f;
            }

            double pitch2 = currentPitch * std::pow (2.0, (osc2Tune + (osc2Detune / 100.0) + (vDetune / 100.0)) / 12.0);
            double phaseInc2 = (pitch2 / currentSampleRate) * juce::MathConstants<double>::twoPi;
            phase2[v] += phaseInc2;
            if (phase2[v] >= juce::MathConstants<double>::twoPi) 
                phase2[v] -= juce::MathConstants<double>::twoPi;

            float waveSample = 0.0f;
            switch (currentWave2)
            {
                case WaveType::Sine:     waveSample = (float) std::sin (phase2[v]); break;
                case WaveType::Saw:      waveSample = (float) (1.0 - (phase2[v] / juce::MathConstants<double>::pi)); break;
                case WaveType::Square:   waveSample = (phase2[v] < juce::MathConstants<double>::pi) ? 1.0f : -1.0f; break;
                case WaveType::Triangle: waveSample = (float) (2.0 * std::abs (2.0 * (phase2[v] / juce::MathConstants<double>::twoPi) - 1.0) - 1.0); break;
            }
            sampleL += waveSample * masterGain2 * vGain * (1.0f - vPan);
            sampleR += waveSample * masterGain2 * vGain * vPan;
        }

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