//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    2d_sprt.c
    
	2000/10/05 T.Shibata

	$Id: 2d_sprt_pause.c,v 1.1.1.3 2002/11/19 11:48:35 Yoshizawa1 Exp $
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

#include "BP_UIAdjust.h"

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

//#include	"camera.h"
#include 	"sprite_2d.h"
#include	"../util/ts_util.h"
#include "../../yamashita/2D/msn.h"

//#define	ACTOR_PRIO		(254)


//#define TRI_CODE		(GV_StrCode("telop"))
//#define TRI_CODE		(GV_StrCode("staff"))

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))
#define SET_COLOR_2DPRIM3(_prim,_a,_col) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | (_col)))

typedef	struct {
	GV_ACT_EX		actor;
	SPR_OBJ   		*sprite;
	int				name,map;
	short			alpha;
	short			step;
	short			time;
	short			flag;

} Work;

#define CHANL_NUM	(0)
#define FADEIN_TIME		(8)
#define FADEOUT_TIME	(8)

enum {
	STEP_INV_WAIT = 0,
	STEP_FADEIN,
	STEP_VIS_WAIT,
	STEP_FADEOUT,
};

enum {
	MSG_2DSPRT_PAUSE_UNLOCK,
	MSG_2DSPRT_PAUSE_LOCK,
	MSG_2DSPRT_PAUSE_KILL,
};


static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int	num;

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case MSG_2DSPRT_PAUSE_UNLOCK:
			work->flag &= ~2;
			break;
		  case MSG_2DSPRT_PAUSE_LOCK:
			work->flag |= 2;
			break;
		  case MSG_2DSPRT_PAUSE_KILL:
			return -1;
			break;
		  default:
			printf("telop.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return (0);
}

//2d_sprt.c
extern int g_PauseLock;

static void Act( Work *work )
{
   int newValue=0;
	int		alpha = 0;

	if(    (GM_GameStatus & STATE_VR_ANOTHER)
		&& (MSN_2DSTATUS & MSN_2DSTAT_WINDOW_PAUSE)){
		return;
	}
	if(GM_IsGameOver()){
		//GV_DestroyActor( work ) ;
		SPR_HIDE(work->sprite);
		return;
	}
//	if( CheckMesgParam( work ) < 0 ){	
//		GV_DestroyActor( work ) ;
//		SPR_HIDE(work->sprite);
//		return;
//	}
	if( g_PauseLock ){
		work->step = 0;
		SPR_HIDE(work->sprite);
		SET_COLOR_2DPRIM2(work->sprite,128,128,128,0);
		return;
	}
	switch(work->step)
   {
     case STEP_INV_WAIT:
      if(GV_PauseLevel & GV_PAUSE_PAUSE)
      {
			//ポーズ開始
			work->step = STEP_FADEIN;
			work->time = FADEIN_TIME;
			SPR_SHOW(work->sprite);
		}
		break;

     case STEP_FADEIN:
		alpha = (int)(128.0f*(float)(FADEIN_TIME - work->time)/(float)FADEIN_TIME);

		if(--work->time < 0)
      {
			work->step = STEP_VIS_WAIT;
			work->time = 90;
		}

		break;
	  case STEP_VIS_WAIT:

      //BP JG - pulse the alpha of the level name.
      work->time+=2;
      if ( work->time > 180 ) work->time = 0;
      newValue = 32+(96*sin(((float)work->time)*3.145/180.0f));
      alpha = newValue;

#ifdef DEBUG_MODE
		if( GV_PadDataDirect[0].press & PAD_SEL ) work->flag ^= work->flag;
#endif
		if(!(GV_PauseLevel&GV_PAUSE_PAUSE))
      {
			work->step = STEP_FADEOUT;
			work->time = FADEOUT_TIME;
		}

		break;
	  case STEP_FADEOUT:
      alpha = (int)(newValue*(float)work->time/(float)FADEOUT_TIME);

		if( --work->time < 0)
      {
			work->step = STEP_INV_WAIT;
			alpha = 0;
			SPR_HIDE(work->sprite);
		}
		break;
	default:
		printf("STEP ERR!!!\n");
	}
	SET_COLOR_2DPRIM2(work->sprite,work->alpha,work->alpha,work->alpha,alpha);
	//SET_COLOR_2DPRIM3(work->sprite,alpha,0x00000000);
#ifdef DEBUG_MODE
	if(work->flag&1){
		SPR_HIDE(work->sprite);
	}else{
		SPR_SHOW(work->sprite);
	}
#endif

#ifdef KP_XBOX
	if( (GV_PauseLevel&GV_PAUSE_PAUSE) ){
		// PAUSE表示がmustだって。
		if( GV_Time % 90 < 50 ){
			MENU_ResetColor();
			MENU_Locate( DRAW_WIDTH / 2, DRAW_HEIGHT / 8 * 5 - 32, MENU_MODE_CENTER );
			MENU_Printf( "PAUSED" );
		}
	}
#endif

}

