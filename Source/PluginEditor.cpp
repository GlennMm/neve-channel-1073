#include "PluginEditor.h"

namespace Neve1073
{

// Neve Look and Feel Implementation
Neve1073Editor::NeveLookAndFeel::NeveLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1E2D45));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xFFE8E4DC));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF3A5070));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF1E2D45));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xFFE8E4DC));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF2B3D5B));
}

void Neve1073Editor::NeveLookAndFeel::drawChickenHeadKnob(
    juce::Graphics& g, float cx, float cy, float radius,
    float angle, juce::Colour baseColor, bool /*isLarge*/)
{
    // Shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(cx - radius + 3, cy - radius + 4, radius * 2, radius * 2);

    // Outer rim (darker)
    g.setColour(baseColor.darker(0.4f));
    g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);

    // Main knob body with gradient
    float innerRadius = radius * 0.9f;
    juce::ColourGradient knobGradient(
        baseColor.brighter(0.3f), cx - innerRadius * 0.3f, cy - innerRadius * 0.3f,
        baseColor.darker(0.3f), cx + innerRadius * 0.3f, cy + innerRadius * 0.3f, true);
    g.setGradientFill(knobGradient);
    g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2, innerRadius * 2);

    // Highlight arc (top)
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    juce::Path highlight;
    highlight.addArc(cx - innerRadius + 3, cy - innerRadius + 3,
                     (innerRadius - 3) * 2, (innerRadius - 3) * 2,
                     -juce::MathConstants<float>::pi * 0.8f,
                     -juce::MathConstants<float>::pi * 0.2f, true);
    g.strokePath(highlight, juce::PathStrokeType(3.0f));

    // Pointer (chicken head shape) - white/cream colored
    juce::Path pointer;
    float pointerLength = radius * 0.75f;
    float pointerBaseWidth = radius * 0.4f;
    float pointerTipWidth = radius * 0.15f;

    // Tapered pointer shape
    pointer.startNewSubPath(0, -pointerLength);  // Tip
    pointer.lineTo(-pointerTipWidth, -pointerLength * 0.7f);
    pointer.lineTo(-pointerBaseWidth * 0.5f, -pointerLength * 0.2f);
    pointer.lineTo(-pointerBaseWidth * 0.3f, pointerLength * 0.15f);
    pointer.lineTo(pointerBaseWidth * 0.3f, pointerLength * 0.15f);
    pointer.lineTo(pointerBaseWidth * 0.5f, -pointerLength * 0.2f);
    pointer.lineTo(pointerTipWidth, -pointerLength * 0.7f);
    pointer.closeSubPath();

    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(cx, cy));

    // Pointer shadow
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillPath(pointer, juce::AffineTransform::translation(1.5f, 1.5f));

    // Pointer fill
    g.setColour(juce::Colour(0xFFF0EBE0));
    g.fillPath(pointer);

    // Pointer edge highlight
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.strokePath(pointer, juce::PathStrokeType(0.5f));
}

void Neve1073Editor::NeveLookAndFeel::drawRotarySelectorKnob(
    juce::Graphics& g, float cx, float cy, float radius,
    float angle, int /*numPositions*/, const juce::StringArray& /*labels*/)
{
    // Shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(cx - radius + 3, cy - radius + 4, radius * 2, radius * 2);

    // Outer chrome ring
    juce::ColourGradient chromeGradient(
        juce::Colour(0xFFD8D8D8), cx - radius, cy - radius,
        juce::Colour(0xFF505050), cx + radius, cy + radius, false);
    g.setGradientFill(chromeGradient);
    g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);

    // Inner knob body (dark gray with texture)
    float innerRadius = radius * 0.82f;
    juce::ColourGradient knobGradient(
        juce::Colour(0xFF5A5A5A), cx, cy - innerRadius,
        juce::Colour(0xFF2A2A2A), cx, cy + innerRadius, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2, innerRadius * 2);

    // Knurled texture (radial lines)
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    for (int i = 0; i < 36; ++i)
    {
        float tickAngle = (float)i * juce::MathConstants<float>::twoPi / 36.0f;
        float x1 = cx + std::cos(tickAngle) * innerRadius * 0.35f;
        float y1 = cy + std::sin(tickAngle) * innerRadius * 0.35f;
        float x2 = cx + std::cos(tickAngle) * innerRadius * 0.92f;
        float y2 = cy + std::sin(tickAngle) * innerRadius * 0.92f;
        g.drawLine(x1, y1, x2, y2, 1.0f);
    }

    // White pointer line
    juce::Path pointer;
    pointer.addRectangle(-2.5f, -innerRadius * 0.92f, 5.0f, innerRadius * 0.55f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(cx, cy));

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillPath(pointer, juce::AffineTransform::translation(1.0f, 1.0f));

    g.setColour(juce::Colour(0xFFF0EBE0));
    g.fillPath(pointer);

    // Center cap
    float capRadius = innerRadius * 0.22f;
    juce::ColourGradient capGradient(
        juce::Colour(0xFF808080), cx - capRadius, cy - capRadius,
        juce::Colour(0xFF404040), cx + capRadius, cy + capRadius, false);
    g.setGradientFill(capGradient);
    g.fillEllipse(cx - capRadius, cy - capRadius, capRadius * 2, capRadius * 2);
}

