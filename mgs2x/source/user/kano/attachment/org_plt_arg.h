/*
	org_plt_arg.h
	セルゲイアタッチメント用パラメータヘッダ

	2001/06/17 T.Shibata

	$Id: org_plt_arg.h,v 1.1.1.3 2002/11/19 11:43:06 Yoshizawa1 Exp $
*/
#ifndef ORG_PLT_ARG_H
#define ORG_PLT_ARG_H

#define	MDL_CODE_OP_SSK		(5695259)		//demo_org_plant_ssk
#define	MDL_CODE_OP_HLST	(14108894)		//demo_org_plant_hlst
#define	MDL_CODE_OP_RADIO	(8609514)		//demo_org_plant_radio

#define	MDL_OP_SSK		(0)		//crg_skn
#define	MDL_OP_HLST		(1)		//crg_hlst
#define	MDL_OP_RADIO	(2)		//crg_waist_parts01

static FVECTOR org_plt_shift[]={
    {  112.0f,  47.0f,  87.0f, 1.0f },
    { -117.0f,  93.0f,  72.0f, 1.0f },
    {   81.0f, 147.0f, -77.0f, 1.0f },
};

static FVECTOR org_plt_shift2[]={
    { 37.0f,  -164.0f, 122.0f, 1.0f },
};

static ATTACHMENT_ARGUMENT2 org_plt_arg2[] = {
	{
		MDL_CODE_OP_SSK,
		NULL,
		NULL,
		0,
		&org_plt_shift[MDL_OP_SSK],
		17,
		&org_plt_shift2[MDL_OP_SSK],
    },
};

static ATTACHMENT_ARGUMENT3 org_plt_arg3[]={
    {
		MDL_CODE_OP_HLST,
		NULL,
		0,
		&org_plt_shift[MDL_OP_HLST],
		4,
    },
    {
		MDL_CODE_OP_RADIO,
		NULL,
		0,
		&org_plt_shift[MDL_OP_RADIO],
		4,
    },
};

#endif
