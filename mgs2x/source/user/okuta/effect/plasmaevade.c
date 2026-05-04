//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    plasmaevade.c
    プラズマ回避エフェクト
    2001/08/03 Masafumi Okuta
    $Id: plasmaevade.c,v 1.1.1.3 2002/11/19 11:47:53 Yoshizawa1 Exp $
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

#define N_PRIMS		(256)		// プリミティブ数
#define N_VERTS		(32)		// 頂点数

// スクラッチパッド
#define	SCR_POS	(SCRPAD_ADDR)		
#define	SCR_UVR	(SCRPAD_ADDR + 0x800)

#define NODE_SPD_MIN	(12)	// １フレームで描画状態にするスプライトの数:最小
#define NODE_SPD_MAX	(20)	// １フレームで描画状態にするスプライトの数:最大

enum{ // メッセージ
MESSAGE_KILL,	// 破棄
};

extern float _TS_Sin( int s );

typedef struct _PLASMA_NODE{ // プラズマノード
    int				nLevel;		// 階層
    int				nDrawFlag;	// 描画フラグ
    int				nDrawNum;	// 描画する頂点数
    int				nDrawSpd;	// 1フレームに追加する描画頂点数
    int				nModeCntr;	// モードカウンタ
    FVECTOR* 			pvecNodeTop1;	// 位置ワークのトップへのポインタ
    FVECTOR* 			pvecNodeTop2;	// 位置ワークのトップへのポインタ
    DG_PRIM2_UVRGBWH*		puvrgbTop1;	// テクスチャ情報ワークのトップへのポインタ
    DG_PRIM2_UVRGBWH*		puvrgbTop2;	// テクスチャ情報ワークのトップへのポインタ
    float			fAlphaHead;	// 先頭のα値
    float			fAlphaTail;	// 末尾のα値
    float			fSize;		// サイズ

    void			(*ActFunc)( struct _PLASMA_NODE*, int );	// 処理関数

    struct _PLASMA_NODE* 	pplsmPrimary;	// 第一分岐
    struct _PLASMA_NODE* 	pplsmSecondary;	// 第二分岐

    void*			pvWork;		// struct _PLASMAEVADE*
}PLASMA_NODE;

typedef struct _PLASMAEVADE{
    // 基本システム
    GV_ACT_EX		actor;
    int			name;
    GV_MSG*		msg;
    
    // 拡張管理
    int			nCntr;					// カウンタ		
    int			nStraightNode;				// 直線ノードレベル
    int			nBranchNode;				// 分岐ノードレベル
    int			nPointNum;				// 参照点数
    int			bRepelHit;				// 弾きポイントに到達
    FMATRIX		matOrg;					// 始点座標系
    FMATRIX		matRepel;				// 終点座標系
    FVECTOR*		vecPoint;				// 参照点情報
    float*		fOffsetRad;				// 半径
    float		fOneDist;				// 距離
    float		fOneRad;				// 半径

    int			nBaseAng;				// 基本方向
    int			nAddAngMin;				// 加算方向:最小
    int			nAddAngMax;				// 加算方向:最大

    float		fAlphaMax;				// α値

    int			nRandSeed;				// ランダムシード

    // プラズマノード関連
    int			nNodeNum;				// ノード数
    PLASMA_NODE*	pplsmTop;				// プラズマノードの先頭
    PLASMA_NODE		plsmData[N_PRIMS];			// プラズマノードワーク

    // 描画ワーク
    FVECTOR		vecPos1[N_PRIMS * N_VERTS];
    FVECTOR		vecPos2[N_PRIMS * N_VERTS];
    DG_PRIM2_UVRGBWH	uvrgbwh1[N_PRIMS * N_VERTS];
    DG_PRIM2_UVRGBWH	uvrgbwh2[N_PRIMS * N_VERTS];
    DG_PRIM2*		prim;
}Work;

