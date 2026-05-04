/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	Edge Lzma SPU Version*/
/*	This file is the optimized asm version of the SPU code with start entry "LzmaDecode_spu_asm"*/
/*	The C implementation is in "edgelzma_LzmaDecode.c"*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/



/* set this to 0 or 1 (normally 1)*/




/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	uninitialized data*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/







.equ picLoc, 22
.equ picLabel, 23
.equ picOffset, 22
.equ vs, 2
.equ inStream, 3
.equ inSize, 4
.equ gInSizeProcessed, 14
.equ outStream, 7
.equ outSize, 8
.equ gOutSizeProcessed, 17
.equ ProbsAdrs, 10
.equ ProbsQw, 13
.equ p, 10
.equ nowPos, 6
.equ previousByte, 9
.equ pbAdrs, 11
.equ pbQw, 15
.equ _1, 21
.equ tPosStateMask, 11
.equ posStateMask, 11
.equ pb, 11
.equ lpAdrs, 12
.equ lpQw, 17
.equ lp, 12
.equ tLiteralPosMask, 24
.equ literalPosMask, 18
.equ lcQw, 5
.equ lc, 2
.equ state, 15
.equ rep0, 17
.equ rep1, 19
.equ rep2, 23
.equ rep3, 24
.equ len, 36
.equ lcPlsLp, 12
.equ _0x00000300, 17
.equ tNumProbs, 12
.equ _0x00000737, 14
.equ numProbsMns1, 12
.equ tIndex, 12
.equ index, 12
.equ index_, 13
.equ _0x04000400, 14
.equ BufferByt1Adrs, 14
.equ BufferByt2Adrs, 16
.equ BufferByt3Adrs, 18
.equ BufferByt4Adrs, 5
.equ BufferByt1Qw, 19
.equ BufferByt2Qw, 16
.equ BufferByt3Qw, 18
.equ BufferByt4Qw, 5
.equ BufferByt1AtByt0, 19
.equ BufferByt2AtByt1, 16
.equ BufferByt3AtByt2, 17
.equ BufferByt4AtByt3, 5
.equ _0x00ff00ff, 15
.equ BufferByts12, 16
.equ BufferByts34, 5
.equ _0x0000ffff, 13
.equ BufferLim, 4
.equ error, 12
.equ nowPosLtOutSize, 35
.equ posState, 38
.equ stateShl4, 36
.equ stateShl4PlsPosState, 36
.equ tProb, 36
.equ atProbQw, 41
.equ probMns2, 43
.equ atProb, 43
.equ atProbLenQw, 36
.equ probLen, 37
.equ probLenMns2, 40
.equ atProbLen, 40
.equ atProb1Qw, 42
.equ prob1, 38
.equ prob1Mns2, 44
.equ atProb1, 44
.equ atProb2Qw, 42
.equ prob2, 38
.equ prob2Mns2, 44
.equ atProb2, 44
.equ _0x800MnsAtProb, 39
.equ _0x800MnsAtProbShr5, 39
.equ atProbMask, 40
.equ _0x800MnsAtProbLen, 42
.equ _0x800MnsAtProbLenShr5, 42
.equ atProbLenMask, 39
.equ tProbL, 39
.equ tProbR, 9
.equ lcMns8, 29
.equ _0x00000600, 30
.equ _0x00000e70, 31
.equ stateGe7, 35
.equ nowPosMnsRep0, 35
.equ matchByteQw, 35
.equ matchByteAdrs, 36
.equ tMatchByte, 35
.equ matchByte, 35
.equ bit, 44
.equ symbolPls0x100, 36
.equ tProbLit, 36
.equ CodeLtBoundPlus1, 45
.equ breakIfNz, 41
.equ breakIfZ, 42
.equ matchByteAdrsMns3, 36
.equ tBit, 38
.equ tError, 44
.equ symbolMpy2, 37
.equ symbol, 37
.equ probLit, 36
.equ probLit_, 39
.equ prob, 36
.equ RangeLt0x01000000, 35
.equ _0x01000000, 26
.equ Range, 25
.equ Range_, 42
.equ RangeIfT, 39
.equ atBufferQw, 37
.equ Buffer, 20
.equ atBufferAtByt0, 37
.equ BufferIfT, 40
.equ CodeIfT, 37
.equ Code, 5
.equ Code_, 5
.equ _0x01020310, 16
.equ atProbLitQw, 42
.equ atProbLitQw_, 40
.equ probLitMns2, 38
.equ atProbLit, 45
.equ atProbLit_, 40
.equ RangeShr11, 42
.equ boundMpyLo, 35
.equ boundMpyHi, 37
.equ bound, 35
.equ CodeLtBound, 37
.equ RangeIfF, 25
.equ CodeIfF, 39
.equ _0x800MnsAtProbLit, 47
.equ _0x00000800, 27
.equ _0x800MnsAtProbLitShr5, 47
.equ atProbLitIfT, 40
.equ atProbLitShr5, 46
.equ atProbLitIfF, 46
.equ atProbLitMask, 41
.equ symbolMpy2Plus1, 25
.equ symbolNotLt0x100, 39
.equ _0x01020380, 28
.equ atOutStreamNowPosQw, 14
.equ atOutStreamNowPosMask, 38
.equ stateGe4, 15
.equ stateGe10, 35
.equ stateIf4To9, 36
.equ stateIfGe10, 39
.equ stateIfGe4, 35
.equ atProbShr5, 9
.equ stateMpy2, 39
.equ _3, 13
.equ _0x00000664, 32
.equ stateShl5, 44
.equ posStateMpy2, 39
.equ tState, 47
.equ previousByteQw, 41
.equ previousByteAdrs, 42
.equ tPreviousByte, 41
.equ distance, 9
.equ posStateShl4, 38
.equ offset, 35
.equ numBits, 39
.equ atProbLenShr5, 39
.equ tProbLen, 9
.equ _0x00000204, 40
.equ i, 37
.equ lenMpy2, 38
.equ _0x800MnsAtProb1, 49
.equ _0x800MnsAtProb1Shr5, 46
.equ atProb1IfT, 46
.equ atProb1IfF, 44
.equ CodeLtBoundPls1, 46
.equ _0x800MnsAtProb2, 45
.equ _0x800MnsAtProb2Shr5, 39
.equ atProb2IfT, 39
.equ atProb2IfF, 45
.equ posSlotMpy2, 38
.equ posSlot, 37
.equ dontLoop, 41
.equ tLen, 48
.equ stateNotLt4, 47
.equ _0x00000360, 33
.equ lenGe4, 35
.equ posSlotGe4, 35
.equ tNumDirectBits, 17
.equ numDirectBits, 39
.equ tRep0, 37
.equ posSlotGe14, 38
.equ CodeNotGeRange, 41
.equ rep0IfT, 38
.equ _0x00000644, 34
.equ mi, 37
.equ miMpy2, 37
.equ prob3, 41
.equ atProb3Qw, 44
.equ prob3Mns2, 46
.equ atProb3, 46
.equ _0x800MnsAtProb3, 43
.equ _0x800MnsAtProb3Shr5, 43
.equ atProb3IfT, 43
.equ atProb3Shr5, 51
.equ atProb3IfF, 45
.equ atProb3Mask, 50
.equ miMpy2Plus1, 37
.equ rep0IfF, 52
.equ rep0Eq0, 35
.equ nowPos16, 37
.equ atProb1Mask, 50
.equ atProb2Mask, 46
.equ rep0Ge32, 43
.equ maxLen, 38
.equ lenTooBig, 43
.equ numBytsToDup, 9
.equ dupIndex, 39
.equ dupAdrs, 41
.equ dup0Qw, 44
.equ dup1Qw, 46
.equ dup2Qw, 40
.equ numBytsBeforeDup0, 39
.equ negNumDup0Byts, 38
.equ dup0QwShl, 45
.equ dup0Partial, 38
.equ dup1QwShl, 39
.equ dup2QwShr, 41
.equ dup1Partial, 39
.equ rep0Mns1, 43
.equ dup1MaskIndex, 47
.equ _0xf0, 48
.equ dup1MaskIndexBig, 51
.equ dup0MaskIndex, 48
.equ gDup0MaskTbl, 44
.equ dup0Mask, 44
.equ gDup1MaskTbl, 49
.equ dup1Mask, 46
.equ dup0, 42
.equ dup1, 40
.equ tGRepTbl, 44
.equ gRepTbl, 44
.equ repQw, 50
.equ repInByt0, 43
.equ rep, 43
.equ numBytsSet, 40
.equ numBytsSetMns4, 9
.equ numBytsNotSet, 42
.equ negNumBytsNotSet, 45
.equ atOutStreamNowPosQwShr, 14
.equ atOutStreamNowPosQw0s, 14
.equ dup0Shr, 46
.equ negNumBytsSet, 40
.equ dup1Shr, 39
.equ tDup0_, 39
.equ negNumDup1BytsSet, 44
.equ neg_bytsSetPlsDup1Set, 40
.equ dup1Shl, 45
.equ tDup0Shr_, 48
.equ dup1_, 45
.equ dup0_shr, 38
.equ dup1__, 9
.equ dup0__shr, 38
.equ tNumBytsSet, 9
.equ previousByteInByt3, 9
.equ dup1QwShr, 38
.equ moreBytsToDup, 38
.equ dup0Shl, 49
.equ numDup1BytsNotSet, 43
.equ dup0Shr_, 40
.equ outStreamBackup, 35
.equ outStream16, 36
.equ neg_bytsSetPlsDup0Byts, 38
.equ tAtOutStreamNowPosQw, 14
.equ dup0QwShlShr, 40
.equ negNumDup0Byts_, 39
.equ numDup0Byts_, 39
.equ dup0BytsExist, 40
.equ dup0BytsExistQw, 14
.equ numDup0Byts_pls16, 38
.equ numDup0Byts__, 38
.equ dupAdrsPls16, 43
.equ dupAdrs_, 41
.equ negNumDup0Byts__, 38
.equ numBytsBeforeDup0__, 39
.equ _0x1010Etc, 12
.equ _0x101112Etc, 43
.equ _0x000102Etc, 15
.equ _0x000102EtcShl, 39
.equ _0x101112EtcShr, 38
.equ farDupMask, 38
.equ numBytsToDup_, 9
.equ numBytsToDup__, 47
.equ moreBytsToDup_, 39
.equ dup0Shr__, 38
.equ atProbLen_, 42
.equ atProb1Shr5, 51
.equ atProb2Shr5, 47
.equ tLen2, 40
.equ tRange, 48
.equ tatProbMask, 44
.equ tProbL_, 41
.equ tatBufferQw, 35
.equ tBufferIfT, 45
.equ tRangeLt0x01000000, 36
.equ tRangeIfT, 9
.equ tatProb, 42
.equ tnowPos, 37
.equ atInSizeProcessed, 3
.equ inSizeProcessed, 2
.equ atInSizeProcessedQw, 5
.equ atInSizeProcessedMask, 4
.equ outSizeProcessed, 7
.equ atOutSizeProcessedQw, 8
.equ atOutSizeProcessedMask, 9
.equ index__, 15
.equ _0x000102Etc_, 12
.equ error_, 37
.equ tProb_, 9
.equ atProb_, 25
.equ atProbQw_, 38
.equ prob_, 9
.equ atBufferQw_, 40
.equ BufferIfT_, 41
.equ RangeLt0x01000000_, 43
.equ atBufferAtByt0_, 39
.equ RangeShr11_, 40
.equ boundMpyLo_, 41
.equ boundMpyHi_, 40
.equ CodeIfT_, 39
.equ bound_, 39
.equ CodeLtBound_, 43
.equ error__, 35
.equ symbolNotLt0x100_, 36
.equ symbolMpy2_, 35
.equ atBufferQw__, 36
.equ RangeLt0x01000000__, 38
.equ RangeIfT_, 37
.equ probLitMns2_, 41
.equ atBufferAtByt0__, 36
.equ RangeShr11__, 43
.equ atProbLit__, 41
.equ BufferIfT__, 35
.equ boundMpyLo__, 37
.equ CodeIfT__, 36
.equ boundMpyHi__, 43
.equ atProbLitShr5_, 44
.equ atProbLitMask_, 45
.equ _0x800MnsAtProbLit_, 46
.equ _0x800MnsAtProbLitShr5_, 36
.equ atProbLitIfF_, 38
.equ bound__, 43
.equ atProbLitIfT_, 36
.equ CodeLtBound__, 44
.equ RangeIfF_, 35
.equ probLit__, 41
.equ atBufferQw___, 46
.equ atProbLit___, 36
.equ RangeIfT__, 42
.equ CodeIfF_, 47
.equ probLitMns2__, 45
.equ symbolNotLt0x100__, 46
.equ RangeShr11___, 41
.equ error___, 9
.equ probLit___, 38
.equ atProbLitQw__, 39
.equ RangeLt0x01000000___, 40
.equ Range__, 25
.equ RangeShr11____, 37
.equ symbolMpy2Plus1_, 35
.equ BufferIfT___, 42
.equ boundMpyLo___, 43
.equ atProbLitMask__, 38
.equ _0x800MnsAtProbLit__, 45
.equ atProbLitIfF__, 40
.equ bound___, 37
.equ CodeLtBound___, 41
.equ symbol_, 35
.equ atBufferQw____, 42
.equ RangeIfT___, 40
.equ probLit____, 43
.equ CodeIfF__, 37
.equ atProbLitQw___, 36
.equ probLitMns2___, 39
.equ atBufferAtByt0___, 40
.equ atProbLit____, 39
.equ BufferIfT____, 37
.equ CodeIfT___, 40
.equ boundMpyHi___, 42
.equ atProbLitMask___, 43
.equ _0x800MnsAtProbLitShr5__, 40
.equ atProbLitIfF___, 37
.equ bound____, 38
.equ atProbLitIfT__, 39
.equ CodeLtBound____, 40
.equ atBufferQw_____, 41
.equ atProbLit_____, 37
.equ probLit_____, 42
.equ CodeIfF___, 38
.equ probLitMns2____, 37
.equ Range___, 39
.equ atBufferAtByt0____, 41
.equ RangeShr11_____, 44
.equ CodeIfT____, 38
.equ atProbLitShr5__, 41
.equ atProbLitMask____, 44
.equ _0x800MnsAtProbLitShr5___, 38
.equ atProbLitIfF____, 41
.equ bound_____, 40
.equ atProbLitIfT___, 38
.equ CodeLtBound_____, 46
.equ probLit______, 45
.equ atBufferQw______, 47
.equ atProbLit______, 38
.equ RangeIfT____, 41
.equ CodeIfF____, 40
.equ atProbLitQw____, 38
.equ probLitMns2_____, 48
.equ symbolNotLt0x100___, 47
.equ atProbMask_, 37
.equ atProb__, 9
.equ atProbQw__, 9
.equ tProb__, 37
.equ RangeLt0x01000000____, 36
.equ prob__, 37
.equ atBufferQw_______, 39
.equ probMns2_, 40
.equ atProb___, 40
.equ boundMpyHi____, 41
.equ _0x800MnsAtProb_, 42
.equ error____, 43
.equ _0x800MnsAtProbShr5_, 42
.equ atProbMask__, 44
.equ CodeLtBound______, 36
.equ Range____, 35
.equ stateGe7_, 15
.equ atProbShr5_, 36
.equ atProbMask___, 39
.equ tProbL__, 43
.equ atProb____, 36
.equ stateMpy2_, 40
.equ BufferIfT_____, 36
.equ atBufferAtByt0_____, 35
.equ nowPosMnsRep0_, 42
.equ RangeIfT_____, 9
.equ atProbQw___, 37
.equ prob___, 40
.equ RangeLt0x01000000_____, 45
.equ CodeIfT_____, 35
.equ probMns2__, 46
.equ Range_____, 9
.equ RangeShr11______, 25
.equ boundMpyLo____, 44
.equ boundMpyHi_____, 25
.equ _0x800MnsAtProb__, 46
.equ _0x800MnsAtProbShr5__, 46
.equ stateGe7__, 47
.equ error_____, 42
.equ bound______, 25
.equ atProbQw____, 25
.equ atProbQw_____, 39
.equ atProbMask____, 35
.equ probMns2___, 25
.equ error______, 36
.equ boundMpyLo_____, 42
.equ boundMpyHi______, 36
.equ _0x800MnsAtProb___, 37
.equ atOutStreamNowPosMask_, 43
.equ _0x800MnsAtProbShr5___, 37
.equ bound_______, 36
.equ CodeLtBound_______, 42
.equ atProb_____, 15
.equ atProbQw______, 36
.equ atProbShr5__, 37
.equ atProbMask_____, 9
.equ atProbShr5___, 35
.equ atProbMask______, 41
.equ tProb___, 25
.equ atProb______, 35
.equ stateMpy2__, 36
.equ atProbQw_______, 35
.equ RangeLt0x01000000______, 41
.equ RangeShr11_______, 35
.equ probMns2____, 39
.equ error_______, 40
.equ boundMpyLo______, 39
.equ boundMpyHi_______, 35
.equ _0x800MnsAtProb____, 40
.equ _0x800MnsAtProbShr5____, 40
.equ atProbShr5____, 39
.equ atProbMask_______, 42
.equ RangeIfT______, 43
.equ tProb____, 35
.equ prob____, 25
.equ error________, 41
.equ atProb_______, 37
.equ CodeLtBound________, 35
.equ atProbShr5_____, 40
.equ stateGe7___, 9
.equ tProb_____, 15
.equ tState_, 25
.equ RangeIfT_______, 25
.equ RangeLt0x01000000_______, 39
.equ CodeIfT______, 41
.equ error_________, 44
.equ probLen_, 9
.equ atProbLenMask_, 43
.equ RangeIfT________, 44
.equ atProbLen__, 39
.equ atProbLenQw_, 35
.equ RangeShr11________, 39
.equ probLenMns2_, 36
.equ atProbLenQw__, 41
.equ atProbLen___, 36
.equ boundMpyHi________, 39
.equ _0x800MnsAtProbLen_, 40
.equ _0x800MnsAtProbLenShr5_, 40
.equ atProbLenShr5_, 38
.equ atProbLenMask__, 42
.equ RangeShr11_________, 45
.equ tError_, 46
.equ error__________, 46
.equ boundMpyLo_______, 50
.equ boundMpyHi_________, 45
.equ BufferIfT______, 49
.equ lenMpy2_, 36
.equ CodeLtBound_________, 45
.equ CodeIfF_____, 51
.equ len_, 9
.equ i_, 36
.equ tProbR_, 35
.equ tProbL___, 38
.equ prob_____, 35
.equ dontLoop_, 40
.equ tError__, 45
.equ error___________, 45
.equ boundMpyLo________, 46
.equ posSlotMpy2_, 37
.equ bound________, 41
.equ BufferIfT_______, 47
.equ atProb2_, 39
.equ atProb2Qw_, 39
.equ CodeLtBoundPls1_, 42
.equ CodeIfF______, 41
.equ posSlot_, 36
.equ rep0_, 37
.equ tProbL____, 37
.equ tProbR__, 38
.equ prob______, 38
.equ numDirectBits_, 17
.equ atBufferQw________, 38
.equ dontLoop__, 39
.equ atBufferAtByt0______, 38
.equ i__, 40
.equ RangeLt0x01000000________, 42
.equ tError___, 43
.equ atBufferQw_________, 45
.equ RangeShr11__________, 47
.equ BufferIfT________, 48
.equ dontLoop___, 49
.equ atBufferAtByt0_______, 45
.equ boundMpyHi__________, 47
.equ CodeIfT_______, 45
.equ bound_________, 47
.equ atProb3_, 43
.equ CodeIfF_______, 45
.equ atProb3Qw_, 43
.equ _0x1010Etc_, 43
.equ numBytsSet_, 9
.equ dup2Qw_, 41
.equ moreBytsToDup__, 41
.equ dup0_, 38
.equ moreBytsToDup___, 9
.equ numBytsToDup___, 39
.equ dup1___, 40
.equ gInSizeProcessed_, 2
.equ RangeLt0x01000000_________, 5
.equ gOutSizeProcessed_, 7
.equ Buffer_, 5
.equ error____________, 4
.equ atInSizeProcessedQw_, 3
.equ atOutSizeProcessedQw_, 4
.set	INTERLEAVE_CODE	, 1