void Neve1073Editor::NeveLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::String name = slider.getName();

    if (name.contains("MicGain"))
    {
        drawChickenHeadKnob(g, centreX, centreY, radius, angle, juce::Colour(0xFFBB2222), true);
    }
    else if (name.contains("LineGain"))
    {
        drawChickenHeadKnob(g, centreX, centreY, radius, angle, juce::Colour(0xFF606060), true);
    }
    else if (name.contains("Freq") || name.contains("HPF"))
    {
        drawRotarySelectorKnob(g, centreX, centreY, radius, angle, 6, {});
    }
    else
    {
        // Small gain knobs
        drawChickenHeadKnob(g, centreX, centreY, radius, angle, juce::Colour(0xFF505050), false);
    }
}

void Neve1073Editor::NeveLookAndFeel::drawToggleButton(
    juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    bool isOn = button.getToggleState();

    // Button background
    g.setColour(juce::Colour(0xFF151515));
    g.fillRoundedRectangle(bounds, 4.0f);

    auto innerBounds = bounds.reduced(2.0f);
    g.setColour(isOn ? juce::Colour(0xFF2A3A50) : juce::Colour(0xFF202020));
    g.fillRoundedRectangle(innerBounds, 3.0f);

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillRoundedRectangle(innerBounds, 3.0f);
    }

    // LED indicator
    float ledRadius = 5.0f;
    float ledX = bounds.getCentreX();
    float ledY = bounds.getY() + 12.0f;

    if (isOn)
    {
        g.setColour(juce::Colour(0xFF00FF00).withAlpha(0.4f));
        g.fillEllipse(ledX - ledRadius - 4, ledY - ledRadius - 4,
                      (ledRadius + 4) * 2, (ledRadius + 4) * 2);
    }

    g.setColour(isOn ? juce::Colour(0xFF00DD00) : juce::Colour(0xFF2A2A2A));
    g.fillEllipse(ledX - ledRadius, ledY - ledRadius, ledRadius * 2, ledRadius * 2);

    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.fillEllipse(ledX - ledRadius + 1.5f, ledY - ledRadius + 1.5f, ledRadius * 0.7f, ledRadius * 0.7f);

    // Text
    g.setColour(juce::Colour(0xFFE8E4DC));
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), bounds.withTrimmedTop(22.0f),
               juce::Justification::centred);
}

void Neve1073Editor::NeveLookAndFeel::drawComboBox(
    juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);

    g.setColour(juce::Colour(0xFF1A2535));
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(juce::Colour(0xFF3A5070));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    juce::Path arrow;
    float arrowX = (float)width - 10.0f;
    float arrowY = (float)height * 0.5f;
    arrow.addTriangle(arrowX - 3, arrowY - 2, arrowX + 3, arrowY - 2, arrowX, arrowY + 2);
    g.setColour(box.isEnabled() ? juce::Colour(0xFFE8E4DC) : juce::Colours::grey);
    g.fillPath(arrow);
}

