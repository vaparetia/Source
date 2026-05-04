.set	INTERLEAVE_CODE	, 1







/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	uninitialized data*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/







.equ opEndLess1, 8
.equ opEndLess2, 26
.equ opEndLess3, 6
.equ opGtOpEndLess2, 25
.equ opGtOpEndLess3, 28
.equ opGtOpEndQword, 21
.equ opEndQword, 8
.equ op_, 18
.equ opGtOpEnd, 27
.equ opNumBytesRightGt2, 27
.equ secondQwordOfst, 27
.equ _1, 9
.equ atOutLen, 21
.equ _0x4000, 17
.equ mask_00BA_00BA_00BA_00BA, 16
.equ picLoc, 18
.equ picLabel, 2
.equ picOffset, 2
.equ in, 3
.equ in_len, 4
.equ out, 7
.equ gOutLen, 8
.equ neg1, 10
.equ _16, 13
.equ ip_end, 3
.equ op_end, 5
.equ op, 22
.equ qwAtOp, 23
.equ ip, 11
.equ qwAtIp, 12
.equ qwAtIpPlus16, 24
.equ ipNumBytesLeft, 14
.equ ipNumBytesRight, 14
.equ ipBytes, 12
.equ atIp, 18
.equ atIpGt17, 25
.equ t, 4
.equ tGt3, 18
.equ ipEndGtIp, 18
.equ tGt15, 18
.equ negIpNumBytesRight, 14
.equ ipNextBytes, 14
.equ ipBytes16, 12
.equ ipBytesLead0s, 12
.equ num0Bytes, 12
.equ num0BytesEq4, 14
.equ num0BytesShl, 18
.equ deltaT, 18
.equ num0BytesEq16, 27
.equ ipBytesShr, 12
.equ ipToOpMask, 14
.equ opNumBytesLeft, 18
.equ opNumBytesRight, 24
.equ tGtOpNumBytesRight, 21
.equ numBytesCopied, 21
.equ tGt16, 18
.equ m_pos, 18
.equ _0x801, 15
.equ tShr2, 27
.equ atIpShl2, 25
.equ qwAtMpos, 21
.equ qwAtMposPlus16, 27
.equ mposNumBytesLeft, 4
.equ leftT, 18
.equ rightF, 25
.equ doBranch, 18
.equ mposNumBytesRight, 25
.equ mposBytes, 4
.equ negMposNumBytesRight, 18
.equ mposNextBytes, 18
.equ mposBytes16, 4
.equ negOpNumBytesLeft, 18
.equ mposBytesShr, 25
.equ mposToOpMask, 29
.equ mposBytesShl, 4
.equ tGt31, 21
.equ tGt63, 25
.equ tShr2And7, 18
.equ atIpShl3, 21
.equ tShr5, 4
.equ deltaMpos, 27
.equ mposEqOp, 21
.equ atMpos, 18
.equ opMask, 21
.equ tTooBig, 27
.equ numBytesToDup, 31
.equ dupLen, 28
.equ dupLenGt16, 30
.equ dupLenShl4, 28
.equ dupShufMask, 28
.equ dupMask, 29
.equ gDupTbl, 19
.equ dupBytes, 21
.equ dupBytesShr, 21
.equ qwAtIpLess2, 18
.equ _neg2, 20
.equ ipLess5, 4
.equ atIpLess2, 4
.equ copyMask, 14
.equ ipBytesShl, 18
.equ tGt1, 21
.equ tGt2, 4
.equ numBytesToCopy, 4
.equ ipEqIpEnd, 5
.equ outLen, 5
.equ out_len, 2
.equ qwAtOutLen, 4
.equ outLenMask, 6
.equ atOutLen_, 6
.equ num0BytesShl_, 14
.equ ipNumBytesLeft_, 21
.equ qwAtIp_, 25
.equ deltaT_, 12
.equ ipNumBytesLeft__, 18
.equ opGtOpEndQword_, 12
.equ tGt15_, 21
.equ atIpShl2_, 4
.equ m_pos_, 4
.equ opNumBytesLeft_, 28
.equ opNumBytesRight_, 21
.equ negOpNumBytesLeft_, 25
.equ qwAtOp_, 21
.equ tShr2_, 18
.equ m_pos__, 25
.equ mposNumBytesLeft_, 21
.equ qwAtMpos_, 25
.equ deltaT__, 14
.equ num0BytesEq16_, 18
.equ ipNumBytesRight_, 27
.equ negIpNumBytesRight_, 12
.equ qwAtIp__, 21
.equ deltaMpos_, 18
.equ ipNextBytes_, 21
.equ num0BytesShl__, 21
.equ ipNumBytesLeft___, 25
.equ qwAtIp___, 27
.equ ipNumBytesRight__, 28
.equ negIpNumBytesRight__, 21
.equ num0BytesEq16__, 14
.equ atIp_, 21
.equ deltaMpos__, 21
.equ ipEndGtIp_, 4
.equ opGtOpEndQword__, 29
.equ mposNumBytesRight_, 29
.equ negMposNumBytesRight_, 29
.equ mposBytes_, 21
.equ leftT_, 25
.equ mposNextBytes_, 27
.equ rightF_, 29
.equ opNumBytesLeft__, 30
.equ opNumBytesRight__, 31
.equ mposBytes16_, 21
.equ negOpNumBytesLeft__, 30
.equ doBranch_, 25
.equ mposNumBytesLeft__, 30
.equ mposNumBytesRight__, 21
.equ qwAtMposPlus16_, 29
.equ negMposNumBytesRight__, 21
.equ mposBytes__, 23
.equ opGtOpEndQword___, 25
.equ mposNextBytes__, 21
.equ numBytesToDup_, 29
.equ tShr2__, 4
.equ atIpShl2__, 18
.equ rightF__, 27
.equ mposNumBytesLeft___, 28
.equ mposBytes___, 18
.equ mposNumBytesLeft____, 25
.equ qwAtOp__, 18
.equ qwAtMpos__, 4
.equ atMpos_, 4
.equ op__, 25
.equ negOpNumBytesLeft___, 27
.equ opNumBytesRight___, 18
.equ opNumBytesRightGt2_, 21
.equ ipBytesShr_, 24
.equ ipBytes16_, 4
.equ secondQwordOfst_, 12
.equ qwAtOp___, 14
.equ gOutLen_, 4
.global lzo1x_decompress_safe_asm
.extern _gOutLen
.extern	_gDupTbl