static void Die( Work *work )
{
	if(work->sprite) SPR_Destroy_2D_Object(work->sprite);
#ifdef KP_XBOX
	GM_ReleasePadDisconnect();
#endif
	
}
/*
-tri $s:ＴＲＩコード
-x $s:ＴＥＸコード
-pos $i:座標ＸＹ二つ(0,0)<->(512,384)
-size $i:サイズＷＨ二つ(0,0)<->(512,384)
*/

#define FLAG_RIGHT 	(0x0001)
#define FLAG_BOTTOM (0x0002)
#define FLAG_CENTER (0x0010)
#define FLAG_PRIO 	(0x0100)

#define FLAG_ALPHA 	(0x0000)
#define FLAG_ADD 	(0x1000)
#define FLAG_SUB 	(0x2000)

static int GetResources( Work *work )
{
	int			tri_code;
	int			tex_code;
	int			w,h;
   float			new_w,new_h;
	float		   sw,sh;
	SPR_POS		pos;
	int			flag;
   DG_TEX*     pTexture;
	work->step = 0;

	tri_code = GCL_GetOptionValue( 't', 0 );
	tex_code = GCL_GetOptionValue( 'x', 0 );

	//SPR_LoadTexture(tri_code);
	work->sprite = SPR_Create_2D_Object(SP_SPRITE_F, 4, NULL);
	if(work->sprite == NULL){ printf("ERR!! Make2DObj[work->sprite]\n"); return -1; }
	SPR_ObjSetTexture(work->sprite, tex_code, SPR_LoadTexture(tri_code));

	if(GCL_GetOption('p'))
   {
		pos.x = (float)GCL_GetNextInt();
		pos.y = (float)GCL_GetNextInt();
	}
   else
   {
		pos.x = 0.0f;
		pos.y = 0.0f;
	}

   pTexture = ((SPR_SPRITE*)work->sprite)->head.tex.dgtex;

	if(GCL_GetOption('s'))
   {
		w = (float)GCL_GetNextInt();
		h = (float)GCL_GetNextInt();
	}
   else
   {
		DG_GetTexelInfo( &w, &h, NULL, NULL, pTexture );
	}

	if(GCL_GetOption('b'))
   {
		sw = (float)GCL_GetNextInt()*0.0001f;
		sh = (float)GCL_GetNextInt()*0.0001f;
	}
   else
   {
		sw = 1.0f;
		sh = 1.0f;
	}


	work->alpha = GCL_GetOptionValue( 'a', 128 );
	//printf("FI[%d]W[%d]FO[%d]\n",work->timer[0],work->timer[1],work->timer[2]);
	flag = GCL_GetOptionValue( 'f', 0 );

   new_w = (float)w;    // use floats to pass into the spr 
   new_h = (float)h;

   // BP - Hack to resize stage names
   // check and see if this is a stage name - the only textures this size is our stage names.
   if( BP_UI_IsPixelPerfect(pTexture->BP_TextureHandle) )
   {
      int bpWidth, bpHeight;
      BP_UI_GetTextureSize(pTexture->BP_TextureHandle, &bpWidth, &bpHeight);

      new_w = bpWidth * 512.0f / 1280.0f;
      new_h = bpHeight * 400.0f / 720.0f;
   }

	if(flag & FLAG_CENTER)
   {
		pos.x -= new_w * 0.5f * sw;
		pos.y -= new_h * 0.5f * sh;
	}
   else
   {
		if(flag & FLAG_RIGHT)
      {
			pos.x -= new_w;
		}
		if(flag & FLAG_BOTTOM)
      {
			pos.y -= new_h;
		}
	}



   
   SPR_SetPosSprite(work->sprite, &pos);
	SPR_SetSizeSprite(work->sprite, new_w*sw, new_h*sh );

	work->sprite->head.flags |= SPR_FLAG_ALPHA;
	switch( flag & 0xf000 ){
	  case FLAG_ALPHA:
		work->sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
		break;
	  case FLAG_ADD:
		work->sprite->head.alpha = SCE_GS_SET_ALPHA(0, 2, 0, 1, 0);
		break;
	  case FLAG_SUB:
		work->sprite->head.alpha = SCE_GS_SET_ALPHA(2, 0, 0, 1, 0);
		break;
	  default:
		work->sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
		break;
	}

	SET_COLOR_2DPRIM(work->sprite,0x00808080);

	SPR_HIDE(work->sprite);


	work->flag = 2;
//printf("2d\n");
#ifdef KP_XBOX
	GM_CreatePadDisconnect();
#endif
	
	return 0;
}
void *New2DSprite_Pause( int name, int where )
{
	Work	*work;

	//DG_FrameCount = 2;

	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
								  sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		work->map = where;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
		GV_SetActorMessageKill( work, name );
	}

	return work ;
}

