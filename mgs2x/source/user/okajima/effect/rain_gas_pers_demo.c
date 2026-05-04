//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	霧雨（透視変換バージョン）：デモ用

	2000/01/18 S.Okajima
	$Id: rain_gas_pers_demo.c,v 1.1.1.3 2002/11/19 11:47:10 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

extern float	OK_slow_param;

extern int big_thund_flash_flag ;

extern	int		G_wind_intense;
extern	SVECTOR	G_wind_rot;

SVECTOR	OK_rain_gas_rot_pers;

/*----------------------------------------------------------------*/

#define	SCREEN_NEAR			( 51.0f )

#define	POS_VS_UVS	(1 + 1 + 2)

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

#define	PARAM1		(MAX_FVECTOR / POS_VS_UVS)
#define	PARAM2		(PARAM1 * 16)

#define	SCR_SPACE	(SCRPAD_ADDR)
#define	SCR_POS		(SCRPAD_ADDR + PARAM2)
#define	SCR_UVS		(SCRPAD_ADDR + PARAM2 + PARAM2 )



/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
//#define	N_VERTS2			(32)
//#define	N_VERTS2			(24)
#define	N_VERTS2			(16)
/* スクラッチパットぎりぎりまでとる（uvrgba も 同時に転送するので1/(1+2） */
#define	N_PRIMS2			(1)



/* 画面前バンダリ */
#define	BOUND_WIDTH	(6000.0f)
/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE	(4500.0f)

#define	P_SIZE			(50.0f * CENTER_DISTANCE)
#define	P_ALPHA_MAX			(16)
#define	P_ALPHA_MAX_SLOW	(48)
#define	P_ALPHA_ADD		(4)
#define	P_ALPHA_SUB		(2)
#define	P_RGB_MAX		(96)

#define	WIN_MIN			(200.0f)

#define	FLASH_COL		(16)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;

	int		on_flag;

	int		one_time;
	int		stable_continue;
	int		rand_continue;
	int		before_col;

	CVECTOR		col;
	FVECTOR bound_0;
	FVECTOR bound_1;

	FVECTOR space_pos[N_VERTS2 * N_PRIMS2];

	FMATRIX		*world;

	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i,j;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	fvtemp;
	FVECTOR	*pos;
	FVECTOR	*space;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	b_dis;
	FVECTOR wind;
	float	screen_near_x;
	float	screen_near_y;
	float	alpha_max;
	int	dest=0;
	GV_MSG *msg;
	int mes_num;
	int num;


	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			work->on_flag = 0 ;
			break;
		  case 1:
			work->on_flag = 1 ;
			break;
		  case 2:
		  case -1:
			GV_DestroyActor( work ) ;
			break;
		  default:
			break;
		}
		msg--;
	}

	if( GM_CheckPlayerStatus(PLAYER_CB_BOX) && GM_CheckPlayerStatus(PLAYER_WATCH) ){
		DG_InvisiblePrim2( work->prim );
		return;
	}else{
		if( !work->on_flag ){
			DG_InvisiblePrim2( work->prim );
			return;
		}
		DG_VisiblePrim2( work->prim );
	}

	if( work->one_time==0 && big_thund_flash_flag==1 ){
		extern void *NewFlush( int decay, int mode );
		NewFlush( 2, 0 );
	}
	work->one_time = big_thund_flash_flag;


	wind.vx=0.0f;
	wind.vz=0.0f;
	if( G_wind_intense*0.5f > WIN_MIN){
		wind.vy = -G_wind_intense*0.5f;
	}else{
		wind.vy = -WIN_MIN;
	}

