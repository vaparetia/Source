//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    waterspread.c
    水飛抹
    2001/07/26 Masafumi Okuta
    $Id: waterspread.c,v 1.1.1.3 2002/11/19 11:47:55 Yoshizawa1 Exp $
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
#include	"../../mode/demo/eft_con.h"

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))


#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_RND			(SCRPAD_ADDR + 0x3000)
#define	SCR_UVR			(SCRPAD_ADDR + 0x3000)
#define RANDAM_FIELD_NUM	(0x1000 / 4)
#define N_PRIMS			(32)
#define N_VERTS			(32)
#define N_POLY			(N_VERTS / 4)

#define	BUBBLE_SIZE_MIN			(40.0f)
#define	BUBBLE_SIZE_RAND		(20.0f)
#define	BUBBLE_SPEED			( 8.0f )

#define LIFE_MIN	(48)
#define LIFE_MAX	(64)

// 色
#define	COL_R	(128)
#define	COL_G	(128)
#define	COL_B	(128)
#define	COL_A	(128)

extern float GM_WaterLevel;
extern float _TS_Sin( int s );

enum{ // メッセージ
SPREAD_MESSAGE_ACTIVE,	// アクティブモード
SPREAD_MESSAGE_SLEEP,	// スリープモード
SPREAD_MESSAGE_STOP,	// 生成終了
SPREAD_MESSAGE_RESTART,	// 生成再開
SPREAD_MESSAGE_KILL,	// 破棄
SPREAD_MESSAGE_RAD_CHG,	// 半径変更
};

typedef struct _WATERSPREAD {
    // 基本システム
    GV_ACT_EX		actor;
    int			name;				// 名前ID
    GV_MSG*		msg;				// 自分宛メッセージ	

    // 拡張システム
    int			nSleep;				// スリープモード
    int			nFeed;				// フェードモード
    int			nFeedCntr;			// フェードカウンタ
    u_char*		ucLife;				// 生存時間列
    FVECTOR*		vecStep;			// 移動ベクトル列	

    // 引数
    int			nDemoFlag;			// デモフラグ
    FMATRIX		matDemo;			// デモ用
    EFTCONTROL*		eftctrl;			// デモエフェクトコントロール
    int			nPrims;				// プリミティブ数
    int			nAddNum;			// 同時生成数
    float		fRadMin;			// 半径最小
    float		fRadMax;			// 半径最大
    int			nLifeMin;			// 生存時間最小
    int			nLifeMax;			// 生存時間最大
    float		fSizeBase;			// サイズベース
    float		fSizeAdd;			// サイズランダム幅
    float		fGravity;			// 重力
    int			nRGBA;				// 開始時のRGBA値
    float		fSpreadRadBase;			// 移動ベクトルXZ成分:基本	
    float		fSpreadRadAdd;			// 移動ベクトルXZ成分:ランダム幅
    float		fSpreadHeightBase;		// 移動ベクトルY成分:基本
    float		fSpreadHeightAdd;		// 移動ベクトルY成分:ランダム幅

    // 外部参照
    FMATRIX*		pmatOrg;			// 中心

    // 描画ワーク
    DG_PRIM2*		prim;
} Work;

