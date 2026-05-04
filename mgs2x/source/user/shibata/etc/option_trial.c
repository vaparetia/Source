//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    option.c
	オプション画面 2000/08/28 T.Shibata

	$Id: option_trial.c,v 1.1.1.3 2002/11/19 11:48:44 Yoshizawa1 Exp $ 
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

#include 	"sprite_2d.h"
#include	"../util/ts_util.h"

#define	ACTOR_PRIO		(254)


#define	SELECT_COLOR_R	(160)
#define	SELECT_COLOR_G	( 40)
#define	SELECT_COLOR_B	( 30)

#define	NOT_SELECT_COLOR_R	(64)
#define	NOT_SELECT_COLOR_G	(90)
#define	NOT_SELECT_COLOR_B	(80)

/*/////////////////////////////////////////////
//		オプション
//
//	言語
//	振動		ON/OFF			(GM_CONFIG_VIBRATION_OFF)
//	字幕		ON/OFF			(GM_CONFIG_CAPTION_OFF)
//	血			ON/OFF			(GM_CONFIG_BLOOD_OFF)
//	サウンド	STE/MONO		(GM_CONFIG_SOUND_MONAURAL)
//
//	出る		EXIT
//
/////////////////////////////////////////////*/

enum {
	OPTN_MENU_VIB = 0,
	//	OPTN_MENU_RDR,
	OPTN_MENU_CPT,
	OPTN_MENU_BLD,
	OPTN_MENU_OWNV,
	OPTN_MENU_LETT, // BP FIX - was sound, now letterbox
	//	OPTN_MENU_DEF,
	OPTN_MENU_EXIT,
	OPTN_MENU_BOTTOM,
};

enum {
	OPTN_STEP_PRE = 0,
	OPTN_STEP_WAIT,
	OPTN_STEP_AFTER,
	OPTN_STEP_EXIT,
};

#if 0
static char *Optn_Menu_Str[] = {
	"Vibration",
	//	"Radar",
	"Caption",
	"Blood",
	"Sound",
	"Default",
	"Exit",
};
#endif

#define CHOICES_POS_X	(144.0f)
#define CHOICES_POS_Y	(219.43f)
#define	ADD_Y			(15.0f)
#define CHOICES_SIZE_X	(80.0f)
#define CHOICES_SIZE_Y	(10.0f)


#define ON_POS_X	(256.0f + 32.0f)
#define ON_POS_Y	(219.43f)
#define ONOFF_SIZE_X	(40.0f)
#define ONOFF_SIZE_Y	(10.0f)

static int Choices_V[] = {
	16*2,16*4,16*5,16*24,16*16,16*7,
};

typedef	struct {
	GV_ACT_EX	actor;
	int			mode;
	int			step;
	int			which;
	int			status;

	int				timer;
	SPR_OBJ			*head;

	SPR_OBJ   		*option;       				//オプション
	SPR_OBJ   		*on_off[OPTN_MENU_EXIT][2]; //オンオフ
	SPR_OBJ   		*choices[OPTN_MENU_BOTTOM];	//選択肢
} Work;

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))

#define	STATUS_DEFAULT	(0)
//#define TEX_CODE_STR		(GV_StrCode("menu_fnt_alp_ovl"))
#define TEX_CODE_STR		(9167091)


extern int sd_set_cli ( int );

//各種フラグを見て ステータスを返す
static int Load_Status()
{
	int status = 0;

//	printf("Load Status\n");
	if( GM_Configuration & GM_CONFIG_VIBRATION_OFF ) 	status |= (1<<OPTN_MENU_VIB);	//Vibration
//	if( GM_Configuration & GM_CONFIG_RADAR_OFF ) 		status |= (1<<OPTN_MENU_RDR);	//Rader
	if( GM_Configuration & GM_CONFIG_CAPTION_OFF ) 		status |= (1<<OPTN_MENU_CPT);	//Caption
	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) 		status |= (1<<OPTN_MENU_BLD);	//Blood
	if( GM_Configuration & GM_CONFIG_CUTSCENES_LETTERBOXED ) 	status |= (1<<OPTN_MENU_LETT);	//Letterbox
	if( GM_Configuration & GM_CONFIG_SHUKAN_REVERSE ) 	status |= (1<<OPTN_MENU_OWNV);	//own_view

	return status;
}

