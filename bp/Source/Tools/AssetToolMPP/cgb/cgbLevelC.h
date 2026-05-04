/*   SCE CONFIDENTIAL                                       */
/*   PLAYSTATION(R)3 Programmer Tool Runtime Library 200.002 */
/*   Copyright (C) 2007 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#ifndef __CGB_LEVEL_C_H__
#define __CGB_LEVEL_C_H__

#if !defined(WIN32) && !defined(LINUX)
#include <cell/cgb.h>	
#else
#include <cgb.h>
#endif

//array
#define CGPA_MASK 0x10
#define CGPA_IS_ARRAY 0x10

#ifdef __cplusplus
extern "C" {
#endif
	
uint16_t cellCgbLevelCMapGetCgType(CellCgbProgram *program, const uint32_t map_index);
uint16_t cellCgbLevelCMapGetCgResource(CellCgbProgram *program, const uint32_t map_index);
uint16_t cellCgbLevelCMapGetVariability(CellCgbProgram *program, const uint32_t map_index);
uint16_t cellCgbLevelCMapGetDirection(CellCgbProgram *program, const uint32_t map_index);
uint16_t cellCgbLevelCMapGetValueCount(CellCgbProgram *program, const uint32_t map_index);
bool     cellCgbLevelCMapIsArray(CellCgbProgram *program, const uint32_t map_index);


#ifdef __cplusplus
}
#endif

#endif
