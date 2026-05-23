.set	INTERLEAVE_CODE	, 1

/* Set this to 0 or 1 (normally 1)*/
/*Note: wrkmem will not be cleared before use in the assembly version unless this is on.*/
/*	If this is off, you'll get legal (but non-reproduceable) compressed data.*/
.set PRECLEAR_WORKING_MEMORY , 1







/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	uninitialized data*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/







.equ wrkmemOfst, 40
.equ wrkmemOfst_, 48
.equ wrkmemPlus0x10, 41
.equ wrkmemPlus0x20, 42
.equ wrkmemPlus0x30, 43
.equ wrkmemPlus0x40, 44
.equ wrkmemPlus0x50, 45
.equ wrkmemPlus0x60, 46
.equ wrkmemPlus0x70, 47
.equ ipIfGotoLiteral, 50
.equ rval1, 57
.equ _0xe0e0303, 22
.equ shufMaskDictInput, 54
.equ _0xffff0e0e, 21
.equ op_, 46
.equ shufMaskOp_, 54
.equ label9TInAllFields, 69
.equ _0x00010203, 10
.equ numEqBytesEq6, 46
.equ lvalIf14, 53
.equ rvalIf14, 50
.equ rvalIf16, 5
.equ lval, 51
.equ _0, 9
.equ _0x03030303, 11
.equ _0x4000, 14
.equ _0x800, 13
.equ _0x80038080, 15
.equ _0x803e, 2
.equ _0x80808001, 6
.equ _0x80808002, 18
.equ _0xbfff, 16
.equ _0xffe, 17
.equ _0xfffe, 19
.equ _0xffff, 20
.equ _1, 23
.equ _16, 25
.equ _17, 26
.equ _2, 24
.equ _32, 27
.equ _32897, 28
.equ _33, 29
.equ _9, 30
.equ _neg1, 31
.equ _neg2, 32
.equ atDictDindex, 54
.equ atDictDindex2, 64
.equ atIp0, 54
.equ atIp1, 52
.equ atIp1Shl5, 57
.equ atIp2, 53
.equ atIp2Shl10, 58
.equ atIp3Shl16, 59
.equ atOpLess2, 51
.equ atOutLen, 2
.equ deltaIp, 47
.equ deltaMoff, 49
.equ dict, 7
.equ dictDindex2Lval, 61
.equ dictPlusDindex2Mpy2, 70
.equ dindexMpy2, 57
.equ dindex2Mpy2, 58
.equ doLoop, 50
.equ end, 47
.equ gOutLen, 2
.equ gotoAdrs, 70
.equ gotoLiteral, 58
.equ ifTrue, 3
.equ ii, 43
.equ iiBytes16, 46
.equ iiBytes16Shr, 46
.equ iiBytesL, 46
.equ iiBytesR, 47
.equ in, 3
.equ inLenGt13, 46
.equ in_end, 41
.equ in_len, 4
.equ ip, 48
.equ ipBytes16, 55
.equ ipBytesL, 51
.equ ipBytesR, 52
.equ ipEndGtIp, 50
.equ ipLessIn, 60
.equ ip_end, 44
.equ label10T, 71
.equ label11T, 58
.equ label13pt5, 33
.equ label17, 34
.equ label22, 35
.equ label33To37Values, 55
.equ label33To37ValuesIf33, 42
.equ label33To37ValuesIf33Or34, 55
.equ label33To37ValuesIf34, 40
.equ label33To37ValuesIf36, 44
.equ label33To37ValuesIf36Or37, 56
.equ label33To37ValuesIf37, 43
.equ label40, 36
.equ label8T, 63
.equ label9T, 68
.equ literal, 37
.equ m, 46
.equ m3_m4_offset, 38
.equ mBytes16, 49
.equ mBytes16EqIpBytes16, 49
.equ mBytes16NeIpBytes16, 49
.equ mBytesL, 49
.equ mBytesR, 52
.equ mOff2Eq0, 71
.equ mOff2Gt0xbfff, 72
.equ mOff2GtIpLessIn, 60
.equ m_len, 43
.equ m_off, 54
.equ m_pos, 65
.equ maxIpBytes, 47
.equ maxIpBytesGt0, 50
.equ maxIpBytesTooBig, 50
.equ mlenDelta, 50
.equ mlenGt255, 46
.equ mlenGt33, 50
.equ mlenGt9, 54
.equ mlenLess1, 50
.equ mlenLess1Shl5, 50
.equ mlenLess2, 43
.equ moff2, 59
.equ moff2Gt0x800, 67
.equ moffLess0x4000, 49
.equ moffLess0x4000And0x4000, 49
.equ moffLess0x4000And0x4000Shr11, 49
.equ moffAnd4000, 52
.equ moffAnd4000Shr11, 52
.equ moffAnd4000Shr11Or16, 52
.equ moffAnd63, 43
.equ moffAnd63Shl2, 43
.equ moffAnd7, 53
.equ moffAnd7Shl2, 53
.equ moffDelta, 57
.equ moffEq0, 66
.equ moffGt0x4000, 47
.equ moffGt0x800, 73
.equ moffGt0xbfff, 63
.equ moffGtIpLessIn, 69
.equ moffShr3, 43
.equ moffShr6, 47
.equ mpos2, 63
.equ mpos2Byte3EqIpByte3, 72
.equ mpos2Bytes16, 67
.equ mpos2Bytes16EqIpBytes16, 55
.equ mpos2BytesL, 67
.equ mpos2BytesR, 69
.equ mposByte3EqIpByte3, 72
.equ mposBytes0To2NeIpBytes0To2, 60
.equ mposBytes16, 57
.equ mposBytes16EqIpBytes16, 57
.equ mposBytes16NeqIpBytes16, 55
.equ mposBytesL, 57
.equ mposBytesR, 63
.equ negNumIiBytesR, 50
.equ negNumIpBytesR, 54
.equ negNumMBytesR, 52
.equ negNumMpos2BytesR, 71
.equ negNumMposBytesR, 66
.equ negNumOpBytesL, 52
.equ neqBits, 46
.equ numBytesToCopy, 47
.equ numBytesToOutput, 53
.equ numBytesToOutputShl8, 52
.equ numEqBytesPlus19, 46
.equ numEqualBytes, 49
.equ numEqualBytesEq16, 49
.equ numEqualBytesPlus16, 49
.equ numIiBytesL, 46
.equ numIpBytesL, 52
.equ numLoops, 51
.equ numLoopsToDo, 51
.equ numLoopsTooBig, 54
.equ numMBytesL, 55
.equ numMpos2BytesL, 68
.equ numMposBytesL, 67
.equ numOpBytesL, 52
.equ numOpBytesR, 53
.equ op, 45
.equ op1Lval, 43
.equ opEqOut, 3
.equ opLess5, 51
.equ opLessOut, 2
.equ opRval, 50
.equ opRvalIf1st, 50
.equ opRvalIf2nd, 55
.equ opRvalIf33, 57
.equ opRvalIf33Or34, 50
.equ opRvalIf36, 56
.equ opRvalIf36Or37, 49
.equ opRvalIf37, 49
.equ opRvalIf3rd, 43
.equ out, 12
.equ out_len, 4
.equ picLabel, 13
.equ picLoc, 8
.equ picOffset, 8
.equ qwAtDictDindex, 59
.equ qwAtDictDindex2, 61
.equ qwAtIi, 47
.equ qwAtIiPlus16, 49
.equ qwAtIp, 51
.equ qwAtIpPlus16, 53
.equ qwAtM, 49
.equ qwAtMPlus16, 54
.equ qwAtMpos, 68
.equ qwAtMpos2, 67
.equ qwAtMpos2Plus16, 69
.equ qwAtMposPlus16, 70
.equ qwAtOp, 5
.equ qwAtOpLess2, 50
.equ qwAtOutLen, 3
.equ rval, 54
.equ rval0, 54
.equ rvalShl5, 57
.equ safeStoreAdrs, 42
.equ selMaskOp, 52
.equ selMaskY, 39
.equ selMaskZ, 41
.equ shufMaskDictDindex, 63
.equ shufMaskDictDindex2, 57
.equ shufMaskOp, 51
.equ shufMaskOp1, 47
.equ shufMaskOpLess2, 56
.equ shufMaskOpPlus1, 3
.equ shufMaskOpPlus2, 3
.equ shufMaskOutLen, 5
.equ t, 47
.equ tBeyondOpQw, 10
.equ tGt0, 66
.equ tGt17, 51
.equ tGt18, 70
.equ tGt238, 4
.equ tGt3, 52
.equ tGt30, 50
.equ tLess3, 57
.equ tLit, 50
.equ tPlus17, 10
.equ tTooBig, 51
.equ tt, 50
.equ ttGt255, 10
.equ ttLess1, 51
.equ wrkmem, 7
.equ label33To37ValuesIf34_, 29
.equ label33To37ValuesIf33_, 40
.equ label33To37ValuesIf36_, 39
.equ label33To37ValuesIf37_, 30
.equ tLit_, 53
.equ tGt17_, 54
.equ tGt30_, 51
.equ ipIfGotoLiteral_, 56
.equ m_off_, 62
.equ label8T_, 57
.equ atDictDindex2_, 59
.equ t_, 64
.equ gotoAdrs_, 66
.equ m_off__, 59
.equ label10T_, 60
.equ mposBytes16EqIpBytes16_, 55
.equ qwAtDictDindex2_, 57
.equ m_pos_, 60
.equ gotoAdrs__, 57
.equ rval_, 5
.equ numOpBytesL_, 51
.equ numOpBytesR_, 49
.equ negNumOpBytesL_, 50
.equ selMaskOp_, 49
.equ qwAtIi_, 50
.equ t__, 51
.equ qwAtIiPlus16_, 46
.equ numIiBytesL_, 49
.equ tTooBig_, 52
.equ negNumIiBytesR_, 5
.equ iiBytesL_, 47
.equ numBytesToCopy_, 53
.equ iiBytesR_, 5
.equ ip_, 47
.equ moffGt0x800_, 46
.equ m_off___, 49
.equ gotoAdrs___, 54
.equ shufMaskOp__, 52
.equ opRval_, 43
.equ negNumIpBytesR_, 50
.equ ipBytesR_, 50
.equ ipBytes16_, 50
.equ m_off____, 46
.equ mlenLess2_, 56
.equ opRval__, 47
.equ shufMaskOp___, 47
.equ numLoops_, 46
.equ numOpBytesL__, 47
.equ numOpBytesR__, 47
.equ selMaskOp__, 50
.equ numBytesToOutput_, 54
.equ numBytesToOutputShl8_, 50
.equ shufMaskOp____, 46
.equ shufMaskOp_____, 43
.equ tGt0_, 2
.equ ii_, 2
.equ qwAtIi__, 4
.equ qwAtIiPlus16__, 6
.equ numIiBytesL__, 7
.equ shufMaskOp______, 3
.equ qwAtOp_, 3
.equ op__, 5
.equ tGt3_, 3
.equ qwAtOpLess2_, 3
.equ opLess5_, 5
.equ atOpLess2_, 5
.equ shufMaskOpLess2_, 10
.equ tGt18_, 3
.equ tLess3_, 10
.equ tt_, 5
.equ op___, 11
.equ ttLess1_, 10
.equ numLoops__, 10
.equ numOpBytesL___, 13
.equ negNumOpBytesL__, 14
.equ numOpBytesR___, 13
.equ selMaskOp___, 14
.equ numLoopsTooBig_, 15
.equ numBytesToOutput__, 13
.equ numBytesToOutputShl8__, 14
.equ shufMaskOp_______, 10
.equ numOpBytesL____, 10
.equ iiBytesL__, 4
.equ numOpBytesR____, 6
.equ negNumOpBytesL___, 7
.equ iiBytes16_, 4
.equ selMaskOp____, 5
.equ numBytesToCopy__, 6
.equ iiBytes16Shr_, 4
.equ qwAtIi___, 3
.equ t___, 4
.equ qwAtIiPlus16___, 7
.equ numIiBytesL___, 11
.equ negNumIiBytesR__, 6
.equ iiBytesL___, 3
.equ numBytesToCopy___, 13
.equ shufMaskOp________, 2
.equ qwAtOp__, 2
.equ gOutLen_, 4
.equ op____, 2
.equ qwAtOutLen_, 2
.global lzopro_lzo1x_1_14_compress_asm
.extern _gOutLen

