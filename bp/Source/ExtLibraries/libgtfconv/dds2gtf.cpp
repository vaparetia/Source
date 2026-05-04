/*  SCE CONFIDENTIAL
 *  PLAYSTATION(R)3 Programmer Tool Runtime Library 240.001
 *  Copyright (C) 2008 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "convert.h"
#include "gtfconv.h"

using namespace gtfconv;

namespace gtfconv{

#ifdef NDEBUG
// for release
const uint8_t HEADER_PAD_CHAR = 0;
const uint8_t IMAGE_PAD_CHAR = 0;
#else
// for debug
const uint8_t HEADER_PAD_CHAR = 'P';
const uint8_t IMAGE_PAD_CHAR = 0xEE;
#endif

bool dds2gtfConvFormat(CellUtilDDSHeader ddsh, uint8_t *pformat, uint32_t *premap, bool hasVaryingAlpha)
{
	uint8_t format = 0;
   uint32_t remap = hasVaryingAlpha ? utilStr2Remap("ARGB") : utilStr2Remap("1RGB");

	uint8_t a_bits = utilCountBit(ddsh.ddspf.abitMask);
	uint8_t r_bits = utilCountBit(ddsh.ddspf.rbitMask);
	uint8_t g_bits = utilCountBit(ddsh.ddspf.gbitMask);
	uint8_t b_bits = utilCountBit(ddsh.ddspf.bbitMask);

	if(ddsh.ddspf.flags & DDPF_RGB){
		if(ddsh.ddspf.rgbBitCount == 8){
			// 8bit
			if(a_bits == 8 || r_bits == 8 || g_bits == 8 || b_bits == 8){
				format = CELL_GCM_TEXTURE_B8;
				//utilStr2Remap("1BBB");
			}
		}else if(ddsh.ddspf.rgbBitCount == 16){
			// 16bit
			if(a_bits == 0){
				if(r_bits == 5 && g_bits == 6 && b_bits == 5){
					format = CELL_GCM_TEXTURE_R5G6B5;
					remap = utilStr2Remap("1RGB");
				}else if(r_bits == 6 && g_bits == 5 && b_bits == 5){
					format = CELL_GCM_TEXTURE_R6G5B5;
					remap = utilStr2Remap("1RGB");
				}else if(r_bits == 5 && g_bits == 5 && b_bits == 5){
						format = CELL_GCM_TEXTURE_D1R5G5B5;
						remap = utilStr2Remap("1RGB");
				}else if(r_bits == 4 && g_bits == 4 && b_bits == 4){
						format = CELL_GCM_TEXTURE_A4R4G4B4;
						remap = utilStr2Remap("1RGB");
				}
			}else if(a_bits == 1){
				if(ddsh.ddspf.abitMask == 0x00008000){
					format = CELL_GCM_TEXTURE_A1R5G5B5;
				}else if(ddsh.ddspf.abitMask == 0x00000001){
					format = CELL_GCM_TEXTURE_R5G5B5A1;
				}
			}else if(a_bits == 4){
				format = CELL_GCM_TEXTURE_A4R4G4B4;
			}else if((a_bits == 8 && r_bits == 8) || (g_bits == 8 && b_bits == 8)){
				format = CELL_GCM_TEXTURE_G8B8;
			}else if(a_bits == 16 || r_bits == 16 || g_bits == 16 || b_bits == 16){
				format = CELL_GCM_TEXTURE_X16;
			}
		}else if(ddsh.ddspf.rgbBitCount == 24){
			// 24bit
			format = CELL_GCM_TEXTURE_D8R8G8B8;
			remap = utilStr2Remap("1RGB");
		}else if(ddsh.ddspf.rgbBitCount == 32){
			// 32bit
			if(r_bits == 8){
				if(ddsh.ddspf.flags & DDPF_ALPHAPIXELS){
					format = CELL_GCM_TEXTURE_A8R8G8B8;
				}else{
					format = CELL_GCM_TEXTURE_D8R8G8B8;
					remap = utilStr2Remap("1RGB");
				}
			}else if((a_bits == 16 && r_bits == 16) || (g_bits == 16 && b_bits == 16) ||
				(a_bits == 16 && g_bits == 16) || (a_bits == 16 && b_bits == 16) || 
				(r_bits == 16 && g_bits == 16) || (r_bits == 16 && b_bits == 16)){
				format = CELL_GCM_TEXTURE_Y16_X16;
			}
		}

	}else if(ddsh.ddspf.flags & DDPF_FOURCC){
		if(ddsh.ddspf.fourCC == FOURCC_DXT1) format = CELL_GCM_TEXTURE_COMPRESSED_DXT1;
		if(ddsh.ddspf.fourCC == FOURCC_DXT2) format = CELL_GCM_TEXTURE_COMPRESSED_DXT23;
		if(ddsh.ddspf.fourCC == FOURCC_DXT3) format = CELL_GCM_TEXTURE_COMPRESSED_DXT23;
		if(ddsh.ddspf.fourCC == FOURCC_DXT4) format = CELL_GCM_TEXTURE_COMPRESSED_DXT45;
		if(ddsh.ddspf.fourCC == FOURCC_DXT5) format = CELL_GCM_TEXTURE_COMPRESSED_DXT45;

		if(ddsh.ddspf.fourCC == FOURCC_R16F)			format = CELL_GCM_TEXTURE_Y16_X16_FLOAT;
		if(ddsh.ddspf.fourCC == FOURCC_G16R16F)			format = CELL_GCM_TEXTURE_Y16_X16_FLOAT;
		if(ddsh.ddspf.fourCC == FOURCC_A16B16G16R16F)	format = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		if(ddsh.ddspf.fourCC == FOURCC_R32F)			format = CELL_GCM_TEXTURE_X32_FLOAT;
		if(ddsh.ddspf.fourCC == FOURCC_A32B32G32R32F)	format = CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;

		if(ddsh.ddspf.fourCC == FOURCC_R8G8_B8G8)	format = CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8, remap = utilStr2Remap("AGRB");
		if(ddsh.ddspf.fourCC == FOURCC_G8R8_G8B8)	format = CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8, remap = utilStr2Remap("AGRB");
		if(ddsh.ddspf.fourCC == FOURCC_YVYU)		format = CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8, remap = utilStr2Remap("AGRB");
		if(ddsh.ddspf.fourCC == FOURCC_YUY2)		format = CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8, remap = utilStr2Remap("AGRB");

		// unsupported
		if(ddsh.ddspf.fourCC == FOURCC_DDS)	 format = 0;
		if(ddsh.ddspf.fourCC == FOURCC_RXGB) format = 0;
		if(ddsh.ddspf.fourCC == FOURCC_ATI1) format = 0;
		if(ddsh.ddspf.fourCC == FOURCC_ATI2) format = 0;

	}else if(ddsh.ddspf.flags & DDSF_LUMINANCE){
		if(ddsh.ddspf.rgbBitCount == 16){
			if(r_bits == 16){
				format = CELL_GCM_TEXTURE_X16;
			}else if((a_bits == 8 && r_bits == 8) || (g_bits == 8 && b_bits == 8)){
				format = CELL_GCM_TEXTURE_G8B8;
			}
		}
	}else if(ddsh.ddspf.flags & DDSF_BUMPDUDV){
		if(ddsh.ddspf.rgbBitCount == 16){
			format = CELL_GCM_TEXTURE_Y16_X16;
		}else if(ddsh.ddspf.rgbBitCount == 32){
			format = CELL_GCM_TEXTURE_A8R8G8B8;
		}
	}

	if(format == 0){
		if(ddsh.ddspf.rgbBitCount == 8){
			format = CELL_GCM_TEXTURE_B8;
			//utilStr2Remap("1BBB");
		}else if(ddsh.ddspf.rgbBitCount == 16){
			format = CELL_GCM_TEXTURE_X16;
		}else if(ddsh.ddspf.rgbBitCount == 32){
			format = CELL_GCM_TEXTURE_A8R8G8B8;
		}else if(ddsh.ddspf.rgbBitCount == 64){
			format = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		}else if(ddsh.ddspf.rgbBitCount == 128){
			format = CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
		}else{
			return false;
		}
	}

	if(pformat){
		*pformat = format;
	}

	if(premap){
		*premap = remap;
	}

	return true;
}

bool dds2gtfConvHeader(CellGcmTexture *texture, CellUtilDDSHeader ddsh, bool bSwizzle, bool bUnNorm, bool hasVaryingAlpha)
{
	if(ddsh.magic != FOURCC_DDS) return false;
	if(ddsh.size != 124) return false;
	if(!(ddsh.flags & DDSD_CAPS)) return false;
	if(!(ddsh.flags & DDSD_PIXELFORMAT)) return false;
	if(!(ddsh.flags & DDSD_WIDTH)) return false;
	if(!(ddsh.flags & DDSD_HEIGHT)) return false;
	if((ddsh.flags & DDSD_PITCH) && (ddsh.flags & DDSD_LINEARSIZE)) return false;
	if(!(ddsh.caps1 & DDSCAPS_TEXTURE)) return false;

	/*
	if(ddsh.caps1 & DDSCAPS_MIPMAP){
		if(!(ddsh.caps1 & DDSCAPS_COMPLEX)) return false;
	}
	if(ddsh.caps1 & DDSCAPS2_VOLUME){
		if(!(ddsh.caps1 & DDSCAPS_COMPLEX)) return false;
	}
	if(ddsh.caps1 & DDSCAPS2_CUBEMAP){
		if(!(ddsh.caps1 & DDSCAPS_COMPLEX)) return false;
	}
	*/

	if(ddsh.ddspf.size != 32) return false;


	CellGcmTexture tex;
	dds2gtfConvFormat(ddsh, &tex.format, &tex.remap, hasVaryingAlpha);
	tex.width = static_cast<uint16_t>(ddsh.width);
	tex.height = static_cast<uint16_t>(ddsh.height);
	tex.mipmap = 1;
	tex.depth = 1;
	tex.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
	tex.cubemap = CELL_GCM_FALSE;
	tex.pitch = 0;
	tex.location = 0;
	tex.offset = 0;
	tex._padding = 0;

	if(ddsh.caps2 & DDSCAPS2_VOLUME){
		if(ddsh.flags & DDSD_DEPTH){
			tex.depth = static_cast<uint16_t>(ddsh.depth);
			tex.dimension = CELL_GCM_TEXTURE_DIMENSION_3;
		}
	}

	if(ddsh.flags & DDSD_MIPMAPCOUNT){
		uint8_t mipmap = utilGetMipmapSize(tex.width, tex.height, tex.depth);
		
		if(mipmap != ddsh.mipMapCount){
			return false;
		}

		tex.mipmap = mipmap;
	}

	if(ddsh.caps2 & DDSCAPS2_CUBEMAP){
		uint8_t cube = 0;
		if(ddsh.caps2 & DDSCAPS2_CUBEMAP_POSITIVEX) ++cube;
		if(ddsh.caps2 & DDSCAPS2_CUBEMAP_NEGATIVEX) ++cube;
		if(ddsh.caps2 & DDSCAPS2_CUBEMAP_POSITIVEY) ++cube;
		if(ddsh.caps2 & DDSCAPS2_CUBEMAP_NEGATIVEY) ++cube;
		if(ddsh.caps2 & DDSCAPS2_CUBEMAP_POSITIVEZ) ++cube;
		if(ddsh.caps2 & DDSCAPS2_CUBEMAP_NEGATIVEZ) ++cube;

		if(cube != 6){
			return false;
		}
		tex.cubemap = CELL_GCM_TRUE;
	}

	if(tex.dimension == CELL_GCM_TEXTURE_DIMENSION_3){
		if(tex.width > 512 || tex.height > 512 || tex.depth > 512){
			return false;
		}
	}else if(tex.dimension == CELL_GCM_TEXTURE_DIMENSION_2){
		if(tex.width > 4096 || tex.height > 4096){
			return false;
		}
	}

	// swizzle check
	uint8_t raw_format = gtfGetRawFormat(tex.format);
	bool is_dxt = gtfIsDxtn(raw_format);
	bool is_swizzle = gtfIsSwizzlable(tex);
	if(!is_dxt){
		is_swizzle = is_swizzle && bSwizzle;
	}

	if(is_swizzle){
		tex.format = static_cast<uint8_t>(tex.format & ~CELL_GCM_TEXTURE_LN);
		tex.pitch = 0;
	}else{
		tex.format = static_cast<uint8_t>(tex.format | CELL_GCM_TEXTURE_LN);
		tex.pitch = gtfGetPitch(tex.format, tex.width);
	}

	// normalize check
	if(bUnNorm){
		tex.format = static_cast<uint8_t>(tex.format | CELL_GCM_TEXTURE_UN);
	}

	*texture = tex;

	return true;
}

