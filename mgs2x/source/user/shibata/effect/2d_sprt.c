//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    2d_sprt.c
    
	2000/10/05 T.Shibata

	$Id: 2d_sprt.c,v 1.1.1.3 2002/11/19 11:48:35 Yoshizawa1 Exp $
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

//#include	"camera.h"
#include 	"sprite_2d.h"
#include	"../util/ts_util.h"

#include "BP_UIAdjust.h"
#include "BP_Camera.h"//BP_CINEMA
#include "../../mode/demo/libdemo.h"//BP_CINEMA


//#define	ACTOR_PRIO		(254)


//#define TRI_CODE		(GV_StrCode("telop"))
//#define TRI_CODE		(GV_StrCode("staff"))

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))
#define SET_COLOR_2DPRIM3(_prim,_a,_col) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | (_col)))

typedef	struct sprt2d_Work
{
	GV_ACT_EX		actor;
	SPR_OBJ   		*sprite;
	int				lock;
	short			alpha;
	short			step;
	short			time;
	short			timer[3];
} Work;

#define CHANL_NUM	(0)
enum {
	STEP_FADEIN = 0,
	STEP_WAIT,
	STEP_FADEOUT,
};


extern int DM_FrameSkip;

int	g_PauseLock;

static void Act( Work *work )
{
	int		alpha = work->alpha;

	if( GM_IsGameOver() ){//GM_CheckGameStatus( STATE_DISP_GAMEOVER ) ){
		GV_DestroyActor( work );
	}

if( !(GV_PauseLevel & GV_LEVEL_NORMAL) ){
	work->timer[work->step] -= DM_FrameSkip+1;
	
	switch(work->step){
	case STEP_FADEIN:
#if 0 //BP_PS2 def PSX2
			alpha = (int)((float)alpha*(float)(work->time - work->timer[STEP_FADEIN])/(float)work->time);
#else
		/*0DIV yano 2002.03.14*/
		{
			float dmdm = (float)work->time;
			if( dmdm == 0.0f ) dmdm = 0.0000001f;
			alpha = (int)((float)alpha*(float)(work->time - work->timer[STEP_FADEIN])/dmdm);
		}
#endif
		if(alpha > 128) alpha = 128;		
		if(work->timer[STEP_FADEIN] < 0){
			work->time = work->timer[STEP_FADEOUT];
			work->step = STEP_WAIT;
		}
	

		break;
	case STEP_WAIT:
		if(work->timer[STEP_WAIT] < 0){
			work->step = STEP_FADEOUT;
		}
		break;
	case STEP_FADEOUT:
#if 0 //BP_PS2 def PSX2
		alpha = (int)((float)alpha*(float)work->timer[STEP_FADEOUT]/(float)work->time);
#else
		/*0DIV yano 2002.03.14*/
		{
			float dmdm = (float)work->time;
			if( dmdm == 0.0f ) dmdm = 0.0000001f;
			alpha = (int)((float)alpha*(float)work->timer[STEP_FADEOUT]/dmdm);
		}
#endif
		if(alpha < 0) alpha = 0;
		if(work->timer[STEP_FADEOUT] < 0){
			GV_DestroyActor( work ) ;
		}
		break;
	default:
		printf("STEP ERR!!!\n");
	}

	SET_COLOR_2DPRIM3(work->sprite,alpha,0x00c0c0c0);
}
}

