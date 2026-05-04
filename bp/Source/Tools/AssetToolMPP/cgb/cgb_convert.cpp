/* SCE CONFIDENTIAL
 * PLAYSTATION(R)3 Programmer Tool Runtime Library 200.002
 *                Copyright (C) 2007 Sony Computer Entertainment Inc.
 *                                               All Rights Reserved.
 */

/*
 * cgb_convert.cpp : Defines the entry point for the console application.
 */

#include "stdafx.h"
#include <Cg/cg.h>

//shb format
#include "format/cgb_format.h"

//nv format
#include "format/cgBinary.h"

#include "cgb_parameterinfo.h"

#define STL_NAMESPACE std::

//nv format reader
#include "nvbReader.h"

#include "cgnv2rtStr.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "cgb_report.h"

static int _vpCount = 0;
static int _fpCount = 0;
static int _totalUcodeSize = 0;

//for the level B, for FP, the constants needs a special table, the resource is offseted by FP_RESOURCE_START to indicate
//it's in the array, FP_RESOURCE_START have to be bigger than any resource and power of 2 for easy masking out later
#define FP_RESOURCE_START 1024

#define MAKEFOURCC_(a, b, c, d) (((unsigned int)(unsigned char)(a) << 24 ) | ((unsigned int)(unsigned char)(b) << 16) | \
	((unsigned int)(unsigned char)(c) << 8) | ((unsigned int)(unsigned char)(d) << 0 ))
#define CGB_FOURCC MAKEFOURCC_('C','G','B','\0')

static const nvbr_endianness elfEndianness  = NVB_MSB;
static nvbr_endianness hostEndianness = NVB_MSB;

#define ENDSWAP16(n) 		((((n) & 0xff00) >> 8)|(((n) & 0xff) << 8))
#define ENDSWAP32(n)	    ((((n) & 0xff) << 24)|(((n) & 0xff00) << 8)|(((n) & 0xff0000) >> 8)|(((n) & 0xff000000)>>24))
#define CNV2END16(n)		( (hostEndianness != elfEndianness) ?  ENDSWAP16(n) : (n) )
#define CNV2END32(n)		( (hostEndianness != elfEndianness) ?  ENDSWAP32(n) : (n) )

#define CGB_BIG_ENDIAN ( hostEndianness != 1 )

//#define CGB_DEV_REPORT(a) REPORT(a)
#define CGB_DEV_REPORT(a)

//#define CGB_DEV_TRACE

unsigned int inline static swap16(const unsigned int v)
{
	return (v>>16) | (v<<16);
}

template<class Type> static size_t array_size(STL_NAMESPACE vector<Type> &array)
{
	return (unsigned int)array.size()*sizeof(array[0]);
}

template<class Type> static void array_push(char* &parameterOffset, STL_NAMESPACE vector<Type> &array)
{
	size_t dataSize = array.size()*sizeof(array[0]);
	if (dataSize) {
		memcpy(parameterOffset,&array[0],dataSize);
		parameterOffset += dataSize;
	}
}

template<class Type> static void array_push_conv(char* &parameterOffset, STL_NAMESPACE vector<Type> &array)
{
	size_t dataSize = array.size()*sizeof(array[0]);
	if (dataSize) {
		for (int i=0;i<(int)array.size();i++)
		{
			Type item = array[i];
			array[i] = (sizeof(Type) == sizeof(unsigned short)) ? CNV2END16(item) : CNV2END32(item);
		}
		memcpy(parameterOffset,&array[0],dataSize);
		parameterOffset += dataSize;
	}
}

static void array_push_conv_float(char* &parameterOffset, STL_NAMESPACE vector<float> &array)
{
	size_t dataSize = array.size()*sizeof(array[0]);
	if (dataSize) {
		for (int i=0;i<(int)array.size();i++)
		{
			unsigned int item = *(unsigned int*)&array[i];
			item = CNV2END32(item);
			void* tmp = &item;
			array[i] = *(float*)tmp;
		}
		memcpy(parameterOffset,&array[0],dataSize);
		parameterOffset += dataSize;
	}
}

static void array_push_conv_map_entries(char* &parameterOffset, STL_NAMESPACE vector<CellCgbMapEntry> &array)
{
	size_t dataSize = array.size()*sizeof(array[0]);
	if (dataSize) {
		for (int i=0;i<(int)array.size();i++)
		{
			array[i].parent = CNV2END16(array[i].parent);
			array[i].resource = CNV2END16(array[i].resource);
			array[i].name_offset = CNV2END32(array[i].name_offset);
		}
		memcpy(parameterOffset,&array[0],dataSize);
		parameterOffset += dataSize;
	}
}