.text

/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	start of lzopro_lzo1x_1_14_compress_asm*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/

.align 7

lzopro_lzo1x_1_14_compress_asm:

nop $127;	/*reserve room to enable stopd above without changing alignment*/
	lnop;
ila		gOutLen,		_gOutLen;
/*o1*/	brsl		picLoc,			_picLabel;

_picLabel:
/*e2*/	il		_0,			0;
/*oe4*/	shlqbyi/*copy*/	in,			$3,			0;

/*e2*/	ila		_0x00010203,		0x00010203;
/*oe4*/	shlqbyi/*copy*/	in_len,			$4,			0;

/*e2*/	ilh		_0x03030303,		0x0303;
/*oe4*/	shlqbyi/*copy*/	out,			$5,			0;

/*e2*/	ila		picLabel,		_picLabel;			
/*oe4*/	shlqbyi/*copy*/	wrkmem,			$7,			0;

/*e2*/	il		_0x4000,		0x4000;

/*e2*/	sf		picOffset,		picLabel,		picLoc;		

/*e2*/	il		_0x800,			0x0800;
	lnop;

/*e2*/	ilhu		_0x80038080,		0x8003;
/*o1*/	stqx					$6,			picOffset,		gOutLen;

/*e2*/	ila		_0x803e,		0x803e;

/*e2*/	iohl		_0x80038080,		0x8080;

/*e2*/	ilhu		_0x80808001,		0x8080;

/*e2*/	ila		_0xbfff,		0xbfff;

/*e2*/	iohl		_0x80808001,		0x8001;

/*e2*/	il		_0xffe,			0xffe;

/*e2*/	ai		_0x80808002,		_0x80808001,		1;

/*e2*/	ila		_0xfffe,		0xfffe;

/*e2*/	ila		_0xffff,		0xffff;

/*e2*/	ilhu		_0xffff0e0e,		0xffff;

/*e2*/	ilhu		_0xe0e0303,		0xe0e0;

/*e2*/	iohl		_0xffff0e0e,		0x0e0e;

/*e2*/	iohl		_0xe0e0303,		0x0303;

/*e2*/	il		_1,			1;

/*e2*/	il		_2,			2;

/*e2*/	il		_16,			16;

/*e2*/	il		_17,			17;

/*e2*/	il		_32,			32;

/*e2*/	ila		_32897,			32897;

/*e2*/	il		_33,			33;

/*e2*/	il		_9,			9;

/*e2*/	il		_neg1,			-1;

/*e2*/	il		_neg2,			-2;

/*e2*/	ila		label13pt5,		_label13pt5;

/*e2*/	ila		label17,		_label17;

/*e2*/	a		label13pt5,		label13pt5,		picOffset;

/*e2*/	a		label17,		label17,		picOffset;

/*e2*/	ila		label22,		_label22;

/*e2*/	ila		label40,		_label40;

/*e2*/	a		label22,		label22,		picOffset;

/*e2*/	a		label40,		label40,		picOffset;

/*e2*/	ila		literal,		_literalAndNextLoop;/*Note: this was originally _literal without interleave*/

/*e2*/	ila		m3_m4_offset,		_m3_m4_offset;

/*e2*/	a		literal,		literal,		picOffset;

/*e2*/	a		m3_m4_offset,		m3_m4_offset,		picOffset;
/*o4*/	fsmbi		selMaskY/*1 if Y field*/, 0x0F00;

ila		wrkmemOfst,		0x10000-0x80;
/*o15*/	hbrr		_clearWrkmemLoopFrom,	_clearWrkmemLoop;

