//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    bloodweep.c
    滴り血:ブラッドピチョン君
    2001/07/28 Masafumi Okuta
    $Id: bloodweep.c,v 1.1.1.3 2002/11/19 11:47:49 Yoshizawa1 Exp $
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
#include	"../../okajima/effect/blood.h"

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))

#define	SET_FLAG( a,b )		(a) |= (b)
#define	UNSET_FLAG( a,b )	(a) &= ~(b)
#define	CLEAR_FLAG( a ) 	(a) = 0
#define	BODYWORLD(a,b) 		(a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define	SCR_POS			(SCRPAD_ADDR)

// 滴のプリミティブ
#define N_PRIMS		(1)
#define N_VERTS		(1)

// 滴り部分のプリミティブ
#define N_PRIMS_DRIP		(4)
#define N_VERTS_DRIP		(32)
#define N_VERTS_DRIP_CIR	(15)	// 円分割数

#define DRIP_FALL	(5.f)

#define SIZE		(40)
#define GRAVITY		(9.8f * 10.f)
#define LIFE		(COUNT_VMODE(120))

#define WDP_CHK_HZX  (HZX_CHK_RECOIL_TYPE | HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR)

extern float _TS_Sin( int s );

enum{ // 拡張フラグ
EXFLAG_NORM_ALP = 0x00000001, // 通常半透明
EXFLAG_ADD_ALP  = 0x00000002, // 加算半透明
EXFLAG_SUB_ALP  = 0x00000004, // 減算半透明
// チェック用
EXFLAG_CHK_ALPHA  = 0x0000000f, // 半透明モードチェック

};

typedef struct _BLOODWEEP {
    // 基本システム
    GV_ACT_EX		actor;
    
    // 拡張管理
    FVECTOR		vecPos;					// 滴位置
    FVECTOR		vecFloor;				// 着地法線
    FVECTOR		vecBoundary;				// 着地床のバウンダリ
    float		fDropHRate;				// 滴の大きさ
    float		fKillY;					// 消す高さ
    float		fDefaultY[N_PRIMS_DRIP * N_VERTS_DRIP];	// 発生時の高さ
    void		(*ActFunc)( struct _BLOODWEEP* );	// 処理関数

    // 引数から取得するデータ
    FMATRIX		matOrg;					// 座標系
    int			nCntr;					// カウンタ		
    int			nDripCntr;				// 溜りカウンタ		
    float		fFallSpd;				// 落下速度
    float		fDropSize;				// 滴のサイズ
    float		fDripRad;				// 滴り部分の半径
    float		fLentor;				// 粘性
    int			nRGBA;					// RGBA値
    int			nRandSeed;				// ランダムシード
    int			nFlag;					// フラグ
    u_char		ucDripAlpha;				// α値
    u_char		ucPadding;

    // 描画系
    DG_PRIM2*		prim;					// 水滴
    DG_PRIM2*		primDrip;				// 発生点のエフェクト
} Work;
typedef	void	( *BWEEPACTMODE )( Work* );

// 動作関数宣言
static void ActDrip( Work* work );
static void ActDrop( Work* work ); 


// 動作関数更新
static inline void SetActMode( Work* work, BWEEPACTMODE actmode)
{
    work->ActFunc = actmode;
}

