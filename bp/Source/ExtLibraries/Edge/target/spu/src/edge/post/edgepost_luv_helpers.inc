/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

//--------------------------------------------------------------------------------------------------
/**
	Misc math helper macros
**/
//--------------------------------------------------------------------------------------------------

.ifndef EDGEPOST_LUV_HELPERS_INC
.set EDGEPOST_LUV_HELPERS_INC, 1

// declared usefull constants
.macro EDGEPOST_DECLARE_LUV_HELPER_CONSTANTS

	// Encoding constant

	.reg		kEdgePost_Luv_Encode_ShufUV : readoptional
	il128		kEdgePost_Luv_Encode_ShufUV, "Dd00Hh00Ll00Pp00"
	
	.reg		kEdgePost_Luv_Encode_M01, kEdgePost_Luv_Encode_M02, kEdgePost_Luv_Encode_M03 : readoptional
	.reg		kEdgePost_Luv_Encode_M11, kEdgePost_Luv_Encode_M12, kEdgePost_Luv_Encode_M13 : readoptional
	.reg		kEdgePost_Luv_Encode_M21, kEdgePost_Luv_Encode_M22, kEdgePost_Luv_Encode_M23 : readoptional
	ilf32		kEdgePost_Luv_Encode_M01, 0.2209f
	ilf32		kEdgePost_Luv_Encode_M02, 0.1138f
	ilf32		kEdgePost_Luv_Encode_M03, 0.0102f
	ilf32		kEdgePost_Luv_Encode_M11, 0.3390f
	ilf32		kEdgePost_Luv_Encode_M12, 0.6780f
	ilf32		kEdgePost_Luv_Encode_M13, 0.1130f
	ilf32		kEdgePost_Luv_Encode_M21, 0.4184f
	ilf32		kEdgePost_Luv_Encode_M22, 0.7319f
	ilf32		kEdgePost_Luv_Encode_M23, 0.2969f
	
	.reg		kEdgePost_Luv_Encode_kShuf0, kEdgePost_Luv_Encode_kShuf1, kEdgePost_Luv_Encode_kShuf2 : readoptional
	.reg		kEdgePost_Luv_Encode_kShuf3, kEdgePost_Luv_Encode_kShuf4, kEdgePost_Luv_Encode_kShuf5 : readoptional
	il128		kEdgePost_Luv_Encode_kShuf0, "Bb00"
	il128		kEdgePost_Luv_Encode_kShuf1, "Cc00"
	il128		kEdgePost_Luv_Encode_kShuf2, "Dd00"
	il128		kEdgePost_Luv_Encode_kShuf3, "00Bb"
	il128		kEdgePost_Luv_Encode_kShuf4, "00Cc"
	il128		kEdgePost_Luv_Encode_kShuf5, "00Dd"
	
	// Decoding constant
	
	.reg		kEdgePost_Luv_Decode_UnpackU : readoptional
	.reg		kEdgePost_Luv_Decode_UnpackV : readoptional
	.reg		kEdgePost_Luv_Decode_UnpackL : readoptional
	il128		kEdgePost_Luv_Decode_UnpackU, "000A000E000I000M"
	il128		kEdgePost_Luv_Decode_UnpackV, "000B000F000J000N"
	il128		kEdgePost_Luv_Decode_UnpackL, 0x0000ffff_0000ffff_0000ffff_0000ffff
	
	.reg		kEdgePost_Luv_Decode_M1, kEdgePost_Luv_Decode_M2, kEdgePost_Luv_Decode_M3 : readoptional
	il128		kEdgePost_Luv_Decode_M1, 0x00000000_40c00b78_c02cd9e8_bfe6594b
	il128		kEdgePost_Luv_Decode_M2, 0x00000000_bfaa7efa_404695ea_c0b8b50b
	il128		kEdgePost_Luv_Decode_M3, 0x00000000_3e9a0275_bf8b4a23_40b40ebf
	
	// LogLuv constants
	.reg		kEdgePost_Luv_Decode_k255F, kEdgePost_Luv_Decode_kC0, kEdgePost_Luv_Decode_kC1 : readoptional
	ilf32		kEdgePost_Luv_Decode_k255F, 255.f
	ilf32		kEdgePost_Luv_Decode_kC0, .5f
	ilf32		kEdgePost_Luv_Decode_kC1, -63.5f
	
