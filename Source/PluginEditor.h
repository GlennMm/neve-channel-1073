#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace Neve1073
{

/**
 * VU Meter Component - Vintage style with LED segments
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
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        // Meter background (recessed look)
        g.setColour(juce::Colour(0xFF0A0A0A));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Inner shadow
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 2.0f, 1.0f);

        // Calculate level
        float levelDb = 20.0f * std::log10(std::max(0.0001f, currentLevel));
        float normalizedLevel = juce::jlimit(0.0f, 1.0f, (levelDb + 60.0f) / 60.0f);
        float peakNormalized = juce::jlimit(0.0f, 1.0f, (peakDb + 60.0f) / 60.0f);

        // LED segment style meter
        int numSegments = 20;
        float segmentHeight = (bounds.getHeight() - 4.0f) / numSegments;
        float segmentWidth = bounds.getWidth() - 6.0f;
        float startX = bounds.getX() + 3.0f;
        float startY = bounds.getY() + 2.0f;

        for (int i = 0; i < numSegments; ++i)
        {
            float segmentPos = 1.0f - (float)i / (float)numSegments;
            float y = startY + i * segmentHeight;

            // Determine segment color based on position
            juce::Colour segmentColor;
            if (segmentPos > 0.9f)
                segmentColor = juce::Colour(0xFFCC3333);  // Red (clip)
            else if (segmentPos > 0.75f)
                segmentColor = juce::Colour(0xFFCC9933);  // Orange (hot)
            else if (segmentPos > 0.5f)
                segmentColor = juce::Colour(0xFFCCCC33);  // Yellow
            else
                segmentColor = juce::Colour(0xFF33AA33);  // Green

            // Draw lit or unlit segment
            if (segmentPos <= normalizedLevel)
            {
                // Lit segment with glow
                g.setColour(segmentColor);
                g.fillRoundedRectangle(startX, y + 1, segmentWidth, segmentHeight - 2, 1.0f);

                // Subtle glow effect
                g.setColour(segmentColor.withAlpha(0.3f));
                g.fillRoundedRectangle(startX - 1, y, segmentWidth + 2, segmentHeight, 2.0f);
            }
            else
            {
                // Unlit segment (dim)
                g.setColour(segmentColor.withAlpha(0.15f));
                g.fillRoundedRectangle(startX, y + 1, segmentWidth, segmentHeight - 2, 1.0f);
            }

            // Peak hold indicator
            if (std::abs(segmentPos - peakNormalized) < (1.0f / numSegments) && peakNormalized > 0.01f)
            {
                g.setColour(juce::Colours::white.withAlpha(0.9f));
                g.fillRoundedRectangle(startX, y + 1, segmentWidth, segmentHeight - 2, 1.0f);
            }
        }

        // Outer bezel
        juce::ColourGradient bezelGradient(
            juce::Colour(0xFF3A3A3A), bounds.getX(), bounds.getY(),
            juce::Colour(0xFF1A1A1A), bounds.getRight(), bounds.getBottom(), false);
        g.setGradientFill(bezelGradient);
        g.drawRoundedRectangle(bounds, 3.0f, 2.0f);
    }

    void timerCallback() override
    {
        float newLevel = inputMeter ? processor.getInputLevel() : processor.getOutputLevel();

        // Smooth attack/decay
        if (newLevel > currentLevel)
            currentLevel = currentLevel * 0.3f + newLevel * 0.7f;  // Fast attack
        else
            currentLevel = currentLevel * 0.92f + newLevel * 0.08f;  // Slow decay

        // Peak hold
        float newDb = 20.0f * std::log10(std::max(0.0001f, newLevel));
        if (newDb > peakDb)
        {
            peakDb = newDb;
            peakHoldCounter = 30;  // Hold for 1 second at 30fps
        }
        else if (peakHoldCounter > 0)
        {
            peakHoldCounter--;
        }
        else
        {
            peakDb -= 1.5f;  // Decay peak indicator
        }

        repaint();
    }

private:
    Neve1073Processor& processor;
    bool inputMeter;
    float currentLevel = 0.0f;
    float peakDb = -60.0f;
    int peakHoldCounter = 0;
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
        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;
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

    // Preset controls
    juce::ComboBox presetCombo;
    juce::TextButton savePresetButton{"Save"};
    juce::TextButton deletePresetButton{"Del"};

    void setupSlider(juce::Slider& slider, const juce::String& suffix = "");
    void setupComboBox(juce::ComboBox& combo);
    void setupLabel(juce::Label& label);
    void updatePresetList();
    void onPresetSelected();
    void onSavePreset();
    void onDeletePreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Neve1073Editor)
};

} // namespace Neve1073
