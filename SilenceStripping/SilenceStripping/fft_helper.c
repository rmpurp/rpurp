//
//  fft_helper.c
//  SilenceStripping
//
//  Created by Ryan Purpura on 3/9/20.
//  Copyright © 2020 Ryan Purpura. All rights reserved.
//

#include "fft_helper.h"

FFTData *createFFTData(int numberOfSamples) {
    FFTData *fftData = (FFTData*) malloc(sizeof(fftData));
    vDSP_Length log2n = log2f(numberOfSamples);
    fftData->fftSetup = vDSP_create_fftsetup(log2n, FFT_RADIX2);
    int nOver2 = numberOfSamples / 2;
    fftData->complexA.realp = (float*) malloc((nOver2 + 1) * sizeof(float) );
    fftData->complexA.imagp = (float*) malloc((nOver2 + 1) * sizeof(float) );
    fftData->magnitude = (float *) malloc((nOver2 + 1) * sizeof(float));
    fftData->phase = (float *) malloc((nOver2 + 1) * sizeof(float));
    fftData->firstTime = true;
    fftData->numberOfSamples = numberOfSamples;
    return fftData;
}

void releaseFFTData(FFTData *fftData) {
    vDSP_destroy_fftsetup(fftData->fftSetup);
    free(fftData->complexA.realp);
    free(fftData->complexA.imagp);
    free(fftData);
    fftData = NULL;
}


void computeFFT(FFTData *fftData, float *samples) {
    int nOver2 = fftData->numberOfSamples / 2;
    vDSP_Length log2n = log2f(fftData->numberOfSamples);
    float fftFactor = 1.0 / 2;

    vDSP_ctoz((COMPLEX *)samples, 2, &(fftData->complexA), 1, nOver2);
    vDSP_fft_zrip(fftData->fftSetup, &(fftData->complexA), 1, log2n, FFT_FORWARD);
    
    vDSP_vsmul(fftData->complexA.realp, 1, &fftFactor, fftData->complexA.realp, 1, nOver2);
    vDSP_vsmul(fftData->complexA.imagp, 1, &fftFactor, fftData->complexA.imagp, 1, nOver2);

    fftData->complexA.realp[nOver2] = fftData->complexA.imagp[0];
    fftData->complexA.imagp[nOver2] = 0;
    fftData->complexA.imagp[0] = 0;
    
    vDSP_zvmags(&(fftData->complexA), 1, fftData->magnitude, 1, nOver2 + 1);
    vDSP_zvphas(&(fftData->complexA), 1, fftData->phase, 1, nOver2 + 1);
    fftData->firstTime = false;
}

void computeInverseFFT(FFTData *fftData, float *outputSamples) {
    int nOver2 = fftData->numberOfSamples / 2;
    vDSP_Length log2n = log2f(fftData->numberOfSamples);
    float fftFactor = 1.0 / fftData->numberOfSamples;

    
    vDSP_fft_zrip(fftData->fftSetup, &(fftData->complexA), 1, log2n, FFT_INVERSE);

    vDSP_vsmul(fftData->complexA.realp, 1, &fftFactor, fftData->complexA.realp, 1, nOver2);
    vDSP_vsmul(fftData->complexA.imagp, 1, &fftFactor, fftData->complexA.imagp, 1, nOver2);
        
    vDSP_zvmags(&(fftData->complexA), 1, fftData->magnitude, 1, nOver2 + 1);
    vDSP_zvphas(&(fftData->complexA), 1, fftData->phase, 1, nOver2 + 1);
    
    vDSP_ztoc(&(fftData->complexA), 1, (COMPLEX *)outputSamples, 2, nOver2);    
}
