//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    英語スペシャルのメニュー
    sp_menu_eng.c
    
	2000/11/29 T.Shibata

	$Id: sp_menu_page.c,v 1.1.1.3 2002/11/19 11:48:45 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"font.h"
#include	"gameheader.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"

#include 	"sprite_2d.h"
#include	"../util/ts_util.h"

#include "mode/menu/xtextscn.h"

#include "sp_menu_shared.h"

//aaa_sp_menu
#define TRI_CODE		(11286891)//(2637787)
#define TEX_CODE_BG		(14376023)
#define TEX_CODE_CTRLER		(12082711)

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))
#define SET_COLOR_2DPRIM3(_prim,_a,_col) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | (_col)))

#define FADEIN_TIME			(8)
#define CATEGORY_MOVE_TIME	(60)

#define FADEOUT_TIME	(16)

#define	TRN2DFIG_Y(_y)	(((_y)*384.0f)/448.0f)

#define CHANL_NUM	(0)

#define REPEAT_FIRST	20
#define REPEAT_NEXT	3
#define REPEAT_SECOND	(REPEAT_FIRST+30)

#define N_MAXMENU0		(8)//(15)
#define N_MAXMENU1		(8)//(8)
#define N_MAXMENU2		(8)//(9)
#define N_MAXMENU_NO	(0)

#define	N_ALLMENU		(N_MAXMENU0+N_MAXMENU1+N_MAXMENU2)

#define	N_MAX_EXP_PAGE	(4)

/* 説明分テキストを初期化する */
extern void MENU_ClearTextTexture( void *work );
/* 説明文を表示する */
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2,
								int u1, int v1, int u2, int v2, int col );
/* テキストテクスチャ管理デーモン */
extern void *NewTextScreenControl( void );


#define BP_TO_WIDESCREEN_OFFSETX (88.0f)
#define BP_TO_WIDESCREEN_OFFSETY (0.0f)


typedef struct {
    int status;
    int auto_status;
    int press;

    int repeat_count;
    int repeat_next_count;
} PadData;


typedef	struct sp_menu_page_Work
{
	GV_ACT_EX		actor;
	
	void			*exp_work;
	SPR_OBJ   		*bground;	//大ボス

	SPR_OBJ   		*sub_parent0;
	SPR_OBJ   		*sub_parent1;

	//sub_parent0
	SPR_OBJ   		*title0;
	SPR_OBJ   		*title_base;

	SPR_OBJ   		*cursor;
	SPR_OBJ   		*line[2];
	SPR_OBJ   		*category[4];
	//SPR_OBJ   	*menu[15];

	//sub_parent1
	SPR_OBJ   		*title1;
	SPR_OBJ   		*title_sp;
	SPR_OBJ   		*mv_base;
	SPR_OBJ   		*menu_name;
	SPR_OBJ   		*next_page;
	SPR_OBJ   		*exit;
	SPR_OBJ   		*ctrler;
	SPR_OBJ   		*cursor1;
//	SPR_OBJ   		*botton[10];
	SPR_OBJ   		*line_strip[4];

	int				tri_id;
	int				name;
	int				see_flags;

	int				proc_id[N_ALLMENU];
	char			*exp_str[N_ALLMENU][N_MAX_EXP_PAGE];
	char			*name_str[N_ALLMENU];
	char			*name_str2[N_ALLMENU];
	short			key_data[N_ALLMENU];

	int				end_id;
	int				force_end_flag;

	short			step;
	short			s_step;
	short			now_category;
	short			timer;

	short			now_menu[3];
	short			menu_to_num;

	int				now_key_data;
	float			category_y;
	float			menu_size;
	int				exp_str_len;
	int				title_str_len;
	short			now_page;
	short			pre_page;
	float			name_pos0;
	float			name_pos1;
	SPR_POS			cursor_pos;
	SPR_POS			cursor_size;

	SPR_POS			line_pos[2];

	SPR_POS			line_strip0_pos[2][2];
	SPR_POS			line_strip1_pos[2][4];

	PadData			pad;
} Work;

#define		STR_FLAGS_RETURN		(0x0001)	//改行フラグ

enum {
	STEP_FADEIN = 0,
	STEP_VIS_CATE,
	STEP_VIS_LINE,
	STEP_VIS_MENU,

	STEP_CATEGORY_WAIT,
	STEP_CATEGORY_CURSOR,

	STEP_CATEGORY_MENU,

	STEP_MENU_WAIT,
	STEP_MENU_CURSOR,

	STEP_MENU_CATEGORY,

	STEP_MENU_MOVIE,

	STEP_MOVIE_PRE,
	STEP_MOVIE_WAIT,
	STEP_MOVIE_NEXT,
	STEP_MOVIE_CLOSE,

	STEP_MOVIE_MENU,

	STEP_FADEOUT,
	STEP_KILL,
};

static int Category_Tex_Code[7][4] = {
	{//日本語
		9588390,
		9653926,
		9719462,
		13144248,
	},
	{//英語
		9588390,
		9653926,
		9719462,
		13144248,
	},
	{//フランス
		9702605,
		9702733,
		9702861,
		13144248,
	},
	{//ドイツ
		4656333,
		4656461,
		4656589,
		13144248,
	},
	{//イタリア
		16125133,
		16125261,
		16125389,
		13144248,
	},
	{//スペイン
		7670989,
		7671117,
		7671245,
		13144248,
	},
	{//韓国
		9588390,
		9653926,
		9719462,
		13144248,
	},
};

static SPR_POS Category_Size[4] = {
	{ 0.75f*136.0f, TRN2DFIG_Y(48.0f) },
	{ 0.75f*136.0f, TRN2DFIG_Y(48.0f) },
	{ 0.75f*136.0f, TRN2DFIG_Y(48.0f) },
	{  0.75f*76.0f, TRN2DFIG_Y(24.0f) },
};

static float Category_Cor_Size_Y[4] = {
	TRN2DFIG_Y(34.0f),
	TRN2DFIG_Y(34.0f),
	TRN2DFIG_Y(34.0f),
	TRN2DFIG_Y(24.0f),
};

#define CATEGORY_POS_X	(-20.0f)

static float Category_Pos_Y[4] = {
	TRN2DFIG_Y(72.0f),
	TRN2DFIG_Y(138.0f),
	TRN2DFIG_Y(204.0f),
	TRN2DFIG_Y(386.0f),
};

#if 0
static int Menu_Tex_Code[3] = {
	15487653,
	15553189,
	15618725,
};
#endif

static float MenuSize0_X[15] = {
	84.0f,
	120.0f,
	56.0f,
	40.0f,
	184.0f,
	184.0f,
	92.0f,
	74.0f,
	92.0f,
	148.0f,
	110.0f,
	54.0f,
	112.0f,
	74.0f,
	164.0f,
};

static float MenuSize1_X[8] = {
	108.0f,
	92.0f,
	200.0f,
	128.0f,
	162.0f,
	148.0f,
	186.0f,
	186.0f,
};

static float MenuSize2_X[9] = {
	110.0f,
	144.0f,
	128.0f,
	74.0f,
	182.0f,
	130.0f,
	150.0f,
	150.0f,
	92.0f,
};

#define MENU_SIZE_Y	(TRN2DFIG_Y(27.0f))


static float *MenuSize_X[3] = {
	MenuSize0_X,
	MenuSize1_X,
	MenuSize2_X,
};
#if 0
static float MenuTexSize_W[3] = {
	184.0f,
	200.0f,
	182.0f,
};
#endif

#define		LINE_POS01		(120)   //(192.0f-16.0f)

#define 	MENU_POS_X		(BP_TO_WIDESCREEN_OFFSETX+LINE_POS01+8.0f)
#define		PUT_TXT_X1		(512+256)//+96)//(512)
#define		PUT_TXT_X0		((int)MENU_POS_X)
#define 	MENU_SIZE_X		(0.75f*(PUT_TXT_X1-PUT_TXT_X0))


#define 	TXT_SIZE_X	(0.75f*((float)(PUT_TXT_X1 - PUT_TXT_X0)))

static float InitMenuPos_Y[3] = {
	TRN2DFIG_Y(76.0f),//TRN2DFIG_Y(22.0f),
	TRN2DFIG_Y(76.0f),
	TRN2DFIG_Y(76.0f),
};



static SPR_POS Line0_Pos[4][2] = {
	{ { 81.0f, TRN2DFIG_Y( 88.0f) },{ LINE_POS01, TRN2DFIG_Y( 88.0f) } },
	{ { 81.0f, TRN2DFIG_Y(154.0f) },{ LINE_POS01, TRN2DFIG_Y(154.0f) } },
	{ { 81.0f, TRN2DFIG_Y(220.0f) },{ LINE_POS01, TRN2DFIG_Y(220.0f) } },
	{ { 0.0f, TRN2DFIG_Y(0.0f) },{ 0.0f, 0.0f } },
};

static SPR_POS Line1_Pos[4][2] = {
//	{ { LINE_POS01, TRN2DFIG_Y(19.0f) },{ LINE_POS01, TRN2DFIG_Y(421.0f) } },
	{ { LINE_POS01, TRN2DFIG_Y(73.0f) },{ LINE_POS01, TRN2DFIG_Y(287.0f) } },
	{ { LINE_POS01, TRN2DFIG_Y(73.0f) },{ LINE_POS01, TRN2DFIG_Y(287.0f) } },
//	{ { LINE_POS01, TRN2DFIG_Y(73.0f) },{ LINE_POS01, TRN2DFIG_Y(314.0f) } },
	{ { LINE_POS01, TRN2DFIG_Y(73.0f) },{ LINE_POS01, TRN2DFIG_Y(287.0f) } },
	{ { 0.0f, TRN2DFIG_Y(0.0f) },{ 0.0f, 0.0f } },
};


#define	TEX_CODE_CURSOR			(15175597)

#define CURSOR_ADD_SIZE_X	(28.0f)
#define CURSOR_ADD_SIZE_Y	(TRN2DFIG_Y(20.0f))
#define CURSOR_MENU_SIZE_Y	(TRN2DFIG_Y(18.0f))

#define CURSOR_ADD_SIZE_X1	(28.0f)
#define CURSOR_ADD_SIZE_Y1	(TRN2DFIG_Y(16.0f))

#define CURSOR_SHIFT_X		(-14.0f)
#define CURSOR_SHIFT_Y		(TRN2DFIG_Y(-10.0f))

#define CURSOR_SHIFT_X1		(-(BP_TO_WIDESCREEN_OFFSETX+8))
//#define CURSOR_SHIFT_X1		(-14.0f)
#define CURSOR_SHIFT_Y1		(TRN2DFIG_Y(-6.0f))