/*e2*/	ai		wrkmemPlus0x10,		wrkmem,			0x10;

/*e2*/	ai		wrkmemPlus0x20,		wrkmem,			0x20;

/*e2*/	ai		wrkmemPlus0x30,		wrkmem,			0x30;

/*e2*/	ai		wrkmemPlus0x40,		wrkmem,			0x40;

/*e2*/	ai		wrkmemPlus0x50,		wrkmem,			0x50;

/*e2*/	ai		wrkmemPlus0x60,		wrkmem,			0x60;

/*e2*/	ai		wrkmemPlus0x70,		wrkmem,			0x70;
	lnop;

.align	5
	
_clearWrkmemLoop: /*================================================================*/
/*o1*/	stqx		_0,			wrkmem,			wrkmemOfst;

/*o1*/	stqx		_0,			wrkmemPlus0x10,		wrkmemOfst;

	nop $90;
/*o1*/	stqx		_0,			wrkmemPlus0x20,		wrkmemOfst;

/*e2*/	ori/*copy*/	wrkmemOfst_,		wrkmemOfst,		0;
/*o1*/	stqx		_0,			wrkmemPlus0x30,		wrkmemOfst;

/*o1*/	stqx		_0,			wrkmemPlus0x40,		wrkmemOfst;

/*o1*/	stqx		_0,			wrkmemPlus0x50,		wrkmemOfst;

	ai		wrkmemOfst,		wrkmemOfst,		-0x80;
/*o1*/	stqx		_0,			wrkmemPlus0x60,		wrkmemOfst_;

/*o1*/	stqx		_0,			wrkmemPlus0x70,		wrkmemOfst_;

_clearWrkmemLoopFrom:
/*o1*/	brnz/*frequent*/	wrkmemOfst_,		_clearWrkmemLoop;
/*================================================================*/

ila		label33To37ValuesIf34,	_m3_m4_len;
/*o4*/	fsmbi		selMaskZ/*1 if Z field*/, 0x00F0;

/*e2*/	ila		label33To37ValuesIf33,	_m3_m4_offset;

/*e2*/	a		label33To37ValuesIf34,	label33To37ValuesIf34,	picOffset;

/*e2*/	a		label33To37ValuesIf33,	label33To37ValuesIf33,	picOffset;

/*e2*/	selb		label33To37ValuesIf34,	label33To37ValuesIf34,	_1,			selMaskY;

/*e2*/	selb		label33To37ValuesIf33,	label33To37ValuesIf33,	_1,			selMaskY;

/*e2*/	selb		label33To37ValuesIf34_,	label33To37ValuesIf34,	_33,			selMaskZ;

/*e2*/	selb		label33To37ValuesIf33_,	label33To37ValuesIf33,	_0,			selMaskZ;
	lnop;

/*e2*/	ila		safeStoreAdrs,		_gSafeStoreAdrs;
/*oe4*/	shlqbyi/*copy*/	label33To37ValuesIf37,	label33To37ValuesIf34_,	0;

	nop $39;
/*oe4*/	shlqbyi/*copy*/	label33To37ValuesIf36,	label33To37ValuesIf33_,	0;

/*e2*/	a		safeStoreAdrs,		safeStoreAdrs,		picOffset;
/*oe4*/	shlqbyi/*copy*/	op,			$5/*out*/,		0;

/*e2*/	clgti		inLenGt13,		in_len,			13;
/*oe2*/	ori/*copy*/	t,			in_len,			0;

/*e2*/	selb		label33To37ValuesIf37,	label33To37ValuesIf37,	_0x4000,		selMaskY;
	lnop;

/*e2*/	selb		label33To37ValuesIf36_,	label33To37ValuesIf36,	_0x4000,		selMaskY;
/*o6*/	lqd		qwAtOp,			0($5/*op*/); /* enable output to start non-qword aligned*/

/*e2*/	selb		label33To37ValuesIf37_,	label33To37ValuesIf37,	_9,			selMaskZ;
/*o?*/	brz/*iffAtEnd*/	inLenGt13,		_label43;	

/*--------------------------------------------------	*/

/*e2*/	a		in_end,			in,			in_len;
/*oe4*/	shlqbyi/*copy*/	dict,			wrkmem,			0;

/*eo2*/	ori/*copy*/	ii,			in/*ip*/,			0;

/*e2*/	ai		ip_end,			in_end,			-13;

/*e2*/	andi		numIiBytesL,		ii,			15;
/*o6*/	lqd		qwAtIi,			0(ii);

/*e2*/	ai		ip,			in,			3;
/*o6*/	lqd		qwAtIiPlus16,		16(ii);

_literal:
/*e2*/	sf		tLit,			ii,			ip;
/*o15*/	hbrr		_nextFrom,		_next;

/*e2*/	ai		ip,			ip,			1;

/*e2*/	clgti		tGt17,			tLit,			17;

/*e2*/	clgti		tGt30,			tLit,			30;

/*e2*/	sf		ip,			tGt17,			ip;



/*e2*/	sf		ip,			tGt30,			ip;
	lnop;
	


/*e2*/	clgt		ipEndGtIp,		ip_end,			ip;
/*o6*/	lqd		qwAtIp,			0(ip);

/*e2*/	andi		numIpBytesL,		ip,			15;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*// about 4 or maybe more delays here for hint*/







/*branch on even adrs*/
_nextFrom:
/*o1*/	brnz/*alwaysExceptEnd*/ ipEndGtIp,	_next;

/*o18*/	br		_done;


/*NOTE: _label40 code has been moved here from below (in interleaved version)*/
/*	to speed it up*/

.align 7

_label40:
/*e2*/	andi		numIiBytesL,		ip/*ii*/,			15;
	lnop;

/*e2*/	clgt		ipEndGtIp,		ip_end,			ip;
/*o6*/	lqd		qwAtIi,			0(ip/*ii*/);

/*e2*/	ori/*copy*/	ii,			ip,			0;
/*o6*/	lqd		qwAtIiPlus16,		16(ip/*ii*/);

	nop $49;
/*o1*/	brz/*end only*/	ipEndGtIp,		_done;
		
_next:
/*e2*/	ai		negNumIpBytesR,		numIpBytesL,		-16;
	lnop;





/*NOTE: this is the most frequent loop */

_literalAndNextLoop: /*==============================================================*/
/*o4*/	shlqby		ipBytesL,		qwAtIp,			numIpBytesL;

/*o4*/	rotqmby		ipBytesR,		qwAtIpPlus16,		negNumIpBytesR;

/*e2*/	sf	/*lit*/	tLit_,			ii,			ip;
/*o1*/	brz/*onlyAtEnd, lit*/			ipEndGtIp,		_done;

/*e2*/	ai	/*lit*/	ipIfGotoLiteral,	ip,			1;
/*{o15}	hbrr		_literalAndNextLoopFrom,_literalAndNextLoop*/
	lnop; /* It is faster for code to fall thru than goto _literalAndNextLoop*/

/*e2*/	clgti	/*lit*/	tGt17_,			tLit_,			17;

/*e2*/	or		ipBytes16,		ipBytesL,		ipBytesR;

/*e2*/	clgti	/*lit*/	tGt30_,			tLit_,			30;
	lnop;
	
/*e2*/	sf	/*lit*/	ipIfGotoLiteral,	tGt17_,			ipIfGotoLiteral;
/*o4*/	shufb		atIp1,			ipBytes16,		ipBytes16,		_0x80808001;

/*o4*/	shufb		atIp2,			ipBytes16,		ipBytes16,		_0x80808002;

/*e2*/	sf	/*lit*/	ipIfGotoLiteral_,	tGt30_,			ipIfGotoLiteral;

/*e4*/	rotmi		atIp0,			ipBytes16,		-24;

/*e4*/	shli		atIp1Shl5,		atIp1,			5;
/*o6*/	lqd	/*lit*/	qwAtIp,			0(ipIfGotoLiteral_);