.text

.align 7

/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	start of lzo1x_decompress_safe*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/

lzo1x_decompress_safe_asm:

/*e2*/	ila		picLabel,		_picLabel;			
	hbrr		_copyTBytesFrom,	_copyTBytes;

/*e2!*/	ori		in,			$3,			0;

/*e2*/	ori		in_len,			$4,			0;

/*e2*/	ori		out,			$5,			0;

/*e2*/	ila		gOutLen,		_gOutLen;

/*e2*/	il		_1,			1;

/*e2*/	il		neg1,			-1;

/*e2!*/	ori		ip,			in,			0;
/*o6!*/	lqd		qwAtIp,			0(in/*ip*/);

/*e2*/	il		_16,			16;

/*e2!*/	andi		ipNumBytesLeft,		ip,			15;

/*e2*/	il		_0x801,			0x801;

/*e2*/	ilhu		mask_00BA_00BA_00BA_00BA, 0x8080;

/*e2*/	il		_0x4000,		0x4000;
/*o4??*/	brsl		picLoc,			_picLabel;
_picLabel:

/*e2*/	ila		gDupTbl,		_gDupTbl;
/*o4!*/	shlqby		ipBytes,		qwAtIp,			ipNumBytesLeft;

/*e2*/	il		_neg2,			-2;
/*o6*/	lqd		atOutLen,		0($6/*out_len*/);

/*e2*/	ori		op,			$5/*out*/,		0;

/*e2*/	sf		picOffset,		picLabel,		picLoc;		

/*eo4!*/	rotmi		atIp,			ipBytes,		-24;

/*e2*/	iohl		mask_00BA_00BA_00BA_00BA, 0x0100;

/*e2*/	sfi		ipNumBytesRight,	ipNumBytesLeft,		16;
/*o6*/	lqd		qwAtOp,			0(op); /*THIS LINE NEEDED IFF OP CAN BE OFF QWORD BOUND*/

/*e2*/	a		op_end,			$5/*out*/,		atOutLen;
/*o6*/	lqd		qwAtIpPlus16,		16(in/*ip*/);

/*e2!*/	clgti		atIpGt17,		atIp,			17;
/*o1*/	stqx		$6,			picOffset,		gOutLen;

/*e2*/	ai		opEndLess1,		op_end,			-1;

/*e2*/	ai		opEndLess2,		op_end,			-2;

/*e2*/	a		gDupTbl,		gDupTbl,		picOffset;
/*o4*/	rotqby		atOutLen_,		atOutLen,		$6;/*out_len*/

/*e2*/	ai		opEndLess3,		op_end,			-3;
	
/*e2*/	ori		opEndQword,		opEndLess1,		15;

/*e2*/	a		ip_end,			in,			in_len;
/*o?!*/	brz/*?*/		atIpGt17,		_mainLoop;	

/*e2!*/	ai		t,			atIp,			-17;
/*o4*/	shlqbyi/*b4 brz*/	ipBytes,		ipBytes,		1;

/*e2*/	ai/*b4 brz*/	ipNumBytesRight,	ipNumBytesRight,	-1;

/*e2!*/	clgti		tGt3,			t,			3;

/*e2*/	ai/*b4 brz*/	ip,			ip,			1;
/*o?*/	brz/*1Per16,After3Above*/ ipNumBytesRight, _reloadIp;
_reloadIpRet:	

	/*brz below on even adrs*/
/*o?!*/	brz/*?*/		tGt3,			_match_next;

_copyTBytesFrom:
/*o?!*/	br/*100%*/	_copyTBytes;

.align 7

_mainLoop:
/*eo4*/	rotmi		t,			ipBytes,		-24;

/*e2*/	clgt		ipEndGtIp,		ip_end,			ip;

/*e2*/	ai		ip,			ip,			1;
/*o4*/	shlqbyi		ipBytes,		ipBytes,		1;

/*e2*/	ai		ipNumBytesRight,	ipNumBytesRight,	-1;
/*o1*/	brz/*iff error*/	ipEndGtIp,		_ipEndLeIp; /*NOTE: this branch only done if error*/

/*e2*/	clgti		tGt15,			t,			15;

/*o?*/	brz/*1 per 16*/	ipNumBytesRight,	_reloadIp2;
_reloadIpRet2:	

	/*brnz below is on even word*/
/*o?*/	brnz/*?*/		tGt15,			_match;

/*o?*/	brnz/*?*/		t,			_tNe0;

_whileAtIpEq0Loop: /* ======================================================================*/
/*e2*/	sfi		negIpNumBytesRight,	ipNumBytesRight,		0;
	lnop;



/*o4*/	rotqmby		ipNextBytes,		qwAtIpPlus16,		negIpNumBytesRight;







/*e2*/	or		ipBytes16,		ipBytes,		ipNextBytes;


 
/*e2*/	clz		ipBytesLead0s/*per word*/,ipBytes16;



/*e4*/	rotmi		num0Bytes/*per word*/,	ipBytesLead0s,		-3;
	






/*e2*/	ceqi		num0BytesEq4,		num0Bytes,		4;
/*o4*/	shlqbyi		num0BytesShl,		num0Bytes,		4;







/*e2*/	and		num0BytesShl,		num0BytesShl,		num0BytesEq4;
	lnop;
	


/*e2*/	a		num0Bytes,		num0Bytes,		num0BytesShl;
/*o4*/	shlqbyi		num0BytesShl,		num0BytesShl,		4;







/*e2*/	and		num0BytesShl,		num0BytesShl,		num0BytesEq4;
	lnop;
	


/*e2*/	a		num0Bytes,		num0Bytes,		num0BytesShl;
/*o4*/	shlqbyi		num0BytesShl,		num0BytesShl,		4;







/*e2*/	and		num0BytesShl_,		num0BytesShl,		num0BytesEq4;



/*e2*/	a		num0Bytes/*0:16*/,	num0Bytes,		num0BytesShl_;



/*e2*/	a		ip,			ip,			num0Bytes;

/*e4*/	shli		deltaT,			num0Bytes,		8;

/*e2*/	andi		ipNumBytesLeft_,		ip,			15;
/*o6*/	lqd		qwAtIp_,			0(ip);

/*e2*/	ceqi		num0BytesEq16,		num0Bytes,		16;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*e2*/	sfi		ipNumBytesRight,	ipNumBytesLeft_,		16;

