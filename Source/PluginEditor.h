#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace Neve1073
{

/**
 * VU Meter Component
 */
class VUMeter : public juce::Component, public juce::Timer
{
public:
    VUMeter(Neve1073Processor& proc, bool isInput)
        : processor(proc), inputMeter(isInput)
    {
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(bounds, 2.0f);

        // Meter fill
        float levelDb = 20.0f * std::log10(std::max(0.0001f, currentLevel));
        float normalizedLevel = juce::jlimit(0.0f, 1.0f, (levelDb + 60.0f) / 60.0f);

        juce::ColourGradient gradient(
            juce::Colour(0xFF00AA00), 0, bounds.getBottom(),
            juce::Colour(0xFFFF0000), 0, bounds.getY(), false);
        gradient.addColour(0.7, juce::Colour(0xFFAAAA00));

        g.setGradientFill(gradient);
        float meterHeight = bounds.getHeight() * normalizedLevel;
        g.fillRoundedRectangle(bounds.getX() + 2, bounds.getBottom() - meterHeight,
                               bounds.getWidth() - 4, meterHeight, 1.0f);

        // Border
        g.setColour(juce::Colour(0xFF3A3A3A));
        g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
    }

    void timerCallback() override
    {
        float newLevel = inputMeter ? processor.getInputLevel() : processor.getOutputLevel();
        // Smooth decay
        if (newLevel > currentLevel)
            currentLevel = newLevel;
        else
            currentLevel = currentLevel * 0.9f + newLevel * 0.1f;
        repaint();
    }

private:
    Neve1073Processor& processor;
    bool inputMeter;
    float currentLevel = 0.0f;
};

/**
 * Neve 1073 Channel Strip Plugin Editor
 */
class Neve1073Editor : public juce::AudioProcessorEditor
{
public:
    explicit Neve1073Editor(Neve1073Processor& processor);
    ~Neve1073Editor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    Neve1073Processor& processorRef;

    // Custom look and feel for vintage appearance
    class VintageLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        VintageLookAndFeel();
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override;
        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                          int buttonX, int buttonY, int buttonW, int buttonH,
                          juce::ComboBox& box) override;
    };

    VintageLookAndFeel vintageLnF;

    // VU Meters
    VUMeter inputMeter;
    VUMeter outputMeter;

    // Input section
    juce::Slider inputGainSlider;
    juce::Slider inputDriveSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputDriveAttachment;

    // Preamp section
    juce::Slider preampGainSlider;
    juce::Slider preampBiasSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preampGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preampBiasAttachment;

    // Low EQ section
    juce::ComboBox lowFreqCombo;
    juce::Slider lowGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lowFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowGainAttachment;

    // Mid EQ section
    juce::ComboBox midFreqCombo;
    juce::Slider midGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> midFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midGainAttachment;

    // High EQ section
    juce::Slider highGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highGainAttachment;

    // HPF section
    juce::ComboBox hpfFreqCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> hpfFreqAttachment;

    // Output section
    juce::Slider outputGainSlider;
    juce::Slider outputDriveSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputDriveAttachment;

    // EQ enable
    juce::ToggleButton eqEnabledButton{"EQ"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqEnabledAttachment;

    // Mix control
    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    // Settings
    juce::ComboBox oversamplingCombo;
    juce::ComboBox qualityCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversamplingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> qualityAttachment;

    // Labels
    juce::Label titleLabel;
    juce::Label inputLabel{"", "INPUT"};
    juce::Label preampLabel{"", "PREAMP"};
    juce::Label lowLabel{"", "LOW"};
    juce::Label midLabel{"", "MID"};
    juce::Label highLabel{"", "HIGH"};
    juce::Label hpfLabel{"", "HPF"};
    juce::Label outputLabel{"", "OUTPUT"};
    juce::Label mixLabel{"", "MIX"};

    void setupSlider(juce::Slider& slider, const juce::String& suffix = "");
    void setupComboBox(juce::ComboBox& combo);
    void setupLabel(juce::Label& label);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Neve1073Editor)
};

} // namespace Neve1073
