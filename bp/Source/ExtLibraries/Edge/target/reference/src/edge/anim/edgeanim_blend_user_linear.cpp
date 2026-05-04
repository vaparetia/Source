/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"

/*
 * Reference C++ implementation 
 * This function is meant be used by PS3 unit tests and PC implementation, not by PS3 runtime.
 */

extern "C"
void edgeAnimBlendUserLinear(float* outputChannels,
                             uint8_t* outputWeights,
                             const float* leftChannels,
                             const uint8_t* leftWeights,
                             const float* rightChannels,
                             const uint8_t* rightWeights,
                             const uint8_t* channelFlags,
                             float alpha, 
                             unsigned int count)
{   
    const uint8_t* const leftWeightsBase = leftWeights;
    const uint8_t* const rightWeightsBase = rightWeights;
    const uint8_t* const channelFlagsBase = channelFlags;

    for (int channelLoop = count ; channelLoop ; channelLoop--) {
        
        uint8_t leftWeightByte = leftWeightsBase? *leftWeights : 0xFF;
        uint8_t rightWeightByte = rightWeightsBase? *rightWeights : 0xFF;
        uint8_t flags = channelFlagsBase? *channelFlags : 0x00;
        bool leftValid = leftWeightByte != 0;
        bool rightValid = rightWeightByte != 0;
    
        bool modeClamp  = !!(flags & EDGE_ANIM_USER_CHANNEL_FLAG_CLAMP01);
    
        float outputValue = 0.0f;
        uint8_t outputWeightByte = 0x00;

        // Both channels defined - performs a weighted blend
        if (leftValid && rightValid) {   
            float leftW = leftWeightByte * (1.0f / 255.0f);
            float rightW = rightWeightByte * (1.0f / 255.0f);     
            float blendFactor = (rightW > leftW)? ((rightW - leftW + alpha * leftW ) / rightW)
                                                : alpha * rightW / leftW;
            float outputWeightFloat = (1.0f - blendFactor) * leftW + blendFactor * rightW;
            int outputWeightInt = (int) (outputWeightFloat * 255.0f + 0.5f);
            outputWeightInt = outputWeightInt < 0 ? 0 : outputWeightInt > 255 ? 255 : outputWeightInt;

            float leftValue = *leftChannels;
            float rightValue = *rightChannels;

            outputValue = (rightValue - leftValue) * blendFactor + leftValue;            
            outputWeightByte = (unsigned char) outputWeightInt;
        } 
        // Only leftValue channels defined - output is defined and is the leftValue channel, regardless of user alpha
        else if (leftValid) {
            outputValue = *leftChannels;
            outputWeightByte = leftWeightByte;
        }
        // Only rightValue channel defined - output is defined and is the rightValue channel, regardless of user alpha
        else if (rightValid) {
            outputValue = *rightChannels;
            outputWeightByte = rightWeightByte;
        }
        // Not channel defined - output invalid
        else {            
            outputWeightByte = 0x00;
        }

        // Clamp to [0..1]?
        if (modeClamp) {
            outputValue = (outputValue > 1.0f)? 1.0f : (outputValue < 0.0f)? 0.0f: outputValue;
        }
        *outputChannels = outputValue;
        *outputWeights = outputWeightByte;


        // Next     
        channelFlags++;
        leftChannels++;
        leftWeights++;
        rightChannels++;
        rightWeights++;
        outputChannels++;
        outputWeights++;
    }
}
