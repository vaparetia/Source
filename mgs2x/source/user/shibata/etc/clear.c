//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    clear.c
    クリアコード表示
	2000/11/10 T.Shibata

	$Id: clear.c,v 1.1.1.3 2002/11/19 11:48:43 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"rankenc.h"

#define		MAX_CODECHAR	(12)

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))
#define SET_COLOR_2DPRIM3(_prim,_a,_col) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | (_col)))
#define	TRN2DFIG_Y(_y)	(((_y)*384.0f)/448.0f)

#define LOCATE_X	(34.0f)
#define LOCATE_Y	(TRN2DFIG_Y(177.0f))

#define STR_SPACE	(4.0f)

#define CHARSIZE_X	(18.0f*1.2f)
#define CHARSIZE_Y	(TRN2DFIG_Y(14.0f*1.6f))

#define TEXSIZE_X	(18.0f)
#define TEXSIZE_Y	(14.0f)

#define N_TEXCHAR_X	(10)
#define N_TEXCHAR_Y	(8)

#define	TEX_CODE_CL_BG	(14795731)
#define TEX_CODE_CL_CD	(12025773)

#define	CHAR_COLOR		(0x003f24cd)
//#define	CHAR_COLOR		(0x003f24a0)
//#define	CHAR_COLOR		(0x00808080)
#define VIS_INTERVAL	(16)

typedef	struct {
	GV_ACT_EX		actor;
	char			string[16];
	int				end_proc;
	short			timer;
	short			step;

	short			s_step;
	unsigned short	now_chara;
	int				eng;

	SPR_OBJ   		*sprite[4];
	SPR_OBJ   		*url[4][2];
	SPR_OBJ   		*code[MAX_CODECHAR];
} Work;

enum {
	STEP_CLCODE_FADEIN = 0,
	STEP_CLCODE_CHARA,
	STEP_CLCODE_BG,
	STEP_CLCODE_WAIT,
	STEP_CLCODE_FADEOUT,
	STEP_CLCODE_KILL,
};

static FVECTOR SpriteData[4] = {
	{  0.0f, TRN2DFIG_Y(  0.0f), 512.0f, TRN2DFIG_Y(448.0f)},
	{ 32.0f, TRN2DFIG_Y(162.0f), 112.0f, TRN2DFIG_Y( 10.0f)},
	{ 32.0f, TRN2DFIG_Y(184.0f),  10.0f, TRN2DFIG_Y(  8.0f)},
	{  0.0f, TRN2DFIG_Y( 48.0f), 512.0f, TRN2DFIG_Y(288.0f)},
};

static int SpriteTexData[4][2] = {
	{             -1, 0x80000000 },
	{ TEX_CODE_CL_CD, 0x00808080 },
	{             -1, CHAR_COLOR },
	{ TEX_CODE_CL_BG, 0x00808080 }
};

static int URL_TexCode[4][2] = {
	{ 10716029, 10781565 },	//pc
	{  7490927,  7556463 },	//ez_web
	{  9159636,  9225172 },	//i_mode
	{ 11256788, 11322324 }, //j_sky
};

static FVECTOR URL_TexData[4][2] = {
	{
		{  36.0f, TRN2DFIG_Y(309.0f), 232.0f, TRN2DFIG_Y(10.0f)},
		{ 299.0f, TRN2DFIG_Y(302.0f), 178.0f, TRN2DFIG_Y(20.0f)},
	},
	{
		{  35.0f, TRN2DFIG_Y(307.0f),  70.0f, TRN2DFIG_Y(12.0f)},
		{ 253.0f, TRN2DFIG_Y(302.0f), 224.0f, TRN2DFIG_Y(20.0f)},
	},
	{
		{  35.0f, TRN2DFIG_Y(305.0f),  64.0f, TRN2DFIG_Y(14.0f)},
		{ 276.0f, TRN2DFIG_Y(302.0f), 202.0f, TRN2DFIG_Y(20.0f)},
	},
	{
		{  35.0f, TRN2DFIG_Y(305.0f),  64.0f, TRN2DFIG_Y(14.0f)},
		{ 276.0f, TRN2DFIG_Y(303.0f), 202.0f, TRN2DFIG_Y(20.0f)},
	},
};