// Editor Implementation
Neve1073Editor::Neve1073Editor(Neve1073Processor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
{
    setLookAndFeel(&neveLnF);

    // Setup all sliders
    micGainSlider.setName("MicGain");
    setupRotarySlider(micGainSlider);

    lineGainSlider.setName("LineGain");
    setupRotarySlider(lineGainSlider);

    hfGainSlider.setName("HFGain");
    setupRotarySlider(hfGainSlider);

    midFreqSlider.setName("MidFreq");
    setupRotarySlider(midFreqSlider, 6);

    midGainSlider.setName("MidGain");
    setupRotarySlider(midGainSlider);

    lfFreqSlider.setName("LFFreq");
    setupRotarySlider(lfFreqSlider, 4);

    lfGainSlider.setName("LFGain");
    setupRotarySlider(lfGainSlider);

    hpfFreqSlider.setName("HPFFreq");
    setupRotarySlider(hpfFreqSlider, 5);

    addAndMakeVisible(eqlButton);
    addAndMakeVisible(phaseButton);

    mixSlider.setName("Mix");
    setupRotarySlider(mixSlider);

    oversamplingCombo.addItemList({"1x", "2x", "4x", "8x"}, 1);
    addAndMakeVisible(oversamplingCombo);

    qualityCombo.addItemList({"Eco", "Normal", "High"}, 1);
    addAndMakeVisible(qualityCombo);

    presetCombo.onChange = [this] { onPresetSelected(); };
    addAndMakeVisible(presetCombo);
    updatePresetList();

    // Parameter attachments
    auto& params = processorRef.getParameters();

    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_INPUT_GAIN, micGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_OUTPUT_GAIN, lineGainSlider);
    highGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_HIGH_GAIN, hfGainSlider);
    midFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_MID_FREQ, midFreqSlider);
    midGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_MID_GAIN, midGainSlider);
    lowFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_LOW_FREQ, lfFreqSlider);
    lowGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_LOW_GAIN, lfGainSlider);
    hpfFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_HPF_FREQ, hpfFreqSlider);
    eqEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, Neve1073Processor::PARAM_EQ_ENABLED, eqlButton);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Neve1073Processor::PARAM_MIX, mixSlider);
    oversamplingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Neve1073Processor::PARAM_OVERSAMPLING, oversamplingCombo);
    qualityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Neve1073Processor::PARAM_QUALITY, qualityCombo);

    setSize(240, 720);
}

Neve1073Editor::~Neve1073Editor()
{
    setLookAndFeel(nullptr);
}

void Neve1073Editor::setupRotarySlider(juce::Slider& slider, int numSteps)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    if (numSteps > 0)
        slider.setRange(0, numSteps - 1, 1);
    addAndMakeVisible(slider);
}