#define CONFIG_FLAG_ALLOFF (~(GM_CONFIG_VIBRATION_OFF|GM_CONFIG_CAPTION_OFF|GM_CONFIG_BLOOD_OFF|GM_CONFIG_CUTSCENES_LETTERBOXED|GM_CONFIG_SHUKAN_REVERSE))
//ステータスを見て 各種フラグを書き換える
static void Save_Status( int status )
{
#ifdef DEBUG_MODE
	printf("Save Status %x\n",status);
	printf("Vibration "); (status & (1<<OPTN_MENU_VIB))?printf("OFF\n"):printf("ON\n");
//	printf("Rader     "); (status & (1<<OPTN_MENU_RDR))?printf("OFF\n"):printf("ON\n");
	printf("Caption   "); (status & (1<<OPTN_MENU_CPT))?printf("OFF\n"):printf("ON\n");
	printf("Blood     "); (status & (1<<OPTN_MENU_BLD))?printf("OFF\n"):printf("ON\n");
	printf("own_view  "); (status & (1<<OPTN_MENU_OWNV))?printf("REV\n"):printf("NOR\n");
	printf("Letterbox "); (status & (1<<OPTN_MENU_LETT))?printf("LET\n"):printf("FUL\n");
#endif
	GM_Configuration &= CONFIG_FLAG_ALLOFF;
	if( status & (1<<OPTN_MENU_VIB)) GM_Configuration |= GM_CONFIG_VIBRATION_OFF;	//Vibration
//	if( status & (1<<OPTN_MENU_RDR)) GM_Configuration |= GM_CONFIG_RADAR_OFF;		//Rader
	if( status & (1<<OPTN_MENU_CPT)) GM_Configuration |= GM_CONFIG_CAPTION_OFF;		//Caption
	if( status & (1<<OPTN_MENU_BLD)) GM_Configuration |= GM_CONFIG_BLOOD_OFF;		//Blood
	if( status & (1<<OPTN_MENU_OWNV)) GM_Configuration |= GM_CONFIG_SHUKAN_REVERSE;	//own_view

	if( status & (1<<OPTN_MENU_LETT)){
		GM_Configuration |= GM_CONFIG_CUTSCENES_LETTERBOXED;
//		sd_set_cli( 0xff000005 ) ; // BP CHANGE
	}else{
//		sd_set_cli( 0xff000006 ) ; // BP CHANGE
	}


	printf("Configuration[%x]\n",GM_Configuration);
																						  
}

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
	tex->u = SPR_FIXED( off_u + u + 0.5f );
	tex->v = SPR_FIXED( off_v + v + 0.5f );
	tex->w = SPR_FIXED(w - 1);
	tex->h = SPR_FIXED(h - 1);
#endif
}