typedef	void	( *PLSMACTMODE )( PLASMA_NODE*, int );

// シード固定ランダム
static inline int   PLSM_GetRandom( int* pnSeed, int min, int max){ return ( GM_IRnd( pnSeed) % ((max - min) + 1) + min ); }
static inline int   PLSM_Rand( int* pnSeed, int a){ return ( GM_IRnd( pnSeed) % a ); }
static inline float PLSM_Rnd( int* pnSeed){ return ( GM_Rnd( pnSeed) ); }
static inline float PLSM_FRnd( int* pnSeed){ return ( GM_FRnd( pnSeed) ); }

// ノード動作関数
static void ActDrawOff( PLASMA_NODE* 	pplsm, int nClock);
static void ActDrawing( PLASMA_NODE* 	pplsm, int nClock);
static void ActDrawEnd( PLASMA_NODE* 	pplsm, int nClock);

// 動作関数設定
static inline void SetActMode( PLASMA_NODE* pplsm, PLSMACTMODE actmode)
{
    pplsm->ActFunc = actmode;
    pplsm->nModeCntr = 0;
}

// メッセージ受信
static void CheckMessage( Work* work )
{
    GV_MSG	*msg ;
    int n_msg, code ;

    n_msg = GV_ReceiveMessage( work->name, &work->msg ); // メッセージ取得
    msg = work->msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ];
	switch( code ) {
	case MESSAGE_KILL:	// 破棄
	    GV_DestroyActor( work);
	    break;
	}
    }
}

// ノード単位のスプライト設定:再帰呼出
static void CalcInitNode( FVECTOR *vec, int node_num0, int node_num1, int* pnRandSeed )
{
    int		node_num;
    float	th;
    FVECTOR*	fv0;
    FVECTOR*	fv1;
    FVECTOR*	fv2;

    node_num = node_num1 - node_num0;
    if( node_num <= 1 ) return;
    node_num = node_num0 + node_num/2;

    fv0  = fv1 = fv2 = vec;
    fv0 += node_num0;
    fv1 += node_num1;
    fv2 += node_num;

    fv0->vw = fv1->vy - fv0->vy;
    fv1->vw = fv1->vz - fv0->vz;
    fv2->vw = fv1->vw * 0.25f * PLSM_FRnd( pnRandSeed);		/* len */
    
    // Z値の差を揺れ幅にする
    th = asinf( fv0->vw / fv1->vw );
    th = (float)(((int)(th*4096.0f/TPI))&4095)*TPI/4096.0f;
    fv2->vx = fv0->vx + (fv1->vx - fv0->vx)*0.5f + fv2->vw * PLSM_FRnd( pnRandSeed);
    fv2->vy = fv0->vy + fv0->vw*0.5f + fv2->vw * cosf(th);
    fv2->vz = fv0->vz + fv1->vw*0.5f - fv2->vw * sinf(th);

    CalcInitNode( vec, node_num0, node_num,  pnRandSeed ); // ２点の中点より小さい方
    CalcInitNode( vec, node_num,  node_num1, pnRandSeed ); // ２点の中点より大きい方
}

// 非表示
static void ActDrawOff( PLASMA_NODE* 	pplsm, int nClock)
{
    int i;
    for ( i = 0; i < N_VERTS; i++ ){
	(pplsm->puvrgbTop1 + i)->a = 0;
	(pplsm->puvrgbTop2 + i)->a = 0;
    }
}