void Neve1073Editor::paint(juce::Graphics& g)
{
    // Main background
    g.fillAll(neveBlue);

    // Subtle noise texture
    juce::Random rng(42);
    for (int i = 0; i < 800; ++i)
    {
        float x = rng.nextFloat() * (float)getWidth();
        float y = rng.nextFloat() * (float)getHeight();
        g.setColour(juce::Colours::white.withAlpha(rng.nextFloat() * 0.015f));
        g.fillRect(x, y, 1.0f, 1.0f);
    }

    float cx = getWidth() * 0.5f;
    g.setColour(creamWhite);

    // === INPUT GAIN SECTION ===
    float inputKnobY = 75.0f;
    float inputRadius = 58.0f;

    // "dB" label top-left
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("dB", 15, 30, 25, 14, juce::Justification::centred);

    // "LINE" label top-right
    g.drawText("LINE", getWidth() - 45, 30, 35, 14, juce::Justification::centred);

    // Input gain markings in arc
    g.setFont(juce::Font(9.0f));
    juce::StringArray inputMarks = {"80", "70", "60", "50", "40", "30", "20", "10", "0", "-10", "-20"};
    float startAngle = -2.5f;
    float endAngle = 2.5f;
    for (int i = 0; i < inputMarks.size(); ++i)
    {
        float t = (float)i / (float)(inputMarks.size() - 1);
        float angle = startAngle + t * (endAngle - startAngle);
        float tx = cx + std::cos(angle - juce::MathConstants<float>::halfPi) * inputRadius;
        float ty = inputKnobY + std::sin(angle - juce::MathConstants<float>::halfPi) * inputRadius;
        g.drawText(inputMarks[i], (int)tx - 15, (int)ty - 6, 30, 12, juce::Justification::centred);
    }

    // "MIC." label left side
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("MIC.", 8, (int)inputKnobY + 30, 35, 14, juce::Justification::centred);

    // "OFF" label below input knob
    g.setFont(juce::Font(9.0f));
    g.drawText("OFF", (int)cx - 15, (int)inputKnobY + 48, 30, 12, juce::Justification::centred);

    // === OUTPUT GAIN SECTION ===
    float outputKnobY = 175.0f;
    float outputRadius = 52.0f;

    g.drawText("-20", 18, (int)outputKnobY - 8, 28, 12, juce::Justification::centred);
    g.drawText("+20", getWidth() - 48, (int)outputKnobY - 8, 28, 12, juce::Justification::centred);
    g.drawText("OFF", (int)cx - 15, (int)outputKnobY + 42, 30, 12, juce::Justification::centred);

    // Divider line
    g.setColour(creamWhite.withAlpha(0.25f));
    g.drawHorizontalLine(235, 15, (float)getWidth() - 15);

    // === HF SECTION ===
    g.setColour(creamWhite);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("HF", (int)cx - 15, 248, 30, 14, juce::Justification::centred);
    g.setFont(juce::Font(9.0f));
    g.drawText("12kHz", (int)cx - 22, 262, 44, 12, juce::Justification::centred);

    // HF gain knob markings
    float hfKnobY = 305.0f;
    g.setFont(juce::Font(8.0f));
    g.drawText("+16", 20, (int)hfKnobY - 15, 25, 12, juce::Justification::centred);
    g.drawText("-16", getWidth() - 48, (int)hfKnobY - 15, 25, 12, juce::Justification::centred);
    g.drawText("0", (int)cx - 10, (int)hfKnobY + 38, 20, 12, juce::Justification::centred);

    // Divider
    g.setColour(creamWhite.withAlpha(0.25f));
    g.drawHorizontalLine(355, 15, (float)getWidth() - 15);

    // === MID SECTION ===
    g.setColour(creamWhite);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("MID", (int)cx - 18, 365, 36, 14, juce::Justification::centred);

    // Mid frequency selector markings
    float midKnobY = 420.0f;
    float midRadius = 52.0f;
    g.setFont(juce::Font(8.0f));
    juce::StringArray midFreqs = {"7.2", "4.8", "3.2", "1.6", ".7", ".36"};
    for (int i = 0; i < midFreqs.size(); ++i)
    {
        float t = (float)i / (float)(midFreqs.size() - 1);
        float angle = startAngle + t * (endAngle - startAngle);
        float tx = cx + std::cos(angle - juce::MathConstants<float>::halfPi) * midRadius;
        float ty = midKnobY + std::sin(angle - juce::MathConstants<float>::halfPi) * midRadius;
        g.drawText(midFreqs[i], (int)tx - 14, (int)ty - 5, 28, 10, juce::Justification::centred);
    }
    g.drawText("kHz", (int)cx - 14, (int)midKnobY - 6, 28, 10, juce::Justification::centred);

    // Divider
    g.setColour(creamWhite.withAlpha(0.25f));
    g.drawHorizontalLine(475, 15, (float)getWidth() - 15);

    // === LF SECTION ===
    g.setColour(creamWhite);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("LF", (int)cx - 12, 485, 24, 14, juce::Justification::centred);

    // LF frequency selector markings
    float lfKnobY = 535.0f;
    float lfRadius = 50.0f;
    g.setFont(juce::Font(8.0f));
    juce::StringArray lfFreqs = {"220", "110", "60", "35"};
    for (int i = 0; i < lfFreqs.size(); ++i)
    {
        float t = (float)i / (float)(lfFreqs.size() - 1);
        float angle = startAngle + t * (endAngle - startAngle);
        float tx = cx + std::cos(angle - juce::MathConstants<float>::halfPi) * lfRadius;
        float ty = lfKnobY + std::sin(angle - juce::MathConstants<float>::halfPi) * lfRadius;
        g.drawText(lfFreqs[i], (int)tx - 16, (int)ty - 5, 32, 10, juce::Justification::centred);
    }
    g.drawText("Hz", (int)cx - 10, (int)lfKnobY - 6, 20, 10, juce::Justification::centred);

    // Divider
    g.setColour(creamWhite.withAlpha(0.25f));
    g.drawHorizontalLine(585, 15, (float)getWidth() - 15);

    // === HPF SECTION ===
    g.setColour(creamWhite);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("HPF", (int)cx - 18, 592, 36, 14, juce::Justification::centred);

    // HPF frequency selector markings
    float hpfKnobY = 640.0f;
    float hpfRadius = 48.0f;
    g.setFont(juce::Font(8.0f));
    juce::StringArray hpfFreqs = {"OFF", "50", "80", "160", "300"};
    for (int i = 0; i < hpfFreqs.size(); ++i)
    {
        float t = (float)i / (float)(hpfFreqs.size() - 1);
        float angle = startAngle + t * (endAngle - startAngle);
        float tx = cx + std::cos(angle - juce::MathConstants<float>::halfPi) * hpfRadius;
        float ty = hpfKnobY + std::sin(angle - juce::MathConstants<float>::halfPi) * hpfRadius;
        g.drawText(hpfFreqs[i], (int)tx - 18, (int)ty - 5, 36, 10, juce::Justification::centred);
    }

    // === BOTTOM PANEL ===
    g.setColour(neveBlueDark);
    g.fillRect(0, getHeight() - 35, getWidth(), 35);
    g.setColour(neveBlueLight.withAlpha(0.3f));
    g.drawHorizontalLine(getHeight() - 35, 0, (float)getWidth());

    // Corner screws
    auto drawScrew = [&](float sx, float sy) {
        g.setColour(juce::Colour(0xFF353535));
        g.fillEllipse(sx - 6, sy - 6, 12, 12);
        g.setColour(juce::Colour(0xFF555555));
        g.drawEllipse(sx - 5, sy - 5, 10, 10, 1.0f);
        g.setColour(juce::Colour(0xFF252525));
        g.drawLine(sx - 3.5f, sy, sx + 3.5f, sy, 1.5f);
    };

    drawScrew(12, 12);
    drawScrew((float)getWidth() - 12, 12);
}

