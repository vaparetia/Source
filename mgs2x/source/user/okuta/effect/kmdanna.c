//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    kmdanna.c
    かもめの糞
    2001/07/03 Masafumi Okuta
    $Id: kmdanna.c,v 1.1.1.3 2002/11/19 11:47:51 Yoshizawa1 Exp $
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
#include	"camera.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))

// RGBA設定
#define	COL_R		(139)
#define	COL_G		(150)
#define	COL_B		(115)
#if 0
#define COL_R		(244)
#define COL_G		(255)
#define COL_B		(175)
#endif
#define ALPHA		(128)

// プリミティブ,頂点数
#define N_PRIMS		(1)
#define N_VERTS		(2)

// スクラッチパッドアドレス
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)


// テクスチャstrcode
#define TEX_DROP_01_ALP  	(7840500) 	// "kmo_droppings01_alp"
#define TEX_DROP_02_ALP  	(8889076) 	// "kmo_droppings02_alp"
#define TEX_DROP_03_ALP  	(9937652) 	// "kmo_droppings03_alp"
#define TEX_DROP_04_ALP  	(10986228) 	// "kmo_droppings04_alp"

#define TEX_CHI01_MSK  	(10984814) // "chi01_msk"
#define TEX_CHI02_MSK  	(12033390) // "chi02_msk"
#define TEX_CHI03_MSK  	(13081966) // "chi03_msk"
#define TEX_CHI04_MSK  	(14130542) // "chi04_msk"
#define TEX_CHI05_MSK  	(15179118) // "chi05_msk"
#define TEX_CHI01_ALP  	(10972307) // "chi01_alp"
#define TEX_CHI02_ALP  	(12020883) // "chi02_alp"
#define TEX_CHI03_ALP  	(13069459) // "chi03_alp"
#define TEX_CHI04_ALP  	(14118035) // "chi04_alp"
#define TEX_CHI05_ALP  	(15166611) // "chi05_alp"
#define TEX_BOMB06_ALP  (9868771)  // "bombgas6_alp"
#define TEX_ALP_MASK    (7733153)  // 
#define TEX_SPREAD01	(15638432) // 	
#define TEX_SPREAD02	(11775409) // 	

#define RAISE		(0)		// 優先
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define SIZE_BASE	(20.f)
#define SIZE_ADD	(20)

#define LIFE_MAX	(COUNT_VMODE(900))	// 寿命
#define SPR_TIME	(COUNT_VMODE(20))	// 拡散時間

#define FALL_SPEED	(STEP_VMODEF(-200.f)) 	// 落下速度

typedef struct _KAMOME_DANNA {
    // 基本システム
    GV_ACT_EX		actor;
    int			map;				// 所属するマップ
    HZX_GROUP_ID	hzx_id;				// HZXのグループID 
    
    // 拡張管理
    FVECTOR		vecCenter;			// 糞の中心
    FVECTOR		vecPre;				// 前のフレームでの糞の中心
    SVECTOR		vecRot;				// 回転ベクトル
    int			nLife;				// 寿命カウンタ
    int			nLifeMax;			// 寿命
    float		fSize;				// 大きさ
    float		fSizeAdd;			// 加算分大きさ
    int			nFloorHit;
    int			nCntr;
    float		fHeight;			// 落下高さ

    FVECTOR		vecBoundary;	

    TARGET		trg;
    POWER_TARGET	powTrg;
    
    // 描画用
    FVECTOR		vecVert[ N_PRIMS * N_VERTS ];	// 頂点データ
    DG_PRIM2*		prim;		
} Work;

// 画面にはいっているか判定
static  int  InScreen( FVECTOR *pos )
{
    DG_CHANL*		cp;
    FVECTOR   		ret;   
    GM_CameraSet	*cam;

    cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

    /* 位置決定 */
    cp = DG_Chanl(0) ;
    pos->vw = 1.f;
    _sceVu0ApplyMatrix( &ret, &cp->eye_pers, pos) ;

    /* カメラの範囲内に入っているかどうか */
    if( ret.vz > ret.vw )	return 0;

    if( ret.vw < 0 )	ret.vw = -ret.vw;

    if( (ret.vx > ret.vw) || (ret.vx < (-ret.vw)) )	return 0 ;
    if( (ret.vy > ret.vw) || (ret.vy < (-ret.vw)) )	return 0 ;
    
    return 1 ;
}

