//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    hexagonal.c
    亀甲エフェクト Demo-Version [吉良さんのソースから切り出し]
    2001/07/31 Masafumi Okuta
    $Id: hexagonal.c,v 1.1.1.3 2002/11/19 11:47:51 Yoshizawa1 Exp $
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

#define	SET_FLAG( a,b )		(a) |= (b)
#define	UNSET_FLAG( a,b )	(a) &= ~(b)
#define	CLEAR_FLAG( a ) 	(a) = 0
#define	BODYWORLD(a,b) 		(a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVR			(SCRPAD_ADDR + 0x1000)

/* -------------------------------------------------------------------------
   デフォルト値
  ------------------------------------------------------------------------- */
#define N_VERTS  16   /* 頂点数                   */
#define N_PRIMS  128  /* プリミティブ数           */

#define DEF_MAX_TIME  300
#define DEF_MAX_ALPHA 16
#define DEF_HEX_SIZE  200      /* デフォルトの正六角形外接円の半径 */
#define DEF_MAX_POINT   6      /* デフォルトの発生点数             */
#define DEF_SPAN       15

#define DEF_COLOR_R   100
#define DEF_COLOR_G   255
#define DEF_COLOR_B   128

/* -------------------------------------------------------------------------
   
   ------------------------------------------------------------------------- */
#define MIN_LIFE  4   /* 最短寿命(最小頂点数) */
//#define MIN_LIFE  4   /* 最短寿命(最小頂点数) */

#define SQRT_3  1.7320508F      /* SQRT(3) の値 */
#define X_STEP  (SQRT_3 / 2.0F)
#define Y_STEP  (0.5F)
#define RAND(_m)    ((BP_PS2_rand() >> 16) % (_m))
#define AlphaFunc(alpha, t_cnt, max_time)  \
                ((alpha) *       \
		 ((max_time) - (t_cnt)) / (max_time) * \
		 ((max_time) - (t_cnt)) / (max_time) * \
		 ((max_time) - (t_cnt)) / (max_time))

#define NEW_LIMIT()  (20 + RAND(17))

enum{ // メッセージ
MESSAGE_ACTIVE,	// アクティブモード
MESSAGE_SLEEP,	// スリープモード
};

typedef struct _ALP_BUFFER{ // α値計算用構造体
    int   nAlpha;
    int   nCntr;
    int   nLife;
} ALP_BUFFER;

typedef struct HEXAGONAL_PATTERN {
    // 基本システム
    GV_ACT_EX       	actor;		// 
    int			name;		// 名前
    GV_MSG*		msg;				// 自分宛メッセージ	
    
    // 拡張システム
    FMATRIX*		pmatRefer;	// 追従する座標
    FVECTOR		vecPrev;	// １フレーム前
    int			nRGBA;		// RGBカラーと最大のα値
    float		fHexRad;	// 六角形の大きさ
    FVECTOR		vecCenter;	// 表示エリアの中心
    float		fRegionRad;	// 表示する半径
    float		fFloorY;	// 床の高さ	
    int			nMaxTime;	// 減衰所要時間
    int			nMaxPoint;	// １Hexからの発生数
    int             	nPrims;     	// プリミティブ数                     
    int			nVerts;		// 頂点数
    int             	nActNum;     	// 表示中の線の本数 
    int	   	        nUseBegin;   	// 使用済領域開始点 
    int			nSleep;		// スリープモード
    FVECTOR          	vecHexRel[6];  	// 六各形の中心から、各頂点までの相対ベクトル

    // 描画用
    ALP_BUFFER*		alpList;	// α値ワーク	
    DG_PRIM2*		prim;
} Work;


typedef struct _MOVE_INFO{
    int 	next_id;
    float 	fStepX;
    float 	fStepY;
} MOVE_INFO;

