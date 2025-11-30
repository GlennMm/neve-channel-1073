#pragma once

#include <juce_dsp/juce_dsp.h>

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
 */
class TransformerSaturation
{
public:
    enum class Type
    {
        Input,   // 10468 - Mu-metal, harder saturation, +6dBV threshold
        Output   // LO1166 - Gapped core, softer saturation
    };

    TransformerSaturation(Type type = Type::Input);

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setDrive(float driveDb);       // Saturation amount in dB
    void setDCOffset(float offset);     // Asymmetry control (0-1)
    void setLowFreqSaturation(float amount); // LF saturation emphasis

    float processSample(float input);
    void processBlock(float* buffer, int numSamples);

private:
    Type transformerType;
    double sampleRate = 44100.0;

    // Parameters
    float drive = 1.0f;
    float dcOffset = 0.05f;
    float lfSatAmount = 0.5f;

    // State
    float lfState = 0.0f;
    float prevInput = 0.0f;
    float prevOutput = 0.0f;

    // Lowpass for LF extraction
    float lpCoeff = 0.001f;

    // Saturation functions
    float rationalTanh(float x) const;
    float softClip(float x, float threshold) const;
    float asymmetricSaturate(float x, float offset, float drive) const;
};

} // namespace Neve1073
