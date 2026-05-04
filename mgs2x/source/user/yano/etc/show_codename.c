//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   show_codename.c
   称号表示
   
   2002/07/30	Y.Yano
   $Id: show_codename.c,v 1.3 2002/11/23 12:46:56 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <string.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"

#include "BP_TrophySystem.h"
#include "BP_TrophyLogicMGS2.h"

/*----------------------------------------------------------*/
enum {
	PHASE_START = 0,
	PHASE_SHOW_CODENAME,
	PHASE_FADEIN_PICTURE,
	PHASE_SHOW_PICTURE,
	PHASE_END,
};

#define ANI_PLUS_ALPHA 50
#define ANI_FONT_W (48*TARGET_ASPECT_X)
#define ANI_FONT_H 48
#define STR_PLUS_ALPHA 50
#define STR_FONT_W (18*TARGET_ASPECT_X)
#define STR_FONT_H 12
#define STR_FONT_U 18.0f
#define STR_FONT_V 12.0f

#define MAX_STRLEN 20

#define NODE_FONT_R			120
#define NODE_FONT_G			20
#define NODE_FONT_B			20
	
/*----------------------------------------------------------*/

typedef struct {
	int t_p;	/* 0:t-p 1:t 2:p タンカー／プラント */
	int level;	/* ゲームレベル */
	int radoff;	/* レーダー使わずクリア */
	int alert;	/* 危険モード回数 */
	int kill;	/* 殺傷人数 */
	int ration;	/* レーション食べた数 */
	int time;	/* プレイタイム(ｍ単位) */
	int conti;	/* コンティニュー回数 */
	int save;	/* セーブ回数 */
	int s_item;	/* 特殊アイテム使用 */
	int shoot;	/* 発砲数 */
	int damage;	/* ダメージ本数(最大体力の半分が単位) */
	int m_kill;	/* メカ破壊数 */
	int sea_louse;	/* フナムシついてクリア */
	int clearing;	/* クリアリング回避回数 */
} RESULT;

typedef	struct _work {
	GV_ACT_EX	actor ;
	int			handle_2d;	/* L2Dのハンドル */
	SPR_OBJ     *str_spr[ MAX_STRLEN ]; /* 文字のスプライト */
	SPR_OBJ     *ani_spr;				/* 画像のスプライト */

	int			animal;		/* 動物通し番号 */
	int			a_strlen;	/* 動物の名前の文字数 */
	int 		str_num;	/* 表示中の文字の順番 */

	int 		str_alp;	/* 文字のアルファ */
	int 		ani_alp;	/* 画像のアルファ */
	int 		ani_cnt;	/* クリアコードを表示するタイミングを計る用のカウンター */

	int 		anim_num;	/* 何番目のテクスチャーを表示しているか */
	int 		anim_cnt;	/* 何番目のテクスチャーを表示しているか */

	RESULT		result;		/* クリア結果まとめたもの */

	int 		phase;
	
} Work;

typedef struct _anime {
	int time[4];
	float u[4], v[4];
} Animal_info;


