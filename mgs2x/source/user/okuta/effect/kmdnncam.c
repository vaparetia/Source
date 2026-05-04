//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    kmdnncam.c
    カメラつくカモメのフン
    2001/07/05 Masafumi Okuta 
    $Id: kmdnncam.c,v 1.1.1.3 2002/11/19 11:47:51 Yoshizawa1 Exp $
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

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"

extern int MatToXRot( FMATRIX* );
extern int MatToYRot( FMATRIX* );

inline int BP_AdjustTick(int);
inline int BP_AdjustTick2(int);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))

/*----------------------------------------------------------------*/
#define	RAISE_ADD	( 20000 )
#define	RAISE_SPR	( 10000 )

#define INIT_BASE_RAD	(8.f)
#define INIT_ADD_RAD	(0.3f)

#define	COL_R_ADD	(139)
#define	COL_G_ADD	(150)
#define	COL_B_ADD	(115)
#define	COL_A_ADD	(128)

#define	COL_R2_ADD	(139)
#define	COL_G2_ADD	(150)
#define	COL_B2_ADD	(115)
#define	COL_A2_ADD	(128)

#define	SCREEN_NEAR	( 51.0f )	// カメラから離す距離

#define N_PRIMS		(1)		// 
#define N_VERTS		(32 + 2)	// 
#define N_VERTS_CIR	(32)		// 
#define N_CIRCLE_DIV	(16)		// 

#define SPREAD_SPD	(0.1f)	// 拡散速度
#define DROP_SPD	(0.2f)	// 落下速度
#define DIR_RATE	(1.8f)	// 指向移動レート

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

// テクスチャstrcode
#define TEX_DROP_01_ALP  	(7840500) 	// "kmo_droppings01_alp"
#define TEX_DROP_02_ALP  	(8889076) 	// "kmo_droppings02_alp"
#define TEX_DROP_03_ALP  	(9937652) 	// "kmo_droppings03_alp"
#define TEX_DROP_04_ALP  	(10986228) 	// "kmo_droppings04_alp"

typedef	struct	{
    // 基本システム
    GV_ACT_EX	actor;

    // 拡張システム
    int		nLife; 				// 存在時間
    int		nAlpha;				// 本体α値
    int		nAlphaSpr;			// 飛抹α値
    float	fCenterX;			// 中心X
    float	fCenterY;			// 中心Y
    float	fAddX;				// 中心X加算分
    float	fAddY;				// 中心Y加算分
    float	fDirSpd;			// 指向速度
    float	fDropSpd;			// 落下速度
    float	fSpreadSpd;			// 拡散速度
    SVECTOR	svecPreCam;			// １フレーム前のカメラ方向	
    float	fRadius[N_PRIMS*N_VERTS];	// 半径
    float	fAddPos[N_PRIMS*N_VERTS*2];	// 加算位置
    int		nDelay[N_VERTS]; 		// 飛抹ディレイ
    float	fSprAddPos[N_PRIMS*N_VERTS*2];	// 飛抹加算位置

    // 描画関連
    DG_PRIM2*	prim;				// 本体
    DG_PRIM2*	prim_spr;			// 飛抹	
} Work;

