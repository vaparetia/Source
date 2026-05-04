/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>
#include <cell/gcm_spu.h>

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"
#include "edge/geom/edgegeom_masks.h"

/**
 *	Transforms 4 vertices at a time into screen space, precomputes off-screen test, w=0 test, 
 *  quantizations (dependent on multisample type) for each vertex (for degenerate test) and 
 *  stuffs results into Z and W components.  Flips X and Y coordinates for vertices behind 
 *  the inversion point.
 * 
 *  The loop:
 * 	1) Load 4 vertices 
 * 	2) Transpose vertices
 * 	3) Multiply by local to screen matrix
 * 	4) Compute 1/W
 * 	5) Homogenize X, Y, Z
 * 	6) Flip sign of X, Y, Z if W is negative
 * 	7) Compare w against zero (which invalidates some tests)
 * 	8) Convert X, Y to signed ints
 *  9) Mask off any necessary bits to quantize according to sample type
 * 	10)Combine quantized X and Y into a word.
 * 	11)Untranspose transformed X, Y, Z
 * 	12)Compare against frustum
 * 	13)Collect frustum test results into bit pattern
 * 	14)Move frustum test bits into Z component
 * 	15)Move quantized X and Y with W==0 as high bit into W
 * 	16)Store results
 * 
 * 	@param ctx EdgeGeom job context
 */
