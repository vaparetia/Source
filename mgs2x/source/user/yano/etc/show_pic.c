//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	show_pic.c
	1枚絵表示

	2002/07/25 Y.Yano
	$Id: show_pic.c,v 1.1.1.3 2002/11/19 11:52:00 Yoshizawa1 Exp $
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
#endif

#include <libutl.h>
#include	"gameheader.h"
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"

typedef struct{
	GV_ACT_EX actor;
	int timer;

	int fadein_time;
	int fadeout_time;

	int proc;
	int ans;

	int key_status;
	int key_press;

	SPR_OBJ *handle;
	int tri_handle;
	int tri_strcode;
	int bmp_strcode;

}Work;


#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) \
	SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))


#define DISP_CHANL			4


#ifdef PSX2
#define PAD_OKU (PAD_R | PAD_A)
#define PAD_MOD (PAD_L | PAD_B)
#else /* KP_XBOX */
#define PAD_OKU (PAD_R | PAD_B)
#define PAD_MOD (PAD_L | PAD_A)
#endif

#define TO_NOTHING 	0
#define TO_OKURI 	1
#define TO_MODORI 	2


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=work->ans;

		GCL_ExecProc(work->proc,&arg);
	}
}

static void Key(Work *work)
{
	work->key_status=GV_PadDataDirect[0].status;
	work->key_press=GV_PadDataDirect[0].press;
}

static int ShowAct(Work *work)
{
	if( work->key_press & PAD_OKU ){
		work->ans = TO_OKURI;
		if( work->proc != 0 && work->proc != 1 ){
			/* サウンドフェード開始 */
			GM_SdSet( SNG_FOUTS_S );
		}
	} else if( work->key_press & PAD_MOD ){
		work->ans = TO_MODORI;
		if( work->proc != 0 && work->proc != 1 ){
			/* サウンドフェード開始 */
			GM_SdSet( SNG_FOUTS_S );
		}
	}

	if(work->timer==0){
		/* 開始 */
		SPR_SHOW(work->handle);
	} else if( work->timer > work->fadeout_time ){
		/* 終了 */
		SPR_HIDE(work->handle);
		return 1;
	}

	if( work->timer < work->fadein_time ){
		int alpha=work->timer*128/work->fadein_time;
		SET_COLOR_2DPRIM2(work->handle,128,128,128,alpha);
		
	} else if( work->ans == TO_NOTHING ){
		SET_COLOR_2DPRIM2(work->handle,128,128,128,128);
		work->timer = work->fadein_time;

	} else if( work->timer < work->fadeout_time ){
		int alpha=(work->fadeout_time-work->timer)*128/(work->fadeout_time-work->fadein_time);
		SET_COLOR_2DPRIM2(work->handle,128,128,128,alpha);

	} else{
		SET_COLOR_2DPRIM2(work->handle,128,128,128,0);
	}

	return 0;
}

static void Act(Work *work)
{
	Key( work );

	if( ShowAct(work) ){
		if(work->handle) SPR_Destroy_2D_Object(work->handle);
		if(work->tri_handle>=0) SPR_KillTexture(work->tri_handle);
		CallLocalProc(work);
		if( work->proc == 0 || work->proc == 1 ){ 
			/* シナリオ呼び出しでないときのみvr_book.cにsignalを送る */
			GV_CallParentSignalFunc( work, work->ans, 0 );
		}
		GV_DestroyActor(work);
	}

	work->timer++;
}

static void Die(Work *work)
{
}


/* ------------------------------------------------------------------------ */



/* 資源を獲得 */
static int GetResources(Work *work)
{
	work->fadein_time = DIRECT_TICK(90);
	work->fadeout_time = work->fadein_time + DIRECT_TICK(90);

	work->tri_handle=SPR_LoadTexture(work->tri_strcode);
	if(work->tri_handle<0) return 0;

	work->handle = SPR_Create_2D_Object(SP_SPRITE, DISP_CHANL, NULL);
	if(work->handle == NULL){
		printf("ERR!! Make2DObj[work->handle]\n");
		return 0;
	}

	SPR_ObjSetTexture(work->handle, work->bmp_strcode, work->tri_handle);
#ifdef BP_PSX2_GCC	
	SPR_SetPosSprite(work->handle, &(SPR_POS){ 0.0f, 0.0f});
#else
	SPR_SetPosSprite(work->handle, (SPR_POS*)&DG_ZeroVector );
#endif	
	SPR_SetSizeSprite(work->handle, 512.0f, 384.0f );
	work->handle->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->handle->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(work->handle,0x00000000);
	

    return 1;
}

/* 初期化部メイン */
void *NewShowPictureForVRBook( int mode, int strcode )
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->bmp_strcode = strcode;
		work->tri_strcode = GV_StrCode("jpnw_news");

		work->handle=NULL;
		work->timer=0;
		work->proc=0;
		work->ans= TO_NOTHING;

		work->fadein_time=64;
		work->fadeout_time=192;

		work->tri_handle=-1;
		

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}

/* シナリオ起動 */
static void GetOptionValue( Work *work )
{
	/* テクスチャ名のstrcode */
	work->bmp_strcode = GCL_GetOptionValue( 'b' , 0 );

	/* フェードイン時間 */
	GCL_GetOption( 't' );
	work->fadein_time = GCL_GetNextInt();
	work->fadeout_time = GCL_GetNextInt();

	/* 終了プロック */
	work->proc = GCL_GetOptionValue( 'p' , 0 );

}
void *NewShowPictureForVRBookScn( int name )
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

	GetOptionValue( work );

    if(work!=NULL) {
		work->tri_strcode = GV_StrCode("jpnw_news");

		work->handle=NULL;
		work->timer=0;
		work->ans= TO_NOTHING;

		work->tri_handle=-1;
		

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;

}
