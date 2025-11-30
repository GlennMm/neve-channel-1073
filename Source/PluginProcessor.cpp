#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace Neve1073
{

Neve1073Processor::Neve1073Processor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Neve1073", createParameterLayout())
    , inputTransformers{TransformerSaturation(TransformerSaturation::Type::Input),
                        TransformerSaturation(TransformerSaturation::Type::Input)}
    , outputTransformers{TransformerSaturation(TransformerSaturation::Type::Output),
                         TransformerSaturation(TransformerSaturation::Type::Output)}
    , lowShelfEQs{ShelfEQ(ShelfEQ::Type::LowShelf), ShelfEQ(ShelfEQ::Type::LowShelf)}
    , highShelfEQs{ShelfEQ(ShelfEQ::Type::HighShelf), ShelfEQ(ShelfEQ::Type::HighShelf)}
{
}

Neve1073Processor::~Neve1073Processor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Neve1073Processor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Input/Output gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_INPUT_GAIN, 1},
        "Input Gain",
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f),
        0.0f, "dB"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_OUTPUT_GAIN, 1},
        "Output Gain",
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f),
        0.0f, "dB"));

    // Transformer drives
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_INPUT_DRIVE, 1},
        "Input Drive",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f),
        0.0f, "dB"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_OUTPUT_DRIVE, 1},
        "Output Drive",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f),
        0.0f, "dB"));

    // Preamp
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_PREAMP_GAIN, 1},
        "Preamp Gain",
        juce::NormalisableRange<float>(-10.0f, 60.0f, 0.5f),
        0.0f, "dB"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_PREAMP_BIAS, 1},
        "Preamp Bias",
        juce::NormalisableRange<float>(0.35f, 0.65f, 0.01f),
        0.5f));

    // Low shelf EQ
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{PARAM_LOW_FREQ, 1},
        "Low Freq",
        juce::StringArray{"35 Hz", "60 Hz", "110 Hz", "220 Hz"},
        2));  // Default 110 Hz

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_LOW_GAIN, 1},
        "Low Gain",
        juce::NormalisableRange<float>(-16.0f, 16.0f, 0.1f),
        0.0f, "dB"));

    // Mid peak EQ
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{PARAM_MID_FREQ, 1},
        "Mid Freq",
        juce::StringArray{"360 Hz", "700 Hz", "1.6 kHz", "3.2 kHz", "4.8 kHz", "7.2 kHz"},
        2));  // Default 1.6 kHz

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_MID_GAIN, 1},
        "Mid Gain",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f),
        0.0f, "dB"));

    // High shelf EQ
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_HIGH_GAIN, 1},
        "High Gain",
        juce::NormalisableRange<float>(-16.0f, 16.0f, 0.1f),
        0.0f, "dB"));

    // High-pass filter
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{PARAM_HPF_FREQ, 1},
        "HPF Freq",
        juce::StringArray{"Off", "50 Hz", "80 Hz", "160 Hz", "300 Hz"},
        0));  // Default Off

    // EQ enable
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{PARAM_EQ_ENABLED, 1},
        "EQ Enabled",
        true));

    // Mix (dry/wet)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{PARAM_MIX, 1},
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f, "%"));

    // Oversampling
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{PARAM_OVERSAMPLING, 1},
        "Oversampling",
        juce::StringArray{"1x (Off)", "2x", "4x", "8x"},
        2));  // Default 4x

    // Quality (DSP accuracy)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{PARAM_QUALITY, 1},
        "Quality",
        juce::StringArray{"Eco", "Normal", "High"},
        1));  // Default Normal

    return {params.begin(), params.end()};
}

void Neve1073Processor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Allocate dry buffer for mix
    dryBuffer.setSize(2, samplesPerBlock);

    // Initialize oversampling
    updateOversampling();

    inputGainSmooth.prepare(sampleRate);
    outputGainSmooth.prepare(sampleRate);
    mixSmooth.prepare(sampleRate);

    updateDSPFromParameters();
}