#if TRANSFORMVERTEXESFORCULL_INTRINSICS
void transformVertexesForCull(EdgeGeomSpuContext *ctx)
{
    //constants, shuffles
	const qword fOne = (qword)spu_splats(1.0f);
	const qword floatZero = (qword)spu_splats(0.0f);

	//distribute viewport scale and offset values
	vec_char16 pViewportScales = (vec_char16)si_from_ptr(ctx->viewportInfo.viewportScales);
	vec_char16 pViewportOffsets = (vec_char16)si_from_ptr(ctx->viewportInfo.viewportOffsets);
	vec_float4 vpScale = (vec_float4)si_lqd(pViewportScales, 0x00);
	vec_float4 vpOffset = (vec_float4)si_lqd(pViewportOffsets, 0x00);
	const vec_uchar16 s_AAAA = (vec_uchar16)si_ila(0x10203);
	const vec_uchar16 s_BBBB = (vec_uchar16)si_orbi((qword)s_AAAA, 0x04);
	const vec_uchar16 s_CCCC = (vec_uchar16)si_orbi((qword)s_AAAA, 0x08);
	const vec_uchar16 s_DDDD = (vec_uchar16)si_orbi((qword)s_AAAA, 0x0C);
	const vec_uchar16 s_BbAa = (vec_uchar16)si_rotqbyi((qword)s_AaBb, 8);
	const vec_uchar16 s_ABcd = (vec_uchar16)si_rotqbyi((qword)s_cdAB, 8);
	qword viewportScaleX = (qword)spu_shuffle(vpScale, vpScale, s_AAAA);
	qword viewportScaleY = (qword)spu_shuffle(vpScale, vpScale, s_BBBB);
	qword viewportScaleZ = (qword)spu_shuffle(vpScale, vpScale, s_CCCC);
	qword viewportOffsetX = (qword)spu_shuffle(vpOffset, vpOffset, s_AAAA);
	qword viewportOffsetY = (qword)spu_shuffle(vpOffset, vpOffset, s_BBBB);
	qword viewportOffsetZ = (qword)spu_shuffle(vpOffset, vpOffset, s_CCCC);

	vec_char16 pScissorArea = (vec_char16)si_from_ptr(ctx->viewportInfo.scissorArea);
	vec_ushort8 scissorArea = (vec_ushort8)si_lqd(pScissorArea,0x00);
	vec_ushort8 scissorAreaSum = spu_add(scissorArea, spu_rlmaskqwbyte(scissorArea, -4));
	//spread, convert, broadcast broadcast broadcast broadcast
	qword scissorAreaW = si_shufb((qword)scissorArea, (qword)scissorAreaSum, (qword)s_0A0B0c0d);
	qword scissorAreaF = si_cuflt(scissorAreaW, 0);

	const vec_float4 depthRange0 = (vec_float4)spu_shuffle(scissorArea, scissorArea, s_CCCC);
	const vec_float4 depthRange1 = (vec_float4)spu_shuffle(scissorArea, scissorArea, s_DDDD);

	const vec_uint4 flipDepthRange = spu_cmpgt(depthRange0, depthRange1);
	const qword frustumMaxZz = (qword)spu_sel(depthRange1, depthRange0, flipDepthRange);
	const qword frustumMinZz = (qword)spu_sel(depthRange0, depthRange1, flipDepthRange);

	const qword frustumMax = si_shufb(scissorAreaF, frustumMaxZz, (qword)s_CDa0);
	const qword frustumMin = si_shufb(scissorAreaF, frustumMinZz, (qword)s_ABa0);

	const qword m_00FF = si_fsmbi(0x00FF);
	const qword frustumMinX = si_shufb(frustumMin, frustumMin, (qword)s_AAAA);
	const qword frustumMinY = si_shufb(frustumMin, frustumMin, (qword)s_BBBB);
	const qword frustumMinZ = si_shufb(frustumMin, frustumMin, (qword)s_CCCC);
	const qword frustumMaxX = si_shufb(frustumMax, frustumMax, (qword)s_AAAA);
	const qword frustumMaxY = si_shufb(frustumMax, frustumMax, (qword)s_BBBB);
	const qword frustumMaxZ = si_shufb(frustumMax, frustumMax, (qword)s_CCCC);
	const qword s_AaCcEeGg = (qword){0x00,0x01,0x10,0x11, 0x04,0x05,0x14,0x15, 0x08,0x09,0x18,0x19, 0x0C,0x0D,0x1C,0x1D};
	const qword s_ABa0CDc0 = (qword){0x00,0x01,0x02,0x03, 0x10,0x11,0x80,0x80, 0x04,0x05,0x06,0x07, 0x14,0x15,0x80,0x80};
	const qword s_EFe0GHg0 = si_orbi(s_ABa0CDc0, 0x8);//(qword){0x08,0x09,0x0A,0x0B, 0x18,0x19,0x80,0x80, 0x0C,0x0D,0x0E,0x0F, 0x1C,0x1D,0x80,0x80};
	const qword s_ABab = si_selb((qword)s_ABcd, (qword)s_BCab, m_00FF);
	const qword s_CcDd = si_rotqbyi((qword)s_DdCc, 8);
	const qword s_CDcd = si_orbi((qword)s_ABab, 0x08);
	const qword s_CDab = si_xorbi((qword)s_ABcd, 0x08);
	

	//load and distribute local to world and view projection matrices
	vec_char16 pLocalToWorldMatrix = (vec_char16)si_from_ptr(&(ctx->localToWorldMatrix.matrixData));
	vec_float4 localToWorld0 = (vec_float4)si_lqd(pLocalToWorldMatrix, 0x00);
	vec_float4 localToWorld1 = (vec_float4)si_lqd(pLocalToWorldMatrix, 0x10);
	vec_float4 localToWorld2 = (vec_float4)si_lqd(pLocalToWorldMatrix, 0x20);
	vec_float4 localToWorld3 =  {0.0f, 0.0f, 0.0f, 1.0f};

	vec_char16 pViewProjectionMatrix = (vec_char16)si_from_ptr(ctx->viewportInfo.viewProjectionMatrix);
	vec_float4 viewProjMat0 = (vec_float4)si_lqd(pViewProjectionMatrix, 0x00);
	vec_float4 viewProjMat1 = (vec_float4)si_lqd(pViewProjectionMatrix, 0x10);
	vec_float4 viewProjMat2 = (vec_float4)si_lqd(pViewProjectionMatrix, 0x20);
	vec_float4 viewProjMat3 = (vec_float4)si_lqd(pViewProjectionMatrix, 0x30);

	//compute LocalToProj matrix from LocalToWorld and ViewportProjection matrices
	vec_float4 vp00 = spu_shuffle(viewProjMat0, viewProjMat0, s_AAAA);
	vec_float4 vp10 = spu_shuffle(viewProjMat1, viewProjMat1, s_AAAA);
	vec_float4 vp20 = spu_shuffle(viewProjMat2, viewProjMat2, s_AAAA);
	vec_float4 vp30 = spu_shuffle(viewProjMat3, viewProjMat3, s_AAAA);

	vec_float4 localToProjContrib00 = spu_mul(vp00, localToWorld0);
	vec_float4 localToProjContrib10 = spu_mul(vp10, localToWorld0);
	vec_float4 localToProjContrib20 = spu_mul(vp20, localToWorld0);
	vec_float4 localToProjContrib30 = spu_mul(vp30, localToWorld0);

	vec_float4 vp01 = spu_shuffle(viewProjMat0, viewProjMat0, s_BBBB);
	vec_float4 vp11 = spu_shuffle(viewProjMat1, viewProjMat1, s_BBBB);
	vec_float4 vp21 = spu_shuffle(viewProjMat2, viewProjMat2, s_BBBB);
	vec_float4 vp31 = spu_shuffle(viewProjMat3, viewProjMat3, s_BBBB);

	vec_float4 localToProjContrib01 = spu_madd(vp01, localToWorld1, localToProjContrib00);
	vec_float4 localToProjContrib11 = spu_madd(vp11, localToWorld1, localToProjContrib10);
	vec_float4 localToProjContrib21 = spu_madd(vp21, localToWorld1, localToProjContrib20);
	vec_float4 localToProjContrib31 = spu_madd(vp31, localToWorld1, localToProjContrib30);

	vec_float4 vp02 = spu_shuffle(viewProjMat0, viewProjMat0, s_CCCC);
	vec_float4 vp12 = spu_shuffle(viewProjMat1, viewProjMat1, s_CCCC);
	vec_float4 vp22 = spu_shuffle(viewProjMat2, viewProjMat2, s_CCCC);
	vec_float4 vp32 = spu_shuffle(viewProjMat3, viewProjMat3, s_CCCC);

	vec_float4 localToProjContrib02 = spu_madd(vp02, localToWorld2, localToProjContrib01);
	vec_float4 localToProjContrib12 = spu_madd(vp12, localToWorld2, localToProjContrib11);
	vec_float4 localToProjContrib22 = spu_madd(vp22, localToWorld2, localToProjContrib21);
	vec_float4 localToProjContrib32 = spu_madd(vp32, localToWorld2, localToProjContrib31);

	vec_float4 vp03 = spu_shuffle(viewProjMat0, viewProjMat0, s_DDDD);
	vec_float4 vp13 = spu_shuffle(viewProjMat1, viewProjMat1, s_DDDD);
	vec_float4 vp23 = spu_shuffle(viewProjMat2, viewProjMat2, s_DDDD);
	vec_float4 vp33 = spu_shuffle(viewProjMat3, viewProjMat3, s_DDDD);

	vec_float4 localToProj0 = spu_madd(vp03, localToWorld3, localToProjContrib02);
	vec_float4 localToProj1 = spu_madd(vp13, localToWorld3, localToProjContrib12);
	vec_float4 localToProj2 = spu_madd(vp23, localToWorld3, localToProjContrib22);
	vec_float4 localToProj3 = spu_madd(vp33, localToWorld3, localToProjContrib32);

	//distribute localToProj
	qword localToProj00 = (qword)spu_shuffle(localToProj0, localToProj0, s_AAAA);
	qword localToProj10 = (qword)spu_shuffle(localToProj1, localToProj1, s_AAAA);
	qword localToProj20 = (qword)spu_shuffle(localToProj2, localToProj2, s_AAAA);
	qword localToProj30 = (qword)spu_shuffle(localToProj3, localToProj3, s_AAAA);

	qword localToProj01 = (qword)spu_shuffle(localToProj0, localToProj0, s_BBBB);
	qword localToProj11 = (qword)spu_shuffle(localToProj1, localToProj1, s_BBBB);
	qword localToProj21 = (qword)spu_shuffle(localToProj2, localToProj2, s_BBBB);
	qword localToProj31 = (qword)spu_shuffle(localToProj3, localToProj3, s_BBBB);

	qword localToProj02 = (qword)spu_shuffle(localToProj0, localToProj0, s_CCCC);
	qword localToProj12 = (qword)spu_shuffle(localToProj1, localToProj1, s_CCCC);
	qword localToProj22 = (qword)spu_shuffle(localToProj2, localToProj2, s_CCCC);
	qword localToProj32 = (qword)spu_shuffle(localToProj3, localToProj3, s_CCCC);

	qword localToProj03 = (qword)spu_shuffle(localToProj0, localToProj0, s_DDDD);
	qword localToProj13 = (qword)spu_shuffle(localToProj1, localToProj1, s_DDDD);
	qword localToProj23 = (qword)spu_shuffle(localToProj2, localToProj2, s_DDDD);
	qword localToProj33 = (qword)spu_shuffle(localToProj3, localToProj3, s_DDDD);

	unsigned int numIterations = (ctx->spuConfigInfo.numVertexes + 3) / 4;

	int adjustVertexPointer = numIterations * 0x40; // each iteration processes 64 bytes
	adjustVertexPointer += 2*0x40; // We run two extra iterations of the loop before we hit useful data
	int negativeAdjustVertexPointer = -adjustVertexPointer;

    const qword s_000EFGHH00000000 = (qword)
    {
        0x80, 0x80, 0x80, 0x04,
        0x05, 0x06, 0x07, 0x07,
        0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80
    };

	qword outputOffset = (qword)spu_splats(0);
    qword outputOffsetInc = (qword)
	{ 
		0x00, 0x00, 0x00, 0x00,   
		0x00, 0x00, 0x40, 0x40, 
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

	vec_uint4 pTransVerts = (vec_uint4)si_from_ptr(ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF]);

    qword pTransVerts0 = (qword)pTransVerts;
    qword pTransVerts1 = si_ai(pTransVerts0, 0x10);
    qword pTransVerts2 = si_ai(pTransVerts0, 0x20);
    qword pTransVerts3 = si_ai(pTransVerts0, 0x30);


	qword pVerts0 = (qword)spu_add((vec_uint4)si_from_ptr(ctx->positionTable), adjustVertexPointer);
	qword pVerts1 = si_ai(pVerts0, 0x10);
	qword pVerts2 = si_ai(pVerts0, 0x20);
	qword pVerts3 = si_ai(pVerts0, 0x30);

	qword inputOffset = si_from_int(negativeAdjustVertexPointer);

	const qword signMask = (qword)spu_splats(0x80000000);
	
	// predeclare variables
	qword xxxxProj = fOne, yyyyProj = fOne, zzzzProj = fOne, wwwwProj = fOne;
	qword wwwwSafe = fOne, wwwwRecipTmp0 = fOne;
	qword xxxxScreen = fOne, yyyyScreen = fOne, zzzzScreen = fOne;

	/*  Loop iteration delimited by tabbing
		For example:
		1.  First iteration
			2.  Second Iteration
				3.  Third Iteration
	*/

	// Each output qword contains the following data:
	// word 0: screen-space X coordinate (floating-point)
	// word 1: screen-space Y coordinate (floating-point)
	// word 2: flags
	//         word2 & (1<<0): unused (always zero)
	//         word2 & (1<<1): wSafe   (set if the vertex's projected W value was > 0)
	//         word2 & (1<<2): Z Under (set if the vertex was below the negative Z plane)
	//         word2 & (1<<3): Z Over  (set if the vertex was above the positive Z plane)
	//         word2 & (1<<4): Y Under (set if the vertex was below the negative Y plane)
	//         word2 & (1<<5): Y Over  (set if the vertex was above the positive Y plane)
	//         word2 & (1<<6): X Under (set if the vertex was below the negative X plane)
	//         word2 & (1<<7): X Over  (set if the vertex was above the positive X plane)
	// word3: unused (always zero)
	do
	{
		// Load four input verts (must happen before the stores)
		const qword xyz0 = si_lqx(pVerts0, inputOffset);
		const qword xyz1 = si_lqx(pVerts1, inputOffset);
		const qword xyz2 = si_lqx(pVerts2, inputOffset);
		const qword xyz3 = si_lqx(pVerts3, inputOffset);
		inputOffset = si_ai(inputOffset, 0x40);

        		// Perform frustum tests
        		qword xxxxOver  = si_fcgt(xxxxScreen, frustumMaxX);
        		qword yyyyOver  = si_fcgt(yyyyScreen, frustumMaxY);
        		qword zzzzOver  = si_fcgt(zzzzScreen, frustumMaxZ);
        		qword xxxxUnder = si_fcgt(frustumMinX, xxxxScreen);
        		qword yyyyUnder = si_fcgt(frustumMinY, yyyyScreen);
        		qword zzzzUnder = si_fcgt(frustumMinZ, zzzzScreen);

        		// Transpose frustum test results and wSafe back into mixed format,
				// and convert to a bitfield in the preferred word.
        		qword xxxxOverUnder = si_shufb(xxxxOver, xxxxUnder, s_AaCcEeGg);
        		qword yyyyOverUnder = si_shufb(yyyyOver, yyyyUnder, s_AaCcEeGg);
        		qword zzzzOverUnder = si_shufb(zzzzOver, zzzzUnder, s_AaCcEeGg);
        		qword xyxyOverUnder01 = si_shufb(xxxxOverUnder, yyyyOverUnder, (qword)s_AaBb);
        		qword xyxyOverUnder23 = si_shufb(xxxxOverUnder, yyyyOverUnder, (qword)s_CcDd);
        		qword zwzwOverUnderSafe01 = si_shufb(zzzzOverUnder, wwwwSafe, s_ABa0CDc0);
        		qword zwzwOverUnderSafe23 = si_shufb(zzzzOverUnder, wwwwSafe, s_EFe0GHg0);
        		qword flagBytes0 = si_shufb(xyxyOverUnder01, zwzwOverUnderSafe01, (qword)s_ABab);
        		qword flagBytes1 = si_shufb(xyxyOverUnder01, zwzwOverUnderSafe01, (qword)s_CDcd);
        		qword flagBytes2 = si_shufb(xyxyOverUnder23, zwzwOverUnderSafe23, (qword)s_ABab);
        		qword flagBytes3 = si_shufb(xyxyOverUnder23, zwzwOverUnderSafe23, (qword)s_CDcd);
        		qword flagBits0  = si_gbh(flagBytes0);
        		qword flagBits1  = si_gbh(flagBytes1);
        		qword flagBits2  = si_gbh(flagBytes2);
        		qword flagBits3  = si_gbh(flagBytes3);

				// Build final output qwords
        		qword outTmp0 = si_shufb(xxxxScreen, yyyyScreen, (qword)s_AaBb);
        		qword outTmp1 = si_shufb(xxxxScreen, yyyyScreen, (qword)s_CcDd);
        		qword out0 = si_shufb(outTmp0, flagBits0, (qword)s_ABab);
        		qword out1 = si_shufb(outTmp0, flagBits1, (qword)s_CDab);
        		qword out2 = si_shufb(outTmp1, flagBits2, (qword)s_ABab);
        		qword out3 = si_shufb(outTmp1, flagBits3, (qword)s_CDab);

				// Store output.  The increment/shuffle gambit is to make sure we
				// don't startg advancing the output pointer until the third loop
				// iteration, when we have valid results to store.
        		outputOffset = si_a(outputOffset, outputOffsetInc);
        		outputOffsetInc = si_shufb(outputOffsetInc, outputOffsetInc, (qword)s_000EFGHH00000000);
        		si_stqx(out0, pTransVerts0, outputOffset);
        		si_stqx(out1, pTransVerts1, outputOffset);
        		si_stqx(out2, pTransVerts2, outputOffset);
        		si_stqx(out3, pTransVerts3, outputOffset);


		    // If the projection-space W is less than zero, we need to ignore the backface
			// culling test for any triangles using this vertex.  We'll store this bit
		    wwwwSafe = si_fcgt(wwwwProj, floatZero);

			// Finish computing 1/w, using Newton-Raphson for higher precision
		    qword wwwwRecipTmp1 = si_fi(wwwwProj, wwwwRecipTmp0);
    		qword wwwwRecipTmp2 = si_fnms(wwwwRecipTmp1, wwwwProj, fOne);
    		qword wwwwRecip = si_fma(wwwwRecipTmp2, wwwwRecipTmp1, wwwwRecipTmp1);

			// Perform the perspective divide
    		qword xxxxUnit = si_fm(xxxxProj, wwwwRecip);
    		qword yyyyUnit = si_fm(yyyyProj, wwwwRecip);
    		qword zzzzUnit = si_fm(zzzzProj, wwwwRecip);

			// If W is negative, we need to invert the projection-space X/Y/Z values
    		qword wwwwSign = si_and(wwwwProj, signMask);
    		qword xxxxFlipped = si_xor(xxxxUnit, wwwwSign);
    		qword yyyyFlipped = si_xor(yyyyUnit, wwwwSign);
    		qword zzzzFlipped = si_xor(zzzzUnit, wwwwSign);

			// Scale and bias X/Y/Z into screen-space
    		xxxxScreen = si_fma(xxxxFlipped, viewportScaleX, viewportOffsetX);
    		yyyyScreen = si_fma(yyyyFlipped, viewportScaleY, viewportOffsetY);
    		zzzzScreen = si_fma(zzzzFlipped, viewportScaleZ, viewportOffsetZ);


		// Transpose input vertices into channel format
		qword xposeTemp0 = si_shufb(xyz0, xyz1, (qword)s_AaBb);
		qword xposeTemp1 = si_shufb(xyz2, xyz3, (qword)s_BbAa);
		qword xposeTemp2 = si_shufb(xyz0, xyz1, (qword)s_CcCc);
		qword xposeTemp3 = si_shufb(xyz2, xyz3, (qword)s_CcCc);
		qword xxxxIn     = si_selb( xposeTemp0, xposeTemp1, m_00FF);
		qword yyyyIn     = si_shufb(xposeTemp1, xposeTemp0, (qword)s_cdAB);
		qword zzzzIn     = si_selb( xposeTemp2, xposeTemp3, m_00FF);
		
		// Transform input vertices into projection-space
		qword xformTmp0_0 = si_fma(xxxxIn, localToProj00, localToProj03);
		qword xformTmp0_1 = si_fma(xxxxIn, localToProj10, localToProj13);
		qword xformTmp0_2 = si_fma(xxxxIn, localToProj20, localToProj23);
		qword xformTmp0_3 = si_fma(xxxxIn, localToProj30, localToProj33);
		qword xformTmp1_0 = si_fma(yyyyIn, localToProj01, xformTmp0_0);
		qword xformTmp1_1 = si_fma(yyyyIn, localToProj11, xformTmp0_1);
		qword xformTmp1_2 = si_fma(yyyyIn, localToProj21, xformTmp0_2);
		qword xformTmp1_3 = si_fma(yyyyIn, localToProj31, xformTmp0_3);
		xxxxProj    = si_fma(zzzzIn, localToProj02, xformTmp1_0);
		yyyyProj    = si_fma(zzzzIn, localToProj12, xformTmp1_1);
		zzzzProj    = si_fma(zzzzIn, localToProj22, xformTmp1_2);
		wwwwProj    = si_fma(zzzzIn, localToProj32, xformTmp1_3);

		// Begin computing 1/w (we finish this computation in the 2nd iteration)
		wwwwRecipTmp0 = si_frest(wwwwProj);

	} while (si_to_int(inputOffset) != 0);
}
#endif

