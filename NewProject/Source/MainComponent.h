#pragma once

#include <JuceHeader.h>
#include "AudioServer.h"
#include "VirtualAudioDevice.h"

//==============================================================================
/*
    Main control interface for the MacEQ audio server.
    Provides device selection, audio routing control, and monitoring.
*/
class MainComponent  : public juce::Component,
                       private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void timerCallback() override;
    
    void startButtonClicked();
    void stopButtonClicked();
    void refreshDevicesButtonClicked();
    void inputDeviceChanged();
    void outputDeviceChanged();
    void bypassChanged();
    
    void updateDeviceLists();
    void updateUIState();
    void checkVirtualDeviceSetup();
    
    //==============================================================================
    // Audio engine
    std::unique_ptr<AudioServer> audioServer;
    
    //==============================================================================
    // UI Components
    juce::GroupComponent deviceGroup;
    
    juce::Label inputDeviceLabel;
    juce::ComboBox inputDeviceCombo;
    
    juce::Label outputDeviceLabel;
    juce::ComboBox outputDeviceCombo;
    
    juce::TextButton refreshDevicesButton;
    juce::TextButton startButton;
    juce::TextButton stopButton;
    
    juce::ToggleButton bypassButton;
    
    juce::GroupComponent statusGroup;
    juce::Label statusLabel;
    juce::TextEditor statusText;
    
    juce::GroupComponent levelGroup;
    juce::Label inputLevelLabel;
    juce::Label outputLevelLabel;
    
    // Level meters (simple text display for now)
    juce::Label inputLevelValueL;
    juce::Label inputLevelValueR;
    juce::Label outputLevelValueL;
    juce::Label outputLevelValueR;
    
    juce::GroupComponent infoGroup;
    juce::TextEditor infoText;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
