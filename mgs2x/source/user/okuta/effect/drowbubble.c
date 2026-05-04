//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    drowbubble.c
    溺死時の泡
    2001/07/26 Masafumi Okuta
    $Id: drowbubble.c,v 1.1.1.3 2002/11/19 11:47:50 Yoshizawa1 Exp $
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

inline int BP_AdjustTick(int);
inline int BP_AdjustTick2(int);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))

#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_RND			(SCRPAD_ADDR + 0x3000)
#define	SCR_UVR			(SCRPAD_ADDR + 0x3000)
#define RANDAM_FIELD_NUM	(0x1000 / 4)
#define N_PRIMS			(16)
#define N_VERTS			(32)

#define	BUBBLE_SIZE_MIN			(10.0f)
#define	BUBBLE_SIZE_RAND		(10.0f)
#define	BUBBLE_SPEED			( -P_GRAVITY / 8.0f )

// 色
#define	COL_R	(128)//(96)
#define	COL_G	(122)//(102)
#define	COL_B	(128)//(128)
#define	COL_A	(32)

extern float GM_WaterLevel;

extern void TS_VecToRot( SVECTOR *rot, FVECTOR *vec );
extern float _TS_Sin( int s );

enum{ // メッセージ
WATER_MESSAGE_ACTIVE,	// アクティブモード
WATER_MESSAGE_SLEEP,	// スリープモード
};

typedef struct _WATERBUBBLE {
    // 基本システム
    GV_ACT_EX		actor;
    int			name;			// 名前ID
    int			map;			// 所属するマップ
    GV_MSG*		msg;			// 自分宛メッセージ	

    // 拡張システム
    FVECTOR		vecPre;			// 位置
    FVECTOR		vecPos;			// 位置
    FVECTOR		vecDir;			// 方向
    float		fSizeMin;		// サイズ（最小）
    float		fSizeMax;		// サイズ（最大）
    int			nLifeMin;		// 生存時間（最小）
    int			nLifeMax;		// 生存時間（最大）
    int			nRGBA;			// 色
    int			nSleep;			// スリープモード

    int			nAppear;
    int			nCntr;

    FVECTOR		vecBubbleDir[ N_PRIMS * N_VERTS ];	// 方向
    int			nBubbleLife[ N_PRIMS * N_VERTS ];	// 生存時間	

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
	case WATER_MESSAGE_ACTIVE:	// アクティブモード
	    work->nSleep = 0;
	    break;
	case WATER_MESSAGE_SLEEP:	// スリープモード
	    work->nSleep = 1;
	    break;
	default :
	    break;
	}
	msg++ ;
    }
}