static const Animal_info anime_time[] = {
	/* {time0,time1,time2,time3, u0, u1, u2, u3, v0,v1,v2,v3} */
	{8,8,8,8, 0,32,64,96, 0,0,0,0 },
	{36,8,27,8, 128,160,192,224, 0,0,0,0 },
	{168,6,72,4, 256,288,320,352, 0,0,0,0 },
	{10,10,10,10, 384,416,448,480, 0,0,0,0 },
	{144,3,3,3, 0,32,64,96, 32,32,32,32 },
	{11,11,11,11, 128,160,192,224, 32,32,32,32 },
	{11,9,11,9, 256,288,320,352, 32,32,32,32 },
	{6,6,6,6, 384,416,448,480, 32,32,32,32 },
	{4,4,4,4, 0,32,64,96, 64,64,64,64 },
	{7,7,7,7, 128,160,192,224, 64,64,64,64 },
	{12,9,12,9, 256,288,320,352, 64,64,64,64 },
	{10,9,12,9, 384,416,448,480, 64,64,64,64 },
	{13,11,11,11, 0,32,64,96, 96,96,96,96 },
	{9,9,9,9, 128,160,192,224, 96,96,96,96 },
	{6,6,6,6, 256,288,320,352, 96,96,96,96 },
	{6,6,6,6, 384,416,448,480, 96,96,96,96 },
	{11,9,11,9, 0,32,64,96, 128,128,128,128 },
	{6,6,6,6, 128,160,192,224, 128,128,128,128 },
	{8,8,8,8, 256,288,320,352, 128,128,128,128 },
	{10,10,10,10, 384,416,448,480, 128,128,128,128 },
	{108,3,3,3, 0,32,64,96, 160,160,160,160 },
	{108,4,4,4, 128,160,192,224, 160,160,160,160 },
	{9,9,9,9, 256,288,320,352, 160,160,160,160 },
	{9,9,9,9, 384,416,448,480, 160,160,160,160 },
	{6,6,6,6, 0,32,64,96, 192,192,192,192 },
	{6,6,6,6, 128,160,192,224, 192,192,192,192 },
	{11,9,11,9, 256,288,320,352, 192,192,192,192 },
	{156,1,1,1, 384,416,448,480, 192,192,192,192 },
	{144,6,7,6, 0,32,64,96, 224,224,224,224 },
	{5,5,5,5, 128,160,192,224, 224,224,224,224 },
	{7,7,7,7, 256,288,320,352, 224,224,224,224 },
	{120,4,4,4, 384,416,448,480, 224,224,224,224 },
	{4,4,4,4, 0,32,64,96, 256,256,256,256 },
	{6,6,6,6, 128,160,192,224, 256,256,256,256 },
	{9,9,9,9, 256,288,320,352, 256,256,256,256 },
	{8,8,8,8, 384,416,448,480, 256,256,256,256 },
	{144,4,4,4, 0,32,64,96, 288,288,288,288 },
	{10,9,12,9, 128,160,192,224, 288,288,288,288 },
	{11,11,11,11, 256,288,320,352, 288,288,288,288 },
	{108,1,2,2, 384,416,448,480, 288,288,288,288 },
	{10,10,10,10, 0,32,64,96, 320,320,320,320 },
	{168,2,22,3, 128,160,192,224, 320,320,320,320 },
	{6,6,6,6, 256,288,320,352, 320,320,320,320 },
	{10,10,10,10, 384,416,448,480, 320,320,320,320 },
	{168,6,120,4, 0,32,64,96, 352,352,352,352 },
	{11,10,11,10, 128,160,192,224, 352,352,352,352 },
	{13,11,11,11, 256,288,320,352, 352,352,352,352 },
	{9,9,9,9, 384,416,448,480, 352,352,352,352 },
	{120,4,108,3, 0,32,64,96, 384,384,384,384 },
	{7,7,7,7, 128,160,192,224, 384,384,384,384 },
	{168,4,4,4, 256,288,320,352, 384,384,384,384 },
	{156,1,1,1, 384,416,448,480, 384,384,384,384 },
	{168,3,18,4, 0,32,64,96, 416,416,416,416 },
	{84,1,1,1, 128,160,192,224, 416,416,416,416 },
};

enum {
	PIG = 0,
	KOALA,
	ZEBRA,
	PIRANHA,
	FLYING_SQUIRREL,
	SPIDER,//5
	COMODO_DRAGON,
	PUMA,
	PIGEON,
	HOUND,
	RABBIT,//10
	unknown_1,
	ELEPHANT,
	JAWS,
	unknown_2,
	CENTIPEDE,//15
	ALLIGATOR,
	PANTHER,
	HAWK,
	FOX,
	MOUSE,//20
	CAPYBARA,
	HIPPOPOTAMUS,
	SHARK,
	BAT,
	TARANTULA,//25
	IGUANA,
	LEOPARD,
	FALCON,
	DOBERMAN,
	COW,//30
	GAZELLE,
	SEA_LOUCE,
	SWALLOW,
	CHICKEN,
	CAT,//35
	DEER,
	SLOTH,
	WHALE,
	HYENA,
	MONGOOSE,//40
	TASMANIAN_DEVIL,
	FLYING_FOX,
	JACKAL,
	OSTRICH,
	GIANT_PANDA,//45
	MAMMOTH,
	ORCA,
	NIGHT_OWL,
	SCORPION,
	CROCODILE,//50
	JAGUAR,
	EAGLE,
	BIG_BOSS,//53
};
 
static const char *animal_string[] = {
	"PIG", //0
	"KOALA",
	"ZEBRA",
	"PIRANHA",
	"FLYING SQUIRREL",
	"SPIDER",//5
	"KOMODO DRAGON",
	"PUMA",
	"PIGEON",
	"HOUND",
	"RABBIT",//10
	"_unknown_",
	"ELEPHANT",
	"JAWS",
	"_unknown_",
	"CENTIPEDE",//15
	"ALLIGATOR",
	"PANTHER",
	"HAWK",
	"FOX",
	"MOUSE",//20
	"CAPYBARA",
	"HIPPOPOTAMUS",
	"SHARK",
	"BAT",
	"TARANTULA",//25
	"IGUANA",
	"LEOPARD",
	"FALCON",
	"DOBERMAN",
	"COW",//30
	"GAZELLE",
	"SEA LOUCE",
	"SWALLOW",
	"CHICKEN",
	"CAT",//35
	"DEER",
	"SLOTH",
	"WHALE",
	"HYENA",
	"MONGOOSE",//40
	"TASMANIAN DEVIL",
	"FLYING FOX",
	"JACKAL",
	"OSTRICH",
	"GIANT PANDA",//45
	"MAMMOTH",
	"ORCA",
	"NIGHT OWL",
	"SCORPION",
	"CROCODILE",//50
	"JAGUAR",
	"EAGLE",
	"BIG BOSS", //53
};

