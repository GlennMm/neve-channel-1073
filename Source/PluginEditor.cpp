#include "PluginEditor.h"

namespace Neve1073
{

// Vintage Look and Feel
Neve1073Editor::VintageLookAndFeel::VintageLookAndFeel()
{
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

    juce::ColourGradient gradient(
        juce::Colour(0xFF4A4A4A), centreX, centreY - radius,
        juce::Colour(0xFF2A2A2A), centreX, centreY + radius, false);
    g.setGradientFill(gradient);
    g.fillEllipse(rx, ry, rw, rw);

    g.setColour(juce::Colour(0xFF1A1A1A));
    g.drawEllipse(rx, ry, rw, rw, 2.0f);

    juce::Path p;
    auto pointerLength = radius * 0.6f;
    auto pointerThickness = 3.0f;
    p.addRoundedRectangle(-pointerThickness * 0.5f, -radius + 6.0f,
                          pointerThickness, pointerLength, 1.0f);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(juce::Colour(0xFFE8E8E8));
    g.fillPath(p);

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
    , inputMeter(p, true)
    , outputMeter(p, false)
{
    setLookAndFeel(&vintageLnF);

    // VU Meters
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);

    // Setup sliders
    setupSlider(inputGainSlider, " dB");
    setupSlider(inputDriveSlider, " dB");
    setupSlider(preampGainSlider, " dB");
    setupSlider(preampBiasSlider);
    setupSlider(lowGainSlider, " dB");
    setupSlider(midGainSlider, " dB");
    setupSlider(highGainSlider, " dB");
    setupSlider(outputGainSlider, " dB");
    setupSlider(outputDriveSlider, " dB");
    setupSlider(mixSlider, " %");

    // Setup combo boxes
    lowFreqCombo.addItemList({"35 Hz", "60 Hz", "110 Hz", "220 Hz"}, 1);
    setupComboBox(lowFreqCombo);

    midFreqCombo.addItemList({"360 Hz", "700 Hz", "1.6 kHz", "3.2 kHz", "4.8 kHz", "7.2 kHz"}, 1);
    setupComboBox(midFreqCombo);

    hpfFreqCombo.addItemList({"Off", "50 Hz", "80 Hz", "160 Hz", "300 Hz"}, 1);
    setupComboBox(hpfFreqCombo);

    oversamplingCombo.addItemList({"1x (Off)", "2x", "4x", "8x"}, 1);
    setupComboBox(oversamplingCombo);

    qualityCombo.addItemList({"Eco", "Normal", "High"}, 1);
    setupComboBox(qualityCombo);

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
    setupLabel(mixLabel);

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

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_MIX, mixSlider);

    oversamplingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Neve1073Processor::PARAM_OVERSAMPLING, oversamplingCombo);

    qualityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Neve1073Processor::PARAM_QUALITY, qualityCombo);

    setSize(900, 420);
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
    int sectionWidth = (getWidth() - 60) / 8;  // Account for meters
    for (int i = 1; i < 8; ++i)
    {
        g.drawVerticalLine(30 + i * sectionWidth, 50.0f, (float)getHeight() - 50.0f);
    }

    // Bottom panel
    g.setColour(juce::Colour(0xFF222222));
    g.fillRect(0, getHeight() - 45, getWidth(), 45);

    // Bottom panel labels
    g.setColour(juce::Colour(0xFFAAAAAA));
    g.setFont(10.0f);
    g.drawText("OVERSAMPLING", 10, getHeight() - 40, 100, 15, juce::Justification::centredLeft);
    g.drawText("QUALITY", 200, getHeight() - 40, 80, 15, juce::Justification::centredLeft);
}

void Neve1073Editor::resized()
{
    auto bounds = getLocalBounds();
    int sectionWidth = (bounds.getWidth() - 60) / 8;  // Account for meters
    int knobSize = 65;
    int knobY = 95;
    int comboHeight = 22;
    int meterWidth = 15;

    // Title
    titleLabel.setBounds(0, 8, bounds.getWidth(), 30);

    // Input VU meter (left edge)
    inputMeter.setBounds(8, 70, meterWidth, 250);

    // Input section
    int section = 0;
    int xOffset = 30;
    inputLabel.setBounds(xOffset + section * sectionWidth, 45, sectionWidth, 20);
    inputGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 18);
    inputDriveSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 95, knobSize, knobSize + 18);

    // Preamp section
    section = 1;
    preampLabel.setBounds(xOffset + section * sectionWidth, 45, sectionWidth, 20);
    preampGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 18);
    preampBiasSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 95, knobSize, knobSize + 18);

    // Low EQ section
    section = 2;
    lowLabel.setBounds(xOffset + section * sectionWidth, 45, sectionWidth, 20);
    lowFreqCombo.setBounds(xOffset + section * sectionWidth + 8, knobY, sectionWidth - 16, comboHeight);
    lowGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 35, knobSize, knobSize + 18);

    // Mid EQ section
    section = 3;
    midLabel.setBounds(xOffset + section * sectionWidth, 45, sectionWidth, 20);
    midFreqCombo.setBounds(xOffset + section * sectionWidth + 8, knobY, sectionWidth - 16, comboHeight);
    midGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 35, knobSize, knobSize + 18);

    // High EQ section
    section = 4;
    highLabel.setBounds(xOffset + section * sectionWidth, 45, sectionWidth, 20);
    highGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 18);
    eqEnabledButton.setBounds(xOffset + section * sectionWidth + (sectionWidth - 50) / 2, knobY + 95, 50, 25);

    // HPF section
    section = 5;
    hpfLabel.setBounds(xOffset + section * sectionWidth, 45, sectionWidth, 20);
    hpfFreqCombo.setBounds(xOffset + section * sectionWidth + 8, knobY + 25, sectionWidth - 16, comboHeight);

    // Mix section
    section = 6;
    mixLabel.setBounds(xOffset + section * sectionWidth, 45, sectionWidth, 20);
    mixSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 18);

    // Output section
    section = 7;
    outputLabel.setBounds(xOffset + section * sectionWidth, 45, sectionWidth, 20);
    outputGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY, knobSize, knobSize + 18);
    outputDriveSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 95, knobSize, knobSize + 18);

    // Output VU meter (right edge)
    outputMeter.setBounds(bounds.getWidth() - 23, 70, meterWidth, 250);

    // Bottom panel controls
    oversamplingCombo.setBounds(10, getHeight() - 25, 100, 20);
    qualityCombo.setBounds(200, getHeight() - 25, 80, 20);
}

} // namespace Neve1073
