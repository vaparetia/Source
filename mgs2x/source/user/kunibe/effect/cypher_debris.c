//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  cypher_debris.c
  サイファー破片
  2000/06/20 Yuuta Kunibe	
  $Id: cypher_debris.c,v 1.1.1.3 2002/11/19 11:44:35 Yoshizawa1 Exp $
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

#include	"../../shibata/util/ts_util.h"



#define	CMDL_NUM	(1)
#define COLOR		(128)
#define ALPHA		(128)

#define	GRAVITY		(20.0f)

#define	SMOKE_COUNT	(3)
#define	LIFE		(180)

#define	MIN_SPEED	(20.0f)
#define	MIN_SPEED2	(MIN_SPEED*MIN_SPEED)

/*----------------------------------------------------------*/
/*			extern                              */
/*----------------------------------------------------------*/
extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );
extern void *NewRunningSmoke2( FVECTOR* pos, float size );


enum {
    CYPHER_DEBRIS	= 0x00,
    GUN_CAMERA_DEBRIS	= 0x01,
    GUN_CYPHER_DEBRIS	= 0x02,
};


typedef	struct	{

    GV_ACT_EX	actor;
    
    DG_COMDL	*comdl;
    int		n_comdl;

    FVECTOR	pos;
    FVECTOR	vec;
    SVECTOR	rot;

    int		parts;
    int		splash_flag;
    
    int 	count;
    int		se_flr_flag;
    int		se_seg_flag;

} Work;




static void Act( Work *work )
{

    DG_COMDL_POS	*comdl_pos;
    FVECTOR		to;
    SVECTOR		rot_tmp;
    int			hazard_flag;
    FVECTOR		vectmp;


    /* グループ更新 */
    work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );

    comdl_pos = work->comdl->pos;	

    
    _sceVu0AddVector( &to, &work->pos, &work->vec );

    hazard_flag = HZX_OnlineHazardCheck( 
			    GM_GetHzxGroupID( GM_CurrentStageMap ),
			    &work->pos,
			    &to,
			    HZX_CHK_FIX,
			    HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
			    HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE );

    if ( hazard_flag & 1 ) {			/* 壁衝突 */
	HZX_GetOnlinePoint( &to );
	work->vec.vx = -work->vec.vx * 0.25f;
	work->vec.vy = -0.90f;
	work->vec.vz = -work->vec.vz * 0.25f;
	_sceVu0AddVector( &work->pos, &to, &work->vec );

	if ( !work->se_seg_flag ) {
	    GM_SeSetMode( SD_W_MAGAZI01, &work->pos, GM_SEMODE_NORMAL );
	    work->se_seg_flag = 1;
	}
    }
    else if ( hazard_flag & 2 ) {		/* 床衝突 */

	HZX_GetOnlinePoint( &to );

	work->vec.vx *= 0.90f;
	work->vec.vy = -work->vec.vy * 0.25f;
	work->vec.vz *= 0.90f;

	
	if ( _sceVu0InnerProduct( &work->vec, &work->vec ) <= MIN_SPEED2  ) {	/* 停止 */
	    DG_COPY_VEC( &work->vec, &DG_ZeroVector );
	    work->rot.vx = 0;
	    work->rot.vy = 0;
	    work->rot.vz = 0;
	    TS_MatToRot( &rot_tmp, &comdl_pos->world );
	    rot_tmp.vx = 0;
	    rot_tmp.vz = 0;
	    DG_SetPos(&DG_UnitMatrix);
	    DG_RotatePos(&rot_tmp);
	    DG_GetPos(&comdl_pos->world);
	}
	else {
	    work->rot.vx = irnd()%256;
	    work->rot.vy = irnd()%256;
	    work->rot.vz = irnd()%256;
	    _sceVu0AddVector( &work->pos, &to, &work->vec );
	}

	if ( !work->se_flr_flag ) {
	    GM_SeSetMode( SD_W_MAGAZI01, &work->pos, GM_SEMODE_NORMAL );	    
	    work->se_flr_flag = 1;
	}

    }
    else {    
	_sceVu0AddVector( &work->pos, &work->pos, &work->vec );
	work->vec.vx *= 0.95f;
	work->vec.vy -= GRAVITY;
	work->vec.vz *= 0.95f;
    }


    /* 水柱呼び出し */
    if ( !( work->splash_flag ) ) {
	if ( work->pos.vy < GM_WaterLevel ) {
	    extern void *SetSplushSequence4( FVECTOR *pos, float step );	// 大きい破片用
	    extern void *SetSplushSequence5( FVECTOR *pos, float step );	// 残りの破片用
	    DG_COPY_VEC( &vectmp, &work->pos );
	    vectmp.vy = GM_WaterLevel;
	    if ( work->parts == 0 ) {
		SetSplushSequence4( &vectmp, work->vec.vy );
	    }
	    else {
		SetSplushSequence5( &vectmp, work->vec.vy );
	    }
	    work->splash_flag = 1;
	}
    }



    /* 回転更新 */
    DG_SetPos( &comdl_pos->world );
    DG_RotatePos( &work->rot );
    DG_GetPos( &comdl_pos->world );

    /* 位置更新 */
    DG_COPY_VEC( (FVECTOR *)comdl_pos->world.m[3], &work->pos );
    comdl_pos->world.m[3][3] = 1.0f;

    if ( work->count > 120 ) {
	comdl_pos->color.vw -= 2;
    }

    if ( work->count == SMOKE_COUNT ) {
	NewRunningSmoke2( &work->pos, 800.0f+frnd()*200.0f );	
    }	    
    
    if ( ++work->count >= LIFE ) {
	GV_DestroyActor( work );
    }    
    
}


