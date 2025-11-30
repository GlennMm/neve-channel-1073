#include "TransformerSaturation.h"
#include <cmath>

namespace Neve1073
{

TransformerSaturation::TransformerSaturation(Type type)
    : transformerType(type)
{
    // Set type-specific defaults
    if (type == Type::Input)
    {
        dcOffset = 0.05f;   // More asymmetry
        lfSatAmount = 0.7f; // More LF saturation
    }
    else // Output
    {
        dcOffset = 0.02f;   // Less asymmetry (gapped core)
        lfSatAmount = 0.4f; // Less LF saturation
    }
}

void TransformerSaturation::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;

    // LF extraction filter coefficient (~30Hz cutoff)
    lpCoeff = static_cast<float>(1.0 - std::exp(-2.0 * juce::MathConstants<double>::pi * 30.0 / sampleRate));

    // Prepare hysteresis
    hysteresis.prepare(sr);

    reset();
}

void TransformerSaturation::reset()
{
    lfState = 0.0f;
    prevInput = 0.0f;
    prevOutput = 0.0f;
    adaa.reset();
    hysteresis.reset();
}

void TransformerSaturation::setDrive(float driveDb)
{
    drive = std::pow(10.0f, driveDb / 20.0f);

    // Update hysteresis parameters based on drive
    hysteresis.setDrive(driveDb / 12.0f);  // Normalize to 0-1 range
    hysteresis.setSaturation(std::min(1.0f, driveDb / 12.0f));
}

void TransformerSaturation::setDCOffset(float offset)
{
    dcOffset = juce::jlimit(0.0f, 0.2f, offset);
    hysteresis.setBias(offset);
}

void TransformerSaturation::setLowFreqSaturation(float amount)
{
    lfSatAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void TransformerSaturation::setQuality(Quality q)
{
    quality = q;
}

float TransformerSaturation::rationalTanh(float x) const
{
    // Padé approximation of tanh - fast and accurate
    // tanh(x) ≈ x(27 + x²) / (27 + 9x²)
    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

float TransformerSaturation::softClip(float x, float threshold) const
{
    // Soft clipping with smooth knee
    if (std::abs(x) < threshold)
        return x;

    float sign = (x > 0.0f) ? 1.0f : -1.0f;
    float absX = std::abs(x);
    float knee = threshold * 0.5f;

    // Smooth transition region
    if (absX < threshold + knee)
    {
        float t = (absX - threshold) / knee;
        return sign * (threshold + knee * t * (2.0f - t) * 0.5f);
    }

    // Hard limit
    return sign * (threshold + knee * 0.5f + (1.0f - std::exp(-(absX - threshold - knee))) * 0.3f);
}

float TransformerSaturation::asymmetricSaturate(float x, float offset, float drv) const
{
    // Apply DC offset for even-order harmonic generation
    float biased = x + offset;

    // Apply saturation
    float saturated = rationalTanh(biased * drv);

    // Remove DC offset from output
    return saturated - rationalTanh(offset * drv);
}

float TransformerSaturation::processSample(float input)
{
    // Extract low frequency content (transformer core integration)
    lfState += lpCoeff * (input - lfState);

    // Calculate frequency-dependent drive
    // Low frequencies drive the core harder due to longer integration time
    float lfEnergy = std::abs(lfState);
    float dynamicDrive = drive * (1.0f + lfEnergy * lfSatAmount * 3.0f);

    float output;

    switch (quality)
    {
        case Quality::Low:
            // Basic saturation (fastest)
            output = asymmetricSaturate(input, dcOffset, dynamicDrive);
            break;

        case Quality::Medium:
            // ADAA for cleaner harmonics
            output = adaa.processAsymmetricSoftClip(input * dynamicDrive, dcOffset);
            break;

        case Quality::High:
            // Full hysteresis modeling (most accurate)
            {
                float preHyst = input * dynamicDrive;
                float hystOut = hysteresis.process(preHyst);
                // Blend hysteresis with ADAA
                float adaaOut = adaa.processFirstOrderTanh(preHyst);
                output = hystOut * 0.6f + adaaOut * 0.4f;
            }
            break;
    }

    // Soft limiting based on transformer type
    float threshold = (transformerType == Type::Input) ? 0.9f : 0.95f;
    output = softClip(output, threshold);

    // Subtle inductive effect (high frequency roll-off simulation)
    float inductance = 0.02f * ((transformerType == Type::Input) ? 0.5f : 0.3f);
    output += inductance * (input - prevInput) * (1.0f + std::abs(prevOutput));

    prevInput = input;
    prevOutput = output;

    return output;
}

void TransformerSaturation::processBlock(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = processSample(buffer[i]);
    }
}

} // namespace Neve1073
