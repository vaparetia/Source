/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ANIM_STRUCTS_PS3_H__
#define __EDGE_ANIM_STRUCTS_PS3_H__

#if !defined(__PPU__) && !defined(__SPU__)
#include <scalar/cpp/vectormath_aos.h>
#else
#include <vectormath/cpp/vectormath_aos.h>
#endif

#include "edge/anim/edgeanim_macros.h"
#include "edge/anim/edgeanim_structs.h"

// Joint transform
// Note: this structure is declared in a separate file to avoid adding a dependency on vectormath to tools

typedef struct EDGE_ALIGNED(16) EdgeAnimJointTransform
{
	Vectormath::Aos::Quat		rotation;
	Vectormath::Aos::Point3		translation;
	Vectormath::Aos::Vector4	scale;
} EdgeAnimJointTransform;


#endif // __EDGE_ANIM_STRUCTS_PS3_H__