/*e2*/	sf		deltaT_,			num0Bytes,		deltaT;



/*e2*/	a		t,			t,			deltaT_;
	lnop;
	
/*o4*/	shlqby		ipBytes,		qwAtIp_,			ipNumBytesLeft_;

/*Note: hinting this does not seem to help*/
/*o?*/	brnz/*?*/		num0BytesEq16,		_whileAtIpEq0Loop;
/*====================================================================================*/
	
/*eo4*/	rotmi		atIp,			ipBytes,		-24;
/*o4*/	shlqbyi		ipBytes,		ipBytes,		1;

/*e2*/	ai		t,			t,			15;
/*e2*/	ai		ipNumBytesRight,	ipNumBytesRight,	-1;

/*e2*/	ai		ip,			ip,			1;
	lnop;



/*e2*/	a		t,			t,			atIp;
/*o?*/	brz/*1 per 16*/	ipNumBytesRight,	_reloadIp3;
_tNe0:

ai		t,			t,			3;
	lnop;
	
_copyTBytes:
/*e2!*/	sfi		negIpNumBytesRight,	ipNumBytesRight,	0;
	
/*e2*/	andi		opNumBytesLeft,		op,			15;
	
/*e2*/	clgt		opGtOpEndQword,		op,			opEndQword;
/*o4!*/	rotqmby		ipNextBytes,		qwAtIpPlus16,		negIpNumBytesRight;

/*e2*/	sfi		opNumBytesRight,	opNumBytesLeft,		16;
	lnop;
	
/*e2*/	sfi		negOpNumBytesLeft,	opNumBytesLeft,		0;
/*o1!*/	brnz/*iff error*/	opGtOpEndQword,		_output_overrun;

/*e2*/	clgt		tGtOpNumBytesRight,	t,			opNumBytesRight;
	lnop;
	
/*e2!*/	or		ipBytes16,		ipBytes,		ipNextBytes;
/*o4*/	rotqmby		ipToOpMask,		neg1,			negOpNumBytesLeft;

/*e2*/	selb		numBytesCopied,		t,			opNumBytesRight,		tGtOpNumBytesRight;
	
/*o4!*/	rotqmby		ipBytesShr,		ipBytes16,		negOpNumBytesLeft;

/*e2*/	a		ip,			ip,			numBytesCopied;

/*e2*/	sf		t,			numBytesCopied,		t;

/*e2*/	andi		ipNumBytesLeft__,		ip,			15;
/*o6*/	lqd		qwAtIp_,			0(ip);

/*e2!*/	selb		qwAtOp,			qwAtOp,			ipBytesShr,		ipToOpMask;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*e2*/	sfi		ipNumBytesRight,	ipNumBytesLeft__,		16;

/*o1!*/	stqd		qwAtOp,			0(op);

/*e2!*/	a		op,			op,			numBytesCopied;

/*o15*/	hbrr		_copyTBytesLoop_from,	_copyTBytesLoop;

/*o4*/	shlqby		ipBytes,		qwAtIp_,			ipNumBytesLeft__;

/*Note: hinting this does not help*/
/*o?!*/	brz/*?*/		t,			_first_literal_run;

_copyTBytesLoop: /*===================================================================================================*/
/*e2*/	clgt		opGtOpEndQword_,		op,			opEndQword;

/*--------------------------------------------------------------------------------------------------------------------*/
/*e2*/	sfi		negIpNumBytesRight,	ipNumBytesRight,	0;

/*--------------------------------------------------------------------------------------------------------------------*/
	nop $11;
/*o1*/	brnz/*iff error*/	opGtOpEndQword_,		_output_overrun;
/*--------------------------------------------------------------------------------------------------------------------*/

 /*o4!*/	shlqby		ipBytes,		qwAtIp_,			ipNumBytesLeft__; /*this line from previous loop*/
/*--------------------------------------------------------------------------------------------------------------------*/

/*o4!*/	rotqmby		ipNextBytes,		qwAtIpPlus16,		negIpNumBytesRight;
/*--------------------------------------------------------------------------------------------------------------------*/
/*e2*/	clgti		tGt16,			t,			16;

/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/*e2*/	selb		numBytesCopied,		t,			_16,			tGt16;

/*--------------------------------------------------------------------------------------------------------------------*/
/*e2!*/	or		qwAtOp/*ipBytes16*/,	ipBytes,		ipNextBytes;

/*--------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		ip,			ip,			numBytesCopied;

/*--------------------------------------------------------------------------------------------------------------------*/
/*e2*/	sf		t,			numBytesCopied,		t;
/*o1!*/	stqd		qwAtOp,			0(op);
/*--------------------------------------------------------------------------------------------------------------------*/
/*e2*/	andi		ipNumBytesLeft__,		ip,			15;
/*o6!*/	lqd		qwAtIp_,			0(ip);
/*--------------------------------------------------------------------------------------------------------------------*/
/*e2*/	a		op,			op,			numBytesCopied;
/*o6!*/	lqd		qwAtIpPlus16,		16(ip);
/*--------------------------------------------------------------------------------------------------------------------*/
/*e2*/	sfi		ipNumBytesRight,	ipNumBytesLeft__,		16;
_copyTBytesLoop_from:
/*o?*/	brnz		t,			_copyTBytesLoop;
/*====================================================================================================================*/
	nop $42;

/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/

/*o4!*/	shlqby		ipBytes,		qwAtIp_,			ipNumBytesLeft__;	/*this line from previous loop*/
/*--------------------------------------------------------------------------------------------------------------------*/

_first_literal_run:

/*NOTE: if code is fallen into, there are 3 lost cycles for ipBytes,*/
/*	but if code is branched to, there are no lost cycles for ipBytes*/

/*{only if fallen into}*/

/*{only if fallen into}*/

/*{only if fallen into}*/

/*eo4!*/	rotmi		t,			ipBytes,		-24;
/*o4*/	shlqbyi/*br brz*/	ipBytes,		ipBytes,		1;

/*e2*/	ai/*b4 brz*/	ipNumBytesRight,	ipNumBytesRight,	-1;

/*e2*/	ai/*b4 brz*/	ip,			ip,			1;

/*e2*/	sf		m_pos,			_0x801,			op;
/*o?*/	brz/*1 per 16*/	ipNumBytesRight,	_reloadIp4;
_reloadIpRet4:

/*e2!*/	clgti		tGt15_,			t,			15;

