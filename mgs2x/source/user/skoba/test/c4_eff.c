//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	Ｃ４ランプエフェクト  main.c
	2001/03/23 S.Kobayashi
	$Id: c4_eff.c,v 1.1.1.3 2002/11/19 11:50:24 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#ifndef KP_XBOX
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
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include    "etc.h"

#ifdef KP_XBOX	// 遮蔽計算を切る
#define GM_SeSetMode( a, b, c ) GM_SeSetModeAddr( a, b, c, GM_INVALID_ADDR )
#endif


/* ---------------------------------------------------------------- */
/* ポリゴン＆ラインの最大頂点数は６４まで！！ */
#define N_PRIMS1	(5)
#define N_VERTS1	(4)

/* ポリゴン＆ラインの最大頂点数は６４まで！！ */
#define N_PRIMS2	(21)
#define N_VERTS2	(16)

/* 通常スプライトの最大頂点数は３２まで！！ */
/* 回転スプライトの最大頂点数は１６まで！！ */
#define N_PRIMS3	(1)
#define N_VERTS3	(1)

#define N_PRIMS_ICE	(1)
#define N_VERTS_ICE	(1)

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))
#define SPRITE_FLAG (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA)
#define ALPHA (128)
#define LAMP_TEX (2998348) // xlit04a_alp.pic 

#define VISIBLE_FLAG    ( 0x2 )
#define INVISIBLE_FLAG  ( 0x4 )
#define FLUSH_COLD (0x10)

/* state */
#define LAMP_STATE_LIKE_CEILING	(0x00000001)

enum {
	SK_FATMAN_MODE = 0x100 ,
};

static FVECTOR LampColor = {250.0F, 100.0F, 255.0F, 128.0F}; // ランプの色

static FVECTOR default_pos = { -38.0F , 93.0F , 133.0F };

typedef	struct _work {
	GV_ACT_EX	actor;
	DG_PRIM2	*prim;

	FMATRIX   *pWorld;
	FVECTOR       pos;	
	FVECTOR     speed; 
	int		      map;
	int           num;
	float        time;
	int         state;

	float       angle;
	float      invers;
	FVECTOR     color;
	int         count;
	FVECTOR max_color;
	char     	*pStatus;
	char      	*pLevel;
	int			*pCount ;
	int			*pNo ;

	int        se_num;

	int          name;
    int     demo_flag;
	int          flag;
	void ( *act )(struct _work *);
	FVECTOR		*shift_pos ;
} Work;

static FVECTOR SK_max_color = { 0.0f , 255.0f , 128.0f , 82.0f };

static	void	CheckMessage( Work *pWork )
{
	int			n_msg ;
	GV_MSG		*msg ;

	if ( pWork->name == 0 ){ // 通常は処理しない
		return;
	}
	n_msg = GV_ReceiveMessage( pWork->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		if( msg->message[ 0 ] == VISIBLE_FLAG ){
			// default invisible
			DG_VisiblePrim2( pWork->prim );
			pWork->demo_flag = VISIBLE_FLAG;
		}
		if( msg->message[ 0 ] == INVISIBLE_FLAG ){
			DG_InvisiblePrim2( pWork->prim );
			pWork->demo_flag = INVISIBLE_FLAG;
		}
		msg ++ ;
	}
}

static void ColorTrans( DG_PRIM2_UVRGBWH *pUvrgbwh , Work *pWork )  // 色情報の転送＋ライトの設定
{
	int i;

	pWork->color.vx = pWork->color.vx < 0.0f ? 0.0f : pWork->color.vx; 
	pWork->color.vy = pWork->color.vy < 0.0f ? 0.0f : pWork->color.vy; 
	pWork->color.vz = pWork->color.vz < 0.0f ? 0.0f : pWork->color.vz; 
	pWork->color.vw = pWork->color.vw < 0.0f ? 0.0f : pWork->color.vw; 
	// スイッチのエフェクト
	for ( i = 0 ; i < N_VERTS3 ; i++ ){
		pUvrgbwh[ i ].r = ( u_char )pWork->color.vx;
		pUvrgbwh[ i ].g = ( u_char )pWork->color.vy;
		pUvrgbwh[ i ].b = ( u_char )pWork->color.vz;
		pUvrgbwh[ i ].a = ( u_char )pWork->color.vw;
	}
}