static void CheckMessage( Work* work )
{
    GV_MSG	*msg ;
    int n_msg, code ;

    n_msg = GV_ReceiveMessage( work->name, &work->msg ); // メッセージ取得
    msg = work->msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ];
	switch( code ) {
	case SPREAD_MESSAGE_ACTIVE:	// アクティブモード
	    work->nSleep 	= 0;
	    work->nFeed  	= 0;
	    work->nFeedCntr  	= 0;
	    work->prim->flag &= ~DG_PRIM2_INVISIBLE;
	    break;
	case SPREAD_MESSAGE_SLEEP:	// スリープモード
	    work->nSleep 	= 1;
	    work->nFeed  	= 0;
	    work->nFeedCntr  	= 0;
	    work->prim->flag |= DG_PRIM2_INVISIBLE;
	    break;
	case SPREAD_MESSAGE_STOP:
	    work->nSleep 	= 0;
	    work->nFeed  	= 1;
	    work->nFeedCntr  	= LIFE_MAX;
	    break;
	case SPREAD_MESSAGE_RESTART:
	    work->nSleep 	= 0;
	    work->nFeed  	= 0;
	    work->nFeedCntr  	= LIFE_MAX;
	    work->prim->flag &= ~DG_PRIM2_INVISIBLE;
	    break;
	case SPREAD_MESSAGE_KILL:	// 破棄
	    GV_DestroyActor( work ) ;
	    break;
	case SPREAD_MESSAGE_RAD_CHG:	// 半径変更
	    work->fRadMin = (float)msg->message[ 1 ];
	    work->fRadMax = (float)msg->message[ 2 ];
	    break;
	default :
	    break;
	}
	msg++ ;
    }
}

