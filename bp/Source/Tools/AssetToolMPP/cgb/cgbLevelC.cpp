/* SCE CONFIDENTIAL
 * PLAYSTATION(R)3 Programmer Tool Runtime Library 200.002
 *                Copyright (C) 2007 Sony Computer Entertainment Inc.
 *                                               All Rights Reserved.
 */

#include <string.h>
#include <stdio.h>

#include "format/cgb_format.h"

#include <Cg/cg.h>

typedef struct CellCgbProgram
{
   const CellCgbHeader *header;

   const char *ucode;

   size_t constantCount;
   const unsigned short *constantResources;
   const float *constantValues;

   size_t mapEntryCount;
   const CellCgbMapEntry *mapEntries;
   size_t fpOffsetCount;
   const unsigned short *fpOffsets;
   const char *stringTable;

   unsigned int uniformCount;

   const CellCgbParameterInfo* parameterInfos;

} CellCgbProgram;

//documentation
/** @file */

// vertex and fragment configuration structures

#if !defined(WIN32) && !defined(LINUX)
#define ENDSWAP(a)	a
#else

#define ENDSWAP(a) convert_endianness(a)
template< typename T > inline T convert_endianness( const T value)
{
	if ( sizeof( T ) == 1 )
	{
		return value;
	}
	if ( sizeof( T ) == 2 )
	{
		return ( ((value & 0x00FF) << 8)
			| ((value & 0xFF00) >> 8) );
	}
	if ( sizeof( T ) == 4 )
	{
		return ( ((value & 0x000000FF) << 24)
			| ((value & 0x0000FF00) << 8)
			| ((value & 0x00FF0000) >> 8)
			| ((value & 0xFF000000) >> 24) );
	}
	if ( sizeof( T ) == 8 )
	{
		T result = value;
		for ( int ii = 0; ii < 4; ++ii )
		{
			char ch = *( (( char* ) &result) + ii );
			*( (( char* ) &result) +      ii  ) = *( (( char* ) &result) + (7 - ii) );
			*( (( char* ) &result) + (7 - ii) ) = ch;
		}
		return result;
	}
	// exception
	return value;
}

#endif



#define PAD16(a) ((unsigned int)(a)+15 & ~(16-1))

