/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

/*////////////////////////////////////////////////////////////////////////*/

/*	Edge Zlib SPU Version*/
/*	This file is the optimized asm version of the SPU code for*/
/*	"longest_match", the C implementation of which is in*/
/*	"edgezlib_deflate.c".*/

/*////////////////////////////////////////////////////////////////////////*/


.equ pDeflateState, 3
.equ cur_match, 4
.equ cmp128, 14
.equ cmp16, 14
.equ cmp16hi, 14
.equ cmp16hineg, 14
.equ lz, 29
.equ matchCountGt15, 35
.equ offset, 14
.equ strend, 15
.equ zero, 16
.equ one, 12
.equ minusone, 6
.equ fortyfour, 2
.equ fiftysix, 7
.equ hundredeight, 6
.equ hundredtwentyfour, 9
.equ fifteen, 33
.equ hundredtwenty, 10
.equ hundredsixteen, 13
.equ hundrdforty, 14
.equ fiftytwo, 8
.equ mask_ffff, 16
.equ scanStart, 17
.equ scan, 14
.equ scan_inMatchCountLoop, 30
.equ match_inMatchCountLoop, 28
.equ mask128, 11
.equ scanMask128, 19
.equ matchMask128, 21
.equ scanOffset, 34
.equ matchOffset, 31
.equ scanPrev, 27
.equ scanNext, 32
.equ matchPrev, 33
.equ matchNext, 26
.equ scanPrevRotated, 27
.equ scanNextRotated, 29
.equ matchPrevRotated, 14
.equ matchNextRotated, 35
.equ scan128, 27
.equ match128, 14
.equ bScanGtStrend, 36
.equ best_len, 10
.equ best_lenOUT, 25
.equ nice_match, 5
.equ nice_match_orig, 5
.equ bLookAheadGtNiceMatch, 26
.equ diffStrendScan, 14
.equ len, 14
.equ bNiceMatchGtLen, 26
.equ bLenGtBestLen, 19
.equ orig_chain_length, 9
.equ chain_length_shft2, 23
.equ chain_length, 9
.equ good_match, 14
.equ bGoodMatchGtBestLen, 14
.equ chain_length_is_0, 14
.equ calc_limit, 2
.equ limit, 2
.equ bCurMatchGtLimit, 19
.equ match, 21
.equ max_dist, 2
.equ strstart, 6
.equ window, 7
.equ wmask, 8
.equ curMatchANDWMask, 27
.equ offsetForPrev, 23
.equ prev, 20
.equ curMatchQword, 23
.equ curMatchQwordOffst, 28
.equ curMatchQwordRot, 28
.equ cur_match32, 23
.equ match_start, 24
.equ match_start_qword, 5
.equ match_start_ins_msk, 6
.equ lookahead, 13
.equ bLookAheadGtBestLen, 2
.equ scan_end, 22
.equ scan_end1, 18
.equ match_at_best_len, 19
.equ bMtchBstLenEqScnEnd, 19
.equ bMtchBstLenEqScnEndH, 19
.equ match_at_best_len1, 14
.equ bMtchBstLen1EqScnEnd1, 14
.equ bMtchBstLen1EqScnEnd1H, 14
.equ pMatch_best_len, 14
.equ match_best_len_qw, 19
.equ match_best_len_qw_rot, 26
.equ match_best_len1_qw, 28
.equ match_best_len1_qw_rot, 14
.equ match0qw, 27
.equ match0qwRot, 29
.equ match0, 27
.equ scan0qw, 30
.equ scan0qwRot, 26
.equ scan0, 26
.equ bMatch0EqScan0, 26
.equ bMatch0EqScan0H, 26
.equ match1qw, 21
.equ match1qwRot, 19
.equ match1, 19
.equ scan1qw, 31
.equ scan1qwRot, 29
.equ scan1, 29
.equ bMatch1EqScan1, 14
.equ bMatch1EqScan1H, 14
.equ scan_end_qw, 21
.equ scan_end_qwRot, 18
.equ scan_end1_qw, 19
.equ scan_end1_qwRot, 26
.equ pScanEnd, 27
.equ scan_endX_qw, 22
.equ scan_endX_qwRot, 25
.equ scan_end1X_qw, 19
.equ scan_end1X_qwRot, 18
.equ pScanEndX, 19
.equ match_start_qw, 24
.equ good_match_qw, 14
.equ good_match_qw_rot, 21
.equ lookahead_qw, 13
.equ lookahead_qw_rot, 18
.equ prev_qw, 20
.equ wmask_qw, 8
.equ wmask_qw_rot, 12
.equ bStrStartGtMaxDist, 15
.equ prev_length_qw, 10
.equ prev_length_qw_rot, 12
.equ chain_length_qw, 9
.equ chain_length_qw_rot, 14
.equ nice_match_qw, 5
.equ w_size_qw, 2
.equ w_size_qw_rot, 8
.equ w_size, 2
.equ strstart_qw, 6
.equ strstart_qw_rot, 12
.equ window_qw, 7
.equ window_qw_rot, 13
.equ bBstLen1Match0BothPass, 14
.section .text.align64
.align 6
.type _edgeZlib_longest_match_spu_asm, @function
.global _edgeZlib_longest_match_spu_asm

