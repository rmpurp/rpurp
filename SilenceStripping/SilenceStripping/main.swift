//
//  main.swift
//  SilenceStripping
//
//  Created by Ryan Purpura on 1/31/20.
//  Copyright © 2020 Ryan Purpura. All rights reserved.
//


import Foundation
import AVFoundation

let startTime = CFAbsoluteTimeGetCurrent()

test_fft()


let timeElapsed = CFAbsoluteTimeGetCurrent() - startTime
print("Time elapsed for : \(timeElapsed) s.")

//
let url = URL(fileURLWithPath: "/Users/rpurp/Desktop/beginning.mp3")
let outURL = URL(fileURLWithPath: "/Users/rpurp/Desktop/out.aac")

let outputFormatSettings = [
    AVFormatIDKey: kAudioFormatMPEG4AAC,
    AVSampleRateKey: Float64(44100)
    ] as [String : Any]

let bufferFormatSettings = [
    AVFormatIDKey: kAudioFormatLinearPCM,
    AVLinearPCMBitDepthKey: 32,
    AVLinearPCMIsFloatKey: true,
    AVSampleRateKey: Float64(44100),
    AVNumberOfChannelsKey: 1
    ] as [String : Any]

let audioFile = try AVAudioFile(forReading: url)
let outAudioFile = try AVAudioFile(forWriting: outURL, settings: outputFormatSettings, commonFormat: .pcmFormatFloat32, interleaved: false)

let outputFrameCapacity: UInt32 = 2048
let inputFrameCapacity: UInt32 = 44100

// let silenceSampling: UInt32 = 200

let inputBuffer = AVAudioPCMBuffer(pcmFormat: AVAudioFormat(settings: bufferFormatSettings)!, frameCapacity: inputFrameCapacity)!
let outputBuffer = AVAudioPCMBuffer(pcmFormat: AVAudioFormat(settings: bufferFormatSettings)!, frameCapacity: outputFrameCapacity)!
let outputChannelData = outputBuffer.floatChannelData!.pointee

try audioFile.read(into: inputBuffer, frameCount: inputFrameCapacity)
let inputChannelData = inputBuffer.floatChannelData!.pointee

let pvData: UnsafeMutablePointer<PhaseVocoderData>! = createPhaseVocoderData(inputChannelData, Int32(inputFrameCapacity), outputChannelData, 0)
phase_vocoder(pvData, 1.0)
outputBuffer.frameLength = AVAudioFrameCount(pvData.pointee.outputBufferPosition);
try outAudioFile.write(from: outputBuffer)

print("Starting...")

/*
let crossFadeLength: UInt32 = 100

var count: UInt32 = 0


var fadeOutMultipliers = [Float]()
var fadeInMultipliers = [Float]()

for i in 0..<crossFadeLength {
    let t = (Float(i) - Float(crossFadeLength) / 2.0) / Float(crossFadeLength)
    fadeOutMultipliers.append(sqrt(1.0 / 2.0 * (1 - t)))
    fadeInMultipliers.append(sqrt(1.0 / 2.0 * (1 + t)))

}


while audioFile.framePosition < audioFile.length {
    
    try audioFile.read(into: inputBuffer, frameCount: inputFrameCapacity)
    let inputChannelData = inputBuffer.floatChannelData!.pointee
    
    var sum: Float32 = 0
    
    for i in stride(from: 0, to: Int(inputFrameCapacity), by: Int(inputFrameCapacity / silenceSampling)) {
        sum += abs(inputChannelData[i])
    }
    
    if sum / Float(silenceSampling) > 0.01 {
        // Invariant: There is always inputFrameCapacity space left in the outputChannelData
        outputChannelData.advanced(by: Int(count)).assign(from: inputChannelData, count: Int(inputFrameCapacity))
        count += inputFrameCapacity
        
        
    } else {
        for i in 0..<Int(crossFadeLength) {
            outputChannelData[count] = inputChannelData[i] * fadeOutMultipliers[i] + inputChannelData[inputFrameCapacity] * fadeInMultipliers[i]
            count += 1
        }
    }
    
    if count >= outputFrameCapacity - inputFrameCapacity {
        outputBuffer.frameLength = UInt32(count)
        try outAudioFile.write(from: outputBuffer)
        count = 0
        print("five minutes done.")
    }

    
}

outputBuffer.frameLength = UInt32(count)
try outAudioFile.write(from: outputBuffer)
count = 0


print("Done")
*/