static void array_push_conv_parameter_infos(char* &parameterOffset, STL_NAMESPACE vector<CellCgbParameterInfo> &array)
{
	size_t dataSize = array.size()*sizeof(array[0]);
	if (dataSize) {
		for (int i=0;i<(int)array.size();i++)
		{
			array[i].cg_type = CNV2END16(array[i].cg_type);
			array[i].cg_resource = CNV2END16(array[i].cg_resource);
			array[i].flags = CNV2END16(array[i].flags);
			array[i].value_count = CNV2END16(array[i].value_count);
		}
		memcpy(parameterOffset,&array[0],dataSize);
		parameterOffset += dataSize;
	}
}

template<class Type> static void structure_push(char* &parameterOffset, Type &structure)
{
	size_t dataSize = sizeof(structure);
	if (dataSize) {
		memcpy(parameterOffset,&structure,dataSize);
		parameterOffset += dataSize;
	}
}

static void buffer_push(char* &parameterOffset, const void *data, size_t dataSize)
{
	if (dataSize) {
		memcpy(parameterOffset,data,dataSize);
		parameterOffset += dataSize;
	}
}

//debug function
static void dbgPrintMapEntry(std::vector<CellCgbMapEntry> &map_entries, std::vector<char> &string_table, std::vector<unsigned short> &fp_resources, bool bIsVertexProgram, int index, int offset)
{
	int offset2 = offset;
	while (offset2--)
	{
		REPORT(("  "));
	}
	if (map_entries[index].resource == 0xffff) //this is not a terminal node
	{
		REPORT(("%s\n", &string_table[0] + map_entries[index].name_offset));
		int count = (int)map_entries.size();
		for (int i=index+1;i<count;i++) //the children are always after the parent for now
		{
			if (map_entries[i].parent == index)
				dbgPrintMapEntry(map_entries,string_table,fp_resources,bIsVertexProgram,i,offset+1);
		}
	}
	else
	{
		if (map_entries[index].resource < FP_RESOURCE_START)
		{
			REPORT(("%s: %i\n", &string_table[0] + map_entries[index].name_offset, map_entries[index].resource));
		}
		else
		{
			ASSERT(bIsVertexProgram == false);
			int offset = map_entries[index].resource - FP_RESOURCE_START;
			REPORT(("%s: reg %i|off ", &string_table[0] + map_entries[index].name_offset, fp_resources[offset]));
			int embeddedConstantCount = fp_resources[offset+1];
			for (int i=0;i<embeddedConstantCount;i++)
			{
				if (i==embeddedConstantCount-1)
				{
					REPORT(("%i",fp_resources[offset+2+i]));
				}
				else
				{
					REPORT(("%i,",fp_resources[offset+2+i]));
				}
			}
			REPORT((" (idx %i)\n", offset));
		}
	}
}

static unsigned short getResource(CgNVParameter &parameter, bool bIsVertexProgram, STL_NAMESPACE vector<unsigned short> &fp_resources, STL_NAMESPACE vector<unsigned int> &ec)
{
	unsigned short resource = 0xffff;
	if (parameter.type >= CG_SAMPLER1D && parameter.type <= CG_SAMPLERCUBE)
	{
		//sampler case is shared between vertex and fragment
		ASSERT(parameter.var == CG_UNIFORM);
		ASSERT(parameter.res >= CG_TEXUNIT0 && parameter.res <= CG_TEXUNIT15);
		resource = parameter.res - CG_TEXUNIT0;
	}
	else if (bIsVertexProgram)
	{
		if (parameter.var == CG_VARYING) //we only have vertex input attrib here
		{
			resource = parameter.res - CG_ATTR0;
			ASSERT(parameter.res>=CG_ATTR0 && parameter.res<=CG_ATTR15);
		}
		else
			resource = parameter.rin;
	}
	else
	{
		ASSERT(parameter.var == CG_UNIFORM);
		resource = (unsigned short)fp_resources.size() + FP_RESOURCE_START;
		fp_resources.push_back((unsigned short)parameter.rin);
		fp_resources.push_back((unsigned short)ec.size());
		for (int j=0;j<(int)ec.size();j++)
		{
			fp_resources.push_back((unsigned short)ec[j]);
		}
	}
	return resource;
}

static unsigned short getSimpleResource(CgNVParameter &parameter)
{
	unsigned short resource = 0xffff;
	if (parameter.type >= CG_SAMPLER1D && parameter.type <= CG_SAMPLERCUBE)
	{
		//sampler case is shared between vertex and fragment
		ASSERT(parameter.var == CG_UNIFORM);
		ASSERT(parameter.res >= CG_TEXUNIT0 && parameter.res <= CG_TEXUNIT15);
		resource = parameter.res - CG_TEXUNIT0;
	}
	else if (parameter.var == CG_VARYING) //we only have vertex input attrib here
	{
		resource = parameter.res - CG_ATTR0;
		ASSERT(parameter.res>=CG_ATTR0 && parameter.res<=CG_ATTR15);
	}
	else
		resource = parameter.rin;
	return resource;
}

