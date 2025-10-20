#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * AudioServer manages the virtual audio device and routes system audio
 * through the EQ processing chain before outputting to the real hardware.
 */
class AudioServer : public juce::AudioIODeviceCallback
{
public:
    //==============================================================================
    AudioServer();
    ~AudioServer() override;
    
    //==============================================================================
    // Setup and configuration
    bool initialize();
    void shutdown();
    
    bool startAudioProcessing();
    void stopAudioProcessing();
    
    bool isRunning() const { return running; }
    
    //==============================================================================
    // Device management
    juce::StringArray getAvailableInputDevices() const;
    juce::StringArray getAvailableOutputDevices() const;
    
    bool setInputDevice(const juce::String& deviceName);
    bool setOutputDevice(const juce::String& deviceName);
    
    juce::String getCurrentInputDevice() const;
    juce::String getCurrentOutputDevice() const;
    
    //==============================================================================
    // Audio callback from AudioIODevice
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                         int numInputChannels,
                                         float* const* outputChannelData,
                                         int numOutputChannels,
                                         int numSamples,
                                         const juce::AudioIODeviceCallbackContext& context) override;
    
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    
    //==============================================================================
    // Audio processing chain access
    class ProcessorChain
    {
    public:
        ProcessorChain() = default;
        
        void prepare(double sampleRate, int samplesPerBlock, int numChannels);
        void process(juce::AudioBuffer<float>& buffer);
        void reset();
        
        // EQ will be added here
        bool isBypassed() const { return bypassed; }
        void setBypassed(bool shouldBeBypassed) { bypassed = shouldBeBypassed; }
        
    private:
        double currentSampleRate = 44100.0;
        int currentBlockSize = 512;
        int currentNumChannels = 2;
        bool bypassed = false;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorChain)
    };
    
    ProcessorChain& getProcessorChain() { return processorChain; }
    
    //==============================================================================
    // Monitoring
    float getInputLevel(int channel) const;
    float getOutputLevel(int channel) const;
    
    double getSampleRate() const { return currentSampleRate; }
    int getBufferSize() const { return currentBufferSize; }
    
private:
    //==============================================================================
    juce::AudioDeviceManager deviceManager;
    ProcessorChain processorChain;
    
    bool running = false;
    double currentSampleRate = 0.0;
    int currentBufferSize = 0;
    int currentNumInputChannels = 0;
    int currentNumOutputChannels = 0;
    
    // Level monitoring
    std::atomic<float> inputLevels[2] = { 0.0f, 0.0f };
    std::atomic<float> outputLevels[2] = { 0.0f, 0.0f };
    
    // Audio buffer for processing
    juce::AudioBuffer<float> processingBuffer;
    
    //==============================================================================
    void updateLevels(const float* const* inputData, 
                     const float* const* outputData,
                     int numInputs, int numOutputs, int numSamples);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioServer)
};

