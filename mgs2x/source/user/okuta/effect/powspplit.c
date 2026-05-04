//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    kmdanna.c
    かもめの糞
    2001/07/03 Masafumi Okuta
    $Id: powspplit.c,v 1.1.1.3 2002/11/19 11:47:53 Yoshizawa1 Exp $
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

extern int BP_AdjustTick(int);
extern int BP_AdjustTick2(int);
extern float BP_AdjustTick3(float);
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))
#define STEP_VMODE(_a) (BP_AdjustTick2(_a))
#define STEP_VMODEF(_a) (BP_AdjustTick3(_a))


// RGBA設定
#define COL_R		(255)
#define COL_G		(30)
#define COL_B		(30)
#define ALPHA		(164)

// プリミティブ,頂点数
#define N_PRIMS		(1)
#define N_VERTS		(3)

// スクラッチパッドアドレス
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

// テクスチャstrcode
#define TEX_LIGHT	(3726825)	// "svc_bonbori"

#define RAISE		(0)		// 優先
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define SIZE		(50.f)
#define	RAISE_SHIFT	(30.f)

// 表示位置
static FVECTOR Tbl_vecCenter[3] = {
    { 7155.f, 1650.f, -246830.f, 1.f },
    { 6200.f, 1650.f, -246830.f, 1.f },
    { 5255.f, 1650.f, -246830.f, 1.f },
};

enum{ // 点滅モード
ADD_BLINK,
SUB_BLINK,
};

enum{ // メッセージ
MESSAGE_KILL, // 破棄
};

typedef struct _KAMOME_DANNA {
    // 基本システム
    GV_ACT_EX		actor;
    int			name;				// 名前
    int			map;				// 所属するマップ
    HZX_GROUP_ID	hzx_id;				// HZXのグループID 
    GV_MSG*		msg;				// 自分宛メッセージ	

    int			nCntr;				// カウンタ
    int			nTiming;			// 間隔
    int			nFlag;				// 点滅のフラグ
    float		fRate;				// レート
    float		fAddRate;			// 変化レート
    
    // 拡張管理
    FVECTOR		vecCenter;			// 中心
    float		fSize;				// 大きさ
    float		fAlpha;				// α
    
    // 描画用
    DG_PRIM2*		prim;		
} Work;

