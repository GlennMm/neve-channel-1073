#pragma once

#include <juce_dsp/juce_dsp.h>

namespace Neve1073
{

/**
 * Preamp Stage Model (BA283/BA284)
 *
 * Models the discrete Class A gain stages:
 * - Two cascaded common-emitter stages (BC184C)
 * - Emitter follower output (2N3055)
 * - 24VDC single supply, 60-80mA quiescent
 *
 * Key characteristics:
 * - Asymmetric clipping from DC bias point (4.7kΩ trim)
 * - Odd-order harmonics increasing with drive
 * - Dynamic duty-cycle clipping
 */
class PreampStage
{
public:
    PreampStage();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setGain(float gainDb);         // Input gain (-20 to +80 dB)
    void setBias(float bias);           // DC bias point (0-1)
    void setSaturation(float amount);   // Saturation amount (0-1)

    float processSample(float input);
    void processBlock(float* buffer, int numSamples);

private:
    double sampleRate = 44100.0;

    // Parameters
    float gain = 1.0f;
    float dcBias = 0.5f;      // Operating point (0.5 = symmetric)
    float saturation = 0.3f;  // Saturation amount

    // State
    float capacitorState = 0.0f;  // Input coupling capacitor
    float prevSample = 0.0f;

    // Envelope follower for smooth noise gating
    float envelope = 0.0f;
    static constexpr float noiseThreshold = 0.0001f;

    // Processing
    float asymmetricClip(float x, float bias) const;
    float transistorSaturate(float x) const;
};

} // namespace Neve1073