static void Act( Work *work )
{
    int 	i, j, clock;
    DG_PRIM2* 	prim;

    // メッセージチェック
    CheckMessage( work);

    // デモ用
    if ( work->nDemoFlag){
	DM_EftControlMatrix( work->eftctrl, &work->matDemo);
    }

    // スリープモードチェック
    if ( work->nSleep ) return;

    // プリミティブ更新
    prim = work->prim;
    GM_GroupPrim2( prim, GM_CurrentStageMap );
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( prim ) )
    {
       return;
    }
    clock = prim->buffer_clock;

    {
	u_char*	 pucAlpha;
	int	 nAddNum;
	float	 fAddRad;
	FVECTOR* pvecPos;
	FVECTOR* pvecStep;
	FVECTOR* pvecStepTop;
	FVECTOR* pvecPosTop;
	FVECTOR* pvecPreTop;
	DG_PRIM2_UVRGB* uvrgb;

	fAddRad     = work->fRadMax - work->fRadMin;
	pvecStep    = work->vecStep;
	pvecStepTop = work->vecStep;
	pucAlpha    = work->ucLife;
	pvecPosTop  = prim->pos[ clock ];
	pvecPreTop  = prim->pos[ (clock+1) % 2 ];
	uvrgb 	    = prim->uvrgb[clock];

	nAddNum = 0;
	for ( i = 0; i < work->nPrims; i++ ){
	    pvecPos = SCR_POS;
	    Mao_CopyMemToScr( pvecPreTop, SCR_POS, sizeof(FVECTOR), N_VERTS);
	    for ( j = 0; j < N_POLY; j++ ){
		// ワークリセットはα値をトリガーにする
		if ( (*pucAlpha) > 0 ){ // 動作
		    // α更新
		    (*pucAlpha)--;
		    uvrgb->a = (*pucAlpha);
		    uvrgb++;
		    uvrgb->a = (*pucAlpha);
		    uvrgb++;
		    uvrgb->a = (*pucAlpha);
		    uvrgb++;
		    uvrgb->a = (*pucAlpha) >> 1; // 水面めりこみを軽減するための措置
		    uvrgb++;

		    // 移動
		    _sceVu0AddVector( pvecPos, pvecPos, pvecStep);
		    pvecPos++;
		    _sceVu0AddVector( pvecPos, pvecPos, pvecStep);
		    pvecPos++;
		    _sceVu0AddVector( pvecPos, pvecPos, pvecStep);
		    pvecPos++;
		    _sceVu0AddVector( pvecPos, pvecPos, pvecStep);
		    pvecPos++;

		    pvecStep->vy -= work->fGravity;
		    pvecStep++;
		}else if ( !work->nFeed && nAddNum < work->nAddNum){ // α = 0 でリセット
		    FVECTOR	vecCenter;
		    FVECTOR	vecDir;
		    FVECTOR*	pvec;
		    int 	nAng = BP_PS2_rand() & 4095;
		    int 	nAngPoly = BP_PS2_rand() & 4095;
		    float 	fRad  = fAddRad * rnd() + work->fRadMin;
		    float 	fRadStep = work->fSpreadRadBase + rnd() * work->fSpreadRadAdd;
		    float 	fSize = work->fSizeBase + rnd() * work->fSizeAdd;
		    float	fX, fY;

		    pvec = pvecPos;

		    // 中心を設定（相対）
		    vecCenter.vx = _TS_Sin( nAng ) * fRad ;
		    vecCenter.vy = frnd() * 100.f + 20.f;
		    vecCenter.vz = _TS_Sin( nAng + 1024 ) * fRad;
		    vecCenter.vw = 1.f;

		    // 移動ベクトル設定（相対）
		    pvecStep->vx = _TS_Sin( nAng ) * fRadStep;
		    pvecStep->vy = work->fSpreadHeightBase + rnd() * work->fSpreadHeightAdd;
		    pvecStep->vz = _TS_Sin( nAng + 1024 ) * fRadStep;
		    pvecStep->vw = 1.f;

		    // 方向ベクトルを作成
		    _sceVu0Normalize( &vecDir, pvecStep);
		    _sceVu0ScaleVector( &vecDir, &vecDir, fSize * 2.f);

		    // 移動ベクトルを絶対化
		    DG_SetPos( work->pmatOrg);
		    DG_RotVector( pvecStep, pvecStep, 1);
		    pvecStep++;

		    // 頂点設定
		    fX = _TS_Sin( nAngPoly ) * fSize;
		    fY = _TS_Sin( nAngPoly + 1024 ) * fSize;

		    _sceVu0CopyVector( pvecPos, &vecCenter);
		    _sceVu0AddVector( pvecPos, pvecPos, &vecDir);
		    pvecPos++;

		    _sceVu0CopyVector( pvecPos, &vecCenter);
		    pvecPos->vx += fX;
		    pvecPos->vy += fY;
		    pvecPos++;

		    _sceVu0CopyVector( pvecPos, &vecCenter);
		    pvecPos->vx -= fX;
		    pvecPos->vy -= fY;
		    pvecPos++;

		    _sceVu0CopyVector( pvecPos, &vecCenter);
		    _sceVu0SubVector( pvecPos, pvecPos, &vecDir);
		    pvecPos++;

		    // 頂点位置の絶対化
		    DG_SetPos( work->pmatOrg);
		    DG_PutVector( pvec, pvec, 4);

		    // α値リセット
		    (*pucAlpha) = MAO_GetRandom( LIFE_MIN, LIFE_MAX);
		    uvrgb->a = (*pucAlpha);
		    uvrgb++;
		    uvrgb->a = (*pucAlpha);
		    uvrgb++;
		    uvrgb->a = (*pucAlpha);
		    uvrgb++;
		    uvrgb->a = (*pucAlpha) >> 1; // めりこみ対策
		    uvrgb++;

		    nAddNum++; // 生成数更新
		}else{
		    pvecPos 	+=	4;
		    uvrgb 	+=	4;
		    pvecStep++;
		}
		pucAlpha++;
	    }

	    // スクラッチパッド->メインメモリ
	    Mao_CopyScrToMem( pvecPosTop, SCR_POS, sizeof(FVECTOR), N_VERTS);

	    pvecStepTop += N_POLY;
	    pvecPosTop  += N_VERTS;
	    pvecPreTop  += N_VERTS;
	}
    }

    // フェードからスリープへ
    if ( work->nFeed && work->nFeedCntr > 0){
	work->nFeedCntr--;
	if ( work->nFeedCntr == 0 ){
	    work->nSleep = 1;
	    work->nFeed  = 0;
	    work->prim->flag |= DG_PRIM2_INVISIBLE;
	}
    }

}