//#define CURSOR_ADD_POS_X	(16.0f)
//#define CURSOR_ADD_POS_Y	(TRN2DFIG_Y(16.0f))

//#define CURSOR_COLOR		(0x30b08040)
//#define CURSOR_COLOR		(0x3081a381)
#define CURSOR_COLOR		(0x30adc1a3)

#define LINE0_COLOR_R		(163)
#define LINE0_COLOR_G		(193)
#define LINE0_COLOR_B		(173)
#define LINE0_COLOR_A		(128)

#define LINE1_COLOR_R		(163)
#define LINE1_COLOR_G		(193)
#define LINE1_COLOR_B		(173)
#define LINE1_COLOR_A		(128)

#define MENU_COLOR0			(0x80a0a0a0)	//見てない
#define MENU_COLOR1			(0x80606060)	//みた
#define MENU_COLOR2			(0x80803030)	//無い

#define CATEGORY_COLOR		(0x80a0a0a0)
#define CTRL_COLOR			(0x00808080)
#define EXIT_COLOR			(0x00808080)
#define TITLE_COLOR			(0x40808080)

#define MENU_NAME_COLOR		(0x00808080)

#define TITLE_BASE_COLOR	(0x00adc1a3)
static SPR_POS InitLineStrip0[2] = {
	{ 256.0f, 192.0f },
	{ 256.0f, 192.0f+TRN2DFIG_Y(25.0f)}
};

static SPR_POS InitLineStrip1[4] = {
	{ 156.0f, 192.0f+TRN2DFIG_Y(24.0f)-TRN2DFIG_Y(7.0f)},
	{ 156.0f, 192.0f+TRN2DFIG_Y(24.0f)},
	{ 156.0f, 192.0f+TRN2DFIG_Y(24.0f)},
	{ 156.0f, 192.0f+TRN2DFIG_Y(24.0f)-TRN2DFIG_Y(8.0f)},
};
#if 0
#define BOTTON_COLOR	(0x002060a0)

static SPR_POS BottonPos[10] = {
	{ 190.0f, TRN2DFIG_Y(116.0f) },
	{ 175.0f, TRN2DFIG_Y(132.0f) },
	{ 175.0f, TRN2DFIG_Y(100.0f) },
	{ 160.0f, TRN2DFIG_Y(116.0f) },

	{  92.0f, TRN2DFIG_Y(142.0f) },
	{ 140.0f, TRN2DFIG_Y(142.0f) },

	{ 65.0f, TRN2DFIG_Y(71.0f) },
	{ 69.0f, TRN2DFIG_Y(58.0f) },
	{ 166.0f, TRN2DFIG_Y(71.0f) },
	{ 166.0f, TRN2DFIG_Y(58.0f) },
};

static SPR_POS BottonSize[10] = {
	{ 20.0f, TRN2DFIG_Y(16.0f) },
	{ 20.0f, TRN2DFIG_Y(16.0f) },
	{ 20.0f, TRN2DFIG_Y(16.0f) },
	{ 20.0f, TRN2DFIG_Y(16.0f) },

	{ 32.0f, TRN2DFIG_Y(28.0f) },
	{ 32.0f, TRN2DFIG_Y(28.0f) },

	{ 32.0f, TRN2DFIG_Y(14.0f) },
	{ 28.0f, TRN2DFIG_Y(14.0f) },
	{ 32.0f, TRN2DFIG_Y(14.0f) },
	{ 28.0f, TRN2DFIG_Y(14.0f) },
};

static int BottonTexCode[10] = {
	2252258,
	2252258,
	2252258,
	2252258,
	5070340,
	5070340,
	15031384,
	15096920,
	10837081,
	10902617,
};
#endif

static int Title_Sp_TexCode[7][3] = {
	{//日本語
		4560860,
		4626396,
		4691932,
	},
	{//英語
		4560860,
		4626396,
		4691932,
	},
	{//フランス語
		3073650,
		3073778,
		3073906,
	},
	{//ドイツ語
		14804593,
		14804721,
		14804849,
	},
	{//イタリア語
		9496178,
		9496306,
		9496434,
	},
	{//スペイン語
		1042034,
		1042162,
		1042290,
	},
	{//ハングリアン語
		4560860,
		4626396,
		4691932,
	},
};

#if 0
static FVECTOR Title_Sp_TexData[3] = {
	{ 212.0f, TRN2DFIG_Y(29.0f),  90.0f, TRN2DFIG_Y(10.0f)},
	{ 189.0f, TRN2DFIG_Y(29.0f), 134.0f, TRN2DFIG_Y(10.0f)},
	{ 208.0f, TRN2DFIG_Y(29.0f),  96.0f, TRN2DFIG_Y(10.0f)},
};
#endif


static int MaxMenuNum[] ={
	N_MAXMENU0,
	N_MAXMENU1,
	N_MAXMENU2,
	N_MAXMENU_NO,
};

static int Flag_Offset[] = {
	0,
	N_MAXMENU0,
	N_MAXMENU0+N_MAXMENU1,
	N_MAXMENU0+N_MAXMENU1+N_MAXMENU2,
};



/* ------------------------------------------------------------------------- */

static void PadKeyInit( Work *work )
{
    work->pad.status=work->pad.auto_status
		=work->pad.press=0;
    work->pad.repeat_count=0;
}

static void PadKeyAct( Work *work )
{
    work->pad.press=GV_PadDataDirect[0].press;

    if(GV_PadDataDirect[0].status && work->pad.status==GV_PadDataDirect[0].status){
		if(work->pad.repeat_count>work->pad.repeat_next_count){
			if(work->pad.repeat_next_count>REPEAT_SECOND){
				work->pad.repeat_count=work->pad.repeat_next_count;
			}
			else{
				work->pad.repeat_next_count+=REPEAT_NEXT;
			}
			work->pad.auto_status=work->pad.status;
		}
		else work->pad.auto_status=0;

		work->pad.repeat_count++;
    }
    else{
		work->pad.auto_status
			=work->pad.status=GV_PadDataDirect[0].status;
		work->pad.repeat_count=0;
		work->pad.repeat_next_count=REPEAT_FIRST;
    }
}

/* ------------------------------------------------------------------------- */

#if 0
static void SetSpriteUV( SPR_SPRITE *prim, int u, int v, int w, int h  )
{
	SPR_TEX		*tex = &prim->head.tex;
	int			tw,th,off_u,off_v;

	DG_GetTexelInfo( &tw, &th, &off_u, &off_v, tex->dgtex );
//	printf("off_u %d:off_v %d:size_w %d:size_h %d\n",off_u,off_v,tw,th);
//	printf("u %d:v %d:w %d:h %d\n",u,v,w,h);
	tex->u = ((off_u + u)<<4) + (1<<3);
	tex->v = ((off_v + v)<<4) + (1<<3);
	tex->w = (w<<4) - (1<<4);
	tex->h = (h<<4) - (1<<4);
}
#endif

/* ------------------------------------------------------------------------ */

static void PrintMenu( Work *work, int cate_num, int alpha, int mode )
{
	int i,color;
	float locate_y = InitMenuPos_Y[cate_num];

	for( i = 0; i < MaxMenuNum[cate_num]; i++ ){
		if(!(work->see_flags & 1 << (i+Flag_Offset[cate_num]))){
			color = 0x00606060;
		}else{
			color = 0x00303030;
		}

		if(mode && i==work->menu_to_num){
			color = 0x00606060;
		}
		MENU_PutTextScreen( work->exp_work,
							PUT_TXT_X0, (int)(locate_y - TRN2DFIG_Y(2.0f)),
							PUT_TXT_X1, (int)(locate_y + MENU_SIZE_Y),
							0, 1+(FONT_BUFFER_HEIGHT(1,0)+2)*i,
							1024, 1+(FONT_BUFFER_HEIGHT(1,0)+2)*(i+1), (alpha<<24)|color );

		locate_y += MENU_SIZE_Y;
	}
}

static void PrintMovieExp( Work *work, int alpha_name, int alpha_exp )
{
	float	ftemp0,ftemp1;//,ftemp2,ftemp3;
	
	ftemp0 = work->name_pos0 - 8.0f;//256.0f - 472.0f * (float)work->exp_str_len/640.0f / 2.0f + /*space*/2.0f;
	ftemp1 = ftemp0 + 472.0f + /*space*/2.0f;

	MENU_PutTextScreen( work->exp_work,
						(int)ftemp0, (int)(260.0f*384.0f/448.0f)-5,
						(int)ftemp1, (int)(388.0f*384.0f/448.0f)-5,
						0, 0,
						640, 160,
						(alpha_exp<<24)|0x00808080 );

	if( work->name_str2[(Flag_Offset[work->now_category]+work->now_menu[work->now_category])] ){
		
		MENU_PutTextScreen( work->exp_work,
							(int)work->name_pos0, (int)TRN2DFIG_Y(240.0f),
							(int)work->name_pos1, (int)TRN2DFIG_Y(240.0f+12.0f+2.0f),
							0, 160,
							FONT_BUFFER_WIDTH(24,0), 160 + FONT_BUFFER_HEIGHT(1,0)+2,
							(alpha_name<<24)|0x00606060 );
		
		MENU_PutTextScreen( work->exp_work,
							(int)work->name_pos0, (int)TRN2DFIG_Y(240.0f+12.0f+2.0f),
							(int)work->name_pos1, (int)TRN2DFIG_Y(240.0f+24.0f+2.0f),
							0, 160 + FONT_BUFFER_HEIGHT(1,0)+12,
							FONT_BUFFER_WIDTH(24,0), 160 + FONT_BUFFER_HEIGHT(1,0)+12 + FONT_BUFFER_HEIGHT(1,0)+2,
							(alpha_name<<24)|0x00606060 );

		
	}else{
		MENU_PutTextScreen( work->exp_work,
							(int)work->name_pos0, (int)(240.0f*384.0f/448.0f)-8,
							(int)work->name_pos1, (int)TRN2DFIG_Y(240.0f+24.0f+2.0f)-8,
							0, 160,
							FONT_BUFFER_WIDTH(24,0), 160 + FONT_BUFFER_HEIGHT(1,0)+2,
							(alpha_name<<24)|0x00606060 );
	}
}