/*e4*/	rotmi		atIpShl2,		ipBytes,		-22;

/*e4*/	rotmi		tShr2,			t,			-2;
/*o?!*/	brnz/*?*/		tGt15_,			_match; /*note m_pos is ignored if you go to _match*/

/*e2*/	ai/*b4 brz*/	ipNumBytesRight,	ipNumBytesRight,	-1;
/*o4*/	shlqbyi/*b4 brz*/	ipBytes,		ipBytes,		1;

/*e2*/    ai/*b4 brz*/	ip,			ip,			1;
	lnop;

/*e2*/	andi		atIpShl2_,		atIpShl2,		-4;
/*o?*/	brz/*1 per 16*/	ipNumBytesRight,	_reloadIp5;
_reloadIpRet5:

/*e2*/	sf		m_pos,			tShr2,			m_pos;



/*e2*/	sf		m_pos_,			atIpShl2_,		m_pos;



/*e2*/	clgt		leftT,			out,			m_pos_;
/*o6*/	lqd		qwAtMpos,		0(m_pos_);

/*e2*/	clgt		rightF,			op,			m_pos_;
/*o6*/	lqd		qwAtMposPlus16,		16(m_pos_);

/*e2*/	andi		mposNumBytesLeft,	m_pos_,			15;
/*o15*/	hbrr		_gotoMatchDoneFrom,	_match_done;			/*MOVED UP FROM SECTION BELOW!!!*/

/*e2*/	orc		doBranch,		leftT,			rightF;

/*e2!*/	sfi		mposNumBytesRight,	mposNumBytesLeft,	16;	/*MOVED UP FROM SECTION BELOW!!!*/

/*e2*/	andi		opNumBytesLeft_,		op,			15;	/*MOVED UP FROM SECTION BELOW!!!*/
/*o?*/	brnz/*iff error*/	doBranch,		_lookbehind_overrun;		

sfi		negMposNumBytesRight,	mposNumBytesRight,	0;
/*o4*/	shlqby		mposBytes,		qwAtMpos,		mposNumBytesLeft;

/*e2*/	sfi		opNumBytesRight_,	opNumBytesLeft_,		16;
	lnop;

/*e2*/	sfi		negOpNumBytesLeft_,	opNumBytesLeft_,		0;
/*o4!*/	rotqmby		mposNextBytes,		qwAtMposPlus16,		negMposNumBytesRight;

/*e2*/	clgti		opNumBytesRightGt2,	opNumBytesRight_,	2;
	lnop;
	
/*e2*/	clgt		opGtOpEndLess3,		op,			opEndLess3;
/*o4*/	rotqmby		mposToOpMask,		neg1,			negOpNumBytesLeft_;

/*e2*/	andc		secondQwordOfst,	_16,			opNumBytesRightGt2;

/*e2!*/	or		mposBytes16,		mposBytes,		mposNextBytes;

/*e2*/	ori/*copy*/	op_,			op,			0;
/*o1!*/	brnz/*iff error*/	opGtOpEndLess3,		_output_overrun;

/*e2!*/	ai		op,			op,			3;
/*o4!*/	rotqmby		mposBytesShr,		mposBytes16,		negOpNumBytesLeft_;

/*o4*/	shlqby		mposBytesShl,		mposBytes16,		opNumBytesRight_;





/*e2!*/	selb		qwAtOp_,			qwAtOp,			mposBytesShr,		mposToOpMask;

/*o1*/	stqx/*1st of 2*/	mposBytesShl,		op_,			secondQwordOfst;

/*o1!*/	stqd/*2nd of 2*/	qwAtOp_,			0(op_);

/*o6*/	lqd/*after stq's*/qwAtOp,			0(op); /* FETCH QWORD FROM 1 OF 2 STQD'S ABOVE*/

_gotoMatchDoneFrom:
/*o?!*/	br/*100%*/	_match_done;

.align 5

_match:
/*e2*/	clgti		tGt15,			t,			15;
/*o15*/	hbrr		_gotoIfTCodesJoinFrom,	_ifTCodesJoin;

/*e2*/	clgti		tGt31,			t,			31;
	lnop;

/*e2*/	clgti		tGt63,			t,			63;
/*o?*/	brz/*1st of 3*/	tGt15,			_tLe15;

/*e4!*/	rotmi		tShr2_,			t,			-2;
/*o?*/	brz/*2nd of 3*/	tGt31,			_tLe31;

/*e4*/	rotmi		atIpShl3,		ipBytes,		-(24-3);
/*o?*/	brz/*3rd of 3*/	tGt63,			_tLe63;

/*e2*/	ai		m_pos__,			op,			-1;
/*o4*/	shlqbyi/*b4 ret6*/ ipBytes,		ipBytes,		1;

/*e2*/	ai/*b4 ret6*/	ip,			ip,			1;

/*e2*/	ai/*b4 ret6*/	ipNumBytesRight,	ipNumBytesRight,	-1;

/*e2!*/	andi		tShr2And7,		tShr2_,			7;

/*e2*/	andi		atIpShl3,		atIpShl3,		-8;

/*e2!*/	sf		m_pos,			tShr2And7,		m_pos__;

/*e4*/	rotmi		tShr5,			t,			-5;

/*e2!*/	sf		m_pos,			atIpShl3,		m_pos;

/*o?*/	brz/*1 in 16*/	ipNumBytesRight,	_reloadIp6;
_reloadIpRet6:

/*e2*/	andi		mposNumBytesLeft_,	m_pos,			15;
/*o6!*/	lqd		qwAtMpos_,		0(m_pos);

/*e2*/	ai		t,			tShr5,			-1;
/*o6!*/	lqd		qwAtMposPlus16,		16(m_pos);

	nop $127;
_gotoIfTCodesJoinFrom:
/*o?*/	br		_ifTCodesJoin;

.align 5

_tLe63:
/*e2*/	andi		t,			t,			31;

/*e2!*/	sfi		negIpNumBytesRight,	ipNumBytesRight,	0;

	/*brnz below at even word adrs*/
/*o?*/	brnz/*?*/		t,			_tLe63_tNe0;

/*o4!*/	rotqmby		ipNextBytes,		qwAtIpPlus16,		negIpNumBytesRight;





_tLe63_whileAtIpEq0Loop: /*============================================================================*/


/*e2!*/	or		ipBytes16,		ipBytes,		ipNextBytes;



/*e2!*/	clz		ipBytesLead0s/*per word*/,ipBytes16;



