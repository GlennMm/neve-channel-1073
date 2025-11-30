#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "DSP/TransformerSaturation.h"
#include "DSP/PreampStage.h"
#include "DSP/InductorEQ.h"
#include "DSP/ShelfEQ.h"
#include "DSP/HighPassFilter.h"
#include "DSP/ParameterSmoothing.h"

namespace Neve1073
{

/**
 * Neve 1073 Channel Strip Plugin Processor
 *
 * Signal chain:
 * Input → Input Transformer → Preamp → Low Shelf EQ → Mid Peak EQ →
 * High Shelf EQ → HPF → Output Stage → Output Transformer → Output
 */
class Neve1073Processor : public juce::AudioProcessor
{
public:
    Neve1073Processor();
    ~Neve1073Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter tree
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

    // VU meter levels (for GUI)
    float getInputLevel() const { return inputLevel.load(); }
    float getOutputLevel() const { return outputLevel.load(); }

    // Parameter IDs
    static constexpr const char* PARAM_INPUT_GAIN = "inputGain";
    static constexpr const char* PARAM_OUTPUT_GAIN = "outputGain";
    static constexpr const char* PARAM_INPUT_DRIVE = "inputDrive";
    static constexpr const char* PARAM_OUTPUT_DRIVE = "outputDrive";
    static constexpr const char* PARAM_PREAMP_GAIN = "preampGain";
    static constexpr const char* PARAM_PREAMP_BIAS = "preampBias";
    static constexpr const char* PARAM_LOW_FREQ = "lowFreq";
    static constexpr const char* PARAM_LOW_GAIN = "lowGain";
    static constexpr const char* PARAM_MID_FREQ = "midFreq";
    static constexpr const char* PARAM_MID_GAIN = "midGain";
    static constexpr const char* PARAM_HIGH_GAIN = "highGain";
    static constexpr const char* PARAM_HPF_FREQ = "hpfFreq";
    static constexpr const char* PARAM_EQ_ENABLED = "eqEnabled";
    static constexpr const char* PARAM_MIX = "mix";
    static constexpr const char* PARAM_OVERSAMPLING = "oversampling";
    static constexpr const char* PARAM_QUALITY = "quality";

private:
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP components (per channel)
    std::array<TransformerSaturation, 2> inputTransformers;
    std::array<TransformerSaturation, 2> outputTransformers;
    std::array<PreampStage, 2> preampStages;
    std::array<InductorEQ, 2> midEQs;
    std::array<ShelfEQ, 2> lowShelfEQs;
    std::array<ShelfEQ, 2> highShelfEQs;
    std::array<HighPassFilter, 2> highPassFilters;

    // Smoothed parameters
    SmoothedParameter inputGainSmooth{0.0f, 50.0f};
    SmoothedParameter outputGainSmooth{0.0f, 50.0f};
    SmoothedParameter mixSmooth{1.0f, 50.0f};

    // Oversampling (multiple instances for different factors)
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    int currentOversamplingFactor = 2;  // 2^2 = 4x default
    int lastOversamplingChoice = 2;

    // Dry buffer for mix
    juce::AudioBuffer<float> dryBuffer;

    // Level metering
    std::atomic<float> inputLevel{0.0f};
    std::atomic<float> outputLevel{0.0f};

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    void updateDSPFromParameters();
    void updateOversampling();
    float calculateRMS(const juce::AudioBuffer<float>& buffer) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Neve1073Processor)
};

} // namespace Neve1073