// 滴りフェーズ
static void ActDrip( Work* work ) 
{
    int		i, j;
    DG_PRIM2*	prim 	   =  work->prim;
    DG_PRIM2*	primDrip   =  work->primDrip;
    int		nCurrClock =  work->prim->buffer_clock;
    int		nPrevClock = (work->prim->buffer_clock + 1) % 2 ;

    // 表示チェック
    SET_FLAG( prim->flag, DG_PRIM2_INVISIBLE);		// 水滴は非表示
    UNSET_FLAG( primDrip->flag, DG_PRIM2_INVISIBLE);	// 溜りは表示
    
    { //
	float fLentorRateOut = 0.f;
	float fLentorRateIn  = 0.f; 
	float fBaseAffectOut = 0.f;
	float fBaseAffectIn  = 0.f; 
	float fDripFall      = DRIP_FALL * work->fLentor; 
	FVECTOR* pvecPrim    = primDrip->pos[nCurrClock];
	FVECTOR* pvecPrev    = primDrip->pos[nPrevClock];
	
	for ( i = 0; i < N_PRIMS_DRIP; i++ ){
	    fLentorRateOut = 1.f - (float)i 	/ (float)N_PRIMS_DRIP * work->fLentor;
	    fLentorRateIn  = 1.f - (float)(i+1) / (float)N_PRIMS_DRIP * work->fLentor;

	    if ( i != N_PRIMS_DRIP - 1){
		fBaseAffectOut = (float)i 	  / (float)N_PRIMS_DRIP;
		fBaseAffectIn  = (float)(i+1) / (float)N_PRIMS_DRIP;
		fBaseAffectOut = (fBaseAffectOut * fBaseAffectOut) * (fLentorRateOut * fLentorRateOut);
		fBaseAffectIn  = (fBaseAffectIn * fBaseAffectIn) * (fLentorRateIn * fLentorRateIn);
	    }else{
		float fPrevAffIn;
		fPrevAffIn     = fBaseAffectIn;
		fBaseAffectOut = (float)i 	  / (float)N_PRIMS_DRIP;
		fBaseAffectIn  = (float)(i+1) / (float)N_PRIMS_DRIP;
		fBaseAffectOut = (fBaseAffectOut * fBaseAffectOut) * (fLentorRateOut * fLentorRateOut);
		fBaseAffectIn  = (fBaseAffectIn * fBaseAffectIn) * (fLentorRateIn * fLentorRateIn);
		fBaseAffectIn  = fBaseAffectIn * (1.f - work->fLentor) + fPrevAffIn * work->fLentor;
	    }
	    
	    for ( j = 0; j < N_VERTS_DRIP_CIR; j++ ){
		// 外周
		pvecPrim->vy = pvecPrev->vy - fBaseAffectOut * fDripFall;
		pvecPrim++;
		pvecPrev++;
		// 内周
		pvecPrim->vy = pvecPrev->vy - fBaseAffectIn * fDripFall;
		pvecPrim++;
		pvecPrev++;
	    }
	    // 末尾と先頭の接合頂点
	    // 外周
	    pvecPrim->vy = pvecPrev->vy - fBaseAffectOut * fDripFall;
	    pvecPrim++;
	    pvecPrev++;
	    // 内周
	    pvecPrim->vy = pvecPrev->vy - fBaseAffectIn * fDripFall;
	    pvecPrim++;
	    pvecPrev++;
	}
    }

    if ( work->nDripCntr-- == 0 ){
	FVECTOR* pvecPrim = primDrip->pos[nCurrClock];
	work->vecPos.vy = (pvecPrim + (N_PRIMS_DRIP-1) * N_VERTS_DRIP)->vy;
	work->fDropHRate = 1.f + 2.f * work->fLentor;
	
	// 滴落下処理へ
	SetActMode( work, ActDrop);
	// 表示チェック
	UNSET_FLAG( prim->flag, DG_PRIM2_INVISIBLE);		// 水滴表示
	UNSET_FLAG( primDrip->flag, DG_PRIM2_INVISIBLE);	// 溜り表示
    }
}
// 落下フェーズ
static void ActDrop( Work* work ) 
{
    int		i, j;
    DG_PRIM2*	prim 	   =  work->prim;
    DG_PRIM2*	primDrip   =  work->primDrip;
    int		nCurrClock =  work->prim->buffer_clock;
    int		nCurrDripClock =  work->primDrip->buffer_clock;
    int		nPrevDripClock = (work->primDrip->buffer_clock + 1) % 2 ;

    // 溜り
    if ( !(primDrip->flag & DG_PRIM2_INVISIBLE) ){ 
	FVECTOR* 	pvecPrim = primDrip->pos[nCurrDripClock];
	FVECTOR* 	pvecPrev = primDrip->pos[nPrevDripClock];
	DG_PRIM2_UVRGB*	uvrgbCurr = primDrip->uvrgb[nCurrDripClock];
	DG_PRIM2_UVRGB*	uvrgbPrev = primDrip->uvrgb[nPrevDripClock];
	float* 	 pfDefY   = work->fDefaultY;

	// 元に戻しつつフェードアウト
	for ( i = 0; i < N_PRIMS_DRIP * N_VERTS_DRIP; i++ ){
	    pvecPrim->vy = pvecPrev->vy * 0.5f + (*pfDefY) * 0.5f;
	    uvrgbCurr->a = work->ucDripAlpha;
	    pvecPrim++;
	    pvecPrev++;
	    uvrgbCurr++;
	    uvrgbPrev++;
	    pfDefY++;
	}

	// α減衰
	if ( work->ucDripAlpha > 0 ) {
	    work->ucDripAlpha--;
	}else{
	    UNSET_FLAG( primDrip->flag, DG_PRIM2_INVISIBLE);	// 溜り非表示
	} 
    }

    // 滴
    if ( !(prim->flag & DG_PRIM2_INVISIBLE) ){
	FVECTOR* 		pvecPrim;
	DG_PRIM2_UVRGBWH*	uvrgbwh;

	pvecPrim = prim->pos[ nCurrClock ];
	uvrgbwh  = prim->uvrgb[ nCurrClock ];
	for ( i = 0; i < N_PRIMS; i++){
	    for ( j = 0; j < N_VERTS; j++){
		_sceVu0CopyVector( pvecPrim, &work->vecPos);
		uvrgbwh->h = work->fDropSize * work->fDropHRate;
		pvecPrim++;
		uvrgbwh++;
	    }
	}
	work->fDropHRate = work->fDropHRate * 0.95f + 0.05f;

	// 高さチェック
	work->vecPos.vy -= GRAVITY;
	if ( work->vecPos.vy <= work->fKillY ){
	    work->vecPos.vy = work->fKillY;
	    // 飛抹
	    {
		int nRGBADec;
		FMATRIX mat;
		extern void* NewMAOParticle( int, FMATRIX*, int, int, int, int, int, float, 
					 float, float, float, float, int, int, int, float, int);
		{
		    SVECTOR svec;
		    GV_VecToRot( &work->vecFloor, &svec);
		    DG_SetPos2( &work->vecPos, &svec);
		    DG_GetPos( &mat);
		}
		// 効果音
		GM_SeSetMode( SD_A_DRIPPY13, &work->vecPos, GM_SEMODE_NORMAL);

		nRGBADec = work->nRGBA & 0xffffff00;

		NewMAOParticle( MAO_GetRandom( 1323242, 4321432), &mat, 2, 16, COUNT_VMODE(15), COUNT_VMODE(25),
				512, STEP_VMODEF(16.f), STEP_VMODEF(24.f), 0.01f, 20.f, 30.f, 
				9776993, work->nRGBA, nRGBADec, 0.8f, work->nFlag);
	    }

	    {
		extern void* NewBloodFloor( FVECTOR* pvecInitPos, FVECTOR* pvecBoundary );	// 床平面
		NewBloodFloor( &work->vecPos, &work->vecBoundary);
	    }
	    SET_FLAG( prim->flag, DG_PRIM2_INVISIBLE);	// 水滴非表示
	}
    }

    // カウンタ更新
    work->nCntr--;
    if ( work->nCntr < 0 ){
	GV_DestroyActor( work ); // 破棄
    }
}
// 動作関数
static void Act( Work* work )
{
    // 所属グループ更新
    GM_GroupPrim2( work->prim, GM_CurrentStageMap);
    GM_GroupPrim2( work->primDrip, GM_CurrentStageMap);

    // バッファ切替え
     //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
    DG_SwitchBuffPrim2( work->primDrip );

    // 動作関数の呼出
    work->ActFunc( work);
}

