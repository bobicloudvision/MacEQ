#include "VirtualAudioDevice.h"

//==============================================================================
VirtualAudioDevice::VirtualAudioDevice()
{
}

VirtualAudioDevice::~VirtualAudioDevice()
{
}

//==============================================================================
juce::Array<VirtualAudioDevice::DeviceInfo> VirtualAudioDevice::getAllAudioDevices()
{
    juce::Array<DeviceInfo> devices;
    
    // Get all audio devices from CoreAudio
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioHardwarePropertyDevices;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    UInt32 dataSize = 0;
    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                     &propertyAddress,
                                                     0,
                                                     nullptr,
                                                     &dataSize);
    
    if (status != noErr)
        return devices;
    
    int numDevices = dataSize / sizeof(AudioDeviceID);
    juce::HeapBlock<AudioDeviceID> deviceIDs(numDevices);
    
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                       &propertyAddress,
                                       0,
                                       nullptr,
                                       &dataSize,
                                       deviceIDs.getData());
    
    if (status != noErr)
        return devices;
    
    // Get info for each device
    for (int i = 0; i < numDevices; ++i)
    {
        DeviceInfo info;
        info.deviceID = deviceIDs[i];
        info.name = getDeviceName(deviceIDs[i]);
        info.numInputChannels = getDeviceNumChannels(deviceIDs[i], true);
        info.numOutputChannels = getDeviceNumChannels(deviceIDs[i], false);
        info.isInput = info.numInputChannels > 0;
        info.isOutput = info.numOutputChannels > 0;
        info.defaultSampleRate = getDeviceSampleRate(deviceIDs[i]);
        info.isVirtual = isDeviceVirtual(deviceIDs[i]);
        
        devices.add(info);
    }
    
    return devices;
}

juce::Array<VirtualAudioDevice::DeviceInfo> VirtualAudioDevice::getVirtualAudioDevices()
{
    auto allDevices = getAllAudioDevices();
    juce::Array<DeviceInfo> virtualDevices;
    
    for (const auto& device : allDevices)
    {
        if (device.isVirtual)
            virtualDevices.add(device);
    }
    
    return virtualDevices;
}

bool VirtualAudioDevice::isDeviceVirtual(AudioDeviceID deviceID)
{
    // Check if device transport type indicates virtual/aggregate
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioDevicePropertyTransportType;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    UInt32 transportType = 0;
    UInt32 dataSize = sizeof(UInt32);
    
    OSStatus status = AudioObjectGetPropertyData(deviceID,
                                                &propertyAddress,
                                                0,
                                                nullptr,
                                                &dataSize,
                                                &transportType);
    
    if (status == noErr)
    {
        // Virtual and aggregate devices typically have these transport types
        if (transportType == kAudioDeviceTransportTypeVirtual ||
            transportType == kAudioDeviceTransportTypeAggregate)
        {
            return true;
        }
    }
    
    // Also check device name for common virtual audio drivers
    auto name = getDeviceName(deviceID);
    if (name.containsIgnoreCase("BlackHole") ||
        name.containsIgnoreCase("Soundflower") ||
        name.containsIgnoreCase("Virtual") ||
        name.containsIgnoreCase("Loopback"))
    {
        return true;
    }
    
    return false;
}

//==============================================================================
AudioDeviceID VirtualAudioDevice::getSystemDefaultOutputDevice()
{
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    AudioDeviceID deviceID = kAudioObjectUnknown;
    UInt32 dataSize = sizeof(AudioDeviceID);
    
    AudioObjectGetPropertyData(kAudioObjectSystemObject,
                              &propertyAddress,
                              0,
                              nullptr,
                              &dataSize,
                              &deviceID);
    
    return deviceID;
}

AudioDeviceID VirtualAudioDevice::getSystemDefaultInputDevice()
{
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioHardwarePropertyDefaultInputDevice;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    AudioDeviceID deviceID = kAudioObjectUnknown;
    UInt32 dataSize = sizeof(AudioDeviceID);
    
    AudioObjectGetPropertyData(kAudioObjectSystemObject,
                              &propertyAddress,
                              0,
                              nullptr,
                              &dataSize,
                              &deviceID);
    
    return deviceID;
}

bool VirtualAudioDevice::setSystemDefaultOutputDevice(AudioDeviceID deviceID)
{
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    UInt32 dataSize = sizeof(AudioDeviceID);
    
    OSStatus status = AudioObjectSetPropertyData(kAudioObjectSystemObject,
                                                &propertyAddress,
                                                0,
                                                nullptr,
                                                dataSize,
                                                &deviceID);
    
    return status == noErr;
}

bool VirtualAudioDevice::setSystemDefaultInputDevice(AudioDeviceID deviceID)
{
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioHardwarePropertyDefaultInputDevice;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    UInt32 dataSize = sizeof(AudioDeviceID);
    
    OSStatus status = AudioObjectSetPropertyData(kAudioObjectSystemObject,
                                                &propertyAddress,
                                                0,
                                                nullptr,
                                                dataSize,
                                                &deviceID);
    
    return status == noErr;
}