static MOVE_INFO HexStepTbl[6][3] = {
  /* 0 */ {{5, -X_STEP,  Y_STEP}, {1,  X_STEP,  Y_STEP}, {3, 0.0F, -1.0F}},
  /* 1 */ {{0, -X_STEP, -Y_STEP}, {2,  0.0F,    1.0F  }, {4, X_STEP, -Y_STEP}},
  /* 2 */ {{1,  0.0F,   -1.0F  }, {3, -X_STEP,  Y_STEP}, {5, X_STEP, Y_STEP}},
  /* 3 */ {{2,  X_STEP, -Y_STEP}, {4, -X_STEP, -Y_STEP}, {0, 0.0F,   1.0F}},
  /* 4 */ {{3,  X_STEP,  Y_STEP}, {5,  0.0F,   -1.0F  }, {1, -X_STEP, Y_STEP}},
  /* 5 */ {{4,  0.0F,    1.0F  }, {0,  X_STEP, -Y_STEP}, {2, -X_STEP, -Y_STEP}}
};

static void CheckMessage( Work* work )
{
    GV_MSG	*msg ;
    int n_msg, code ;

    n_msg = GV_ReceiveMessage( work->name, &work->msg ); // メッセージ取得
    msg = work->msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ];
	switch( code ) {
	case MESSAGE_ACTIVE:	// アクティブモード
	    work->nSleep = 0;
	    break;
	case MESSAGE_SLEEP:	// スリープモード
	    work->nSleep = 1;
	    break;
	default :
	    break;
	}
	msg++ ;
    }
}

