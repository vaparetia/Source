/*
	crg_arg.h
	セルゲイアタッチメント用パラメータヘッダ

	2000/04/19 T.Shibata
	$Id: crg_arg.h,v 1.1.1.3 2002/11/19 11:43:05 Yoshizawa1 Exp $
*/
#ifndef CRG_ARG_H
#define CRG_ARG_H

#define MDL_CODE_HLST	(2683944)		//crg_hlst
#define MDL_CODE_SKN	(10580856)		//crg_skn
#define MDL_CODE_PARTS0	(12120022)		//crg_waist_parts00
#define MDL_CODE_PARTS1	(12120023)		//crg_waist_parts01

#define MDL_HLST	(0)		//crg_hlst
#define MDL_SKN		(1)		//crg_skn
#define MDL_PARTS1	(2)		//crg_waist_parts01
#define MDL_PARTS0	(3)		//crg_waist_parts00

static FVECTOR crg_shift[]={
    { -185.0f,  95.0f, 125.0f, 0.0f },
    {  -90.0f, -20.0f, 205.0f, 0.0f },
    {  175.0f,  95.0f, 125.0f, 0.0f },
    {  185.0f, 105.0f, -70.0f, 0.0f },
};

static FVECTOR crg_shift2[]={
    { -80.0f,  -40.0f, 140.0f, 0.0f },
    {  40.0f, -270.0f, 200.0f, 0.0f },
    {  50.0f,  -25.0f, 120.0f, 0.0f },
};

static ATTACHMENT_ARGUMENT2 crg_arg2[]={

    {
		MDL_CODE_HLST,
		NULL,
		NULL,
		0,
		&crg_shift[MDL_HLST],
		13,
		&crg_shift2[MDL_HLST],
    },

    {
		MDL_CODE_SKN,
		NULL,
		NULL,
		0,
		&crg_shift[MDL_SKN],
		13,
		&crg_shift2[MDL_SKN],
    },
	
    {
		MDL_CODE_PARTS1,
		NULL,
		NULL,
		0,
		&crg_shift[MDL_PARTS1],
		17,
		&crg_shift2[MDL_PARTS1],
    },
};

static ATTACHMENT_ARGUMENT3 crg_arg3[]={
    {
		MDL_CODE_PARTS0,
		NULL,
		0,
		&crg_shift[MDL_PARTS0],
		4,
    },
};

#endif
