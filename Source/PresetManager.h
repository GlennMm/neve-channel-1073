#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

namespace Neve1073
{

/**
 * Preset Manager for Neve 1073 Channel Strip
 * Handles factory presets and user preset save/load functionality
 */
class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& apvts);

    // Preset info structure
    struct PresetInfo
    {
        juce::String name;
        juce::String category;
        juce::String author;
        bool isFactory;
    };

    // Get list of all available presets
    juce::StringArray getPresetNames() const;
    juce::StringArray getFactoryPresetNames() const;
    juce::StringArray getUserPresetNames() const;

    // Load a preset by name
    bool loadPreset(const juce::String& presetName);

    // Save current settings as a user preset
    bool saveUserPreset(const juce::String& presetName);

    // Delete a user preset
    bool deleteUserPreset(const juce::String& presetName);

    // Get current preset name
    juce::String getCurrentPresetName() const { return currentPresetName; }

    // Check if current state matches a preset
    bool isCurrentStateModified() const;

    // Get preset directory
    juce::File getUserPresetDirectory() const;

    // Refresh user presets from disk
    void refreshUserPresets();

    // Get preset info
    PresetInfo getPresetInfo(const juce::String& presetName) const;

private:
    juce::AudioProcessorValueTreeState& parameters;
    juce::String currentPresetName{"Init"};

    // Factory presets stored in memory
    struct FactoryPreset
    {
        juce::String name;
        juce::String category;
        std::map<juce::String, float> values;
    };

    std::vector<FactoryPreset> factoryPresets;
    juce::StringArray userPresetNames;

    void initializeFactoryPresets();
    void applyPresetValues(const std::map<juce::String, float>& values);
    std::map<juce::String, float> getCurrentValues() const;
    juce::File getPresetFile(const juce::String& presetName) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

} // namespace Neve1073