.endmacro


.macro EDGEPOST_RGB_TO_LUV( L, u, v, r, g, b )
	// Color space transformation
	.localreg	Xp_Y_XYZp_x, Xp_Y_XYZp_y, Xp_Y_XYZp_z
	fm			Xp_Y_XYZp_x, r, kEdgePost_Luv_Encode_M01
	fma			Xp_Y_XYZp_x, g, kEdgePost_Luv_Encode_M02, Xp_Y_XYZp_x
	fma			Xp_Y_XYZp_x, b, kEdgePost_Luv_Encode_M03, Xp_Y_XYZp_x
	fm			Xp_Y_XYZp_y, r, kEdgePost_Luv_Encode_M11
	fma			Xp_Y_XYZp_y, g, kEdgePost_Luv_Encode_M12, Xp_Y_XYZp_y
	fma			Xp_Y_XYZp_y, b, kEdgePost_Luv_Encode_M13, Xp_Y_XYZp_y
	fm			Xp_Y_XYZp_z, r, kEdgePost_Luv_Encode_M21
	fma			Xp_Y_XYZp_z, g, kEdgePost_Luv_Encode_M22, Xp_Y_XYZp_z
	fma			Xp_Y_XYZp_z, b, kEdgePost_Luv_Encode_M23, Xp_Y_XYZp_z
	
	// calculate U & V
	.localreg	iz
	frest		iz, Xp_Y_XYZp_z
	fi			Xp_Y_XYZp_z, Xp_Y_XYZp_z, iz
	fm			v, Xp_Y_XYZp_y, Xp_Y_XYZp_z
	fm			u, Xp_Y_XYZp_x, Xp_Y_XYZp_z
	mov			L, Xp_Y_XYZp_y
.endmacro

.macro EDGEPOST_CONVERT_TO_PACKED_LUV_SIMD( packed, r, g, b )

	.localreg L, u, v
	EDGEPOST_RGB_TO_LUV( L, u, v, r, g, b)
	
	// convert to integer
	EDGEPOST_CLAMP_FLOAT( u )
	EDGEPOST_CLAMP_FLOAT( v )
	EDGEPOST_FLOAT_TO_U8( u, u)
	EDGEPOST_FLOAT_TO_U8( v, v)
	cfltu		L, L, 11
	EDGEPOST_CLAMP_PIXEL16( L )
	
	// assemble and pack
	shufb		packed, u, v, kEdgePost_Luv_Encode_ShufUV
	or			packed, packed, L
.endmacro

.macro EDGEPOST_CONVERT_TO_PACKED_LUV( packed, p0, p1, p2, p3 )
	.localreg	r, g, b, r1, g1, b1
	shufb		r, p0, p1, kEdgePost_Luv_Encode_kShuf0
	shufb		g, p0, p1, kEdgePost_Luv_Encode_kShuf1
	shufb		b, p0, p1, kEdgePost_Luv_Encode_kShuf2
	shufb		r1, p2, p3, kEdgePost_Luv_Encode_kShuf3
	shufb		g1, p2, p3, kEdgePost_Luv_Encode_kShuf4
	shufb		b1, p2, p3, kEdgePost_Luv_Encode_kShuf5
	or			r, r, r1
	or			g, g, g1
	or			b, b, b1
	EDGEPOST_CONVERT_TO_PACKED_LUV_SIMD( packed, r, g, b)
.endmacro

