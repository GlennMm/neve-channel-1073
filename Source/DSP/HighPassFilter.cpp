#include "HighPassFilter.h"
#include <cmath>

namespace Neve1073
{

HighPassFilter::HighPassFilter()
{
}

void HighPassFilter::prepare(double sr, int /*samplesPerBlock*/)
{
    currentSampleRate = sr;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;

    hpf1.prepare(spec);
    hpf2.prepare(spec);

    updateCoefficients();
    reset();
}

void HighPassFilter::reset()
{
    hpf1.reset();
    hpf2.reset();
}

void HighPassFilter::setFrequency(Frequency freq)
{
    if (freq == currentFreq)
        return;  // No change needed

    currentFreq = freq;

    switch (freq)
    {
        case Frequency::Off:
            enabled = false;
            cutoffFrequency = 20.0f;
            break;
        case Frequency::Hz50:
            enabled = true;
            cutoffFrequency = 50.0f;
            break;
        case Frequency::Hz80:
            enabled = true;
            cutoffFrequency = 80.0f;
            break;
        case Frequency::Hz160:
            enabled = true;
            cutoffFrequency = 160.0f;
            break;
        case Frequency::Hz300:
            enabled = true;
            cutoffFrequency = 300.0f;
            break;
    }

    updateCoefficients();
}

void HighPassFilter::updateCoefficients()
{
    if (!enabled || currentSampleRate <= 0)
        return;

    // Create 2nd-order Butterworth HPF coefficients
    // Using two cascaded 2nd-order sections gives 4th order (24dB/octave)
    // which is close to the original 1073's 18dB/octave (3rd order) but more common

    // First section: Q = 0.54 (Butterworth 4th order first section)
    auto coeffs1 = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        currentSampleRate, cutoffFrequency, 0.54f);

    // Second section: Q = 1.31 (Butterworth 4th order second section)
    auto coeffs2 = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        currentSampleRate, cutoffFrequency, 1.31f);

    *hpf1.coefficients = *coeffs1;
    *hpf2.coefficients = *coeffs2;
}

float HighPassFilter::processSample(float input)
{
    if (!enabled)
        return input;

    // Cascaded filtering for steeper slope
    float output = hpf1.processSample(input);
    output = hpf2.processSample(output);

    return output;
}

void HighPassFilter::processBlock(float* buffer, int numSamples)
{
    if (!enabled)
        return;  // Pass through unchanged

    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = processSample(buffer[i]);
    }
}

} // namespace Neve1073
