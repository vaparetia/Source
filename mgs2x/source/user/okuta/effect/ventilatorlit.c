//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ventilatorlit.c
    天窓の光
    2001/07/30 Masafumi Okuta
    $Id: ventilatorlit.c,v 1.1.1.3 2002/11/19 11:47:53 Yoshizawa1 Exp $
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
#define	SCR_UVR			(SCRPAD_ADDR+0x1000)

#define WIDE		(400.f)

// プリミティブ
#define N_VERTS		(16)

extern float _TS_Sin( int s );
enum{ // メッセージ
MESSAGE_ACTIVE,	// アクティブモード
MESSAGE_SLEEP,	// スリープモード
};
typedef struct _BLOODWEEP {
    // 基本システム
    GV_ACT_EX		actor;
    int			name;
    int			map;
    GV_MSG*		msg;			// 自分宛メッセージ	
    
    // 拡張管理
    int			nSleep;
    short*		nDefaultY;		// 発生時の高さ
    u_char*		ucDripAlpha;		// α値

    // 引数から取得するデータ
    FVECTOR		vecBound1;		// 最小点
    FVECTOR		vecBound2;		// 最大点
    FVECTOR		vecBound3;		// 最小点
    FVECTOR		vecBound4;		// 最大点
    short		nNumW;			// 横の数
    short		nNumH;			// 縦の数
    float		fSizeUpper;		// 上の大きさ
    float		fSizeLower;		// 下の大きさ
    int			nRGBA1;			// RGBA上
    int			nRGBA2;			// RGBA下

    // 描画系
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
	case MESSAGE_ACTIVE:	// アクティブモード
	    work->nSleep = 0;
	    UNSET_FLAG( work->prim->flag, DG_PRIM2_INVISIBLE);
	    break;
	case MESSAGE_SLEEP:	// スリープモード
	    work->nSleep = 1;
	    SET_FLAG( work->prim->flag, DG_PRIM2_INVISIBLE);
	    break;
	default :
	    break;
	}
	msg++ ;
    }
}

// 動作関数
static void Act( Work* work )
{
    CheckMessage( work);

    if ( work->nSleep ) return;

    // 所属グループ更新
    GM_GroupPrim2( work->prim, GM_CurrentStageMap);
    // バッファ切替え
    DG_SwitchBuffPrim2( work->prim );
}

// 破棄関数
static void Die( Work* work )
{
    work->prim 	   = MAO_FreePrim2( work->prim );
}

