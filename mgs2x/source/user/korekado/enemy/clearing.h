
/*
	clearing.h
	クリアリングヘッダ

	1997/07/13 Y.Korekado
	$Id: clearing.h,v 1.1.1.3 2002/11/19 11:44:06 Yoshizawa1 Exp $

*/
#ifndef __CLEARING___
#define	__CLEARING___ 1

enum {
	RDATA_CON_NONE,
	RDATA_CON_START_SIGN,
	RDATA_CON_DE_END,
	RDATA_CON_CLE_SIGN,
	RDATA_CON_END,
	RDATA_CON_END_SIGN
} ;


#if 0
HZX_CLE_PTP	hzx_cle_ptp[] = {
	{
		8500.0F, 6000.0F, -15500.0F,
		8500.0F, 6000.0F, -15000.0F,
		1,	//act
		0,	//time
		0,	//dir
		0	//pad
	},
	{
		8500.0F, 6000.0F, -15500.0F,
		8500.0F, 6000.0F, -15000.0F,
		2,	//act
		30000,	//time
		0,	//dir
		RDATA_CON_START_SIGN	//pad
	},
	{
		8500.0F, 6000.0F, -14500.0F,
		9000.0F, 6000.0F, -14500.0F,
		1,	//act
		0,	//time
		1024,	//dir
		RDATA_CON_DE_END	//pad
	},
	{
		9500.0F, 6000.0F, -14500.0F,
		10000.0F, 6000.0F, -14500.0F,
		1,	//act
		60,	//time
		1024,	//dir
		RDATA_CON_END	//pad
	},

	{
		8500.0F, 6000.0F, -16500.0F,
		8500.0F, 6000.0F, -16000.0F,
		1,	//act
		0,	//time
		0,	//dir
		0	//pad
	},
	{
		8500.0F, 6000.0F, -16500.0F,
		8000.0F, 6000.0F, -16500.0F,
		1,	//act
		0,	//time
		3072,	//dir
		RDATA_CON_END	//pad
	},

	{
		9500.0F, 6000.0F, -12500.0F,
		9000.0F, 6000.0F, -12500.0F,
		1,	//act
		0,	//time
		3072,	//dir
		0	//pad
	},
	{
		8500.0F, 6000.0F, -12500.0F,
		8500.0F, 6000.0F, -13000.0F,
		0,	//act
		0,	//time
		2042,	//dir
		0	//pad
	},
	{
		8500.0F, 6000.0F, -13500.0F,
		9000.0F, 6000.0F, -13500.0F,
		1,	//act
		0,	//time
		1024,	//dir
		RDATA_CON_DE_END	//pad
	},
	{
		8500.0F, 6000.0F, -13500.0F,
		9000.0F, 6000.0F, -13500.0F,
		2,	//act
		30000,	//time
		1024,	//dir
		RDATA_CON_CLE_SIGN	//pad
	},
	{
		10500.0F, 6000.0F, -13500.0F,
		11500.0F, 6000.0F, -13500.0F,
		8,	//act
		0,	//time
		1024,	//dir
		0	//pad
	},
	{
		11500.0F, 6000.0F, -13500.0F,
		12500.0F, 6000.0F, -13500.0F,
		11,	//act
		30000,	//time
		2048,	//dir
		0	//pad
	},
	{
		11500.0F, 6000.0F, -13500.0F,
		12500.0F, 6000.0F, -13500.0F,
		1,	//act
		60,	//time
		2048,	//dir
		RDATA_CON_END	//pad
	},
	{
		11500.0F, 6000.0F, -13500.0F,
		12500.0F, 6000.0F, -13500.0F,
		2,	//act
		30000,	//time
		2048,	//dir
		RDATA_CON_END_SIGN	//pad
	},

	{
		10500.0F, 5000.0F, -12500.0F,
		9500.0F, 5000.0F, -12500.0F,
		1,	//act
		0,	//time
		3072,	//dir
		0	//pad
	},
	{
		8500.0F, 6000.0F, -12500.0F,
		9500.0F, 6000.0F, -12500.0F,
		1,	//act
		0,	//time
		1024,	//dir
		RDATA_CON_END	//pad
	}
} ;

HZX_CLE_ROOT hzx_cle_root[] = {
	{
		4,
		0,
		0,0,
		NULL
	},
	{
		2,
		0,
		0,0,
		NULL
	},
	{
		8,
		0,
		0,0,
		NULL
	},
	{
		2,
		0,
		0,0,
		NULL
	}
} ;

HZX_CLE_AREA hzx_cle_area[] = {
	{
		9000.0F, 6000.0F, -15000.0F,0.0F,
		13000.0F,8000.0F,-13000.0F,0.0F,
		1,
		300,
		4,
		NULL,
	}
} ;

#endif

#endif