int32_t gtfconv_dds2gtf_image(GTF_CONV_T *gtfOutput, const GTF_BUFFER_T *ddsInput, uint32_t option)
{
	if(!ddsInput) return CELL_GTFCONV_ERROR;
	if(ddsInput->size < sizeof(CellUtilDDSHeader)) return CELL_GTFCONV_ERROR;

	bool bSwizzle = (option & CELL_GTFCONV_FLAG_SWIZZLIZE) == CELL_GTFCONV_FLAG_SWIZZLIZE;
	bool bUnNorm = (option & CELL_GTFCONV_FLAG_UNNORMALIZE) == CELL_GTFCONV_FLAG_UNNORMALIZE;
   bool bVaryingAlpha = (option & CELL_GTFCONV_FLAG_HAS_VARYING_ALPHA) == CELL_GTFCONV_FLAG_HAS_VARYING_ALPHA;

	CellUtilDDSHeader ddsh;
	memmove(&ddsh, ddsInput->addr, sizeof(CellUtilDDSHeader));

	if(ddsCheckSpec(ddsh) == false){
		return CELL_GTFCONV_ERROR;
	}

	PRINTF("    DDS Size: file=%d, image=%d\n", ddsInput->size, ddsInput->size - sizeof(CellUtilDDSHeader));

	uint8_t *in_image = ddsInput->addr + sizeof(CellUtilDDSHeader);
	
	CellGcmTexture texture;
	if(dds2gtfConvHeader(&texture, ddsh, bSwizzle, bUnNorm, bVaryingAlpha) == false){
		return CELL_GTFCONV_ERROR;
	}

	// gtf spec
	gtfCheckSpec(texture);

	// calc layout num
	uint8_t cube = 1;
	if(texture.cubemap == CELL_GCM_TRUE){
		cube = 6;
	}
	uint32_t layout_num = cube * texture.mipmap;

	layout_t *layout_array = new layout_t[layout_num];
	memset(layout_array, 0, sizeof(layout_t) * layout_num);

	convertLayOutBuffer(layout_array, texture, ddsh);

	uint32_t gtf_image_size = 0;
	bool is_swizzle = gtfIsSwizzle(texture.format);
	if(is_swizzle){
		gtf_image_size = layout_array[layout_num - 1].gtf_swizzle_offset + layout_array[layout_num - 1].gtf_swizzle_size;
	}else{
		gtf_image_size = layout_array[layout_num - 1].gtf_linear_offset + layout_array[layout_num - 1].gtf_linear_size;
	}

	uint32_t dds_image_size = layout_array[layout_num - 1].dds_offset + layout_array[layout_num - 1].dds_size;
	if(ddsInput->size - sizeof(CellUtilDDSHeader) < dds_image_size){
		if(layout_array) delete [] layout_array, layout_array = 0;
		return CELL_GTFCONV_ERROR;
	}

	if(!gtfOutput || gtfOutput->size < gtf_image_size)
	{
		if(layout_array) delete [] layout_array, layout_array = 0;
		return static_cast<int32_t>(gtf_image_size);
	}

	// init image
	memset(gtfOutput->addr, IMAGE_PAD_CHAR, gtf_image_size);
	
	// convert!!
	convertBufferByLayout(gtfOutput->addr, in_image, layout_array, layout_num, texture, true);
	if(layout_array) delete [] layout_array, layout_array = 0;

	gtfOutput->gcmt = texture;

	return CELL_GTFCONV_OK;
}

} // namespace gtfconv