static bool isMatrix(CGtype type)
{
	return ((type >= CG_FLOAT1x1 && type <= CG_FLOAT4x4) ||
		(type >= CG_HALF1x1 && type <= CG_HALF4x4) ||
		(type >= CG_BOOL1x1 && type <= CG_BOOL4x4));
}

static size_t getStride(CGtype type)
{
	if ((type >= CG_FLOAT1x1 && type <= CG_FLOAT1x4) ||
		(type >= CG_HALF1x1 && type <= CG_HALF1x4) ||
		(type >= CG_BOOL1x1 && type <= CG_BOOL1x4))
		return 1;
	else if ((type >= CG_FLOAT2x1 && type <= CG_FLOAT2x4) ||
		(type >= CG_HALF2x1 && type <= CG_HALF2x4) ||
		(type >= CG_BOOL2x1 && type <= CG_BOOL2x4))
		return 2;
	else if ((type >= CG_FLOAT3x1 && type <= CG_FLOAT3x4) ||
		(type >= CG_HALF3x1 && type <= CG_HALF3x4) ||
		(type >= CG_BOOL3x1 && type <= CG_BOOL3x4))
		return 3;
	else if ((type >= CG_FLOAT4x1 && type <= CG_FLOAT4x4) ||
		(type >= CG_HALF4x1 && type <= CG_HALF4x4) ||
		(type >= CG_BOOL4x1 && type <= CG_BOOL4x4))
		return 4;
	else
		return 0;
}


static void checkArray(nvb_struct *pNvbStruct,int parameterIndex, bool bIsVertexProgram, bool *isArrayPacked, size_t *itemCount, size_t *stride, char* currentArrayName, size_t size)
{
	//retrieve the array name
	CgNVParameter parameter;
	STL_NAMESPACE vector<float> dv;
	STL_NAMESPACE vector<unsigned int> ec;
	dv.clear(); ec.clear();
	nvbReaderGetParameter(parameterIndex, pNvbStruct, &parameter, dv, ec);
	strncpy(currentArrayName,parameter.name,size);
	currentArrayName[size-1] = '\0';
	char *nameEnd = strrchr(currentArrayName,'[');
	if (!nameEnd)
	{
		*itemCount = 0;
		*stride = 0;
		currentArrayName[0] = '\0';
		*isArrayPacked = false;
		return;
	}
	*nameEnd = '\0';
	unsigned short currentResource = 0xffff;
	if (bIsVertexProgram && parameter.is_referenced)
		currentResource = getSimpleResource(parameter);

	*stride = getStride(parameter.type); //0 if it's not a matrix
	size_t registerStride = *stride == 0 ? 1 : *stride;

	char currentMatrixName[1024];
	currentMatrixName[0] = '\0';
	bool withinMatrix = false;

	//explore the parameter list to determine the size of the array and check if it's
	*isArrayPacked = true;
	int parameterEnd = (int)pNvbStruct->numberOfParams;
	int index = parameterIndex;

	while (index < parameterEnd)
	{
		dv.clear(); ec.clear();
		nvbReaderGetParameter(index, pNvbStruct, &parameter, dv, ec);

		if (isMatrix(parameter.type))
		{
			//only the matrix root has the matrix type
			withinMatrix = true;
			strncpy(currentMatrixName,parameter.name,sizeof(currentMatrixName));
			currentMatrixName[sizeof(currentMatrixName)-1] = '\0';
		}
		else if (withinMatrix)
		{
			//the name of the root will be entirely repeated in the name of the following parameter which exists
			if (!strncmp(parameter.name,currentMatrixName,strlen(currentMatrixName)))
			{
				//we are still within the same matrix, skip
				index++;
				continue;
			}
			else
				withinMatrix = false;
		}

		char tmpParameterName[1024];
		strncpy(tmpParameterName,parameter.name,sizeof(tmpParameterName));
		tmpParameterName[sizeof(tmpParameterName)-1] = '\0';

		nameEnd = strrchr(tmpParameterName,'[');
		if (!nameEnd)
			break;
		*nameEnd = '\0';

		if (strcmp(currentArrayName,tmpParameterName) != 0)
			break;

		//check if the array is packed here
		//if it's a vertex program the resource have to be consecutive
		//if it's an array of constant in a fragment program if it's referenced it's enough
		if (*isArrayPacked )
		{
			if ( !parameter.is_referenced)
				*isArrayPacked = false;
			else
			{
				if (bIsVertexProgram || (parameter.type >= CG_SAMPLER1D && parameter.type <= CG_SAMPLERCUBE))
				{
					unsigned short resource;
					resource = getSimpleResource(parameter);
					if (resource != currentResource)
						*isArrayPacked = false;
					else
						currentResource += (unsigned int) registerStride;
				} // else for fragment program constant, we don't need to check the contiguous resources, since the resources
				//are list of offsets, we will consider the array packed if all the items are referenced
			}
		}
		index++;
	}

	//TODO: tmp for fragment, I will disable the packing of the parameter until the packing of the patching location is done
	if (!bIsVertexProgram)
		*isArrayPacked = false;

	*itemCount = (index - parameterIndex)/(*stride+1);
}

