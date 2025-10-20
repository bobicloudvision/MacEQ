#pragma once

#include <JuceHeader.h>
#include <CoreAudio/CoreAudio.h>

//==============================================================================
/**
 * VirtualAudioDevice provides utilities for working with virtual audio devices
 * on macOS. This includes detection of loopback devices and management of
 * audio routing.
 * 
 * For full system audio capture, you'll need a virtual audio driver like:
 * - BlackHole (https://github.com/ExistentialAudio/BlackHole)
 * - Soundflower
 * - Or a custom CoreAudio driver
 */
class VirtualAudioDevice
{
public:
    //==============================================================================
    struct DeviceInfo
    {
        juce::String name;
        AudioDeviceID deviceID;
        bool isInput;
        bool isOutput;
        int numInputChannels;
        int numOutputChannels;
        double defaultSampleRate;
        bool isVirtual;  // Detected as virtual/aggregate device
    };
    
    //==============================================================================
    VirtualAudioDevice();
    ~VirtualAudioDevice();
    
    //==============================================================================
    // Device discovery
    static juce::Array<DeviceInfo> getAllAudioDevices();
    static juce::Array<DeviceInfo> getVirtualAudioDevices();
    static bool isDeviceVirtual(AudioDeviceID deviceID);
    
    //==============================================================================
    // Aggregate device creation (combines multiple devices)
    static AudioDeviceID createAggregateDevice(const juce::String& name,
                                               AudioDeviceID inputDevice,
                                               AudioDeviceID outputDevice,
                                               juce::String& errorMessage);
    
    static bool destroyAggregateDevice(AudioDeviceID deviceID);
    
    //==============================================================================
    // System default device management
    static AudioDeviceID getSystemDefaultOutputDevice();
    static AudioDeviceID getSystemDefaultInputDevice();
    
    static bool setSystemDefaultOutputDevice(AudioDeviceID deviceID);
    static bool setSystemDefaultInputDevice(AudioDeviceID deviceID);
    
    //==============================================================================
    // Device information
    static juce::String getDeviceName(AudioDeviceID deviceID);
    static juce::String getDeviceManufacturer(AudioDeviceID deviceID);
    static int getDeviceNumChannels(AudioDeviceID deviceID, bool isInput);
    static double getDeviceSampleRate(AudioDeviceID deviceID);
    
    //==============================================================================
    // Virtual device recommendations
    struct VirtualDeviceSetup
    {
        bool hasVirtualDevice;
        juce::String recommendedDevice;
        juce::String setupInstructions;
    };
    
    static VirtualDeviceSetup checkVirtualDeviceSetup();
    
private:
    //==============================================================================
    static juce::String getDeviceStringProperty(AudioDeviceID deviceID, 
                                               AudioObjectPropertySelector selector);
    static UInt32 getDeviceUInt32Property(AudioDeviceID deviceID,
                                         AudioObjectPropertySelector selector);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualAudioDevice)
};