/*e4*/	shli		atIp2Shl10,		atIp2,			10;
/*o4*/	shufb		atIp3Shl16,		ipBytes16,		ipBytes16,		_0x80038080;

/*e2*/	andi	/*lit*/	numIpBytesL,		ipIfGotoLiteral_,	15;
/*o6*/	lqd	/*lit*/	qwAtIpPlus16,		16(ipIfGotoLiteral_);

/*e2*/	clgt	/*lit*/	ipEndGtIp,		ip_end,			ipIfGotoLiteral_;

/*e2*/	xor		rval0,			atIp0,			atIp1Shl5;

/*e2*/	xor		rval1,			atIp2Shl10,		atIp3Shl16;



/*e2*/	xor		rval,			rval0,			rval1;



/*o4*/	shli		rvalShl5,		rval,			5;







/*e2*/	a		rval,			rval,			rvalShl5;



/*e4*/	rotmi		rval,			rval,			-5+1;
	lnop;
	






/*e2*/	and		dindexMpy2, 		rval,			_0xfffe;
/*o4*/	shufb		shufMaskDictInput,	rval,			rval,			_0xe0e0303;



/*e2*/	and/*fromBelow*/	dindex2Mpy2,		dindexMpy2,		_0xffe;
/*o6*/	lqx		qwAtDictDindex,		dict,			dindexMpy2;



/*e2*/	and		shufMaskDictInput,	shufMaskDictInput,	_0xffff0e0e;

/*e2*/	sf		ipLessIn,		in,			ip;

/*e2*/	ai		shufMaskDictInput,	shufMaskDictInput,	1;

/*e2*/	xor/*from below*/	dindex2Mpy2,		dindex2Mpy2,		_0x803e;

/*o4*/	shufb		atDictDindex,		qwAtDictDindex,		qwAtDictDindex,		shufMaskDictInput;

/*o6*/	lqx/*fromBelow*/	qwAtDictDindex2,	dict,			dindex2Mpy2;





/*e2*/	sf		m_off,			atDictDindex,		ip;
	lnop;
		


/*e2*/	and		m_off_,			m_off,			_0xffff;
/*o4*/	chx		shufMaskDictDindex,	dict,			dindexMpy2;

/*e2*/	ai		negNumIpBytesR/*nextLoop*/,numIpBytesL,		-16;
/*o4*/	rotqby/*fromBelow*/atDictDindex2,		qwAtDictDindex2,	dindex2Mpy2;

/*e2*/	sf/*fromBelow*/	m_pos,			m_off_,			ip;/*b4OrAfterIpChangedBySpeculativeNextLiteral*/

/*e2*/	ceqi		moffEq0,		m_off_,			0;
	lnop;

/*e2*/	andi/*fromBelow*/	numMposBytesL,		m_pos,			15;
/*o4*/	shufb		qwAtDictDindex,		ip,			qwAtDictDindex,		shufMaskDictDindex;

/*e2*/	clgt		moffGt0xbfff,		m_off_,			_0xbfff;
/*o6*/	lqd/*fromBelow*/	qwAtMpos,		0(m_pos);

/*e2*/	clgt		moffGtIpLessIn,		m_off_,			ipLessIn;
/*o6*/	lqd/*fromBelow*/	qwAtMposPlus16,		16(m_pos);

/*e2*/	or		label8T,		moffEq0,		moffGt0xbfff;

/*e2*/	ai/*fromBelow*/	negNumMposBytesR,	numMposBytesL,		-16;
/*o6*/	stqx					qwAtDictDindex,		dict,			dindexMpy2;

/*e2*/	or		label8T_,		label8T,		moffGtIpLessIn;

/*e4*/	rotmi/*fromBelow*/ atDictDindex2_,		atDictDindex2,		-16;

/*e2*/	selb	/*lit*/	ip,			ip,			ipIfGotoLiteral_,	label8T_;
/*_literalAndNextLoopFrom:*/
/*o?*/	brnz/*infrequent*/			label8T_,		_literalAndNextLoop;

shlqby		mposBytesL,		qwAtMpos,		numMposBytesL;

/*o4*/	rotqmby		mposBytesR,		qwAtMposPlus16,		negNumMposBytesR;

/*e2*/	sf		moff2,			atDictDindex2_,		ip;

/*e2*/	sf		t_,			ii,			ip;

/*e2*/	and		moff2,			moff2,			_0xffff;

/*e2*/	or		mposBytes16,		mposBytesL,		mposBytesR;

/*e2*/	sf		mpos2,			moff2,			ip;

/*e2*/	clgti		tGt0,			t_,			0;
	lnop;
	
/*e2*/	ceqb		mposBytes16EqIpBytes16,	mposBytes16,		ipBytes16;
/*o6*/	lqd		qwAtMpos2,		0(mpos2);

/*e2*/	andi		numMpos2BytesL,		mpos2,			15;
/*o6*/	lqd		qwAtMpos2Plus16,	16(mpos2);

/*e2*/	clgti		tGt18,			t_,			18;
	lnop;
	
/*e2*/	ai		negNumMpos2BytesR,	numMpos2BytesL,		-16;
/*o4*/	shufb		mposByte3EqIpByte3,	mposBytes16EqIpBytes16,	mposBytes16EqIpBytes16,	_0x03030303;

/*e2*/	selb		gotoAdrs,		label13pt5,		label17,		tGt18;

/*e2*/	clgt		moffGt0x800,		m_off_,			_0x800;

/*e2*/	selb		gotoAdrs_,		label22,		gotoAdrs,		tGt0;
/*o4*/	shlqby		mpos2BytesL,		qwAtMpos2,		numMpos2BytesL;

/*e2*/	orc		label9T,		mposByte3EqIpByte3,	moffGt0x800;
/*o4*/	rotqmby		mpos2BytesR,		qwAtMpos2Plus16,	negNumMpos2BytesR;

/*e2*/	a		dictPlusDindex2Mpy2,	dict,			dindex2Mpy2;
/*o15*/	hbrr		gotoAdrsFrom,		_literalAndNextLoop;/*_literalAndNextLoop, _label17, _label13pt5 or _label22*/

/*e2*/	ceqi		mOff2Eq0,		moff2,			0;

/*e2*/	clgt		mOff2Gt0xbfff,		moff2,			_0xbfff;

/*e2*/	or		mpos2Bytes16,		mpos2BytesL,		mpos2BytesR;

/*e2*/	clgt		mOff2GtIpLessIn,	moff2,			ipLessIn;

/*e2*/	ceqb		mpos2Bytes16EqIpBytes16, mpos2Bytes16,		ipBytes16;
	lnop;
	
/*e2*/	clgt		moff2Gt0x800,		moff2,			_0x800;
/*o4*/	shufb		label9TInAllFields,	label9T,		label9T,		_0x00010203;

/*e2*/	or		label10T,		mOff2Eq0,		mOff2Gt0xbfff;
/*o4*/	shufb		mpos2Byte3EqIpByte3,	mpos2Bytes16EqIpBytes16, mpos2Bytes16EqIpBytes16, _0x03030303;

/*e2*/	selb		m_off__,			moff2,			m_off_,			label9T;

/*e2*/	or		label10T_,		label10T,		mOff2GtIpLessIn;

/*e2*/	selb		mposBytes16EqIpBytes16_,	mpos2Bytes16EqIpBytes16,mposBytes16EqIpBytes16,	label9TInAllFields;
/*o4*/	chx		shufMaskDictDindex2,	dict,			dindex2Mpy2;

/*e2*/	orc		label11T,		mpos2Byte3EqIpByte3,	moff2Gt0x800;

/*e2*/	xori		mposBytes16NeqIpBytes16,mposBytes16EqIpBytes16_,	-1;

/*e2*/	orc		gotoLiteral,		label10T_,		label11T;
	lnop;
	
/*e2*/	clgti		mposBytes0To2NeIpBytes0To2,mposBytes16NeqIpBytes16,255;
/*o4*/	shufb		qwAtDictDindex2_,	ip,			qwAtDictDindex2,	shufMaskDictDindex2;

