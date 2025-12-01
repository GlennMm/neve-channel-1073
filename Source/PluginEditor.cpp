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
    float angle, juce::Colour baseColor, bool isLarge)
{
    // Shadow
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillEllipse(cx - radius + 2, cy - radius + 3, radius * 2, radius * 2);

    // Outer rim (darker)
    g.setColour(baseColor.darker(0.3f));
    g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);

    // Main knob body with gradient
    float innerRadius = radius * 0.92f;
    juce::ColourGradient knobGradient(
        baseColor.brighter(0.2f), cx - innerRadius * 0.5f, cy - innerRadius * 0.5f,
        baseColor.darker(0.2f), cx + innerRadius * 0.5f, cy + innerRadius * 0.5f, true);
    g.setGradientFill(knobGradient);
    g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2, innerRadius * 2);

    // Highlight arc (top-left)
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    juce::Path highlight;
    highlight.addArc(cx - innerRadius + 2, cy - innerRadius + 2,
                     (innerRadius - 2) * 2, (innerRadius - 2) * 2,
                     -juce::MathConstants<float>::pi * 0.75f,
                     -juce::MathConstants<float>::pi * 0.25f, true);
    g.strokePath(highlight, juce::PathStrokeType(2.0f));

    // Pointer (chicken head shape)
    juce::Path pointer;
    float pointerLength = radius * (isLarge ? 0.85f : 0.75f);
    float pointerWidth = radius * 0.35f;

    // Create chicken-head pointer shape
    pointer.startNewSubPath(0, -pointerLength);
    pointer.lineTo(-pointerWidth * 0.5f, -pointerLength * 0.3f);
    pointer.lineTo(-pointerWidth * 0.3f, pointerLength * 0.1f);
    pointer.lineTo(pointerWidth * 0.3f, pointerLength * 0.1f);
    pointer.lineTo(pointerWidth * 0.5f, -pointerLength * 0.3f);
    pointer.closeSubPath();

    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(cx, cy));

    // Pointer shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillPath(pointer, juce::AffineTransform::translation(1.0f, 1.0f));

    // Pointer fill - white/cream colored
    g.setColour(juce::Colour(0xFFE8E4DC));
    g.fillPath(pointer);

    // Pointer outline
    g.setColour(juce::Colour(0xFFA0A0A0));
    g.strokePath(pointer, juce::PathStrokeType(0.5f));

    // Center dot
    float dotRadius = radius * 0.12f;
    g.setColour(juce::Colour(0xFF404040));
    g.fillEllipse(cx - dotRadius, cy - dotRadius, dotRadius * 2, dotRadius * 2);
}