// 描画中
static void ActDrawing( PLASMA_NODE* 	pplsm, int nClock)
{
    int			i;
    float		fAlpDiff;
    float		fAlp;
    int 		nDrawEnd = pplsm->nDrawNum + pplsm->nDrawSpd;

    Work* work = (Work*)pplsm->pvWork;

    fAlpDiff = (pplsm->fAlphaHead - pplsm->fAlphaTail) / (float)N_VERTS;
    fAlp = pplsm->fAlphaHead;

    if ( nDrawEnd >= N_VERTS){ // ノードの終点まで到達
	nDrawEnd = N_VERTS;
	SetActMode( pplsm, ActDrawEnd);

	// 弾き点到達
	if ( !work->bRepelHit && pplsm->nLevel >= work->nStraightNode ){
	    // 電磁場エフェクト生成
	    void* NewElectroField( FMATRIX*, int, int);
	    work->bRepelHit = 1;
	    GV_SetActorChild( work, NewElectroField( &work->matRepel, work->nBaseAng, work->nRandSeed) );
	}

	// 子ノードを描画中モードに移行させる
	if ( pplsm->pplsmPrimary != NULL && pplsm->pplsmPrimary->nDrawFlag == 0){
	    SetActMode( pplsm->pplsmPrimary, ActDrawing);
	}   
	if ( pplsm->pplsmSecondary != NULL && pplsm->pplsmSecondary->nDrawFlag == 0){
	    SetActMode( pplsm->pplsmSecondary, ActDrawing);
	}   
    }

    // α設定
    for ( i = 0; i < nDrawEnd; i++ ){
	(pplsm->puvrgbTop1 + i)->a  = (int)fAlp;
	(pplsm->puvrgbTop2 + i)->a  = (int)fAlp;
	fAlp += fAlpDiff;
    }

    // 描画数更新
    pplsm->nDrawNum = nDrawEnd;
}

// 描画終了
static void ActDrawEnd( PLASMA_NODE* 	pplsm, int nClock)
{
    int i;
    Work* work = (Work*)pplsm->pvWork;

    // 先頭ならα減衰
    if ( work->pplsmTop == pplsm){
	if ( pplsm->nModeCntr >= COUNT_VMODE(30)) work->fAlphaMax *= 0.9f;
    }

    // α値更新
    for ( i = 0; i < N_VERTS; i++ ){
	if ( (pplsm->puvrgbTop1 + i)->a > (int)work->fAlphaMax ){
	    (pplsm->puvrgbTop1 + i)->a = (int)work->fAlphaMax;
	}
	if ( (pplsm->puvrgbTop2 + i)->a > (int)work->fAlphaMax ){
	    (pplsm->puvrgbTop2 + i)->a = (int)work->fAlphaMax;
	}
    }
}

// ノードの更新
static void UpdateNode( PLASMA_NODE* 	pplsm,
			int*		pNum,
			int		nLevel,
			int		nClock,
                        float		fAlpha)
{
    // α値
    pplsm->fAlphaHead = fAlpha;
    pplsm->fAlphaTail = fAlpha * 0.9f;

    // 動作関数の呼出
    pplsm->ActFunc( pplsm, nClock);
    pplsm->nModeCntr++;

    (*pNum)++;

    // 再帰呼出
    if ( pplsm->pplsmPrimary != NULL)   UpdateNode( pplsm->pplsmPrimary, pNum, nLevel+1, nClock, pplsm->fAlphaTail);
    if ( pplsm->pplsmSecondary != NULL) UpdateNode( pplsm->pplsmSecondary, pNum, nLevel+1, nClock, pplsm->fAlphaTail);
}

// 再帰で非表示にする
static void SendOff( PLASMA_NODE* 	pplsm)
{
    pplsm->nDrawNum = 0;
    SetActMode( pplsm, ActDrawOff);
    if ( pplsm->pplsmPrimary != NULL)   SendOff( pplsm->pplsmPrimary);
    if ( pplsm->pplsmSecondary != NULL) SendOff( pplsm->pplsmSecondary);
}

// 描画開始メッセージ
static void BeginDraw( Work *work )
{
    PLASMA_NODE* 	pplsm;

    if ( work->pplsmTop == NULL) return;

    // 一度非表示にしておく(再帰)
    pplsm = work->pplsmTop;
    SendOff( pplsm);
    
    // 描画モードに移行
    pplsm = work->pplsmTop;
    SetActMode( pplsm, ActDrawing);	
}