static const char animal_strlen[] = {
	3,
	5,
	5,
	7,
	15,
	6,
	13,
	4,
	6,
	5,
	6,
	9,
	8,
	4,
	9,
	9,
	9,
	7,
	4,
	3,
	5,
	8,
	12,
	5,
	3,
	9,
	6,
	7,
	6,
	8,
	3,
	7,
	9,
	7,
	7,
	3,
	4,
	5,
	5,
	5,
	8,
	15,
	10,
	6,
	7,
	11,
	7,
	4,
	9,
	8,
	9,
	6,
	5,
	8,
};

/* ゲームレベル */
enum {
	ES = 10,/* v_e */
	NM = 20,/* easy */
	HD = 30,/* normal */
	VH = 40,/* v_h */
	EX = 50,/* ex */
};

static int JudgeTitle( RESULT *res ){
	/* プレイ結果から称号を求める */
	/* 優先順位の高いものから判定していく */
	
	/* 優秀成績1 */
	if(res->t_p    == 0 	&&
	   res->radoff == 1 	&&
	   res->alert  <= 3 	&&
	   res->kill   == 0 	&&
	   res->ration == 0 	&&
	   res->time   <= 180 	&&
	   res->conti  == 0 	&&
	   res->save   <= 8 	&&
	   res->s_item == 0 	&&
	   res->shoot  <= 700 	&&
	   res->damage <= 10		){
		switch( res->level ){
		  case NM:
			return 9;/* HOUND */
		  case HD:
			return 29;/* DOBERMAN */
		  case VH:
			return 19;/* FOX */
		  case EX:
			return 53;/* BIG BOSS */
		}
	}
	/* 優秀成績2 */
	if(res->t_p    == 0 	&&
	   res->alert  <= 3 	&&
	   res->kill   == 0 	&&
	   res->ration == 0 	&&
	   res->time   <= 180 	&&
	   res->conti  == 0 	&&
	   res->save   <= 16 	&&
	   res->s_item == 0 		){
		switch( res->level ){
		  case HD:
			return 9;/* HOUND */
		  case VH:
			return 29;/* DOBERMAN */
		  case EX:
			return 19;/* FOX */
		}
	}
	/* 優秀成績3 */
	if(res->t_p    == 0 	&&
	   res->alert  <= 4 	&&
	   res->kill   == 0 	&&
	   res->ration <= 3 	&&
	   res->time   <= 195 	&&
	   res->conti  == 0 	&&
	   res->s_item == 0 		){
		switch( res->level ){
		  case VH:
			return 9;/* HOUND */
		  case EX:
			return 29;/* DOBERMAN */
		}
	}
	/* 優秀成績4 */
	if(res->t_p    == 0 	&&
	   res->alert  <= 5 	&&
	   res->kill   == 0 	&&
	   res->time   <= 210 	&&
	   res->conti  == 0 	&&
	   res->s_item == 0 		){
		switch( res->level ){
		  case EX:
			return 9;/* HOUND */
		}
	}
	/* だめだめ成績 */
	if(res->t_p    == 0 	&&
	   res->alert  >= 250 	&&
	   res->kill   >= 250 	&&
	   res->ration >= 31 	&&
	   res->time   >= 1800 	&&
	   res->conti  >= 60 	&&
	   res->save   >= 100 		){
		switch( res->level ){
		  case ES:
			return 34;/* CHICKEN */
		  case NM:
			return 34;/* CHICKEN */
		  case HD:
			return 20;/* MOUSE */
		  case VH:
			return 10;/* RABBIT */
		  case EX:
			return 44;/* OSTRICH */
		}
	}
	/* フナムシクリア */
	if(res->t_p != 1 &&
	   res->sea_louse == 1 ){
		return 32;/* SEA LOUCE */
	}
	/* 見つからずクリア */
	if( res->t_p == 1 ){
		if( res->alert == 0 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return 4;/* FLYING SQUIRREL */
			  case HD:
				return 24;/* BAT */
			  case VH:
				return 42;/* FLYING FOX */
			  case EX:
				return 48;/* NIGHT OWL */
			}
		}
	} else {
		if( res->alert <= 3 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return 4;/* FLYING SQUIRREL */
			  case HD:
				return 24;/* BAT */
			  case VH:
				return 42;/* FLYING FOX */
			  case EX:
				return 48;/* NIGHT OWL */
			}
		}
	}		
	/* 殺さずクリア */
	if( res->kill == 0 ){
		return PIGEON;
	}
	/* プレイ時間短い */
	switch( res->t_p ){
	  case 0:
		if( res->time <= 180 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return SWALLOW;
			  case HD:
				return FALCON;
			  case VH:
				return HAWK;
			  case EX:
				return EAGLE;
			}
		}
		break;
	  case 1:
		if( res->time <= 18 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return SWALLOW;
			  case HD:
				return FALCON;
			  case VH:
				return HAWK;
			  case EX:
				return EAGLE;
			}
		}
		break;
	  case 2:
		if( res->time <= 165 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return SWALLOW;
			  case HD:
				return FALCON;
			  case VH:
				return HAWK;
			  case EX:
				return EAGLE;
			}
		}
		break;
	}
	/* クリアリング回避しまくり */
	switch( res->t_p ){
	  case 0:
		if( res->clearing >= 150 ){
			return GAZELLE;
		}
		break;
	  case 1:
		if( res->clearing >= 50 ){
			return GAZELLE;
		}
		break;
	  case 2:
		if( res->clearing >= 100 ){
			return GAZELLE;
		}
		break;
	}
	/* 見ミつかりまくり */
	switch( res->t_p ){
	  case 0:
		if( res->alert >= 250 ){
			return COW    ;
		}
		break;
	  case 1:
		if( res->alert  >= 50 ){
			return COW    ;
		}
		break;
	  case 2:
		if( res->alert   >= 200 ){
			return COW    ;
		}
		break;
	}
	/* 殺コロしまくり */
	switch( res->t_p ){
	  case 0:
		if( res->kill >= 250 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return PIRANHA;
			  case HD:
				return SHARK;
			  case VH:
				return JAWS;
			  case EX:
				return ORCA;
			}
		}
		break;
	  case 1:
		if( res->kill >= 50 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return PIRANHA;
			  case HD:
				return SHARK;
			  case VH:
				return JAWS;
			  case EX:
				return ORCA;
			}
		}
		break;
	  case 2:
		if( res->kill >= 200 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return PIRANHA;
			  case HD:
				return SHARK;
			  case VH:
				return JAWS;
			  case EX:
				return ORCA;
			}
		}
		break;
	}
	/* レーション食クいまくり */
	if( res->ration >= 31 ){
		switch( res->level ){
		  case ES:
		  case NM:
			return PIG;
		  case HD:
			return ELEPHANT;
		  case VH:
			return MAMMOTH;
		  case EX:
			return WHALE;
		}
	}
	/* プレイ時間ジカン長ナガい */
	switch( res->t_p ){
	  case 0:
		if( res->time >= 1800 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return KOALA;
			  case HD:
				return CAPYBARA;
			  case VH:
				return SLOTH;
			  case EX:
				return GIANT_PANDA;
			}
		}
		break;
	  case 1:
		if( res->time >= 300 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return KOALA;
			  case HD:
				return CAPYBARA;
			  case VH:
				return SLOTH;
			  case EX:
				return GIANT_PANDA;
			}
		}
		break;
	  case 2:
		if( res->time >= 1500 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return KOALA;
			  case HD:
				return CAPYBARA;
			  case VH:
				return SLOTH;
			  case EX:
				return GIANT_PANDA;
			}
		}
		break;
	}
	/* セーブいっぱい */
	switch( res->t_p ){
	  case 0:
		if( res->save >= 100 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return CAT;
			  case HD:
				return DEER;
			  case VH:
				return ZEBRA;
			  case EX:
				return HIPPOPOTAMUS;
			}
		}
		break;
	  case 1:
		if( res->save >= 25 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return CAT;
			  case HD:
				return DEER;
			  case VH:
				return ZEBRA;
			  case EX:
				return HIPPOPOTAMUS;
			}
		}
		break;
	  case 2:
		if( res->save >= 75 ){
			switch( res->level ){
			  case ES:
			  case NM:
				return CAT;
			  case HD:
				return DEER;
			  case VH:
				return ZEBRA;
			  case EX:
				return HIPPOPOTAMUS;
			}
		}
		break;
	}
	/* 一般成績用 */
	{
		int a, b;
		switch( res->t_p ){
		  case 0:
			if( res->conti <= 40 ){
				if( res->kill <= 70 ){
					a = 0;
				} else {
					a = 1;
				}
			} else {
				if( res->kill <= 70 ){
					a = 2;
				} else {
					a = 3;
				}
			}
			break;
		  case 1:
			if( res->conti <= 10 ){
				if( res->kill <= 15 ){
					a = 0;
				} else {
					a = 1;
				}
			} else {
				if( res->kill <= 15 ){
					a = 2;
				} else {
					a = 3;
				}
			}
			break;
		  case 2:
			if( res->conti <= 30 ){
				if( res->kill <= 60 ){
					a = 0;
				} else {
					a = 1;
				}
			} else {
				if( res->kill <= 60 ){
					a = 2;
				} else {
					a = 3;
				}
			}
			break;
		  default:
			return -1;
		}

		switch( res->t_p ){
		  case 0:
			if( res->alert <= 20 ){
				b = 0;
			} else if( res->alert <= 50 ){
				b = 1;
			} else if( res->alert <= 80 ){
				b = 2;
			} else {
				b = 3;
			}
			break;
		  case 1:
			if( res->alert <= 5 ){
				b = 0;
			} else if( res->alert <= 15 ){
				b = 1;
			} else if( res->alert <= 30 ){
				b = 2;
			} else {
				b = 3;
			}
			break;
		  case 2:
			if( res->alert <= 15 ){
				b = 0;
			} else if( res->alert <= 40 ){
				b = 1;
			} else if( res->alert <= 70 ){
				b = 2;
			} else {
				b = 3;
			}
			break;
		}
		
		switch( a ){
		  case 0:
			switch( b ){
			  case 0:
				return SCORPION;
				break;
			  case 1:
				return JAGUAR;
				break;
			  case 2:
				return JACKAL;
				break;
			  case 3:
				return IGUANA;
				break;
			}
			break;
		  case 1:
			switch( b ){
			  case 0:
				return TARANTULA;
				break;
			  case 1:
				return PANTHER;
				break;
			  case 2:
				return TASMANIAN_DEVIL;
				break;
			  case 3:
				return CROCODILE;
				break;
			}
			break;
		  case 2:
			switch( b ){
			  case 0:
				return CENTIPEDE;
				break;
			  case 1:
				return LEOPARD;
				break;
			  case 2:
				return MONGOOSE;
				break;
			  case 3:
				return COMODO_DRAGON;
				break;
			}
			break;
		  case 3:
			switch( b ){
			  case 0:
				return SPIDER;
				break;
			  case 1:
				return PUMA;
				break;
			  case 2:
				return HYENA;
				break;
			  case 3:
				return ALLIGATOR;
				break;
			}
			break;
		}
	}

	return -1;
}

