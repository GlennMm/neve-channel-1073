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
    inductorCurrent = 0.0f;
    inductorEnergy = 0.0f;
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

float InductorEQ::getInductance(Frequency freq) const
{
    // T1530 inductor taps for lower frequencies
    // T1280 (200mH) for higher frequencies
    switch (freq)
    {
        case Frequency::Hz360:  return 2.0f;    // 2H
        case Frequency::Hz700:  return 1.1f;    // 1.1H
        case Frequency::Hz1600: return 0.45f;   // 0.45H
        case Frequency::Hz3200: return 0.2f;    // 200mH
        case Frequency::Hz4800: return 0.2f;    // 200mH
        case Frequency::Hz7200: return 0.2f;    // 200mH
        default: return 0.45f;
    }
}

float InductorEQ::getDCResistance(Frequency freq) const
{
    // DC resistance varies with inductor tap
    switch (freq)
    {
        case Frequency::Hz360:  return 85.0f;   // 85Ω
        case Frequency::Hz700:  return 59.0f;   // 59Ω
        case Frequency::Hz1600: return 35.0f;   // 35Ω
        case Frequency::Hz3200: return 32.0f;   // 32Ω (T1280)
        case Frequency::Hz4800: return 32.0f;
        case Frequency::Hz7200: return 32.0f;
        default: return 35.0f;
    }
}

void InductorEQ::setFrequency(Frequency freq)
{
    currentFreq = freq;
    inductance = getInductance(freq);
    dcResistance = getDCResistance(freq);
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

    // Add DC resistance damping effect (higher resistance = lower Q)
    float resistanceDamping = 1.0f - (dcResistance / 1000.0f);
    Q *= resistanceDamping;

    // TPT SVF coefficients
    g = static_cast<float>(std::tan(juce::MathConstants<double>::pi * freq / sampleRate));
    k = 1.0f / Q;

    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}

float InductorEQ::saturateInductorCore(float current) const
{
    // Model soft magnetic saturation of inductor core
    // Based on B-H curve of ferrite/iron core material
    if (inductorSat < 0.001f)
        return current;

    // Saturation threshold depends on inductance (larger = saturates easier)
    float threshold = 0.7f / (1.0f + inductance);
    float absI = std::abs(current);

    if (absI < threshold)
        return current;

    float sign = current > 0 ? 1.0f : -1.0f;
    float excess = absI - threshold;

    // Langevin-like saturation curve (realistic for magnetic materials)
    float satFactor = inductorSat * (1.0f + inductance);
    float saturated = threshold + (1.0f - threshold) * std::tanh(excess * satFactor / (1.0f - threshold));

    // Add subtle odd harmonics from core saturation
    float harmonic = 0.02f * inductorSat * excess * excess * sign;

    return sign * saturated + harmonic;
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

    // Model inductor current and energy storage
    float T = static_cast<float>(1.0 / sampleRate);

    // Inductor: V = L * dI/dt, so dI = V * dt / L
    float dI = bandpass * T / inductance;
    inductorCurrent += dI;

    // Energy dissipation through DC resistance
    float dissipation = inductorCurrent * dcResistance * T * 0.0001f;
    inductorCurrent -= dissipation;

    // Apply core saturation to current
    float saturatedCurrent = saturateInductorCore(inductorCurrent);

    // Energy in inductor: E = 0.5 * L * I^2
    inductorEnergy = 0.5f * inductance * saturatedCurrent * saturatedCurrent;

    // Modulate bandpass with saturated behavior
    float satEffect = (inductorCurrent - saturatedCurrent) * inductorSat;
    float saturatedBP = bandpass - satEffect;

    // Peaking EQ output
    // Note: linearGain - 1.0 gives the boost/cut amount
    float gainAmount = linearGain - 1.0f;

    // Add subtle resonant "ring" based on stored energy
    float resonance = inductorEnergy * 0.1f * inductorSat;

    return input + gainAmount * saturatedBP + resonance * bandpass;
}

void InductorEQ::processBlock(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = processSample(buffer[i]);
    }
}

} // namespace Neve1073
