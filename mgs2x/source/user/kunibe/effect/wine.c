//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    wine.c
    ワイン
    2000/06/23 Yuuta Kunibe	
    $Id: wine.c,v 1.1.1.3 2002/11/19 11:44:54 Yoshizawa1 Exp $
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

#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../../shibata/util/ts_util.h"



#define GLASS_CODE	( GV_StrCode("wineglass_fat") )
#define WINE_CODE	( GV_StrCode("wine_liq_vanim_50hlf_mt") )



typedef struct {

    GV_ACT_EX		actor;
    int			name;

    DG_OBJS		*glass;
    DG_OBJS		*wine;
	FMATRIX		*root;

    CV2_DEF 		*cdef;
    VERTEX_ANIME_WORK 	*anime;
    int			key;

    SVECTOR		wine_rot;
    short		add_rot;

    FMATRIX		light[2];
    int			count;

} Work ;



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
		case 0:
			if ( msg->message[2] != 0 ) {
				work->anime->p[work->key] = 0.0f;	/* 前のkeyのパラメータをクリア */
				work->key = msg->message[1];
				work->anime->count = DIRECT_TICK( msg->message[2] );
				work->anime->p[work->key] = 1.0f;
			}
			break;
		}
		msg++;
    }

	return;

}



static void WineVertexAnimation( Work *work )
{

    static float BaseRot[3] = { (float)( 4096 * 8 / 360 ), (float)( 4096 * 10 / 360 ), (float)( 4096 * 15 / 360 ) };


    SimpleVertexAnimation( work->anime );    


	RecieveMessage( work );


#if 0 	

    if ( GV_PadData[1].press & PAD_L1 ) {
		printf("wine animation set : %d -> 0\n", work->key);
		work->anime->p[work->key] = 0.0f;
		work->key = 0;
		work->anime->count = 60;
		work->anime->p[work->key] = 1.0f;
    }
    else if ( GV_PadData[1].press & PAD_L2 ) {
		printf("wine animation set : %d -> 1\n", work->key);
		work->anime->p[work->key] = 0.0f;
		work->key = 1;
		work->anime->count = 60;
		work->anime->p[work->key] = 1.0f;
    }
    else if ( GV_PadData[1].press & PAD_R2 ) {
		printf("wine animation set : %d -> 2\n", work->key);
		work->anime->p[work->key] = 0.0f;
		work->key = 2;
		work->anime->count = 60;
		work->anime->p[work->key] = 1.0f;
    }

#endif	


    /*if ( --work->count == 0 ) {

		work->anime->p[work->key] = 0.0f;
		if ( work->key == 0 ) {
			work->key = 1;
		}
		else {
			work->key = 0;
		}
		work->anime->count = 60;
		work->anime->p[work->key] = 1.0f;

		work->count = 10 + irnd()%30;

    }*/

    /* 回転速度補間処理 */
    work->add_rot = (short)( (float)work->add_rot * 0.95f + BaseRot[work->key] * 0.05f  );

}



static void Act(Work *work)
{


    FVECTOR		offset;
    static FVECTOR	wine_offset = { -1.5F, -135.0F, 100.0F, 1.0f };

    
    GM_GroupObjs( work->glass, GM_CurrentStageMap );
    GM_GroupObjs( work->wine,  GM_CurrentStageMap );

    work->wine->flag |= DG_FLAG_SEMITRANS;


    WineVertexAnimation( work );
    
    /* ワインマトリクス更新 */
    work->wine_rot.vz += work->add_rot;
    //DG_SetPos( &GM_PlayerBody->objs->objs[6].world );
    DG_SetPos( work->root );
    DG_RotVector( &wine_offset, &offset, 1 );
    DG_RotatePos( &work->wine_rot );
    DG_GetPos( &work->wine->world );
#if 1 //def KP_XBOX /*yano 2002.03.26*/
	offset.vw = 0.0f;
#endif
    _sceVu0AddVector( (FVECTOR *)work->wine->world.m[3], (FVECTOR *)work->wine->world.m[3], &offset );
    

}