static void SetFontUV( SPR_OBJ *spr, int animal, int num )
{
	char ch;
	int  no;

	printf("animal [%d]\n",animal);
	ch = animal_string[ animal ][ num ];
	printf("-----%c\n",ch);
	
	no = ch - 'A';/* a:0番,b:1番,... */
	if( ch == ' ' ){
		/* スペースの時は26番 */
		no = 26;
	}
	if( no < 0 || no > 26 ){
		printf("称号名が変です。");
		no = 26;
		//ASSERT( 0 );
	}

	/* UV値代入 *//* Vはそのまま */
	spr->ex_hd.tex.u = spr->ex_hd.tex.pu + SPR_FIXED( no * STR_FONT_U );
	spr->ex_hd.tex.w = SPR_FIXED( STR_FONT_U );
	spr->ex_hd.tex.h = SPR_FIXED( STR_FONT_V );
}

static void AnimalAnimation( Work *work, int animal )
{
	/* アニメーションを再生 */
	SPR_OBJ *spr = work->ani_spr;
	
	if( work->anim_cnt > anime_time[animal].time[work->anim_num] ){
		work->anim_cnt = 0;
		work->anim_num ++;
		if( work->anim_num >= 4 ){
			work->anim_num = 0;
		}
	}

	/* 0.5テクセル内側に */
	spr->ex_hd.tex.u = SPR_FIXED( anime_time[animal].u[work->anim_num] + 0.5f );
	spr->ex_hd.tex.v = SPR_FIXED( anime_time[animal].v[work->anim_num] + 0.5f );
	spr->ex_hd.tex.w = SPR_FIXED( 31.0f );
	spr->ex_hd.tex.h = SPR_FIXED( 31.0f );

	work->anim_cnt ++;

	return ;
}

