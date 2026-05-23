/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimEvaluateUserConst( uint32_t constCount,
								 const void* constData,
								 const uint16_t* constTable,
								 float* outputUserChannels )
{
	// constant channels
	const float* constUserData = (const float*) constData;
	for( ; constCount; constCount-- )
	{
		outputUserChannels[ *constTable++ ] = *constUserData++;
	}
}
