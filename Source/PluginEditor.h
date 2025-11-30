#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace Neve1073
{

/**
 * Neve 1073 Channel Strip Plugin Editor
 *
 * Simple, clean GUI with rotary knobs for all parameters.
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

    // Labels
    juce::Label titleLabel;
    juce::Label inputLabel{"", "INPUT"};
    juce::Label preampLabel{"", "PREAMP"};
    juce::Label lowLabel{"", "LOW"};
    juce::Label midLabel{"", "MID"};
    juce::Label highLabel{"", "HIGH"};
    juce::Label hpfLabel{"", "HPF"};
    juce::Label outputLabel{"", "OUTPUT"};

    void setupSlider(juce::Slider& slider, const juce::String& suffix = "");
    void setupComboBox(juce::ComboBox& combo);
    void setupLabel(juce::Label& label);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Neve1073Editor)
};

} // namespace Neve1073
