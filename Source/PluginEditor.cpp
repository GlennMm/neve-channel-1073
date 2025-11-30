#include "PluginEditor.h"

namespace Neve1073
{

// Classic Neve colors
namespace NeveColors
{
    const juce::Colour neveBlue(0xFF344B7A);
    const juce::Colour neveBlueDark(0xFF263757);
    const juce::Colour neveBlueLight(0xFF4A6499);
    const juce::Colour panelDark(0xFF1E1E1E);
    const juce::Colour panelMid(0xFF2D2D2D);
    const juce::Colour panelLight(0xFF3D3D3D);
    const juce::Colour metalLight(0xFFC8C8C8);
    const juce::Colour metalMid(0xFFA0A0A0);
    const juce::Colour metalDark(0xFF707070);
    const juce::Colour cream(0xFFE8E0D0);
    const juce::Colour creamDark(0xFFD0C8B8);
    const juce::Colour knobSilver(0xFFB8B8B8);
    const juce::Colour indicatorRed(0xFFCC3333);
    const juce::Colour indicatorGreen(0xFF33AA33);
}

// Vintage Look and Feel
Neve1073Editor::VintageLookAndFeel::VintageLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, NeveColors::neveBlue);
    setColour(juce::Slider::thumbColourId, NeveColors::cream);
    setColour(juce::ComboBox::backgroundColourId, NeveColors::neveBlueDark);
    setColour(juce::ComboBox::textColourId, NeveColors::cream);
    setColour(juce::ComboBox::outlineColourId, NeveColors::neveBlueLight);
    setColour(juce::PopupMenu::backgroundColourId, NeveColors::panelDark);
    setColour(juce::PopupMenu::textColourId, NeveColors::cream);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, NeveColors::neveBlue);
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void Neve1073Editor::VintageLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 6.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Knob base shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(centreX - radius + 2, centreY - radius + 3, radius * 2.0f, radius * 2.0f);

    // Outer ring (chrome bezel)
    float bezelWidth = radius * 0.12f;
    juce::ColourGradient bezelGradient(
        NeveColors::metalLight, centreX - radius, centreY - radius,
        NeveColors::metalDark, centreX + radius, centreY + radius, false);
    g.setGradientFill(bezelGradient);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Inner knob body
    float innerRadius = radius - bezelWidth;

    // Knob gradient (3D effect)
    juce::ColourGradient knobGradient(
        NeveColors::panelLight, centreX, centreY - innerRadius,
        NeveColors::panelDark, centreX, centreY + innerRadius, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

    // Subtle radial texture on knob
    for (int i = 0; i < 36; ++i)
    {
        float tickAngle = (float)i * juce::MathConstants<float>::twoPi / 36.0f;
        float x1 = centreX + std::cos(tickAngle) * innerRadius * 0.3f;
        float y1 = centreY + std::sin(tickAngle) * innerRadius * 0.3f;
        float x2 = centreX + std::cos(tickAngle) * innerRadius * 0.85f;
        float y2 = centreY + std::sin(tickAngle) * innerRadius * 0.85f;
        g.setColour(juce::Colours::black.withAlpha(0.1f));
        g.drawLine(x1, y1, x2, y2, 0.5f);
    }

    // Position indicator (classic white line)
    juce::Path pointer;
    float pointerLength = innerRadius * 0.75f;
    float pointerWidth = 3.0f;
    pointer.addRoundedRectangle(-pointerWidth * 0.5f, -innerRadius + 4.0f,
                                 pointerWidth, pointerLength, 1.5f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    // Pointer shadow
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillPath(pointer, juce::AffineTransform::translation(1.0f, 1.0f));

    // Pointer fill
    g.setColour(NeveColors::cream);
    g.fillPath(pointer);

    // Center cap
    float capRadius = innerRadius * 0.25f;
    juce::ColourGradient capGradient(
        NeveColors::metalLight, centreX - capRadius, centreY - capRadius,
        NeveColors::metalDark, centreX + capRadius, centreY + capRadius, false);
    g.setGradientFill(capGradient);
    g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    // Draw tick marks around the knob
    g.setColour(NeveColors::cream.withAlpha(0.6f));
    int numTicks = 11;
    for (int i = 0; i < numTicks; ++i)
    {
        float tickAngle = rotaryStartAngle + (float)i / (float)(numTicks - 1) * (rotaryEndAngle - rotaryStartAngle);
        float tickInner = radius + 4.0f;
        float tickOuter = radius + 8.0f;
        float x1 = centreX + std::cos(tickAngle) * tickInner;
        float y1 = centreY + std::sin(tickAngle) * tickInner;
        float x2 = centreX + std::cos(tickAngle) * tickOuter;
        float y2 = centreY + std::sin(tickAngle) * tickOuter;

        // Thicker tick at center (0 position for bipolar)
        float thickness = (i == numTicks / 2) ? 2.0f : 1.0f;
        g.drawLine(x1, y1, x2, y2, thickness);
    }

    (void)slider; // Unused
}

void Neve1073Editor::VintageLookAndFeel::drawComboBox(
    juce::Graphics& g, int width, int height, bool isButtonDown,
    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);

    // Background with beveled edge effect
    juce::ColourGradient bgGradient(
        isButtonDown ? NeveColors::neveBlueDark : NeveColors::neveBlue,
        0, 0,
        NeveColors::neveBlueDark,
        0, (float)height, false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds, 3.0f);

    // Top highlight
    g.setColour(NeveColors::neveBlueLight.withAlpha(0.4f));
    g.drawHorizontalLine(1, 2.0f, (float)width - 2.0f);

    // Bottom shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawHorizontalLine(height - 2, 2.0f, (float)width - 2.0f);

    // Border
    g.setColour(NeveColors::neveBlueDark);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    // Arrow
    juce::Path arrow;
    float arrowX = (float)width - 15.0f;
    float arrowY = (float)height * 0.5f;
    arrow.addTriangle(arrowX - 4, arrowY - 3, arrowX + 4, arrowY - 3, arrowX, arrowY + 3);
    g.setColour(box.isEnabled() ? NeveColors::cream : NeveColors::cream.withAlpha(0.3f));
    g.fillPath(arrow);
}