static void MakeOneLightBox( Work* 		work, 		// 
			     DG_TEX*		tex,
			     FVECTOR* 		pvecUpper1,	// 
			     FVECTOR* 		pvecUpper2,	// 
			     FVECTOR* 		pvecLower1,	// 
			     FVECTOR* 		pvecLower2,	// 
			     FVECTOR* 		pvecPrim,	// 
			     DG_PRIM2_UVRGB*	uvrgb)		// 
{
    int i;
    u_char		r1,g1,b1,a1;
    u_char		r2,g2,b2,a2;
    // 奥
    pvecPrim->vx = pvecUpper1->vx;    pvecPrim->vy = pvecUpper1->vy;
    pvecPrim->vz = pvecUpper1->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecUpper2->vx;    pvecPrim->vy = pvecUpper1->vy;
    pvecPrim->vz = pvecUpper1->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecLower1->vx;    pvecPrim->vy = pvecLower1->vy;
    pvecPrim->vz = pvecLower1->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecLower2->vx;    pvecPrim->vy = pvecLower1->vy;
    pvecPrim->vz = pvecLower1->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    // 前
    pvecPrim->vx = pvecUpper1->vx;    pvecPrim->vy = pvecUpper1->vy;
    pvecPrim->vz = pvecUpper2->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecUpper2->vx;    pvecPrim->vy = pvecUpper1->vy;
    pvecPrim->vz = pvecUpper2->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecLower1->vx;    pvecPrim->vy = pvecLower1->vy;
    pvecPrim->vz = pvecLower2->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecLower2->vx;    pvecPrim->vy = pvecLower1->vy;
    pvecPrim->vz = pvecLower2->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    // 左
    pvecPrim->vx = pvecUpper1->vx;    pvecPrim->vy = pvecUpper1->vy;
    pvecPrim->vz = pvecUpper1->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecUpper1->vx;    pvecPrim->vy = pvecUpper1->vy;
    pvecPrim->vz = pvecUpper2->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecLower1->vx;    pvecPrim->vy = pvecLower1->vy;
    pvecPrim->vz = pvecLower1->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecLower1->vx;    pvecPrim->vy = pvecLower1->vy;
    pvecPrim->vz = pvecLower2->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    // 右
    pvecPrim->vx = pvecUpper2->vx;    pvecPrim->vy = pvecUpper1->vy;
    pvecPrim->vz = pvecUpper1->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecUpper2->vx;    pvecPrim->vy = pvecUpper1->vy;
    pvecPrim->vz = pvecUpper2->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecLower2->vx;    pvecPrim->vy = pvecLower1->vy;
    pvecPrim->vz = pvecLower1->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;
    pvecPrim->vx = pvecLower2->vx;    pvecPrim->vy = pvecLower1->vy;
    pvecPrim->vz = pvecLower2->vz;    pvecPrim->vw = 1.f;
    pvecPrim++;

    // RGBA値
    r1 = ((work->nRGBA1 & 0xff000000) >> 24);
    g1 = ((work->nRGBA1 & 0x00ff0000) >> 16);
    b1 = ((work->nRGBA1 & 0x0000ff00) >> 8);
    a1 =  (work->nRGBA1 & 0x000000ff);

    r2 = ((work->nRGBA2 & 0xff000000) >> 24);
    g2 = ((work->nRGBA2 & 0x00ff0000) >> 16);
    b2 = ((work->nRGBA2 & 0x0000ff00) >> 8);
    a2 =  (work->nRGBA2 & 0x000000ff);

    for ( i = 0; i < N_VERTS; i++){
	static float fU[4] = { 0.f, 1.f, 0.f, 1.f };
	static float fV[4] = { 0.33f, 0.33f, 0.66f, 0.66f };
//	static float fV[4] = { 0.f, 0.f, 1.f, 1.f };
	uvrgb->u = FTOI12( fU[i%4] * tex->u_scale + tex->u_offset ); //
	uvrgb->v = FTOI12( fV[i%4] * tex->v_scale + tex->v_offset ); //
	uvrgb->q = 4096;
	if ( i % 4 < 2 ){
	    uvrgb->f = 0x8fff;
	    uvrgb->r = r1;
	    uvrgb->g = g1;
	    uvrgb->b = b1;
	    uvrgb->a = a1;
	}else{
	    uvrgb->f = 0x0fff;
	    uvrgb->r = r2;
	    uvrgb->g = g2;
	    uvrgb->b = b2;
	    uvrgb->a = a2;
	}
	uvrgb++;
    }
}

static int InitPacket( Work* 		work, 		// 
		       DG_PRIM2*	prim,		// 
		       DG_TEX* 		tex,		// 
		       long64		tagAlpha,	// 
		       u_int		nRGBA,		// 
		       int		nRaise)
{
    int			j, k;
    FVECTOR		vec1;
    FVECTOR		vec2;
    FVECTOR		vec3;
    FVECTOR		vec4;
    FVECTOR*		pvecVert1;
    FVECTOR*		pvecVert2;
    DG_PRIM2_UVRGB*	uvrgb1;
    DG_PRIM2_UVRGB*	uvrgb2;
    FVECTOR*		pvecVert;
    DG_PRIM2_UVRGB*	uvrgb;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    pvecVert1 = prim->pos[0];
    pvecVert2 = prim->pos[1];
    uvrgb1    = prim->uvrgb[0];
    uvrgb2    = prim->uvrgb[1];

    pvecVert 	= SCR_POS;
    uvrgb	= SCR_UVR;

    _sceVu0CopyVector( &vec1, &work->vecBound1);
    _sceVu0CopyVector( &vec3, &work->vecBound3);
    {
	for ( j = 0; j < work->nNumH; j++){
	    vec1.vx = work->vecBound1.vx;
	    vec1.vy = work->vecBound1.vy;
	    vec2.vy = work->vecBound2.vy;
	    vec2.vz = vec1.vz + work->fSizeUpper;
	    vec3.vx = work->vecBound3.vx;
	    vec3.vy = work->vecBound3.vy;
	    vec4.vy = work->vecBound4.vy;
	    vec4.vz = vec3.vz + work->fSizeLower;
	    for ( k = 0; k < work->nNumW; k++){
		vec2.vx  = vec1.vx + work->fSizeUpper;
		vec4.vx  = vec3.vx + work->fSizeLower;
		MakeOneLightBox( work, tex, &vec1, &vec2, &vec3, &vec4, SCR_POS, SCR_UVR);
		vec1.vx  = vec2.vx;
		vec3.vx  = vec4.vx;

		// スクラッチパッド->メインメモリ
		Mao_CopyScrToMem( pvecVert1, SCR_POS, sizeof(FVECTOR), N_VERTS);
		Mao_CopyScrToMem( pvecVert2, SCR_POS, sizeof(FVECTOR), N_VERTS);
		Mao_CopyScrToMem( uvrgb1, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_VERTS);
		Mao_CopyScrToMem( uvrgb2, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_VERTS);
		pvecVert1 	+= N_VERTS;
		pvecVert2 	+= N_VERTS;
		uvrgb1 		+= N_VERTS;
		uvrgb2 		+= N_VERTS;
	    }
	    vec1.vz = vec2.vz;
	    vec3.vz = vec4.vz;
	}
    }
    return 1;
}

