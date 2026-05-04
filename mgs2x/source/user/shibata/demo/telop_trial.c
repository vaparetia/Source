//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    telop_trial.c
    テロップ
	2000/10/05 T.Shibata

	$Id: telop_trial.c,v 1.1.1.3 2002/11/19 11:48:35 Yoshizawa1 Exp $
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
#include	"gameheader.h"

#include	"camera.h"
#include 	"sprite_2d.h"
#include	"../util/ts_util.h"

#include "libfs.h"

//#define	ACTOR_PRIO		(254)


#define TRI_CODE		(GV_StrCode("staff"))

#define MAX_TELOP		(4)

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))
#define SET_COLOR_2DPRIM3(_prim,_a,_col) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | (_col)))

typedef	struct {
	GV_ACT_EX		actor;
	int				name;
	float			ratio;
	int				flags;
	short			tex_num,prim_num;

	SPR_OBJ   		*fst[MAX_TELOP][2];			//アルファのみ				↓描画順
	SPR_OBJ   		*snd[MAX_TELOP][2];    		//フレームのアルファ加算	↓
//	SPR_OBJ   		*thd[MAX_TELOP][2];    		//アルファ補正	   			＿

	short	   		alpha[MAX_TELOP];
	short			step[MAX_TELOP];
	float			h[MAX_TELOP];
	float			off_x[MAX_TELOP];
	float			off_y[MAX_TELOP];
	float			add_x[MAX_TELOP];
	FVECTOR			color[MAX_TELOP];

	int				time_index;
} Work;

#define		TELOP_JPN	(0)
#define		TELOP_ENG	(1)

#define		TELOP_FLAGS_ENG	(0x0080)

#define		MAX_TEX		(25)
/*
01スネーク声優
02オタコン声優/オルガ声優
03オセロット声優/ゴルルコビッチ大佐声優
04 written by カントク/フクシマさん
05水谷さん
06植原さん
07豊田さん
08マツハナさん
09村岡さん
10ハレーグレッグソンウィリアムズ
11村中りか
12新川さん
13吉岡社長
14 produced byカントク
16スネークテロップ
17オルガテロップ
18毛利さん

19
Character Art Director				Hideki Sasaki
アートディレクター（キャラクター）	佐々木英樹
20
Mechanical Art Director				Mineshi Kimura
アートディレクター（メカニック）	木村峰士
21
Background Art Director				Yutaka Negishi
アートディレクター(背景)			根岸豊
22
Main Technical Programmer			Kunio Takabe
メインテクニカルプログラマ			高部邦夫
23
Main Programmer (enemy AI)			Yuji Korekado
メインプログラマ（敵兵ＡＩ）		是角有二
24
Main Programmer (player)			Makoto Sonoyama
メインプログラマ（プレイヤー）		園山誠人
25
Main Programmer (effects)			Shigeo Okajima
メインプログラマ（エフェクト）		岡嶋滋生
*/

//デフォルト英語
static int Tex_Code_E[MAX_TEX][2] = {
	{ 10536465, 10864145, },
	{ 12633617, 12961297, },
	{ 14730769, 15058449, },
	{    50706,   378386, },
	{  2147858,  2475538, },
	{  4245010,  4572690, },
	{  6342162,  6669842, },
	{  8439314,  8766994, },
	{ 10536466, 10864146, },
	{ 12633618, 12961298, },
	{ 10536469, 10864149, },
	{ 12633621, 12961301, },
	{ 14730773, 15058453, },
	{    50710,   378390, },
	{  2147862,  2475542, },
	{  6342166,  6669846, },
	{  8439318,  8766998, },
	{ 10536470, 10864150, },
	
	{ 12633622, 12961302, },
	{ 10536473, 10864153, },	//20

	{ 12633625, 12961305, },	//21
	{ 14730777, 15058457, },	//22
	{    50714,   378394, },	//23
	{  2147866,  2475546, },	//24
	{  4245018,  4572698, },	//25
	
};

