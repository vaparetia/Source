/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

.if DXT_MODE == 1
.cfunc void edgeDxtCompress1(void* outputBlocks, const void* inputPixels, uint32_t rowStride, uint32_t blockCount)
.elif DXT_MODE == 0x1a
.cfunc void edgeDxtCompress1a(void* outputBlocks, const void* inputPixels, uint32_t rowStride, uint32_t blockCount)
.elif DXT_MODE == 3
.cfunc void edgeDxtCompress3(void* outputBlocks, const void* inputPixels, uint32_t rowStride, uint32_t blockCount)
.else
.cfunc void edgeDxtCompress5(void* outputBlocks, const void* inputPixels, uint32_t rowStride, uint32_t blockCount)
.endif

	// early out if blockCount is zero
	brz		blockCount, end

	// shuffle masks
	.reg 	shufABab, shufCDcd
	il128 	shufABab, "ABab"
	il128 	shufCDcd, "CDcd"

	.reg	shufBADC
	il128	shufBADC, "BADC"

	.reg	mask0011
	il128	mask0011, 0x00000000_00000000_ffffffff_ffffffff

	.reg	shufGatherRRGG, shufGatherBBAA
	il128	shufGatherRRGG, 0x0105090d_1115191d_02060a0e_12161a1e
	il128	shufGatherBBAA, 0x03070b0f_13171b1f_0004080c_1014181c

	.reg	shufGrbgSum0, shufGrbgSum1, shufGrbgSum2, shufGrbgSum3
	il128	shufGrbgSum0, 0x80800001_80801011_80800203_80801011
	il128	shufGrbgSum1, 0x80800405_80801415_80800607_80801415
	il128	shufGrbgSum2, 0x80800809_80801819_80800a0b_80801819
	il128	shufGrbgSum3, 0x80800c0d_80801c1d_80800e0f_80801c1d

	.reg 	shufB0B0
	il128	shufB0B0, "B0B0"

	.reg	shufEvenBytes, shufOddBytes
	il128	shufEvenBytes, 0x80008002_80048006_8008800a_800c800e
	il128	shufOddBytes, 0x80018003_80058007_8009800b_800d800f

	.reg	shufFlipMaxMinGB, shufFlipMaxMinMask
	il128	shufFlipMaxMinGB, 0x00010a0b_04050e0f_08090203_0c0d0607
	il128	shufFlipMaxMinMask, 0x80800008_80800008_80800008_80800008

	.reg	shufGatherMax565Bits, shufGatherMin565Bits, mask565Bits
	il128	shufGatherMax565Bits, 0x01010101_01020202_02020203_03030303
	il128	shufGatherMin565Bits, 0x09090909_090a0a0a_0a0a0a0b_0b0b0b0b
	il128	mask565Bits, 0x80402010_08804020_10080480_40201008

	.reg	shufBytesForLower565, shufBytesForUpper565, maskLower565
	il128	shufBytesForLower565, 0x80808010_80018012_80808018_8009801a
	il128	shufBytesForUpper565, 0x80808001_80028003_80808009_800a800b
	il128	maskLower565, 0x00000007_00030007_00000007_00030007

	.reg	shufBBBBBBBB
	il128	shufBBBBBBBB, "BBBBBBBB"

	.reg	magicDiv3
	ilh		magicDiv3, 21846	// ( 1 << 16 )/3 + 1

	.reg	shufReplicateByte03, shufReplicateByte05, shufReplicateByte07
	.reg	shufReplicateByte0b, shufReplicateByte0d, shufReplicateByte0f
	.reg	shufReplicateByte11, shufReplicateByte15, shufReplicateByte19, shufReplicateByte1d
	ilh		shufReplicateByte03, 0x0303
	ilh		shufReplicateByte05, 0x0505
	ilh		shufReplicateByte07, 0x0707
	ilh		shufReplicateByte0b, 0x0b0b
	ilh		shufReplicateByte0d, 0x0d0d
	ilh		shufReplicateByte0f, 0x0f0f
	ilh		shufReplicateByte11, 0x1111
	ilh		shufReplicateByte15, 0x1515
	ilh		shufReplicateByte19, 0x1919
	ilh		shufReplicateByte1d, 0x1d1d

	.reg	shufGatherDiffRow0RB, shufGatherDiffRow0GG
	.reg	shufGatherDiffRow1RB, shufGatherDiffRow1GG
	.reg	shufGatherDiffRow2RB, shufGatherDiffRow2GG
	.reg	shufGatherDiffRow3RB, shufGatherDiffRow3GG
	il128	shufGatherDiffRow0RB, 0x00108080_01118080_02128080_03138080
	il128	shufGatherDiffRow1RB, 0x04148080_05158080_06168080_07178080
	il128	shufGatherDiffRow2RB, 0x08188080_09198080_0a1a8080_0b1b8080
	il128	shufGatherDiffRow3RB, 0x0c1c8080_0d1d8080_0e1e8080_0f1f8080
	il128	shufGatherDiffRow0GG, 0x00011010_04051111_08091212_0c0d1313
	il128	shufGatherDiffRow1GG, 0x00011414_04051515_08091616_0c0d1717
	il128	shufGatherDiffRow2GG, 0x00011818_04051919_08091a1a_0c0d1b1b
	il128	shufGatherDiffRow3GG, 0x00011c1c_04051d1d_08091e1e_0c0d1f1f

	.reg	shufGatherSumbResults, shufGatherIndexBits0, shufGatherIndexBits1 : readoptional
	il128	shufGatherSumbResults, 0x0105090d_03070b0f_1115191d_13171b1f
	il128	shufGatherIndexBits0, 0x03130212_01110010_07170616_05150414
	il128	shufGatherIndexBits1, 0x0b1b0a1a_09190818_0f1f0e1e_0d1d0c1c

	.reg	shufMakeDxtColourBlock, shufMakeDxtIndexBlock
