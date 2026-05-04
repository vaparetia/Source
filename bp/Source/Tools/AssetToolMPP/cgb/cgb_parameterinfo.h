/* SCE CONFIDENTIAL
 * PLAYSTATION(R)3 Programmer Tool Runtime Library 200.002
 *                Copyright (C) 2007 Sony Computer Entertainment Inc.
 *                                               All Rights Reserved.
 */

#ifndef CGB_PARAMETERINFO_HEADER
#define CGB_PARAMETERINFO_HEADER

#include <Cg/cg.h>

#include "format/cgBinary.h"
#include "format/cgb_format.h"

#include <vector>

typedef struct CgNVParameter
{
	CGtype type;
	CGresource res;
	CGenum var;
	int rin;
	const char *name;
	const char *sem;
	CGenum dir;
	int no;
	bool is_referenced;
	bool is_shared;
} CgNVParameter;

void addParameterInfo(std::vector<CellCgbParameterInfo> &cg_parameter_info, CgNVParameter &nv_parameter);


#endif // #ifndef CGB_PARAMETERINFO_HEADER