//デフォ日本語
static int Tex_Code_J[MAX_TEX][2] = {
	{ 10864145, 10536465, },
	{ 12961297, 12633617, },
	{ 15058449, 14730769, },
	{   378386,    50706, },
	{  2475538,  2147858, },
	{  4572690,  4245010, },
	{  6669842,  6342162, },
	{  8766994,  8439314, },
	{ 10864146, 10536466, },
	{ 12961298, 12633618, },
	{ 10864149, 10536469, },
	{ 12961301, 12633621, },
	{ 15058453, 14730773, },
	{   378390,    50710, },
	{  2475542,  2147862, },
	{  6669846,  6342166, },
	{  8766998,  8439318, },
	{ 10864150, 10536470, },
	
	{ 12961302, 12633622, },
	{ 10864153, 10536473, },

	{ 12961305, 12633625, },	//21
	{ 15058457, 14730777, },	//22
	{   378394,    50714, },	//23
	{  2475546,  2147866, },	//24
	{  4572698,  4245018, },	//25
};

#define CHANL_NUM	(0)
#define	DEF_MODE	(0)

#define STEP_SLEEP		(0x0000)
#define STEP_FADEIN		(0x1000)
#define STEP_PRE_WAIT	(0x2000)
#define STEP_WAIT		(0x3000)
#define STEP_AFT_WAIT	(0x4000)
#define STEP_FADEOUT	(0x5000)

#define MSG_TELOP_RUN			(0)
#define MSG_TELOP_DEF_CHANGE	(1)

#define TELOP_FLAG_ACTIVE 	(0x0080)
#define TELOP_FLAG_TO_JPN	(0x0000)
#define TELOP_FLAG_TO_ENG	(0x0001)

#define		TEX_SIZE_W		(256.0f*TARGET_ASPECT_X)
#define		TEX_SIZE_H		(128.0f)

#define	ADD_OFFSET_0	(16.0f)
#define	ADD_OFFSET_1	(0.5f)
#define	ADD_OFFSET_2	(0.5f)

#define	ADD_ADD_OFFSET_0	((ADD_OFFSET_1 - ADD_OFFSET_0)/32.0f)
#define	ADD_ADD_OFFSET_1	((ADD_OFFSET_2 - ADD_OFFSET_1)/32.0f)

#define ADD_OFF_Y0		(TEX_SIZE_H*3.0f/8.0f/32.0f)
#define ADD_OFF_Y1		(TEX_SIZE_H*3.0f/8.0f/32.0f)

//#define	COLOR_R	(64)
//#define	COLOR_G	(96)
//#define	COLOR_B	(128)

#define	INIT_COLOR_R	(0)
#define	INIT_COLOR_G	(0)
#define	INIT_COLOR_B	(0)

//#define	MAX_COLOR_R	(240.0f)
//#define	MAX_COLOR_G	(254.0f)
//#define	MAX_COLOR_B	(180.0f)

#define	MAX_COLOR_R	(180.0f)
#define	MAX_COLOR_G	(210.0f)
#define	MAX_COLOR_B	(244.0f)

#define TIME_FADE_IN	(24)
#define TIME_PRE_WAIT	(8)
#define TIME_WAIT		(160)
#define TIME_AFT_WAIT	(16)
#define TIME_FADE_OUT	(48)

//256

#define TIME_FADE_IN_EX		((24 *180)/256)
#define TIME_PRE_WAIT_EX	((8  *180)/256)
#define TIME_WAIT_EX		((160*180)/256)
#define TIME_AFT_WAIT_EX	((16 *180)/256)
#define TIME_FADE_OUT_EX	((48 *180)/256)

//196

#define	DEF_SNAKE_TEX	(15)

static int TimeFadeIn[2] = {
	TIME_FADE_IN,
	TIME_FADE_IN_EX,
};

static int TimePreWait[2] = {
	TIME_PRE_WAIT,
	TIME_PRE_WAIT_EX,
};

static int TimeWait[2] = {
	TIME_WAIT,
	TIME_WAIT_EX,
};

static int TimeAftWait[2] = {
	TIME_AFT_WAIT,
	TIME_AFT_WAIT_EX,
};

static int TimeFadeOut[2] = {
	TIME_FADE_OUT,
	TIME_FADE_OUT_EX,
};

