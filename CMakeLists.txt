cmake_minimum_required(VERSION 3.22)

project(Neve1073 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Plugin settings
juce_add_plugin(Neve1073
    COMPANY_NAME "Neve1073"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Nv73
    PLUGIN_CODE Nv73
    FORMATS VST3 Standalone
    PRODUCT_NAME "Neve 1073 Channel Strip"
)

# Source files
target_sources(Neve1073
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/PresetManager.cpp
        Source/DSP/TransformerSaturation.cpp
        Source/DSP/PreampStage.cpp
        Source/DSP/InductorEQ.cpp
        Source/DSP/ShelfEQ.cpp
        Source/DSP/HighPassFilter.cpp
        Source/DSP/Oversampling.cpp
        Source/DSP/ParameterSmoothing.cpp
)

target_include_directories(Neve1073
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(Neve1073
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Neve1073
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Platform-specific settings
if(APPLE)
    set_target_properties(Neve1073 PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
    )
endif()

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA IMPORTED_TARGET alsa)
    if(ALSA_FOUND)
        target_link_libraries(Neve1073 PRIVATE PkgConfig::ALSA)
    endif()
endif()