/* set this to 0 or 1 (normally 1, if you enable this you must have INTERLEAVE_CODE enabled)*/
.set INTERLEAVE_2ND_MOST_FREQUENT_LOOP , 1


/* for start of routine*/
.global LzmaDecode_spu_asm
.extern _gInSizeProcessed
.extern _gOutSizeProcessed

/* for code before main loop*/
.extern	_gRepTbl
.extern	_gDup0MaskTbl
.extern	_gDup1MaskTbl
.text
.align 7

LzmaDecode_spu_asm:
/*e2*/	ori		vs,			$3,			0;
/*o15*/	hbrr		_picBranch,		_picLabel;

/*e2*/	ori		inStream,		$4,			0;
	lnop;

/*e2*/	ori		inSize,			$5,			0;
/*o6*/	lqd		lcQw,			0(vs);

/*e2*/	ori		outStream,		$7,			0;

/*e2*/	ai		ProbsAdrs,		vs,			12;

/*e2*/	ai		pbAdrs,			vs,			8;
	lnop;

/*e2*/	ai		lpAdrs,			vs,			4;
/*o6*/	lqd		ProbsQw,		0(ProbsAdrs);

/*e2*/	ai		BufferByt1Adrs,		inStream,		1;
/*o6*/	lqd		pbQw,			0(pbAdrs);

/*e2*/	ai		BufferByt2Adrs,		inStream,		2;
/*o6*/	lqd		lpQw,			0(lpAdrs);

/*e2*/	ai		BufferByt3Adrs,		inStream,		3;
/*o4*/	rotqby		lc,			lcQw,			vs;

/*e2*/	ai		BufferByt4Adrs,		inStream,		4;
/*o6*/	lqd		BufferByt1Qw,		0(BufferByt1Adrs);

/*e2*/	ai		Buffer,			inStream,		5;
/*o6*/	lqd		BufferByt2Qw,		0(BufferByt2Adrs);

/*e2*/	a		BufferLim,		inStream,		inSize;	
/*o4*/	rotqby		p,			ProbsQw,		ProbsAdrs;

/*e2*/	ila		_0x0000ffff,		0xffff;
/*o4*/	rotqby		pb,			pbQw,			pbAdrs;

/*e2*/	ilh		_0x00ff00ff,		0x00ff;
/*o4*/	rotqby		lp,			lpQw,			lpAdrs;

/*Note: the above hint needed 16 cycles total or else Frontend indicates a cycle delay before the brsl below*/
/*e2*/	il		_0x00000300,		0x300;
/*o15*/	hbrr		_initProbsFrom,		_initProbsTo;

/*e2*/	il		_1,			1;
_picBranch:
/*o4*/	brsl		picLoc,			_picLabel;
_picLabel:

/*e2*/	ila		picLabel,		_picLabel;			
/*o6*/	lqd		BufferByt3Qw,		0(BufferByt3Adrs);

/*e4*/	shl		tPosStateMask,		_1,			pb;
/*o6*/	lqd		BufferByt4Qw,		0(BufferByt4Adrs);

/*e4*/	shl		tLiteralPosMask,	_1,			lp;
/*o4*/	rotqby		BufferByt1AtByt0,	BufferByt1Qw,		BufferByt1Adrs;

/*e2*/	sf		picOffset,		picLabel,		picLoc;		
/*o4*/	rotqby		BufferByt2AtByt1,	BufferByt2Qw,		BufferByt1Adrs;

/*e2*/	a		lcPlsLp,		lc,			lp;

/*e2*/	ai		posStateMask,		tPosStateMask,		-1;

/*e4*/	shl		tNumProbs,		_0x00000300,		lcPlsLp;
/*o4*/	rotqby		BufferByt3AtByt2,	BufferByt3Qw,		BufferByt1Adrs;

/*e2*/	ai		literalPosMask,		tLiteralPosMask,	-1;
/*o4*/	rotqby		BufferByt4AtByt3,	BufferByt4Qw,		BufferByt1Adrs;

/*e2*/	il		_0x00000737/*735*/,	0x738-1;

/*e2*/	selb		BufferByts12,		BufferByt1AtByt0,	BufferByt2AtByt1,	_0x00ff00ff;

/*e2*/	a		numProbsMns1,		_0x00000737/*735*/,	tNumProbs;

/*e2*/	selb		BufferByts34,		BufferByt3AtByt2,	BufferByt4AtByt3,	_0x00ff00ff;

/*e2*/	a		tIndex,			numProbsMns1,		numProbsMns1;

/*e2*/	ilh		_0x04000400,		0x800/2;

/*e2*/	andi		index,			tIndex,			-16;

/*e2*/	selb		Code,			BufferByts12,		BufferByts34,		_0x0000ffff;

/*e2*/	ai		index_,			index,			-16;
/*o1*/	stqx		_0x04000400,		index,			p;

/*e2*/	clgt		error,			Buffer,			BufferLim;
	lnop;

/*-----------------------------------------------------------------------*/
_initProbsTo:
/*e2*/	ai		index__/*#2*/,		index_,			-16;
/*o1*/	stqx		_0x04000400,		index_,			p;

/*e2*/	ilhu		_0x01020310,		0x0102;
/*o?*/	brz		index_,			_initProbsExit;

/*e2*/	ai		index_/*#2*/,		index__/*#2*/,		-16;
/*o1*/	stqx		_0x04000400,		index__/*#2*/,		p;

/*e2*/	iohl		_0x01020310,		0x0310;
_initProbsFrom:
/*o?*/	brnz		index__/*#2*/,		_initProbsTo;
/*-----------------------------------------------------------------------*/
_initProbsExit:

/*e2*/	il		_3,			3;
/*o?*/	brnz		error,			_error1;

/*e2*/	ilh		_0x1010Etc,		0x1010;
	lnop;

/*e2*/	ila		gInSizeProcessed,	_gInSizeProcessed;
/*o4*/	cbd		_0x000102Etc/*0x10111203 14151617 18191A1B 1C1D1E1F*/, 0(_3);

/*e2*/	ori		outSize,		$8,			0;
	lnop;

/*e2*/	ila		gOutSizeProcessed,	_gOutSizeProcessed;
/*o6*/	stqx		$6,			picOffset,		gInSizeProcessed;

/*e2*/	il		nowPos,			0;
hbrr		_initialBranchToMainLoop,_mainLoopDoBrzOnly;
andc		_0x000102Etc_/*#2*/,	_0x000102Etc,		_0x1010Etc;
/*o1*/	stqx		$9,			picOffset,		gOutSizeProcessed;

/*e2*/	il		previousByte,		0;
/*o6*/	lqx		atOutStreamNowPosQw,	nowPos,			outStream;

/*e2*/	il		state,			0;

/*e2*/	il		rep0,			1;

/*e2*/	il		rep1,			1;

/*e2*/	il		rep2,			1;

/*e2*/	il		rep3,			1;

/*e2*/	il		Range,			-1;

/*e2*/	ilhu		_0x01000000,		0x0100;

/*e2*/	il		_0x00000800,		0x800;

/*e2*/	ai		_0x01020380,		_0x01020310,		0x80-0x10;

/*e2*/	ai		lcMns8,			lc,			-8;

/*e2*/	il		_0x00000600,		0x300*2;

/*e2*/	il		_0x00000e70/*e6c*/,	0x738*2;

/*e2*/	il		_0x00000664,		0x332*2;

/*e2*/	il		_0x00000360,		0x1b0*2;

/*e2*/	il		_0x00000644,		0x322*2;
	lnop;

clgt		nowPosLtOutSize,	outSize,		nowPos;
_initialBranchToMainLoop:
/*o19*/	br		_mainLoopDoBrzOnly;
/*Note: there will be 1 cycle delay after the branch above (for nowPosLtOutSize)*/


.align	7
_dupDone:
/*e2*/	a		tNumBytsSet,		nowPos/*done*/,		outStreamBackup;



/*e2*/	andi		numBytsSet_,		tNumBytsSet,		0xF;



/*e2*/	ai		numBytsSetMns4,		numBytsSet_,		-4;

/*e2*/	clgt		nowPosLtOutSize,	outSize,		nowPos; 		/* used by _mainLoopDoBrzOnly*/

/*o4*/	rotqby		previousByteInByt3,	atOutStreamNowPosQw,	numBytsSetMns4;







/*e2*/	andi		previousByte,		previousByteInByt3,	0xFF;

/* fall through into _mainLoopDoBrzOnly*/

_mainLoopDoBrzOnly:
/*e4*/	shli		stateShl4,		state,			4;
/*o?*/	brz		nowPosLtOutSize,	_mainLoopExit;
/*-----------------------------------------------------------*/

/*e2*/	clgt		RangeLt0x01000000,	_0x01000000,		Range;
/*o6*/	lqd		atBufferQw,		0(Buffer);

/*e2*/	and		posState,		nowPos,			posStateMask;

/*eo4*/	shli		RangeIfT,		Range,			8;

/*e2*/	a		stateShl4PlsPosState,	stateShl4,		posState;

/*e2*/	ai		BufferIfT,		Buffer,			1;

/*e2*/	a		tProb,			stateShl4PlsPosState,	stateShl4PlsPosState;
	lnop;

/*e2*/	selb		Range/*#2*/,		Range,			RangeIfT,		RangeLt0x01000000;
/*o4*/	rotqby		atBufferAtByt0,		atBufferQw,		Buffer;

/*e2*/	and		tProbL,			nowPos,			literalPosMask;
/*o6*/	lqx		atProbQw,		tProb,			p;

/*e2*/	a		prob,			p,			tProb;

/*e4*/	rotmi		RangeShr11,		Range/*#2*/,		-11;

/*e2*/	ai		probMns2,		prob,			-2;
/*o4*/	shufb		CodeIfT,		Code,			atBufferAtByt0,		_0x01020310;

/*e2*/	ceq		tError,			Buffer,			BufferLim;

/*e4*/	shl		tProbL/*#2*/,		tProbL,			lc;

/*e4*/	rotm		tProbR,			previousByte,		lcMns8;
/*o4*/	rotqby		atProb/*hw1*/,		atProbQw,		probMns2;

/*e2*/	selb		Code/*#2*/,		Code,			CodeIfT,		RangeLt0x01000000;
/*o1*/	hbrr		_beforeCount3,		_fast_count11;

/*e2*/	and		error_,			RangeLt0x01000000,	tError;

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT,		RangeLt0x01000000;

/*e7*/	mpyu		boundMpyLo,		RangeShr11,		atProb;
/*o?*/	brnz		error_,			_error2;

/*e7*/	mpyh		boundMpyHi,		RangeShr11,		atProb;

/*e2*/	a		tProb_/*#2*/,		tProbL/*#2*/,		tProbR;

/*e2*/	sfh		_0x800MnsAtProb,	atProb,			_0x00000800;

/*e7*/	mpya		tProb_/*#3*/,		_0x00000600/*0x300*2*/,	tProb_/*#2*/,		_0x00000e70;/*e6c*//*0x736*2*/

/*e4*/	rothmi		_0x800MnsAtProbShr5,	_0x800MnsAtProb,	-5;




  
/*e2*/	a		bound,			boundMpyHi,		boundMpyLo;



/*e2*/	clgt		CodeLtBound,		bound,			Code;/*#2*/
/*o4*/	chd		atProbMask,		0(prob);

	nop $36;

/*o?*/	brz		CodeLtBound,		_count19;
/*--------------------------------------------------------*/

/*e2*/	ah		atProb_/*#2*/,		atProb,			_0x800MnsAtProbShr5;

/*e2*/	il		symbol,			1;

/*o4*/	shufb		atProbQw_/*#2*/,		atProb_/*#2*/,		atProbQw,		atProbMask;

/*e2*/	ori		Range,			bound,			0;

/*e2*/	cgti		stateGe7,		state,			7-1;



/*o1*/	stqd		atProbQw_/*#2*/,		0(prob);

/*e2*/	a		prob_/*#2*/,		p,			tProb_;/*#3*/
_beforeCount3:
/*o?*/	brz		stateGe7,		_fast_count11;

sf		nowPosMnsRep0,		rep0,			nowPos;
/*o15*/	hbrr		_beforeCount10,		_count4;



/*e2*/	a		matchByteAdrs,		outStream,		nowPosMnsRep0;
/*o6*/	lqx		matchByteQw,		nowPosMnsRep0,		outStream;



/*e2*/	ai		matchByteAdrsMns3,	matchByteAdrs,		-3;







/*o4*/	rotqby		tMatchByte/*byt3*/,	matchByteQw,		matchByteAdrsMns3;







/*e2*/	andi		matchByte,		tMatchByte,		0xFF;



/*e2*/	a		tMatchByte/*matchByte#2*/,matchByte,		matchByte;

/*e2*/	ai		symbolPls0x100,		symbol,			0x100;

/*e2*/	andi		tBit,			tMatchByte/*matchByte#2*/,0x100;



/*e2*/	a		tProbLit,		symbolPls0x100,		tBit;
	lnop;
	