/**
 * 	Culls triangles.  Performs offscreen, front/back-face(optional),
 * 	and no-pixel (separate for 4xRGMS) triangle tests.  Does not cull
 * 	triangles which are on screen and cross near plane.
 * 
 *  Operates on 4 triangles/12 indices at a time.
 * 
 *  Offscreen test uses bitwise-and of flags preprocessed in transform loop, 
 * 	stored in Z component of each vertex (see above).  Near plane test also uses
 *  preprocess test results.
 * 
 *  Cross product is used for back/front-face tests (to get direction
 *  of normal) and zero-area tests.  Backface test is not used if w =
 *  0.
 * 
 *  4XRGMS pixel center test (optional) computes tight triangle
 *  bounding box, then quantizes min/max corners and computes
 *  quantized edge lengths.  Min X % 4 and Min Y % 4 are combined into
 *  4-bit index into lookup table of max-edge length to hit
 *  pixel-center.  Values in table are compared against quantized BB
 *  edge lengths to determine if a pixel is hit.  This table will not
 *  cull 100% of no-pixel triangles, but gets very many and is fast.
 * 
 *  Test results are combined to determine which of the 4 triangles
 *  will be culled.  The resulting bit pattern is used as an index
 *  into tables of shuffle masks used to construct new index list.
 * 
 *  @param ctx EdgeGeom job context
 *  @param cullingFlavor specifies none, front face, back face, ff with RGMS, or bf with RGMS
 *  @return the number of visible indices
 */