void Neve1073Processor::updateOversampling()
{
    int oversamplingChoice = static_cast<int>(*parameters.getRawParameterValue(PARAM_OVERSAMPLING));

    // Map choice to factor: 0=1x(0), 1=2x(1), 2=4x(2), 3=8x(3)
    currentOversamplingFactor = oversamplingChoice;

    if (currentOversamplingFactor == 0)
    {
        // No oversampling
        oversampling.reset();
    }
    else
    {
        oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
            2, currentOversamplingFactor,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true);
        oversampling->initProcessing(static_cast<size_t>(currentBlockSize));
    }

    // Calculate oversampled rate
    double oversampledRate = currentSampleRate * std::pow(2.0, currentOversamplingFactor);
    int oversampledBlockSize = currentBlockSize * static_cast<int>(std::pow(2, currentOversamplingFactor));

    // Prepare all DSP components at appropriate rate
    for (int ch = 0; ch < 2; ++ch)
    {
        inputTransformers[ch].prepare(oversampledRate, oversampledBlockSize);
        outputTransformers[ch].prepare(oversampledRate, oversampledBlockSize);
        preampStages[ch].prepare(oversampledRate, oversampledBlockSize);
        midEQs[ch].prepare(oversampledRate, oversampledBlockSize);
        lowShelfEQs[ch].prepare(oversampledRate, oversampledBlockSize);
        highShelfEQs[ch].prepare(oversampledRate, oversampledBlockSize);
        highPassFilters[ch].prepare(oversampledRate, oversampledBlockSize);
    }

    lastOversamplingChoice = oversamplingChoice;
}

void Neve1073Processor::releaseResources()
{
    oversampling.reset();
    dryBuffer.setSize(0, 0);
}

void Neve1073Processor::updateDSPFromParameters()
{
    // Check if oversampling changed
    int oversamplingChoice = static_cast<int>(*parameters.getRawParameterValue(PARAM_OVERSAMPLING));
    if (oversamplingChoice != lastOversamplingChoice)
    {
        updateOversampling();
    }

    // Get parameter values
    float inputDrive = *parameters.getRawParameterValue(PARAM_INPUT_DRIVE);
    float outputDrive = *parameters.getRawParameterValue(PARAM_OUTPUT_DRIVE);
    float preampGain = *parameters.getRawParameterValue(PARAM_PREAMP_GAIN);
    float preampBias = *parameters.getRawParameterValue(PARAM_PREAMP_BIAS);

    int lowFreqIdx = static_cast<int>(*parameters.getRawParameterValue(PARAM_LOW_FREQ));
    float lowGain = *parameters.getRawParameterValue(PARAM_LOW_GAIN);

    int midFreqIdx = static_cast<int>(*parameters.getRawParameterValue(PARAM_MID_FREQ));
    float midGain = *parameters.getRawParameterValue(PARAM_MID_GAIN);

    float highGain = *parameters.getRawParameterValue(PARAM_HIGH_GAIN);

    int hpfFreqIdx = static_cast<int>(*parameters.getRawParameterValue(PARAM_HPF_FREQ));

    int qualityIdx = static_cast<int>(*parameters.getRawParameterValue(PARAM_QUALITY));
    auto quality = static_cast<TransformerSaturation::Quality>(qualityIdx);

    // Update DSP components
    for (int ch = 0; ch < 2; ++ch)
    {
        inputTransformers[ch].setDrive(inputDrive);
        inputTransformers[ch].setQuality(quality);
        outputTransformers[ch].setDrive(outputDrive);
        outputTransformers[ch].setQuality(quality);

        preampStages[ch].setGain(preampGain);
        preampStages[ch].setBias(preampBias);
        preampStages[ch].setSaturation(preampGain / 60.0f);

        lowShelfEQs[ch].setLowFrequency(static_cast<ShelfEQ::LowFrequency>(lowFreqIdx));
        lowShelfEQs[ch].setGainDb(lowGain);

        midEQs[ch].setFrequency(static_cast<InductorEQ::Frequency>(midFreqIdx));
        midEQs[ch].setGainDb(midGain);

        highShelfEQs[ch].setGainDb(highGain);

        highPassFilters[ch].setFrequency(static_cast<HighPassFilter::Frequency>(hpfFreqIdx));
    }
}