static void Act( Work *work )
{
    int 	j, nCurrClock, nPrevClock;
    DG_PRIM2* 	prim;


    // メッセージチェック
    CheckMessage( work);

    // スリープモードチェック
    if ( work->nSleep ) return;

    prim = work->prim;
    GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

    nPrevClock = prim->buffer_clock;
     //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
    nCurrClock = prim->buffer_clock;

    { 
	int*	 pnLife   	      = work->nBubbleLife;
	FVECTOR* pvecDir	      = work->vecBubbleDir;	
	FVECTOR* pvecPrimPrev 	      = prim->pos[ nPrevClock];
	FVECTOR* pvecPrim 	      = prim->pos[ nCurrClock ];
	DG_PRIM2_UVRGBWH* uvrgbwh     = prim->uvrgb[nCurrClock];
	int	 nBeforeNum;	
	int	 nTmpNum;
	float*	 pRand;
	FVECTOR* pvecScr;

	extern void OK_frnd_to_scr( void *dst, int num );

	OK_frnd_to_scr( SCR_RND, RANDAM_FIELD_NUM );

	pRand = (float *)SCR_RND; // 小数点乱数テーブル
		
	// 新規投入数を決める
	nBeforeNum = work->nAppear;
	nTmpNum = MAO_GetRandom(28,36);
	if ( nBeforeNum + nTmpNum > N_PRIMS*N_VERTS ){
	    nTmpNum = N_PRIMS*N_VERTS - nBeforeNum;
	}

	// 中心位置更新
	if ( nBeforeNum < N_PRIMS*N_VERTS ) {
	    // 新規生成
	    pvecScr  = SCR_POS;
	    pvecScr += nBeforeNum;
	    uvrgbwh  = prim->uvrgb[nCurrClock];
	    uvrgbwh += nBeforeNum;
	    Mao_CopyMemToScr( pvecPrimPrev, SCR_POS, sizeof(FVECTOR), work->nAppear);
	    for ( j = 0 ; j < nTmpNum ; j++ ){
		pvecScr->vx = work->vecPos.vx + frnd() * 50.f;
		pvecScr->vy = work->vecPos.vy + frnd() * 50.f;
		pvecScr->vz = work->vecPos.vz + frnd() * 50.f;
		pvecScr->vw = BUBBLE_SIZE_MIN + rnd() * BUBBLE_SIZE_RAND;
		uvrgbwh->a = 0;

		pvecScr++;
		uvrgbwh++;
	    }
	}else if ( work->nAppear != 0 ){
	    Mao_CopyMemToScr( pvecPrimPrev, SCR_POS, sizeof(FVECTOR), work->nAppear);
	}
	
	// 動作中の泡更新
	pvecScr = SCR_POS;
	uvrgbwh = prim->uvrgb[nCurrClock];
	for ( j = 0 ; j < work->nAppear ; j++ ){
	    // ゆらゆら
	    pvecScr->vx += (pvecScr->vw * (*(pRand++)) + pvecDir->vx * 0.5f);
	    pvecScr->vy += (pvecScr->vw * BUBBLE_SPEED + pvecDir->vy * 0.5f);
	    pvecScr->vz += (pvecScr->vw * (*(pRand++)) + pvecDir->vz * 0.5f);

	    if ( pvecScr->vy >= GM_WaterLevel ){ // 水面に到達
		float size = rnd();
		size *= size * size;
		pvecScr->vw = BUBBLE_SIZE_MIN + size * BUBBLE_SIZE_RAND;
		uvrgbwh->a = 0;
	    }else{
		if ( (*pnLife) < COL_A )	uvrgbwh->a = (*pnLife);
		else				uvrgbwh->a = COL_A;
	    }
	    
	    _sceVu0ScaleVector( pvecDir, pvecDir, 0.98f);

	    if ( (*pnLife) > 0 ) (*pnLife)--;

	    pvecDir++;
	    pnLife++;
	    pvecScr++;
	    uvrgbwh++;
	}
	work->nAppear = (nBeforeNum + nTmpNum);
	if ( work->nAppear >= N_PRIMS*N_VERTS) work->nAppear =  N_PRIMS*N_VERTS;

	// スクラッチパッド->メインメモリ
	if ( work->nAppear != 0 ){
	    Mao_CopyScrToMem( pvecPrim, SCR_POS, sizeof(FVECTOR), work->nAppear);
	}
    }

    if ( work->nCntr > 0){
	work->nCntr--;
	if ( work->nCntr == 0 ){
	    GV_DestroyActor( work);
	}
    }
}

