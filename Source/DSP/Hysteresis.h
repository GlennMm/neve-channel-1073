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
        reset();
    }

    void reset()
    {
        M = 0.0;
        H = 0.0;
        H_prev = 0.0;
        M_prev = 0.0;
    }

    // Hysteresis parameters
    void setDrive(float drive)
    {
        // Scale saturation point
        Ms = 1.0 + drive * 0.5;  // Saturation magnetization
    }

    void setSaturation(float sat)
    {
        // 0-1 range
        a = 20.0 + (1.0 - sat) * 80.0;  // Shape parameter (lower = more saturation)
    }

    void setWidth(float width)
    {
        // Hysteresis loop width (coercivity)
        k = 0.5 + width * 2.0;
    }

    void setBias(float bias)
    {
        // DC bias affects even harmonics
        dcBias = bias * 0.1;
    }

    float process(float input)
    {
        // Apply DC bias for even harmonic generation
        double H_input = static_cast<double>(input) + dcBias;

        // Calculate dH/dt (rate of change of input)
        double dH = (H_input - H_prev) / T;

        // Langevin function for anhysteretic magnetization
        double He = H_input + alpha * M;  // Effective field
        double Man = langevin(He);

        // Calculate dM/dt using simplified J-A equation
        double dM_dH;
        if (std::abs(dH) < 1e-8)
        {
            dM_dH = 0.0;
        }
        else
        {
            double delta = (dH >= 0) ? 1.0 : -1.0;
            double denom = k * delta - alpha * (Man - M);

            if (std::abs(denom) < 1e-8)
            {
                dM_dH = 0.0;
            }
            else
            {
                dM_dH = (Man - M) / denom;
            }
        }

        // Integrate using trapezoidal rule
        double dM = dM_dH * (H_input - H_prev);
        M = M_prev + dM;

        // Clamp to saturation
        M = std::clamp(M, -Ms, Ms);

        // Update state
        H_prev = H_input;
        M_prev = M;

        // Output is magnetization (normalized)
        return static_cast<float>(M / Ms);
    }

private:
    double fs = 44100.0;
    double T = 1.0 / 44100.0;

    // Jiles-Atherton parameters
    double Ms = 1.0;      // Saturation magnetization
    double a = 50.0;      // Shape parameter
    double k = 1.0;       // Coercivity (loop width)
    double alpha = 0.01;  // Inter-domain coupling
    double c = 0.5;       // Reversibility

    double dcBias = 0.0;

    // State
    double M = 0.0;       // Magnetization
    double H = 0.0;       // Field
    double H_prev = 0.0;
    double M_prev = 0.0;

    // Langevin function: L(x) = coth(x) - 1/x
    double langevin(double x) const
    {
        double xNorm = x / a;
        if (std::abs(xNorm) < 0.001)
        {
            // Taylor series for small x
            return Ms * xNorm / 3.0;
        }
        return Ms * (1.0 / std::tanh(xNorm) - 1.0 / xNorm);
    }
};

} // namespace Neve1073
