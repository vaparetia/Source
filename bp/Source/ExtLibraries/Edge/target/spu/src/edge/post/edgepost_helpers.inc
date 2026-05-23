/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

//--------------------------------------------------------------------------------------------------
/**
	Misc helper macros
**/
//--------------------------------------------------------------------------------------------------

.ifndef EDGEPOST_HELPERS_INC
.set EDGEPOST_HELPERS_INC, 1

// declared usefull constants
.macro EDGEPOST_DECLARE_HELPER_CONSTANTS
	.reg		kEdgePostSix : readoptional
	.reg		kEdgePostFifteen : readoptional
	.reg		kEdgePostTwenty : readoptional
	.reg		kEdgePostMaxPixel : readoptional
	.reg		kEdgePostMaxUShort : readoptional
	.reg		kEdgePostShuf01 : readoptional
	.reg		kEdgePostShuf23 : readoptional
	.reg		kEdgePostUnpack0 : readoptional
	.reg		kEdgePostUnpack1 : readoptional
	.reg		kEdgePostUnpack2 : readoptional
	.reg		kEdgePostUnpack3 : readoptional
	.reg		kEdgePostUnpack0_16 : readoptional
	.reg		kEdgePostUnpack1_16 : readoptional
	.reg		kEdgePostPack_16 : readoptional
	.reg		kEdgePostDoubleAlpha : readoptional
	.reg		kEdgePostFloatOne : readoptional
	.reg		kEdgePostFloatZero : readoptional
	.reg		kEdgePostToFloatFactor : readoptional
	.reg		kEdgePostFromFloatFactor : readoptional
	.reg		kEdgePostReplicateX : readoptional
	.reg		kEdgePostReplicateY : readoptional
	.reg		kEdgePostReplicateZ : readoptional
	.reg		kEdgePostReplicateW : readoptional
	.reg		kEdgePostUnpackReds : readoptional
	.reg		kEdgePostUnpackGreens : readoptional
	.reg		kEdgePostUnpackBlues : readoptional
	.reg		kEdgePostUnpackAlphas : readoptional
	
	ila			kEdgePostSix, 6
	ila			kEdgePostFifteen, 15
	ila			kEdgePostTwenty, 20
	ila			kEdgePostMaxPixel, 0xff
	ila			kEdgePostMaxUShort, 0xffff
	il128		kEdgePostShuf01, "DHLPdhlp00000000"
	il128		kEdgePostShuf23, "00000000DHLPdhlp"
	il128		kEdgePostUnpack0, "000A000B000C000D"
	il128		kEdgePostUnpack1, "000E000F000G000H"
	il128		kEdgePostUnpack2, "000I000J000K000L"
	il128		kEdgePostUnpack3, "000M000N000O000P"
	il128		kEdgePostUnpack0_16, "0a0b0c0d"
	il128		kEdgePostUnpack1_16, "0e0f0g0h"
	il128		kEdgePostPack_16, 0x02030607_0a0b0e0f_12131617_1a1b1e1f
	il128		kEdgePostDoubleAlpha, 0x00020002_00010001_00010001_00010001
	ilf32		kEdgePostToFloatFactor, 256.0f/255.0f
	ilf32		kEdgePostFromFloatFactor, 255.0f/256.0f
	ilf32		kEdgePostFloatOne, 1.0f
	ilf32		kEdgePostFloatZero, .0f
	il128		kEdgePostReplicateX, "aaaa"
	il128		kEdgePostReplicateY, "bbbb"
	il128		kEdgePostReplicateZ, "cccc"
	il128		kEdgePostReplicateW, "dddd"
	il128		kEdgePostUnpackReds, "000B000F000J000N"
	il128		kEdgePostUnpackGreens, "000C000G000K000O"
	il128		kEdgePostUnpackBlues, "000D000H000L000P"
	il128		kEdgePostUnpackAlphas, "000A000E000I000M"
	
	// FP16 constants
	.reg		kEdgePostFp16expHoleMask, kEdgePostFp16DecSignMask, kEdgePostFp16EncSignMask : readoptional
	.reg		kEdgePostFp16expRebias, kEdgePostFp16expMax, kEdgePostFp16expMin : readoptional
	.reg		kEdgePostFp16Max, kEdgePostFp16Shuf0 : readoptional
	il128		kEdgePostFp16expHoleMask, 0x70000000_70000000_70000000_70000000
	il128		kEdgePostFp16DecSignMask,0x80000000_80000000_80000000_80000000
	il128		kEdgePostFp16EncSignMask,0x80008000_80008000_80008000_80008000
	il128		kEdgePostFp16expRebias, 0x38000000_38000000_38000000_38000000
	il128		kEdgePostFp16expMax, 0x47804780_47804780_47804780_47804780
	il128		kEdgePostFp16expMin, 0x38003800_38003800_38003800_38003800
	il128		kEdgePostFp16Max, 0x7BFF7Bff_7BFF7Bff_7BFF7Bff_7BFF7Bff
	il128		kEdgePostFp16Shuf0, "ABEFIJMNabefijmn"
