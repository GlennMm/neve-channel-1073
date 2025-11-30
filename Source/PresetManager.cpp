#include "PresetManager.h"
#include "PluginProcessor.h"

namespace Neve1073
{

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : parameters(apvts)
{
    initializeFactoryPresets();
    refreshUserPresets();
}

void PresetManager::initializeFactoryPresets()
{
    // Default/Init preset - neutral settings
    factoryPresets.push_back({
        "Init",
        "Default",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 0.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 0.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 0.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.5f},
            {Neve1073Processor::PARAM_LOW_FREQ, 1.0f},
            {Neve1073Processor::PARAM_LOW_GAIN, 0.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 2.0f},
            {Neve1073Processor::PARAM_MID_GAIN, 0.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 0.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 0.0f},
            {Neve1073Processor::PARAM_OUTPUT_GAIN, 0.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 0.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 1.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Classic 1073 - the signature sound
    factoryPresets.push_back({
        "Classic 1073",
        "Character",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 3.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 4.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 6.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.55f},
            {Neve1073Processor::PARAM_LOW_FREQ, 1.0f},  // 60 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 3.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 2.0f},  // 1.6 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 2.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 4.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 1.0f},  // 50 Hz
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -3.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 2.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Warm Vocals - smooth and present
    factoryPresets.push_back({
        "Warm Vocals",
        "Vocals",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 0.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 2.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 4.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.52f},
            {Neve1073Processor::PARAM_LOW_FREQ, 1.0f},  // 60 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, -2.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 2.0f},  // 1.6 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 3.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 5.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 2.0f},  // 80 Hz
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -2.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 1.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Punchy Drums - attack and body
    factoryPresets.push_back({
        "Punchy Drums",
        "Drums",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 6.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 6.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 8.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.58f},
            {Neve1073Processor::PARAM_LOW_FREQ, 2.0f},  // 110 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 4.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 3.0f},  // 3.2 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 2.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 3.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 1.0f},  // 50 Hz
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -4.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 3.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Fat Bass - low end emphasis
    factoryPresets.push_back({
        "Fat Bass",
        "Bass",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 3.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 5.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 6.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.6f},
            {Neve1073Processor::PARAM_LOW_FREQ, 0.0f},  // 35 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 6.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 1.0f},  // 700 Hz
            {Neve1073Processor::PARAM_MID_GAIN, -2.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 2.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 0.0f},  // Off
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -3.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 4.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Bright Guitar - cut and presence
    factoryPresets.push_back({
        "Bright Guitar",
        "Guitar",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 2.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 3.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 5.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.53f},
            {Neve1073Processor::PARAM_LOW_FREQ, 2.0f},  // 110 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 2.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 4.0f},  // 4.8 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 4.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 6.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 2.0f},  // 80 Hz
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -2.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 2.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Analog Warmth - subtle saturation
    factoryPresets.push_back({
        "Analog Warmth",
        "Character",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 0.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 3.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 3.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.54f},
            {Neve1073Processor::PARAM_LOW_FREQ, 1.0f},  // 60 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 1.5f},
            {Neve1073Processor::PARAM_MID_FREQ, 2.0f},  // 1.6 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 0.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, -1.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 0.0f},  // Off
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -1.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 2.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 75.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 2.0f}
        }
    });

    // Aggressive Drive - heavy saturation
    factoryPresets.push_back({
        "Aggressive Drive",
        "Character",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 9.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 10.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 12.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.65f},
            {Neve1073Processor::PARAM_LOW_FREQ, 1.0f},  // 60 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 2.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 3.0f},  // 3.2 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 3.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 4.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 2.0f},  // 80 Hz
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -8.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 6.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 2.0f}
        }
    });

    // Mix Glue - parallel processing
    factoryPresets.push_back({
        "Mix Glue",
        "Mixing",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 2.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 4.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 4.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.52f},
            {Neve1073Processor::PARAM_LOW_FREQ, 0.0f},  // 35 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 2.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 2.0f},  // 1.6 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 1.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 2.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 0.0f},  // Off
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -2.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 2.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 50.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Kick Drum - focused low end
    factoryPresets.push_back({
        "Kick Drum",
        "Drums",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 4.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 5.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 6.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.55f},
            {Neve1073Processor::PARAM_LOW_FREQ, 1.0f},  // 60 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 5.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 3.0f},  // 3.2 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 3.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 2.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 0.0f},  // Off
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -3.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 3.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Snare Crack - midrange punch
    factoryPresets.push_back({
        "Snare Crack",
        "Drums",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 3.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 4.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 5.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.54f},
            {Neve1073Processor::PARAM_LOW_FREQ, 3.0f},  // 220 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, 3.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 3.0f},  // 3.2 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 5.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 4.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 2.0f},  // 80 Hz
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -3.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 2.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });

    // Airy Vocals - open and bright
    factoryPresets.push_back({
        "Airy Vocals",
        "Vocals",
        {
            {Neve1073Processor::PARAM_INPUT_GAIN, 0.0f},
            {Neve1073Processor::PARAM_INPUT_DRIVE, 1.0f},
            {Neve1073Processor::PARAM_PREAMP_GAIN, 2.0f},
            {Neve1073Processor::PARAM_PREAMP_BIAS, 0.5f},
            {Neve1073Processor::PARAM_LOW_FREQ, 2.0f},  // 110 Hz
            {Neve1073Processor::PARAM_LOW_GAIN, -3.0f},
            {Neve1073Processor::PARAM_MID_FREQ, 4.0f},  // 4.8 kHz
            {Neve1073Processor::PARAM_MID_GAIN, 4.0f},
            {Neve1073Processor::PARAM_HIGH_GAIN, 8.0f},
            {Neve1073Processor::PARAM_HPF_FREQ, 3.0f},  // 160 Hz
            {Neve1073Processor::PARAM_OUTPUT_GAIN, -2.0f},
            {Neve1073Processor::PARAM_OUTPUT_DRIVE, 0.0f},
            {Neve1073Processor::PARAM_EQ_ENABLED, 1.0f},
            {Neve1073Processor::PARAM_MIX, 100.0f},
            {Neve1073Processor::PARAM_OVERSAMPLING, 2.0f},
            {Neve1073Processor::PARAM_QUALITY, 1.0f}
        }
    });
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : factoryPresets)
        names.add(preset.name);
    names.addArray(userPresetNames);
    return names;
}

juce::StringArray PresetManager::getFactoryPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : factoryPresets)
        names.add(preset.name);
    return names;
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    return userPresetNames;
}

