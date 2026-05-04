//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  solidus_blade_light.c
  民主刀、共和刀の刃の光
  2000/07/18 Yuuta Kunibe	
  $Id: solidus_blade_light.c,v 1.1.1.3 2002/11/19 11:44:49 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../../okajima/etc/ok_util.h"
#include	"../../shibata/util/ts_util.h"




/*----------------------------------------------------------*/
/*			extern                              */
/*----------------------------------------------------------*/



#define	COL_R		(60)
#define	COL_G		(162)
#define	COL_B		(255)

#define	ALPHA		(128)


#define	LIGHT_TEX	( GV_StrCode("drop01_msk") )

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)



static FVECTOR BladeLightPos[] = {
    { 18.0F, -227.5F, 443.75F },	// 共和刀
    { -18.0F, -210.0F, 347.0F },	// 民主刀
};



typedef	struct	{

    GV_ACT_EX	actor;
    int		name;
	int			code[3];

    FMATRIX	*root;
    
    DG_COMDL	*comdl;
    int		node;

	float	add_alpha;
	int		fade_count;
	
    int		kind;
    int 	*flag;

} Work;


static void RecieveMessage( Work *work )
{

    GV_MSG*	msg;
    int 	n_msg;


    if ( work->name == 0 ) {
		return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

		switch ( msg->message[ 0 ] ) {
		case 0:			/* fade_in, fade_out */
			work->fade_count = msg->message[2];
			if ( work->fade_count != 0 ) {
				work->add_alpha = ( (float)msg->message[1] - work->comdl->pos->color.vw ) / (float)work->fade_count;
			}
			break;
		}
		msg++;
    }

	return;
}




static void Act( Work *work )
{

    DG_COMDL_POS	*comdl_pos;


    /* グループ更新 */
    work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );

    comdl_pos = work->comdl->pos;	

    DG_COPY_MAT( &comdl_pos->world, work->root );




    /* フェードインアウト */
    if ( work->name == 0 ) {	// ゲーム中
		if ( *work->flag ) {
			if ( comdl_pos->color.vw < 128.0f ) {
				comdl_pos->color.vw += 8.0f;
			}
		}
		else {
			if ( comdl_pos->color.vw > 0.0f ) {
				comdl_pos->color.vw -= 4.0f;
			}
		}
    }
    else {						// デモ中

		RecieveMessage( work );

		if ( work->fade_count > 0 ) {
			comdl_pos->color.vw += work->add_alpha;
			if ( comdl_pos->color.vw < 0.0f ) {
				comdl_pos->color.vw = 0.0f;
			}
			work->fade_count--;
		}

    }


    /* テンプライト設置 */
    if ( comdl_pos->color.vw > 0.0f ) {

		FVECTOR  pos;

		DG_SetPos( &comdl_pos->world );
		DG_PutVector( &BladeLightPos[work->kind], &pos, 1 );
		DG_SetTmpLight2 (
						 &pos,
						 comdl_pos->color.vw * 5.0f,
						 comdl_pos->color.vw * 10.0f,
						 (COL_R/2) | (COL_G/2)<<8 | (COL_B/2)<<16,
						 LIT_FLAG_CHARAONLY );

    }

}


static void Die( Work *work )
{
    if ( work->comdl ) {
		DG_DequeueComdlObjs( work->comdl );
		DG_FreeComdl( work->comdl );
    }
}


static int InitCmdl( Work *work )
{

	DG_DEF			*def;
	DG_COMDL		*comdl;
	DG_COMDL_POS 		*comdl_pos;

	
	def = NULL;

	if ( work->kind ) {
	    def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("mst_hl_cm"), 'k' ) );
	    work->node = HUMAN21_HIDARI_TE;
	}
	else {
	    def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("kwt_hl_cm"), 'k' ) );
	    work->node = HUMAN21_MIGI_TE;
	}

	if ( !def ) {
	    return -1;
	}
	

	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, 1, 0 );
	if( !comdl ) {
	    return -1;
	}
	DG_QueueComdlObjs( comdl );


	work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );
	    
	
	comdl_pos = comdl->pos;

	/* コモデルMATRIX初期化 */
	DG_COPY_MAT( &comdl_pos->world, &work->root );

	/* コモデル色初期化 -> とりあえず蛇手の色に合わせてみた */
	comdl_pos->color.vx = (float)COL_R;
	comdl_pos->color.vy = (float)COL_G;
	comdl_pos->color.vz = (float)COL_B;
	comdl_pos->color.vw = 0.0f;

	return 0;

}

static int GetResources( Work *work, FMATRIX *root, int kind )
{

    work->root = root;
    work->kind = kind;

    work->fade_count = 0;
    work->add_alpha = 0.0f;
	
    return InitCmdl( work );


}




void *NewSolidusBladeLight( FMATRIX *root, int kind, int *flag )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->flag = flag;
		if ( GetResources( work, root, kind ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work;

}


void *NewDemoSolidusBladeLight( int name, FMATRIX *root, int kind )
{

	Work		*work ;


	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->flag = NULL;
		if ( GetResources( work, root, kind ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work;

}