.macro EDGEPOST_CONVERT_FROM_LUV_SIMD( L, u, v, p0, p1, p2, p3 )
	
	// convert to XYZ model
	.localreg	x, y, z
	mov			y, L
	frest		z, v
	fi			z, v, z
	fm			z, z, y
	fm			x, u, z
	
	// convert to RGB
	.localreg	rgb0, x0, y0, z0
	.localreg	rgb1, x1, y1, z1
	.localreg	rgb2, x2, y2, z2
	.localreg	rgb3, x3, y3, z3
	
	// expand values
	shufb		x0, x, x, kEdgePostReplicateX
	shufb		x1, x, x, kEdgePostReplicateY
	shufb		x2, x, x, kEdgePostReplicateZ
	shufb		x3, x, x, kEdgePostReplicateW
	shufb		y0, y, y, kEdgePostReplicateX
	shufb		y1, y, y, kEdgePostReplicateY
	shufb		y2, y, y, kEdgePostReplicateZ
	shufb		y3, y, y, kEdgePostReplicateW
	shufb		z0, z, z, kEdgePostReplicateX
	shufb		z1, z, z, kEdgePostReplicateY
	shufb		z2, z, z, kEdgePostReplicateZ
	shufb		z3, z, z, kEdgePostReplicateW
	
	// 4 matrix multiplications
	fm			rgb0, z0, kEdgePost_Luv_Decode_M3
	fm			rgb1, z1, kEdgePost_Luv_Decode_M3
	fm			rgb2, z2, kEdgePost_Luv_Decode_M3
	fm			rgb3, z3, kEdgePost_Luv_Decode_M3
	fma			rgb0, y0, kEdgePost_Luv_Decode_M2, rgb0
	fma			rgb1, y1, kEdgePost_Luv_Decode_M2, rgb1
	fma			rgb2, y2, kEdgePost_Luv_Decode_M2, rgb2
	fma			rgb3, y3, kEdgePost_Luv_Decode_M2, rgb3
	fma			p0, x0, kEdgePost_Luv_Decode_M1, rgb0
	fma			p1, x1, kEdgePost_Luv_Decode_M1, rgb1
	fma			p2, x2, kEdgePost_Luv_Decode_M1, rgb2
	fma			p3, x3, kEdgePost_Luv_Decode_M1, rgb3
	
.endmacro

.macro EDGEPOST_CONVERT_FROM_PACKED_LUV( packed, p0, p1, p2, p3 )
	
	// unpack to simd
	.localreg	u, v, L
	shufb		u, packed, packed, kEdgePost_Luv_Decode_UnpackU
	shufb		v, packed, packed, kEdgePost_Luv_Decode_UnpackV
	and			L, packed, kEdgePost_Luv_Decode_UnpackL
	
	// convert to floats
	EDGEPOST_U8_TO_FLOAT( u, u)
	EDGEPOST_U8_TO_FLOAT( v, v)
	cuflt		L, L, 11
	
	EDGEPOST_CONVERT_FROM_LUV_SIMD( L, u, v, p0, p1, p2, p3 )
.endmacro

.macro EDGEPOST_CONVERT_FROM_PACKED_LOGLUV( packed, p0, p1, p2, p3 )
	
	// unpack to simd
	.localreg	L0, L1, u, v
	shufb		u, packed, packed, kEdgePostUnpackReds
	shufb		v, packed, packed, kEdgePostUnpackGreens
	shufb		L0, packed, packed, kEdgePostUnpackBlues
	shufb		L1, packed, packed, kEdgePostUnpackAlphas
	
	// convert to floats
	EDGEPOST_U8_TO_FLOAT( u, u)
	EDGEPOST_U8_TO_FLOAT( v, v)
	EDGEPOST_U8_TO_FLOAT( L0, L0)
	EDGEPOST_U8_TO_FLOAT( L1, L1)
	
	// unpack L
	.reg		L, Le
	fma			L, L0, kEdgePost_Luv_Decode_k255F, L1
	fma			L, L, kEdgePost_Luv_Decode_kC0, kEdgePost_Luv_Decode_kC1
	EDGEPOST_EXP2( Le, L )
	
	// call base macro	
	EDGEPOST_CONVERT_FROM_LUV_SIMD( Le, u, v, p0, p1, p2, p3 )
.endmacro

.endif