.if (DXT_MODE == 1) || (DXT_MODE == 0x1a)
	il128	shufMakeDxtColourBlock, 0x03021312_80808080_80808080_80808080
	il128 	shufMakeDxtIndexBlock, "00Bb0000"
.else
	il128	shufMakeDxtColourBlock, 0x80808080_80808080_03021312_80808080
	il128 	shufMakeDxtIndexBlock, "000000Bb"
.endif

.if DXT_MODE == 0x1a
	.reg	shufDxt1aIndex0, shufDxt1aIndex1, shufCenterEven, shufCenterOdd
	.reg	shufRow0RG, shufRow1RG, shufRow2RG, shufRow3RG
	.reg	shufRow0B, shufRow1B, shufRow2B, shufRow3B
	.reg	shufReplicateCandidate, shufInvertDxt1a
	.reg	code00, code01, code10, code11
	il128	shufDxt1aIndex0, "DDCCBBAAHHGGFFEE"
	il128	shufDxt1aIndex1, "LLKKJJIIPPOONNMM"
	il128	shufCenterEven, "00000F0000000F00"
	il128	shufCenterOdd,  "0D000H000D000H00"
	il128	shufRow0RG, "0aA00bB00cC00dD0"
	il128	shufRow1RG, "0eE00fF00gG00hH0"
	il128	shufRow2RG, "0iI00jJ00kK00lL0"
	il128	shufRow3RG, "0mM00nN00oO00pP0"
	il128	shufRow0B, "abcAefgBijkCmnoD"
	il128	shufRow1B, "abcEefgFijkGmnoH"
	il128	shufRow2B, "abcIefgJijkKmnoL"
	il128	shufRow3B, "abcMefgNijkOmnoP"
	il128	shufReplicateCandidate, "AAAAAAAAAAAAAAAA"
	il128	shufInvertDxt1a, "gecahfdb"
	ilh		code00, 0
	ilh		code01, 1
	ilh		code10, 0x100
	ilh		code11, 0x101
.endif

.if DXT_MODE == 3
	.reg 	gatherAlphaBlockLo, gatherAlphaBlockHi, selectAlphaBlockLoHi
	il128	gatherAlphaBlockLo, 0x0f010305_07090b0d_80808080_80808080
	il128	gatherAlphaBlockHi, 0x01030507_090b0d0f_80808080_80808080
	ilh 	selectAlphaBlockLoHi, 0xf0f0
.endif

.if DXT_MODE == 5
	.reg	gatherAlphaMax, gatherAlphaMin
	il128	gatherAlphaMax, 0x80808000_80808000_80808000_80808000
	il128	gatherAlphaMin, 0x80808008_80808008_80808008_80808008

	.reg	gatherAlpha
	il128 	gatherAlpha, 0x80808000_80808004_80808008_8080800c

	.reg	float7, float0_5
	il128	float7, 0x40E00000_40E00000_40E00000_40E00000
	il128	float0_5, 0x3F000000_3F000000_3F000000_3F000000

	.reg 	gatherAlphaIndices01
	.reg	gatherAlphaIndices23
	il128	gatherAlphaIndices01, 0x03070b0f_13171b1f_80808080_80808080
	il128	gatherAlphaIndices23, 0x80808080_80808080_03070b0f_13171b1f

	.reg	alphaIndexRemap
	il128	alphaIndexRemap, 0x01070605_04030200_00000000_00000000

	.reg	alphaIndexBitGather0, alphaIndexBitGather1, alphaIndexBitGather2
	il128	alphaIndexBitGather0, 0x02020101_01000000_05040404_03030302
	il128	alphaIndexBitGather1, 0x07070706_06060505_0a0a0909_09080808
	il128	alphaIndexBitGather2, 0x0d0c0c0c_0b0b0b0a_0f0f0f0e_0e0e0d0d

	.reg	alphaIndexBitMask0, alphaIndexBitMask1, alphaIndexBitMask2
	il128	alphaIndexBitMask0, 0x02010402_01040201_01040201_04020104
	il128	alphaIndexBitMask1, 0x04020104_02010402_02010402_01040201
	il128	alphaIndexBitMask2, 0x01040201_04020104_04020104_02010402

	.reg	gatherAlphaMaxMin
	il128	gatherAlphaMaxMin, 0x00088080_80808080_80808080_80808080
.endif

	// precompute the load offsets
	.reg	rowStride2, rowStride3
	a		rowStride2, rowStride, rowStride
	a		rowStride3, rowStride2, rowStride
	
	// shuffle mask to invert red and green
	.reg	identityShuffle, swapRedWithGreenShuffle
	il128	identityShuffle, "abcdefghijklmnop"
	il128	swapRedWithGreenShuffle, "acbdegfhikjlmonp"
	
loop:
	// load the input data as ARGB
	.reg 	row0, row1, row2, row3
	lqd		row0, 0x00( inputPixels )
	lqx		row1, rowStride, inputPixels
	lqx		row2, rowStride2, inputPixels
	lqx		row3, rowStride3, inputPixels

	// shuffle the input rows into replicated values
	.reg	tempGather0, tempGather1, tempGather2, tempGather3
	.reg 	red, gre, blu
	shufb	tempGather0, row0, row1, shufGatherRRGG
	shufb	tempGather1, row2, row3, shufGatherRRGG
	shufb	tempGather2, row0, row1, shufGatherBBAA
	shufb	tempGather3, row2, row3, shufGatherBBAA
	shufb	red, tempGather0, tempGather1, shufABab
	shufb	gre, tempGather0, tempGather1, shufCDcd
	shufb	blu, tempGather2, tempGather3, shufABab