// コールバック関数
static void KMMDannaCallBack( TARGET* off,    // 攻撃側
                              TARGET* def,    // 防御側
                              void*   work)   // かもめ糞ワーク
{
    Work* workDnn;
//printf("HITT!!!!!!!!!!!! 0x%x 0x%x 0x%x\n", off, def, GM_PlayerTarget);
    if ( def == GM_PlayerTarget ){
	if ( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) && InScreen( &off->center ) ){ // 主観時
	    extern void* NewKamomeDannaCamera( FVECTOR*, int);
	    NewKamomeDannaCamera( &off->center, 300);

	    workDnn = (Work*)work;
	    workDnn->prim->flag |= DG_PRIM2_INVISIBLE;
	}
	GV_DestroyActor( work );
    }
}

// キャラとの当たりを判定する為の攻撃ターゲットを設定
static void CheckTarget( Work* work )
{
    { // 落下攻撃ターゲット更新
	FVECTOR		vecForce;	// 力積
	TARGET*		offence;	// 攻撃用ターゲット
	POWER_TARGET*	power;

	offence = &( work->trg );
	power	= &( work->powTrg );

	GM_SetTarget( offence, TARGET_ONLINE|TARGET_OFFENSE|TARGET_POWER|TARGET_SEEK|TARGET_CHILD|TARGET_THROUGH, 
		      0, BOTH_SIDE, &DG_ZeroVector, &DG_ZeroVector ); 
	GM_SetTargetWeaponType( offence, WP_NONE ); 	// 武器タイプ
	GM_SetPowerTarget( offence, power, POWER_ONCE, 255, 0, 0, &vecForce );
	{ // コールバック
	    extern void	GM_SetTargetCallBack( TARGET*, TARGET_CALLBACK, void* );
	    GM_SetTargetCallBack( offence, KMMDannaCallBack, work);
	}
	GM_MoveOnlineTarget( &work->trg, &work->vecPre, &work->vecCenter );
	GM_PutTarget( offence );				   // 攻撃ターゲット配置
    }
}