static int ResultToCodename( Work *work )
{
	/* クリア結果をまとめる */
	int codename;
	RESULT *res = &work->result;

	/* 何編をクリアしたのか */
	if( (GM_TitleMenuStatus & TITLE_MENU_STORY_SEL_MASK) == TITLE_MENU_STORY_SEL_TANDP ){
		res->t_p = 0;
	} else if( (GM_TitleMenuStatus & TITLE_MENU_STORY_SEL_MASK) == TITLE_MENU_STORY_SEL_TANKER ){
		res->t_p = 1;
	} else {
		res->t_p = 2;
	}
	printf("t_p::%d\n",res->t_p);
	/* ゲームレベル */
	res->level = GM_GameLevel;
	if( res->level == 60 ){
		res->level = 50;/* EEはEXと同等 */
	}
	printf("level::%d\n",res->level);
	/* レーダー使わずクリア */
	if( GM_ClearCodeFlag & GM_CLEAR_RADAR_USED ){
		res->radoff = 0;
	} else {
		res->radoff = 1;
	}
	printf("radoff::%d\n",res->radoff);
	/* 危険モード回数 */
	res->alert = GM_AlertCount;
	printf("alert::%d\n",res->alert);
	/* 殺傷人数 */
	res->kill = GM_KillCount;
	printf("kill::%d\n",res->kill);
	/* レーション食べた数 */
	res->ration = GM_RationUseCount;
	printf("ration::%d\n",res->ration);
	/* プレイタイム(分単位) */
	res->time = (int)(GM_PlayTime+(59*DIRECT_TICK(60))) / 60 / DIRECT_TICK( 60 );/* 秒以下は切り上げ */
	printf("time::%d\n",res->time);
	/* コンティニュー回数 */
	res->conti = GM_ContinueCount;
	printf("conti::%d\n",res->conti);
	/* セーブ回数 */
	res->save = GM_SaveCount;
	printf("save::%d\n",res->save);
	/* 特殊アイテム使用 */
	if( GM_ClearCodeFlag & GM_CLEAR_SPECIAL_ITEM_USED ){
		res->s_item = 1;
	} else {
		res->s_item = 0;
	}
	printf("s_item::%d\n",res->s_item);
	/* 発砲数 */
	res->shoot = GM_ShootCount;
	printf("shoot::%d\n",res->shoot);
	/* ダメージ本数(最大体力が単位で49捨50入) */
	res->damage = (int) ((GM_DamageCount+50) / GM_VitalityMax);
	printf("damage::%d\n",res->damage);
	/* メカ破壊数 */
	res->m_kill = GM_MecaKillCount;
	printf("m_kill::%d\n",res->m_kill);
	/* フナムシついてクリア */
	if( GM_ShipwormFlag ){
		res->sea_louse = 1;
	} else {
		res->sea_louse = 0;
	}
	printf("sea::%d\n",res->sea_louse);
	/* クリアリング回避回数 */
	res->clearing = GM_ClearingCount;
	printf("clearing::%d\n",res->clearing);
	

	codename = JudgeTitle( res );
	if( codename < 0 ){
		printf("show_codename.c----Title judge error!\n");
		ASSERT( 0 );
	}
#if 0
////
	codename = TASMANIAN_DEVIL;
	codename = FLYING_SQUIRREL;
	codename = BIG_BOSS;
////
#endif
	return codename;/* 動物番号を返す */
}