.if (DXT_MODE == 3) || (DXT_MODE == 0x1a)
	.reg	alpha
	shufb	alpha, tempGather2, tempGather3, shufCDcd
.endif

.if DXT_MODE == 0x1a

	// check if any pixel of the 4x4 block is transparent
	.reg	dxt1PixelSelector, dxt1aBlockType, dxt1aIndexMask, t0
	clgtbi	dxt1PixelSelector, alpha, 0x7f
	gbb		t0, dxt1PixelSelector
	ceqhi	dxt1aBlockType, t0, -1
	shufb	dxt1aBlockType, dxt1aBlockType, dxt1aBlockType, shufBBBBBBBB
	
	// build index mask for transparent pixels
	.reg	A0, A1
	nor		A1, dxt1PixelSelector, dxt1PixelSelector
	shufb	A0, A1, A1, shufDxt1aIndex0
	shufb	A1, A1, A1, shufDxt1aIndex1
	gbb		A0, A0
	gbb		A1, A1
	shufb	dxt1aIndexMask, A0, A1, shufMakeDxtIndexBlock

	// find a valid opaque pixel and replicate it across all transparent pixels
	.reg	redm, blum, grem
	clz		t0, t0
	rotqby	redm, red, t0
	rotqby	grem, gre, t0
	rotqby	blum, blu, t0
	shufb	redm, redm, redm, shufReplicateCandidate
	shufb	grem, grem, grem, shufReplicateCandidate
	shufb	blum, blum, blum, shufReplicateCandidate
	selb	red, redm, red, dxt1PixelSelector
	selb	gre, grem, gre, dxt1PixelSelector
	selb	blu, blum, blu, dxt1PixelSelector
	
	// reshuffle back into rows (this will destroy alpha information, not a problem as it's not used anymore)
	shufb	row0, gre, red, shufRow0RG
	shufb	row1, gre, red, shufRow1RG
	shufb	row2, gre, red, shufRow2RG
	shufb	row3, gre, red, shufRow3RG
	shufb	row0, blu, row0, shufRow0B
	shufb	row1, blu, row1, shufRow1B
	shufb	row2, blu, row2, shufRow2B
	shufb	row3, blu, row3, shufRow3B
.endif

	// test whether all red components are equals as it creates a singularity when calculating sample correlation between r.g and r.b
	.reg	redAllEqual
	ila		redAllEqual, 0
	shufb	redAllEqual, red, red, redAllEqual
	ceqb	redAllEqual, redAllEqual, red
	gbb		redAllEqual, redAllEqual
	ceqhi	redAllEqual, redAllEqual, -1
	shufb	redAllEqual, redAllEqual, redAllEqual, shufBBBBBBBB
	
	// get colour range between rows
	.reg	rangeMask0, rangeMask1
	.reg	rowMinA, rowMinB, rowMaxA, rowMaxB
	.reg	rangeMask2, rangeMask3
	.reg	rowMin, rowMax
	clgtb	rangeMask0, row0, row1
	clgtb	rangeMask1, row2, row3
	selb	rowMinA, row0, row1, rangeMask0
	selb	rowMinB, row2, row3, rangeMask1
	selb	rowMaxA, row1, row0, rangeMask0
	selb	rowMaxB, row3, row2, rangeMask1
	clgtb	rangeMask2, rowMinA, rowMinB
	clgtb	rangeMask3, rowMaxA, rowMaxB
	selb	rowMin, rowMinA, rowMinB, rangeMask2
	selb	rowMax, rowMaxB, rowMaxA, rangeMask3

	// get colour range within the final row
	.reg	tempMax0Max1Min0Min1, tempMax2Max3Min2Min3
	.reg	rangeMask4, rangeMask5
	.reg	tempMax4Max5Min4Min5, tempMax5Max4Min5Min4
	.reg	rangeMask6, rangeMask7
	.reg	candidateMaxMaxMinMin
	shufb	tempMax0Max1Min0Min1, rowMax, rowMin, shufABab
	shufb	tempMax2Max3Min2Min3, rowMax, rowMin, shufCDcd
	clgtb	rangeMask4, tempMax0Max1Min0Min1, tempMax2Max3Min2Min3
	xor		rangeMask5, rangeMask4, mask0011
	selb	tempMax4Max5Min4Min5, tempMax2Max3Min2Min3, tempMax0Max1Min0Min1, rangeMask5
	shufb	tempMax5Max4Min5Min4, tempMax4Max5Min4Min5, tempMax4Max5Min4Min5, shufBADC
	clgtb	rangeMask6, tempMax4Max5Min4Min5, tempMax5Max4Min5Min4
	xor		rangeMask7, rangeMask6, mask0011
	selb	candidateMaxMaxMinMin, tempMax5Max4Min5Min4, tempMax4Max5Min4Min5, rangeMask7

.if DXT_MODE == 0x1a

	// eliminate transparent pixels from covariance calculation
	// NOTE: virtual register code00 happens to be exactly the value we want (a splatted 0 over all bytes)
	selb	red, code00, red, dxt1PixelSelector
	selb	gre, code00, gre, dxt1PixelSelector
	selb	blu, code00, blu, dxt1PixelSelector