//	OK_DirectionSmoother( &OK_rain_gas_rot_pers, &G_wind_rot, 0.995f );
	OK_DirectionSmoother( &OK_rain_gas_rot_pers, &G_wind_rot, 0.999f - 0.04f*OK_slow_param );



	DG_SetPos2( &DG_ZeroVector, &OK_rain_gas_rot_pers );
	DG_PutVector( &wind, &wind, 1 );

	wind.vy *= 0.25f;
	_sceVu0ScaleVector( &wind, &wind, OK_slow_param );


	/* カメラのまえのバンダリをつくる */
	b_dis.vx=0.0f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );
	_sceVu0AddVector( &bound_0, &b_dis, &work->bound_0 ) ;
	_sceVu0AddVector( &bound_1, &b_dis, &work->bound_1 ) ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	OK_Mem_Scr( SCR_SPACE, work->space_pos,            sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Mem_Scr( SCR_POS,   work->prim->pos[ clock ],   sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Mem_Scr( SCR_UVS,   work->prim->uvrgb[ clock ], sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2) ;

	space   = SCR_SPACE;
	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;
	if( big_thund_flash_flag ){
		if( work->rand_continue > 0 ){
			work->rand_continue--;
			work->before_col=(int)((float)FLASH_COL*0.2f*rnd() + (float)FLASH_COL*0.8f);
		}else{
			if( work->stable_continue > 0 ){
				work->stable_continue--;
			}else{
				work->stable_continue = irnd() % 128 + 4;
				work->rand_continue   = irnd() % 8 + 4;
			}
			dest=FLASH_COL;
			work->before_col=dest;
		}

		for ( i = 0 ; i < N_PRIMS2 ; i++ ){
			for ( j = 0 ; j < N_VERTS2 ; j++ ){
				uvrgbwh->a = work->before_col ;
				if( !vu0_CheckBoundingBox( space, &bound_0, &bound_1 ) ){
					fvtemp.vx = (float)( BOUND_WIDTH * 2.0f * rnd() );
					fvtemp.vy = (float)( BOUND_WIDTH * 2.0f * rnd() );
					fvtemp.vz = (float)( BOUND_WIDTH * 2.0f * rnd() );
					_sceVu0AddVector( space, &bound_1, &fvtemp );
				}

				_sceVu0AddVector( space, space, &wind );

				space++;
				uvrgbwh++;
			}
		}
	}else{
		alpha_max = (float)work->col.cd;


		for ( i = 0 ; i < N_PRIMS2 ; i++ ){
			for ( j = 0 ; j < N_VERTS2 ; j++ ){
				if( vu0_CheckBoundingBox( space, &bound_0, &bound_1 ) ){
					/* バンダリ内だと明るくする */
					uvrgbwh->a += P_ALPHA_ADD ;
					if( uvrgbwh->a > alpha_max ) uvrgbwh->a = alpha_max ;
				}else{
					if( uvrgbwh->a >  P_ALPHA_SUB ){ // 引きすぎないようにする
						/* バンダリ外だと暗くする */
						uvrgbwh->a -= P_ALPHA_SUB ;
					}else{
						/* これ以上暗く出来ないので位置・明るさを初期化する */
						fvtemp.vx = (float)( BOUND_WIDTH * 2.0f * rnd() );
						fvtemp.vy = (float)( BOUND_WIDTH * 2.0f * rnd() );
						fvtemp.vz = (float)( BOUND_WIDTH * 2.0f * rnd() );
						_sceVu0AddVector( space, &bound_1, &fvtemp );
					}
				}

				_sceVu0AddVector( space, space, &wind );

				space++;
				uvrgbwh++;
			}
		}
	}
	
	/* 以上で３Ｄ空間の点の移動は終わり */

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	/* 座標を全部透視変換する */
	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( SCR_SPACE, SCR_POS, N_VERTS2 * N_PRIMS2 );

	space   = SCR_SPACE;
//if(GV_Time%60==0 )printf("a:%6.4f %6.4f  %6.4f  %6.4f \n",space->vx,space->vy,space->vz,space->vw);
	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( j = 0 ; j < N_VERTS2 ; j++ ){


//if(GV_Time%60==0 && i==0 && j==0)printf("b:%6.4f %6.4f  %6.4f  %6.4f \n",pos->vx,pos->vy,pos->vz,pos->vw);

			if( pos->vz < pos->vw ){
				pos->vw = (pos->vw  > 0.0f)? pos->vw: -pos->vw;

/*
			space->vw=1.0f;
if(GV_Time%60==0 && i==0 && j==0)printf("a:%6.4f %6.4f %6.4f %6.4f \n",space->vx,space->vy,space->vz,space->vw);
if(GV_Time%60==0 && i==0 && j==0)printf("b:%6.4f %6.4f %6.4f %6.4f \n",pos->vx,pos->vy,pos->vz,pos->vw);
if(GV_Time%60==0 && i==0 && j==0)printf("c:%6.4f %6.4f %6.4f \n",pos->vx/pos->vw,pos->vy/pos->vw,pos->vz/pos->vw);
*/

				uvrgbwh->h = uvrgbwh->w = P_SIZE  / pos->vw;
				pos->vz = SCREEN_NEAR;
				pos->vx = screen_near_x * pos->vx / pos->vw;
				pos->vy = screen_near_y * pos->vy / pos->vw;

//				pos->vx = 51.0f * ASPECT_X() * pos->vx / pos->vw *0.5f;
//				pos->vy = 51.0f / (ASPECT_Y() * 512.0f / 224.0f ) * pos->vy / pos->vw *0.5f;

//				AN_Test_Eye2( space, 2 );
			}else{
				uvrgbwh->h = uvrgbwh->w = 0 ;
				DG_COPY_VEC( pos, &DG_ZeroVector );
			}

			space++;
			pos++;
			uvrgbwh++;
		}
	}


	OK_Scr_Mem( work->space_pos,            SCR_SPACE, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( work->prim->pos[ clock ],   SCR_POS,   sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( work->prim->uvrgb[ clock ], SCR_UVS,   sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;
	int		col_r;
	int		col_g;
	int		col_b;
	int		col_a;

	col_r = work->col.r;
	col_g = work->col.g;
	col_b = work->col.b;
	col_a = work->col.cd;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos     = SCR_POS ;
	uvrgbwh = SCR_UVS ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			pos->vx = 0.0f ;
			pos->vy = 0.0f ;
			pos->vz = 0.0f ;

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = 0 ;
			uvrgbwh->h = 0 ;

			uvrgbwh->r = col_r ;
			uvrgbwh->g = col_g ;
			uvrgbwh->b = col_b ;
			uvrgbwh->a = 0 ;

			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh ++ ;
		}
	}

	OK_Scr_Mem( work->space_pos,  SCR_POS, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
//	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
//	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;


	return 1;
}

static int GetResources( Work *work )
{
	FVECTOR		*pos;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int	i;

	work->on_flag = 1 ;

/*
printf("FVECTOR:%d\n",sizeof(FVECTOR));
printf("DG_PRIM2_UVRGB:%d\n",sizeof(DG_PRIM2_UVRGB));
printf("DG_PRIM2_UVRGBWH:%d\n",sizeof(DG_PRIM2_UVRGBWH));
*/

	/* 表示範囲 */
	work->bound_0.vx =  BOUND_WIDTH ;
	work->bound_0.vy =  BOUND_WIDTH ;
	work->bound_0.vz =  BOUND_WIDTH ;
	work->bound_1.vx = -BOUND_WIDTH ;
	work->bound_1.vy = -BOUND_WIDTH ;
	work->bound_1.vz = -BOUND_WIDTH ;

	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS2, N_VERTS2 );
//	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	work->one_time = 0;

	pos = work->space_pos;
	for ( i = 0 ; i < N_PRIMS2*N_VERTS2 ; i++ ){
		pos->vx = (float)( BOUND_WIDTH * 2.0f * rnd() );
		pos->vy = (float)( BOUND_WIDTH * 2.0f * rnd() );
		pos->vz = (float)( BOUND_WIDTH * 2.0f * rnd() );
		pos++;
	}

	return 0 ;
}

void *NewRainFogPersDemo( int name, u_int col )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->col.r = (col>>24)&0xff;
		work->col.g = (col>>16)&0xff;
		work->col.b = (col>>8)&0xff;
		work->col.cd= (col)&0xff;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
