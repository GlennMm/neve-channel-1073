#pragma once

#include <juce_dsp/juce_dsp.h>

namespace Neve1073
{

/**
 * Parameter Smoothing
 *
 * Provides smooth parameter transitions to prevent zipper noise.
 * Uses exponential smoothing with ~50-100ms ramp time.
 */
class SmoothedParameter
{
public:
    SmoothedParameter(float initialValue = 0.0f, float smoothTimeMs = 50.0f);

    void prepare(double sampleRate);
    void reset(float value);

    void setTargetValue(float newTarget);
    void setSmoothTime(float timeMs);

    float getNextValue();
    float getCurrentValue() const { return currentValue; }
    float getTargetValue() const { return targetValue; }

    bool isSmoothing() const;

private:
    float currentValue;
    float targetValue;
    float smoothTimeMs;
    float coefficient;
    double sampleRate = 44100.0;

    void updateCoefficient();
};

} // namespace Neve1073