static void CreateMovieExp( Work *work )
{
	MENU_ClearTextTexture( work->exp_work );
	// 説明分テキストを展開する
	work->exp_str_len = MENU_CreateTextTexture( work->exp_work,
												16, TUTORIAL_TEXT_Y, 
												640-32+1, TUTORIAL_TEXT_HEIGHT,
												0, 12, 0, work->exp_str[(Flag_Offset[work->now_category]+
																	  work->now_menu[work->now_category])][work->now_page] );

	// 題名
	if( work->name_str2[(Flag_Offset[work->now_category]+work->now_menu[work->now_category])] ){
		
		work->title_str_len = MENU_CreateTextTexture( work->exp_work,
													  16, TUTORIAL_HEADER_Y,
													  FONT_BUFFER_WIDTH(24,0), FONT_BUFFER_HEIGHT(4,0),
													  0, 0, 0, 
													  work->name_str2[(Flag_Offset[work->now_category]+
																	  work->now_menu[work->now_category])] );
	}else{
		work->title_str_len = MENU_CreateTextTexture( work->exp_work,
													  16, 160+2,
													  FONT_BUFFER_WIDTH(24,0), FONT_BUFFER_HEIGHT(1,0),
													  0, 0, 0,
													  work->name_str[(Flag_Offset[work->now_category]+
																	  work->now_menu[work->now_category])] );
	}

}


/* ------------------------------------------------------------------------ */
static void SpMenu_FadeIn( Work *work )
{

	SET_COLOR_2DPRIM3(work->bground,work->timer<<2,0x00808080);

	if(++work->timer > 32){
		work->step++;
		work->timer = 0;
		SET_COLOR_2DPRIM(work->title0,0x80acc37a);
		SET_COLOR_2DPRIM(work->title_base,0x40000000|TITLE_BASE_COLOR);
	}
}


#define CATE_MOVE_Y	(TRN2DFIG_Y(14.0f))
static void SpMenu_VisCate( Work *work )
{
	int i;
	SPR_POS	locate;

	work->category_y += CATE_MOVE_Y*1.5f;
	if(work->category_y > Category_Pos_Y[work->now_category]){

		locate.x = CATEGORY_POS_X;
		locate.y = Category_Pos_Y[work->now_category];
		SPR_SetPosSprite(work->category[work->now_category], &locate );

		if(++work->now_category >= 4){
			work->s_step = 0;
			work->now_category = 0;
			work->category_y = 0.0f;
			work->step++;
			return;
		}
	}

	locate.x = CATEGORY_POS_X;
	locate.y = work->category_y;

	for( i = work->now_category; i < 4; i++ ){
		SPR_SetPosSprite(work->category[work->now_category], &locate );
	}
}

#define LINE_MOVE_X	(8.0f)
#define LINE_MOVE_Y	(TRN2DFIG_Y(18.0f))

static void SpMenu_VisLine( Work *work )
{

	if(work->s_step == 0){
		work->line_pos[0].x += LINE_MOVE_X*2.0f;
		work->line_pos[1].x += LINE_MOVE_X*2.0f;

		if( work->line_pos[0].x > Line0_Pos[0][1].x ){
			work->line_pos[0].x = Line0_Pos[0][1].x;
			work->line_pos[1].x = Line0_Pos[0][1].x;
			work->s_step++;
 		}
		SPR_SetPosLine(work->line[0],&Line0_Pos[0][0], &work->line_pos[0]);
		SPR_SetPosLine(work->line[1],&work->line_pos[0], &work->line_pos[0]);
	}else{
		int check = 2;
		
		work->line_pos[0].y -= LINE_MOVE_Y*2.0f;
		work->line_pos[1].y += LINE_MOVE_Y*2.0f;

		if( work->line_pos[0].y < Line1_Pos[0][0].y){
			work->line_pos[0].y = Line1_Pos[0][0].y;
			check--;
		}
		if( work->line_pos[1].y > Line1_Pos[0][1].y){
			work->line_pos[1].y = Line1_Pos[0][1].y;
			check--;
		}
		
		SPR_SetPosLine(work->line[1],&work->line_pos[0], &work->line_pos[1]);

		if(!check){
			work->s_step = 0;
			work->step++;
		}
	}
	
}

static void SpMenu_VisMenu( Work *work )
{
	int i,color;
	float locate_y = InitMenuPos_Y[0];

	work->menu_size += 32.0f;
	if(work->menu_size > MENU_SIZE_X){
		work->menu_size = MENU_SIZE_X;
		
		SPR_SHOW(work->cursor);
		work->step++;
	}

	for( i = 0; i < N_MAXMENU0; i++ ){
		if(!(work->see_flags & 1 << i)){
			color = 0x80606060;
		}else{
			color = 0x80303030;
		}

		MENU_PutTextScreen( work->exp_work,
							PUT_TXT_X0, (int)(locate_y - TRN2DFIG_Y(2.0f)),
						    (int)((float)PUT_TXT_X0 + work->menu_size), (int)(locate_y + MENU_SIZE_Y),
							0, 1+(FONT_BUFFER_HEIGHT(1,0)+2)*i,
							1024, 1+(FONT_BUFFER_HEIGHT(1,0)+2)*(i+1), color );

		locate_y += MENU_SIZE_Y;
	}	
}

static void SpMenu_Category( Work *work )
{
	int check = 0;
	PadKeyAct( work );

	if(work->pad.auto_status & PAD_U){
		if(work->now_category){
			work->menu_to_num = work->now_category - 1;
			work->step = STEP_CATEGORY_CURSOR;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);
			check = 1;
		}
	}else if(work->pad.auto_status & PAD_D){
		if(work->now_category < 3){
			work->menu_to_num = work->now_category + 1;
			work->step = STEP_CATEGORY_CURSOR;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);
			check = 1;
		}
	}else if(work->pad.auto_status & (PAD_R|PAD_OK)){
		if(work->now_category!=3){
			work->step = STEP_CATEGORY_MENU;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);
		}else{
			if(!(work->pad.auto_status & PAD_R)){
				work->step = STEP_FADEOUT;
				GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
			}
		}
	}else if(work->pad.auto_status & PAD_CANCEL){
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
		work->step = STEP_FADEOUT;
	}

	PrintMenu( work, work->now_category, 128, 0 );

	SPR_SetPosLine(work->line[0],&Line0_Pos[work->menu_to_num][0], &Line0_Pos[work->menu_to_num][1]);
	SPR_SetPosLine(work->line[1],&Line1_Pos[work->menu_to_num][0], &Line1_Pos[work->menu_to_num][1]);
}

#define COTE_COR_MOVE_Y	(TRN2DFIG_Y(16.5f))
#define COTE_COR_MOVE_X	(26.0f*1.5f)

static void SpMenu_Category_Cursor( Work *work )
{
	SPR_POS			pos,size;
	int				check = 3;

	if(++work->timer > 4){
		PrintMenu( work, work->menu_to_num, 128, 0 );
	}else if(work->timer == 2){
		int	i;
		MENU_ClearTextTexture( work->exp_work );
		for( i = 0; i < MaxMenuNum[work->menu_to_num]; i++ ){
			float wide;
			wide = (float)MENU_CreateTextTexture( work->exp_work, 2, 2 + (FONT_BUFFER_HEIGHT(1,0)+2)*i,
												  1024-4, FONT_BUFFER_HEIGHT(1,0)+2,
												  0, 0, 0, work->name_str[i+Flag_Offset[work->menu_to_num]] );

         MenuSize_X[work->menu_to_num][i] = (wide * (TXT_SIZE_X) / 1024.0f)*1.3f;
		}
	}

	if( work->cursor_pos.y > Category_Pos_Y[work->menu_to_num] ){
		work->cursor_pos.y -= COTE_COR_MOVE_Y;
		if(work->cursor_pos.y < Category_Pos_Y[work->menu_to_num]){
			work->cursor_pos.y = Category_Pos_Y[work->menu_to_num];
			check--;
		}
	}else{
		work->cursor_pos.y += COTE_COR_MOVE_Y;
		if(work->cursor_pos.y > Category_Pos_Y[work->menu_to_num]){
			work->cursor_pos.y = Category_Pos_Y[work->menu_to_num];
			check--;
		}
	}

	if( work->cursor_size.x > Category_Size[work->menu_to_num].x ){
		work->cursor_size.x -= COTE_COR_MOVE_X;
		if( work->cursor_size.x < Category_Size[work->menu_to_num].x ){
			work->cursor_size.x = Category_Size[work->menu_to_num].x;
			check--;
		}
	}else{
		work->cursor_size.x += COTE_COR_MOVE_X;
		if( work->cursor_size.x > Category_Size[work->menu_to_num].x ){
			work->cursor_size.x = Category_Size[work->menu_to_num].x;
			check--;
		}
	}

	if( work->cursor_size.y > Category_Cor_Size_Y[work->menu_to_num] ){
		work->cursor_size.y -= COTE_COR_MOVE_Y;
		if( work->cursor_size.y < Category_Cor_Size_Y[work->menu_to_num] ){
			work->cursor_size.y = Category_Cor_Size_Y[work->menu_to_num];
			check--;
		}
	}else{
		work->cursor_size.y += COTE_COR_MOVE_Y;
		if( work->cursor_size.y > Category_Cor_Size_Y[work->menu_to_num] ){
			work->cursor_size.y = Category_Cor_Size_Y[work->menu_to_num];
			check--;
		}
	}

	pos.x = work->cursor_pos.x + CURSOR_SHIFT_X;
	pos.y = work->cursor_pos.y + CURSOR_SHIFT_Y;
	size.x = work->cursor_size.x + CURSOR_ADD_SIZE_X;
	size.y = work->cursor_size.y + CURSOR_ADD_SIZE_Y;


	SPR_SetPosSprite(work->cursor, &pos );
	SPR_SetSizeSprite(work->cursor, size.x, size.y );

	if(!check){
		work->timer = 0;
		work->now_category = work->menu_to_num;
		work->step = STEP_CATEGORY_WAIT;
	}
}