static void FreezeAct( Work *pWork )
{
	DG_PRIM2_UVRGBWH *pUvrgbwh;
	FVECTOR              *pos;
	int                 clock;

	clock = pWork->prim->buffer_clock;
	pUvrgbwh = pWork->prim->uvrgb[clock];
	pos = pWork->prim->pos[clock];
	/* 色を決める */
	pWork->color.vw -= 1.0f;

	WorldPos( pos , pWork->pWorld , pWork->shift_pos ); // ランプの場所を決める

	ColorTrans( pUvrgbwh , pWork );

	if ( pWork->color.vw == 0 ){
		GV_DestroyActor( pWork );
	}
}

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;
	float tmp;

	switch ( signal ){
	case 0x2 :
		if ( pWork->pStatus == NULL || *pWork->pStatus & 0x1 ){
			pWork->count++;
			if ( pWork->flag & SK_FATMAN_MODE ){
				tmp = ( 255.0f / DIRECT_TICK(180.0f)) / 14.0f;
			} else {
				tmp = ( 255.0f / DIRECT_TICK(180.0f));
			}
			pWork->max_color.vx += tmp;
			pWork->max_color.vy -= tmp;
			pWork->max_color.vz -= tmp;
		}
		break;
	case 0x4 :
		pWork->color.vx = 255.0f;
		pWork->color.vy = 0.0f;
		pWork->color.vz = 0.0f;
		GM_SeSetMode( SD_A_FREEZE02, (FVECTOR *)pWork->pWorld->m[ 3 ] , GM_SEMODE_BOMB ) ; // ピキーン
		pWork->act = ( void * )FreezeAct;
		break;
	case FLUSH_COLD :
		pWork->color.vw = 0.0f;		
		pWork->act = ( void * )FreezeAct;
		break;
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
	CheckMessage( pWork );
	pWork->act( pWork );
}

static void NormalAct( Work *pWork )
{
	DG_PRIM2_UVRGBWH *pUvrgbwh;
	FVECTOR              *pos;
	float	        min_color;
	float	        max_color;
	int                 clock;
	float                 rad; // ラジアン

	clock    = pWork->prim->buffer_clock;
	pUvrgbwh = pWork->prim->uvrgb[ clock ];
	pos      = pWork->prim->pos[ clock ];
	// 色の初期
	min_color = 0;
	max_color = 255;

	/* 色を決める */
	rad = pWork->angle * PI / 180.f;
	pWork->angle += pWork->invers;
	if ( pWork->angle > 180 ){
		pWork->angle = 180.f;
		pWork->invers *= -1.f;
        {
			/* プレイヤーに近かったらＳＥ */
			if ( GM_PlayerBody != NULL && pWork->demo_flag == VISIBLE_FLAG ) {
				FVECTOR		plpos ;

				GV_MatToVec( &GM_PlayerBody->objs->objs[ HUMAN21_KUBI ].world, &plpos ) ;
				if ( GV_VecLen3F2( &plpos, ( FVECTOR * )pWork->pWorld->m[ 3 ] ) < 2000.0F ) {
					GM_SeSetMode( SD_A_C4LEDBR1, (FVECTOR *)pWork->pWorld->m[ 3 ] , GM_SEMODE_BOMB ) ;
				}
			}

        }
		//GM_SeSetMode( SD_A_C4LEDBR1, (FVECTOR *)pWork->pWorld->m[ 3 ] , GM_SEMODE_BOMB ) ;
	} else if ( pWork->angle < 0 ){
		pWork->angle = 0.f;
		pWork->invers *= -1.f;
	}
	pWork->color.vx = pWork->max_color.vx * sinf( rad );
	pWork->color.vy = pWork->max_color.vy * sinf( rad );
	pWork->color.vz = pWork->max_color.vz * sinf( rad );
	pWork->color.vw = pWork->max_color.vw * sinf( rad );

	WorldPos( pos , pWork->pWorld , pWork->shift_pos ); // ランプの場所を決める

	ColorTrans( pUvrgbwh , pWork );

	if( pWork->state & LAMP_STATE_LIKE_CEILING ) {
		int flag;
		extern int	KR_CeilingCheck( float height ) ;
		flag = KR_CeilingCheck( pWork->pWorld->m[3][1] ) ;
		if( flag == 0 ){
			/* 可視化 */
			DG_VisiblePrim2( pWork->prim );
		} else {
			/* 不可視化 */
			pWork->prim->flag |= flag;
		}
	}
}

