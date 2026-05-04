/*
	ene_arg.h
	敵兵アタッチメント用パラメータヘッダ

	2000/04/19 Y.Korekado
	$Id: ene_arg.h,v 1.1.1.3 2002/11/19 11:43:05 Yoshizawa1 Exp $
*/

static FVECTOR ene_shift[]={
    { -185.0f,  27.5f,  61.5f,   0.0f, },
    {  -67.5f,  -2.5f, 152.5f,   0.0f, },
    {  120.0f,  22.5f, -60.0f,   0.0f, },
};
static FVECTOR ene_shift2[]={
    {  -65.0f, -10.0f,  42.5f,   0.0f, },
    {   32.5f,-140.0f, 112.5f,   0.0f, },
    {   20.0f, -92.5f,-117.5f,   0.0f, },
};
static FVECTOR ene_shift3[]={
    {    0.0f, -40.0f,-140.0f,   0.0f, },
};

static ATTACHMENT_ARGUMENT2 ene_arg2[]={
    {
	0x291040 /* GV_StrCode("gbs_hlst") */ ,
	NULL,
	NULL,
	0,
	ene_shift+0,
	13,
	ene_shift2+0,
    },
    {
	0x2a96f2 /* GV_StrCode("gbs_knif") */ ,
	NULL,
	NULL,
	0,
	ene_shift+1,
	13,
	ene_shift2+1,
    },
    {
	0x615b12 /* GV_StrCode("gbs_mag") */ ,
	NULL,
	NULL,
	0,
	ene_shift+2,
	17,
	ene_shift2+2,
    },
};

static ATTACHMENT_ARGUMENT3 ene_arg3[]={
    {
	0x5b0984 /* GV_StrCode("gbs_bp") */ ,
	NULL,
	2,
	ene_shift3+0,
	2,
    },
};