static void SpMenu_Category_Menu( Work *work )
{
	SPR_POS			pos,size;
	int				check;
	float			to_size_x = MenuSize_X[work->now_category][work->now_menu[work->now_category]];
	float			to_pos_y = InitMenuPos_Y[work->now_category] + MENU_SIZE_Y * work->now_menu[work->now_category];

	switch( work->s_step ){
	case 0:
		check = 3;
		if( work->cursor_size.x > to_size_x ){
			work->cursor_size.x -= COTE_COR_MOVE_X*1.5f;
			if( work->cursor_size.x < to_size_x ){
				work->cursor_size.x = to_size_x;
				check--;
			}
		}else{
			work->cursor_size.x += COTE_COR_MOVE_X*1.5f;
			if( work->cursor_size.x > to_size_x ){
				work->cursor_size.x = to_size_x;
				check--;
			}
		}

		work->cursor_size.y -= COTE_COR_MOVE_Y*1.5f;
		if( work->cursor_size.y < CURSOR_MENU_SIZE_Y ){
			work->cursor_size.y = CURSOR_MENU_SIZE_Y;
			check--;
		}
		
		work->cursor_pos.x += COTE_COR_MOVE_X*4.0f;
		if(work->cursor_pos.x > MENU_POS_X){
			work->cursor_pos.x = MENU_POS_X;
			check--;
		}
		
		if(!check){
			work->s_step++;
		}
		PrintMenu( work, work->now_category, 128, 0 );
		break;
	case 1:
		
		if( work->cursor_pos.y > to_pos_y ){
			work->cursor_pos.y -= COTE_COR_MOVE_Y*2.0f;
			if(work->cursor_pos.y < to_pos_y){
				work->cursor_pos.y = to_pos_y;
				work->s_step++;
			}
		}else{
			work->cursor_pos.y += COTE_COR_MOVE_Y*2.0f;
			if(work->cursor_pos.y > to_pos_y){
				work->cursor_pos.y = to_pos_y;
				work->s_step++;
			}
		}
		PrintMenu( work, work->now_category, 128, 0 );
		break;
	case 2:
		PrintMenu( work, work->now_category, 128, 0 );

		work->menu_to_num = work->now_menu[work->now_category];

//		for(i=0;i<MaxMenuNum[work->now_category];i++){
//			if(i==work->menu_to_num || !(work->see_flags & 1 << (i+Flag_Offset[work->now_category]))){
//				//SET_COLOR_2DPRIM(work->menu[i],MENU_COLOR0);
//			}else{
//				//SET_COLOR_2DPRIM(work->menu[i],MENU_COLOR1);
//			}
//		}

		work->s_step = 0;
		work->step = STEP_MENU_WAIT;
		

		break;
	default:
		break;
	}

	pos.x = work->cursor_pos.x + CURSOR_SHIFT_X1;
	pos.y = work->cursor_pos.y + CURSOR_SHIFT_Y1;
	size.x = work->cursor_size.x + CURSOR_ADD_SIZE_X1;
	size.y = work->cursor_size.y + CURSOR_ADD_SIZE_Y1;

	SPR_SetPosSprite(work->cursor, &pos );
	SPR_SetSizeSprite(work->cursor, size.x, size.y );
}

static void SpMenu_Menu( Work *work )
{
	PadKeyAct( work );

	if(work->pad.auto_status & PAD_U){
		if(work->now_menu[work->now_category]){
			work->menu_to_num = work->now_menu[work->now_category] - 1;
			work->step = STEP_MENU_CURSOR;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);

		}
	}else if(work->pad.auto_status & PAD_D){
		if(work->now_menu[work->now_category] < MaxMenuNum[work->now_category]-1){
			work->menu_to_num = work->now_menu[work->now_category] + 1;
			work->step = STEP_MENU_CURSOR;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);

		}
	}else if(work->pad.auto_status & PAD_OK){
		
		work->step = STEP_MENU_MOVIE;
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);		


	}else if(work->pad.auto_status & (PAD_L|PAD_CANCEL)){
		work->step = STEP_MENU_CATEGORY;
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_V_CANS02);
	}
	PrintMenu( work, work->now_category, 128, 1 );
}

static void SpMenu_Menu_Cursor( Work *work )
{
	SPR_POS			pos,size;
	float			to_size_x = MenuSize_X[work->now_category][work->menu_to_num];
	float			to_pos_y = InitMenuPos_Y[work->now_category] + MENU_SIZE_Y * work->menu_to_num;
	int				check = 2;

	if( work->cursor_pos.y > to_pos_y ){
		work->cursor_pos.y -= COTE_COR_MOVE_Y;
		if(work->cursor_pos.y < to_pos_y){
			work->cursor_pos.y = to_pos_y;
			check--;
		}
	}else{
		work->cursor_pos.y += COTE_COR_MOVE_Y;
		if(work->cursor_pos.y > to_pos_y){
			work->cursor_pos.y = to_pos_y;
			check--;;
		}
	}

	if( work->cursor_size.x > to_size_x ){
		work->cursor_size.x -= COTE_COR_MOVE_X*3.0f;
		if( work->cursor_size.x < to_size_x ){
			work->cursor_size.x = to_size_x;
			check--;
		}
	}else{
		work->cursor_size.x += COTE_COR_MOVE_X*3.0f;
		if( work->cursor_size.x > to_size_x ){
			work->cursor_size.x = to_size_x;
			check--;
		}
	}
	
	pos.x = work->cursor_pos.x + CURSOR_SHIFT_X1;
	pos.y = work->cursor_pos.y + CURSOR_SHIFT_Y1;
	size.x = work->cursor_size.x + CURSOR_ADD_SIZE_X1;
	size.y = work->cursor_size.y + CURSOR_ADD_SIZE_Y1;

	SPR_SetPosSprite(work->cursor, &pos );
	SPR_SetSizeSprite(work->cursor, size.x, size.y );

	PrintMenu( work, work->now_category, 128, 1 );

	if(!check){
		work->now_menu[work->now_category] = work->menu_to_num;
		work->step = STEP_MENU_WAIT;
	}

}

static void SpMenu_Menu_Category( Work *work )
{
	SPR_POS			pos,size;
	int				check;

	switch( work->s_step ){
	case 0:
		check = 3;

		if( work->cursor_pos.y > Category_Pos_Y[work->now_category] ){
			work->cursor_pos.y -= COTE_COR_MOVE_Y*3.0f;
			if(work->cursor_pos.y < Category_Pos_Y[work->now_category]){
				work->cursor_pos.y = Category_Pos_Y[work->now_category];
				check--;
			}
		}else{
			work->cursor_pos.y += COTE_COR_MOVE_Y*3.0f;
			if(work->cursor_pos.y > Category_Pos_Y[work->now_category]){
				work->cursor_pos.y = Category_Pos_Y[work->now_category];
				check--;
			}
		}

		if( work->cursor_size.x > Category_Size[work->now_category].x ){
			work->cursor_size.x -= COTE_COR_MOVE_X*1.5f;
			if( work->cursor_size.x < Category_Size[work->now_category].x ){
				work->cursor_size.x = Category_Size[work->now_category].x;
				check--;
			}
		}else{
			work->cursor_size.x += COTE_COR_MOVE_X*1.5f;
			if( work->cursor_size.x > Category_Size[work->now_category].x ){
				work->cursor_size.x = Category_Size[work->now_category].x;
				check--;
			}
		}

		work->cursor_size.y += COTE_COR_MOVE_Y*1.5f;
		if( work->cursor_size.y > Category_Cor_Size_Y[work->now_category] ){
			work->cursor_size.y = Category_Cor_Size_Y[work->now_category];
			check--;
		}
		
		if(!check){
			work->s_step++;
		}

		break;
	case 1:
	
		work->cursor_pos.x -= COTE_COR_MOVE_X*4.0f;
		if(work->cursor_pos.x < CATEGORY_POS_X){
			work->cursor_pos.x = CATEGORY_POS_X;
			work->s_step++;
		}
		break;
	case 2:

		work->s_step = 0;
		work->step = STEP_CATEGORY_WAIT;
		work->menu_to_num = work->now_category;

		break;
	default:
		break;
	}

	PrintMenu( work, work->now_category, 128, 0 );

	pos.x = work->cursor_pos.x + CURSOR_SHIFT_X;
	pos.y = work->cursor_pos.y + CURSOR_SHIFT_Y;
	size.x = work->cursor_size.x + CURSOR_ADD_SIZE_X;
	size.y = work->cursor_size.y + CURSOR_ADD_SIZE_Y;

	SPR_SetPosSprite(work->cursor, &pos );
	SPR_SetSizeSprite(work->cursor, size.x, size.y );

}

static void SpMenu_Menu_Movie(Work *work)
{
	int i,alpha = 128 - (work->timer<<2);
	SPR_POS	pos;

	SET_COLOR_2DPRIM3(work->cursor,48*alpha/128,CURSOR_COLOR&0xffffff);
	SET_COLOR_2DPRIM3(work->title0,alpha,0x00acc37a);
	SET_COLOR_2DPRIM3(work->title_base,alpha>>1,TITLE_BASE_COLOR);


	for( i = 0; i < 2; i++ ){
		SPR_SetColorLine(work->line[i], 0, LINE0_COLOR_R,LINE0_COLOR_G,LINE0_COLOR_B,alpha);
		SPR_SetColorLine(work->line[i], 1, LINE0_COLOR_R,LINE0_COLOR_G,LINE0_COLOR_B,alpha);
	}

	for( i = 0; i < 4; i++ ){
		SET_COLOR_2DPRIM3(work->category[i],alpha,CATEGORY_COLOR&0xffffff);
	}
#if 0
	for( i = 0; i < MaxMenuNum[work->now_category]; i++ ){
		if(work->see_flags & 1 << (i+Flag_Offset[work->now_category])){
			//SET_COLOR_2DPRIM3(work->menu[i],alpha,MENU_COLOR1&0xffffff);
		}else{
			//SET_COLOR_2DPRIM3(work->menu[i],alpha,MENU_COLOR0&0xffffff);
		}
	}
#endif

	PrintMenu( work, work->now_category, alpha, 1 );
	if(++work->timer>32){
		work->step++;
		work->timer = 0;
		work->now_page = 0;
		SPR_HIDE(work->sub_parent0);
		SPR_SHOW(work->sub_parent1);


		work->line_strip0_pos[0][0] = InitLineStrip0[0];
		work->line_strip0_pos[0][1] = InitLineStrip0[1];
		work->line_strip0_pos[1][0] = InitLineStrip0[0];
		work->line_strip0_pos[1][1] = InitLineStrip0[1];

		work->line_strip1_pos[0][0] = InitLineStrip1[0];
		work->line_strip1_pos[0][1] = InitLineStrip1[1];
		work->line_strip1_pos[0][2] = InitLineStrip1[2];
		work->line_strip1_pos[0][3] = InitLineStrip1[3];

		work->line_strip1_pos[1][0] = InitLineStrip1[0];
		work->line_strip1_pos[1][1] = InitLineStrip1[1];
		work->line_strip1_pos[1][2] = InitLineStrip1[2];
		work->line_strip1_pos[1][3] = InitLineStrip1[3];

		work->see_flags |= 1<<(Flag_Offset[work->now_category]+work->now_menu[work->now_category]);
		work->now_key_data = work->key_data[Flag_Offset[work->now_category]+work->now_menu[work->now_category]];

		CreateMovieExp( work );
	/*
		MENU_ClearTextTexture( work->exp_work );
        // 説明分テキストを展開する
		work->exp_str_len = MENU_CreateTextTexture( work->exp_work,
													16, 4, 
													640-32+1, 160-8,
													0, 12, work->exp_str[(Flag_Offset[work->now_category]+
																		  work->now_menu[work->now_category])][0] );

        // 題名
		work->title_str_len = MENU_CreateTextTexture( work->exp_work,
													  16, 160+2,
													  FONT_BUFFER_WIDTH(24,0), FONT_BUFFER_HEIGHT(1,0),
													  0, 0, 
													  work->name_str[(Flag_Offset[work->now_category]+
													  work->now_menu[work->now_category])] );
	*/

		
		pos.x = 51.699997f;//256.0f - 472.0f * (float)work->exp_str_len/640.0f / 2.0f + /*space*/2.0f + 12.0f;
		//printf("---------------  %f\n",pos.x);
		pos.y = 204;
		work->name_pos0 = pos.x + 4.0f;//256.0f - 472.0f * (float)work->exp_str_len/640.0f / 2.0f + /*space*/2.0f + 16.0f;
		work->name_pos1 = work->name_pos0 + 512.0f;
		

		SPR_SetPosSprite(work->menu_name, &pos );
		SPR_ObjSetTexture(work->ctrler, Title_Sp_TexCode[GM_Language][work->now_category], work->tri_id);
//		SPR_SetPosSprite(work->ctrler, (SPR_POS*)&Title_Sp_TexData[work->now_category] );
//		SPR_SetSizeSprite(work->ctrler, Title_Sp_TexData[work->now_category].vz,
//						  Title_Sp_TexData[work->now_category].vw );

		SET_COLOR_2DPRIM3(work->title_sp, 0, 0);
		
		printf("HIDE\n");
	}
}

