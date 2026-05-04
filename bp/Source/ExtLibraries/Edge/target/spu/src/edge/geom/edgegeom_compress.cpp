/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_printf.h"
#include "edge/edge_assert.h"

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"
#include "edge/geom/edgegeom_compress.h"
#include "edge/geom/edgegeom_masks.h"

#if COMPRESSVERTEXESBYDESCRIPTION_INTRINSICS
void *CompressVertexesByDescription(EdgeGeomSpuContext *ctx,
	const EdgeGeomVertexStreamDescription *streamDesc)
{
	// The static geometry path should not be compressing/outputting vertices!
	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
		return (void*)ctx->vertexesLs;

	void *outVertexes = ctx->freePtr;
	ctx->vertexesLs = (uint32_t)outVertexes;

	if(ctx->spuConfigInfo.numVertexes == 0)
		return outVertexes;

	// Assorted masks and constants
	const qword s_AAAA = si_ila(0x10203);
	const qword s_abcd = si_orbi((qword)s_ABCD, 0x10);
	const qword s_000A000A000A000A = (qword)spu_splats(0x80808000);
	const qword s_000B000B000B000B = si_ori(s_000A000A000A000A, 0x01);
	const qword s_000E000E000E000E = si_ori(s_000A000A000A000A, 0x04);
	const qword s_DDDDDDDDDDDDDDDD = si_ilh(0x0303);

	const qword u8Bias    = (qword)spu_splats(0.5f);
	const qword u8nScale  = (qword)spu_splats(255.0f);
	const qword u8nBias   = (qword)spu_splats(0.5f);
	const qword i16nScale = (qword)spu_splats(65535.0f / 2.0f);
	const qword i16nBias  = (qword)spu_splats(-0.5f);
	const qword f16ExpRebias = (qword)spu_splats(0x38000000);
	const qword f16SignMask  = (qword)spu_splats(0x80008000);
	const qword f16MaxHalf   = (qword)spu_splats(0x7BFF7BFF);
	const qword f16ExpMin    = (qword)spu_splats(0x38003800);
	const qword f16ExpMax    = (qword)spu_splats(0x47804780);
	const qword x11y11z10nXzMask   = (qword)s_EAeaEAea;
	const qword x11y11z10nYMask    = (qword)si_orbi(si_rotqbyi(x11y11z10nXzMask,1), 0x04);
	const qword x11y11z10nBitShift = (qword)(vec_int4){-5, -2,  0,  0};
	const qword x11y11z10nSelbMask = (qword)spu_splats(0x003FF800);
	const qword x11y11z10nScale    = (qword)(vec_int4){0x3F7FE000, 0x3F7FE000, 0x3F7FC000, 0x00000000};
	const qword x11y11z10nBias     = (qword)(vec_int4){0xBA000000, 0xBA000000, 0xBA800000, 0x00000000};

	// We assume a multiple of four vertexes
	const qword vertexCount  = (qword)spu_splats( (ctx->spuConfigInfo.numVertexes+3)&~3 );
	const qword vertexStride = (qword)spu_splats((int)streamDesc->stride);
	qword dummy[2]; // writes past the end of the buffer are routed here instead
	const qword pSafeBuffer = si_shufb(si_from_ptr(&dummy), s_AAAA, s_AAAA);

	// The main compression loops below process four vertexes per
	// iteration.  These vertexes are not necessarily directly
	// sequential; for efficiency, we process four vertexes which
	// start at the same relative offset within each quadword (that
	// way, we can perform the *exact* same loads & shuffles to
	// extract the relevant attribute from each vertex).  So, rather
	// than looping over all the vertexes from start to finish, we
	// have an outer loop that iterates over each quadword byte offset
	// (0 to 15) and an inner loop that iterates over all vertexes
	// that start at that byte offset (four at a time).
	//
	// The spacing between vertexes with identical byte offsets is
	// dependent upon the vertex stride – in particular, the largest
	// power of two (from 1 to 16) that evenly divides the stride.  If
	// the input stride is 17, its maximum POT divisor is 1, and
	// vertexes that start on the same offset will be 16 vertexes
	// apart.  If the input stride is 32, its maximum POT divisor (up
	// to 16, recall) is 16, and every vertex starts at the same
	// offset.
	//
	// Likewise, the maximum POT divisor determines how many
	// iterations the outer loop needs to run.  For 17-byte vertexes,
	// there will be vertexes starting at every byte offset from 0 to
	// 15, so the outer loop will have 16 iterations, each of which
	// will process 1/16th of the total vertexes.  For 12-byte
	// vertexes (maximum POT divisor: 4), vertexes will only start at
	// offsets 0, 4, 8 and 12, so the outer loop only needs 4
	// iterations, processing 1/4th of the vertexes per iteration.
	// For 32-byte vertexes (maximum POT divisor: 16), all vertexes
	// start at offset 0, so the outer loop needs only one iteration
	// to process all the vertexes.
	int maximumStrideDivisorLog = 0;
	int tempStride = streamDesc->stride;
#ifdef EDGE_GEOM_DEBUG
	EDGE_ASSERT_MSG( streamDesc->stride, ("ERROR: output stream description has 0 size!\n") );
#endif
	while((tempStride & 0x1) == 0 && maximumStrideDivisorLog < 4)
	{
		maximumStrideDivisorLog++;
		tempStride >>= 1;
	}
	qword pAttrSrcOffsets = (qword)(vec_int4){0, 16, 32, 48};
	pAttrSrcOffsets = si_ai(si_shl(pAttrSrcOffsets, (qword)spu_splats(4-maximumStrideDivisorLog)), 1);
	const qword pAttrSrcInc = (qword)spu_splats((64 >> maximumStrideDivisorLog)*0x10);
	const qword destOffsetsInc = si_shl(vertexStride, (qword)spu_splats(6-maximumStrideDivisorLog));
	const uint32_t maxVertIndexOffset = 16 >> maximumStrideDivisorLog;
	qword destOffsetsBase = (qword)(vec_int4){0,1,2,3};
	destOffsetsBase = si_shl( destOffsetsBase, (qword)spu_splats(4-maximumStrideDivisorLog) );

	const qword pAttrDest1 = si_from_ptr(outVertexes);
	const qword pAttrDest2 = si_ai(pAttrDest1, 16);

	const qword srcOffset2 = si_from_int( 1*(16>>maximumStrideDivisorLog) * 0x10 );
	const qword srcOffset3 = si_from_int( 2*(16>>maximumStrideDivisorLog) * 0x10 );
	const qword srcOffset4 = si_from_int( 3*(16>>maximumStrideDivisorLog) * 0x10 );

	// Contains the relative offset from each of the pAttrDestN pointers to the safe buffer.
	const qword safeOffsets = si_sf(si_shufb(pAttrDest1, pAttrDest1, s_AAAA), pSafeBuffer);

	qword isSrcBad;
	qword nextAttrDesc = si_from_ptr(streamDesc->blocks);

	// Process each attribute sequentially.
	for(uint32_t iAttr=0; iAttr<streamDesc->numAttributes; ++iAttr)
	{
		// Load the attribute description
		const qword attrDesc = si_rotqby(si_lqd(nextAttrDesc, 0), nextAttrDesc);
		nextAttrDesc = si_ai(nextAttrDesc, 8);
		// Which uniform table are we reading from?
		const qword id = si_shufb(attrDesc, s_DDDDDDDDDDDDDDDD, s_DDDDDDDDDDDDDDDD);
		const qword matchBits = si_gbb(si_ceqb(id, ctx->uniformTableToAttributeIdMapping));
		const int srcTableIndex = si_to_int(si_ai(si_clz(matchBits), -16));
		EDGE_ASSERT_MSG(srcTableIndex >= 0, ("Attribute ID %d does not have an associated uniform table associated", si_to_int(id)) );
		const qword pSrcTable = si_shufb(si_from_ptr(ctx->uniformTables[srcTableIndex]), s_AAAA, s_AAAA);
		// pointer to the end of the destination uniform table
		const qword pUniformEnd = si_a(pSrcTable, si_shli(vertexCount, 4));
		// Pointers into the source table, 16 vertexes apart
		qword pAttrSrcBase = si_a(pAttrSrcOffsets, pSrcTable);
		// Offset of the attribute within each vertex
		const qword attribByteOffset = si_shufb(attrDesc, attrDesc, s_000A000A000A000A);
		// Size of the attribute (usually componentCount*componentSize, but not always)
		const qword attribSize = si_shufb(attrDesc, attrDesc, s_000E000E000E000E);
		// This mask has its N leftmost bits set, where N is attribSize.
		const qword attribSizeMask = si_and(si_rotm(si_ilhu(0xFFFF), si_sfi(attribSize, 0)),
			si_ila(0xFFFF));

		// Most attribute types will need to have their compressed
		// components gathered into the leftmost portion of the quad
		// before they're written to the destination stream.  For
		// efficiency, we apply the type-specific gather shuffle to
		// the insert mask we calculate below, so that insert and
		// gather can be performed in one step.
		qword gatherMask;
		// We process every 4 vertexes at a time.  The four vertexes
		// are staggered based on the vertex stride, such that our
		// attribute will start at the same byte offset within each
		// quadword, so we only need one insertion mask per iteration.
		const int attribFormat = si_to_int(si_shufb(attrDesc, attrDesc, s_000B000B000B000B));

		switch(attribFormat)
		{
		case EDGE_GEOM_ATTRIBUTE_FORMAT_F32: // F32 will be commented, the rest are fairly boilerplate.
			gatherMask = (qword)s_ABCD;
			for(uint32_t vertIndexOffset=0; vertIndexOffset<maxVertIndexOffset; ++vertIndexOffset)
			{
				qword pAttrSrc = si_a(pAttrSrcBase, (qword)spu_splats(vertIndexOffset<<4));
				// When determining whether a given set of four
				// attributes are safe to write to the output array,
				// we compare the current pAttrSrc against the end of
				// the uniform table.  For attribute types whose
				// compression loop is pipelined, the uniform table
				// end pointer must be adjusted, since the pAttrSrc
				// will actually be one iteration ahead when the
				// comparison is made.
				// In the case of F32, the loop is not pipelined, so no adjustment
				// is necessary.
				qword pAdjustedUniformEnd = si_a(pUniformEnd, si_mpyi(pAttrSrcInc,0));
				// dest offset starts for the Nth iteration starts at
				// the byte offset of the attribute in the Nth vertex.
				qword destOffsets = si_mpya(vertexStride,
					si_a( (qword)spu_splats(vertIndexOffset), destOffsetsBase ), attribByteOffset );
				// Build a shuffle mask to insert the attribute, given
				// its byte offset within each quadword.
				const qword quadOffset = si_andi(destOffsets, 0xF);
				const qword selMaskGen1 = si_rotm(attribSizeMask, si_sfi(quadOffset,0));
				const qword selMask1 = si_fsmb(selMaskGen1);
				const qword insertAndGatherMask1 = si_selb((qword)s_abcd,
					si_rotqmby(gatherMask, si_sfi(quadOffset,0)), selMask1);
				// If the attribute straddles two quadwords, we need to
				// load two quads per vertex to extract the full
				// attribute.  Otherwise, we can just load one.
				const int straddlesQuad = si_to_int(si_cgti(si_a(quadOffset, attribSize), 16));
				if (straddlesQuad) // slower loop with two load/shufb/store blocks per vertex
				{
					const qword selMaskGen2 = si_shl(attribSizeMask, si_sfi(quadOffset,16));
					const qword selMask2 = si_fsmb(selMaskGen2);
					const qword insertAndGatherMask2 = si_selb((qword)s_abcd,
						si_shlqby(gatherMask, si_sfi(quadOffset,16)), selMask2);
					do
					{
						// If any of the source pointers are past the
						// end of the source array, their writes must
						// be redirected to the safe buffer.
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load source
						qword in1 = si_lqd(pAttrSrc, 0x000);
						qword in2 = si_lqx(pAttrSrc, srcOffset2);
						qword in3 = si_lqx(pAttrSrc, srcOffset3);
						qword in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						//    nothing to do for F32s
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest1b = si_lqx(pAttrDest2, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest2b = si_lqx(pAttrDest2, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest3b = si_lqx(pAttrDest2, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						qword dest4b = si_lqx(pAttrDest2, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(in1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(in2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(in3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(in4, dest4a, insertAndGatherMask1);
						qword out1b = si_shufb(in1, dest1b, insertAndGatherMask2);
						qword out2b = si_shufb(in2, dest2b, insertAndGatherMask2);
						qword out3b = si_shufb(in3, dest3b, insertAndGatherMask2);
						qword out4b = si_shufb(in4, dest4b, insertAndGatherMask2);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						si_stqx(out1b, pAttrDest2, finalDestOffset1);
						si_stqx(out2b, pAttrDest2, finalDestOffset2);
						si_stqx(out3b, pAttrDest2, finalDestOffset3);
						si_stqx(out4b, pAttrDest2, finalDestOffset4);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
				else // faster loop with one load/shufb/store block per vertex
				{
					do
					{
						// If any of the source pointers are past the
						// end of the source array, they must be
						// redirected to the safe buffer.
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load source
						qword in1 = si_lqd(pAttrSrc, 0x000);
						qword in2 = si_lqx(pAttrSrc, srcOffset2);
						qword in3 = si_lqx(pAttrSrc, srcOffset3);
						qword in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						//    nothing to do for F32s
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(in1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(in2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(in3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(in4, dest4a, insertAndGatherMask1);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
			}
			break;

		case EDGE_GEOM_ATTRIBUTE_FORMAT_F16:
			gatherMask = (qword)s_ABCD;
			for(uint32_t vertIndexOffset=0; vertIndexOffset<maxVertIndexOffset; ++vertIndexOffset)
			{
				qword pAdjustedUniformEnd = si_a(pUniformEnd, si_mpyi(pAttrSrcInc,1));
				qword pAttrSrc = si_a(pAttrSrcBase, (qword)spu_splats(vertIndexOffset<<4));
				qword destOffsets = si_mpya(vertexStride,
					si_a( (qword)spu_splats(vertIndexOffset), destOffsetsBase ), attribByteOffset );
				const qword quadOffset = si_andi(destOffsets, 0xF);
				const qword selMaskGen1 = si_rotm(attribSizeMask, si_sfi(quadOffset,0));
				const qword selMask1 = si_fsmb(selMaskGen1);
				const qword insertAndGatherMask1 = si_selb((qword)s_abcd,
					si_rotqmby(gatherMask, si_sfi(quadOffset,0)), selMask1);

				// preamble
				// load source
				qword in1 = si_lqd(pAttrSrc, 0x000);
				qword in2 = si_lqx(pAttrSrc, srcOffset2);
				qword in3 = si_lqx(pAttrSrc, srcOffset3);
				qword in4 = si_lqx(pAttrSrc, srcOffset4);
				// convert
				qword in12  = si_shufb(in1, in2, (qword)s_ABEFIJMNabefijmn);
				qword in34  = si_shufb(in3, in4, (qword)s_ABEFIJMNabefijmn);
				qword nz12  = si_andc(in12, f16SignMask);
				qword nz34  = si_andc(in34, f16SignMask);
				qword m12   = si_cgth(f16ExpMin, nz12);
				qword m34   = si_cgth(f16ExpMin, nz34);
				qword x12   = si_cgth(f16ExpMax, nz12);
				qword x34   = si_cgth(f16ExpMax, nz34);
				qword r1    = si_rotqbii(si_sf(f16ExpRebias, in1), 3);
				qword r2    = si_rotqbii(si_sf(f16ExpRebias, in2), 3);
				qword r3    = si_rotqbii(si_sf(f16ExpRebias, in3), 3);
				qword r4    = si_rotqbii(si_sf(f16ExpRebias, in4), 3);
				qword r12   = si_selb(f16MaxHalf, si_shufb(r1, r2, (qword)s_ABEFIJMNabefijmn), x12);
				qword r34   = si_selb(f16MaxHalf, si_shufb(r3, r4, (qword)s_ABEFIJMNabefijmn), x34);
				qword n12   = si_selb(r12, in12, f16SignMask);
				qword n34   = si_selb(r34, in34, f16SignMask);
				qword conv1 = si_andc(n12, m12);
				qword conv3 = si_andc(n34, m34);
				qword conv2 = si_rotqbyi(conv1, 8);
				qword conv4 = si_rotqbyi(conv3, 8);
				pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);

				const int straddlesQuad = si_to_int(si_cgti(si_a(quadOffset, attribSize), 16));
				if (straddlesQuad) // slower loop with two load/shufb/store blocks per vertex
				{
					const qword selMaskGen2 = si_shl(attribSizeMask, si_sfi(quadOffset,16));
					const qword selMask2 = si_fsmb(selMaskGen2);
					const qword insertAndGatherMask2 = si_selb((qword)s_abcd,
						si_shlqby(gatherMask, si_sfi(quadOffset,16)), selMask2);
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest1b = si_lqx(pAttrDest2, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest2b = si_lqx(pAttrDest2, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest3b = si_lqx(pAttrDest2, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						qword dest4b = si_lqx(pAttrDest2, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						qword out1b = si_shufb(conv1, dest1b, insertAndGatherMask2);
						qword out2b = si_shufb(conv2, dest2b, insertAndGatherMask2);
						qword out3b = si_shufb(conv3, dest3b, insertAndGatherMask2);
						qword out4b = si_shufb(conv4, dest4b, insertAndGatherMask2);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						si_stqx(out1b, pAttrDest2, finalDestOffset1);
						si_stqx(out2b, pAttrDest2, finalDestOffset2);
						si_stqx(out3b, pAttrDest2, finalDestOffset3);
						si_stqx(out4b, pAttrDest2, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						in12  = si_shufb(in1, in2, (qword)s_ABEFIJMNabefijmn);
						in34  = si_shufb(in3, in4, (qword)s_ABEFIJMNabefijmn);
						nz12  = si_andc(in12, f16SignMask);
						nz34  = si_andc(in34, f16SignMask);
						m12   = si_cgth(f16ExpMin, nz12);
						m34   = si_cgth(f16ExpMin, nz34);
						x12   = si_cgth(f16ExpMax, nz12);
						x34   = si_cgth(f16ExpMax, nz34);
						r1    = si_rotqbii(si_sf(f16ExpRebias, in1), 3);
						r2    = si_rotqbii(si_sf(f16ExpRebias, in2), 3);
						r3    = si_rotqbii(si_sf(f16ExpRebias, in3), 3);
						r4    = si_rotqbii(si_sf(f16ExpRebias, in4), 3);
						r12   = si_selb(f16MaxHalf, si_shufb(r1, r2, (qword)s_ABEFIJMNabefijmn), x12);
						r34   = si_selb(f16MaxHalf, si_shufb(r3, r4, (qword)s_ABEFIJMNabefijmn), x34);
						n12   = si_selb(r12, in12, f16SignMask);
						n34   = si_selb(r34, in34, f16SignMask);
						conv1 = si_andc(n12, m12);
						conv3 = si_andc(n34, m34);
						conv2 = si_rotqbyi(conv1, 8);
						conv4 = si_rotqbyi(conv3, 8);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
				else // faster loop with one load/shufb/store block per vertex
				{
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert	
						in12  = si_shufb(in1, in2, (qword)s_ABEFIJMNabefijmn);
						in34  = si_shufb(in3, in4, (qword)s_ABEFIJMNabefijmn);
						nz12  = si_andc(in12, f16SignMask);
						nz34  = si_andc(in34, f16SignMask);
						m12   = si_cgth(f16ExpMin, nz12);
						m34   = si_cgth(f16ExpMin, nz34);
						x12   = si_cgth(f16ExpMax, nz12);
						x34   = si_cgth(f16ExpMax, nz34);
						r1    = si_rotqbii(si_sf(f16ExpRebias, in1), 3);
						r2    = si_rotqbii(si_sf(f16ExpRebias, in2), 3);
						r3    = si_rotqbii(si_sf(f16ExpRebias, in3), 3);
						r4    = si_rotqbii(si_sf(f16ExpRebias, in4), 3);
						r12   = si_selb(f16MaxHalf, si_shufb(r1, r2, (qword)s_ABEFIJMNabefijmn), x12);
						r34   = si_selb(f16MaxHalf, si_shufb(r3, r4, (qword)s_ABEFIJMNabefijmn), x34);
						n12   = si_selb(r12, in12, f16SignMask);
						n34   = si_selb(r34, in34, f16SignMask);
						conv1 = si_andc(n12, m12);
						conv3 = si_andc(n34, m34);
						conv2 = si_rotqbyi(conv1, 8);
						conv4 = si_rotqbyi(conv3, 8);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
			}
			break;


		case EDGE_GEOM_ATTRIBUTE_FORMAT_U8:
			gatherMask = (qword)s_AEIM000000000000;
			for(uint32_t vertIndexOffset=0; vertIndexOffset<maxVertIndexOffset; ++vertIndexOffset)
			{
				qword pAdjustedUniformEnd = si_a(pUniformEnd, si_mpyi(pAttrSrcInc,1));
				qword pAttrSrc = si_a(pAttrSrcBase, (qword)spu_splats(vertIndexOffset<<4));
				qword destOffsets = si_mpya(vertexStride,
					si_a( (qword)spu_splats(vertIndexOffset), destOffsetsBase ), attribByteOffset );
				const qword quadOffset = si_andi(destOffsets, 0xF);
				const qword selMaskGen1 = si_rotm(attribSizeMask, si_sfi(quadOffset,0));
				const qword selMask1 = si_fsmb(selMaskGen1);
				const qword insertAndGatherMask1 = si_selb((qword)s_abcd,
					si_rotqmby(gatherMask, si_sfi(quadOffset,0)), selMask1);

				// preamble
				// load source
				qword in1 = si_lqd(pAttrSrc, 0x000);
				qword in2 = si_lqx(pAttrSrc, srcOffset2);
				qword in3 = si_lqx(pAttrSrc, srcOffset3);
				qword in4 = si_lqx(pAttrSrc, srcOffset4);
				// add a half to prevent always rounding down
				qword in1Tmp = si_fa(in1, u8Bias);
				qword in2Tmp = si_fa(in2, u8Bias);
				qword in3Tmp = si_fa(in3, u8Bias);
				qword in4Tmp = si_fa(in4, u8Bias);
				// convert
				qword conv1 = si_cfltu(in1Tmp, 24);
				qword conv2 = si_cfltu(in2Tmp, 24);
				qword conv3 = si_cfltu(in3Tmp, 24);
				qword conv4 = si_cfltu(in4Tmp, 24);
				pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
				
				const int straddlesQuad = si_to_int(si_cgti(si_a(quadOffset, attribSize), 16));
				if (straddlesQuad) // slower loop with two load/shufb/store blocks per vertex
				{
					const qword selMaskGen2 = si_shl(attribSizeMask, si_sfi(quadOffset,16));
					const qword selMask2 = si_fsmb(selMaskGen2);
					const qword insertAndGatherMask2 = si_selb((qword)s_abcd,
						si_shlqby(gatherMask, si_sfi(quadOffset,16)), selMask2);
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest1b = si_lqx(pAttrDest2, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest2b = si_lqx(pAttrDest2, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest3b = si_lqx(pAttrDest2, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						qword dest4b = si_lqx(pAttrDest2, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						qword out1b = si_shufb(conv1, dest1b, insertAndGatherMask2);
						qword out2b = si_shufb(conv2, dest2b, insertAndGatherMask2);
						qword out3b = si_shufb(conv3, dest3b, insertAndGatherMask2);
						qword out4b = si_shufb(conv4, dest4b, insertAndGatherMask2);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						si_stqx(out1b, pAttrDest2, finalDestOffset1);
						si_stqx(out2b, pAttrDest2, finalDestOffset2);
						si_stqx(out3b, pAttrDest2, finalDestOffset3);
						si_stqx(out4b, pAttrDest2, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// add a half to prevent always rounding down
						in1Tmp = si_fa(in1, u8Bias);
						in2Tmp = si_fa(in2, u8Bias);
						in3Tmp = si_fa(in3, u8Bias);
						in4Tmp = si_fa(in4, u8Bias);
						// convert
						conv1 = si_cfltu(in1Tmp, 24);
						conv2 = si_cfltu(in2Tmp, 24);
						conv3 = si_cfltu(in3Tmp, 24);
						conv4 = si_cfltu(in4Tmp, 24);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
				else // faster loop with one load/shufb/store block per vertex
				{
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// add a half to prevent always rounding down
						in1Tmp = si_fa(in1, u8Bias);
						in2Tmp = si_fa(in2, u8Bias);
						in3Tmp = si_fa(in3, u8Bias);
						in4Tmp = si_fa(in4, u8Bias);
						// convert
						conv1 = si_cfltu(in1Tmp, 24);
						conv2 = si_cfltu(in2Tmp, 24);
						conv3 = si_cfltu(in3Tmp, 24);
						conv4 = si_cfltu(in4Tmp, 24);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
			}
			break;


		case EDGE_GEOM_ATTRIBUTE_FORMAT_I16:
			gatherMask = (qword)s_ACEG0000;
			for(uint32_t vertIndexOffset=0; vertIndexOffset<maxVertIndexOffset; ++vertIndexOffset)
			{
				qword pAdjustedUniformEnd = si_a(pUniformEnd, si_mpyi(pAttrSrcInc,1));
				qword pAttrSrc = si_a(pAttrSrcBase, (qword)spu_splats(vertIndexOffset<<4));
				qword destOffsets = si_mpya(vertexStride,
					si_a( (qword)spu_splats(vertIndexOffset), destOffsetsBase ), attribByteOffset );
				const qword quadOffset = si_andi(destOffsets, 0xF);
				const qword selMaskGen1 = si_rotm(attribSizeMask, si_sfi(quadOffset,0));
				const qword selMask1 = si_fsmb(selMaskGen1);
				const qword insertAndGatherMask1 = si_selb((qword)s_abcd,
					si_rotqmby(gatherMask, si_sfi(quadOffset,0)), selMask1);

				// preamble
				// load source
				qword in1 = si_lqd(pAttrSrc, 0x000);
				qword in2 = si_lqx(pAttrSrc, srcOffset2);
				qword in3 = si_lqx(pAttrSrc, srcOffset3);
				qword in4 = si_lqx(pAttrSrc, srcOffset4);
				// convert
				qword conv1 = si_cflts(in1, 16);
				qword conv2 = si_cflts(in2, 16);
				qword conv3 = si_cflts(in3, 16);
				qword conv4 = si_cflts(in4, 16);
				pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
						
				const int straddlesQuad = si_to_int(si_cgti(si_a(quadOffset, attribSize), 16));
				if (straddlesQuad) // slower loop with two load/shufb/store blocks per vertex
				{
					const qword selMaskGen2 = si_shl(attribSizeMask, si_sfi(quadOffset,16));
					const qword selMask2 = si_fsmb(selMaskGen2);
					const qword insertAndGatherMask2 = si_selb((qword)s_abcd,
						si_shlqby(gatherMask, si_sfi(quadOffset,16)), selMask2);
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest1b = si_lqx(pAttrDest2, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest2b = si_lqx(pAttrDest2, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest3b = si_lqx(pAttrDest2, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						qword dest4b = si_lqx(pAttrDest2, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						qword out1b = si_shufb(conv1, dest1b, insertAndGatherMask2);
						qword out2b = si_shufb(conv2, dest2b, insertAndGatherMask2);
						qword out3b = si_shufb(conv3, dest3b, insertAndGatherMask2);
						qword out4b = si_shufb(conv4, dest4b, insertAndGatherMask2);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						si_stqx(out1b, pAttrDest2, finalDestOffset1);
						si_stqx(out2b, pAttrDest2, finalDestOffset2);
						si_stqx(out3b, pAttrDest2, finalDestOffset3);
						si_stqx(out4b, pAttrDest2, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						conv1 = si_cflts(in1, 16);
						conv2 = si_cflts(in2, 16);
						conv3 = si_cflts(in3, 16);
						conv4 = si_cflts(in4, 16);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
				else // faster loop with one load/shufb/store block per vertex
				{
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						conv1 = si_cflts(in1, 16);
						conv2 = si_cflts(in2, 16);
						conv3 = si_cflts(in3, 16);
						conv4 = si_cflts(in4, 16);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
			}
			break;


		case EDGE_GEOM_ATTRIBUTE_FORMAT_U8N:
			gatherMask = (qword)s_AEIM000000000000;
			for(uint32_t vertIndexOffset=0; vertIndexOffset<maxVertIndexOffset; ++vertIndexOffset)
			{
				qword pAdjustedUniformEnd = si_a(pUniformEnd, si_mpyi(pAttrSrcInc,1));
				qword pAttrSrc = si_a(pAttrSrcBase, (qword)spu_splats(vertIndexOffset<<4));
				qword destOffsets = si_mpya(vertexStride,
					si_a( (qword)spu_splats(vertIndexOffset), destOffsetsBase ), attribByteOffset );
				const qword quadOffset = si_andi(destOffsets, 0xF);
				const qword selMaskGen1 = si_rotm(attribSizeMask, si_sfi(quadOffset,0));
				const qword selMask1 = si_fsmb(selMaskGen1);
				const qword insertAndGatherMask1 = si_selb((qword)s_abcd,
					si_rotqmby(gatherMask, si_sfi(quadOffset,0)), selMask1);

				// preamble
				// load source
				qword in1 = si_lqd(pAttrSrc, 0x000);
				qword in2 = si_lqx(pAttrSrc, srcOffset2);
				qword in3 = si_lqx(pAttrSrc, srcOffset3);
				qword in4 = si_lqx(pAttrSrc, srcOffset4);
				// convert	
				qword conv1 = si_cfltu(si_fma(in1, u8nScale, u8nBias), 24);
				qword conv2 = si_cfltu(si_fma(in2, u8nScale, u8nBias), 24);
				qword conv3 = si_cfltu(si_fma(in3, u8nScale, u8nBias), 24);
				qword conv4 = si_cfltu(si_fma(in4, u8nScale, u8nBias), 24);
				pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);

				const int straddlesQuad = si_to_int(si_cgti(si_a(quadOffset, attribSize), 16));
				if (straddlesQuad) // slower loop with two load/shufb/store blocks per vertex
				{
					const qword selMaskGen2 = si_shl(attribSizeMask, si_sfi(quadOffset,16));
					const qword selMask2 = si_fsmb(selMaskGen2);
					const qword insertAndGatherMask2 = si_selb((qword)s_abcd,
						si_shlqby(gatherMask, si_sfi(quadOffset,16)), selMask2);
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest1b = si_lqx(pAttrDest2, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest2b = si_lqx(pAttrDest2, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest3b = si_lqx(pAttrDest2, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						qword dest4b = si_lqx(pAttrDest2, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						qword out1b = si_shufb(conv1, dest1b, insertAndGatherMask2);
						qword out2b = si_shufb(conv2, dest2b, insertAndGatherMask2);
						qword out3b = si_shufb(conv3, dest3b, insertAndGatherMask2);
						qword out4b = si_shufb(conv4, dest4b, insertAndGatherMask2);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						si_stqx(out1b, pAttrDest2, finalDestOffset1);
						si_stqx(out2b, pAttrDest2, finalDestOffset2);
						si_stqx(out3b, pAttrDest2, finalDestOffset3);
						si_stqx(out4b, pAttrDest2, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						conv1 = si_cfltu(si_fma(in1, u8nScale, u8nBias), 24);
						conv2 = si_cfltu(si_fma(in2, u8nScale, u8nBias), 24);
						conv3 = si_cfltu(si_fma(in3, u8nScale, u8nBias), 24);
						conv4 = si_cfltu(si_fma(in4, u8nScale, u8nBias), 24);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
				else // faster loop with one load/shufb/store block per vertex
				{
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						conv1 = si_cfltu(si_fma(in1, u8nScale, u8nBias), 24);
						conv2 = si_cfltu(si_fma(in2, u8nScale, u8nBias), 24);
						conv3 = si_cfltu(si_fma(in3, u8nScale, u8nBias), 24);
						conv4 = si_cfltu(si_fma(in4, u8nScale, u8nBias), 24);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
			}
			break;


		case EDGE_GEOM_ATTRIBUTE_FORMAT_I16N:
			gatherMask = (qword)s_ACEG0000;
			for(uint32_t vertIndexOffset=0; vertIndexOffset<maxVertIndexOffset; ++vertIndexOffset)
			{
				qword pAdjustedUniformEnd = si_a(pUniformEnd, si_mpyi(pAttrSrcInc,1));
				qword pAttrSrc = si_a(pAttrSrcBase, (qword)spu_splats(vertIndexOffset<<4));
				qword destOffsets = si_mpya(vertexStride,
					si_a( (qword)spu_splats(vertIndexOffset), destOffsetsBase ), attribByteOffset );
				const qword quadOffset = si_andi(destOffsets, 0xF);
				const qword selMaskGen1 = si_rotm(attribSizeMask, si_sfi(quadOffset,0));
				const qword selMask1 = si_fsmb(selMaskGen1);
				const qword insertAndGatherMask1 = si_selb((qword)s_abcd,
					si_rotqmby(gatherMask, si_sfi(quadOffset,0)), selMask1);

				// preamble
				// load source
				qword in1 = si_lqd(pAttrSrc, 0x000);
				qword in2 = si_lqx(pAttrSrc, srcOffset2);
				qword in3 = si_lqx(pAttrSrc, srcOffset3);
				qword in4 = si_lqx(pAttrSrc, srcOffset4);
				// convert
				qword conv1 = si_cflts(si_fma(in1, i16nScale, i16nBias), 16);
				qword conv2 = si_cflts(si_fma(in2, i16nScale, i16nBias), 16);
				qword conv3 = si_cflts(si_fma(in3, i16nScale, i16nBias), 16);
				qword conv4 = si_cflts(si_fma(in4, i16nScale, i16nBias), 16);
				pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);

				const int straddlesQuad = si_to_int(si_cgti(si_a(quadOffset, attribSize), 16));
				if (straddlesQuad) // slower loop with two load/shufb/store blocks per vertex
				{
					const qword selMaskGen2 = si_shl(attribSizeMask, si_sfi(quadOffset,16));
					const qword selMask2 = si_fsmb(selMaskGen2);
					const qword insertAndGatherMask2 = si_selb((qword)s_abcd,
						si_shlqby(gatherMask, si_sfi(quadOffset,16)), selMask2);
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest1b = si_lqx(pAttrDest2, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest2b = si_lqx(pAttrDest2, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest3b = si_lqx(pAttrDest2, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						qword dest4b = si_lqx(pAttrDest2, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						qword out1b = si_shufb(conv1, dest1b, insertAndGatherMask2);
						qword out2b = si_shufb(conv2, dest2b, insertAndGatherMask2);
						qword out3b = si_shufb(conv3, dest3b, insertAndGatherMask2);
						qword out4b = si_shufb(conv4, dest4b, insertAndGatherMask2);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						si_stqx(out1b, pAttrDest2, finalDestOffset1);
						si_stqx(out2b, pAttrDest2, finalDestOffset2);
						si_stqx(out3b, pAttrDest2, finalDestOffset3);
						si_stqx(out4b, pAttrDest2, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						conv1 = si_cflts(si_fma(in1, i16nScale, i16nBias), 16);
						conv2 = si_cflts(si_fma(in2, i16nScale, i16nBias), 16);
						conv3 = si_cflts(si_fma(in3, i16nScale, i16nBias), 16);
						conv4 = si_cflts(si_fma(in4, i16nScale, i16nBias), 16);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
				else // faster loop with one load/shufb/store block per vertex
				{
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						conv1 = si_cflts(si_fma(in1, i16nScale, i16nBias), 16);
						conv2 = si_cflts(si_fma(in2, i16nScale, i16nBias), 16);
						conv3 = si_cflts(si_fma(in3, i16nScale, i16nBias), 16);
						conv4 = si_cflts(si_fma(in4, i16nScale, i16nBias), 16);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
			}
			break;


		case EDGE_GEOM_ATTRIBUTE_FORMAT_X11Y11Z10N:
			gatherMask = (qword)s_ABCD;
			for(uint32_t vertIndexOffset=0; vertIndexOffset<maxVertIndexOffset; ++vertIndexOffset)
			{
				qword pAdjustedUniformEnd = si_a(pUniformEnd, si_mpyi(pAttrSrcInc,1));
				qword pAttrSrc = si_a(pAttrSrcBase, (qword)spu_splats(vertIndexOffset<<4));
				qword destOffsets = si_mpya(vertexStride,
					si_a( (qword)spu_splats(vertIndexOffset), destOffsetsBase ), attribByteOffset );
				const qword quadOffset = si_andi(destOffsets, 0xF);
				const qword selMaskGen1 = si_rotm(attribSizeMask, si_sfi(quadOffset,0));
				const qword selMask1 = si_fsmb(selMaskGen1);
				const qword insertAndGatherMask1 = si_selb((qword)s_abcd,
					si_rotqmby(gatherMask, si_sfi(quadOffset,0)), selMask1);

				// preamble
				// load source
				qword in1 = si_lqd(pAttrSrc, 0x000);
				qword in2 = si_lqx(pAttrSrc, srcOffset2);
				qword in3 = si_lqx(pAttrSrc, srcOffset3);
				qword in4 = si_lqx(pAttrSrc, srcOffset4);
				// convert
				qword temp1 = si_rotm(si_cflts(si_fma(in1, x11y11z10nScale, x11y11z10nBias),
						31), x11y11z10nBitShift);
				qword temp2 = si_rotm(si_cflts(si_fma(in2, x11y11z10nScale, x11y11z10nBias),
						31), x11y11z10nBitShift);
				qword temp3 = si_rotm(si_cflts(si_fma(in3, x11y11z10nScale, x11y11z10nBias),
						31), x11y11z10nBitShift);
				qword temp4 = si_rotm(si_cflts(si_fma(in4, x11y11z10nScale, x11y11z10nBias),
						31), x11y11z10nBitShift);
				qword xz12 = si_shufb(temp1, temp2, x11y11z10nXzMask);
				qword xz34 = si_shufb(temp3, temp4, x11y11z10nXzMask);
				qword y12  = si_shufb(temp1, temp2, x11y11z10nYMask);
				qword y34  = si_shufb(temp3, temp4, x11y11z10nYMask);
				qword conv1 = si_selb(xz12, y12, x11y11z10nSelbMask);
				qword conv3 = si_selb(xz34, y34, x11y11z10nSelbMask);
				qword conv2 = si_rotqbyi(conv1, 4);
				qword conv4 = si_rotqbyi(conv3, 4);
				pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
						
				const int straddlesQuad = si_to_int(si_cgti(si_a(quadOffset, attribSize), 16));
				if (straddlesQuad) // slower loop with two load/shufb/store blocks per vertex
				{
					const qword selMaskGen2 = si_shl(attribSizeMask, si_sfi(quadOffset,16));
					const qword selMask2 = si_fsmb(selMaskGen2);
					const qword insertAndGatherMask2 = si_selb((qword)s_abcd,
						si_shlqby(gatherMask, si_sfi(quadOffset,16)), selMask2);
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest1b = si_lqx(pAttrDest2, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest2b = si_lqx(pAttrDest2, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest3b = si_lqx(pAttrDest2, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						qword dest4b = si_lqx(pAttrDest2, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						qword out1b = si_shufb(conv1, dest1b, insertAndGatherMask2);
						qword out2b = si_shufb(conv2, dest2b, insertAndGatherMask2);
						qword out3b = si_shufb(conv3, dest3b, insertAndGatherMask2);
						qword out4b = si_shufb(conv4, dest4b, insertAndGatherMask2);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						si_stqx(out1b, pAttrDest2, finalDestOffset1);
						si_stqx(out2b, pAttrDest2, finalDestOffset2);
						si_stqx(out3b, pAttrDest2, finalDestOffset3);
						si_stqx(out4b, pAttrDest2, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						temp1 = si_rotm(si_cflts(si_fma(in1, x11y11z10nScale, x11y11z10nBias),
								31), x11y11z10nBitShift);
						temp2 = si_rotm(si_cflts(si_fma(in2, x11y11z10nScale, x11y11z10nBias),
								31), x11y11z10nBitShift);
						temp3 = si_rotm(si_cflts(si_fma(in3, x11y11z10nScale, x11y11z10nBias),
								31), x11y11z10nBitShift);
						temp4 = si_rotm(si_cflts(si_fma(in4, x11y11z10nScale, x11y11z10nBias),
								31), x11y11z10nBitShift);
						xz12 = si_shufb(temp1, temp2, x11y11z10nXzMask);
						xz34 = si_shufb(temp3, temp4, x11y11z10nXzMask);
						y12  = si_shufb(temp1, temp2, x11y11z10nYMask);
						y34  = si_shufb(temp3, temp4, x11y11z10nYMask);
						conv1 = si_selb(xz12, y12, x11y11z10nSelbMask);
						conv3 = si_selb(xz34, y34, x11y11z10nSelbMask);
						conv2 = si_rotqbyi(conv1, 4);
						conv4 = si_rotqbyi(conv3, 4);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
				else // faster loop with one load/shufb/store block per vertex
				{
					do
					{
						isSrcBad = si_cgt(pAttrSrc, pAdjustedUniformEnd);
						qword finalDestOffset1 = si_selb(destOffsets, safeOffsets, isSrcBad);
						qword finalDestOffset2 = si_rotqbyi(finalDestOffset1, 4);
						qword finalDestOffset3 = si_rotqbyi(finalDestOffset1, 8);
						qword finalDestOffset4 = si_rotqbyi(finalDestOffset1, 12);
						// load destination
						qword dest1a = si_lqx(pAttrDest1, finalDestOffset1);
						qword dest2a = si_lqx(pAttrDest1, finalDestOffset2);
						qword dest3a = si_lqx(pAttrDest1, finalDestOffset3);
						qword dest4a = si_lqx(pAttrDest1, finalDestOffset4);
						// insert and gather
						qword out1a = si_shufb(conv1, dest1a, insertAndGatherMask1);
						qword out2a = si_shufb(conv2, dest2a, insertAndGatherMask1);
						qword out3a = si_shufb(conv3, dest3a, insertAndGatherMask1);
						qword out4a = si_shufb(conv4, dest4a, insertAndGatherMask1);
						// store
						si_stqx(out1a, pAttrDest1, finalDestOffset1);
						si_stqx(out2a, pAttrDest1, finalDestOffset2);
						si_stqx(out3a, pAttrDest1, finalDestOffset3);
						si_stqx(out4a, pAttrDest1, finalDestOffset4);
						// load source
						in1 = si_lqd(pAttrSrc, 0x000);
						in2 = si_lqx(pAttrSrc, srcOffset2);
						in3 = si_lqx(pAttrSrc, srcOffset3);
						in4 = si_lqx(pAttrSrc, srcOffset4);
						// convert
						temp1 = si_rotm(si_cflts(si_fma(in1, x11y11z10nScale, x11y11z10nBias),
								31), x11y11z10nBitShift);
						temp2 = si_rotm(si_cflts(si_fma(in2, x11y11z10nScale, x11y11z10nBias),
								31), x11y11z10nBitShift);
						temp3 = si_rotm(si_cflts(si_fma(in3, x11y11z10nScale, x11y11z10nBias),
								31), x11y11z10nBitShift);
						temp4 = si_rotm(si_cflts(si_fma(in4, x11y11z10nScale, x11y11z10nBias),
								31), x11y11z10nBitShift);
						xz12 = si_shufb(temp1, temp2, x11y11z10nXzMask);
						xz34 = si_shufb(temp3, temp4, x11y11z10nXzMask);
						y12  = si_shufb(temp1, temp2, x11y11z10nYMask);
						y34  = si_shufb(temp3, temp4, x11y11z10nYMask);
						conv1 = si_selb(xz12, y12, x11y11z10nSelbMask);
						conv3 = si_selb(xz34, y34, x11y11z10nSelbMask);
						conv2 = si_rotqbyi(conv1, 4);
						conv4 = si_rotqbyi(conv3, 4);
						// increment
						destOffsets = si_a(destOffsets, destOffsetsInc);
						pAttrSrc = si_a(pAttrSrc, pAttrSrcInc);
					}
					while(si_to_int(isSrcBad) == 0);
				}
			}
			break;
		} // end switch
	} // end attribute loop
	
	return outVertexes;
}
#endif

uint32_t edgeGeomGetOutputVertexStride(EdgeGeomSpuContext *ctx, uint32_t outputFormatId)
{
	switch(outputFormatId)
	{
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3:
		return 12;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N:
		return 20;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_I16Nc4:
		return 24;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N:
		return 24;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_EMPTY:
		return 0;
	default:
		if(ctx->customFormatInfo.outputStrideCallback != 0)
			return (*ctx->customFormatInfo.outputStrideCallback)(ctx, outputFormatId);
		else if (ctx->customFormatInfo.outputStreamDesc != 0)
			return ctx->customFormatInfo.outputStreamDesc->stride;
	}
	return 0; // error!
}

void *edgeGeomCompressVertexes(EdgeGeomSpuContext *ctx)
{
	// Re-compressing vertex data is forbidden in the static geometry fast path; you shouldn't
	// be outputting any vertex data at all!
	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH) {
		return 0;
	}

	void *outVertexes = edgeGeomGetFreePtr(ctx);
	ctx->vertexesLs = (uint32_t)outVertexes;

	if(ctx->spuConfigInfo.numVertexes == 0)
	{
		return outVertexes;
	}

	// This sets up some global decompression structures which are
	// independent of the specific vertex format.  To reduce code
	// bloat, it's done once outside the loop instead of being
	// duplicated inside each case block.
	EDGE_GEOM_COMPRESS_INIT_GLOBAL_COMMON(ctx, ctx->spuConfigInfo.numVertexes, outVertexes);
		
	switch(ctx->spuConfigInfo.outputVertexFormatId)
	{
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3:
		{
			EDGE_GEOM_COMPRESS_SPECIALCASE_F32_3(ctx, ctx->spuConfigInfo.numVertexes, outVertexes);
		}
		break;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(20);
			
			EDGE_GEOM_COMPRESS_INIT_F32(EDGE_GEOM_ATTRIBUTE_ID_POSITION, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_NORMAL, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_TANGENT, 16, 4);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(EDGE_GEOM_ATTRIBUTE_ID_POSITION);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_NORMAL);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_TANGENT);
				
				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_I16Nc4:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(24);
			
			EDGE_GEOM_COMPRESS_INIT_F32(EDGE_GEOM_ATTRIBUTE_ID_POSITION, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_NORMAL, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_I16N(EDGE_GEOM_ATTRIBUTE_ID_TANGENT, 16, 8);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(EDGE_GEOM_ATTRIBUTE_ID_POSITION);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_NORMAL);
				EDGE_GEOM_COMPRESS_LOOP_I16N(EDGE_GEOM_ATTRIBUTE_ID_TANGENT);
				
				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N:
		{
			EDGE_GEOM_COMPRESS_INIT_GLOBAL(24);
			
			EDGE_GEOM_COMPRESS_INIT_F32(EDGE_GEOM_ATTRIBUTE_ID_POSITION, 0, 12);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_NORMAL, 12, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_TANGENT, 16, 4);
			EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_BINORMAL, 20, 4);
			
			do
			{
				EDGE_GEOM_COMPRESS_LOOP_START();
				
				EDGE_GEOM_COMPRESS_LOOP_F32(EDGE_GEOM_ATTRIBUTE_ID_POSITION);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_NORMAL);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_TANGENT);
				EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(EDGE_GEOM_ATTRIBUTE_ID_BINORMAL);
				
				EDGE_GEOM_COMPRESS_LOOP_END();
			} while(!EDGE_GEOM_COMPRESS_LOOP_DONE());
		}
		break;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_EMPTY:
		break;
	default:
		{
			// Callback takes precedence over stream description.  If
			// the callback returns non-zero, it will fall back on the
			// stream description.
			bool success = false;
			if(ctx->customFormatInfo.compressOutputCallback != 0)
			{
				uint32_t err = (*ctx->customFormatInfo.compressOutputCallback)(ctx,
					ctx->spuConfigInfo.numVertexes,	outVertexes,
					ctx->spuConfigInfo.outputVertexFormatId);
				success = (err == 0);
			}
#if ENABLE_COMPRESS_BY_DESCRIPTION
			if (!success && ctx->customFormatInfo.outputStreamDesc != 0)
			{
				CompressVertexesByDescription(ctx, ctx->customFormatInfo.outputStreamDesc);
				success = true;
			}
#endif // ENABLE_COMPRESS_BY_DESCRIPTION
			if (!success)
			{
				EDGE_ASSERT_MSG( false,
					("EDGE error: unknown output vertex format ID %d for vertex stream",
						ctx->spuConfigInfo.outputVertexFormatId) );
			}
			break;
		}
	}

	edgeGeomSetFreePtr(ctx, (void*)((uint32_t)edgeGeomGetFreePtr(ctx) + ((edgeGeomGetOutputVertexStride(ctx, ctx->spuConfigInfo.outputVertexFormatId) * ctx->spuConfigInfo.numVertexes + 0xF) & ~0xF)));

	return outVertexes;
}