/*e4!*/	rotmi		num0Bytes/*per word*/,	ipBytesLead0s,		-3;
	lnop;







/*e2!*/	ceqi		num0BytesEq4,		num0Bytes,		4;
/*o4!*/	shlqbyi		num0BytesShl,		num0Bytes,		4;







/*e2!*/	and		num0BytesShl,		num0BytesShl,		num0BytesEq4;
	lnop;



/*e2!*/	a		num0Bytes,		num0Bytes,		num0BytesShl;
/*o4!*/	shlqbyi		num0BytesShl,		num0BytesShl,		4;







/*e2!*/	and		num0BytesShl,		num0BytesShl,		num0BytesEq4;
	lnop;



/*e2!*/	a		num0Bytes,		num0Bytes,		num0BytesShl;
/*o4!*/	shlqbyi		num0BytesShl,		num0BytesShl,		4;







/*e2!*/	and		num0BytesShl_,		num0BytesShl,		num0BytesEq4;
	lnop;



/*e2!*/	a		num0Bytes/*0:16*/,	num0Bytes,		num0BytesShl_;
/*e4*/	shli		deltaT__,			num0Bytes,		8;



/*e2!*/	a		ip,			ip,			num0Bytes;
/*e2*/	ceqi		num0BytesEq16_,		num0Bytes,		16;



/*e2!*/	andi		ipNumBytesLeft_,		ip,			15;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*e2*/	sf		deltaT_,			num0Bytes,		deltaT__;
/*o6*/	lqd		qwAtIp_,			0(ip);

/*e2!*/	sfi		ipNumBytesRight_,	ipNumBytesLeft_,		16;

/*e2*/	a		t,			t,			deltaT_;

 /*e2!*/	sfi		negIpNumBytesRight_,	ipNumBytesRight_,	0;		/*FOR NEXT LOOP!*/



 /*o4!*/	rotqmby		ipNextBytes,		qwAtIpPlus16,		negIpNumBytesRight_; /*FOR NEXT LOOP!*/

/*o4*/	shlqby		ipBytes,		qwAtIp_,			ipNumBytesLeft_;

/*Note: it does not seem to help to hint this*/
/*o?*/	brnz		num0BytesEq16_,		_tLe63_whileAtIpEq0Loop;

/*=======================================================================================================*/

/*e2*/	ai		t,			t,			31;

/*e2*/	ai/*b4 brz7*/	ipNumBytesRight,	ipNumBytesRight_,	-1;

/*e4!*/	rotmi		atIp,			ipBytes,		-24;
/*o4*/	shlqbyi/*b4 brz7*/ ipBytes,		ipBytes,		1;

/*e2*/	ai/*b4 brz7*/	ip,			ip,			1;
/*o?*/	brz/*1 per 16*/	ipNumBytesRight,	_reloadIp7;
_reloadIpRet7:

 /*e2!*/	sfi/*after brz7*/	negIpNumBytesRight,	ipNumBytesRight,	0;	/*MOVED UP FROM CODE BELOW _tLe63_tNe0 below)

{e2!*/	a		t,			t,			atIp;

_tLe63_tNe0: /* Note: this line moved up: {e2!}	sfi		negIpNumBytesRight,	ipNumBytesRight,	0*/
/*e2*/	ai		ip,			ip,			2;
/*o4!*/	rotqmby		ipNextBytes,		qwAtIpPlus16,		negIpNumBytesRight;

	nop $127;
/*o15*/	hbrr		_gotoTCodesJoinFrom2,	_ifTCodesJoin;

/*e2*/	andi		ipNumBytesLeft__,		ip,			15;
/*o6*/	lqd		qwAtIp__,			0(ip);

/*e2*/	ai		m_pos__,			op,			-1;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*e2!*/	or		ipBytes16,		ipBytes,		ipNextBytes;

/*e2*/	sfi		ipNumBytesRight,	ipNumBytesLeft__,		16;

/*o4!*/	shufb		deltaMpos,		ipBytes16,		ipBytes16,		mask_00BA_00BA_00BA_00BA;



/*o4*/	shlqby		ipBytes,		qwAtIp__,			ipNumBytesLeft__;



/*e4!*/	rotmi		deltaMpos_,		deltaMpos,		-2;







/*e2!*/	sf		m_pos,			deltaMpos_,		m_pos__;



/*e2*/	andi		mposNumBytesLeft_,	m_pos,			15;
/*o6!*/	lqd		qwAtMpos_,		0(m_pos);

/*o6!*/	lqd		qwAtMposPlus16,		16(m_pos);

_gotoTCodesJoinFrom2:	
/*o?!*/	br/*100%*/	_ifTCodesJoin;

.align 5

_tLe31:
/*e2!*/	andi		deltaMpos_,		t,			8;

/*e2!*/	sfi		negIpNumBytesRight,	ipNumBytesRight,		0;	/*MOVED UP FROM LOOP BELOW*/

/*e4!*/	shli		deltaMpos_,		deltaMpos_,		11;
	lnop;

/*e2*/	andi		t,			t,			7;
/*o4!*/	rotqmby		ipNextBytes_,		qwAtIpPlus16,		negIpNumBytesRight; /*MOVED UP FROM LOOP BELOW*/





/*e2!*/	sf		m_pos,			deltaMpos_,		op;/*m_pos*/
/*o?!*/	brnz/*?*/		t,			_tLe31_tNe0;

_tLe31_whileAtIpEq0Loop: /* each loop can inc. t by 255 ================================================*/
/*{this cycle lost iff code looped to from below}*/

/*e2!*/	or		ipBytes16,		ipBytes,		ipNextBytes_;



/*e2!*/	clz		ipBytesLead0s/*per word*/,ipBytes16;



/*e4!*/	rotmi		num0Bytes/*per word*/,	ipBytesLead0s,		-3;
	lnop;







/*e2*/	ceqi		num0BytesEq4,		num0Bytes,		4;
/*o4!*/	shlqbyi		num0BytesShl__,		num0Bytes,		4;







/*e2!*/	and		num0BytesShl__,		num0BytesShl__,		num0BytesEq4;
	lnop;



/*e2!*/	a		num0Bytes,		num0Bytes,		num0BytesShl__;
/*o4!*/	shlqbyi		num0BytesShl__,		num0BytesShl__,		4;







/*e2!*/	and		num0BytesShl__,		num0BytesShl__,		num0BytesEq4;
	lnop;



