/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"

/*
 * Reference C++ implementation using vectormath.
 * This function is meant be used by PS3 unit tests and PC implementation, not by PS3 runtime.
 */

extern "C"
void edgeAnimBlendJointsLinear(EdgeAnimJointTransform* outputJoints, 
                               uint8_t* outputWeights,
                               const EdgeAnimJointTransform* leftJoints,
                               const uint8_t* leftWeights,
                               const EdgeAnimJointTransform* rightJoints, 
                               const uint8_t* rightWeights,
                               float alpha, unsigned int count)
{   
    const uint8_t* const leftWeightsBase = leftWeights;
    const uint8_t* const rightWeightsBase = rightWeights;

    for (int skelLoop = count ; skelLoop ; skelLoop--) {
        
        uint8_t leftByte = leftWeightsBase? *leftWeights : 0xFF;
        uint8_t rightByte = rightWeightsBase? *rightWeights : 0xFF;
        bool leftValid = leftByte != 0;
        bool rightValid = rightByte != 0;

        // Both joints defined - performed a weighted blend
        if (leftValid && rightValid) {   
            float leftW = leftByte * (1.0f / 255.0f);
            float rightW = rightByte * (1.0f / 255.0f);     
            float blendFactor = (rightW > leftW)? ((rightW - leftW + alpha * leftW ) / rightW)
                                                : alpha * rightW / leftW;
            float outputWeight = (1.0f - blendFactor) * leftW + blendFactor * rightW;
            int outW = (int) (outputWeight * 255.0f + 0.5f);
            outW = outW < 0 ? 0 : outW > 255 ? 255 : outW;

            outputJoints->rotation = slerp(blendFactor, leftJoints->rotation, rightJoints->rotation);
            outputJoints->translation = lerp(blendFactor, leftJoints->translation, rightJoints->translation);
            outputJoints->scale = lerp(blendFactor, leftJoints->scale, rightJoints->scale);
            *outputWeights = (unsigned char)(outputWeight * 255.0f + 0.5f);
        } 
        // Only left joint defined - output is defined and is the left joint, regardless of user alpha
        else if (leftValid) {
            outputJoints->rotation = leftJoints->rotation;
            outputJoints->translation = leftJoints->translation;
            outputJoints->scale = leftJoints->scale;
            *outputWeights = leftByte;
        }
        // Only right joint defined - output is defined and is the right joint, regardless of user alpha
        else if (rightValid) {
            outputJoints->rotation = rightJoints->rotation;
            outputJoints->translation = rightJoints->translation;
            outputJoints->scale = rightJoints->scale;
            *outputWeights = rightByte;
        }
        // Not joint defined - output invalid
        else {
            *outputWeights = 0x00;
        }

        // Next     
        leftJoints++;
        leftWeights++;
        rightJoints++;
        rightWeights++;
        outputJoints++;
        outputWeights++;
    }
}