#define CEL_NUM_W	(32)
#define CEL_NUM_H	(3)

#ifdef ENGLISH
#define CEL_SIZE_W	(14)
#define CEL_SIZE_H	(18)

static float FontSize_X[32*3] = {
	18.0f,  3.0f,  6.0f, 11.0f, 10.0f, 14.0f, 12.0f,  3.0f,
	 6.0f,  6.0f,  8.0f, 11.0f,  3.0f,  7.0f,  3.0f,  7.0f,
	11.0f,  5.0f, 10.0f, 10.0f, 11.0f, 10.0f, 11.0f,  9.0f,
	11.0f, 10.0f,  4.0f,  3.0f,  9.0f, 10.0f,  9.0f,  9.0f,

	14.0f, 12.0f, 11.0f, 11.0f, 11.0f,  9.0f, 10.0f, 12.0f,
	10.0f,  3.0f,  7.0f, 11.0f,  9.0f, 12.0f, 11.0f, 12.0f,
	10.0f, 12.0f, 10.0f, 10.0f, 11.0f, 11.0f, 12.0f, 14.0f,
	11.0f, 11.0f, 10.0f,  5.0f, 11.0f,  5.0f, 10.0f, 18.0f,

	18.0f, 10.0f,  9.0f, 10.0f,  9.0f,  9.0f,  6.0f,  9.0f,
	 9.0f,  3.0f,  6.0f,  8.0f,  3.0f, 13.0f,  9.0f, 10.0f,
	 9.0f,  9.0f,  6.0f,  9.0f,  7.0f,  9.0f,  9.0f, 14.0f,
	 9.0f, 10.0f,  9.0f,  5.0f,  2.0f,  5.0f, 18.0f, 18.0f,
};

#else
#define CEL_SIZE_W	(16)
#define CEL_SIZE_H	(16)

static float FontSize_X[32*3] = {
	16.0f,  5.0f,  7.0f, 12.0f, 13.0f, 14.0f, 16.0f,  3.0f,
	 5.0f,  5.0f, 12.0f, 11.0f,  4.0f, 11.0f,  4.0f, 13.0f,
	16.0f,  8.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f,
	16.0f, 16.0f,  4.0f,  4.0f,  9.0f,  9.0f,  9.0f, 14.0f,

	16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f,
	16.0f,  5.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f,
	16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f,
	16.0f, 16.0f, 16.0f,  6.0f, 13.0f,  6.0f,  7.0f, 11.0f,

	16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f,
	16.0f,  5.0f, 10.0f, 16.0f,  7.0f, 16.0f, 16.0f, 16.0f,
	16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f,
	16.0f, 16.0f, 16.0f,  8.0f,  3.0f,  8.0f, 16.0f,  4.0f,
};

#endif

static void SetSpriteUV( SPR_SPRITE *prim, int u, int v, int w, int h  )
{
	SPR_TEX		*tex = &prim->head.tex;
	int			tw,th,off_u,off_v;

	DG_GetTexelInfo( &tw, &th, &off_u, &off_v, tex->dgtex );
//	printf("off_u %d:off_v %d:size_w %d:size_h %d\n",off_u,off_v,tw,th);
//	printf("u %d:v %d:w %d:h %d\n",u,v,w,h);
#ifdef PSX2 /*2001.04.04yano*/
	tex->u = ((off_u + u)<<4) + (1<<3);
	tex->v = ((off_v + v)<<4) + (1<<3);
	tex->w = (w<<4) - (1<<4);
	tex->h = (h<<4) - (1<<4);
#else
	tex->u = SPR_FIXED( off_u + u + 0.5f );
	tex->v = SPR_FIXED( off_v + v + 0.5f );
	tex->w = SPR_FIXED(w - 1);
	tex->h = SPR_FIXED(h - 1);
#endif
}

