/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#if defined(__PPU__) || defined(__SPU__)
#include "edge/anim/edgeanim_spu.h"
#else
#include "edge/anim/edgeanim_windows.h"
#endif

/*
 * Reference C++ implementation
 * This function is meant be used by PS3 unit tests and PC implementation, not by PS3 runtime.
 */

extern "C"
void edgeAnimBlendUserRelative(float* outputChannels, 
                               uint8_t* outputWeights,
                               const float* leftChannels,
                               const uint8_t* leftWeights,
                               const float* rightChannels, 
                               const uint8_t* rightWeights,
                               const uint8_t* channelFlags,
                               float alpha, 
                               EdgeAnimRelativeBlendMode blendMode,                            
                               unsigned int count)
{   
    const uint8_t* const leftWeightsBase = leftWeights;
    const uint8_t* const rightWeightsBase = rightWeights;
    const uint8_t* const channelFlagsBase = channelFlags;

    const bool subtract = (blendMode == EDGE_ANIM_RELATIVE_COMPOSE_SUB);

    const bool compose  = (blendMode == EDGE_ANIM_RELATIVE_COMPOSE_ADD)  
                       || (blendMode == EDGE_ANIM_RELATIVE_COMPOSE_SUB);    

    for (int channelLoop = count ; channelLoop ; channelLoop--) {
        
        uint8_t leftWeightByte = leftWeightsBase? *leftWeights : 0xFF;
        uint8_t rightWeightByte = rightWeightsBase? *rightWeights : 0xFF;
        uint8_t flags = channelFlagsBase? *channelFlags : 0x00;

        bool leftValid = leftWeightByte != 0;
        bool rightValid = rightWeightByte !=0;

        bool modeClamp  = !!(flags & EDGE_ANIM_USER_CHANNEL_FLAG_CLAMP01);
        bool modeMinMax = !!(flags & EDGE_ANIM_USER_CHANNEL_FLAG_MINMAX);

        float outputValue = 0.0f;
        uint8_t outputWeightByte = 0x00;
        
        // Both channels defined 
        if (leftValid && rightValid) {          

            float leftValue = *leftChannels;
            float rightValue = *rightChannels;            
                                
            // min/max / fuzzy (hacked for guerrilla)
            // - add = max(leftValue, rightValue)
            // - sub = min(leftValue, rightValue)
            if (modeMinMax) {
                if ((subtract && (leftValue < rightValue)) 
                 || (!subtract && (rightValue < leftValue))) {
                    outputValue = leftValue;                    
                }
                else {
                    outputValue = rightValue;                               
                }
            }
            // default
            // - add mode : performs a weighted blend between leftValue(0.0f) and leftValue + rightValue(1.0f)
            // - compose mode :  leftValue +/- rightValue (no blend)
			// output weight = left weight + right weight
            else {
                float rightW = rightWeightByte * (1.0f / 255.0f);
                float blendFactor = compose? 1.0f: rightW * alpha;
                outputValue = (subtract? -rightValue: rightValue) * blendFactor + leftValue;                
            }
            uint32_t outputWeightInt = (uint32_t) leftWeightByte + (uint32_t)rightWeightByte;
            outputWeightByte = outputWeightInt > 0xFFU? (uint8_t) 0xFFU: (uint8_t) outputWeightInt;
        } 
        // Only leftValue channel defined (add mode) - output is defined and is the leftValue channel, regardless of user alpha
        else if (leftValid && !subtract) {
            outputValue = *leftChannels;            
            outputWeightByte = leftWeightByte;
        }
		// Only rightValue channel defined (compose add mode) - output is defined and is the right joint, regardless of user alpha
        else if (rightValid && compose && !subtract) {
            outputValue = *rightChannels;            
            outputWeightByte = rightWeightByte;
        }
		// Output is invalid for the following cases:
		// - both values undefined
		// - left value undefined (add delta mode)
		// - either value undefined (sub mode)
        else {
            outputWeightByte = 0;
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
