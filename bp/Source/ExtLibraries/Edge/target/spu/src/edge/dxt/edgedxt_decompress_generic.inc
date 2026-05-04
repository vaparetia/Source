/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

.if DXT_MODE == 1
.cfunc void edgeDxtDecompress1(const void* inputBlocks, void* outputPixels, uint32_t rowStride, uint32_t blockCount)
.elif DXT_MODE == 3
.cfunc void edgeDxtDecompress3(const void* inputBlocks, void* outputPixels, uint32_t rowStride, uint32_t blockCount)
.else
.cfunc void edgeDxtDecompress5(const void* inputBlocks, void* outputPixels, uint32_t rowStride, uint32_t blockCount)
.endif

	// early out if blockCount is zero
	brz		blockCount, end

	// shuffle masks
	.reg	shufBytesUpperTemp, shufBytesUpper
	.reg	shufBytesLowerTemp, shufBytesLower
	.reg	maskLower565
	il128	shufBytesUpperTemp, 0x80808080_80008011_80808080_80028013
	il128	shufBytesUpper, 0x80808010_80058007_80808012_800d800f
	il128	shufBytesLowerTemp, 0x80808080_800f8010_80808080_80018012
	il128	shufBytesLower, 0x8080801f_800f8007_80808011_800d800f
	il128	maskLower565, 0x00000007_00030007_00000007_00030007

	.reg	magicDiv3
	ilh		magicDiv3, 21846	// ( 1 << 16 )/3 + 1

.if DXT_MODE == 1
	.reg	shufGatherPaletteAvg
	il128	shufGatherPaletteAvg, 0x80808080_80808080_c0030507_80808080
.endif
	.reg 	shufGatherPaletteMid
	il128	shufGatherPaletteMid, 0x80808080_80808080_c0110515_c0190d1d

	.reg 	shufGatherPalette
	il128	shufGatherPalette, 0xc0030507_c00b0d0f_18191a1b_1c1d1e1f

.if DXT_MODE == 1 
	.reg 	shufAAAAAAAA
	il128	shufAAAAAAAA, "AAAAAAAA"
.endif

	.reg 	shufIndexHighBits, shufIndexLowBits
	il128 	shufIndexHighBits, 0x06040200_0e0c0a08_16141210_1e1c1a18
	orbi 	shufIndexLowBits, shufIndexHighBits, 0x01

	.reg 	mask04
	ilh		mask04, 0x0404

	.reg 	shufLookup0, shufLookup1, shufLookup2, shufLookup3
	il128 	shufLookup0, 0x00000000_01010101_02020202_03030303
	orbi	shufLookup1, shufLookup0, 0x04
	orbi	shufLookup2, shufLookup0, 0x08
	orbi	shufLookup3, shufLookup0, 0x0c

	.reg 	shufAAAA
	il128 	shufAAAA, "AAAA"

.if DXT_MODE == 3
	.reg	shufGatherAlpha
	il128	shufGatherAlpha, 0x001f0110_02110312_04130514_06150716
.endif

.if DXT_MODE == 5
	.reg	shufAAAAAAAAAAAAAAAA, shuf000A000A000A000A, shuf000B000B000B000B
	il128	shufAAAAAAAAAAAAAAAA, "AAAAAAAAAAAAAAAA"
	il128	shuf000A000A000A000A, "000A000A000A000A"
	il128	shuf000B000B000B000B, "000B000B000B000B"

	.reg 	bias	
	ilf32	bias, 0.5f

	.reg	scale5alpha0, scale5alpha1
	.reg	scale7alpha0a, scale7alpha0b, scale7alpha1a, scale7alpha1b
	il128	scale5alpha0, 0x3F4CCCCD_3F19999A_3ECCCCCD_3E4CCCCD		// 0.8f, 0.6f, 0.4f, 0.2f
	il128	scale5alpha1, 0x3E4CCCCD_3ECCCCCD_3F19999A_3F4CCCCD		// 0.2f, 0.4f, 0.6f, 0.8f
	il128	scale7alpha0a, 0x3F5B6DB7_3F36DB6E_3F124925_3EDB6DB7	// 6.0f/7.0f, 5.0f/7.0f, 4.0f/7.0f, 3.0f/7.0f
	il128	scale7alpha0b, 0x3E924925_3E124925_00000000_00000000	// 2.0f/7.0f, 1.0f/7.0f, 0.0f, 0.0f
	il128	scale7alpha1a, 0x3E124925_3E924925_3EDB6DB7_3F124925	// 1.0f/7.0f, 2.0f/7.0f, 3.0f/7.0f, 4.0f/7.0f
	il128	scale7alpha1b, 0x3F36DB6E_3F5B6DB7_00000000_00000000	// 5.0f/7.0f, 6.0f/7.0f, 0.0f, 0.0f

	.reg	shufGatherMid5, shufGatherMid7
	.reg	maskCombine
	il128	shufGatherMid5, 0x80800307_0b0f80c0_80808080_80808080
	il128 	shufGatherMid7, 0x80800307_0b0f1317_80808080_80808080
	il128	maskCombine, 0xffff0000_00000000_00000000_00000000

	.reg 	shufGatherIndexBytes0, shufGatherIndexBytes1, shufGatherIndexBytes2
	.reg	maskIndexTestBits0, maskIndexTestBits1, maskIndexTestBits2
	il128	shufGatherIndexBytes0, 0x02020203_03030404_05050506_06060707
	il128	shufGatherIndexBytes1, 0x02020203_03040404_05050506_06070707
	il128	shufGatherIndexBytes2, 0x02020303_03040404_05050606_06070707
	il128	maskIndexTestBits0, 0x01084002_10800420_01084002_10800420
	il128	maskIndexTestBits1, 0x02108004_20010840_02108004_20010840
	il128	maskIndexTestBits2, 0x04200108_40021080_04200108_40021080