static void alignTo16(unsigned int &value, unsigned int &padding)
{
	unsigned int res = value&(16-1);
	if (res)
		padding = 16-res;
	else
		padding = 0;
	value += padding;
}

static int convert(const void *source, size_t size, void **destination)
{
	unsigned char contentFlag = CELL_CGB_CONSTANT_TABLE | CELL_CGB_LOOKUP_TABLE | CELL_CGB_PARAMETER_INFO;

	//read the NV file
	nvb_struct nvbStruct;
	nvbr_error err =  nvbReaderLoadFromString((const char *)source, size, &nvbStruct);
	if(err != NVB_READER_OK)
	{
		REPORT(("error reading nv format file\n"));
		return -1;
	}
	CGB_DEV_REPORT(("Profile %d, Revision %d, Size %d, Ucode %d\n", nvbStruct.profile, nvbStruct.revision, nvbStruct.size, nvbStruct.ucodeSize ));

	//endianess
	nvbr_endianness elfEndianness = NVB_MSB;
	bool doSwap = !(nvbStruct.endianness == elfEndianness);
	{	// Initialize hostEndianness
		const int ii = 1;
		const char* cp = (const char*) &ii;
		hostEndianness = ( 1 == cp[0] ) ? NVB_LSB : NVB_MSB;
	}

	//check the profile
	CGprofile NVProfile = nvbStruct.profile;

	//hack to counter removal of TypeC during beta
	if (NVProfile == (CGprofile)7005 )
		NVProfile = CG_PROFILE_SCE_VP_RSX;
	if (NVProfile == (CGprofile)7006 )
		NVProfile = CG_PROFILE_SCE_FP_RSX;

	bool bIsVertexProgram = false;
	if (NVProfile == CG_PROFILE_SCE_VP_TYPEB || NVProfile == CG_PROFILE_SCE_VP_RSX)
	{
		bIsVertexProgram = true;
		_vpCount++;
	}
	else if (NVProfile == CG_PROFILE_SCE_FP_TYPEB || NVProfile == CG_PROFILE_SCE_FP_RSX)
	{
		bIsVertexProgram = false;
		_fpCount++;
	}
	else
	{
		REPORT(("error: unknown shader profile\n"));
		return -1;
	}

	//Fill the shader header structure and save it into the shadertab

	CellCgbVertexConfigurationInternal vertexConf;
	CellCgbFragmentConfigurationInternal fragmentConf;
	memset(&vertexConf,0,sizeof(vertexConf));
	memset(&fragmentConf,0,sizeof(fragmentConf));

	if (bIsVertexProgram)
	{
		const CgBinaryVertexProgram *nvVertex = nvbStruct.vertexProgram;

		if (doSwap) //here we go directly from nv to elf endiannes without going to the platform endianness
		{
			vertexConf.attributeInputMask = ENDSWAP16((uint16_t)nvVertex->attributeInputMask);
			vertexConf.registerCount = (uint8_t)(ENDSWAP32(nvVertex->registerCount));
			vertexConf.attributeOutputMask = ENDSWAP32(nvVertex->attributeOutputMask);
			vertexConf.userClipMask = ENDSWAP32(nvVertex->userClipMask);
		}
		else
		{
			if (CGB_BIG_ENDIAN)
			{
				vertexConf.attributeInputMask = (uint16_t)nvVertex->attributeInputMask;
				vertexConf.registerCount = (uint8_t)nvVertex->registerCount;
			}
			else
			{
				vertexConf.attributeInputMask = ENDSWAP16((uint16_t)(ENDSWAP32(nvVertex->attributeInputMask)));
				vertexConf.registerCount = (uint8_t)(ENDSWAP32(nvVertex->registerCount));
				vertexConf.attributeOutputMask = nvVertex->attributeOutputMask;
				vertexConf.userClipMask = nvVertex->userClipMask;
			}
		}
	}
	else
	{
		//TODO: here we are losing the partialType, is that needed ?
		const CgBinaryFragmentProgram *nvFragment = nvbStruct.fragmentProgram;
		if (doSwap)
		{
			fragmentConf.registerCount = nvFragment->registerCount;
			fragmentConf.attributeInputMask = ENDSWAP32(nvFragment->attributeInputMask);
			fragmentConf.texCoordsInputMask = ENDSWAP16(nvFragment->texCoordsInputMask);
			fragmentConf.texCoords2D = ENDSWAP16(nvFragment->texCoords2D);
			fragmentConf.texCoordsCentroid = ENDSWAP16(nvFragment->texCoordsCentroid);
		}
		else
		{
			fragmentConf.registerCount = nvFragment->registerCount;
			fragmentConf.attributeInputMask = nvFragment->attributeInputMask;
			fragmentConf.texCoordsInputMask = nvFragment->texCoordsInputMask;
			fragmentConf.texCoords2D = nvFragment->texCoords2D;
			fragmentConf.texCoordsCentroid = nvFragment->texCoordsCentroid;
		}
		unsigned int flags = (1<<15) | (1<<10) |
			(nvFragment->depthReplace ? 0xE : 0x0) |
			(nvFragment->outputFromH0 ? 0x00 : 0x40) |
			(nvFragment->pixelKill ? 0x80 : 0x00);
		fragmentConf.fragmentControl = CNV2END32(flags);
	}

	//shader specific info ( the shader header is in the shader tab )

	//ucode
	unsigned int *tmp = (unsigned int *) nvbStruct.ucode;
	const char *ucode;
	unsigned int *buffer = NULL;
	if (doSwap)
	{
		int size = (int)nvbStruct.ucodeSize/sizeof(unsigned int);
		buffer = new unsigned int[size];
		for (int i=0;i<size;i++)
		{
			unsigned int val = ENDSWAP32(tmp[i]);
			if (!bIsVertexProgram)
				val = swap16(val);
			buffer[i] = val;
		}
		ucode = (const char*)buffer;
	}
	else
	{
		ucode = (const char*)tmp;
		// !!!xxx this is to workaround what appears to be a linux platform specific bug
		// that manifests as a memory overwrite in properly allocated memory during a STL_NAMESPACE vector resize
		int size = (int)nvbStruct.ucodeSize/sizeof(unsigned int);
		buffer = new unsigned int[size];
		for (int i=0;i<size;i++)
		{
			buffer[i] = tmp[i];
		}
		ucode = (const char*)buffer;
		// end workaround
	}

	int ucodeSize = nvbStruct.ucodeSize;

	//ucode, ucodeSize;
	_totalUcodeSize += ucodeSize;

	//the nv format doesn't store the structure of the Cg parameters.
	//the layout has to be recovered from the parameters names.

	CgNVParameter parameter;
	STL_NAMESPACE vector<float> dv;
	STL_NAMESPACE vector<unsigned int> ec;

	//level A: constants for vertex programs
	std::vector<float> constant_values; //by group of 4
	std::vector<unsigned short> constant_resources;

	//level B: name->resource lookup table
	std::vector<char> string_table;
	std::vector<CellCgbMapEntry> map_entries;
	std::vector<unsigned short> fp_resources;

	//level C: Cg parameter info
	std::vector<CellCgbParameterInfo> cg_parameter_info;

	int parameterCount = (int)nvbStruct.numberOfParams;
	char currentArrayName[1024];
	currentArrayName[0] = '\0';
	bool isArrayPacked = false;
	char currentMatrixName[1024];
	currentMatrixName[0] = '\0';
	bool withinMatrix = false;

	//level A: constants for vertex programs
	for (int i = 0; i < parameterCount; i++)
	{
		dv.clear(); ec.clear();
		nvbReaderGetParameter(i, &nvbStruct, &parameter, dv, ec);
		CGB_DEV_REPORT(("parameter: %s\n",parameter.name));

		if (bIsVertexProgram)
		{
			if (dv.size() && parameter.rin!=-1)
			{
				//register
				constant_resources.push_back(parameter.rin);
				//value
				for (int k=0;k<(int)dv.size();k++)
					constant_values.push_back(dv[k]);
				for (int k=(int)dv.size();k<4;k++)
					constant_values.push_back(.0f);
			}
		}
	}

	//level B: name->resource lookup table
	for (int i = 0; i < parameterCount; i++)
	{
		dv.clear(); ec.clear();
		nvbReaderGetParameter(i, &nvbStruct, &parameter, dv, ec);
		CGB_DEV_REPORT(("parameter: %s\n",parameter.name));

		//This is how we handle matrices, we only iterate on the "matrix" element in the fpo
		bool bIsMatrix = isMatrix(parameter.type);
		if (bIsMatrix)
		{
			//only the matrix root has the matrix type
			withinMatrix = true;
			strncpy(currentMatrixName,parameter.name,sizeof(currentMatrixName));
			currentMatrixName[sizeof(currentMatrixName)-1] = '\0';
		}
		else if (withinMatrix)
		{
			//the name of the root will be entirely repeated in the name of the following parameter which exists
			if (!strncmp(parameter.name,currentMatrixName,strlen(currentMatrixName))&& parameter.name[strlen(currentMatrixName)] == '[')
			{
				if (bIsVertexProgram)
					continue; //we are still within the same matrix, skip
				//not for fp, for fp every row of the matrix have to have its entry if it's referenced
			}
			else
				withinMatrix = false;
		}

		//array dimensions
		size_t itemCount = 0;
		size_t stride = 0;

		//NOTE: I keep the unreferenced shared because, which could be a problem,
		//since developers are using shared as a way to assign registers
		//fp: only matrix content (bIsMatrix == false && ec.size() --> because matrix are marked as referenced by the compilo even if not used)
		//vp: only matrix header (bIsMatrix == true),
		if ( ((!withinMatrix) || ( (bIsVertexProgram || ( !bIsMatrix && ec.size())) && ( !bIsVertexProgram || bIsMatrix ))) &&
			 (parameter.is_referenced || parameter.is_shared)&& ( parameter.var == CG_UNIFORM || (bIsVertexProgram && parameter.var == CG_VARYING && parameter.dir == CG_IN)))
		{
			//are we already in a pack array ?
			//get array name
			if (isArrayPacked)
			{
				char tmpArrayName[sizeof(currentArrayName)];
				strncpy(tmpArrayName,parameter.name,sizeof(currentArrayName));
				tmpArrayName[sizeof(currentArrayName)-1] = '\0';
				char *nameEnd = strrchr(tmpArrayName,'[');
				if (nameEnd)
				{
					*nameEnd = '\0';
					//check for the case where we have jumped into another array and we didn't had an item 0
					//which was referenced
					if (!strcmp(tmpArrayName,currentArrayName) != 0)
					{
						//the array is packed, there is no need to do anything for sub items in the array
						continue;
					}
				}
				isArrayPacked = false;
			}

			char buffer[1024];
			strncpy(buffer,parameter.name,sizeof(buffer));
			buffer[sizeof(buffer)-1] = '\0';
			char *pBuffer = buffer;
			unsigned short parent = 0xffff;
			while (pBuffer)
			{
				bool bHasArraySubscript = false;
				char *leafNameEnd = strpbrk(pBuffer, ".[]");
				if (leafNameEnd)
				{
					if ((*leafNameEnd) == ']')
					{
						//we have an array subscript
						bHasArraySubscript = true;
						char *closingBrace = leafNameEnd;
						//advance leafNameEnd
						if (bIsVertexProgram)
						{
							//for VP we want to cut all matrices here, since they are always packed
							leafNameEnd = strpbrk(leafNameEnd+1, ".]");
						}
						else
						{
							//for FP that's the reverse than VP, we always want to unfold matrices into vectors
							leafNameEnd = strpbrk(leafNameEnd+1, ".[");
						}
						if (leafNameEnd)
							*leafNameEnd = 0;
						//remove the closing brace, pBuffer now contains the index in the matrix / array
						*closingBrace = '\0';
					}
					else
						*leafNameEnd = '\0';
				}

				//treat the array/matrix case
				if (bHasArraySubscript)
				{
					//we have an array or a matrix, we need to determine several things:
					// - the real name of the map entry: this will be the array/matrix index, so we need to remove the closing ']'
					// - do we need to add this entry:
					//   - if the array/matrix is packed, we only save the array entry not the cells
					//   - if the array is not packed we will save the individual items

					//early rejection case, if the array is not terminal then it is not packed
					if (!leafNameEnd)
					{
						//we are sure the array is a terminal node now, it might be packed
						int index = atoi(pBuffer);
						if (index == 0)
						{
							//we need to determine if we are in a packed array / matrix or not
							//if we are in a packed array we will jump several parameter
							checkArray(&nvbStruct,i,bIsVertexProgram,&isArrayPacked,&itemCount,&stride,currentArrayName,sizeof(currentArrayName));
							if (isArrayPacked)
							{
                        if (contentFlag & CELL_CGB_PARAMETER_INFO)
                        {
                           cg_parameter_info[parent].flags |= CGPA_IS_ARRAY;

                           //Since this is an array type (and we can deduce how many actual values we need to set from the parameter type) it makes more sense
                           //for "value_count" to contain the number of array elements!
                           cg_parameter_info[parent].value_count = itemCount;
                        }

								//I need to update the parent which is actually a terminal node now
								map_entries[parent].resource = getResource(parameter,bIsVertexProgram,fp_resources,ec);
                        

								//I need to continue and skip (stride+1) *itemCount parameters
								//I move onto the last param of the packed array
								i+= ((int)stride+1)*(int)itemCount - 1;
								pBuffer = NULL;
								continue;
							}
						}
						// else we know we are in an unpacked array already, the check has already
					}
				}

				CellCgbMapEntry map_entry;
				map_entry.name_offset = stringTableAddUnique(string_table,pBuffer);
				map_entry.parent = parent;
				//we set the resource only on a terminal node of a token
				if (leafNameEnd)
					map_entry.resource = 0xffff;
				else
				{
					CGB_DEV_REPORT(("res: %i rin: %i\n",parameter.res,parameter.rin));
					map_entry.resource = getResource(parameter,bIsVertexProgram,fp_resources,ec);
				}

				bool entryExist = false;

				//check if we already have that entry in the map if we are not at the end of the token yet
				if (leafNameEnd)
				{
					int count = (int)map_entries.size();
					for (int j=0;j<count;j++)
					{
						if ( (map_entries[j].name_offset == map_entry.name_offset) &&
							 (map_entries[j].parent == map_entry.parent) &&
							 (map_entries[j].resource == map_entry.resource))
						{
							entryExist = true;
							//mark that entry as parent
							parent = j;
							break;
						}
					}
				}

				//if the entry doesn't already exist add it
				if (!entryExist)
				{
					parent = (unsigned short)map_entries.size();
					map_entries.push_back(map_entry);

					// for every entry that was pushed into the map, add its the parameter info
					if (contentFlag & CELL_CGB_PARAMETER_INFO)
					    {
						// add level C info here
						addParameterInfo(cg_parameter_info, parameter);
					    }

				}

				//keep parsing the token or end the loop if we are at the end
				if (leafNameEnd)
					pBuffer = leafNameEnd+1;
				else
					pBuffer = NULL;
			}
		}

	}

	//TODO: remove this debug information

#ifdef DEV_TRACE
	//print the level B map
	REPORT(("\n"));
	REPORT(("lookup table:\n"));
	int entryCount = (int)map_entries.size();
	for (int i=0;i<entryCount;i++)
	{
		if (map_entries[i].parent == 0xffff)
		{
			//print the entry and its children
			dbgPrintMapEntry(map_entries, string_table, fp_resources, bIsVertexProgram, i, 0);
		}
	}

#endif

	//determine the size of the data
	//TODO: just dump to get an idea of the size at the moment
	//TODO: the shader headers are not here at the moment

	unsigned int headerAndUcodeSize = (unsigned int)sizeof(CellCgbHeader);
	headerAndUcodeSize += bIsVertexProgram ? sizeof(vertexConf) : sizeof(fragmentConf);
	//compute ucode offset and realign
	unsigned int align_ucode = 0;
	alignTo16(headerAndUcodeSize,align_ucode);
	headerAndUcodeSize += ucodeSize;

	CellCgbLevelAHeader levelAHeader;
	memset(&levelAHeader,0,sizeof(CellCgbLevelAHeader));
	CellCgbLevelBHeader levelBHeader;
	memset(&levelBHeader,0,sizeof(CellCgbLevelBHeader));
	CellCgbLevelCHeader levelCHeader;
	memset(&levelCHeader,0,sizeof(CellCgbLevelCHeader));

	size_t dataSize = 0;

	levelAHeader.block_size = (unsigned short)sizeof(CellCgbLevelAHeader);
	levelAHeader.block_size += (unsigned short)array_size(constant_resources);

	//alignment point
	unsigned int align_constants = 0;
	unsigned int tmp_size = (unsigned int)headerAndUcodeSize + (unsigned int)levelAHeader.block_size;
	alignTo16(tmp_size,align_constants);
	levelAHeader.block_size += align_constants;

	levelAHeader.block_size += (unsigned short)array_size(constant_values);
	levelAHeader.constant_count = CNV2END16((unsigned short)constant_resources.size());

	dataSize += levelAHeader.block_size;
	levelAHeader.block_size = CNV2END16(levelAHeader.block_size);
	CGB_DEV_REPORT(("level A: %i bytes\n",dataSize));

	levelBHeader.block_size = (unsigned short)sizeof(CellCgbLevelBHeader);
	levelBHeader.block_size += (unsigned short)array_size(map_entries);
	levelBHeader.block_size += (unsigned short)array_size(fp_resources);
	levelBHeader.block_size += (unsigned short)array_size(string_table);
	levelBHeader.entry_count = CNV2END16((unsigned short)map_entries.size());
	levelBHeader.fp_offset_count = CNV2END16((unsigned short)fp_resources.size());

	dataSize += levelBHeader.block_size;
	levelBHeader.block_size = CNV2END16(levelBHeader.block_size);
	CGB_DEV_REPORT(("level A + B: %i bytes\n",dataSize));

	if (contentFlag & CELL_CGB_PARAMETER_INFO)
	{
		levelCHeader.block_size = (unsigned short)sizeof(CellCgbLevelCHeader);
		levelCHeader.block_size += (unsigned short)array_size(cg_parameter_info);
		levelCHeader.info_count = CNV2END16((unsigned short)cg_parameter_info.size());

		dataSize += levelCHeader.block_size;
		levelCHeader.block_size = CNV2END16(levelCHeader.block_size);
		CGB_DEV_REPORT(("level A + B + C: %i bytes\n",dataSize));
	}


	dataSize += headerAndUcodeSize;
	CGB_DEV_REPORT(("ucode size  %i bytes\n",dataSize));
	CGB_DEV_REPORT(("total size %i bytes\n",dataSize));

	char *parameterOffset = new char[dataSize];
	*destination = parameterOffset;

	//copy the data

	CellCgbHeader header;
	header.fourcc = CNV2END32(CGB_FOURCC); //don't convert fourcc code
	header.format_version = 0;
	header.compiler_version = 0;
	header.profile = bIsVertexProgram ? 0 : 1;
	header.ucode_size = CNV2END16((unsigned short)ucodeSize);
	header.content = contentFlag;

	structure_push(parameterOffset,header);
	if (bIsVertexProgram)
		structure_push(parameterOffset,vertexConf);
	else
		structure_push(parameterOffset,fragmentConf);

	//align ucode on 16 bytes
	memset(parameterOffset,0,align_ucode);
	parameterOffset += align_ucode;
	ASSERT(((unsigned int)(parameterOffset - (char*)*destination)&(16-1)) == 0);
	buffer_push(parameterOffset,ucode,ucodeSize);

	structure_push(parameterOffset,levelAHeader);
	array_push_conv(parameterOffset,constant_resources);
	//align the constant block on 16
	memset(parameterOffset,0,align_constants);
	parameterOffset += align_constants;
	ASSERT(levelAHeader.constant_count == 0 || ((unsigned int)(parameterOffset - (char*)*destination)&(16-1)) == 0);
	if (hostEndianness != elfEndianness)
		array_push_conv_float(parameterOffset,constant_values);
	else
		array_push(parameterOffset,constant_values);

	structure_push(parameterOffset,levelBHeader);
	array_push_conv_map_entries(parameterOffset,map_entries);
	array_push_conv(parameterOffset,fp_resources);
	array_push(parameterOffset,string_table);

	if (contentFlag & CELL_CGB_PARAMETER_INFO)
	{
		structure_push(parameterOffset,levelCHeader);
		array_push_conv_parameter_infos(parameterOffset,cg_parameter_info);
	}

	return (int)dataSize;
}


