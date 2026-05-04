//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sample.c
		髪の毛(長髪タイプ)シミュレーション
		髪の毛用サンプルデータ

	2000/06/02 K.Kano
	$Id: sample.c,v 1.1.1.3 2002/11/19 11:43:12 Yoshizawa1 Exp $
*/


#include "hair.h"


//オセロットの当たりチェック関節の配列
const u_char Demo_CollisionObjs_Rev[] = { 2, 12, 11 };


const SAMPLE_HAIR_PARAMETER hair_sample[]={
	/* ライデン */

	/* 0 : 髪の毛 */
    {
		250*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },
	/* 1 : 水中髪の毛 */
    {
		250*0.000001f,		/* -p */
		1000*0.001f,		/* -m */
//		100000*0.000001f,	/* -k */
		1000*0.000001f,		/* -k */
//		5*0.01f,			/* -a */
		400*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
//		30*0.01f,			/* -w */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },
	/* 2 : 水から上がった直後の髪の毛 */
    {
		250*0.000001f,		/* -p */
		500*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		95*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },
	/* 3 : 少し乾いた髪の毛 */
    {
		250*0.000001f,		/* -p */
		300*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		85*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* オセロット */
	/* 4 : 髪の毛 */
    {
		250*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		0*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 5 : フォーチュン裾 */
	{
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		50*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 6 : フォーチュン袖 */
	{
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 7 : フォーチュン髪の毛 */
	{
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		50*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 8 : ゴルルゴマスク */
	/* -r 3891,2935,3003 */
	{
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 9 : ヴァンプ髪の毛 */
	{
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 10 : ヴァンプナイフ */
	{
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-100,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 11 : ヴァンプコート */
	{
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-100,				/* -l */
		90*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 12 : ピーターのフード */
    {
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		800*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		100*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 13 : 死体ピーターのフード */
    {
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		90*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 14 : 死体ピーターのオーバータイ */
    {
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		50*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 15 : 潜水ライデンの髪の毛 */
	{
		250*0.000001f,		/* -p */
//		50*0.001f,			/* -m */
		200*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		100*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		50*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 16 : ローズの髪の毛(ros_hair_f,ros_hair_b) */
	{
		250*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 17 : エマの髪の毛 */
	{
		250*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		10,					/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 18 : エマのＩＤ */
	{
		250*0.000001f,		/* -p */
#if 0
		25*0.001f,			/* -m */
#elif 0
		200*0.001f,			/* -m */
#else
		100*0.001f,			/* -m */
#endif
		1000*0.000001f,		/* -k */
#if 0
		300*0.01f,			/* -a */
#elif 0
		100*0.01f,			/* -a */
#else
		200*0.01f,			/* -a */
#endif
		-0.7f+1.0f,			/* -e */
		50,					/* -l */
		90*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 19 : ソリダスのマント */
	{
		1000*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		100000*0.000001f,	/* -k */
		5000*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
	},

	/* 20 : プラントオルガの髪の毛 */
    {
		250*0.000001f,		/* -p */
		25*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		50*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 21 : オタコンの裾 */
    {
#if 0
		250*0.000001f,		/* -p */
		5*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
#else
		0*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		100*0.01f,			/* -w */
		0*0.01f,			/* -j */
#endif
    },

	/* 22 : 濡れたエマの髪 */
    {
		250*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		7000*0.000001f,		/* -k */
		50*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		100,				/* -l */
		30*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 23 : ゴルルゴ兵の服 */
    {
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		50*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 24 : ライデン髪の毛エレベーター用 */
    {
		250*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		95*0.01f,			/* -j */
    },

	/* 25 : ライデンドッグタグ */
    {
		250*0.000001f,		/* -p */
		25*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		90*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 26 : ライデンの潜水フィン */
    {
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		50*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 27 : オタコンの前髪 */
    {
		250*0.000001f,		/* -p */
		500*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		50*0.01f,			/* -j */
    },


	/* 28 : メリルの前髪 */
    {
		250*0.000001f,		/* -p */
		50*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		300*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },

	/* 29 : タキシードの裾 */
    {
		250*0.000001f,		/* -p */
		100*0.001f,			/* -m */
		1000*0.000001f,		/* -k */
		500*0.01f,			/* -a */
		-0.7f+1.0f,			/* -e */
		-3000,				/* -l */
		80*0.01f,			/* -w */
		0*0.01f,			/* -j */
    },
};

static const unsigned char Col_HairNormal1[]={
	2,11,12,
};

static const unsigned char Col_HairNormal2[]={
	11,12,
};

static const unsigned char Col_Fortune1[]={
	13,14,17,18,
};

static const unsigned char Col_Fortune2R[]={
	2,3,4,5,13,
};

static const unsigned char Col_Fortune2L[]={
	2,7,8,9,17,
};

static const unsigned char Col_VampHair[]={
	3,7,11,12,
};

static const unsigned char Col_NakedVampHair[]={
	2,3,7,11,12,
};

static const unsigned char Col_VampKnife[]={
	0,13,17,
};

static const unsigned char Col_VampCoat[]={
	0,13,14,17,18,
};

static const unsigned char Col_Peter[]={
	2,11,
};

static const unsigned char Col_RaidenInWater[]={
	2,11,
};

static const unsigned char emahair_colobjs[]={ 11,12, };
static const unsigned char emaid_colobjs[]={ 2,11,3,7, };

static const unsigned char Col_PlantOrga[]={
	11,12,
};

#if 0

static const unsigned char Col_OtaconJacket[]={
	0,1,13,17,
};

#else

static const unsigned char Col_OtaconJacket[]={
	13,17,0,1,2,
};

#endif

static const unsigned char Col_RaidenDoctag[]={
	2,3,7,
};

static const unsigned char Col_OtaconHair[]={
	12,
};

static const unsigned char Col_TaxedoSnake[]={
	0, 1, 2, 13, 17
} ;

const unsigned char *hair_colcheck[]={
	/* フォーチュン裾 */
	Col_Fortune1,
	/* フォーチュン右袖 */
	Col_Fortune2R,
	/* フォーチュン左袖 */
	Col_Fortune2L,
	/* フォーチュン髪の毛 */
	Col_HairNormal2,
	/* ヴァンプ髪の毛 */
	Col_VampHair,
	/* ヴァンプナイフ */
	Col_VampKnife,
	/* ヴァンプコート */
	Col_VampCoat,
	/* ピーターのフード */
	Col_Peter,
	/* 死体ピーターのフード */
	Col_Peter,
	/* 死体ピーターのオーバータイ */
	Col_Peter,
	/* 潜水ライデンの髪の毛 */
	Col_RaidenInWater,
	/* ローズの髪の毛(ros_hair_f,ros_hair_b) */
	Col_HairNormal1,
	/* エマの髪の毛 */
	emahair_colobjs,
	/* エマのＩＤ */
	emaid_colobjs,
	/* ソリダスのマント */
	NULL,
	/* プラントオルガの髪の毛 */
	Col_PlantOrga,
	/* ソリダスのマント２ */
	NULL,
	/* オタコンの裾 */
	Col_OtaconJacket,
	/* エマの髪の毛 */
	emahair_colobjs,
	/* ゴルルゴ兵の服 */
	NULL,
	/* ライデンのドックタグ */
	Col_RaidenDoctag,
	/* ライデンのフィン(右) */
	NULL,
	/* ライデンのフィン(左) */
	NULL,
	/* オタコンの前髪 */
	Col_OtaconHair,
	/* 裸ヴァンプの髪の毛 */
	Col_NakedVampHair,
	/* タキシードの裾 */
	Col_TaxedoSnake,
};

const unsigned char n_hair_colcheck[]={
	sizeof(Col_Fortune1)/sizeof(Col_Fortune1[0]),
	sizeof(Col_Fortune2R)/sizeof(Col_Fortune2R[0]),
	sizeof(Col_Fortune2L)/sizeof(Col_Fortune2L[0]),
	sizeof(Col_HairNormal2)/sizeof(Col_HairNormal2[0]),
	sizeof(Col_VampHair)/sizeof(Col_VampHair[0]),
	sizeof(Col_VampKnife)/sizeof(Col_VampKnife[0]),
	sizeof(Col_VampCoat)/sizeof(Col_VampCoat[0]),
	sizeof(Col_Peter)/sizeof(Col_Peter[0]),
	sizeof(Col_Peter)/sizeof(Col_Peter[0]),
	sizeof(Col_Peter)/sizeof(Col_Peter[0]),
	sizeof(Col_RaidenInWater)/sizeof(Col_RaidenInWater[0]),
	sizeof(Col_HairNormal1)/sizeof(Col_HairNormal1[0]),
	sizeof(emahair_colobjs)/sizeof(emahair_colobjs[0]),
	sizeof(emaid_colobjs)/sizeof(emaid_colobjs[0]),
	0xff,
	sizeof(Col_PlantOrga)/sizeof(Col_PlantOrga[0]),
	0xff,
	sizeof(Col_OtaconJacket)/sizeof(Col_OtaconJacket[0]),
	sizeof(emahair_colobjs)/sizeof(emahair_colobjs[0]),
	0,
	sizeof(Col_RaidenDoctag)/sizeof(Col_RaidenDoctag[0]),
	0,
	0,
	sizeof(Col_OtaconHair)/sizeof(Col_OtaconHair[0]),
	sizeof(Col_NakedVampHair)/sizeof(Col_NakedVampHair[0]),
	sizeof(Col_TaxedoSnake)/sizeof(Col_TaxedoSnake[0]),
};


static const unsigned char Root_Fortune1[]={
	0,1,
};

static const unsigned char Root_Fortune2R[]={
	3,4,
};

static const unsigned char Root_Fortune2L[]={
	7,8,
};

static const unsigned char Root_Hair[]={
	12,
};

static const unsigned char Root_VampHair[]={
	2,11,12,
};

static const unsigned char Root_VampKnife[]={
	0,
};

static const unsigned char Root_VampCoat[]={
	0,13,17,
};

static const unsigned char Root_Peter[]={
	2,
};

static const unsigned char Root_RaidenInWater[]={
	11,12,
};

static const unsigned char emahair_objnum[]={ 11,12, };
static const unsigned char emaid_objnum[]={ 2, };

static const unsigned char solmant_objnum[]={
	0,1,2,3,4,7,8,11,13,14,17,18,
};

static const unsigned char Root_PlantOrga[]={
	11,12,
};

static const unsigned char solmant2_objnum[]={
	0,1,2,13,14,17,18,
};

#if 0

static const unsigned char OtaconJacket_objnum[]={
	0,1,13,17,
};

#else

static const unsigned char OtaconJacket_objnum[]={
	0,1,2,13,17,
};

#endif

static const unsigned char golcloth_objnum[]={
	0,
};

static const unsigned char Root_Doctag[]={
	2,
};

static const unsigned char Root_RaidenRightFin[]={
	15,16,
};

static const unsigned char Root_RaidenLeftFin[]={
	19,20,
};

static const unsigned char Root_OtaconHair[]={
	12,0xff,
};
static const unsigned char Root_TaxedoSnake[]={
	0, 1, 2, 13, 17
} ;

const unsigned char *hair_root[]={
	Root_Fortune1,
	Root_Fortune2R,
	Root_Fortune2L,
	Root_Hair,
	Root_VampHair,
	Root_VampKnife,
	Root_VampCoat,
	Root_Peter,
	Root_Peter,
	Root_Peter,
	Root_RaidenInWater,
	Root_Hair,
	emahair_objnum,
	emaid_objnum,
	solmant_objnum,
	Root_PlantOrga,
	solmant2_objnum,
	OtaconJacket_objnum,
	emahair_objnum,
	golcloth_objnum,
	Root_Doctag,
	Root_RaidenRightFin,
	Root_RaidenLeftFin,
	Root_OtaconHair,
	Root_VampHair,
	Root_TaxedoSnake,
};

const unsigned char n_hair_root[]={
	sizeof(Root_Fortune1)/sizeof(Root_Fortune1[0]),
	sizeof(Root_Fortune2R)/sizeof(Root_Fortune2R[0]),
	sizeof(Root_Fortune2L)/sizeof(Root_Fortune2L[0]),
	sizeof(Root_Hair)/sizeof(Root_Hair[0]),
	sizeof(Root_VampHair)/sizeof(Root_VampHair[0]),
	sizeof(Root_VampKnife)/sizeof(Root_VampKnife[0]),
	sizeof(Root_VampCoat)/sizeof(Root_VampCoat[0]),
	sizeof(Root_Peter)/sizeof(Root_Peter[0]),
	sizeof(Root_Peter)/sizeof(Root_Peter[0]),
	sizeof(Root_Peter)/sizeof(Root_Peter[0]),
	sizeof(Root_RaidenInWater)/sizeof(Root_RaidenInWater[0]),
	sizeof(Root_Hair)/sizeof(Root_Hair[0]),
	sizeof(emahair_objnum)/sizeof(emahair_objnum[0]),
	sizeof(emaid_objnum)/sizeof(emaid_objnum[0]),
	sizeof(solmant_objnum)/sizeof(solmant_objnum[0]),
	sizeof(Root_PlantOrga)/sizeof(Root_PlantOrga[0]),
	sizeof(solmant2_objnum)/sizeof(solmant2_objnum[0]),
	sizeof(OtaconJacket_objnum)/sizeof(OtaconJacket_objnum[0]),
	sizeof(emahair_objnum)/sizeof(emahair_objnum[0]),
	sizeof(golcloth_objnum)/sizeof(golcloth_objnum[0]),
	sizeof(Root_Doctag)/sizeof(Root_Doctag[0]),
	sizeof(Root_RaidenRightFin)/sizeof(Root_RaidenRightFin[0]),
	sizeof(Root_RaidenLeftFin)/sizeof(Root_RaidenLeftFin[0]),
	sizeof(Root_OtaconHair)/sizeof(Root_OtaconHair[0]),
	sizeof(Root_VampHair)/sizeof(Root_VampHair[0]),
	sizeof(Root_TaxedoSnake)/sizeof(Root_TaxedoSnake[0]),
};


const float p_array[][2]={
	{
		0.0f,
		1.0f,
	},
	{
		1.0f/48.0f,
		47.0f/48.0f,
	},
	{
		2.0f/48.0f,
		46.0f/48.0f,
	},
	{
		3.0f/48.0f,
		45.0f/48.0f,
	},
	{
		1.0f/12.0f,
		11.0f/12.0f,
	},
	{
		2.0f/12.0f,
		10.0f/12.0f,
	},
	{
		3.0f/12.0f,
		9.0f/12.0f,
	},
	{
		4.0f/12.0f,
		8.0f/12.0f,
	},
	{
		5.0f/12.0f,
		7.0f/12.0f,
	},
	{
		6.0f/12.0f,
		6.0f/12.0f,
	},
	{
		7.0f/12.0f,
		5.0f/12.0f,
	},
	{
		8.0f/12.0f,
		4.0f/12.0f,
	},
	{
		9.0f/12.0f,
		3.0f/12.0f,
	},
	{
		10.0f/12.0f,
		2.0f/12.0f,
	},
	{
		11.0f/12.0f,
		1.0f/12.0f,
	},
	{
		1.0f,
		0.0f,
	},
};


void CalcHairCoordinate(HAIR_WORK *work)
{
	int n_ex_models;
	int i;

	int n_parents=work->n_parents;
	FMATRIX *tmat=work->tmat[0 /* n_parents-1 */ ];

#ifdef DEBUG_MODE
	// printf("Recalc HairEvm\n");
#endif

    if(tmat){
		for(i=0;i<n_parents;i++){
			if(work->tmat[i]==NULL){
				int parent=work->parent[i];

				vu0_Ldv0(&(work->t[i]));
				vu0_Ldm0(&(work->wmat[parent]));

				vu0_Mulv0m0v0();

				vu0_Stm0(&(work->wmat[i]));
				vu0_Stv0(&(work->x[i]));
				vu0_Stv0((FVECTOR *)&(work->wmat[i].m[3][0]));
			}
			else{
				vu0_Ldm0(work->tmat[i]);
				vu0_Ldm1(&(work->root));

				vu0_Mulm2m0m1();

				vu0_Stm2(&(work->wmat[i]));
				fpu_CopyVector(&(work->x[i]),(FVECTOR *)&(work->wmat[i].m[3][0]));
			}
			fpu_ClearVector(&(work->v[i]));
		}
    }
    else{
		fpu_CopyMatrix(&(work->wmat[0]),&(work->root));
		fpu_CopyVector(&(work->x[0]),(FVECTOR *)&(work->wmat[0].m[3][0]));
		fpu_ClearVector(&(work->v[0]));
    }


	n_ex_models=work->n_ex_models;

	for(i=n_parents;i<n_ex_models;i++){
		int parent=work->parent[i];

		vu0_Ldv0(&(work->t[i]));
		vu0_Ldm0(&(work->wmat[parent]));

		vu0_Mulv0m0v0();

		vu0_Stm0(&(work->wmat[i]));
		vu0_Stv0(&(work->x[i]));
		vu0_Stv0((FVECTOR *)&(work->wmat[i].m[3][0]));
		fpu_ClearVector(&(work->v[i]));
	}

	if(work->target!=NULL){
		fpu_CopyVector(&(work->target_pos),
					   (FVECTOR *)&(work->target->objs->world.m[3][0]));
	}
}

void hair_target_callback(TARGET *ofs,TARGET *def,void *param)
{
    FVECTOR *v=(FVECTOR *)param;
	
	/* powerを持たないターゲットが当たる場合もある
	   M.Sonoyama 修正 2000/07/14 */
	if ( ofs->power != NULL ) {
		fpu_AddVectors(v,v,&(ofs->power->force));
		// fpu_AddVectors(v+1,v+1,&(ofs->power->force));

		GM_SeSetMode(SD_A_FLAG_L01,(FVECTOR *)&(def->world.m[3][0]),GM_SEMODE_BOMB);

#ifdef DEBUG_MODE
		printf("HairEvm : Call Se Sound\n");
#endif
	}
    def->damaged=0;
}