static void Die( Work *work )
{
    work->prim = MAO_FreePrim2( work->prim );
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
    DG_PRIM2_UVRGBWH*	uvrgbwh1;
    DG_PRIM2_UVRGBWH*	uvrgbwh2;
    DG_PRIM2_UVRGBWH*	uvrgbwh;
    int		j;
    int		isize;
    int		col_r;
    int		col_g;
    int		col_b;
    int		col_a;
    float	size;

    prim->raise = -10000;

    DG_ConfigPrim2Tex( prim, tex );

    pos      = prim->pos[ 0 ];
    pos1     = prim->pos[ 0 ];
    pos2     = prim->pos[ 1 ];
    uvrgbwh1 = prim->uvrgb[ 0 ];
    uvrgbwh2 = prim->uvrgb[ 1 ];
    {
	float   fPow;
	SVECTOR vecRot;
	SVECTOR vecRotTmp;
	FVECTOR vecDir;
	
	_sceVu0Normalize( &vecDir, &work->vecDir);
	fPow = _MAO_FVecLen3( &work->vecDir);
	TS_VecToRot( &vecRot, &vecDir);

	vecRotTmp.vx = vecRotTmp.vy = 0 ;

	// 頂点位置データ取得
	pos = SCR_POS;
	for ( j = 0 ; j < N_PRIMS*N_VERTS ; j++ ){
	    FVECTOR vecDir;
	    int nAng = MAO_GetRandom(-16, 16);
	    size = rnd();
	    size *= size * size;

	    work->nBubbleLife[j] = MAO_GetRandom( work->nLifeMin, work->nLifeMax);
#if 0
	    // Y回転
	    work->vecBubbleDir[j].vx = -_TS_Sin( nAng ) * fPow;
	    work->vecBubbleDir[j].vy = 0.f;
	    work->vecBubbleDir[j].vz = _TS_Sin( nAng-1024 ) * fPow;

	    // X回転
	    work->vecBubbleDir[j].vy += _TS_Sin( nAng ) * work->vecBubbleDir[j].vz;
	    work->vecBubbleDir[j].vz += _TS_Sin( nAng-1024 ) * work->vecBubbleDir[j].vz;
#endif
	    pos->vw = work->fSizeMin + size * ( work->fSizeMax - work->fSizeMin);

	    vecRotTmp.vx = vecRotTmp.vx + nAng;
	    vecRotTmp.vy = vecRotTmp.vy + nAng;
	    vecRotTmp.vz = 0 ;
	    DG_SetPos2( &DG_ZeroVector, &vecRotTmp);
	    _sceVu0ScaleVector( &vecDir, &work->vecDir, 
				1.f - (pos->vw - work->fSizeMin) / ( work->fSizeMax - work->fSizeMin) * 0.5f + 0.5f);
	    DG_RotVector( &vecDir, &work->vecBubbleDir[j], 1);

	    pos++;
	}
#if 0
	DG_SetPos2( &DG_ZeroVector, &vecRot);
	DG_RotVector( work->vecBubbleDir, work->vecBubbleDir, N_PRIMS*N_VERTS);
#endif
	Mao_CopyScrToMem( pos1, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( pos2, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS);

	// カラーデータ取得
	col_r = (color >> 24);
	col_g = (color >> 16) & 0xff;
	col_b = (color >>  8) & 0xff;
	col_a =  color & 0xff;

	uvrgbwh = SCR_POS;
	pos = pos1;
	for ( j = 0 ; j < N_PRIMS*N_VERTS ; j++ ){
	    isize  = (int)pos->vw;
	    uvrgbwh->w  = isize;
	    uvrgbwh->h  = isize;
	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	    uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	    uvrgbwh->q0 = 4096 ;
	    uvrgbwh->q1 = 4096 ;
	    uvrgbwh->f0 = 0x0fff ;
	    uvrgbwh->f1 = 0x0fff ;
	    uvrgbwh->r  = col_r ;
	    uvrgbwh->g  = col_g ;
	    uvrgbwh->b  = col_b ;
	    uvrgbwh->a  = 0; // col_a;

	    pos++;
	    uvrgbwh++;
	}
	Mao_CopyScrToMem( uvrgbwh1, SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( uvrgbwh2, SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);
    }
	
    return 1;
}

static int GetResources( Work*		work)
{

    work->nAppear = 0;
    work->nSleep = 0;

    { // 描画設定
	DG_PRIM2* 	prim;
	DG_TEX*		tex;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 
					  N_PRIMS, N_VERTS );
	if ( prim == NULL ){
	    printf("null prim\n");
	    return -1;
	}
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

//	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	InitPacket( work, prim, tex, work->nRGBA );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
    }

    return 0;
}

void* NewDrowingBubble( int		name,		// 名前ID
			FVECTOR*	pvecPos, 	// 出現位置
			FVECTOR*	pvecInitDir,	// 初期移動ベクトル
			float		fSizeMin,	// サイズ（最小）
			float		fSizeMax,	// サイズ（最大）
			int 		nLifeMin,	// 生存時間（最小）
			int 		nLifeMax,	// 生存時間（最大）
			int 		nRGBA )		// 色
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );

	work->name 	= name;
	_sceVu0CopyVector( &work->vecPos, pvecPos);
	_sceVu0CopyVector( &work->vecDir, pvecInitDir);
	work->fSizeMin  = fSizeMin;
        work->fSizeMax  = fSizeMax;
	work->nLifeMin	= nLifeMin;
	work->nLifeMax	= nLifeMax;
	work->nRGBA	= nRGBA;

	work->nCntr	= nLifeMax + COUNT_VMODE(180);

	if ( GetResources( work ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    
    return (void *)work ;
}