.endif

	// If all 16 red components are equals than we swap red with green (effectively choosing a different set of axis)
	.reg	saved_red, saved_gre
	mov		saved_red, red
	mov		saved_gre, gre
	selb	red, red, saved_gre, redAllEqual
	selb	gre, gre, saved_red, redAllEqual
	
	// We also need to swap red and green inside candidateMaxMaxMinMin
	.reg	edgeCaseShuffleMask
	selb	edgeCaseShuffleMask, identityShuffle, swapRedWithGreenShuffle, redAllEqual
	shufb	candidateMaxMaxMinMin, candidateMaxMaxMinMin, candidateMaxMaxMinMin, edgeCaseShuffleMask
	
	// sum across red, green and blue
	.reg	sumbRedRed, sumbGreenBlue
	.reg	grbrSumTemp0, grbrSumTemp1, grbrSumTemp2, grbrSumTemp3
	.reg	grbrSumTemp4, grbrSumTemp5
	.reg	grbrSum
	sumb	sumbRedRed, red, red
	sumb	sumbGreenBlue, blu, gre
	shufb	grbrSumTemp0, sumbGreenBlue, sumbRedRed, shufGrbgSum0
	shufb	grbrSumTemp1, sumbGreenBlue, sumbRedRed, shufGrbgSum1
	shufb	grbrSumTemp2, sumbGreenBlue, sumbRedRed, shufGrbgSum2
	shufb	grbrSumTemp3, sumbGreenBlue, sumbRedRed, shufGrbgSum3
	a		grbrSumTemp4, grbrSumTemp0, grbrSumTemp1
	a		grbrSumTemp5, grbrSumTemp2, grbrSumTemp3
	a		grbrSum, grbrSumTemp4, grbrSumTemp5

	// compute rsum*gsum and rsum*bsum (use 16bit mul since we still can)
	.reg	r0r0Sum
	.reg	prodOfSums
	shufb 	r0r0Sum, grbrSum, grbrSum, shufB0B0
	mpyu	prodOfSums, grbrSum, r0r0Sum	

.if DXT_MODE == 0x1a

	// eliminate transparent pixels from covariance calculation
	// NOTE: virtual register code11 happens to be exactly the value we want (a splatted 1 over all bytes)
	selb	red, code11, red, dxt1PixelSelector
	selb	gre, code11, gre, dxt1PixelSelector
	selb	blu, code11, blu, dxt1PixelSelector
.endif

	// do the dot product of red.gre and red.blu for the sum of products
	.reg	redEven, redOdd, greEven, greOdd, bluEven, bluOdd
	.reg	redGreenSumTemp0, redBlueSumTemp0
	.reg	redGreenSumTemp1, redBlueSumTemp1
	.reg	redGreenSumTemp2, redBlueSumTemp2
	.reg	redGreenSumTemp3, redBlueSumTemp3
	.reg	redGreenSumTemp4, redBlueSumTemp4
	.reg	redGreenSumTemp5, redBlueSumTemp5
	.reg	redGreenSumTemp6, redBlueSumTemp6
	.reg	sumOfProdsTemp0, sumOfProdsTemp1, sumOfProdsTemp2, sumOfProdsTemp3
	.reg	sumOfProds, sumOfProds16
	shufb	redEven, red, red, shufEvenBytes
	shufb	greEven, gre, gre, shufEvenBytes
	shufb	bluEven, blu, blu, shufEvenBytes
	shufb	redOdd, red, red, shufOddBytes
	shufb	greOdd, gre, gre, shufOddBytes
	shufb	bluOdd, blu, blu, shufOddBytes
	mpyhhu	redGreenSumTemp0, redEven, greEven
	mpyu	redGreenSumTemp1, redEven, greEven
	mpyhhu	redGreenSumTemp2, redOdd, greOdd
	mpyu	redGreenSumTemp3, redOdd, greOdd
	a		redGreenSumTemp4, redGreenSumTemp0, redGreenSumTemp1
	a		redGreenSumTemp5, redGreenSumTemp2, redGreenSumTemp3
	a		redGreenSumTemp6, redGreenSumTemp4, redGreenSumTemp5
	mpyhhu	redBlueSumTemp0, redEven, bluEven
	mpyu	redBlueSumTemp1, redEven, bluEven
	mpyhhu	redBlueSumTemp2, redOdd, bluOdd
	mpyu	redBlueSumTemp3, redOdd, bluOdd
	a		redBlueSumTemp4, redBlueSumTemp0, redBlueSumTemp1
	a		redBlueSumTemp5, redBlueSumTemp2, redBlueSumTemp3
	a		redBlueSumTemp6, redBlueSumTemp4, redBlueSumTemp5
	shufb	sumOfProdsTemp0, redGreenSumTemp6, redBlueSumTemp6, shufABab
	shufb	sumOfProdsTemp1, redGreenSumTemp6, redBlueSumTemp6, shufCDcd
	a		sumOfProdsTemp2, sumOfProdsTemp0, sumOfProdsTemp1
	shufb	sumOfProdsTemp3, sumOfProdsTemp2, sumOfProdsTemp2, shufBADC
	a		sumOfProds, sumOfProdsTemp2, sumOfProdsTemp3
	roti	sumOfProds16, sumOfProds, 4

	// flip green and blue
	.reg	flippedMaxMaxMinMin
	shufb	flippedMaxMaxMinMin, candidateMaxMaxMinMin, candidateMaxMaxMinMin, shufFlipMaxMinGB
	
	// do the compare to get the covariance
	.reg	covarianceCompare
	clgt	covarianceCompare, prodOfSums, sumOfProds16

	// switch green and/or blue max and min based on covariance
	.reg	flipMask, orderedMaxMaxMinMin
	shufb	flipMask, covarianceCompare, covarianceCompare, shufFlipMaxMinMask
	selb	orderedMaxMaxMinMin, candidateMaxMaxMinMin, flippedMaxMaxMinMin, flipMask

	// we might need to swap red and green back to previous state
	mov		red, saved_red
	mov		gre, saved_gre
	shufb	orderedMaxMaxMinMin, orderedMaxMaxMinMin, orderedMaxMaxMinMin, edgeCaseShuffleMask
	
	// gather the final endpoint into a 565 value
	.reg 	finalMax565Temp0, finalMin565Temp0
	.reg 	finalMax565Temp1, finalMin565Temp1
	.reg 	finalMax565Temp2, finalMin565Temp2
	.reg	dxtBlockCand0, dxtBlockCand1
	.reg	dxtBlock0, dxtBlock1
	shufb	finalMax565Temp0, orderedMaxMaxMinMin, orderedMaxMaxMinMin, shufGatherMax565Bits
	shufb 	finalMin565Temp0, orderedMaxMaxMinMin, orderedMaxMaxMinMin, shufGatherMin565Bits
	and		finalMax565Temp1, finalMax565Temp0, mask565Bits
	and		finalMin565Temp1, finalMin565Temp0, mask565Bits
	ceqb	finalMax565Temp2, finalMax565Temp1, mask565Bits
	ceqb	finalMin565Temp2, finalMin565Temp1, mask565Bits
	gbb		dxtBlockCand0, finalMax565Temp2
	gbb		dxtBlockCand1, finalMin565Temp2

	// compare the endpoint value
	.reg	needReorderTemp, needReorder
	clgth	needReorderTemp, dxtBlockCand1, dxtBlockCand0
	shufb	needReorder, needReorderTemp, needReorderTemp, shufBBBBBBBB