.endif

.if DXT_MODE != 1
	.reg	shufMergeAlpha0, shufMergeAlpha1, shufMergeAlpha2, shufMergeAlpha3
	.reg	shufMergeAlphaOffset
	il128	shufMergeAlpha0, 0x10010203_11050607_12090a0b_130d0e0f
	ilhu	shufMergeAlphaOffset, 0x0400
	a		shufMergeAlpha1, shufMergeAlpha0, shufMergeAlphaOffset
	a		shufMergeAlpha2, shufMergeAlpha1, shufMergeAlphaOffset
	a		shufMergeAlpha3, shufMergeAlpha2, shufMergeAlphaOffset
.endif

	// precompute the load offsets
	.reg	rowStride2, rowStride3
	a		rowStride2, rowStride, rowStride
	a		rowStride3, rowStride2, rowStride

loop:
	// grab the quadword
	.reg 	blockUnaligned
	lqd		blockUnaligned, 0x00(inputBlocks)

	.reg	colorBlockRaw
.if DXT_MODE == 1
	// align the color block into the preferred slot
	rotqby	colorBlockRaw, blockUnaligned, inputBlocks
.else
	// rotate the color block into the preferred slot
	rotqbyi colorBlockRaw, blockUnaligned, 8
.endif

	// byte-swap the 16-bit color values to bigendian
	.reg 	block
	rothi	block, colorBlockRaw, 8

	// expand values from 565 to 000R_0G0B_000R_0G0B
	.reg	blockRot5, blockRot3, endPointsUpperTemp, endPointsUpper
	.reg 	blockRot7, blockRot6, endPointsLowerTemp, endPointsLower
	.reg	endPoints
	rotqbii blockRot5, block, 5
	rotqbii blockRot3, block, 3
	shufb	endPointsUpperTemp, blockRot5, blockRot3, shufBytesUpperTemp
	shufb	endPointsUpper, endPointsUpperTemp, block, shufBytesUpper
	rotqbii blockRot7, block, 7
	rotqbii blockRot6, block, 6
	shufb	endPointsLowerTemp, blockRot7, blockRot6, shufBytesLowerTemp
	shufb	endPointsLower, endPointsLowerTemp, blockRot3, shufBytesLower
	selb	endPoints, endPointsUpper, endPointsLower, maskLower565

	// swap the endpoints
	.reg	endPointsSwapped
	rotqbyi	endPointsSwapped, endPoints, 8

	// compute the average point (for DXT1 color blocks only)
	.reg	avgPointX2
	ah		avgPointX2, endPoints, endPointsSwapped
.if DXT_MODE == 1
	.reg	avgPoint, paletteAvg
	rothmi	avgPoint, avgPointX2, -1
	shufb	paletteAvg, avgPoint, avgPoint, shufGatherPaletteAvg
.endif

	// compute the midpoints for the 4-color encoding
	.reg	midPointsX3, midPointsEven, midPointsOdd
	ah 		midPointsX3, endPoints, avgPointX2
	mpyhhu	midPointsEven, midPointsX3, magicDiv3
	mpyu	midPointsOdd, midPointsX3, magicDiv3