bool PresetManager::loadPreset(const juce::String& presetName)
{
    // Check factory presets first
    for (const auto& preset : factoryPresets)
    {
        if (preset.name == presetName)
        {
            applyPresetValues(preset.values);
            currentPresetName = presetName;
            return true;
        }
    }

    // Try to load user preset from file
    auto presetFile = getPresetFile(presetName);
    if (presetFile.existsAsFile())
    {
        auto xml = juce::XmlDocument::parse(presetFile);
        if (xml != nullptr && xml->hasTagName("Neve1073Preset"))
        {
            std::map<juce::String, float> values;
            for (auto* param : xml->getChildIterator())
            {
                if (param->hasTagName("Parameter"))
                {
                    auto id = param->getStringAttribute("id");
                    auto value = (float)param->getDoubleAttribute("value");
                    values[id] = value;
                }
            }
            applyPresetValues(values);
            currentPresetName = presetName;
            return true;
        }
    }

    return false;
}

bool PresetManager::saveUserPreset(const juce::String& presetName)
{
    auto presetDir = getUserPresetDirectory();
    if (!presetDir.exists())
        presetDir.createDirectory();

    auto presetFile = getPresetFile(presetName);
    auto values = getCurrentValues();

    juce::XmlElement xml("Neve1073Preset");
    xml.setAttribute("name", presetName);
    xml.setAttribute("version", "1.0");

    for (const auto& [id, value] : values)
    {
        auto* param = xml.createNewChildElement("Parameter");
        param->setAttribute("id", id);
        param->setAttribute("value", value);
    }

    if (xml.writeTo(presetFile))
    {
        currentPresetName = presetName;
        refreshUserPresets();
        return true;
    }

    return false;
}

