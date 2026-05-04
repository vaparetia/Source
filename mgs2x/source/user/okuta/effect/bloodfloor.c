//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    bloodfloor.c
    血:床飛抹
    2001/07/04 Masafumi Okuta
    $Id: bloodfloor.c,v 1.1.1.3 2002/11/19 11:47:49 Yoshizawa1 Exp $
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
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"
#include	"../../okajima/effect/blood.h"


inline int BP_AdjustTick(int);
inline int BP_AdjustTick2(int);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))

// RGBA設定
//#ifdef JAPANESE_BP_IGNORE()
#if 1
#define ALPHA		(64)
#else
#define ALPHA		(128)
#endif

// プリミティブ,頂点数
#define N_PRIMS		(1)
#define N_VERTS		(16 + 2)
#define N_CIRCLE_DIV	(8)

// スクラッチパッドアドレス
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)


// テクスチャstrcode
#define TEX_CHI01_ALP  	(10972307) // "chi01_alp"
#define TEX_CHI02_ALP  	(12020883) // "chi02_alp"
#define TEX_CHI03_ALP  	(13069459) // "chi03_alp"
#define TEX_CHI04_ALP  	(14118035) // "chi04_alp"
#define TEX_CHI05_ALP  	(15166611) // "chi05_alp"

#define RAISE		(10000)		// 優先
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define SIZE_BASE	(220.f)
#define SIZE_ADD	(40.f)

#define LIFE_MAX	(COUNT_VMODE(600))	// 寿命
#define SPR_TIME	(COUNT_VMODE(20))	// 拡散時間

