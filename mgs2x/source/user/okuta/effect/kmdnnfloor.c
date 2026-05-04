//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    kmdnnfloor.c
    かもめの糞:床飛抹
    2001/07/04 Masafumi Okuta
    $Id: kmdnnfloor.c,v 1.1.1.3 2002/11/19 11:47:51 Yoshizawa1 Exp $
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

inline int BP_AdjustTick(int);
inline int BP_AdjustTick2(int);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))

// RGBA設定
#define COL_R		(139)//(234)
#define COL_G		(150)//(245)
#define COL_B		(115)//(165)
#define ALPHA		(128)
#define COL_R2		(139)//(234)
#define COL_G2		(150)//(245)
#define COL_B2		(115)//(165)
#define ALPHA2		(128)
#define COL_R3		(139)//(224)
#define COL_G3		(150)//(235)
#define COL_B3		(115)//(165)
#define ALPHA3		(128)

// プリミティブ,頂点数
#define N_PRIMS		(1)
#define N_VERTS		(16 + 2)
#define N_CIRCLE_DIV	(8)

// スクラッチパッドアドレス
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)


// テクスチャstrcode
#define TEX_DROP_01_ALP  	(7840500) 	// "kmo_droppings01_alp"
#define TEX_DROP_02_ALP  	(8889076) 	// "kmo_droppings02_alp"
#define TEX_DROP_03_ALP  	(9937652) 	// "kmo_droppings03_alp"
#define TEX_DROP_04_ALP  	(10986228) 	// "kmo_droppings04_alp"

// テクスチャstrcode
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

#define SIZE_BASE	(60.f)
#define SIZE_ADD	(20)

#define LIFE_MAX	(COUNT_VMODE(600))	// 寿命
#define SPR_TIME	(COUNT_VMODE(20))	// 拡散時間

#define FLOOR_OFFSET	(4.f)			// 優先負け防止用Ｙ値オフセット

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
    int		nAlphaSpr;			// α
    int		nSprCntr;			// 拡散時間

    // 描画用
    FVECTOR	vecVert[ N_PRIMS * N_VERTS ];	// 頂点データ
    DG_PRIM2*	prim;		
    FVECTOR	vecVertSpr[ N_PRIMS * N_VERTS ];	// 頂点データ
    DG_PRIM2*	primSpr;		
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

void UpdatePacket( DG_PRIM2* 	prim, 		// 
			  FVECTOR* 	pvecMove,	// 
			  FVECTOR* 	pvecBound,	// 
			  SVECTOR* 	psvecRot )
{
   int			i, k;

   FVECTOR*		pvecPos;
   FVECTOR*		pvecPre;
   DG_PRIM2_UVRGB*	uvrgb;
   DG_PRIM2_UVRGB*	uvrgbPre;

   pvecPos 	= prim->pos[prim->buffer_clock];
   pvecPre 	= prim->pos[1 - prim->buffer_clock];
   uvrgb   	= prim->uvrgb[prim->buffer_clock]; 
   uvrgbPre  	= prim->uvrgb[1 - prim->buffer_clock];

   for ( i = 0 ; i < N_PRIMS ; i++ )
   {
      for ( k = 0 ; k < N_VERTS ; k++ )
      {
         FVECTOR vec;
         if ( k >= ( N_VERTS / 2) && k < ( N_VERTS * 3 / 4) )
         {
            _sceVu0CopyVector(&vec, pvecMove);
            uvrgb->a = uvrgbPre->a;
         }
         else
         {
            _sceVu0ScaleVector(&vec, pvecMove, 0.1f);
            if ( uvrgb->a > 30 ) uvrgb->a = uvrgbPre->a - 1;
         }
         _sceVu0AddVector( pvecPos, pvecPre, &vec);

         // 床範囲チェック
         if ( pvecPos->vx < pvecBound->vx ) 	    pvecPos->vx = pvecBound->vx;
         else if ( pvecPos->vx > pvecBound->vz ) pvecPos->vx = pvecBound->vz;
         if ( pvecPos->vz < pvecBound->vy ) 	    pvecPos->vz = pvecBound->vy;
         else if ( pvecPos->vz > pvecBound->vw ) pvecPos->vz = pvecBound->vw;

         pvecPre++;
         pvecPos++;
         uvrgb++;
         uvrgbPre++;
      }
   }
}