static void release(void *archive)
{
	delete[] (char*)archive;
}

int cgbGenerateFromNVBuffer(const void *source, size_t size, const char *destination)
{
	void *archive = NULL;
	int res = convert(source,size,&archive);
	if (res>=0)
	{
		FILE *fp = fopen(destination,"wb");
		if (!fp)
		{
			REPORT(("cannot open output file: %s\n",destination));
			res = -1;
		}
		else
		{
			fwrite(archive,res,1,fp);
			fclose(fp);
			res = 0;
		}
	}
	if (archive)
	release(archive);
	return res;
}

int cgbGenerateFromNVBufferEncode(const void *source, size_t size, size_t *outputSize, void **intermediateRepresentation)
{
	void *archive = NULL;
	int res = convert(source,size,&archive);
	if (res>=0)
	{
		*outputSize = res;
		*intermediateRepresentation = archive;
		res = 0;
	}
	else
		*intermediateRepresentation = NULL;
	return res;
}

int cgbGenerateFromNVBufferFinalize(void *intermediateRepresentation, void *outputBuffer, size_t outputSize)
{
	void *archive = intermediateRepresentation;
	if (archive)
	{
		if (outputBuffer)
		{
			memcpy(outputBuffer,archive,outputSize);//trust the size passed as parameter here
		}
		release(archive);
	}
	return 0;
}

