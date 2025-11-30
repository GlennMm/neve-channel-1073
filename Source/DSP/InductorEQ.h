#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>

namespace Neve1073
{

/**
 * Inductor-based Mid Peak EQ (BA211)
 *
 * Models the signature LC resonant mid band:
 * - T1530 inductor (2H, 1.1H, 0.45H taps)
 * - T1280 inductor (200mH for higher frequencies)
 * - Switched capacitors for frequency selection
 *
 * Key characteristics:
 * - Q increases with frequency (360Hz-7.2kHz)
 * - ±18dB boost/cut
 * - Non-complementary boost/cut curves
 * - Subtle inductor saturation at high levels
 * - Resonant behavior with energy storage
 */
class InductorEQ
{
public:
    // Available frequency selections
    enum class Frequency
    {
        Hz360,    // T1530 @ 2H + 100nF
        Hz700,    // T1530 @ 1.1H + 47nF
        Hz1600,   // T1530 @ 0.45H + 22nF
        Hz3200,   // T1280 @ 200mH
        Hz4800,   // T1280 @ 200mH
        Hz7200    // T1280 @ 200mH
    };

    InductorEQ();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setFrequency(Frequency freq);
    void setGainDb(float gainDb);      // -18 to +18 dB
    void setInductorSaturation(float amount); // 0-1

    float processSample(float input);
    void processBlock(float* buffer, int numSamples);

private:
    double sampleRate = 44100.0;

    // Current settings
    Frequency currentFreq = Frequency::Hz1600;
    float gainDb = 0.0f;
    float linearGain = 1.0f;
    float inductorSat = 0.3f;

    // Filter state (TPT SVF implementation)
    float ic1eq = 0.0f;
    float ic2eq = 0.0f;

    // Filter coefficients
    float g = 0.0f;   // Frequency coefficient
    float k = 0.0f;   // Damping (1/Q)
    float a1 = 0.0f;
    float a2 = 0.0f;
    float a3 = 0.0f;

    // Inductor core state for saturation
    float inductorCurrent = 0.0f;
    float inductorEnergy = 0.0f;

    // Inductor physical properties per frequency
    float inductance = 0.45f;    // Henries
    float dcResistance = 35.0f;  // Ohms
    float corePermeability = 1.0f;

    void calculateCoefficients();
    float getFrequencyHz(Frequency freq) const;
    float getQ(Frequency freq) const;
    float getInductance(Frequency freq) const;
    float getDCResistance(Frequency freq) const;
    float saturateInductorCore(float current) const;
};

} // namespace Neve1073
