/* SCE CONFIDENTIAL
 * PLAYSTATION(R)3 Programmer Tool Runtime Library 200.002
 *                Copyright (C) 2007 Sony Computer Entertainment Inc.
 *                                               All Rights Reserved.
 */

#include "stdafx.h"
#include "nvbReader.h"

#include <stdio.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#define nvbrENDIANSWAP32(n)	    ( (hostEndianness != endianness) ?  ((((n) & 0xff) << 24)|(((n) & 0xff00) << 8)|(((n) & 0xff0000) >> 8)|(((n) & 0xff000000)>>24)) : (n) )

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
nvbr_error nvbReaderLoad(const char* filename, nvb_struct* pNvbStruct)
{
	if(!filename || !pNvbStruct)	return NVB_READER_BAD_PARAMS;
	return NVB_READER_OK;

	FILE *fp = fopen(filename, "rb");
	if(!fp)		return NVB_READER_IO_ERROR;
	fseek(fp,0,SEEK_END);
	size_t size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	char *buf = new char[size];
	fread(buf,1,size,fp);
	fclose(fp);

	nvbr_error err = nvbReaderLoadFromString((const char*)buf, size, pNvbStruct);
	delete [] buf;
	return err;
}


nvbr_error nvbReaderLoadFromString(const char* source, size_t length, nvb_struct* pNvbStruct)
{
	if(!source || !length || !pNvbStruct)	return NVB_READER_BAD_PARAMS;
	if(length < sizeof(CgBinaryProgram))	return NVB_READER_WRONG_FORMAT;

	nvbr_endianness hostEndianness, endianness;
	{
		const int ii = 1;
		const char* cp = (const char*) &ii;
		hostEndianness = ( 1 == cp[0] ) ? NVB_LSB : NVB_MSB;
		endianness = hostEndianness;
	}

	CgBinaryProgram *pHeader = (CgBinaryProgram *)source;
	if(pHeader->binaryFormatRevision != CG_BINARY_FORMAT_REVISION)
	{
		endianness = (hostEndianness == NVB_LSB) ? NVB_MSB : NVB_LSB;
		if(nvbrENDIANSWAP32(pHeader->binaryFormatRevision) != CG_BINARY_FORMAT_REVISION)	return NVB_READER_WRONG_FORMAT;
	}

	pNvbStruct->endianness       = endianness;
	pNvbStruct->profile          = (CGprofile) nvbrENDIANSWAP32((unsigned int) pHeader->profile);
	pNvbStruct->revision         = nvbrENDIANSWAP32(pHeader->binaryFormatRevision);
	pNvbStruct->size             = nvbrENDIANSWAP32(pHeader->totalSize);
	pNvbStruct->numberOfParams   = nvbrENDIANSWAP32(pHeader->parameterCount);
	pNvbStruct->ucodeSize        = nvbrENDIANSWAP32(pHeader->ucodeSize);
	pNvbStruct->ucode            = (pNvbStruct->ucodeSize != 0) ? source + nvbrENDIANSWAP32(pHeader->ucode) : NULL;
	pNvbStruct->fragmentProgram  = (CgBinaryFragmentProgram *) (source + nvbrENDIANSWAP32(pHeader->program));
	pNvbStruct->vertexProgram    = (CgBinaryVertexProgram *) (source + nvbrENDIANSWAP32(pHeader->program));

	pNvbStruct->params.clear();
	pNvbStruct->defaultValues.clear();
	pNvbStruct->embeddedConstants.clear();

	CgBinaryParameter *paramsArray = (CgBinaryParameter *) (source + nvbrENDIANSWAP32(pHeader->parameterArray));
	for(unsigned int i = 0; i < pNvbStruct->numberOfParams;  ++i)
	{
		CgBinaryParameter *param = paramsArray + i;
		nvb_paramInfo paramInfo;
		CgNVParameter *pNvParam = &(paramInfo.nvParam);
		pNvParam->type          = (CGtype)     nvbrENDIANSWAP32((unsigned int) param->type);
		pNvParam->res           = (CGresource) nvbrENDIANSWAP32((unsigned int) param->res);
		pNvParam->var           = (CGenum)     nvbrENDIANSWAP32((unsigned int) param->var);
		pNvParam->rin           =              nvbrENDIANSWAP32(param->resIndex);
		pNvParam->name          = (const char*)(param->name != 0) ? (source + nvbrENDIANSWAP32(param->name)) : NULL;
		pNvParam->sem           = (const char*)(param->semantic != 0) ? (source + nvbrENDIANSWAP32(param->semantic)) : NULL;
		pNvParam->dir           = (CGenum)     nvbrENDIANSWAP32((unsigned int) param->direction);
		pNvParam->no            = (int)        nvbrENDIANSWAP32((unsigned int) param->paramno);
		pNvParam->is_referenced =              (nvbrENDIANSWAP32((unsigned int) param->isReferenced) != 0);
		pNvParam->is_shared     =              (nvbrENDIANSWAP32((unsigned int) param->isShared) != 0);

		CgBinaryFloatOffset defaultValueOffset = (nvbrENDIANSWAP32(param->defaultValue));
		if(!defaultValueOffset)		paramInfo.defaultValIndex = -1;
		else
		{
			paramInfo.defaultValIndex = (int) pNvbStruct->defaultValues.size();
			float *vp = (float *)(char *)(source + defaultValueOffset);		// legal since C99 states that (char*) types are related to every ptr type
			for(unsigned int ii = 0; ii < 4; ++ii)
			{	// C99 specifically allows (and recommends) "casting" of unrelated types through unions
				union FREP32
				{
					float        f32;
					unsigned int u32;
				};
				union FREP32 res;
				res.f32 = vp[ii];
				res.u32 = nvbrENDIANSWAP32(res.u32);
				pNvbStruct->defaultValues.push_back(res.f32);
			}

		}

		CgBinaryEmbeddedConstantOffset embeddedConstantsOffset = (nvbrENDIANSWAP32(param->embeddedConst));
		if(!embeddedConstantsOffset)
		{
			paramInfo.embeddedConstIndex = -1;
			paramInfo.embeddedConstCount = 0;
		}
		else
		{
			CgBinaryEmbeddedConstant *ec = (CgBinaryEmbeddedConstant *)(source + embeddedConstantsOffset);
			paramInfo.embeddedConstIndex = (int) pNvbStruct->embeddedConstants.size();
			paramInfo.embeddedConstCount = nvbrENDIANSWAP32(ec->ucodeCount);
			for(unsigned int ii = 0; ii < paramInfo.embeddedConstCount; ++ii)
			{
				pNvbStruct->embeddedConstants.push_back(nvbrENDIANSWAP32(ec->ucodeOffset[ii]));
			}
		}

		pNvbStruct->params.push_back(paramInfo);
	}

/*
	{
		// test to make the compiler yell about strict aliasing
		float feufeu;
		float *pfff = &feufeu;
		int *toto = (int *)pfff;
	}
*/
	return NVB_READER_OK;
}