static void UpdatePacket2( DG_PRIM2 *prim, FVECTOR *pvecBound )
{
   int			i, k;
   FVECTOR*		pvecPos;

   pvecPos 	= prim->pos[prim->buffer_clock];
   for ( i = 0 ; i < N_PRIMS ; i++ )
   {
      for ( k = 0 ; k < N_VERTS ; k++ )
      {
         // 床範囲チェック
         if ( pvecPos->vx < pvecBound->vx ) 	    pvecPos->vx = pvecBound->vx;
         else if ( pvecPos->vx > pvecBound->vz ) pvecPos->vx = pvecBound->vz;
         if ( pvecPos->vz < pvecBound->vy ) 	    pvecPos->vz = pvecBound->vy;
         else if ( pvecPos->vz > pvecBound->vw ) pvecPos->vz = pvecBound->vw;

         pvecPos++;
      }
   }
}

static void UpdateAlpha( DG_PRIM2 *prim, int nAlpha)
{
   int			i, k;
   DG_PRIM2_UVRGB*	uvrgb;

   uvrgb	= prim->uvrgb[prim->buffer_clock]; 

   for ( i = 0 ; i < N_PRIMS ; i++ )
   {
      for ( k = 0 ; k < N_VERTS ; k++ )
      {
         // αチェック
         uvrgb->a = nAlpha;
         uvrgb++;
      }
   }
}

// 動作関数
static void Act( Work* work )
{
   work->prim->flag &= ~DG_PRIM2_INVISIBLE;
   work->primSpr->flag &= ~DG_PRIM2_INVISIBLE;

   // バッファ切替え
   DG_SwitchBuffPrim2( work->prim );
   DG_SwitchBuffPrim2( work->primSpr );


   work->nLife--;

   if ( work->nLife < work->nAlpha )  work->nAlpha  = work->nLife;
   if ( work->nLife < work->nAlphaSpr ) work->nAlphaSpr = work->nLife;

   if ( work->nLife <= 0 )
   {
      GV_DestroyActor( work );
   }
   else
   {
      if ( work->nSprCntr < SPR_TIME)
      {
         // 着地して
         work->nSprCntr++;
         _sceVu0AddVector( &work->vecCenter, &work->vecCenter, &work->vecMove);
         _sceVu0ScaleVector( &work->vecMove, &work->vecMove, 0.9f);

         UpdatePacket( work->prim, &work->vecMove, &work->vecBoundary, &work->vecRot );
         UpdatePacket2( work->primSpr, &work->vecBoundary );
      }

      UpdateAlpha( work->prim, work->nAlpha);
      UpdateAlpha( work->primSpr, work->nAlphaSpr);
   }
}

// 破棄関数
static void Die( Work* work )
{
    work->prim  = MAO_FreePrim2( work->prim );
    work->primSpr = MAO_FreePrim2( work->primSpr );
}

