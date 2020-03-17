//
//  phase_vocoder.h
//  SilenceStripping
//
//  Created by Ryan Purpura on 3/8/20.
//  Copyright © 2020 Ryan Purpura. All rights reserved.
//

#ifndef phase_vocoder_h
#define phase_vocoder_h

#include <stdio.h>
#include <Accelerate/Accelerate.h>
#include <math.h>
#include "fft_helper.h"

#define PI 3.14159265358979323846


typedef struct PhaseVocoderData {
    float *inputFrame;
    int numberOfSamples;
    float *buffer;
    float *outputBuffer;
    int outputBufferPosition;
    float *previousPhase;
    float *outputPhase;
    bool firstTime;
} PhaseVocoderData;

PhaseVocoderData *createPhaseVocoderData(float *inputSamples, int numberOfSamples, float *outputBuffer, int outputBufferPosition);

void hanning(int length, float *output);


void phase_vocoder(PhaseVocoderData *phaseVocoderData, float speed);





#endif /* phase_vocoder_h */
