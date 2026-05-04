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
 * Reference C++ implementation using vectormath.
 * This function is meant be used by PS3 unit tests and PC implementation, not by PS3 runtime.
 */

extern "C"
void edgeAnimBlendJointsRelative(EdgeAnimJointTransform* outputJoints, 
                                 uint8_t* outputWeights,
                                 const EdgeAnimJointTransform* leftJoints,
                                 const uint8_t* leftWeights,
                                 const EdgeAnimJointTransform* rightJoints, 
                                 const uint8_t* rightWeights,
                                 float alpha, 
                                 EdgeAnimRelativeBlendMode blendMode,
                                 unsigned int count)
{   
    const uint8_t* const leftWeightsBase = leftWeights;
    const uint8_t* const rightWeightsBase = rightWeights;

    Vectormath::Aos::Point3 kOrigin(0.0f, 0.0f, 0.0f);

    const bool subtract = (blendMode == EDGE_ANIM_RELATIVE_COMPOSE_SUB);

    const bool compose  = (blendMode == EDGE_ANIM_RELATIVE_COMPOSE_ADD)  
                       || (blendMode == EDGE_ANIM_RELATIVE_COMPOSE_SUB);

    for (int skelLoop = count ; skelLoop ; skelLoop--) {
        
        uint8_t leftWeightByte = leftWeightsBase? *leftWeights : 0xFF;
        uint8_t rightWeightByte = rightWeightsBase? *rightWeights : 0xFF;
        bool leftValid = leftWeightByte != 0;
        bool rightValid = rightWeightByte !=0;

        // Both joints defined 
        // - compose mode :  left +/- right (no blend)
        // - add mode : performs a weighted blend between left(0.0f) and left + right(1.0f)
		// output weight = left weight + right weight
        if (leftValid && rightValid) {          
            float rightW = rightWeightByte * (1.0f / 255.0f);
            float blendFactor = compose? 1.0f : rightW * alpha;   
            
			Vectormath::Aos::Quat    rot   = (subtract? Vectormath::Aos::conj(rightJoints->rotation) * leftJoints->rotation 
			                                          : leftJoints->rotation * rightJoints->rotation); 
            Vectormath::Aos::Point3  trans = (subtract? -1.0f
                                                      : 1.0f) * (rightJoints->translation - kOrigin) + leftJoints->translation;
            Vectormath::Aos::Vector4 scale = (subtract? Vectormath::Aos::divPerElem(leftJoints->scale, rightJoints->scale)
                                                      : Vectormath::Aos::mulPerElem(leftJoints->scale, rightJoints->scale));

            outputJoints->rotation = slerp(blendFactor, leftJoints->rotation, rot);
            outputJoints->translation = lerp(blendFactor, leftJoints->translation, trans);
            outputJoints->scale = lerp(blendFactor, leftJoints->scale, scale);

            uint32_t outputWeightInt = (uint32_t) leftWeightByte + (uint32_t) rightWeightByte;
            *outputWeights = outputWeightInt > 0xFFU? (uint8_t) 0xFFU: (uint8_t) outputWeightInt;
        } 
        // Only left joint defined (add mode) - output is defined and is the left joint, regardless of user alpha
        else if (leftValid && !subtract) {
            outputJoints->rotation = leftJoints->rotation;
            outputJoints->translation = leftJoints->translation;
            outputJoints->scale = leftJoints->scale;
            *outputWeights = leftWeightByte;
        }
        // Only right joint defined (compose add mode) - output is defined and is the right joint, regardless of user alpha
        else if (rightValid && compose && !subtract) {
			outputJoints->rotation = rightJoints->rotation;
			outputJoints->translation = rightJoints->translation;
			outputJoints->scale = rightJoints->scale;
			*outputWeights = rightWeightByte;
		}
		// Output is invalid for the following cases:
		// - both joints undefined
		// - left joint undefined (add delta mode)
		// - either joint undefined (sub mode)
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