//static Work* _KmmDannaCam = NULL;
static int _KmmDannaCamCntr = 0;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
    DG_PRIM2*	prim;
    DG_PRIM2*	prim_spr;
    FVECTOR*	pvecPos;
    FVECTOR 	vecZ = { 0.f, 0.f, 1.f, 1.f };
    SVECTOR	svecCurrCam;
    SVECTOR	svecDiffCam;
    float	screen_near_x;
    float	screen_near_y;
    float*	pfRad;
    float*	pfAdd;
    int		nMoveDir;
    int		clock;
    int		i,k;
    DG_PRIM2_UVRGB*	puvrgb;

    prim = work->prim;

    // 主観チェック
    if( !GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) ){
	DG_InvisiblePrim2( prim );
	GV_DestroyActor( work ) ;
	return;
    }

    DG_VisiblePrim2( prim );
    GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
    DG_SwitchBuffPrim2( prim );
    clock = prim->buffer_clock;

    // 画面サイズ取得
    screen_near_x = SCREEN_NEAR / (ASPECT_X()                                       ) / DG_Chanls->screen;
    screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

    // 現在の方向を取得
    svecCurrCam.vx = MatToXRot( &DG_Chanls[0].eye);
    svecCurrCam.vy = MatToYRot( &DG_Chanls[0].eye);
    svecCurrCam.vz = 0;

    // 前のフレームとの方向差取得
    svecDiffCam.vx = svecCurrCam.vx - work->svecPreCam.vx;
    svecDiffCam.vy = svecCurrCam.vy - work->svecPreCam.vy;
    svecDiffCam.vz = svecCurrCam.vz - work->svecPreCam.vz;

    // カメラのZ軸のX,Z成分から指向速度を求める
    //    work->fDirSpd = DG_SQRT( (pvecCamZ->vx + pvecCamZ->vx) * (pvecCamZ->vz + pvecCamZ->vz)); 
    work->fDropSpd   *= 0.98f;
    work->fDirSpd    *= 0.98f;
    work->fSpreadSpd *= 0.8f;
    { // カメラの移動方向を取得
	float fAng;

	DG_SetPos2( &DG_ZeroVector, &svecDiffCam);
	DG_PutVector( &vecZ, &vecZ, 1);

	fAng = atan2f( -vecZ.vx, vecZ.vy ) ;
	nMoveDir = (int) (2048.0f * fAng / ( float )M_PI) ;
	nMoveDir &= 4095;

	_sceVu0ScaleVector( &vecZ, &vecZ, 2.f * work->fDirSpd );
//printf("%d :: %f %f\n", nMoveDir, vecZ.vx, vecZ.vy);
    }

    work->fAddX += vecZ.vx * work->fDirSpd * 0.45f;
    work->fAddY += -vecZ.vy * work->fDirSpd * 0.45f + work->fDropSpd * 0.4f;

    pvecPos = prim->pos[clock];
    pfRad   = work->fRadius;
    pfAdd   = work->fAddPos;
    puvrgb  = prim->uvrgb[clock]; 
    for ( i = 0 ; i < N_PRIMS; i++ ){
	FVECTOR* pvecTop;
	DG_PRIM2_UVRGB*	puvrgbTop;

	pvecTop   = pvecPos;
	puvrgbTop = puvrgb;
	for ( k = 0 ; k < N_VERTS_CIR; k++ ){
	    int 	nDir;
	    float 	fCos,fSin, fRateMove, fRateDrop;
	    extern float _TS_Sin( int s );

	    nDir = ((int)(k / 2) % N_CIRCLE_DIV) * 4096 / N_CIRCLE_DIV;
	    fCos = _TS_Sin( nDir  );
	    fSin = _TS_Sin( nDir + 1024);

	    fRateMove = (float)GV_DiffDirAbs( nMoveDir, nDir) / (float)2048.f * 0.8f + 0.2f;   // 移動分影響レート
	    fRateDrop = ((float)GV_DiffDirAbs( 2048, nDir) / (float)2048.f) * 0.8f + 0.2f;     // 落下分影響レート

	    // 半径更新
	    (*pfRad) += work->fSpreadSpd;	// 拡散

	    // 頂点設定
	    if ( k % 2 == 0){
		pvecPos->vx = work->fCenterX + work->fAddX;
		pvecPos->vy = work->fCenterY + work->fAddY;
		pvecPos->vz = SCREEN_NEAR;
		pfAdd++;
		pfAdd++;
	    }else{
		(*pfAdd) += vecZ.vx * fRateMove * work->fDirSpd;
		pvecPos->vx = work->fCenterX + fCos * (*pfRad) + (*pfAdd);
		pfAdd++;
		(*pfAdd) += -vecZ.vy * fRateMove * work->fDirSpd + work->fDropSpd * fRateDrop;
		pvecPos->vy = work->fCenterY + fSin * (*pfRad) + (*pfAdd);// * work->fDirSpd;
		pfAdd++;
		pvecPos->vz = SCREEN_NEAR;

	    }
	    // α設定
	    puvrgb->a = work->nAlpha * (2.f / DG_Chanls[0].screen);

	    pfRad++;
	    pvecPos++;
	    puvrgb++;
	}
	{ // 末尾と先頭を結ぶ
	    _sceVu0CopyVector( pvecPos, pvecTop);
	    pvecPos++;
	    pvecTop++;
	    _sceVu0CopyVector( pvecPos, pvecTop);
	    pvecPos++;

	    (*puvrgb) = (*puvrgbTop);
	    puvrgb++;
	    puvrgbTop++;
	    (*puvrgb) = (*puvrgbTop);
	    puvrgb++;
	}
    }

    // 飛抹
    prim_spr = work->prim_spr;
    GM_GroupPrim2( prim_spr, GM_CurrentStageMap ) ;
    DG_SwitchBuffPrim2( prim_spr );
    clock = prim_spr->buffer_clock;

    DG_VisiblePrim2( prim_spr );
    for ( k = 0 ; k < N_VERTS_CIR; k++ ){
	if( work->nDelay[k] > 0 ){
	    work->nDelay[k]--;
	}
    }

    {
	float* 		pfAdd;
	FVECTOR* 	pvecPos;

	pfAdd = work->fSprAddPos;
	pvecPos = prim_spr->pos[clock];
        puvrgb  = prim_spr->uvrgb[clock]; 
	for ( i = 0 ; i < N_PRIMS; i++ ){
	    FVECTOR* 		pvecTop;
	    DG_PRIM2_UVRGB*	puvrgbTop;

	    pvecTop   = pvecPos;
	    puvrgbTop = puvrgb;
	    for ( k = 0 ; k < N_VERTS_CIR; k++ ){
		int 	nDir;
		float 	fCos,fSin, fRateDrop;
		extern float _TS_Sin( int s );

		nDir = ((int)(k / 2) % N_CIRCLE_DIV) * 4096 / N_CIRCLE_DIV;
		fCos = _TS_Sin( nDir  );
		fSin = _TS_Sin( nDir + 1024);

		fRateDrop = ((float)GV_DiffDirAbs( 2048, nDir) / (float)2048.f) * 0.8f + 0.2f;     // 落下分影響レート
		// 座標設定
		if ( k % 2 == 0){
		    pvecPos->vx = work->fCenterX;
		    pvecPos->vy = work->fCenterY;
		    pvecPos->vz = SCREEN_NEAR;
		    pfAdd++;
		    pfAdd++;
		}else{
		    pvecPos->vx = work->fCenterX + fCos * 12.f + (*pfAdd);
		    pfAdd++;
		    (*pfAdd) += work->fDropSpd * fRateDrop * 0.5f;
		    pvecPos->vy = work->fCenterY + fSin * 12.f + (*pfAdd);
		    pfAdd++;
		    pvecPos->vz = SCREEN_NEAR;
		}
		pvecPos++;

		// α設定でディレイを実現
		if( work->nDelay[k] > 0 || k % 2 == 0 ) puvrgb->a = 0;
		else			  		puvrgb->a = work->nAlphaSpr * (2.f / DG_Chanls[0].screen);
		puvrgb++;
	    }
	    { // 末尾と先頭を結ぶ
		_sceVu0CopyVector( pvecPos, pvecTop);
		pvecPos++;
		pvecTop++;
		_sceVu0CopyVector( pvecPos, pvecTop);
		pvecPos++;

		(*puvrgb) = (*puvrgbTop);
		puvrgb++;
		puvrgbTop++;
		(*puvrgb) = (*puvrgbTop);
		puvrgb++;
	    }
	}
    }
