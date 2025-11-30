#pragma once

#include <juce_dsp/juce_dsp.h>

namespace Neve1073
{

/**
 * Shelf EQ (B205)
 *
 * Models the RC-based high and low shelf EQ:
 * - High shelf: Fixed 12kHz (1073), ~6dB/octave
 * - Low shelf: 35Hz, 60Hz, 110Hz, 220Hz, ~6dB/octave
 * - ±16dB range
 *
 * Key characteristics:
 * - Non-complementary boost/cut curves
 * - Gentle 6dB/octave slopes (first-order)
 */
class ShelfEQ
{
public:
    enum class Type
    {
        LowShelf,
        HighShelf
    };

    // Low shelf frequencies
    enum class LowFrequency
    {
        Hz35,
        Hz60,
        Hz110,
        Hz220
    };

    // High shelf frequencies (1073 only has 12kHz, but we add options)
    enum class HighFrequency
    {
        kHz12
    };

    ShelfEQ(Type type);

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setLowFrequency(LowFrequency freq);
    void setHighFrequency(HighFrequency freq);
    void setGainDb(float gainDb);  // -16 to +16 dB

    float processSample(float input);
    void processBlock(float* buffer, int numSamples);

private:
    Type filterType;
    double sampleRate = 44100.0;

    // Settings
    float frequency = 110.0f;
    float gainDb = 0.0f;
    float linearGain = 1.0f;

    // Filter state (first-order)
    float z1 = 0.0f;

    // Coefficients
    float b0 = 1.0f, b1 = 0.0f;
    float a1 = 0.0f;

    void calculateCoefficients();
};

} // namespace Neve1073