static void FattmanAct( Work *pWork )
{
	DG_PRIM2_UVRGBWH *pUvrgbwh;
	FVECTOR              *pos;
	float	        min_color;
	int                 clock;
	int                   tmp;
	float                 rad; // ラジアン

	clock    = pWork->prim->buffer_clock;
	pUvrgbwh = pWork->prim->uvrgb[ clock ];
	pos      = pWork->prim->pos[ clock ];
	// 色の初期
	min_color = 0;
	/* 色を決める */
	rad = pWork->angle * PI / 180.f;
	pWork->angle += pWork->invers;
	if ( pWork->angle > 180 ){
		pWork->angle = 180.f;
		pWork->invers *= -1.f;
#if 0
		if ( ( *pWork->pStatus & 0x2 || *pWork->pStatus & 0x1 ) && pWork->demo_flag == VISIBLE_FLAG ){// サウンド
          static int seNos[] = { SD_E_C4TIME11, SD_E_C4TIME12, SD_E_C4TIME13 } ;
	      int		 seNo ;														  
          if ( *pWork->pStatus & 0x4 ) seNo = seNos[ 1 ] ;
          else if ( *pWork->pStatus & 0x8 ) seNo = seNos[ 2 ] ;
          else seNo = seNos[ 0 ] ;															   
		  GM_SeSetMode( seNo, (FVECTOR *)pWork->pWorld->m[ 3 ] , GM_SEMODE_BOMB ) ;
		}
#endif
	} else if ( pWork->angle < 0 ){
		pWork->angle = 0.f;
		pWork->invers *= -1.f;
	}
	// 色だし
	tmp = ( int )*pWork->pLevel;
	tmp %= 15; // 補正一応
//	printf("tmp = %d\n",tmp);
//	printf("color = %f %f %f\n",pWork->max_color.vx,pWork->max_color.vy,pWork->max_color.vz);
	pWork->color.vx = pWork->max_color.vx * tmp;
	pWork->color.vy = pWork->max_color.vy * tmp;
	pWork->color.vz = pWork->max_color.vz * tmp;
	pWork->color.vw = pWork->max_color.vw * tmp;
//	printf("%f %f\n",pWork->max_color.vx,pWork->max_color.vy);
	WorldPos( pos , pWork->pWorld , pWork->shift_pos ); // ランプの場所を決める

	ColorTrans( pUvrgbwh , pWork );

	/* 音ならし */
	if ( pWork->pNo == NULL || pWork->pCount == NULL ) return ;
	{
		int 	SeSet[] = { SD_E_C4TIME12, SD_E_C4TIME14, SD_E_C4TIME22,
							SD_E_C4TIME24, SD_E_C4TIME32, SD_E_C4TIME34 } ;
		int 	SeSet2[] = { SD_E_C4TIME11, SD_E_C4TIME13, SD_E_C4TIME21,
							 SD_E_C4TIME23, SD_E_C4TIME31, SD_E_C4TIME33 } ;
		int		TimeAdj[] = { 0, 30, 15, 45, 8, 23 } ;
		int			n ;

		n = *pWork->pNo % 6 ;
		if ( ( ( GV_Time - TimeAdj[ n ] ) % DIRECT_TICK( 60 ) ) == 0 ) {
			if ( *pWork->pCount > DIRECT_TICK( 60 * 10 ) ) {
				//printf( "[%d] se1 %d\n", GV_Time, *pWork->pNo % 6 ) ;
				GM_SeSetMode( SeSet[ n ], (FVECTOR *)pWork->pWorld->m[ 3 ] , GM_SEMODE_NORMAL ) ;
			} else {
				//printf( "[%d] se2 %d\n", GV_Time, *pWork->pNo % 6 ) ;
				GM_SeSetMode( SeSet2[ n ], (FVECTOR *)pWork->pWorld->m[ 3 ] , GM_SEMODE_NORMAL ) ;
			}
		}
	}
}

static void Die( Work *pWork )
{
	if ( pWork->prim != NULL ){
		GM_FreePrim2( pWork->prim );
	}
}