static void Die( Work *work )
{
    if ( work->comdl ) {
	DG_DequeueComdlObjs( work->comdl );
	DG_FreeComdl( work->comdl );
    }
}


static int InitCmdl( Work *work, int kind )
{

	int			i;
	DG_DEF			*def;
	DG_COMDL		*comdl;
	DG_COMDL_POS 		*comdl_pos;

	
	work->n_comdl = CMDL_NUM;

	def = NULL;

	switch ( kind ) {
	case CYPHER_DEBRIS:
	    switch ( work->parts ) {
	    case 0:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("cyp_frg1_cm"), 'k' ) );
		break;
	    case 1:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("cyp_frg2_cm"), 'k' ) );
		break;
	    case 2:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("cyp_frg3_cm"), 'k' ) );
		break;
	    }
	    break;
	case GUN_CAMERA_DEBRIS:
	    switch ( work->parts ) {
	    case 0:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("gcm_frg1_cm"), 'k' ) );
		break;
	    case 1:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("gcm_frg2_cm"), 'k' ) );
		break;
	    case 2:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("gcm_frg3_cm"), 'k' ) );
		break;
	    }
	    break;
	case GUN_CYPHER_DEBRIS:
	    switch ( work->parts ) {
	    case 0:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("gcyp_frg1_cm"), 'k' ) );
		break;
	    case 1:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("gcyp_frg2_cm"), 'k' ) );
		break;
	    case 2:
		def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("gcyp_frg3_cm"), 'k' ) );
		break;
	    }
	    break;
	}

	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, CMDL_NUM, 0 );
	if( !comdl ) {
	    return -1;
	}
	DG_QueueComdlObjs( comdl );


	work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );
	    
	
	comdl_pos = comdl->pos;

	for ( i = 0; i < CMDL_NUM; i++ ) {
		extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );


//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )


		/* コモデルMATRIX初期化 */
		DG_COPY_MAT( &comdl_pos->world, &DG_UnitMatrix );
		DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], &work->pos );

		/* コモデル色初期化 */
		comdl_pos->color.vx = COLOR;
		comdl_pos->color.vy = COLOR;
		comdl_pos->color.vz = COLOR;
		comdl_pos->color.vw = ALPHA;
		//NewDbugSprite( (FVECTOR*)comdl_pos->world.m[3], 100.0f );
		comdl_pos++;
	}

	return 0;

}

static int GetResources( Work *work, FVECTOR *pos, int kind, int parts )
{

    FMATRIX	mat;
    
    work->count = 0;
    work->parts = parts;

    /* 初期位置 */
    DG_COPY_VEC( &work->pos, pos );

    /* 初期速度 */
    DG_SetPos( &DG_UnitMatrix );
    work->rot.vx = irnd()%4096;
    work->rot.vy = irnd()%4096;
    work->rot.vz = irnd()%4096;
    DG_RotatePos( &work->rot );
    DG_GetPos( &mat );
    _sceVu0ScaleVector( &work->vec, (FVECTOR *)mat.m[2], 400.0f );	

    /* 回転 */
    work->rot.vx = irnd()%256;
    work->rot.vy = irnd()%256;
    work->rot.vz = irnd()%256;

    work->splash_flag = 0;
    work->se_flr_flag = 0;
    work->se_seg_flag = 0;
    
    return InitCmdl( work, kind );
		

}

/*-------- void *NewCypherDebris( FVECTOR *pos, int kind ) ----------
     FVECTOR *pos : サイファー座標
     int     kind : サイファー種類
          0 : サイファー
	  1 : ガンカメラ
	  2 : ガンサイファー
-------------------------------------------------------------------*/	  
void *NewCypherDebris( FVECTOR *pos, int kind, int parts )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );
	//work = (Work *)GV_NewEffectPrio( GV_ACTOR_EFFECT, sizeof( Work ), 10 );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, kind, parts ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;
}