#define	DIFF_COLOR_R0	(MAX_COLOR_R-64.0f)
#define	DIFF_COLOR_G0	(MAX_COLOR_G-64.0f)
#define	DIFF_COLOR_B0	(MAX_COLOR_B-64.0f)
#define	DIFF_COLOR_R1	(-64.0f)
#define	DIFF_COLOR_G1	(-64.0f)
#define	DIFF_COLOR_B1	(-64.0f)
#define	DIFF_COLOR_R2	(128.0f)
#define	DIFF_COLOR_G2	(128.0f)
#define	DIFF_COLOR_B2	(128.0f)
#define	DIFF_COLOR_R3	(-MAX_COLOR_R)
#define	DIFF_COLOR_G3	(-MAX_COLOR_G)
#define	DIFF_COLOR_B3	(-MAX_COLOR_B)

#define	ADD_COLOR_R0	(DIFF_COLOR_R0/24.0f)
#define	ADD_COLOR_G0	(DIFF_COLOR_G0/24.0f)
#define	ADD_COLOR_B0	(DIFF_COLOR_B0/24.0f)

#define	SUB_COLOR_R0	(DIFF_COLOR_R1/8.0f)
#define	SUB_COLOR_G0	(DIFF_COLOR_G1/8.0f)
#define	SUB_COLOR_B0	(DIFF_COLOR_B1/8.0f)

#define	ADD_COLOR_R1	(DIFF_COLOR_R2/16.0f)
#define	ADD_COLOR_G1	(DIFF_COLOR_G2/16.0f)
#define	ADD_COLOR_B1	(DIFF_COLOR_B2/16.0f)

#define	SUB_COLOR_R1	(DIFF_COLOR_R3/48.0f)
#define	SUB_COLOR_G1	(DIFF_COLOR_G3/48.0f)
#define	SUB_COLOR_B1	(DIFF_COLOR_B3/48.0f)

static void *TelopWork = NULL;



static void SetSpriteUV( SPR_SPRITE *prim, int u, int v, int w, int h  )
{
	SPR_TEX		*tex = &prim->head.tex;
	int			tw,th,off_u,off_v;

	DG_GetTexelInfo( &tw, &th, &off_u, &off_v, tex->dgtex );
//	printf("off_u %d:off_v %d:size_w %d:size_h %d\n",off_u,off_v,tw,th);
//	printf("u %d:v %d:w %d:h %d\n",u,v,w,h);

#ifdef PSX2
	tex->u = ((off_u + u)<<4) + (1<<3);
	tex->v = ((off_v + v)<<4) + (1<<3);
	tex->w = (w<<4) - (1<<4);
	tex->h = (h<<4) - (1<<4);
#else
	tex->u = SPR_FIXED(off_u + u + 0.5f) ;
	tex->v = SPR_FIXED(off_v + v + 0.5f) ;
	tex->w = SPR_FIXED(w - 1) ;
	tex->h = SPR_FIXED(h - 1) ;
#endif
}