static void Die(Work *work)
{

    if ( work->glass ) {
		DG_DequeueObjs( work->glass );
		DG_FreeObjs( work->glass );
    }
    if ( work->wine ) {
		if ( work->anime ) ExitVertexAnimation( work->anime ) ;
		DG_DequeueObjs( work->wine );
		DG_FreeObjs( work->wine );
    }

}


static DG_OBJS *InitObjs( Work* work, int objcode )
{


    DG_DEF		*def;
    DG_OBJS		*objs;

    if ( !( def = (DG_DEF *)GV_GetCache( GV_CacheID( objcode, 'k' ) ) ) ) {
		return NULL;
    }

    if ( !( objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 ) ) ) { /* 後でDG_FLAG_ONERIECE入れる */
		return NULL;
    }

    if ( DG_QueueObjs( objs ) < 0 ) {
		return NULL;
    }

    return objs;

}


static DG_OBJS *InitWine( Work* work, int objcode )
{

    int     i;
    CV2_DEF *cdef;
    DG_DEF  *def;
    DG_OBJS *objs;

    if ( !( cdef = work->cdef = GV_GetCache( GV_CacheID( objcode, 'c' ) ) ) ) {
		printf("wine.c : wine cv2 data not found!\n");
		return NULL;
    }
    if ( !( def  = GV_GetCache( GV_CacheID( objcode, 'k' ) ) ) ) {
		printf("wine.c : wine model not found!\n");
		return NULL;
    }
    def->n_models = def->n_x_models = 1;

    if ( !( objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 ) ) ) { /* 後でDG_FLAG_ONEPIECE入れる */
		return NULL;
    }

    if ( DG_QueueObjs( objs ) < 0 ) {
		return NULL;
    }


    if ( !(work->anime = InitVertexAnimation( objs->objs,
					      cdef->models,
					      DG_VANIME_VERTS|DG_VANIME_NORMS,
					      cdef->n_models ) ) ) {
		printf("wine.c : InitVertexAnimation return NULL!\n");
		return NULL;
    }

    for ( i = 0 ; i < cdef->n_models ; i++ ){

	if ( cdef->models[i].n_verts == cdef->models[0].n_verts ){
	    work->anime->key[i] = &cdef->models[i];
	    work->anime->p[i] = 0.0f;
	}else{
	    printf( "Animation Vertexies are not same!! :: NewPutVanimeObject\n" ) ;
	    return NULL;
	}

    }

    work->key = 0;
    work->anime->p[work->key] = 1.0f;

    return objs;

}



static int GetResources( Work *work, FMATRIX *mat )
{

	work->root = mat;
	work->count = 5+irnd()%10;

	/* ワイングラス初期化 */
	if ( !( work->glass = InitObjs( work, GLASS_CODE ) ) ) {
	    return -1;
	}
	GM_GroupObjs( work->glass, GM_CurrentStageMap );
	work->glass->root = work->root;

	/* ワイン初期化 */
	if ( !( work->wine = InitWine( work, WINE_CODE ) ) ) {
	    return -1;
	}	
	GM_GroupObjs( work->wine, GM_CurrentStageMap );


	work->wine->flag |= DG_FLAG_SEMITRANS;
   work->wine->flag |= DG_FLAG_FORCEMSAA;

	/* ワイン回転パラメータ取得 */
	work->wine_rot.vx = 0;
	work->wine_rot.vy = 0;
	work->wine_rot.vz = 0;
	work->add_rot = 4096 * 10 / 360;

	return 0;

}


void *NewWine( FMATRIX *mat, int name )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect( GV_ACTOR_USER, sizeof(Work) );
	if ( work ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name;
		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor(work);
			return NULL ;
		}
	}

	return (void *)work ;
}
