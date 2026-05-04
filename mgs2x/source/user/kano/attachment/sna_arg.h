/*
	ene_arg.h
	スネークアタッチメント用パラメータヘッダ

	2000/04/19 Y.Korekado
	$Id: sna_arg.h,v 1.1.1.3 2002/11/19 11:43:06 Yoshizawa1 Exp $
*/

static FVECTOR sna_shift[] = {
	{ 95.0f,85.0f,107.5f,0.0f },
	{ -95.0f,85.0f,107.5f,0.0f },
	{ 72.5f,37.5f,120.0f,0.0f },
	{ -72.5f,37.5f,120.0f,0.0f }
} ;

static ATTACHMENT_ARGUMENT3 sna_arg3[]={
	{
		0x2bc559		/* GV_StrCode("sna_mag1") */ ,
		NULL,
		2,				/* 胸 */
		sna_shift+0,
		3,				/* frames */
	},
	{
		0x2bc55a		/* GV_StrCode("sna_mag2") */ ,
		NULL,
		2,				/* 胸 */
		sna_shift+1,
		3,				/* frames */
	},
	{
		0x2bc55b		/* GV_StrCode("sna_mag3") */,
		NULL,
		2,				/* 胸 */
		sna_shift+2,
		3,				/* frames */
	},
	{
		0x2bc55c		/* GV_StrCode("sna_mag4") */,
		NULL,
		2,				/* 胸 */
		sna_shift+3,
		3,				/* frames */
	},
};