// 動作関数
static void Act( Work *work )
{
    int nClock;
    int nNum = 0;
    int nLevel = 0;

    // メッセージ受信
    CheckMessage( work );

    // プリミティブ更新
    GM_GroupPrim2( work->prim, GM_CurrentStageMap );	// マップ設定
    DG_SwitchBuffPrim2( work->prim );			// バッファ切替え
    nClock = work->prim->buffer_clock; 			// いじってもよいバッファ番号の取得

    // ノード更新
    UpdateNode( work->pplsmTop, &nNum, nLevel, nClock, work->fAlphaMax);
}

// 破棄関数
static void Die( Work *work )
{
    work->prim       = MAO_FreePrim2( work->prim );

    if ( work->vecPoint != NULL) 	GV_Free( work->vecPoint);
    if ( work->fOffsetRad != NULL) 	GV_Free( work->fOffsetRad);
}

// プラズマノードを作成:(再帰呼出)
static PLASMA_NODE* MakePlasmaNode( PLASMA_NODE* 	pplsmTop,  	// プラズマノードワークの先頭
				    DG_TEX*		tex,		// テクスチャ情報
				    Work*		work,		// プラズマ構造体
				    FVECTOR*		pvecPrev,	// ノードの始点
				    FVECTOR*		pvecPoint,	// ノードの終点
				    float*		pfOffsetRad,	// 半径
				    FVECTOR* 		pvecWork1, 	// 頂点座標データの先頭:１
				    FVECTOR* 		pvecWork2, 	// 頂点座標データの先頭:２
				    DG_PRIM2_UVRGBWH* 	puvrgbWork1, 	// 頂点UVRGBAデータの先頭:１
				    DG_PRIM2_UVRGBWH* 	puvrgbWork2, 	// 頂点UVRGBAデータの先頭:２
				    int* 		pnWorkNum, 	// 登録数カウント
				    int 		nLevel,		// ノード階層
				    int			nAng,		// ノードの終点を中心からどの方向にするか決める角度
				    int			nAddAngMin,	// 次ノードに渡す際にnAngに加算するランダム角度の最小	
				    int			nAddAngMax,	// 次ノードに渡す際にnAngに加算するランダム角度の最大
				    float		fSize)		// スプライトのサイズ
{
    FVECTOR		vecFrom;
    FVECTOR		vecTo;
    FVECTOR* 		pvecTop1; 
    FVECTOR* 		pvecTop2; 
    DG_PRIM2_UVRGBWH* 	puvrgb1; 
    DG_PRIM2_UVRGBWH* 	puvrgb2;
    PLASMA_NODE* 	pplsm;

    // 登録数チェック
    if ( (*pnWorkNum) >= N_PRIMS){
	return NULL;
    }
    (*pnWorkNum)++;

    // ポインタ設定
    pvecTop1 = pvecWork1   + N_VERTS * (*pnWorkNum);
    pvecTop2 = pvecWork2   + N_VERTS * (*pnWorkNum);
    puvrgb1  = puvrgbWork1 + N_VERTS * (*pnWorkNum);
    puvrgb2  = puvrgbWork2 + N_VERTS * (*pnWorkNum);
    pplsm    = pplsmTop + (*pnWorkNum);

    // ノードレベルチェック:階層が一定以上になったら再帰をやめる
    if ( nLevel >= work->nPointNum){ 
	return NULL;
    }

    // プラズマノードワーク設定
    pplsm->nLevel	= nLevel;
    pplsm->nDrawFlag    = 0;
    pplsm->nDrawNum     = 0;
    pplsm->nDrawSpd     = PLSM_GetRandom( &work->nRandSeed, NODE_SPD_MIN,NODE_SPD_MAX);

    pplsm->pvecNodeTop1 = pvecTop1;
    pplsm->pvecNodeTop2 = pvecTop2;
    pplsm->puvrgbTop1   = puvrgb1;
    pplsm->puvrgbTop2   = puvrgb2;

    SetActMode( pplsm, ActDrawOff); // 動作関数の設定:非表示状態

    pplsm->pvWork = (void*)work;

    _sceVu0CopyVector( &vecFrom, pvecPrev);
    _sceVu0CopyVector( &vecTo,   pvecPoint);
    vecFrom.vw = vecTo.vw = 1.f;

    // 終点の位置を決定
    vecTo.vx += _TS_Sin( nAng) * (*pfOffsetRad);
    vecTo.vy += _TS_Sin( nAng + 1024) * (*pfOffsetRad);

    { // ノードにスプライト生成
	int 		i;
	FVECTOR* 	pvecPos;
	DG_PRIM2_UVRGBWH* uvrgbwh;

	pvecPos = SCR_POS;
	uvrgbwh = SCR_UVR;
	_sceVu0CopyVector( pvecPos, &vecFrom);
	_sceVu0CopyVector( pvecPos+(N_VERTS), &vecTo);
	(pvecPos+(N_VERTS-1))->vw = _MAO_FVec2Len3( &vecFrom, &vecTo) * 2.f;

	for ( i = 0; i < N_VERTS; i++){
	    // スプライトのサイズ	
	    uvrgbwh->w  = (int)(10 + (fSize * 80.f));
	    uvrgbwh->h  = (int)(10 + (fSize * 80.f));

	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );/* 左上 */
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );/* 左上 */
	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );/* 右下 */
	    uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );/* 右下 */
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f0 = 0x8fff; // 描画しないようにしておく
	    uvrgbwh->f1 = 0x8fff;

	    uvrgbwh->r  = 16;
	    uvrgbwh->g  = 32;
	    uvrgbwh->b  = 48;
	    uvrgbwh->a  = 0;

	    pvecPos++;
	    uvrgbwh++;
	}
	// プラズマの揺れをつくる
	CalcInitNode( SCR_POS, 0, N_VERTS, &work->nRandSeed);

	// スクラッチパッドからメインメモリへ転送
	Mao_CopyScrToMem( pvecTop1, SCR_POS, sizeof(FVECTOR), N_VERTS );
	Mao_CopyScrToMem( pvecTop2, SCR_POS, sizeof(FVECTOR), N_VERTS );
	Mao_CopyScrToMem( puvrgb1, SCR_UVR, sizeof(DG_PRIM2_UVRGBWH), N_VERTS );
	Mao_CopyScrToMem( puvrgb2, SCR_UVR, sizeof(DG_PRIM2_UVRGBWH), N_VERTS );

	if ( nLevel > work->nStraightNode ) fSize *= 0.85f; // 枝分かれ後のノードならサイズを減衰
    }

    // かならず１本は生成
    pplsm->pplsmPrimary   = MakePlasmaNode( pplsmTop, tex, work, &vecTo, pvecPoint+1, 
					    pfOffsetRad+1, pvecWork1, pvecWork2, 
					    puvrgbWork1, puvrgbWork2, pnWorkNum, nLevel+1, 
					    nAng + PLSM_GetRandom( &work->nRandSeed, nAddAngMin, nAddAngMax), 
					    nAddAngMin, nAddAngMax, fSize);
    
    // ２本目は条件次第
    if ( nLevel > work->nStraightNode){ // 枝分かれノードなら
	if ( nLevel - work->nStraightNode >= work->nBranchNode / 2 ){
	    if ( PLSM_Rand( &work->nRandSeed, 10) < 2){
		pplsm->pplsmSecondary = MakePlasmaNode( pplsmTop, tex, work, &vecTo, pvecPoint+1,
							pfOffsetRad+1, pvecWork1, pvecWork2, 
							puvrgbWork1, puvrgbWork2, pnWorkNum, nLevel+1, 
							nAng + PLSM_GetRandom( &work->nRandSeed, nAddAngMin, nAddAngMax), 
							nAddAngMin, nAddAngMax, fSize);
	    }
	}else{
	    pplsm->pplsmSecondary = MakePlasmaNode( pplsmTop, tex, work, &vecTo, pvecPoint+1,
						    pfOffsetRad+1, pvecWork1, pvecWork2, 
						    puvrgbWork1, puvrgbWork2, pnWorkNum, nLevel+1, 
						    nAng + PLSM_GetRandom( &work->nRandSeed, nAddAngMin, nAddAngMax), 
						    nAddAngMin, nAddAngMax, fSize);
	}
    }else{ // ストレートノードなら
	if ( PLSM_Rand( &work->nRandSeed, 10) < 2){
	    pplsm->pplsmSecondary = MakePlasmaNode( pplsmTop, tex, work, &vecTo, pvecPoint+1,
						    pfOffsetRad+1, pvecWork1, pvecWork2, 
						    puvrgbWork1, puvrgbWork2, pnWorkNum, nLevel+1, 
						    nAng + PLSM_GetRandom( &work->nRandSeed, nAddAngMin, nAddAngMax), 
						    nAddAngMin, nAddAngMax, fSize);
	}
    }

    return (pplsm);
}

