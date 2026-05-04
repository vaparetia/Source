/* SCE CONFIDENTIAL
 * PLAYSTATION(R)3 Programmer Tool Runtime Library 200.002
 *                Copyright (C) 2007 Sony Computer Entertainment Inc.
 *                                               All Rights Reserved.
 */

#include "stdafx.h"
#include "cgb_parameterinfo.h"
#include "cgb_report.h"

//#include "cgnv2rtStr.h"

unsigned short getFlags(CgNVParameter &parameter)
{
	CGenum var = (CGenum)parameter.var;
	CGenum dir = (CGenum)parameter.dir;

	//the following is error prone, use 2 enum array to do the matching instead
	//variability
	unsigned short flags = 0;
	if (var == CG_VARYING)
		flags |= CGPV_VARYING;
	else if (var == CG_UNIFORM)
		flags |= CGPV_UNIFORM;
	else if (var == CG_CONSTANT)
		flags |= CGPV_CONSTANT;
	else if (var == CG_MIXED)
		flags |= CGPV_MIXED;

	//direction
	if (dir == CG_IN)
		flags |= CGPD_IN;
	else if (dir == CG_OUT)
		flags |= CGPD_OUT;
	else if (dir == CG_INOUT)
		flags |= CGPD_INOUT;

	return flags;
}


#define CG_DATATYPE_MACRO(name, compiler_name, enum_name, base_enum, nrows, ncols,classname) \
	nrows ,
static int _typesRowCount[] =
    {
#include <Cg/cg_datatypes.h>
    };

#undef CG_DATATYPE_MACRO
#define CG_DATATYPE_MACRO(name, compiler_name, enum_name, base_enum, nrows, ncols,classname) \
	ncols ,
static int _typesColCount[] =
    {
#include <Cg/cg_datatypes.h>
    };

unsigned int getTypeRowCount( CGtype parameterType )
{
    int typeIndex = parameterType - 1 - CG_TYPE_START_ENUM;
    return _typesRowCount[typeIndex];
}

unsigned int getTypeColCount( CGtype parameterType )
{
    int typeIndex = parameterType - 1 - CG_TYPE_START_ENUM;
    return _typesColCount[typeIndex];
}


void addParameterInfo(std::vector<CellCgbParameterInfo> &cg_parameter_info, CgNVParameter &nv_parameter)
{
    CellCgbParameterInfo info;

    info.cg_type = (unsigned short)nv_parameter.type;
    info.cg_resource = (unsigned short)nv_parameter.res;

    info.flags = getFlags(nv_parameter);

    int rows = getTypeRowCount(nv_parameter.type);
    int cols = getTypeColCount(nv_parameter.type);

    // rows equal zero for all non matrix types, cols equal zero for sampler types
    info.value_count = ((rows == 0) ? 1 : rows) * ((cols == 0) ? 1 : cols);

    cg_parameter_info.push_back(info);
}