int32_t gtfconv_dds2gtf(GTF_BUFFER_T *gtfOutput, const GTF_BUFFER_T *ddsInput, uint8_t inputCount, uint32_t option)
{
	const uint32_t GTF_TEXTURE_ALIGN = 128;
	bool bVerbose = (option & CELL_GTFCONV_FLAG_VERBOSE) == CELL_GTFCONV_FLAG_VERBOSE;
	bool bVerbPrep = (gtfOutput == 0) && bVerbose;
	bool bVerbPacked = (inputCount != 1) && bVerbose;

	if(bVerbPacked){
		bVerbPrep = false;
	}	

	uint32_t gtf_header_size = sizeof(CellGtfFileHeader) + sizeof(CellGtfTextureAttribute) * inputCount;
	gtf_header_size = utilGetAlign(gtf_header_size, GTF_TEXTURE_ALIGN);

	// init header
	uint8_t *gtf_header_buffer = new uint8_t[gtf_header_size];
	memset(gtf_header_buffer, HEADER_PAD_CHAR, gtf_header_size);

	CellGtfFileHeader *gtf_header = reinterpret_cast<CellGtfFileHeader*>(gtf_header_buffer);
	CellGtfTextureAttribute *gtf_attr = reinterpret_cast<CellGtfTextureAttribute*>(gtf_header_buffer + sizeof(CellGtfFileHeader));

	if(gtfOutput){
		memset(gtfOutput->addr, IMAGE_PAD_CHAR, gtfOutput->size);
	}

	for(uint8_t i = 0; i < inputCount; ++i)
	{
		SET_PRINTF_MODE(bVerbPrep);
		int32_t gtf_image_size = gtfconv_dds2gtf_image(0, &ddsInput[i], option);

		gtf_attr[i].id = i;
		if(i == 0){
            gtf_attr[i].offsetToTex = gtf_header_size;
		}else{
			gtf_attr[i].offsetToTex = gtf_attr[i - 1].offsetToTex + gtf_attr[i - 1].textureSize;
			gtf_attr[i].offsetToTex = utilGetAlign(gtf_attr[i].offsetToTex, GTF_TEXTURE_ALIGN);
		}
		if(gtf_image_size < 0){
			if(gtf_header_buffer){
				delete [] gtf_header_buffer, gtf_header_buffer = 0;
			}
			return CELL_GTFCONV_ERROR;
		}

		GTF_CONV_T gtf_conv;
		memset(&gtf_conv, 0, sizeof(GTF_CONV_T));
		
		gtf_conv.size = gtf_image_size;
		gtf_attr[i].textureSize = gtf_image_size;

		if(gtfOutput){
			gtf_conv.addr = gtfOutput->addr + gtf_attr[i].offsetToTex;

         gtfconv_dds2gtf_image(&gtf_conv, &ddsInput[i], option);

			gtf_attr[i].tex = gtf_conv.gcmt;

			SET_PRINTF_MODE(bVerbPacked);
			if(i == 0) PRINTF("  PackageInfo:\n");
			PRINTF("    [%02x] offset=%08x, size=%08x\n", i, gtf_attr[i].offsetToTex, gtf_image_size);
		}
	}

	uint32_t total_size = gtf_attr[inputCount - 1].offsetToTex + gtf_attr[inputCount - 1].textureSize;
	total_size = utilGetAlign(total_size, GTF_TEXTURE_ALIGN);

	if(!gtfOutput || gtfOutput->size < total_size)
	{
		if(gtf_header_buffer){
			delete [] gtf_header_buffer, gtf_header_buffer = 0;
		}
		return static_cast<int32_t>(total_size);
	}

	// file header
	{
		gtf_header->version = to_big_endian32(GTF_VERSION);
		gtf_header->size = to_big_endian32(total_size - gtf_header_size);
		gtf_header->numTexture = to_big_endian32(inputCount);

		for(uint8_t i = 0; i < inputCount; ++i){
			gtf_attr[i].id = to_big_endian32(gtf_attr[i].id);
			gtf_attr[i].offsetToTex = to_big_endian32(gtf_attr[i].offsetToTex);
			gtf_attr[i].textureSize = to_big_endian32(gtf_attr[i].textureSize);
			gtf_attr[i].tex.remap = to_big_endian32(gtf_attr[i].tex.remap);
			gtf_attr[i].tex.pitch = to_big_endian32(gtf_attr[i].tex.pitch);
			gtf_attr[i].tex.width = to_big_endian16(gtf_attr[i].tex.width);
			gtf_attr[i].tex.height = to_big_endian16(gtf_attr[i].tex.height);
			gtf_attr[i].tex.depth = to_big_endian16(gtf_attr[i].tex.depth);
		}

		// init header
		memset(gtfOutput->addr, HEADER_PAD_CHAR, gtf_header_size);
		memmove(gtfOutput->addr, gtf_header_buffer, gtf_header_size);
		if(gtf_header_buffer){
			delete [] gtf_header_buffer, gtf_header_buffer = 0;
		}
	}

	return CELL_GTFCONV_OK;
}

int32_t gtfconv_dds2gtf_get_size(const GTF_BUFFER_T *ddsInput, uint8_t inputCount, uint32_t option)
{
	return gtfconv_dds2gtf(0, ddsInput, inputCount, option);
}
