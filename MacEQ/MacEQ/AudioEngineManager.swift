//
//  AudioEngineManager.swift
//  MacEQ
//
//  System-wide audio equalizer engine
//

import AVFoundation
import Combine

class AudioEngineManager: ObservableObject {
    private let audioEngine = AVAudioEngine()
    private var eqNode: AVAudioUnitEQ!
    
    @Published var isRunning = false
    @Published var availableOutputDevices: [AudioDevice] = []
    @Published var selectedInputDevice: AudioDevice?
    @Published var selectedOutputDevice: AudioDevice?
    @Published var errorMessage: String?
    
    // EQ bands (10-band equalizer)
    let eqBands: [Float] = [32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]
    @Published var eqGains: [Float] = Array(repeating: 0, count: 10) // -12dB to +12dB
    
    init() {
        setupAudio()
        loadAudioDevices()
    }
    
    private func setupAudio() {
        // Create 10-band parametric EQ
        eqNode = AVAudioUnitEQ(numberOfBands: 10)
        eqNode.globalGain = 0
        
        // Configure each band
        for (index, frequency) in eqBands.enumerated() {
            let band = eqNode.bands[index]
            band.frequency = frequency
            band.gain = 0
            band.bypass = false
            band.filterType = .parametric
            
            // Set bandwidth based on frequency
            if frequency < 100 {
                band.bandwidth = 1.0
            } else if frequency < 1000 {
                band.bandwidth = 0.7
            } else {
                band.bandwidth = 0.5
            }
        }
        
        // Setup audio engine
        let inputNode = audioEngine.inputNode
        let outputNode = audioEngine.outputNode
        let mainMixer = audioEngine.mainMixerNode
        
        // Get the format from input
        let inputFormat = inputNode.inputFormat(forBus: 0)
        let outputFormat = outputNode.outputFormat(forBus: 0)
        
        // Attach EQ node
        audioEngine.attach(eqNode)
        
        // Connect: Input -> EQ -> MainMixer -> Output
        audioEngine.connect(inputNode, to: eqNode, format: inputFormat)
        audioEngine.connect(eqNode, to: mainMixer, format: inputFormat)
        audioEngine.connect(mainMixer, to: outputNode, format: outputFormat)
    }
    
    func loadAudioDevices() {
        var devices: [AudioDevice] = []
        
        // Get audio devices using Core Audio
        var propertyAddress = AudioObjectPropertyAddress(
            mSelector: kAudioHardwarePropertyDevices,
            mScope: kAudioObjectPropertyScopeGlobal,
            mElement: kAudioObjectPropertyElementMain
        )
        
        var dataSize: UInt32 = 0
        let status = AudioObjectGetPropertyDataSize(
            AudioObjectID(kAudioObjectSystemObject),
            &propertyAddress,
            0,
            nil,
            &dataSize
        )
        
        if status == noErr {
            let deviceCount = Int(dataSize) / MemoryLayout<AudioDeviceID>.size
            var deviceIDs = [AudioDeviceID](repeating: 0, count: deviceCount)
            
            AudioObjectGetPropertyData(
                AudioObjectID(kAudioObjectSystemObject),
                &propertyAddress,
                0,
                nil,
                &dataSize,
                &deviceIDs
            )
            
            for deviceID in deviceIDs {
                if let device = getAudioDevice(deviceID: deviceID) {
                    devices.append(device)
                }
            }
        }
        
        DispatchQueue.main.async {
            self.availableOutputDevices = devices
            
            // Select default output device
            if let defaultDevice = devices.first(where: { $0.isDefault }) {
                self.selectedOutputDevice = defaultDevice
            } else if let firstDevice = devices.first {
                self.selectedOutputDevice = firstDevice
            }
            
            // Select first available input
            if let defaultInput = devices.first(where: { $0.hasInput }) {
                self.selectedInputDevice = defaultInput
            }
        }
    }
    