/*e2!*/	a		num0Bytes,		num0Bytes,		num0BytesShl__;
/*o4!*/	shlqbyi		num0BytesShl__,		num0BytesShl__,		4;







/*e2!*/	and		num0BytesShl_,		num0BytesShl__,		num0BytesEq4;



/*e2!*/	a		num0Bytes/*0:16*/,	num0Bytes,		num0BytesShl_;



/*e2!*/	a		ip,			ip,			num0Bytes;

/*e4*/	shli		deltaT__,			num0Bytes,		8;

/*e2!*/	andi		ipNumBytesLeft___,		ip,			15;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o6*/	lqd		qwAtIp___,			0(ip);

/*e2!*/	sfi		ipNumBytesRight__,	ipNumBytesLeft___,		16;

/*e2*/	sf		deltaT__,			num0Bytes,		deltaT__;

 /*e2!*/	sfi		negIpNumBytesRight__,	ipNumBytesRight__,	0;	/*COPIED FROM TOP OF LOOP*/

/*e2*/	a		t,			t,			deltaT__;
	lnop;

/*e2*/	ceqi		num0BytesEq16__,		num0Bytes,		16;
 /*o4!*/	rotqmby		ipNextBytes_,		qwAtIpPlus16,		negIpNumBytesRight__; /*COPIED FROM TOP OF LOOP*/

/*o4*/	shlqby		ipBytes,		qwAtIp___,			ipNumBytesLeft___;

/*Note: it does not seem to help to hint this*/
/*o?*/	brnz/*?*/		num0BytesEq16__,		_tLe31_whileAtIpEq0Loop;
/*=======================================================================================================*/

/*e4!*/	rotmi		atIp_,			ipBytes,		-24;

/*e2*/	ai/*b4 brz8*/	ipNumBytesRight,	ipNumBytesRight__,	-1;

/*e2*/	ai		t,			t,			7;
/*o4*/	shlqbyi/*b4 brz8*/ ipBytes,		ipBytes,		1;

/*e2*/	ai/*b4 brz8*/	ip,			ip,			1;
/*o?!*/	brz/*1 per 16*/	ipNumBytesRight,	_reloadIp8;
_reloadIpRet8:

/*e2!*/	sfi/*after brz8*/	negIpNumBytesRight,	ipNumBytesRight,	0;	/*COPIED FROM BELOW*/

/*e2!*/	a		t,			t,			atIp_;

_tLe31_tNe0: /* Moved this code up: {e2!}	sfi		negIpNumBytesRight,	ipNumBytesRight,	0*/
/*e2*/	ai		ip,			ip,			2;
/*o4!*/	rotqmby		ipNextBytes_,		qwAtIpPlus16,		negIpNumBytesRight;

/*e2*/	andi		ipNumBytesLeft___,		ip,			15;
/*o15*/	hbrr		_gotoIfTCodesJoinFrom2,	_ifTCodesJoin;

	nop $10;
/*o6*/	lqd		qwAtIp___,			0(ip);

/*e2*/	sfi		ipNumBytesRight,	ipNumBytesLeft___,		16;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*e2!*/	or		ipBytes16,		ipBytes,		ipNextBytes_;



/*o4!*/	shufb		deltaMpos__,		ipBytes16,		ipBytes16,		mask_00BA_00BA_00BA_00BA;



/*o4*/	shlqby		ipBytes,		qwAtIp___,			ipNumBytesLeft___;



/*e4!*/	rotmi		deltaMpos__,		deltaMpos__,		-2;







/*e2!*/	sf		m_pos,			deltaMpos__,		m_pos;



/*o?*/	ceq		mposEqOp,		m_pos,			op;

/*e2!*/	sf		m_pos,			_0x4000,		m_pos;

/*o?*/	brnz/*iff end*/	mposEqOp,		_eof_found;

/*e2*/	andi		mposNumBytesLeft_,	m_pos,			15;
/*o6!*/	lqd		qwAtMpos_,		0(m_pos);

/*o6!*/	lqd		qwAtMposPlus16,		16(m_pos);

_gotoIfTCodesJoinFrom2:	
/*o?!*/	br/*100%*/	_ifTCodesJoin;

.align 5

_tLe15:
/*e4!*/	rotmi		tShr2__,			t,			-2;
/*o15*/	hbrr		_gotoMatchDoneFrom2,	_match_done;

/*e4!*/	rotmi		atIpShl2__,		ipBytes,		-(24-2);
/*o4*/	cbd		opMask,			0(op);

/*e2*/	ai/*b4 brz9*/	ipNumBytesRight,	ipNumBytesRight,	-1;
/*o4*/	shlqbyi/*b4 brz9*/ ipBytes,		ipBytes,		1;

/*e2*/	ai		m_pos__,			op,			-1;
	lnop;

/*e2*/	ai/*b4 brz9*/	ip,			ip,			1;
/*o?*/	brz/*1 per 16*/	ipNumBytesRight,	_reloadIp9;
_reloadIpRet9:	

/*e2!*/	sf		m_pos_,			tShr2__,			m_pos__;

/*e2!*/	andi		atIpShl2__,		atIpShl2__,		-4;

/*e2!*/	clgt		opGtOpEndLess2,		op,			opEndLess2;

/*e2!*/	sf		m_pos_,			atIpShl2__,		m_pos_;	

	nop $24;
/*o1!*/	brnz/*iff error*/	opGtOpEndLess2,		_output_overrun;

/*e2*/	clgt		leftT,			out,			m_pos_;
/*o6!*/	lqd		qwAtMpos_,		0(m_pos_);

/*e2*/	clgt		rightF__,			op,			m_pos_;

/*e2*/	andi		mposNumBytesLeft___,	m_pos_,			15;

/*e2*/	orc		doBranch,		leftT,			rightF__;

/*e2*/	ai		m_pos_,			m_pos_,			1;

	nop $17;
/*o?*/	brnz/*iff error*/	doBranch,		_lookbehind_overrun;		

/*o4!*/	shlqby		mposBytes___,		qwAtMpos_,		mposNumBytesLeft___;

/*e2*/	andi		mposNumBytesLeft____,	m_pos_,			15;





/*e4!*/	rotmi		atMpos,			mposBytes___,		-24;







/*o4!*/	shufb		qwAtOp__,			atMpos,			qwAtOp,			opMask;

/*o4*/	cbx		opMask,			op,			_1;