/*e2*/	andc		gotoLiteral,		gotoLiteral,		label9T;

/*e2*/	selb		dictDindex2Lval,	dictPlusDindex2Mpy2,	safeStoreAdrs,		label9T;

/*e2*/	or		gotoLiteral,		gotoLiteral,		mposBytes0To2NeIpBytes0To2;
	lnop;
	
/*e2*/	selb		m_pos_,			mpos2,			m_pos,			label9T;
/*o1*/	stqd					qwAtDictDindex2_,	0(dictDindex2Lval);

/*e2*/	selb		gotoAdrs__,		gotoAdrs_,		literal,		gotoLiteral;

/*e2*/	selb	/*lit*/	ip,			ip,			ipIfGotoLiteral_,	gotoLiteral;

	nop $56;
gotoAdrsFrom:
/*o1?*/	bi/*mostToLiteralAndNextLoop*/ gotoAdrs__;

.align 6

_label13pt5:
/*o6**/	lqx		qwAtOpLess2,		op,			_neg2;

/*o15*/	hbrr		_label20From,		_label20;





/*e2*/	ai		opLess5,		op,			-2-3;
/*e2*/	clgti/*b4OrAfterBrz*/ tGt3/*T ifLabel16*/,	t_,			3;



/*e2*/	ai		lvalIf14,		op,			-2;
/*o4**/	rotqby		atOpLess2/*byte3*/,	qwAtOpLess2,		opLess5;

/*o4*/	cbd		shufMaskOp_,		0(op);

/*o4*/	cbd		shufMaskOpLess2,	0(lvalIf14);

/*e2*/	ai		tLess3,			t_,			-3;
/*o4*/	shufb		tGt3/*allFields*/,	tGt3,			tGt3,			_0x00010203;

/*e2**/	or		atOpLess2/*byte3*/,	atOpLess2,		t_;

/*o4*/	shufb		rvalIf16,		tLess3,			qwAtOp,			shufMaskOp_;

/*o4**/	shufb		rvalIf14,		atOpLess2,		qwAtOpLess2,		shufMaskOpLess2;



/*e2*/	selb		lval,			lvalIf14,		op/*lvalIf16*/,		tGt3;/*T ifLabel16*/

/*e2*/	sf		op,			tGt3,			op;

/*e2**/	selb		rval_,			rvalIf14,		rvalIf16,		tGt3;/*T ifLabel16*/



/*o1**/	stqd/*b4 lqd*/				rval_,			0(lval);

/*o6*/	lqd/*after stqd*/	qwAtOp,			0(op);

	nop $127;
_label20From:		
/*o1**/	br/*100%*/	_label20;

.align 6

_label17:
/*e2*/	ai		tt,			t_,			255-18;

/*e2*/	ai		ttLess1,		tt,			-1;



/*o7*/	mpyu		numLoops,		ttLess1,		_32897;













/*o4*/	rotmi		numLoops,		numLoops,		-23;

_label18Loop:
/*e2**/	andi		numOpBytesL,		op,			15;



/*e2**/	sfi		numOpBytesR,		numOpBytesL,		16;

/*e2*/	sfi		negNumOpBytesL,		numOpBytesL,		0;
	lnop;



/*e2**/	clgt		numLoopsTooBig,		numLoops,		numOpBytesR;
/*o4*/	rotqmby		selMaskOp,		_neg1,			negNumOpBytesL;



/*e2**/	selb		numBytesToOutput,	numLoops,		numOpBytesR,		numLoopsTooBig;



/*e2*/	andc		qwAtOp,			qwAtOp,			selMaskOp;

/*e4**/	shli		numBytesToOutputShl8,	numBytesToOutput,	8;

/*e2*/	a		tt,			tt,			numBytesToOutput;

/*e2*/	sf		numLoops,		numBytesToOutput,	numLoops;
/*o1*/	stqd					qwAtOp,			0(op);

/*e2*/	a		op,			op,			numBytesToOutput;
	lnop;

/*e2**/	sf		tt,			numBytesToOutputShl8,	tt;
/*o?**/	brnz/*infrequent*/ numLoops,		_label18Loop;

/*o4*/	cbd		shufMaskOp,		0(op);







/*o4*/	shufb		qwAtOp,			tt,			qwAtOp,			shufMaskOp;

/*e2*/	ai		op,			op,			1;





/*o1*/	stqx					qwAtOp,			op,			_neg1;

_label20:
/*e2**/	ai		negNumIiBytesR,		numIiBytesL,		-16;
	lnop;

/*e2*/	andi		numOpBytesL_,		op,			15;
/*o4*/	shlqby		iiBytesL,		qwAtIi,			numIiBytesL;

	nop $98;
/*o4**/	rotqmby		iiBytesR,		qwAtIiPlus16,		negNumIiBytesR;

/*e2*/	sfi		numOpBytesR_,		numOpBytesL_,		16;

/*e2*/	sfi		negNumOpBytesL_,		numOpBytesL_,		0;

/*e2*/	clgt		tTooBig,		t_,			numOpBytesR_;

/*e2**/	or		iiBytes16,		iiBytesL,		iiBytesR;

/*e2*/	selb		numBytesToCopy,		t_,			numOpBytesR_,		tTooBig;

/*o4*/	rotqmby		selMaskOp_,		_neg1,			negNumOpBytesL_;

/*e2*/	a		ii,			ii,			numBytesToCopy;
/*o4**/	rotqmby		iiBytes16Shr,		iiBytes16,		negNumOpBytesL_;

	nop $42;

/*o6*/	lqd		qwAtIi_,			0(ii);

/*e2*/	sf		t__,			numBytesToCopy,		t_;
/*o15**/	hbrr/*4codeBelow*/ _label20LoopFrom,	_label20Loop;

/*e2**/	selb		qwAtOp,			qwAtOp,			iiBytes16Shr,		selMaskOp_;
/*o6*/	lqd		qwAtIiPlus16_,		16(ii);

/*e2*/	andi		numIiBytesL_,		ii,			15;
	lnop;

/*e2*/	clgti/*4codeBelow*/ tTooBig_,		t__,			16;
/*o1**/	stqd					qwAtOp,			0(op);

/*e2**/	a		op,			op,			numBytesToCopy;
/*o?**/	brz/*infrequent!*/ t__,			_label22;		
	
_label20Loop: /*===========================================================================================*/
/*e2**/	ai		negNumIiBytesR_,		numIiBytesL_,		-16;

/*o4*/	shlqby		iiBytesL_,		qwAtIi_,			numIiBytesL_;

/*e2*/	selb		numBytesToCopy_,		t__,			_16,			tTooBig_;
/*o4**/	rotqmby		iiBytesR_,		qwAtIiPlus16_,		negNumIiBytesR_;



/*e2*/	a		ii,			ii,			numBytesToCopy_;

/*e2*/	sf		t__,			numBytesToCopy_,		t__;

/*e2**/	or		qwAtOp/*iiBytes16*/,	iiBytesL_,		iiBytesR_;
/*o6*/	lqd		qwAtIi_,			0(ii);

/*e2*/	andi		numIiBytesL_,		ii,			15;
/*o6*/	lqd		qwAtIiPlus16_,		16(ii);

/*e2*/	clgti/*4NextLoop*/ tTooBig_,		t__,			16;
/*o1**/	stqd					qwAtOp,			0(op);

/*e2**/	a		op,			op,			numBytesToCopy_;
_label20LoopFrom:
/*o?**/	brnz/*frequent!*/	t__,			_label20Loop;
/*========================================================================================================*/

_label22:
/*o4**/	gbb		neqBits,		mposBytes16NeqIpBytes16;







/*e2**/	ori		neqBits,		neqBits,		0x7f;



/*e2**/	clz		numEqBytesPlus19,	neqBits;



/*e2**/	ai		deltaIp,		numEqBytesPlus19,	-19+3+1;

