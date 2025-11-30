#pragma once

#include <juce_dsp/juce_dsp.h>

namespace Neve1073
{

/**
 * Oversampling wrapper for anti-aliasing
 *
 * Uses JUCE's built-in oversampling with 4x factor
 * for moderate saturation processing.
 */
class Oversampling
{
public:
    Oversampling();
    ~Oversampling();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Get oversampled sample rate
    double getOversampledRate() const { return oversampledRate; }

    // Process with oversampling
    template<typename ProcessFn>
    void process(juce::AudioBuffer<float>& buffer, ProcessFn&& processFn)
    {
        if (oversampler == nullptr)
        {
            processFn(buffer);
            return;
        }

        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto oversampledBlock = oversampler->processSamplesUp(block);

        // Create a temporary buffer from the oversampled block
        juce::AudioBuffer<float> tempBuffer(
            static_cast<int>(oversampledBlock.getNumChannels()),
            static_cast<int>(oversampledBlock.getNumSamples())
        );

        for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
        {
            auto* src = oversampledBlock.getChannelPointer(ch);
            auto* dst = tempBuffer.getWritePointer(static_cast<int>(ch));
            std::copy(src, src + oversampledBlock.getNumSamples(), dst);
        }

        processFn(tempBuffer);

        // Copy back
        for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
        {
            auto* src = tempBuffer.getReadPointer(static_cast<int>(ch));
            auto* dst = oversampledBlock.getChannelPointer(ch);
            std::copy(src, src + oversampledBlock.getNumSamples(), dst);
        }

        oversampler->processSamplesDown(block);
    }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    double oversampledRate = 44100.0;
    static constexpr int oversamplingFactor = 2; // 4x = 2^2
};

} // namespace Neve1073
