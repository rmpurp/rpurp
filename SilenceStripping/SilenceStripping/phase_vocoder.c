//
//  phase_vocoder.c
//  SilenceStripping
//
//  Created by Ryan Purpura on 3/8/20.
//  Copyright © 2020 Ryan Purpura. All rights reserved.
//

#include "phase_vocoder.h"


void find_peaks(float *inputFrame, int frameSize, bool *output) {
    if (frameSize <= 0 || frameSize >= INT_MAX - 4) {
        exit(1);
    }
    
    int paddedSize = frameSize + 4;
    float paddedInput[paddedSize];
    
    paddedInput[0] = -1;
    paddedInput[1] = -1;
    paddedInput[paddedSize - 2] = -1;
    paddedInput[paddedSize - 1] = -1;
    
    memcpy(paddedInput + 2, inputFrame, frameSize * sizeof(float));
    
    for (int i = 2; i < paddedSize - 2; i++) {
        if (paddedInput[i] >= paddedInput[i - 1]
            && paddedInput[i] >= paddedInput[i - 2]
            && paddedInput[i] >= paddedInput[i + 1]
            && paddedInput[i] >= paddedInput[i + 2]) {
            output[i - 2] = true;
        } else {
            output[i - 2] = false;
        }
    }
}

void getClosestPeaks(bool *peaks, int length, int *closestPeak) {
    int previous = -1;
    for (int i = 0; i < length; i++) {
        if (peaks[i]) {
            if (previous >= 0) {
                for (int j = previous; j < (previous + i) / 2 + 1; j++) {
                    closestPeak[j] = previous;
                }
                
                for (int j = (previous + i) / 2 + 1; j < i; j++) {
                    closestPeak[j] = i;
                }
            } else {
                for (int j = 0; j < i; j++) {
                    closestPeak[j] = i;
                }
            }
            previous = i;
        }
    }
    
    for (int j = previous; j < length; j++) {
        closestPeak[j] = previous;
    }
}

void hanning(int length, float *output) {
    float a = 0;
    float scalingFactor = 2 * PI / length;
    vDSP_vramp(&a, &scalingFactor, output, 1, length);
    vvcosf(output, output, &length);
    
    float negativeOne = -1;
    float one = 1;
    vDSP_vsmsa(output, 1, &negativeOne, &one, output, 1, length);
    
    float oneHalf = 0.5;
    vDSP_vsmul(output, 1, &oneHalf, output, 1, length);
}
PhaseVocoderData *createPhaseVocoderData(float *inputSamples, int numberOfSamples, float *outputBuffer, int outputBufferPosition) {
    PhaseVocoderData *data = (PhaseVocoderData *) malloc(sizeof(PhaseVocoderData));
    data->inputFrame = inputSamples;
    data->numberOfSamples = numberOfSamples;
    data->outputBuffer = outputBuffer;
    data->buffer = malloc( (numberOfSamples / 2 + 1) * sizeof(float));
    data->outputBufferPosition = outputBufferPosition;
    data->previousPhase = (float *) malloc( (numberOfSamples / 2 + 1) * sizeof(float));
    data->outputPhase = (float *) malloc( (numberOfSamples / 2 + 1) * sizeof(float));
    data->firstTime = true;
    return data;
}

void centerFrequency(int n, float* output) {
    int N = n / 2 + 1;
    float a = 0;
    float scalingFactor = 2.0 * PI / n;
    vDSP_vramp(&a, &scalingFactor, output, 1, N);
}