static void SpMenu_MoviePre(Work *work)
{
   float lowerYPos = 192;  //192
	SPR_POS	locate;
	SPR_POS	size;
//	int	i;
	float	ftemp0,ftemp1,ftemp2,ftemp3,ftemp4;

	switch(work->s_step){
	case 0:
		ftemp0 = (work->category_y += 24.0f);

		if(work->category_y > 236.0f){
			work->s_step++;
			work->category_y = 0.0f;
			ftemp0 = 236.0f;
		}
		ftemp1 = ftemp0 - 8.0f;

		work->line_strip0_pos[0][0].x = 256.0f - ftemp0;
		work->line_strip0_pos[0][1].x = 256.0f - ftemp0;
		work->line_strip0_pos[1][0].x = 256.0f + ftemp0;
		work->line_strip0_pos[1][1].x = 256.0f + ftemp0;

		work->line_strip1_pos[0][0].x = 256.0f - ftemp1;
		work->line_strip1_pos[0][1].x = 256.0f - ftemp1;
		work->line_strip1_pos[0][2].x = 256.0f + ftemp1;
		work->line_strip1_pos[0][3].x = 256.0f + ftemp1;

		work->line_strip1_pos[1][0].x = 256.0f - ftemp1;
		work->line_strip1_pos[1][1].x = 256.0f - ftemp1;
		work->line_strip1_pos[1][2].x = 256.0f + ftemp1;
		work->line_strip1_pos[1][3].x = 256.0f + ftemp1;

		locate.x = 256.0f - ftemp1;
		locate.y = 192.0f;

		size.x = ftemp1*2.0f;
		size.y = TRN2DFIG_Y(16.0f);

		SPR_SetPosSprite(work->title1, &locate );
		SPR_SetSizeSprite(work->title1, size.x, size.y );

		SPR_SetPosLineStrip(work->line_strip[0], 0, 2, work->line_strip0_pos[0]);
		SPR_SetPosLineStrip(work->line_strip[1], 0, 2, work->line_strip0_pos[1]);
		SPR_SetPosLineStrip(work->line_strip[2], 0, 4, work->line_strip1_pos[0]);
		SPR_SetPosLineStrip(work->line_strip[3], 0, 4, work->line_strip1_pos[1]);
		break;
	case 1:
		ftemp3 = ftemp2 = ftemp0 = (work->category_y += 24.0f);

		if( work->category_y > 198.0f){
			ftemp0 = 198.0f;
			ftemp2 = 198.0f;
			ftemp3 = 198.0f;
			work->s_step++;
			work->category_y = 0.0f;
		}


		ftemp4 = ftemp1 = ftemp0*172.0f/198.0f;

		if(ftemp2 > 224.0f - 216.0f+8.0f ){
			ftemp2 = 224.0f - 216.0f+8.0f;
		}

		if(ftemp3 > 224.0f - 216.0f){
			ftemp3 = 224.0f - 216.0f;
		}

		if(ftemp4 > 396.0f - 8.0f - 224.0f){
			ftemp4 = 396.0f - 8.0f - 224.0f;
		}

		work->line_strip0_pos[0][0].y = (lowerYPos - TRN2DFIG_Y(ftemp0))+8;
		work->line_strip0_pos[0][1].y = (lowerYPos + TRN2DFIG_Y(ftemp1))-8;
		work->line_strip0_pos[1][0].y = (lowerYPos - TRN2DFIG_Y(ftemp0))+8;
		work->line_strip0_pos[1][1].y = (lowerYPos + TRN2DFIG_Y(ftemp1))-8;

      lowerYPos = 182+1;
		work->line_strip1_pos[0][0].y = lowerYPos + TRN2DFIG_Y(ftemp2+1.0f);
		work->line_strip1_pos[0][1].y = lowerYPos + TRN2DFIG_Y(ftemp3);
		work->line_strip1_pos[0][2].y = lowerYPos + TRN2DFIG_Y(ftemp3);
		work->line_strip1_pos[0][3].y = lowerYPos + TRN2DFIG_Y(ftemp2);

      lowerYPos = 182;
		work->line_strip1_pos[1][0].y = lowerYPos + TRN2DFIG_Y(ftemp4-1.0f);
		work->line_strip1_pos[1][1].y = lowerYPos + TRN2DFIG_Y(ftemp1);
		work->line_strip1_pos[1][2].y = lowerYPos + TRN2DFIG_Y(ftemp1);
		work->line_strip1_pos[1][3].y = lowerYPos + TRN2DFIG_Y(ftemp4);

		locate.x = 28.0f;
		locate.y = (192 - TRN2DFIG_Y(ftemp0))+8;
		
		SPR_SetPosSprite(work->title1, &locate );

		SPR_SetPosLineStrip(work->line_strip[0], 0, 2, work->line_strip0_pos[0]);
		SPR_SetPosLineStrip(work->line_strip[1], 0, 2, work->line_strip0_pos[1]);
		SPR_SetPosLineStrip(work->line_strip[2], 0, 4, work->line_strip1_pos[0]);
		SPR_SetPosLineStrip(work->line_strip[3], 0, 4, work->line_strip1_pos[1]);

		break;
	case 2:
		
		SET_COLOR_2DPRIM3(work->ctrler, work->timer*4, CTRL_COLOR);
		SET_COLOR_2DPRIM3(work->menu_name, work->timer*4, MENU_NAME_COLOR);
		SET_COLOR_2DPRIM3(work->exit, work->timer*4, EXIT_COLOR);
		if( work->now_page == work->now_key_data-1 ) SET_COLOR_2DPRIM3(work->cursor1, work->timer*2, CURSOR_COLOR&0xffffff);
		SET_COLOR_2DPRIM3(work->mv_base, work->timer*2, 0);
		SET_COLOR_2DPRIM3(work->title_sp, work->timer*2, 0x00808080);
#if 0
		for( i = 0; i < 10; i++ ){
			if(work->now_key_data & (1<<i)){
				SET_COLOR_2DPRIM3(work->botton[i], work->timer*4, BOTTON_COLOR);
			}
		}
#endif
		ftemp0 = 256.0f - 472.0f * (float)work->exp_str_len/640.0f / 2.0f + /*space*/2.0f;
		ftemp1 = ftemp0 + 472.0f + /*space*/2.0f;
		ftemp2 = work->name_pos0;//ftemp0 + 16.0f;
		ftemp3 = work->name_pos1;//ftemp2 + 512.0f;

		PrintMovieExp( work, work->timer*4, work->timer*4 );
		if(++work->timer > 32){
			work->s_step++;
			work->timer = 0;
		}
		break;
	case 3:
		
		ftemp0 = 256.0f - 472.0f * (float)work->exp_str_len/640.0f / 2.0f + /*space*/2.0f;
		ftemp1 = ftemp0 + 472.0f + /*space*/2.0f;
		ftemp2 = work->name_pos0;//ftemp0 + 16.0f;
		ftemp3 = work->name_pos1;//ftemp2 +512.0f;

		PrintMovieExp( work, 128, 128 );
		if(work->proc_id[(Flag_Offset[work->now_category]+work->now_menu[work->now_category])]){
			GCL_ExecProc( work->proc_id[(Flag_Offset[work->now_category]+
										 work->now_menu[work->now_category])], NULL );
		}
		work->step++;
		work->s_step = 0;
		break;
	}

}

static void SpMenu_Movie(Work *work)
{
	GV_MSG *msg;
	int mes_num;
	int num;
	int alpha;

	PrintMovieExp( work, 128, 128 );
	if( work->now_page < work->now_key_data-1 ){
		if( ++work->timer == 32 ) work->timer = 0;
		alpha = abs((work->timer%32)-16) * 8;
		SET_COLOR_2DPRIM3(work->next_page, alpha, MENU_NAME_COLOR);
	}
#if 0
	for( i = 0; i < 10; i++ ){
		if(work->now_key_data & (1<<i)){
			SET_COLOR_2DPRIM3(work->botton[i], alpha<<1, BOTTON_COLOR);
		}
	}
#endif


	mes_num=GV_ReceiveMessage( work->name, &msg );
	
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		case 0:
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
			work->timer = 0;
			work->step = STEP_MOVIE_CLOSE;
			return;
			break;
		}
		msg--;
	}

	PadKeyAct( work );
#if 0
	if( work->pad.auto_status & (PAD_L1|PAD_L) ){
		if( work->now_page > 0 ){
			work->pre_page = work->now_page;
			work->now_page--;
			work->step = STEP_MOVIE_NEXT;
			work->timer = 0;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGEN001);
		}
	}else if( work->pad.auto_status & (PAD_R1|PAD_R) ){
		if( work->now_page < (N_MAX_EXP_PAGE-1) &&
			work->now_page < (work->now_key_data-1) ){
			work->pre_page = work->now_page;
			work->now_page++;
			work->step = STEP_MOVIE_NEXT;
			work->timer = 0;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGEN001);
		}

	}
