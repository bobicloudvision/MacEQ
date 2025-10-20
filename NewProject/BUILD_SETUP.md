# MacEQ Build Setup Guide

## Prerequisites

### 1. JUCE Framework
You need JUCE installed on your system:

- **Download:** https://juce.com/get-juce/
- **Install Location:** `/Applications/JUCE/` (recommended)
- The Projucer application should be accessible

### 2. Xcode
- **Xcode 14.0** or later
- **macOS SDK 12.0** or later
- Command Line Tools installed: `xcode-select --install`

### 3. Virtual Audio Device (for testing)
- **BlackHole:** https://github.com/ExistentialAudio/BlackHole
- Download and install BlackHole 16ch (recommended)

## Build Steps

### Step 1: Open in Projucer

1. Launch **Projucer** from Applications
2. Open `NewProject.jucer`
3. Verify module paths point to your JUCE installation
   - Go to **Modules** tab
   - Check paths point to `/Applications/JUCE/modules`

### Step 2: Configure Exporter

1. Click on **Xcode (macOS)** in exporters list
2. Under **Debug/Release Configurations:**
   - Verify **Target Name** is set
   - Check **Architecture** is set to your Mac's architecture
   
### Step 3: Add CoreAudio Framework

Since we use CoreAudio APIs directly:

1. In Projucer, select **Xcode (macOS)** exporter
2. Under **Extra Frameworks**, verify/add: `CoreAudio`
3. This should already be included, but verify it's there

### Step 4: Save and Generate Xcode Project

1. Click **Save Project** (Cmd+S) in Projucer
2. Click **Save and Open in IDE**
3. Projucer will generate the Xcode project

### Step 5: Build in Xcode

1. Xcode should open automatically
2. Select **NewProject** scheme
3. Choose your Mac as the destination
4. Build: **Product → Build** (Cmd+B)
5. Run: **Product → Run** (Cmd+R)

## Troubleshooting

### Build Errors

**"JuceHeader.h not found"**
- Ensure JUCE module paths are correct in Projucer
- Re-save the project in Projucer
- Clean build folder in Xcode (Cmd+Shift+K)

**CoreAudio Framework errors**
- Verify CoreAudio is added to Extra Frameworks
- Check deployment target is macOS 12.0 or later

**C++17 Standard errors**
- In Xcode: Build Settings → C++ Language Dialect → C++17 or later

### Runtime Issues

**App crashes on launch**
- Check Console.app for crash logs
- Verify all JUCE modules are properly linked

**No audio devices showing**
- Ensure you have audio permission
- System Settings → Privacy & Security → Microphone
- Add your app to allowed apps

**No virtual devices detected**
- Install BlackHole or Soundflower
- Restart the application
- Click "Refresh Devices" button

## Project Structure After Build

```
NewProject/
├── NewProject.jucer           # JUCE project file
├── README.md                  # Usage documentation
├── BUILD_SETUP.md            # This file
├── Source/                    # Source code
│   ├── Main.cpp
│   ├── MainComponent.h/cpp
│   ├── AudioServer.h/cpp
│   └── VirtualAudioDevice.h/cpp
├── JuceLibraryCode/          # Auto-generated JUCE code
└── Builds/
    └── MacOSX/
        ├── NewProject.xcodeproj/  # Xcode project
        └── build/                  # Build output
```

## Development Workflow

### Making Changes

1. Edit source files in `Source/` directory
2. Build in Xcode (Cmd+B)
3. No need to regenerate from Projucer unless:
   - Adding new source files
   - Changing module settings
   - Modifying project settings

### Adding New Files

1. Create new `.h`/`.cpp` files in `Source/`
2. Open `NewProject.jucer` in Projucer
3. Add files to **File Explorer** tree
4. Save and regenerate Xcode project
5. Build in Xcode

### Debugging

1. Set breakpoints in Xcode
2. Run with debugger (Cmd+R)
3. Check Console output (DBG() statements)
4. Use Instruments for profiling

## Code Signing & Distribution

### Development Build

For local development, Xcode will handle code signing automatically with your Apple ID.

### Distribution

To distribute the app:

1. **Code Sign:** 
   - Need Apple Developer account
   - Configure signing in Xcode
   
2. **Notarize:**
   - Required for distribution outside Mac App Store
   - Use `notarytool` with Xcode

3. **Create Installer:**
   - Package as DMG or PKG
   - Include BlackHole installation instructions

## Performance Optimization

### Release Build

1. Switch to **Release** scheme in Xcode
2. Optimization level: `-O3` (should be default)
3. Enable Link-Time Optimization (LTO)

### Audio Processing

- Keep audio callback code optimized
- No memory allocation in audio thread
- No blocking operations
- Use lock-free data structures for thread communication

## Testing Audio Routing

### Quick Test Setup

1. Install BlackHole
2. System Settings → Sound → Output → BlackHole 16ch
3. Launch MacEQ
4. Input: BlackHole 16ch
5. Output: Your speakers/headphones
6. Start Audio Processing
7. Play any system audio (Music, YouTube, etc.)

### Verify Audio Flow

- Check level meters in MacEQ show activity
- Verify you hear audio through speakers
- Test bypass toggle
- Monitor CPU usage (should be low)

## Next Steps

After successful build:

1. Test basic audio routing
2. Implement EQ bands (in ProcessorChain::process())
3. Add visual spectrum analyzer
4. Create preset system
5. Add menu bar integration

## Getting Help

- **JUCE Forum:** https://forum.juce.com/
- **JUCE Docs:** https://docs.juce.com/
- **CoreAudio Docs:** https://developer.apple.com/documentation/coreaudio

## Notes

- The app requires microphone permission to access audio inputs
- Virtual audio devices are detected automatically
- System audio routing changes don't require app restart
- Background operation mode can be added later