void Neve1073Editor::NeveLookAndFeel::drawRotarySelectorKnob(
    juce::Graphics& g, float cx, float cy, float radius,
    float angle, int /*numPositions*/, const juce::StringArray& /*labels*/)
{
    // Shadow
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillEllipse(cx - radius + 2, cy - radius + 3, radius * 2, radius * 2);

    // Outer chrome ring
    juce::ColourGradient chromeGradient(
        juce::Colour(0xFFD0D0D0), cx - radius, cy - radius,
        juce::Colour(0xFF606060), cx + radius, cy + radius, false);
    g.setGradientFill(chromeGradient);
    g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);

    // Inner knob body (dark gray)
    float innerRadius = radius * 0.85f;
    juce::ColourGradient knobGradient(
        juce::Colour(0xFF606060), cx, cy - innerRadius,
        juce::Colour(0xFF303030), cx, cy + innerRadius, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2, innerRadius * 2);

    // Knurled texture (radial lines)
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    for (int i = 0; i < 32; ++i)
    {
        float tickAngle = (float)i * juce::MathConstants<float>::twoPi / 32.0f;
        float x1 = cx + std::cos(tickAngle) * innerRadius * 0.4f;
        float y1 = cy + std::sin(tickAngle) * innerRadius * 0.4f;
        float x2 = cx + std::cos(tickAngle) * innerRadius * 0.9f;
        float y2 = cy + std::sin(tickAngle) * innerRadius * 0.9f;
        g.drawLine(x1, y1, x2, y2, 1.0f);
    }

    // White pointer line
    juce::Path pointer;
    pointer.addRectangle(-2.0f, -innerRadius * 0.9f, 4.0f, innerRadius * 0.5f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(cx, cy));

    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillPath(pointer, juce::AffineTransform::translation(1.0f, 1.0f));

    g.setColour(juce::Colour(0xFFE8E4DC));
    g.fillPath(pointer);

    // Center cap
    float capRadius = innerRadius * 0.25f;
    juce::ColourGradient capGradient(
        juce::Colour(0xFF909090), cx - capRadius, cy - capRadius,
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

    // Determine knob style based on component name
    juce::String name = slider.getName();

    if (name.contains("MicGain"))
    {
        // Red chicken-head knob
        drawChickenHeadKnob(g, centreX, centreY, radius, angle, juce::Colour(0xFFCC2020), true);
    }
    else if (name.contains("LineGain") || name.contains("Output"))
    {
        // Gray chicken-head knob
        drawChickenHeadKnob(g, centreX, centreY, radius, angle, juce::Colour(0xFF707070), true);
    }
    else if (name.contains("Freq") || name.contains("HPF"))
    {
        // Rotary selector with frequency markings
        drawRotarySelectorKnob(g, centreX, centreY, radius, angle, 6, {});
    }
    else
    {
        // Small gray gain knob
        drawChickenHeadKnob(g, centreX, centreY, radius, angle, juce::Colour(0xFF606060), false);
    }
}

void Neve1073Editor::NeveLookAndFeel::drawToggleButton(
    juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    bool isOn = button.getToggleState();

    // Button background - recessed look
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.fillRoundedRectangle(bounds, 3.0f);

    // Inner area
    auto innerBounds = bounds.reduced(2.0f);
    g.setColour(isOn ? juce::Colour(0xFF3A5070) : juce::Colour(0xFF252525));
    g.fillRoundedRectangle(innerBounds, 2.0f);

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillRoundedRectangle(innerBounds, 2.0f);
    }

    // LED indicator
    float ledRadius = 4.0f;
    float ledX = bounds.getCentreX();
    float ledY = bounds.getY() + 10.0f;

    // LED glow
    if (isOn)
    {
        g.setColour(juce::Colour(0xFF00FF00).withAlpha(0.3f));
        g.fillEllipse(ledX - ledRadius - 3, ledY - ledRadius - 3,
                      (ledRadius + 3) * 2, (ledRadius + 3) * 2);
    }

    // LED body
    g.setColour(isOn ? juce::Colour(0xFF00CC00) : juce::Colour(0xFF333333));
    g.fillEllipse(ledX - ledRadius, ledY - ledRadius, ledRadius * 2, ledRadius * 2);

    // LED highlight
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.fillEllipse(ledX - ledRadius + 1, ledY - ledRadius + 1, ledRadius * 0.8f, ledRadius * 0.8f);

    // Text
    g.setColour(juce::Colour(0xFFE8E4DC));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), bounds.withTrimmedTop(18.0f),
               juce::Justification::centred);
}

void Neve1073Editor::NeveLookAndFeel::drawComboBox(
    juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);

    g.setColour(juce::Colour(0xFF1E2D45));
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(juce::Colour(0xFF3A5070));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    // Arrow
    juce::Path arrow;
    float arrowX = (float)width - 12.0f;
    float arrowY = (float)height * 0.5f;
    arrow.addTriangle(arrowX - 4, arrowY - 3, arrowX + 4, arrowY - 3, arrowX, arrowY + 3);
    g.setColour(box.isEnabled() ? juce::Colour(0xFFE8E4DC) : juce::Colours::grey);
    g.fillPath(arrow);
}