#if 0
    // 寿命チェック
    if( work->nLife > 0 ){
	work->nLife--;
	if( work->nLife <= 0 ){
	    GV_DestroyActor( work ) ;
	}
    }
    // α減衰
    if ( work->nLife <= COUNT_VMODE(128)){
	work->nAlpha = work->nLife;
	if ( work->nAlpha < 0 ) work->nAlpha = 0;
	work->nAlphaSpr = work->nLife;
	if ( work->nAlphaSpr < 0 ) work->nAlphaSpr = 0;
    }
#endif

    // カメラ方向を取得しておく
    work->svecPreCam = svecCurrCam;
}

// 破棄関数
static void Die( Work *work )
{
    work->prim     = MAO_FreePrim2( work->prim );
    work->prim_spr = MAO_FreePrim2( work->prim_spr );
//    _KmmDannaCam = NULL; // 使用可能状態にしておく

    _KmmDannaCamCntr--;
}

/*----------------------------------------------------------------*/
static int InitPacket( DG_PRIM2 *prim, DG_TEX *tex, int nFlag )
{
    int			i, k;
    int			col_r;
    int			col_g;
    int			col_b;
    int			col_a;
    FVECTOR*		pvec1;
    FVECTOR*		pvec2;
    DG_PRIM2_UVRGB*	uvrgb0;
    DG_PRIM2_UVRGB*	uvrgb1;

    if ( nFlag == 0){
	col_r = COL_R_ADD;
	col_g = COL_G_ADD;
	col_b = COL_B_ADD;
	col_a = COL_A_ADD;
    }else{
	col_r = COL_R2_ADD;
	col_g = COL_G2_ADD;
	col_b = COL_B2_ADD;
	col_a = COL_A2_ADD;
    }

    DG_ConfigPrim2Tex( prim, tex );

    pvec1  = prim->pos[0];
    pvec2  = prim->pos[1];
    uvrgb0 = prim->uvrgb[0] ;
    uvrgb1 = prim->uvrgb[1] ;
    for ( i = 0 ; i < N_PRIMS; i++ ){
	int 	nDirBase;
	nDirBase = MAO_Rand(4096);
	for ( k = 0 ; k < N_VERTS; k++ ){
	    int 	nDir;
	    float 	fCos,fSin;
	    extern float _TS_Sin( int s );
	    nDir = nDirBase + ((int)(k / 2) % N_CIRCLE_DIV) * 4096 / N_CIRCLE_DIV;
	    nDir %= 4096;
	    fCos = _TS_Sin( nDir  );
	    fSin = _TS_Sin( nDir + 1024);
	    if ( k % 2 == 0){
		uvrgb0->u = uvrgb1->u = FTOI12( (0.5f) * tex->u_scale + tex->u_offset );
		uvrgb0->v = uvrgb1->v = FTOI12( (0.5f) * tex->v_scale + tex->v_offset );
	    }else{
		uvrgb0->u = uvrgb1->u = FTOI12( ((fCos * 0.5f) + 0.5f) * tex->u_scale + tex->u_offset );
		uvrgb0->v = uvrgb1->v = FTOI12( ((fSin * 0.5f) + 0.5f) * tex->v_scale + tex->v_offset );
	    }
	    uvrgb0->q = uvrgb1->q = 4096 ;
	    uvrgb0->f = uvrgb1->f = 0x0fff ;
	    uvrgb0->r = uvrgb1->r = col_r ;
	    uvrgb0->g = uvrgb1->g = col_g ;
	    uvrgb0->b = uvrgb1->b = col_b ;
	    if ( nFlag == 1 ){
		if ( k % 2 == 0) 	uvrgb0->a = uvrgb1->a = 0; 
		else 			uvrgb0->a = uvrgb1->a = col_a;
	    }else{
		uvrgb0->a = uvrgb1->a = col_a;
	    }
	    _sceVu0CopyVector( pvec1, &DG_ZeroVector);
	    _sceVu0CopyVector( pvec2, &DG_ZeroVector);
	    pvec1++;
	    pvec2++;
	    uvrgb0++;
	    uvrgb1++;
	}
    }

    return 1;
}
// 初期設定
static int GetResources( Work *work, FVECTOR* pvecCenter, int nLife )
{
    int			i, k;
    DG_PRIM2*		prim;
    DG_PRIM2*		prim_spr;
    DG_TEX*		tex;
    FVECTOR* 		pvecCamZ      = (FVECTOR*)DG_Chanls[0].eye.m[2];
    FMATRIX*		pmatWorld2Scr = &DG_Chanls[0].eye_pers;

    work->nLife  	= nLife; 	// 表示時間
    work->nAlpha 	= COL_A_ADD; 	// 本体α値
    work->nAlphaSpr 	= COL_A2_ADD; 	// 飛抹α値

    {
	float	screen_near_x;
	float	screen_near_y;
	FVECTOR vecScreen;
	// 透視変換
	_sceVu0ApplyMatrix( &vecScreen, pmatWorld2Scr, pvecCenter);
	if ( vecScreen.vw != 0.f){
	    vecScreen.vx /= vecScreen.vw;
	    vecScreen.vy /= vecScreen.vw;
	    vecScreen.vz /= vecScreen.vw;
	}
	
	// 画面サイズを取得
	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls[0].screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls[0].width / DG_Chanls[0].height) / DG_Chanls[0].screen;

	// 中心を設定
	work->fCenterX = vecScreen.vx * screen_near_x;
	work->fCenterY = vecScreen.vy * screen_near_y;

	// 中心加算分
	work->fAddX = 0.f;
	work->fAddY = 0.f;
    }

    // 落下速度
    work->fDropSpd = DROP_SPD;

    // カメラのZ軸のX,Z成分から指向速度を求める
    work->fDirSpd = DG_SQRT( (pvecCamZ->vx + pvecCamZ->vx) * (pvecCamZ->vz + pvecCamZ->vz)) * DIR_RATE; 

    // 拡散速度
    work->fSpreadSpd = SPREAD_SPD;

    // １フレーム前のカメラ方向
    work->svecPreCam.vx = MatToXRot( &DG_Chanls[0].eye);
    work->svecPreCam.vy = MatToYRot( &DG_Chanls[0].eye);
    work->svecPreCam.vz = 0;

    // 位置設定
    {
	float* pfRad;
	float* pfAdd;

	pfRad = work->fRadius;
	pfAdd = work->fAddPos;
	for ( i = 0 ; i < N_PRIMS; i++ ){
	    for ( k = 0 ; k < N_VERTS; k++ ){
		// 半径設定
		(*pfRad) = INIT_BASE_RAD + (INIT_ADD_RAD * frnd());
		pfRad++;

		// 加算分X、Y値初期化
		(*pfAdd) = 0.f; // X
		pfAdd++;
		(*pfAdd) = 0.f; // Y
		pfAdd++;
	    }
	}
    }