typedef struct _KAMOME_DANNA {
    // 基本システム
    GV_ACT_EX	actor;
    int		map;				// 所属するマップ
    
    // 拡張管理
    FVECTOR	vecCenter;			// 糞の中心
    FVECTOR	vecMove;			// 移動ベクトル
    FVECTOR	vecBoundary;			// 床平面
    SVECTOR	vecRot;				// 回転ベクトル
    int		nLife;				// 寿命カウンタ
    int		nLifeMax;			// 寿命
    float	fSize;				// 大きさ
    float	fSizeAdd;			// 加算分大きさ
    int		nAlpha;				// α
    int		nSprCntr;			// 拡散時間

    // 描画用
    DG_PRIM2*	prim;		
    DG_PRIM2*	prim2;		
} Work;
/* Ｚ軸方向が０方向、Ｚからの方向を求める form ok_util.c */
static void CalcDirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot )
{
    SVECTOR	temp_rot ;
    FVECTOR	vec ;
    float tmp ;

    _sceVu0SubVector( &vec, to, from ) ;

    tmp = atan2f( vec.vx, vec.vz ) ;
    rot->vy = 4095 & ( short )( ( tmp * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */
    temp_rot.vx = 0.0f;
    temp_rot.vy = -rot->vy;
    temp_rot.vz = 0.0f;

    DG_SetPos2( &DG_ZeroVector, &temp_rot );
    DG_PutVector( &vec, &vec, 1 );

    tmp = -atan2f( vec.vy, vec.vz ) ;
    rot->vx = 4095 & ( short )( ( tmp * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */

    rot->vz = 0 ;
}

// ポリゴンの方向をフロアの法線から取得
static void CalcFloorRotate( Work* work, HZX_FLR* floor, FVECTOR* pvecMove, SVECTOR* pvecRot)
{
    FVECTOR vecNorm;

    // 法線取得
    vecNorm.vx = floor->p1.h;
    vecNorm.vy = floor->p3.h;
    vecNorm.vz = floor->p2.h;

    CalcDirVecXY( &DG_ZeroVector, &vecNorm, pvecRot);

    {
	float fSpeed;
	FMATRIX mat;
    	DG_SetPos2( &work->vecCenter, pvecRot);
	DG_GetPos( &mat);

	if ( mat.m[2][0] != 0.f && mat.m[2][2] != 0.f){
	    fSpeed = DG_SQRT( mat.m[2][0] * mat.m[2][0] + mat.m[2][2] * mat.m[2][2]);
	}else{
	    fSpeed = 0.f;
	}

	_sceVu0ScaleVector( pvecMove, (FVECTOR*)&mat.m[1][0], -fSpeed * 10.f);
    }
}

static void UpdateAlpha( DG_PRIM2* 	prim, 		// 
			 int		nAlpha)		//
{
    int			i, k;
    DG_PRIM2_UVRGB*	uvrgb;

    uvrgb   	= prim->uvrgb[prim->buffer_clock]; 

    for ( i = 0 ; i < N_PRIMS ; i++ ){
	for ( k = 0 ; k < N_VERTS ; k++ ){
	    // αチェック
	    uvrgb->a = nAlpha;
	    uvrgb++;
	}
    }
}
// 動作関数
static void Act( Work* work )
{

    // バッファ切替え
    DG_SwitchBuffPrim2( work->prim );
    DG_SwitchBuffPrim2( work->prim2 );

    UpdateAlpha( work->prim, work->nAlpha);
    UpdateAlpha( work->prim2, work->nAlpha);

    work->nLife--;

    if ( work->nLife < work->nAlpha )  work->nAlpha  = work->nLife;

    if ( work->nLife == 0 ){
	GV_DestroyActor( work );
    }

}

// 破棄関数
static void Die( Work* work )
{
    work->prim  = MAO_FreePrim2( work->prim );
    work->prim2 = MAO_FreePrim2( work->prim2 );
}

// パケット初期化
static int InitPacket( DG_PRIM2* 	prim, 		// 
		       DG_TEX* 		tex, 		// 
		       long64		tagAlpha,	// 
		       int		nRGBA,		// 
		       FVECTOR* 	pvecCenter, 	// 
		       SVECTOR* 	psvecRot, 	// 
		       int		nRaise,		//
		       float 		size, 		// 
		       float 		speed )		// 
{
    u_char		r,g,b,a;
    int			i, k;
    float		fSize;
    FVECTOR*		pvecPos;
    DG_PRIM2_UVRGB*	uvrgb;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    pvecPos 	= SCR_POS;		// 
    uvrgb	= SCR_UVS;		// 

    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff) >> 1;

    for ( i = 0 ; i < N_PRIMS ; i++ ){
	int nRandAng;
	fSize = size;

	nRandAng = MAO_Rand(4096);
	for ( k = 0 ; k < N_VERTS ; k++ ){
	    extern float _TS_Sin( int s );
	    int 	nDir, nDirUV;
	    float 	fCos, fSin;
	    float 	fCosU, fSinV;
	    nDir   = (int)((k / 2) * 4096 / N_CIRCLE_DIV) + nRandAng;
	    nDirUV = (int)((k / 2) * 4096 / N_CIRCLE_DIV);
	    fCos = _TS_Sin( nDir  );
	    fSin = _TS_Sin( nDir + 1024);
	    fCosU = _TS_Sin( nDirUV  );
	    fSinV = _TS_Sin( nDirUV + 1024);

	    // 座標設定
	    if ( k % 2 == 0){
		float fAddRad;
		fAddRad = SIZE_ADD * rnd();
		pvecPos->vx = (fSize + fAddRad) * fCos;
		pvecPos->vy = (fSize + fAddRad) * fSin;
		pvecPos->vz = 0.f;
		pvecPos->vw = 1.f;
		uvrgb->u = FTOI12( ((fCosU * 0.5f) + 0.5f) * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( ((fSinV * 0.5f) + 0.5f) * tex->v_scale + tex->v_offset );
	    }else{
		pvecPos->vx = 0.f;
		pvecPos->vy = 0.f;
		pvecPos->vz = 0.f;
		pvecPos->vw = 1.f;
		uvrgb->u = FTOI12( 0.5f * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( 0.5f * tex->v_scale + tex->v_offset );
	    }

	    // UVQF設定
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    // RGBA設定
	    uvrgb->r  = r;
	    uvrgb->g  = g;
	    uvrgb->b  = b;
	    uvrgb->a  = a;
	    
	    pvecPos++;
	    uvrgb++;
	}
    }
    // 回転をかける
    DG_SetPos2( pvecCenter, psvecRot);
    DG_PutVector( SCR_POS, SCR_POS, N_PRIMS*N_VERTS );

    // スクラッチパッド->メインメモリへ
    Mao_CopyScrToMem( prim->pos[0], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->pos[1], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->uvrgb[0], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->uvrgb[1], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );

    return 1;
}
// リソース初期化
static int GetResources( Work*		work, 		// ワーク
			 FVECTOR* 	pvecInitPos,	// 発生位置
			 FVECTOR*	pvecBoundary)	// 床平面
{
    int		nTexNum2;
    DG_TEX*	tex;

    // 基本システムワーク初期化
    work->map = GM_CurrentStageMap;	// マップID取得

    _sceVu0CopyVector( &work->vecBoundary, pvecBoundary);	// 床平面	

    // 拡張管理ワーク初期化
    _sceVu0CopyVector( &work->vecCenter, pvecInitPos);	// 中心位置
    _sceVu0CopyVector( &work->vecMove, &DG_ZeroVector);	// 移動
    work->nLife = work->nLifeMax = LIFE_MAX;
    work->vecRot = DG_ZeroSVector;

    {
	int 		floor_flag;
	HZX_FLR		flr[2];
	int		flr_atrs[2];
	float		flr_height[2];

	work->vecCenter.vy += 120.f;
	floor_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
					    &work->vecCenter , HZX_CHK_F_FLOOR, HZX_FLOOR_NO_PLAYER );
	if ( floor_flag & 1){
	    HZX_GetLevelHazard( flr, flr_atrs );
	    CalcFloorRotate( work, flr, &work->vecMove, &work->vecRot );
	    HZX_GetLevelHeight( flr_height );

	    work->vecCenter.vy = flr_height[0]+2.f;
	}
    }

    work->fSize    = SIZE_BASE;
    work->fSizeAdd = (float)MAO_GetRandom( 10, 40);
    work->nSprCntr = 0;

    // 描画用ワーク初期化
    // プリミティブ本体の作成
    work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
    work->nAlpha = ALPHA;
    if ( work->prim == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャの設定
    tex = DG_GetTexture( TEX_CHI05_ALP );

    // パケットの初期化
    InitPacket( work->prim, tex, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ), MAO_SetRGBA( COLOR_R, COLOR_G, COLOR_B, ALPHA),
		&work->vecCenter, &work->vecRot, RAISE, work->fSize, 1.f);

    // プリミティブ本体の作成
    work->prim2 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
    if ( work->prim2 == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャの設定
    {
	int nTex[5] = {
	    TEX_CHI01_ALP,
	    TEX_CHI02_ALP,
	    TEX_CHI03_ALP,
	    TEX_CHI04_ALP,
	    TEX_CHI05_ALP,
	};
	nTexNum2 = BP_PS2_rand() % 5;
	tex = DG_GetTexture( nTex[ nTexNum2 ] );
    }
    // パケットの初期化
    {
	FVECTOR vec;
	vec.vx = work->vecCenter.vx + frnd() * 10.f;
	vec.vy = work->vecCenter.vy;
	vec.vz = work->vecCenter.vz + frnd() * 10.f;
	vec.vw = 1.f;
	InitPacket( work->prim2, tex, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ), 
		    MAO_SetRGBA( COLOR_R, COLOR_G, COLOR_B, ALPHA),
		    &vec, &work->vecRot, RAISE, work->fSize, 1.f);
    }

    return 0;
}

// 床の血
void* NewBloodFloor( FVECTOR* pvecInitPos, 	// 発生位置
		     FVECTOR* pvecBoundary )	// 床平面
{
    Work*	work;

    OPERATOR();

    // BLOOD-OFF mode check
    if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, pvecInitPos, pvecBoundary ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}
