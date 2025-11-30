#include "ShelfEQ.h"
#include <cmath>

namespace Neve1073
{

ShelfEQ::ShelfEQ(Type type)
    : filterType(type)
{
    if (type == Type::LowShelf)
        frequency = 110.0f;
    else
        frequency = 12000.0f;
}

void ShelfEQ::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
    calculateCoefficients();
    reset();
}

void ShelfEQ::reset()
{
    z1 = 0.0f;
}

void ShelfEQ::setLowFrequency(LowFrequency freq)
{
    switch (freq)
    {
        case LowFrequency::Hz35:  frequency = 35.0f;  break;
        case LowFrequency::Hz60:  frequency = 60.0f;  break;
        case LowFrequency::Hz110: frequency = 110.0f; break;
        case LowFrequency::Hz220: frequency = 220.0f; break;
    }
    calculateCoefficients();
}

void ShelfEQ::setHighFrequency(HighFrequency /*freq*/)
{
    frequency = 12000.0f;
    calculateCoefficients();
}

void ShelfEQ::setGainDb(float gain)
{
    gainDb = juce::jlimit(-16.0f, 16.0f, gain);
    linearGain = std::pow(10.0f, gainDb / 20.0f);
    calculateCoefficients();
}

void ShelfEQ::calculateCoefficients()
{
    // First-order shelf filter (6dB/octave)
    // Based on RBJ cookbook simplified for first-order

    float A = linearGain;
    float w0 = static_cast<float>(2.0 * juce::MathConstants<double>::pi * frequency / sampleRate);
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);

    // Adjusted for non-complementary curves (1073 characteristic)
    float boostFactor = (gainDb > 0) ? 1.0f : 0.85f;

    if (filterType == Type::LowShelf)
    {
        // First-order low shelf
        float alpha = sinw0 / 2.0f * boostFactor;
        float sqrtA = std::sqrt(A);

        float a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * sqrtA * alpha;
        b0 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * sqrtA * alpha)) / a0;
        b1 = (A * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
        a1 = -((A - 1.0f) + (A + 1.0f) * cosw0 - 2.0f * sqrtA * alpha) / a0;
    }
    else // HighShelf
    {
        // First-order high shelf
        float alpha = sinw0 / 2.0f * boostFactor;
        float sqrtA = std::sqrt(A);

        float a0 = (A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * sqrtA * alpha;
        b0 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * sqrtA * alpha)) / a0;
        b1 = (-A * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
        a1 = -((A - 1.0f) - (A + 1.0f) * cosw0 - 2.0f * sqrtA * alpha) / a0;
    }

    // Normalize for unity passband
    if (std::abs(gainDb) < 0.1f)
    {
        b0 = 1.0f;
        b1 = 0.0f;
        a1 = 0.0f;
    }
}

float ShelfEQ::processSample(float input)
{
    // Direct Form I
    float output = b0 * input + b1 * z1 - a1 * z1;

    // Actually we need to store input for b1 and output for a1
    // Let me fix this with proper state

    float y = b0 * input + z1;
    z1 = b1 * input - a1 * y;

    return y;
}

void ShelfEQ::processBlock(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = processSample(buffer[i]);
    }
}

} // namespace Neve1073
