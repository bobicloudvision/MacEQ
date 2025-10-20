#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(800, 600);
    
    // Initialize audio server
    audioServer = std::make_unique<AudioServer>();
    audioServer->initialize();
    
    // Setup device group
    deviceGroup.setText("Audio Devices");
    deviceGroup.setTextLabelPosition(juce::Justification::centredLeft);
    addAndMakeVisible(deviceGroup);
    
    // Input device
    inputDeviceLabel.setText("Input Device:", juce::dontSendNotification);
    addAndMakeVisible(inputDeviceLabel);
    
    inputDeviceCombo.onChange = [this] { inputDeviceChanged(); };
    addAndMakeVisible(inputDeviceCombo);
    
    // Output device
    outputDeviceLabel.setText("Output Device:", juce::dontSendNotification);
    addAndMakeVisible(outputDeviceLabel);
    
    outputDeviceCombo.onChange = [this] { outputDeviceChanged(); };
    addAndMakeVisible(outputDeviceCombo);
    
    // Refresh devices button
    refreshDevicesButton.setButtonText("Refresh Devices");
    refreshDevicesButton.onClick = [this] { refreshDevicesButtonClicked(); };
    addAndMakeVisible(refreshDevicesButton);
    
    // Control buttons
    startButton.setButtonText("Start Audio Processing");
    startButton.onClick = [this] { startButtonClicked(); };
    startButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green.darker());
    addAndMakeVisible(startButton);
    
    stopButton.setButtonText("Stop Audio Processing");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red.darker());
    stopButton.setEnabled(false);
    addAndMakeVisible(stopButton);
    
    // Bypass button
    bypassButton.setButtonText("Bypass Processing");
    bypassButton.onClick = [this] { bypassChanged(); };
    addAndMakeVisible(bypassButton);
    
    // Status group
    statusGroup.setText("Status");
    statusGroup.setTextLabelPosition(juce::Justification::centredLeft);
    addAndMakeVisible(statusGroup);
    
    statusLabel.setText("Status:", juce::dontSendNotification);
    addAndMakeVisible(statusLabel);
    
    statusText.setMultiLine(true);
    statusText.setReadOnly(true);
    statusText.setCaretVisible(false);
    statusText.setText("Audio server initialized. Select devices and press Start.");
    addAndMakeVisible(statusText);
    
    // Level meters group
    levelGroup.setText("Audio Levels");
    levelGroup.setTextLabelPosition(juce::Justification::centredLeft);
    addAndMakeVisible(levelGroup);
    
    inputLevelLabel.setText("Input:", juce::dontSendNotification);
    addAndMakeVisible(inputLevelLabel);
    
    outputLevelLabel.setText("Output:", juce::dontSendNotification);
    addAndMakeVisible(outputLevelLabel);
    
    inputLevelValueL.setText("L: 0.0", juce::dontSendNotification);
    addAndMakeVisible(inputLevelValueL);
    
    inputLevelValueR.setText("R: 0.0", juce::dontSendNotification);
    addAndMakeVisible(inputLevelValueR);
    
    outputLevelValueL.setText("L: 0.0", juce::dontSendNotification);
    addAndMakeVisible(outputLevelValueL);
    
    outputLevelValueR.setText("R: 0.0", juce::dontSendNotification);
    addAndMakeVisible(outputLevelValueR);
    
    // Info group
    infoGroup.setText("Setup Information");
    infoGroup.setTextLabelPosition(juce::Justification::centredLeft);
    addAndMakeVisible(infoGroup);
    
    infoText.setMultiLine(true);
    infoText.setReadOnly(true);
    infoText.setCaretVisible(false);
    addAndMakeVisible(infoText);
    
    // Initialize
    updateDeviceLists();
    checkVirtualDeviceSetup();
    
    // Start UI update timer
    startTimerHz(10); // 10 Hz update rate for level meters
}