static int InitNewTelop( Work *work, FVECTOR *pos )
{
	int		i,prim_num = work->prim_num%4;
   int		*tex_code = BP_Area_JP() ? Tex_Code_J[work->tex_num%MAX_TEX] : Tex_Code_E[work->tex_num%MAX_TEX];
	SPR_OBJ	**fst = work->fst[prim_num];
	SPR_OBJ	**snd = work->snd[prim_num];
	int		w,h;

	SPR_LoadTexture(TRI_CODE);

	if(work->step[prim_num]&0xf000){
		printf("telop prim full\n");
		return -1;
	}
	
	for( i = 0; i < 2; i++ )
   {
      // scale the text pos in by a few % to avoid safe zone issues.
      pos->x = (((((((TEX_SIZE_W/2)+pos->x)/512.0f)-0.5f)*0.98f)+0.5f)*512.0f)-(TEX_SIZE_W/2);
      pos->x+= (256-TEX_SIZE_W)/2;

		SPR_SetPosSprite( *fst, (SPR_POS*)pos);
		SPR_ObjSetTexture( *fst, tex_code[i], 0);
		DG_GetTexelInfo( &w, &h, NULL, NULL, ((SPR_SPRITE*)*fst)->head.tex.dgtex );
		SPR_SetSizeSprite(*fst, (float)w*TARGET_ASPECT_X, (float)(h));
		
		SPR_ObjSetTexture(*snd, GV_StrCode("alpha03_alp_ovl"), 0);
		SPR_SetSizeSprite(*snd, (float)w*TARGET_ASPECT_X, (float)(h));
		SetSpriteUV( (SPR_SPRITE*)*snd, TEX_SIZE_W/2.0f, TEX_SIZE_H*3.0f/8.0f,
					 TEX_SIZE_W/2.0f, TEX_SIZE_H/4.0f );

		SPR_SHOW(*fst);
		SPR_SHOW(*snd);

		fst++;
		snd++;
	}

	//printf("[%f,%f][%d,%d]\n",pos->vx,pos->vy,w,h);
	DG_COPY_VEC( &work->color[prim_num], &DG_ZeroVector );
	//work->step[prim_num] = STEP_SLEEP;

	work->h[prim_num] = (float)h;//TEX_SIZE_H/4.0f;
	work->off_x[prim_num] = TEX_SIZE_W/2.0f * rnd();
	work->off_y[prim_num] = (TEX_SIZE_H-(float)h)*rnd();//TEX_SIZE_H*3.0f/8.0f;
	work->add_x[prim_num] = ADD_OFFSET_0;
	work->step[prim_num] = STEP_FADEIN;
	work->alpha[prim_num] = 128;

	work->prim_num++;

	return 0;
}

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int	num;
	FVECTOR	pos;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		case MSG_TELOP_RUN:
			pos.vx = (float)msg->message[1];
			pos.vy = (float)msg->message[2];
			work->tex_num = msg->message[3];

			if( work->tex_num == DEF_SNAKE_TEX ) work->time_index = 1;
			else work->time_index = 0;
			if( work->tex_num > MAX_TEX ){ printf("テロップオーバーフロー\n"); }
			if( InitNewTelop( work, &pos ) ){
				printf("telop prim init err!!\n");
			}
			printf("[%d][%d]\n",work->tex_num,work->time_index);
			break;
		case MSG_TELOP_DEF_CHANGE:

			work->flags ^= TELOP_FLAG_TO_ENG;

			if(work->flags&TELOP_FLAG_TO_ENG){
				work->ratio = 0.0f;
				work->flags &= ~(TELOP_FLAG_ACTIVE);
			}else{
				work->ratio = 1.0f;
				work->flags &= ~(TELOP_FLAG_ACTIVE);
			}

			break;
		default:
			printf("telop.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return (0);
}

extern int DM_FrameSkip ;