// プリミティブ更新
static void UpdatePacket( DG_PRIM2* 	prim, 		// プリミティブワーク
			  FVECTOR* 	pvecCenter, 	// 中心
			  FVECTOR* 	pvecVertTop,	// 頂点データ先頭
			  SVECTOR* 	psvecRot, 	// 回転データ
			  float 	size) 		// サイズ
{
    int			i, k;
    float		fSize;
    float		angle;
    float		ftemp;
    SVECTOR		svtemp;
    FVECTOR*		pvecPos;
    FVECTOR*		pvecVert;
    FVECTOR		fvtemp;

    pvecPos 	= SCR_POS;		// 
    pvecVert	= pvecVertTop;		// 

    for ( i = 0 ; i < N_PRIMS ; i++ ){
	fSize = size;
	for ( k = 0 ; k < N_VERTS ; k++ ){
	    DG_COPY_VEC( pvecPos, pvecCenter );

	    /* 速度ランダマイズ */
	    ftemp       = 1.f*rnd();
	    angle       = rnd()*TPI*0.03f;
	    fvtemp.vz   = ftemp * vu0_Cos( angle );
	    fvtemp.vx   = ftemp * vu0_Sin( angle );
	    svtemp.vz   = irnd()%4096;
	    DG_SetPos2( &DG_ZeroVector, &svtemp );
	    DG_RotVector( &fvtemp, pvecVert, 1 );
	    
	    pvecPos++;
	    pvecVert++;
	}
    }
    // 回転をかける
    DG_SetPos2( pvecCenter, psvecRot);
    DG_RotVector( SCR_POS, SCR_POS, N_PRIMS*N_VERTS );

    // スクラッチパッド->メインメモリへ
    Mao_CopyScrToMem( prim->pos[prim->buffer_clock], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
}

// 動作関数
static void Act( Work* work )
{
    // バッファ切替え
    DG_SwitchBuffPrim2( work->prim );

    _sceVu0CopyVector( &work->vecPre, &work->vecCenter);
    work->vecCenter.vy += FALL_SPEED;

    { 
	if ( work->nFloorHit ){ // 接地エフェクトを出す
	    extern void* NewKamomeDannaFloor( FVECTOR* pvecInitPos, FVECTOR* pvecBoundary );
	    if ( work->vecCenter.vy < work->fHeight){
		work->vecCenter.vy = work->fHeight;
		NewKamomeDannaFloor( &work->vecCenter, &work->vecBoundary);
		GV_DestroyActor( work );
	    }
	}else{
	    if ( work->vecCenter.vy < work->fHeight ){
		GV_DestroyActor( work );
	    }
	}
    }

    // 動的ハザードチェック
    if ( work->nCntr % 4 == 0 ){
	int 		floor_flag;
	HZX_FLR		flr[2];
	int		flr_atrs[2];
	float		flr_height[2];
	floor_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
					   &work->vecCenter , HZX_CHK_F_FLOOR, HZX_FLOOR_NO_PLAYER );
	if ( floor_flag & 1){
	    extern void OK_FloorToBoundary( HZX_FLR *flr, FVECTOR *fv );

	    HZX_GetLevelHazard( flr, flr_atrs );
	    HZX_GetLevelHeight( flr_height );

	    OK_FloorToBoundary( flr, &work->vecBoundary); // 床平面取得
	    
	    work->fHeight   = flr_height[0];
	    work->nFloorHit = 1;
	}
    }
    work->nLife--;
    if ( work->nLife == 0 ){
	GV_DestroyActor( work );
    }

    // 当たりチェック
    CheckTarget( work );

    // エフェクト更新
    UpdatePacket( work->prim, &work->vecCenter, work->vecVert, &work->vecRot, work->fSize);

    work->nCntr++;
}