void Neve1073Editor::resized()
{
    int cx = getWidth() / 2;

    // Input gain knob
    micGainSlider.setBounds(cx - 40, 35, 80, 80);

    // Output gain knob
    lineGainSlider.setBounds(cx - 35, 140, 70, 70);

    // HF gain knob
    hfGainSlider.setBounds(cx - 30, 275, 60, 60);

    // MID section
    midFreqSlider.setBounds(cx - 35, 385, 70, 70);
    midGainSlider.setBounds(getWidth() - 55, 400, 45, 45);

    // LF section
    lfFreqSlider.setBounds(cx - 35, 500, 70, 70);
    lfGainSlider.setBounds(getWidth() - 55, 515, 45, 45);

    // HPF selector
    hpfFreqSlider.setBounds(cx - 35, 605, 70, 70);

    // Buttons
    eqlButton.setBounds(15, getHeight() - 80, 50, 40);
    phaseButton.setBounds(70, getHeight() - 80, 60, 40);

    // Mix knob
    mixSlider.setBounds(getWidth() - 55, getHeight() - 80, 45, 45);

    // Bottom controls
    presetCombo.setBounds(10, getHeight() - 28, 100, 20);
    oversamplingCombo.setBounds(115, getHeight() - 28, 45, 20);
    qualityCombo.setBounds(165, getHeight() - 28, 55, 20);
}

void Neve1073Editor::updatePresetList()
{
    auto& pm = processorRef.getPresetManager();
    presetCombo.clear(juce::dontSendNotification);

    auto factoryPresets = pm.getFactoryPresetNames();
    int itemId = 1;
    for (const auto& name : factoryPresets)
        presetCombo.addItem(name, itemId++);

    auto userPresets = pm.getUserPresetNames();
    if (!userPresets.isEmpty())
    {
        presetCombo.addSeparator();
        for (const auto& name : userPresets)
            presetCombo.addItem(name, itemId++);
    }

    auto currentPreset = pm.getCurrentPresetName();
    for (int i = 0; i < presetCombo.getNumItems(); ++i)
    {
        if (presetCombo.getItemText(i) == currentPreset)
        {
            presetCombo.setSelectedItemIndex(i, juce::dontSendNotification);
            break;
        }
    }
}

void Neve1073Editor::onPresetSelected()
{
    auto selectedName = presetCombo.getText();
    if (selectedName.isNotEmpty())
        processorRef.getPresetManager().loadPreset(selectedName);
}

} // namespace Neve1073