#if EDGEGEOMCULLTRIANGLES_INTRINSICS
uint32_t edgeGeomCullTriangles(EdgeGeomSpuContext *ctx, uint32_t cullingFlavor, int32_t indexBias,
	EdgeGeomCullingResults *detailedResults)
{
	(void)detailedResults; // Only used in C implementation; see edgegeom_cull_c.cpp

	if(cullingFlavor == EDGE_GEOM_CULL_NONE)
	{
		// If the indexesOffset isn't -1, then the index buffer should
		// be considered "constant" (we can't modify or output it),
		// and we can reclaim that space for other calculations.
		if (ctx->spuConfigInfo.indexesOffset != 0xFFFFFFFF) {
			edgeGeomSetFreePtr(ctx, (void*)ctx->indexesLs);
		}
		return ctx->numVisibleIndexes;
	}

	// Culling uses a spare uniform table for intermediate
	// calculations.  If the spare uniform table bit is not set in the
	// SpuConfigInfo flags, then the spare uniform table does not
	// exist!  This is an error!
	if ((ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_INCLUDES_EXTRA_UNIFORM_TABLE) == 0)
	{
		EDGE_PRINTF("ERROR: attempt to use culling without allocating an extra uniform table!\n");
		return ctx->numVisibleIndexes;
	}

    if (ctx->transformForCullCallbackInfo.transformCallback) {
        ctx->transformForCullCallbackInfo.transformCallback(ctx, ctx->transformForCullCallbackInfo.transformCallbackUserData);
    }
    else{
        transformVertexesForCull(ctx);
    }
        
	const qword s_00Cc = si_rotqbyi((qword)s_Cc00, 8);
#if ENABLE_4XRGMS_CULLING
	const qword s_00Aa = si_rotqbyi((qword)s_Aa00, 8);
	const vec_uchar16 s_BbAa = (vec_uchar16)si_rotqbyi((qword)s_AaBb, 8);
	const vec_uchar16 s_CDab = (vec_uchar16)si_xorbi((qword)s_cdAB, 0x10);
#endif // ENABLE_4XRGMS_CULLING

	const vec_uchar16 s_ABcd = (vec_uchar16)si_rotqbyi((qword)s_cdAB, 8);
	const qword s_AAAA = si_ila(0x10203);
	const qword s_DDDDDDDDDDDDDDDD = (qword)spu_splats(0x03030303);

	const qword negativeOne = (qword)spu_splats(0xffffffff);
	const qword fZero = (qword)spu_splats(0.0f);

	const uint8_t vpSampleFlavor = ctx->viewportInfo.sampleFlavor;
	const bool useRgms = vpSampleFlavor == CELL_GCM_SURFACE_SQUARE_ROTATED_4;

	const qword nearMask = si_il(1 << 2); // masks out "Z Under" test from culling flags

	const unsigned int cullType = cullingFlavor | (useRgms ? (0x1 << 31) : 0);

	qword flipBit = (qword)spu_splats(0x80000000);
	qword useFace = (qword)spu_splats(0);
	
    float *pVertices = (float*)ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF];

    pVertices = (float *)((uint32_t)pVertices + indexBias * sizeof(qword));
    
    const qword pVertexes = si_from_ptr(pVertices);

	const unsigned int numTriangles = ctx->numVisibleIndexes / 3;
	unsigned int iterCount = ((numTriangles + 3) >> 2);
	qword iterCountBC = si_shufb(si_from_uint(iterCount), si_from_uint(iterCount), (qword)s_AAAA);

	const int modTriangles = numTriangles & 0x3;

	const qword inLoopMaskBase = si_from_uint(0x0);
	const qword lastLoopMask0Base = si_from_int((modTriangles == 0) ? 0 : (0xf >> modTriangles));

	const qword pIndexStream0 = si_from_uint(ctx->indexesLs);
	const qword pIndexStream1 = si_ai(pIndexStream0, 0x10);
   
	qword pOut = si_shufb(pIndexStream0, pIndexStream0, (qword)s_AAAA);

	qword pIndexStreamOffset = (qword)spu_splats(0);
	qword pIndexStreamOffsetInc = si_from_int(0x18);

	qword localNumVisibleIndexes = (qword)spu_splats(0x0);

	const qword pMasks00 = si_from_ptr(cullMaskTable00);
	const qword pMasks01 = si_from_ptr(cullMaskTable01);

	const qword pPrevMask = si_from_ptr(cullPreviousTable);
	qword prevIndex = (qword)spu_splats(0);
	qword prevRotAmount = (qword)spu_splats(0);

	qword numBytes = (qword)spu_splats(0);
	qword prevIndexTmp = (qword)spu_splats(0);

	qword dummyBuffer[3];
	const qword pDummy = si_shufb(si_from_ptr(dummyBuffer), (qword)s_AAAA, (qword)s_AAAA);
	const qword endOfBuffer = si_a(pOut, si_shufb(si_from_int( (numTriangles*6 + 0xF) & ~0xF ), (qword)s_AAAA, (qword)s_AAAA));
	const qword endOfBufferOffset = (qword)(vec_int4){-17, -33, 0, 0};
	const qword endOfBuffer0 = si_a(endOfBuffer, endOfBufferOffset);
#if ENABLE_4XRGMS_CULLING
	const qword endOfBuffer1 = si_shlqbyi(endOfBuffer0, 4);