static void Act( Work *work )
{
	SPR_OBJ	**fst;// = work->fst;
	SPR_OBJ	**snd;// = work->snd;
	CVECTOR	color;
	int		i,j,time,skip = DM_FrameSkip+1;
	float	ratio[2];

	CheckMesgParam( work );

	//pad check
#if 0
#if ENGLISH
#else
	if( GV_PadDataDirect[0].press & PAD_A ){
		work->flags |= TELOP_FLAG_ACTIVE;
		work->flags ^= TELOP_FLAG_TO_ENG;
	}
#endif
#endif
	//ratio 
	if(work->flags&TELOP_FLAG_ACTIVE){
		if(work->flags&TELOP_FLAG_TO_ENG){
			work->ratio -= 1.0f/16.0f*(float)skip;
			if(work->ratio < 0.0f){
				work->ratio = 0.0f;
				work->flags &= ~(TELOP_FLAG_ACTIVE);
			}
		}else{
			work->ratio += 1.0f/16.0f*(float)skip;
			if(work->ratio > 1.0f){
				work->ratio = 1.0f;
				work->flags &= ~(TELOP_FLAG_ACTIVE);
			}
		}
	}
	for( i = 0; i < MAX_TELOP; i++ ){
		time = (work->step[i])&0x0fff;

		switch(work->step[i]&0xf000){
		case STEP_SLEEP:
			break;
		case STEP_FADEIN:
			work->color[i].vx += ADD_COLOR_R0*(float)skip;
			work->color[i].vy += ADD_COLOR_G0*(float)skip;
			work->color[i].vz += ADD_COLOR_B0*(float)skip;

			work->add_x[i] += ADD_ADD_OFFSET_0*(float)skip;
			//work->off_y[i] -= ADD_OFF_Y0*(float)skip;
			//work->h[i] += ADD_OFF_Y0*2.0f*(float)skip;

			if( time >= TimeFadeIn[work->time_index] ){
				work->step[i] = STEP_PRE_WAIT;
				work->add_x[i] = ADD_OFFSET_1;
				time = 0;
			}

			break;
		case STEP_PRE_WAIT:
			work->color[i].vx += SUB_COLOR_R0*(float)skip;
			work->color[i].vy += SUB_COLOR_G0*(float)skip;
			work->color[i].vz += SUB_COLOR_B0*(float)skip;

			work->add_x[i] += ADD_ADD_OFFSET_0*(float)skip;
			//work->off_y[i] -= ADD_OFF_Y0*(float)skip;
			//work->h[i] += ADD_OFF_Y0*2.0f*(float)skip;

			if( time >= TimePreWait[work->time_index] ){
				work->step[i] = STEP_WAIT;
				work->add_x[i] = ADD_OFFSET_1;
				time = 0;
			}

			break;
		case STEP_WAIT:
			if(time >= TimeWait[work->time_index]){
				work->step[i] = STEP_AFT_WAIT;
				time = 0;
			}
			break;
		case STEP_AFT_WAIT:
			//work->color[i].vx += ADD_COLOR_R1*(float)skip;
			//work->color[i].vy += ADD_COLOR_G1*(float)skip;
			//work->color[i].vz += ADD_COLOR_B1*(float)skip;

			work->add_x[i] += ADD_ADD_OFFSET_1*(float)skip;
			//work->off_y[i] += ADD_OFF_Y1*(float)skip;
			//work->h[i] -= ADD_OFF_Y1*2.0f*(float)skip;

			work->alpha[i] -= 2*skip;

			if(time >= TimeAftWait[work->time_index] ){
				work->step[i] = STEP_FADEOUT;
				time = 0;
			}
			break;
		case STEP_FADEOUT:
			//work->color[i].vx += SUB_COLOR_R1*(float)skip;
			//work->color[i].vy += SUB_COLOR_G1*(float)skip;
			//work->color[i].vz += SUB_COLOR_B1*(float)skip;

			work->add_x[i] += ADD_ADD_OFFSET_1*(float)skip;
			//work->off_y[i] += ADD_OFF_Y1*(float)skip;
			//work->h[i] -= ADD_OFF_Y1*2.0f*(float)skip;

			work->alpha[i] -= 2*skip;

			if( time >= TimeFadeOut[work->time_index]){
				SPR_HIDE(work->fst[i][0]);
				SPR_HIDE(work->fst[i][1]);
				work->step[i] = STEP_SLEEP;
				time = 0;
			}
			break;
		default:
			printf("STEP ERR!!!\n");
		}
		work->step[i] = (work->step[i]&0xf000)|((time + skip)&0x0fff);

		if(work->step[i]&0xf000){
			work->off_x[i] -= work->add_x[i]*(float)skip;
			if(work->off_x[i] < 0.0f){
				work->off_x[i] += TEX_SIZE_W/2.0f;
			}
#if 0
			if(work->off_y[i] < 0.0f){
				work->off_y[i] = 0.0f;
			}
			if(work->h[i] > TEX_SIZE_H){
				work->h[i] = TEX_SIZE_H;
			}
#endif
			ratio[0] = work->ratio;
			ratio[1] = 1.0f - work->ratio;
		
			//printf("h[%f]off_x[%f]add_x[%f]off_y[%f]\n",work->h[i],work->off_x[i],work->add_x[i],work->off_y[i]);
			
			if(work->color[i].vx > 255.0f){
				work->color[i].vx = 255.0f;
			}
			if(work->color[i].vy > 255.0f){
				work->color[i].vy = 255.0f;
			}
			if(work->color[i].vz > 255.0f){
				work->color[i].vz = 255.0f;
			}
			if( work->alpha[i] < 0 ) work->alpha[i] = 0;

			fst = work->fst[i];
			snd = work->snd[i];
			for( j = 0; j < 2; j++ ){
				if(ratio[j] == 0.0f){
					SPR_HIDE(*fst);
				}else{
					color.r = (int)(work->color[i].vx*ratio[j]);
					color.g = (int)(work->color[i].vy*ratio[j]);
					color.b = (int)(work->color[i].vz*ratio[j]);

					color.cd = 0;
					SET_COLOR_2DPRIM3( *snd, 0x80, (*(int*)&color) );
					SET_COLOR_2DPRIM3( *fst, work->alpha[i], 0x00 );
					SetSpriteUV( (SPR_SPRITE*)*snd, (int)work->off_x[i], (int)work->off_y[i],
								 (int)TEX_SIZE_W/2.0f, (int)work->h[i] );
					//printf("col[%d][%d][%3d:%3d:%3d]\n",i,j,color.r,color.g,color.b);
					SPR_SHOW(*fst);
			
				}
				fst++; snd++;
			}
		}
	}
				
		
	//printf("off_y %f size_h %f\n",work->off_y,work->h);
	   


//	SPR_SHOW(fst);
//	SPR_SHOW(snd);
//	SPR_HIDE(thd);

//	SET_COLOR_2DPRIM3( fst, 128, work->col );
//	SET_COLOR_2DPRIM3( snd, 128, work->col );
//	SET_COLOR_2DPRIM3( thd, 128, work->col );

}