#else
	if( work->pad.auto_status & PAD_OK ){ 
		if( work->now_page < (N_MAX_EXP_PAGE-1) &&
			work->now_page < (work->now_key_data-1) ){
			work->pre_page = work->now_page;
			work->now_page++;
			work->step = STEP_MOVIE_NEXT;
			work->timer = 0;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGEN001);
		}else if( work->now_page == (work->now_key_data-1) ){
			GV_PadDataDirect[ 0 ].release |= PAD_CANCEL;
		}
	}
#endif
	
#if 0
	if(work->pad.auto_status & PAD_CANCEL){
		GV_MSG 	msg;
		int msg_data[1];
		msg.address = work->name;
		msg.message = msg_data;
		msg.message_len = 1;
		msg_data[0] = 0;
		GV_SendMessage( &msg );
	}
#endif

}

static void SpMenu_MovieNextPage( Work *work )
{
	int		alpha;
//	float	ftemp0,ftemp1,ftemp2,ftemp3;

//	ftemp0 = 256.0f - 472.0f * (float)work->exp_str_len/640.0f / 2.0f + /*space*/2.0f;
//	ftemp1 = ftemp0 + 472.0f + /*space*/2.0f;
//	ftemp2 = work->name_pos0;
//	ftemp3 = work->name_pos1;

	{
		GV_MSG *msg;
		int mes_num;
		int num;

		mes_num=GV_ReceiveMessage( work->name, &msg );
		
		msg += mes_num-1;
		while( --mes_num >= 0 ){
			num=msg->message[0];
			switch( num ){
			  case 0:
				work->force_end_flag = 1;
				break;
			}
			msg--;
		}
	}

	if( ++work->timer < 9 ){
		alpha = 128 - work->timer*16;
		if( work->pre_page == work->now_key_data-1 ) SET_COLOR_2DPRIM3(work->cursor1, alpha/2, CURSOR_COLOR&0xffffff);
		SET_COLOR_2DPRIM3(work->next_page, alpha, MENU_NAME_COLOR);

		PrintMovieExp( work, 128, alpha );
	}else if( work->timer == 9 ){
		//テキスト再展開
		PrintMovieExp( work, 128, 0 );
		CreateMovieExp( work );
	/*
		MENU_ClearTextTexture( work->exp_work );
        // 説明分テキストを展開する
		work->exp_str_len = MENU_CreateTextTexture( work->exp_work,
													16, 4, 
													640-32+1, 160-8,
													0, 12, work->exp_str[(Flag_Offset[work->now_category]+
																		  work->now_menu[work->now_category])][work->now_page] );

        // 題名
		work->title_str_len = MENU_CreateTextTexture( work->exp_work,
													  16, 160+2,
													  FONT_BUFFER_WIDTH(24,0), FONT_BUFFER_HEIGHT(1,0),
													  0, 0, 
													  work->name_str[(Flag_Offset[work->now_category]+
													  work->now_menu[work->now_category])] );
	*/
	}else if( work->timer < 17 ){
		alpha = (work->timer-9)*16;
		PrintMovieExp( work, 128, alpha );
		if( work->now_page == work->now_key_data-1 ) SET_COLOR_2DPRIM3(work->cursor1, alpha/2, CURSOR_COLOR&0xffffff);
	}else if( work->timer == 17 ){
		PrintMovieExp( work, 128, 128 );
		work->step = STEP_MOVIE_WAIT;
		work->timer = 0;
		if( work->force_end_flag ){
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
			work->timer = 0;
			work->step = STEP_MOVIE_CLOSE;
			work->force_end_flag = 0;
		}
	}
}

static void SpMenu_MovieClose(Work *work)
{
   float lowerYPos = 184;
	SPR_POS	locate;
	SPR_POS	size;
	int	i;
	float	ftemp0,ftemp1,ftemp2,ftemp3,ftemp4;

	switch(work->s_step){
	case 0:

//		ftemp0 = 256.0f - 472.0f * (float)work->exp_str_len/640.0f / 2.0f + /*space*/2.0f;
//		ftemp1 = ftemp0 + 472.0f + /*space*/2.0f;
//		ftemp2 = work->name_pos0;
//		ftemp3 = work->name_pos1;
		PrintMovieExp( work, 128-work->timer*4, 128-work->timer*4 );
		/*
		MENU_PutTextScreen( work->exp_work,
							(int)ftemp0, (int)(260.0f*384.0f/448.0f),
							(int)ftemp1, (int)(388.0f*384.0f/448.0f),
							0, 0, 640, 160, ((128-work->timer*4)<<24)|0x00808080 );

		MENU_PutTextScreen( work->exp_work,
							(int)ftemp2, (int)(240.0f*384.0f/448.0f),
							(int)ftemp3,(int)TRN2DFIG_Y(240.0f+24.0f),
							0, 160, FONT_BUFFER_WIDTH(24,0), 160 + FONT_BUFFER_HEIGHT(1,0),
							((128-work->timer*4)<<24)|0x00606060 );
		*/
		SET_COLOR_2DPRIM3(work->ctrler, 128-work->timer*4, CTRL_COLOR);
		SET_COLOR_2DPRIM3(work->exit, 128-work->timer*4, EXIT_COLOR);
		SET_COLOR_2DPRIM3(work->menu_name, 128-work->timer*4, MENU_NAME_COLOR);
		SET_COLOR_2DPRIM3(work->cursor1, 64-work->timer*2, CURSOR_COLOR&0xffffff);
		SET_COLOR_2DPRIM3(work->mv_base, 64-work->timer*2, 0);
		SET_COLOR_2DPRIM3(work->title_sp, 64-work->timer*2, 0x00808080);
		if(work->now_page < work->now_key_data-1)
			SET_COLOR_2DPRIM3(work->next_page, 128-work->timer*4, MENU_NAME_COLOR);
#if 0
		for( i = 0; i < 10; i++ ){
			if(work->now_key_data & (1<<i)){
				SET_COLOR_2DPRIM3(work->botton[i], 128 - work->timer*4, BOTTON_COLOR);
			}
		}
#endif
		if(++work->timer > 32){
			work->s_step++;
			work->timer = 0;
		}
		break;
	case 1:
		ftemp3 = ftemp2 = ftemp0 = 198.0f - (work->category_y += 24.0f);

		if( work->category_y > 198.0f){
			ftemp0 = 0.0f;
			ftemp2 = 0.0f;
			ftemp3 = 0.0f;
			work->s_step++;
			work->category_y = 0.0f;
		}


		ftemp4 = ftemp1 = ftemp0*172.0f/198.0f;

		if(ftemp1 < 25.0f) ftemp1 = 25.0f;

		if(ftemp2 > lowerYPos - 216.0f+8.0f ){
			ftemp2 = lowerYPos - 216.0f+8.0f;
		}

		if(ftemp3 > lowerYPos - 216.0f){
			ftemp3 = lowerYPos - 216.0f;
		}

		if(ftemp4 < 25.0f - 8.0f){
			ftemp4 = 25.0f - 8.0f;
		}

		work->line_strip0_pos[0][0].y = 192.0f - TRN2DFIG_Y(ftemp0);
		work->line_strip0_pos[0][1].y = 192.0f + TRN2DFIG_Y(ftemp1);
		work->line_strip0_pos[1][0].y = 192.0f - TRN2DFIG_Y(ftemp0);
		work->line_strip0_pos[1][1].y = 192.0f + TRN2DFIG_Y(ftemp1);

		work->line_strip1_pos[0][0].y = 192.0f + TRN2DFIG_Y(ftemp2+1.0f);
		work->line_strip1_pos[0][1].y = 192.0f + TRN2DFIG_Y(ftemp3);
		work->line_strip1_pos[0][2].y = 192.0f + TRN2DFIG_Y(ftemp3);
		work->line_strip1_pos[0][3].y = 192.0f + TRN2DFIG_Y(ftemp2);

		work->line_strip1_pos[1][0].y = 192.0f + TRN2DFIG_Y(ftemp4-1.0f);
		work->line_strip1_pos[1][1].y = 192.0f + TRN2DFIG_Y(ftemp1);
		work->line_strip1_pos[1][2].y = 192.0f + TRN2DFIG_Y(ftemp1);
		work->line_strip1_pos[1][3].y = 192.0f + TRN2DFIG_Y(ftemp4);

		locate.x = 28.0f;
		locate.y = 192.0f - TRN2DFIG_Y(ftemp0);
		
		SPR_SetPosSprite(work->title1, &locate );

		SPR_SetPosLineStrip(work->line_strip[0], 0, 2, work->line_strip0_pos[0]);
		SPR_SetPosLineStrip(work->line_strip[1], 0, 2, work->line_strip0_pos[1]);
		SPR_SetPosLineStrip(work->line_strip[2], 0, 4, work->line_strip1_pos[0]);
		SPR_SetPosLineStrip(work->line_strip[3], 0, 4, work->line_strip1_pos[1]);

		break;
	case 2:
		ftemp0 = 236.0f - (work->category_y += 24.0f);

		if(work->category_y > 236.0f ){
			work->s_step++;
			work->category_y = 0.0f;
			ftemp0 = 0.0f;
		}
		ftemp1 = ftemp0 - 8.0f;
		if( ftemp1 < 0.0f ) ftemp1 = 0.0f;

		work->line_strip0_pos[0][0].x = 256.0f - ftemp0;
		work->line_strip0_pos[0][1].x = 256.0f - ftemp0;
		work->line_strip0_pos[1][0].x = 256.0f + ftemp0;
		work->line_strip0_pos[1][1].x = 256.0f + ftemp0;

		work->line_strip1_pos[0][0].x = 256.0f - ftemp1;
		work->line_strip1_pos[0][1].x = 256.0f - ftemp1;
		work->line_strip1_pos[0][2].x = 256.0f + ftemp1;
		work->line_strip1_pos[0][3].x = 256.0f + ftemp1;

		work->line_strip1_pos[1][0].x = 256.0f - ftemp1;
		work->line_strip1_pos[1][1].x = 256.0f - ftemp1;
		work->line_strip1_pos[1][2].x = 256.0f + ftemp1;
		work->line_strip1_pos[1][3].x = 256.0f + ftemp1;

		locate.x = 256.0f - ftemp1;
		locate.y = 192.0f;

		size.x = ftemp1*2.0f;
		size.y = TRN2DFIG_Y(16.0f);

		SPR_SetPosSprite(work->title1, &locate );
		SPR_SetSizeSprite(work->title1, size.x, size.y );

		SPR_SetPosLineStrip(work->line_strip[0], 0, 2, work->line_strip0_pos[0]);
		SPR_SetPosLineStrip(work->line_strip[1], 0, 2, work->line_strip0_pos[1]);
		SPR_SetPosLineStrip(work->line_strip[2], 0, 4, work->line_strip1_pos[0]);
		SPR_SetPosLineStrip(work->line_strip[3], 0, 4, work->line_strip1_pos[1]);
		break;
	case 3:
		SPR_SHOW(work->sub_parent0);
		SPR_HIDE(work->sub_parent1);
		work->step++;
		MENU_ClearTextTexture( work->exp_work );
		for( i = 0; i < MaxMenuNum[work->now_category]; i++ ){
			MENU_CreateTextTexture( work->exp_work,
									2, 2 + (FONT_BUFFER_HEIGHT(1,0)+2)*i,
									1024-4, FONT_BUFFER_HEIGHT(1,0)+2,
									0, 0, 0, work->name_str[i+Flag_Offset[work->now_category]] );
		}

		work->s_step = 0;
		break;
	default:
		break;
	}

}