static void Die( Work *work )
{
	if(work->sprite) SPR_Destroy_2D_Object(work->sprite);
	if( work->lock ) g_PauseLock = 0;
}
/*
-tri $s:ＴＲＩコード
-x $s:ＴＥＸコード
-pos $i:座標ＸＹ二つ(0,0)<->(512,384)
-size $i:サイズＷＨ二つ(0,0)<->(512,384)
-count $v:カウント
*/
//カウントｘ　フェードインタイム
//カウントｙ　ウェイト
//カウントｚ　フェードインアウト

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
   float       new_w,new_h;
	float		   sw,sh;
	SPR_POS		pos;
	int			flag;
   int         resizeEverything;
   DG_TEX*     pTexture;
	work->step = STEP_FADEIN;
	

   resizeEverything = 1;

	tri_code = GCL_GetOptionValue( 't', 0 );
	tex_code = GCL_GetOptionValue( 'x', 0 );

	//SPR_LoadTexture(tri_code);
	work->sprite = SPR_Create_2D_Object(SP_SPRITE_F, 4, NULL);        //BP JG - use the SP_SPRITE_F for perfect size/positing.
	if(work->sprite == NULL){ printf("ERR!! Make2DObj[work->sprite]\n"); return -1; }
	SPR_ObjSetTexture(work->sprite, tex_code, SPR_LoadTexture(tri_code));

	if(GCL_GetOption('p')){
		pos.x = (float)GCL_GetNextInt();
		pos.y = (float)GCL_GetNextInt();
	}else{
		pos.x = 0.0f;
		pos.y = 0.0f;
	}

   pTexture = ((SPR_SPRITE*)work->sprite)->head.tex.dgtex;

	if(GCL_GetOption('s')){
		w = GCL_GetNextInt();
		h = GCL_GetNextInt();
	}else{
		DG_GetTexelInfo( &w, &h, NULL, NULL, pTexture );
	}

	if(GCL_GetOption('b')){
		sw = (float)GCL_GetNextInt()*0.0001f;
		sh = (float)GCL_GetNextInt()*0.0001f;
	}else{
		sw = 1.0f;
		sh = 1.0f;
	}

   //AS - Hack to fix incorrect aspect ratio for act_telop3_alp_ovl.bmp.tga
   //(Peek tutorial text), as the override texture is much wider than the original one
   if ( tex_code == 0x00226ACE )
   {
      w = 409;
   }

   //AS - Hack to fix incorrect aspect ratio for act_telop03_jp_alp_ovl.bmp.tga
   if ( tex_code == 0x0013283d )
   {
      h = 63;
      w = 403;
   }

   //BP - Hack to fix incorrect size rendering of replacement MGS2:SoL HD logo
   // Appears at end of d00t opening cinema
   // title_mgs2_alp_ovl.bmp.tga
   if ( (tex_code == 0x00A57131) && 
         (w==422) && (h==73) )
   {
      pos.x = 83;
      pos.y = 40;

      w = 346;
      h = 125;
   }

   //BP - Hack to remove PS2 'Sons of Liberty' texture
   // Appears at end of d00t opening cinema
   // title_sol_alp_ovl.bmp.tga
   if ( (tex_code == 0x005F550B) && 
        (w==422) && (h==24) )
   {
      // move it offscreen.
      pos.x = 1000;
   }

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

      resizeEverything = 0;
   }

	if(GCL_GetOption('c')){
		work->timer[0] = DIRECT_TICK(GCL_GetNextInt());
		work->timer[1] = DIRECT_TICK(GCL_GetNextInt());
		work->timer[2] = DIRECT_TICK(GCL_GetNextInt());
	}else{
		work->timer[0] = DIRECT_TICK(64);
		work->timer[1] = DIRECT_TICK(128);
		work->timer[2] = DIRECT_TICK(64);
	}
	work->alpha = GCL_GetOptionValue( 'a', 128 );

	//printf("FI[%d]W[%d]FO[%d]\n",work->timer[0],work->timer[1],work->timer[2]);
	flag = GCL_GetOptionValue( 'f', 0 );
	
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
	
	work->lock = 0;
	if(GCL_GetOption('l')){
		g_PauseLock = work->lock = 1;
	}

   // BP Hack - just resize everything to bring it back to aspect correct.
   if ( resizeEverything )
   {
      // Arm unhack:
      // Armature fix:
      // for the hint sprites in paddemos/scndemos, don't override the position
      // fixes MGSTWO-3222
      static char const *skAreasWithHintedPadDemos[] = { "w00a", "w11a", "w14a", "w43a" };
      int reposition = TRUE;
      // playing a pad/scn demo, and printing a hint to the top left corner
      if (GM_CheckGameStatus(STATE_PAD_DEMO|STATE_SCN_DEMO) && pos.x == 16.0f && pos.y == 38.0f)
      {
         int loopIndex;
         for (loopIndex = 0; loopIndex < sizeof(skAreasWithHintedPadDemos)/sizeof(skAreasWithHintedPadDemos[0]); ++loopIndex)
         {
            if (!strcmp(GM_GetArea(), skAreasWithHintedPadDemos[loopIndex]))
            {
               reposition = FALSE;
               break;
            }
         }
      }
      if (reposition) // end fix
      {
         pos.x += ((1.0f-TARGET_ASPECT_X)*new_w)/2;
      }
      new_w *= TARGET_ASPECT_X;   //JG scale it back (aspect) before it's stretch at render.
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

	work->time = work->timer[0];
#ifdef KP_XBOX
	if( work->time == 0 ) work->time = 1;	// 2002/02/21 K.Uehara 0.0check
#endif

	SPR_SHOW(work->sprite);
//printf("2d\n");
	return 0;
}
void *New2DSprite( int name, int where )
{
	Work	*work;

	//DG_FrameCount = 2;

	work = (Work*)GV_CreateActor( GV_ACTOR_EFFECT, GV_CLASS_OBJECT,
								  sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
		GV_SetActorMessageKill( work, name );
	}

	return work ;
}