MainComponent::~MainComponent()
{
    stopTimer();
    
    if (audioServer)
    {
        audioServer->stopAudioProcessing();
        audioServer->shutdown();
    }
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    // Device group
    auto deviceBounds = bounds.removeFromTop(150);
    deviceGroup.setBounds(deviceBounds);
    
    auto deviceContent = deviceBounds.reduced(10, 25);
    
    inputDeviceLabel.setBounds(deviceContent.removeFromTop(25).removeFromLeft(100));
    inputDeviceCombo.setBounds(deviceContent.removeFromTop(25).reduced(5, 0));
    
    deviceContent.removeFromTop(5);
    
    outputDeviceLabel.setBounds(deviceContent.removeFromTop(25).removeFromLeft(100));
    outputDeviceCombo.setBounds(deviceContent.removeFromTop(25).reduced(5, 0));
    
    deviceContent.removeFromTop(5);
    
    refreshDevicesButton.setBounds(deviceContent.removeFromTop(30).reduced(5, 0));
    
    bounds.removeFromTop(10);
    
    // Control buttons
    auto buttonRow = bounds.removeFromTop(40);
    startButton.setBounds(buttonRow.removeFromLeft(200).reduced(5, 0));
    stopButton.setBounds(buttonRow.removeFromLeft(200).reduced(5, 0));
    bypassButton.setBounds(buttonRow.removeFromLeft(200).reduced(5, 0));
    
    bounds.removeFromTop(10);
    
    // Status group
    auto statusBounds = bounds.removeFromTop(100);
    statusGroup.setBounds(statusBounds);
    
    auto statusContent = statusBounds.reduced(10, 25);
    statusText.setBounds(statusContent);
    
    bounds.removeFromTop(10);
    
    // Level meters
    auto levelBounds = bounds.removeFromTop(100);
    levelGroup.setBounds(levelBounds);
    
    auto levelContent = levelBounds.reduced(10, 25);
    
    auto inputRow = levelContent.removeFromTop(25);
    inputLevelLabel.setBounds(inputRow.removeFromLeft(60));
    inputLevelValueL.setBounds(inputRow.removeFromLeft(100).reduced(5, 0));
    inputLevelValueR.setBounds(inputRow.removeFromLeft(100).reduced(5, 0));
    
    levelContent.removeFromTop(10);
    
    auto outputRow = levelContent.removeFromTop(25);
    outputLevelLabel.setBounds(outputRow.removeFromLeft(60));
    outputLevelValueL.setBounds(outputRow.removeFromLeft(100).reduced(5, 0));
    outputLevelValueR.setBounds(outputRow.removeFromLeft(100).reduced(5, 0));
    
    bounds.removeFromTop(10);
    
    // Info group
    infoGroup.setBounds(bounds);
    infoText.setBounds(bounds.reduced(10, 25));
}

//==============================================================================
void MainComponent::timerCallback()
{
    // Update level meters
    if (audioServer && audioServer->isRunning())
    {
        float inL = audioServer->getInputLevel(0);
        float inR = audioServer->getInputLevel(1);
        float outL = audioServer->getOutputLevel(0);
        float outR = audioServer->getOutputLevel(1);
        
        inputLevelValueL.setText("L: " + juce::String(inL, 3), juce::dontSendNotification);
        inputLevelValueR.setText("R: " + juce::String(inR, 3), juce::dontSendNotification);
        outputLevelValueL.setText("L: " + juce::String(outL, 3), juce::dontSendNotification);
        outputLevelValueR.setText("R: " + juce::String(outR, 3), juce::dontSendNotification);
    }
}

//==============================================================================
void MainComponent::startButtonClicked()
{
    if (!audioServer)
        return;
    
    if (audioServer->startAudioProcessing())
    {
        statusText.setText("Audio processing started!\n" +
                          juce::String("Sample Rate: ") + juce::String(audioServer->getSampleRate()) + " Hz\n" +
                          juce::String("Buffer Size: ") + juce::String(audioServer->getBufferSize()) + " samples");
        updateUIState();
    }
    else
    {
        statusText.setText("Failed to start audio processing. Check device selection.");
    }
}