void Neve1073Editor::VintageLookAndFeel::drawToggleButton(
    juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();

    // Button background
    juce::ColourGradient bgGradient(
        isOn ? NeveColors::neveBlue : NeveColors::panelMid,
        0, bounds.getY(),
        isOn ? NeveColors::neveBlueDark : NeveColors::panelDark,
        0, bounds.getBottom(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Highlight on hover
    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillRoundedRectangle(bounds, 4.0f);
    }

    // Top highlight edge
    g.setColour((isOn ? NeveColors::neveBlueLight : NeveColors::panelLight).withAlpha(0.4f));
    g.drawHorizontalLine((int)bounds.getY() + 1, bounds.getX() + 3, bounds.getRight() - 3);

    // Bottom shadow edge
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawHorizontalLine((int)bounds.getBottom() - 2, bounds.getX() + 3, bounds.getRight() - 3);

    // Border
    g.setColour(isOn ? NeveColors::neveBlueDark : juce::Colours::black.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // LED indicator
    float ledX = bounds.getX() + 8.0f;
    float ledY = bounds.getCentreY();
    float ledRadius = 4.0f;

    // LED glow when on
    if (isOn)
    {
        g.setColour(NeveColors::indicatorGreen.withAlpha(0.3f));
        g.fillEllipse(ledX - ledRadius - 2, ledY - ledRadius - 2, (ledRadius + 2) * 2, (ledRadius + 2) * 2);
    }

    // LED body
    g.setColour(isOn ? NeveColors::indicatorGreen : NeveColors::indicatorRed.darker(0.5f));
    g.fillEllipse(ledX - ledRadius, ledY - ledRadius, ledRadius * 2, ledRadius * 2);

    // LED highlight
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.fillEllipse(ledX - ledRadius + 1, ledY - ledRadius + 1, ledRadius, ledRadius);

    // Text
    g.setColour(NeveColors::cream);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    auto textBounds = bounds.withTrimmedLeft(20.0f);
    g.drawText(button.getButtonText(), textBounds, juce::Justification::centred);
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

    // Title - Classic Neve branding
    titleLabel.setText("NEVE 1073", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, NeveColors::cream);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Section labels with cream color on dark background
    setupLabel(inputLabel);
    setupLabel(preampLabel);
    setupLabel(lowLabel);
    setupLabel(midLabel);
    setupLabel(highLabel);
    setupLabel(hpfLabel);
    setupLabel(outputLabel);
    setupLabel(mixLabel);

    // EQ enable button with vintage styling
    eqEnabledButton.setColour(juce::ToggleButton::textColourId, NeveColors::cream);
    eqEnabledButton.setColour(juce::ToggleButton::tickColourId, NeveColors::neveBlue);
    eqEnabledButton.setColour(juce::ToggleButton::tickDisabledColourId, NeveColors::panelLight);
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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 16);
    slider.setColour(juce::Slider::textBoxTextColourId, NeveColors::cream);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, NeveColors::panelDark.withAlpha(0.5f));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setTextValueSuffix(suffix);
    addAndMakeVisible(slider);
}