/*.align	3*/
_count4: /*=============================================================================================*/
  /*e2*/	a		tProbLit/*#2*/,		tProbLit,		tProbLit;
  /*o4*/	shlqbyi/*move*/	matchByte/*#2*/,		tMatchByte/*matchByte#2*/,0;

  /*eo4*/shli		RangeIfT,		Range,			8;
  /*o6*/	lqd		atBufferQw_,		0(Buffer);

  /*e2*/ai		BufferIfT_,		Buffer,			1;
  /*o6*/	lqx		atProbLitQw,		tProbLit/*#2*/,		prob_;			

  /*e2*/	clgt		RangeLt0x01000000_,	_0x01000000,		Range;
  /*o4*/	shlqbyi/*move*/	bit,			tBit,			0;

  /*e2*/	a		probLit,		prob_,			tProbLit;/*#2*/

  /*e2*/	selb		Range/*#2*/,		Range,			RangeIfT,		RangeLt0x01000000_;

  /*e2*/	ai		probLitMns2,		probLit,		-2;
	lnop;
	
/*e2*/	a		tMatchByte/*matchByte#2*/,matchByte,		matchByte;
  /*o4*/	rotqby		atBufferAtByt0_,		atBufferQw_,		Buffer;

  /*e4*/	rotmi		RangeShr11_,		Range/*#2*/,		-11;
  /*o4*/	rotqby		atProbLit/*hw1*/,	atProbLitQw,		probLitMns2;

  /*e2*/selb		Buffer/*#2*/,		Buffer,			BufferIfT_,		RangeLt0x01000000_;

/*e2*/	andi		tBit,			tMatchByte/*matchByte#2*/,0x100;

  /*e2*/	a		symbolMpy2,		symbol,			symbol;

  /*e7*/	mpyu		boundMpyLo_,		RangeShr11_,		atProbLit;

  /*e7*/	mpyh		boundMpyHi_,		RangeShr11_,		atProbLit;

  /*e4*/	rothmi		atProbLitShr5,		atProbLit,		-5;

  /*e2*/	sfh		_0x800MnsAtProbLit,	atProbLit,		_0x00000800;
  /*o4*/	shufb		CodeIfT_,		Code,			atBufferAtByt0_,		_0x01020310;

  

  /*e4*/	rothmi		_0x800MnsAtProbLitShr5, _0x800MnsAtProbLit,	-5;

  /*e2*/	sfh		atProbLitIfF,		atProbLitShr5,		atProbLit;

  /*e2*/	selb		Code/*#2*/,		Code,			CodeIfT_,		RangeLt0x01000000_;

  /*e2*/	a		bound_,			boundMpyHi_,		boundMpyLo_;

  /*e2*/	ah		atProbLitIfT,		atProbLit,		_0x800MnsAtProbLitShr5;
  /*o4*/	chd		atProbLitMask,		0(probLit);
	
  /*e2*/	clgt		CodeLtBound_,		bound_,			Code;/*#2*/

  /*e2*/	sf		RangeIfF,		bound_,			Range;/*#2*/
  
  /*e2*/	selb		atProbLit_/*#2*/,		atProbLitIfF,		atProbLitIfT,		CodeLtBound_;

  /*e2*/	ai		CodeLtBoundPlus1,	CodeLtBound_,		1;

  /*e2*/	selb		Range/*#3*/,		RangeIfF,		bound_,			CodeLtBound_;
  /*o4*/	shufb		atProbLitQw_/*#2*/,	atProbLit_/*#2*/,	atProbLitQw,		atProbLitMask;

  /*e2*/	sf		CodeIfF,		bound_,			Code;/*#2*/

  /*e2*/	a		symbol/*#2*/,		symbolMpy2,		CodeLtBoundPlus1;

  /*e2*/	and		breakIfNz,		bit,			CodeLtBound_;

  /*e2*/	or		breakIfZ,		bit,			CodeLtBound_;
	
  /*e2*/	selb		Code/*#3*/,		CodeIfF,		Code/*#2*/,		CodeLtBound_;
  /*o1*/	stqd		atProbLitQw_/*#2*/,	0(probLit);

/*e2*/	ai		symbolPls0x100,		symbol,			0x100;
	lnop;
	  
  /*e2*/	cgti		symbolNotLt0x100,	symbol/*#2*/,		0x100-1;
  /*o?*/	brnz		breakIfNz,		_count10;
/*-------------------------------------------------*/

/*e2*/	a		tProbLit,		symbolPls0x100,		tBit;
  /*o?*/	brz		breakIfZ,		_count10;
/*-------------------------------------------------*/

	nop $38;
_beforeCount10:	
  /*o?*/	brz		symbolNotLt0x100,	_count4;	/*......................................................*/
/*===============================================================================================================*/

/*17 or 18 cycles to reload pipeline*/

.align	4 /* 3*/
_count10:
/*e2*/	clgt		error__,			Buffer,			BufferLim;
/*o15*/	hbrr		_beforeCount19,		_mainLoopDoBrzOnly;
/*NOTE: above hbrr put here so hint has time to register if brnz goes to _count14pt5*/

  /*e2*/	cgti		symbolNotLt0x100_/*#2*/,	symbol/*#2*/,		0x100-1;

/*o?*/	brnz		error__,			_error3;

	nop $35;
/*o?*/	brnz		symbolNotLt0x100_/*#2*/,	_count14pt5;

_count11: 
/*e2*/	a		symbolMpy2_,		symbol,			symbol;
/*o6*/	lqd		atBufferQw__,		0(Buffer);

/*e2*/	clgt		RangeLt0x01000000__,	_0x01000000,		Range;
/*oe4*/	shufb/*shl 8*/	RangeIfT_,		Range,			Range,			_0x01020380;

/*e2*/	a		probLit_,		prob_,			symbolMpy2_;
/*o6*/	lqx		atProbLitQw_,		symbolMpy2_,		prob_;



/*e2*/	ai		probLitMns2_,		probLit_,		-2;

/*e2*/	selb		Range_,			Range,			RangeIfT_,		RangeLt0x01000000__;

/*o4*/	rotqby		atBufferAtByt0__,		atBufferQw__,		Buffer;

/*e4*/	rotmi		RangeShr11__,		Range_,			-11;

	nop $80;
/*o4*/	rotqby		atProbLit__/*hw1*/,		atProbLitQw_,		probLitMns2_;



/*.align	3*/
_count11Loop:
  /*xx*/	/* finish: previous pass*/
/*xx*/	/* start:  new pass*/
/*======================================================================================================================*/
  /*e2*/	ai		symbolMpy2Plus1,	symbolMpy2_,		1;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ai		BufferIfT__,		Buffer,			1;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e7*/	mpyu		boundMpyLo__,		RangeShr11__,		atProbLit__;
  /*o4*/	shufb		CodeIfT__,		Code,			atBufferAtByt0__,		_0x01020310;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e7*/	mpyh		boundMpyHi__,		RangeShr11__,		atProbLit__;
  /*o15*/	hbrr		_beforeCount14pt5,	_count11Loop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e4*/	rothmi		atProbLitShr5_,		atProbLit__,		-5;
  /*o4*/	chd		atProbLitMask_,		0(probLit_);
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sfh		_0x800MnsAtProbLit_,	atProbLit__,		_0x00000800;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Code_,			Code,			CodeIfT__,		RangeLt0x01000000__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e4*/	rothmi		_0x800MnsAtProbLitShr5_, _0x800MnsAtProbLit_,	-5;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Buffer,			Buffer,			BufferIfT__,		RangeLt0x01000000__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sfh		atProbLitIfF_,		atProbLitShr5_,		atProbLit__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	a		bound__,			boundMpyHi__,		boundMpyLo__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ah		atProbLitIfT_,		atProbLit__,		_0x800MnsAtProbLitShr5_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	clgt		CodeLtBound__,		bound__,			Code_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sf		RangeIfF_,		bound__,			Range_;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	a		symbol,			symbolMpy2Plus1,	CodeLtBound__;
  /*o4*/	shlqbii/*move*/	atProbLitQw_,		atProbLitQw_,		0;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Range,			RangeIfF_,		bound__,			CodeLtBound__;
  /*o4*/	shlqbii/*move*/	probLit__,		probLit_,		0;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		symbolMpy2_,		symbol,			symbol;
/*o6*/	lqd		atBufferQw___,		0(Buffer);
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		atProbLit___,		atProbLitIfF_,		atProbLitIfT_,		CodeLtBound__;
/*oe4*/	shufb/*shl 8*/	RangeIfT__,		Range,			Range,			_0x01020380;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		probLit_,		prob_,			symbolMpy2_;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2v*/	sf		CodeIfF_,		bound__,			Code_;
  /*o4v*/	shufb		atProbLitQw_,		atProbLit___,		atProbLitQw_,		atProbLitMask_;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	clgt		RangeLt0x01000000__,	_0x01000000,		Range;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	ai		probLitMns2__,		probLit_,		-2;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	selb		Range_,			Range,			RangeIfT__,		RangeLt0x01000000__;
/*o4*/	rotqby		atBufferAtByt0__,		atBufferQw___,		Buffer;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	cgti		symbolNotLt0x100__,	symbol,			0x100-1;
  /*o1*/	stqd		atProbLitQw_,		0(probLit__);
/*----------------------------------------------------------------------------------------------------------------------*/
/*e4*/	rotmi		RangeShr11___,		Range_,			-11;
/*o6*/	lqx		atProbLitQw_,		symbolMpy2_,		prob_;
/*----------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------*/
/*o4*/	shlqbyi/*delay!*/	RangeShr11__,		RangeShr11___,		0;
/*----------------------------------------------------------------------------------------------------------------------*/
/*o15*/	hbrr		_freeHintedBranchToCount14pt5, _count14pt5;
/*----------------------------------------------------------------------------------------------------------------------*/
	nop $84;
/*o4*/	rotqby		atProbLit__/*hw1*/,		atProbLitQw_,		probLitMns2__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Code,			CodeIfF_,		Code_,			CodeLtBound__;
_beforeCount14pt5:
  /*o?*/	brz		symbolNotLt0x100__,	_count11Loop;
/*======================================================================================================================*/

/*17 or 18 cycles to reload pipeline, which allows free hint to register!*/

/*e2*/	clgt		error___,			Buffer,			BufferLim;
/*o15*/	hbrr		_beforeCount19,		_mainLoopDoBrzOnly;
/*NOTE: above hbrr put here so hint has time to register if brz goes to _count14pt5*/

	nop $8;	

_freeHintedBranchToCount14pt5:
/*o?*/	brz		error___,			_count14pt5;

	nop $127;
/*o?*/	br		_error4;

/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	interleaved version of most frequent flow*/
/*	This is the more frequent version, from before count3, where we know:*/
/*		symbol will be 1 at start*/
/*		The first 3 loops share the exact same "prob" qword.*/
/*		The next loop prob qword is unique and known*/
/*		The last 4 loop prob qwords are unique but can't be prefetched at the start*/
/*		The first 3 or 4 loops could be unrolled, and the last 4 or 5 loops could be looped code*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/

.align 5
_fast_count11:

/* In this code, "symbol" starts as 1 for the first loop.  In the subsequent loops it is 0b1x, 0b1xx, 0b1xxx, 0b1xxxx, 0b1xxxxx,*/
/*	0b1xxxxxx, and 0b1xxxxxxx.  "p" (the start of the probs array) is qword aligned.  The start of the "Literal" portion of the*/
/*	probs array is offset 0x738*2 from p.  Note 0x738 has been increased from 0x736 to ensure it is qword aligned.*/
/*	The probLit is thus guaranteed to be qword aligned.  This then guarantees that the first 3 iterations of the loop will load and store*/
/*	probLit qwords from the same qword, and thus it only has to be loaded once, and stored once, since the qword can be held in*/
/*	a register for the first 3 iterations.*/

/* The interleaved version is optimized for 1 loop per change of symbol, where probLit fetches of one loop are in a different*/
/*	qword from the next loop, and thus you don't have to worry about having a store before the next load.*/
/*	To be clear: the probLit store of one loop is *after* the probLit fetch of the next loop, and this only works because*/
/*	they are in different qwords.  In the first 3 iterations of the loop, all loads and stores are in the same qword,*/
/*	so I used the same flow as the main loop, but removed all the loads and stores (except the first load and last store) so that they*/
/*	operate from the same qword in a register, which happens to have the data to store of one loop valid before the next loop needs*/
/*	to fetch the data to read from the register.  Thus the initial 3 iterations are the same, with only a few load, store, and*/
/*	loop instructions removed.*/

/* Each loop executes two pipelines for speed.  The first pipeline has "{xx}" at the start of the line.*/
/*	The second pipeline has "  {xx}" at the start of the line (the 2 spaces indicate it's a latter pipeline stage).*/
/*	The first letter is 'e' or 'o' for the even or odd pipeline*/
/*	The second number is the number of latency cycles, or how long to complete*/
/*	For the first iteration, there is no second pipeline to finish, and so those instructions are removed.*/
/*	For the last iteration (in the main loop) which only needs to complete the second(final) pipeline,*/
/*	the first pipeline can be ignored as it does not alter any values needed afterwards.*/

/* Thus the code shown below is for:*/
/*			symbol*/
/*			-----------*/
/*	start		0b00000001*/

/*	finish		0b00000001*/
/*	start		0b0000001x*/

/*	finish		0b0000001x*/
/*	start		0b000001xx*/

/*  looped code:	1st pass	2nd pass	3rd pass	4th pass	5th pass	6th pass*/
/*	finish		0b000001xx	0b00001xxx	0b0001xxxx	0b001xxxxx	0b01xxxxxx	0b1xxxxxxx*/
/*	start		0b00001xxx	0b0001xxxx	0b001xxxxx	0b01xxxxxx	0b1xxxxxxx	(ignored)*/


	/*		symbol:*/
/*xx*/	/* start:	0b00000001*/
/*======================================================================================================================*/
/*e2*/	a		symbolMpy2_,		symbol,			symbol;	
/*o6*/	lqd		atBufferQw__,		0(Buffer);
/*-----------------------------------------------------------------------------------------------------------------------*/
	nop $77; /*added to maintain aligned*/
/*oe4*/	shufb/*shl 8*/	RangeIfT_,		Range,			Range,			_0x01020380;
/*-----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		probLit___,		prob_,			symbolMpy2_;
/*o6*/	lqx		atProbLitQw__,		symbolMpy2_,		prob_;
/*-----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	clgt		RangeLt0x01000000___,	_0x01000000,		Range;
/*-----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	ai		probLitMns2_,		probLit___,		-2;
/*-----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	selb		Range__,			Range,			RangeIfT_,		RangeLt0x01000000___;
/*-----------------------------------------------------------------------------------------------------------------------*/
/*o4*/	rotqby		atBufferAtByt0__,		atBufferQw__,		Buffer;
/*-----------------------------------------------------------------------------------------------------------------------*/
/*e4*/	rotmi		RangeShr11____,		Range__,			-11;
/*-----------------------------------------------------------------------------------------------------------------------*/
/*o4*/	rotqby		atProbLit__/*hw1*/,		atProbLitQw__,		probLitMns2_;
/*-----------------------------------------------------------------------------------------------------------------------*/
	nop $127; /*added to maintain alignment)
{o15*/	hbrr		_fast_beforeCount14pt5,	_fast_count11Loop; /*hint put here since this would be a cycle delay below anyway*/
/*======================================================================================================================*/

/*.align	3*/
	/*		symbol:*/
  /*xx*/	/* finish:	0b00000001*/
/*xx*/	/* start:	0b0000001x*/
/*======================================================================================================================*/
  /*e2*/	ai		symbolMpy2Plus1_,	symbolMpy2_,		1;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ai		BufferIfT___,		Buffer,			1;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e7*/	mpyu		boundMpyLo___,		RangeShr11____,		atProbLit__;
  /*o4*/	shufb		CodeIfT__,		Code,			atBufferAtByt0__,		_0x01020310;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e7*/	mpyh		boundMpyHi,		RangeShr11____,		atProbLit__;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e4*/	rothmi		atProbLitShr5_,		atProbLit__,		-5;
  /*o4*/	chd		atProbLitMask__,		0(probLit___);
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sfh		_0x800MnsAtProbLit__,	atProbLit__,		_0x00000800;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Code_,			Code,			CodeIfT__,		RangeLt0x01000000___;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e4*/	rothmi		_0x800MnsAtProbLitShr5_, _0x800MnsAtProbLit__,	-5;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Buffer,			Buffer,			BufferIfT___,		RangeLt0x01000000___;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sfh		atProbLitIfF__,		atProbLitShr5_,		atProbLit__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	a		bound___,			boundMpyHi,		boundMpyLo___;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ah		atProbLitIfT_,		atProbLit__,		_0x800MnsAtProbLitShr5_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	clgt		CodeLtBound___,		bound___,			Code_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sf		RangeIfF,		bound___,			Range__;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	a		symbol_,			symbolMpy2Plus1_,	CodeLtBound___;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Range,			RangeIfF,		bound___,			CodeLtBound___;
	lnop; /* was: {o4} shlqbii{move}	probLit_, probLit, 0*/
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		symbolMpy2_,		symbol_,			symbol_;
/*o6*/	lqd		atBufferQw____,		0(Buffer);
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		atProbLit___,		atProbLitIfF__,		atProbLitIfT_,		CodeLtBound___;
/*oe4*/	shufb/*shl 8*/	RangeIfT___,		Range,			Range,			_0x01020380;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		probLit____,		prob_,			symbolMpy2_;
	lnop; /* was:{o6} lqx atProbLitQw,	symbolMpy2,		prob*/
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2v*/	sf		CodeIfF__,		bound___,			Code_;
  /*o4v*/	shufb		atProbLitQw___,		atProbLit___,		atProbLitQw__,		atProbLitMask__;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	clgt		RangeLt0x01000000__,	_0x01000000,		Range;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	ai		probLitMns2___,		probLit____,		-2;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	selb		Range__,			Range,			RangeIfT___,		RangeLt0x01000000__;
