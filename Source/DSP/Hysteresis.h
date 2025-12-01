#pragma once

#include <cmath>
#include <algorithm>

namespace Neve1073
{

/**
 * Simplified Hysteresis Model for Transformer Cores
 *
 * Based on the Jiles-Atherton model, simplified for real-time audio.
 * Models magnetic hysteresis in transformer cores producing:
 * - Even-order harmonics (especially 2nd)
 * - Frequency-dependent saturation
 * - Memory/history effects
 */
class Hysteresis
{
public:
    Hysteresis() = default;

    void prepare(double sampleRate)
    {
        fs = sampleRate;
        T = 1.0 / fs;
        // DC blocking filter coefficient (~5Hz cutoff)
        dcBlockCoeff = 1.0 - std::exp(-2.0 * 3.14159265359 * 5.0 / fs);
        reset();
    }

    void reset()
    {
        M = 0.0;
        H_prev = 0.0;
        M_prev = 0.0;
        dcBlockState = 0.0;
    }

    void setDrive(float drive)
    {
        Ms = 1.0 + drive * 0.3;
    }

    void setSaturation(float sat)
    {
        a = 25.0 + (1.0 - sat) * 50.0;
    }

    void setWidth(float width)
    {
        k = 0.3 + width * 1.5;
    }

    void setBias(float bias)
    {
        dcBias = bias * 0.05;  // Reduced bias effect
    }

    float process(float input)
    {
        // Gate very small signals to prevent noise/drift
        if (std::abs(input) < 1e-6f)
        {
            // Decay state toward zero
            M *= 0.9999;
            M_prev = M;
            H_prev = 0.0;
            return 0.0f;
        }

        double H_input = static_cast<double>(input);

        // Calculate dH/dt
        double dH = H_input - H_prev;

        // Langevin function for anhysteretic magnetization
        double He = H_input + alpha * M + dcBias;  // Apply bias here
        double Man = langevin(He);

        // Calculate dM/dt using simplified J-A equation
        double dM_dH = 0.0;
        if (std::abs(dH) > 1e-10)
        {
            double delta = (dH >= 0) ? 1.0 : -1.0;
            double denom = k * delta - alpha * (Man - M);

            if (std::abs(denom) > 1e-10)
            {
                dM_dH = (Man - M) / denom;
                // Clamp to prevent instability
                dM_dH = std::clamp(dM_dH, -10.0, 10.0);
            }
        }

        // Integrate
        double dM = dM_dH * dH;
        M = M_prev + dM;

        // Clamp to saturation
        M = std::clamp(M, -Ms, Ms);

        // Update state
        H_prev = H_input;
        M_prev = M;

        // DC blocking on output
        double output = M / Ms;
        dcBlockState += dcBlockCoeff * (output - dcBlockState);
        output = output - dcBlockState;

        return static_cast<float>(output);
    }

private:
    double fs = 44100.0;
    double T = 1.0 / 44100.0;

    // Jiles-Atherton parameters
    double Ms = 1.0;
    double a = 40.0;
    double k = 0.5;
    double alpha = 0.001;  // Reduced coupling
    double dcBias = 0.0;

    // State
    double M = 0.0;
    double H_prev = 0.0;
    double M_prev = 0.0;

    // DC blocking
    double dcBlockCoeff = 0.001;
    double dcBlockState = 0.0;

    double langevin(double x) const
    {
        double xNorm = x / a;
        if (std::abs(xNorm) < 0.0001)
        {
            return Ms * xNorm / 3.0;
        }
        // coth(x) - 1/x, with protection against division by zero
        double cothX = 1.0 / std::tanh(xNorm);
        return Ms * (cothX - 1.0 / xNorm);
    }
};

} // namespace Neve1073