.endmacro

// unpack 4 8bit pixels
.macro EDGEPOST_UNPACK_PIXELS8( p0, p1, p2, p3, s )
	shufb		p0, s, s, kEdgePostUnpack0
	shufb		p1, s, s, kEdgePostUnpack1
	shufb		p2, s, s, kEdgePostUnpack2
	shufb		p3, s, s, kEdgePostUnpack3
.endmacro

// pack 4 8bit pixels
.macro EDGEPOST_PACK_PIXELS8( r, p0, p1, p2, p3)
	shufb		p0, p0, p1, kEdgePostShuf01
	shufb		p2, p2, p3, kEdgePostShuf23
	or			r, p0, p2
.endmacro

// unpack 2 16 bit pixels ( 12.4)
.macro EDGEPOST_UNPACK_PIXELS16( p0, p1, s )
	shufb		p0, s, s, kEdgePostUnpack0_16
	shufb		p1, s, s, kEdgePostUnpack1_16
.endmacro

// pack 2 16bit pixels ( 12.4)
.macro EDGEPOST_PACK_PIXELS16( r, p0, p1 )
	shufb		r, p0, p1, kEdgePostPack_16
.endmacro

// clamp to 255 componentwise
.macro EDGEPOST_CLAMP_PIXEL8( p )
	.localreg	m
	cgt			m, p, kEdgePostMaxPixel
	selb		p, p, kEdgePostMaxPixel, m
.endmacro

// clamp to 0xffff componentwise
.macro EDGEPOST_CLAMP_PIXEL16( p )
	.localreg	m
	cgt			m, p, kEdgePostMaxUShort
	selb		p, p, kEdgePostMaxUShort, m
.endmacro

// clamp to 1.0 componentwise
.macro EDGEPOST_CLAMP_FLOAT( p )
	.localreg	m
	fcgt		m, p, kEdgePostFloatOne
	selb		p, p, kEdgePostFloatOne, m
.endmacro

// convert unsigned to floats
.macro EDGEPOST_U8_TO_FLOAT( d, s)
	cuflt		d, s, 8
	fm			d, d, kEdgePostToFloatFactor
.endmacro

// convert floats to unsigned
.macro EDGEPOST_FLOAT_TO_U8( d, s )
	fm			d, s, kEdgePostFromFloatFactor
	cfltu		d, d, 8
.endmacro

// convert FP16 to floats
//	fp16 is in the high nibble
.macro EDGEPOST_FP16_TO_FLOAT( d, s )
	.localreg	t0, t1
	rotmai		t0, s, -3
	andc		t0, t0, kEdgePostFp16expHoleMask
	andc		t1, s, kEdgePostFp16DecSignMask
	clgti		t1, t1, 0
	and			t1, t1, kEdgePostFp16expRebias
	a			d, t0, t1
.endmacro

// fp16 is left in the high nibble
.macro EDGEPOST_FLOAT_TO_FP16( d, s )
	.localreg	nz, m, x, e
	andc		nz, s, kEdgePostFp16EncSignMask
	clgth		m, kEdgePostFp16expRebias, nz
	clgth		x, kEdgePostFp16expMax, nz
	sf			e, kEdgePostFp16expRebias, s
	rotqbii		e, e, 3
	selb		e, kEdgePostFp16Max, e, x
	selb		d, e, s, kEdgePostFp16EncSignMask
	andc		d, d, m
.endmacro

// pack 8 floats into one qword as 8 fp16
.macro EDGEPOST_PACK_TWO_FP16( d, s0, s1 )
	.localreg	nz, m, x, e, s, r0, r1
	shufb		s, s0, s1, kEdgePostFp16Shuf0
	andc		nz, s, kEdgePostFp16EncSignMask
	clgth		m, kEdgePostFp16expMin, nz
	clgth		x, kEdgePostFp16expMax, nz
	sf			r0, kEdgePostFp16expRebias, s0
	sf			r1, kEdgePostFp16expRebias, s1
	rotqbii		r0, r0, 3
	rotqbii		r1, r1, 3
	shufb		e, r0, r1, kEdgePostFp16Shuf0
	selb		e, kEdgePostFp16Max, e, x
	selb		d, e, s, kEdgePostFp16EncSignMask
	andc		d, d, m
.endmacro

.endif
