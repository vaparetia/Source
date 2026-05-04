//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    forttear.c
    フォーチュン涙 : ピチョン君(waterdrop.c)から抜粋して改良
    2001/07/30 Masafumi Okuta
    $Id: forttear.c,v 1.1.1.3 2002/11/19 11:47:50 Yoshizawa1 Exp $
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

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))

#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define N_PRIMS		(1)
#define N_VERTS		(1)

#define SIZE		(40)
#define GRAVITY		(9.8f * 10.f)
#define LIFE		(COUNT_VMODE(1200)) 

#define WDP_CHK_HZX  (HZX_CHK_RECOIL_TYPE | HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR)

typedef struct _WATERDROP {
    // 基本システム
    GV_ACT_EX		actor;
    
    // 拡張管理
    int			nCntr;					// カウンタ		
    float		fKillY;					// 消す高さ
    float		fGravity;				// 落下速度
    float		fSize;					// サイズ
    int			nRGBAAdd;				// 加算プリミティブのRGBA値
    int			nRGBASub;				// 減算プリミティブのRGBA値
    int			nRandSeed;				// ランダムシード
    FVECTOR		vecPos;					// 位置
    FVECTOR		vecNorm;				// 着地法線
    
    int			nSpreadNum;    
    int			nSpreadLifeMin;	
    int			nSpreadLifeMax;	
    int			nSpreadAng;	
    float		fSpreadSpdMin;
    float		fSpreadSpdMax;
    float		fSpreadGravity;
    float		fSpreadSizeMin;
    float		fSpreadSizeMax;
    int      		nSpreadTexcode;
    int      		nSpreadRGBA;

    // 描画系
    DG_PRIM2*		primAdd;				// 水滴加算
    DG_PRIM2*		primSub;				// 水滴減算
} Work;

// 動作関数
static void Act( Work* work )
{
    int i, j;
    FVECTOR* pvecPrimAdd;
    FVECTOR* pvecPrimSub;

    pvecPrimAdd = work->primAdd->pos[ work->primAdd->buffer_clock];
    pvecPrimSub = work->primSub->pos[ work->primSub->buffer_clock];
    for ( i = 0; i < N_PRIMS; i++){
	for ( j = 0; j < N_VERTS; j++){
	    _sceVu0CopyVector( pvecPrimAdd, &work->vecPos);
	    _sceVu0CopyVector( pvecPrimSub, &work->vecPos);
	}
    }

    // 高さチェック
    work->vecPos.vy -= work->fGravity;
    if ( work->vecPos.vy <= work->fKillY ){
	work->vecPos.vy = work->fKillY;
	// 飛抹
	{
	    int nDecRGBA;
	    FMATRIX mat;
	    extern void* NewMAOParticle( int, FMATRIX*, int, int, int, int, int, float, 
					 float, float, float, float, int, int, int, float, int);


	    {
		SVECTOR svec;
		GV_VecToRot( &work->vecNorm, &svec);
		DG_SetPos2( &work->vecPos, &svec);
		DG_GetPos( &mat);
	    }
	    // 効果音
	    GM_SeSetMode( SD_A_DRIPPY13, &work->vecPos, GM_SEMODE_NORMAL);

	    nDecRGBA = work->nRGBAAdd & 0xffffff00;
	    NewMAOParticle( work->nRandSeed, &mat, 1, work->nSpreadNum, 
			    work->nSpreadLifeMin, work->nSpreadLifeMax,
			    work->nSpreadAng, work->fSpreadSpdMin, work->fSpreadSpdMax, 
			    work->fSpreadGravity, work->fSpreadSizeMin, work->fSpreadSizeMax, 
			    work->nSpreadTexcode, work->nSpreadRGBA, nDecRGBA, 0.8f, 2);
	}

	GV_DestroyActor( work );
    }

    // カウンタ更新
    work->nCntr++;
    if ( work->nCntr >= LIFE ){ // 保険
	GV_DestroyActor( work );
    }
}

