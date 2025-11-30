#include "HighPassFilter.h"
#include <cmath>

namespace Neve1073
{

HighPassFilter::HighPassFilter()
{
}

void HighPassFilter::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
    calculateCoefficients();
    reset();
}

void HighPassFilter::reset()
{
    x1 = x2 = 0.0f;
    y1 = y2 = 0.0f;
    z1 = 0.0f;
}

void HighPassFilter::setFrequency(Frequency freq)
{
    switch (freq)
    {
        case Frequency::Off:
            enabled = false;
            frequency = 20.0f;
            break;
        case Frequency::Hz50:
            enabled = true;
            frequency = 50.0f;
            break;
        case Frequency::Hz80:
            enabled = true;
            frequency = 80.0f;
            break;
        case Frequency::Hz160:
            enabled = true;
            frequency = 160.0f;
            break;
        case Frequency::Hz300:
            enabled = true;
            frequency = 300.0f;
            break;
    }
    calculateCoefficients();
}

void HighPassFilter::calculateCoefficients()
{
    if (!enabled)
    {
        // Bypass coefficients
        b0 = 1.0f; b1 = 0.0f; b2 = 0.0f;
        a1 = 0.0f; a2 = 0.0f;
        c0 = 1.0f; c1 = 0.0f; d1 = 0.0f;
        return;
    }

    // Third-order Butterworth HPF
    // Implemented as cascaded second-order + first-order sections

    double w0 = 2.0 * juce::MathConstants<double>::pi * frequency / sampleRate;
    double K = std::tan(w0 / 2.0);
    double K2 = K * K;

    // Second-order section (Q = 1 for Butterworth)
    double Q = 1.0;
    double norm = 1.0 / (1.0 + K / Q + K2);

    b0 = static_cast<float>(norm);
    b1 = static_cast<float>(-2.0 * norm);
    b2 = static_cast<float>(norm);
    a1 = static_cast<float>(2.0 * (K2 - 1.0) * norm);
    a2 = static_cast<float>((1.0 - K / Q + K2) * norm);

    // First-order section
    double norm1 = 1.0 / (1.0 + K);
    c0 = static_cast<float>(norm1);
    c1 = static_cast<float>(-norm1);
    d1 = static_cast<float>((1.0 - K) / (1.0 + K));
}

float HighPassFilter::processSample(float input)
{
    if (!enabled)
        return input;

    // Second-order section (biquad)
    float biquadOut = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = biquadOut;

    // First-order section
    float output = c0 * biquadOut + c1 * z1 + d1 * z1;

    // Actually fix the first-order implementation
    float firstOrderOut = c0 * biquadOut + z1;
    z1 = c1 * biquadOut - d1 * firstOrderOut;

    return firstOrderOut;
}

void HighPassFilter::processBlock(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = processSample(buffer[i]);
    }
}

} // namespace Neve1073