_edgeZlib_longest_match_spu_asm:
	il			fortyfour,				0x2C;
	lqd			nice_match_qw,			0x90(pDeflateState);
	il			hundredeight,			0x6C;
	hbrr		_HintEnterMainLoop,		_MainLoopBody;
	il			fiftysix,				0x38;
	lqx			w_size_qw,				pDeflateState,			fortyfour;
	ai			w_size_qw_rot,			pDeflateState,			0x2C;
	lqx			strstart_qw,			pDeflateState,			hundredeight;
	il			hundredtwentyfour,		0x7C;
	lqx			window_qw,				pDeflateState,			fiftysix;
	il			hundredtwenty,			0x78;
	fsmbi		mask128,				0xFFFF;	
	ai			strstart_qw_rot,		pDeflateState,			0x6C;
	lqx			chain_length_qw,		pDeflateState,			hundredtwentyfour;
	ai			window_qw_rot,			pDeflateState,			0x38;
	lqx			prev_length_qw,			pDeflateState,			hundredtwenty;
	ai			chain_length_qw_rot,	pDeflateState,			0x7C;
	rotqby		w_size,					w_size_qw,				w_size_qw_rot;
	il			fiftytwo,				0x34;
	rotqby		strstart,				strstart_qw,			strstart_qw_rot;
	ai			prev_length_qw_rot,		pDeflateState,			0x78;
	rotqby		window,					window_qw,				window_qw_rot;
	il			hundredsixteen,			0x74;
	rotqby		nice_match_orig,		nice_match_qw,			pDeflateState;
	ai			max_dist,				w_size,					-0x106;
	rotqby		orig_chain_length,		chain_length_qw,		chain_length_qw_rot;
	il			hundrdforty,			0x8C;
	lqx			wmask_qw,				pDeflateState,			fiftytwo;
	clgt		bStrStartGtMaxDist,		strstart,				max_dist;
	fsmbi		zero,					0x0000;
	a			scanStart,				window,					strstart;
	rotqby		best_len,				prev_length_qw,			prev_length_qw_rot;
	sf			calc_limit,				max_dist,				strstart;
	fsmbi		minusone,				0xFFFF;
	ai			wmask_qw_rot,			pDeflateState,			0x34;
	lqx			lookahead_qw,			pDeflateState,			hundredsixteen;
	ai			lookahead_qw_rot,		pDeflateState,			0x74;
	lqx			good_match_qw,			pDeflateState,			hundrdforty;
	a			pScanEndX,				scanStart,				best_len;
	lqd			prev_qw,				0x40(pDeflateState);
	ai			good_match_qw_rot,		pDeflateState,			0x8C;
	rotqby		wmask,					wmask_qw,				wmask_qw_rot;
	il			one,					1;
	lqx			scan_endX_qw,			scanStart,				best_len;
	rotmi		chain_length_shft2,		orig_chain_length,		-2;
	lqd			match_start_qw,			0x70(pDeflateState);
	ai			scan_endX_qwRot,		pScanEndX,				0xD;
	rotqby		lookahead,				lookahead_qw,			lookahead_qw_rot;
	ai			scan_end1X_qwRot,		pScanEndX,				0xC;
	lqx			scan_end1X_qw,			pScanEndX,				minusone;
	selb		limit,					zero,					calc_limit,			bStrStartGtMaxDist;
	rotqby		good_match,				good_match_qw,			good_match_qw_rot;
	ai			strend,					scanStart,				258;
	fsmbi		mask_ffff,				0x3333;
	a			match,					window,					cur_match;
	rotqby		prev,					prev_qw,				pDeflateState;
	clgt		bLookAheadGtNiceMatch,	lookahead,				nice_match_orig;
	rotqby		match_start,			match_start_qw,			pDeflateState;
	and			curMatchANDWMask,		cur_match,				wmask;
	rotqby		scan_end,				scan_endX_qw,			scan_endX_qwRot;
	clgt		bGoodMatchGtBestLen,	good_match,				best_len;
	rotqby		scan_end1,				scan_end1X_qw,			scan_end1X_qwRot;
	selb		nice_match,				lookahead,				nice_match_orig,	bLookAheadGtNiceMatch;
	lnop;
	selb		chain_length,			chain_length_shft2,		orig_chain_length,	bGoodMatchGtBestLen;