// リソース初期化
static int GetResources( Work*		work,
			 int		name,	
			 int		map)	
{
    char*	opt;
    DG_TEX*	tex;
    extern int ENE_GCL_GetFV( char *ptr, FVECTOR *fvec );
    extern int GCL_GetNextIV( int *vec );

    work->name = name;
    work->map  = map;

    work->nSleep = 0;

    if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){	// 座標取得
	if ( ENE_GCL_GetFV( opt, &work->vecBound1 ) != 0 ){
	    work->vecBound1.vx = 0.f;
	    work->vecBound1.vy = 0.f;
	    work->vecBound1.vz = 0.f;
	    work->vecBound1.vw = 1.f;
	}
	if ( ENE_GCL_GetFV( opt, &work->vecBound2 ) != 0 ){
	    work->vecBound2.vx = 0.f;
	    work->vecBound2.vy = 0.f;
	    work->vecBound2.vz = 0.f;
	    work->vecBound2.vw = 1.f;
	}
	if ( ENE_GCL_GetFV( opt, &work->vecBound3 ) != 0 ){
	    work->vecBound3.vx = 0.f;
	    work->vecBound3.vy = 0.f;
	    work->vecBound3.vz = 0.f;
	    work->vecBound3.vw = 1.f;
	}
	if ( ENE_GCL_GetFV( opt, &work->vecBound4 ) != 0 ){
	    work->vecBound4.vx = 0.f;
	    work->vecBound4.vy = 0.f;
	    work->vecBound4.vz = 0.f;
	    work->vecBound4.vw = 1.f;
	}
    }else{
	work->vecBound1.vx = 0.f;
	work->vecBound1.vy = 0.f;
	work->vecBound1.vz = 0.f;
	work->vecBound1.vw = 1.f;
	work->vecBound2.vx = 0.f;
	work->vecBound2.vy = 0.f;
	work->vecBound2.vz = 0.f;
	work->vecBound2.vw = 1.f;
	work->vecBound3.vx = 0.f;
	work->vecBound3.vy = 0.f;
	work->vecBound3.vz = 0.f;
	work->vecBound3.vw = 1.f;
	work->vecBound4.vx = 0.f;
	work->vecBound4.vy = 0.f;
	work->vecBound4.vz = 0.f;
	work->vecBound4.vw = 1.f;
    }

    work->nNumW  	= 1;
    work->nNumH  	= 1;
    work->fSizeUpper    = (work->vecBound2.vx - work->vecBound1.vx) / (float)work->nNumW;
    work->fSizeLower    = (work->vecBound4.vx - work->vecBound3.vx) / (float)work->nNumW;
    work->nRGBA1	= GCL_GetOptionValue( 'c', 0x34323030);
    work->nRGBA2	= GCL_GetOptionValue( 'd', 0x34323030);


    // 描画用ワーク初期化
    // 滴プリミティブ本体の作成
    work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE, work->nNumW * work->nNumH, N_VERTS);
    if ( work->prim == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャ設定
//    tex = DG_GetTexture( 8781729 ); // drop02_msk
    tex = DG_GetTexture( 10972307 ); // drop02_msk
    InitPacket( work, work->prim, tex, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ), work->nRGBA1, 10000);

    return 0;
}

// 滴り血を生成
void* NewVentilatorLight( int name, int map)
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	if ( GetResources( work, name, map) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}