// 指定された位置のHexを光らせる
static void BlightNexHex( Work* 	work)
{
    int			i, j;
    DG_PRIM2*		prim = work->prim;
    int 		nCenterX, nCenterZ;
    int			nPoints;
    int			nClock = prim->buffer_clock;
    float 		fTmp;
    FVECTOR 		vecCenter;
    FVECTOR 		vecAdd;
    FVECTOR*		pvecRef = (FVECTOR*)&work->pmatRefer->m[3][0];

    // 中心を求める
    nCenterZ = (int)(pvecRef->vz / ( work->fHexRad * 1.5f) + 0.5f);
    fTmp     = (float)( 1 - (nCenterZ & 1)) * 0.5f;
    nCenterX = (int)(pvecRef->vx / ( work->fHexRad * SQRT_3) + fTmp);

    // マップ上の座標に変換
    vecCenter.vx = work->fHexRad * SQRT_3 * ((float)(nCenterZ & 1) * 0.5f + (float)nCenterX);
    vecCenter.vy = work->fFloorY;
    vecCenter.vz = work->fHexRad * 1.5f * (float)nCenterZ;
    vecCenter.vw = 1.f;

    nPoints = MAO_Rand( work->nMaxPoint);
    vecAdd.vy = 0.f;
    vecAdd.vw = 1.f;

    //    if( MAO_Rand(DEF_SPAN) > 0) return;

    {
	int		nRoot;
	int		nLife;
	int		r;
	int		nIndex;
	u_char		ucAlpha;
	u_char		ucR 	   = (u_char)((work->nRGBA & 0xff000000) >> 24);
	u_char		ucG 	   = (u_char)((work->nRGBA & 0x00ff0000) >> 16);
	u_char		ucB 	   = (u_char)((work->nRGBA & 0x0000ff00) >> 8);
	u_char		ucAlphaMax =  (u_char)(work->nRGBA & 0x000000ff);
	FVECTOR		vecPoint;
	FVECTOR* 	pvecVert;
	DG_PRIM2_UVRGB*	uvrgb;

	pvecVert = prim->pos[nClock];
	uvrgb 	 = prim->uvrgb[nClock];
	for ( i = 0; i < nPoints; i++){
	    FVECTOR*		pvecScr  = SCR_POS;
	    DG_PRIM2_UVRGB* 	uvrgbScr = SCR_UVR;

	    if ( work->nActNum >= work->nPrims) break;

	    nRoot   = MAO_Rand(6);
	    ucAlpha = ucAlphaMax;
	    nLife   = MIN_LIFE + MAO_Rand( work->nVerts - MIN_LIFE);

	    _sceVu0AddVector( &vecPoint, &vecCenter, &work->vecHexRel[nRoot]);
	    _sceVu0CopyVector( pvecScr, &vecPoint);
	    uvrgb->r = ucR;
	    uvrgb->g = ucG;
	    uvrgb->b = ucB;
	    uvrgb->a = ucAlpha;
	    uvrgb->f = 0x0fff;

	    r = RAND(2) + 1;
	    for( j = 1; j < nLife; j++){
		vecAdd.vx = HexStepTbl[nRoot][r].fStepX * work->fHexRad;
		vecAdd.vy = 0.f;
		vecAdd.vz = HexStepTbl[nRoot][r].fStepY * work->fHexRad;
		nRoot      = HexStepTbl[nRoot][r].next_id;
		_sceVu0AddVector(&vecPoint, &vecPoint, &vecAdd);

#if 1
		/* リージョンチェックが必要な場合、中心点からの距離が
		   限界距離以上であれば、それ以上の描画を行わない。 */
		{
		    /* 中心点を原点とした座標を求める */
		    _sceVu0SubVector(&vecAdd, &vecPoint, &work->vecCenter);
	      
		    /* xz 平面上の距離を求める */
		    fTmp = DG_SQRT(vecAdd.vx * vecAdd.vx + vecAdd.vz * vecAdd.vz);
	      
		    /* 距離が work->region_r より大きければ、中断 */
		    if(fTmp > work->fRegionRad) break;
		}
#endif
		_sceVu0CopyVector( pvecScr+j, &vecPoint);
		(uvrgbScr+j)->r = ucR;
		(uvrgbScr+j)->g = ucG;
		(uvrgbScr+j)->b = ucB;
		(uvrgbScr+j)->a = ucAlpha - ucAlpha * j / (nLife - 1);
		(uvrgbScr+j)->f = 0x0fff;
#if 1
		if ( j % 2 == 0 ){	
		    r = 2; 
		}else{		
		    r = MAO_Rand(2);
		}
#else 
		r = MAO_Rand(2);
#endif
	    }
	    // 残りは頂点キックしない
	    while ( j < work->nVerts){
		(uvrgbScr+j)->f = 0x8fff;
		j++;
	    }
	    
	    // 未使用領域を求める
	    nIndex = ( work->nUseBegin + work->nActNum) % work->nPrims;
	    Mao_CopyScrToMem( pvecVert + work->nVerts * nIndex,  SCR_POS, sizeof(FVECTOR), work->nVerts);
	    Mao_CopyScrToMem( uvrgb + work->nVerts * nIndex,  SCR_UVR, sizeof(DG_PRIM2_UVRGB), work->nVerts);
	    
	    work->alpList[nIndex].nAlpha = ucAlpha;
	    work->alpList[nIndex].nCntr  = 0;
	    work->alpList[nIndex].nLife  = nLife;

	    work->nActNum++;
	}
    }
}