#if 0
//表示系
static int OptionWinView( Work *work )
{
	int		i,y = 128;

	MENU_SetColor( 64, 90, 80 );
	
	for( i = 0; i < OPTN_MENU_BOTTOM; i++ ){
		MENU_Locate( 160, y, MENU_MODE_LEFT );
		if( i != work->which ){
			//MENU_Printf( "%s",Optn_Menu_Str[i] );
		}else{
			MENU_SetColor( 160, 40, 30 );
			//MENU_Printf( "%s",Optn_Menu_Str[i] );
			MENU_SetColor( 64, 90, 80 );
		}
		
		if( i < OPTN_MENU_EXIT ){
			MENU_Locate( 256+16, y, MENU_MODE_LEFT );
			if( work->status & (1<<i) ){	//OFF
				MENU_Locate( 288+16, y, MENU_MODE_LEFT );
				MENU_Printf( "/" );
				MENU_Locate( 304+16, y, MENU_MODE_LEFT );
				MENU_SetColor( 160, 40, 30 );
				MENU_SetColor( 64, 90, 80 );
			}else{							//ON
				MENU_SetColor( 160, 40, 30 );
				MENU_SetColor( 64, 90, 80 );
				MENU_Locate( 288+16, y, MENU_MODE_LEFT );
				MENU_Printf( "/" );
				MENU_Locate( 304+16, y, MENU_MODE_LEFT );
			}
		}
		y+=8;
	}
	MENU_SetColor( 255, 255, 255 );
	return 0;
}
#endif
//pad系		戻り値  !=0:exit 
static int OptionWinPad( Work *work )
{
	GV_PAD	*pad = GV_PadDataDirect ;
	int		check = work->which;

	if ( pad->press & PAD_U ) work->which--;
	else if ( pad->press & PAD_D ) work->which++;
	
	if(work->which > OPTN_MENU_EXIT) work->which = OPTN_MENU_VIB;
	else if(work->which < OPTN_MENU_VIB) work->which = OPTN_MENU_EXIT;

	
	if( pad->press & (PAD_U|PAD_D) && (work->which^check) ){
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);
	}

	if ( (pad->press & PAD_R) && work->which < OPTN_MENU_EXIT ){	//OFF
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);
		work->status |= (1<<work->which);
	} else if ( (pad->press & PAD_L) && work->which < OPTN_MENU_EXIT ){	//ON
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);
		work->status &= ~(1<<work->which);
	} else if ( pad->press & PAD_OK ){	//決定
		//if( work->which == OPTN_MENU_DEF) work->status = STATUS_DEFAULT;
		if( work->which == OPTN_MENU_EXIT){
			Save_Status( work->status );
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);
			return -1;
		}
	} else if ( pad->press & PAD_CANCEL ) {
		Save_Status( work->status );
		return -1;
	}

	return 0;
}

static void Act( Work *work )
{
	int			i;

//	MENU_Locate( 256, 112, MENU_MODE_CENTER ) ;
//	MENU_SetColor( 255, 255, 255 ) ;
//	MENU_Printf( "OPTION\n" ) ;

	switch(work->step){
		
	case OPTN_STEP_PRE:
		SET_COLOR_2DPRIM2(work->option,0xe0,0xe0,0xe0,work->timer*4);

		for( i = 0; i < OPTN_MENU_BOTTOM; i++ ){
			if(work->which == i){
				SET_COLOR_2DPRIM2(work->choices[i],192,192,192,work->timer*4);
			}else{
				SET_COLOR_2DPRIM2(work->choices[i],64,90,80,work->timer*4);
			}
			if( i < OPTN_MENU_EXIT){
				if(work->status & (1<<i)){
					SET_COLOR_2DPRIM2(work->on_off[i][0],64,90,80,work->timer*4);
					SET_COLOR_2DPRIM2(work->on_off[i][1],160,40,30,work->timer*4);
				}else{
					SET_COLOR_2DPRIM2(work->on_off[i][0],160,40,30,work->timer*4);
					SET_COLOR_2DPRIM2(work->on_off[i][1],64,90,80,work->timer*4);
				}
			}
		}

		if(++work->timer > 32){
			work->timer = 0;
			work->step++;
		}
		break;
	case OPTN_STEP_WAIT:
		for( i = 0; i < OPTN_MENU_BOTTOM; i++ ){
			if(work->which == i){
				//SET_COLOR_2DPRIM2(work->choices[i],160,40,30,0x80);
				SET_COLOR_2DPRIM2(work->choices[i],192,192,192,0x80);
			}else{
				//SET_COLOR_2DPRIM2(work->choices[i],64,64,64,0x80);
				SET_COLOR_2DPRIM2(work->choices[i],64,90,80,0x80);
			}
			if( i < OPTN_MENU_EXIT){
				if(work->status & (1<<i)){
					//SET_COLOR_2DPRIM2(work->on_off[i][0],32,32,64,0x80);
					SET_COLOR_2DPRIM2(work->on_off[i][0],64,90,80,0x80);
					SET_COLOR_2DPRIM2(work->on_off[i][1],160,40,30,0x80);
				}else{
					SET_COLOR_2DPRIM2(work->on_off[i][0],160,40,30,0x80);
					//SET_COLOR_2DPRIM2(work->on_off[i][1],32,32,64,0x80);
					SET_COLOR_2DPRIM2(work->on_off[i][1],64,90,80,0x80);
				}
			}
		}
		if(OptionWinPad( work )){
			work->step++;	
		}
		break;
	case OPTN_STEP_AFTER:
		SET_COLOR_2DPRIM2(work->option,0xe0,0xe0,0xe0,128-work->timer*4);

		for( i = 0; i < OPTN_MENU_BOTTOM; i++ ){
			if(work->which == i){
				SET_COLOR_2DPRIM2(work->choices[i],192,192,192,128-work->timer*4);
			}else{
				SET_COLOR_2DPRIM2(work->choices[i],64,90,80,128-work->timer*4);
			}
			if( i < OPTN_MENU_EXIT){
				if(work->status & (1<<i)){
					SET_COLOR_2DPRIM2(work->on_off[i][0],64,90,80,128-work->timer*4);
					SET_COLOR_2DPRIM2(work->on_off[i][1],160,40,30,128-work->timer*4);
				}else{
					SET_COLOR_2DPRIM2(work->on_off[i][0],160,40,30,128-work->timer*4);
					SET_COLOR_2DPRIM2(work->on_off[i][1],64,90,80,128-work->timer*4);
				}
			}
		}
		
		if(++work->timer > 32){
			work->timer = 0;
			work->step++;
		}
		break;
	case OPTN_STEP_EXIT:
		//printf("Option Exit\n");
		GV_CallParentSignalFunc( work,1,0 );
		GV_DestroyActor( work );
		break;
	default:
		printf("Option Step Err!!\n");
		break;
	}

	//表示
	//OptionWinView( work );


}

