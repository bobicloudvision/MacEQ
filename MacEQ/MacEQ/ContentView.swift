//
//  ContentView.swift
//  MacEQ
//
//  System-wide Audio Equalizer
//

import SwiftUI

struct ContentView: View {
    @StateObject private var audioManager = AudioEngineManager()
    @State private var showDeviceSettings = false
    
    var body: some View {
        VStack(spacing: 0) {
            // Header
            HStack {
                Image(systemName: "waveform.path.ecg")
                    .font(.system(size: 24))
                    .foregroundColor(.blue)
                Text("MacEQ")
                    .font(.system(size: 24, weight: .bold))
                
                Spacer()
                
                Button(action: { showDeviceSettings.toggle() }) {
                    Image(systemName: "gearshape.fill")
                        .font(.system(size: 18))
                }
                .buttonStyle(.plain)
                .popover(isPresented: $showDeviceSettings) {
                    DeviceSettingsView(audioManager: audioManager)
                }
            }
            .padding()
            .background(Color(NSColor.windowBackgroundColor))
            
            Divider()
            
            // Power Button
            HStack {
                Spacer()
                
                Button(action: {
                    if audioManager.isRunning {
                        audioManager.stop()
                    } else {
                        audioManager.start()
                    }
                }) {
                    HStack {
                        Image(systemName: audioManager.isRunning ? "stop.circle.fill" : "play.circle.fill")
                            .font(.system(size: 20))
                        Text(audioManager.isRunning ? "Stop" : "Start")
                            .font(.system(size: 16, weight: .semibold))
                    }
                    .foregroundColor(.white)
                    .padding(.horizontal, 24)
                    .padding(.vertical, 12)
                    .background(audioManager.isRunning ? Color.red : Color.green)
                    .cornerRadius(25)
                }
                .buttonStyle(.plain)
                
                Spacer()
            }
            .padding(.vertical, 16)
            
            // Error message
            if let error = audioManager.errorMessage {
                Text(error)
                    .foregroundColor(.red)
                    .font(.caption)
                    .padding(.horizontal)
                    .padding(.bottom, 8)
            }
            
            // Status indicator
            HStack {
                Circle()
                    .fill(audioManager.isRunning ? Color.green : Color.gray)
                    .frame(width: 8, height: 8)
                Text(audioManager.isRunning ? "Active" : "Inactive")
                    .font(.caption)
                    .foregroundColor(.secondary)
            }
            .padding(.bottom, 8)
            
            // EQ Sliders
            ScrollView {
                VStack(spacing: 20) {
                    // Preset buttons
                    HStack(spacing: 12) {
                        PresetButton(title: "Flat", action: {
                            audioManager.resetEQ()
                        })
                        
                        PresetButton(title: "Bass Boost", action: {
                            applyPreset(type: .bassBoost)
                        })
                        
                        PresetButton(title: "Treble Boost", action: {
                            applyPreset(type: .trebleBoost)
                        })
                        
                        PresetButton(title: "Vocal", action: {
                            applyPreset(type: .vocal)
                        })
                    }
                    .padding(.top, 16)
                    
                    Divider()
                        .padding(.vertical, 8)
                    
                    // EQ Bands
                    HStack(alignment: .bottom, spacing: 16) {
                        ForEach(0..<audioManager.eqBands.count, id: \.self) { index in
                            EQSliderView(
                                frequency: formatFrequency(audioManager.eqBands[index]),
                                gain: Binding(
                                    get: { audioManager.eqGains[index] },
                                    set: { newValue in
                                        audioManager.updateEQBand(index: index, gain: newValue)
                                    }
                                )
                            )
                        }
                    }
                    .padding(.horizontal, 20)
                    .padding(.bottom, 20)
                }
            }
            
            Divider()
            
            // Footer with device info
            HStack {
                if let outputDevice = audioManager.selectedOutputDevice {
                    HStack(spacing: 6) {
                        Image(systemName: outputDevice.name.lowercased().contains("bluetooth") ? "antenna.radiowaves.left.and.right" : "speaker.wave.2.fill")
                            .font(.caption)
                        Text(outputDevice.name)
                            .font(.caption)
                            .lineLimit(1)
                    }
                    .foregroundColor(.secondary)
                }
                
                Spacer()
                
                Button("Reset All") {
                    audioManager.resetEQ()
                }
                .buttonStyle(.plain)
                .font(.caption)
            }
            .padding()
            .background(Color(NSColor.windowBackgroundColor))
        }
        .frame(minWidth: 800, minHeight: 500)
    }
    
    private func formatFrequency(_ freq: Float) -> String {
        if freq >= 1000 {
            return String(format: "%.0fK", freq / 1000)
        } else {
            return String(format: "%.0f", freq)
        }
    }
    
    private func applyPreset(type: PresetType) {
        let gains: [Float]
        
        switch type {
        case .bassBoost:
            gains = [8, 6, 4, 2, 0, 0, 0, 0, 0, 0]
        case .trebleBoost:
            gains = [0, 0, 0, 0, 0, 2, 4, 6, 8, 8]
        case .vocal:
            gains = [-2, -1, 0, 2, 4, 4, 2, 0, -1, -2]
        }
        
        for (index, gain) in gains.enumerated() {
            audioManager.updateEQBand(index: index, gain: gain)
        }
    }
    
