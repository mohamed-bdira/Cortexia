#include "CortexiaSynth.h"

void CortexiaSynthesiser::setVoicing (VoiceMode newMode, float portaSeconds, bool always)
{
    if (mode != newMode)
    {
        heldNotes.clear();
        for (int ch = 1; ch <= 16; ++ch)
            allNotesOff (ch, true);
    }

    mode = newMode;
    portaSec = juce::jmax (0.0f, portaSeconds);
    alwaysGlide = always;
    pushGlideStateToVoices();
}

void CortexiaSynthesiser::pushGlideStateToVoices()
{
    for (int i = 0; i < getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<SynthVoice*> (getVoice (i)))
            voice->setVoicing (portaSec, alwaysGlide, lastPitchHz);
}

void CortexiaSynthesiser::handleMidiEvent (const juce::MidiMessage& message)
{
    if (message.isNoteOn() && message.getFloatVelocity() > 0.0f)
        handleNoteOn (message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    else if (message.isNoteOff() || (message.isNoteOn() && message.getFloatVelocity() <= 0.0f))
        handleNoteOff (message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    else if (message.isAllNotesOff() || message.isAllSoundOff())
    {
        heldNotes.clear();
        juce::Synthesiser::handleMidiEvent (message);
    }
    else
    {
        juce::Synthesiser::handleMidiEvent (message);
    }
}

void CortexiaSynthesiser::handleNoteOn (int channel, int note, float velocity)
{
    const double newHz = juce::MidiMessage::getMidiNoteInHertz (note);
    pushGlideStateToVoices();

    if (mode == VoiceMode::Poly)
    {
        juce::Synthesiser::noteOn (channel, note, velocity);
        lastPitchHz = newHz;
        pushGlideStateToVoices();
        return;
    }

    const bool wasHolding = ! heldNotes.empty();
    heldNotes.push_back (note);

    auto* voice = findActiveMonoVoice();
    if (voice == nullptr)
    {
        juce::Synthesiser::noteOn (channel, note, velocity);
    }
    else
    {
        const bool retriggerEnv = (mode == VoiceMode::Mono) || (mode == VoiceMode::Legato && ! wasHolding);
        voice->retriggerPitch (note, retriggerEnv);
    }

    lastPitchHz = newHz;
    pushGlideStateToVoices();
}

void CortexiaSynthesiser::handleNoteOff (int channel, int note, float velocity)
{
    if (mode == VoiceMode::Poly)
    {
        juce::Synthesiser::noteOff (channel, note, velocity, true);
        return;
    }

    removeHeldNote (note);

    auto* voice = findActiveMonoVoice();
    if (heldNotes.empty())
    {
        if (voice != nullptr)
            stopVoice (voice, velocity, true);
    }
    else if (voice != nullptr)
    {
        const int remain = heldNotes.back();
        voice->retriggerPitch (remain, false);
        lastPitchHz = juce::MidiMessage::getMidiNoteInHertz (remain);
        pushGlideStateToVoices();
    }
}

SynthVoice* CortexiaSynthesiser::findActiveMonoVoice()
{
    for (int i = 0; i < getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<SynthVoice*> (getVoice (i)))
            if (voice->isVoiceActive())
                return voice;

    return nullptr;
}

void CortexiaSynthesiser::removeHeldNote (int note)
{
    for (int i = (int) heldNotes.size(); --i >= 0;)
        if (heldNotes[(size_t) i] == note)
            heldNotes.erase (heldNotes.begin() + i);
}
