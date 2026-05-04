//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	IRSランプエフェクト  main.c
	2001/03/26 S.Kobayashi
	$Id: irs_lamp.c,v 1.1.1.3 2002/11/19 11:50:24 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include    "../test/etc.h"

#define N_PRIMS	(3)
#define N_VERTS	(1)

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))
#define LAMP_FLAG ( DG_PRIM2_SPRT | DG_PRIM2_TEX | DG_PRIM2_FOG | DG_PRIM2_ALPHA )

#define ALPHA (64)
#define LAMP_TEX (4059425) // xlit01b_ms

#define TOP_INTERVAL  ( DIRECT_TICK( 20 ) )
#define INTERVAL      ( DIRECT_TICK( 40 ) )
#define RAISE_LAMP (1000)

enum
{
    ATTACH_ON   = 0x0000,
    ATTACH_OFF  = 0x0001
};

static FVECTOR LampColor = {40.0F, 255.0F, 100.0F, 0.0F} ; // ランプの色

typedef struct _work {
	GV_ACT_EX actor;

    DG_PRIM2  *prim;
	FVECTOR     pos;  // スイッチのランプの中心位置
	SVECTOR     rot;
    DG_TEX     *tex;
	FVECTOR   color;    // ランプの色
	int       count;
	int    interval;   // 点滅間隔
	float      sign;   // 符号
	int alpha_max[ 3 ];

	CONTROL   *ctrl;  // くっつけ用
	int  attach_flg;
	int  attach_put;

	int        name;
	int         map;
    int    ( *act )( struct _work * );
} Work;

// プロトタイプ
static int BreakBOX_Act2( Work * );
static int BOX_NONE_Act( Work * );

extern void *NewIRS_Box_Break_Effect(FVECTOR  *, SVECTOR  *);
extern void  *NewSpark1(int n_packets, FVECTOR *center, float min_speed, 
                       float speed_wide, float gravity, SVECTOR *rot, 
		       SVECTOR *rot_wide, FVECTOR *color, float length, int count );

static void ColorTrans( DG_PRIM2_UVRGBWH *pUvrgbwh , Work *pWork )  // 色情報の転送＋ライトの設定
{
	int i;

	// スイッチのエフェクト
	for ( i = 0 ; i < N_VERTS ; i++ ){
#if 0
		pUvrgbwh[ i ].r = pWork->color.vx;
		pUvrgbwh[ i ].g = pWork->color.vy;
		pUvrgbwh[ i ].b = pWork->color.vz;
#endif
		pUvrgbwh[ i ].a = pWork->color.vw;
	}
}

static int PrimLampMove( Work *pWork )
{
	FVECTOR  *pos;
	FVECTOR  pos2;
	int     i , j;

	if ( pWork->attach_flg == ATTACH_ON ){ // 対象オブジェクトの場所に更新
		pWork->prim->as_world = pWork->ctrl->object->objs->objs[ pWork->attach_put ].world;
		pWork->prim->group_id = pWork->map = pWork->ctrl->map;
	}
	pos2 = pWork->pos;
	pos = MEM_ADDR1;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( j = 0 ; j < N_VERTS ; j++ ){
			DG_COPY_VEC( pos , &pos2 );
			pos++;
		}
	}
	OK_Scr_Mem( pWork->prim->pos[ pWork->prim->buffer_clock ], MEM_ADDR1 , sizeof(FVECTOR), N_PRIMS * N_VERTS );

	return 0;
}


