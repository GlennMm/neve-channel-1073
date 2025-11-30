#include "Oversampling.h"

namespace Neve1073
{

Oversampling::Oversampling()
{
}

Oversampling::~Oversampling()
{
}

void Oversampling::prepare(double sampleRate, int samplesPerBlock)
{
    // Create 4x oversampling (2^2)
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        2,  // numChannels (stereo)
        oversamplingFactor,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true  // isMaxQuality
    );

    oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));
    oversampledRate = sampleRate * std::pow(2.0, oversamplingFactor);
}

void Oversampling::reset()
{
    if (oversampler != nullptr)
        oversampler->reset();
}

} // namespace Neve1073
