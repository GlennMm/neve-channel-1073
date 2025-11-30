#include "PluginEditor.h"

namespace Neve1073
{

// Vintage Look and Feel
Neve1073Editor::VintageLookAndFeel::VintageLookAndFeel()
{
    // Neve-inspired color scheme
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF2B5797));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xFFE8E8E8));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF3A3A3A));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xFFE8E8E8));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF5A5A5A));
}

void Neve1073Editor::VintageLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& /*slider*/)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Knob body (gradient)
    juce::ColourGradient gradient(
        juce::Colour(0xFF4A4A4A), centreX, centreY - radius,
        juce::Colour(0xFF2A2A2A), centreX, centreY + radius, false);
    g.setGradientFill(gradient);
    g.fillEllipse(rx, ry, rw, rw);

    // Knob outline
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.drawEllipse(rx, ry, rw, rw, 2.0f);

    // Pointer
    juce::Path p;
    auto pointerLength = radius * 0.6f;
    auto pointerThickness = 3.0f;
    p.addRoundedRectangle(-pointerThickness * 0.5f, -radius + 6.0f,
                          pointerThickness, pointerLength, 1.0f);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(juce::Colour(0xFFE8E8E8));
    g.fillPath(p);

    // Indicator ring (position)
    g.setColour(juce::Colour(0xFF2B5797));
    juce::Path arcPath;
    arcPath.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f,
                          0.0f, rotaryStartAngle, angle, true);
    g.strokePath(arcPath, juce::PathStrokeType(3.0f));
}

void Neve1073Editor::VintageLookAndFeel::drawComboBox(
    juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
    juce::ComboBox& box)
{
    auto cornerSize = 4.0f;
    juce::Rectangle<int> boxBounds(0, 0, width, height);

    g.setColour(findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

    g.setColour(findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

    // Arrow
    juce::Rectangle<int> arrowZone(width - 20, 0, 15, height);
    juce::Path path;
    path.addTriangle((float)arrowZone.getX() + 3.0f, (float)arrowZone.getCentreY() - 2.0f,
                     (float)arrowZone.getCentreX(), (float)arrowZone.getCentreY() + 3.0f,
                     (float)arrowZone.getRight() - 3.0f, (float)arrowZone.getCentreY() - 2.0f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(box.isEnabled() ? 0.9f : 0.2f));
    g.fillPath(path);
}

// Editor implementation
Neve1073Editor::Neve1073Editor(Neve1073Processor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
{
    setLookAndFeel(&vintageLnF);

    // Setup all components
    setupSlider(inputGainSlider, " dB");
    setupSlider(inputDriveSlider, " dB");
    setupSlider(preampGainSlider, " dB");
    setupSlider(preampBiasSlider);
    setupSlider(lowGainSlider, " dB");
    setupSlider(midGainSlider, " dB");
    setupSlider(highGainSlider, " dB");
    setupSlider(outputGainSlider, " dB");
    setupSlider(outputDriveSlider, " dB");

    setupComboBox(lowFreqCombo);
    setupComboBox(midFreqCombo);
    setupComboBox(hpfFreqCombo);

    // Title
    titleLabel.setText("NEVE 1073", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF2B5797));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Section labels
    setupLabel(inputLabel);
    setupLabel(preampLabel);
    setupLabel(lowLabel);
    setupLabel(midLabel);
    setupLabel(highLabel);
    setupLabel(hpfLabel);
    setupLabel(outputLabel);

    // EQ enable button
    eqEnabledButton.setColour(juce::ToggleButton::textColourId, juce::Colour(0xFFE8E8E8));
    eqEnabledButton.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFF2B5797));
    addAndMakeVisible(eqEnabledButton);

    // Create parameter attachments
    auto& params = processorRef.getParameters();

    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_INPUT_GAIN, inputGainSlider);
    inputDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_INPUT_DRIVE, inputDriveSlider);

    preampGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_PREAMP_GAIN, preampGainSlider);
    preampBiasAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_PREAMP_BIAS, preampBiasSlider);

    lowFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Neve1073Processor::PARAM_LOW_FREQ, lowFreqCombo);
    lowGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_LOW_GAIN, lowGainSlider);

    midFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Neve1073Processor::PARAM_MID_FREQ, midFreqCombo);
    midGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_MID_GAIN, midGainSlider);

    highGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_HIGH_GAIN, highGainSlider);

    hpfFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Neve1073Processor::PARAM_HPF_FREQ, hpfFreqCombo);

    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_OUTPUT_GAIN, outputGainSlider);
    outputDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_OUTPUT_DRIVE, outputDriveSlider);

    eqEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, Neve1073Processor::PARAM_EQ_ENABLED, eqEnabledButton);

    setSize(800, 400);
}