_HintEnterMainLoop:
	br			_MainLoopBody;

.align 6
_MainLoopConditionTest:
	ai			chain_length,			chain_length,			-1;
	and			cur_match,				mask_ffff,				cur_match32;
	ceqi		chain_length_is_0,		chain_length,			0;
	hbrr		_HintLoopBack1,			_MainLoopConditionTest;
	clgt		bCurMatchGtLimit,		cur_match,				limit;
	lnop;
	a			match,					window,					cur_match;
	brnz		chain_length_is_0,		_Exit2;
	and			curMatchANDWMask,		cur_match,				wmask;
	brz			bCurMatchGtLimit,		_Exit2;
_MainLoopBody:
	a			pMatch_best_len,		match,					best_len;
	lqx			match_best_len_qw,		match,					best_len;
	a			offsetForPrev,			curMatchANDWMask,		curMatchANDWMask;
	shlqbyi		best_lenOUT,			best_len,				0x00;
	ai			match_best_len_qw_rot,	pMatch_best_len,		0xD;
	lqx			match0qw,				window,					cur_match;
	a			curMatchQwordOffst,		offsetForPrev,			prev;
	lqx			curMatchQword,			offsetForPrev,			prev;
	ai			match0qwRot,			match,					0xD;
	lqd			scan0qw,				0(scanStart);
	ai			curMatchQwordRot,		curMatchQwordOffst,		0xE;
	rotqby		match_at_best_len,		match_best_len_qw,		match_best_len_qw_rot;
	ai			scan0qwRot,				scanStart,				0xD;
	rotqby		cur_match32,			curMatchQword,			curMatchQwordRot;
	ceqb		bMtchBstLenEqScnEnd,	match_at_best_len,		scan_end;
	lqx			match_best_len1_qw,		pMatch_best_len,		minusone;
	xsbh		bMtchBstLenEqScnEndH,	bMtchBstLenEqScnEnd;
	rotqby		match0,					match0qw,				match0qwRot;
	ai			match_best_len1_qw_rot,	pMatch_best_len,		0xC;
_HintLoopBack1:
	brhz		bMtchBstLenEqScnEndH,	_MainLoopConditionTest;
	ai			match1qwRot,			match,					0xE;
	hbrr		_HintLoopBack2,			_MainLoopConditionTest;
	ai			scan1qwRot,				scanStart,				0xE;
	rotqby		scan0,					scan0qw,				scan0qwRot;
	ai			scan_inMatchCountLoop,	scanStart,				3;
	rotqby		match_at_best_len1,		match_best_len1_qw,		match_best_len1_qw_rot;
	ai			match_inMatchCountLoop,	match,					3;
	lqx			scan1qw,				scanStart,				one;
	ceqb		bMatch0EqScan0,			match0,					scan0;
	lqx			match1qw,				match,					one;
	ceqb		bMtchBstLen1EqScnEnd1,	match_at_best_len1,		scan_end1;
	lqd			scanPrev,				0(scan_inMatchCountLoop);
	xsbh		bMatch0EqScan0H,		bMatch0EqScan0;
	lqd			scanNext,				16(scan_inMatchCountLoop);
	xsbh		bMtchBstLen1EqScnEnd1H,	bMtchBstLen1EqScnEnd1;
	lqd			matchPrev,				0(match_inMatchCountLoop);
	and			bBstLen1Match0BothPass,	bMatch0EqScan0H,		bMtchBstLen1EqScnEnd1H;
	lqd			matchNext,				16(match_inMatchCountLoop);
	andi		scanOffset,				scan_inMatchCountLoop,	0xF;
	rotqby		scan1,					scan1qw,				scan1qwRot;
	andi		matchOffset,			match_inMatchCountLoop,	0xF;
	rotqby		match1,					match1qw,				match1qwRot;
	nop $13;