static int BreakBOX_Act( Work *pWork )
{
	DG_PRIM2_UVRGBWH *pUvrgbwh;
	DG_PRIM2_UVRGBWH *pUvrgbwh_pre;
	FMATRIX world;
	FVECTOR pos;
	SVECTOR	rot;
	SVECTOR	rot_wide;
	FVECTOR	color;
	float   min_color;
	float	max_color;
	float	tmp;
	int       i;

	pUvrgbwh     = pWork->prim->uvrgb[ pWork->prim->buffer_clock ];
	pUvrgbwh_pre = pWork->prim->uvrgb[ 1 - pWork->prim->buffer_clock ];
	// 点滅させる
	if( pWork->count < 40 ){
		pWork->count++;
		min_color = LampColor.vx;
		max_color = LampColor.vy;
		// 色を設定
		if(pWork->interval == 0){
			pWork->color.vy += 220.0F * pWork->sign ;
			if(pWork->color.vy >= max_color ){
				pWork->color.vy = max_color ;
				if(pWork->sign == 1){
					pWork->interval = 1 ;
					pWork->sign = -1 ;
				}
			}

			if(pWork->color.vy <= min_color){
				pWork->color.vy = min_color ;
				if(pWork->sign == -1){
					pWork->interval = irnd() % 20 ;
					pWork->sign = 1 ;
				}
			}
			tmp = (LampColor.vz - min_color) * (pWork->color.vy - min_color) / (max_color - min_color);
			pWork->color.vz = min_color + tmp;
		}else{
			pWork->interval--;
		}
	}else{
	    // 最後に火花
		pWork->color = LampColor;
		pWork->act = (void *)BreakBOX_Act2;

		// 火花のエフェクトを飛ばす
		rot_wide.vx = 376;
		rot_wide.vy = 376;
		rot_wide.vz = 0;
		color.vx = 255.0F;
		color.vy = 128.0F;
		color.vz = 128.0F;
		color.vw = 50.0F;
		// 座標計算
		if ( pWork->attach_flg == ATTACH_ON ){
			world = pWork->prim->as_world;
			WorldPos( &pos , &world , &pWork->pos );
			rot.vx = pWork->ctrl->rot.vx + pWork->rot.vx;	
			rot.vy = pWork->ctrl->rot.vy + pWork->rot.vy;	
			rot.vz = pWork->ctrl->rot.vz + pWork->rot.vz;	
		} else {
			pos = pWork->pos;
			rot = pWork->rot;	
		}
		NewSpark1(24,&pos,36.0F,36.0F,0.0F,&rot,&rot_wide,&color,0.5F,20);
		DG_SetTmpLight2(
			&pos,
			400.0f,
			800.0f,
			80 | 40<<8 | 40 <<16,
			LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY ) ;
	}
	for ( i = 0 ; i < N_VERTS * N_PRIMS ; i++ ){
		ColorTrans( pUvrgbwh , pWork );
		pUvrgbwh++;
		pUvrgbwh_pre++;
	}
	// ランプの移動
	PrimLampMove( pWork );

	return 0 ;
}

// 破壊時の動作 (後編)
static int BreakBOX_Act2( Work *pWork )
{
	DG_PRIM2_UVRGBWH *pUvrgbwh;
	DG_PRIM2_UVRGBWH *pUvrgbwh_pre;
	float	  min_color,max_color;
	float	  tmp;
	int         i;

	min_color = 0.0F;
	max_color = LampColor.vy ;

	pUvrgbwh     = pWork->prim->uvrgb[ pWork->prim->buffer_clock ];
	pUvrgbwh_pre = pWork->prim->uvrgb[ 1 - pWork->prim->buffer_clock ];
	// 色を設定
	pWork->color.vy -= 3.0F ;
	if(pWork->color.vy > min_color){
		tmp =  (LampColor.vz - min_color) * (pWork->color.vy - min_color) / (max_color - min_color) ;
		pWork->color.vz = min_color + tmp ;
		tmp =  (LampColor.vx - min_color) * (pWork->color.vy - min_color) / (max_color - min_color) ;
		pWork->color.vx = min_color + tmp ;
	}else{
		pWork->color.vx = min_color;
		pWork->color.vy = min_color;
		pWork->color.vz = min_color;
		pWork->act	  = (void *)BOX_NONE_Act;
	}
	for ( i = 0 ; i < N_VERTS * N_PRIMS ; i++ ){
		ColorTrans( pUvrgbwh , pWork );
		pUvrgbwh++;
		pUvrgbwh_pre++;
	}
	// ランプの移動
	PrimLampMove( pWork );
	
	return 0 ;
}

// 何もしないときの動作
static int BOX_NONE_Act( Work *pWork )
{
	GV_DestroyActor( pWork );
	return 0 ;
}

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;

	switch ( signal ){
	case 0x2 :
		pWork->act = ( void * )BreakBOX_Act;
		break;
	case 0x4 :
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

static void Act( Work *pWork )
{
	DG_SwitchBuffPrim2( pWork->prim );
	pWork->act( pWork );
}

//  普通時の動作
static int NormalAct( Work *pWork )
{
	DG_PRIM2_UVRGBWH *pUvrgbwh;
	DG_PRIM2_UVRGBWH *pUvrgbwh_pre;
	float	 min_color;
	float	 max_color;
	int      alpha;
	int        i;

	pUvrgbwh     = pWork->prim->uvrgb[ pWork->prim->buffer_clock ];
	pUvrgbwh_pre = pWork->prim->uvrgb[ 1 - pWork->prim->buffer_clock ];
	/* 色の初期 */
	min_color = LampColor.vx;
	max_color = LampColor.vy;

	/* 色を決める */
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		alpha = ( int )pUvrgbwh_pre->a;
		if(pWork->interval == 0){
			// color
			if ( pWork->sign < 0 ){
				alpha -= 1;
				if ( alpha <= 8 ){
					alpha = 8;
					if( i == 0 ){
						pWork->interval = INTERVAL;
						pWork->sign = 1;
					}
				}
			} else {
				alpha += 1;
				if ( alpha > pWork->alpha_max[ i ] ){
					alpha = pWork->alpha_max[ i ];
					if( i == 0 ){
						pWork->interval = TOP_INTERVAL;
						pWork->sign = -1;
					}
				}
			}
			pWork->color.vw = ( u_char )alpha;
		}else{
			pWork->interval--;
			pWork->color.vw = ( u_char )alpha;
		}
		ColorTrans( pUvrgbwh , pWork );
		pUvrgbwh++;
		pUvrgbwh_pre++;
	}
	// ランプの移動
	PrimLampMove( pWork );

	return 0 ;
}