static void ClCode_Skip( Work *work )
{
	GV_PAD	*pad = &GV_PadDataDirect[0];

	if(pad->press & (PAD_OK|PAD_STA)){
		int	i;

		SET_COLOR_2DPRIM3(work->sprite[1],128,SpriteTexData[1][1]);
		SET_COLOR_2DPRIM3(work->sprite[2],128,SpriteTexData[2][1]);
		SET_COLOR_2DPRIM3(work->sprite[3],128,SpriteTexData[3][1]);

		SET_COLOR_2DPRIM(work->url[0][0],0x80808080);
		SET_COLOR_2DPRIM(work->url[0][1],0x80808080);

		for( i = 0; i < MAX_CODECHAR; i++ ){
			SET_COLOR_2DPRIM3(work->code[i],128,CHAR_COLOR);
		}
		SPR_SHOW(work->sprite[2]);
		SPR_SHOW(work->sprite[3]);
		SPR_SHOW(work->url[0][0]);
		SPR_SHOW(work->url[0][1]);
		work->step = STEP_CLCODE_WAIT;
		work->timer = 128;
		work->now_chara = 0;

		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_TYPING02);
	}
}

static void ClCode_FadeIn( Work *work )
{
	SET_COLOR_2DPRIM3(work->sprite[1],work->timer<<2,SpriteTexData[1][1]);

	if(++work->timer > 32){
		work->step = STEP_CLCODE_CHARA;
		work->timer = 0;

		SPR_SHOW(work->sprite[2]);
	}
	ClCode_Skip( work );
}

static void ClCode_Chara( Work *work )
{
	if(++work->timer > VIS_INTERVAL){
		int now_chara = work->now_chara;
		if(now_chara < 12){
			SET_COLOR_2DPRIM3(work->code[now_chara],128,CHAR_COLOR);
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_TYPING01);
		}else{
			work->step = STEP_CLCODE_BG;
			work->timer = 0;
			work->now_chara = 0;
			SPR_SHOW(work->sprite[3]);
			SPR_SHOW(work->url[0][0]);
			SPR_SHOW(work->url[0][1]);
			return;
		}
		work->timer = 0;
		work->now_chara++;
	}
	ClCode_Skip( work );
}

static void ClCode_BGround( Work *work )
{
	SET_COLOR_2DPRIM3(work->sprite[3],work->timer<<1,SpriteTexData[3][1]);

	SET_COLOR_2DPRIM3(work->url[0][0],work->timer<<1,0x00808080);
	SET_COLOR_2DPRIM3(work->url[0][1],work->timer<<1,0x00808080);

	if(++work->timer > 64){
		work->step = STEP_CLCODE_WAIT;
		work->timer = 128;
		SET_COLOR_2DPRIM3(work->sprite[2],128,SpriteTexData[2][1]);
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_TYPING02);
	}
	ClCode_Skip( work );
}

static void ClCode_Wait( Work *work )
{
	GV_PAD	*pad = &GV_PadDataDirect[0];
	int alpha = ++work->timer % 64;
	int alpha1;
	
	if(work->eng){
		alpha1 = 129;
	}else{
		alpha1 = work->timer % (128+256+32);
	}

	alpha = abs(alpha - 32);
	SET_COLOR_2DPRIM3(work->sprite[2],alpha<<1,SpriteTexData[2][1]);
	
	if(alpha1 == 0){
		if(work->timer == (128+256+32)<<1) work->timer = 0;
		SPR_HIDE(work->url[work->now_chara&0x3][0]);
		SPR_HIDE(work->url[work->now_chara&0x3][1]);
		work->now_chara++;
		SPR_SHOW(work->url[work->now_chara&0x3][0]);
		SPR_SHOW(work->url[work->now_chara&0x3][1]);
		SET_COLOR_2DPRIM(work->url[work->now_chara&0x3][0],0);
		SET_COLOR_2DPRIM(work->url[work->now_chara&0x3][1],0);
	}else if(alpha1 < 128){
		SET_COLOR_2DPRIM3(work->url[work->now_chara&0x3][0],alpha1,0x00808080);
		SET_COLOR_2DPRIM3(work->url[work->now_chara&0x3][1],alpha1,0x00808080);
		//printf("alpha = [%d]\n",alpha1);
	}else if(alpha1 > 128+256){
		alpha1 = 128 - ((alpha1 - (128+256))<<2);
		SET_COLOR_2DPRIM3(work->url[work->now_chara&0x3][0],alpha1,0x00808080);
		SET_COLOR_2DPRIM3(work->url[work->now_chara&0x3][1],alpha1,0x00808080);
		//printf("alpha = [%d]\n",alpha1);
	}

	if(pad->press & (PAD_OK|PAD_STA)){
		work->timer = 64;
		work->step = STEP_CLCODE_FADEOUT;

		SET_COLOR_2DPRIM3(work->sprite[2],128,SpriteTexData[2][1]);
		
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
	}
}

