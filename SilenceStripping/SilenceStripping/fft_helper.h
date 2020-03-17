//
//  fft_helper.h
//  SilenceStripping
//
//  Created by Ryan Purpura on 3/9/20.
//  Copyright © 2020 Ryan Purpura. All rights reserved.
//

#ifndef fft_helper_h
#define fft_helper_h

#include <stdio.h>
#include <Accelerate/Accelerate.h>
#include <math.h>

// FFT Stuff
typedef struct FFTData {
    FFTSetup fftSetup;
    DSPSplitComplex complexA;
    int numberOfSamples;
    float *magnitude;
    float *phase;
    bool firstTime;
} FFTData;

FFTData *createFFTData(int numberOfSamples);
void releaseFFTData(FFTData *fftHelper);
void computeFFT(FFTData *fftHelperRef, float *samples);
void computeInverseFFT(FFTData *fftData, float *outputSamples);
void test_fft(void);


#endif /* fft_helper_h */
