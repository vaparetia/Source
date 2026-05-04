//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    waterbubble.c
    水泡
    2001/07/26 Masafumi Okuta
    $Id: waterbubble.c,v 1.1.1.3 2002/11/19 11:47:53 Yoshizawa1 Exp $
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

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_RND			(SCRPAD_ADDR + 0x3000)
#define	SCR_UVR			(SCRPAD_ADDR + 0x3000)
#define RANDAM_FIELD_NUM	(0x1000 / 4)
#define N_PRIMS			(12)
#define N_VERTS			(16)

#define	BUBBLE_SIZE_MIN			(10.0f)
#define	BUBBLE_SIZE_RAND		(10.0f)
#define	BUBBLE_SPEED			( -P_GRAVITY / 12.0f )

// 色
#define	COL_R	(128)//(96)
#define	COL_G	(122)//(102)
#define	COL_B	(128)//(128)
#define	COL_A	(32)

extern float GM_WaterLevel;

enum{ // メッセージ
WATER_MESSAGE_ACTIVE,	// アクティブモード
WATER_MESSAGE_SLEEP,	// スリープモード
};

typedef struct _WATERBUBBLE {
    // 基本システム
    GV_ACT_EX		actor;
    int			name;				// 名前ID
    int			map;				// 所属するマップ
    GV_MSG*		msg;				// 自分宛メッセージ	

    // 拡張システム
    int			nSleep;				// スリープモード
    int			nReferNum;			// 参照する位置の数
    short*		nAppearNum;			// 出現数列 		: nReferNum個
    FVECTOR*		vecCenterPrev;			// 参照する泡中心位置（1フレーム前）

    short*		nLife;				// 外部参照 : 生存時間列 	   : nReferNum個
    FVECTOR*		vecCenter;			// 外部参照 : 参照する泡中心位置列 : nReferNum個

    // 描画ワーク
    DG_PRIM2*		prim;
} Work;

// メッセージチェック
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
    int 	i, j, clock;
    DG_PRIM2* 	prim;


    // メッセージチェック
    CheckMessage( work);

    // スリープモードチェック
    if ( work->nSleep ) return;

    prim = work->prim;
    GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( prim ) )
    {
       return;
    }
    clock = prim->buffer_clock;

    { // 
	short*	 pnAppear = work->nAppearNum;
	FVECTOR* pvecPre  = work->vecCenterPrev;
	short*	 pnLife   = work->nLife;
	FVECTOR* pvecPos  = work->vecCenter;
	FVECTOR* pvecPrimPrev 	= prim->pos[ (clock + 1) % 2];
	FVECTOR* pvecPrim 	= prim->pos[ clock ];
	FVECTOR* pvecScr;
	DG_PRIM2_UVRGBWH* uvrgbwh     = prim->uvrgb[clock];
	DG_PRIM2_UVRGBWH* uvrgbwhUnit = prim->uvrgb[clock];
	int	 nBeforeNum;	
	int	 nTmpNum;
	FVECTOR  vecDiff;
	float*	 pRand;
	u_char	 ucAlpha;	

	extern void OK_frnd_to_scr( void *dst, int num );

	OK_frnd_to_scr( SCR_RND, RANDAM_FIELD_NUM );

	for ( i = 0; i < work->nReferNum; i++ ){
	    // α値
	    if ( (*pnLife) < COL_A )	ucAlpha = (*pnLife);
	    else			ucAlpha = COL_A;

	    if ( (*pnLife) > 0 ){ // 動作中
		pRand = (float *)SCR_RND; // 小数点乱数テーブル
		
		// 新規投入数を決める
		nBeforeNum = (*pnAppear);
		(*pnAppear) += 16;
		if ( (*pnAppear) > N_PRIMS*N_VERTS ){
		    nTmpNum = N_PRIMS*N_VERTS - nBeforeNum;
		    (*pnAppear) = N_PRIMS*N_VERTS;
		}else{
		    nTmpNum = 16;
		}

		// 差分取得
		_sceVu0SubVector( &vecDiff, pvecPos, pvecPre);

		// 中心位置更新
		if ( nBeforeNum < N_PRIMS*N_VERTS ) {
		    // 新規生成
		    pvecScr  = SCR_POS;
		    pvecScr += nBeforeNum;
		    uvrgbwh  = uvrgbwhUnit;
		    uvrgbwh += nBeforeNum;
		    Mao_CopyMemToScr( pvecPrimPrev, SCR_POS, sizeof(FVECTOR), (*pnAppear));
		    for ( j = 0 ; j < nTmpNum ; j++ ){
			float fRate = rnd();
			pvecScr->vx = pvecPre->vx + vecDiff.vx * fRate + frnd() * 50.f;
			pvecScr->vy = pvecPre->vy + vecDiff.vy * fRate + frnd() * 50.f;
			pvecScr->vz = pvecPre->vz + vecDiff.vz * fRate + frnd() * 50.f;
			uvrgbwh->a = 0;
			pvecScr++;
			uvrgbwh++;
		    }
		}else if ( (*pnAppear) != 0 ){
		    Mao_CopyMemToScr( pvecPrimPrev, SCR_POS, sizeof(FVECTOR), (*pnAppear));
		}

		pvecScr = SCR_POS;
		uvrgbwh = uvrgbwhUnit;
		for ( j = 0 ; j < (*pnAppear) ; j++ ){
		    // ゆらゆら
		    pvecScr->vx += pvecScr->vw * (*(pRand++));
		    pvecScr->vy += pvecScr->vw * BUBBLE_SPEED;
		    pvecScr->vz += pvecScr->vw * (*(pRand++));

		    if ( pvecScr->vy >= GM_WaterLevel ){ // 水面に到達
			float size = rnd();
			size *= size * size;
			pvecScr->vw = BUBBLE_SIZE_MIN + size * BUBBLE_SIZE_RAND;
			uvrgbwh->a = 0;
		    }else{
			uvrgbwh->a = ucAlpha;
		    }

		    pvecScr++;
		    uvrgbwh++;
		}

		// スクラッチパッド->メインメモリ
		if ( (*pnAppear) != 0 ){
		    Mao_CopyScrToMem( pvecPrim, SCR_POS, sizeof(FVECTOR), (*pnAppear));
		}
		
		// 1フレーム前
		_sceVu0CopyVector( pvecPre, pvecPos); 

		if ( (*pnLife) == 1 ){ // リセット
		    (*pnAppear) = 0;
		}
	    }

	    pnAppear++;
	    pvecPre++;
	    pnLife++;
	    pvecPos++;

	    pvecPrimPrev += N_PRIMS*N_VERTS;
	    pvecPrim 	 += N_PRIMS*N_VERTS;
	    uvrgbwhUnit  += N_PRIMS*N_VERTS;
	}
    }
}