/*-----------------------------------------------------------------*/

static void Act( Work *work )
{
#if 0
	//////アニメーションテスト//////
	  if( GV_PadDataDirect[0].press & PAD_R1 ){
		  work->animal ++;
		  if( work->animal > 53 ){
			  work->animal = 0;
		  }
	  } else if( GV_PadDataDirect[0].press & PAD_L1 ){
		  work->animal --;
		  if( work->animal < 0 ){
			  work->animal = 53;
		  }
	  }

#endif

	switch( work->phase ){
	  case PHASE_START:
		{
			int status;
			status = L2D_ActionStatus( work->handle_2d );
			if( status < 0 ) return ;
		}
		/* 文字、アニメ画像などの初期化 */
		/* 文字表示スプライト */
		{
			SPR_POS pos;
			
			{
				SPR_OBJ *nul, *spr, *font;
				int 	len = work->a_strlen;
				int 	i;
				
				/* l2dから座標情報だけもらう */
				nul = L2D_GetObject( work->handle_2d, GV_StrCode("NULL_a_z_font") );
				//pos2 = nul->sprite.pos;

				pos.x = 0.0f; pos.y = 0.0f;
				/* コピー用フォントspriteの元 */
				font = L2D_GetObject( work->handle_2d, GV_StrCode("a_z_font") );
				for( i = 0; i < len; i ++ ){
					spr = work->str_spr[ i ] = SPR_DuplicateTree( font );
					SPR_SetPosSprite( spr, &pos );
					pos.x += (float)STR_FONT_W;	/* 座標を一文字分ずつずらしていく */
					SPR_SetColorSprite( spr, NODE_FONT_R, NODE_FONT_G, NODE_FONT_B, 0 );
					SPR_SetSizeSprite( spr, STR_FONT_W, STR_FONT_H );
					/* 文字からフォントテクスチャのｕｖを割り出す */
					SetFontUV( spr, work->animal, i );
					SPR_SHOW( spr );
				}
			}
			/* アニメーション領域スプライト */
			{
				SPR_OBJ *nul, *spr;
				int 	tex_handle;

#if 0
				/* 称号文字の右に出す */
				nul = L2D_GetObject( work->handle_2d, GV_StrCode("NULL_a_z_font") );
				spr = work->ani_spr = SPR_Create_2D_Object( SP_SPRITE, DG_CHANL_MENU, nul );
				pos.x += (float)STR_FONT_W;
#else
				/* 文字と左揃い */
				nul = L2D_GetObject( work->handle_2d, GV_StrCode("NULL_anime") );
				spr = work->ani_spr = SPR_Create_2D_Object( SP_SPRITE, DG_CHANL_MENU, nul );
				pos.x = 0.0f;
				pos.y = 0.0f;
#endif
				SPR_SetPosSprite( spr, &pos );

				SPR_SetColorSprite( spr, 128, 128, 128, 0 );
				SPR_SetSizeSprite( spr, ANI_FONT_W, ANI_FONT_H );
				SPR_SetPriority( spr, 5 ); /* プライオリティ設定 */
				SPR_ALPHA( spr, SCE_GS_SET_ALPHA(0,1,0,1,0x80));
				SPR_SET_FLAGS( spr, SPR_FLAG_ALPHA );

				tex_handle = SPR_LoadTexture( GV_StrCode("animal") );
				SPR_ObjSetTexture( spr, GV_StrCode("animal_alp_ovl"), tex_handle );
				SPR_SHOW( spr );
			}
		}
		work->phase = PHASE_SHOW_CODENAME;
		break;
	  case PHASE_SHOW_CODENAME:
		/* コードネーム文字の表示 */
		{
			int alp;
			int len      = work->a_strlen;			
			int str_num  = work->str_num;/* 何番目の文字を表示しているか */
			SPR_OBJ *spr = work->str_spr[ str_num ];
			
			work->str_alp += STR_PLUS_ALPHA;
			if( work->str_alp >= 128 ){
				alp = 128;
				work->str_alp = 0;
				work->str_num ++;
				GM_SdSet( SD_S_GO_TYPE1 );
				if( work->str_num >= len ){
					work->phase = PHASE_FADEIN_PICTURE;
				}
			} else {
				alp = work->str_alp;
			}
			SPR_SetColorSprite( spr, NODE_FONT_R, NODE_FONT_G, NODE_FONT_B, alp );
		}

		break;
	  case PHASE_FADEIN_PICTURE:
		/* 画像アニメーションのフェードイン */
		{
			int alp;
			SPR_OBJ *spr = work->ani_spr;
			
			work->ani_alp += ANI_PLUS_ALPHA;
			if( work->ani_alp >= 128 ){
				work->ani_alp = 128;
				work->phase = PHASE_SHOW_PICTURE;
				GM_SdSet( SD_S_R_CURSOR );
			}
			alp = work->ani_alp;
			SPR_SetColorSprite( spr, 128, 128, 128, alp );
		}
		/* 画像のアニメーション再生 */
		AnimalAnimation( work, work->animal );
		break;
	  case PHASE_SHOW_PICTURE:
		/* 画像のアニメーション再生&タイミング */
		/* 画像のアニメーション再生 */
		AnimalAnimation( work, work->animal );
		/* クリアコード再生までのタイミングを待って、シグナルを送る */
		work->ani_cnt ++ ;
		if( work->ani_cnt > DIRECT_TICK(30) ){/* 時間はいま適当*/
			int signal = 0x01;
			int value = 0;
			GV_CallParentSignalFunc( work, signal, value );
		}
		break;
	  case PHASE_END:
		/* 不使用 */
		break;
	  default:
		printf("show_codename.c---- phase error!\n");
		break;
	}

#if 0
	/* L2Dアクション再生 */
	if( work->action != 0 ){
		int status;
		status = L2D_ActionStatus( work->handle_2d );
		if( status == L2D_STAT_ACK ){
			L2D_EvokeAction( work->handle_2d , work->action );
			work->action = 0;
		}
	}
#endif
}