static int GetResources( Work *pWork , FMATRIX *pWorld , int where )
{
	DG_TEX	             *tex;
	DG_PRIM2	        *prim;
	DG_PRIM2_UVRGBWH *uvrgbwh;
	FVECTOR			     *pos;
	FVECTOR              ftmp;
	int				 	 i, k;
	
	pWork->map = where;
	GM_CurrentMap = where;

	pWork->state = 0 ;
	pWork->flag = 0;
	pWork->time = 0.F;
	pWork->invers = 10.f;
	pWork->angle = 0.f;
	pWork->count = 1;
	pWork->speed.vw = 50.F;
	pWork->demo_flag = VISIBLE_FLAG;
	// シグナルの登録
	GV_SetActorSignalFunc( pWork , ReceiveSignal );
	/* テクスチャ取得 */
	tex = DG_GetTexture( LAMP_TEX );
	if( tex == NULL ){
		printf("texture isn't! -----c4_eft.c\n");
		ASSERT( 0 );
	}
	prim = pWork->prim = GM_MakePrim2( SPRITE_FLAG , N_PRIMS3 , N_VERTS3 );
	if ( prim == NULL ){
		return ( -1 );
	}
	DG_ConfigPrim2Tex( prim, tex );
	prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	// 配置場所
	pWork->pWorld = pWorld;
	WorldPos(&ftmp , pWorld , pWork->shift_pos ); // ランプの場所を決める
    // act登録
	if ( pWork->pStatus == NULL || pWork->pLevel == NULL ){
		pWork->max_color.vx = 0.0f;
		pWork->max_color.vy = 255.0f;
		pWork->max_color.vz = 128.0f;
		pWork->max_color.vw = 82.0f;
		pWork->act = (void *)NormalAct;
	} else {
		switch ( *pWork->pStatus ){
		case 0x4 : // 2
			pWork->se_num = SD_E_C4TIME12;
			break;
		case 0x8 : // 3
			pWork->se_num = SD_E_C4TIME13;
			break;
		default  : // 1
			pWork->se_num = SD_E_C4TIME11;
			break;
		}
		_sceVu0DivVector( &pWork->max_color , &SK_max_color , 14 );
		vecprintf( &pWork->max_color );
		pWork->flag = SK_FATMAN_MODE;
		pWork->act = (void *)FattmanAct;
	}
	// 色の初期化
	pWork->color.vx = LampColor.vx;
	pWork->color.vy = LampColor.vz;
	pWork->color.vz = LampColor.vy;
	pWork->color.vw = LampColor.vw;
	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	for ( i = 0 ; i < N_PRIMS3 ; i++ ){
		for ( k = 0 ; k < N_VERTS3 ; k++ ){
			DG_COPY_VEC( pos , &ftmp );
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
			uvrgbwh->w = 64;
			uvrgbwh->h = 64;
			uvrgbwh->r = pWork->color.vx;
			uvrgbwh->g = pWork->color.vy;
			uvrgbwh->b = pWork->color.vz;
			uvrgbwh->a = ALPHA;
			pos++;
			uvrgbwh++;
		}
	}
	_MemCopy( prim->pos[ 0 ],   MEM_ADDR1, sizeof(FVECTOR),          N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->pos[ 1 ],   MEM_ADDR1, sizeof(FVECTOR),          N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_VERTS3 * N_PRIMS3 ) ;

	pWork->prim->flag |= ( DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return ( 0 );
}

void *NewC4_LampEX( FMATRIX *pWorld , int where , char *pStatus , 
				   char *pLevel , int name, FVECTOR *shift )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_EFFECT , sizeof( Work ) ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->pStatus = pStatus; 
		pWork->pLevel = pLevel; 
		pWork->name = name; 
		pWork->shift_pos = shift ;
		if ( GetResources( pWork , pWorld , where ) < 0 ) {
			GV_DestroyActor( &pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

void *NewC4_Lamp( FMATRIX *pWorld , int where , char *pStatus , char *pLevel , int name )
{
	return NewC4_LampEX( pWorld, where, pStatus, pLevel, name, &default_pos ) ;
}

void *NewC4_LampCeiling( FMATRIX *pWorld , int where , char *pStatus ,char *pLevel , int name, FVECTOR *shift)
{
	Work	*pWork ;

	pWork = (Work *)NewC4_LampEX( pWorld, where, pStatus, pLevel, name, shift ) ;
	if ( pWork != NULL ) {
		pWork->state |= LAMP_STATE_LIKE_CEILING ;
	}

	return (void *)pWork ;
}

void	PL_AddC4LampParam( void *ptr, int *Count, int *No )
{
	Work	*work ;

	work = ( Work * )ptr ;
	work->pCount = Count ;
	work->pNo = No ;
}