bool PresetManager::deleteUserPreset(const juce::String& presetName)
{
    auto presetFile = getPresetFile(presetName);
    if (presetFile.existsAsFile())
    {
        presetFile.deleteFile();
        refreshUserPresets();
        return true;
    }
    return false;
}

void PresetManager::refreshUserPresets()
{
    userPresetNames.clear();
    auto presetDir = getUserPresetDirectory();

    if (presetDir.exists())
    {
        for (const auto& file : presetDir.findChildFiles(juce::File::findFiles, false, "*.xml"))
        {
            userPresetNames.add(file.getFileNameWithoutExtension());
        }
    }

    userPresetNames.sort(true);
}

juce::File PresetManager::getUserPresetDirectory() const
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
#if JUCE_MAC
    return appDataDir.getChildFile("Application Support/Neve1073/Presets");
#elif JUCE_WINDOWS
    return appDataDir.getChildFile("Neve1073/Presets");
#else
    return appDataDir.getChildFile(".neve1073/presets");
#endif
}

juce::File PresetManager::getPresetFile(const juce::String& presetName) const
{
    return getUserPresetDirectory().getChildFile(presetName + ".xml");
}

void PresetManager::applyPresetValues(const std::map<juce::String, float>& values)
{
    for (const auto& [id, value] : values)
    {
        if (auto* param = parameters.getParameter(id))
        {
            param->setValueNotifyingHost(param->convertTo0to1(value));
        }
    }
}

std::map<juce::String, float> PresetManager::getCurrentValues() const
{
    std::map<juce::String, float> values;

    auto paramIds = {
        Neve1073Processor::PARAM_INPUT_GAIN,
        Neve1073Processor::PARAM_INPUT_DRIVE,
        Neve1073Processor::PARAM_PREAMP_GAIN,
        Neve1073Processor::PARAM_PREAMP_BIAS,
        Neve1073Processor::PARAM_LOW_FREQ,
        Neve1073Processor::PARAM_LOW_GAIN,
        Neve1073Processor::PARAM_MID_FREQ,
        Neve1073Processor::PARAM_MID_GAIN,
        Neve1073Processor::PARAM_HIGH_GAIN,
        Neve1073Processor::PARAM_HPF_FREQ,
        Neve1073Processor::PARAM_OUTPUT_GAIN,
        Neve1073Processor::PARAM_OUTPUT_DRIVE,
        Neve1073Processor::PARAM_EQ_ENABLED,
        Neve1073Processor::PARAM_MIX,
        Neve1073Processor::PARAM_OVERSAMPLING,
        Neve1073Processor::PARAM_QUALITY
    };

    for (const auto& id : paramIds)
    {
        if (auto* param = parameters.getParameter(id))
        {
            values[id] = param->convertFrom0to1(param->getValue());
        }
    }

    return values;
}

bool PresetManager::isCurrentStateModified() const
{
    // Check if current values differ from the loaded preset
    auto currentValues = getCurrentValues();

    // Check factory presets
    for (const auto& preset : factoryPresets)
    {
        if (preset.name == currentPresetName)
        {
            for (const auto& [id, value] : preset.values)
            {
                auto it = currentValues.find(id);
                if (it != currentValues.end() && std::abs(it->second - value) > 0.001f)
                    return true;
            }
            return false;
        }
    }

    // For user presets, always assume modified if not a factory preset
    return true;
}

PresetManager::PresetInfo PresetManager::getPresetInfo(const juce::String& presetName) const
{
    for (const auto& preset : factoryPresets)
    {
        if (preset.name == presetName)
            return {preset.name, preset.category, "Factory", true};
    }

    return {presetName, "User", "User", false};
}

} // namespace Neve1073
