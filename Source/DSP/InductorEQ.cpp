#include "InductorEQ.h"
#include <cmath>

namespace Neve1073
{

InductorEQ::InductorEQ()
{
}

void InductorEQ::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
    calculateCoefficients();
    reset();
}

void InductorEQ::reset()
{
    ic1eq = 0.0f;
    ic2eq = 0.0f;
    inductorState = 0.0f;
}

float InductorEQ::getFrequencyHz(Frequency freq) const
{
    switch (freq)
    {
        case Frequency::Hz360:  return 360.0f;
        case Frequency::Hz700:  return 700.0f;
        case Frequency::Hz1600: return 1600.0f;
        case Frequency::Hz3200: return 3200.0f;
        case Frequency::Hz4800: return 4800.0f;
        case Frequency::Hz7200: return 7200.0f;
        default: return 1600.0f;
    }
}

float InductorEQ::getQ(Frequency freq) const
{
    // Q increases with frequency (1073 characteristic)
    // Lower freqs: L and C both switched, ~constant Q
    // Higher freqs: only C switched with fixed L, Q increases
    switch (freq)
    {
        case Frequency::Hz360:  return 1.5f;
        case Frequency::Hz700:  return 1.6f;
        case Frequency::Hz1600: return 1.8f;
        case Frequency::Hz3200: return 2.2f;
        case Frequency::Hz4800: return 2.6f;
        case Frequency::Hz7200: return 3.0f;
        default: return 1.8f;
    }
}

void InductorEQ::setFrequency(Frequency freq)
{
    currentFreq = freq;
    calculateCoefficients();
}

void InductorEQ::setGainDb(float gain)
{
    gainDb = juce::jlimit(-18.0f, 18.0f, gain);
    linearGain = std::pow(10.0f, gainDb / 20.0f);
    calculateCoefficients();
}

void InductorEQ::setInductorSaturation(float amount)
{
    inductorSat = juce::jlimit(0.0f, 1.0f, amount);
}

void InductorEQ::calculateCoefficients()
{
    float freq = getFrequencyHz(currentFreq);
    float Q = getQ(currentFreq);

    // For boost/cut asymmetry, adjust Q slightly
    // Boosts are narrower, cuts are wider (1073 characteristic)
    if (gainDb > 0)
    {
        Q *= (1.0f + gainDb / 36.0f);  // Increase Q for boost
    }
    else if (gainDb < 0)
    {
        Q *= (1.0f + gainDb / 72.0f);  // Decrease Q for cut
    }

    // TPT SVF coefficients
    g = static_cast<float>(std::tan(juce::MathConstants<double>::pi * freq / sampleRate));
    k = 1.0f / Q;

    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}

float InductorEQ::saturateInductor(float x) const
{
    // Model soft magnetic saturation of inductor core
    if (inductorSat < 0.001f)
        return x;

    float threshold = 0.8f;
    float absX = std::abs(x);

    if (absX < threshold)
        return x;

    float sign = x > 0 ? 1.0f : -1.0f;
    float excess = absX - threshold;
    float satAmount = inductorSat * 0.5f;

    // Soft saturation curve
    return sign * (threshold + (1.0f - threshold) * std::tanh(excess * (1.0f + satAmount) / (1.0f - threshold)));
}

float InductorEQ::processSample(float input)
{
    // TPT State Variable Filter implementation
    // Peaking EQ: output = input + gain * bandpass

    float v3 = input - ic2eq;
    float v1 = a1 * ic1eq + a2 * v3;
    float v2 = ic2eq + a2 * ic1eq + a3 * v3;

    ic1eq = 2.0f * v1 - ic1eq;
    ic2eq = 2.0f * v2 - ic2eq;

    // Bandpass output (v1 scaled by k)
    float bandpass = v1 * k;

    // Apply inductor saturation to the bandpass (reactive element)
    float lpCoeff = static_cast<float>(1.0 - std::exp(-2.0 * juce::MathConstants<double>::pi * 100.0 / sampleRate));
    inductorState += lpCoeff * (bandpass - inductorState);
    float saturatedBP = saturateInductor(bandpass + inductorState * inductorSat * 0.2f);

    // Peaking EQ output
    // Note: linearGain - 1.0 gives the boost/cut amount
    float gainAmount = linearGain - 1.0f;

    return input + gainAmount * saturatedBP;
}

void InductorEQ::processBlock(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = processSample(buffer[i]);
    }
}

} // namespace Neve1073
