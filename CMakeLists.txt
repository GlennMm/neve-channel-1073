cmake_minimum_required(VERSION 3.22)

project(Neve1073 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch JUCE
include(FetchContent)

# Disable JUCE extras for cross-compilation (VST3 helper causes issues)
if(CMAKE_CROSSCOMPILING)
    set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Don't build JUCE extras" FORCE)
    set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Don't build JUCE examples" FORCE)
endif()

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Plugin settings
# For cross-compilation, only build Standalone to avoid VST3 helper issues
if(CMAKE_CROSSCOMPILING)
    set(PLUGIN_FORMATS Standalone)
else()
    set(PLUGIN_FORMATS VST3 Standalone)
endif()

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
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "Neve 1073 Channel Strip"
)

# Exclude VST3 helper from build when cross-compiling (it requires native Windows.h)
if(CMAKE_CROSSCOMPILING AND TARGET juce_vst3_helper)
    set_target_properties(juce_vst3_helper PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

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

if(UNIX AND NOT APPLE AND NOT CMAKE_CROSSCOMPILING)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA IMPORTED_TARGET alsa)
    if(ALSA_FOUND)
        target_link_libraries(Neve1073 PRIVATE PkgConfig::ALSA)
    endif()
endif()

# Windows-specific settings
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Neve1073 PRIVATE
        _WIN32_WINNT=0x0601
        WINVER=0x0601
    )
    if(MINGW)
        target_link_options(Neve1073 PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()