.if DXT_MODE == 1
	.reg paletteMid
	shufb	paletteMid, midPointsEven, midPointsOdd, shufGatherPaletteMid

	// work out if we're a 3 or 4-color block
	.reg 	blockRot2Bytes, isMidBlock, isMidBlockMask
	rotqbyi	blockRot2Bytes, block, 2
	clgth	isMidBlock, block, blockRot2Bytes
	shufb	isMidBlockMask, isMidBlock, shufAAAAAAAA, shufAAAAAAAA

	// select the correct encoding
	.reg	paletteTemp
	selb	paletteTemp, paletteAvg, paletteMid, isMidBlockMask
.else
	// just use the 4-color encoding
	.reg	paletteTemp
	shufb	paletteTemp, midPointsEven, midPointsOdd, shufGatherPaletteMid
.endif

	// arrange our 4 color palette into a single ARGBx4 register
	.reg	palette
	shufb	palette, endPoints, paletteTemp, shufGatherPalette

	// form the index values
	.reg	indexPart0, indexPart1
	.reg	indexBits0, indexBits1
	.reg	indexHighBits, indexLowBits
	.reg	indexHighValues
	.reg	indexValues
	rotqbyi	indexPart0, colorBlockRaw, 2
	rotqbyi	indexPart1, colorBlockRaw, 4
	fsmb	indexBits0, indexPart0
	fsmb	indexBits1, indexPart1
	shufb	indexHighBits, indexBits0, indexBits1, shufIndexHighBits
	shufb	indexLowBits, indexBits0, indexBits1, shufIndexLowBits
	andbi 	indexHighValues, indexHighBits, 0x08
	selb	indexValues, indexHighValues, indexLowBits, mask04

	// form the row palette lookups
	.reg	rowLookupTemp0, rowLookupTemp1, rowLookupTemp2, rowLookupTemp3
	.reg	rowLookup0, rowLookup1, rowLookup2, rowLookup3
	shufb	rowLookupTemp0, indexValues, indexValues, shufLookup0
	shufb	rowLookupTemp1, indexValues, indexValues, shufLookup1
	shufb	rowLookupTemp2, indexValues, indexValues, shufLookup2
	shufb	rowLookupTemp3, indexValues, indexValues, shufLookup3
	or		rowLookup0, rowLookupTemp0, shufAAAA
	or		rowLookup1, rowLookupTemp1, shufAAAA
	or		rowLookup2, rowLookupTemp2, shufAAAA
	or		rowLookup3, rowLookupTemp3, shufAAAA

	// look up the rows (using shufb as a palette lookup)
	.reg	colorRow0, colorRow1, colorRow2, colorRow3
	shufb	colorRow0, palette, palette, rowLookup0
	shufb	colorRow1, palette, palette, rowLookup1
	shufb	colorRow2, palette, palette, rowLookup2
	shufb	colorRow3, palette, palette, rowLookup3
	
.if DXT_MODE == 3
	// pull out the 4-bit alpha values
	.reg	alphaBlockRot, alphaLo, alphaHi
	rotqbii	alphaBlockRot, blockUnaligned, 4
	andbi	alphaLo, blockUnaligned, 0x0f
	andbi	alphaHi, alphaBlockRot, 0x0f

	// shuffle into a full vector of 8 bit values
	.reg	alphaValuesTemp0, alphaValuesTemp1, alphaValues
	shufb	alphaValuesTemp0, alphaLo, alphaHi, shufGatherAlpha
	rotqbii	alphaValuesTemp1, alphaValuesTemp0, 4
	or		alphaValues, alphaValuesTemp0, alphaValuesTemp1	
.endif

