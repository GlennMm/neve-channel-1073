#include "ParameterSmoothing.h"
#include <cmath>

namespace Neve1073
{

SmoothedParameter::SmoothedParameter(float initialValue, float smoothTime)
    : currentValue(initialValue)
    , targetValue(initialValue)
    , smoothTimeMs(smoothTime)
    , coefficient(0.0f)
{
}

void SmoothedParameter::prepare(double sr)
{
    sampleRate = sr;
    updateCoefficient();
}

void SmoothedParameter::reset(float value)
{
    currentValue = value;
    targetValue = value;
}

void SmoothedParameter::setTargetValue(float newTarget)
{
    targetValue = newTarget;
}

void SmoothedParameter::setSmoothTime(float timeMs)
{
    smoothTimeMs = timeMs;
    updateCoefficient();
}

void SmoothedParameter::updateCoefficient()
{
    if (smoothTimeMs <= 0.0f || sampleRate <= 0.0)
    {
        coefficient = 1.0f;
    }
    else
    {
        // Time constant for ~99% convergence
        float tau = smoothTimeMs / 1000.0f / 4.6f;
        coefficient = static_cast<float>(1.0 - std::exp(-1.0 / (tau * sampleRate)));
    }
}

float SmoothedParameter::getNextValue()
{
    if (std::abs(currentValue - targetValue) < 1e-6f)
    {
        currentValue = targetValue;
    }
    else
    {
        currentValue += coefficient * (targetValue - currentValue);
    }
    return currentValue;
}

bool SmoothedParameter::isSmoothing() const
{
    return std::abs(currentValue - targetValue) > 1e-6f;
}

} // namespace Neve1073