// 表示中のものを更新
static void UpdateActiveHex( Work* 	work)
{
    int			i, j;
    int			nPrevClock, nCurrClock;
    DG_PRIM2*		prim;
    ALP_BUFFER* 	pAlp;
    FVECTOR*		pvecPrev;		
    FVECTOR*		pvecCurr;		
    FVECTOR*		pvecScr;
    DG_PRIM2_UVRGB*	uvrgbPrev;
    DG_PRIM2_UVRGB*	uvrgbCurr;
    DG_PRIM2_UVRGB*	uvrgbScr;

    prim = work->prim;
    pAlp = work->alpList;

    nPrevClock = prim->buffer_clock;
    DG_SwitchBuffPrim2( prim);
    nCurrClock = prim->buffer_clock;

    pvecPrev 	= prim->pos[nPrevClock];
    pvecCurr 	= prim->pos[nCurrClock];
    pvecScr	= SCR_POS;
    uvrgbPrev	= prim->uvrgb[nPrevClock];
    uvrgbCurr	= prim->uvrgb[nCurrClock];
    uvrgbScr	= SCR_UVR;
    
    {
	int nEndCnt = 0;
	int nIndex;
	for ( i = 0; i < work->nActNum; i++){
	    nIndex = (work->nUseBegin + i) % work->nPrims;
	    pAlp[nIndex].nCntr += TIME_BASE;
	    if ( pAlp[nIndex].nCntr >= work->nMaxTime ){ // 表示時間終了
		nEndCnt++;
		continue;
	    }
	    // スクラッチパッドに転送
	    Mao_CopyMemToScr( pvecPrev + work->nVerts * nIndex, SCR_POS, sizeof(FVECTOR), work->nVerts);
	    Mao_CopyMemToScr( uvrgbPrev + work->nVerts * nIndex, SCR_UVR, sizeof(DG_PRIM2_UVRGB), work->nVerts);

	    // α値を更新
	    for ( j = 0; j < pAlp[nIndex].nLife; j++){
		u_char  ucAlpha;
		int	nTime;

		ucAlpha = pAlp[nIndex].nAlpha;
		nTime   = pAlp[nIndex].nCntr;
		ucAlpha = AlphaFunc( ucAlpha, nTime, work->nMaxTime);
		(uvrgbScr + j)->a = ucAlpha;
	    }
	    // メインメモリに転送
	    Mao_CopyScrToMem( pvecCurr + work->nVerts * nIndex, SCR_POS, sizeof(FVECTOR), work->nVerts);
	    Mao_CopyScrToMem( uvrgbCurr + work->nVerts * nIndex, SCR_UVR, sizeof(DG_PRIM2_UVRGB), work->nVerts);
	}
	// 残りを解放
	work->nUseBegin = ( work->nUseBegin + nEndCnt) % work->nPrims;
	work->nActNum -= nEndCnt;
    }
}

// 動作関数
static void Act( Work* work )
{
    // メッセージチェック
    CheckMessage( work);

    // 表示中のHexエフェクトの更新
    UpdateActiveHex( work);

    // 新規に追加
    if ( !work->nSleep ){
	BlightNexHex( work);
    }
}

// 破棄関数
static void Die( Work* work )
{
    work->prim 	   = MAO_FreePrim2( work->prim );
    if ( work->alpList != NULL ) GV_Free(work->alpList);
}

// 相対座標な六角形の頂点を作成
static void MakeRelativeHex(Work * work)
{
    int i;
    float xp, yp;

    xp = work->fHexRad * X_STEP;
    yp = work->fHexRad * Y_STEP;

    work->vecHexRel[0].vx = work->vecHexRel[3].vx = 0.0F;
    work->vecHexRel[0].vz = -work->fHexRad;
    work->vecHexRel[3].vz =  work->fHexRad;

    work->vecHexRel[1].vx = work->vecHexRel[2].vx =  xp;
    work->vecHexRel[4].vx = work->vecHexRel[5].vx = -xp;

    work->vecHexRel[1].vz = work->vecHexRel[5].vz = -yp;
    work->vecHexRel[2].vz = work->vecHexRel[4].vz =  yp;

    for(i = 0; i < 6; i++){
	work->vecHexRel[i].vy = 0.0F;
	work->vecHexRel[i].vw = 1.0F;
    }
}

