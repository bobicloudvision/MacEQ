#include "AudioServer.h"

//==============================================================================
AudioServer::AudioServer()
{
}

AudioServer::~AudioServer()
{
    shutdown();
}

//==============================================================================
bool AudioServer::initialize()
{
    // Initialize the audio device manager
    auto result = deviceManager.initialiseWithDefaultDevices(2, 2);
    
    if (!result.isEmpty())
    {
        DBG("Failed to initialize audio device manager: " + result);
        return false;
    }
    
    return true;
}

void AudioServer::shutdown()
{
    stopAudioProcessing();
    deviceManager.closeAudioDevice();
}

//==============================================================================
bool AudioServer::startAudioProcessing()
{
    if (running)
        return true;
    
    // Set this object as the audio callback
    deviceManager.addAudioCallback(this);
    
    // Try to open the default audio device if not already open
    if (deviceManager.getCurrentAudioDevice() == nullptr)
    {
        auto error = deviceManager.initialiseWithDefaultDevices(2, 2);
        if (!error.isEmpty())
        {
            DBG("Failed to open audio device: " + error);
            deviceManager.removeAudioCallback(this);
            return false;
        }
    }
    
    running = true;
    DBG("Audio processing started");
    return true;
}

void AudioServer::stopAudioProcessing()
{
    if (!running)
        return;
    
    deviceManager.removeAudioCallback(this);
    running = false;
    
    DBG("Audio processing stopped");
}

//==============================================================================
juce::StringArray AudioServer::getAvailableInputDevices() const
{
    juce::StringArray devices;
    
    if (auto* deviceType = deviceManager.getCurrentDeviceTypeObject())
    {
        auto inputDeviceNames = deviceType->getDeviceNames(true);
        for (const auto& name : inputDeviceNames)
            devices.add(name);
    }
    
    return devices;
}

juce::StringArray AudioServer::getAvailableOutputDevices() const
{
    juce::StringArray devices;
    
    if (auto* deviceType = deviceManager.getCurrentDeviceTypeObject())
    {
        auto outputDeviceNames = deviceType->getDeviceNames(false);
        for (const auto& name : outputDeviceNames)
            devices.add(name);
    }
    
    return devices;
}

bool AudioServer::setInputDevice(const juce::String& deviceName)
{
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.inputDeviceName = deviceName;
    
    auto error = deviceManager.setAudioDeviceSetup(setup, true);
    
    if (!error.isEmpty())
    {
        DBG("Failed to set input device: " + error);
        return false;
    }
    
    return true;
}

bool AudioServer::setOutputDevice(const juce::String& deviceName)
{
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.outputDeviceName = deviceName;
    
    auto error = deviceManager.setAudioDeviceSetup(setup, true);
    
    if (!error.isEmpty())
    {
        DBG("Failed to set output device: " + error);
        return false;
    }
    
    return true;
}

juce::String AudioServer::getCurrentInputDevice() const
{
    if (auto* device = deviceManager.getCurrentAudioDevice())
        return device->getName();
    
    return {};
}

juce::String AudioServer::getCurrentOutputDevice() const
{
    if (auto* device = deviceManager.getCurrentAudioDevice())
        return device->getName();
    
    return {};
}

//==============================================================================
void AudioServer::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                   int numInputChannels,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(context);
    
    // Ensure processing buffer is the right size
    if (processingBuffer.getNumChannels() != numOutputChannels ||
        processingBuffer.getNumSamples() < numSamples)
    {
        processingBuffer.setSize(numOutputChannels, numSamples, false, false, true);
    }
    
    // Copy input to processing buffer
    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (channel < numInputChannels && inputChannelData[channel] != nullptr)
        {
            // Copy input to output
            processingBuffer.copyFrom(channel, 0, inputChannelData[channel], numSamples);
        }
        else
        {
            // Clear channel if no input
            processingBuffer.clear(channel, 0, numSamples);
        }
    }
    
    // Apply processing chain
    processorChain.process(processingBuffer);
    
    // Copy processed audio to output
    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (outputChannelData[channel] != nullptr)
        {
            juce::FloatVectorOperations::copy(outputChannelData[channel],
                                              processingBuffer.getReadPointer(channel),
                                              numSamples);
        }
    }
    
    // Update level meters
    updateLevels(inputChannelData, outputChannelData, 
                numInputChannels, numOutputChannels, numSamples);
}

void AudioServer::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device == nullptr)
        return;
    
    currentSampleRate = device->getCurrentSampleRate();
    currentBufferSize = device->getCurrentBufferSizeSamples();
    currentNumInputChannels = device->getActiveInputChannels().countNumberOfSetBits();
    currentNumOutputChannels = device->getActiveOutputChannels().countNumberOfSetBits();
    
    DBG("Audio device starting: " + juce::String(currentSampleRate) + " Hz, " +
        juce::String(currentBufferSize) + " samples, " +
        juce::String(currentNumInputChannels) + " in, " +
        juce::String(currentNumOutputChannels) + " out");
    
    // Prepare the processor chain
    processorChain.prepare(currentSampleRate, currentBufferSize, 
                          juce::jmax(currentNumInputChannels, currentNumOutputChannels));
    
    // Allocate processing buffer
    processingBuffer.setSize(currentNumOutputChannels, currentBufferSize);
}

void AudioServer::audioDeviceStopped()
{
    DBG("Audio device stopped");
    processorChain.reset();
}

//==============================================================================
void AudioServer::updateLevels(const float* const* inputData,
                               const float* const* outputData,
                               int numInputs, int numOutputs, int numSamples)
{
    // Update input levels
    for (int ch = 0; ch < juce::jmin(numInputs, 2); ++ch)
    {
        if (inputData[ch] != nullptr)
        {
            auto level = juce::FloatVectorOperations::findMaximum(inputData[ch], numSamples);
            inputLevels[ch].store(level, std::memory_order_relaxed);
        }
    }
    
    // Update output levels
    for (int ch = 0; ch < juce::jmin(numOutputs, 2); ++ch)
    {
        if (outputData[ch] != nullptr)
        {
            auto level = juce::FloatVectorOperations::findMaximum(outputData[ch], numSamples);
            outputLevels[ch].store(level, std::memory_order_relaxed);
        }
    }
}

float AudioServer::getInputLevel(int channel) const
{
    if (channel >= 0 && channel < 2)
        return inputLevels[channel].load(std::memory_order_relaxed);
    
    return 0.0f;
}

float AudioServer::getOutputLevel(int channel) const
{
    if (channel >= 0 && channel < 2)
        return outputLevels[channel].load(std::memory_order_relaxed);
    
    return 0.0f;
}

//==============================================================================
// ProcessorChain implementation
//==============================================================================
void AudioServer::ProcessorChain::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    currentNumChannels = numChannels;
    
    // Future: Initialize EQ bands here
}

void AudioServer::ProcessorChain::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;
    
    // Future: Apply EQ processing here
    // For now, audio passes through unchanged
}

void AudioServer::ProcessorChain::reset()
{
    // Future: Reset EQ state here
}