#ifdef CGB_TARGET_STANDALONE_CONVERTER
static int convertFile(const char *source, const char *destination)
{
	FILE *fp = fopen(source,"rb");
	if (fp==NULL)
	{
		REPORT(("error cannot open file %s\n",source));
		return -1;
	}
	fseek(fp,0,SEEK_END);
	size_t size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	char *buf = new char[size];
	fread(buf,1,size,fp);
	fclose(fp);
	int res = cgbGenerateFromNVBuffer(buf,size,destination);
	delete[] buf;
	return res;
}

int main(int argc, char* argv[])
{
	char *input = NULL;
	char *output = NULL;

	//bool bHelpNeeded = false;
	//bool bVersion = false;

	//get input and ouput file names
	int i;
	for (i=1;i<argc;i++)
	{
		if (argv[i][0] == '-')
		{
			//ignore for now
		}
		else if (input == NULL)
			input = argv[i];
		else if (output == NULL)
			output = argv[i];
	}

	if (input == NULL)
	{
		REPORT(("error no input\n"));
		return -1;
	}

	//check output
	char *destination;
	char buffer[1024];
	if (output == NULL)
	{
		char *tmp0 = strrchr(input,'/');
		char *tmp1 = strrchr(input,'\\');
		if (tmp1 > tmp0)
			tmp0 = tmp1;
		if (tmp0)
		{
			//create the destination file: append out.ext to the path of the file
			size_t length = tmp0 - input + 1;
			strncpy(buffer,input,length);
			strcpy(buffer + length,"out");
			char *ext = strrchr(input,'.');
			if (ext)
				strcat(buffer,ext);
			destination = buffer;
		}
		else
			destination = "out";
	}
	else
		destination = output;

	int res = convertFile(input,destination);
	return res;
}
#endif