.if (DXT_MODE == 0x1a)
	
	// invert reorder logic if we are dealing with a 3-color block
	nor		needReorderTemp, needReorder, needReorder
	selb	needReorder, needReorderTemp, needReorder, dxt1aBlockType
.endif

	// reorder the endpoints if necessary
	.reg	reversedMaxMaxMinMin, finalMaxMaxMinMin
	rotqbyi	reversedMaxMaxMinMin, orderedMaxMaxMinMin, 8
	selb	finalMaxMaxMinMin, orderedMaxMaxMinMin, reversedMaxMaxMinMin, needReorder
	selb	dxtBlock0, dxtBlockCand0, dxtBlockCand1, needReorder
	selb	dxtBlock1, dxtBlockCand1, dxtBlockCand0, needReorder

	// project the final endpoint into the value expanded from 565 precision, store as ushort
	.reg	finalMaxMaxMinMinRot2, finalMaxMaxMinMinRot3
	.reg	projectedMaxMinLower, projectedMaxMinUpper
	.reg	projectedMaxMin
	rotqbii	finalMaxMaxMinMinRot2, finalMaxMaxMinMin, 2
	rotqbii	finalMaxMaxMinMinRot3, finalMaxMaxMinMin, 3
	shufb	projectedMaxMinLower, finalMaxMaxMinMinRot2, finalMaxMaxMinMinRot3, shufBytesForLower565
	shufb	projectedMaxMinUpper, finalMaxMaxMinMin, finalMaxMaxMinMin, shufBytesForUpper565
	selb	projectedMaxMin, projectedMaxMinUpper, projectedMaxMinLower, maskLower565

	// compute ushort intermediate colours
	.reg	projectedMaxMinX2, projectedMinMax
	.reg	interpMid0Mid1X3, interpMid0Mid1Even, interpMid0Mid1Odd
	ah		projectedMaxMinX2, projectedMaxMin, projectedMaxMin
	rotqbyi	projectedMinMax, projectedMaxMin, 8
	ah		interpMid0Mid1X3, projectedMinMax, projectedMaxMinX2
	mpyhhu	interpMid0Mid1Even, interpMid0Mid1X3, magicDiv3
	mpyu	interpMid0Mid1Odd, interpMid0Mid1X3, magicDiv3


.if DXT_MODE == 0x1a
	
	// if this is a 3-color block than midpoints will be as follow :
	// Mid1 = Mid0 = (Min+Max)>>1
	.reg	interpCenter, interpCenterEven, interpCenterOdd
	ah		interpCenter, projectedMinMax, projectedMaxMin
	rothmi	interpCenter, interpCenter, -1
	shufb	interpCenterEven, interpCenter, interpCenter, shufCenterEven
	shufb	interpCenterOdd, interpCenter, interpCenter, shufCenterOdd
	selb	interpMid0Mid1Even, interpCenterEven, interpMid0Mid1Even, dxt1aBlockType
	selb	interpMid0Mid1Odd, interpCenterOdd, interpMid0Mid1Odd, dxt1aBlockType
