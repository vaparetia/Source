/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEGEOM_DECOMPRESS_H
#define EDGEGEOM_DECOMPRESS_H

/* from edgegeom_masks.cpp: */
/* all shuffle masks used in this code MUST be local variables. No external links allowed! */

// We expect many of these variable declarations to be unused and
// compiled out of existence; defining them this way will suppress the
// compiler's spurious warning messages.
#if !defined(EDGE_VAR)
#define EDGE_VAR __attribute__(__unused__)
#endif

//
// Processing macros
//

#define EDGE_GEOM_DECOMPRESS_STORE(attrId)										\
	do {																\
		if (addMissingComponents##attrId)										\
		{																\
			attr0##attrId = (vec_char16)spu_sel((vec_uint4)attr0##attrId, kMissingComponents, missingComponentMask##attrId); \
			attr1##attrId = (vec_char16)spu_sel((vec_uint4)attr1##attrId, kMissingComponents, missingComponentMask##attrId); \
		}																\
		si_stqx(attr0##attrId, pAttr0##attrId, uniformTableOffsets[loopCycleCount##attrId]); \
		si_stqx(attr1##attrId, pAttr1##attrId, uniformTableOffsets[loopCycleCount##attrId]); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_EXTRACT(attrId)										\
	do {																\
		inAttr0##attrId = useFastExtract0##attrId ? inQuads[inQuad0##attrId##0] : \
			spu_shuffle(inQuads[inQuad0##attrId##0], inQuads[inQuad0##attrId##1], shuf0##attrId); \
		inAttr1##attrId = useFastExtract1##attrId ? inQuads[inQuad1##attrId##0] : \
			spu_shuffle(inQuads[inQuad1##attrId##0], inQuads[inQuad1##attrId##1], shuf1##attrId); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_F32(attrId)                        \
	do{                                                     \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_F16(attrId)                        \
	do{                                                     \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_U8(attrId)                         \
	do{                                                     \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_I16(attrId)                        \
	do{                                                     \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_U8N(attrId)                         \
	do{                                                     \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_I16N(attrId)                        \
	do{                                                     \
	} while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(attrId)                  \
	do{                                                     \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_FIXED_POINT(attrId)                \
	do{														   \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(attrId)					 \
	do{															 \
		EDGE_GEOM_DECOMPRESS_C_UNIT_VECTOR(attrId);                 \
		EDGE_GEOM_DECOMPRESS_STORE(attrId);							 \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_F32(attrId)				\
	do {													\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);						\
		attr0##attrId = inAttr0##attrId;					\
		attr1##attrId = inAttr1##attrId;					\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_F16(attrId)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);									\
		const vec_int4 fDataHole0 = spu_andc(spu_rlmaska((vec_int4)inAttr0##attrId, -3), expHoleMask); \
		const vec_int4 fDataHole1 = spu_andc(spu_rlmaska((vec_int4)inAttr1##attrId, -3), expHoleMask); \
		const vec_int4 expBias0 = spu_and((vec_int4)spu_cmpgt(spu_andc((vec_uint4)inAttr0##attrId, signMask), 0), expRebias); \
		const vec_int4 expBias1 = spu_and((vec_int4)spu_cmpgt(spu_andc((vec_uint4)inAttr1##attrId, signMask), 0), expRebias); \
		attr0##attrId = (vec_char16)spu_add(fDataHole0, expBias0);		\
		attr1##attrId = (vec_char16)spu_add(fDataHole1, expBias1); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_U8(attrId)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);									\
		attr0##attrId = (vec_char16)spu_convtf((vec_uint4)inAttr0##attrId, 16); \
		attr1##attrId = (vec_char16)spu_convtf((vec_uint4)inAttr1##attrId, 16); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_I16(attrId)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);									\
		attr0##attrId = (vec_char16)spu_convtf((vec_int4)inAttr0##attrId, 16); \
		attr1##attrId = (vec_char16)spu_convtf((vec_int4)inAttr1##attrId, 16); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_U8N(attrId)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);									\
		attr0##attrId = (vec_char16)spu_madd(spu_convtf((vec_uint4)inAttr0##attrId, 16), u8nScale, u8nBias); \
		attr1##attrId = (vec_char16)spu_madd(spu_convtf((vec_uint4)inAttr1##attrId, 16), u8nScale, u8nBias); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_I16N(attrId)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);									\
		attr0##attrId = (vec_char16)spu_madd(spu_convtf((vec_int4)inAttr0##attrId, 16), i16nScale, i16nBias); \
		attr1##attrId = (vec_char16)spu_madd(spu_convtf((vec_int4)inAttr1##attrId, 16), i16nScale, i16nBias); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_X11Y11Z10N(attrId)					\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);									\
		const vec_float4 fData0 = spu_convtf(spu_and(spu_sl((vec_int4)inAttr0##attrId, shiftValues), bitMask), 31); \
		const vec_float4 fData1 = spu_convtf(spu_and(spu_sl((vec_int4)inAttr1##attrId, shiftValues), bitMask), 31); \
		attr0##attrId = (vec_char16)spu_madd(fData0, x11y11z10Scale, x11y11z10Bias); \
		attr1##attrId = (vec_char16)spu_madd(fData1, x11y11z10Scale, x11y11z10Bias); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_A_FIXED_POINT(attrId)	\
	do{												\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);\
		const vec_int4 word1_1 = (vec_int4)spu_rlmaskqwbytebc(spu_rlmaskqw(inAttr0##attrId, rshift1##attrId), rshift1##attrId);	\
		const vec_int4 word1_2 = (vec_int4)spu_rlmaskqwbytebc(spu_rlmaskqw(inAttr0##attrId, rshift2##attrId), rshift2##attrId);	\
		const vec_int4 word1_3 = (vec_int4)spu_rlmaskqwbytebc(spu_rlmaskqw(inAttr0##attrId, rshift3##attrId), rshift3##attrId);	\
		const vec_int4 word1_4 = (vec_int4)spu_rlmaskqwbytebc(spu_rlmaskqw(inAttr0##attrId, rshift3##attrId), rshift4##attrId);	\
		const vec_int4 word2_1 = (vec_int4)spu_rlmaskqwbytebc(spu_rlmaskqw(inAttr1##attrId, rshift1##attrId), rshift1##attrId);	\
		const vec_int4 word2_2 = (vec_int4)spu_rlmaskqwbytebc(spu_rlmaskqw(inAttr1##attrId, rshift2##attrId), rshift2##attrId);	\
		const vec_int4 word2_3 = (vec_int4)spu_rlmaskqwbytebc(spu_rlmaskqw(inAttr1##attrId, rshift3##attrId), rshift3##attrId);	\
		const vec_int4 word2_4 = (vec_int4)spu_rlmaskqwbytebc(spu_rlmaskqw(inAttr1##attrId, rshift3##attrId), rshift4##attrId);	\
		fixed1##attrId = spu_sel( spu_sel(word1_1, word1_2, mask_0F0F),	\
			spu_sel(word1_3, word1_4, mask_0F0F),		\
			mask_00FF);													\
		fixed2##attrId = spu_sel( spu_sel(word2_1, word2_2, mask_0F0F),	\
			spu_sel(word2_3, word2_4, mask_0F0F),		\
			mask_00FF);													\
	} while(0);

#define EDGE_GEOM_DECOMPRESS_B_FIXED_POINT(attrId)	\
	do{												\
		const vec_int4 finalFixed1 = spu_add(spu_and(fixed1##attrId, garbageMask##attrId), fixedPointOffset##attrId);\
		const vec_int4 finalFixed2 = spu_add(spu_and(fixed2##attrId, garbageMask##attrId), fixedPointOffset##attrId);\
		const vec_float4 fixedInt1 = spu_convtf(spu_rlmaska(finalFixed1, fixedIntShift##attrId), 0);\
		const vec_float4 fixedInt2 = spu_convtf(spu_rlmaska(finalFixed2, fixedIntShift##attrId), 0);\
		const vec_float4 fixedMantissa1 = spu_convtf((vec_uint4)spu_sl(finalFixed1, fixedMantissaShift##attrId), 32); \
		const vec_float4 fixedMantissa2 = spu_convtf((vec_uint4)spu_sl(finalFixed2, fixedMantissaShift##attrId), 32); \
		attr0##attrId = (vec_char16)spu_add( fixedInt1, fixedMantissa1);\
		attr1##attrId = (vec_char16)spu_add( fixedInt2, fixedMantissa2);\
	} while(0);

#define EDGE_GEOM_DECOMPRESS_A_UNIT_VECTOR(attrId)					\
	do {                                                                \
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrId);                                    \
		iData0##attrId = spu_sel(formatMask, spu_rlmask((vec_uint4)inAttr0##attrId,rotValues), formatSelect); \
		iData1##attrId = spu_sel(formatMask, spu_rlmask((vec_uint4)inAttr1##attrId,rotValues), formatSelect); \
		fData0##attrId = spu_madd((vec_float4)spu_shuffle(iData0##attrId, iData1##attrId, s_BCbc),fSwUnitVectorScale, fSwUnitVectorBias); \
		fData0Sq##attrId = spu_mul(fData0##attrId, fData0##attrId);                             \
	} while(0)                                                          


#define EDGE_GEOM_DECOMPRESS_B_UNIT_VECTOR(attrId)					\
	do {                                                                \
		fToSqrt##attrId = spu_sub(spu_splats(1.0f),spu_add(fData0Sq##attrId, spu_slqwbyte(fData0Sq##attrId, 4))); \
		complexMask0##attrId = (vec_uchar16)si_lqx(pMaskTable,(vec_char16)iData0##attrId); \
		complexMask1##attrId = (vec_uchar16)si_lqx(pMaskTable,(vec_char16)iData1##attrId); \
		fSqrt##attrId = spu_mul(fToSqrt##attrId,spu_rsqrte(fToSqrt##attrId));					\
		fData0Holder##attrId = fData0##attrId;                                          \
	} while (0)                                                         

#define EDGE_GEOM_DECOMPRESS_C_UNIT_VECTOR(attrId)					\
	do {                                                                \
		s_output0##attrId = (vec_uchar16)spu_sel(outputMask, (vec_uint4)complexMask0##attrId, outputSelect); \
		s_output1##attrId = (vec_uchar16)spu_add(spu_sel(outputMask, (vec_uint4)complexMask1##attrId, outputSelect), shuffleAdjust); \
		attr0##attrId = (vec_char16)spu_shuffle(spu_sel((vec_uint4)fSqrt##attrId, (vec_uint4)complexMask0##attrId, swSelect), (vec_uint4)fData0Holder##attrId, s_output0##attrId);	\
		attr1##attrId = (vec_char16)spu_shuffle(spu_sel((vec_uint4)fSqrt##attrId, (vec_uint4)complexMask1##attrId, swSelect), (vec_uint4)fData0Holder##attrId, s_output1##attrId);	\
	} while (0)

#define EDGE_GEOM_DECOMPRESS_LOOP_START()					\
	do {} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_END()											\
	do {																\
		uniformTableOffsets[4] = uniformTableOffsets[3];				\
		uniformTableOffsets[3] = (vec_char16)spu_add((vec_int4)uniformTableOffsets[3], 0x20); \
		EDGE_GEOM_DECOMPRESS_LOAD_COMMON();										\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_F32(attrId)			\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrId);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_F32(attrId);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_F16(attrId)			\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrId);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_F16(attrId);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_U8(attrId)				\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrId);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_U8(attrId);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_I16(attrId)			\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrId);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_I16(attrId);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_U8N(attrId)				\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrId);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_U8N(attrId);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_I16N(attrId)			\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrId);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_I16N(attrId);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(attrId)			\
	do {											\
		EDGE_GEOM_DECOMPRESS_STORE(attrId);				\
		EDGE_GEOM_DECOMPRESS_PROCESS_X11Y11Z10N(attrId);	\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_FIXED_POINT(attrId)        \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_B_FIXED_POINT(attrId);\
		EDGE_GEOM_DECOMPRESS_STORE(attrId);            \
		EDGE_GEOM_DECOMPRESS_A_FIXED_POINT(attrId); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(attrId)          \
	do {                                            \
		EDGE_GEOM_DECOMPRESS_C_UNIT_VECTOR(attrId);         \
		EDGE_GEOM_DECOMPRESS_STORE(attrId);               \
		EDGE_GEOM_DECOMPRESS_B_UNIT_VECTOR(attrId);       \
		EDGE_GEOM_DECOMPRESS_A_UNIT_VECTOR(attrId);     \
	} while (0) 

#define EDGE_GEOM_DECOMPRESS_PRIME_F32(attrId)               \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_PROCESS_F32(attrId);         \
	} while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_F16(attrId)               \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_PROCESS_F16(attrId);         \
	} while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_U8(attrId)               \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_PROCESS_U8(attrId);         \
	} while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_I16(attrId)               \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_PROCESS_I16(attrId);         \
	} while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_U8N(attrId)               \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_PROCESS_U8N(attrId);         \
	} while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_I16N(attrId)               \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_PROCESS_I16N(attrId);         \
	} while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_X11Y11Z10N(attrId)               \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_PROCESS_X11Y11Z10N(attrId);         \
	} while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_FIXED_POINT(attrId)       \
	do{                                             \
		EDGE_GEOM_DECOMPRESS_A_FIXED_POINT(attrId);  \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PRIME_UNIT_VECTOR(attrId)          \
	do {                                            \
		EDGE_GEOM_DECOMPRESS_A_UNIT_VECTOR(attrId);     \
	} while (0) 

#define EDGE_GEOM_DECOMPRESS_LOAD_COMMON()								\
	do {																\
		if (kStrideTimes2 % 16 == 0)									\
		{																\
			switch(kNumQuads)											\
			{															\
			case 16:													\
				inQuads[15] = si_lqd(si_from_ptr(pInput), 0xF0);		\
				/* fall through */										\
			case 15:													\
				inQuads[14] = si_lqd(si_from_ptr(pInput), 0xE0);		\
				/* fall through */										\
			case 14:													\
				inQuads[13] = si_lqd(si_from_ptr(pInput), 0xD0);		\
				/* fall through */										\
			case 13:													\
				inQuads[12] = si_lqd(si_from_ptr(pInput), 0xC0);		\
				/* fall through */										\
			case 12:													\
				inQuads[11] = si_lqd(si_from_ptr(pInput), 0xB0);		\
				/* fall through */										\
			case 11:													\
				inQuads[10] = si_lqd(si_from_ptr(pInput), 0xA0);		\
				/* fall through */										\
			case 10:													\
				inQuads[9] = si_lqd(si_from_ptr(pInput), 0x90);			\
				/* fall through */										\
			case 9:														\
				inQuads[8] = si_lqd(si_from_ptr(pInput), 0x80);			\
				/* fall through */										\
			case 8:														\
				inQuads[7] = si_lqd(si_from_ptr(pInput), 0x70);			\
				/* fall through */										\
			case 7:														\
				inQuads[6] = si_lqd(si_from_ptr(pInput), 0x60);			\
				/* fall through */										\
			case 6:														\
				inQuads[5] = si_lqd(si_from_ptr(pInput), 0x50);			\
				/* fall through */										\
			case 5:														\
				inQuads[4] = si_lqd(si_from_ptr(pInput), 0x40);			\
				/* fall through */										\
			case 4:														\
				inQuads[3] = si_lqd(si_from_ptr(pInput), 0x30);			\
				/* fall through */										\
			case 3:														\
				inQuads[2] = si_lqd(si_from_ptr(pInput), 0x20);			\
				/* fall through */										\
			case 2:														\
				inQuads[1] = si_lqd(si_from_ptr(pInput), 0x10);			\
				/* fall through */										\
			case 1:														\
				inQuads[0] = si_lqd(si_from_ptr(pInput), 0x00);			\
				/* fall through */										\
			}															\
		}																\
		else															\
		{																\
			vec_char16 inTemp[kNumQuads+1];								\
			const vec_char16 quadOff = spu_and((vec_char16)si_from_ptr(pInput), 0x00000000F); \
			const vec_uchar16 inShuf = (vec_uchar16)spu_add((vec_int4)s_ABCD, (vec_int4)spu_shuffle(quadOff, quadOff, s_DDDDDDDDDDDDDDDD));	\
			switch(kNumQuads)											\
			{															\
			case 16:													\
				inTemp[ 16] = si_lqd(si_from_ptr(pInput), 0x100);		\
				inTemp[ 15] = si_lqd(si_from_ptr(pInput), 0xF0);		\
				inQuads[15] = spu_shuffle(inTemp[15], inTemp[16], inShuf); \
				/* fall through */										\
			case 15:													\
				inTemp[ 15] = si_lqd(si_from_ptr(pInput), 0xF0);		\
				inTemp[ 14] = si_lqd(si_from_ptr(pInput), 0xE0);		\
				inQuads[14] = spu_shuffle(inTemp[14], inTemp[15], inShuf); \
				/* fall through */										\
			case 14:													\
				inTemp[ 14] = si_lqd(si_from_ptr(pInput), 0xE0);		\
				inTemp[ 13] = si_lqd(si_from_ptr(pInput), 0xD0);		\
				inQuads[13] = spu_shuffle(inTemp[13], inTemp[14], inShuf); \
				/* fall through */										\
			case 13:													\
				inTemp[ 13] = si_lqd(si_from_ptr(pInput), 0xD0);		\
				inTemp[ 12] = si_lqd(si_from_ptr(pInput), 0xC0);		\
				inQuads[12] = spu_shuffle(inTemp[12], inTemp[13], inShuf); \
				/* fall through */										\
			case 12:													\
				inTemp[ 12] = si_lqd(si_from_ptr(pInput), 0xC0);		\
				inTemp[ 11] = si_lqd(si_from_ptr(pInput), 0xB0);		\
				inQuads[11] = spu_shuffle(inTemp[11], inTemp[12], inShuf); \
				/* fall through */										\
			case 11:													\
				inTemp[ 11] = si_lqd(si_from_ptr(pInput), 0xB0);		\
				inTemp[ 10] = si_lqd(si_from_ptr(pInput), 0xA0);		\
				inQuads[10] = spu_shuffle(inTemp[10], inTemp[11], inShuf); \
				/* fall through */										\
			case 10:													\
				inTemp[10] = si_lqd(si_from_ptr(pInput), 0xA0);			\
				inTemp[ 9] = si_lqd(si_from_ptr(pInput), 0x90);			\
				inQuads[9] = spu_shuffle(inTemp[9], inTemp[10], inShuf); \
				/* fall through */										\
			case 9:														\
				inTemp[ 9] = si_lqd(si_from_ptr(pInput), 0x90);			\
				inTemp[ 8] = si_lqd(si_from_ptr(pInput), 0x80);			\
				inQuads[8] = spu_shuffle(inTemp[8], inTemp[9], inShuf);	\
				/* fall through */										\
			case 8:														\
				inTemp[ 8] = si_lqd(si_from_ptr(pInput), 0x80);			\
				inTemp[ 7] = si_lqd(si_from_ptr(pInput), 0x70);			\
				inQuads[7] = spu_shuffle(inTemp[7], inTemp[8], inShuf);	\
				/* fall through */										\
			case 7:														\
				inTemp[ 7] = si_lqd(si_from_ptr(pInput), 0x70);			\
				inTemp[ 6] = si_lqd(si_from_ptr(pInput), 0x60);			\
				inQuads[6] = spu_shuffle(inTemp[6], inTemp[7], inShuf);	\
				/* fall through */										\
			case 6:														\
				inTemp[ 6] = si_lqd(si_from_ptr(pInput), 0x60);			\
				inTemp[ 5] = si_lqd(si_from_ptr(pInput), 0x50);			\
				inQuads[5] = spu_shuffle(inTemp[5], inTemp[6], inShuf);	\
				/* fall through */										\
			case 5:														\
				inTemp[ 5] = si_lqd(si_from_ptr(pInput), 0x50);			\
				inTemp[ 4] = si_lqd(si_from_ptr(pInput), 0x40);			\
				inQuads[4] = spu_shuffle(inTemp[4], inTemp[5], inShuf);	\
				/* fall through */										\
			case 4:														\
				inTemp[ 4] = si_lqd(si_from_ptr(pInput), 0x40);			\
				inTemp[ 3] = si_lqd(si_from_ptr(pInput), 0x30);			\
				inQuads[3] = spu_shuffle(inTemp[3], inTemp[4], inShuf);	\
				/* fall through */										\
			case 3:														\
				inTemp[ 3] = si_lqd(si_from_ptr(pInput), 0x30);			\
				inTemp[ 2] = si_lqd(si_from_ptr(pInput), 0x20);			\
				inQuads[2] = spu_shuffle(inTemp[2], inTemp[3], inShuf);	\
				/* fall through */										\
			case 2:														\
				inTemp[ 2] = si_lqd(si_from_ptr(pInput), 0x20);			\
				inTemp[ 1] = si_lqd(si_from_ptr(pInput), 0x10);			\
				inQuads[1] = spu_shuffle(inTemp[1], inTemp[2], inShuf);	\
				/* fall through */										\
			case 1:														\
				inTemp[ 1] = si_lqd(si_from_ptr(pInput), 0x10);			\
				inTemp[ 0] = si_lqd(si_from_ptr(pInput), 0x00);			\
				inQuads[0] = spu_shuffle(inTemp[0], inTemp[1], inShuf);	\
				break;													\
			default:													\
				break;													\
			}															\
		}																\
		pInput += kStrideTimes2;										\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(attrType, attrId, attrOffset, attrComponentCount) \
	EDGE_VAR int uniformIndex##attrId;									\
	const qword unusedTableBits##attrId = si_gbb(si_ceqb(unusedAttrId, \
			ctx->uniformTableToAttributeIdMapping));					\
	uniformIndex##attrId = si_to_int(si_ai(si_clz(unusedTableBits##attrId), -16)); \
	ctx->uniformTableToAttributeIdMapping = si_shufb(si_from_int(attrId), \
		ctx->uniformTableToAttributeIdMapping,							\
		si_cbd(si_from_int(uniformIndex##attrId), 0));					\
																		\
	switch(attrId)														\
	{																	\
	case EDGE_GEOM_ATTRIBUTE_ID_POSITION:								\
		*ppPositionTable = (void*)pUniforms[uniformIndex##attrId];		\
		break;															\
	case EDGE_GEOM_ATTRIBUTE_ID_NORMAL:									\
		*ppNormalTable = (void*)pUniforms[uniformIndex##attrId];		\
		break;															\
	case EDGE_GEOM_ATTRIBUTE_ID_TANGENT:								\
		*ppTangentTable = (void*)pUniforms[uniformIndex##attrId];		\
		break;															\
	case EDGE_GEOM_ATTRIBUTE_ID_BINORMAL:								\
		*ppBinormalTable = (void*)pUniforms[uniformIndex##attrId];		\
		break;															\
	default:															\
		break;															\
	}																	\
	const bool addMissingComponents##attrId = (((EdgeGeomAttributeId)attrId) == EDGE_GEOM_ATTRIBUTE_ID_POSITION && attrComponentCount < 4); \
	const vec_uint4 missingComponentMask##attrId = (vec_uint4)spu_maskw(0xF >> attrComponentCount); \
	const int offset0##attrId = attrOffset;							\
	const bool attribIsAligned0##attrId = (offset0##attrId % 16 == 0); \
	const bool useFastExtract0##attrId = isF32##attrId && attribIsAligned0##attrId; \
	const int inQuad0##attrId##0 = offset0##attrId / 16;				\
	const int inQuad0##attrId##1 = (inQuad0##attrId##0 + 1 >= kNumQuads) ? kNumQuads-1 : inQuad0##attrId##0 + 1; \
	vec_uchar16 shuf0##attrId;											\
	if (useFastExtract0##attrId)										\
	{																	\
		/* No shuffle masks necessary */								\
	}																	\
	else if (!attribIsAligned0##attrId)								\
	{																	\
		const vec_int4 quadOff = (vec_int4)si_from_int(offset0##attrId % 16); \
		const vec_uchar16 temp = (vec_uchar16)spu_add((vec_int4)s_ABCD, spu_shuffle(quadOff, quadOff, s_DDDDDDDDDDDDDDDD));	\
		shuf0##attrId = spu_shuffle(temp, temp, shufSeed##attrId);	\
	}																	\
	else																\
	{																	\
		shuf0##attrId = spu_shuffle(s_ABCD, s_ABCD, shufSeed##attrId); \
	}																	\
	if (doFixedPointSetW##attrId) {                                        \
		shuf0##attrId = (vec_uchar16)spu_sel((vec_uint4)shuf0##attrId, spu_splats(0xc0c0c0c0), mask_000F);    \
	}                                                                   \
																		\
	const int offset1##attrId = attrOffset + kStride;					\
	const bool attribIsAligned1##attrId = (offset1##attrId % 16 == 0); \
	const bool useFastExtract1##attrId = isF32##attrId && attribIsAligned1##attrId; \
	const int inQuad1##attrId##0 = offset1##attrId / 16;				\
	const int inQuad1##attrId##1 = (inQuad1##attrId##0 + 1 >= kNumQuads) ? kNumQuads-1 : inQuad1##attrId##0 + 1; \
	vec_uchar16 shuf1##attrId;											\
	if (useFastExtract1##attrId)										\
	{																	\
		/* No shuffle masks necessary */								\
	}																	\
	else if (!attribIsAligned1##attrId)								\
	{																	\
		const vec_int4 quadOff = (vec_int4)si_from_int(offset1##attrId % 16);		\
		const vec_uchar16 temp = (vec_uchar16)spu_add((vec_int4)s_ABCD, spu_shuffle(quadOff, quadOff, s_DDDDDDDDDDDDDDDD));	\
		shuf1##attrId = spu_shuffle(temp, temp, shufSeed##attrId);	\
	}																	\
	else																\
	{																	\
		shuf1##attrId = spu_shuffle(s_ABCD, s_ABCD, shufSeed##attrId); \
	}																	\
	if (doFixedPointSetW##attrId) {                                        \
		shuf1##attrId = (vec_uchar16)spu_sel((vec_uint4)shuf1##attrId, spu_splats(0xc0c0c0c0), mask_000F);    \
	}                                                                   \
																		\
	vec_char16 attr0##attrId;											\
	vec_char16 attr1##attrId;											\
	vec_char16 inAttr0##attrId;										\
	vec_char16 inAttr1##attrId;										\
	const qword pAttr0##attrId = si_from_ptr( (char*)(pUniforms[uniformIndex##attrId])+kVertCount*0x10); \
	const qword pAttr1##attrId = si_from_ptr( (char*)(pUniforms[uniformIndex##attrId])+kVertCount*0x10+0x10) ;	\
	EDGE_GEOM_DECOMPRESS_PRIME_##attrType(attrId);
	
#define EDGE_GEOM_DECOMPRESS_INIT_F32(attrId, attrOffset, attrComponentCount)			\
	const vec_uchar16 shufSeed##attrId = s_ABCD;						\
	const bool isF32##attrId = true;									\
	const bool doFixedPointSetW##attrId = false;                           \
	const uint32_t loopCycleCount##attrId = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(F32, attrId, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_F16(attrId, attrOffset, attrComponentCount)			\
	const vec_uchar16 shufSeed##attrId = s_A0B0C0D0;					\
	const bool isF32##attrId = false;									\
	const bool doFixedPointSetW##attrId = false;                           \
	const uint32_t loopCycleCount##attrId = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(F16, attrId, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_U8(attrId, attrOffset, attrComponentCount)				\
	const vec_uchar16 shufSeed##attrId = s_0A000B000C000D00;			\
	const bool isF32##attrId = false;									\
	const bool doFixedPointSetW##attrId = false;                           \
	const uint32_t loopCycleCount##attrId = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(U8, attrId, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_I16(attrId, attrOffset, attrComponentCount)						\
	const vec_uchar16 shufSeed##attrId = s_A0B0C0D0;					\
	const bool isF32##attrId = false;									\
	const bool doFixedPointSetW##attrId = false;                           \
	const uint32_t loopCycleCount##attrId = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(I16, attrId, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_U8N(attrId, attrOffset, attrComponentCount)							\
	const vec_uchar16 shufSeed##attrId = s_0A000B000C000D00;			\
	const bool isF32##attrId = false;									\
	const bool doFixedPointSetW##attrId = false;                           \
	const uint32_t loopCycleCount##attrId = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(U8N, attrId, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_I16N(attrId, attrOffset, attrComponentCount)						\
	const vec_uchar16 shufSeed##attrId = s_A0B0C0D0;					\
	const bool isF32##attrId = false;									\
	const bool doFixedPointSetW##attrId = false;                           \
	const uint32_t loopCycleCount##attrId = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(I16N, attrId, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(attrId, attrOffset, attrComponentCount)					\
	const vec_uchar16 shufSeed##attrId = s_CD00BC00AB000000;			\
	const bool isF32##attrId = false;									\
	const bool doFixedPointSetW##attrId = false;                           \
	const uint32_t loopCycleCount##attrId = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(X11Y11Z10N, attrId, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_FIXED_POINT(attrId, attrOffset, attrComponentCount, numIntBits0, numPrecisionBits0, numIntBits1, numPrecisionBits1, numIntBits2, numPrecisionBits2, numIntBits3, numPrecisionBits3) \
	EDGE_VAR vec_int4 fixed1##attrId = (vec_int4)spu_splats(0);			\
	EDGE_VAR vec_int4 fixed2##attrId = (vec_int4)spu_splats(0);			\
	const bool hasW##attrId = ((numIntBits3) + (numPrecisionBits3)) > 0; \
	const vec_int4 fixedIntBits##attrId = (vec_int4){(numIntBits0),(numIntBits1),(numIntBits2),(numIntBits3)}; \
	const vec_int4 fixedMantissaBits##attrId = (vec_int4){(numPrecisionBits0),(numPrecisionBits1),(numPrecisionBits2),(numPrecisionBits3)}; \
	const vec_int4 totalBits##attrId = spu_add(fixedIntBits##attrId, (vec_int4)fixedMantissaBits##attrId); \
	const vec_int4 totalUnusedBits##attrId = spu_sub(32, totalBits##attrId);	\
	const vec_int4 garbageMask##attrId = spu_rlmask((vec_int4)spu_splats(0xFFFFFFFF), \
		(vec_int4)spu_sub(0, totalUnusedBits##attrId));				\
	vec_int4 rshifts##attrId = spu_sub(0, spu_add(						\
		spu_add(totalUnusedBits##attrId, spu_rlmaskqwbyte(totalUnusedBits##attrId, -4)), \
		spu_add(spu_rlmaskqwbyte(totalUnusedBits##attrId, -8), spu_rlmaskqwbyte(totalUnusedBits##attrId, -12)) \
			));															\
	rshifts##attrId = spu_sel( spu_add(rshifts##attrId, 7), \
		rshifts##attrId, spu_and((vec_uint4)spu_splats(7), (vec_uint4)spu_maskb(0xFFF0)) ); \
	const int rshift1##attrId = si_to_int((qword)rshifts##attrId);		\
	const int rshift2##attrId = si_to_int((qword)spu_slqwbyte(rshifts##attrId, 4)); \
	const int rshift3##attrId = si_to_int((qword)spu_slqwbyte(rshifts##attrId, 8)); \
	const int rshift4##attrId = si_to_int((qword)spu_slqwbyte(rshifts##attrId, 12)); \
	const vec_int4 fixedIntShift##attrId = spu_sub(0, fixedMantissaBits##attrId);	\
	const vec_uint4 fixedMantissaShift##attrId = (vec_uint4)spu_sub(32, fixedMantissaBits##attrId); \
	const vec_uchar16 shufSeed##attrId = s_ABCD;						\
	const bool isF32##attrId = false;									\
	const bool doFixedPointSetW##attrId = !hasW##attrId;				\
	const uint32_t loopCycleCount##attrId = 3;							\
	const EDGE_VAR vec_int4 fixedPointOffset##attrId = (vec_int4){pFixedOffsets[fixedOffsetNum*4+0], pFixedOffsets[fixedOffsetNum*4+1], pFixedOffsets[fixedOffsetNum*4+2], pFixedOffsets[fixedOffsetNum*4+3]}; \
	++fixedOffsetNum;													\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(FIXED_POINT, attrId, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(attrId, attrOffset, attrComponentCount)	\
	vec_float4 fToSqrt##attrId            =  (vec_float4)spu_splats(0); \
	vec_uchar16 complexMask0##attrId      = (vec_uchar16)spu_splats(0); \
	vec_uchar16 complexMask1##attrId      = (vec_uchar16)spu_splats(0); \
	vec_float4 fData0##attrId             =  (vec_float4)spu_splats(0); \
	vec_float4 fData0Holder##attrId       =  (vec_float4)spu_splats(0); \
	vec_float4 fData0Sq##attrId           =  (vec_float4)spu_splats(0); \
	vec_uint4 iData0##attrId              =   (vec_uint4)spu_splats(0); \
	vec_uint4 iData1##attrId              =   (vec_uint4)spu_splats(0); \
	vec_uchar16 s_output0##attrId         = (vec_uchar16)spu_splats(0); \
	vec_uchar16 s_output1##attrId         = (vec_uchar16)spu_splats(0); \
	vec_float4 fSqrt##attrId              =  (vec_float4)spu_splats(0); \
	const vec_uchar16 shufSeed##attrId    = s_00C000AB00BC0000;		\
	const bool isF32##attrId              = false;						\
	const bool doFixedPointSetW##attrId = false;						\
	const uint32_t loopCycleCount##attrId = 4;							\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(UNIT_VECTOR, attrId, attrOffset, attrComponentCount);      

#define EDGE_GEOM_DECOMPRESS_INIT_GLOBAL_COMMON(ctx, vertexes, vertCount, fixedOffsets)		\
	void ** const ppPositionTable = &ctx->positionTable;						\
	void ** const ppNormalTable   = &ctx->normalTable;							\
	void ** const ppTangentTable  = &ctx->tangentTable;						\
	void ** const ppBinormalTable = &ctx->binormalTable;						\
	float ** const pUniforms = (float **)ctx->uniformTables;									\
	const int kVertCount = ((vertCount)+1) & ~0x1;						\
	vec_char16 uniformTableOffsets[5] = {(vec_char16)si_from_int(0), (vec_char16)si_from_int(0), (vec_char16)si_from_int(0), (vec_char16)si_from_int(-kVertCount*0x10), (vec_char16)si_from_int(0) }; \
	char *pInput = (char*)(vertexes);									\
	const EDGE_VAR vec_uint4 mask_000F           = (vec_uint4)spu_maskb(0x000F); \
	/* shuffle masks */													\
	extern const EDGE_VAR vec_uchar16 s_00C000AB00BC0000;				\
	extern const EDGE_VAR vec_uchar16 s_0A000B000C000D00;				\
	extern const EDGE_VAR vec_uchar16 s_A0B0C0D0;						\
	extern const EDGE_VAR vec_uchar16 s_BCbc;							\
	extern const EDGE_VAR vec_uchar16 s_CD00BC00AB000000;				\
	extern const EDGE_VAR vec_uchar16 s_ABCD;							\
	const EDGE_VAR vec_uchar16 s_DDDDDDDDDDDDDDDD = (vec_uchar16)spu_splats(0x03030303); \
	const EDGE_VAR qword unusedAttrId = si_shufb(si_from_int(EDGE_GEOM_ATTRIBUTE_ID_UNKNOWN), \
		(qword)s_DDDDDDDDDDDDDDDD, (qword)s_DDDDDDDDDDDDDDDD);			\
	/* U8N variables */													\
	const EDGE_VAR vec_float4 u8nScale = (vec_float4)spu_splats(1.0f / 255.0f);	\
	const EDGE_VAR vec_float4 u8nBias  = (vec_float4)spu_splats(0.0f);	\
	/* I16N variables */													\
	const EDGE_VAR vec_float4 i16nScale = (vec_float4)spu_splats(2.0f / 65535.0f);	\
	const EDGE_VAR vec_float4 i16nBias  = (vec_float4)spu_splats(1.0f / 65535.0f);	\
	/* F16 variables */													\
	const EDGE_VAR vec_int4 expHoleMask = (vec_int4)spu_splats(0x70000000); \
	const EDGE_VAR vec_uint4 signMask   = (vec_uint4)spu_splats(0x80000000); \
	const EDGE_VAR vec_int4 expRebias   = (vec_int4)spu_splats(0x38000000); \
	/* X11Y11Z10N variables */											\
	const EDGE_VAR vec_int4 bitMask               =    (vec_int4){0xFFE00000, 0xFFE00000, 0xFFC00000, 0x00000000}; \
	const EDGE_VAR vec_uint4 shiftValues          =   (vec_uint4){5, 2, 0, 0}; \
	const EDGE_VAR vec_float4 x11y11z10Scale      =  (vec_float4)(vec_int4){0x3F801002, 0x3F801002, 0x3F802008, 0x00000000}; \
	const EDGE_VAR vec_float4 x11y11z10Bias       =  (vec_float4)(vec_int4){0x3A001002, 0x3A001002, 0x3A802008, 0x3F800000}; \
	/* FIXED_POINT variables */											\
	const EDGE_VAR vec_uint4 signBit             = (vec_uint4)spu_splats(0x80000000);\
	const EDGE_VAR vec_uint4 mask_00FF           = (vec_uint4)spu_maskb(0x00FF);\
	const EDGE_VAR vec_uint4 mask_0F0F           = (vec_uint4)spu_maskb(0x0F0F);\
	/* UNIT_VECTOR variables */											\
	const EDGE_VAR vec_int4 rotValues            =   (vec_int4){-4, -6, -4, 0};	\
	const EDGE_VAR vec_uint4 formatSelect        =   (vec_uint4){0xf0, 0x3ff, 0x3ff, 0x0}; \
	const EDGE_VAR vec_uint4 formatMask          =   (vec_uint4){0x0, 0x3f800000, 0x3f800000, 0x0}; \
	const EDGE_VAR vec_float4 fSwUnitVectorScale =   (vec_float4)spu_splats(0x46352e7c); \
	const EDGE_VAR vec_float4 fSwUnitVectorBias  =   (vec_float4)spu_splats(0xc6353150); \
	const EDGE_VAR vec_uint4 outputMask          =   (vec_uint4){0x0, 0x0, 0x0, 0x0c0d0e0f}; \
	const EDGE_VAR vec_uint4 outputSelect        =   (vec_uint4){0x7fffffff, 0x7fffffff, 0x7fffffff, 0x0}; \
	const EDGE_VAR vec_uint4 swSelect            =   (vec_uint4){0x80000000, 0x80000000, 0x80000000, 0xffffffff}; \
	const EDGE_VAR vec_uint4 shuffleAdjust       =   (vec_uint4){0x08080808,0x08080808,0x08080808,0x0}; \
	extern const EDGE_VAR qword uvMaskTable[12];						\
	const EDGE_VAR vec_char16 pMaskTable         = (vec_char16)si_from_ptr(uvMaskTable); \
	EDGE_VAR uint32_t fixedOffsetNum = 0;								\
	EDGE_VAR uint32_t *pFixedOffsets = (uint32_t *)fixedOffsets;

#define EDGE_GEOM_DECOMPRESS_SET_STRIDE(stride) \
	const int kStride = stride;											\
	const int kStrideTimes2 = 2*kStride;								\
	const int kNumQuads = (kStrideTimes2+15) / 16;						\
	vec_char16 inQuads[kNumQuads];

#define EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(stride) \
	const vec_uint4 kMissingComponents = (vec_uint4)(vec_float4){0,0,0,1.0f}; \
	EDGE_GEOM_DECOMPRESS_SET_STRIDE(stride);							\
	EDGE_GEOM_DECOMPRESS_LOAD_COMMON();

#define EDGE_GEOM_DECOMPRESS_LOOP_DONE() \
	(si_to_int(uniformTableOffsets[3]) == 0)

// This is a special-case macro that handles streams consisting of a
// single 3-element F32 attribute (such as shadow map position-only
// streams).  We can schedule this code roughly 2x faster if we handle
// it separately.
#define EDGE_GEOM_DECOMPRESS_SPECIALCASE_F32_3(ctx, vertexes, vertCount) \
	float ** const pUniforms = (float **)((ctx)->uniformTables);		\
	const qword unusedTableBits = si_gbb(si_ceqb(unusedAttrId, \
			ctx->uniformTableToAttributeIdMapping));					\
	int uniformIndex = si_to_int(si_ai(si_clz(unusedTableBits), -16)); \
	ctx->uniformTableToAttributeIdMapping = si_shufb(si_from_int(EDGE_GEOM_ATTRIBUTE_ID_POSITION), \
		ctx->uniformTableToAttributeIdMapping,							\
		si_cbd(si_from_int(uniformIndex), 0));							\
	edgeGeomSetPositionUniformTable(ctx, (qword*)(pUniforms[uniformIndex])); \
	const int kVertCount = ((vertCount)+3) & ~0x3;						\
	qword outOffset = si_from_int(-kVertCount*0x10);					\
																		\
	const qword pOutput0 = si_from_ptr((char*)(pUniforms[uniformIndex]) + (kVertCount*0x10) + 0x00); \
	const qword pOutput1 = si_from_ptr((char*)(pUniforms[uniformIndex]) + (kVertCount*0x10) + 0x10); \
	const qword pOutput2 = si_from_ptr((char*)(pUniforms[uniformIndex]) + (kVertCount*0x10) + 0x20); \
	const qword pOutput3 = si_from_ptr((char*)(pUniforms[uniformIndex]) + (kVertCount*0x10) + 0x30); \
	char *pInput = (char*)(vertexes);									\
	const qword k1_0f = (qword)spu_splats(1.0f);						\
	const qword s_ABCD = si_andbi(si_cwd((qword)spu_splats(0), 0), 0xF);		\
	const qword s_BCDa = si_ah(s_ABCD, si_ilh(0x0404));					\
	const qword s_CDab = si_ah(s_ABCD, si_ilh(0x0808));					\
	const qword s_Dabc = si_ah(s_BCDa, si_ilh(0x0808));					\
	const qword mask_000F = si_fsmbi(0x000F);							\
																		\
	qword inQuad0 = si_lqd(si_from_ptr(pInput), 0x00);					\
	qword inQuad1 = si_lqd(si_from_ptr(pInput), 0x10);					\
	qword inQuad2 = si_lqd(si_from_ptr(pInput), 0x20);					\
	pInput += 0x30;														\
	qword nextOffset;													\
	do																	\
	{																	\
		const qword finalOut0 = si_selb(inQuad0, k1_0f, mask_000F);		\
		const qword outQuad1  = si_shufb(inQuad0, inQuad1, s_Dabc);		\
		const qword outQuad2  = si_shufb(inQuad1, inQuad2, s_CDab);		\
		const qword finalOut3 = si_shufb(inQuad2, k1_0f, s_BCDa);		\
		inQuad0 = si_lqd(si_from_ptr(pInput), 0x00);					\
		inQuad1 = si_lqd(si_from_ptr(pInput), 0x10);					\
		inQuad2 = si_lqd(si_from_ptr(pInput), 0x20);					\
		const qword finalOut1 = si_selb(outQuad1, k1_0f, mask_000F);	\
		const qword finalOut2 = si_selb(outQuad2, k1_0f, mask_000F);	\
		pInput += 0x30;													\
		nextOffset = si_ai(outOffset, 0x40);							\
		si_stqx(finalOut0, pOutput0, outOffset);						\
		si_stqx(finalOut1, pOutput1, outOffset);						\
		si_stqx(finalOut2, pOutput2, outOffset);						\
		si_stqx(finalOut3, pOutput3, outOffset);						\
		outOffset = nextOffset;											\
	} while(si_to_int(nextOffset) != 0);    

#endif // EDGEGEOM_DECOMPRESS_H