// プリミティブ初期化
static int InitPacket( Work* work,
		       DG_PRIM2* prim)
{
    int			i;
    FVECTOR*		pvecScr  = SCR_POS;
    DG_PRIM2_UVRGB*	uvrgbScr = SCR_UVR;
    FVECTOR*		pvec1    = prim->pos[0];
    FVECTOR*		pvec2    = prim->pos[1];
    DG_PRIM2_UVRGB*	uvrgb1 	 = prim->uvrgb[0];
    DG_PRIM2_UVRGB*	uvrgb2 	 = prim->uvrgb[1];

    // スクラッチパッドをクリア
    for ( i = 0; i < work->nVerts; i++){
	_sceVu0CopyVector( (pvecScr+i), &DG_ZeroVector);
    }
    GV_ZeroMemory( uvrgbScr, sizeof(DG_PRIM2_UVRGB) * work->nVerts );

    // １プリミティブ分のデータを作成
    for ( i = 0; i < work->nVerts; i++) {
	(uvrgbScr+i)->f = 0x8fff;	// 頂点キックなし
    }

    // スクラッチパッドからプリミティブ分コピー
    for ( i = 0; i < work->nPrims; i++) {
	Mao_CopyScrToMem( pvec1 + i * work->nVerts, SCR_POS, sizeof(FVECTOR), work->nVerts );
	Mao_CopyScrToMem( pvec2 + i * work->nVerts, SCR_POS, sizeof(FVECTOR), work->nVerts );
	Mao_CopyScrToMem( uvrgb1 + i * work->nVerts, SCR_UVR, sizeof(DG_PRIM2_UVRGB), work->nVerts );
	Mao_CopyScrToMem( uvrgb2 + i * work->nVerts, SCR_UVR, sizeof(DG_PRIM2_UVRGB), work->nVerts );
    }

    return 0;
}

// リソースを初期化
static int GetResources( Work*		work,
			 int 		name, 
			 FMATRIX* 	pmatRefer,
			 FVECTOR* 	pvecCenter,
			 int		nRGBA,
			 float		fHexRad,
			 float		fRegionRad,
			 float		fFloorY)
{
    work->name = name;

    { // 拡張システム初期化
	work->pmatRefer 	= pmatRefer;
	_sceVu0CopyVector( &work->vecPrev, (FVECTOR*)&work->pmatRefer->m[3][0]);
	work->nRGBA 		= nRGBA;
	work->fHexRad 		= fHexRad;
	work->fRegionRad 	= fRegionRad;
	_sceVu0CopyVector( &work->vecCenter, pvecCenter);
	work->fFloorY		= fFloorY;

	work->nMaxTime		= DEF_MAX_TIME;
	work->nMaxPoint		= DEF_MAX_POINT;

	work->nPrims		= N_PRIMS;
	work->nVerts		= N_VERTS;

	work->nActNum		= 0;
	work->nUseBegin		= 0;

	work->nSleep		= 0;

	// 相対座標な六角形の頂点を作成
	MakeRelativeHex( work);
    }

    { // 描画ワーク初期化
	work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA, work->nPrims, work->nVerts);
	DG_SetPrim2Alpha( work->prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)); // 加算

	// α値データワーク用のメモリを確保
	if ( (work->alpList = (ALP_BUFFER*)GV_Malloc( sizeof(ALP_BUFFER) * work->nPrims) ) == NULL){
	    MAO_PRINTF("Cannot Alloc Alpha BUffer!!");
	    return -1;
	}
	GV_ZeroMemory(work->alpList, sizeof(ALP_BUFFER) * work->nPrims );

	// プリミティブ初期化
	InitPacket( work, work->prim);
    }

    return 0;
}

// 亀甲エフェクト Demo-Version
void* NewHexagonalPattern( int 		name, 
			   FMATRIX* 	pmatRefer,
			   FVECTOR* 	pvecCenter,
			   int		nRGBA,
			   float	fHexRad,
			   float	fRegionRad,
			   float	fFloorY)
{
    Work * work;

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;

    if ( work != NULL ) {
	GV_SetActor(&(work->actor), Act, Die);
	GV_ActorEX(&(work->actor));
	if ( GetResources(work, name, pmatRefer, pvecCenter, nRGBA, fHexRad, fRegionRad, fFloorY) ){
	    GV_DestroyActor(work);
	    return NULL;
	}
    }

    return (void*)work;
}