// 破棄関数
static void Die( Work* work )
{
    work->prim 	   = MAO_FreePrim2( work->prim );
    work->primDrip = MAO_FreePrim2( work->primDrip );
}

static int InitPacket( Work* 		work, 		// 
		       DG_PRIM2*	prim,		// 
		       DG_TEX* 		tex,		// 
		       long64		tagAlpha,	// 
		       u_int		nRGBA,		// 
		       int		nRaise)
{
    u_short		r,g,b,a;
    int			i, j;
    FVECTOR*		pvecVert1;
    FVECTOR*		pvecVert2;
    DG_PRIM2_UVRGBWH*	uvrgbwh1;
    DG_PRIM2_UVRGBWH*	uvrgbwh2;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    // RGBA値
    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    pvecVert1 = prim->pos[0];
    pvecVert2 = prim->pos[1];
    uvrgbwh1  = prim->uvrgb[0];
    uvrgbwh2  = prim->uvrgb[1];
    for ( i = 0; i < N_PRIMS; i++){
	for ( j = 0; j < N_VERTS; j++){
	    _sceVu0CopyVector( pvecVert1, &work->vecPos);
	    _sceVu0CopyVector( pvecVert2, &work->vecPos);

	    uvrgbwh2->w  = uvrgbwh1->w  = work->fDropSize;
	    uvrgbwh2->h  = uvrgbwh1->h  = (int)(work->fDropSize * 1.5f);

	    uvrgbwh2->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );/* 左上 */
	    uvrgbwh2->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );/* 左上 */
	    uvrgbwh2->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );/* 右下 */
	    uvrgbwh2->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );/* 右下 */
	    uvrgbwh2->q0 = uvrgbwh1->q0 = 4096;
	    uvrgbwh2->q1 = uvrgbwh1->q1 = 4096;
	    uvrgbwh2->f0 = uvrgbwh1->f0 = 0x0fff;
	    uvrgbwh2->f1 = uvrgbwh1->f1 = 0x0fff;

	    uvrgbwh2->r  = uvrgbwh1->r  = r;
	    uvrgbwh2->g  = uvrgbwh1->g  = g;
	    uvrgbwh2->b  = uvrgbwh1->b  = b;
	    uvrgbwh2->a  = uvrgbwh1->a  = a;

	    pvecVert1++;
	    pvecVert2++;
	    uvrgbwh1++;
	    uvrgbwh2++;
	}
    }
    return 1;
}