void Neve1073Editor::setupComboBox(juce::ComboBox& combo)
{
    combo.setColour(juce::ComboBox::textColourId, NeveColors::cream);
    combo.setColour(juce::ComboBox::backgroundColourId, NeveColors::neveBlue);
    combo.setColour(juce::ComboBox::outlineColourId, NeveColors::neveBlueDark);
    addAndMakeVisible(combo);
}

void Neve1073Editor::setupLabel(juce::Label& label)
{
    label.setFont(juce::Font(11.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, NeveColors::cream);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void Neve1073Editor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Main background - dark brushed metal
    g.fillAll(NeveColors::panelDark);

    // Brushed metal texture (horizontal lines)
    juce::Random rng(12345);
    for (int y = 0; y < getHeight(); y += 2)
    {
        float alpha = 0.02f + rng.nextFloat() * 0.03f;
        g.setColour(juce::Colours::white.withAlpha(alpha));
        g.drawHorizontalLine(y, 0, (float)getWidth());
    }

    // Top header panel with Neve blue
    juce::Rectangle<float> headerBounds(0, 0, (float)getWidth(), 45.0f);
    juce::ColourGradient headerGradient(
        NeveColors::neveBlue, 0, 0,
        NeveColors::neveBlueDark, 0, 45, false);
    g.setGradientFill(headerGradient);
    g.fillRect(headerBounds);

    // Header beveled edges
    g.setColour(NeveColors::neveBlueLight.withAlpha(0.5f));
    g.drawHorizontalLine(0, 0, (float)getWidth());
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawHorizontalLine(44, 0, (float)getWidth());

    // Section panels with beveled edges
    int sectionWidth = (getWidth() - 60) / 8;
    int xOffset = 30;
    int panelY = 50;
    int panelHeight = getHeight() - 100;

    for (int i = 0; i < 8; ++i)
    {
        juce::Rectangle<float> sectionBounds(
            (float)(xOffset + i * sectionWidth + 3),
            (float)panelY,
            (float)(sectionWidth - 6),
            (float)panelHeight);

        // Section background
        juce::ColourGradient sectionGradient(
            NeveColors::panelMid, sectionBounds.getX(), sectionBounds.getY(),
            NeveColors::panelDark, sectionBounds.getX(), sectionBounds.getBottom(), false);
        g.setGradientFill(sectionGradient);
        g.fillRoundedRectangle(sectionBounds, 4.0f);

        // Inset shadow (top-left dark)
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawRoundedRectangle(sectionBounds, 4.0f, 1.0f);

        // Inner highlight (bottom-right light)
        g.setColour(NeveColors::panelLight.withAlpha(0.2f));
        g.drawLine(sectionBounds.getX() + 4, sectionBounds.getBottom() - 1,
                   sectionBounds.getRight() - 4, sectionBounds.getBottom() - 1, 1.0f);
    }

    // Bottom panel
    juce::Rectangle<float> bottomBounds(0, (float)(getHeight() - 48), (float)getWidth(), 48.0f);
    juce::ColourGradient bottomGradient(
        NeveColors::panelMid, 0, bottomBounds.getY(),
        NeveColors::panelDark, 0, (float)getHeight(), false);
    g.setGradientFill(bottomGradient);
    g.fillRect(bottomBounds);

    // Bottom panel top edge highlight
    g.setColour(NeveColors::panelLight.withAlpha(0.3f));
    g.drawHorizontalLine(getHeight() - 48, 0, (float)getWidth());

    // Bottom panel labels
    g.setColour(NeveColors::cream.withAlpha(0.7f));
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("OVERSAMPLING", 10, getHeight() - 43, 100, 14, juce::Justification::centredLeft);
    g.drawText("QUALITY", 200, getHeight() - 43, 80, 14, juce::Justification::centredLeft);

    // VU Meter labels
    g.setColour(NeveColors::cream.withAlpha(0.6f));
    g.setFont(juce::Font(9.0f));
    g.drawText("IN", 5, getHeight() - 78, 20, 12, juce::Justification::centred);
    g.drawText("OUT", getWidth() - 25, getHeight() - 78, 20, 12, juce::Justification::centred);

    // Decorative screws in corners
    auto drawScrew = [&](float cx, float cy) {
        float screwRadius = 5.0f;

        // Screw shadow
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(cx - screwRadius + 1, cy - screwRadius + 1, screwRadius * 2, screwRadius * 2);

        // Screw body
        juce::ColourGradient screwGradient(
            NeveColors::metalLight, cx - screwRadius, cy - screwRadius,
            NeveColors::metalDark, cx + screwRadius, cy + screwRadius, false);
        g.setGradientFill(screwGradient);
        g.fillEllipse(cx - screwRadius, cy - screwRadius, screwRadius * 2, screwRadius * 2);

        // Screw slot
        g.setColour(NeveColors::panelDark);
        g.drawLine(cx - 3, cy, cx + 3, cy, 1.5f);
    };

    drawScrew(12, 12);
    drawScrew((float)getWidth() - 12, 12);
    drawScrew(12, (float)getHeight() - 12);
    drawScrew((float)getWidth() - 12, (float)getHeight() - 12);

    (void)bounds; // Unused after initial assignment
}

void Neve1073Editor::resized()
{
    auto bounds = getLocalBounds();
    int sectionWidth = (bounds.getWidth() - 60) / 8;  // Account for meters
    int knobSize = 70;
    int knobY = 75;
    int comboHeight = 24;
    int meterWidth = 18;

    // Title - positioned in header
    titleLabel.setBounds(0, 10, bounds.getWidth(), 30);

    // Input VU meter (left edge)
    inputMeter.setBounds(6, 55, meterWidth, 260);

    // Section labels are positioned in the header panel
    int labelY = 55;
    int xOffset = 30;

    // Input section
    int section = 0;
    inputLabel.setBounds(xOffset + section * sectionWidth, labelY, sectionWidth, 18);
    inputGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 20, knobSize, knobSize + 20);
    inputDriveSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 115, knobSize, knobSize + 20);

    // Preamp section
    section = 1;
    preampLabel.setBounds(xOffset + section * sectionWidth, labelY, sectionWidth, 18);
    preampGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 20, knobSize, knobSize + 20);
    preampBiasSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 115, knobSize, knobSize + 20);

    // Low EQ section
    section = 2;
    lowLabel.setBounds(xOffset + section * sectionWidth, labelY, sectionWidth, 18);
    lowFreqCombo.setBounds(xOffset + section * sectionWidth + 10, knobY + 25, sectionWidth - 20, comboHeight);
    lowGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 60, knobSize, knobSize + 20);

    // Mid EQ section
    section = 3;
    midLabel.setBounds(xOffset + section * sectionWidth, labelY, sectionWidth, 18);
    midFreqCombo.setBounds(xOffset + section * sectionWidth + 10, knobY + 25, sectionWidth - 20, comboHeight);
    midGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 60, knobSize, knobSize + 20);

    // High EQ section
    section = 4;
    highLabel.setBounds(xOffset + section * sectionWidth, labelY, sectionWidth, 18);
    highGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 20, knobSize, knobSize + 20);
    eqEnabledButton.setBounds(xOffset + section * sectionWidth + (sectionWidth - 55) / 2, knobY + 120, 55, 28);

    // HPF section
    section = 5;
    hpfLabel.setBounds(xOffset + section * sectionWidth, labelY, sectionWidth, 18);
    hpfFreqCombo.setBounds(xOffset + section * sectionWidth + 10, knobY + 55, sectionWidth - 20, comboHeight);

    // Mix section
    section = 6;
    mixLabel.setBounds(xOffset + section * sectionWidth, labelY, sectionWidth, 18);
    mixSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 20, knobSize, knobSize + 20);

    // Output section
    section = 7;
    outputLabel.setBounds(xOffset + section * sectionWidth, labelY, sectionWidth, 18);
    outputGainSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 20, knobSize, knobSize + 20);
    outputDriveSlider.setBounds(xOffset + section * sectionWidth + (sectionWidth - knobSize) / 2, knobY + 115, knobSize, knobSize + 20);

    // Output VU meter (right edge)
    outputMeter.setBounds(bounds.getWidth() - 24, 55, meterWidth, 260);

    // Bottom panel controls
    oversamplingCombo.setBounds(10, getHeight() - 28, 110, 22);
    qualityCombo.setBounds(200, getHeight() - 28, 90, 22);
}

} // namespace Neve1073
