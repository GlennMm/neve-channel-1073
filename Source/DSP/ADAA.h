#pragma once

#include <cmath>

namespace Neve1073
{

/**
 * Antiderivative Anti-Aliasing (ADAA)
 *
 * Reduces aliasing from nonlinear waveshaping without heavy oversampling.
 * Based on the paper "Antiderivative Antialiasing for Memoryless Nonlinearities"
 * by Parker et al.
 *
 * Supports first-order and second-order ADAA for tanh and soft clipping.
 */
class ADAA
{
public:
    ADAA() = default;

    void reset()
    {
        x1 = 0.0f;
        x2 = 0.0f;
        ad1_x1 = 0.0f;
        ad2_x1 = 0.0f;
        ad2_x2 = 0.0f;
    }

    // First-order ADAA tanh
    float processFirstOrderTanh(float x)
    {
        float result;
        float diff = x - x1;

        if (std::abs(diff) < tolerance)
        {
            // Use midpoint evaluation to avoid division by zero
            result = tanhDerivative((x + x1) * 0.5f);
        }
        else
        {
            result = (tanhAD1(x) - ad1_x1) / diff;
        }

        x1 = x;
        ad1_x1 = tanhAD1(x);

        return result;
    }

    // Second-order ADAA tanh (higher quality)
    float processSecondOrderTanh(float x)
    {
        float result;
        float d1 = calcD1Tanh(x, x1);
        float d2 = calcD1Tanh(x1, x2);

        float diff = x - x2;
        if (std::abs(diff) < tolerance)
        {
            // Fallback to first-order
            result = d1;
        }
        else
        {
            result = 2.0f * (d1 - d2) / diff;
        }

        x2 = x1;
        x1 = x;
        ad2_x2 = ad2_x1;
        ad2_x1 = tanhAD2(x);

        return result;
    }

    // First-order ADAA soft clip
    float processFirstOrderSoftClip(float x)
    {
        float result;
        float diff = x - x1;

        if (std::abs(diff) < tolerance)
        {
            result = softClipDerivative((x + x1) * 0.5f);
        }
        else
        {
            result = (softClipAD1(x) - ad1_x1) / diff;
        }

        x1 = x;
        ad1_x1 = softClipAD1(x);

        return result;
    }

    // Asymmetric soft clip with ADAA
    float processAsymmetricSoftClip(float x, float dcOffset)
    {
        float biased = x + dcOffset;
        float result = processFirstOrderSoftClip(biased);
        return result - softClip(dcOffset);  // Remove DC
    }

private:
    float x1 = 0.0f;
    float x2 = 0.0f;
    float ad1_x1 = 0.0f;
    float ad2_x1 = 0.0f;
    float ad2_x2 = 0.0f;

    static constexpr float tolerance = 1e-5f;

    // tanh and its antiderivatives
    static float tanhFunc(float x)
    {
        return std::tanh(x);
    }

    static float tanhDerivative(float x)
    {
        float t = std::tanh(x);
        return 1.0f - t * t;
    }

    // First antiderivative of tanh: ln(cosh(x))
    static float tanhAD1(float x)
    {
        // log(cosh(x)) = log((e^x + e^-x)/2) = x - log(2) + log(1 + e^-2x) for x > 0
        // Use numerically stable form
        float absX = std::abs(x);
        if (absX > 10.0f)
            return absX - 0.693147f;  // ln(2)
        return std::log(std::cosh(x));
    }

    // Second antiderivative of tanh
    static float tanhAD2(float x)
    {
        // Integral of ln(cosh(x)) - approximation
        float absX = std::abs(x);
        if (absX < 0.001f)
            return x * x * 0.5f;
        return x * tanhAD1(x) - 0.5f * x * x + dilog(1.0f + std::exp(-2.0f * absX));
    }

    // Simplified dilogarithm approximation
    static float dilog(float x)
    {
        if (x <= 0.0f) return 0.0f;
        if (x >= 2.0f) return 1.6449f;  // pi^2/6
        return x * (1.0f - x * 0.25f);  // Simple approximation
    }

    float calcD1Tanh(float x0, float x1_)
    {
        float diff = x0 - x1_;
        if (std::abs(diff) < tolerance)
            return tanhFunc((x0 + x1_) * 0.5f);
        return (tanhAD1(x0) - tanhAD1(x1_)) / diff;
    }

    // Soft clip: x / (1 + |x|)
    static float softClip(float x)
    {
        return x / (1.0f + std::abs(x));
    }

    static float softClipDerivative(float x)
    {
        float denom = 1.0f + std::abs(x);
        return 1.0f / (denom * denom);
    }

    // First antiderivative of soft clip: sign(x) * (|x| - ln(1 + |x|))
    static float softClipAD1(float x)
    {
        float absX = std::abs(x);
        float sign = (x >= 0.0f) ? 1.0f : -1.0f;
        return sign * (absX - std::log(1.0f + absX));
    }
};

} // namespace Neve1073