static void ClCode_FadeOut( Work *work )
{
	int	i;

	SET_COLOR_2DPRIM3(work->sprite[1],work->timer<<1,SpriteTexData[1][1]);
	SET_COLOR_2DPRIM3(work->sprite[2],work->timer<<1,SpriteTexData[2][1]);
	SET_COLOR_2DPRIM3(work->sprite[3],work->timer<<1,SpriteTexData[3][1]);
	SET_COLOR_2DPRIM3(work->url[work->now_chara&0x3][0],work->timer<<1,0x00808080);
	SET_COLOR_2DPRIM3(work->url[work->now_chara&0x3][1],work->timer<<1,0x00808080);

	for( i = 0; i < MAX_CODECHAR; i++ ){
		SET_COLOR_2DPRIM3(work->code[i],work->timer<<1,CHAR_COLOR);
	}

	if(--work->timer < 0){
		work->step = STEP_CLCODE_KILL;
		work->timer = 0;
	}
}

static void ClCode_Kill( Work *work )
{
	GCL_ExecProc( work->end_proc, NULL );
	GV_DestroyActor( work );
}

static void Act( Work *work )
{
//	GV_PAD	*pad = &GV_PadDataDirect[0];
#if 0
	MENU_Locate( 32, 32, MENU_MODE_LEFT );
	MENU_Printf( "%s",work->string );
#endif
	
	switch(work->step){
	case STEP_CLCODE_FADEIN:
		ClCode_FadeIn(work);
		break;
	case STEP_CLCODE_CHARA:
		ClCode_Chara(work);
		break;
	case STEP_CLCODE_BG:
		ClCode_BGround(work);
		break;
	case STEP_CLCODE_WAIT:
		ClCode_Wait(work);
		break;
	case STEP_CLCODE_FADEOUT:
		ClCode_FadeOut(work);
		break;
	case STEP_CLCODE_KILL:
		ClCode_Kill(work);
		break;
	default:
		printf("cl_step ERR!! \n");
		break;
	}
/*
	if(pad->press & (PAD_OK|PAD_STA)){
		GCL_ExecProc( work->end_proc, NULL );
		GV_DestroyActor( work );
	}
*/
}

static void Die( Work *work )
{
	int i;

	for( i = 0; i < 4; i++ ){
		if(work->sprite[i]) SPR_Destroy_2D_Object(work->sprite[i]);
		if(work->url[i][0]) SPR_Destroy_2D_Object(work->url[i][0]);
		if(work->url[i][1]) SPR_Destroy_2D_Object(work->url[i][1]);
	}

	for( i = 0; i < MAX_CODECHAR; i++ ){
		if(work->code[i]) SPR_Destroy_2D_Object(work->code[i]);
	}
}