_HintLoopBack2:
	brhz		bBstLen1Match0BothPass,	_MainLoopConditionTest;

	ceqb		bMatch1EqScan1,			match1,					scan1;
	shlqby		scanMask128,			mask128,				scanOffset;
	nop $41;
	shlqby		matchMask128,			mask128,				matchOffset;
	xsbh		bMatch1EqScan1H,		bMatch1EqScan1;
	rotqby		scanPrevRotated,		scanPrev,				scanOffset;
	nop $13;
	brhz		bMatch1EqScan1H,		_MainLoopConditionTest;
	nop $63;
	rotqby		matchPrevRotated,		matchPrev,				matchOffset;
_MatchCountLoop:
	ai			match_inMatchCountLoop,	match_inMatchCountLoop,	16;
	rotqby		scanNextRotated,		scanNext,				scanOffset;
	il			fifteen,				0xF;
	rotqby		matchNextRotated,		matchNext,				matchOffset;
	clgt		bScanGtStrend,			scan_inMatchCountLoop,	strend;
	lqd			matchNext,				16(match_inMatchCountLoop);
	nop $29;
	lqd			scanNext,				32(scan_inMatchCountLoop);
	selb		scan128,				scanNextRotated,		scanPrevRotated,	scanMask128;
	selb		match128,				matchNextRotated,		matchPrevRotated,	matchMask128;
	ceqb		cmp128,					scan128,				match128;
	lnop;
	ori			scanPrevRotated,		scanNextRotated,		0;
	gbb			cmp16,					cmp128;
	shlqbyi		cmp16hi,				cmp16,					2;
	nor			cmp16hineg,				cmp16hi,				cmp16hi;
	clz			lz,						cmp16hineg;
	shlqbyi		matchPrevRotated,		matchNextRotated,		0x00;
	clgti		matchCountGt15,			lz,						15;
	brnz		bScanGtStrend,			_ScanGtStrend;
	nop $34;
	brnz		matchCountGt15,			_MatchCountWas16;				/*lz may actually have been greater than 16 due to uninited data.  We interpret this as 16.*/
/*_ExitMatchCountLoop:*/
	selb		offset,					lz,						fifteen,		matchCountGt15;
	hbrr		_HintLoopBack3,			_MainLoopConditionTest;
	a			scan,					scan_inMatchCountLoop,	offset;
	lnop;
_ExitMatchCountLoopWithoutFinalAddition:
	sf			diffStrendScan,			scan,					strend;
	sfi			len,					diffStrendScan,			258;
	cgt			bLenGtBestLen,			len,					best_len;
	lqx			scan_end_qw,			scanStart,				len;
	cgt			bNiceMatchGtLen,		nice_match,				len;
	lnop;
	a			pScanEnd,				scanStart,				len;
	brz			bLenGtBestLen,			_MainLoopConditionTest;
	ori			best_lenOUT,			len,					0;
	brz			bNiceMatchGtLen,		_Exit1;
	ai			scan_end_qwRot,			pScanEnd,				0xD;
	lqx			scan_end1_qw,			pScanEnd,				minusone;
	ai			scan_end1_qwRot,		pScanEnd,				0xC;
	shlqbyi		best_len,				len,					0x00;
	ori			match_start,			cur_match,				0;
	rotqby		scan_end,				scan_end_qw,			scan_end_qwRot;
	rotqby		scan_end1,				scan_end1_qw,			scan_end1_qwRot;
_HintLoopBack3:
	br			_MainLoopConditionTest;


_Exit1:
	ori			match_start,			cur_match,				0;
_Exit2:
	hbr			_HintReturn,			$lr;
	clgt		bLookAheadGtBestLen,	lookahead,				best_lenOUT;
	lqd			match_start_qword,		0x70(pDeflateState);
	nop $2;
	cwd			match_start_ins_msk,	0x00(pDeflateState);
	nop $34;
	shufb		match_start_qword,		match_start,			match_start_qword,	match_start_ins_msk;
	nop $7;
	stqd		match_start_qword,		0x70(pDeflateState);
	selb		$3,						lookahead,				best_lenOUT,		bLookAheadGtBestLen;
_HintReturn:
	bi			$lr;


_ScanGtStrend:
	ai			scan,					scan_inMatchCountLoop,	-1;
	br			_ExitMatchCountLoopWithoutFinalAddition;


_MatchCountWas16:
	ai			scan_inMatchCountLoop,	scan_inMatchCountLoop,	16;
	br			_MatchCountLoop;

	.size _edgeZlib_longest_match_spu_asm, .-_edgeZlib_longest_match_spu_asm



/*////////////////////////////////////////////////////////////////////////*/
