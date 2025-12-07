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
        tanh_x1 = 0.0f;
        tanh_ad1_x1 = 0.0f;
        tanh_x2 = 0.0f;
        tanh_ad2_x1 = 0.0f;
        tanh_ad2_x2 = 0.0f;
        sc_x1 = 0.0f;
        sc_ad1_x1 = 0.0f;
    }

    // First-order ADAA tanh
    float processFirstOrderTanh(float x)
    {
        float result;
        float diff = x - tanh_x1;
        float ad1_x = tanhAD1(x);  // Always compute current antiderivative

        if (std::abs(diff) < tolerance)
        {
            // Use direct function evaluation at midpoint
            result = tanhFunc((x + tanh_x1) * 0.5f);
        }
        else
        {
            result = (ad1_x - tanh_ad1_x1) / diff;
        }

        // Always update state to prevent stale values causing pops
        tanh_ad1_x1 = ad1_x;
        tanh_x1 = x;

        return result;
    }

    // Second-order ADAA tanh (higher quality)
    float processSecondOrderTanh(float x)
    {
        float result;
        float d1 = calcD1Tanh(x, tanh_x1);
        float d2 = calcD1Tanh(tanh_x1, tanh_x2);

        float diff = x - tanh_x2;
        if (std::abs(diff) < tolerance)
        {
            result = d1;
        }
        else
        {
            result = 2.0f * (d1 - d2) / diff;
        }

        tanh_x2 = tanh_x1;
        tanh_x1 = x;
        tanh_ad2_x2 = tanh_ad2_x1;
        tanh_ad2_x1 = tanhAD2(x);

        return result;
    }

    // First-order ADAA soft clip (separate state from tanh)
    float processFirstOrderSoftClip(float x)
    {
        float result;
        float diff = x - sc_x1;
        float ad1_x = softClipAD1(x);  // Always compute current antiderivative

        if (std::abs(diff) < tolerance)
        {
            result = softClip((x + sc_x1) * 0.5f);
        }
        else
        {
            result = (ad1_x - sc_ad1_x1) / diff;
        }

        // Always update state to prevent stale values causing pops
        sc_ad1_x1 = ad1_x;
        sc_x1 = x;

        return result;
    }

    // Asymmetric soft clip with ADAA
    float processAsymmetricSoftClip(float x, float dcOffset)
    {
        float biased = x + dcOffset;
        float result = processFirstOrderSoftClip(biased);
        return result - softClip(dcOffset);
    }

private:
    // Separate state for tanh processing
    float tanh_x1 = 0.0f;
    float tanh_ad1_x1 = 0.0f;
    float tanh_x2 = 0.0f;
    float tanh_ad2_x1 = 0.0f;
    float tanh_ad2_x2 = 0.0f;

    // Separate state for soft clip processing
    float sc_x1 = 0.0f;
    float sc_ad1_x1 = 0.0f;

    static constexpr float tolerance = 1e-5f;

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
        float absX = std::abs(x);
        if (absX > 10.0f)
            return absX - 0.693147f;  // ln(2)
        return std::log(std::cosh(x));
    }

    // Second antiderivative of tanh
    static float tanhAD2(float x)
    {
        float absX = std::abs(x);
        if (absX < 0.001f)
            return x * x * 0.5f;
        return x * tanhAD1(x) - 0.5f * x * x + dilog(1.0f + std::exp(-2.0f * absX));
    }

    static float dilog(float x)
    {
        if (x <= 0.0f) return 0.0f;
        if (x >= 2.0f) return 1.6449f;
        return x * (1.0f - x * 0.25f);
    }

    float calcD1Tanh(float x0, float x1_val)
    {
        float diff = x0 - x1_val;
        if (std::abs(diff) < tolerance)
            return tanhFunc((x0 + x1_val) * 0.5f);
        return (tanhAD1(x0) - tanhAD1(x1_val)) / diff;
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

    static float softClipAD1(float x)
    {
        float absX = std::abs(x);
        float sign = (x >= 0.0f) ? 1.0f : -1.0f;
        return sign * (absX - std::log(1.0f + absX));
    }
};

} // namespace Neve1073
