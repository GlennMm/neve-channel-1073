#include "PreampStage.h"
#include <cmath>

namespace Neve1073
{

PreampStage::PreampStage()
{
}

void PreampStage::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
    reset();
}

void PreampStage::reset()
{
    capacitorState = 0.0f;
    prevSample = 0.0f;
}

void PreampStage::setGain(float gainDb)
{
    gain = std::pow(10.0f, gainDb / 20.0f);
}

void PreampStage::setBias(float bias)
{
    dcBias = juce::jlimit(0.3f, 0.7f, bias);
}

void PreampStage::setSaturation(float amount)
{
    saturation = juce::jlimit(0.0f, 1.0f, amount);
}

float PreampStage::asymmetricClip(float x, float bias) const
{
    // Model asymmetric clipping from DC operating point
    // bias = 0.5 is symmetric, <0.5 clips positive earlier, >0.5 clips negative earlier

    float positiveThreshold = 1.0f - (bias - 0.5f) * 0.6f;
    float negativeThreshold = 1.0f + (bias - 0.5f) * 0.6f;

    if (x > 0)
    {
        if (x > positiveThreshold)
        {
            // Soft knee into clipping
            float excess = x - positiveThreshold;
            float knee = positiveThreshold * 0.2f;
            return positiveThreshold + knee * std::tanh(excess / knee);
        }
    }
    else
    {
        if (x < -negativeThreshold)
        {
            float excess = -x - negativeThreshold;
            float knee = negativeThreshold * 0.2f;
            return -(negativeThreshold + knee * std::tanh(excess / knee));
        }
    }

    return x;
}

float PreampStage::transistorSaturate(float x) const
{
    // Model transistor stage saturation (odd harmonics)
    // Uses polynomial approximation of transistor transfer curve

    float sat = saturation * 2.0f;

    if (std::abs(x) < 0.001f)
        return x;

    // Polynomial soft clipping (odd harmonics)
    float x2 = x * x;
    float x3 = x2 * x;

    // Approximation of transistor transfer curve
    float output = x - sat * 0.2f * x3 / (1.0f + std::abs(x));

    // Add subtle even harmonics from Class A bias
    output += sat * 0.05f * x2 * (x > 0 ? 1.0f : -1.0f);

    return output;
}

float PreampStage::processSample(float input)
{
    // Input coupling capacitor (blocks DC, 6.8-22µF tantalum)
    // High-pass at ~7Hz
    float hpCoeff = static_cast<float>(1.0 - std::exp(-2.0 * juce::MathConstants<double>::pi * 7.0 / sampleRate));
    capacitorState += hpCoeff * (input - capacitorState);
    float acCoupled = input - capacitorState;

    // Apply gain
    float amplified = acCoupled * gain;

    // First stage saturation
    float stage1 = transistorSaturate(amplified);

    // Asymmetric clipping from DC bias point
    float clipped = asymmetricClip(stage1, dcBias);

    // Second stage (less gain, more headroom)
    float stage2 = transistorSaturate(clipped * 0.7f) * 1.4f;

    // Emitter follower (unity gain, adds subtle coloration)
    float output = stage2 + 0.02f * (stage2 - prevSample) * saturation;

    prevSample = stage2;

    return output;
}

void PreampStage::processBlock(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = processSample(buffer[i]);
    }
}

} // namespace Neve1073