// Editor Implementation
Neve1073Editor::Neve1073Editor(Neve1073Processor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
{
    setLookAndFeel(&neveLnF);

    // MIC gain (red chicken-head) - maps to input gain
    micGainSlider.setName("MicGain");
    setupRotarySlider(micGainSlider);

    // LINE/output gain (gray chicken-head)
    lineGainSlider.setName("LineGain");
    setupRotarySlider(lineGainSlider);

    // HF (High) gain
    hfGainSlider.setName("HFGain");
    setupRotarySlider(hfGainSlider);

    // MID frequency selector
    midFreqSlider.setName("MidFreq");
    setupRotarySlider(midFreqSlider, 6);

    // MID gain
    midGainSlider.setName("MidGain");
    setupRotarySlider(midGainSlider);

    // LF frequency selector
    lfFreqSlider.setName("LFFreq");
    setupRotarySlider(lfFreqSlider, 4);

    // LF gain
    lfGainSlider.setName("LFGain");
    setupRotarySlider(lfGainSlider);

    // HPF frequency selector
    hpfFreqSlider.setName("HPFFreq");
    setupRotarySlider(hpfFreqSlider, 5);

    // EQL button
    addAndMakeVisible(eqlButton);

    // PHASE button (for authenticity, not connected)
    addAndMakeVisible(phaseButton);

    // Mix slider (small)
    mixSlider.setName("Mix");
    setupRotarySlider(mixSlider);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // Bottom panel controls
    oversamplingCombo.addItemList({"1x", "2x", "4x", "8x"}, 1);
    addAndMakeVisible(oversamplingCombo);

    qualityCombo.addItemList({"Eco", "Normal", "High"}, 1);
    addAndMakeVisible(qualityCombo);

    presetCombo.onChange = [this] { onPresetSelected(); };
    addAndMakeVisible(presetCombo);
    updatePresetList();

    // Create parameter attachments
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

    // Vertical strip layout - similar to 500 series
    setSize(220, 650);
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
    {
        slider.setRange(0, numSteps - 1, 1);
    }

    addAndMakeVisible(slider);
}

void Neve1073Editor::paint(juce::Graphics& g)
{
    // Main background - authentic Neve dark blue
    g.fillAll(neveBlue);

    // Subtle texture/grain
    juce::Random rng(42);
    for (int i = 0; i < 500; ++i)
    {
        float x = rng.nextFloat() * getWidth();
        float y = rng.nextFloat() * getHeight();
        g.setColour(juce::Colours::white.withAlpha(rng.nextFloat() * 0.02f));
        g.fillRect(x, y, 1.0f, 1.0f);
    }

    // Top section darker area
    g.setColour(neveBlueDark);
    g.fillRect(0, 0, getWidth(), 20);

    // Section divider lines
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawHorizontalLine(20, 0, (float)getWidth());

    // Labels in cream/white
    g.setColour(creamWhite);
    g.setFont(juce::Font(10.0f, juce::Font::bold));

    // dB label for input
    g.drawText("dB", 10, 28, 30, 14, juce::Justification::centred);

    // LINE label
    g.drawText("LINE", getWidth() - 45, 28, 35, 14, juce::Justification::centred);

    // MIC label
    g.drawText("MIC.", 10, 115, 35, 14, juce::Justification::centred);

    // Draw gain markings around input knob
    float inputCx = getWidth() * 0.5f;
    float inputCy = 80.0f;
    float markRadius = 52.0f;
    g.setFont(juce::Font(8.0f));

    juce::StringArray inputMarks = {"-20", "-10", "0", "10", "20", "30", "40", "50", "60", "70", "OFF"};
    float startAngle = -2.4f;
    float endAngle = 2.4f;
    for (int i = 0; i < inputMarks.size(); ++i)
    {
        float angle = startAngle + (float)i / (float)(inputMarks.size() - 1) * (endAngle - startAngle);
        float tx = inputCx + std::cos(angle - juce::MathConstants<float>::halfPi) * markRadius;
        float ty = inputCy + std::sin(angle - juce::MathConstants<float>::halfPi) * markRadius;
        g.drawText(inputMarks[i], (int)tx - 15, (int)ty - 6, 30, 12, juce::Justification::centred);
    }

    // Output gain section
    float outputCy = 170.0f;
    g.drawText("-20", 15, (int)outputCy - 10, 25, 12, juce::Justification::centred);
    g.drawText("+20", getWidth() - 40, (int)outputCy - 10, 25, 12, juce::Justification::centred);
    g.drawText("OFF", (int)inputCx - 15, (int)outputCy + 45, 30, 12, juce::Justification::centred);

    // Divider after gain section
    g.setColour(creamWhite.withAlpha(0.3f));
    g.drawHorizontalLine(215, 10, (float)getWidth() - 10);

    // EQ section header
    g.setColour(creamWhite);
    g.setFont(juce::Font(9.0f, juce::Font::bold));

    // HF section - fixed 12kHz shelf
    g.drawText("12", (int)inputCx - 10, 225, 20, 12, juce::Justification::centred);
    g.drawText("kHz", (int)inputCx - 10, 235, 20, 12, juce::Justification::centred);

    // Gain markings for HF
    g.setFont(juce::Font(7.0f));
    g.drawText("+", 25, 260, 15, 10, juce::Justification::centred);
    g.drawText("-", getWidth() - 40, 260, 15, 10, juce::Justification::centred);
    g.drawText("0", (int)inputCx - 8, 305, 16, 10, juce::Justification::centred);

    // MID frequency selector markings
    float midCy = 370.0f;
    g.setFont(juce::Font(7.0f));
    juce::StringArray midFreqs = {"7.2", "4.8", "3.2", "1.6", ".7", ".36"};
    float midMarkRadius = 48.0f;
    for (int i = 0; i < midFreqs.size(); ++i)
    {
        float angle = startAngle + (float)i / (float)(midFreqs.size() - 1) * (endAngle - startAngle);
        float tx = inputCx + std::cos(angle - juce::MathConstants<float>::halfPi) * midMarkRadius;
        float ty = midCy + std::sin(angle - juce::MathConstants<float>::halfPi) * midMarkRadius;
        g.drawText(midFreqs[i], (int)tx - 12, (int)ty - 5, 24, 10, juce::Justification::centred);
    }
    g.drawText("kHz", (int)inputCx - 12, (int)midCy - 8, 24, 10, juce::Justification::centred);

    // LF frequency selector markings
    float lfCy = 460.0f;
    juce::StringArray lfFreqs = {"220", "110", "60", "35"};
    for (int i = 0; i < lfFreqs.size(); ++i)
    {
        float angle = startAngle + (float)i / (float)(lfFreqs.size() - 1) * (endAngle - startAngle);
        float tx = inputCx + std::cos(angle - juce::MathConstants<float>::halfPi) * midMarkRadius;
        float ty = lfCy + std::sin(angle - juce::MathConstants<float>::halfPi) * midMarkRadius;
        g.drawText(lfFreqs[i], (int)tx - 15, (int)ty - 5, 30, 10, juce::Justification::centred);
    }
    g.drawText("Hz", (int)inputCx - 10, (int)lfCy - 8, 20, 10, juce::Justification::centred);

    // HPF frequency selector markings
    float hpfCy = 545.0f;
    juce::StringArray hpfFreqs = {"OFF", "50", "80", "160", "300"};
    for (int i = 0; i < hpfFreqs.size(); ++i)
    {
        float angle = startAngle + (float)i / (float)(hpfFreqs.size() - 1) * (endAngle - startAngle);
        float tx = inputCx + std::cos(angle - juce::MathConstants<float>::halfPi) * midMarkRadius;
        float ty = hpfCy + std::sin(angle - juce::MathConstants<float>::halfPi) * midMarkRadius;
        g.drawText(hpfFreqs[i], (int)tx - 15, (int)ty - 5, 30, 10, juce::Justification::centred);
    }
    g.drawText("Hz", (int)inputCx - 10, (int)hpfCy - 8, 20, 10, juce::Justification::centred);

    // Bottom panel
    g.setColour(neveBlueDark);
    g.fillRect(0, getHeight() - 45, getWidth(), 45);

    g.setColour(creamWhite.withAlpha(0.5f));
    g.setFont(juce::Font(7.0f));
    g.drawText("PRESET", 5, getHeight() - 43, 40, 10, juce::Justification::centredLeft);
    g.drawText("OS", 5, getHeight() - 22, 20, 10, juce::Justification::centredLeft);
    g.drawText("Q", 80, getHeight() - 22, 15, 10, juce::Justification::centredLeft);
    g.drawText("MIX", 140, getHeight() - 22, 25, 10, juce::Justification::centredLeft);

    // Corner screws
    auto drawScrew = [&](float sx, float sy) {
        g.setColour(juce::Colour(0xFF404040));
        g.fillEllipse(sx - 5, sy - 5, 10, 10);
        g.setColour(juce::Colour(0xFF606060));
        g.drawEllipse(sx - 5, sy - 5, 10, 10, 1.0f);
        g.setColour(juce::Colour(0xFF303030));
        g.drawLine(sx - 3, sy, sx + 3, sy, 1.5f);
    };

    drawScrew(10, 10);
    drawScrew((float)getWidth() - 10, 10);
}

void Neve1073Editor::resized()
{
    int cx = getWidth() / 2;
    int knobSize = 70;
    int smallKnobSize = 55;
    int selectorSize = 65;

    // Input gain (red chicken-head) - top
    micGainSlider.setBounds(cx - knobSize / 2, 40, knobSize, knobSize);

    // Output/Line gain (gray chicken-head)
    lineGainSlider.setBounds(cx - knobSize / 2, 130, knobSize, knobSize);

    // HF Gain (fixed 12kHz)
    hfGainSlider.setBounds(cx - smallKnobSize / 2, 250, smallKnobSize, smallKnobSize);

    // MID frequency selector
    midFreqSlider.setBounds(cx - selectorSize / 2, 335, selectorSize, selectorSize);

    // MID Gain (small, to the side or integrated)
    midGainSlider.setBounds(getWidth() - 50, 350, 40, 40);

    // LF frequency selector
    lfFreqSlider.setBounds(cx - selectorSize / 2, 425, selectorSize, selectorSize);

    // LF Gain
    lfGainSlider.setBounds(getWidth() - 50, 440, 40, 40);

    // HPF frequency selector
    hpfFreqSlider.setBounds(cx - selectorSize / 2, 510, selectorSize, selectorSize);

    // EQL and PHASE buttons
    eqlButton.setBounds(10, 580, 45, 35);
    phaseButton.setBounds(60, 580, 55, 35);

    // Mix knob
    mixSlider.setBounds(165, getHeight() - 40, 35, 35);

    // Bottom panel controls
    presetCombo.setBounds(5, getHeight() - 32, getWidth() - 10, 18);
    oversamplingCombo.setBounds(20, getHeight() - 18, 50, 15);
    qualityCombo.setBounds(90, getHeight() - 18, 45, 15);
}

void Neve1073Editor::updatePresetList()
{
    auto& pm = processorRef.getPresetManager();
    presetCombo.clear(juce::dontSendNotification);

    auto factoryPresets = pm.getFactoryPresetNames();
    int itemId = 1;

    for (const auto& name : factoryPresets)
    {
        presetCombo.addItem(name, itemId++);
    }

    auto userPresets = pm.getUserPresetNames();
    if (!userPresets.isEmpty())
    {
        presetCombo.addSeparator();
        for (const auto& name : userPresets)
        {
            presetCombo.addItem(name, itemId++);
        }
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
    {
        processorRef.getPresetManager().loadPreset(selectedName);
    }
}

} // namespace Neve1073