.endif

	// shuffle palette into replicated values
	.reg 	red0, gre0, blu0
	.reg 	red1, gre1, blu1
	.reg 	red2, gre2, blu2
	.reg 	red3, gre3, blu3
	shufb	red0, projectedMaxMin, projectedMaxMin, shufReplicateByte03
	shufb	gre0, projectedMaxMin, projectedMaxMin, shufReplicateByte05
	shufb	blu0, projectedMaxMin, projectedMaxMin, shufReplicateByte07
	shufb	red1, projectedMaxMin, projectedMaxMin, shufReplicateByte0b
	shufb	gre1, projectedMaxMin, projectedMaxMin, shufReplicateByte0d
	shufb	blu1, projectedMaxMin, projectedMaxMin, shufReplicateByte0f
	shufb	red2, interpMid0Mid1Even, interpMid0Mid1Odd, shufReplicateByte11
	shufb	gre2, interpMid0Mid1Even, interpMid0Mid1Odd, shufReplicateByte05
	shufb	blu2, interpMid0Mid1Even, interpMid0Mid1Odd, shufReplicateByte15
	shufb	red3, interpMid0Mid1Even, interpMid0Mid1Odd, shufReplicateByte19
	shufb	gre3, interpMid0Mid1Even, interpMid0Mid1Odd, shufReplicateByte0d
	shufb	blu3, interpMid0Mid1Even, interpMid0Mid1Odd, shufReplicateByte1d

	// compute absolute differences
	.reg	diffr0, diffr1, diffr2, diffr3
	.reg	diffg0, diffg1, diffg2, diffg3
	.reg	diffb0, diffb1, diffb2, diffb3
	absdb	diffr0, red, red0
	absdb	diffg0, gre, gre0
	absdb	diffb0, blu, blu0
	absdb	diffr1, red, red1
	absdb	diffg1, gre, gre1
	absdb	diffb1, blu, blu1
	absdb	diffr2, red, red2
	absdb	diffg2, gre, gre2
	absdb	diffb2, blu, blu2
	absdb	diffr3, red, red3
	absdb	diffg3, gre, gre3
	absdb	diffb3, blu, blu3

	// shuffle the errors back into rows for summation (green counted twice intentionally)
	.reg	row0Diff0RB, row0Diff1RB, row0Diff2RB, row0Diff3RB
	.reg	row1Diff0RB, row1Diff1RB, row1Diff2RB, row1Diff3RB
	.reg	row2Diff0RB, row2Diff1RB, row2Diff2RB, row2Diff3RB
	.reg	row3Diff0RB, row3Diff1RB, row3Diff2RB, row3Diff3RB
	.reg	row0Diff0, row0Diff1, row0Diff2, row0Diff3
	.reg	row1Diff0, row1Diff1, row1Diff2, row1Diff3
	.reg	row2Diff0, row2Diff1, row2Diff2, row2Diff3
	.reg	row3Diff0, row3Diff1, row3Diff2, row3Diff3
	shufb	row0Diff0RB, diffr0, diffb0, shufGatherDiffRow0RB	
	shufb	row0Diff1RB, diffr1, diffb1, shufGatherDiffRow0RB	
	shufb	row0Diff2RB, diffr2, diffb2, shufGatherDiffRow0RB	
	shufb	row0Diff3RB, diffr3, diffb3, shufGatherDiffRow0RB	
	shufb	row1Diff0RB, diffr0, diffb0, shufGatherDiffRow1RB	
	shufb	row1Diff1RB, diffr1, diffb1, shufGatherDiffRow1RB	
	shufb	row1Diff2RB, diffr2, diffb2, shufGatherDiffRow1RB	
	shufb	row1Diff3RB, diffr3, diffb3, shufGatherDiffRow1RB	
	shufb	row2Diff0RB, diffr0, diffb0, shufGatherDiffRow2RB	
	shufb	row2Diff1RB, diffr1, diffb1, shufGatherDiffRow2RB	
	shufb	row2Diff2RB, diffr2, diffb2, shufGatherDiffRow2RB	
	shufb	row2Diff3RB, diffr3, diffb3, shufGatherDiffRow2RB	
	shufb	row3Diff0RB, diffr0, diffb0, shufGatherDiffRow3RB	
	shufb	row3Diff1RB, diffr1, diffb1, shufGatherDiffRow3RB	
	shufb	row3Diff2RB, diffr2, diffb2, shufGatherDiffRow3RB	
	shufb	row3Diff3RB, diffr3, diffb3, shufGatherDiffRow3RB	
	shufb	row0Diff0, row0Diff0RB, diffg0, shufGatherDiffRow0GG
	shufb	row0Diff1, row0Diff1RB, diffg1, shufGatherDiffRow0GG
	shufb	row0Diff2, row0Diff2RB, diffg2, shufGatherDiffRow0GG
	shufb	row0Diff3, row0Diff3RB, diffg3, shufGatherDiffRow0GG
	shufb	row1Diff0, row1Diff0RB, diffg0, shufGatherDiffRow1GG
	shufb	row1Diff1, row1Diff1RB, diffg1, shufGatherDiffRow1GG
	shufb	row1Diff2, row1Diff2RB, diffg2, shufGatherDiffRow1GG
	shufb	row1Diff3, row1Diff3RB, diffg3, shufGatherDiffRow1GG
	shufb	row2Diff0, row2Diff0RB, diffg0, shufGatherDiffRow2GG
	shufb	row2Diff1, row2Diff1RB, diffg1, shufGatherDiffRow2GG
	shufb	row2Diff2, row2Diff2RB, diffg2, shufGatherDiffRow2GG
	shufb	row2Diff3, row2Diff3RB, diffg3, shufGatherDiffRow2GG
	shufb	row3Diff0, row3Diff0RB, diffg0, shufGatherDiffRow3GG
	shufb	row3Diff1, row3Diff1RB, diffg1, shufGatherDiffRow3GG
	shufb	row3Diff2, row3Diff2RB, diffg2, shufGatherDiffRow3GG
	shufb	row3Diff3, row3Diff3RB, diffg3, shufGatherDiffRow3GG

	// sum across for the error terms
	.reg	row01Error0, row23Error0
	.reg	row01Error1, row23Error1
	.reg	row01Error2, row23Error2
	.reg	row01Error3, row23Error3
	sumb	row01Error0, row1Diff0, row0Diff0
	sumb	row01Error1, row1Diff1, row0Diff1
	sumb	row01Error2, row1Diff2, row0Diff2
	sumb	row01Error3, row1Diff3, row0Diff3
	sumb	row23Error0, row3Diff0, row2Diff0
	sumb	row23Error1, row3Diff1, row2Diff1
	sumb	row23Error2, row3Diff2, row2Diff2
	sumb	row23Error3, row3Diff3, row2Diff3