extern "C"
{


/**
@short Gets the Cg type of a given parameter with a map entry.

This function returns the Cg type of a given map entry.
Use this function to retrieve Cg type.
The behavior of the function is not specified if <c><i>map_index</i></c> has not been returned by cellCgbMapLookup(),
or is not in the range 0 to cellCgbMapGetLength()-1

@param program The Cgb program handle
@param map_index The index in the map for this parameter

@return Returns the Cg type at the given map index in the info table.

@sa cellCgbMapLookup, cellCgbLevelCMapGetCgResource, cellCgbLevelCMapGetVariability, cellCgbLevelCMapGetDirection, cellCgbLevelCMapGetValueCount
*/
uint16_t cellCgbLevelCMapGetCgType(CellCgbProgram *program, const uint32_t map_index)
{
    if (program->parameterInfos)
	{
	    const CellCgbParameterInfo *info = program->parameterInfos + map_index;
	    return (uint16_t)ENDSWAP(info->cg_type);
	}
    return 0xffff;
}

/**
@short Gets the Cg resource of a given parameter with a map entry.

This function returns the Cg resource of a given map entry.
Use this function to retrieve Cg resource.
The behavior of the function is not specified if <c><i>map_index</i></c> has not been returned by cellCgbMapLookup(),
or is not in the range 0 to cellCgbMapGetLength()-1

@param program The Cgb program handle
@param map_index The index in the map for this parameter

@return Returns the Cg resource at the given map index in the info table.

@sa cellCgbMapLookup, cellCgbLevelCMapGetCgType, cellCgbLevelCMapGetVariability, cellCgbLevelCMapGetDirection, cellCgbLevelCMapGetValueCount
*/
uint16_t cellCgbLevelCMapGetCgResource(CellCgbProgram *program, const uint32_t map_index)
{
    if (program->parameterInfos)
	{
	    const CellCgbParameterInfo *info = program->parameterInfos + map_index;
	    return (uint16_t)ENDSWAP(info->cg_resource);
	}
    return 0xffff;
}

/**
@short Gets the parameter variability of a given parameter with a map entry.

This function returns the parameter variability of a given map entry.
Use this function to retrieve parameter variability.
The behavior of the function is not specified if <c><i>map_index</i></c> has not been returned by cellCgbMapLookup(),
or is not in the range 0 to cellCgbMapGetLength()-1

@param program The Cgb program handle
@param map_index The index in the map for this parameter

@return Returns the parameter variability at the given map index in the info table.

@sa cellCgbMapLookup, cellCgbLevelCMapGetCgType, cellCgbLevelCMapGetCgResource, cellCgbLevelCMapGetDirection, cellCgbLevelCMapGetValueCount
*/
uint16_t cellCgbLevelCMapGetVariability(CellCgbProgram *program, const uint32_t map_index)
{
    if (program->parameterInfos)
	{
	    const CellCgbParameterInfo *info = program->parameterInfos + map_index;
	    uint16_t variability = ENDSWAP(info->flags) & CGPV_MASK;
	    if (variability == CGPV_VARYING)
		return CG_VARYING;
	    else if (variability == CGPV_UNIFORM)
		return CG_UNIFORM;
	    else if (variability == CGPV_CONSTANT)
		return CG_CONSTANT;
	    else if (variability == CGPV_MIXED)
		return CG_MIXED;
	}
    return 0xffff;
}

/**
@short Gets the parameter direction of a given parameter with a map entry.

This function returns the parameter direction of a given map entry.
Use this function to retrieve parameter direction.
The behavior of the function is not specified if <c><i>map_index</i></c> has not been returned by cellCgbMapLookup(),
or is not in the range 0 to cellCgbMapGetLength()-1

@param program The Cgb program handle
@param map_index The index in the map for this parameter

@return Returns the parameter direction at the given map index in the info table.

@sa cellCgbMapLookup, cellCgbLevelCMapGetCgType, cellCgbLevelCMapGetCgResource, cellCgbLevelCMapGetVariability, cellCgbLevelCMapGetValueCount
*/
uint16_t cellCgbLevelCMapGetDirection(CellCgbProgram *program, const uint32_t map_index)
{
    if (program->parameterInfos)
	{
	    const CellCgbParameterInfo *info = program->parameterInfos + map_index;
	    uint16_t direction = ENDSWAP(info->flags) & CGPD_MASK;
	    if (direction == CGPD_IN)
		return CG_IN;
	    else if (direction == CGPD_OUT)
		return CG_OUT;
	    else if (direction == CGPD_INOUT)
		return CG_INOUT;
	}
    return 0xffff;
}

/**
@short Gets the value count of a given parameter with a map entry.

This function returns the value count of a given map entry.
Use this function to retrieve value count.
The behavior of the function is not specified if <c><i>map_index</i></c> has not been returned by cellCgbMapLookup(),
or is not in the range 0 to cellCgbMapGetLength()-1

@param program The Cgb program handle
@param map_index The index in the map for this parameter

@return Returns the value count at the given map index in the info table.

@sa cellCgbMapLookup, cellCgbLevelCMapGetCgType, cellCgbLevelCMapGetCgResource, cellCgbLevelCMapGetVariability, cellCgbLevelCMapGetDirection
*/
uint16_t cellCgbLevelCMapGetValueCount(CellCgbProgram *program, const uint32_t map_index)
{
    if (program->parameterInfos)
	{
	    const CellCgbParameterInfo *info = program->parameterInfos + map_index;
	    return (uint16_t)ENDSWAP(info->value_count);
	}
    return 0xffff;
}

bool cellCgbLevelCMapIsArray(CellCgbProgram *program, const uint32_t map_index)
{
   if (program->parameterInfos)
   {
      const CellCgbParameterInfo *info = program->parameterInfos + map_index;

      uint16_t arrayFlags = ENDSWAP(info->flags) & CGPA_MASK;

      return (arrayFlags & CGPA_IS_ARRAY) != 0;
   }
   
   return false;
}
};