static void Die( Work *work )
{
	
	if(work->head) SPR_Destroy_2D_Object(work->head);
}

static int GetResources( Work *work, int mode )
{
	int 		i;
	SPR_OBJ		*prim,*papa,*prim2;
	SPR_POS		pos;
	work->mode = mode;
	work->timer = 0;
	work->status = Load_Status();
	work->step = 0;
	work->which = 0;

	//SPR_LoadTexture(GV_StrCode("logo"));
	SPR_LoadTexture(3656015);
	//親分初期化
	work->head = papa = SPR_Create_2D_Object(SP_EMPTY, 0, NULL);
	if(papa == NULL){ printf("Err!! Make 2DPrim head\n"); return -1; }
#ifdef BP_PS2
	SPR_SetPosEmpty(papa, &(SPR_POS){0.0f,16.0f});
#else
	{
		SPR_POS tmp = {0.0f,16.0f} ;
		SPR_SetPosEmpty(papa, &tmp );
	}
#endif
	SPR_SHOW(papa);
	
	//オプション初期化
	work->option = prim = SPR_Create_2D_Object(SP_SPRITE, 0, papa);
	if(prim == NULL){ printf("Err!! Make 2DPrim option\n"); return -1; }
#ifdef BP_PS2
	SPR_SetPosSprite(prim, &(SPR_POS){256.0f - 48.0f * 0.7f, 192.0f } );
#else
	{
		SPR_POS tmp = {256.0f - 48.0f * 0.7f, 192.0f} ;
		SPR_SetPosSprite(prim, &tmp );
	}
#endif
	SPR_ObjSetTexture(prim, TEX_CODE_STR, 0);
	prim->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 80);
	prim->head.flags |= SPR_FLAG_ALPHA;
	SPR_SetSizeSprite(prim,160.0f * 0.7f, 16.0f * 0.7f);
	SetSpriteUV( (SPR_SPRITE*)prim, 0, 16, 160, 16  );
	SET_COLOR_2DPRIM(prim,0x00e0e0e0);
	SPR_SHOW(prim);
	
	for( i = 0; i < OPTN_MENU_BOTTOM; i++ ){
		//選択肢初期化
		work->choices[i] = prim = SPR_Create_2D_Object(SP_SPRITE, 0, papa);
		if(prim == NULL){ printf("Err!! Make 2DPrim choices[%d]\n",i); return -1; }

		pos.x = CHOICES_POS_X;
		pos.y = CHOICES_POS_Y + ADD_Y * (float)i;
		if( i == OPTN_MENU_EXIT ) pos.y += ADD_Y;
		SPR_SetPosSprite(prim, &pos );
		SPR_ObjSetTexture(prim, TEX_CODE_STR, 0);
		prim->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 80);
		prim->head.flags |= SPR_FLAG_ALPHA;
		SPR_SetSizeSprite(prim, 80.0f, 10.0f);
		SetSpriteUV( (SPR_SPRITE*)prim, 0, Choices_V[i], 160, 16  );
		SET_COLOR_2DPRIM(prim,0x00808080);
		SPR_SHOW(prim);
		//オンオフ初期化
		if( i < OPTN_MENU_EXIT){
			work->on_off[i][0] = prim = SPR_Create_2D_Object(SP_SPRITE, 0, papa);
			if(prim == NULL){ printf("Err!! Make 2DPrim on_off[%d][0]\n",i); return -1; }
			SPR_ObjSetTexture(prim, TEX_CODE_STR, 0);
			prim->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 80);
			prim->head.flags |= SPR_FLAG_ALPHA;
			SET_COLOR_2DPRIM(prim,0x00808080);
			SPR_SHOW(prim);

			work->on_off[i][1] = prim2 = SPR_Create_2D_Object(SP_SPRITE, 0, papa);
			if(prim2 == NULL){ printf("Err!! Make 2DPrim on_off[%d][1]\n",i); return -1; }
			SPR_ObjSetTexture(prim2, TEX_CODE_STR, 0);
			prim2->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 80);
			prim2->head.flags |= SPR_FLAG_ALPHA;
			SET_COLOR_2DPRIM(prim2,0x00808080);
			SPR_SHOW(prim2);

			if( i == OPTN_MENU_LETT ){
				pos.x = ON_POS_X - 16.0f;//32.0f;
				pos.y = ON_POS_Y + ADD_Y * (float)i;
				SPR_SetPosSprite(prim, &pos );
				pos.x = ON_POS_X + 40.0f + 16.0f;
				pos.y = ON_POS_Y + ADD_Y * (float)i;
				SPR_SetPosSprite(prim2, &pos );

				SPR_SetSizeSprite(prim, 40.0f + 32.0f, 10.0f);
				SetSpriteUV( (SPR_SPRITE*)prim, 0, 160, 160, 16 );
				//SPR_SetSizeSprite(prim2, 40.0f + 32.0f, 10.0f);
				SPR_SetSizeSprite(prim2, 96, 10.0f);
				SetSpriteUV( (SPR_SPRITE*)prim2, 0, 272, 160, 16 );
				//SetSpriteUV( (SPR_SPRITE*)prim2, 0, 208, 160, 16 );
			}else if(i == OPTN_MENU_OWNV){
				pos.x = ON_POS_X - 16.0f;//32.0f;
				pos.y = ON_POS_Y + ADD_Y * (float)i;
				SPR_SetPosSprite(prim, &pos );
				pos.x = ON_POS_X + 40.0f + 16.0f;
				pos.y = ON_POS_Y + ADD_Y * (float)i;
				SPR_SetPosSprite(prim2, &pos );

				SPR_SetSizeSprite(prim, 40.0f + 32.0f, 10.0f);
				SetSpriteUV( (SPR_SPRITE*)prim, 0, 16*12, 160, 16 );
				//SPR_SetSizeSprite(prim2, 40.0f + 32.0f, 10.0f);
				SPR_SetSizeSprite(prim2, 40.0f + 32.0f, 10.0f);
				SetSpriteUV( (SPR_SPRITE*)prim2, 0, 16*25, 160, 16 );

			}else{
				pos.x = ON_POS_X - 16.0f;
				pos.y = ON_POS_Y + (ADD_Y) * (float)i;
				SPR_SetPosSprite(prim, &pos );
				pos.x = ON_POS_X + 40.0f + 16.0f;
				pos.y = ON_POS_Y + ADD_Y * (float)i;
				SPR_SetPosSprite(prim2, &pos );

				SPR_SetSizeSprite(prim, 40.0f, 10.0f);
				SetSpriteUV( (SPR_SPRITE*)prim, 0, 128, 64, 16 );
				SPR_SetSizeSprite(prim2, 40.0f, 10.0f);
				SetSpriteUV( (SPR_SPRITE*)prim2, 0, 144, 64, 16 );
			}
		}
	}
	//printf("Option Start\n");
	return 0;
}

void *NewGameOption_Trial( int mode )
{
	Work		*work;

	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