// 破棄関数
static void Die( Work* work )
{
    work->primAdd = MAO_FreePrim2( work->primAdd );
    work->primSub = MAO_FreePrim2( work->primSub );
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

	    uvrgbwh2->w  = uvrgbwh1->w  = (int)work->fSize;
	    uvrgbwh2->h  = uvrgbwh1->h  = (int)work->fSize;

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

// リソース初期化
static int GetResources( Work*		work,
			 FVECTOR*	pvecPos,
			 FVECTOR*	pvecNorm,
			 float		fSpreadY,
			 float		fGravity,	
			 float		fSize,	
			 int      	nTexcode,
			 int      	nRGBAAdd,
			 int      	nRGBASub,
			 int		nSpreadNum,	
			 int		nSpreadLifeMin,	
			 int		nSpreadLifeMax,	
			 int		nSpreadAng,	
			 float		fSpreadSpdMin,
			 float		fSpreadSpdMax,
			 float		fSpreadGravity,
			 float		fSpreadSizeMin,
			 float		fSpreadSizeMax,
			 int      	nSpreadTexcode,
			 int      	nSpreadRGBA,
			 int      	nRandSeed)
{
    DG_TEX*	tex;

    // 引数から取得
    _sceVu0CopyVector( &work->vecPos, pvecPos);
    _sceVu0Normalize( &work->vecNorm, pvecNorm);
    work->fKillY = fSpreadY;
    work->fGravity = fGravity;
    work->fSize  = fSize;
    work->nRandSeed = nRandSeed;
    work->nRGBAAdd = nRGBAAdd;
    work->nRGBASub = nRGBASub;

    // 描画用ワーク初期化
    // 加算プリミティブ本体の作成
    work->primAdd = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
    if ( work->primAdd == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // 減算プリミティブ本体の作成
    work->primSub = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
    if ( work->primSub == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    
    // テクスチャ設定
    tex = DG_GetTexture( nTexcode );
//    tex = DG_GetTexture( 4642619 ); // drop02_msk

    InitPacket( work, work->primAdd, tex, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ), nRGBAAdd, 1000);
    InitPacket( work, work->primSub, tex, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ), nRGBASub, -1000);
    work->primSub->flag |= DG_PRIM_AS_CUSTOMWORLD;
    work->primSub->as_world.m[3][0] = 5.f;

    work->nSpreadNum 	 = nSpreadNum;
    work->nSpreadLifeMin = nSpreadLifeMin;
    work->nSpreadLifeMax = nSpreadLifeMax;
    work->nSpreadAng	 = nSpreadAng;
    work->fSpreadSpdMin  = fSpreadSpdMin;
    work->fSpreadSpdMax  = fSpreadSpdMax;
    work->fSpreadGravity = fSpreadGravity;
    work->fSpreadSizeMin = fSpreadSizeMax;
    work->nSpreadTexcode = nSpreadTexcode;
    work->nSpreadRGBA    = nSpreadRGBA;

    return 0;
}

// 涙を生成
void* NewFortuneTear( FVECTOR* 	pvecPos, 
		      FVECTOR* 	pvecNorm, 
		      float    	fSpreadY,
		      float	fGravity,	
		      float	fSize,	
		      int      	nTexcode,
		      int      	nRGBAAdd,
		      int      	nRGBASub,
		      int	nSpreadNum,	
		      int	nSpreadLifeMin,	
		      int	nSpreadLifeMax,	
		      int	nSpreadAng,	
		      float	fSpreadSpdMin,
		      float	fSpreadSpdMax,
		      float	fSpreadGravity,
		      float	fSpreadSizeMin,
		      float	fSpreadSizeMax,
		      int      	nSpreadTexcode,
		      int      	nSpreadRGBA,
		      int      	nRandSeed)	
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, pvecPos, pvecNorm, fSpreadY, fGravity, fSize, 
			   nTexcode, nRGBAAdd, nRGBASub, 
			   nSpreadNum, nSpreadLifeMin, nSpreadLifeMax, nSpreadAng,	
			   fSpreadSpdMin, fSpreadSpdMax, fSpreadGravity, fSpreadSizeMin,
			   fSpreadSizeMax, nSpreadTexcode, nSpreadRGBA, nRandSeed ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}
