#pragma once

#include <juce_dsp/juce_dsp.h>
#include "ADAA.h"
#include "Hysteresis.h"

namespace Neve1073
{

/**
 * Transformer Saturation Model
 *
 * Models the frequency-dependent saturation characteristics of:
 * - 10468 Microphone Input Transformer (Mu-metal core, even-order harmonics)
 * - LO1166 Output Transformer (gapped core, softer saturation)
 *
 * Key characteristics:
 * - Low frequencies saturate first (longer integration time)
 * - Even-order harmonics dominant (2nd harmonic)
 * - Asymmetric saturation from DC bias
 * - Magnetic hysteresis for authentic transformer behavior
 * - ADAA for alias-free saturation
 */
class TransformerSaturation
{
public:
    enum class Type
    {
        Input,   // 10468 - Mu-metal, harder saturation, +6dBV threshold
        Output   // LO1166 - Gapped core, softer saturation
    };

    enum class Quality
    {
        Low,     // Basic saturation (fast)
        Medium,  // ADAA only
        High     // ADAA + Hysteresis (most accurate)
    };

    TransformerSaturation(Type type = Type::Input);

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setDrive(float driveDb);       // Saturation amount in dB
    void setDCOffset(float offset);     // Asymmetry control (0-1)
    void setLowFreqSaturation(float amount); // LF saturation emphasis
    void setQuality(Quality q);         // Processing quality

    float processSample(float input);
    void processBlock(float* buffer, int numSamples);

private:
    Type transformerType;
    Quality quality = Quality::Medium;
    double sampleRate = 44100.0;

    // Parameters
    float drive = 1.0f;
    float dcOffset = 0.05f;
    float lfSatAmount = 0.5f;

    // State
    float lfState = 0.0f;
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float dcBlockState = 0.0f;

    // Envelope follower for smooth noise gating
    float envelope = 0.0f;
    float envelopeAttack = 0.01f;
    float envelopeRelease = 0.0001f;
    static constexpr float noiseThreshold = 0.0001f;

    // Filter coefficients
    float lpCoeff = 0.001f;
    float dcBlockCoeff = 0.001f;

    // ADAA processor for alias-free saturation
    ADAA adaa;

    // Hysteresis model for magnetic core
    Hysteresis hysteresis;

    // Saturation functions
    float rationalTanh(float x) const;
    float softClip(float x, float threshold) const;
    float asymmetricSaturate(float x, float offset, float drv) const;
};

} // namespace Neve1073