Neve1073Editor::~Neve1073Editor()
{
    setLookAndFeel(nullptr);
}

void Neve1073Editor::setupSlider(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFE8E8E8));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setTextValueSuffix(suffix);
    addAndMakeVisible(slider);
}

void Neve1073Editor::setupComboBox(juce::ComboBox& combo)
{
    combo.setColour(juce::ComboBox::textColourId, juce::Colour(0xFFE8E8E8));
    addAndMakeVisible(combo);
}

void Neve1073Editor::setupLabel(juce::Label& label)
{
    label.setFont(juce::Font(12.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void Neve1073Editor::paint(juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient bgGradient(
        juce::Colour(0xFF2A2A2A), 0.0f, 0.0f,
        juce::Colour(0xFF1A1A1A), 0.0f, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Section dividers
    g.setColour(juce::Colour(0xFF3A3A3A));

    int sectionWidth = getWidth() / 7;
    for (int i = 1; i < 7; ++i)
    {
        g.drawVerticalLine(i * sectionWidth, 50.0f, (float)getHeight() - 10.0f);
    }

    // Bottom panel
    g.setColour(juce::Colour(0xFF222222));
    g.fillRect(0, getHeight() - 30, getWidth(), 30);
}

void Neve1073Editor::resized()
{
    auto bounds = getLocalBounds();
    int sectionWidth = bounds.getWidth() / 7;
    int knobSize = 70;
    int knobY = 100;
    int comboHeight = 24;

    // Title
    titleLabel.setBounds(0, 10, bounds.getWidth(), 30);

    // Input section
    int section = 0;
    inputLabel.setBounds(section * sectionWidth, 45, sectionWidth, 20);
    inputGainSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 20);
    inputDriveSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 100, knobSize, knobSize + 20);

    // Preamp section
    section = 1;
    preampLabel.setBounds(section * sectionWidth, 45, sectionWidth, 20);
    preampGainSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 20);
    preampBiasSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 100, knobSize, knobSize + 20);

    // Low EQ section
    section = 2;
    lowLabel.setBounds(section * sectionWidth, 45, sectionWidth, 20);
    lowFreqCombo.setBounds(section * sectionWidth + 10, knobY, sectionWidth - 20, comboHeight);
    lowGainSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 40, knobSize, knobSize + 20);

    // Mid EQ section
    section = 3;
    midLabel.setBounds(section * sectionWidth, 45, sectionWidth, 20);
    midFreqCombo.setBounds(section * sectionWidth + 10, knobY, sectionWidth - 20, comboHeight);
    midGainSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 40, knobSize, knobSize + 20);

    // High EQ section
    section = 4;
    highLabel.setBounds(section * sectionWidth, 45, sectionWidth, 20);
    highGainSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 20);
    eqEnabledButton.setBounds(section * sectionWidth + (sectionWidth - 60) / 2, knobY + 100, 60, 30);

    // HPF section
    section = 5;
    hpfLabel.setBounds(section * sectionWidth, 45, sectionWidth, 20);
    hpfFreqCombo.setBounds(section * sectionWidth + 10, knobY + 30, sectionWidth - 20, comboHeight);

    // Output section
    section = 6;
    outputLabel.setBounds(section * sectionWidth, 45, sectionWidth, 20);
    outputGainSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 20);
    outputDriveSlider.setBounds(section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 100, knobSize, knobSize + 20);
}

} // namespace Neve1073
