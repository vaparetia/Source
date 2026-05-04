/*
	org_arg.h
	オルガアタッチメント用パラメータヘッダ

	2000/04/19 Y.Korekado
	$Id: org_arg.h,v 1.1.1.3 2002/11/19 11:43:05 Yoshizawa1 Exp $
*/

static FVECTOR org_shift[]={
    { -109.8f,  90.67f,  55.0f,  0.0f, },
    {  76.0f,  90.67f, 107.5f,   0.0f, },
};
static FVECTOR org_shift2[]={
    {  -35.0f, -10.0f,  86.5f,   0.0f, },
    {   55.5f, -127.5f, 106.5f,  0.0f, },
};
static SVECTOR org_rot1[]={
    {  0, -626, -114, 0 },
    {  0, 91, 171, 0 },
    {  0, 1786, 0, 0 },
};
static FVECTOR org_shift3[]={
    {  67.0f,  90.67f, -51.5f,   0.0f, },
};

static ATTACHMENT_ARGUMENT2 org_arg2[]={
    {
	11210614 /* GV_StrCode("org_gbhlst") */ ,
	NULL,
	org_rot1+0,
	0,
	org_shift+0,
	13,
	org_shift2+0,
    },
    {
	11293163 /* GV_StrCode("org_sk_cover") */ ,
	NULL,
	org_rot1+1,
	0,
	org_shift+1,
	17,
	org_shift2+1,
    },
};

static ATTACHMENT_ARGUMENT3 org_arg3[]={
    {
	12896276 /* GV_StrCode("org_radio") */ ,
	org_rot1+2,
	0,
	org_shift3+0,
	4,
    },
};