    enum PresetType {
        case bassBoost, trebleBoost, vocal
    }
}

struct EQSliderView: View {
    let frequency: String
    @Binding var gain: Float
    
    var body: some View {
        VStack(spacing: 8) {
            // Gain value display
            Text(String(format: "%+.1f", gain))
                .font(.caption2)
                .foregroundColor(.secondary)
                .frame(height: 20)
            
            // Vertical slider
            GeometryReader { geometry in
                ZStack(alignment: .bottom) {
                    // Background track
                    RoundedRectangle(cornerRadius: 3)
                        .fill(Color.gray.opacity(0.2))
                        .frame(width: 6)
                    
                    // Center line (0 dB)
                    Rectangle()
                        .fill(Color.blue.opacity(0.5))
                        .frame(width: 20, height: 1)
                        .position(x: geometry.size.width / 2, y: geometry.size.height / 2)
                    
                    // Active track
                    let normalizedGain = CGFloat((gain + 12) / 24) // Normalize from -12...12 to 0...1
                    let centerY = geometry.size.height / 2
                    let sliderY = geometry.size.height * (1 - normalizedGain)
                    
                    if sliderY < centerY {
                        // Positive gain
                        RoundedRectangle(cornerRadius: 3)
                            .fill(LinearGradient(
                                gradient: Gradient(colors: [Color.green, Color.blue]),
                                startPoint: .bottom,
                                endPoint: .top
                            ))
                            .frame(width: 6, height: centerY - sliderY)
                            .offset(y: sliderY - centerY)
                    } else {
                        // Negative gain
                        RoundedRectangle(cornerRadius: 3)
                            .fill(LinearGradient(
                                gradient: Gradient(colors: [Color.blue, Color.orange]),
                                startPoint: .top,
                                endPoint: .bottom
                            ))
                            .frame(width: 6, height: sliderY - centerY)
                    }
                    
                    // Slider thumb
                    Circle()
                        .fill(Color.white)
                        .frame(width: 20, height: 20)
                        .shadow(color: .black.opacity(0.2), radius: 2, x: 0, y: 1)
                        .position(
                            x: geometry.size.width / 2,
                            y: geometry.size.height * (1 - normalizedGain)
                        )
                        .gesture(
                            DragGesture(minimumDistance: 0)
                                .onChanged { value in
                                    let newGain = (1 - value.location.y / geometry.size.height) * 24 - 12
                                    gain = Float(max(-12, min(12, newGain)))
                                }
                        )
                }
                .frame(maxWidth: .infinity)
            }
            .frame(height: 200)
            
            // Frequency label
            Text(frequency)
                .font(.caption)
                .foregroundColor(.primary)
            
            Text("Hz")
                .font(.caption2)
                .foregroundColor(.secondary)
        }
        .frame(width: 50)
    }
}

struct PresetButton: View {
    let title: String
    let action: () -> Void
    
    var body: some View {
        Button(action: action) {
            Text(title)
                .font(.caption)
                .padding(.horizontal, 12)
                .padding(.vertical, 6)
                .background(Color.blue.opacity(0.1))
                .cornerRadius(8)
        }
        .buttonStyle(.plain)
    }
}

struct DeviceSettingsView: View {
    @ObservedObject var audioManager: AudioEngineManager
    
    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text("Audio Device Settings")
                .font(.headline)
                .padding(.bottom, 8)
            
            // Input device selection
            VStack(alignment: .leading, spacing: 8) {
                Text("Input Device:")
                    .font(.subheadline)
                    .foregroundColor(.secondary)
                
                Picker("Input Device", selection: $audioManager.selectedInputDevice) {
                    Text("System Default").tag(nil as AudioDevice?)
                    ForEach(audioManager.availableOutputDevices.filter { $0.hasInput }) { device in
                        Text(device.name).tag(device as AudioDevice?)
                    }
                }
                .labelsHidden()
            }
            
            Divider()
            
            // Output device selection
            VStack(alignment: .leading, spacing: 8) {
                Text("Output Device:")
                    .font(.subheadline)
                    .foregroundColor(.secondary)
                
                Picker("Output Device", selection: $audioManager.selectedOutputDevice) {
                    ForEach(audioManager.availableOutputDevices.filter { $0.hasOutput }) { device in
                        HStack {
                            if device.name.lowercased().contains("bluetooth") {
                                Image(systemName: "antenna.radiowaves.left.and.right")
                            } else {
                                Image(systemName: "speaker.wave.2")
                            }
                            Text(device.name)
                            if device.isDefault {
                                Text("(Default)")
                                    .foregroundColor(.secondary)
                            }
                        }
                        .tag(device as AudioDevice?)
                    }
                }
                .labelsHidden()
            }
            
            Divider()
            
            Button("Refresh Devices") {
                audioManager.loadAudioDevices()
            }
            .buttonStyle(.borderedProminent)
            
            Spacer()
        }
        .padding()
        .frame(width: 350, height: 300)
    }
}

#Preview {
    ContentView()
}