/*e2*/	ceqi		numEqBytesEq6,		numEqBytesPlus19,	19+6;

/*e2**/	a		ip_,			ip,			deltaIp;



/*e2**/	a		ip,			ip_,			numEqBytesEq6;



/*o6**/	lqd		qwAtIp,			0(ip);

/*e2*/	andi		numIpBytesL,		ip,			15;
/*o6**/	lqd		qwAtIpPlus16,		16(ip);

	nop $45;
/*o?**/	brnz/*infrequent!*/ numEqBytesEq6,	_label28;

/*------------------------------------*/

/*e2*/    clgt		moffGt0x800_ /*F if 1st*/,	m_off__,			_0x800;
	lnop;

/*e2**/	clgt		moffGt0x4000/*T if 3rd*/, m_off__,			_0x4000;            
/*o6*/	lqx		qwAtIp,			ip,			_neg1;

/*e2*/	ai		ip,			ip,			-1;

/*e2**/	selb		deltaMoff,		_1,			_0x4000,		moffGt0x4000;

/*e2*/	sf		m_len,			ii,			ip;

/*e2**/	sf		m_off___,			deltaMoff,		m_off__;

/*e2*/    ai		mlenLess1,		m_len,			-1;

/*e2*/	ai		mlenLess2,		m_len,			-2;
    
/*e2**/	and		moffAnd4000,		m_off___,			_0x4000;

/*e2**/	andi		moffAnd7,		m_off___,			7;

/*e4*/	shli		mlenLess1Shl5,		mlenLess1,		5;

/*e4**/	rotmi		moffAnd4000Shr11,	moffAnd4000,		-11;

/*e4**/	shli		moffAnd7Shl2,		moffAnd7,		2;

/*e2*/	selb		gotoAdrs___,		label40,		m3_m4_offset,		moffGt0x800_;

/*e2*/	ori		opRvalIf2nd,		mlenLess2,		32;
	lnop;

/*e2**/	ori		moffAnd4000Shr11Or16,	moffAnd4000Shr11,	16;
/*o15*/	hbr		_gotoAdrsFrom,		gotoAdrs___;

/*e2**/	or		opRvalIf1st,		mlenLess1Shl5,		moffAnd7Shl2;

/*e2**/	or		opRvalIf3rd,		moffAnd4000Shr11Or16,	mlenLess2;

/*e2**/	selb		opRval,			opRvalIf1st,		opRvalIf2nd,		moffGt0x800_;
/*o4*/	cbd		shufMaskOp__,		0(op);



/*e2**/	selb		opRval_,			opRval,			opRvalIf3rd,		moffGt0x4000;



/*o4**/	shufb		qwAtOp,			opRval_,			qwAtOp,			shufMaskOp__;

/*e4*/	rotmi		moffShr3,		m_off___,			-3;
/*o4*/	cbx		shufMaskOp1,		op,			_1;



/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o1**/	stqd					qwAtOp,			0(op);

/*e2**/	ai		op,			op,			1;
/*o4**/	shufb		qwAtOp,			moffShr3,		qwAtOp,			shufMaskOp1;



/*e2**/	selb		op1Lval,		op,			safeStoreAdrs,		moffGt0x800_;

/*e2**/	ai		op,			op,			1;

/*e2*/	andi		numIpBytesL,		ip,			15;
/*o1**/	stqd					qwAtOp,			0(op1Lval);

/*e2**/	a		op,			op,			moffGt0x800_;
_gotoAdrsFrom:	
/*o?**/	bi		gotoAdrs___;

.align 6
	
_label28:
/*e2*/	ai		m,			m_pos_,			9;
/*oe4*/	shlqbyi/*copy*/	end,			in_end,			0;
sf		maxIpBytes,		ip,			in_end;/*end*/
/*o6*/	lqd		qwAtM,			0(m);



/*e2*/	cgti/*arithmetic*/ maxIpBytesGt0,		maxIpBytes,		0;
/*o6*/	lqd		qwAtMPlus16,		16(m);

/*e2*/	andi		numMBytesL,		m,			15;

/*o?*/	brz/*infrequentOrTooSoon!*/ maxIpBytesGt0,_label30;

_label29Loop: /* 16 bytes per loop*/
/*e2*/	ai		negNumIpBytesR_,		numIpBytesL,		-16;
/*o4*/	shlqby		ipBytesL,		qwAtIp,			numIpBytesL;

/*e2*/	ai		negNumMBytesR,		numMBytesL,		-16;
/*o4*/	shlqby		mBytesL,		qwAtM,			numMBytesL;

/*o4*/	rotqmby		ipBytesR_,		qwAtIpPlus16,		negNumIpBytesR_;

/*o4*/	rotqmby		mBytesR,		qwAtMPlus16,		negNumMBytesR;

	nop $127;
/*o15*/	hbrr		_label29LoopFrom,	_label29Loop;



/*e2*/	or		ipBytes16_,		ipBytesL,		ipBytesR_;

/*e2*/	or		mBytes16,		mBytesL,		mBytesR;



/*e2*/	ceqb		mBytes16EqIpBytes16,	mBytes16,		ipBytes16_;



/*e2*/	xori		mBytes16NeIpBytes16,	mBytes16EqIpBytes16,	-1;



/*o4*/	gbb		mBytes16NeIpBytes16,	mBytes16NeIpBytes16;







/*e2*/	clz		numEqualBytesPlus16,	mBytes16NeIpBytes16;



/*e2*/	ai		numEqualBytes/*0:16*/,	numEqualBytesPlus16,	-16;



/*e2*/	clgt		maxIpBytesTooBig,	maxIpBytes,		numEqualBytes;



/*e2*/	selb		numLoopsToDo,		maxIpBytes,		numEqualBytes,		maxIpBytesTooBig;

/*e2*/	ceqi		numEqualBytesEq16,	numEqualBytes,		16;

/*e2*/	a		m,			m,			numLoopsToDo;

/*e2*/	and		doLoop,			numEqualBytesEq16,	maxIpBytesTooBig;

/*e2*/	a		ip,			ip,			numLoopsToDo;
/*o6*/	lqd		qwAtM,			0(m);

/*e2*/	sf		maxIpBytes,		numLoopsToDo,		maxIpBytes;
/*o6*/	lqd		qwAtMPlus16,		16(m);

/*e2*/	andi		numMBytesL,		m,			15;
/*o6*/	lqd		qwAtIp,			0(ip);

/*e2*/	andi		numIpBytesL,		ip,			15;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

	nop $49;
_label29LoopFrom:
/*o?*/	brnz/*frequent!*/ doLoop,			_label29Loop;

_label30:
/*e2**/	sf		m_len,			ii,			ip;
/*o4*/	shufb		m_off____,			m_off__,			m_off__,			_0x00010203;



/*o4**/	shufb		m_len,			m_len,			m_len,			_0x00010203;



/*e2*/	clgt		moffGt0x4000,		m_off____,			_0x4000;

/*e2*/	sf		moffLess0x4000,		_0x4000,		m_off____;

/*e2**/	clgti		mlenGt33,		m_len,			33;

/*e2*/	and		moffLess0x4000And0x4000,moffLess0x4000,		_0x4000;

/*e2**/	clgti		mlenGt9,		m_len,			9;

/*e2**/	selb		label33To37ValuesIf33Or34, label33To37ValuesIf33_, label33To37ValuesIf34_, mlenGt33;

/*e2**/	selb		label33To37ValuesIf36Or37, label33To37ValuesIf36_, label33To37ValuesIf37_, mlenGt9;

/*e4*/	rotmi		moffLess0x4000And0x4000Shr11,moffLess0x4000And0x4000,-11;

/*e2**/	selb		label33To37Values,	label33To37ValuesIf33Or34, label33To37ValuesIf36Or37, moffGt0x4000;



/*e2*/	ai		mlenLess2_,		m_len,			-2;
/*o15*/	hbr		_gotoAdrsFrom2,		label33To37Values;

/*e2*/	ori		opRvalIf37,		moffLess0x4000And0x4000Shr11,16;