static void Die( Work *work )
{
    work->prim = MAO_FreePrim2( work->prim );
    if ( work->vecCenterPrev != NULL)	GV_Free( work->vecCenterPrev);
    if ( work->nAppearNum    != NULL)	GV_Free( work->nAppearNum);
}
// パケット初期化
static int InitPacket( Work* 		work, 	 
		       DG_PRIM2*	prim,	
		       DG_TEX* 		tex, 	
		       u_int		color)
{
    FVECTOR*		pos;
    FVECTOR*		pos1;
    FVECTOR*		pos2;
    DG_PRIM2_UVRGBWH*	uvrgbwh1;
    DG_PRIM2_UVRGBWH*	uvrgbwh2;
    DG_PRIM2_UVRGBWH*	uvrgbwh;
    int		i, j;
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
    for ( i = 0; i < work->nReferNum; i++ ){
	pos = SCR_POS;
	for ( j = 0 ; j < N_PRIMS*N_VERTS ; j++ ){
	    size = rnd();
	    size *= size * size;
	    pos->vw = BUBBLE_SIZE_MIN + size * BUBBLE_SIZE_RAND;
	    pos++;
	}
	Mao_CopyScrToMem( pos1, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( pos2, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS);

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
	    uvrgbwh->a  = col_a;

	    pos++;
	    uvrgbwh++;
	}
	Mao_CopyScrToMem( uvrgbwh1, SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);
	Mao_CopyScrToMem( uvrgbwh2, SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);

	pos1 += N_PRIMS*N_VERTS;
	pos2 += N_PRIMS*N_VERTS;
	uvrgbwh1 += N_PRIMS*N_VERTS;
	uvrgbwh2 += N_PRIMS*N_VERTS;
    }
	
    return 1;
}

static int GetResources( Work*		work, 
			 int		name,
			 FVECTOR*	pvecRefer, 
			 short*		pnLife, 
			 int 		nReferNum, 
			 int 		col )
{
    int i;

    // 引数のデータを取得
    work->name	    = name;
    work->nReferNum = nReferNum;
    work->vecCenter = pvecRefer;
    work->nLife     = pnLife;

    work->nSleep = 0;

    // 中心
    if( ( work->vecCenterPrev = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * work->nReferNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->vecCenterPrev, sizeof(FVECTOR) * work->nReferNum );

    // 出現数	
    if( ( work->nAppearNum = (short *)GV_Malloc(sizeof(short) * work->nReferNum ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! WaterBubble Buffer !!\n");
	return -1 ;
    }
    GV_ZeroMemory(work->nAppearNum, sizeof(short) * work->nReferNum );

    { // 初期値設定
	short*	 pnAppear = work->nAppearNum;
	FVECTOR* pvecPre  = work->vecCenterPrev;
	short*	 pnLife   = work->nLife;
	FVECTOR* pvecPos  = work->vecCenter;
	for ( i = 0; i < work->nReferNum; i++ ){
	    // 位置設定
	    _sceVu0CopyVector( pvecPre, pvecPos); 

	    (*pnAppear) = 0;

	    pnAppear++;
	    pvecPre++;
	    pnLife++;
	    pvecPos++;
	}
    }

    { // 描画設定
	DG_PRIM2* 	prim;
	DG_TEX*		tex;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 
					  N_PRIMS * work->nReferNum, N_VERTS );
	if ( prim == NULL ){
	    printf("null prim\n");
	    return -1;
	}
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

//	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	InitPacket( work, prim, tex, MAO_SetRGBA( COL_R, COL_G, COL_B, COL_A ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
    }

    return 0;
}

void* NewWaterBubble( int	name,
		      FVECTOR*	pvecRefer, 
		      short*	pnLife, 
		      int 	nReferNum, 
		      int 	col )
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );

	if ( GetResources( work, name, pvecRefer, pnLife, nReferNum, col ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    
    return (void *)work ;
}