void MainComponent::stopButtonClicked()
{
    if (!audioServer)
        return;
    
    audioServer->stopAudioProcessing();
    statusText.setText("Audio processing stopped.");
    updateUIState();
}

void MainComponent::refreshDevicesButtonClicked()
{
    updateDeviceLists();
    checkVirtualDeviceSetup();
}

void MainComponent::inputDeviceChanged()
{
    if (!audioServer)
        return;
    
    auto selectedIndex = inputDeviceCombo.getSelectedItemIndex();
    if (selectedIndex >= 0)
    {
        auto deviceName = inputDeviceCombo.getItemText(selectedIndex);
        audioServer->setInputDevice(deviceName);
        statusText.setText("Input device changed to: " + deviceName);
    }
}

void MainComponent::outputDeviceChanged()
{
    if (!audioServer)
        return;
    
    auto selectedIndex = outputDeviceCombo.getSelectedItemIndex();
    if (selectedIndex >= 0)
    {
        auto deviceName = outputDeviceCombo.getItemText(selectedIndex);
        audioServer->setOutputDevice(deviceName);
        statusText.setText("Output device changed to: " + deviceName);
    }
}

void MainComponent::bypassChanged()
{
    if (!audioServer)
        return;
    
    audioServer->getProcessorChain().setBypassed(bypassButton.getToggleState());
    
    if (bypassButton.getToggleState())
        statusText.setText("Processing bypassed - audio passing through unchanged.");
    else
        statusText.setText("Processing active - EQ applied to audio.");
}

//==============================================================================
void MainComponent::updateDeviceLists()
{
    if (!audioServer)
        return;
    
    // Save current selections
    auto currentInput = inputDeviceCombo.getText();
    auto currentOutput = outputDeviceCombo.getText();
    
    // Update input devices
    inputDeviceCombo.clear();
    auto inputDevices = audioServer->getAvailableInputDevices();
    for (int i = 0; i < inputDevices.size(); ++i)
        inputDeviceCombo.addItem(inputDevices[i], i + 1);
    
    // Restore or select first
    int inputIndex = inputDevices.indexOf(currentInput);
    inputDeviceCombo.setSelectedItemIndex(inputIndex >= 0 ? inputIndex : 0, juce::dontSendNotification);
    
    // Update output devices
    outputDeviceCombo.clear();
    auto outputDevices = audioServer->getAvailableOutputDevices();
    for (int i = 0; i < outputDevices.size(); ++i)
        outputDeviceCombo.addItem(outputDevices[i], i + 1);
    
    // Restore or select first
    int outputIndex = outputDevices.indexOf(currentOutput);
    outputDeviceCombo.setSelectedItemIndex(outputIndex >= 0 ? outputIndex : 0, juce::dontSendNotification);
}

void MainComponent::updateUIState()
{
    bool running = audioServer && audioServer->isRunning();
    
    startButton.setEnabled(!running);
    stopButton.setEnabled(running);
    inputDeviceCombo.setEnabled(!running);
    outputDeviceCombo.setEnabled(!running);
    refreshDevicesButton.setEnabled(!running);
}

void MainComponent::checkVirtualDeviceSetup()
{
    auto setup = VirtualAudioDevice::checkVirtualDeviceSetup();
    infoText.setText(setup.setupInstructions);
    
    if (setup.hasVirtualDevice)
    {
        // Try to auto-select the virtual device as input
        auto inputDevices = audioServer->getAvailableInputDevices();
        int virtualIndex = inputDevices.indexOf(setup.recommendedDevice);
        if (virtualIndex >= 0)
        {
            inputDeviceCombo.setSelectedItemIndex(virtualIndex, juce::sendNotification);
        }
    }
}