// 破棄関数
static void Die( Work* work )
{
    work->prim  = MAO_FreePrim2( work->prim );
}
// パケット初期化
static int InitPacket( DG_PRIM2* 	prim, 		// プリミティブワーク
		       DG_TEX* 		tex, 		// テクスチャワーク
		       long64		tagAlpha,	// αタグ
		       int		nRGBA,		// 色
		       FVECTOR* 	pvecCenter, 	// 中心位置
		       FVECTOR* 	pvecVertTop,	// 頂点データ先頭
		       SVECTOR* 	psvecRot, 	// 回転データ
		       int		nRaise,		// Z優先
		       float 		size, 		// サイズ
		       float 		speed )		// 回転量
{
    u_char		r,g,b,a;
    int			i, k;
    float		angle;
    float		ftemp;
    SVECTOR		svtemp;
    FVECTOR*		pvecPos;
    FVECTOR*		pvecVert;
    FVECTOR		fvtemp;
    DG_PRIM2_UVRGBWH*	uvrgbwh;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    pvecPos 	= SCR_POS;		// 
    uvrgbwh	= SCR_UVS;		// 
    pvecVert	= pvecVertTop;		// 
    fvtemp.vy = 0.0f;
    svtemp.vx = 0;
    svtemp.vy = 0;
    for ( i = 0 ; i < N_PRIMS ; i++ ){
	{
	    extern float _RsinF( int d );
	    angle = _RsinF( ((psvecRot->vz + i * MAO_GetRandom(128, 512)) & 4095));
	}
	for ( k = 0 ; k < N_VERTS ; k++ ){
	    DG_COPY_VEC( pvecPos, pvecCenter );

	    /* 速度ランダマイズ */
	    ftemp       = speed*rnd();
	    angle       = rnd()*TPI*0.03f;
	    fvtemp.vz   = ftemp * vu0_Cos( angle );
	    fvtemp.vx   = ftemp * vu0_Sin( angle );
	    svtemp.vz   = irnd()%4096;
	    DG_SetPos2( &DG_ZeroVector, &svtemp );
	    DG_RotVector( &fvtemp, pvecVert, 1 );
	    
	    angle       = rnd()*TPI;
	    uvrgbwh->w  = (int)(size * vu0_Cos( angle ));
	    uvrgbwh->h  = (int)(size * vu0_Sin( angle ));

	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	    uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	    uvrgbwh->q0 = 4096 ;
	    uvrgbwh->q1 = 4096 ;
	    uvrgbwh->f0 = 0x0fff ;
	    uvrgbwh->f1 = 0x0fff ;

	    uvrgbwh->r  = COL_R ;
	    uvrgbwh->g  = COL_G ;
	    uvrgbwh->b  = COL_B ;
	    uvrgbwh->a  = ALPHA ;


	    pvecPos ++;
	    pvecVert++;
	    uvrgbwh ++ ;
	}
    }
    // 回転をかける
    DG_SetPos2( pvecCenter, psvecRot);
    DG_RotVector( SCR_POS, SCR_POS, N_PRIMS*N_VERTS );

    // スクラッチパッド->メインメモリへ
    Mao_CopyScrToMem( prim->pos[0], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->pos[1], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->uvrgb[0], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->uvrgb[1], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

    return 1;
}
// リソース初期化
static int GetResources( Work*		work, 		// ワーク
			 FVECTOR* 	pvecInitPos )	// 発生位置
{
    DG_TEX*	tex;

    // 基本システムワーク初期化
    work->map = GM_CurrentStageMap;	// マップID取得
    work->hzx_id = GM_GetHzxGroupID( work->map );

    // 拡張管理ワーク初期化
    _sceVu0CopyVector( &work->vecCenter, pvecInitPos);	// 中心位置
    _sceVu0CopyVector( &work->vecPre, pvecInitPos);
    work->nLife = work->nLifeMax = LIFE_MAX;
    work->vecRot = DG_ZeroSVector;

    work->fSize    = SIZE_BASE;
    work->fSizeAdd = (float)MAO_GetRandom( -10, 40);

    work->nFloorHit = 0;
    work->fHeight   = -50000;

    {
	int 		floor_flag;
	HZX_FLR		flr[2];
	int		flr_atrs[2];
	float		flr_height[2];

	floor_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
					   &work->vecCenter , HZX_CHK_F_FLOOR, HZX_FLOOR_NO_PLAYER );
	if ( floor_flag & 1){
	    extern void OK_FloorToBoundary( HZX_FLR *flr, FVECTOR *fv );
	    HZX_GetLevelHazard( flr, flr_atrs );
	    HZX_GetLevelHeight( flr_height );
	    OK_FloorToBoundary( flr, &work->vecBoundary); // 床平面取得
	    work->fHeight   = flr_height[0];
	    work->nFloorHit = 1;
	}
    }
    
    { // 落下攻撃ターゲット設定
	TARGET*		offence;	// 攻撃用ターゲット
	POWER_TARGET*	power;

	offence = &( work->trg );
	power	= &( work->powTrg );

	GM_SetTarget( offence, TARGET_ONLINE|TARGET_OFFENSE|TARGET_POWER|TARGET_SEEK|TARGET_CHILD, 
		      0, BOTH_SIDE, &DG_ZeroVector, &DG_ZeroVector ); 
	GM_SetTargetWeaponType( offence, WP_NONE ); 	// 武器タイプをふみつけで設定
    }
    // 描画用ワーク初期化
    // プリミティブ本体の作成
    work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
    if ( work->prim == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャの設定
//  tex = DG_GetTexture( TEX_CHI01_MSK );
    tex = DG_GetTexture( TEX_DROP_02_ALP );
    // パケットの初期化
    InitPacket( work->prim, tex, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ), MAO_SetRGBA( COL_R, COL_G, COL_B, ALPHA),
		&work->vecCenter, work->vecVert, &work->vecRot, RAISE-100, work->fSize, 1.f);

    return 0;
}

// かもめの糞呼出
void* NewKamomeDanna( FVECTOR* pvecInitPos )	// 発生位置
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, pvecInitPos ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}