/*o4*/	rotqby		atBufferAtByt0___,		atBufferQw____,		Buffer;
/*----------------------------------------------------------------------------------------------------------------------*/
	/* was: {e2} cgti symbolNotLt0x100, symbol, 0x100-1*/
	/* was: {o1} stqd atProbLitQw_, 0(probLit_)*/
/*----------------------------------------------------------------------------------------------------------------------*/
/*e4*/	rotmi		RangeShr11,		Range__,			-11;
/*o4*/	rotqby		atProbLit____/*hw1*/,		atProbLitQw___,		probLitMns2___;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Code,			CodeIfF__,		Code_,			CodeLtBound___;
	lnop; /* was: {o?} brz symbolNotLt0x100, _fast_count11Loop*/
/*======================================================================================================================*/

/*.align	3*/
	/*		symbol:*/
  /*xx*/	/* finish:	0b0000001x*/
/*xx*/	/* start:	0b000001xx*/
/*======================================================================================================================*/
  /*e2*/	ai		symbolMpy2Plus1_,	symbolMpy2_,		1;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ai		BufferIfT____,		Buffer,			1;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e7*/	mpyu		boundMpyLo_,		RangeShr11,		atProbLit____;
  /*o4*/	shufb		CodeIfT___,		Code,			atBufferAtByt0___,		_0x01020310;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e7*/	mpyh		boundMpyHi___,		RangeShr11,		atProbLit____;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e4*/	rothmi		atProbLitShr5_,		atProbLit____,		-5;
  /*o4*/	chd		atProbLitMask___,		0(probLit____);
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sfh		_0x800MnsAtProbLit__,	atProbLit____,		_0x00000800;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Code_,			Code,			CodeIfT___,		RangeLt0x01000000__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e4*/	rothmi		_0x800MnsAtProbLitShr5__, _0x800MnsAtProbLit__,	-5;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Buffer,			Buffer,			BufferIfT____,		RangeLt0x01000000__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sfh		atProbLitIfF___,		atProbLitShr5_,		atProbLit____;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	a		bound____,			boundMpyHi___,		boundMpyLo_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ah		atProbLitIfT__,		atProbLit____,		_0x800MnsAtProbLitShr5__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	clgt		CodeLtBound____,		bound____,			Code_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sf		RangeIfF,		bound____,			Range__;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	a		symbol_,			symbolMpy2Plus1_,	CodeLtBound____;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Range,			RangeIfF,		bound____,			CodeLtBound____;
	lnop; /* was:{o4} shlqbii{move} probLit_, probLit, 0*/
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		symbolMpy2_,		symbol_,			symbol_;
/*o6*/	lqd		atBufferQw_____,		0(Buffer);
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		atProbLit_____,		atProbLitIfF___,		atProbLitIfT__,		CodeLtBound____;
/*oe4*/	shufb/*shl 8*/	RangeIfT,		Range,			Range,			_0x01020380;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		probLit_____,		prob_,			symbolMpy2_;
	lnop; /* was:{o6} lqx atProbLitQw,	symbolMpy2,		prob*/
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sf		CodeIfF___,		bound____,			Code_;
  /*o4*/	shufb		atProbLitQw___,		atProbLit_____,		atProbLitQw___,		atProbLitMask___;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	clgt		RangeLt0x01000000_,	_0x01000000,		Range;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	ai		probLitMns2____,		probLit_____,		-2;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	selb		Range___,			Range,			RangeIfT,		RangeLt0x01000000_;
/*o4*/	rotqby		atBufferAtByt0____,		atBufferQw_____,		Buffer;
/*----------------------------------------------------------------------------------------------------------------------*/
	/* was: {e2} cgti symbolNotLt0x100, symbol, 0x100-1*/
	/* was: {o1} stqd atProbLitQw_, 0(probLit_)*/
/*----------------------------------------------------------------------------------------------------------------------*/
/*e4*/	rotmi		RangeShr11_____,		Range___,			-11;
/*o4*/	rotqby		atProbLit/*hw1*/,		atProbLitQw___,		probLitMns2____;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Code,			CodeIfF___,		Code_,			CodeLtBound____;
	lnop; /* was: {o?} brz symbolNotLt0x100, _fast_count11Loop*/
/*======================================================================================================================*/

/*.align	3*/
_fast_count11Loop:
	/*	symbol:	1st pass	2nd pass	3rd pass	4th pass	5th pass	6th pass*/
  /*xx*/	/* finish:	0b000001xx	0b00001xxx	0b0001xxxx	0b001xxxxx	0b01xxxxxx	0b1xxxxxxx*/
/*xx*/	/* start:	0b00001xxx	0b0001xxxx	0b001xxxxx	0b01xxxxxx	0b1xxxxxxx	(ignored)*/
/*======================================================================================================================*/
  /*e2*/	ai		symbolMpy2Plus1,	symbolMpy2_,		1;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ai		BufferIfT__,		Buffer,			1;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e7*/	mpyu		boundMpyLo__,		RangeShr11_____,		atProbLit;
  /*o4*/	shufb		CodeIfT____,		Code,			atBufferAtByt0____,		_0x01020310;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e7*/	mpyh		boundMpyHi_,		RangeShr11_____,		atProbLit;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e4*/	rothmi		atProbLitShr5__,		atProbLit,		-5;
  /*o4*/	chd		atProbLitMask____,		0(probLit_____);
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sfh		_0x800MnsAtProbLit_,	atProbLit,		_0x00000800;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Code_,			Code,			CodeIfT____,		RangeLt0x01000000_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e4*/	rothmi		_0x800MnsAtProbLitShr5___, _0x800MnsAtProbLit_,	-5;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Buffer,			Buffer,			BufferIfT__,		RangeLt0x01000000_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sfh		atProbLitIfF____,		atProbLitShr5__,		atProbLit;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	a		bound_____,			boundMpyHi_,		boundMpyLo__;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ah		atProbLitIfT___,		atProbLit,		_0x800MnsAtProbLitShr5___;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	clgt		CodeLtBound_____,		bound_____,			Code_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sf		RangeIfF_,		bound_____,			Range___;
	lnop;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	a		symbol,			symbolMpy2Plus1,	CodeLtBound_____;
  /*o4*/	shlqbii/*move*/	atProbLitQw__,		atProbLitQw___,		0;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Range,			RangeIfF_,		bound_____,			CodeLtBound_____;
  /*o4*/	shlqbii/*move*/	probLit______,		probLit_____,		0;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		symbolMpy2_,		symbol,			symbol;
/*o6*/	lqd		atBufferQw______,		0(Buffer);
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		atProbLit______,		atProbLitIfF____,		atProbLitIfT___,		CodeLtBound_____;
/*oe4*/	shufb/*shl 8*/	RangeIfT____,		Range,			Range,			_0x01020380;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		probLit_____,		prob_,			symbolMpy2_;
/*o6*/	lqx		atProbLitQw___,		symbolMpy2_,		prob_;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	sf		CodeIfF____,		bound_____,			Code_;
  /*o4*/	shufb		atProbLitQw____,		atProbLit______,		atProbLitQw__,		atProbLitMask____;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	clgt		RangeLt0x01000000_,	_0x01000000,		Range;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	ai		probLitMns2_____,		probLit_____,		-2;
/*----------------------------------------------------------------------------------------------------------------------*/
/*e2*/	selb		Range___,			Range,			RangeIfT____,		RangeLt0x01000000_;
/*o4*/	rotqby		atBufferAtByt0____,		atBufferQw______,		Buffer;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	cgti		symbolNotLt0x100___,	symbol,			0x100-1;
  /*o1*/	stqd		atProbLitQw____,		0(probLit______);
/*----------------------------------------------------------------------------------------------------------------------*/
/*e4*/	rotmi		RangeShr11_____,		Range___,			-11;
/*o4*/	rotqby		atProbLit/*hw1*/,		atProbLitQw___,		probLitMns2_____;
/*----------------------------------------------------------------------------------------------------------------------*/
  /*e2*/	selb		Code,			CodeIfF____,		Code_,			CodeLtBound_____;
_fast_beforeCount14pt5:
  /*o?*/	brz		symbolNotLt0x100___,	_fast_count11Loop;
/*======================================================================================================================*/

/*e2*/	clgt		error___,			Buffer,			BufferLim;

	
	
/*o?*/	brnz		error___,			_error5;

nop $127;
/*o15*/	hbrr		_beforeCount19,		_mainLoopDoBrzOnly;
/*NOTE: above hbrr put here so hint has time to register*/

/*.align	3*/
_count14pt5:
/*e2*/	cgti		stateGe10,		state,			10-1;
	lnop;
/*{o15}	hbrr		_beforeCount19,		_mainLoopDoBrzOnly*/
/*NOTE: above hbrr moved to callers so there's enough room to register!*/

/*e2*/	ai		stateIf4To9,		state,			-3;
/*o4*/	cbx		atOutStreamNowPosMask,	nowPos,			outStream;

/*e2*/	ai		stateIfGe10,		state,			-6;

/*e2*/	andi		previousByte,		symbol,			0xff;

/*e2*/	cgti		stateGe4,		state,			4-1;
	lnop;

/*e2*/	selb		stateIfGe4,		stateIf4To9,		stateIfGe10,		stateGe10;
/*o4*/	shufb		atOutStreamNowPosQw,	previousByte,		atOutStreamNowPosQw,	atOutStreamNowPosMask;



/*e2*/	selb		state,			stateGe4,		stateIfGe4,		stateGe4;



/*o6*/	stqx		atOutStreamNowPosQw,	nowPos,			outStream;

/*e2*/	ai		nowPos/*#2*/,		nowPos,			1;



/*e2*/	clgt		nowPosLtOutSize,	outSize,		nowPos;/*#2*/

/*NOTE: without the nop/lnop below, one(of 2) of the branches to _count14pt5*/
/* that hints the br below would have a pipeline reload after the br*/
	nop $127;
	lnop;

/*NOTE: if the code above was fallen through, there will be 1 cycle delay as the hint could not be early enough	*/

	nop $127;
_beforeCount19:
/*o?*/	br		_mainLoopDoBrzOnly;

.align 5
_count19:	
/*e4*/	rothmi		atProbShr5,		atProb,			-5;
/*o15*/	hbrr		_aboveCount22,		_count35;

/*o4*/	chd		atProbMask_,		0(prob);





/*e2*/	sfh		atProb__/*#2*/,		atProbShr5,		atProb;



/*o4*/	shufb		atProbQw__/*#2*/,		atProb__/*#2*/,		atProbQw,		atProbMask_;

/*e2*/	ai		tProb__,			p,			0xc0*2;

/*e2*/	sf		Range/*#2*/,		bound,			Range;

/*e2*/	a		stateMpy2,		state,			state;

/*e4*/	shli		RangeIfT___,		Range/*#2*/,		8;
/*o1*/	stqd/*st1*/	atProbQw__/*#2*/,		0(prob);

/*o6*/	lqx/*after st1*/	atProbQw__/*#3*/,		stateMpy2,		tProb__;

/*e2*/	clgt		RangeLt0x01000000____,	_0x01000000,		Range;/*#2*/

/*e2*/	a		prob__/*#2*/,		tProb__,			stateMpy2;
/*o6*/	lqd		atBufferQw_______,		0(Buffer);

/*e2*/	selb		Range/*#3*/,		Range/*#2*/,		RangeIfT___,		RangeLt0x01000000____;

/*e2*/	ai		probMns2_,		prob__/*#2*/,		-2;

/*e4*/	rotmi		RangeShr11___,		Range/*#3*/,		-11;

/*o4*/	rotqby		atProb___/*hw1,#3*/,		atProbQw__/*#3*/,		probMns2_;

/*e2*/	ai		BufferIfT___,		Buffer,			1;

/*o4*/	rotqby		atBufferAtByt0_,		atBufferQw_______,		Buffer;

/*e2*/	sf		Code/*#2*/,		bound,			Code;

/*e7*/	mpyu		boundMpyLo,		RangeShr11___,		atProb___;/*#3*/

/*e7*/	mpyh		boundMpyHi____,		RangeShr11___,		atProb___;/*#3*/

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT___,		RangeLt0x01000000____;

/*e2*/	sfh		_0x800MnsAtProb_,	atProb___/*#3*/,		_0x00000800;
/*o4*/	shufb		CodeIfT_,		Code/*#2*/,		atBufferAtByt0_,		_0x01020310;

/*e2*/	clgt		error____,			Buffer,			BufferLim;
	lnop;
	
/*e4*/	rothmi		_0x800MnsAtProbShr5_,	_0x800MnsAtProb_,	-5;
/*o4*/	chd		atProbMask__/*#2*/,		0(prob__/*#2*/);



/*e2*/	selb		Code/*#3*/,		Code/*#2*/,		CodeIfT_,		RangeLt0x01000000____;
/*o?*/	brnz		error____,			_error6;

/*e2*/	a		bound/*#2*/,		boundMpyHi____,		boundMpyLo;



/*e2*/	clgt		CodeLtBound______,		bound/*#2*/,		Code;/*#3*/



	nop $35;
/*o?*/	brz		CodeLtBound______,		_count22;
/*----------------------------------------------*/

/*e2*/	ah		atProb_/*#4*/,		atProb___/*#3*/,		_0x800MnsAtProbShr5_;

/*e2*/	ori		Range____/*#4*/,		bound/*#2*/,		0;

/*e2*/	ori		rep3,			rep2,			0;
/*o4*/	shufb		atProbQw__/*#4*/,		atProb_/*#4*/,		atProbQw__/*#3*/,		atProbMask__;/*#2*/

/*e2*/	cgti		stateGe7_,		state,			7-1;

/*e2*/	ori		rep2/*#2*/,		rep1,			0;

/*e2*/	ori		rep1/*#2*/,		rep0,			0;
	lnop;

/*e2*/	selb		state/*#2*/,		stateGe7_,		_3,			stateGe7_;
/*o1*/	stqd		atProbQw__/*#4*/,		0(prob__/*#2*/);

/*e2*/	a		prob_/*#3*/,		p,			_0x00000664;/*0x332*2*/
_aboveCount22:	
/*o?*/	br		_count35;
							 
.align	5
_count22:
/*e4*/	rothmi		atProbShr5_,		atProb___,			-5;
/*o4*/	chd		atProbMask___,		0(prob__);

/*e2*/	ai		tProbL_,			p,			0xcc*2;
/*o6*/	lqd		atBufferQw____,		0(Buffer);

/*e2*/	ai		tProbL__/*tProbL#2*/,	p,			0xf0*2;

/*e4*/	shli		stateShl5,		state,			4+1;

/*e2*/	sfh		atProb____/*#2*/,		atProbShr5_,		atProb___;

/*e2*/	a		stateMpy2_,		state,			state;

/*e2*/	sf		Code/*#2*/,		bound,			Code;
/*o4*/	shufb		atProbQw__/*#2*/,		atProb____/*#2*/,		atProbQw__,		atProbMask___;

/*e2*/	ai		BufferIfT_____,		Buffer,			1;

/*e2*/	a		posStateMpy2,		posState,		posState;

/*e2*/	sf		Range/*#2*/,		bound,			Range;
/*o4*/	rotqby		atBufferAtByt0_____,		atBufferQw____,		Buffer;

/*e2*/	sf		nowPosMnsRep0_,		rep0,			nowPos;
/*o1*/	stqd		atProbQw__/*#2*/,		0(prob__);

/*eo4*/	shli		RangeIfT_____,		Range/*#2*/,		8;
/*o6*/	lqx		atProbQw___/*#3*/,		tProbL_,			stateMpy2_;

/*e2*/	a		prob___/*#2*/,		tProbL_,			stateMpy2_;
/*o6*/	lqx		previousByteQw,		nowPosMnsRep0_,		outStream;

