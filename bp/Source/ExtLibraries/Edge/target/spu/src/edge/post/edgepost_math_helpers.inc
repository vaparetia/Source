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

.ifndef EDGEPOST_MATH_HELPERS_INC
.set EDGEPOST_MATH_HELPERS_INC, 1

// declared usefull constants
.macro EDGEPOST_DECLARE_MATH_HELPER_CONSTANTS

	// Exp2
	.reg		kEdgePost_Exp2_c0, kEdgePost_Exp2_ln2 : readoptional
	.reg		kEdgePost_Exp2_f0, kEdgePost_Exp2_f1, kEdgePost_Exp2_f2, kEdgePost_Exp2_f3 : readoptional
	.reg		kEdgePost_Exp2_f4, kEdgePost_Exp2_f5, kEdgePost_Exp2_f6, kEdgePost_Exp2_f7 : readoptional
	ilhu		kEdgePost_Exp2_c0, 16255
	iohl		kEdgePost_Exp2_c0, 65535
	ilf32		kEdgePost_Exp2_ln2, 0.69314718055995f
	ilf32		kEdgePost_Exp2_f0, -0.0001413161f
	ilf32		kEdgePost_Exp2_f1, 0.0013298820f
	ilf32		kEdgePost_Exp2_f2, -0.0083013598f
	ilf32		kEdgePost_Exp2_f3, 0.0416573475f
	ilf32		kEdgePost_Exp2_f4, -0.1666653019f
	ilf32		kEdgePost_Exp2_f5, 0.4999999206f
	ilf32		kEdgePost_Exp2_f6, -0.9999999995f
	ilf32		kEdgePost_Exp2_f7, 1.f
	
	// Log2
	.reg		kEdgePost_Log2_f0, kEdgePost_Log2_f1, kEdgePost_Log2_f2 : readoptional
	.reg		kEdgePost_Log2_f3, kEdgePost_Log2_f4, kEdgePost_Log2_f5 : readoptional
	.reg		kEdgePost_Log2_f6, kEdgePost_Log2_f7                    : readoptional
	ilf32		kEdgePost_Log2_f0, -0.0093104962134977f
	ilf32		kEdgePost_Log2_f1, 0.052064690894143f
	ilf32		kEdgePost_Log2_f2, -0.13753123777116f
	ilf32		kEdgePost_Log2_f3, 0.24187369696082f
	ilf32		kEdgePost_Log2_f4, -0.34730547155299f
	ilf32		kEdgePost_Log2_f5, 0.47868480909345f
	ilf32		kEdgePost_Log2_f6, -0.72116591947498f
	ilf32		kEdgePost_Log2_f7, 1.4426898816672f
	
	// Gamma
	.reg		kEdgePost_Gamma_f0, kEdgePost_Gamma_f1 : readoptional
	.reg		kEdgePost_Gamma_f2, kEdgePost_Gamma_f3 : readoptional
	.reg		kEdgePost_Gamma_f4                     : readoptional
	ilf32		kEdgePost_Gamma_f0, 12.92f
	ilf32		kEdgePost_Gamma_f1, 1.f / 2.4f
	ilf32		kEdgePost_Gamma_f2, 1.055f
	ilf32		kEdgePost_Gamma_f3, -0.055f
	ilf32		kEdgePost_Gamma_f4, .0031308f
	
.endmacro

// 
.macro EDGEPOST_EXP2( d, s )
	.localreg	t0, t1, t2, hi, lo
	rotmai		t0, s, -31
	andc		t0, t0, kEdgePost_Exp2_c0
	fa			t0, t0, s
	cflts		t0, t0, 0
	csflt		t1, t0, 0
	fs			t1, t1, s
	fm			t1, t1, kEdgePost_Exp2_ln2
	ai			t0, t0, 127
	shli		t0, t0, 23
	fm			t2, t1, t1
	fm			t2, t2, t2
	fma			hi, t1, kEdgePost_Exp2_f0, kEdgePost_Exp2_f1
	fma			hi, t1, hi, kEdgePost_Exp2_f2
	fma			hi, t1, hi, kEdgePost_Exp2_f3
	fma			lo, t1, kEdgePost_Exp2_f4, kEdgePost_Exp2_f5
	fma			lo, t1, lo, kEdgePost_Exp2_f6
	fma			lo, t1, lo, kEdgePost_Exp2_f7
	fma			lo, t2, hi, lo
	fm			d, lo, t0
.endmacro

// 
.macro EDGEPOST_LOG2( d, s )
	.localreg	exponent, x, x2, x4, hi, lo
	rotmi		exponent, s, -23
	andi		exponent, exponent, 0xff
	ai			exponent, exponent, -127
	shli		x, exponent, 23
	sf			x, x, s
	fs			x, x, kEdgePostFloatOne
	fm			x2, x, x
	fm			x4, x2, x2
	fma			hi, x, kEdgePost_Log2_f0, kEdgePost_Log2_f1
	fma			hi, x, hi, kEdgePost_Log2_f2
	fma			hi, x, hi, kEdgePost_Log2_f3
	fma			hi, x, hi, kEdgePost_Log2_f4
	fma			lo, x, kEdgePost_Log2_f5, kEdgePost_Log2_f6
	fma			lo, x, lo, kEdgePost_Log2_f7
	fm			lo, x, lo
	fma			x4, x4, hi, lo
	csflt		exponent, exponent, 0
	fa			d, x4, exponent
.endmacro

//	Emulate RSX: out = ((f<=.0031308) ? 12.92 * f : 1.055 * pow(f,1.0/2.4) - 0.055)
.macro EDGEPOST_GAMMA( d, s )
	.localreg	d0, d1, m
	fm			d0, s, kEdgePost_Gamma_f0							// 12.92
	EDGEPOST_LOG2( d1, s)
	fm			d1, d1, kEdgePost_Gamma_f1							// 1.0 / 2.4
	EDGEPOST_EXP2( d1, d1)
	fma			d1, d1, kEdgePost_Gamma_f2, kEdgePost_Gamma_f3		// 1.055, -0.055
	fcgt		m, s, kEdgePost_Gamma_f4							// .0031308
	selb		d, d0, d1, m
.endmacro


.endif