static void SpMenu_Movie_Menu(Work *work)
{
	int i,alpha = work->timer<<2;

	SET_COLOR_2DPRIM3(work->cursor,48*alpha/128,CURSOR_COLOR&0xffffff);
	SET_COLOR_2DPRIM3(work->title0,alpha,0x00acc37a);
	SET_COLOR_2DPRIM3(work->title_base,alpha>>1,TITLE_BASE_COLOR);

	for( i = 0; i < 2; i++ ){
		SPR_SetColorLine(work->line[i], 0, LINE0_COLOR_R,LINE0_COLOR_G,LINE0_COLOR_B,alpha);
		SPR_SetColorLine(work->line[i], 1, LINE0_COLOR_R,LINE0_COLOR_G,LINE0_COLOR_B,alpha);
	}

	for( i = 0; i < 4; i++ ){
		SET_COLOR_2DPRIM3(work->category[i],alpha,CATEGORY_COLOR&0xffffff);
	}
#if 0
	for( i = 0; i < MaxMenuNum[work->now_category]; i++ ){
		if(work->now_menu[work->now_category]!=i &&
		   work->see_flags & 1 << (i+Flag_Offset[work->now_category])){
			//SET_COLOR_2DPRIM3(work->menu[i],alpha,MENU_COLOR1&0xffffff);
		}else{
			//SET_COLOR_2DPRIM3(work->menu[i],alpha,MENU_COLOR0&0xffffff);
		}
	}
#endif
	PrintMenu( work, work->now_category, alpha, 1 );

	if(++work->timer>32){
		work->step = STEP_MENU_WAIT;
		work->timer = 0;
	}
	
}

static void SpMenu_FadeOut(Work *work)
{

	int i,alpha = 128 - (work->timer<<2);
	if(alpha < 0) alpha=0;
	SET_COLOR_2DPRIM3(work->cursor,alpha,CURSOR_COLOR&0xffffff);
	SET_COLOR_2DPRIM3(work->title0,alpha,0x00acc37a);
	SET_COLOR_2DPRIM3(work->title_base,alpha>>1,TITLE_BASE_COLOR);


	for( i = 0; i < 2; i++ ){
		SPR_SetColorLine(work->line[i], 0, LINE0_COLOR_R,LINE0_COLOR_G,LINE0_COLOR_B,alpha);
		SPR_SetColorLine(work->line[i], 1, LINE0_COLOR_R,LINE0_COLOR_G,LINE0_COLOR_B,alpha);
	}

	for( i = 0; i < 4; i++ ){
		SET_COLOR_2DPRIM3(work->category[i],alpha,CATEGORY_COLOR&0xffffff);
	}
#if 0
	for( i = 0; i < MaxMenuNum[work->now_category]; i++ ){
		if(work->see_flags & 1 << (i+Flag_Offset[work->now_category])){
			//SET_COLOR_2DPRIM3(work->menu[i],alpha,MENU_COLOR1&0xffffff);
		}else{
			//SET_COLOR_2DPRIM3(work->menu[i],alpha,MENU_COLOR0&0xffffff);
		}
	}
#endif

	PrintMenu( work, work->now_category, alpha, 0 );

	if(work->timer >= 32){	
		SET_COLOR_2DPRIM3(work->bground,128 - ((work->timer-32)<<2),0x00808080);
	}

	if(++work->timer > 64){
		SPR_HIDE(work->bground);
		work->timer = 0;
		work->step++;
	}

	
}


static void SpMenu_Kill(Work *work)
{	
	GCL_ARGS	gcl_args;
	int			data;

	data = work->see_flags;

	gcl_args.argc = 1;
	gcl_args.argv = &data;
	GCL_ExecProc( work->end_id, &gcl_args );

	GV_DestroyActor( work );
}

/* ------------------------------------------------------------------------ */

static void (*SpMenuAct[])(Work *) = {
	SpMenu_FadeIn,
	SpMenu_VisCate,
	SpMenu_VisLine,
	SpMenu_VisMenu,

	SpMenu_Category,
	SpMenu_Category_Cursor,
	
	SpMenu_Category_Menu,

	SpMenu_Menu,
	SpMenu_Menu_Cursor,

	SpMenu_Menu_Category,

	SpMenu_Menu_Movie,
	SpMenu_MoviePre,
	SpMenu_Movie,
	SpMenu_MovieNextPage,
	SpMenu_MovieClose,
	SpMenu_Movie_Menu,
	SpMenu_FadeOut,
	SpMenu_Kill,
};
/* ------------------------------------------------------------------------ */
static void Act( Work *work )
{
	SpMenuAct[work->step](work);
}

static void Die( Work *work )
{
	printf("die\n");
	if(work->bground) SPR_Destroy_2D_Object(work->bground);	
}


static int GetResources( Work *work )
{
	SPR_OBJ	*sprite,*papa,*sub_papa;	
	SPR_POS locate;
	int		tri_id;
	int		i,j;
	
	work->see_flags = GCL_GetOptionValue( 'f', 0 );
	work->end_id = GCL_GetOptionValue( 'e', 0 );


	for( i = 0; i < N_ALLMENU; i++ ){
		if(GCL_GetNextOption() == 'd'){
			int		ret;
			work->proc_id[i] = GCL_GetNextInt();
			work->name_str[i] = BP_GCL_LOOKUP_NEW_FONT_STRING((char*)GCL_GetNextInt());
			ret = GCL_GetNextInt();
			work->key_data[i] = GCL_GetNextInt()+1;
			for( j = 0; j < 4 && j < work->key_data[i]; j++ ){
				work->exp_str[i][j] = BP_GCL_LOOKUP_NEW_FONT_STRING((char*)GCL_GetNextInt());
			}
			if(ret){
				work->name_str2[i] = BP_GCL_LOOKUP_NEW_FONT_STRING((char*)GCL_GetNextInt());
			}else{
				work->name_str2[i] = NULL;
			}
		}

	}

	work->exp_work = NewTextScreenControl();
	if(!work->exp_work) return -1;
	GV_SetActorChild( work, work->exp_work );/* メモリーリークしていたので追加 2002.07.26yano */

	work->step = 0;
	work->s_step = 0;
	work->now_category = 0;
	work->menu_size = 0;
	work->timer = 0;
	work->now_menu[0] = 0;
	work->now_menu[1] = 0;
	work->now_menu[2] = 0;
	work->menu_to_num = 0;

	work->category_y = 0.0f;

	work->cursor_pos.x = CATEGORY_POS_X;
	work->cursor_pos.y = Category_Pos_Y[0];
	work->cursor_size.x = Category_Size[0].x;
	work->cursor_size.y = Category_Cor_Size_Y[0];
	work->line_pos[0].x = Line0_Pos[0][0].x;
	work->line_pos[0].y = Line0_Pos[0][0].y;
	work->line_pos[1].x = Line0_Pos[0][0].x;
	work->line_pos[1].y = Line0_Pos[0][0].y;

	work->tri_id = tri_id = SPR_LoadTexture(TRI_CODE);

	papa = work->bground = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
	if(papa == NULL){ printf("ERR!! Make2DObj[papa]\n"); return -1; }
	SPR_ObjSetTexture(papa, TEX_CODE_BG, tri_id);
	papa->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	papa->head.flags |= SPR_FLAG_ALPHA;
#ifdef BP_PS2 // yano
	SPR_SetPosSprite(papa, &(SPR_POS){0.0f,0.0f} );
#else
	{
		SPR_POS dmdm = {0.0f,0.0f};
		SPR_SetPosSprite(papa, &dmdm );
	}
#endif
	SPR_SetSizeSprite(papa, 512.0f, 384.0f );
	SET_COLOR_2DPRIM(papa,0);
	SPR_SetPriority( papa, 0);
	SPR_SHOW(papa);

	//第一フェーズ
	sub_papa = work->sub_parent0 = SPR_Create_2D_Object(SP_EMPTY, 4, papa);
#ifdef BP_PS2 // yano
	SPR_SetPosEmpty( sub_papa, &(SPR_POS){0.0f,0.0f});
#else
	{
		SPR_POS dmdm = {BP_TO_WIDESCREEN_OFFSETX+0.0f,BP_TO_WIDESCREEN_OFFSETY+0.0f};
		SPR_SetPosEmpty( sub_papa, &dmdm );
	}
#endif
	SPR_SHOW( sub_papa );

	locate.x = CATEGORY_POS_X + CURSOR_SHIFT_X;
	locate.y = Category_Pos_Y[0] + CURSOR_SHIFT_Y;

	sprite = work->cursor = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
	SPR_ObjSetTexture(sprite, TEX_CODE_CURSOR, tri_id);
	SPR_SetPosSprite(sprite, &locate );
	SPR_SetSizeSprite(sprite, Category_Size[0].x + CURSOR_ADD_SIZE_X, Category_Cor_Size_Y[0] + CURSOR_ADD_SIZE_Y );
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite,CURSOR_COLOR);
	SPR_SetPriority( sprite, 2);

	sprite = work->title0 = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
	SPR_ObjSetTexture(sprite, 285786, tri_id);
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 32.0f, TRN2DFIG_Y(25.0f) } );
#else
	{
		SPR_POS dmdm = { BP_TO_WIDESCREEN_OFFSETX+32.0f, BP_TO_WIDESCREEN_OFFSETY+TRN2DFIG_Y(25.0f+12.0f) };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	SPR_SetSizeSprite(sprite, 0.75f*140.0f, TRN2DFIG_Y(10.0f));
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite,0x00808080);
	SPR_SetPriority( sprite, 3);
	SPR_SHOW(sprite);

	sprite = work->title_base = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 24.0f, TRN2DFIG_Y(22.0f) } );
