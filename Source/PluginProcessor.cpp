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

    return {params.begin(), params.end()};
}

void Neve1073Processor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Create 4x oversampling
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        2, 2,  // 2 channels, 2^2 = 4x
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true);
    oversampling->initProcessing(static_cast<size_t>(samplesPerBlock));

    double oversampledRate = sampleRate * 4.0;

    // Prepare all DSP components at oversampled rate
    for (int ch = 0; ch < 2; ++ch)
    {
        inputTransformers[ch].prepare(oversampledRate, samplesPerBlock * 4);
        outputTransformers[ch].prepare(oversampledRate, samplesPerBlock * 4);
        preampStages[ch].prepare(oversampledRate, samplesPerBlock * 4);
        midEQs[ch].prepare(oversampledRate, samplesPerBlock * 4);
        lowShelfEQs[ch].prepare(oversampledRate, samplesPerBlock * 4);
        highShelfEQs[ch].prepare(oversampledRate, samplesPerBlock * 4);
        highPassFilters[ch].prepare(oversampledRate, samplesPerBlock * 4);
    }

    inputGainSmooth.prepare(sampleRate);
    outputGainSmooth.prepare(sampleRate);

    updateDSPFromParameters();
}

void Neve1073Processor::releaseResources()
{
    oversampling.reset();
}

void Neve1073Processor::updateDSPFromParameters()
{
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

    // Update DSP components
    for (int ch = 0; ch < 2; ++ch)
    {
        inputTransformers[ch].setDrive(inputDrive);
        outputTransformers[ch].setDrive(outputDrive);

        preampStages[ch].setGain(preampGain);
        preampStages[ch].setBias(preampBias);
        preampStages[ch].setSaturation(preampGain / 60.0f);  // Scale saturation with gain

        lowShelfEQs[ch].setLowFrequency(static_cast<ShelfEQ::LowFrequency>(lowFreqIdx));
        lowShelfEQs[ch].setGainDb(lowGain);

        midEQs[ch].setFrequency(static_cast<InductorEQ::Frequency>(midFreqIdx));
        midEQs[ch].setGainDb(midGain);

        highShelfEQs[ch].setGainDb(highGain);

        highPassFilters[ch].setFrequency(static_cast<HighPassFilter::Frequency>(hpfFreqIdx));
    }
}

void Neve1073Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Safety check - ensure oversampling is initialized
    if (oversampling == nullptr)
        return;

    // Update parameters
    updateDSPFromParameters();

    float inputGainDb = *parameters.getRawParameterValue(PARAM_INPUT_GAIN);
    float outputGainDb = *parameters.getRawParameterValue(PARAM_OUTPUT_GAIN);
    bool eqEnabled = *parameters.getRawParameterValue(PARAM_EQ_ENABLED) > 0.5f;

    inputGainSmooth.setTargetValue(std::pow(10.0f, inputGainDb / 20.0f));
    outputGainSmooth.setTargetValue(std::pow(10.0f, outputGainDb / 20.0f));

    // Apply input gain with smoothing
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float gain = inputGainSmooth.getNextValue();
        for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
        {
            buffer.setSample(ch, sample, buffer.getSample(ch, sample) * gain);
        }
    }

    // Upsample
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto oversampledBlock = oversampling->processSamplesUp(block);

    int numOversampledSamples = static_cast<int>(oversampledBlock.getNumSamples());
    int numChannels = juce::jmin(2, static_cast<int>(oversampledBlock.getNumChannels()));

    // Process each channel
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = oversampledBlock.getChannelPointer(static_cast<size_t>(ch));

        // Input transformer
        inputTransformers[ch].processBlock(channelData, numOversampledSamples);

        // Preamp stage
        preampStages[ch].processBlock(channelData, numOversampledSamples);

        // EQ section (if enabled)
        if (eqEnabled)
        {
            lowShelfEQs[ch].processBlock(channelData, numOversampledSamples);
            midEQs[ch].processBlock(channelData, numOversampledSamples);
            highShelfEQs[ch].processBlock(channelData, numOversampledSamples);
        }

        // High-pass filter (always active if not "Off")
        highPassFilters[ch].processBlock(channelData, numOversampledSamples);

        // Output transformer
        outputTransformers[ch].processBlock(channelData, numOversampledSamples);
    }

    // Downsample
    oversampling->processSamplesDown(block);

    // Apply output gain with smoothing (reset smoother for fresh pass)
    outputGainSmooth.setTargetValue(std::pow(10.0f, outputGainDb / 20.0f));
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float gain = outputGainSmooth.getNextValue();
        for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
        {
            buffer.setSample(ch, sample, buffer.getSample(ch, sample) * gain);
        }
    }
}

bool Neve1073Processor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only mono or stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input must match output
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