/*e2*/	clgt		RangeLt0x01000000_____,	_0x01000000,		Range;/*#2*/
/*o4*/	shufb		CodeIfT_____,		Code/*#2*/,		atBufferAtByt0_____,		_0x01020310;

/*e2*/	ai		probMns2__,		prob___/*#2*/,		-2;

/*e2*/	selb		Range_____/*#3*/,		Range/*#2*/,		RangeIfT_____,		RangeLt0x01000000_____;

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT_____,		RangeLt0x01000000_____;
	lnop;

/*e4*/	rotmi		RangeShr11______,		Range_____/*#3*/,		-11;
/*o4*/	rotqby		atProb____/*hw1,#3*/,		atProbQw___/*#3*/,		probMns2__;

/*e2*/	selb		Code/*#3*/,		Code/*#2*/,		CodeIfT_____,		RangeLt0x01000000_____;
/*o6*/	lqd		tatBufferQw/*atBufferQw#2*/,0(Buffer/*#2*/);

/*e2*/	ai		tBufferIfT/*BufferIfT#2*/,Buffer/*#2*/,		1;

/*e2*/	a		tProbL__/*#3*/,		tProbL__/*tProbL#2*/,	stateShl5;

/*e7*/	mpyu		boundMpyLo____,		RangeShr11______,		atProb____;/*#3*/

/*e7*/	mpyh		boundMpyHi_____,		RangeShr11______,		atProb____;/*#3*/

/*e2*/	sfh		_0x800MnsAtProb__,	atProb____/*#3*/,		_0x00000800;

/*e2*/	a		previousByteAdrs,	nowPosMnsRep0_,		outStream;

/*e4*/	rothmi		_0x800MnsAtProbShr5__,	_0x800MnsAtProb__,	-5;
	lnop;

/*e2*/	cgti		stateGe7__,		state,			7-1;
/*o4*/	rotqby		tPreviousByte,		previousByteQw,		previousByteAdrs;

/*e2*/	clgt		error_____,			Buffer,			BufferLim;

/*e2*/	a		tState,			stateGe7__,		stateGe7__;

/*e2*/	a		bound______/*#2*/,		boundMpyHi_____,		boundMpyLo____;
/*o?*/	brnz		error_____,			_error7;

/*e2*/	ah		tatProb/*atProb#4*/,	atProb____/*#3*/,		_0x800MnsAtProbShr5__;
/*o4*/	chd		tatProbMask/*atProbMask#2*/,0(prob___/*#2*/);

/*e2*/	clgt		CodeLtBound_____,		bound______/*#2*/,		Code;/*#3*/

/*e2*/	ori		tRange/*Range#4*/,	bound______/*#2*/,		0;

	nop $45;
/*o?*/	brz		CodeLtBound_____,		_count28;
/*---------------------------------------------------------*/

/*eo4*/	shli		tRangeIfT/*RangeIfT#2*/,	tRange/*Range#4*/,	8;
/*o4*/	shufb		atProbQw____/*#4*/,		tatProb/*atProb#4*/,	atProbQw___/*#3*/,		tatProbMask;/*atProbMask#2*/

/*e2*/	clgt		tRangeLt0x01000000/*RangeLt0x01000000#2*/,_0x01000000,tRange;/*Range#4*/
/*o4*/	rotqby		atBufferAtByt0_____/*#2*/,	tatBufferQw/*atBufferQw#2*/,Buffer;/*#2*/



/*e2*/	selb		Buffer/*#3*/,		Buffer/*#2*/,		tBufferIfT/*BufferIfT#2*/,tRangeLt0x01000000;/*RangeLt0x01000000#2*/

/*e2*/	selb		Range_____/*#5*/,		tRange/*Range#4*/,	tRangeIfT/*RangeIfT#2*/,	tRangeLt0x01000000;/*RangeLt0x01000000#2*/

	nop $55;
/*o4*/	shufb		CodeIfT_____/*#2*/,		Code/*#3*/,		atBufferAtByt0_____/*#2*/,	_0x01020310;

/*e4*/	rotmi		RangeShr11____/*#2*/,		Range_____/*#5*/,		-11;
/*o1*/	stqd		atProbQw____/*#4*/,		0(prob___/*#2*/);

/*e2*/	a		prob___/*#3*/,		tProbL__/*#3*/,		posStateMpy2;
/*o6*/	lqx		atProbQw_____/*#5*/,		tProbL__/*#3*/,		posStateMpy2;



/*e2*/	selb		Code/*#4*/,		Code/*#3*/,		CodeIfT_____/*#2*/,		tRangeLt0x01000000;/*RangeLt0x01000000#2*/
/*o4v*/	chd		atProbMask____/*#3*/,		0(prob___/*#3*/);

/*e2*/	ai		probMns2___/*#2*/,		prob___/*#3*/,		-2;
/*o15*/	hbrr		_beforeCount27,		_mainLoopDoBrzOnly;





/*o4*/	rotqby		atProb_/*hw1,#5*/,		atProbQw_____/*#5*/,		probMns2___;/*#2*/

/*e2*/	clgt		error______,			Buffer,			BufferLim;





/*e7*/	mpyu		boundMpyLo_____/*#2*/,		RangeShr11____/*#2*/,		atProb_;/*#5*/
/*o?*/	brnz		error______,			_error8;

/*e7*/	mpyh		boundMpyHi______/*#2*/,		RangeShr11____/*#2*/,		atProb_;/*#5*/
	lnop;

/*e2*/	sfh		_0x800MnsAtProb___/*#2*/,	atProb_/*#5*/,		_0x00000800;
/*o4*/	cbx		atOutStreamNowPosMask_,	nowPos,			outStream;



/*e4*/	rothmi		_0x800MnsAtProbShr5___/*#2*/,_0x800MnsAtProb___/*#2*/,	-5;







/*e2*/	a		bound_______/*#3*/,		boundMpyHi______/*#2*/,		boundMpyLo_____;/*#2*/



/*e2*/	clgt		CodeLtBound_______/*#2*/,	bound_______/*#3*/,		Code;/*#4*/



/*o?*/	brz		CodeLtBound_______/*#2*/,	_count27;
/*----------------------------------------------------------------*/

/*e2*/	ah		atProb_____/*#6*/,		atProb_/*#5*/,		_0x800MnsAtProbShr5___;/*#2*/
/*o4*/	rotqmbyi	previousByte,		tPreviousByte,		-3;

/*e2*/	ai		tnowPos/*nowPos#2*/,	nowPos,			1;
	lnop;

/*e2*/	ori		Range/*#6*/,		bound_______/*#3*/,		0;
/*o4*/	shufb		atProbQw______/*#6*/,		atProb_____/*#6*/,		atProbQw_____/*#5*/,		atProbMask____;/*#3*/

/*e2*/	clgt		nowPosLtOutSize,	outSize,		tnowPos;/*nowPos#2*/
	lnop;
	
/*e2*/	sfi		state/*#2*/,		tState,			9;
/*o4*/	shufb		atOutStreamNowPosQw,	previousByte,		atOutStreamNowPosQw,	atOutStreamNowPosMask_;

/*e2*/	ceqi		error_,			nowPos,			0;

/*o1*/	stqd		atProbQw______/*#6*/,		0(prob___/*#3*/);

	lnop;
/*o?*/	brnz		error_,			_error9;

	lnop;
/*o6*/	stqx		atOutStreamNowPosQw,	nowPos,			outStream;

/*e2*/	ai		nowPos/*#2*/,		nowPos,			1;
_beforeCount27:
/*o?*/	br		_mainLoopDoBrzOnly;

.align	5
_count27:
/*e4*/	rothmi		atProbShr5__,		atProb_,			-5;
/*o15*/	hbrr		_beforeCount28,		_count69;

/*e2*/	sf		Range____/*#2*/,		bound_______,			Range_____;
	lnop;

/*e2*/	sf		Code/*#2*/,		bound_______,			Code;
/*o4*/	chd		atProbMask_____,		0(prob___);



/*e2*/	sfh		atProb_/*#2*/,		atProbShr5__,		atProb_;



/*o4*/	shufb		atProbQw__/*#2*/,		atProb_/*#2*/,		atProbQw_____,		atProbMask_____;







/*o1*/	stqd		atProbQw__/*#2*/,		0(prob___);

/*DOWNCODE: if the following 2 nop's are not added, then the br to _count69 will have a full pipeline reload for some reason!*/
	nop $127;
	
	/*4 additional cycles*/
	
	nop $127;
_beforeCount28:
/*o?*/	br		_count69;
/*-------------------------------------------------------*/

.align	5
_count28:
/*e4*/	rothmi		atProbShr5___/*#2*/,		atProb____/*#2*/,		-5;
/*o6*/	lqd		atBufferQw_______,		0(Buffer);

/*e2*/	sf		Range_____/*#3*/,		bound______,			Range_____;/*#2*/
/*o4*/	chd		atProbMask______/*#2*/,		0(prob___);

/*e2*/	sf		Code/*#3*/,		bound______,			Code;/*#2*/
/*o15*/	hbrr		_beforeCount31,		_count34pt5;

/*e2*/	ai		tProb___,			p,			0xd8*2;

/*e2*/	sfh		atProb______/*#3*/,		atProbShr5___/*#2*/,	atProb____;/*#2*/

/*e2*/	a		stateMpy2__,		state,			state;
	lnop;

/*eo4*/	shli		RangeIfT__,		Range_____/*#3*/,		8;
/*o4*/	shufb		atProbQw_______/*#3*/,		atProb______/*#3*/,		atProbQw___/*#2*/,		atProbMask______;/*#2*/

/*e2*/	ai		BufferIfT____,		Buffer,			1;
/*o4*/	rotqby		atBufferAtByt0_,		atBufferQw_______,		Buffer;

/*e2*/	clgt		RangeLt0x01000000______,	_0x01000000,		Range_____;/*#3*/

/*1 cycle unintended dependency stall exposed by frontend.  Code is infrequent so it is left as is*/

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT____,		RangeLt0x01000000______;

/*e2*/	selb		Range_____/*#4*/,		Range_____/*#3*/,		RangeIfT__,		RangeLt0x01000000______;
/*o1*/	stqd		atProbQw_______/*#3*/,		0(prob___);

/*e2*/	a		prob__/*#2*/,		tProb___,			stateMpy2__;
/*o6*/	lqx		atProbQw____/*#4*/,		tProb___,			stateMpy2__;

/*e4*/	rotmi		RangeShr11_______,		Range_____/*#4*/,		-11;
/*o4*/	shufb		CodeIfT__,		Code/*#3*/,		atBufferAtByt0_,		_0x01020310;

/*e2*/	ai		probMns2____,		prob__/*#2*/,		-2;

/*e2*/	clgt		error_______,			Buffer,			BufferLim;



/*e2*/	selb		Code/*#4*/,		Code/*#3*/,		CodeIfT__,		RangeLt0x01000000______;
/*o?*/	brnz		error_______,			_error10;

/*o4*/	rotqby		atProb____/*hw1,#4*/,		atProbQw____/*#4*/,		probMns2____;







/*e7*/	mpyu		boundMpyLo______,		RangeShr11_______,		atProb____;/*#4*/

/*e7*/	mpyh		boundMpyHi_______,		RangeShr11_______,		atProb____;/*#4*/

/*e2*/	sfh		_0x800MnsAtProb____,	atProb____/*#4*/,		_0x00000800;



/*e4*/	rothmi		_0x800MnsAtProbShr5____,	_0x800MnsAtProb____,	-5;
	lnop;







/*e2*/	a		bound/*#2*/,		boundMpyHi_______,		boundMpyLo______;
/*o4*/	chd		atProbMask___/*#3*/,		0(prob__/*#2*/);



/*e2*/	clgt		CodeLtBound___,		bound/*#2*/,		Code;/*#4*/



/*o?*/	brz		CodeLtBound___,		_count31;
/*------------------------------------------------*/

/*e2*/	ah		atProb__/*#5*/,		atProb____/*#4*/,		_0x800MnsAtProbShr5____;



/*o4*/	shufb		atProbQw__/*#5*/,		atProb__/*#5*/,		atProbQw____/*#4*/,		atProbMask___;/*#3*/

/*e2*/	ori		Range____/*#5*/,		bound/*#2*/,		0;





/*o1*/	stqd		atProbQw__/*#5*/,		0(prob__/*#2*/);

/*e2*/	ori		distance,		rep1,			0;
_beforeCount31:	
/*o?*/	br		_count34pt5;
/*---------------------------------------------------*/

.align	5
_count31:
/*e4*/	rothmi		atProbShr5____,		atProb____,			-5;
/*o6*/	lqd		atBufferQw_,		0(Buffer);

/*e2*/	sf		Range_____/*#2*/, 		bound,			Range_____;
/*o15*/	hbrr		_beforeCount34,		_count34pt3;

/*e2*/	ai		BufferIfT_,		Buffer,			1;
/*o4*/	chd		atProbMask_______,		0(prob__);

/*eo4*/	shli		RangeIfT______,		Range_____/*#2*/,		8;

/*e2*/	sfh		atProb____/*#2*/,		atProbShr5____,		atProb____;

/*e2*/	sf		Code/*#2*/,		bound,			Code;
	lnop;

/*e2*/	ai		tProb____,			p,			0xe4*2;
/*o4*/	shufb		atProbQw____/*#2*/,		atProb____/*#2*/,		atProbQw____,		atProbMask_______;

/*e2*/	a		stateMpy2__,		state,			state;
/*o4*/	rotqby		atBufferAtByt0_,		atBufferQw_,		Buffer;

/*e2*/	clgt		RangeLt0x01000000___,	_0x01000000,		Range_____;/*#2*/

/*1 unintended delay noticed by frontend, but since code is infrequent it will be left this way*/

/*e2*/	selb		Range_____/*#3*/,		Range_____/*#2*/,		RangeIfT______,		RangeLt0x01000000___;

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT_,		RangeLt0x01000000___;
/*o1*/	stqd		atProbQw____/*#2*/,		0(prob__);

/*e2*/	a		prob____/*#2*/,		tProb____,			stateMpy2__;
/*o6*/	lqx		atProbQw______/*#3*/,		tProb____,			stateMpy2__;

/*e4*/	rotmi		RangeShr11_______,		Range_____/*#3*/,		-11;
/*o4*/	shufb		CodeIfT,		Code/*#2*/,		atBufferAtByt0_,		_0x01020310;

/*e2*/	ai		probMns2____,		prob____/*#2*/,		-2;

/*e2*/	clgt		error________,			Buffer,			BufferLim;



/*e2*/	selb		Code/*#3*/,		Code/*#2*/,		CodeIfT,		RangeLt0x01000000___;
/*o?*/	brnz		error________,			_error11;

/*o4*/	rotqby		atProb_______/*hw1,#3*/,		atProbQw______/*#3*/,		probMns2____;







/*e7*/	mpyu		boundMpyLo______,		RangeShr11_______,		atProb_______;/*#3*/

/*e7*/	mpyh		boundMpyHi_______,		RangeShr11_______,		atProb_______;/*#3*/

/*e2*/	sfh		_0x800MnsAtProb____,	atProb_______/*#3*/,		_0x00000800;



/*e4*/	rothmi		_0x800MnsAtProbShr5____,	_0x800MnsAtProb____,	-5;







/*e2*/	a		bound_/*#2*/,		boundMpyHi_______,		boundMpyLo______;



/*e2*/	clgt		CodeLtBound________,		bound_/*#2*/,		Code;/*#3*/
/*o4*/	chd		atProbMask______/*#2*/,		0(prob____/*#2*/);

	nop $34;

/*o?*/	brz		CodeLtBound________,		_count34;
/*--------------------------------------------------------*/

/*e2*/	ah		atProb__/*#4*/,		atProb_______/*#3*/,		_0x800MnsAtProbShr5____;

/*e2*/	ori		Range____/*#4*/,		bound_/*#2*/,		0;

/*o4*/	shufb		atProbQw__/*#4*/,		atProb__/*#4*/,		atProbQw______/*#3*/,		atProbMask______;/*#2*/







/*o1*/	stqd		atProbQw__/*#4*/,		0(prob____/*#2*/);

/*e2*/	ori		distance,		rep2,			0;
_beforeCount34:
/*o?*/	br		_count34pt3;
/*-------------------------------------------------------*/
        
.align	5
_count34:
/*e4*/	rothmi		atProbShr5_____/*#2*/,	atProb_______/*#4*/,		-5;

/*e2*/	sf		Range____/*#5*/,		bound_/*#2*/,		Range_____;/*#4*/

/*o4*/	chd		atProbMask_____/*#3*/,		0(prob____/*#2*/);

/*e2*/	sf		Code/*#4*/,		bound_/*#2*/,		Code;/*#3*/