static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d );
	}
	
}


/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	int	handle;
	int num;

	/* クリア結果から称号を決める */
	num = ResultToCodename( work );
	if( num < 0 ){
		printf("show_codename.c----Codename is not fixed.ERROR!\n");
		return -1;
	}
	work->animal = num;
	work->a_strlen = animal_strlen[ num ];

	work->handle_2d = -1;
	handle = L2D_LoadLayout( GV_StrCode("code_name"), DG_CHANL_MENU, 7, 0 );
	if ( handle < 0 ){
		printf("code_name.l2d----error!\n");
		return -1;
	}
	work->handle_2d = handle ;
	
	work->phase = PHASE_START;

	/* スタートボタンきかない */
	GM_SetGameStatus( STATE_PAUSE_DISABLE );

	/* DefaultAction再生 */
	{
		int status;
		status = L2D_ActionStatus( work->handle_2d );
		if( status == L2D_STAT_ACK ){
			int res;
			res = L2D_EvokeAction( work->handle_2d , GV_StrCode("DefaultAction") );
			if( res < 0 ){
				return -1;
			}
		}

	}

   if( work->result.t_p == 0 )
   {
      //BP - unlock "beat game" trophy only if tanker and plant chapters were played
      BP_TrophySystem_UnlockTrophy( kTRP_BeatGame );
      //In case user never saves, need to indicate that they are allowed to unlock trophies
      //in casting theater and boss survival once they return to the front end.
      bp_trophy_set_tanker_cleared_by_user();
      bp_trophy_set_plant_cleared_by_user();
   }
   else if( work->result.t_p == 1 )
   {
      bp_trophy_set_tanker_cleared_by_user();
   }
   else
   {
      bp_trophy_set_plant_cleared_by_user();
   }

   if( GM_KillCount == 0 )
   {
      //AndyO: Removed in MGS_2_and_3_Trophy_Info_v2_110527_rev.xls
      //BP_TrophySystem_UnlockTrophy( kTRP_BeatGameNoKills );
   }

   if( !bp_linkvars.mSkippedACinematic )
   {
      //AndyO: Removed in MGS_2_and_3_Trophy_Info_v2_110527_rev.xls
      //BP_TrophySystem_UnlockTrophy( kTRP_WatchAllCinematics );
   }

   if( GM_AlertCount <= 3 && work->result.t_p == 0 )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_BeatGameNoAlert );
   }

	return 0 ;
}