// 動作関数
static void Act( Work* work )
{
    int 		i,k, nAlpha, nPrevBuff;
    FVECTOR*		pvecPos;
    FMATRIX 		mat;
    DG_PRIM2_UVRGBWH*	uvrgbwh;
    float	screen_near_x;
    float	screen_near_y;
    float	physic_raise;

    screen_near_x = (ASPECT_X()                                       ) * DG_Chanls->screen;
    screen_near_y = (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) * DG_Chanls->screen;

    nPrevBuff = work->prim->buffer_clock;
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    pvecPos 	= SCR_POS;		// 
    uvrgbwh	= SCR_UVS;		// 

    if ( work->nFlag == ADD_BLINK){
	work->fRate += work->fAddRate;
	if ( work->fRate >= 1.f){
	    work->fRate = 1.f;
	    work->nFlag = SUB_BLINK;
	}
    }else{
	work->fRate -= work->fAddRate;
	if ( work->fRate <= 0.f){
	    work->fRate = 0.f;
	    work->nFlag = ADD_BLINK;
	}
    }

    nAlpha = (int)(work->fAlpha * work->fRate);

    _sceVu0CopyMatrix( &mat, &DG_Chanls[0].eye);
    Mao_CopyMemToScr( work->prim->uvrgb[ nPrevBuff ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
    for ( i = 0 ; i < N_PRIMS ; i++ ){
	for ( k = 0 ; k < N_VERTS ; k++ ){
	    _sceVu0CopyVector( pvecPos, &Tbl_vecCenter[k] );

	    /* 座標を透視変換する */
	    DG_SetPos( &DG_Chanls->eye_pers_no_offset ); //BP_CAMERA - replaced "eye_pers" with "eye_pers_no_offset" to fix projection/inverse projection
	    DG_PutVector( pvecPos, pvecPos, 1 );

	    if( pvecPos->vz < pvecPos->vw  &&  pvecPos->vw > RAISE_SHIFT + 51.0f ){
		physic_raise = pvecPos->vw - RAISE_SHIFT;

		pvecPos->vz = physic_raise;
		pvecPos->vx = physic_raise * pvecPos->vx / screen_near_x / pvecPos->vw;
		pvecPos->vy = physic_raise * pvecPos->vy / screen_near_y / pvecPos->vw;

		/* カメラ座標をワールド座標に変換 */
		DG_SetPos( &DG_Chanls->eye );
		DG_PutVector( pvecPos, pvecPos, 1 );
	    }

	    uvrgbwh->a = nAlpha;

	    pvecPos++;
	    uvrgbwh++;
	}
    }

    Mao_CopyScrToMem( work->prim->pos[ work->prim->buffer_clock ], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( work->prim->uvrgb[ work->prim->buffer_clock ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

    { // メッセージ
	GV_MSG	*msg ;
	int n_msg, code ;

	n_msg = GV_ReceiveMessage( work->name, &work->msg ); // メッセージ取得
	msg = work->msg ;

	while ( n_msg-- > 0 ) {
	    code = msg->message[ 0 ];
	    switch( code ) {
	    case MESSAGE_KILL :	// 破棄
		GV_DestroyActor( work);
		return;
		break;
	    default :
		break;
	    }
	    msg++ ;
	}
    }

    work->nCntr++;
}
// 動作関数
static void Die( Work* work )
{
    work->prim  = MAO_FreePrim2( work->prim );
}

// パケット初期化
static int InitPacket( DG_PRIM2* 	prim, 		// プリミティブワーク
		       DG_TEX* 		tex, 		// テクスチャワーク
		       long64		tagAlpha,	// αタグ
		       int		nRGBA,		// 色
		       FVECTOR* 	pvecCenter, 	// 中心
		       int		nRaise,		// Z優先
		       float 		fSize )		// サイズ
{
    u_char		r,g,b,a;
    int			i, k;
    FVECTOR*		pvecPos;
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
    for ( i = 0 ; i < N_PRIMS ; i++ ){
	for ( k = 0 ; k < N_VERTS ; k++ ){
	    DG_COPY_VEC( pvecPos, &pvecCenter[k] );
	    uvrgbwh->w  = (int)fSize;
	    uvrgbwh->h  = (int)fSize;

	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	    uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	    uvrgbwh->q0 = 4096 ;
	    uvrgbwh->q1 = 4096 ;
	    uvrgbwh->f0 = 0x0fff ;
	    uvrgbwh->f1 = 0x0fff ;

	    uvrgbwh->r  = r;
	    uvrgbwh->g  = g;
	    uvrgbwh->b  = b;
	    uvrgbwh->a  = a;

	    pvecPos++;
	    uvrgbwh++ ;
	}
    }
    // スクラッチパッド->メインメモリへ
    Mao_CopyScrToMem( prim->pos[0], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->pos[1], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->uvrgb[0], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
    Mao_CopyScrToMem( prim->uvrgb[1], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

    return 1;
}

// 初期化
static	int GetResources( Work* work, int name, int where )
{	
    DG_TEX*	tex;

    // 基本システムワーク初期化
    work->name 	 = name;
    work->map 	 = where;	// マップID取得
    work->hzx_id = GM_GetHzxGroupID( work->map );

    work->nCntr   = 0;
    work->nTiming = COUNT_VMODE( GCL_GetOptionValue( 't', 60 ) ) ;
    if ( work->nTiming == 0 ) work->nTiming = 60;

    work->fRate = 0.f;
    work->fAddRate = (1.f / (float)work->nTiming);


    // 拡張管理ワーク初期化
    work->fSize    = SIZE;
    work->fAlpha   = ALPHA;	

    // 描画用ワーク初期化
    // プリミティブ本体の作成
    work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS);
    if ( work->prim == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャの設定
    tex = DG_GetTexture( TEX_LIGHT );
    // パケットの初期化
    InitPacket( work->prim, tex, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ), MAO_SetRGBA( COL_R, COL_G, COL_B, work->fAlpha),
		&Tbl_vecCenter[0], RAISE, work->fSize);
    return 0;
}

// 電源供給ライト
void* NewPowerSupplyLight( int name, int where )
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, name, where ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}
