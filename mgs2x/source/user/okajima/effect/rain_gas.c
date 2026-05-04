//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rain_gas.c
	霧雨

	1999/09/29 S.Okajima
	$Id: rain_gas.c,v 1.1.1.3 2002/11/19 11:47:10 Yoshizawa1 Exp $
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

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

extern int big_thund_flash_flag ;   

extern	int		G_wind_intense;
extern	SVECTOR	G_wind_rot;

SVECTOR	OK_rain_gas_rot;

/*----------------------------------------------------------------*/

#define	POS_VS_UVS	(1 + 2)

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

#define	PARAM1		(MAX_FVECTOR / POS_VS_UVS)
#define	PARAM2		(PARAM1 * 16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + PARAM2 )



/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
#define	N_VERTS2			(16)
/* スクラッチパットぎりぎりまでとる（uvrgba も 同時に転送するので1/(1+2） */
//#define	N_PRIMS2			(PARAM1/N_VERTS2)
#define	N_PRIMS2			(1)



/* 画面前バンダリ */
#define	BOUND_WIDTH	(5000.0f)
/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE	(4000.0f)

#define	P_SIZE			(2500.0f)
#define	P_ALPHA_MAX		(128)
#define	P_ALPHA_ADD		(32)
#define	P_ALPHA_SUB		(16)
#define	P_RGB_MAX		(16)

#define	WIN_MIN			(200)

#define	FLASH_COL		(40)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;


	int		rand_count;
	int		rand_continue;
	int		before_col;

	FVECTOR wind;

	FVECTOR bound_0;
	FVECTOR bound_1;


	FMATRIX		*world;

	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i,j;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	fvtemp;
	FVECTOR	*pos;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	b_dis;

   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }

	clock = 1 - work->prim->buffer_clock;

	work->wind.vx=0.0f;
	work->wind.vz=0.0f;
	if( G_wind_intense*0.5f > WIN_MIN){
//if(GV_Time%10==0)printf("a1\n");
		work->wind.vy = -G_wind_intense*0.5f;
	}else{
//if(GV_Time%10==0)printf("a2\n");
		work->wind.vy = -WIN_MIN;
	}

	OK_DirectionSmoother( &OK_rain_gas_rot, &G_wind_rot, 0.995f );

	DG_SetPos2( &DG_ZeroVector, &OK_rain_gas_rot );

	DG_PutVector( &work->wind, &work->wind, 1 );
//if(GV_Time%10==0)printf("%f %f %f\n",work->wind.vx,work->wind.vy,work->wind.vz);


	b_dis.vx=0.0f;
//	b_dis.vy=-CENTER_DISTANCE*0.5f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE;

	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );
	_sceVu0AddVector( &bound_0, &b_dis, &work->bound_0 ) ;
	_sceVu0AddVector( &bound_1, &b_dis, &work->bound_1 ) ;
	DG_SwitchBuffPrim2( work->prim );

	OK_Mem_Scr( SCR_POS, work->prim->pos[ clock ],   sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Mem_Scr( SCR_UVS, work->prim->uvrgb[ clock ], sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;

	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;

	if( big_thund_flash_flag ){
		if( work->rand_continue > 0 ){
			work->rand_continue--;
		}else if( work->rand_continue == 0 ){
			work->rand_count = irnd() % 4 + 4;
		}

		if( irnd()%8==0 ){
			work->before_col=(int)((float)FLASH_COL*0.1f*rnd() + (float)FLASH_COL*0.9f);
		}else{
			int	dest=0;
			OK_IntSmoother( &work->before_col, &dest, 0.9f );
		}

		for ( i = 0 ; i < N_PRIMS2 ; i++ ){
			for ( j = 0 ; j < N_VERTS2 ; j++ ){
				uvrgbwh->a = work->before_col ;
				if( vu0_CheckBoundingBox( pos, &bound_0, &bound_1 ) ){
					pos->vx += work->wind.vx;
					pos->vy += work->wind.vy*0.25f;
					pos->vz += work->wind.vz;
				}else{
					fvtemp.vx = (float)( BOUND_WIDTH * 2.0f * rnd() );
					fvtemp.vy = (float)( BOUND_WIDTH * 2.0f * rnd() );
					fvtemp.vz = (float)( BOUND_WIDTH * 2.0f * rnd() );
					_sceVu0AddVector( pos, &bound_1, &fvtemp );
				}
				pos++;
				uvrgbwh++;
			}
		}
	}else{
		for ( i = 0 ; i < N_PRIMS2 ; i++ ){
			for ( j = 0 ; j < N_VERTS2 ; j++ ){
				if( vu0_CheckBoundingBox( pos, &bound_0, &bound_1 ) ){
					uvrgbwh->a += P_ALPHA_ADD ;
					if( uvrgbwh->a > P_ALPHA_MAX ) uvrgbwh->a = P_ALPHA_MAX ;
					pos->vx += work->wind.vx;
					pos->vy += work->wind.vy*0.25f;
					pos->vz += work->wind.vz;
				}else{
					if( uvrgbwh->a >  P_ALPHA_SUB ){
						uvrgbwh->a -= P_ALPHA_SUB ;
					}else{
						fvtemp.vx = (float)( BOUND_WIDTH * 2.0f * rnd() );
						fvtemp.vy = (float)( BOUND_WIDTH * 2.0f * rnd() );
						fvtemp.vz = (float)( BOUND_WIDTH * 2.0f * rnd() );
						_sceVu0AddVector( pos, &bound_1, &fvtemp );
					}
				}
				pos++;
				uvrgbwh++;
			}
		}
	}

	OK_Scr_Mem( work->prim->pos[ clock ],   SCR_POS, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( work->prim->uvrgb[ clock ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, j, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

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

			uvrgbwh->w = P_SIZE ;
			uvrgbwh->h = P_SIZE ;

			uvrgbwh->r = P_RGB_MAX ;
			uvrgbwh->g = P_RGB_MAX ;
			uvrgbwh->b = P_RGB_MAX ;
			uvrgbwh->a = P_ALPHA_MAX ;

			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh ++ ;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

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

	tex = work->tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewRainFog( void )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