/*e2*/	ori		opRvalIf33,		mlenLess2_,		32;

/*e2*/	or		opRvalIf36,		opRvalIf37,		mlenLess2_;
	lnop;

/*e2*/	selb		opRvalIf33Or34,		opRvalIf33,		_32,			mlenGt33;
/*o4*/	shlqbyi		moffDelta,		label33To37Values,	4;

/*e2*/	selb		opRvalIf36Or37,		opRvalIf36,		opRvalIf37,		mlenGt9;
/*o4*/	cbd		shufMaskOp_,		0(op);



/*e2*/	selb		opRval__,			opRvalIf33Or34,		opRvalIf36Or37,		moffGt0x4000;

/*o4*/	shlqbyi		mlenDelta,		moffDelta,		4;

/*o4*/	shufb		qwAtOp,			opRval__,			qwAtOp,			shufMaskOp_;

/*e2*/	sf		m_off___,			moffDelta,		m_off____;



/*e2*/	sf		m_len,			mlenDelta,		m_len;

/*o1*/	stqd					qwAtOp,			0(op);

/*e2*/	ai		op,			op,			1;
_gotoAdrsFrom2:
/*o1*/	bi/*predicted*/	label33To37Values;

.align 6

_m3_m4_len:
/*e2*/	clgti		mlenGt255,		m_len,			255;
/*o4*/	cbd/*4CodeAtLabel39*/ shufMaskOp___,		0(op);

/*e2*/	ai/*4CodeBelowBrz*/ mlenLess1,		m_len,			-1;

/*o?*/	brz/*infrequentOrTooSoon!*/ mlenGt255,	_label39;
/*------------------------------------*/

/*o7*/	mpyu		numLoops_,		mlenLess1,		_32897;













/*e4*/	rotmi		numLoops_,		numLoops_,		-23;
/*---------------------------------------------------*/

_label38Loop:
/*e2*/	andi		numOpBytesL__,		op,			15;



/*e2*/	sfi		negNumOpBytesL_,		numOpBytesL__,		0;

/*e2*/	sfi		numOpBytesR__,		numOpBytesL__,		16;

/*o4*/	rotqmby		selMaskOp__,		_neg1,			negNumOpBytesL_;

/*e2*/	clgt		numLoopsTooBig,		numLoops_,		numOpBytesR__;



/*e2*/	selb		numBytesToOutput_,	numLoops_,		numOpBytesR__,		numLoopsTooBig;

/*e2*/	andc		qwAtOp,			qwAtOp,			selMaskOp__;

/*e4*/	shli		numBytesToOutputShl8_,	numBytesToOutput_,	8;

/*e2*/	a		m_len,			m_len,			numBytesToOutput_;
/*o4**/	cbx/*4CodeAtLabel39*/ shufMaskOp___,		op/*b4Add*/,		numBytesToOutput_; /*ea is 0(op after add)*/

/*e2*/	sf		numLoops_,		numBytesToOutput_,	numLoops_;
/*o1*/	stqd					qwAtOp,			0(op);

/*e2*/	a		op,			op,			numBytesToOutput_;
	lnop;

/*e2*/	sf		m_len,			numBytesToOutputShl8_,	m_len;
/*o?*/	brnz/*~50%!*/	numLoops_,	_label38Loop;
/*-----------------------------------------------------------------------------*/

_label39:
/*eo2*/	ori/*copy*/	op_,			op,			0;
/*o4*/	shufb		qwAtOp,			m_len,			qwAtOp,			shufMaskOp___;

/*e2*/	ai		op,			op,			1;





/*o1*/	stqd					qwAtOp,			0(op_);
/*------------------------------------------------------------------------------*/

_m3_m4_offset:
/*e2**/	andi		moffAnd63,		m_off___,			63;
/*o15*/	hbrr		_label40From,		_label40;



/*e4*/	shli		moffAnd63Shl2,		moffAnd63,		2;
/*o4*/	cbd		shufMaskOp____,		0(op);





/*o4*/	rotmi		moffShr6,		m_off___,			-6;

/*o4*/	shufb		qwAtOp,			moffAnd63Shl2,		qwAtOp,			shufMaskOp____;

/*o4*/	cbx		shufMaskOp_____,		op,			_1;





/*o1*/	stqd					qwAtOp,			0(op);

/*o4*/	shufb		qwAtOp,			moffShr6,		qwAtOp,			shufMaskOp_____;







/*o1*/	stqx					qwAtOp,			op,			_1;

/*e2*/	ai		op,			op,			2;
_label40From:
	br/*100%*/	_label40; /*NOTE: branch added since _label40 moved to before _next*/
/*-----------------------------------------------------------------------------------*/

/*NOTE: _label40 code has been moved (in interleaved version)*/
/*	to just before _next code to speed it up*/

_done:
/*e2*/	sf		atOutLen,		out,			op;

/*e2*/	sf		t/*return value*/,	ii,			in_end;

/*e2*/	a		op,			out,			atOutLen;
/*o6*/	lqx		qwAtOp,			out,			atOutLen;
/*--------------------------------------*/

_label43:
/*e2*/	clgti		tGt0_,			t,			0;



/*o?*/	brz/*assumeTooShortToHint*/ tGt0_,		_label55;
/*---------------------------------------*/

/*e2*/	a		ii_,			in,			in_len;

/*e2*/	ceq		opEqOut,		op,			out;

/*e2*/	clgti		tGt238,			t,			238;

/*e2*/	sf		ii_,			t,			ii_;



/*e2*/	andc		ifTrue,			opEqOut,		tGt238;
/*o6*/	lqd		qwAtIi__,			0(ii_);

	nop $1;
/*o6*/	lqd		qwAtIiPlus16__,		16(ii_);

/*e2*/	andi		numIiBytesL__,		ii_,			15;
/*o?*/	brz/*hintDidntMatter*/ ifTrue,		_label46;
/*-----------------------------------------*/

/*o15*/	hbrr		_label53From,		_label53;

/*o4*/	cbd		shufMaskOp______,		0(op);



/*e2*/	ai		tPlus17,		t,			17;



/*o4*/	shufb		qwAtOp_,			tPlus17,		qwAtOp,			shufMaskOp______;







	nop $47;
/*o1*/	stqd					qwAtOp_,			0(op);

/*e2*/	ai		op__,			op,			1;



	shli/*delay*/	op__,			op__,			0;







	ai/*delay*/	op,			op__,			0;
_label53From:	
/*o1*/	br/*100%*/	_label53;
/*----------------------------------------*/

.align 6

_label46:
/*e2*/	clgti		tGt3_,			t,			3;



/*o?*/	brnz/*assumeTooShortToHint*/ tGt3_,	_label48;
/*----------------------------------------*/

/*o6*/	lqx		qwAtOpLess2_,		op,			_neg2;

/*o15*/	hbrr		_label53From2,		_label53;





/*e2*/	ai		opLess5_,		op,			-2-3;



/*o4*/	rotqby		atOpLess2_/*in byte3*/,	qwAtOpLess2_,		opLess5_;



/*o4*/	cbx		shufMaskOpLess2_,	op,			_neg2;



/*e2*/	or		rval_/*in byte3*/,		atOpLess2_,		t;



/*o4*/	shufb		qwAtOpLess2_,		rval_,			qwAtOpLess2_,		shufMaskOpLess2_;	







/*o1*/	stqx/*b4 lqd*/				qwAtOpLess2_,		op,			_neg2;

/*o6*/	lqd/*after stqx*/	qwAtOp_,			0(op);

_label53From2:	
/*o1*/	br/*100%*/	_label53;
/*-------------------------------------------*/

.align 6

_label48:
/*e2*/	clgti		tGt18_,			t,			18;



/*o?*/	brnz/*?*/		tGt18_,			_label50;
/*------------------------------------------*/

/*o15*/	hbrr		_label53From3,		_label53;

/*o4*/	cbd		shufMaskOp______,		0(op);



/*e2*/	ai		tLess3_,			t,			-3;



