cmake_minimum_required(VERSION 3.24)
project(Cortexia VERSION 0.0.1)

# 1. Download JUCE 8 automatically
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.0
)
FetchContent_MakeAvailable(juce)

# ==========================================
# TARGET A: THE INSTRUMENT (Channel Rack)
# ==========================================
juce_add_plugin(Cortexia
    PLUGIN_MANUFACTURER_CODE "Aico"
    PLUGIN_CODE "Co01"
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Cortexia"
    COMPANY_NAME "Haramonics"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    NEEDS_WEBVIEW2 TRUE
)

# Generate the header for Cortexia
juce_generate_juce_header(Cortexia)

target_sources(Cortexia PRIVATE
    # Core
    Source/Core/PluginProcessor.cpp
    Source/Core/PluginProcessor.h
    Source/Core/PluginEditor.cpp
    Source/Core/PluginEditor.h

    # DSP
    Source/DSP/LFO.cpp
    Source/DSP/LFO.h
    Source/DSP/Oscillator.cpp
    Source/DSP/Oscillator.h
    Source/DSP/Wavetable.cpp
    Source/DSP/Wavetable.h
    Source/DSP/StateVariableFilter.cpp
    Source/DSP/StateVariableFilter.h
    Source/DSP/SynthVoice.cpp
    Source/DSP/SynthVoice.h
    Source/DSP/CortexiaSynth.cpp
    Source/DSP/CortexiaSynth.h

    # GUI
    Source/GUI/ModernLookAndFeel.cpp
    Source/GUI/ModernLookAndFeel.h
    Source/GUI/WaveformDisplay.cpp
    Source/GUI/WaveformDisplay.h
)

target_include_directories(Cortexia PRIVATE Source)

target_link_libraries(Cortexia PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Notice JUCE_VST3_CAN_REPLACE_VST2=0 is added here
target_compile_definitions(Cortexia PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# ==========================================
# TARGET B: THE EFFECT (Mixer)
# ==========================================
juce_add_plugin(CortexiaFX
    PLUGIN_MANUFACTURER_CODE "Aico"
    PLUGIN_CODE "CoFX"  
    FORMATS VST3 AU
    PRODUCT_NAME "CortexiaFX"
    COMPANY_NAME "Haramonics"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    NEEDS_WEBVIEW2 TRUE
)

# Generate the header for CortexiaFX
juce_generate_juce_header(CortexiaFX)

target_sources(CortexiaFX PRIVATE
    # Core
    Source/Core/PluginProcessor.cpp
    Source/Core/PluginProcessor.h
    Source/Core/PluginEditor.cpp
    Source/Core/PluginEditor.h

    # DSP
    Source/DSP/LFO.cpp
    Source/DSP/LFO.h
    Source/DSP/Oscillator.cpp
    Source/DSP/Oscillator.h
    Source/DSP/Wavetable.cpp
    Source/DSP/Wavetable.h
    Source/DSP/StateVariableFilter.cpp
    Source/DSP/StateVariableFilter.h
    Source/DSP/SynthVoice.cpp
    Source/DSP/SynthVoice.h
    Source/DSP/CortexiaSynth.cpp
    Source/DSP/CortexiaSynth.h

    # GUI
    Source/GUI/ModernLookAndFeel.cpp
    Source/GUI/ModernLookAndFeel.h
    Source/GUI/WaveformDisplay.cpp
    Source/GUI/WaveformDisplay.h
)

target_include_directories(CortexiaFX PRIVATE Source)

target_link_libraries(CortexiaFX PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Fixed the target name to CortexiaFX and added the VST2 override
target_compile_definitions(CortexiaFX PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)