void phase_vocoder(PhaseVocoderData *phaseVocoderData, float speed) {
    int synthesisHop = phaseVocoderData->numberOfSamples / 4;
    int analysisHop = (int) (speed * synthesisHop);
    
    int fftLength = phaseVocoderData->numberOfSamples / 2 + 1;

    float centerFreqs[fftLength];
    centerFrequency(phaseVocoderData->numberOfSamples, centerFreqs);
    
    float analysisWindow[phaseVocoderData->numberOfSamples];
    float synthesisWindow[phaseVocoderData->numberOfSamples];
    
    hanning(phaseVocoderData->numberOfSamples, analysisWindow);
    hanning(phaseVocoderData->numberOfSamples, synthesisWindow);
    

    FFTData *fftData = createFFTData(phaseVocoderData->numberOfSamples);
    computeFFT(fftData, phaseVocoderData->inputFrame);
    
    if (phaseVocoderData->firstTime) {
        memcpy(phaseVocoderData->outputPhase, fftData->phase, fftLength * sizeof(float));
        memcpy(phaseVocoderData->previousPhase, fftData->phase, fftLength * sizeof(float));
        memcpy(phaseVocoderData->outputBufferPosition + phaseVocoderData->outputBuffer,
               phaseVocoderData->inputFrame,
               phaseVocoderData->numberOfSamples * sizeof(float));
        
        phaseVocoderData->outputBufferPosition += phaseVocoderData->numberOfSamples;
    } else {
        float centerFrequency[fftLength];
        // vectorize me
        bool peaks[fftLength];
        find_peaks(phaseVocoderData->inputFrame, phaseVocoderData->numberOfSamples, peaks);
        
        int closestPeaks[fftLength];
        getClosestPeaks(peaks, fftLength, closestPeaks);
        
        for (int i = 0; i < fftLength; i++) {
            if (peaks[i]) {
                phaseVocoderData->buffer[i] = phaseVocoderData->previousPhase[i] - analysisHop * centerFrequency[i];
                phaseVocoderData->buffer[i] += PI;
                phaseVocoderData->buffer[i] = fmodf(phaseVocoderData->buffer[i], 2 * PI);
                phaseVocoderData->buffer[i] -= PI;
                phaseVocoderData->buffer[i] /= analysisHop;
                phaseVocoderData->buffer[i] += centerFreqs[i];
                phaseVocoderData->buffer[i] *= synthesisHop;
                phaseVocoderData->outputPhase[i] += phaseVocoderData->buffer[i];
            }
        }
        for (int i = 0; i < fftLength; i++) {
            phaseVocoderData->outputPhase[i] = phaseVocoderData->outputPhase[closestPeaks[i]];
        }
        
        for (int i = 0; i < fftLength; i++) {
        fftData->complexA.realp[i] = fftData->magnitude[i] * cosf(phaseVocoderData->outputPhase[i]);
        fftData->complexA.imagp[i] = fftData->magnitude[i] * sinf(phaseVocoderData->outputPhase[i]);
        }
        fftData->complexA.imagp[0] = fftData->complexA.realp[fftLength - 1];
        computeInverseFFT(<#FFTData *fftData#>, <#float *outputSamples#>)
        
    }

    releaseFFTData(fftData);
}

//void phase_vocoder(float *inputFrame, float *outputFrame, int frameLength, float speed, float *prevPhase, bool firstTime) {
//    int synthesisHop = frameLength / 4;
//    int analysisHop = (int) (speed * synthesisHop);
//
//    float analysisWindow[frameLength];
//    float synthesisWindow[frameLength];
//    hanning(frameLength, analysisWindow);
//    hanning(frameLength, synthesisWindow);
//
//    int fftLength = frameLength / 2 + 1;
//
//    float real[frameLength / 2];
//    float complex[frameLength / 2];
//    DSPSplitComplex stft;
//    stft.imagp = complex;
//    stft.realp = real;
//    vDSP_ctoz((COMPLEX *)inputFrame, 2, &stft, 1, frameLength);
//    FFTSetup setup = vDSP_create_fftsetup(11, FFT_RADIX2);
//    realFftFreq(frameLength, centerFrequency);
//    vDSP_fft_zip(setup,  &stft, 1, 11, FFT_FORWARD);
//
//
//}

void test_fft(void) {
    float frequencies[] = {1, 5, 25, 30, 75, 100, 300, 500, 512, 1023};
    float tau = PI * 2;
    int len = 2048;
    float signal[len];
    
    FFTData *fftData = createFFTData(len);


    for (int i = 0; i < len; i++) {
        float sum = 0;
        for (int freqIndex = 0; freqIndex < 10; freqIndex++) {
            float normalizedIndex = (float) i / len;
            sum += sinf(normalizedIndex * frequencies[freqIndex] * tau);
        }
        signal[i] = sum;
    }

    for (int j = 0; j < 22; j++) {
        computeFFT(fftData, signal);
    }

//
//    for (int i = 0; i < 1025; i++) {
//        printf("%d\n", i);
//        printf("%f\n", fftData->complexA.realp[i]);
//        printf("%f\n\n", fftData->complexA.imagp[i]);
//    }
    
    releaseFFTData(fftData);
}
