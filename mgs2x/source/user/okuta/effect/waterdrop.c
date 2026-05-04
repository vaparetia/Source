//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    waterdrop.c
    パイプ水滴:ピチョン君
    2001/07/27 Masafumi Okuta
    $Id: waterdrop.c,v 1.1.1.3 2002/11/19 11:47:53 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"
#include	"waterdrop.h"


#define SE_COS_10	(0.9848077530122f)
#define SE_COS_15	(0.9659258262891f)
#define SE_COS_20	(0.9396926207859f)
#define SE_COS_30	(0.8660254037844f)
#define SE_COS_45	(0.7071067811865f)
#define SE_COS_60	(0.5f)
#define SE_COS_75	(0.2588190451025f)
#define SE_COS_80	(0.1736481776669f)
#define SE_COS_85	(0.08715574274766f)
#define SE_COS_90	(0.0f)

// ----------------------------------- SE
static VOLUMECURVE PICHON_BaseVolCurve = {
2,
SE_COS_75,
SE_COS_90,
{ 2000, 3000, -1, -1 },
{ 0x3F, 0x10, 0x00, 0xff, 0xff },
{ 1500, 2500, -1, -1 },
{ 0x3F, 0x10, 0x00, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PICHON_NormVolCurve = {
2,
SE_COS_75,
SE_COS_90,
{ 2000, 3000, -1, -1 },
{ 0x3F, 0x10, 0x00, 0xff, 0xff },
{ 1500, 2500, -1, -1 },
{ 0x3F, 0x10, 0x00, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PICHON_OwnValCurve = {
1,
SE_COS_60,
SE_COS_90,
{ 4800, -1, -1, -1 },
{ 0x3F, 0x00, 0xff, 0xff, 0xff },
{ 4300, -1, -1, -1 },
{ 0x3F, 0x00, 0xff, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PICHON_DemoVolCurve = {
2,
SE_COS_75,
SE_COS_90,
{ 5000, 6000, -1, -1 },
{ 0x3F, 0x04, 0x1f, 0xff, 0xff },
{ 5000, 6000, -1, -1 },
{ 0x3F, 0x04, 0x1f, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PICHON_MicVolCurve = {
2,
SE_COS_20,
SE_COS_30,
{ 6000, 9000, -1, -1 },
{ 0x3F, 0x3F, 0x00, 0xff, 0xff },
{ 0, 5000,-1, -1 },
{ 0x08, 0x00, 0x00, 0xff, 0xff },
1.0f
} ;

static VOLCURVES PICHON_ExtraVolCurves = {
&PICHON_NormVolCurve,
&PICHON_OwnValCurve,
&PICHON_DemoVolCurve,
&PICHON_MicVolCurve
};

// 動作関数
static void Act( WATERDROP* work )
{
    // 高さチェック
    work->vecPos.vy -= GRAVITY;
    if ( work->vecPos.vy <= work->fKillY ){
	work->vecPos.vy = work->fKillY;
	// 飛抹
	{
	    FMATRIX mat;
	    extern void* NewMAOParticle( int, FMATRIX*, int, int, int, int, int, float, 
					 float, float, float, float, int, int, int, float, int);


	    {
		SVECTOR svec;
		GV_VecToRot( &work->vecFloor, &svec);
		DG_SetPos2( &work->vecPos, &svec);
		DG_GetPos( &mat);
	    }

	    // パーティクル
	    if ( GM_GetDGGroupID(work->pWdmng->map) & GM_GetDGGroupID(GM_CurrentStageMap)) {
		// 効果音
		GM_SetVolCurveInDisFromPos( &GM_PlayerPosition, PICHON_BaseVolCurve.in_dis, 
					    PICHON_NormVolCurve.in_dis, 3);
		GM_SeSetFromVolCurve( SD_A_DRIPPY13, &work->vecPos, &PICHON_ExtraVolCurves );

		NewMAOParticle( MAO_GetRandom( 1323242, 4321432), &mat, 2, 16, COUNT_VMODE(15), COUNT_VMODE(25),
				512, STEP_VMODEF(16.f), STEP_VMODEF(24.f), 0.01f, 10.f, 20.f, 
				4642619, 0x30303040, 0x30303000, 0.8f, 2);
	    }
	}

	GV_DestroyActor( work );
    }

    // カウンタ更新
    work->nCntr++;
    if ( work->nCntr >= LIFE ){
	GV_DestroyActor( work );
    }

    // マネージャに登録
    {	
	WATERDROP_MNG*	pWdmng;
	pWdmng = work->pWdmng;
	pWdmng->vecDrop[ pWdmng->nDropNum ] = &work->vecPos;
	pWdmng->nDropNum++;
    }
}

// 破棄関数
static void Die( WATERDROP* work )
{
}


// リソース初期化
static int GetResources( WATERDROP*		work,
			 FVECTOR*	pvecPos)
{
    _sceVu0CopyVector( &work->vecPos, pvecPos);

    {
	int 		floor_flag;
	HZX_FLR		flr[2];
	int		flr_atrs[2];
	float		flr_height[2];
	floor_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( work->pWdmng->map ),
					   &work->vecPos, HZX_CHK_FLOOR | HZX_CHK_RECOIL_TYPE, 0 );
	if ( floor_flag & 1){
	    HZX_GetLevelHazard( flr, flr_atrs );
	    HZX_GetLevelHeight( flr_height );

	    // 法線設定
	    work->vecFloor.vx = flr->p1.h;
	    work->vecFloor.vy = flr->p3.h;
	    work->vecFloor.vz = flr->p2.h;
	    work->vecFloor.vw = 1.f;
	    _sceVu0Normalize( &work->vecFloor, &work->vecFloor);

	    work->fKillY   = flr_height[0];
	}else{

	    work->vecFloor.vx = 0.f;
	    work->vecFloor.vy = 1.f;
	    work->vecFloor.vz = 0.f;
	    work->vecFloor.vw = 1.f;

	    work->fKillY   = -200000;
	}
    }

    return 0;
}

// パイプ水滴を生成
void* NewPipeWaterDrop( WATERDROP_MNG* pWdmng, FVECTOR* pvecPos )	
{
    WATERDROP*	work;

    OPERATOR();

    // 表示数チェック
    if ( pWdmng->nDropNum >= MAX_DROPS) return NULL;

    work = (WATERDROP *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( WATERDROP ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	work->pWdmng = pWdmng;
	if ( GetResources( work, pvecPos ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}
