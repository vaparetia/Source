/* SCE CONFIDENTIAL 
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"

#if !EDGEGEOMSKINVERTEXES_INTRINSICS
static inline float rsqrt(float f)
{
	return si_to_float(si_fi(si_from_float(f), si_frsqest(si_from_float(f))));
}

void edgeGeomSkinVertexes(EdgeGeomSpuContext *ctx, void *matrices, uint32_t matrixCount,
	void *indexesAndWeights)
{
	// Skinning is forbidden in the static geometry fast path!
	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
		return;

	const uint8_t skinningFlavor = ctx->spuConfigInfo.indexesFlavorAndSkinningFlavor & 0x0F;
	if(matrixCount == 0	|| skinningFlavor == EDGE_GEOM_SKIN_NONE)
		return;

	// Skinning requires the extra uniform table, to store the final blended matrices for each vertex.
	if ((ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_INCLUDES_EXTRA_UNIFORM_TABLE) == 0)
	{
		EDGE_PRINTF("ERROR: attempt to use skinning without allocating an extra uniform table!\n");
		return;
	}

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
		float *mIn = (float *)matrices;
		float *mOut = mIn;
		const uint32_t kInMatrixSize = (matrixFormat == EDGE_GEOM_MATRIX_3x4_ROW_MAJOR) ? 12 : 16;
		switch(matrixFormat)
		{
		case EDGE_GEOM_MATRIX_3x4_ROW_MAJOR:
		case EDGE_GEOM_MATRIX_4x4_ROW_MAJOR:
			for (uint32_t i = 0; i < matrixCount; i++) 
			{
				float A = mIn[0];
				float B = mIn[1];
				float C = mIn[2];
				float D = mIn[3];
				float E = mIn[4];
				float F = mIn[5];
				float G = mIn[6];
				float H = mIn[7];
				float I = mIn[8];
				float J = mIn[9];
				float K = mIn[10];
				float L = mIn[11];
				
				mOut[0] = A;
				mOut[1] = F;
				mOut[2] = K;
				mOut[3] = D;
				mOut[4] = B;
				mOut[5] = G;
				mOut[6] = I;
				mOut[7] = H;
				mOut[8] = C;
				mOut[9] = E;
				mOut[10] = J; 
				mOut[11] = L; 
				
				mIn += kInMatrixSize;
				mOut += 12; // output matrices are always 3x4
			}
			break;
		case EDGE_GEOM_MATRIX_4x4_COLUMN_MAJOR:
			for (uint32_t i = 0; i < matrixCount; i++) 
			{
				float A = mIn[0];
				float B = mIn[4];
				float C = mIn[8];
				float D = mIn[12];
				float E = mIn[1];
				float F = mIn[5];
				float G = mIn[9];
				float H = mIn[13];
				float I = mIn[2];
				float J = mIn[6];
				float K = mIn[10];
				float L = mIn[14];
				
				mOut[0] = A;
				mOut[1] = F;
				mOut[2] = K;
				mOut[3] = D;
				mOut[4] = B;
				mOut[5] = G;
				mOut[6] = I;
				mOut[7] = H;
				mOut[8] = C;
				mOut[9] = E;
				mOut[10] = J; 
				mOut[11] = L; 
				
				mIn += kInMatrixSize;
				mOut += 12; // output matrices are always 3x4
			}
			break;
		}
	}

	const float weightScale = 1.0f / 255.0f;

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
	const bool skinNormalsWithAdjoint = hasNonUniformScale && edgeGeomGetNormalUniformTable(ctx) != 0;

	uint8_t *pIndexWeight = (uint8_t *)indexesAndWeights;

	float *positions = (float *)edgeGeomGetPositionUniformTable(ctx);
	float *normals   = (float *)edgeGeomGetNormalUniformTable(ctx);
	float *tangents  = (float *)edgeGeomGetTangentUniformTable(ctx);
	float *binormals = (float *)edgeGeomGetBinormalUniformTable(ctx);

	uint32_t skinnableVectorCount = 0; // excludes position (and normals, if non-uniform scaling is used)
	float *uniforms[3];
	uniforms[skinnableVectorCount] = normals;
	skinnableVectorCount += normals ? 1 : 0;
	uniforms[skinnableVectorCount] = tangents;
	skinnableVectorCount += tangents ? 1 : 0;
	uniforms[skinnableVectorCount] = binormals;
	skinnableVectorCount += binormals ? 1 : 0;

	const uint32_t vertexCount = (ctx->spuConfigInfo.numVertexes+3) & ~3; // round up to a multiple of 4

	//the intrinsics code trashes the data in the extra uniform table.
	uint32_t *extraUniform = (uint32_t *)edgeGeomGetUniformTable(ctx, ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF);
	for (uint32_t i = 0; i < vertexCount; i++) {
		extraUniform[0] = 0xdeadbeef;
		extraUniform[1] = 0xdeadbeef;
		extraUniform[2] = 0xdeadbeef;
		extraUniform[3] = 0xdeadbeef;
		extraUniform += 4;
	}

	for (uint32_t i = 0; i < vertexCount; i++) {
		float M[16];
		if (useSingleBoneSkinning)
		{
			// Load a single matrix
			uint32_t matrixOffset = *pIndexWeight * 12; // 12 floats per matrix
			pIndexWeight++;

			float *mat = (float*)matrices + matrixOffset;
			for(uint32_t iElem=0; iElem<12; ++iElem)
				M[iElem] = mat[iElem];
		}
		else
		{
			// Load and blend four matrices
			float weight1 = weightScale * (float)pIndexWeight[0];
			float weight2 = weightScale * (float)pIndexWeight[2];
			float weight3 = weightScale * (float)pIndexWeight[4];
			float weight4 = weightScale * (float)pIndexWeight[6];

			uint32_t mat1Offset = pIndexWeight[1] * 12;
			uint32_t mat2Offset = pIndexWeight[3] * 12;
			uint32_t mat3Offset = pIndexWeight[5] * 12;
			uint32_t mat4Offset = pIndexWeight[7] * 12;
			pIndexWeight += 8;

			float *mat1 = (float *)matrices + mat1Offset;
			float *mat2 = (float *)matrices + mat2Offset;
			float *mat3 = (float *)matrices + mat3Offset;
			float *mat4 = (float *)matrices + mat4Offset;

			M[0] = weight1 * mat1[0] + weight2 * mat2[0] + weight3 * mat3[0] + weight4 * mat4[0];
			M[1] = weight1 * mat1[1] + weight2 * mat2[1] + weight3 * mat3[1] + weight4 * mat4[1];
			M[2] = weight1 * mat1[2] + weight2 * mat2[2] + weight3 * mat3[2] + weight4 * mat4[2];
			M[3] = weight1 * mat1[3] + weight2 * mat2[3] + weight3 * mat3[3] + weight4 * mat4[3];
			
			M[4] = weight1 * mat1[4] + weight2 * mat2[4] + weight3 * mat3[4] + weight4 * mat4[4];
			M[5] = weight1 * mat1[5] + weight2 * mat2[5] + weight3 * mat3[5] + weight4 * mat4[5];
			M[6] = weight1 * mat1[6] + weight2 * mat2[6] + weight3 * mat3[6] + weight4 * mat4[6];
			M[7] = weight1 * mat1[7] + weight2 * mat2[7] + weight3 * mat3[7] + weight4 * mat4[7];
			
			M[8] = weight1 * mat1[8] + weight2 * mat2[8] + weight3 * mat3[8] + weight4 * mat4[8];
			M[9] = weight1 * mat1[9] + weight2 * mat2[9] + weight3 * mat3[9] + weight4 * mat4[9];
			M[10]= weight1 * mat1[10]+ weight2 * mat2[10]+ weight3 * mat3[10]+ weight4 * mat4[10];
			M[11]= weight1 * mat1[11]+ weight2 * mat2[11]+ weight3 * mat3[11]+ weight4 * mat4[11];
		}

		// Move the fourth column of the matrix into the fourth row
		M[12] = M[3];
		M[13] = M[7];
		M[14] = M[11];
		M[15] = 0.f;
		
		M[3] = 1.f;
		M[7] = 0.f;
		M[11]= 0.f;

		//skin vertex
		switch (skinnableVectorCount)
		{
		case 3:
			{
				float uniforms2[4] = {(uniforms[2])[0], (uniforms[2])[1], (uniforms[2])[2], (uniforms[2])[3]};
	
				(uniforms[2])[0] = uniforms2[0] * M[0] + uniforms2[1] * M[4] + uniforms2[2] * M[8];
				(uniforms[2])[1] = uniforms2[1] * M[1] + uniforms2[2] * M[5] + uniforms2[0] * M[9];
				(uniforms[2])[2] = uniforms2[2] * M[2] + uniforms2[0] * M[6] + uniforms2[1] * M[10];
				(uniforms[2])[3] = uniforms2[3] * M[3] + uniforms2[3] * M[7] + uniforms2[3] * M[11];

#if 0 //BP
				if (hasScale)
				{
					float r = rsqrt(	(uniforms[2])[0] * (uniforms[2])[0] +
						(uniforms[2])[1] * (uniforms[2])[1] +
						(uniforms[2])[2] * (uniforms[2])[2]);
					(uniforms[2])[0] *= r;
					(uniforms[2])[1] *= r;
					(uniforms[2])[2] *= r;
				}
#endif

				(uniforms[2]) += 4;
			}
			// INTENTIONAL FALL-THROUGH
		case 2:
			{
				float uniforms1[4] = {(uniforms[1])[0], (uniforms[1])[1], (uniforms[1])[2], (uniforms[1])[3]};
				(uniforms[1])[0] = uniforms1[0] * M[0] + uniforms1[1] * M[4] + uniforms1[2] * M[8];
				(uniforms[1])[1] = uniforms1[1] * M[1] + uniforms1[2] * M[5] + uniforms1[0] * M[9];
				(uniforms[1])[2] = uniforms1[2] * M[2] + uniforms1[0] * M[6] + uniforms1[1] * M[10];
				(uniforms[1])[3] = uniforms1[3] * M[3] + uniforms1[3] * M[7] + uniforms1[3] * M[11];
	
#if 0 //BP
            if (hasScale)
				{
					float r = rsqrt(	(uniforms[1])[0] * (uniforms[1])[0] +
						(uniforms[1])[1] * (uniforms[1])[1] +
						(uniforms[1])[2] * (uniforms[1])[2]);
					(uniforms[1])[0] *= r;
					(uniforms[1])[1] *= r;
					(uniforms[1])[2] *= r;
				}
#endif
				(uniforms[1]) += 4;
			}
			// INTENTIONAL FALL-THROUGH
		case 1:
			{
				float *nM = M;
				float adjoint[12];

				if (skinNormalsWithAdjoint )
				{
					//compute the adjoint matrix
					adjoint[0] = M[1]  * M[2]  - M[10] * M[5];
					adjoint[1] = M[2]  * M[0]  - M[8]  * M[6];
					adjoint[2] = M[0]  * M[1]  - M[9]  * M[4];
					adjoint[3] = 0;
						
					adjoint[4] = M[6]  * M[5]  - M[9]  * M[2];
					adjoint[5] = M[4]  * M[6]  - M[10] * M[0];
					adjoint[6] = M[5]  * M[4]  - M[8]  * M[1];
					adjoint[7] = 0;
	
					adjoint[8] = M[9]  * M[10] - M[6]  * M[1];
					adjoint[9] = M[10] * M[8]  - M[4]  * M[2];
					adjoint[10]= M[8]  * M[9]  - M[5]  * M[0];
					adjoint[11]= 0;
	
					nM = adjoint;
				}
	
				float uniforms0[4] = {(uniforms[0])[0], (uniforms[0])[1], (uniforms[0])[2], (uniforms[0])[3]};
				(uniforms[0])[0] = uniforms0[0] * nM[0] + uniforms0[1] * nM[4] + uniforms0[2] * nM[8];
				(uniforms[0])[1] = uniforms0[1] * nM[1] + uniforms0[2] * nM[5] + uniforms0[0] * nM[9];
				(uniforms[0])[2] = uniforms0[2] * nM[2] + uniforms0[0] * nM[6] + uniforms0[1] * nM[10];
				(uniforms[0])[3] = uniforms0[3] * nM[3] + uniforms0[3] * nM[7] + uniforms0[3] * nM[11];

#if 0 //BP
            if (hasScale)
				{
					float r = rsqrt(	(uniforms[0])[0] * (uniforms[0])[0] +
						(uniforms[0])[1] * (uniforms[0])[1] +
						(uniforms[0])[2] * (uniforms[0])[2]);
					(uniforms[0])[0] *= r;
					(uniforms[0])[1] *= r;
					(uniforms[0])[2] *= r;
				}
#endif
				(uniforms[0]) += 4;
			}
		default:
			float pos[4] = {positions[0], positions[1], positions[2], positions[3]};
			positions[0] = M[12] + pos[0] * M[0] + pos[1] * M[4] + pos[2] * M[8];
			positions[1] = M[13] + pos[1] * M[1] + pos[2] * M[5] + pos[0] * M[9];
			positions[2] = M[14] + pos[2] * M[2] + pos[0] * M[6] + pos[1] * M[10];
			positions[3] = M[15] + pos[3] * M[3] + pos[3] * M[7] + pos[3] * M[11];

			positions += 4;
		}
	}

	void *earliest = (matrices < indexesAndWeights) ? matrices : indexesAndWeights;
	edgeGeomSetFreePtr(ctx, earliest);
}
#endif