float Neve1073Processor::calculateRMS(const juce::AudioBuffer<float>& buffer) const
{
    float sum = 0.0f;
    int totalSamples = 0;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            sum += data[i] * data[i];
        }
        totalSamples += buffer.getNumSamples();
    }

    return totalSamples > 0 ? std::sqrt(sum / static_cast<float>(totalSamples)) : 0.0f;
}

void Neve1073Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update parameters
    updateDSPFromParameters();

    float inputGainDb = *parameters.getRawParameterValue(PARAM_INPUT_GAIN);
    float outputGainDb = *parameters.getRawParameterValue(PARAM_OUTPUT_GAIN);
    bool eqEnabled = *parameters.getRawParameterValue(PARAM_EQ_ENABLED) > 0.5f;
    float mixPercent = *parameters.getRawParameterValue(PARAM_MIX);

    inputGainSmooth.setTargetValue(std::pow(10.0f, inputGainDb / 20.0f));
    outputGainSmooth.setTargetValue(std::pow(10.0f, outputGainDb / 20.0f));
    mixSmooth.setTargetValue(mixPercent / 100.0f);

    // Store dry signal for mix
    dryBuffer.makeCopyOf(buffer, true);

    // Calculate input level for VU meter
    inputLevel.store(calculateRMS(buffer));

    // Apply input gain with smoothing
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float gain = inputGainSmooth.getNextValue();
        for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
        {
            buffer.setSample(ch, sample, buffer.getSample(ch, sample) * gain);
        }
    }

    int numChannels = juce::jmin(2, buffer.getNumChannels());

    if (oversampling != nullptr && currentOversamplingFactor > 0)
    {
        // Process with oversampling
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto oversampledBlock = oversampling->processSamplesUp(block);

        int numOversampledSamples = static_cast<int>(oversampledBlock.getNumSamples());

        // Process each channel
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = oversampledBlock.getChannelPointer(static_cast<size_t>(ch));

            inputTransformers[ch].processBlock(channelData, numOversampledSamples);
            preampStages[ch].processBlock(channelData, numOversampledSamples);

            if (eqEnabled)
            {
                lowShelfEQs[ch].processBlock(channelData, numOversampledSamples);
                midEQs[ch].processBlock(channelData, numOversampledSamples);
                highShelfEQs[ch].processBlock(channelData, numOversampledSamples);
            }

            highPassFilters[ch].processBlock(channelData, numOversampledSamples);
            outputTransformers[ch].processBlock(channelData, numOversampledSamples);
        }

        oversampling->processSamplesDown(block);
    }
    else
    {
        // Process without oversampling (1x)
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            int numSamples = buffer.getNumSamples();

            inputTransformers[ch].processBlock(channelData, numSamples);
            preampStages[ch].processBlock(channelData, numSamples);

            if (eqEnabled)
            {
                lowShelfEQs[ch].processBlock(channelData, numSamples);
                midEQs[ch].processBlock(channelData, numSamples);
                highShelfEQs[ch].processBlock(channelData, numSamples);
            }

            highPassFilters[ch].processBlock(channelData, numSamples);
            outputTransformers[ch].processBlock(channelData, numSamples);
        }
    }

    // Apply mix (dry/wet) and output gain
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float mix = mixSmooth.getNextValue();
        float gain = outputGainSmooth.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float wet = buffer.getSample(ch, sample);
            float dry = dryBuffer.getSample(ch, sample);
            float mixed = dry * (1.0f - mix) + wet * mix;
            buffer.setSample(ch, sample, mixed * gain);
        }
    }

    // Calculate output level for VU meter
    outputLevel.store(calculateRMS(buffer));
}

bool Neve1073Processor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

juce::AudioProcessorEditor* Neve1073Processor::createEditor()
{
    return new Neve1073Editor(*this);
}

void Neve1073Processor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Neve1073Processor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
        updateDSPFromParameters();
    }
}

} // namespace Neve1073

// Plugin instantiation
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Neve1073::Neve1073Processor();
}