#else
	{
		SPR_POS dmdm = { BP_TO_WIDESCREEN_OFFSETX+24.0f, BP_TO_WIDESCREEN_OFFSETY+TRN2DFIG_Y(22.0f+12.0f) };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	SPR_SetSizeSprite(sprite, 0.75f*(14.0f+140.0f), TRN2DFIG_Y(15.0f));
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite,0x00808080);
	SPR_SetPriority( sprite, 2);
	SPR_SHOW(sprite);

	for( i = 0; i < 2; i++ ){
		sprite = work->line[i] = SPR_Create_2D_Object(SP_LINE, 4, sub_papa);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		SPR_SetPosLine(sprite,&Line0_Pos[0][0], &Line0_Pos[0][0]);
		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		SPR_SetColorLine(sprite, 0, LINE0_COLOR_R,LINE0_COLOR_G,LINE0_COLOR_B,128);
		SPR_SetColorLine(sprite, 1, LINE0_COLOR_R,LINE0_COLOR_G,LINE0_COLOR_B,128);
		SPR_SetPriority( sprite, 3);

		SPR_SHOW(sprite);
	}

	for( i = 0; i < 4; i++ ){
		sprite = work->category[i] = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		SPR_ObjSetTexture(sprite, Category_Tex_Code[GM_Language][i], tri_id);
#ifdef BP_PS2 //yano
		SPR_SetPosSprite(sprite, &(SPR_POS){CATEGORY_POS_X,-TRN2DFIG_Y(48.0f)});
#else
	{
		SPR_POS dmdm = {BP_TO_WIDESCREEN_OFFSETX+CATEGORY_POS_X,BP_TO_WIDESCREEN_OFFSETY+ -TRN2DFIG_Y(48.0f)};
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
		SPR_SetSizeSprite(sprite, Category_Size[i].x, Category_Size[i].y );
		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		SET_COLOR_2DPRIM(sprite,CATEGORY_COLOR);
		SPR_SetPriority( sprite, 3);

		SPR_SHOW(sprite);

	}

	locate.x = MENU_POS_X;
	locate.y = InitMenuPos_Y[0];

	MENU_ClearTextTexture( work->exp_work );

	for( i = 0; i < N_MAXMENU0; i++ ){
#if 0
		sprite = work->menu[i] = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		SPR_ObjSetTexture(sprite, Menu_Tex_Code[0], tri_id);
		SetSpriteUV( (SPR_SPRITE*)sprite, 0, i*27, 184, 27  );
		SPR_SetPosSprite(sprite, &locate );
		SPR_SetSizeSprite(sprite, 0.0f, MENU_SIZE_Y );
		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		if(work->see_flags & 1 << i){
			SET_COLOR_2DPRIM(sprite,MENU_COLOR1);
		}else{
			SET_COLOR_2DPRIM(sprite,MENU_COLOR0);
		}		
		SPR_SetPriority( sprite, 3);
		SPR_SHOW(sprite);
#endif
		float wide;
		wide = (float)MENU_CreateTextTexture( work->exp_work,
											  2, 2 + (FONT_BUFFER_HEIGHT(1,0)+2)*i,
											  1024-4, FONT_BUFFER_HEIGHT(1,0)+2,
											  0, 0, 0, work->name_str[i] );
		MenuSize_X[0][i] = (wide * (TXT_SIZE_X) / 1024.0f)*1.3f;
//		printf(" str wide [%f]\n",wide);

		
//		locate.y += MENU_SIZE_Y;

	}


   //
   // the show movie bit



	//第２フェーズ
	sub_papa = work->sub_parent1 = SPR_Create_2D_Object(SP_EMPTY, 4, papa);
#ifdef BP_PS2 //yano
	SPR_SetPosEmpty( sub_papa, &(SPR_POS){0.0f,0.0f});
#else
	{
		SPR_POS dmdm = {0.0f,0.0f};
		SPR_SetPosEmpty( sub_papa, &dmdm );
	}
#endif
	sprite = work->title1 = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
	//SPR_ObjSetTexture(sprite, TEX_CODE_CURSOR, tri_id);
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 256.0f, 192.0f } );
#else
	{
		SPR_POS dmdm = { 256.0f, BP_TO_WIDESCREEN_OFFSETY+192.0f };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	SPR_SetSizeSprite(sprite, 0.0f, TRN2DFIG_Y(16.0f) );
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite, TITLE_COLOR);
	SPR_SetPriority( sprite, 2);
	SPR_SHOW(sprite);                // movie

	sprite = work->title_sp = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
	SPR_ObjSetTexture(sprite, 285786, tri_id);
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 256.0f - 140.0f/2.0f, TRN2DFIG_Y(29.0f) } );
#else
	{
		SPR_POS dmdm = { (256.0f - 140.0f/2.0f), (BP_TO_WIDESCREEN_OFFSETY+TRN2DFIG_Y(29.0f))+8 };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	SPR_SetSizeSprite(sprite, 140.0f, TRN2DFIG_Y(10.0f) );
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite, 0);
	SPR_SetPriority( sprite, 3);
	SPR_SHOW(sprite);             //title

	sprite = work->ctrler = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 38.0f, TRN2DFIG_Y(57.0f) } );
#else
	{
		SPR_POS dmdm = {24.0f+38.0f, BP_TO_WIDESCREEN_OFFSETY+TRN2DFIG_Y(57.0f) };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	SPR_SetSizeSprite(sprite, 0.75f*196.0f, TRN2DFIG_Y(160.0f) );
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite, 0x00808080);
	SPR_SetPriority( sprite, 2);
	SPR_SHOW(sprite);          // beginner etc sprite.

	sprite = work->exit = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
	SPR_ObjSetTexture(sprite, 13144248, tri_id);
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 218.0f, TRN2DFIG_Y(402.0f) } );
#else
	{
		SPR_POS dmdm = { 7+218.0f, BP_TO_WIDESCREEN_OFFSETY+TRN2DFIG_Y(402.0f)-8 };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	SPR_SetSizeSprite(sprite, 57.0f, TRN2DFIG_Y(24.0f) );
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite, 0x00808080);
	SPR_SetPriority( sprite, 2);
	SPR_SHOW(sprite);

	sprite = work->cursor1 = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
	SPR_ObjSetTexture(sprite, TEX_CODE_CURSOR, tri_id);
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 218.0f-8.0f, TRN2DFIG_Y(402.0f-8.0f) } );
#else
	{
		SPR_POS dmdm = {(7+218.0f-8.0f), BP_TO_WIDESCREEN_OFFSETY+TRN2DFIG_Y(402.0f-8.0f)-8 };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	SPR_SetSizeSprite(sprite, (57.0f+16.0f), TRN2DFIG_Y(24.0f+16.0f) );
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite, 0);
	SPR_SetPriority( sprite, 1);
	SPR_SHOW(sprite);          // exit

	sprite = work->mv_base = SPR_Create_2D_Object(SP_SPRITE_F, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
#ifdef BP_PS2
	SPR_SetPosSprite(sprite, &(SPR_POS){ 246.0f-1.0f, 49.0f-1.0f } );
#else
	{
		SPR_POS dmdm = { 273, 47};
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	SPR_SetSizeSprite(sprite, 169.5, 139.5 );
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite, 0);
	SPR_SetPriority( sprite, 1);
	SPR_SHOW(sprite);

	sprite = work->menu_name = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
	SPR_SetSizeSprite(sprite, 10.0f, TRN2DFIG_Y(14.0f) );
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 32.0f, TRN2DFIG_Y(246.0f) } );
#else
	{
		SPR_POS dmdm = {32.0f, 204 };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite, 0);
	SPR_SetPriority(sprite, 1);
	SPR_SHOW(sprite);

	sprite = work->next_page = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
	if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
	SPR_SetSizeSprite(sprite, 8.0f, TRN2DFIG_Y(10.0f) );
#ifdef BP_PS2 //yano
	SPR_SetPosSprite(sprite, &(SPR_POS){ 462.0f, TRN2DFIG_Y(367.0f) } );
#else
	{
		SPR_POS dmdm = {462.0f, BP_TO_WIDESCREEN_OFFSETY+TRN2DFIG_Y(367.0f) };
		SPR_SetPosSprite(sprite, &dmdm );
	}
#endif
	sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(sprite, 0);
	SPR_SetPriority(sprite, 1);
	SPR_SHOW(sprite);
	
	for( i = 0; i < 4; i++ ){
		sprite = work->line_strip[i] = SPR_Create_2D_Object(SP_LINESTRIP, 4, sub_papa);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		
		SPR_SetLineStripVertexNumber(sprite, (i/2+1)*2);
		SPR_SetPosLineStrip(sprite, 0, (i/2+1)*2, (i<2)?InitLineStrip0:InitLineStrip1);
		SPR_SetColorLineStrip(sprite, 0, LINE1_COLOR_R, LINE1_COLOR_G, LINE1_COLOR_B, LINE1_COLOR_A);
		SPR_SetColorLineStrip(sprite, 1, LINE1_COLOR_R, LINE1_COLOR_G, LINE1_COLOR_B, LINE1_COLOR_A);
		if(i >= 2){
			SPR_SetColorLineStrip(sprite, 2, LINE1_COLOR_R, LINE1_COLOR_G, LINE1_COLOR_B, LINE1_COLOR_A);
			SPR_SetColorLineStrip(sprite, 3, LINE1_COLOR_R, LINE1_COLOR_G, LINE1_COLOR_B, LINE1_COLOR_A);
		}
		SPR_SHOW(sprite);
	}
#if 0
	for( i = 0; i < 10; i++ ){
		sprite = work->botton[i] = SPR_Create_2D_Object(SP_SPRITE, 4, sub_papa);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		SPR_ObjSetTexture(sprite, BottomTexCode[i], tri_id);
		SPR_SetPosSprite(sprite, &BottonPos[i] );
		SPR_SetSizeSprite(sprite, BottonSize[i].x, BottonSize[i].y );
		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 2, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		SET_COLOR_2DPRIM(sprite, 0x00808080);
		SPR_SetPriority( sprite, 2);
		SPR_SHOW(sprite);
	}
#endif


	PadKeyInit( work );

	return 0;
}

void *NewSpecialMenu_Page( int name, int map )
{
	Work	*work;

	work = (Work*)GV_CreateActor( GV_ACTOR_ASSIST/*GV_ACTOR_AFTER2*/, GV_CLASS_OBJECT,
								  sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
	}

	return work ;
}

//285786