.if DXT_MODE == 5
	// compare first two alpha values for encoding (5 vs 7)
	.reg 	alphaBlockRot, isMid7, isMid7Mask
	rotqbyi	alphaBlockRot, blockUnaligned, 1
	clgtb	isMid7, blockUnaligned, alphaBlockRot
	shufb	isMid7Mask, isMid7, isMid7, shufAAAAAAAAAAAAAAAA

	// compute all intermediates
	.reg	alpha0, alpha1
	.reg	alpha0f, alpha1f
	.reg	mid5ftemp, mid7aftemp, mid7bftemp
	.reg	mid5f, mid7af, mid7bf
	.reg	mid5, mid7a, mid7b
	.reg	mid5values, mid7values
	shufb 	alpha0, blockUnaligned, blockUnaligned, shuf000A000A000A000A
	shufb 	alpha1, blockUnaligned, blockUnaligned, shuf000B000B000B000B
	cuflt 	alpha0f, alpha0, 0
	cuflt	alpha1f, alpha1, 0
	fma 	mid5ftemp, alpha0f, scale5alpha0, bias
	fma 	mid7aftemp, alpha0f, scale7alpha0a, bias
	fma		mid7bftemp, alpha0f, scale7alpha0b, bias
	fma 	mid5f, alpha1f, scale5alpha1, mid5ftemp
	fma		mid7af, alpha1f, scale7alpha1a, mid7aftemp
	fma 	mid7bf, alpha1f, scale7alpha1b, mid7bftemp
	cfltu	mid5, mid5f, 0
	cfltu 	mid7a, mid7af, 0
	cfltu	mid7b, mid7bf, 0
	shufb	mid5values, mid5, mid5, shufGatherMid5
	shufb	mid7values, mid7a, mid7b, shufGatherMid7

	// form the final palette
	.reg 	midValues, alphaPalette
	selb 	midValues, mid5values, mid7values, isMid7Mask
	selb	alphaPalette, midValues, blockUnaligned, maskCombine

	// unpack the 3-bit indices (done in an odd way to parallelize)
	.reg	alphaIndexBytes0, alphaIndexBytes1, alphaIndexBytes2
	.reg	alphaIndexTestBit0, alphaIndexTestBit1, alphaIndexTestBit2
	.reg	alphaIndexTestMask0, alphaIndexTestMask1, alphaIndexTestMask2
	.reg 	alphaIndexActualBit0, alphaIndexActualBit1,alphaIndexActualBit2
	.reg	alphaIndicesTemp, alphaIndices
	shufb 	alphaIndexBytes0, blockUnaligned, blockUnaligned, shufGatherIndexBytes0
	shufb	alphaIndexBytes1, blockUnaligned, blockUnaligned, shufGatherIndexBytes1
	shufb 	alphaIndexBytes2, blockUnaligned, blockUnaligned, shufGatherIndexBytes2
	and		alphaIndexTestBit0, alphaIndexBytes0, maskIndexTestBits0
	and		alphaIndexTestBit1, alphaIndexBytes1, maskIndexTestBits1
	and		alphaIndexTestBit2, alphaIndexBytes2, maskIndexTestBits2
	ceqb	alphaIndexTestMask0, alphaIndexTestBit0, maskIndexTestBits0
	ceqb 	alphaIndexTestMask1, alphaIndexTestBit1, maskIndexTestBits1
	ceqb 	alphaIndexTestMask2, alphaIndexTestBit2, maskIndexTestBits2
	andbi	alphaIndexActualBit0, alphaIndexTestMask0, 0x01
	andbi	alphaIndexActualBit1, alphaIndexTestMask1, 0x02
	andbi 	alphaIndexActualBit2, alphaIndexTestMask2, 0x04
	or		alphaIndicesTemp, alphaIndexActualBit0, alphaIndexActualBit1
	or		alphaIndices, alphaIndexActualBit2, alphaIndicesTemp

	// look up the values
	.reg	alphaValues
	shufb	alphaValues, alphaPalette, alphaPalette, alphaIndices
.endif

.if DXT_MODE == 1
	// write out the rows
	stqd	colorRow0, 0x00(outputPixels)
	stqx	colorRow1, outputPixels, rowStride
	stqx	colorRow2, outputPixels, rowStride2
	stqx	colorRow3, outputPixels, rowStride3
.else
	// shuffle into the output alpha channels
	.reg	row0, row1, row2, row3
	shufb 	row0, colorRow0, alphaValues, shufMergeAlpha0
	shufb 	row1, colorRow1, alphaValues, shufMergeAlpha1
	shufb 	row2, colorRow2, alphaValues, shufMergeAlpha2
	shufb 	row3, colorRow3, alphaValues, shufMergeAlpha3

	// write out the rows
	stqd	row0, 0x00(outputPixels)
	stqx	row1, outputPixels, rowStride
	stqx	row2, outputPixels, rowStride2
	stqx	row3, outputPixels, rowStride3
.endif

	// loop branch
.if DXT_MODE == 1
	ai		inputBlocks, inputBlocks, 0x08
.else
	ai		inputBlocks, inputBlocks, 0x10
.endif
	ai		outputPixels, outputPixels, 0x10
	ai		blockCount, blockCount, -1
	brnz	blockCount, loop

end:
.endfunc