/*o1!*/	stqd		qwAtOp__,			0(op);

/*o6!*/	lqd/*after stqd*/	qwAtMpos__,		0(m_pos_);











/*o4!*/	shlqby		mposBytes,		qwAtMpos__,		mposNumBytesLeft____;







/*e4!*/	rotmi		atMpos_,			mposBytes,		-24;

/*e2*/	ori/*copy*/	op__,			op,			0;

/*e2!*/	ai		op,			op,			2;



/*o4!*/	shufb		qwAtOp,			atMpos_,			qwAtOp__,			opMask;







/*o1!*/	stqx		qwAtOp,			op__,			_1;

_gotoMatchDoneFrom2:
/*o?!*/	br/*100%*/	_match_done;

.align 5

_ifTCodesJoin:
/* Note: status of registers upon entry:*/
/* qwAtMpos		not ready in 1st 3 cycles of code*/
/* qwAtMposPlus16	not ready in 1st 4 cycles of code*/

/*e2!*/	sf		dupLen,			m_pos,			op;
/*o15*/	hbrr		_match_doneFrom,	_match_done;

/*e2!*/	clgt		opGtOpEndQword__,		op,			opEndQword;

/*e2!*/	clgti		dupLenGt16,		dupLen,			16;

/*o1!*/	brnz/*iff error*/	opGtOpEndQword__,		_output_overrun;

/*e2!*/	selb		dupLen,			dupLen,			_16,			dupLenGt16;

/*e2*/	sfi		mposNumBytesRight_,	mposNumBytesLeft_,	16;

/*e4!*/	shli		dupLenShl4,		dupLen,			4;	

/*e2*/	sfi		negMposNumBytesRight_,	mposNumBytesRight_,	0;

/*o4*/	shlqby		mposBytes_,		qwAtMpos_,		mposNumBytesLeft_;	

/*e2*/	clgt		leftT_,			out,			m_pos;
/*o4*/	rotqmby		mposNextBytes_,		qwAtMposPlus16,		negMposNumBytesRight_;

/*e2*/	clgt		rightF_,			op,			m_pos;
/*o6!*/	lqx		dupShufMask,		gDupTbl,		dupLenShl4;

/*e2*/	andi		opNumBytesLeft__,		op,			15;

/*e2*/	ai		t,			t,			2;

/*e2*/	sfi		opNumBytesRight__,	opNumBytesLeft__,		16;

/*e2*/	or		mposBytes16_,		mposBytes_,		mposNextBytes_;

/*e2*/	clgt		tTooBig,		t,			opNumBytesRight__;
	lnop;

/*e2*/	sfi		negOpNumBytesLeft__,	opNumBytesLeft__,		0;
/*o4!*/	shufb		dupBytes,		mposBytes16_,		mposBytes16_,		dupShufMask;

/*e2*/	selb		numBytesToDup,		t,			opNumBytesRight__,	tTooBig;

/*e2*/	orc		doBranch_,		leftT_,			rightF_;

/*e2*/	a		m_pos,			m_pos,			numBytesToDup;
/*o4*/	rotqmby		dupMask,		neg1,			negOpNumBytesLeft__;

/*e2*/	sf		t,			numBytesToDup,		t;
/*o4!*/	rotqmby		dupBytesShr,		dupBytes,		negOpNumBytesLeft__;

/*e2*/	andi		mposNumBytesLeft__,	m_pos,			15;
/*o?*/	brnz/*iff error*/	doBranch_,		_lookbehind_overrun;		





/*e2!*/	selb		qwAtOp,			qwAtOp,			dupBytesShr,		dupMask;



/*o1!*/	stqd/*b4 lqd's*/	qwAtOp,			0(op);

/*e2!*/	sfi		mposNumBytesRight__,	mposNumBytesLeft__,	16;	/*MOVED UP FROM TOP OF LOOP*/
/*o6*/	lqd/*after stqd*/	qwAtMpos_,		0(m_pos);

/*e2!*/	a		op,			op,			numBytesToDup;
/*o6*/	lqd/*after stqd*/	qwAtMposPlus16_,		16(m_pos);

/*e2!*/	sfi		negMposNumBytesRight__,	mposNumBytesRight__,	0;	/*MOVED UP FROM TOP OF LOOP*/
/*o15*/	hbrr		_dupLoopBranch,		_dupLoop;

	nop $26;
_match_doneFrom:
/*o?!*/	brz/*?*/		tTooBig,		_match_done;

_dupLoop: /*==================================================================================================*/

/*e2*/	clgti		tTooBig,		t,			16;
/*o4!*/	shlqby		mposBytes__,		qwAtMpos_,		mposNumBytesLeft__;	

/*e2!*/	clgt		opGtOpEndQword___,		op,			opEndQword;
/*o4!*/	rotqmby		mposNextBytes__,		qwAtMposPlus16_,		negMposNumBytesRight__;

/*e2*/	selb		numBytesToDup_,		t,			_16,			tTooBig;

/*o1!*/	brnz/*iff error*/	opGtOpEndQword___,		_output_overrun;

/*e2*/	a		m_pos,			m_pos,			numBytesToDup_;

/*e2!*/	or		mposBytes16_,		mposBytes__,		mposNextBytes__;

/*e2*/	andi		mposNumBytesLeft__,	m_pos,			15;
	nop $32;

/*e2*/	sf		t,			numBytesToDup_,		t;
/*o4!*/	shufb		qwAtOp/*dupBytes*/,	mposBytes16_,		mposBytes16_,		dupShufMask;







 /*e2!*/	sfi		mposNumBytesRight__,	mposNumBytesLeft__,	16;	/*COPIED FROM TOP OF LOOP*/
/*o1!*/	stqd		qwAtOp,			0(op);

/*e2!*/	a		op,			op,			numBytesToDup_;
/*o6!*/	lqd/*after stqd*/	qwAtMpos_,		0(m_pos);

 /*e2!*/	sfi		negMposNumBytesRight__,	mposNumBytesRight__,	0;	/*COPIED FROM TOP OF LOOP*/
/*o6!*/	lqd/*after stqd*/	qwAtMposPlus16_,		16(m_pos);

	nop $26;
_dupLoopBranch:
/*o?!*/	brnz/*frequent?*/	tTooBig,		_dupLoop;
/*==================================================================================================*/

_match_done:
/*e2*/	ai		ipLess5,		ip,			-2-3;
/*o6*/	lqx		qwAtIpLess2,		ip,			_neg2;