static void Die( Work *work )
{
    work->prim = MAO_FreePrim2( work->prim );
    if ( work->vecStep != NULL)		GV_Free( work->vecStep);
    if ( work->ucLife   != NULL)	GV_Free( work->ucLife);
}
// パケット初期化
static int InitPacket( Work* 		work, 	 
		       DG_PRIM2*	prim,	
		       DG_TEX* 		tex, 	
		       int		color)
{
    FVECTOR*		pos;
    FVECTOR*		pos1;
    FVECTOR*		pos2;
    FVECTOR*		pvecStep;
    DG_PRIM2_UVRGB*	uvrgb1;
    DG_PRIM2_UVRGB*	uvrgb2;
    DG_PRIM2_UVRGB*	uvrgb;
    u_char*		pucAlpha;
    int		i, j;
    int		col_r;
    int		col_g;
    int		col_b;
    int		col_a;

    float	fAddRad;

    prim->raise = 3000;

    DG_ConfigPrim2Tex( prim, tex );

    pos      = prim->pos[ 0 ];
    pos1     = prim->pos[ 0 ];
    pos2     = prim->pos[ 1 ];
    uvrgb1 = prim->uvrgb[ 0 ];
    uvrgb2 = prim->uvrgb[ 1 ];
    pucAlpha   = work->ucLife;
    pvecStep   = work->vecStep;

    fAddRad = work->fRadMax - work->fRadMin;
    for ( i = 0; i < work->nPrims; i++ ){
	pos = SCR_POS;
	for ( j = 0; j < N_POLY; j++ ){
	    FVECTOR	vecCenter;
	    FVECTOR	vecDir;
	    int 	nAng = BP_PS2_rand() & 4095;
	    int 	nAngPoly = BP_PS2_rand() & 4095;
	    float 	fRad  = fAddRad * rnd() + work->fRadMin;
	    float 	fSize = work->fSizeBase + rnd() * work->fSizeAdd;
	    vecCenter.vx = _TS_Sin( nAng ) * fRad ;
	    vecCenter.vy = frnd() * 100.f + 20.f;
	    vecCenter.vz = _TS_Sin( nAng + 1024 ) * fRad;
	    vecCenter.vw = 1.f;

	    // 方向ベクトル取得
	    _sceVu0Normalize( &vecDir, pvecStep);
	    _sceVu0ScaleVector( &vecDir, &vecDir, fSize * 2.f);
	    pvecStep++;

	    // 頂点設定
	    _sceVu0CopyVector( pos, &vecCenter);
	    _sceVu0AddVector( pos, pos, &vecDir);
	    pos++;

	    _sceVu0CopyVector( pos, &vecCenter);
	    pos->vx += _TS_Sin( nAngPoly ) * fSize;
	    pos->vy += _TS_Sin( nAngPoly + 1024 ) * fSize;
	    pos++;

	    _sceVu0CopyVector( pos, &vecCenter);
	    pos->vx -= _TS_Sin( nAngPoly ) * fSize;
	    pos->vy -= _TS_Sin( nAngPoly + 1024 ) * fSize;
	    pos++;

	    nAngPoly += 1024;
	    _sceVu0CopyVector( pos, &vecCenter);
	    _sceVu0SubVector( pos, pos, &vecDir);
	    pos->vx += _TS_Sin( nAngPoly ) * fSize;
	    pos->vy += _TS_Sin( nAngPoly + 1024 ) * fSize;
	    pos++;
	}
	// まとめて頂点を絶対化
	DG_SetPos( work->pmatOrg);
	DG_PutVector( SCR_POS, SCR_POS, N_VERTS);
	// スクラッチパッド->メインメモリ
	Mao_CopyScrToMem( pos1, SCR_POS, sizeof(FVECTOR), N_VERTS);
	Mao_CopyScrToMem( pos2, SCR_POS, sizeof(FVECTOR), N_VERTS);

	col_r = (color >> 24);
	col_g = (color >> 16) & 0xff;
	col_b = (color >>  8) & 0xff;
	col_a =  color & 0xff;

	uvrgb = SCR_POS;
	for ( j = 0; j < N_POLY; j++ ){
	    (*pucAlpha) = 0;
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ); //
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ); //
	    uvrgb->a = (*pucAlpha);
	    uvrgb++;
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ); //
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ); //
	    uvrgb->a = (*pucAlpha);
	    uvrgb++;
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ); //
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ); //
	    uvrgb->a = (*pucAlpha);
	    uvrgb++;
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ); //
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ); //
	    uvrgb->a = (*pucAlpha) >> 1;
	    uvrgb++;

	    pucAlpha++;
	}
	uvrgb = SCR_POS;
	for ( j = 0; j < N_VERTS; j++ ){
	    uvrgb->q = 4096 ;
	    if ( j % 4 < 2 ) uvrgb->f = 0x8fff; // No-Kick
	    else	     uvrgb->f = 0x0fff;
	    uvrgb->r  = col_r ;
	    uvrgb->g  = col_g ;
	    uvrgb->b  = col_b ;

	    uvrgb++;
	}
	// スクラッチパッド->メインメモリ
	Mao_CopyScrToMem( uvrgb1, SCR_POS, sizeof(DG_PRIM2_UVRGB), N_VERTS);
	Mao_CopyScrToMem( uvrgb2, SCR_POS, sizeof(DG_PRIM2_UVRGB), N_VERTS);

	pos1 += N_VERTS;
	pos2 += N_VERTS;
	uvrgb1 += N_VERTS;
	uvrgb2 += N_VERTS;
    }
	
    return 1;
}

