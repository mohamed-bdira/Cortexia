#pragma once
#include <JuceHeader.h>
#include "SynthVoice.h"
#include <vector>

enum class VoiceMode { Poly = 0, Mono, Legato };

class CortexiaSynthesiser : public juce::Synthesiser
{
public:
    void setVoicing (VoiceMode mode, float portaSeconds, bool alwaysGlide);

protected:
    void handleMidiEvent (const juce::MidiMessage&) override;

private:
    void handleNoteOn (int channel, int note, float velocity);
    void handleNoteOff (int channel, int note, float velocity);
    void pushGlideStateToVoices();
    SynthVoice* findActiveMonoVoice();
    void removeHeldNote (int note);

    VoiceMode mode = VoiceMode::Poly;
    float portaSec = 0.0f;
    bool alwaysGlide = false;
    std::vector<int> heldNotes;
    double lastPitchHz = 0.0;
};