/*e2*/	sfh		atProb_______/*#5*/,		atProbShr5_____/*#2*/, 	atProb_______;/*#4*/



/*o4*/	shufb		atProbQw______/*#5*/,		atProb_______/*#5*/,		atProbQw______/*#4*/,		atProbMask_____;/*#3*/

/*e2*/	ori		distance/*#2*/,		rep3,			0;

/*e2*/	ori		rep3/*#2*/,		rep2,			0;

	nop $60;
	
/*o1*/	stqd		atProbQw______/*#5*/,		0(prob____/*#2*/);
/*-----------------------------------------------------*/

/*.align	3*/
_count34pt3:
/*e2*/	ori		rep2/*#2*/,		rep1,			0;
	lnop;
/*-----------------------------------------------------*/

/*.align	3*/
_count34pt5:
/*e2*/	ori		rep1/*#2*/,		rep0,			0;

/*e2*/	ori		rep0/*#2*/,		distance/*#2*/,		0;
/*-----------------------------------------------------*/

/*.align	3*/
_count69:
/*e2*/	cgti		stateGe7___,		state,			7-1;

/*e2*/	il		tProb_____/*#2*/,		0x534*2;

/*e2*/	and		tState_,			stateGe7___,		_3;

/*e2*/	a		prob_/*#3*/,		p,			tProb_____;/*#2*/

/*e2*/	ai		state/*#2*/,		tState_,			8;
	lnop;

_count35:
/*e4*/	shli		RangeIfT_______,		Range____,			8;
/*o6*/	lqd		atProbLenQw,		0(prob_);

/*e2vv*/	ori		probLen,		prob_,			0;
/*o15*/	hbrr		_beforeCount38,		_count70;

/*e2*/	clgt		RangeLt0x01000000_______,	_0x01000000,		Range____;
	lnop;

/*e2*/	ai		probLenMns2,		prob_,			-2;
/*o6*/	lqd		atBufferQw_____,		0(Buffer);

/*e2*/	selb		Range/*#2*/,		Range____,			RangeIfT_______,		RangeLt0x01000000_______;
	lnop;



/*e4*/	rotmi		RangeShr11_______,		Range/*#2*/,		-11;
/*o4*/	rotqby		atProbLen/*hw1*/,		atProbLenQw,		probLenMns2;

/*e2*/	ai		BufferIfT___,		Buffer,			1;



/*o4*/	rotqby		atBufferAtByt0____,		atBufferQw_____,		Buffer;

/*e7*/	mpyu		boundMpyLo___,		RangeShr11_______,		atProbLen;

/*e7*/	mpyh		boundMpyHi_______,		RangeShr11_______,		atProbLen;

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT___,		RangeLt0x01000000_______;
	lnop;

/*e2*/	sfh		_0x800MnsAtProbLen,	atProbLen,		_0x00000800;
/*o4*/	shufb		CodeIfT______,		Code,			atBufferAtByt0____,		_0x01020310;

/*e2*/	clgt		error_________,			Buffer,			BufferLim;

/*e4*/	rothmi		_0x800MnsAtProbLenShr5,	_0x800MnsAtProbLen,	-5;



/*e2*/	selb		Code/*#2*/,		Code,			CodeIfT______,		RangeLt0x01000000_______;
/*o?*/	brnz		error_________,			_error12;

/*e2*/	a		bound,			boundMpyHi_______,		boundMpyLo___;
/*o4*/	chd		atProbLenMask,		0(probLen);



/*e2*/	clgt		CodeLtBound___,		bound,			Code;/*#2*/

/*e2*/	ah		atProbLen_/*atProbLen,#2*/,atProbLen,		_0x800MnsAtProbLenShr5;

	nop $40;
/*o?*/	brz		CodeLtBound___,		_count38;
/*-----------------------------------------------------------------------------------------*/

/*e4*/	roti		posStateShl4,		posState,		3+1;
/*o4*/	shufb		atProbLenQw/*#2*/,	atProbLen_/*atProbLen,#2*/,atProbLenQw,		atProbLenMask;

/*e2*/	ai		tProb_,			prob_,			2*2;
	lnop;

/*e2*/	ori		Range/*#3*/,		bound,			0;

/*e2*/	il		offset,			0;

/*e2*/	il		numBits,		3;
/*o1*/	stqd		atProbLenQw/*#2*/,	0(probLen);

/*e2*/	a		probLen_/*#2*/,		tProb_,			posStateShl4;
_beforeCount38:
/*o?*/	br		_count70;

.align	5
_count38:
/*e4*/	rothmi		atProbLenShr5,		atProbLen,		-5;
/*o15*/	hbrr		_beforeCount40,		_count41;

/*e2*/	sf		Range/*#2*/,		bound,			Range;
/*o6*/	lqd		atBufferQw_____,		0(Buffer);

/*e2*/	ai		BufferIfT___,		Buffer,			1;
/*o4*/	chd		atProbLenMask_,		0(probLen);

/*eo4*/	shli		RangeIfT________,		Range/*#2*/,		8;

/*e2*/	sf		atProbLen__/*#2*/,	 	atProbLenShr5,		atProbLen;

/*e2*/	clgt		RangeLt0x01000000___,	_0x01000000,		Range;/*#2*/
	lnop;

/*e2*/	sf		Code/*#2*/,		bound,			Code;
/*o4*/	shufb		atProbLenQw_/*#2*/,	atProbLen__/*#2*/,		atProbLenQw,		atProbLenMask_;

/*e2*/	selb		Range/*#3*/,		Range/*#2*/,		RangeIfT________,		RangeLt0x01000000___;
/*o4*/	rotqby		atBufferAtByt0__,		atBufferQw_____,		Buffer;

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT___,		RangeLt0x01000000___;

/*e4*/	rotmi		RangeShr11________,		Range/*#3*/,		-11;

	nop $71;
/*o1*/	stqd		atProbLenQw_/*#2*/,	0(probLen);

/*e2*/	ai		probLen/*#2*/,		prob_,			1*2;
/*o4*/	shufb		CodeIfT_____,		Code/*#2*/,		atBufferAtByt0__,		_0x01020310;



/*e2*/	ai		probLenMns2_,		probLen/*#2*/,		-2;
/*o6*/	lqd		atProbLenQw__/*#3*/,	0(probLen/*#2*/);



/*e2*/	selb		Code/*#3*/,		Code/*#2*/,		CodeIfT_____,		RangeLt0x01000000___;

/*e2*/	clgt		error__,			Buffer,			BufferLim;





/*o4*/	rotqby		atProbLen___/*hw1,#3*/,	atProbLenQw__/*#3*/,	probLenMns2_;

/*o?*/	brnz		error__,			_error13;





/*e7*/	mpyu		boundMpyLo,		RangeShr11________,		atProbLen___;/*#3*/

/*e7*/	mpyh		boundMpyHi________,		RangeShr11________,		atProbLen___;/*#3*/

/*e2*/	sfh		_0x800MnsAtProbLen_,	atProbLen___/*#3*/,		_0x00000800;



/*e4*/	rothmi		_0x800MnsAtProbLenShr5_,	_0x800MnsAtProbLen_,	-5;







/*e2*/	a		bound/*#2*/,		boundMpyHi________,		boundMpyLo;
/*o4*/	chd		atProbLenMask/*#2*/,	0(probLen/*#2*/);



/*e2*/	clgt		CodeLtBound_______,		bound/*#2*/,		Code;/*#3*/
/*o15*/	hbrr		_beforeCount41,		_count70;

	nop $41;

_beforeCount40:	
/*o?*/	brz		CodeLtBound_______,		_count41;
/*------------------------------------------------------*/

/*17 or 18 cycles to reload pipeline, but it gives time to register hint*/

/*e2*/	ah		atProbLen___/*#4*/,		atProbLen___/*#3*/,		_0x800MnsAtProbLenShr5_;

/*e2*/	ori		Range/*#4*/,		bound/*#2*/,		0;

/*o4*/	shufb		atProbLenQw/*#4*/,	atProbLen___/*#4*/,	atProbLenQw__/*#3*/,	atProbLenMask;/*#2*/

/*e4*/	shli		posStateShl4,		posState,		3+1;

/*e2*/	ai		tProbLen,		prob_,			0x82*2;

/*e2*/	il		offset,			8;

/*e2*/	il		numBits,		3;
/*o1*/	stqd		atProbLenQw/*#4*/,	0(probLen/*#2*/);

/*e2*/	a		probLen_/*#3*/,		tProbLen,		posStateShl4;
_beforeCount41:
/*o?*/	br		_count70;
/*------------------------------------------------------*/

.align	5
_count41:
/*e4*/	rothmi		atProbLenShr5_/*#2*/,	atProbLen___/*#4*/,		-5;

/*e2*/	il		_0x00000204,		0x102*2;

/*e2*/	sf		Range/*#5*/,		bound/*#2*/,		Range;/*#4*/
/*o4*/	chd		atProbLenMask__/*#3*/,	0(probLen/*#3*/);

/*e2*/	sf		Code/*#4*/,		bound/*#2*/,		Code;/*#3*/

/*e2*/	sf		atProbLen___/*#5*/,		atProbLenShr5_/*#2*/,	atProbLen___;/*#4*/

/*Note: Frontend reports 7 extra "H" cycles here for some unknown reason*/

/*e2*/	il		offset/*#2*/,		16;            
	lnop;

/*e2*/	il		numBits/*#2*/,		8;
/*o4*/	shufb		atProbLenQw/*#5*/,	atProbLen___/*#5*/,	atProbLenQw__/*#4*/,	atProbLenMask__;/*#3*/







/*o1*/	stqd		atProbLenQw/*#5*/,	0(probLen/*#3*/);

/*e2*/	a		probLen_/*#4*/,		prob_,			_0x00000204;

_count70:
/*e2*/	il		len,			1;

/*e2*/	ori		i,			numBits,		0;
/*-------------------------------------------------------*/

/*.align	3*/
_count42:
/*e2*/	a		lenMpy2,		len,			len;
/*o15*/	hbrr		_beforeCount45pt3,	_count42;

/*eo4*/	shli		RangeIfT___,		Range,			8;
	lnop;

/*e2*/	ceqi		dontLoop,		i,			1;
/*o6*/	lqx		atProb1Qw,		lenMpy2,		probLen_;

/*e2*/	clgt		RangeLt0x01000000_,	_0x01000000,		Range;

/*e2*/	a		prob1,			probLen_,		lenMpy2;

/*e2*/	selb		Range/*#2*/,		Range,			RangeIfT___,		RangeLt0x01000000_;
/*o6*/	lqd		atBufferQw_,		0(Buffer);

/*e2*/	ai		prob1Mns2,		prob1,			-2;

/*e2*/	ai		i/*#2*/,			i,			-1;

/*e4*/	rotmi		RangeShr11_________,		Range/*#2*/,		-11;
/*o4*/	rotqby		atProb1/*hw1*/,		atProb1Qw,		prob1Mns2;

/*e2*/	ceq		tError_,			Buffer,			BufferLim;
/*NOTE: this cycle was added when overflow checking was added*/

/*e2*/	cgti		stateNotLt4,		state,			4-1;

/*e4*/	shl		tLen,			_1,			numBits;

/*e2*/	and		error__________,			RangeLt0x01000000_,	tError_;

/*e2*/	sfh		_0x800MnsAtProb1,	atProb1,		_0x00000800;
/*o4*/	rotqby		atBufferAtByt0___,		atBufferQw_,		Buffer;

/*e7*/	mpyu		boundMpyLo_______,		RangeShr11_________,		atProb1;
/*o?*/	brnz		error__________,			_error14;

/*e7*/	mpyh		boundMpyHi_________,		RangeShr11_________,		atProb1;

/*e4*/	rothmi		_0x800MnsAtProb1Shr5,	_0x800MnsAtProb1,	-5;

/*e2*/	ai		BufferIfT______,		Buffer,			1;
/*o4*/	shufb		CodeIfT___,		Code,			atBufferAtByt0___,		_0x01020310;

/*e2*/	a		lenMpy2_/*#2*/,		len,			len;

/*e4*/	rothmi		atProb1Shr5,		atProb1,		-5;

/*e2*/	ah		atProb1IfT,		atProb1,		_0x800MnsAtProb1Shr5;

/*e2*/	selb		Code/*#2*/,		Code,			CodeIfT___,		RangeLt0x01000000_;

/*e2*/	a		bound_____,			boundMpyHi_________,		boundMpyLo_______;

/*e2*/	sfh		atProb1IfF,		atProb1Shr5,		atProb1;

/*e2*/	clgt		CodeLtBound_________,		bound_____,			Code;/*#2*/
/*o4*/	chd		atProb1Mask,		0(prob1);

/*e2*/	sf		RangeIfF,		bound_____,			Range;/*#2*/

/*e2*/	selb		atProb1/*#2*/,		atProb1IfF,		atProb1IfT,		CodeLtBound_________;

/*e2*/	ai		CodeLtBoundPls1,	CodeLtBound_________,		1;
	lnop;

/*e2*/	sf		CodeIfF_____,		bound_____,			Code;/*#2*/
/*o4*/	shufb		atProb1Qw/*#2*/,		atProb1/*#2*/,		atProb1Qw,		atProb1Mask;

/*e2*/	a		len/*#2*/,		lenMpy2_/*#2*/,		CodeLtBoundPls1;

/*e2*/	selb		Code/*#3*/,		CodeIfF_____,		Code/*#2*/,		CodeLtBound_________;

/*e2*/	selb		Range/*#3*/,		RangeIfF,		bound_____,			CodeLtBound_________;
	lnop;

/*e2*/	sf		tLen2,			tLen,			len;/*#2*/
/*o1*/	stqd		atProb1Qw/*#2*/,		0(prob1);

/*NOTE: if the following nop-lnop was not added, and if the code fell through*/
/* the brz below, then it would have an entire 18 cycle load in the middle of the*/
/* code (since the brz was 8 bytes before the end of 0x80 bytes).*/
/* So the nop-lnop made that disappear, at the cost of 1 cycle in the most frequent case*/
	nop $127;
	lnop;

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT______,		RangeLt0x01000000_;
_beforeCount45pt3:	
/*o?*/	brz		dontLoop,		_count42;
/*--------------------------------------------------------------------------------*/

/*17 or 18 cycles to reload pipeline*/

/*e2*/	a		len_/*#3*/,		tLen2,			offset;
/*o15*/	brnz		stateNotLt4,		_count67;
/*--------------------------------------------------------------------------------*/
	
/*e2*/	ai		state/*#2*/,		state,			7;

/*e2*/	cgti		lenGe4,			len_/*#3*/,		4-1;

/*e2*/	il		i_/*#3*/,			6;

/*e2*/	selb		tProbR_,			len_/*#3*/,		_3,			lenGe4;

/*e2*/	il		posSlot,		1;

/*e4*/	shli		tProbR_/*#2*/,		tProbR_,			6+1;

/*e2*/	a		tProbL___,			p,			_0x00000360;/*0x1b0*2*/





/*e2*/	a		prob_____,			tProbL___,			tProbR_;/*#2*/

_count48: /* do loop top*/
/*e2*/	a		posSlotMpy2,		posSlot,		posSlot;
/*o6*/	lqd		atBufferQw_______/*#2*/,		0(Buffer/*#2*/);

/*e2*/	ceqi		dontLoop_/*#2*/,		i_/*#3*/,			1;
/*o15*/	hbrr		_beforeCount52,		_count48;

/*eo4*/	shli		RangeIfT____/*#2*/,		Range/*#3*/,		8;
/*o6*/	lqx		atProb2Qw,		posSlotMpy2,		prob_____;

/*e2*/	a		prob2,			prob_____,			posSlotMpy2;

/*e2*/	clgt		RangeLt0x01000000_/*#2*/,	_0x01000000,		Range;/*#3*/

/*e2*/	ai		prob2Mns2,		prob2,			-2;
	lnop;

/*e2*/	selb		Range/*#4*/,		Range/*#3*/,		RangeIfT____/*#2*/,		RangeLt0x01000000_;/*#2*/
/*o4*/	rotqby		atBufferAtByt0_/*#2*/,	atBufferQw_______/*#2*/,		Buffer;/*#2*/

/*e2*/	ai		i_/*#4*/,			i_/*#3*/,			-1;
	lnop;

/*e4*/	rotmi		RangeShr11___/*#2*/,		Range/*#4*/,		-11;
/*o4*/	rotqby		atProb2/*hw1*/,		atProb2Qw,		prob2Mns2;

/*e2*/	ceq		tError__,			Buffer,			BufferLim;

/*o4*/	shufb		CodeIfT_/*#2*/,		Code/*#3*/,		atBufferAtByt0_/*#2*/,	_0x01020310;