static int GetOptionValue( Work *work )
{
	/* シナリオ呼び出し用 */

	return 0;
}
/*----------------------------------------------------------------*/

void *NewShowCodeName( void )
{
	Work		*work ;

	printf("CODENAME SHOW!!\n");
	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

void *NewShowAnimalTitleScn( int name, int map )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;

	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	if ( GetOptionValue( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

/* 称号の境界チェック用のシナリオのデバック関数 */
void COMClearcodeCheck( int name, int map )
{
	int res;
	if( GCL_GetOption( 'a' ) ){
		res = GCL_GetNextInt();
		if( res == 0 ){
			GM_TitleMenuStatus &= ~TITLE_MENU_STORY_SEL_MASK;
			GM_TitleMenuStatus |= TITLE_MENU_STORY_SEL_TANDP;
		} else if( res == 1 ){
			GM_TitleMenuStatus &= ~TITLE_MENU_STORY_SEL_MASK;
			GM_TitleMenuStatus |= TITLE_MENU_STORY_SEL_TANKER;
		} else {
			GM_TitleMenuStatus &= ~TITLE_MENU_STORY_SEL_MASK;
			GM_TitleMenuStatus |= TITLE_MENU_STORY_SEL_PLANT;
		}
	}
	if( GCL_GetOption( 'b' ) ){
		res = GCL_GetNextInt();
		GM_GameLevel = res;
	}
	if( GCL_GetOption( 'c' ) ){
		res = GCL_GetNextInt();
		if( res == 1 ){
			GM_ClearCodeFlag &= ~GM_CLEAR_RADAR_USED;
		} else {
			GM_ClearCodeFlag |= GM_CLEAR_RADAR_USED;
		}
	}
	if( GCL_GetOption( 'd' ) ){
		res = GCL_GetNextInt();
		GM_AlertCount = res;
	}
	if( GCL_GetOption( 'e' ) ){
		res = GCL_GetNextInt();
		GM_KillCount = res;
	}
	if( GCL_GetOption( 'f' ) ){
		res = GCL_GetNextInt();
		GM_RationUseCount = res;
	}
	if( GCL_GetOption( 'g' ) ){
		res = GCL_GetNextInt();
		GM_PlayTime = res;
	}
	if( GCL_GetOption( 'h' ) ){
		res = GCL_GetNextInt();
		GM_ContinueCount = res;
	}
	if( GCL_GetOption( 'i' ) ){
		res = GCL_GetNextInt();
		GM_SaveCount = res;
	}
	if( GCL_GetOption( 'j' ) ){
		res = GCL_GetNextInt();
		if( res == 1 ){
			GM_ClearCodeFlag |= GM_CLEAR_SPECIAL_ITEM_USED;
		} else {
			GM_ClearCodeFlag &= ~GM_CLEAR_SPECIAL_ITEM_USED;
		}
	}
	if( GCL_GetOption( 'k' ) ){
		res = GCL_GetNextInt();
		GM_ShootCount = res;
	}
	if( GCL_GetOption( 'l' ) ){
		res = GCL_GetNextInt();
		GM_DamageCount = res;
	}
	if( GCL_GetOption( 'm' ) ){
		res = GCL_GetNextInt();
		GM_MecaKillCount = res;
	}
	if( GCL_GetOption( 'n' ) ){
		res = GCL_GetNextInt();
		GM_ShipwormFlag = res;
	}
	if( GCL_GetOption( 'o' ) ){
		res = GCL_GetNextInt();
		GM_ClearingCount = res;
	}

	return;
}