// パケット初期化
static int InitPacket( DG_PRIM2* 	prim, 		// 
		       DG_TEX* 		tex, 		// 
		       long64		tagAlpha,	// 
		       int		nRGBA,		// 
		       FVECTOR* 	pvecCenter, 	// 
		       FVECTOR* 	pvecVertTop,	// 
		       SVECTOR* 	psvecRot, 	// 
		       int		nRaise,		//
		       float 		size, 		// 
		       float 		speed )		// 
{
    u_char		r,g,b,a;
    int			i, k;
    float		fSize;
    float		angle;
    FVECTOR*		pvecPos;
    FVECTOR*		pvecVert;
    DG_PRIM2_UVRGB*	uvrgb;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    pvecPos 	= SCR_POS;		// 
    uvrgb	= SCR_UVS;		// 
    pvecVert	= pvecVertTop;		// 

    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    {
	extern float _RsinF( int d );
	angle = _RsinF(psvecRot->vz);
    }
    for ( i = 0 ; i < N_PRIMS ; i++ ){
	fSize = size;
	{
	    extern float _RsinF( int d );
	    angle = _RsinF( ((psvecRot->vz + i * MAO_GetRandom(128, 512)) & 4095));
	}
	for ( k = 0 ; k < N_VERTS ; k++ ){
	    extern float _TS_Sin( int s );
	    int 	nDir;
	    float 	fCos,fSin;
	    nDir = ((int)(k / 2) % N_CIRCLE_DIV) * 4096 / N_CIRCLE_DIV;
	    fCos = _TS_Sin( nDir  );
	    fSin = _TS_Sin( nDir + 1024);
	    // 座標設定
	    if ( k % 2 == 0){
		float fAddRad;
		fAddRad = (float)MAO_Rand(SIZE_ADD);
		pvecPos->vx = (fSize + fAddRad) * fCos;
		pvecPos->vy = (fSize + fAddRad) * fSin;
		pvecPos->vz = 0.f;
		pvecPos->vw = 1.f;
		uvrgb->u = FTOI12( ((fCos * 0.5f) + 0.5f) * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( ((fSin * 0.5f) + 0.5f) * tex->v_scale + tex->v_offset );
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
	    pvecVert++;
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
// パケット初期化
static int InitPacket2( DG_PRIM2* 	prim, 		// 
		       DG_TEX* 		tex, 		// 
		       long64		tagAlpha,	// 
		       int		nRGBA,		// 
		       FVECTOR* 	pvecCenter, 	// 
		       FVECTOR* 	pvecVertTop,	// 
		       SVECTOR* 	psvecRot, 	// 
		       int		nRaise,		//
		       float 		size, 		// 
		       float 		speed )		// 
{
    u_char		r,g,b,a;
    int			i, k;
    float		fSize;
    float		angle;
    FVECTOR*		pvecPos;
    FVECTOR*		pvecVert;
    DG_PRIM2_UVRGB*	uvrgb;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    pvecPos 	= SCR_POS;		// 
    uvrgb	= SCR_UVS;		// 
    pvecVert	= pvecVertTop;		// 

    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    for ( i = 0 ; i < N_PRIMS ; i++ ){
	fSize = size;
	{
	    extern float _RsinF( int d );
	    angle = _RsinF( ((psvecRot->vz + i * MAO_GetRandom(128, 512)) & 4095));
	}
	for ( k = 0 ; k < N_VERTS ; k++ ){
	    extern float _TS_Sin( int s );
	    int 	nDir;
	    float 	fCos,fSin;
	    nDir = ((int)(k / 2) % N_CIRCLE_DIV) * 4096 / N_CIRCLE_DIV;
	    fCos = _TS_Sin( nDir  );
	    fSin = _TS_Sin( nDir + 1024);
	    // 座標設定
	    if ( k % 2 == 0){
		float fAddRad;
		fAddRad = (float)MAO_Rand(SIZE_ADD * 3);
		pvecPos->vx = (fSize + fAddRad) * fCos;
		pvecPos->vy = (fSize + fAddRad) * fSin;
		pvecPos->vz = 0.f;
		pvecPos->vw = 1.f;
		uvrgb->u = FTOI12( ((fCos * 0.5f) + 0.5f) * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( ((fSin * 0.5f) + 0.5f) * tex->v_scale + tex->v_offset );
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
	    pvecVert++;
	    uvrgb++;
	}
    }
    // 回転をかける
    {
	SVECTOR vecRot;
	vecRot = (*psvecRot);
	vecRot.vz = MAO_Rand(4096);
	DG_SetPos2( pvecCenter, &vecRot);
	DG_PutVector( SCR_POS, SCR_POS, N_PRIMS*N_VERTS );
    }

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

	    work->vecCenter.vy = flr_height[0] + FLOOR_OFFSET;
	}
    }

    work->fSize    = SIZE_BASE;
    work->fSizeAdd = (float)MAO_GetRandom( -10, 40);
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
    tex = DG_GetTexture( TEX_DROP_02_ALP );
    // パケットの初期化
    InitPacket( work->prim, tex, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ), MAO_SetRGBA( COL_R, COL_G, COL_B, ALPHA),
		&work->vecCenter, work->vecVert,  &work->vecRot, RAISE, work->fSize, 1.f);
    work->prim->flag |= DG_PRIM2_INVISIBLE;

    //------------------ 
    // プリミティブ減算の作成
    work->primSpr = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
    work->nAlphaSpr = ALPHA3;
    if ( work->primSpr == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャの設定
    tex = DG_GetTexture( TEX_DROP_03_ALP );

    // パケットの初期化
    InitPacket2( work->primSpr, tex, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ), MAO_SetRGBA( COL_R3, COL_G3, COL_B3, ALPHA3),
		&work->vecCenter, work->vecVertSpr, &work->vecRot, RAISE+10000, work->fSize*1.8f, 1.f);
    work->primSpr->flag |= DG_PRIM2_INVISIBLE;

    // 効果音
    GM_SeSetMode( SD_A_FUNOTI01 + BP_PS2_rand() % 2, &work->vecCenter, GM_SEMODE_NORMAL);

    return 0;
}

// カモメのフン地面付着
void* NewKamomeDannaFloor( FVECTOR* pvecInitPos, 	// 発生位置
			   FVECTOR* pvecBoundary )	// 床平面
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
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