static void Die( Work *pWork )
{
	if ( pWork->prim != NULL ){
		GM_FreePrim2( pWork->prim );
	}
}

static int GetResources( Work *pWork , CONTROL *ctrl , FVECTOR *pPos , SVECTOR *rot , int attach_flg , int attach_put , int where )
{
	DG_TEX	             *tex;
	DG_PRIM2	        *prim;
	DG_PRIM2_UVRGBWH *uvrgbwh;
	FVECTOR			     *pos;
	FVECTOR  default_pos = { -57.5F , 117.0F , 125.0F , 1.0F };
	int                  itmp;
	int				 	 i, k;
	

	pWork->map = where;
	GM_CurrentMap = where;
	GM_SetCurrentMap( where );    // カレントマップの設定

	pWork->sign = 1;
	pWork->interval = 0;
	pWork->count = 0;
	pWork->attach_flg = attach_flg;
	pWork->attach_put = attach_put;
	// シグナルの登録
	GV_SetActorSignalFunc( pWork , ReceiveSignal );

	/* テクスチャ取得 */
	tex = DG_GetTexture( LAMP_TEX );

	prim = pWork->prim = GM_MakePrim2( LAMP_FLAG , N_PRIMS , N_VERTS );
	if ( prim == NULL ){
		return ( -1 );
	}
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim , SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE_LAMP;
	// 配置場所
	pWork->rot = *rot;
	DG_SetPos2(pPos , rot);	// 位置決め
	DG_PutVector(&default_pos , &pWork->pos , 1);
	pWork->ctrl = ctrl;
    // act登録
	pWork->act = (void *)NormalAct;
	// 色の初期化
	pWork->color.vx = 32;// LampColor.vx;
	pWork->color.vy = 164;// (LampColor.vy + LampColor.vx) * 0.5F;
	pWork->color.vz = 100;// (LampColor.vz + LampColor.vx) * 0.5F;
	pWork->color.vw = 48.f;
	pWork->alpha_max[ 0 ] = 32;
	pWork->alpha_max[ 1 ] = 16;
	pWork->alpha_max[ 2 ] = 8;
	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	itmp = 100;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos , &pWork->pos );
			/* ＵＶ値は1.3.12の固定小数点なので注意！ */
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;
			/* ＷＨ値は整数なので注意！ */
			uvrgbwh->w = itmp;
			uvrgbwh->h = itmp;
			uvrgbwh->r = pWork->color.vx;
			uvrgbwh->g = pWork->color.vy;
			uvrgbwh->b = pWork->color.vz;
			uvrgbwh->a = pWork->color.vw;
			pWork->color.vw /= 2;
			pos++;
			uvrgbwh++;
		}
		itmp *= 1.4f; 
	}
	_MemCopy( prim->pos[ 0 ],   MEM_ADDR1, sizeof(FVECTOR),          N_VERTS * N_PRIMS ) ;
	_MemCopy( prim->pos[ 1 ],   MEM_ADDR1, sizeof(FVECTOR),          N_VERTS * N_PRIMS ) ;
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_VERTS * N_PRIMS ) ;
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_VERTS * N_PRIMS ) ;

	pWork->prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
   pWork->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;

	return ( 0 );
}

void *NewIrs_Lamp( CONTROL *ctrl , FVECTOR *pPos , SVECTOR *pRot , int attach_flg , int attach_put , int where )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die ) ;
		GV_ActorEX( &pWork->actor );
		if ( GetResources( pWork , ctrl , pPos , pRot , attach_flg , attach_put , where ) < 0 ) {
			GV_DestroyActor( &pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