// 溜りプリミティブ初期化
static int InitPacketDrip( Work* 	work, 		// 
			   DG_PRIM2*	prim,		// 
			   DG_TEX* 	tex,		// 
			   long64	tagAlpha,	// 
			   u_int	nRGBA,		// 
			   int		nRaise)
{
    u_char 	r,g,b,a;
    int 	i,j;
    float	fRadOut;
    float	fRadIn;

    FVECTOR* pvecVert;
    FVECTOR* pvecVertCopy;
    FVECTOR* pvecVertTop1;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    // 最外周のドーナッツを作成
    pvecVert 	= SCR_POS;
    pvecVertTop1 = prim->pos[0];
    pvecVertCopy = prim->pos[0];
    fRadOut = work->fDripRad;
    fRadIn  = fRadOut - work->fDripRad / (float)N_PRIMS_DRIP;
    for ( j = 0; j < N_VERTS_DRIP_CIR; j++ ){
	int nAng = j * 4096 / N_VERTS_DRIP_CIR;
	float fX = _TS_Sin( nAng);
	float fY = _TS_Sin( nAng + 1024);

	pvecVert->vx = fX * fRadOut;
	pvecVert->vy = 0.f;
	pvecVert->vz = fY * fRadOut;
	pvecVert->vw = 1.f;
	pvecVert++;

	pvecVert->vx = fX * fRadIn;
	pvecVert->vy = 0.f;
	pvecVert->vz = fY * fRadIn;
	pvecVert->vw = 1.f;
	pvecVert++;
    }
    pvecVert->vx = 0.f;
    pvecVert->vy = 0.f;
    pvecVert->vz = fRadOut;
    pvecVert->vw = 1.f;
    pvecVert++;

    pvecVert->vx = 0.f;
    pvecVert->vy = 0.f;
    pvecVert->vz = fRadIn;
    pvecVert->vw = 1.f;
    pvecVert++;

    // スクラッチパッド->メインメモリ
    Mao_CopyScrToMem( pvecVertTop1, SCR_POS, sizeof(FVECTOR), N_VERTS_DRIP);
    pvecVertTop1 += N_VERTS_DRIP;

    for ( i = 1; i < N_PRIMS_DRIP; i++ ){
	fRadOut 	= fRadIn;
	fRadIn  	= fRadOut - work->fDripRad / (float)N_PRIMS_DRIP;
	pvecVert 	= SCR_POS;
	for ( j = 0; j < N_VERTS_DRIP_CIR; j++ ){
	    int nAng = j * 4096 / N_VERTS_DRIP_CIR;
	    float fX = _TS_Sin( nAng);
	    float fY = _TS_Sin( nAng + 1024);
	    pvecVertCopy++;

	    _sceVu0CopyVector( pvecVert, pvecVertCopy);
	    pvecVert++;
	    pvecVertCopy++;

	    pvecVert->vx = fX * fRadIn;
	    pvecVert->vy = 0.f;
	    pvecVert->vz = fY * fRadIn;
	    pvecVert->vw = 1.f;
	    pvecVert++;
	}

	pvecVert->vx = 0.f;
	pvecVert->vy = 0.f;
	pvecVert->vz = fRadOut;
	pvecVert->vw = 1.f;
	pvecVert++;

	pvecVert->vx = 0.f;
	pvecVert->vy = 0.f;
	pvecVert->vz = fRadIn;
	pvecVert->vw = 1.f;
	pvecVert++;
	pvecVertCopy += 2;

	Mao_CopyScrToMem( pvecVertTop1, SCR_POS, sizeof(FVECTOR), N_VERTS_DRIP);
	pvecVertTop1 += N_VERTS_DRIP;
    }

    DG_SetPos( &work->matOrg);
    DG_PutVector( prim->pos[0], prim->pos[0], N_PRIMS_DRIP * N_VERTS_DRIP);
    memcpy( prim->pos[1], prim->pos[0], sizeof(FVECTOR) * N_PRIMS_DRIP * N_VERTS_DRIP);

    {
	float* 	 pfDefY = work->fDefaultY;
	FVECTOR* pvec	= prim->pos[0];
	for ( i = 0; i < N_PRIMS_DRIP * N_VERTS_DRIP; i++ ){
	    (*pfDefY) = pvec->vy;
	    pvec++;
	    pfDefY++;
	}
    }
    {
	DG_PRIM2_UVRGB*	uvrgb1;
	DG_PRIM2_UVRGB*	uvrgb2;
	DG_PRIM2_UVRGB*	uvrgbTop;

	uvrgb1 = prim->uvrgb[0];
	uvrgb2 = prim->uvrgb[1];
	uvrgbTop = prim->uvrgb[0];
	for ( i = 0; i < N_PRIMS_DRIP; i++ ){
	    float fOutRate = 1.f - (float)i / (float)N_PRIMS_DRIP;
	    float fInRate  = fOutRate - 1.f / (float)N_PRIMS_DRIP;
	    for ( j = 0; j < N_VERTS_DRIP_CIR; j++ ){
		int nAng = j * 4096 / N_VERTS_DRIP_CIR;
		float fU = _TS_Sin( nAng) 	 * 0.5f * fOutRate + 0.5f;
		float fV = _TS_Sin( nAng + 1024) * 0.5f * fOutRate + 0.5f;
		uvrgb2->u = uvrgb1->u = FTOI12( fU * tex->u_scale + tex->u_offset ); //
		uvrgb2->v = uvrgb1->v = FTOI12( fV * tex->v_scale + tex->v_offset ); //
		uvrgb2->q = uvrgb1->q = 4096;
		uvrgb2->f = uvrgb1->f = 0x0fff;
		uvrgb2->r = uvrgb1->r = r;
		uvrgb2->g = uvrgb1->g = g;
		uvrgb2->b = uvrgb1->b = b;
		uvrgb2->a = uvrgb1->a = a;
		uvrgb1++;
		uvrgb2++;

		fU = _TS_Sin( nAng) 	   * 0.5f * fInRate + 0.5f;
		fV = _TS_Sin( nAng + 1024) * 0.5f * fInRate + 0.5f;
		uvrgb2->u = uvrgb1->u = FTOI12( fU * tex->u_scale + tex->u_offset ); //
		uvrgb2->v = uvrgb1->v = FTOI12( fV * tex->v_scale + tex->v_offset ); //
		uvrgb2->q = uvrgb1->q = 4096;
		uvrgb2->f = uvrgb1->f = 0x0fff;
		uvrgb2->r = uvrgb1->r = r;
		uvrgb2->g = uvrgb1->g = g;
		uvrgb2->b = uvrgb1->b = b;
		uvrgb2->a = uvrgb1->a = a;
		uvrgb1++;
		uvrgb2++;
	    }
	    uvrgb2->u = uvrgb1->u = uvrgbTop->u;
	    uvrgb2->v = uvrgb1->v = uvrgbTop->v;
	    uvrgb2->q = uvrgb1->q = uvrgbTop->q;
	    uvrgb2->f = uvrgb1->f = uvrgbTop->f;
	    uvrgb2->r = uvrgb1->r = uvrgbTop->r;
	    uvrgb2->g = uvrgb1->g = uvrgbTop->g;
	    uvrgb2->b = uvrgb1->b = uvrgbTop->b;
	    uvrgb2->a = uvrgb1->a = uvrgbTop->a;
	    uvrgb1++;
	    uvrgb2++;

	    uvrgb2->u = uvrgb1->u = (uvrgbTop + 1)->u;
	    uvrgb2->v = uvrgb1->v = (uvrgbTop + 1)->v;
	    uvrgb2->q = uvrgb1->q = (uvrgbTop + 1)->q;
	    uvrgb2->f = uvrgb1->f = (uvrgbTop + 1)->f;
	    uvrgb2->r = uvrgb1->r = (uvrgbTop + 1)->r;
	    uvrgb2->g = uvrgb1->g = (uvrgbTop + 1)->g;
	    uvrgb2->b = uvrgb1->b = (uvrgbTop + 1)->b;
	    uvrgb2->a = uvrgb1->a = (uvrgbTop + 1)->a;
	    uvrgb1++;
	    uvrgb2++;

	    uvrgbTop += N_VERTS_DRIP;
	}
    }

    return 1;
}
// リソース初期化
static int GetResources( Work*		work,
			 FMATRIX*	pmatOrg,		// 発生座標系	
			 int		nLife,			// 生存時間
			 int		nDripTime,		// 溜り時間
			 float		fFallSpd,		// 落下速度
			 float		fDropSize,		// 滴大きさ
			 float		fDripRad,		// 溜り半径
			 float		fLentor,		// 粘性
			 int		nRGBA,			// RGBA値
			 int		nRandSeed,		// ランダムシード
			 int		nFlag)			// フラグ
{
    long64	tagAlpha;
    DG_TEX*	tex;

    {	// 引数取得
	_sceVu0CopyMatrix( &work->matOrg, pmatOrg);
	_sceVu0CopyVector( &work->vecPos, (FVECTOR*)&work->matOrg.m[3][0]);
	work->nCntr	= nLife;
	work->nDripCntr	= nDripTime;
	work->fFallSpd  = fFallSpd;
	work->fDropSize = fDropSize;
	work->fDripRad  = fDripRad;
	work->fLentor	= fLentor;
//#ifdef JAPANESE_BP_IGNORE()
#if 1
	work->nRGBA	= (nRGBA & 0xffffff00) + ((nRGBA & 0x000000ff) / 2);
	work->ucDripAlpha = (u_char)(nRGBA & 0x000000ff);
#else
	work->nRGBA	= nRGBA;
	work->ucDripAlpha = (u_char)(nRGBA & 0x000000ff);
#endif
	work->nRandSeed = nRandSeed;
	work->nFlag	= nFlag;
    }

    { // 床チェック
	int 		floor_flag;
	HZX_FLR		flr[2];
	int		flr_atrs[2];
	float		flr_height[2];

	FVECTOR		vecChk;
	vecChk.vx = work->vecPos.vx;
	vecChk.vy = work->fKillY + 100.f; // 着地点＋αぐらい
	vecChk.vz = work->vecPos.vz;
	floor_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
					   &vecChk, HZX_CHK_FLOOR | HZX_CHK_RECOIL_TYPE, 0 );
	if ( floor_flag & 1){
	    extern void OK_FloorToBoundary( HZX_FLR *flr, FVECTOR *fv );

	    HZX_GetLevelHazard( flr, flr_atrs );
	    HZX_GetLevelHeight( flr_height );

	    OK_FloorToBoundary( flr, &work->vecBoundary); // 床平面取得
	    
	    // 法線設定
	    work->vecFloor.vx = flr->p1.h;
	    work->vecFloor.vy = flr->p3.h;
	    work->vecFloor.vz = flr->p2.h;
	    work->vecFloor.vw = 1.f;
	    _sceVu0Normalize( &work->vecFloor, &work->vecFloor);

//	    work->fKillY   = flr_height[0];
	}else{

	    work->vecFloor.vx = 0.f;
	    work->vecFloor.vy = 1.f;
	    work->vecFloor.vz = 0.f;
	    work->vecFloor.vw = 1.f;

//	    work->fKillY   = -200000.f;
	}
    }

    // 描画用ワーク初期化
    // 滴プリミティブ本体の作成
    work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
    if ( work->prim == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャ設定
    tex = DG_GetTexture( 9776993 ); // drop02_msk

    {
	// 拡張フラグを解析	
	if ( work->nFlag & EXFLAG_NORM_ALP)	  tagAlpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ); // 半透明
	else if ( work->nFlag & EXFLAG_ADD_ALP)   tagAlpha = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ); // 加算半透明
	else if ( work->nFlag & EXFLAG_SUB_ALP)   tagAlpha = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ); // 減算半透明
	else					  tagAlpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ); // 半透明
    }

    InitPacket( work, work->prim, tex, tagAlpha, work->nRGBA, 1000);

    // 溜りプリミティブ本体の作成
    work->primDrip = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_DRIP, N_VERTS_DRIP);
    if ( work->primDrip == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    tex = DG_GetTexture( 9776993 ); // drop02_msk
    InitPacketDrip( work, work->primDrip, tex, tagAlpha, work->nRGBA, -1000);

    // 動作関数の設定
    SetActMode( work, ActDrip);

    return 0;
}

// 滴り血を生成
void* NewBloodWeep( FMATRIX* 	pmatOrg,		// 発生座標系
		    int		nLife,			// 生存時間
		    int		nDripTime,		// 溜り時間
		    float	fKillY,			// 着地点
		    float	fFallSpd,		// 落下速度
		    float	fDropSize,		// 滴大きさ
		    float	fDripRad,		// 溜り半径
		    float	fLentor,		// 粘性
		    int		nRGBA,			// RGBA値
                    int		nRandSeed,		// ランダムシード
		    int		nFlag)			// フラグ
{
    Work*	work;

    OPERATOR();

    // BLOOD-OFF mode check
    if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );

	work->fKillY = fKillY;
	if ( GetResources( work, pmatOrg, nLife, nDripTime, fFallSpd, 	
			   fDropSize, fDripRad, fLentor, nRGBA, nRandSeed, nFlag) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}