/*o4*/	shufb		qwAtOp_,			tLess3_,			qwAtOp,			shufMaskOp______;







	nop $47;
/*o1*/	stqd					qwAtOp_,			0(op);

/*e2*/	ai		op__,			op,			1;



	shli/*delay*/	op__,			op__,			0;







	ai/*delay*/	op,			op__,			0;
_label53From3:
/*o1*/	br/*100%*/	_label53;
/*-------------------------------------------*/
	
.align 6
	
_label50:
/*o4*/	cbd		shufMaskOp______,		0(op);







/*o4*/	shufb		qwAtOp_,			_0,			qwAtOp,			shufMaskOp______;

/*e2*/	ai		tt_,			t,			-18;



/*e2*/	clgti		ttGt255,		tt_,			255;

	nop $47;
/*o1*/	stqd					qwAtOp_,			0(op);

/*e2*/	ai		op___,			op,			1;
/*o?*/	brz/*?*/		ttGt255,		_label52;
/*-------------------------------------------*/

/*e2*/	ai		ttLess1_,		tt_,			-1;



/*o7*/	mpyu		numLoops__,		ttLess1_,		_32897;













/*e4*/	rotmi		numLoops__,		numLoops__,		-23;
	lnop;
/*-----------------------------------------------*/

_label51Loop:
/*e2*/	andi		numOpBytesL___,		op___,			15;



/*e2*/	sfi		negNumOpBytesL__,		numOpBytesL___,		0;

/*e2*/	sfi		numOpBytesR___,		numOpBytesL___,		16;

/*o4*/	rotqmby		selMaskOp___,		_neg1,			negNumOpBytesL__;

/*e2*/	clgt		numLoopsTooBig_,		numLoops__,		numOpBytesR___;



/*e2*/	selb		numBytesToOutput__,	numLoops__,		numOpBytesR___,		numLoopsTooBig_;

/*e2*/	andc		qwAtOp_,			qwAtOp_,			selMaskOp___;

/*e4*/	shli		numBytesToOutputShl8__,	numBytesToOutput__,	8;

/*e2*/	a		tt_,			tt_,			numBytesToOutput__;
	lnop;

/*e2*/	sf		numLoops__,		numBytesToOutput__,	numLoops__;
/*o1*/	stqd					qwAtOp_,			0(op___);

/*e2*/	a		op___,			op___,			numBytesToOutput__;
	lnop;

/*e2*/	sf		tt_,			numBytesToOutputShl8__,	tt_;
/*o?*/	brnz/*hintDoesntMatter!*/ numLoops__,	_label51Loop;
/*---------------------------------------*/

_label52:
/*o4*/	cbd		shufMaskOp_______,		0(op___);







/*o4*/	shufb		qwAtOp_,			tt_,			qwAtOp_,			shufMaskOp_______;

/*o1*/	stqd					qwAtOp_,			0(op___);

/*e2*/	ai		op,			op___,			1;

_label53:
/*e2*/	ai		negNumIiBytesR_,		numIiBytesL__,		-16;
	lnop;
	
/*e2*/	andi		numOpBytesL____,		op,			15;	
/*o4*/	shlqby		iiBytesL__,		qwAtIi__,			numIiBytesL__;

/*o4*/	rotqmby		iiBytesR_,		qwAtIiPlus16__,		negNumIiBytesR_;

/*e2*/	sfi		numOpBytesR____,		numOpBytesL____,		16;

/*e2*/	sfi		negNumOpBytesL___,		numOpBytesL____,		0;

/*e2*/	clgt		tBeyondOpQw,		t,			numOpBytesR____;

/*e2*/	or		iiBytes16_,		iiBytesL__,		iiBytesR_;

/*o4*/	rotqmby		selMaskOp____,		_neg1,			negNumOpBytesL___;

/*e2*/	selb		numBytesToCopy__/*1:16*/,	t,			numOpBytesR____,		tBeyondOpQw;
/*o4*/	rotqmby		iiBytes16Shr_,		iiBytes16_,		negNumOpBytesL___;



/*e2*/	a		ii_,			ii_,			numBytesToCopy__;
	lnop;



/*e2*/	selb		qwAtOp,			qwAtOp_,			iiBytes16Shr_,		selMaskOp____;
/*o6*/	lqd		qwAtIi___,			0(ii_);

/*e2*/	sf		t___,			numBytesToCopy__,		t;
/*o6*/	lqd		qwAtIiPlus16___,		16(ii_);

/*e2*/	andi		numIiBytesL___,		ii_,			15;
/*o1*/	stqd					qwAtOp,			0(op);

/*e2*/	a		op,			op,			numBytesToCopy__;
/*o?*/	brz/*hintDidntMatter*/ tBeyondOpQw,	_label55;	
/*---------------------------------------*/
	
/*e2*/	ai		negNumIiBytesR__,		numIiBytesL___,		-16;
	lnop;
/*---------------------------------------*/

_label54:
/*e2*/	clgti		tBeyondOpQw,		t___,			16;

/*o4*/	shlqby		iiBytesL___,		qwAtIi___,			numIiBytesL___;

/*e2*/	selb		numBytesToCopy___/*1:16*/,	t___,			_16,			tBeyondOpQw;
/*o4*/	rotqmby		iiBytesR_,		qwAtIiPlus16___,		negNumIiBytesR__;



/*e2*/	a		ii_,			ii_,			numBytesToCopy___;
	lnop;



/*e2*/	or		qwAtOp/*iiBytes16*/,	iiBytesL___,		iiBytesR_;
/*o6*/	lqd		qwAtIi___,			0(ii_);

/*e2*/	sf		t___,			numBytesToCopy___,		t___;
/*o6*/	lqd		qwAtIiPlus16___,		16(ii_);

/*e2*/	andi		numIiBytesL___,		ii_,			15;
/*o1*/	stqd					qwAtOp,			0(op);

/*e2*/	a		op,			op,			numBytesToCopy___;
/*o?*/	brnz/*hintDidntMatter*/ tBeyondOpQw,	_label54;	
/*--------------------------------------*/

_label55:
	nop $44;
/*o4*/	cbd		shufMaskOp________,		0(op);

/*o15*/	hbr		_returnAdrsFrom,	$lr;/*return adrs*/





/*o4*/	shufb		qwAtOp__,			_17,			qwAtOp,			shufMaskOp________;

/*o4*/	cbx		shufMaskOpPlus1,	op,			_1;





/*o1*/	stqd					qwAtOp__,			0(op);

/*e2*/	ila		gOutLen_,		_gOutLen;
/*o4*/	shufb		qwAtOp__,			_0,			qwAtOp__,			shufMaskOpPlus1;

/*o4*/	cbx		shufMaskOpPlus2,	op,			_2;

/*o6*/	lqx		out_len,		picOffset,		gOutLen_;



/*o1*/	stqx					qwAtOp__,			op,			_1;

/*o4*/	shufb		qwAtOp__,			_0,			qwAtOp__,			shufMaskOpPlus2;





/*o6*/	lqd		qwAtOutLen,		0(out_len);

/*o1*/	stqx					qwAtOp__,			op,			_2;

/*e2*/	ai		op____,			op,			3;
/*o4*/	cwd		shufMaskOutLen,		0(out_len);



/*e2*/	sf		opLessOut,		out,			op____;



/*o4*/	shufb		qwAtOutLen_,		opLessOut,		qwAtOutLen,		shufMaskOutLen;







/*e2*/	il		$3/*return val*/,		0;
/*o1*/	stqd					qwAtOutLen_,		0(out_len);

	nop $127;
_returnAdrsFrom:
/*o?*/	bi		$lr;/*return adrs*/





/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	uninitialized data*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/

.section .bss
.align 5	/* must be >= 4*/

.global	_gOutLen
_gOutLen:
	.word	0,0,0,0

.global	_gSafeStoreAdrs
_gSafeStoreAdrs:
	.word	0,0,0,0



.section .rodata
.align 5	/* must be >= 4*/