static int GetResources( Work*		work, 			//
			 int		name, 			//
			 int		nDemoFlag,		// 
			 FMATRIX*	pmatOrg, 		// 
			 int		nPrims, 		// 
			 int		nAddNum, 		// 
			 float		fRadMin, 		// 
			 float		fRadMax, 		// 
			 int		nLifeMin, 		// 
			 int		nLifeMax, 		// 
			 float		fSizeBase, 		// 
			 float		fSizeAdd, 		// 
			 float		fGravity, 		// 
			 int		nRGBA,	 		// 
			 float		fSpreadRadBase,		// 移動ベクトルXZ成分:基本	
			 float		fSpreadRadAdd,		// 移動ベクトルXZ成分:ランダム幅
			 float		fSpreadHeightBase,	// 移動ベクトルY成分:基本
			 float		fSpreadHeightAdd)      	// 移動ベクトルY成分:ランダム幅
{
    int i;

    // 引数のデータを取得
    work->name	   	= name;
    work->nDemoFlag	= nDemoFlag;
    work->pmatOrg  	= pmatOrg;
    work->nPrims   	= nPrims;
    work->nAddNum	= nAddNum;
    work->fRadMin  	= fRadMin;
    work->fRadMax  	= fRadMax;
    work->nLifeMin 	= nLifeMin;
    work->nLifeMax 	= nLifeMax;
    work->fSizeBase  	= fSizeBase;
    work->fSizeAdd  	= fSizeAdd;
    work->fGravity  	= fGravity * -P_GRAVITY;
    work->nRGBA 	= nRGBA;

    work->fSpreadRadBase 	= fSpreadRadBase;
    work->fSpreadRadAdd 	= fSpreadRadAdd;
    work->fSpreadHeightBase 	= fSpreadHeightBase;
    work->fSpreadHeightAdd 	= fSpreadHeightAdd;

    work->nSleep = 0;

    // 移動ベクトル
    if( ( work->vecStep = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nPrims * N_VERTS ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterSpread Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecStep, sizeof(FVECTOR) * work->nPrims * N_VERTS );

    // 生存時間	
    if( ( work->ucLife = (u_char *)GV_Malloc(sizeof(u_char) * work->nPrims * N_POLY ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterSpread Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->ucLife, sizeof(u_char) * work->nPrims * N_POLY );

    { // 初期値設定
	FVECTOR* pvecStep  = work->vecStep;
	for ( i = 0; i < work->nPrims * N_VERTS; i++ ){
	    int 	nAng = BP_PS2_rand() & 4095;
	    float 	fRad = work->fSpreadRadBase + rnd() * work->fSpreadRadAdd;

	    pvecStep->vx = _TS_Sin( nAng ) * fRad;
	    pvecStep->vy = work->fSpreadHeightBase + rnd() * work->fSpreadHeightAdd;
	    pvecStep->vz = _TS_Sin( nAng + 1024 ) * fRad;
	    pvecStep->vw = 1.f;
	    pvecStep++;
	}
	DG_SetPos( work->pmatOrg);
	DG_RotVector( work->vecStep, work->vecStep, work->nPrims * N_VERTS);
    }

    { // 描画設定
	DG_PRIM2* 	prim;
	DG_TEX*		tex;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->nPrims, N_VERTS );
	if ( prim == NULL ){
	    printf("null prim\n");
	    return -1;
	}
	tex = DG_GetTexture( 8781729 ); // splash

	InitPacket( work, prim, tex, work->nRGBA );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
    }

    return 0;
}

// 水飛抹
void* NewWaterSpread( int		name, 			//
		      FMATRIX*		pmatOrg, 		// 
		      int		nPrims, 		// 
		      int		nAddNum, 		// 
		      float		fRadMin, 		// 
		      float		fRadMax, 		// 
		      int		nLifeMin, 		// 
		      int		nLifeMax, 		// 
		      float		fSizeBase, 		// 
		      float		fSizeAdd, 		// 
		      float		fGravity, 		// 
		      int		nRGBA,	 		// 
		      float		fSpreadRadBase,		// 移動ベクトルXZ成分:基本	
		      float		fSpreadRadAdd,		// 移動ベクトルXZ成分:ランダム幅
		      float		fSpreadHeightBase,	// 移動ベクトルY成分:基本
		      float		fSpreadHeightAdd)      	// 移動ベクトルY成分:ランダム幅
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	_sceVu0CopyMatrix( &work->matDemo, &DG_UnitMatrix);
	if ( GetResources( work, name, 0, pmatOrg, nPrims, nAddNum, fRadMin, fRadMax, nLifeMin, nLifeMax,
			   fSizeBase, fSizeAdd, fGravity, nRGBA, fSpreadRadBase, fSpreadRadAdd, 
			   fSpreadHeightBase, fSpreadHeightAdd) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    
    return (void *)work ;
}


// 水飛抹
void* NewWaterSpreadDemo( int		name, 			//
			  int		nDemoEfName, 		// 
			  int		nPrims, 		// 
			  int		nAddNum, 		// 
			  float		fRadMin, 		// 
			  float		fRadMax, 		// 
			  int		nLifeMin, 		// 
			  int		nLifeMax, 		// 
			  float		fSizeBase, 		// 
			  float		fSizeAdd, 		// 
			  float		fGravity, 		// 
			  int		nRGBA,	 		// 
			  float		fSpreadRadBase,		// 移動ベクトルXZ成分:基本	
			  float		fSpreadRadAdd,		// 移動ベクトルXZ成分:ランダム幅
			  float		fSpreadHeightBase,	// 移動ベクトルY成分:基本
			  float		fSpreadHeightAdd)      	// 移動ベクトルY成分:ランダム幅
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	work->eftctrl = DM_GetEftControl( nDemoEfName );
	DM_EftControlMatrix( work->eftctrl, &work->matDemo);
	if ( GetResources( work, name, 1, &work->matDemo, nPrims, nAddNum, fRadMin, fRadMax, nLifeMin, nLifeMax,
			   fSizeBase, fSizeAdd, fGravity, nRGBA, fSpreadRadBase, fSpreadRadAdd, 
			   fSpreadHeightBase, fSpreadHeightAdd) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    
    return (void *)work ;
}
