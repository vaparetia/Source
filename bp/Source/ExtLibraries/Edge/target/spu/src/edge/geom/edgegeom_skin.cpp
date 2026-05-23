/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"
#include "edge/geom/edgegeom_masks.h"


#if EDGEGEOMSKINVERTEXES_INTRINSICS
void edgeGeomSkinVertexes(EdgeGeomSpuContext *ctx, void *matrices, uint32_t matrixCount, void *indexesAndWeights)
{
	// Skinning is forbidden in the static geometry fast path!
	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
		return;

	const uint8_t skinningFlavor = ctx->spuConfigInfo.indexesFlavorAndSkinningFlavor & 0x0F;
	if(matrixCount == 0 || skinningFlavor == EDGE_GEOM_SKIN_NONE)
	{
		return;
	}

	// Skinning requires the extra uniform table, to store the final blended matrices for each vertex.
	if ((ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_INCLUDES_EXTRA_UNIFORM_TABLE) == 0)
	{
		EDGE_PRINTF("ERROR: attempt to use skinning without allocating an extra uniform table!\n");
		return;
	}

	const qword s_000I000K000M000O = si_ai((qword)s_000A000C000E000G, 8);
	const qword s_AAAA             = si_ila(0x10203);
	const qword s_BBBB             = si_orbi(s_AAAA, 0x04);
	const qword s_CCCC             = si_orbi(s_AAAA, 0x08);
	const qword s_DDDD             = si_orbi(s_AAAA, 0x0C);
	const qword s_CABD             = si_shufb((qword)s_BCAD, s_AAAA, (qword)s_BCAD);
	const qword k1_0f              = si_ilhu(0x3f80);
	const qword mask_FFF0          = si_fsmbi(0xFFF0);
	const qword mask_5555          = si_fsmbi(0x5555);
	
	/* The skinning matrices must be preprocessed before the vertexes
	 * are skinned.  Each matrix is modified in-place using an
	 * operation that's difficult to describe, but should be clear
	 * from this example:
	 *
	 * Original input matrix (48 bytes / 3 quadwords)
	 * A B C D
	 * E F G H
	 * I J K L
	 *
	 * Matrix after preprocessing:
	 * A F K D
	 * B G I H
	 * C E J L
	 *
	 * For 4x4 row-major matrices, the exact same operation is
	 * performed (we simply ignore the fourth row of each matrix). For
	 * 4x4 column-major matrices, we use a different sequence of
	 * shuffles, but the end result is the same.
	 */
	{
		const uint8_t matrixFormat = ctx->spuConfigInfo.skinningMatrixFormat;
		switch(matrixFormat)
		{
		case EDGE_GEOM_MATRIX_3x4_ROW_MAJOR:
		case EDGE_GEOM_MATRIX_4x4_ROW_MAJOR:
			{
				const qword mask_00F0 = si_fsmbi(0x00F0);
				const qword mask_0F00 = si_fsmbi(0x0F00);

				const uint32_t kNumRows = (matrixFormat == EDGE_GEOM_MATRIX_3x4_ROW_MAJOR) ? 3 : 4;
				const uint32_t kMatrixSize = kNumRows*0x10;
				
				const qword pOut1 = si_from_ptr((qword*)matrices + (kNumRows * matrixCount));
				const qword pOut2 = si_ai(pOut1, 0x10);
				const qword pOut3 = si_ai(pOut1, 0x20);
				//const qword pOut4 = si_ai(pOut1, 0x30);
				const qword pIn1  = si_a(pOut1, si_from_int(2*kMatrixSize));
				const qword pIn2  = si_ai(pIn1, 0x10);
				const qword pIn3  = si_ai(pIn1, 0x20);
				//const qword pIn4  = si_a(pIn1, 0x30);
				
				qword inOffset = si_from_int( - (kMatrixSize * matrixCount)); // incremented by kMatrixSize
				qword outOffset = inOffset; // incremented by 0x30

				qword in1 = si_lqx(pOut1, inOffset);
				qword in2 = si_lqx(pOut2, inOffset);
				qword in3 = si_lqx(pOut3, inOffset);
			
				qword m4 = si_selb(in2, in1, mask_00F0);
				qword m5 = si_selb(in3, in1, mask_0F00);
				qword m6 = si_selb(in1, in2, mask_0F00);
				qword out1  = si_selb(m6, in3, mask_00F0);
				qword out2  = si_shufb(in2, m5, (qword)s_bCaD);
				qword out3  = si_shufb(in3, m4, (qword)s_caBD);
			
				inOffset = si_a(inOffset, si_from_int(kMatrixSize));
				in1 = si_lqx(pOut1, inOffset);
				in2 = si_lqx(pOut2, inOffset);
				in3 = si_lqx(pOut3, inOffset);
				inOffset = si_a(inOffset, si_from_int(-kMatrixSize));
			
				qword m1, m2, m3;			
				do
				{
					m1  = in1;
					in1 = si_lqx(pIn1, inOffset);
					m2  = in2;
					in2 = si_lqx(pIn2, inOffset);
					m3  = in3;
					in3 = si_lqx(pIn3, inOffset);
				
					m4 = si_selb(m2, m1, mask_00F0);
					si_stqx(out1, pOut1, outOffset);
					m5 = si_selb(m3, m1, mask_0F00);
					si_stqx(out2, pOut2, outOffset);
					m6 = si_selb(m1, m2, mask_0F00);
					si_stqx(out3, pOut3, outOffset);

					inOffset = si_a(inOffset, si_from_int(kMatrixSize));
					out2 = si_shufb(m2, m5, (qword)s_bCaD);
					out1 = si_selb(m6, m3, mask_00F0);
					out3 = si_shufb(m3, m4, (qword)s_caBD);
					outOffset = si_ai(outOffset, 0x30);
				}
				while(si_to_int(inOffset) != 0);
				break;
			}
		case EDGE_GEOM_MATRIX_4x4_COLUMN_MAJOR:
			{
				const qword mask_0F00 = si_fsmbi(0x0F00);
				const qword mask_00FF = si_fsmbi(0x00FF);
				
				const qword s_CCCa = si_selb(si_orbi(s_AAAA, 0x10), s_CCCC, mask_FFF0);
				const qword s_CCCb = si_selb(si_orbi(s_AAAA, 0x14), s_CCCC, mask_FFF0);
				const qword s_CCCc = si_selb(si_orbi(s_AAAA, 0x18), s_CCCC, mask_FFF0);

				const uint32_t kNumRows = 4;
				const uint32_t kMatrixSize = kNumRows*0x10;
				
				const qword pOut1 = si_from_ptr((qword*)matrices + (kNumRows * matrixCount));
				const qword pOut2 = si_ai(pOut1, 0x10);
				const qword pOut3 = si_ai(pOut1, 0x20);
				const qword pOut4 = si_ai(pOut1, 0x30);
				const qword pIn1  = si_a(pOut1, si_from_int(1*kMatrixSize));
				const qword pIn2  = si_ai(pIn1, 0x10);
				const qword pIn3  = si_ai(pIn1, 0x20);
				const qword pIn4  = si_ai(pIn1, 0x30);
				
				qword inOffset = si_from_int( - (kMatrixSize * matrixCount)); // incremented by kMatrixSize
				qword outOffset = inOffset; // incremented by 0x30 per iteration
				
				qword in1 = si_lqx(pOut1, inOffset);
				qword in2 = si_lqx(pOut2, inOffset);
				qword in3 = si_lqx(pOut3, inOffset);
				qword in4 = si_lqx(pOut4, inOffset);
			
				qword y1 = si_shufb(in1, in4, s_CCCb);
				qword x1 = si_selb(in1, in2, mask_0F00);
				qword y2 = si_shufb(in2, in4, s_CCCc);
				qword x2 = si_selb(in2, in3, mask_0F00);
				qword y3 = si_shufb(in3, in4, s_CCCa);
				qword x3 = si_selb(in3, in1, mask_0F00);

				do
				{
					qword out2 = si_selb(x2, y1, mask_00FF);
					in1 = si_lqx(pIn1, inOffset);
					qword out3 = si_selb(x3, y2, mask_00FF);
					in2 = si_lqx(pIn2, inOffset);
					qword out1 = si_selb(x1, y3, mask_00FF);
					in3 = si_lqx(pIn3, inOffset);
					in4 = si_lqx(pIn4, inOffset);

					inOffset = si_ai(inOffset, kMatrixSize);
					si_stqx(out2, pOut2, outOffset);
					si_stqx(out3, pOut3, outOffset);
					si_stqx(out1, pOut1, outOffset);

					outOffset = si_ai(outOffset, 0x30);
					y1 = si_shufb(in1, in4, s_CCCb);
					x1 = si_selb(in1, in2, mask_0F00);
					y2 = si_shufb(in2, in4, s_CCCc);
					x2 = si_selb(in2, in3, mask_0F00);
					y3 = si_shufb(in3, in4, s_CCCa);
					x3 = si_selb(in3, in1, mask_0F00);
				}
				while(si_to_int(inOffset) != 0);
				break;
			}
		}
	}

	const qword pMatRow1 = si_from_ptr(matrices);
	const qword pMatRow2 = si_ai(pMatRow1, 0x10);
	const qword pMatRow3 = si_ai(pMatRow1, 0x20);

	const qword weightScale = (qword)spu_splats(1.0f / 255.0f);

	qword pIndexesAndWeights = si_from_ptr(indexesAndWeights);
	qword pPositions       = si_from_ptr(edgeGeomGetPositionUniformTable(ctx));
	qword pNormals         = si_from_ptr(edgeGeomGetNormalUniformTable(ctx));   // 0 if no normals
	const qword pTangents  = si_from_ptr(edgeGeomGetTangentUniformTable(ctx));  // 0 if no tangents
	const qword pBinormals = si_from_ptr(edgeGeomGetBinormalUniformTable(ctx)); // 0 if no binormals

	// Determine which path to take through the skinning loop, based
	// on how many non-position skinnable attributes are present, and whether scaling
	// is enabled.
	const bool hasNonUniformScale =
		skinningFlavor == EDGE_GEOM_SKIN_NON_UNIFORM_SCALING ||
		skinningFlavor == EDGE_GEOM_SKIN_SINGLE_BONE_NON_UNIFORM_SCALING;
	const bool hasScale = !(		
		skinningFlavor == EDGE_GEOM_SKIN_NO_SCALING ||
		skinningFlavor == EDGE_GEOM_SKIN_SINGLE_BONE_NO_SCALING);
	const bool useSingleBoneSkinning = 
		skinningFlavor == EDGE_GEOM_SKIN_SINGLE_BONE_NO_SCALING ||
		skinningFlavor == EDGE_GEOM_SKIN_SINGLE_BONE_UNIFORM_SCALING ||
		skinningFlavor == EDGE_GEOM_SKIN_SINGLE_BONE_NON_UNIFORM_SCALING;
	const bool skinNormalsWithAdjoint = hasNonUniformScale && si_to_int(pNormals) != 0;
	uint32_t skinnableVectorCount = 0; // excludes position (and normals, if non-uniform scaling is used)
	qword uniforms[3];
	uniforms[skinnableVectorCount] = pNormals;
	skinnableVectorCount += (si_to_int(pNormals) && !hasNonUniformScale) ? 1 : 0;
	uniforms[skinnableVectorCount] = pTangents;
	skinnableVectorCount += si_to_int(pTangents) ? 1 : 0;
	uniforms[skinnableVectorCount] = pBinormals;
	skinnableVectorCount += si_to_int(pBinormals) ? 1 : 0;
	qword pUniform1  = uniforms[0];
	qword pUniform2  = uniforms[1];
	qword pUniform3  = uniforms[2];

	// We divide the vertexes into groups, and skin each group separately.  This is so
	// we can fit a 3x4 blended bone matrix for each vertex in the spare uniform table.  Within
	// each group we process four vertices per inner loop iteration.  Each group will contain
	// a maximum of [numVertexes/3 & ~3] verts
	uint32_t totalVertsLeft = (ctx->spuConfigInfo.numVertexes+3) & ~3; // round up to a multiple of 4
	// Must have special-case code for extremely small segments, or else maxVertsPerGroup ends up being
	// zero!
	uint32_t maxVertsPerGroup = (totalVertsLeft < 12) ? 4 : (totalVertsLeft/3) & ~0x3;

	// We'll be storing the final blended bone matrix for each vertex in the spare uniform table.
	// Since we process 4 verts per inner loop iteration, the spare table must have room for 4 3x4
	// matrices (= 12 qwords).  In the highly unlikely event that we have a partition with fewer than
	// 12 vertices, we instead store the final bone matrices in a temporary array on the stack.
	const qword pExtraUniformTable = si_from_ptr(ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF]);
	qword matricesOnStack[12];
	const qword pMatrices = (totalVertsLeft < 12) ? si_from_ptr(matricesOnStack) : pExtraUniformTable;

	do
	{
		qword pOutMatrices = pMatrices;
		qword pFinalMatrices = pOutMatrices;

		// blend and store the bone matrices for this group
		uint32_t vertsInGroup = (totalVertsLeft < maxVertsPerGroup) ? totalVertsLeft : maxVertsPerGroup;
		uint32_t vertsLeft = vertsInGroup;

		qword pInPositions = pPositions;
		qword pInIndexesAndWeights = pIndexesAndWeights;


		//// LOOP PREAMBLE

		if (useSingleBoneSkinning)
		{
			// Advance to the beginning of the next group
			pIndexesAndWeights = si_a(pIndexesAndWeights, si_from_int(vertsInGroup));
			
			const qword s_0A0B0A0B0A0B0A0B = si_ai(si_ilh(0x8000), 1);

			// Load the positions and create two permutations of each
			qword pos1_xyzw = si_lqd(pInPositions, 0x00);
			qword pos1_yzxw = si_shufb(pos1_xyzw, s_AAAA, (qword)s_BCAD);
			qword pos1_zxyw = si_shufb(pos1_xyzw, s_AAAA, (qword)s_CABD);
			qword pos2_xyzw = si_lqd(pInPositions, 0x10);
			qword pos2_yzxw = si_shufb(pos2_xyzw, s_AAAA, (qword)s_BCAD);
			qword pos2_zxyw = si_shufb(pos2_xyzw, s_AAAA, (qword)s_CABD);
			pInPositions = si_ai(pInPositions, 0x20);
			
			// Load the bone indexes.  We'll process two at a time.  Don't forget to scale
			// the indexes by 48 to make them byte offsets
			qword indexWeightQuad = si_rotqby(si_lqd(pInIndexesAndWeights, 0x00), pInIndexesAndWeights);
			pInIndexesAndWeights = si_ai(pInIndexesAndWeights, 0x02);			
			qword matrixIndexes = si_shufb(indexWeightQuad, indexWeightQuad, s_0A0B0A0B0A0B0A0B);
			qword mat2 = si_a( si_shli(matrixIndexes, 5), si_shli(matrixIndexes, 4) );
			qword mat1 = si_roti(mat2, 16);
			
			do
			{
				// Load the matrices
				qword mat1_1 = si_lqx(pMatRow1, mat1);
				qword mat1_2 = si_lqx(pMatRow2, mat1);
				qword mat1_3 = si_lqx(pMatRow3, mat1);
				qword M1_4 = si_shufb(si_shufb(mat1_2, mat1_3, (qword)s_0Dd0), mat1_1, (qword)s_dBC0);
				qword M1_1 = si_selb(k1_0f,     mat1_1, mask_FFF0);
				qword M1_2 = si_selb(mask_FFF0, mat1_2, mask_FFF0);
				qword M1_3 = si_selb(mask_FFF0, mat1_3, mask_FFF0);
				
				qword mat2_1 = si_lqx(pMatRow1, mat2);
				qword mat2_2 = si_lqx(pMatRow2, mat2);
				qword mat2_3 = si_lqx(pMatRow3, mat2);
				qword M2_4 = si_shufb(si_shufb(mat2_2, mat2_3, (qword)s_0Dd0), mat2_1, (qword)s_dBC0);
				qword M2_1 = si_selb(k1_0f,     mat2_1, mask_FFF0);
				qword M2_2 = si_selb(mask_FFF0, mat2_2, mask_FFF0);
				qword M2_3 = si_selb(mask_FFF0, mat2_3, mask_FFF0);
				
				// Skin and store the positions
				qword out1 = si_fma(pos1_zxyw, M1_3, si_fma(pos1_yzxw, M1_2,
						si_fma(pos1_xyzw, M1_1, M1_4)));
				qword out2 = si_fma(pos2_zxyw, M2_3, si_fma(pos2_yzxw, M2_2,
						si_fma(pos2_xyzw, M2_1, M2_4)));
				
				// Load the positions and create two permutations of each
				pos1_xyzw = si_lqd(pInPositions, 0x00);
				pos1_yzxw = si_shufb(pos1_xyzw, s_AAAA, (qword)s_BCAD);
				pos1_zxyw = si_shufb(pos1_xyzw, s_AAAA, (qword)s_CABD);
				pos2_xyzw = si_lqd(pInPositions, 0x10);
				pos2_yzxw = si_shufb(pos2_xyzw, s_AAAA, (qword)s_BCAD);
				pos2_zxyw = si_shufb(pos2_xyzw, s_AAAA, (qword)s_CABD);
				pInPositions = si_ai(pInPositions, 0x20);

				indexWeightQuad = si_rotqby(si_lqd(pInIndexesAndWeights, 0x00), pInIndexesAndWeights);
				pInIndexesAndWeights = si_ai(pInIndexesAndWeights, 0x02);
				matrixIndexes = si_shufb(indexWeightQuad, indexWeightQuad, s_0A0B0A0B0A0B0A0B);
				mat2 = si_a( si_shli(matrixIndexes, 5), si_shli(matrixIndexes, 4) );
				mat1 = si_roti(mat2, 16);

				// Store the matrices
				si_stqd(M1_1, pOutMatrices, 0x00);
				si_stqd(M1_2, pOutMatrices, 0x10);
				si_stqd(M1_3, pOutMatrices, 0x20);
				si_stqd(M2_1, pOutMatrices, 0x30);
				si_stqd(M2_2, pOutMatrices, 0x40);
				si_stqd(M2_3, pOutMatrices, 0x50);
				pOutMatrices = si_ai(pOutMatrices, 0x60);
				
				si_stqd(out1, pPositions, 0x00);
				si_stqd(out2, pPositions, 0x10);
				pPositions = si_ai(pPositions, 0x20);
				
				vertsLeft -= 2;
			}
			while(vertsLeft > 0);
		}
		else
		{
			// Advance to the beginning of the next group
			pIndexesAndWeights = si_a(pIndexesAndWeights, si_from_int(vertsInGroup*8));

			// Load the bone indexes and weights for the next two vertexes
			qword indexWeightQuad = si_lqd(pInIndexesAndWeights, 0x00);
			pInIndexesAndWeights = si_ai(pInIndexesAndWeights, 0x10);
			
			// The weights must be converted to floats and scaled back the the [0..1] range.
			// Each weight is then broadcast into all four floats of its own quad.
			qword weights1 = si_fm(weightScale,
				si_cuflt( si_shufb(indexWeightQuad, s_AAAA, (qword)s_000A000C000E000G), 0));
			qword weights2 = si_fm(weightScale,
				si_cuflt( si_shufb(indexWeightQuad, s_AAAA, (qword)s_000I000K000M000O), 0));
			qword w1_1 = si_shufb(weights1, s_AAAA, (qword)s_AAAA);
			qword w1_2 = si_shufb(weights1, s_AAAA, (qword)s_BBBB);
			qword w1_3 = si_shufb(weights1, s_AAAA, (qword)s_CCCC);
			qword w1_4 = si_shufb(weights1, s_AAAA, (qword)s_DDDD);
			qword w2_1 = si_shufb(weights2, s_AAAA, (qword)s_AAAA);
			qword w2_2 = si_shufb(weights2, s_AAAA, (qword)s_BBBB);
			qword w2_3 = si_shufb(weights2, s_AAAA, (qword)s_CCCC);
			qword w2_4 = si_shufb(weights2, s_AAAA, (qword)s_DDDD);

			// matrix indexes must be masked out and scaled by 48, to become byte offsets.
			// Then the offset for each matrix must be moved into the preferred word of its own
			// quadword. We do not need to clear the high halfwords, since their low 4 bits
			// are guarenteed to be zero (and that's enough for an 18-bit address).
			qword matrixIndexes = si_and(indexWeightQuad, mask_5555);
			qword mat2 = si_a( si_shli(matrixIndexes, 5), si_shli(matrixIndexes, 4) );
			qword mat4 = si_shlqbyi(mat2, 4);
			qword mat6 = si_shlqbyi(mat2, 8);
			qword mat8 = si_shlqbyi(mat2, 12);
			qword mat1 = si_roti(mat2, 16);
			qword mat3 = si_roti(mat4, 16);
			qword mat5 = si_roti(mat6, 16);
			qword mat7 = si_roti(mat8, 16);

			// Load the positions and create two permutations of each
			qword pos1_xyzw = si_lqd(pInPositions, 0x00);
			qword pos1_yzxw = si_shufb(pos1_xyzw, s_AAAA, (qword)s_BCAD);
			qword pos1_zxyw = si_shufb(pos1_xyzw, s_AAAA, (qword)s_CABD);
			qword pos2_xyzw = si_lqd(pInPositions, 0x10);
			qword pos2_yzxw = si_shufb(pos2_xyzw, s_AAAA, (qword)s_BCAD);
			qword pos2_zxyw = si_shufb(pos2_xyzw, s_AAAA, (qword)s_CABD);
			pInPositions = si_ai(pInPositions, 0x20);

			do
			{
				// Build the final matrix M1
				qword mat1_1 =  si_fm(w1_1, si_lqx(pMatRow1, mat1));
				qword mat1_2 =  si_fm(w1_1, si_lqx(pMatRow2, mat1));
				qword mat1_3 =  si_fm(w1_1, si_lqx(pMatRow3, mat1));
				qword mat2_1 = si_fma(w1_2, si_lqx(pMatRow1, mat2), mat1_1);
				qword mat2_2 = si_fma(w1_2, si_lqx(pMatRow2, mat2), mat1_2);
				qword mat2_3 = si_fma(w1_2, si_lqx(pMatRow3, mat2), mat1_3);
				qword mat3_1 = si_fma(w1_3, si_lqx(pMatRow1, mat3), mat2_1);
				qword mat3_2 = si_fma(w1_3, si_lqx(pMatRow2, mat3), mat2_2);
				qword mat3_3 = si_fma(w1_3, si_lqx(pMatRow3, mat3), mat2_3);
				qword M1_1a  = si_fma(w1_4, si_lqx(pMatRow1, mat4), mat3_1);
				qword M1_2a  = si_fma(w1_4, si_lqx(pMatRow2, mat4), mat3_2);
				qword M1_3a  = si_fma(w1_4, si_lqx(pMatRow3, mat4), mat3_3);
				qword M1_4   = si_shufb(si_shufb(M1_2a, M1_3a, (qword)s_0Dd0), M1_1a, (qword)s_dBC0);
				qword M1_1   = si_selb(k1_0f,     M1_1a, mask_FFF0);
				qword M1_2   = si_selb(mask_FFF0, M1_2a, mask_FFF0);
				qword M1_3   = si_selb(mask_FFF0, M1_3a, mask_FFF0);

				// Build the final matrix M2
				qword mat5_1 =  si_fm(w2_1, si_lqx(pMatRow1, mat5));
				qword mat5_2 =  si_fm(w2_1, si_lqx(pMatRow2, mat5));
				qword mat5_3 =  si_fm(w2_1, si_lqx(pMatRow3, mat5));
				qword mat6_1 = si_fma(w2_2, si_lqx(pMatRow1, mat6), mat5_1);
				qword mat6_2 = si_fma(w2_2, si_lqx(pMatRow2, mat6), mat5_2);
				qword mat6_3 = si_fma(w2_2, si_lqx(pMatRow3, mat6), mat5_3);
				qword mat7_1 = si_fma(w2_3, si_lqx(pMatRow1, mat7), mat6_1);
				qword mat7_2 = si_fma(w2_3, si_lqx(pMatRow2, mat7), mat6_2);
				qword mat7_3 = si_fma(w2_3, si_lqx(pMatRow3, mat7), mat6_3);
				qword M2_1a  = si_fma(w2_4, si_lqx(pMatRow1, mat8), mat7_1);
				qword M2_2a  = si_fma(w2_4, si_lqx(pMatRow2, mat8), mat7_2);
				qword M2_3a  = si_fma(w2_4, si_lqx(pMatRow3, mat8), mat7_3);
				qword M2_4   = si_shufb(si_shufb(M2_2a, M2_3a, (qword)s_0Dd0), M2_1a, (qword)s_dBC0);
				qword M2_1   = si_selb(k1_0f,     M2_1a, mask_FFF0);
				qword M2_2   = si_selb(mask_FFF0, M2_2a, mask_FFF0);
				qword M2_3   = si_selb(mask_FFF0, M2_3a, mask_FFF0);

				// Skin and store the positions
				qword out1 = si_fma(pos1_zxyw, M1_3, si_fma(pos1_yzxw, M1_2,
						si_fma(pos1_xyzw, M1_1, M1_4)));
				qword out2 = si_fma(pos2_zxyw, M2_3, si_fma(pos2_yzxw, M2_2,
						si_fma(pos2_xyzw, M2_1, M2_4)));

				// Load the bone indexes and weights for the next two vertexes
				indexWeightQuad = si_lqd(pInIndexesAndWeights, 0x00);
				pInIndexesAndWeights = si_ai(pInIndexesAndWeights, 0x10);

				// The weights must be converted to floats and scaled back the the [0..1] range.
				// Each weight is then broadcast into all four floats of its own quad.
				weights1 = si_fm(weightScale,
					si_cuflt( si_shufb(indexWeightQuad, s_AAAA, (qword)s_000A000C000E000G), 0));
				weights2 = si_fm(weightScale,
					si_cuflt( si_shufb(indexWeightQuad, s_AAAA, (qword)s_000I000K000M000O), 0));

				// matrix indexes must be masked out and scaled by 48, to become byte offsets.
				//Then the offset for each matrix must be moved into the preferred word of its own
				// quadword. We do not need to clear the high halfwords, since their low 4 bits
				// are guarenteed to be zero (and that's enough for an 18-bit address).
				matrixIndexes = si_and(indexWeightQuad, mask_5555);
			
				w1_1 = si_shufb(weights1, s_AAAA, (qword)s_AAAA);
				w1_2 = si_shufb(weights1, s_AAAA, (qword)s_BBBB);
				w1_3 = si_shufb(weights1, s_AAAA, (qword)s_CCCC);
				w1_4 = si_shufb(weights1, s_AAAA, (qword)s_DDDD);
				w2_1 = si_shufb(weights2, s_AAAA, (qword)s_AAAA);
				w2_2 = si_shufb(weights2, s_AAAA, (qword)s_BBBB);
				w2_3 = si_shufb(weights2, s_AAAA, (qword)s_CCCC);
				w2_4 = si_shufb(weights2, s_AAAA, (qword)s_DDDD);

				mat2 = si_a( si_shli(matrixIndexes, 5), si_shli(matrixIndexes, 4) );
				mat4 = si_shlqbyi(mat2, 4);
				mat6 = si_shlqbyi(mat2, 8);
				mat8 = si_shlqbyi(mat2, 12);
				mat1 = si_roti(mat2, 16);
				mat3 = si_roti(mat4, 16);
				mat5 = si_roti(mat6, 16);
				mat7 = si_roti(mat8, 16);


				// Load the positions and create two permutations of each
				pos1_xyzw = si_lqd(pInPositions, 0x00);
				pos1_yzxw = si_shufb(pos1_xyzw, s_AAAA, (qword)s_BCAD);
				pos1_zxyw = si_shufb(pos1_xyzw, s_AAAA, (qword)s_CABD);
				pos2_xyzw = si_lqd(pInPositions, 0x10);
				pos2_yzxw = si_shufb(pos2_xyzw, s_AAAA, (qword)s_BCAD);
				pos2_zxyw = si_shufb(pos2_xyzw, s_AAAA, (qword)s_CABD);
				pInPositions = si_ai(pInPositions, 0x20);

				// Store the matrices
				si_stqd(M1_1, pOutMatrices, 0x00);
				si_stqd(M1_2, pOutMatrices, 0x10);
				si_stqd(M1_3, pOutMatrices, 0x20);
				si_stqd(M2_1, pOutMatrices, 0x30);
				si_stqd(M2_2, pOutMatrices, 0x40);
				si_stqd(M2_3, pOutMatrices, 0x50);
				pOutMatrices = si_ai(pOutMatrices, 0x60);

				si_stqd(out1, pPositions, 0x00);
				si_stqd(out2, pPositions, 0x10);
				pPositions = si_ai(pPositions, 0x20);

				vertsLeft -= 2;
			}
			while(vertsLeft > 0);
		}


		// If non-uniform scaling is enabled and normals are present, they must be special-cased
		// to skin with the adjoint matrix
		if (skinNormalsWithAdjoint)
		{
			// Skin normal w/non-uniform scale (includes adjoint calculation)
			vertsLeft = vertsInGroup;
			qword pNormalMatrices = pFinalMatrices;

			// Load matrices
			qword M1_1 = si_lqd(pNormalMatrices, 0x00);
			qword M1_2 = si_lqd(pNormalMatrices, 0x10);
			qword M1_3 = si_lqd(pNormalMatrices, 0x20);
			qword M2_1 = si_lqd(pNormalMatrices, 0x30);
			qword M2_2 = si_lqd(pNormalMatrices, 0x40);
			qword M2_3 = si_lqd(pNormalMatrices, 0x50);
			qword M3_1 = si_lqd(pNormalMatrices, 0x60);
			qword M3_2 = si_lqd(pNormalMatrices, 0x70);
			qword M3_3 = si_lqd(pNormalMatrices, 0x80);
			qword M4_1 = si_lqd(pNormalMatrices, 0x90);
			qword M4_2 = si_lqd(pNormalMatrices, 0xA0);
			qword M4_3 = si_lqd(pNormalMatrices, 0xB0);
			pNormalMatrices = si_ai(pNormalMatrices, 0xC0);
			
			do
			{
				// Convert to adjoint matrices
				qword M1_1_yzxw = si_shufb(M1_1, M1_1, (qword)s_BCAD);
				qword M1_1_zxyw = si_shufb(M1_1, M1_1, (qword)s_CABD);
				qword M1_2_yzxw = si_shufb(M1_2, M1_2, (qword)s_BCAD);
				qword M1_2_zxyw = si_shufb(M1_2, M1_2, (qword)s_CABD);
				qword M1_3_yzxw = si_shufb(M1_3, M1_3, (qword)s_BCAD);
				qword M1_3_zxyw = si_shufb(M1_3, M1_3, (qword)s_CABD);
				
				qword M2_1_yzxw = si_shufb(M2_1, M2_1, (qword)s_BCAD);
				qword M2_1_zxyw = si_shufb(M2_1, M2_1, (qword)s_CABD);
				qword M2_2_yzxw = si_shufb(M2_2, M2_2, (qword)s_BCAD);
				qword M2_2_zxyw = si_shufb(M2_2, M2_2, (qword)s_CABD);
				qword M2_3_yzxw = si_shufb(M2_3, M2_3, (qword)s_BCAD);
				qword M2_3_zxyw = si_shufb(M2_3, M2_3, (qword)s_CABD);
				
				qword M3_1_yzxw = si_shufb(M3_1, M3_1, (qword)s_BCAD);
				qword M3_1_zxyw = si_shufb(M3_1, M3_1, (qword)s_CABD);
				qword M3_2_yzxw = si_shufb(M3_2, M3_2, (qword)s_BCAD);
				qword M3_2_zxyw = si_shufb(M3_2, M3_2, (qword)s_CABD);
				qword M3_3_yzxw = si_shufb(M3_3, M3_3, (qword)s_BCAD);
				qword M3_3_zxyw = si_shufb(M3_3, M3_3, (qword)s_CABD);
				
				qword M4_1_yzxw = si_shufb(M4_1, M4_1, (qword)s_BCAD);
				qword M4_1_zxyw = si_shufb(M4_1, M4_1, (qword)s_CABD);
				qword M4_2_yzxw = si_shufb(M4_2, M4_2, (qword)s_BCAD);
				qword M4_2_zxyw = si_shufb(M4_2, M4_2, (qword)s_CABD);
				qword M4_3_yzxw = si_shufb(M4_3, M4_3, (qword)s_BCAD);
				qword M4_3_zxyw = si_shufb(M4_3, M4_3, (qword)s_CABD);
				
				qword CM1_1 = si_fms(M1_1_yzxw, M1_1_zxyw, si_fm(M1_3_zxyw, M1_2_yzxw));
				qword CM1_2 = si_fms(M1_2_zxyw, M1_2_yzxw, si_fm(M1_3_yzxw, M1_1_zxyw));
				qword CM1_3 = si_fms(M1_3_yzxw, M1_3_zxyw, si_fm(M1_2_zxyw, M1_1_yzxw));
				
				qword CM2_1 = si_fms(M2_1_yzxw, M2_1_zxyw, si_fm(M2_3_zxyw, M2_2_yzxw));
				qword CM2_2 = si_fms(M2_2_zxyw, M2_2_yzxw, si_fm(M2_3_yzxw, M2_1_zxyw));
				qword CM2_3 = si_fms(M2_3_yzxw, M2_3_zxyw, si_fm(M2_2_zxyw, M2_1_yzxw));
				
				qword CM3_1 = si_fms(M3_1_yzxw, M3_1_zxyw, si_fm(M3_3_zxyw, M3_2_yzxw));
				qword CM3_2 = si_fms(M3_2_zxyw, M3_2_yzxw, si_fm(M3_3_yzxw, M3_1_zxyw));
				qword CM3_3 = si_fms(M3_3_yzxw, M3_3_zxyw, si_fm(M3_2_zxyw, M3_1_yzxw));
				
				qword CM4_1 = si_fms(M4_1_yzxw, M4_1_zxyw, si_fm(M4_3_zxyw, M4_2_yzxw));
				qword CM4_2 = si_fms(M4_2_zxyw, M4_2_yzxw, si_fm(M4_3_yzxw, M4_1_zxyw));
				qword CM4_3 = si_fms(M4_3_yzxw, M4_3_zxyw, si_fm(M4_2_zxyw, M4_1_yzxw));
				
				// Load skinnable attributes
				qword in1_xyzw = si_lqd(pNormals, 0x00);
				qword in2_xyzw = si_lqd(pNormals, 0x10);
				qword in3_xyzw = si_lqd(pNormals, 0x20);
				qword in4_xyzw = si_lqd(pNormals, 0x30);
				
				// swizzle the input attributes
				qword in1_yzxw = si_shufb(in1_xyzw, s_AAAA, (qword)s_BCAD);
				qword in2_yzxw = si_shufb(in2_xyzw, s_AAAA, (qword)s_BCAD);
				qword in3_yzxw = si_shufb(in3_xyzw, s_AAAA, (qword)s_BCAD);
				qword in4_yzxw = si_shufb(in4_xyzw, s_AAAA, (qword)s_BCAD);
				qword in1_zxyw = si_shufb(in1_xyzw, s_AAAA, (qword)s_CABD);
				qword in2_zxyw = si_shufb(in2_xyzw, s_AAAA, (qword)s_CABD);
				qword in3_zxyw = si_shufb(in3_xyzw, s_AAAA, (qword)s_CABD);
				qword in4_zxyw = si_shufb(in4_xyzw, s_AAAA, (qword)s_CABD);
				
				// Skin 'em
				qword out1 = si_fma(in1_zxyw, CM1_3, si_fma(in1_yzxw, CM1_2, si_fm(in1_xyzw, CM1_1)));
				qword out2 = si_fma(in2_zxyw, CM2_3, si_fma(in2_yzxw, CM2_2, si_fm(in2_xyzw, CM2_1)));
				qword out3 = si_fma(in3_zxyw, CM3_3, si_fma(in3_yzxw, CM3_2, si_fm(in3_xyzw, CM3_1)));
				qword out4 = si_fma(in4_zxyw, CM4_3, si_fma(in4_yzxw, CM4_2, si_fm(in4_xyzw, CM4_1)));

				////// Next iteration begins here

				// Load matrices
				M1_1 = si_lqd(pNormalMatrices, 0x00);
				M1_2 = si_lqd(pNormalMatrices, 0x10);
				M1_3 = si_lqd(pNormalMatrices, 0x20);
				M2_1 = si_lqd(pNormalMatrices, 0x30);
				M2_2 = si_lqd(pNormalMatrices, 0x40);
				M2_3 = si_lqd(pNormalMatrices, 0x50);
				M3_1 = si_lqd(pNormalMatrices, 0x60);
				M3_2 = si_lqd(pNormalMatrices, 0x70);
				M3_3 = si_lqd(pNormalMatrices, 0x80);
				M4_1 = si_lqd(pNormalMatrices, 0x90);
				M4_2 = si_lqd(pNormalMatrices, 0xA0);
				M4_3 = si_lqd(pNormalMatrices, 0xB0);
				pNormalMatrices = si_ai(pNormalMatrices, 0xC0);

				////// Next iteration ends here

				// Store the skinned attributes
				si_stqd(out1, pNormals, 0x00);
				si_stqd(out2, pNormals, 0x10);
				si_stqd(out3, pNormals, 0x20);
				si_stqd(out4, pNormals, 0x30);
				pNormals = si_ai(pNormals, 0x40);

				vertsLeft -= 4;
			}
			while(vertsLeft > 0);
		}

		// Skin the remaining uniform tables (tangents, binormals, normals (if non-uniform scale)
		vertsLeft = vertsInGroup;

		// This preamble is common among all the case blocks below
		// Load matrices
		qword M1_1 = si_lqd(pFinalMatrices, 0x00);
		qword M1_2 = si_lqd(pFinalMatrices, 0x10);
		qword M1_3 = si_lqd(pFinalMatrices, 0x20);
		qword M2_1 = si_lqd(pFinalMatrices, 0x30);
		qword M2_2 = si_lqd(pFinalMatrices, 0x40);
		qword M2_3 = si_lqd(pFinalMatrices, 0x50);
		qword M3_1 = si_lqd(pFinalMatrices, 0x60);
		qword M3_2 = si_lqd(pFinalMatrices, 0x70);
		qword M3_3 = si_lqd(pFinalMatrices, 0x80);
		qword M4_1 = si_lqd(pFinalMatrices, 0x90);
		qword M4_2 = si_lqd(pFinalMatrices, 0xA0);
		qword M4_3 = si_lqd(pFinalMatrices, 0xB0);
		pFinalMatrices = si_ai(pFinalMatrices, 0xC0);
		
		// Load skinnable attributes
		qword in1_1_xyzw = si_lqd(pUniform1, 0x00);
		qword in1_2_xyzw = si_lqd(pUniform1, 0x10);
		qword in1_3_xyzw = si_lqd(pUniform1, 0x20);
		qword in1_4_xyzw = si_lqd(pUniform1, 0x30);
		
		qword in1_1_yzxw = si_shufb(in1_1_xyzw, s_AAAA, (qword)s_BCAD);
		qword in1_2_yzxw = si_shufb(in1_2_xyzw, s_AAAA, (qword)s_BCAD);
		qword in1_3_yzxw = si_shufb(in1_3_xyzw, s_AAAA, (qword)s_BCAD);
		qword in1_4_yzxw = si_shufb(in1_4_xyzw, s_AAAA, (qword)s_BCAD);
		qword in1_1_zxyw = si_shufb(in1_1_xyzw, s_AAAA, (qword)s_CABD);
		qword in1_2_zxyw = si_shufb(in1_2_xyzw, s_AAAA, (qword)s_CABD);
		qword in1_3_zxyw = si_shufb(in1_3_xyzw, s_AAAA, (qword)s_CABD);
		qword in1_4_zxyw = si_shufb(in1_4_xyzw, s_AAAA, (qword)s_CABD);

		switch(skinnableVectorCount)
		{
		case 1:
			do
			{
				// Skin 'em
				qword out1_1 = si_fma(in1_1_zxyw, M1_3, si_fma(in1_1_yzxw, M1_2,
						si_fm(in1_1_xyzw, M1_1)));
				qword out1_2 = si_fma(in1_2_zxyw, M2_3, si_fma(in1_2_yzxw, M2_2,
						si_fm(in1_2_xyzw, M2_1)));
				qword out1_3 = si_fma(in1_3_zxyw, M3_3, si_fma(in1_3_yzxw, M3_2,
						si_fm(in1_3_xyzw, M3_1)));
				qword out1_4 = si_fma(in1_4_zxyw, M4_3, si_fma(in1_4_yzxw, M4_2,
						si_fm(in1_4_xyzw, M4_1)));
				
				// Load next iteration's matrices
				M1_1 = si_lqd(pFinalMatrices, 0x00);
				M1_2 = si_lqd(pFinalMatrices, 0x10);
				M1_3 = si_lqd(pFinalMatrices, 0x20);
				M2_1 = si_lqd(pFinalMatrices, 0x30);
				M2_2 = si_lqd(pFinalMatrices, 0x40);
				M2_3 = si_lqd(pFinalMatrices, 0x50);
				M3_1 = si_lqd(pFinalMatrices, 0x60);
				M3_2 = si_lqd(pFinalMatrices, 0x70);
				M3_3 = si_lqd(pFinalMatrices, 0x80);
				M4_1 = si_lqd(pFinalMatrices, 0x90);
				M4_2 = si_lqd(pFinalMatrices, 0xA0);
				M4_3 = si_lqd(pFinalMatrices, 0xB0);
				pFinalMatrices = si_ai(pFinalMatrices, 0xC0);
				
				// Load next iteration's skinnable attributes
				in1_1_xyzw = si_lqd(pUniform1, 0x40);
				in1_2_xyzw = si_lqd(pUniform1, 0x50);
				in1_3_xyzw = si_lqd(pUniform1, 0x60);
				in1_4_xyzw = si_lqd(pUniform1, 0x70);

				// swizzle the next iteration's input attributes
				in1_1_yzxw = si_shufb(in1_1_xyzw, s_AAAA, (qword)s_BCAD);
				in1_2_yzxw = si_shufb(in1_2_xyzw, s_AAAA, (qword)s_BCAD);
				in1_3_yzxw = si_shufb(in1_3_xyzw, s_AAAA, (qword)s_BCAD);
				in1_4_yzxw = si_shufb(in1_4_xyzw, s_AAAA, (qword)s_BCAD);
				in1_1_zxyw = si_shufb(in1_1_xyzw, s_AAAA, (qword)s_CABD);
				in1_2_zxyw = si_shufb(in1_2_xyzw, s_AAAA, (qword)s_CABD);
				in1_3_zxyw = si_shufb(in1_3_xyzw, s_AAAA, (qword)s_CABD);
				in1_4_zxyw = si_shufb(in1_4_xyzw, s_AAAA, (qword)s_CABD);

				// Store the skinned attributes
				si_stqd(out1_1, pUniform1, 0x00);
				si_stqd(out1_2, pUniform1, 0x10);
				si_stqd(out1_3, pUniform1, 0x20);
				si_stqd(out1_4, pUniform1, 0x30);
				pUniform1 = si_ai(pUniform1, 0x40);
				
				vertsLeft -= 4;
			}
			while(vertsLeft > 0);
			break;

		case 2:
 			do
			{
				qword in2_1_xyzw = si_lqd(pUniform2, 0x00);
				qword in2_2_xyzw = si_lqd(pUniform2, 0x10);
				qword in2_3_xyzw = si_lqd(pUniform2, 0x20);
				qword in2_4_xyzw = si_lqd(pUniform2, 0x30);
				
				// Swizzle attributes
				qword in2_1_yzxw = si_shufb(in2_1_xyzw, s_AAAA, (qword)s_BCAD);
				qword in2_2_yzxw = si_shufb(in2_2_xyzw, s_AAAA, (qword)s_BCAD);
				qword in2_3_yzxw = si_shufb(in2_3_xyzw, s_AAAA, (qword)s_BCAD);
				qword in2_4_yzxw = si_shufb(in2_4_xyzw, s_AAAA, (qword)s_BCAD);
				
				qword in2_1_zxyw = si_shufb(in2_1_xyzw, s_AAAA, (qword)s_CABD);
				qword in2_2_zxyw = si_shufb(in2_2_xyzw, s_AAAA, (qword)s_CABD);
				qword in2_3_zxyw = si_shufb(in2_3_xyzw, s_AAAA, (qword)s_CABD);
				qword in2_4_zxyw = si_shufb(in2_4_xyzw, s_AAAA, (qword)s_CABD);
				
				// Skin 'em
				qword out1_1 = si_fma(in1_1_zxyw, M1_3, si_fma(in1_1_yzxw, M1_2,
						si_fm(in1_1_xyzw, M1_1)));
				qword out1_2 = si_fma(in1_2_zxyw, M2_3, si_fma(in1_2_yzxw, M2_2,
						si_fm(in1_2_xyzw, M2_1)));
				qword out1_3 = si_fma(in1_3_zxyw, M3_3, si_fma(in1_3_yzxw, M3_2,
						si_fm(in1_3_xyzw, M3_1)));
				qword out1_4 = si_fma(in1_4_zxyw, M4_3, si_fma(in1_4_yzxw, M4_2,
						si_fm(in1_4_xyzw, M4_1)));

				qword out2_1 = si_fma(in2_1_zxyw, M1_3, si_fma(in2_1_yzxw, M1_2,
						si_fm(in2_1_xyzw, M1_1)));
				qword out2_2 = si_fma(in2_2_zxyw, M2_3, si_fma(in2_2_yzxw, M2_2,
						si_fm(in2_2_xyzw, M2_1)));
				qword out2_3 = si_fma(in2_3_zxyw, M3_3, si_fma(in2_3_yzxw, M3_2,
						si_fm(in2_3_xyzw, M3_1)));
				qword out2_4 = si_fma(in2_4_zxyw, M4_3, si_fma(in2_4_yzxw, M4_2,
						si_fm(in2_4_xyzw, M4_1)));
				
				/////////////

				// Load next iteration's matrices
				M1_1 = si_lqd(pFinalMatrices, 0x00);
				M1_2 = si_lqd(pFinalMatrices, 0x10);
				M1_3 = si_lqd(pFinalMatrices, 0x20);
				M2_1 = si_lqd(pFinalMatrices, 0x30);
				M2_2 = si_lqd(pFinalMatrices, 0x40);
				M2_3 = si_lqd(pFinalMatrices, 0x50);
				M3_1 = si_lqd(pFinalMatrices, 0x60);
				M3_2 = si_lqd(pFinalMatrices, 0x70);
				M3_3 = si_lqd(pFinalMatrices, 0x80);
				M4_1 = si_lqd(pFinalMatrices, 0x90);
				M4_2 = si_lqd(pFinalMatrices, 0xA0);
				M4_3 = si_lqd(pFinalMatrices, 0xB0);
				pFinalMatrices = si_ai(pFinalMatrices, 0xC0);
				
				// Load next iteration's skinnable attributes
				in1_1_xyzw = si_lqd(pUniform1, 0x40);
				in1_2_xyzw = si_lqd(pUniform1, 0x50);
				in1_3_xyzw = si_lqd(pUniform1, 0x60);
				in1_4_xyzw = si_lqd(pUniform1, 0x70);

				// swizzle the next iteration's input attributes
				in1_1_yzxw = si_shufb(in1_1_xyzw, s_AAAA, (qword)s_BCAD);
				in1_2_yzxw = si_shufb(in1_2_xyzw, s_AAAA, (qword)s_BCAD);
				in1_3_yzxw = si_shufb(in1_3_xyzw, s_AAAA, (qword)s_BCAD);
				in1_4_yzxw = si_shufb(in1_4_xyzw, s_AAAA, (qword)s_BCAD);
				in1_1_zxyw = si_shufb(in1_1_xyzw, s_AAAA, (qword)s_CABD);
				in1_2_zxyw = si_shufb(in1_2_xyzw, s_AAAA, (qword)s_CABD);
				in1_3_zxyw = si_shufb(in1_3_xyzw, s_AAAA, (qword)s_CABD);
				in1_4_zxyw = si_shufb(in1_4_xyzw, s_AAAA, (qword)s_CABD);

				// Store the skinned attributes
				si_stqd(out1_1, pUniform1, 0x00);
				si_stqd(out1_2, pUniform1, 0x10);
				si_stqd(out1_3, pUniform1, 0x20);
				si_stqd(out1_4, pUniform1, 0x30);
				pUniform1 = si_ai(pUniform1, 0x40);
				si_stqd(out2_1, pUniform2, 0x00);
				si_stqd(out2_2, pUniform2, 0x10);
				si_stqd(out2_3, pUniform2, 0x20);
				si_stqd(out2_4, pUniform2, 0x30);
				pUniform2 = si_ai(pUniform2, 0x40);

				vertsLeft -= 4;
			}
			while(vertsLeft > 0);
			break;

		case 3:
 			do
			{
				qword in2_1_xyzw = si_lqd(pUniform2, 0x00);
				qword in2_2_xyzw = si_lqd(pUniform2, 0x10);
				qword in2_3_xyzw = si_lqd(pUniform2, 0x20);
				qword in2_4_xyzw = si_lqd(pUniform2, 0x30);

				qword in3_1_xyzw = si_lqd(pUniform3, 0x00);
				qword in3_2_xyzw = si_lqd(pUniform3, 0x10);
				qword in3_3_xyzw = si_lqd(pUniform3, 0x20);
				qword in3_4_xyzw = si_lqd(pUniform3, 0x30);
				
				// Swizzle attributes
				qword in2_1_yzxw = si_shufb(in2_1_xyzw, s_AAAA, (qword)s_BCAD);
				qword in2_2_yzxw = si_shufb(in2_2_xyzw, s_AAAA, (qword)s_BCAD);
				qword in2_3_yzxw = si_shufb(in2_3_xyzw, s_AAAA, (qword)s_BCAD);
				qword in2_4_yzxw = si_shufb(in2_4_xyzw, s_AAAA, (qword)s_BCAD);
				
				qword in2_1_zxyw = si_shufb(in2_1_xyzw, s_AAAA, (qword)s_CABD);
				qword in2_2_zxyw = si_shufb(in2_2_xyzw, s_AAAA, (qword)s_CABD);
				qword in2_3_zxyw = si_shufb(in2_3_xyzw, s_AAAA, (qword)s_CABD);
				qword in2_4_zxyw = si_shufb(in2_4_xyzw, s_AAAA, (qword)s_CABD);

				qword in3_1_yzxw = si_shufb(in3_1_xyzw, s_AAAA, (qword)s_BCAD);
				qword in3_2_yzxw = si_shufb(in3_2_xyzw, s_AAAA, (qword)s_BCAD);
				qword in3_3_yzxw = si_shufb(in3_3_xyzw, s_AAAA, (qword)s_BCAD);
				qword in3_4_yzxw = si_shufb(in3_4_xyzw, s_AAAA, (qword)s_BCAD);
				
				qword in3_1_zxyw = si_shufb(in3_1_xyzw, s_AAAA, (qword)s_CABD);
				qword in3_2_zxyw = si_shufb(in3_2_xyzw, s_AAAA, (qword)s_CABD);
				qword in3_3_zxyw = si_shufb(in3_3_xyzw, s_AAAA, (qword)s_CABD);
				qword in3_4_zxyw = si_shufb(in3_4_xyzw, s_AAAA, (qword)s_CABD);
				
				// Skin 'em
				qword out1_1 = si_fma(in1_1_zxyw, M1_3, si_fma(in1_1_yzxw, M1_2,
						si_fm(in1_1_xyzw, M1_1)));
				qword out1_2 = si_fma(in1_2_zxyw, M2_3, si_fma(in1_2_yzxw, M2_2,
						si_fm(in1_2_xyzw, M2_1)));
				qword out1_3 = si_fma(in1_3_zxyw, M3_3, si_fma(in1_3_yzxw, M3_2,
						si_fm(in1_3_xyzw, M3_1)));
				qword out1_4 = si_fma(in1_4_zxyw, M4_3, si_fma(in1_4_yzxw, M4_2,
						si_fm(in1_4_xyzw, M4_1)));

				qword out2_1 = si_fma(in2_1_zxyw, M1_3, si_fma(in2_1_yzxw, M1_2,
						si_fm(in2_1_xyzw, M1_1)));
				qword out2_2 = si_fma(in2_2_zxyw, M2_3, si_fma(in2_2_yzxw, M2_2,
						si_fm(in2_2_xyzw, M2_1)));
				qword out2_3 = si_fma(in2_3_zxyw, M3_3, si_fma(in2_3_yzxw, M3_2,
						si_fm(in2_3_xyzw, M3_1)));
				qword out2_4 = si_fma(in2_4_zxyw, M4_3, si_fma(in2_4_yzxw, M4_2,
						si_fm(in2_4_xyzw, M4_1)));

				qword out3_1 = si_fma(in3_1_zxyw, M1_3, si_fma(in3_1_yzxw, M1_2,
						si_fm(in3_1_xyzw, M1_1)));
				qword out3_2 = si_fma(in3_2_zxyw, M2_3, si_fma(in3_2_yzxw, M2_2,
						si_fm(in3_2_xyzw, M2_1)));
				qword out3_3 = si_fma(in3_3_zxyw, M3_3, si_fma(in3_3_yzxw, M3_2,
						si_fm(in3_3_xyzw, M3_1)));
				qword out3_4 = si_fma(in3_4_zxyw, M4_3, si_fma(in3_4_yzxw, M4_2,
						si_fm(in3_4_xyzw, M4_1)));
				
				/////////////

				// Load next iteration's matrices
				M1_1 = si_lqd(pFinalMatrices, 0x00);
				M1_2 = si_lqd(pFinalMatrices, 0x10);
				M1_3 = si_lqd(pFinalMatrices, 0x20);
				M2_1 = si_lqd(pFinalMatrices, 0x30);
				M2_2 = si_lqd(pFinalMatrices, 0x40);
				M2_3 = si_lqd(pFinalMatrices, 0x50);
				M3_1 = si_lqd(pFinalMatrices, 0x60);
				M3_2 = si_lqd(pFinalMatrices, 0x70);
				M3_3 = si_lqd(pFinalMatrices, 0x80);
				M4_1 = si_lqd(pFinalMatrices, 0x90);
				M4_2 = si_lqd(pFinalMatrices, 0xA0);
				M4_3 = si_lqd(pFinalMatrices, 0xB0);
				pFinalMatrices = si_ai(pFinalMatrices, 0xC0);
				
				// Load next iteration's skinnable attributes
				in1_1_xyzw = si_lqd(pUniform1, 0x40);
				in1_2_xyzw = si_lqd(pUniform1, 0x50);
				in1_3_xyzw = si_lqd(pUniform1, 0x60);
				in1_4_xyzw = si_lqd(pUniform1, 0x70);

				// swizzle the next iteration's input attributes
				in1_1_yzxw = si_shufb(in1_1_xyzw, s_AAAA, (qword)s_BCAD);
				in1_2_yzxw = si_shufb(in1_2_xyzw, s_AAAA, (qword)s_BCAD);
				in1_3_yzxw = si_shufb(in1_3_xyzw, s_AAAA, (qword)s_BCAD);
				in1_4_yzxw = si_shufb(in1_4_xyzw, s_AAAA, (qword)s_BCAD);
				in1_1_zxyw = si_shufb(in1_1_xyzw, s_AAAA, (qword)s_CABD);
				in1_2_zxyw = si_shufb(in1_2_xyzw, s_AAAA, (qword)s_CABD);
				in1_3_zxyw = si_shufb(in1_3_xyzw, s_AAAA, (qword)s_CABD);
				in1_4_zxyw = si_shufb(in1_4_xyzw, s_AAAA, (qword)s_CABD);

				// Store the skinned attributes
				si_stqd(out1_1, pUniform1, 0x00);
				si_stqd(out1_2, pUniform1, 0x10);
				si_stqd(out1_3, pUniform1, 0x20);
				si_stqd(out1_4, pUniform1, 0x30);
				pUniform1 = si_ai(pUniform1, 0x40);
				si_stqd(out2_1, pUniform2, 0x00);
				si_stqd(out2_2, pUniform2, 0x10);
				si_stqd(out2_3, pUniform2, 0x20);
				si_stqd(out2_4, pUniform2, 0x30);
				pUniform2 = si_ai(pUniform2, 0x40);
				si_stqd(out3_1, pUniform3, 0x00);
				si_stqd(out3_2, pUniform3, 0x10);
				si_stqd(out3_3, pUniform3, 0x20);
				si_stqd(out3_4, pUniform3, 0x30);
				pUniform3 = si_ai(pUniform3, 0x40);
								
				vertsLeft -= 4;
			}
			while(vertsLeft > 0);
			break;
		}

		totalVertsLeft -= vertsInGroup;
	}
	while(totalVertsLeft > 0);

#if 0 //BP
	// If scaling was enabled, we need to make one more pass over the
	// vector attributes and renormalize them.
	if (hasScale)
	{
		const uint32_t numVerts = ctx->spuConfigInfo.numVertexes;
		switch(skinnableVectorCount)
		{
		case 3:
			edgeGeomNormalizeUniformTable((qword*)si_to_ptr(uniforms[2]), numVerts);
			// Fall-through
		case 2:
			edgeGeomNormalizeUniformTable((qword*)si_to_ptr(uniforms[1]), numVerts);
			// fall-through
		case 1:
			edgeGeomNormalizeUniformTable((qword*)si_to_ptr(uniforms[0]), numVerts);
			break;
		}
		if (skinNormalsWithAdjoint)
		{
			edgeGeomNormalizeUniformTable(edgeGeomGetNormalUniformTable(ctx), numVerts);
		}
	}
#endif

	// don't need the matrices or indexes/weights anymore
	void *earliest = (matrices < indexesAndWeights) ? matrices : indexesAndWeights;
	edgeGeomSetFreePtr(ctx, earliest);
}
#endif