/*e2*/	and		error___________,			RangeLt0x01000000_,	tError__;

/*e7*/	mpyu		boundMpyLo________/*#2*/,		RangeShr11___/*#2*/,		atProb2;

/*e7*/	mpyh		boundMpyHi____/*#2*/,		RangeShr11___/*#2*/,		atProb2;
/*o?*/	brnz		error___________,			_error15;

/*e2*/	sfh		_0x800MnsAtProb2,	atProb2,		_0x00000800;

/*e4*/	rothmi		atProb2Shr5,		atProb2,		-5;

/*e2*/	selb		Code/*#4*/,		Code/*#3*/,		CodeIfT_/*#2*/,		RangeLt0x01000000_;/*#2*/

/*e4*/	rothmi		_0x800MnsAtProb2Shr5,	_0x800MnsAtProb2,	-5;

/*e2*/	a		posSlotMpy2_/*#2*/,	posSlot,		posSlot;

/*e2*/	sfh		atProb2IfF,		atProb2Shr5,		atProb2;

/*e2*/	a		bound________/*#2*/,		boundMpyHi____/*#2*/,		boundMpyLo________;/*#2*/

/*e2*/	ah		atProb2IfT,	       	atProb2,		_0x800MnsAtProb2Shr5;

/*e2*/	clgt		CodeLtBound__/*#2*/,	bound________/*#2*/,		Code;/*#4*/
/*o4*/	chd		atProb2Mask,		0(prob2);

/*e2*/	ai		BufferIfT_______/*#2*/,		Buffer/*#2*/, 		1;

/*e2*/	selb		atProb2_/*#2*/,		atProb2IfF,		atProb2IfT,		CodeLtBound__;/*#2*/

/*e2*/	sf		RangeIfF/*#2*/,		bound________/*#2*/,		Range;/*#4*/
	lnop;

/*e2*/	selb		Buffer/*#3*/,		Buffer/*#2*/,		BufferIfT_______/*#2*/,		RangeLt0x01000000_;/*#2*/
/*o4*/	shufb		atProb2Qw_/*#2*/,		atProb2_/*#2*/,		atProb2Qw,		atProb2Mask;

/*e2*/	selb		Range/*#5*/,		RangeIfF/*#2*/,		bound________/*#2*/,		CodeLtBound__;/*#2*/
	lnop;

/*e2*/	ai		CodeLtBoundPls1_/*#2*/,	CodeLtBound__/*#2*/,	1;
/*o15*/	hbrr		_beforeCount54,		_count55;

/*e2*/	sf		CodeIfF______/*#2*/,		bound________/*#2*/,		Code;/*#4*/
	lnop;

/*e2*/	a		posSlot/*#2*/,		posSlotMpy2_/*#2*/,	CodeLtBoundPls1_;/*#2*/
/*o1*/	stqd		atProb2Qw_/*#2*/,		0(prob2);

/*e2*/	selb		Code/*#5*/,		CodeIfF______/*#2*/,		Code/*#4*/,		CodeLtBound__;/*#2*/
_beforeCount52:
/*o15*/	brz		dontLoop_/*#2*/,		_count48;
/*----------------------------------------------*/

/*17 or 18 cycles to reload pipeline, which also gives enough time for hint*/

/*e2*/	cgti		posSlotGe4,		posSlot/*#2*/,		(4-1)+64;/*add 64 since test was originally on posSlot#3*/

/*e2*/	ai		posSlot_/*#3*/,		posSlot/*#2*/,		-64;

	nop $34;
/*o15*/	brz		posSlotGe4,		_count64;
/*----------------------------------------------*/

/*e4*/	rotmai		tNumDirectBits,		posSlot_/*#3*/,		-1;
	lnop;
	
/*e2*/	andi		tRep0,			posSlot_/*#3*/,		1;
/*o15*/	hbrr		_beforeCount55,		_count59;
/*NOTE: above hbrr moved here to give time to register*/

/*e2*/	cgti		posSlotGe14,		posSlot_/*#3*/,		14-1;
	
/*e2*/	ori		rep0_,			tRep0,			2;          
	
/*e2*/	ai		numDirectBits,		tNumDirectBits,		-1;
_beforeCount54:
/*o?*/	brnz		posSlotGe14,		_count55;

/*17 or 18 cycles to reload pipeline, but gives time for hint to register*/

shl		rep0/*#2*/,		rep0_,			numDirectBits;
/*{o15}	hbrr		_beforeCount55,		_count59*/
/*NOTE: above hbrr moved to caller to give time to register*/
	lnop;
	
/*e2*/	il		tProbL____,			(0x2b0-1)*2;



/*e2*/	a		tProbL____/*#2*/,		p,			tProbL____;

/*e2*/	sf		tProbR__,			posSlot_,		rep0;/*#2*/



/*e2*/	a		tProbR__/*#2*/,		tProbR__,			tProbR__;



/*e2*/	a		prob______,			tProbL____/*#2*/,		tProbR__;/*#2*/
_beforeCount55:
/*o?*/	br		_count59;

.align	5
_count55:


/*e2*/	ai		numDirectBits_/*#2*/,	numDirectBits,		-4;            
	lnop;
/*--------------------------------------*/

/*.align	3*/
_count56:
	nop $19;
/*o6*/	lqd		atBufferQw________,		0(Buffer);

/*e2*/	ceqi		dontLoop__,		numDirectBits_/*#2*/,	1;
/*o15*/	hbrr		_beforeCount59,		_count56;

/*e2*/	ai		numDirectBits_/*#3*/,	numDirectBits_/*#2*/,	-1;
	lnop;

/*eo4*/	shli		RangeIfT___,		Range,			8;





/*o4*/	rotqby		atBufferAtByt0______,		atBufferQw________,		Buffer;

/*e2*/	clgt		RangeLt0x01000000______,	_0x01000000,		Range;

/*e2*/	ai		BufferIfT___,		Buffer,			1;

/*e2*/	selb		Range/*#2*/,		Range,			RangeIfT___,		RangeLt0x01000000______;

/*o4*/	shufb		CodeIfT____,		Code,			atBufferAtByt0______,	_0x01020310;

/*e2*/	selb		Buffer/*#2*/,		Buffer,			BufferIfT___,		RangeLt0x01000000______;
/*o4*/	rotqmbii	Range/*#3*/,		Range/*#2*/,		-1;

/*e2*/	a		rep0_/*#2*/,		rep0_,			rep0_;

/*e2*/	clgt		error_______,			Buffer,			BufferLim;

/*e2*/	selb		Code/*#2*/,		Code,			CodeIfT____,		RangeLt0x01000000______;

/*e2*/	ori		rep0IfT,		rep0_/*#2*/,		1;

/*e2*/	clgt		CodeNotGeRange,		Range/*#3*/,		Code;/*#2*/
/*o?*/	brnz		error_______,			_error16;

/*e2*/	sf		CodeIfT___/*#2*/,		Range/*#3*/, 		Code;/*#2*/

/*e2*/	selb		rep0_/*#3*/,		rep0IfT,		rep0_/*#2*/,		CodeNotGeRange;

/*e2*/	selb		Code/*#3*/,		CodeIfT___/*#2*/,		Code/*#2*/,		CodeNotGeRange;
_beforeCount59:
/*o?*/	brz		dontLoop__,		_count56;
/*------------------------------------------*/

/*17 or 18 cycles to reload pipeline*/

/*e2*/	a		prob______,			p,			_0x00000644;/*0x322*2*/

/*e4*/	shli		rep0/*#4*/,		rep0_/*#3*/,		4;

/*e2*/	il		numDirectBits/*#4*/,	4;
	lnop;
/*-------------------------------------------*/

/*.align	3*/
_count59:
/*e2*/	il		mi,			1;
/*oe4*/	shlqbyi/*copy*/	i__,			_1,			0;
/*------------------------------------------*/

/*.align	3*/
_count60:
/*eo4*/	shli		RangeIfT____/*#2*/,		Range/*#3*/,		8;
	lnop;

/*e2*/	a		miMpy2,			mi,			mi;
/*o15*/	hbrr		_beforeCount64,		_count60;

/*e2*/	clgt		RangeLt0x01000000________/*#2*/,	_0x01000000,		Range;/*#3*/
	lnop;
	
/*e2*/	ceq		tError___,			Buffer,			BufferLim;
/*o6*/	lqx		atProb3Qw,		miMpy2,			prob______;

/*e2*/	selb		Range/*#4*/,		Range/*#3*/,		RangeIfT____/*#2*/,		RangeLt0x01000000________;/*#2*/
	lnop;
	
/*e2*/	a		prob3,			prob______,			miMpy2;
/*o6*/	lqd		atBufferQw_________/*#2*/,		0(Buffer/*#2*/);

/*e2*/	ai		miMpy2Plus1,		miMpy2,			1;

/*e2*/	ai		prob3Mns2,		prob3,			-2;

/*e4*/	rotmi		RangeShr11__________,		Range/*#4*/,		-11;
	lnop;
	
/*e2*/	and		error____,			RangeLt0x01000000________,	tError___;
/*o4*/	rotqby		atProb3/*hw1*/,		atProb3Qw,		prob3Mns2;

/*e2*/	ai		BufferIfT________/*#2*/,		Buffer/*#2*/,		1;
	lnop;
	
/*e2*/	ceqi		dontLoop___/*#2*/,		numDirectBits/*#4*/,	1;
/*o4*/	rotqby		atBufferAtByt0_______/*#2*/,	atBufferQw_________/*#2*/,		Buffer;/*#2*/

/*e2*/	ai		numDirectBits/*#5*/,	numDirectBits/*#4*/,	-1;

/*e7*/	mpyu		boundMpyLo_______,		RangeShr11__________,		atProb3;

/*e7*/	mpyh		boundMpyHi__________,		RangeShr11__________,		atProb3;
/*o?*/	brnz		error____,			_error17;

/*e2*/	sfh		_0x800MnsAtProb3,	atProb3,		_0x00000800;
/*o4*/	shufb		CodeIfT_______/*#3*/,		Code/*#3*/,		atBufferAtByt0_______/*#2*/,	_0x01020310;

/*e4*/	rothmi		atProb3Shr5,		atProb3,		-5;

/*e4*/	rothmi		_0x800MnsAtProb3Shr5,	_0x800MnsAtProb3,	-5;

/*e2*/	or		rep0IfF,		rep0/*#4*/,		i__;

/*e2*/	selb		Code/*#4*/,		Code/*#3*/,		CodeIfT_______/*#3*/,		RangeLt0x01000000________;/*#2*/

/*e2*/	sfh		atProb3IfF,		atProb3Shr5,		atProb3;

/*e2*/	a		bound_________,			boundMpyHi__________,		boundMpyLo_______;

/*e2*/	ah		atProb3IfT,		atProb3,		_0x800MnsAtProb3Shr5;
	lnop;

/*e2*/	clgt		CodeLtBound_____,		bound_________,			Code;/*#4*/
/*o4*/	chd		atProb3Mask,		0(prob3);

/*e2*/	sf		RangeIfF,		bound_________,			Range;/*#4*/

/*e2*/	selb		atProb3_/*#2*/,		atProb3IfF,		atProb3IfT,		CodeLtBound_____;

/*e2*/	sf		CodeIfF_______,		bound_________,			Code;/*#4*/
	lnop;

/*e2*/	a		mi/*#2*/,			miMpy2Plus1,		CodeLtBound_____;
/*o4*/	shufb		atProb3Qw_/*#2*/,		atProb3_/*#2*/,		atProb3Qw,		atProb3Mask;

/*e2*/	selb		rep0/*#5*/,		rep0IfF,		rep0/*#4*/,		CodeLtBound_____;

/*e2*/	selb		Range/*#5*/,		RangeIfF,		bound_________,			CodeLtBound_____;

/*e2*/	selb		Code/*#5*/, 		CodeIfF_______,		Code/*#4*/,		CodeLtBound_____;
	lnop;

/*e2*/	a		i__/*#2*/,			i__,			i__;
/*o1*/	stqd		atProb3Qw_/*#2*/,		0(prob3);

/*e2*/	selb		Buffer/*#3*/,		Buffer/*#2*/,		BufferIfT________/*#2*/,		RangeLt0x01000000________;/*#2*/
_beforeCount64:
/*o15*/	brz		dontLoop___/*#2*/,		_count60;
/*----------------------------------------*/

/*17 or 18 cycles to reload pipeline*/

/*.align	3*/
_count64:          
/*e2*/	selb		rep0/*#6*/,		posSlot_,		rep0/*#5*/,		posSlotGe4;/*set far above at count52*/



/*e2*/	ceqi		rep0Eq0,		rep0/*#6*/,		0-1;
								
/*e2*/	ai		rep0/*#7*/,		rep0/*#6*/,		1;
	lnop;

/*e2*/	selb		len_/*#2*/,		len_,			rep0Eq0,		rep0Eq0;	
/*o15*/	brnz		rep0Eq0,		_mainLoopExit;
/*-----------------------------------------*/

/*.align	3*/
_count67:
/*e2*/	clgt		error__,			rep0,			nowPos;

/*e2*/	ai		len_/*#3*/,		len_/*#2*/,		2;

	nop $34;
/*o?*/	brnz		error__,			_error18;

andi		outStreamBackup,	outStream,		0xF;

/*e2*/	andi		outStream16,		outStream,		-16;

/*e2*/	a		nowPos16,		nowPos,			outStreamBackup;

/*e2*/	sf		maxLen,			nowPos,			outSize;

/*e2*/	sf		dupIndex,		rep0/*1:31*/,		nowPos16;

/*e2*/	andi		numBytsSet,		nowPos16,		0xF;

/*e2*/	a		dupAdrs,		dupIndex,		outStream16;

/*e2*/	sfi		numBytsNotSet,		numBytsSet,		16;

/*e2*/	clgt		lenTooBig,		len_,			maxLen;
/*o6*/	lqd		dup0Qw,			0(dupAdrs);

/*e2*/	sfi		negNumBytsNotSet,	numBytsNotSet,		0;
/*o6*/	lqd		dup1Qw,			0x10(dupAdrs);

/*e2*/	andi		numBytsBeforeDup0,	dupIndex,		0xF;
	lnop;
	
/*e2*/	selb		numBytsToDup,		len_,			maxLen,			lenTooBig;
/*o4*/	rotqmby		atOutStreamNowPosQwShr,	atOutStreamNowPosQw,	negNumBytsNotSet;

/*e2*/	ai		negNumDup0Byts,		numBytsBeforeDup0,	-16;

/*e2*/	a		nowPos/*done*/,		nowPos,			numBytsToDup;
  
/*e2*/	clgti		rep0Ge32,		rep0,			32-1;
/*o4*/	shlqby		dup0QwShl,		dup0Qw,			numBytsBeforeDup0;

	nop $55;
/*o4*/ 	shlqby		atOutStreamNowPosQw0s,	atOutStreamNowPosQwShr,	numBytsNotSet;

/*e2*/	sfi		negNumBytsSet,		numBytsSet,		0;
/*o?*/	brz		rep0Ge32,		_rep0Lt32;
/*----------------------------------------------------------*/

/*.align	3*/
/*_rep0Ge32:*/
/*e2*/	a		negNumDup0Byts_,	negNumDup0Byts,		numBytsNotSet;
	lnop;

/*e2*/	a		neg_bytsSetPlsDup0Byts,	negNumBytsSet,		negNumDup0Byts;
/*o4*/	rotqmby		dup0QwShlShr,		dup0QwShl,		negNumBytsSet;

/*e2*/	ilh		_0x1010Etc_,		0x1010;
	lnop;
	
/*e2*/	sfi		numDup0Byts_,		negNumDup0Byts_,	0;
/*o4*/	rotqmby		dup1QwShr,		dup1Qw,			neg_bytsSetPlsDup0Byts;

/*e2*/	or		_0x101112Etc,		_0x000102Etc_,		_0x1010Etc_;

/*e2*/	or		tAtOutStreamNowPosQw,	atOutStreamNowPosQw0s,	dup0QwShlShr;

/*e2*/	sf		numBytsToDup,		numBytsNotSet,		numBytsToDup;

/*e2*/	or		atOutStreamNowPosQw,	tAtOutStreamNowPosQw,	dup1QwShr;

/*e2*/	cgti		moreBytsToDup,		numBytsToDup,		0;
	lnop;
	
/*e2*/	cgti 		dup0BytsExist,		numDup0Byts_,		0;
/*o1*/	stqx/*stqx0*/	atOutStreamNowPosQw,	nowPos16,		outStream16;

	nop $37;
/*o?*/	brz		moreBytsToDup,		_dupDone;
/*------------------------------------------------------------*/

/*o4*/	fsm		dup0BytsExistQw,	dup0BytsExist;