static int GetResources_prog( Work *work, int tri_code, int tex_code,
							  FVECTOR *data, int wide, int high, SVECTOR *time, int flags )
{
	int			w,h;
	float		sw,sh;
	SPR_POS		pos;

	work->step = STEP_FADEIN;
	
	//SPR_LoadTexture(tri_code);
	if( flags & FLAG_PRIO )
		work->sprite = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	else
		work->sprite = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);

	if(work->sprite == NULL){ printf("ERR!! Make2DObj[work->sprite]\n"); return -1; }
	SPR_ObjSetTexture(work->sprite, tex_code, SPR_LoadTexture(tri_code));

	pos.x = (float)data->vx;
	pos.y = (float)data->vy;
	if(wide >= 0){
		w = wide;
		h = high;
	}else{
		DG_GetTexelInfo( &w, &h, NULL, NULL, ((SPR_SPRITE*)work->sprite)->head.tex.dgtex );
	}

	sw = data->vz;
	sh = data->vw;

   
   if(time->vx >= 0){
		work->timer[0] = DIRECT_TICK(time->vx);
		work->timer[1] = DIRECT_TICK(time->vy);
		work->timer[2] = DIRECT_TICK(time->vz);
	}else{
		work->timer[0] = DIRECT_TICK(64);
		work->timer[1] = DIRECT_TICK(128);
		work->timer[2] = DIRECT_TICK(64);
	}
    //printf("FI[%d]W[%d]FO[%d]\n",work->timer[0],work->timer[1],work->timer[2]);

	if(flags & FLAG_CENTER){
		pos.x -= (float)w * 0.5f * sw;
		pos.y -= (float)h * 0.5f * sh;
	}else{
		if(flags & FLAG_RIGHT){
			pos.x -= (float)w;
		}
		if(flags & FLAG_BOTTOM){
			pos.y -= (float)h;
		}
	}

   //BP_CINEMA - special case cinema sprite adjustment
   if( BP_Camera_Cinema_IsFullScreen() )
   {
      // Ocelot MGS1 flashback images?
      // MGSTWO-2170
      // Snake_Tanker / Stage / Tanker_Demo / t12a1D 
      if(      (gBP_Demo_Id == 0xEDBDB557)   //(t12a1D.sdt)
            && ((tex_code == 0x00F52239) || (tex_code == 0x00F62239) || (tex_code == 0x00F72239)) )
      {
         // Force full screen vertically
         pos.y = 0;
         h = 400;
      }

      // Ocelot MGS1 flashback images?
      // MGSTWO-1154
      // snake_tanker / stage / Tanker_Demo / t12a3D
      if(      (gBP_Demo_Id == 0xA075145C)   // (t12a3D.sdt)
            && ((tex_code == 0x00F55579) || (tex_code == 0x00F95579)) )
      {
         pos.y = 0;
         h = 400;
      }
   }
   //BP_CINEMA - special case cinema sprite adjustment
	
	SPR_SetPosSprite(work->sprite, &pos);
	SPR_SetSizeSprite(work->sprite, (float)w*sw, (float)h*sh );
	work->sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->sprite->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(work->sprite,0x00808080);

	work->time = work->timer[0];
	work->alpha = 128;
	SPR_SHOW(work->sprite);

//printf("2d\n");

	return 0;
}

void *New2DSprite_Prog(int tri_code, int tex_code,
					   FVECTOR *data, int wide, int high, SVECTOR *time, int flags )
{
	Work	*work;

	//DG_FrameCount = 2;
	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
								  sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources_prog( work, tri_code, tex_code,
								data, wide, high, time, flags  ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
	}

	return work ;
}