static int SpriteInit( Work *work )
{
	SPR_OBJ	*sprite;
	int		i,tri_num;
	FVECTOR	*data = SpriteData;
	SPR_POS	locate = {LOCATE_X,LOCATE_Y};

	tri_num = SPR_LoadTexture(15895067);

	for( i = 0; i < 4; i++ ){
		sprite = work->sprite[i] = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		if(SpriteTexData[i][0] > 0) SPR_ObjSetTexture(sprite, SpriteTexData[i][0], 0);

		SPR_SetPosSprite(sprite, (SPR_POS*)data);
		SPR_SetSizeSprite(sprite, data->vz, data->vw );
		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		SET_COLOR_2DPRIM3(sprite,0x00,SpriteTexData[i][1]);
		data++;

		
		sprite = work->url[i][0] = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		SPR_ObjSetTexture(sprite, URL_TexCode[i][0], tri_num);
		SPR_SetPosSprite(sprite, (SPR_POS*)&URL_TexData[i][0]);
		SPR_SetSizeSprite(sprite, URL_TexData[i][0].vz, URL_TexData[i][0].vw );
		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		SET_COLOR_2DPRIM3(sprite,0x00,0x00808080);

		sprite = work->url[i][1] = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		SPR_ObjSetTexture(sprite, URL_TexCode[i][1], tri_num);
		SPR_SetPosSprite(sprite, (SPR_POS*)&URL_TexData[i][1]);
		SPR_SetSizeSprite(sprite, URL_TexData[i][1].vz, URL_TexData[i][1].vw );
		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		SET_COLOR_2DPRIM3(sprite,0x00,0x00808080);

	}

	SPR_SetPriority( work->sprite[0], 0);
	SPR_SetPriority( work->sprite[1], 2);
	SPR_SetPriority( work->sprite[2], 2);
	SPR_SetPriority( work->sprite[3], 1);

	for( i = 0; i < MAX_CODECHAR && work->string[i]; i++ ){
		char c_data = work->string[i] - 0x20;

		sprite = work->code[i] = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }

		SPR_ObjSetTexture(sprite, 5407817, tri_num);

		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		SPR_SetPosSprite(sprite, &locate);
		SET_COLOR_2DPRIM3(sprite,0x00,CHAR_COLOR);

//ここでサイズチェック
		SetSpriteUV( (SPR_SPRITE*)sprite, CEL_SIZE_W*(c_data%32), CEL_SIZE_H*(c_data/32)-(c_data/32),
					 (int)FontSize_X[(int)c_data], CEL_SIZE_H+1 );
//		SPR_SetSizeSprite(sprite, FontSize_X[(int)c_data], CHARSIZE_Y );
//		locate.x += FontSize_X[(int)c_data] + STR_SPACE;

		SPR_SetSizeSprite(sprite, FontSize_X[(int)c_data]*1.2f, CHARSIZE_Y );

		if(i && i%4 == 3) locate.x += 8.0f;//CHARSIZE_X;
		locate.x += FontSize_X[(int)c_data]*1.2f + STR_SPACE;
		SPR_SHOW(sprite);
		SPR_SetPriority(sprite, 2);
	}

	locate.y = TRN2DFIG_Y(190.0f);
	locate.x -= 4.0f;
	SPR_SetPosSprite(work->sprite[2], &locate);

	SPR_SHOW(work->sprite[0]);
	SPR_SHOW(work->sprite[1]);
	return 0;
}

static int GetResources( Work *work )
{
	int			seed;//,mode;
	INPUT_DATA	base_code[2];

	work->timer = 0;
	work->step = 0;
	work->now_chara = 0;
	work->s_step = 0;

	work->end_proc = GCL_GetOptionValue( 'p', 0 );

	work->eng = GCL_GetOptionValue( 'e', 0 );

	seed = GCL_GetOptionValue( 's', 0 );

	if(!GCL_GetOption('d')) return -1;

	printf("シード値 %d\n",seed);
	base_code[0].data = GCL_GetNextInt();
	base_code[0].len = GCL_GetNextInt();
	base_code[1].data = GCL_GetNextInt();
	base_code[1].len = GCL_GetNextInt();

	printf("input data[0] data[%x]len[%d]\n",base_code[0].data,base_code[0].len);
	printf("input data[1] data[%x]len[%d]\n",base_code[1].data,base_code[1].len);

	encode( work->string, base_code, 2, seed );
	printf("encode [%s]\n",work->string);

	base_code[0].data = 0;
	base_code[1].data = 0;

	decode( base_code, 2, work->string );
	printf("decode data[0] data[%x]len[%d]\n",base_code[0].data,base_code[0].len);
	printf("decode data[1] data[%x]len[%d]\n",base_code[1].data,base_code[1].len);

#if 0
	{
		extern void *NewJimakuDirect( char *mes );
		extern void JimakuSetPos( void *w, int posx, int posy );
		static char str[16] = "0000ggggqqqq";
//		void *a = NewJimakuDirect( str );
		memcpy( work->string, str, strlen(str) );
	}
#endif

	return SpriteInit(work);
}

void *NewClearCode( int name, int map )
{
	Work		*work;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		printf("clear code\n");
	}

	return work ;
}