/*e2*/	a	       	nowPos16,		nowPos16,		numBytsNotSet;

/*e2*/	ai		numDup0Byts_pls16, 	numDup0Byts_,		16;
/*o6*/	lqd/*after stqx0*/dup2Qw,			0x20(dupAdrs);



/*e2*/	selb		numDup0Byts__,		numDup0Byts_pls16,	numDup0Byts_,		dup0BytsExistQw;
	lnop;
	


/*e2*/	sfi		numBytsBeforeDup0__,	numDup0Byts__,		16;
	lnop;

/*e2*/	sfi		negNumDup0Byts__,	numDup0Byts__,		0;
/*o15*/	hbrr		_farDupBranch,		_farDupLoop;

/*e2*/	selb		dup0,			dup1Qw,			dup0Qw,			dup0BytsExistQw;
/*o4*/	shlqby		_0x000102EtcShl,	_0x000102Etc_,		numBytsBeforeDup0__;

/*e2*/	selb		dup1,			dup2Qw,			dup1Qw,			dup0BytsExistQw;
/*o4*/	rotqmby		_0x101112EtcShr,	_0x101112Etc,		negNumDup0Byts__;

/*e2*/	ai		dupAdrsPls16,		dupAdrs,		16;



/*e2*/	selb		dupAdrs_,		dupAdrsPls16,		dupAdrs,		dup0BytsExistQw;

/*e2*/	or		farDupMask,		_0x000102EtcShl,	_0x101112EtcShr;
	lnop;

/*.align	3*/
_farDupLoop:
/*==============================================================================================================*/
	nop $119;
/*--------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------*/
/*o4*/	shufb		atOutStreamNowPosQw,	dup0,			dup1,		farDupMask;
/*--------------------------------------------------------------------------------------------------------------*/
/*e2*/	ori		dup0,			dup1,			0;
/*--------------------------------------------------------------------------------------------------------------*/
/*e2*/	ai		numBytsToDup,		numBytsToDup,		-16;
/*--------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------*/
/*e2*/	cgti		moreBytsToDup_,		numBytsToDup,		0;
/*o1*/	stqx/*stqx1*/	atOutStreamNowPosQw,	nowPos16,		outStream16;
/*--------------------------------------------------------------------------------------------------------------*/
/*e2*/	ai		nowPos16,		nowPos16,		16;
/*o6*/	lqd/*after stqx1*/dup1,			0x20(dupAdrs_);
/*--------------------------------------------------------------------------------------------------------------*/
/*e2*/	ai		dupAdrs_,		dupAdrs_,		16;
_farDupBranch:
/*o?*/	brnz		moreBytsToDup_,		_farDupLoop;
/*==============================================================================================================*/

/*17 or 18 cycles to reload pipeline*/

/* the code below is a hinted copy of _dupDone in order to save pipeline reload cycles*/
/*_dupDone:*/
/*e2*/	a		tNumBytsSet,		nowPos/*done*/,		outStreamBackup;
/*o15*/	hbrr		_farDupDoneBranch,	_mainLoopDoBrzOnly;



/*e2*/	andi		numBytsSet_,		tNumBytsSet,		0xF;



/*e2*/	ai		numBytsSetMns4,		numBytsSet_,		-4;

/*e2*/	clgt		nowPosLtOutSize,	outSize,		nowPos; 		/* used by _mainLoopDoBrzOnly*/

/*o4*/	rotqby		previousByteInByt3,	atOutStreamNowPosQw,	numBytsSetMns4;







/*e2*/	andi		previousByte,		previousByteInByt3,	0xFF;
	lnop;
	








/*NOTE: without the extra nop's, Frontend says it adds 17 cycles after the branch below*/
	nop $127;
	lnop;

	nop $127;
_farDupDoneBranch:
	br		_mainLoopDoBrzOnly;

/*--------------------------------------------------------*/

.align	5
_rep0Lt32:

/*17 or 18 cycles to reload pipeline*/

/*e2*/	ai		rep0Mns1/*0:30*/,		rep0,			-1;
/*o15*/	hbrr		_dupBranch,		_dupLoop;

/*e2*/	ila		tGRepTbl,		_gRepTbl;
/*o6*/	lqd		dup2Qw_,			0x20(dupAdrs);

/*e4*/	shli		dup1MaskIndex,		rep0Mns1,		4;

/*e2*/	a		gRepTbl,		picOffset,		tGRepTbl;

/*e2*/	il		_0xf0,			(16-1)<<4;
	lnop;

/*e2*/	ai		gDup1MaskTbl,		gRepTbl,		_gDup1MaskTbl-_gRepTbl;
/*o6*/	lqx		repQw,			rep0Mns1,		gRepTbl;

/*e2*/	clgt		dup1MaskIndexBig,	dup1MaskIndex,		_0xf0;
/*o4*/	shlqby		dup1QwShl,		dup1Qw,			numBytsBeforeDup0;

/*e2*/	ai		gDup0MaskTbl,		gRepTbl,		_gDup0MaskTbl-_gRepTbl;
/*o41*/	rotqmby		dup2QwShr,		dup2Qw_,			negNumDup0Byts;

/*e2*/	selb		dup0MaskIndex,		dup1MaskIndex,		_0xf0,			dup1MaskIndexBig;
/*o4*/	rotqmby		dup1QwShr,		dup1Qw,			negNumDup0Byts;

/*e2*/	sf		numBytsToDup_,		numBytsNotSet,		numBytsToDup;/*1:maxLen*/
/*o6*/	lqx		dup1Mask,		dup1MaskIndex,		gDup1MaskTbl;

	nop $91;
/*o6*/	lqx		dup0Mask,		dup0MaskIndex,		gDup0MaskTbl;

/*e2*/	ai		numBytsToDup__,		numBytsToDup_,		-16;
/*o4*/	rotqby		repInByt0,		repQw,			rep0Mns1;

/*e2*/	or		dup0Partial,		dup0QwShl,		dup1QwShr;

/*e2*/	or		dup1Partial,		dup1QwShl,		dup2QwShr;

/*e2*/	cgti		moreBytsToDup__,		numBytsToDup__,		0;
	lnop;

/*eo4*/	rotmi		rep/*16:31*/,		repInByt0,		-24;
/*o4*/	shufb		dup1_,			dup0Partial,		dup1Partial,		dup1Mask;

/*o4*/	shufb		dup0_,			dup0Partial,		dup1Partial,		dup0Mask;





/*o4*/	rotqmby		dup1Shr,		dup1_,			negNumBytsSet;

/*e2*/	sfi		negNumDup1BytsSet,	rep,			16;
/*o4*/	rotqmby		dup0Shr,		dup0_,			negNumBytsSet;

	nop $77;
/*o4*/	rotqmby		tDup0Shr_,		dup0_,			negNumBytsSet;

/*e2*/	a		neg_bytsSetPlsDup1Set,	negNumBytsSet,		negNumDup1BytsSet;
/*o4*/	shlqby		dup0Shl,		dup0_,			numBytsNotSet;

/*e2*/	sfi		numDup1BytsNotSet,	rep,			32;
/*o4v*/	shlqby		dup1Shl,		dup1_,			numBytsNotSet;

/*e2*/	or		atOutStreamNowPosQw,	atOutStreamNowPosQw0s,	dup0Shr;
/*o4*/	rotqmby		dup0Shr__,		dup0_,			neg_bytsSetPlsDup1Set;

/*e2*/	cgti		moreBytsToDup___,		numBytsToDup_,		0;
/*o4*/	shlqby		dup0Shr_,		tDup0Shr_,		numDup1BytsNotSet;

/*e2*/	or		tDup0_,			dup0Shl,		dup1Shr;
/*o1*/	stqx		atOutStreamNowPosQw,	nowPos16,		outStream16;

/*e2*/	a		nowPos16,		nowPos16,		numBytsNotSet;
/*o?*/	brz		moreBytsToDup___,		_dupDone;
/*-------------------------------------------------------------------*/

/*e2*/	or		atOutStreamNowPosQw/*dup0_*/,tDup0_,		dup0Shr__;

/*e2*/	or		dup1__,			dup1Shl,		dup0Shr_;

	nop $86;
/*o1*/	stqx		atOutStreamNowPosQw,	nowPos16,		outStream16;

/*e2*/	ai		nowPos16,		nowPos16,		16;
/*o?*/	brz		moreBytsToDup__,		_dupDone;
/*--------------------------------------------------------------*/

	nop $57;
/*o4*/	rotqmby		dup0_shr,		atOutStreamNowPosQw/*dup0_*/,negNumDup1BytsSet;





/*.align	3*/
_dupLoop:
/*==============================================================================================================*/
  /*e2*/	ai		numBytsToDup___,		numBytsToDup__,		-16;					
  /*o4*/	shlqby		dup1___,			atOutStreamNowPosQw/*dup0_*/,numDup1BytsNotSet;
/*--------------------------------------------------------------------------------------------------------------*/
  /*e2*/	or		atOutStreamNowPosQw/*dup0__*/,dup1__,		dup0_shr;
	lnop;
/*--------------------------------------------------------------------------------------------------------------*/
  /*e2*/	cgti		moreBytsToDup___,		numBytsToDup___,		0;
/*--------------------------------------------------------------------------------------------------------------*/
  /*o4*/	rotqmby		dup0__shr,		atOutStreamNowPosQw/*dup0__*/,negNumDup1BytsSet;
/*--------------------------------------------------------------------------------------------------------------*/
	nop $86;
  /*o1*/	stqx		atOutStreamNowPosQw,	nowPos16,		outStream16;
/*--------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ai		nowPos16,		nowPos16,		16;
  /*o?*/	brz		moreBytsToDup___,		_dupDone;
/*--------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ai		numBytsToDup__,		numBytsToDup___,		-16;
  /*o4*/	shlqby		dup1__,			atOutStreamNowPosQw/*dup0__*/,numDup1BytsNotSet;
/*--------------------------------------------------------------------------------------------------------------*/
  /*e2*/	or		atOutStreamNowPosQw/*dup0_*/,dup1___,		dup0__shr;
	lnop;
/*--------------------------------------------------------------------------------------------------------------*/
  /*e2*/	cgti		moreBytsToDup_,		numBytsToDup__,		0;
/*--------------------------------------------------------------------------------------------------------------*/
/*o4*/	rotqmby		dup0_shr,		atOutStreamNowPosQw/*dup0_*/,negNumDup1BytsSet;
/*--------------------------------------------------------------------------------------------------------------*/
	nop $86;
  /*o1*/	stqx		atOutStreamNowPosQw,	nowPos16,		outStream16;
/*--------------------------------------------------------------------------------------------------------------*/
  /*e2*/	ai		nowPos16,		nowPos16,		16;
_dupBranch:
  /*o?*/	brnz		moreBytsToDup_,		_dupLoop;
/*==============================================================================================================*/

/*17 or 18 cycles to reload pipeline*/

/* the code below is a hinted copy of _dupDone in order to save pipeline reload cycles*/
/*_dupDone:*/
/*e2*/	a		tNumBytsSet,		nowPos/*done*/,		outStreamBackup;
/*o15*/	hbrr		_dupDoneBranch,		_mainLoopDoBrzOnly;



/*e2*/	andi		numBytsSet_,		tNumBytsSet,		0xF;



/*e2*/	ai		numBytsSetMns4,		numBytsSet_,		-4;

/*e2*/	clgt		nowPosLtOutSize,	outSize,		nowPos; 		/* used by _mainLoopDoBrzOnly*/

/*o4*/	rotqby		previousByteInByt3,	atOutStreamNowPosQw,	numBytsSetMns4;







/*e2*/	andi		previousByte,		previousByteInByt3,	0xFF;
	lnop;
	








/*NOTE: without the extra nop's, Frontend says it adds 17 cycles after the branch below*/
	nop $127;
	lnop;

	nop $127;
_dupDoneBranch:
	br		_mainLoopDoBrzOnly;

.align	5

_mainLoopExit:
/*e2*/	ila		gInSizeProcessed_,	_gInSizeProcessed;
/*o15*/	hbr		_returnToCaller,	$lr;

/*e2*/	clgt		RangeLt0x01000000_________,	_0x01000000,		Range;
	lnop;
	
/*e2*/	ila		gOutSizeProcessed_,	_gOutSizeProcessed;
/*o6*/	lqx		inSizeProcessed,	picOffset,		gInSizeProcessed_;

/*e2*/	sf		Buffer_,			RangeLt0x01000000_________,	Buffer;

/*o6*/	lqx		outSizeProcessed,	picOffset,		gOutSizeProcessed_;

/*e2*/	sf		atInSizeProcessed,	inStream,		Buffer_;

/*e2*/	clgt		error____________,			Buffer_,			BufferLim;



/*o6*/	lqd		atInSizeProcessedQw,	0(inSizeProcessed);

/*o?*/	brnz		error____________,			_error19;
/*---------------------------------------------*/

/*o4*/	cwd		atInSizeProcessedMask,	0(inSizeProcessed);

/*o6*/	lqd		atOutSizeProcessedQw,	0(outSizeProcessed);

/*o4*/	cwd		atOutSizeProcessedMask,	0(outSizeProcessed);



/*o4*/	shufb		atInSizeProcessedQw_,	atInSizeProcessed,	atInSizeProcessedQw,	atInSizeProcessedMask;





/*o4*/	shufb		atOutSizeProcessedQw_,	nowPos,			atOutSizeProcessedQw,	atOutSizeProcessedMask;

/*o1*/	stqd		atInSizeProcessedQw_,	0(inSizeProcessed);

/*o1*/	il		$3,			0;/*LZMA_RESULT_OK*/



/*o1*/	stqd		atOutSizeProcessedQw_,	0(outSizeProcessed);

	nop $127;
_returnToCaller:

/*o?*/	bi		$lr;
/*-------------------------------------------------------------------------*/

.align	3

_error1:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error2:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error3:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error4:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error5:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;
_error6:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error7:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error8:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error9:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error10:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error11:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error12:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error13:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error14:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error15:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error16:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error17:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error18:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;

_error19:
/*e2*/	il		$3,			1;/*LZMA_RESULT_DATA_ERROR*/
/*o?*/	bi		$lr;





/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	uninitialized data*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/

.section .bss
.align 5	/* must be >= 4*/

.global	_gInSizeProcessed
_gInSizeProcessed:
	.word	0,0,0,0

.global	_gOutSizeProcessed
_gOutSizeProcessed:
	.word	0,0,0,0



.section .rodata
.align 5	/* must be >= 4*/

.global _gRepTbl
/*		rep0:	01,02,03,04,05,06,07,08,09,0A,0B,0C,0D,0E,0F,10,11,12,13,14,15,16,17,18,19,1A,1B,1C,1D,1E,1F,err*/
_gRepTbl:	.byte	16,16,18,16,20,18,21,16,18,20,22,24,26,28,30,16
		.byte							17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0

						
.global _gDup0MaskTbl				/* rep0*/
_gDup0MaskTbl:	.byte 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	/* 01*/
		.byte 	0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01	/* 02*/
		.byte 	0x00, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00	/* 03*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03	/* 04*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00	/* 05*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x01, 0x02, 0x03	/* 06*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x01	/* 07*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	/* 08*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06	/* 09*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05	/* 0A*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04     	/* 0B*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x00, 0x01, 0x02, 0x03	/* 0C*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x00, 0x01, 0x02	/* 0D*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x00, 0x01	/* 0E*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x00	/* 0F*/
		.byte 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F	/* 10+*/

.global _gDup1MaskTbl
_gDup1MaskTbl:	.byte 	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 01*/
		.byte 	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 02*/
		.byte 	0x01, 0x02, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 03*/
		.byte 	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 04*/
		.byte 	0x01, 0x02, 0x03, 0x04, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 05*/
		.byte 	0x04, 0x05, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 06*/
		.byte 	0x02, 0x03, 0x04, 0x05, 0x06, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 07*/
		.byte 	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 08*/
		.byte 	0x07, 0x08, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 09*/
		.byte 	0x06, 0x07, 0x08, 0x09, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 0A*/
		.byte 	0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 0B*/
		.byte 	0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 0C*/
		.byte 	0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 0D*/
		.byte 	0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x80, 0x80, 0x80, 0x80	/* 0E*/
		.byte 	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x80, 0x80	/* 0F*/
		.byte 	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 10*/
		.byte 	0x10, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 11*/
		.byte 	0x10, 0x11, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 12*/
		.byte 	0x10, 0x11, 0x12, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 13*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 14*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 15*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 16*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 17*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 18*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 19*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80	/* 1A*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x80, 0x80, 0x80, 0x80, 0x80	/* 1B*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x80, 0x80, 0x80, 0x80	/* 1C*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x80, 0x80, 0x80	/* 1D*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x80, 0x80	/* 1E*/
		.byte 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x80	/* 1F*/


