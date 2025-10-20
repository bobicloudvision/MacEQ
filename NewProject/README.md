# MacEQ - System-Wide Audio EQ for macOS

MacEQ is an audio server application that allows you to apply EQ processing to system-wide audio on macOS. It uses virtual audio devices to route audio through a processing chain before outputting to your speakers or headphones.

## Architecture

The application consists of three main components:

### 1. AudioServer
The core audio routing engine that:
- Manages audio device connections
- Routes audio through the processing chain
- Monitors audio levels
- Handles real-time audio callbacks

### 2. VirtualAudioDevice
A utility class for working with macOS CoreAudio virtual devices:
- Detects virtual audio devices (BlackHole, Soundflower, etc.)
- Provides device information and capabilities
- Manages system default audio device settings
- Future: Create aggregate devices programmatically

### 3. ProcessorChain
The audio processing pipeline where EQ and effects will be applied:
- Currently a passthrough (bypass mode)
- Ready for EQ implementation
- Handles sample rate and channel configuration

## Setup Requirements

### Virtual Audio Device

To route system audio through MacEQ, you need a virtual audio device. We recommend **BlackHole**:

1. **Download BlackHole:**
   - Visit: https://github.com/ExistentialAudio/BlackHole
   - Download BlackHole 16ch (recommended) or 2ch
   - Install the package

2. **Configure System Audio:**
   - Open **System Settings → Sound**
   - Set **Output** to **BlackHole 16ch** (or 2ch)
   - This routes all system audio to the virtual device

3. **Configure MacEQ:**
   - Set **Input Device** to **BlackHole 16ch**
   - Set **Output Device** to your actual audio hardware (speakers/headphones)
   - Click **Start Audio Processing**

### Audio Flow Diagram

```
[System Audio] → [BlackHole] → [MacEQ Input] → [EQ Processing] → [Hardware Output] → [Speakers]
```

## How to Use

### Basic Operation

1. **Launch MacEQ**
   - The app will scan for available audio devices
   - It will detect if virtual audio devices are available

2. **Select Devices**
   - **Input Device:** Choose the virtual device (e.g., BlackHole)
   - **Output Device:** Choose your real audio hardware

3. **Start Processing**
   - Click "Start Audio Processing"
   - Audio levels should appear in the level meters
   - You should hear system audio through your speakers

4. **Bypass/Enable Processing**
   - Toggle "Bypass Processing" to test audio routing
   - Bypassed: audio passes through unchanged
   - Active: EQ processing is applied (once implemented)

### Troubleshooting

**No sound after starting processing:**
- Check that BlackHole is set as System Output
- Verify correct input/output devices are selected
- Check macOS Sound settings
- Ensure your output device is not muted

**No virtual devices detected:**
- Install BlackHole or another virtual audio driver
- Restart MacEQ after installation
- Click "Refresh Devices"

**Audio crackling or glitches:**
- Try increasing buffer size in audio preferences
- Close other audio applications
- Check CPU usage

## Development

### Building from Source

This project uses JUCE. To build:

1. Open `NewProject.jucer` in Projucer
2. Generate Xcode project
3. Build in Xcode

### Project Structure

```
Source/
├── Main.cpp                  # Application entry point
├── MainComponent.h/cpp       # Main UI and control interface
├── AudioServer.h/cpp         # Audio routing and device management
└── VirtualAudioDevice.h/cpp  # CoreAudio device utilities
```

### Adding EQ Processing

The EQ will be implemented in the `ProcessorChain` class:

```cpp
void AudioServer::ProcessorChain::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    // Initialize EQ bands here
}

void AudioServer::ProcessorChain::process(juce::AudioBuffer<float>& buffer)
{
    // Apply EQ processing here
}
```

## Future Features

- [ ] Parametric EQ with multiple bands
- [ ] Visual frequency spectrum analyzer
- [ ] Preset management
- [ ] Auto-detect optimal audio routing
- [ ] Create aggregate devices programmatically
- [ ] Support for multi-channel audio
- [ ] Real-time frequency response visualization
- [ ] System menu bar integration
- [ ] Background operation mode

## Technical Details

### CoreAudio Integration

The application uses CoreAudio APIs to:
- Enumerate audio devices
- Detect virtual/aggregate devices
- Get/set system default devices
- Query device capabilities

### Audio Callback

The `audioDeviceIOCallbackWithContext` method is called in real-time by the audio system:
- Low-latency processing requirements
- Must not allocate memory or block
- Processes audio in fixed-size buffers

### Thread Safety

- Audio processing happens on a real-time thread
- UI updates happen on the message thread
- Level meters use atomic operations for thread-safe communication

## License

[Add your license here]

## Credits

Built with [JUCE](https://juce.com/) framework.

Virtual audio device support via [BlackHole](https://github.com/ExistentialAudio/BlackHole).