    private func getAudioDevice(deviceID: AudioDeviceID) -> AudioDevice? {
        // Get device name
        var propertyAddress = AudioObjectPropertyAddress(
            mSelector: kAudioObjectPropertyName,
            mScope: kAudioObjectPropertyScopeGlobal,
            mElement: kAudioObjectPropertyElementMain
        )
        
        var dataSize: UInt32 = 0
        AudioObjectGetPropertyDataSize(
            deviceID,
            &propertyAddress,
            0,
            nil,
            &dataSize
        )
        
        var deviceName: CFString?
        let nameStatus = withUnsafeMutablePointer(to: &deviceName) { pointer in
            AudioObjectGetPropertyData(
                deviceID,
                &propertyAddress,
                0,
                nil,
                &dataSize,
                pointer
            )
        }
        
        guard nameStatus == noErr, let name = deviceName else { return nil }
        
        // Check if device has output
        propertyAddress.mSelector = kAudioDevicePropertyStreamConfiguration
        propertyAddress.mScope = kAudioDevicePropertyScopeOutput
        dataSize = 0
        
        AudioObjectGetPropertyDataSize(
            deviceID,
            &propertyAddress,
            0,
            nil,
            &dataSize
        )
        
        let hasOutput = dataSize > 0
        
        // Check if device has input
        propertyAddress.mScope = kAudioDevicePropertyScopeInput
        dataSize = 0
        
        AudioObjectGetPropertyDataSize(
            deviceID,
            &propertyAddress,
            0,
            nil,
            &dataSize
        )
        
        let hasInput = dataSize > 0
        
        // Check if default device
        propertyAddress = AudioObjectPropertyAddress(
            mSelector: kAudioHardwarePropertyDefaultOutputDevice,
            mScope: kAudioObjectPropertyScopeGlobal,
            mElement: kAudioObjectPropertyElementMain
        )
        
        var defaultDeviceID: AudioDeviceID = 0
        dataSize = UInt32(MemoryLayout<AudioDeviceID>.size)
        
        AudioObjectGetPropertyData(
            AudioObjectID(kAudioObjectSystemObject),
            &propertyAddress,
            0,
            nil,
            &dataSize,
            &defaultDeviceID
        )
        
        let isDefault = (deviceID == defaultDeviceID)
        
        return AudioDevice(
            id: deviceID,
            name: name as String,
            hasInput: hasInput,
            hasOutput: hasOutput,
            isDefault: isDefault
        )
    }
    
    func setAudioDevice(input: AudioDevice?, output: AudioDevice?) {
        if let inputID = input?.id {
            var propertyAddress = AudioObjectPropertyAddress(
                mSelector: kAudioHardwarePropertyDefaultInputDevice,
                mScope: kAudioObjectPropertyScopeGlobal,
                mElement: kAudioObjectPropertyElementMain
            )
            
            var deviceID = inputID
            let dataSize = UInt32(MemoryLayout<AudioDeviceID>.size)
            
            AudioObjectSetPropertyData(
                AudioObjectID(kAudioObjectSystemObject),
                &propertyAddress,
                0,
                nil,
                dataSize,
                &deviceID
            )
        }
        
        if let outputID = output?.id {
            var propertyAddress = AudioObjectPropertyAddress(
                mSelector: kAudioHardwarePropertyDefaultOutputDevice,
                mScope: kAudioObjectPropertyScopeGlobal,
                mElement: kAudioObjectPropertyElementMain
            )
            
            var deviceID = outputID
            let dataSize = UInt32(MemoryLayout<AudioDeviceID>.size)
            
            AudioObjectSetPropertyData(
                AudioObjectID(kAudioObjectSystemObject),
                &propertyAddress,
                0,
                nil,
                dataSize,
                &deviceID
            )
        }
    }
    
    func start() {
        do {
            if !audioEngine.isRunning {
                try audioEngine.start()
                DispatchQueue.main.async {
                    self.isRunning = true
                    self.errorMessage = nil
                }
            }
        } catch {
            DispatchQueue.main.async {
                self.errorMessage = "Failed to start audio engine: \(error.localizedDescription)"
                self.isRunning = false
            }
        }
    }
    
    func stop() {
        if audioEngine.isRunning {
            audioEngine.stop()
            DispatchQueue.main.async {
                self.isRunning = false
            }
        }
    }
    
    func updateEQBand(index: Int, gain: Float) {
        guard index < eqNode.bands.count else { return }
        eqGains[index] = gain
        eqNode.bands[index].gain = gain
    }
    
    func resetEQ() {
        for i in 0..<eqNode.bands.count {
            eqNode.bands[i].gain = 0
            eqGains[i] = 0
        }
    }
    
    deinit {
        stop()
    }
}

struct AudioDevice: Identifiable, Hashable {
    let id: AudioDeviceID
    let name: String
    let hasInput: Bool
    let hasOutput: Bool
    let isDefault: Bool
}

