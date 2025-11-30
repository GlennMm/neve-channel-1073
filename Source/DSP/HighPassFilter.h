#pragma once

#include <juce_dsp/juce_dsp.h>

namespace Neve1073
{

/**
 * High-Pass Filter (B182)
 *
 * Models the passive LC high-pass filter:
 * - 18dB/octave (3rd order) slope
 * - Uses T1295 tapped inductor
 * - Requires 5.1kΩ termination for flat response
 *
 * Frequencies: 50Hz, 80Hz, 160Hz, 300Hz (Off position also available)
 */
class HighPassFilter
{
public:
    enum class Frequency
    {
        Off,
        Hz50,
        Hz80,
        Hz160,
        Hz300
    };

    HighPassFilter();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setFrequency(Frequency freq);
    bool isEnabled() const { return enabled; }

    float processSample(float input);
    void processBlock(float* buffer, int numSamples);

private:
    double sampleRate = 44100.0;

    bool enabled = false;
    float frequency = 80.0f;

    // Third-order Butterworth HPF state (cascaded biquad + first order)
    // Biquad state
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;

    // First-order state
    float z1 = 0.0f;

    // Biquad coefficients
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    // First-order coefficients
    float c0 = 1.0f, c1 = 0.0f;
    float d1 = 0.0f;

    void calculateCoefficients();
};

} // namespace Neve1073