/*o4*/	rotqby		atIpLess2/*byte3*/,	qwAtIpLess2,		ipLess5;







/*e2*/	andi		t,			atIpLess2,		3;

	nop $3;

/*o?*/	brz		t,			_mainLoop;

_match_next:
/*e2*/	sfi		negIpNumBytesRight,	ipNumBytesRight,		0;
/*o15*/	hbrr		 _whileToMatchFrom,	_match;

/*e2*/	andi		opNumBytesLeft,		op,			15;
	nop $3;

/*e2!*/	clgti		tGt1/*0 or -1*/,		t,			1;
/*o4*/	shlqbyi/*copy*/	op__,			op,			0;

/*e2!*/	clgti		tGt2/*0 or -1*/,		t,			2;

/*e2*/	sfi		negOpNumBytesLeft___,	opNumBytesLeft,		0;

/*e2*/	sfi		opNumBytesRight___,	opNumBytesLeft,		16;
/*o4*/	rotqmby		ipNextBytes,		qwAtIpPlus16,		negIpNumBytesRight;

/*e2!*/	a		numBytesToCopy/*0:-2*/,	tGt1,			tGt2;

/*e2*/	clgti		opNumBytesRightGt2_,	opNumBytesRight___,	2;

/*e2!*/	sfi		numBytesToCopy/*1:3*/,	numBytesToCopy,		1;

/*e2*/	or		ipBytes16,		ipBytes,		ipNextBytes;

/*e2!*/	a		ip,			ip,			numBytesToCopy;
/*o4*/	rotqmby		copyMask,		neg1,			negOpNumBytesLeft___;

/*e2*/	a		op,			op,			numBytesToCopy;
/*o4*/	rotqmby		ipBytesShr_,		ipBytes16,		negOpNumBytesLeft___;

/*e2!*/	ai		ip,			ip,			1;
/*o4*/	shlqby		ipBytesShl,		ipBytes16,		opNumBytesRight___;

/*e2*/	clgt		opGtOpEnd,		op,			op_end;
/*o4*/	shlqby		ipBytes16_,		ipBytes16,		numBytesToCopy;

/*e2*/	andc		secondQwordOfst_,	_16,			opNumBytesRightGt2_;
/*o6!*/	lqd		qwAtIp__,			0(ip);

/*e2*/	selb		qwAtOp___,			qwAtOp,			ipBytesShr_,		copyMask;
/*o1*/	brnz/*b4 stqx,iff error*/	opGtOpEnd,	_output_overrun;

/*o1*/	stqx/*aftrBrnz,b4Stqd*/ ipBytesShl,	op__,			secondQwordOfst_;

/*o1*/	stqd/*aftrStqx,b4Lqd*/  qwAtOp___,		0(op__);

/*e2*/	andi		ipNumBytesLeft,		ip,			15;
/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*e2*/	clgt		ipEndGtIp,		ip_end,			ip;
/*o6*/	lqd/*afterStqd*/	qwAtOp,			0(op);/*LOAD EITHER ONE OF 2 STQDS ABOVE*/

/*e4*/	rotmi		t,			ipBytes16_,		-24;
/*o4!*/	shlqby		ipBytes,		qwAtIp__,			ipNumBytesLeft;

/*e2*/	sfi		ipNumBytesRight,	ipNumBytesLeft,		16;
_whileToMatchFrom:
/*o?!*/	brnz/*branch unless error*/ ipEndGtIp,	_match;

_ipEndLeIp:
/*	    *out_len = ((lzo_uint) ((op)-(out)));*/
/*	    return (-7);*/
/*e2*/	il		$3,			-7;
/*o?*/	br		_return;



.align 5

_reloadIp:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_reloadIpRet;

_reloadIp2:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_reloadIpRet2;

_reloadIp3:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_tNe0;

_reloadIp4:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_reloadIpRet4;

_reloadIp5:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_reloadIpRet5;

_reloadIp6:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_reloadIpRet6;

_reloadIp7:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_reloadIpRet7;

_reloadIp8:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_reloadIpRet8;

_reloadIp9:
/*e2*/	il		ipNumBytesRight,	16;
/*o6*/	lqd		ipBytes/*qwAtIp*/,	0(ip);

/*o6*/	lqd		qwAtIpPlus16,		16(ip);

/*o?*/	br		_reloadIpRet9;



_eof_found:
/*e2*/	clgt		ipEndGtIp_/*-1 or 0*/,	ip_end,			ip;
/*o?*/	ceq		ipEqIpEnd,		ip,			ip_end;



/*e4*/	shli		$3/*-4 or 0*/,		ipEndGtIp_,		2;







/*e2*/	ai		$3/*-8 or -4*/,		$3,			-4;



/*e2*/	andc		$3/*-8,-4 or 0*/,		$3,			ipEqIpEnd;
	lnop;
/*-----------------------------*/
_return: /* Note: $3 = return value*/
/*e2*/	ila		gOutLen_,		_gOutLen;
/*o15*/	hbr		_endBiFrom,		$lr;

/*e2*/	sf		outLen,			out,			op;

/*o6*/	lqx		out_len,		picOffset,		gOutLen_;











/*o6*/	lqd		qwAtOutLen,		0(out_len);

/*o4*/	cwd		outLenMask,		0(out_len);









/*o4*/	shufb		qwAtOutLen,		outLen,			qwAtOutLen,		outLenMask;







/*o1*/	stqd		qwAtOutLen,		0(out_len);

	nop $127;
_endBiFrom:
/*o?*/	bi/*100%*/	$lr;

_output_overrun:
/*e2*/	il		$3,			-5;
/*o?*/	br		_return;

/*	lookbehind_overrun:*/
/*	    *out_len = ((lzo_uint) ((op)-(out)));*/
/*	    return (-6);*/
/*	}*/
_lookbehind_overrun:
/*e2*/	il		$3,			-6;
/*o?*/	br		_return;





/*/////////////////////////////////////////////////////////////////////////////////////////*/

/*	uninitialized data*/

/*/////////////////////////////////////////////////////////////////////////////////////////*/

.section .bss
.align 5	/* must be >= 4*/

.global	_gOutLen
_gOutLen:
	.word	0,0,0,0



.section .rodata
.align 5	/* must be >= 4*/

.global _gDupTbl
_gDupTbl:	.byte 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	/* 00 (not used)*/
		.byte 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	/* 01*/
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


