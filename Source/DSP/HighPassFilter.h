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
    double currentSampleRate = 44100.0;

    bool enabled = false;
    float cutoffFrequency = 80.0f;
    Frequency currentFreq = Frequency::Off;

    // Use JUCE IIR filters for reliability - two cascaded second-order for 4th order (24dB/oct)
    // or three first-order for 3rd order (18dB/oct)
    juce::dsp::IIR::Filter<float> hpf1;  // First 2nd-order section
    juce::dsp::IIR::Filter<float> hpf2;  // Second 2nd-order section (makes it 4th order)

    void updateCoefficients();
};

} // namespace Neve1073
