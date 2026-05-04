//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rain_cm_con.c
	カメラ画面に飛び散る雨：任意点参照
	2001/04/04 S.Okajima
	$Id: rain_cm_con.c,v 1.1.1.3 2002/11/19 11:47:22 Yoshizawa1 Exp $
*/

/*

加算減算でそれぞれ dg_prim2 をもつ。
別テクスチャはこれらを別個にもつ。

２つのテクスチャを使用する場合、４つの dg_prim2 となる。

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
#include	"libmt.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
#define	RATIO		( 4096 )

#define	SCREEN_NEAR	( 51.0f )

#define	ADD_TRANS	(0)
#define	SUB_TRANS	(1)

/* 固定 */
#define	ADD_SUB		(2)

#define	MAX_TEX_NUM	(2)

#define	R_POS		(1)
#define	R_UVS		(2)

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

#define	PARAM1		(MAX_FVECTOR / (R_POS + R_UVS  +  R_POS + R_UVS))
#define	PARAM2		(PARAM1 * 16)

#define	SCR_POS0	(SCRPAD_ADDR)
#define	SCR_POS1	(SCR_POS0 + PARAM2)
#define	SCR_UVS0	(SCR_POS1 + PARAM2 )
#define	SCR_UVS1	(SCR_UVS0 + PARAM2 + PARAM2 )



/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
#define	N_VERTS2			(8)
#define	N_PRIMS2			(1)



#define	LARGE_NUMBER	(32000*1000)

/* 画面最大距離の１／Xから */
#define	INFLUENCE_DIVIDE	(2.0f)
#define	MAX_DISTANCE_XY	( (int)(bp_sqrtf( (float)(DRAW_WIDTH * DRAW_WIDTH) + (float)(DRAW_HEIGHT * DRAW_HEIGHT))/INFLUENCE_DIVIDE) )   //BP_MATH - emulate PS2 sqrtf

#define	LIGHT_POS_NUM_MAX	(64)

#define	SLIDE_SPEED			(0.4f      *0.2f)
#define	SLIDE_SPEED_SLOW	(0.15f     *0.2f)

//#define	INTERVAL		(120)
#define	INTERVAL		(2)

#define	MAX_UNIT		(64)

//#define	LIFE_TIME		(90)
#define	LIFE_TIME		(30)

#define	COLOR_R			(64)
#define	COLOR_G			(64)
#define	COLOR_B			(64)

//#define	COLOR_A			(64)
#define	COLOR_A			(96)
#define	ALPHA_SP		(96)

#define	SHIFT			(1.0f)
#define	SIZE			(4.0f)
#define	SIZE_RANDAM		(2.0f)


#define	TEXTURE1		( 4642619 /*"drop02_msk"*/ )
#define	TEXTURE2		( 3594043 /*"drop01_msk"*/ )



#define	OK_ASPECT_RATIO			( 0.60f )

#define	PURUPURU_PAT			( 8 )


#define	LENGTH_MAX		( 10000.0f )
#define	LENGTH_MIN		( 5000.0f )

typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		*pos;


	DG_PRIM2	*prim[MAX_TEX_NUM * ADD_SUB] ;
	int			count[MAX_TEX_NUM * N_VERTS2 * N_PRIMS2];
	int			size[MAX_TEX_NUM * N_VERTS2 * N_PRIMS2];
	float		pos_x[MAX_TEX_NUM * N_VERTS2 * N_PRIMS2];
	float		pos_y[MAX_TEX_NUM * N_VERTS2 * N_PRIMS2];

	float		purupuru_x[PURUPURU_PAT];
	float		purupuru_y[PURUPURU_PAT];

	int			time;
} Work ;


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i,j,k,l;
	int		*size;
	int		*count;
	float	*pos_x;
	float	*pos_y;

	float	shift_x;
	float	shift_y;

	float	ftemp;
	float		screen_near_x;
	float		screen_near_y;

	int	cycle;
	int	clock;
	float	intense;
	DG_PRIM2			**prim;
	DG_PRIM2			**prim_keep;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	FVECTOR				*pos0;
	FVECTOR				*pos1;
	FVECTOR				fvtemp;

	ftemp = DG_Chanls->screen;
	ftemp = (ftemp > 4.0f)? 4.0f: ftemp;
	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / ftemp;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / ftemp;

   //AR_PARTICLE_FULL
	clock = AS_WillPrimBuffSwitch() ^ work->prim[0]->buffer_clock;

	prim = work->prim;
	for( i=0; i<MAX_TEX_NUM; i++ ){
		for( j=0; j<ADD_SUB; j++ ){
			DG_SwitchBuffPrim2( *prim );
			DG_VisiblePrim2( *prim ) ;
			GM_GroupPrim2( *prim, GM_CurrentStageMap ) ;
			prim++;
		}
	}


	_sceVu0SubVector( &fvtemp, work->pos, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	ftemp = GV_VecLen3F( &fvtemp );
	if( ftemp > LENGTH_MAX ){
		intense = 0.0f;
	}else if( ftemp < LENGTH_MIN ){
		intense = 1.0f;
	}else{
		intense = (LENGTH_MAX - ftemp) / (LENGTH_MAX - LENGTH_MIN);
	}

	if ( (DG_PointCheckOne( work->pos, 0 ) & 1) ) {	/* ポイントチェック */
		intense = 0.0f;
	}



	//以上で、
	//intense が 強度（負だと風向き逆方向）、fvtemp2.vx,fvtemp2.vy が 画面の中心からの位相強度
	//いづれも１が最大

	cycle = GV_Time % (PURUPURU_PAT*2)+irnd()%PURUPURU_PAT/2;
	if(cycle>=PURUPURU_PAT) cycle -= PURUPURU_PAT;
	cycle = (cycle<PURUPURU_PAT)?cycle:PURUPURU_PAT*2-1-cycle;
	shift_x = SHIFT * work->purupuru_x[cycle];
	shift_y = SHIFT * work->purupuru_y[cycle];



	shift_x = SHIFT;
	shift_y = SHIFT;





	work->time--;


	prim    = work->prim;
	pos_x   = work->pos_x;
	pos_y   = work->pos_y;
	count   = work->count;
	for( i=0; i<MAX_TEX_NUM; i++ ){
		/* 更新座標 */
		pos0     = SCR_POS0;
		for ( k=0; k<N_PRIMS2; k++ ){
			for ( l=0; l<N_VERTS2; l++ ){
				if( (*count)>0 ){
					pos0->vw=1.0f;
					if( (*pos_x) < -1.1f
					 || (*pos_y) < -1.1f
					 || (*pos_x) >  1.1f
					 || (*pos_y) >  1.1f ){
						(*count) = -1;
					}else{
						(*count)--;
					}
				}else{
					if( intense>rnd() ){
						pos0->vw=1.0f;
						(*count) = (int)( (float)LIFE_TIME * (rnd()*0.5f + 0.5f) );
						(*pos_x) = frnd();
						(*pos_y) = frnd();
					}else{
						pos0->vw=0.0f;
					}
				}
				pos0->vx = (*pos_x) * screen_near_x ;
				pos0->vy = (*pos_y) * screen_near_y ;
				pos0->vz = SCREEN_NEAR;	/* 重要 */

				pos0++;
				pos_x++;
				pos_y++;
				count++;
			}
		}
		/* スクラッチパット内の座標の更新は終わり */
		/* --------------------------------------------------------*/

		prim_keep=prim;
		prim++;
		OK_Mem_Scr( SCR_UVS0, (*prim_keep)->uvrgb[ clock ], sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
		OK_Mem_Scr( SCR_UVS1, (*prim)->uvrgb[ clock ],      sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
		OK_Mem_Scr( SCR_POS1, (*prim)->pos[ clock ],        sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
		count = work->count;
		size  = work->size;
		pos0 = SCR_POS0;	/* 上で更新した座標を参照 */
		pos1 = SCR_POS1;	/* 更に計算する */
		uvrgbwh0 = SCR_UVS0;
		uvrgbwh1 = SCR_UVS1;

		for ( k=0; k<N_PRIMS2*N_VERTS2; k++ ){
			if( pos0->vw!=0.0f ){
				uvrgbwh1->a = uvrgbwh0->a = (*count) * COLOR_A / LIFE_TIME;
				uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = (*size);
				pos1->vx = pos0->vx + shift_x ;
				pos1->vy = pos0->vy + shift_y ;
			}else if( (irnd()>>8)%RATIO ){
				uvrgbwh1->a = uvrgbwh0->a = 0;
				uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = 0;
			}
			size++;
			count++;
			uvrgbwh0++;
			uvrgbwh1++;
			pos0++;
			pos1++;
		}

		OK_Scr_Mem( (*prim_keep)->pos[   clock ], SCR_POS0, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( (*prim     )->pos[   clock ], SCR_POS1, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( (*prim_keep)->uvrgb[ clock ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( (*prim     )->uvrgb[ clock ], SCR_UVS1, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
		prim++;

	}
}

static void Die( Work *work )
{
	int	i,j;
	DG_PRIM2 **prim;

	prim=work->prim;
	for( i=0; i<MAX_TEX_NUM; i++ ){
		for( j=0; j<ADD_SUB; j++ ){
			*prim = OK_FreePrim2( *prim );
			prim++;
		}
	}
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int add_sub )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	if( add_sub==ADD_TRANS ){
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else{
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}

	pos     = SCR_POS0 ;
	uvrgbwh = SCR_UVS0 ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			pos->vx = 0.0f ;
			pos->vy = 0.0f ;
			pos->vz = SCREEN_NEAR;

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

//			uvrgbwh->h = uvrgbwh->w = (int)( SIZE_RANDAM*((float)(k)/(float)(N_VERTS2)) + SIZE ) ;
			uvrgbwh->w = 0 ;
			uvrgbwh->h = 0 ;

			uvrgbwh->r = COLOR_R ;
			uvrgbwh->g = COLOR_G ;
			uvrgbwh->b = COLOR_B ;
			uvrgbwh->a = COLOR_A ;

			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh ++ ;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS0, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS0, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;

	return 1;
}


static int GetResources( Work *work )
{
	int			i,j,k,l;
	int			*size;
	int			*count;
	DG_PRIM2	**prim ;
	DG_TEX		*tex[MAX_TEX_NUM] ;
	float	ftemp;

	work->time=0;

	tex[0] = DG_GetTexture( TEXTURE1 );
	tex[1] = DG_GetTexture( TEXTURE2 );

	prim  = work->prim;
	count = work->count;
	size  = work->size;
	for( i=0; i<MAX_TEX_NUM; i++ ){
		for( j=0; j<ADD_SUB; j++ ){
			*prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS2, N_VERTS2 );
//printf("XXXXXXXXX:%x\n",*prim);
			if(*prim==NULL){
				printf("null prim\n");
				return -1;
			}
			InitPacket2( work, *prim, tex[i], j );
			(*prim)->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
			DG_RaisePrim2( *prim, j*100 );

			prim++;
		}
		for ( k=0; k<N_PRIMS2; k++ ){
			for ( l=0; l<N_VERTS2; l++ ){
				(*count)=0;
				(*size) = (int)( SIZE_RANDAM*((float)(k)/(float)(N_VERTS2)) + SIZE ) ;
				count++;
				size++;
			}
		}
	}

	for( i=0; i<PURUPURU_PAT; i++ ){
		ftemp = 3.14159265f * 1.0f * (float)(i) / (float)(PURUPURU_PAT);
		work->purupuru_x[i]= -cosf(ftemp);
		work->purupuru_y[i]=  sinf(ftemp);
	}

	return 0 ;
}



void *NewRainCameraProg( FVECTOR *pos )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->pos = pos;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