#endif // ENABLE_4XRGMS_CULLING
	switch (cullType) 
	{
	case EDGE_GEOM_CULL_BACKFACES_AND_FRUSTUM: //BF and Frustum
		flipBit = si_xor(flipBit,(qword)spu_splats(0x80000000));
		/* intentional fall-through */
	case EDGE_GEOM_CULL_FRONTFACES_AND_FRUSTUM: //FF and Frustum
		useFace = (qword)spu_splats(-1);
		/* intentional fall-through */
	case EDGE_GEOM_CULL_FRUSTUM:
		{
			// check for CW winding order
			qword indexesFlavor = si_from_uchar((ctx->spuConfigInfo.indexesFlavorAndSkinningFlavor >> 4) & 0xF);
			qword reverseWindingOrder = si_or(si_ceq(indexesFlavor, si_from_int(EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CW)), 
													 si_ceq(indexesFlavor, si_from_int(EDGE_GEOM_INDEXES_COMPRESSED_TRIANGLE_LIST_CW)));
			qword reverseWindingOrderBC = si_and(si_shufb(reverseWindingOrder, reverseWindingOrder, s_DDDDDDDDDDDDDDDD),
													  (qword)spu_splats(0x80000000));
			flipBit = si_xor(flipBit, reverseWindingOrderBC);

			/*  Load triangle indexes */
			qword inputQuad0 = si_lqx(pIndexStream0, pIndexStreamOffset);
			qword inputQuad1 = si_lqx(pIndexStream1, pIndexStreamOffset);

			qword indexQuad0 = (qword)spu_splats(0);
			qword indexQuad1 = (qword)spu_splats(0);

			qword s_input = (qword)s_ABCD;
			const qword s_inputMask = {	 0x08, 0x08, 0x08, 0x08,
											0x08, 0x08, 0x08, 0x08,
											0x18, 0x18, 0x18, 0x18,
											0x18, 0x18, 0x18, 0x18};
			qword firstLoopMask =   {   0x0, 0x0, 0x0, 0xf,
										0x0, 0x0, 0x0, 0xf,
										0x0, 0x0, 0x0, 0x0,
										0x0, 0x0, 0x0, 0x0};

			iterCountBC = si_ai(iterCountBC, 2);

			qword cullBits0Tmp1 = (qword)spu_splats(0xf);
			qword doesNotIntersectNear0 = (qword)spu_splats(0);
			qword wSafe0Bit = (qword)spu_splats(0);
			qword cullFrustum0 = (qword)spu_splats(0);

			qword indexQuad0Copy1 = (qword)spu_splats(0);
			qword indexQuad1Copy1 = (qword)spu_splats(0);
			qword indexQuad0Copy0 = (qword)spu_splats(0);
			qword indexQuad1Copy0 = (qword)spu_splats(0);

			qword cullFrustum0Tmp1 = (qword)spu_splats(0);
			qword crossesNear0 = (qword)spu_splats(0);

			qword cullBits0Tmp0 = (qword)spu_splats(0);

			qword xxxx0 = si_il(0);
			qword xxxx1 = si_il(0);
			qword xxxx2 = si_il(0);
			qword yyyy0 = si_il(0);
			qword yyyy1 = si_il(0);
			qword yyyy2 = si_il(0);
			qword xxxx1_0  = si_il(0);
			qword xxxx2_0  = si_il(0);
			qword yyyy1_0  = si_il(0);
			qword yyyy2_0  = si_il(0);
			qword s_CDcd = si_orbi((qword)s_ABab, 0x8);
			
			const qword pixelCenterBias = (qword)spu_splats(0.5f);
			qword pixelCenterScale      = (qword)spu_splats(1.0f);
			switch(ctx->viewportInfo.sampleFlavor)
			{
			case CELL_GCM_SURFACE_CENTER_1:
				pixelCenterScale = (qword)spu_splats(1.0f);
				break;
			case CELL_GCM_SURFACE_DIAGONAL_CENTERED_2:	
			case CELL_GCM_SURFACE_SQUARE_CENTERED_4:
				pixelCenterScale = (qword)spu_splats(2.0f);
				break;
			case CELL_GCM_SURFACE_SQUARE_ROTATED_4:
				// shouldn't get here!
				break;
			}
			do 
			{
				pIndexStreamOffset = si_a(pIndexStreamOffset, pIndexStreamOffsetInc);

				indexQuad0Copy1 = indexQuad0Copy0;
				indexQuad1Copy1 = indexQuad1Copy0;

				indexQuad0Copy0 = indexQuad0;
				indexQuad1Copy0 = indexQuad1;

				indexQuad0 = si_shufb(inputQuad0, inputQuad1, s_input);
				indexQuad1 = si_shufb(inputQuad1, inputQuad1, s_input);

						cullBits0Tmp1 = si_or(firstLoopMask, cullBits0Tmp0);
		
                        firstLoopMask = si_shlqbyi(firstLoopMask, 4);

						qword cullBits0 = si_shli(cullBits0Tmp1,4);
		
						qword s_mask00 = si_lqx(pMasks00, cullBits0);
						qword s_mask01 = si_lqx(pMasks01, cullBits0);
		
						qword keptIndexes0 = si_shufb(indexQuad0Copy1, indexQuad1Copy1, s_mask00);
						qword keptIndexes1 = si_shufb(indexQuad1Copy1, indexQuad1Copy1, s_mask01);
		
						qword bitCount0 = si_shufb(si_cntb(cullBits0), (qword)s_AAAA, (qword)s_AAAA);
		
						/*  now combine w/ previous */
						qword s_prevBase = si_lqx(pPrevMask, prevIndex);
						qword sel_prev = si_rotqmby(negativeOne, prevRotAmount);
		
						qword s_prev = si_selb((qword)s_ABCD, s_prevBase, sel_prev);
		
						pOut = si_a(pOut, numBytes);
		
						qword prevOut = si_lqd(pOut, 0x00);
		
						qword out0 = si_shufb(prevOut, keptIndexes0, s_prev);
						qword out1 = si_shufb(keptIndexes0, keptIndexes1, s_prevBase);
						qword out2 = si_shufb(keptIndexes1, keptIndexes1, s_prevBase);
		
						/*  Load triangle indexes */
						inputQuad0 = si_lqx(pIndexStream0, pIndexStreamOffset);
						inputQuad1 = si_lqx(pIndexStream1, pIndexStreamOffset);
		
						qword pSafeOut2 = si_selb(pOut, pDummy, si_cgt(pOut, endOfBuffer0));
						qword pSafeOut3 = si_rotqbyi(pSafeOut2, 4);

						si_stqd(out0, pOut, 0x00);
						si_stqd(out1, pSafeOut2, 0x10);
						si_stqd(out2, pSafeOut3, 0x20);
		
						qword numBytesTmp = si_sfi(bitCount0, 4);
						numBytes = si_mpyui(numBytesTmp, 6);
		
						localNumVisibleIndexes = si_a(localNumVisibleIndexes, numBytesTmp);
		
						prevIndexTmp = si_andi(si_a(prevIndexTmp, numBytes), 0xf);
						prevRotAmount = si_sfi(prevIndexTmp, 0);
						prevIndex = si_shli(prevIndexTmp,4);

				    /* scale/bias X/Y qwords and truncate to integers */
					qword intX0  = si_cflts(si_fma(xxxx0, pixelCenterScale, pixelCenterBias), 0);
					qword intX1  = si_cflts(si_fma(xxxx1, pixelCenterScale, pixelCenterBias), 0);
					qword intX2  = si_cflts(si_fma(xxxx2, pixelCenterScale, pixelCenterBias), 0);
					qword intY0  = si_cflts(si_fma(yyyy0, pixelCenterScale, pixelCenterBias), 0);
					qword intY1  = si_cflts(si_fma(yyyy1, pixelCenterScale, pixelCenterBias), 0);
					qword intY2  = si_cflts(si_fma(yyyy2, pixelCenterScale, pixelCenterBias), 0);

					/* If all the Xs match and all the Ys match,
					   the triangle misses the pixel center. */
					qword isEqualX01 = si_ceq(intX0, intX1);
					qword isEqualX12 = si_ceq(intX1, intX2);
					qword isEqualX20 = si_ceq(intX2, intX0);
					qword isEqualY01 = si_ceq(intY0, intY1);
					qword isEqualY12 = si_ceq(intY1, intY2);
					qword isEqualY20 = si_ceq(intY2, intY0);
					qword noPixelTmp0 = si_or(
						si_and(isEqualX01, si_and(isEqualX12, isEqualX20)),
						si_and(isEqualY01, si_and(isEqualY12, isEqualY20))
						);

					qword crossTmp0 = si_fm(xxxx1_0, yyyy2_0);
					qword crossTmp1 = si_fm(yyyy1_0, xxxx2_0);
					qword cross0    = si_fs(crossTmp0, crossTmp1);

					cullFrustum0 = si_clgti(cullFrustum0Tmp1, 0);
					qword wSafe0Word = si_cgti(wSafe0Bit, 0);
					qword passesNearPlaneTest = si_and(doesNotIntersectNear0, wSafe0Word);

					qword cross0flipped = si_xor(cross0, flipBit);

					qword isBackFacing0 = si_fcgt(cross0flipped, fZero);

					qword noPixel = si_and(noPixelTmp0, passesNearPlaneTest);
					qword cullBackFace0 = si_and(isBackFacing0, passesNearPlaneTest);

					qword cull0Tmp0  = si_fnms(cullBackFace0, useFace, cullFrustum0);
					qword cull0Final = si_or(cull0Tmp0, noPixel);
					qword cull0 = si_gb(cull0Final);

					iterCountBC = si_ai(iterCountBC, -1);

					qword isLastLoop = si_ceqi(iterCountBC, 1);

					qword lastLoopMask0 = si_selb(inLoopMaskBase, lastLoopMask0Base, isLastLoop);

					cullBits0Tmp0 = si_or(lastLoopMask0, cull0);
					

				s_input = si_xor(s_input, s_inputMask);
				
				/*  Rotate into pointer offsets */
                qword i1 = si_roti(indexQuad0, 4);
				qword i3 = si_shlqbyi(i1, 4);
				qword i5 = si_shlqbyi(i1, 8);
				qword i7 = si_shlqbyi(i1, 12);
		
                qword i9 = si_roti(indexQuad1, 4);
				qword i11 = si_shlqbyi(i9, 4);
		
                qword i0 = si_roti(i1, 16);
                qword i2 = si_roti(i3, 16);
                qword i4 = si_roti(i5, 16);
				qword i6 = si_roti(i7, 16);
		
                qword i8 = si_roti(i9, 16);
				qword i10 = si_roti(i11, 16);

				/*  Load Vertexes */
				qword v0 = si_lqx(pVertexes, i0);
				qword v1 = si_lqx(pVertexes, i1);
				qword v2 = si_lqx(pVertexes, i2);
				qword v3 = si_lqx(pVertexes, i3);
				qword v4 = si_lqx(pVertexes, i4);
				qword v5 = si_lqx(pVertexes, i5);
				qword v6 = si_lqx(pVertexes, i6);
				qword v7 = si_lqx(pVertexes, i7);
				qword v8 = si_lqx(pVertexes, i8);
				qword v9 = si_lqx(pVertexes, i9);
				qword v10 = si_lqx(pVertexes,i10);
				qword v11 = si_lqx(pVertexes,i11);

				/* Pack X and Y values into separate qwords */
				qword x0x3y0y3    = si_shufb(v0, v3,  (qword)s_AaBb);
				qword x1x4y1y4    = si_shufb(v1, v4,  (qword)s_AaBb);
				qword x2x5y2y5    = si_shufb(v2, v5,  (qword)s_AaBb);
				qword x6x9y6y9    = si_shufb(v6, v9,  (qword)s_AaBb);
				qword x7x10y7y10  = si_shufb(v7, v10, (qword)s_AaBb);
				qword x8x11y8y11  = si_shufb(v8, v11, (qword)s_AaBb);
				xxxx0             = si_shufb(x0x3y0y3, x6x9y6y9,   (qword)s_ABab);
				xxxx1             = si_shufb(x1x4y1y4, x7x10y7y10, (qword)s_ABab);
				xxxx2             = si_shufb(x2x5y2y5, x8x11y8y11, (qword)s_ABab);
				yyyy0             = si_shufb(x0x3y0y3, x6x9y6y9,   (qword)s_CDcd);
				yyyy1             = si_shufb(x1x4y1y4, x7x10y7y10, (qword)s_CDcd);
				yyyy2             = si_shufb(x2x5y2y5, x8x11y8y11, (qword)s_CDcd);

				/*  Frustum Test */
				qword zzzz0Tmp0 = si_shufb(v0, v3, (qword)s_Cc00);
				qword zzzz0Tmp1 = si_shufb(v6, v9, s_00Cc);
				qword zzzz0 = si_shufb(zzzz0Tmp0, zzzz0Tmp1, (qword)s_ABcd);

				qword zzzz1Tmp0 = si_shufb(v1, v4, (qword)s_Cc00);
				qword zzzz1Tmp1 = si_shufb(v7, v10, s_00Cc);
				qword zzzz1 = si_shufb(zzzz1Tmp0, zzzz1Tmp1, (qword)s_ABcd);

				qword zzzz2Tmp0 = si_shufb(v2, v5, (qword)s_Cc00);
				qword zzzz2Tmp1 = si_shufb(v8, v11, s_00Cc);
				qword zzzz2 = si_shufb(zzzz2Tmp0, zzzz2Tmp1, (qword)s_ABcd);

				qword cullFlags0 = si_and(zzzz2, si_and(zzzz0, zzzz1));
				wSafe0Bit = si_andi(cullFlags0, (1<<1));
				cullFrustum0Tmp1 = si_andi(cullFlags0, 0xFC);
		
				crossesNear0 = si_or(zzzz2, si_or(zzzz0, zzzz1));
				doesNotIntersectNear0 = si_ceqi(si_and(crossesNear0, nearMask), 0);

				xxxx1_0 = si_fs(xxxx1, xxxx0);
				xxxx2_0 = si_fs(xxxx2, xxxx0);
				yyyy1_0 = si_fs(yyyy1, yyyy0);
				yyyy2_0 = si_fs(yyyy2, yyyy0);
			} while(si_to_uint(iterCountBC) != 0);
		}
		break;

#if ENABLE_4XRGMS_CULLING
	case EDGE_GEOM_CULL_BACKFACES_AND_FRUSTUM | (0x1 << 31): //BF and Frustum
		flipBit = si_xor(flipBit,(qword)spu_splats(0x80000000));
		/* intentional fall-through */
	case EDGE_GEOM_CULL_FRONTFACES_AND_FRUSTUM | (0x1 << 31): //FF and Frustum
		useFace = (qword)spu_splats(-1);
		/* intentional fall-through */
	case EDGE_GEOM_CULL_FRUSTUM | (0x1 << 31):
		{

			const qword rgmsX = {   1, 0, 3, 2,
									3, 1, 1, 0,
									0, 3, 2, 1,
									1, 1, 0, 2};

			const qword rgmsY = {   2, 0, 1, 1,
									1, 3, 2, 0,
									0, 1, 1, 3,
									3, 1, 0, 2};

			const qword sel_Rgms = (qword)spu_splats(0xC);
			const qword sel_RgmsShuf = si_fsmbi(0x1111);
			const qword s_0000 = (qword)spu_splats(0x80808080);

			// check for CW winding order
			qword indexesFlavor = si_from_uchar((ctx->spuConfigInfo.indexesFlavorAndSkinningFlavor >> 4) & 0xF);
			qword reverseWindingOrder = si_or(si_ceq(indexesFlavor, si_from_int(EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CW)), 
													 si_ceq(indexesFlavor, si_from_int(EDGE_GEOM_INDEXES_COMPRESSED_TRIANGLE_LIST_CW)));
			qword reverseWindingOrderBC = si_and(si_shufb(reverseWindingOrder, reverseWindingOrder, s_DDDDDDDDDDDDDDDD),
													  (qword)spu_splats(0x80000000));
			flipBit = si_xor(flipBit, reverseWindingOrderBC);
	
			/*  Load triangle indexes */
			qword inputQuad0 = si_lqx(pIndexStream0, pIndexStreamOffset);
			qword inputQuad1 = si_lqx(pIndexStream1, pIndexStreamOffset);
	
			qword indexQuad0 = (qword)spu_splats(0);
			qword indexQuad1 = (qword)spu_splats(0);
	
			qword s_input = (qword)s_ABCD;
			const qword s_inputMask = {	 0x08, 0x08, 0x08, 0x08,
											0x08, 0x08, 0x08, 0x08,
											0x18, 0x18, 0x18, 0x18,
											0x18, 0x18, 0x18, 0x18};
			qword firstLoopMask = { 0x0, 0x0, 0x0, 0xf,
									0x0, 0x0, 0x0, 0xf,
									0x0, 0x0, 0x0, 0x0,
									0x0, 0x0, 0x0, 0x0};

			iterCountBC = si_ai(iterCountBC, 2);

			qword indexQuad0Copy0 = (qword)spu_splats(0);
			qword indexQuad1Copy0 = (qword)spu_splats(0);

			qword e00 = (qword)spu_splats(0);
			qword e10 = (qword)spu_splats(0);
			qword e01 = (qword)spu_splats(0);
			qword e11 = (qword)spu_splats(0);
			qword e20 = (qword)spu_splats(0);
			qword e30 = (qword)spu_splats(0);
			qword e21 = (qword)spu_splats(0);
			qword e31 = (qword)spu_splats(0);
			qword doesNotIntersectNear0 = (qword)spu_splats(0);
			qword cullFrustum0 = (qword)spu_splats(0);

			qword v0 = (qword)spu_splats(0);
			qword v1 = (qword)spu_splats(0);
			qword v2 = (qword)spu_splats(0);
			qword v3 = (qword)spu_splats(0);
			qword v4 = (qword)spu_splats(0);
			qword v5 = (qword)spu_splats(0);
			qword v6 = (qword)spu_splats(0);
			qword v7 = (qword)spu_splats(0);
			qword v8 = (qword)spu_splats(0);
			qword v9 = (qword)spu_splats(0);
			qword v10 = (qword)spu_splats(0);
			qword v11 = (qword)spu_splats(0);

			qword cullFace0 = (qword)spu_splats(0);
			qword zeroAreaNoPixel0 = (qword)spu_splats(0);

			qword minY0 = (qword)spu_splats(0);
			qword maxY0 = (qword)spu_splats(0);
			qword minX0 = (qword)spu_splats(0);
			qword maxX0 = (qword)spu_splats(0);

			qword cullFace0Gathered = (qword)spu_splats(0);
			qword cullFrustum0Gathered = (qword)spu_splats(0);
			qword zeroAreaNoPixel0Gathered = (qword)spu_splats(0);
			qword inLine0Gathered = (qword)spu_splats(0);

			qword tmp41 = (qword)spu_splats(0);
			qword yyyy2 = (qword)spu_splats(0);
			qword yyyy0 = (qword)spu_splats(0);
			qword yyyy1 = (qword)spu_splats(0);
			qword cmp12 = (qword)spu_splats(0);
			qword tmp40 = (qword)spu_splats(0);
			qword cmp11 = (qword)spu_splats(0);
			qword tmp31 = (qword)spu_splats(0);
			qword xxxx2 = (qword)spu_splats(0);
			qword xxxx0 = (qword)spu_splats(0);
			qword xxxx1 = (qword)spu_splats(0);
			qword cmp02 = (qword)spu_splats(0);
			qword tmp30 = (qword)spu_splats(0);
			qword cmp01 = (qword)spu_splats(0);
			qword cmp10 = (qword)spu_splats(0);
			qword cmp00 = (qword)spu_splats(0);
			qword cullFrustum0Tmp0 = (qword)spu_splats(0);
			qword zzzz2 = (qword)spu_splats(0);
			qword wSafe0Bit = (qword)spu_splats(0);

			const qword sel_00FF = si_fsm(si_from_uint(0x3));

			do 
			{
				pIndexStreamOffset = si_a(pIndexStreamOffset, pIndexStreamOffsetInc);

						cullFrustum0Gathered = si_gb(cullFrustum0);
						inLine0Gathered = si_il(0); // ignored

						qword cull0GatheredTmp0 = si_shufb(cullFace0Gathered, cullFrustum0Gathered, (qword)s_Aa00);
						qword cull0GatheredTmp1 = si_shufb(zeroAreaNoPixel0Gathered, inLine0Gathered, s_00Aa);
						qword cull0Gathered = si_shufb(cull0GatheredTmp0, cull0GatheredTmp1, (qword)s_ABcd);
						qword cull0 = si_orx(cull0Gathered);
						
						
						iterCountBC = si_ai(iterCountBC, -1);
						
						qword isLastLoop = si_ceqi(iterCountBC, 0);
						
						qword lastLoopMask0 = si_selb(inLoopMaskBase, lastLoopMask0Base, isLastLoop);
						
						qword cullBits0Tmp0 = si_or(lastLoopMask0, cull0);
						qword cullBits0Tmp1 = si_or(firstLoopMask, cullBits0Tmp0);
						qword cullBits0 = si_shli(cullBits0Tmp1,4);
						firstLoopMask = si_shlqbyi(firstLoopMask, 4);
						
						qword s_mask00 = si_lqx(pMasks00, cullBits0);
						qword s_mask01 = si_lqx(pMasks01, cullBits0);
						
						qword keptIndexes0 = si_shufb(indexQuad0Copy0, indexQuad1Copy0, s_mask00);
						qword keptIndexes1 = si_shufb(indexQuad1Copy0, indexQuad1Copy0, s_mask01);

				indexQuad0Copy0 = indexQuad0;
				indexQuad1Copy0 = indexQuad1;

				indexQuad0 = si_shufb(inputQuad0, inputQuad1, s_input);
				indexQuad1 = si_shufb(inputQuad1, inputQuad1, s_input);

						//qword bitCount0 = si_shufb(si_cntb(cullBits0), (qword)s_AAAA, (qword)s_AAAA);
						qword bitCount0 = si_cntb(cullBits0);

						/*  now combine w/ previous */
						qword s_prevBase = si_lqx(pPrevMask, prevIndex);
						qword sel_prev = si_rotqmby(negativeOne, prevRotAmount);
						
						qword s_prev = si_selb((qword)s_ABCD, s_prevBase, sel_prev);
						
						pOut = si_a(pOut, numBytes);
						
						qword prevOut = si_lqd(pOut, 0x00);
						
						qword out0 = si_shufb(prevOut, keptIndexes0, s_prev);
						qword out1 = si_shufb(keptIndexes0, keptIndexes1, s_prevBase);
						qword out2 = si_shufb(keptIndexes1, keptIndexes1, s_prevBase);
						
						/*  Load triangle indexes */
						inputQuad0 = si_lqx(pIndexStream0, pIndexStreamOffset);
						inputQuad1 = si_lqx(pIndexStream1, pIndexStreamOffset);

						qword pSafeOut2 = si_selb(pOut, pDummy, si_cgt(pOut, endOfBuffer0));
						qword pSafeOut3 = si_selb(pOut, pDummy, si_cgt(pOut, endOfBuffer1));

						si_stqd(out0, pOut, 0x00);
						si_stqd(out1, pSafeOut2, 0x10);
						si_stqd(out2, pSafeOut3, 0x20);

						qword numBytesTmp = si_sfi(bitCount0, 4);
						numBytes = si_mpyui(numBytesTmp, 6);
						
						localNumVisibleIndexes = si_a(localNumVisibleIndexes, numBytesTmp);
						
						prevIndexTmp = si_andi(si_a(prevIndexTmp, numBytes), 0xf);
						prevRotAmount = si_sfi(prevIndexTmp, 0);
						prevIndex = si_shli(prevIndexTmp,4);

					tmp30 = si_selb(xxxx1, xxxx0, cmp00);
					tmp31 = si_selb(xxxx0, xxxx1, cmp00);

					tmp40 = si_selb(yyyy1, yyyy0, cmp10);
					tmp41 = si_selb(yyyy0, yyyy1, cmp10);

					cmp01 = si_fcgt(tmp30, xxxx2);
					cmp02 = si_fcgt(tmp31, xxxx2);
					cmp11 = si_fcgt(tmp40, yyyy2);
					cmp12 = si_fcgt(tmp41, yyyy2);

					minY0 = si_selb(tmp41, yyyy2, cmp12);
					maxY0 = si_selb(yyyy2, tmp40, cmp11);
					minX0 = si_selb(tmp31, xxxx2, cmp02);
					maxX0 = si_selb(xxxx2, tmp30, cmp01);
					
					qword minX0Quant = si_cflts(minX0, 2);
					qword minY0Quant = si_cflts(minY0, 2);
					
					qword maxX0Quant = si_cflts(maxX0, 2);
					qword maxY0Quant = si_cflts(maxY0, 2);
					
					qword dxQuant = si_sf(minX0Quant, maxX0Quant);
					qword dyQuant = si_sf(minY0Quant, maxY0Quant);
	
					qword yCoord = si_shlqbyi(minY0Quant, 2);
					
					qword s_Rgms = si_selb(s_0000, si_andi(si_selb(minX0Quant, yCoord, sel_Rgms), 0xf), sel_RgmsShuf);
					
					qword xValues = si_shufb(rgmsX, rgmsX, s_Rgms);
					qword yValues = si_shufb(rgmsY, rgmsY, s_Rgms);
					
					qword wSafe0Word = si_cgti(wSafe0Bit, 0);
					qword passesNearPlaneTest = si_and(doesNotIntersectNear0, wSafe0Word);

					qword xMiss = si_cgt(xValues, dxQuant);
					qword yMiss = si_cgt(yValues, dyQuant);
					qword noPixelTmp0 = si_and(xMiss, yMiss);
					qword noPixel = si_and(noPixelTmp0, passesNearPlaneTest);
					
					qword e01A = si_shufb(e00, e10, (qword)s_ABab);
					qword e01B = si_shufb(e01, e11, (qword)s_BAba);
					
					qword e23A = si_shufb(e20, e30, (qword)s_ABab);
					qword e23B = si_shufb(e21, e31, (qword)s_BAba);
					
					qword mul01 = si_fm(e01A, e01B);
					qword mul23 = si_fm(e23A, e23B);
					
					qword mul01shift = si_shlqbyi(mul01, 4);
					qword mul23shift = si_shlqbyi(mul23, 4);
					
					qword cross01 = si_fs(mul01, mul01shift);
					qword cross23 = si_fs(mul23, mul23shift);
					
					qword cross0 = si_shufb(cross01, cross23, (qword)s_ACac);
					
					qword zeroArea0 = si_il(0); // ignored
					
					zeroAreaNoPixel0 = si_or(zeroArea0, noPixel);
					
					qword cross0flipped = si_xor(cross0, flipBit);
					
					qword isBackFacing0 = si_fcgt(cross0flipped, fZero);
					
					qword cullBackFace0 = si_and(isBackFacing0, passesNearPlaneTest);
					
					cullFace0 = si_and(cullBackFace0, useFace);

					cullFrustum0 = si_clgti(cullFrustum0Tmp0, 0);
					
					zeroAreaNoPixel0Gathered = si_gb(zeroAreaNoPixel0);
					cullFace0Gathered = si_gb(cullFace0);

				s_input = si_xor(s_input, s_inputMask);
				
				/*  Rotate into pointer offsets */
                qword i1 = si_roti(indexQuad0, 4);
				qword i3 = si_shlqbyi(i1, 4);
				qword i5 = si_shlqbyi(i1, 8);
				qword i7 = si_shlqbyi(i1, 12);
		
                qword i9 = si_roti(indexQuad1, 4);
				qword i11 = si_shlqbyi(i9, 4);
		
                qword i0 = si_roti(i1, 16);
                qword i2 = si_roti(i3, 16);
                qword i4 = si_roti(i5, 16);
				qword i6 = si_roti(i7, 16);
		
                qword i8 = si_roti(i9, 16);
				qword i10 = si_roti(i11, 16);
				
				/*  Load Vertexes */
				v0 = si_lqx(pVertexes, i0);
				v1 = si_lqx(pVertexes, i1);
				v2 = si_lqx(pVertexes, i2);
				v3 = si_lqx(pVertexes, i3);
				v4 = si_lqx(pVertexes, i4);
				v5 = si_lqx(pVertexes, i5);
				v6 = si_lqx(pVertexes, i6);
				v7 = si_lqx(pVertexes, i7);
				v8 = si_lqx(pVertexes, i8);
				v9 = si_lqx(pVertexes, i9);
				v10 = si_lqx(pVertexes, i10);
				v11 = si_lqx(pVertexes, i11);
				
				qword zzzz0Tmp0 = si_shufb(v0, v3, (qword)s_Cc00);
				qword zzzz0Tmp1 = si_shufb(v6, v9, s_00Cc);
				qword zzzz0 = si_shufb(zzzz0Tmp0, zzzz0Tmp1, (qword)s_ABcd);
				
				qword zzzz1Tmp0 = si_shufb(v1, v4, (qword)s_Cc00);
				qword zzzz1Tmp1 = si_shufb(v7, v10, s_00Cc);
				qword zzzz1 = si_shufb(zzzz1Tmp0, zzzz1Tmp1, (qword)s_ABcd);
				
				qword zzzz2Tmp0 = si_shufb(v2, v5, (qword)s_Cc00);
				qword zzzz2Tmp1 = si_shufb(v8, v11, s_00Cc);
				zzzz2 = si_shufb(zzzz2Tmp0, zzzz2Tmp1, (qword)s_ABcd);
				
				qword cullFlags0 = si_and(zzzz2, si_and(zzzz0, zzzz1));
				wSafe0Bit = si_andi(cullFlags0, (1<<1));
				cullFrustum0Tmp0 = si_andi(cullFlags0, 0xFC);
				
				qword crossesNear0 = si_or(zzzz2, si_or(zzzz0, zzzz1));
				
				doesNotIntersectNear0 = si_ceqi(si_and(crossesNear0, nearMask), 0);
				
				qword tmp00 = si_shufb(v0, v3, (qword)s_AaBb);
				qword tmp01 = si_shufb(v6, v9, (qword)s_BbAa);
				xxxx0 = si_selb(tmp00, tmp01, sel_00FF);
				yyyy0 = si_shufb(tmp00, tmp01, (qword)s_CDab);
				
				qword tmp10 = si_shufb(v1, v4, (qword)s_AaBb);
				qword tmp11 = si_shufb(v7, v10, (qword)s_BbAa);
				xxxx1 = si_selb(tmp10, tmp11, sel_00FF);
				yyyy1 = si_shufb(tmp10, tmp11, (qword)s_CDab);
				
				qword tmp20 = si_shufb(v2, v5, (qword)s_AaBb);
				qword tmp21 = si_shufb(v8, v11, (qword)s_BbAa);
				xxxx2 = si_selb(tmp20, tmp21, sel_00FF);
				yyyy2 = si_shufb(tmp20, tmp21, (qword)s_CDab);
				
				cmp00 = si_fcgt(xxxx0, xxxx1);
				cmp10 = si_fcgt(yyyy0, yyyy1);

				e00 = si_fs(v1, v0);
				e01 = si_fs(v2, v0);
				
				e10 = si_fs(v4, v3);
				e11 = si_fs(v5, v3);
				
				e20 = si_fs(v7, v6);
				e21 = si_fs(v8, v6);
				
				e30 = si_fs(v10, v9);
				e31 = si_fs(v11, v9);
				
			} while(si_to_uint(iterCountBC) != 0);
		}
		break;
#else
	case EDGE_GEOM_CULL_BACKFACES_AND_FRUSTUM | (0x1 << 31): //BF and Frustum
	case EDGE_GEOM_CULL_FRONTFACES_AND_FRUSTUM | (0x1 << 31): //FF and Frustum
	case EDGE_GEOM_CULL_FRUSTUM | (0x1 << 31):
		EDGE_ASSERT_MSG(0, ("4x RGMS culling must be enabled in edgegeom_config.h"));
		break;
#endif // ENABLE_4XRGMS_CULLING
	}

	uint32_t toRet = si_to_uint(localNumVisibleIndexes) * 3;
	ctx->numVisibleIndexes = toRet;
	edgeGeomSetFreePtr(ctx, (void *)((  ((uint32_t)((uint8_t *)ctx->indexesLs + 2*toRet)) + 0xf) & ~0xf));
	return toRet;
}
#endif


