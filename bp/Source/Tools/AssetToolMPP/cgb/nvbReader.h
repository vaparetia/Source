/* SCE CONFIDENTIAL
 * PLAYSTATION(R)3 Programmer Tool Runtime Library 200.002
 *                Copyright (C) 2007 Sony Computer Entertainment Inc.
 *                                               All Rights Reserved.
 */

#ifndef NVBREADER_HEADER
#define NVBREADER_HEADER

#include <Cg/cg.h>
#include "cgb_parameterinfo.h"

#include <vector>

enum nvbr_endianness
{
	NVB_LSB = 1,
	NVB_MSB = 2,
};

enum nvbr_error
{
	NVB_READER_OK,
	NVB_READER_BAD_PARAMS,
	NVB_READER_WRONG_FORMAT,
	NVB_READER_IO_ERROR,
};

typedef struct
{
	CgNVParameter nvParam;
	int           defaultValIndex;          // -1 if the default value's offset was 0
	int           embeddedConstIndex;		// -1 if the embedded constants' offet was 0
	unsigned int  embeddedConstCount;
} nvb_paramInfo;

typedef struct
{
	nvbr_endianness                endianness;
	CGprofile                      profile;
	unsigned int                   revision;
	unsigned int                   size;
	unsigned int                   numberOfParams;
	unsigned int                   ucodeSize;
	const char*                    ucode;
	const CgBinaryFragmentProgram* fragmentProgram;
	const CgBinaryVertexProgram*   vertexProgram;
	std::vector<nvb_paramInfo>     params;
	std::vector<float>             defaultValues;
	std::vector<unsigned int>      embeddedConstants;
} nvb_struct;


nvbr_error nvbReaderLoad(const char* filename, nvb_struct* pNvbStruct);
nvbr_error nvbReaderLoadFromString(const char* source, size_t length, nvb_struct* pNvbStruct);
nvbr_error nvbReaderGetParameter(unsigned int index, nvb_struct * pNvbStruct,
								 CgNVParameter *pParam, std::vector<float>& defaultValue, std::vector<unsigned int>& embeddedConstants);


#endif // #ifndef NVBREADER_HEADER
