#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace Neve1073
{

/**
 * Neve 1073 Channel Strip Plugin Editor
 * Authentic hardware-style GUI matching the classic 500-series module
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

    // Custom look and feel for authentic Neve appearance
    class NeveLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        enum class KnobStyle { RedChickenHead, GrayChickenHead, RotarySelector, SmallKnob };

        NeveLookAndFeel();

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override;

        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                          int buttonX, int buttonY, int buttonW, int buttonH,
                          juce::ComboBox& box) override;

        void drawChickenHeadKnob(juce::Graphics& g, float cx, float cy, float radius,
                                 float angle, juce::Colour baseColor, bool isLarge);

        void drawRotarySelectorKnob(juce::Graphics& g, float cx, float cy, float radius,
                                    float angle, int numPositions, const juce::StringArray& labels);
    };

    NeveLookAndFeel neveLnF;

    // Gain controls - chicken-head style
    juce::Slider micGainSlider;      // Red chicken-head (MIC/LINE gain)
    juce::Slider lineGainSlider;     // Gray chicken-head (output/trim)

    // EQ Frequency selectors
    juce::Slider hfFreqSlider;       // HF frequency (stepped)
    juce::Slider midFreqSlider;      // MID frequency selector
    juce::Slider lfFreqSlider;       // LF frequency selector
    juce::Slider hpfFreqSlider;      // HPF frequency selector

    // EQ Gain controls - small gray knobs
    juce::Slider hfGainSlider;
    juce::Slider midGainSlider;
    juce::Slider lfGainSlider;

    // Buttons
    juce::ToggleButton eqlButton{"EQL"};
    juce::ToggleButton phaseButton{"PHASE"};  // Polarity (unused but for authenticity)

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpfFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqEnabledAttachment;

    // Additional controls for plugin features (bottom panel)
    juce::ComboBox presetCombo;
    juce::ComboBox oversamplingCombo;
    juce::ComboBox qualityCombo;
    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversamplingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> qualityAttachment;

    void setupRotarySlider(juce::Slider& slider, int numSteps = 0);
    void updatePresetList();
    void onPresetSelected();

    // Colors
    static inline const juce::Colour neveBlue{0xFF2B3D5B};
    static inline const juce::Colour neveBlueDark{0xFF1E2D45};
    static inline const juce::Colour neveBlueLight{0xFF3A5070};
    static inline const juce::Colour creamWhite{0xFFE8E4DC};
    static inline const juce::Colour redKnob{0xFFCC2020};
    static inline const juce::Colour grayKnob{0xFF808080};
    static inline const juce::Colour darkGray{0xFF404040};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Neve1073Editor)
};

} // namespace Neve1073