//==============================================================================
juce::String VirtualAudioDevice::getDeviceName(AudioDeviceID deviceID)
{
    return getDeviceStringProperty(deviceID, kAudioObjectPropertyName);
}

juce::String VirtualAudioDevice::getDeviceManufacturer(AudioDeviceID deviceID)
{
    return getDeviceStringProperty(deviceID, kAudioObjectPropertyManufacturer);
}

int VirtualAudioDevice::getDeviceNumChannels(AudioDeviceID deviceID, bool isInput)
{
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioDevicePropertyStreamConfiguration;
    propertyAddress.mScope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    UInt32 dataSize = 0;
    OSStatus status = AudioObjectGetPropertyDataSize(deviceID,
                                                     &propertyAddress,
                                                     0,
                                                     nullptr,
                                                     &dataSize);
    
    if (status != noErr)
        return 0;
    
    juce::HeapBlock<AudioBufferList> bufferList;
    bufferList.malloc(dataSize, 1);
    
    status = AudioObjectGetPropertyData(deviceID,
                                       &propertyAddress,
                                       0,
                                       nullptr,
                                       &dataSize,
                                       bufferList.getData());
    
    if (status != noErr)
        return 0;
    
    int totalChannels = 0;
    for (UInt32 i = 0; i < bufferList->mNumberBuffers; ++i)
        totalChannels += bufferList->mBuffers[i].mNumberChannels;
    
    return totalChannels;
}

double VirtualAudioDevice::getDeviceSampleRate(AudioDeviceID deviceID)
{
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioDevicePropertyNominalSampleRate;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    Float64 sampleRate = 0.0;
    UInt32 dataSize = sizeof(Float64);
    
    AudioObjectGetPropertyData(deviceID,
                              &propertyAddress,
                              0,
                              nullptr,
                              &dataSize,
                              &sampleRate);
    
    return sampleRate;
}

//==============================================================================
juce::String VirtualAudioDevice::getDeviceStringProperty(AudioDeviceID deviceID,
                                                        AudioObjectPropertySelector selector)
{
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = selector;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMain;
    
    CFStringRef string = nullptr;
    UInt32 dataSize = sizeof(CFStringRef);
    
    OSStatus status = AudioObjectGetPropertyData(deviceID,
                                                &propertyAddress,
                                                0,
                                                nullptr,
                                                &dataSize,
                                                &string);
    
    if (status == noErr && string != nullptr)
    {
        juce::String result = juce::String::fromCFString(string);
        CFRelease(string);
        return result;
    }
    
    return {};
}

//==============================================================================
VirtualAudioDevice::VirtualDeviceSetup VirtualAudioDevice::checkVirtualDeviceSetup()
{
    VirtualDeviceSetup setup;
    setup.hasVirtualDevice = false;
    
    auto virtualDevices = getVirtualAudioDevices();
    
    if (!virtualDevices.isEmpty())
    {
        setup.hasVirtualDevice = true;
        setup.recommendedDevice = virtualDevices[0].name;
        setup.setupInstructions = 
            "Virtual audio device detected: " + setup.recommendedDevice + "\n\n"
            "To route system audio through MacEQ:\n"
            "1. Set '" + setup.recommendedDevice + "' as your System Output in Sound Preferences\n"
            "2. Select '" + setup.recommendedDevice + "' as Input in MacEQ\n"
            "3. Select your real audio device as Output in MacEQ\n"
            "4. Start audio processing in MacEQ";
    }
    else
    {
        setup.setupInstructions = 
            "No virtual audio device found.\n\n"
            "To enable system-wide audio processing, you need a virtual audio device.\n\n"
            "Recommended option: Install BlackHole\n"
            "1. Visit: https://github.com/ExistentialAudio/BlackHole\n"
            "2. Download and install BlackHole (16ch recommended)\n"
            "3. Restart MacEQ\n\n"
            "After installation:\n"
            "- Set BlackHole as System Output in Sound Preferences\n"
            "- Use BlackHole as Input in MacEQ\n"
            "- Use your speakers/headphones as Output in MacEQ";
    }
    
    return setup;
}

//==============================================================================
AudioDeviceID VirtualAudioDevice::createAggregateDevice(const juce::String& name,
                                                       AudioDeviceID inputDevice,
                                                       AudioDeviceID outputDevice,
                                                       juce::String& errorMessage)
{
    // Creating aggregate devices requires more complex CoreAudio setup
    // This is a placeholder for future implementation
    errorMessage = "Aggregate device creation not yet implemented. "
                   "Please use existing virtual audio devices like BlackHole.";
    return kAudioObjectUnknown;
}

bool VirtualAudioDevice::destroyAggregateDevice(AudioDeviceID deviceID)
{
    // Placeholder for future implementation
    juce::ignoreUnused(deviceID);
    return false;
}