static void Die( Work *work )
{
//	int i;

	if(work->fst[0][0]) SPR_Destroy_2D_Object(work->fst[0][0]);
	if(work->fst[1][0]) SPR_Destroy_2D_Object(work->fst[1][0]);
	if(work->fst[2][0]) SPR_Destroy_2D_Object(work->fst[2][0]);
	if(work->fst[3][0]) SPR_Destroy_2D_Object(work->fst[3][0]);

	TelopWork = NULL;
}


static int GetResources( Work *work, int name )
{
	int				i,j;
	SPR_OBJ   		*papa;
	SPR_OBJ   		*obj;

	work->name = name;
	work->flags = 0;
	work->ratio = 1.0f;
	work->tex_num = 0;
	work->prim_num = 0;

	SPR_LoadTexture(TRI_CODE);

	for( i = 0; i < MAX_TELOP; i++ ){
		DG_COPY_VEC( &work->color[i], &DG_ZeroVector );
		work->step[i] = STEP_SLEEP;
		work->h[i] = 0.0f;
		work->off_x[i] = 0.0f;
		work->off_y[i] = 0.0f;
		work->add_x[i] = 0.0f;

		for( j = 0; j < 2; j++ ){
		
			//アルファのみ	文字
			work->fst[i][j] = papa = SPR_Create_2D_Object(SP_SPRITE, CHANL_NUM, NULL);
			if(papa == NULL){ printf("ERR!! Make2DObj[work->telop_add]\n"); return -1; }
			papa->head.alpha = SCE_GS_SET_ALPHA(2, 2, 0, 1, 0x00);
			papa->head.flags |= SPR_FLAG_ALPHA ;
         papa->head.flags |= SPR_FLAG_NOALPHATEST ;
#ifdef KP_XBOX
			SPR_SetColorSprite( papa, 128, 128, 128, 128) ;
#else
			SET_COLOR_2DPRIM(papa,0x80000000);
#endif

			//加算		煙
			work->snd[i][j] = obj = SPR_Create_2D_Object(SP_SPRITE, CHANL_NUM, papa);
			if(obj == NULL){ printf("ERR!! Make2DObj[obj]\n"); return -1; }
#ifdef BP_PS2
			SPR_SetPosSprite(obj, &(SPR_POS){ 0.0f, 0.0f});
#else
			{
				SPR_POS tmp = { 0.0f, 0.0f} ;
				SPR_SetPosSprite(obj, &tmp );
			}
#endif
			obj->head.alpha = SCE_GS_SET_ALPHA(0,2, 1, 1, 0x00);
			//obj->head.alpha = SCE_GS_SET_ALPHA(0, 2, 0, 1, 64);
			obj->head.flags |= SPR_FLAG_ALPHA;
			SET_COLOR_2DPRIM(obj,0);

		}
	}

	return 0;
}

void *NewRunTelop_Trial( int name )
{
	Work	*work;

	//DG_FrameCount = 2;

	if(TelopWork) return NULL;
	//return NULL;
	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_OBJECT,
								  sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;

		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
		TelopWork = work;
	}

	return work ;
}
