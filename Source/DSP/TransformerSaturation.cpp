#include "TransformerSaturation.h"
#include <cmath>

namespace Neve1073
{

TransformerSaturation::TransformerSaturation(Type type)
    : transformerType(type)
{
    if (type == Type::Input)
    {
        dcOffset = 0.03f;   // Subtle asymmetry
        lfSatAmount = 0.5f;
    }
    else
    {
        dcOffset = 0.015f;
        lfSatAmount = 0.3f;
    }
}

void TransformerSaturation::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;

    // LF extraction filter coefficient (~30Hz cutoff)
    lpCoeff = static_cast<float>(1.0 - std::exp(-2.0 * juce::MathConstants<double>::pi * 30.0 / sampleRate));

    // DC blocking filter (~5Hz)
    dcBlockCoeff = static_cast<float>(1.0 - std::exp(-2.0 * juce::MathConstants<double>::pi * 5.0 / sampleRate));

    hysteresis.prepare(sr);

    reset();
}

void TransformerSaturation::reset()
{
    lfState = 0.0f;
    prevInput = 0.0f;
    prevOutput = 0.0f;
    dcBlockState = 0.0f;
    envelope = 0.0f;
    adaa.reset();
    hysteresis.reset();
}

void TransformerSaturation::setDrive(float driveDb)
{
    drive = std::pow(10.0f, driveDb / 20.0f);
    hysteresis.setDrive(driveDb / 12.0f);
    hysteresis.setSaturation(std::min(1.0f, driveDb / 12.0f));
}

void TransformerSaturation::setDCOffset(float offset)
{
    dcOffset = juce::jlimit(0.0f, 0.1f, offset);
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
    // Padé approximation of tanh
    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

float TransformerSaturation::softClip(float x, float threshold) const
{
    if (std::abs(x) < threshold)
        return x;

    float sign = (x > 0.0f) ? 1.0f : -1.0f;
    float absX = std::abs(x);
    float knee = threshold * 0.5f;

    if (absX < threshold + knee)
    {
        float t = (absX - threshold) / knee;
        return sign * (threshold + knee * t * (2.0f - t) * 0.5f);
    }

    return sign * (threshold + knee * 0.5f + (1.0f - std::exp(-(absX - threshold - knee))) * 0.3f);
}

float TransformerSaturation::asymmetricSaturate(float x, float offset, float drv) const
{
    float biased = x + offset;
    float saturated = rationalTanh(biased * drv);
    return saturated - rationalTanh(offset * drv);
}

float TransformerSaturation::processSample(float input)
{
    // Envelope follower for smooth gating on silence
    float absInput = std::abs(input);
    if (absInput > envelope)
        envelope += envelopeAttack * (absInput - envelope);
    else
        envelope += envelopeRelease * (absInput - envelope);

    // If signal is very quiet, pass through with minimal processing
    if (envelope < noiseThreshold)
    {
        prevInput = input;
        return input;
    }

    // Extract low frequency content
    lfState += lpCoeff * (input - lfState);

    // Frequency-dependent drive
    float lfEnergy = std::abs(lfState);
    float dynamicDrive = drive * (1.0f + lfEnergy * lfSatAmount * 2.0f);

    float output;

    switch (quality)
    {
        case Quality::Low:
            output = asymmetricSaturate(input, dcOffset, dynamicDrive);
            break;

        case Quality::Medium:
            {
                float scaled = input * dynamicDrive;
                output = adaa.processFirstOrderTanh(scaled + dcOffset) - std::tanh(dcOffset);
            }
            break;

        case Quality::High:
            {
                float scaled = input * dynamicDrive;
                float hystOut = hysteresis.process(scaled);
                float adaaOut = adaa.processFirstOrderTanh(scaled);
                output = hystOut * 0.5f + adaaOut * 0.5f;
            }
            break;
    }

    // Soft limiting
    float threshold = (transformerType == Type::Input) ? 0.92f : 0.96f;
    output = softClip(output, threshold);

    // Subtle inductive effect (reduced to prevent artifacts)
    float inductance = 0.005f * ((transformerType == Type::Input) ? 0.3f : 0.15f);
    output += inductance * (input - prevInput);

    // DC blocking filter (handles any DC offset naturally)
    dcBlockState += dcBlockCoeff * (output - dcBlockState);
    output = output - dcBlockState;

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