nvbr_error nvbReaderGetParameter(unsigned int index, nvb_struct * pNvbStruct,
								 CgNVParameter *pParam, std::vector<float>& defaultValue, std::vector<unsigned int>& embeddedConstants)
{
	if(!pNvbStruct || !pParam)	return NVB_READER_BAD_PARAMS;
	if( (index >= pNvbStruct->numberOfParams) || (index >= pNvbStruct->params.size()) )	return NVB_READER_BAD_PARAMS;
	nvb_paramInfo param = pNvbStruct->params[index];
	if( ((param.defaultValIndex >= 0) && ((unsigned int) param.defaultValIndex > pNvbStruct->defaultValues.size()-4)) ||
		((param.embeddedConstCount > 0) && (param.embeddedConstIndex + param.embeddedConstCount > pNvbStruct->embeddedConstants.size())) )	return NVB_READER_BAD_PARAMS;

	memcpy(pParam, &param, sizeof(CgNVParameter));

	if(param.defaultValIndex >= 0)
	{
		for(unsigned int i = 0; i < 4; ++i)
		{
			defaultValue.push_back(pNvbStruct->defaultValues[param.defaultValIndex + i]);
		}
	}

	for(unsigned int j = 0; j < param.embeddedConstCount; ++j)
	{
		embeddedConstants.push_back(pNvbStruct->embeddedConstants[param.embeddedConstIndex + j]);
	}

	return NVB_READER_OK;
}