//    tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
    tex = DG_GetTexture( TEX_DROP_02_ALP );

    // 本体プリミティブ作成
    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, 
				      N_PRIMS, N_VERTS );
    if ( prim == NULL ){
	printf("null prim\n");
	return -1;
    }
    DG_InvisiblePrim2( prim ); // 非表示
    if( !InitPacket( prim, tex, 0 ) ) return -1; // パケット初期化
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ) ); // Half
    prim->raise = RAISE_ADD; // 優先

    // 飛抹プリミティブ作成
//    tex = DG_GetTexture( 15638432 /*"blood_1bw_msk"*/ );
    tex = DG_GetTexture( TEX_DROP_03_ALP);
    prim_spr = work->prim_spr = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, 
				      N_PRIMS, N_VERTS );
    if ( prim_spr == NULL ){
	printf("null prim_spr\n");
	return -1;
    }
    DG_InvisiblePrim2( prim_spr ); // 非表示
    if( !InitPacket( prim_spr, tex, 1 ) ) return -1; // パケット初期化

    for ( k = 0 ; k < N_VERTS; k++ ){
	work->nDelay[k] = MAO_GetRandom(COUNT_VMODE(2), COUNT_VMODE(5)) * (k % 3);
    }
    {
	float* pfAdd;

	pfAdd = work->fSprAddPos;
	for ( i = 0 ; i < N_PRIMS; i++ ){
	    for ( k = 0 ; k < N_VERTS; k++ ){
		(*pfAdd) = 0; // X
		pfAdd++;
		(*pfAdd) = 0; // Y
		pfAdd++;
	    }
	}
    }

    DG_SetPrim2Alpha( prim_spr, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ) ); // Half
    prim_spr->raise = RAISE_SPR; // 優先

    // 効果音
    GM_SeSetMode( SD_A_FUNHIT01, pvecCenter, GM_SEMODE_NORMAL);

    // 振動
    {
	extern void *NewPadVibration( char *script, int type );
	static	u_char	KamomeDannaCamVibH[] = { 64, 1, 0, 0 } ;
	static	u_char	KamomeDannaCamVibL[] = { 128, 6, 0, 0 } ;
	NewPadVibration( KamomeDannaCamVibH, 1); // 高周波
	NewPadVibration( KamomeDannaCamVibL, 2); // 低周波
    }

    return 0 ;
}

// カモメのフンがカメラ付着
void* NewKamomeDannaCamera( FVECTOR* 	pvecCenter,	// 中心
			    int 	nLife )		// 表示時間
{
    Work*	work;

    OPERATOR();

    // 既に起動していたら生成しない
//    if( _KmmDannaCam != NULL ) return NULL;
    if ( _KmmDannaCamCntr > 8 ) return NULL;

    work = (Work* )GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, pvecCenter, nLife ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL;
	}
    }

//    _KmmDannaCam = work;
    _KmmDannaCamCntr++;

    return (void *)work ;
}