// パケット初期化
static int InitPacket( Work* 		work, 	// 
		       DG_PRIM2*	prim,	
		       DG_TEX* 		tex) 	// テクスチャタグ
{	
    // 優先設定
    prim->raise = 3000;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
					       DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );	// α設定:加算

    return 0;
}
// リソースの初期化
static int GetResources( Work*	work)	// 
{
    int		i;
    FVECTOR	vec;
    FVECTOR	vecFrom;
    FVECTOR	vecTo;
    FVECTOR*	pvecPoint;
    float*	pfOffsetRad;	

    work->nPointNum = work->nStraightNode + work->nBranchNode;
    work->fAlphaMax = 128.f;

    // 参照点ワーク
    if( ( work->vecPoint = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * ( work->nPointNum + 1) ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! PlasmaEvade !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecPoint, sizeof(FVECTOR) * work->nPointNum );

    // 半径ワーク
    if( ( work->fOffsetRad = (float *)GV_Malloc(sizeof(float) * ( work->nPointNum + 1) ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! PlasmaEvade !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->fOffsetRad, sizeof(float) * work->nPointNum );
    
    work->nCntr = 0;

    work->bRepelHit = 0;

    pvecPoint = work->vecPoint;
    pfOffsetRad = work->fOffsetRad;
    _sceVu0CopyVector( &vec, (FVECTOR*)&work->matOrg.m[3][0]);

    _sceVu0CopyVector( &vecFrom, &DG_ZeroVector);
    MAO_RelPos( &vecTo, (FVECTOR*)&work->matRepel.m[3][0], &work->matOrg);

    // 参照点生成
    for ( i = 0; i < work->nPointNum + 1; i++){
	if ( i < work->nStraightNode) {
	    _sceVu0ScaleVector( pvecPoint, &vecTo, (float)i / (float)work->nStraightNode);
	    (*pfOffsetRad) = 50.f;
	}else{
	    float fDist = work->fOneDist * (float)(i - work->nStraightNode) / (float)work->nBranchNode;
	    float fRad  = work->fOneRad * (float)(i - work->nStraightNode) / (float)work->nBranchNode;

	    // 相対で生成
	    pvecPoint->vx = vecTo.vx;
	    pvecPoint->vy = vecTo.vy;
	    pvecPoint->vz = vecTo.vz + fDist * fDist;
	    pvecPoint->vw = 1.f;

	    (*pfOffsetRad) = fRad;
	}
	pvecPoint++;
	pfOffsetRad++;
    }

    { // プリミティブ生成
	int nWorkNum;
	DG_TEX* tex;
	// 描画用ワーク初期化
	// プリミティブ本体の作成
	work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_NOBUFFER, N_PRIMS, N_VERTS);
	if ( work->prim == NULL){
	    MAO_PRINTF("Cannot MakePrim!!\n");
	    return -1;
	}

	// バッファ設定
	DG_SetPrim2Buffer( work->prim, work->vecPos1, work->vecPos2, work->uvrgbwh1, work->uvrgbwh2);

	// テクスチャの設定
	tex = DG_GetTexture( 3594043); 

	// パケットの初期化
	InitPacket( work, work->prim, tex);

	// 親座標系を設定
	work->prim->root = &work->matOrg;

	nWorkNum = 0;
	work->nNodeNum = 0;

	work->pplsmTop = MakePlasmaNode( &work->plsmData[0], tex, work, work->vecPoint, work->vecPoint + 1, 
					 work->fOffsetRad, work->vecPos1, work->vecPos2, 
					 work->uvrgbwh1, work->uvrgbwh2, &nWorkNum, 0, 
					 work->nBaseAng, work->nAddAngMin, work->nAddAngMax, 1.f);

	if ( work->pplsmTop == NULL){
	    MAO_PRINTF(" NO MEMORY !! PlasmaEvade !!\n");
	    return -1;
	}

	BeginDraw( work);
	work->fAlphaMax = 128.f;
	work->bRepelHit = 0;
    }

    return 0;
}

// フォーチュンリニアのオフセット
static FVECTOR FRT_LNR_NozzleOfst = { -24.0f, -1129.501831f, 75.0f, 1.0f } ;

// プラズマ回避エフェクト
void* NewPlasmaEvade( int 	name, 		// 名前ID
		      FMATRIX*	pmatOrg,	// 発生座標系（即時コピー）
		      FMATRIX*	pmatRepel, 	// 弾かれる座標（即時コピー）
		      int	nStraightNode,	// 直線ノード	
		      int	nBranchNode,	// 分岐ノード	
		      float	fOneDist,	// 距離 √
		      float	fOneRad,	// 半径 √
		      int	nBaseAng,	// 基本方向	
		      int	nAddAngMin,	// 下限加算方向
		      int	nAddAngMax,	// 上限加算方向
		      int	nRandSeed)	// ランダムシード
{
    Work*	work;
    FVECTOR tmpfv ;
    FMATRIX tmpfm ;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );

	// 引数取得
	work->name 		= name;

	_sceVu0ApplyMatrix( &tmpfv, pmatOrg, &FRT_LNR_NozzleOfst ) ;
	_sceVu0CopyMatrix( &tmpfm, pmatOrg ) ;
	_sceVu0CopyVector( (FVECTOR *)tmpfm.m[3], &tmpfv );

	MAO_GetAimMatrix( &work->matOrg, 
			  (FVECTOR*)&pmatRepel->m[3][0], 
			  (FVECTOR*)&tmpfm.m[3][0], 
			  (FVECTOR*)&tmpfm.m[1][0]);
	_sceVu0CopyMatrix( &work->matRepel, &work->matOrg);
	_sceVu0CopyVector( (FVECTOR*)&work->matOrg.m[3][0], (FVECTOR*)&tmpfm.m[3][0]);
	_sceVu0CopyVector( (FVECTOR*)&work->matRepel.m[3][0], (FVECTOR*)&pmatRepel->m[3][0]);

	work->nStraightNode 	= nStraightNode;
	work->nBranchNode 	= nBranchNode;
	work->fOneDist		= fOneDist;
	work->fOneRad		= fOneRad;
	work->nBaseAng 	 	= nBaseAng;
	work->nAddAngMin 	= nAddAngMin;
	work->nAddAngMax 	= nAddAngMax;
	work->nRandSeed		= nRandSeed;

	if ( GetResources( work) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}