.if DXT_MODE != 0x1a

	// compute relations between error terms
	.reg	row01B0, row23B0
	.reg	row01B1, row23B1
	.reg	row01B2, row23B2
	.reg	row01B3, row23B3
	.reg	row01B4, row23B4
	.reg 	b0, b1, b2, b3, b4, x0, x1, x2, x3
	clgth	row01B0, row01Error0, row01Error3
	clgth	row01B1, row01Error1, row01Error2
	clgth	row01B2, row01Error0, row01Error2
	clgth	row01B3, row01Error1, row01Error3
	clgth	row01B4, row01Error2, row01Error3
	clgth	row23B0, row23Error0, row23Error3
	clgth	row23B1, row23Error1, row23Error2
	clgth	row23B2, row23Error0, row23Error2
	clgth	row23B3, row23Error1, row23Error3
	clgth	row23B4, row23Error2, row23Error3
	shufb	b0, row01B0, row23B0, shufGatherSumbResults
	shufb	b1, row01B1, row23B1, shufGatherSumbResults
	shufb	b2, row01B2, row23B2, shufGatherSumbResults
	shufb	b3, row01B3, row23B3, shufGatherSumbResults
	shufb	b4, row01B4, row23B4, shufGatherSumbResults
	and		x0, b1, b2
	and 	x1, b0, b3
	and		x2, b0, b4
	or		x3, x0, x1
	
	// construct the index blocks directly from the error terms
	.reg	indexBits0, indexBits1
	.reg	dxtBlock2, dxtBlock3
	shufb	indexBits0, x3, x2, shufGatherIndexBits0
	shufb	indexBits1, x3, x2, shufGatherIndexBits1
	gbb		dxtBlock2, indexBits0
	gbb		dxtBlock3, indexBits1
	
.else

	// compute the index palette from the error term
	.reg	dxtBlock2, dxtBlock3, c01, c23, e01, e23
	clgth	e01, row01Error0, row01Error1
	clgth	e23, row01Error2, row01Error3
	selb	c01, code00, code01, e01
	selb	c23, code10, code11, e23
	selb	e01, row01Error0, row01Error1, e01
	selb	e23, row01Error2, row01Error3, e23
	clgth	e23, e01, e23
	selb	dxtBlock2, c01, c23, e23
	clgth	e01, row23Error0, row23Error1
	clgth	e23, row23Error2, row23Error3
	selb	c01, code00, code01, e01
	selb	c23, code10, code11, e23
	selb	e01, row23Error0, row23Error1, e01
	selb	e23, row23Error2, row23Error3, e23
	clgth	e23, e01, e23
	selb	dxtBlock3, c01, c23, e23
	shufb	dxtBlock2, dxtBlock2, dxtBlock2, shufInvertDxt1a
	shufb	dxtBlock3, dxtBlock3, dxtBlock3, shufInvertDxt1a
	gbb		dxtBlock2, dxtBlock2
	gbb		dxtBlock3, dxtBlock3
.endif

	// build the double-word block
	.reg 	dxtBlockTemp0, dxtBlockTemp1, dxtBlock
	shufb	dxtBlockTemp0, dxtBlock0, dxtBlock1, shufMakeDxtColourBlock
	shufb 	dxtBlockTemp1, dxtBlock2, dxtBlock3, shufMakeDxtIndexBlock
	
.if DXT_MODE == 0x1a
	
	// apply the index mask for transparent pixels
	selb	dxtBlockTemp1, dxtBlockTemp1, dxt1aIndexMask, dxt1aIndexMask
.endif
	
	or		dxtBlock, dxtBlockTemp0, dxtBlockTemp1

.if DXT_MODE == 5
	// get the alpha range
	.reg	alphaMax, alphaMin, alphaRange
	shufb	alphaMax, candidateMaxMaxMinMin, candidateMaxMaxMinMin, gatherAlphaMax
	shufb	alphaMin, candidateMaxMaxMinMin, candidateMaxMaxMinMin, gatherAlphaMin
	sf		alphaRange, alphaMin, alphaMax

	// clamp to one
	.reg	alphaRangeNonZeroMask, alphaRangeOne, alphaRangeAdjusted
	il		alphaRangeOne, 1
	cgti	alphaRangeNonZeroMask, alphaRange, 0
	selb	alphaRangeAdjusted, alphaRangeOne, alphaRange, alphaRangeNonZeroMask

	// compute our floating point scale
	.reg 	rangeFloat
	.reg	scaleTemp1, scaleTemp2, rangeScale
	cuflt	rangeFloat, alphaRangeAdjusted, 0
	frest	scaleTemp1, rangeFloat
	fi		scaleTemp2, rangeFloat, scaleTemp1
	fm		rangeScale, scaleTemp2, float7	

	// generate the indices for each row of alpha values
	.reg	alphaValues0, alphaTemp0, alphaFloat0, indexFloat0, indexInt0
	.reg	alphaValues1, alphaTemp1, alphaFloat1, indexFloat1, indexInt1
	.reg	alphaValues2, alphaTemp2, alphaFloat2, indexFloat2, indexInt2
	.reg	alphaValues3, alphaTemp3, alphaFloat3, indexFloat3, indexInt3
	shufb	alphaValues0, row0, row0, gatherAlpha
	shufb	alphaValues1, row1, row1, gatherAlpha
	shufb	alphaValues2, row2, row2, gatherAlpha
	shufb	alphaValues3, row3, row3, gatherAlpha
	sf		alphaTemp0, alphaMin, alphaValues0
	sf		alphaTemp1, alphaMin, alphaValues1
	sf		alphaTemp2, alphaMin, alphaValues2
	sf		alphaTemp3, alphaMin, alphaValues3
	cuflt	alphaFloat0, alphaTemp0, 0
	cuflt	alphaFloat1, alphaTemp1, 0
	cuflt	alphaFloat2, alphaTemp2, 0
	cuflt	alphaFloat3, alphaTemp3, 0
	fma		indexFloat0, alphaFloat0, rangeScale, float0_5
	fma		indexFloat1, alphaFloat1, rangeScale, float0_5
	fma		indexFloat2, alphaFloat2, rangeScale, float0_5
	fma		indexFloat3, alphaFloat3, rangeScale, float0_5
	cfltu	indexInt0, indexFloat0, 0
	cfltu	indexInt1, indexFloat1, 0
	cfltu	indexInt2, indexFloat2, 0
	cfltu	indexInt3, indexFloat3, 0

	// arrange them into a single register
	.reg	indexCombinedTempA, indexCombinedTempB, indexCombined
	shufb	indexCombinedTempA, indexInt0, indexInt1, gatherAlphaIndices01
	shufb	indexCombinedTempB, indexInt2, indexInt3, gatherAlphaIndices23
	or		indexCombined, indexCombinedTempA, indexCombinedTempB

	// remap the indices by treating it as a shuffle mask
	.reg	indexRemapped
	shufb	indexRemapped, alphaIndexRemap, alphaIndexRemap, indexCombined

	// gather the index bits
	.reg 	alphaIndexBits0, indexBitsMask0, indexBitsEnabled0, indexBitsBlock0
	.reg 	alphaIndexBits1, indexBitsMask1, indexBitsEnabled1, indexBitsBlock1
	.reg 	alphaIndexBits2, indexBitsMask2, indexBitsEnabled2, indexBitsBlock2
	shufb	alphaIndexBits0, indexRemapped, indexRemapped, alphaIndexBitGather0
	shufb	alphaIndexBits1, indexRemapped, indexRemapped, alphaIndexBitGather1
	shufb	alphaIndexBits2, indexRemapped, indexRemapped, alphaIndexBitGather2
	and		indexBitsMask0, alphaIndexBits0, alphaIndexBitMask0
	and		indexBitsMask1, alphaIndexBits1, alphaIndexBitMask1
	and		indexBitsMask2, alphaIndexBits2, alphaIndexBitMask2
	ceqb	indexBitsEnabled0, indexBitsMask0, alphaIndexBitMask0
	ceqb	indexBitsEnabled1, indexBitsMask1, alphaIndexBitMask1
	ceqb	indexBitsEnabled2, indexBitsMask2, alphaIndexBitMask2
	gbb		indexBitsBlock0, indexBitsEnabled0
	gbb		indexBitsBlock1, indexBitsEnabled1
	gbb		indexBitsBlock2, indexBitsEnabled2

	// generate the alpha block
	.reg	alphaBlockMaxMin
	.reg	indexBitsBlock1Rotated, indexBitsBlock2Rotated
	.reg 	alphaBlockTemp0, alphaBlockTemp1
	.reg	alphaBlock
	shufb	alphaBlockMaxMin, candidateMaxMaxMinMin, candidateMaxMaxMinMin, gatherAlphaMaxMin
	rotqbyi	indexBitsBlock1Rotated, indexBitsBlock1, -2
	rotqbyi	indexBitsBlock2Rotated, indexBitsBlock2, -4
	or		alphaBlockTemp0, alphaBlockMaxMin, indexBitsBlock0
	or		alphaBlockTemp1, indexBitsBlock1Rotated, indexBitsBlock2Rotated
	or 		alphaBlock, alphaBlockTemp0, alphaBlockTemp1

	// make the combined block		
	.reg	combinedBlock
	or		combinedBlock, dxtBlock, alphaBlock

	// write it out to memory
	stqd	combinedBlock, 0x00( outputBlocks )
.elif DXT_MODE == 3
	// rotate the alpha by 4 bits to swap lo and hi bits
	.reg	rotatedAlpha
	rotqbii	rotatedAlpha, alpha, 4

	// shuffle and select into the byte locations for the quantised block
	.reg 	alphaBlockLo, alphaBlockHi
	.reg 	alphaBlock
	shufb	alphaBlockLo, rotatedAlpha, rotatedAlpha, gatherAlphaBlockLo
	shufb	alphaBlockHi, alpha, alpha, gatherAlphaBlockHi
	selb	alphaBlock, alphaBlockLo, alphaBlockHi, selectAlphaBlockLoHi

	// make the combined block		
	.reg	combinedBlock
	or		combinedBlock, dxtBlock, alphaBlock

	// write it out to memory
	stqd	combinedBlock, 0x00( outputBlocks )
.else
	// write the colour block out to memory
	.memgroup	mg
	.reg	memTemp0, memTemp1, memShuf
	lqd		[mg] memTemp0, 0x00( outputBlocks )
	cdd		memShuf, 0x00( outputBlocks )
	shufb	memTemp1, dxtBlock, memTemp0, memShuf
	stqd	[mg] memTemp1, 0x00( outputBlocks )
.endif

	// loop branch
	ai		inputPixels, inputPixels, 0x10
.if (DXT_MODE == 1) || (DXT_MODE == 0x1a)
	ai		outputBlocks, outputBlocks, 0x08
.else
	ai		outputBlocks, outputBlocks, 0x10
.endif
	ai		blockCount, blockCount, -1
	brnz	blockCount, loop

end:
.endfunc

