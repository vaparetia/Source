//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	slit_light2.c
	採光

	2001/04/14 S.Okajima
	$Id: slit_light2.c,v 1.1.1.3 2002/11/19 11:47:24 Yoshizawa1 Exp $
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

#define N_VERTS		(16)
#define N_PRIMS		(16)

#define	SCR_POS		(SCRPAD_ADDR)

#define	RAISE			(0)

#define	VEC_MIN		(0.85f)
#define	VEC_RND		(0.40f)

#define	WIDTH		(1750.0f)
#define	HEIGHT		( 700.0f)
#define	WIDTH_HALF	(WIDTH*0.5f)
#define	HEIGHT_HALF	(HEIGHT*0.5f)

#define	COL_R		(16)
#define	COL_G		(16)
#define	COL_B		(16)
#define	COL_A_MIN	(16.0f)
#define	COL_A		(48.0f)
#define	SUB_ALPHA	(1)
#define	ADD_ALPHA	(1)

#define	SIZE_MIN	(256.0f)
#define	SIZE_RND	(256.0f)

#define	DIR_SHIFT_RATIO	(0.05f)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	int			map;

	FVECTOR		center;
	SVECTOR		rot;
	FMATRIX		world;
	FMATRIX		world_inv;

	FVECTOR		direction;
	float		length;

	DG_PRIM2	*prim ;
	FVECTOR		pos[N_PRIMS*N_VERTS];
	FVECTOR		vec[N_PRIMS*N_VERTS];
} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	int	i;
	int		clock;
	int		flag;
	FVECTOR		fvtemp;
	FVECTOR		*pos;
	FVECTOR		*vec;
	DG_PRIM2	*prim ;

	if( (work->map & GM_CurrentStageMap) != 0 ){
		DG_VisiblePrim2( work->prim ) ;
	}else{
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}

	prim = work->prim;
	DG_VisiblePrim2( prim ) ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

	 //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	// 原点中心、ＸＹ座標内
	OK_Mem_Scr( SCR_POS, work->pos,   sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	pos = SCR_POS;
	vec = work->vec;
	uvrgbwh        = prim->uvrgb[1-clock];
	uvrgbwh_before = prim->uvrgb[  clock];
	for( i=0; i<N_PRIMS*N_VERTS; i++ ){
		pos->vx+= vec->vx;
		pos->vy+= vec->vy;

		flag = 0;
		if( pos->vx > WIDTH_HALF
		 || pos->vx <-WIDTH_HALF
		 || pos->vy > HEIGHT_HALF
		 || pos->vy <-HEIGHT_HALF
		 ){ // 外に出ている
			if( uvrgbwh_before->a > SUB_ALPHA ){
				uvrgbwh->a = uvrgbwh_before->a - SUB_ALPHA;
			}else{
				uvrgbwh->a = 0;
				if( pos->vx > WIDTH_HALF ){
					pos->vx =-WIDTH_HALF;
				}else if( pos->vx <-WIDTH_HALF ){
					pos->vx = WIDTH_HALF;
				}
				if( pos->vy > HEIGHT_HALF ){
					pos->vy =-HEIGHT_HALF;
				}else if( pos->vy <-HEIGHT_HALF ){
					pos->vy = HEIGHT_HALF;
				}
			}
		}else{
			if( uvrgbwh_before->a < uvrgbwh_before->pad0 - ADD_ALPHA ){ // 入りつつある
				uvrgbwh->a = uvrgbwh_before->a + ADD_ALPHA;
			}else{						// 定常
				uvrgbwh->a = uvrgbwh_before->pad0;
			}
		}

		pos++;
		vec++;
		uvrgbwh++ ;
		uvrgbwh_before++ ;
	}
	OK_Scr_Mem( work->pos,   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );

	// ワールド変換
	pos = SCR_POS;
	DG_SetPos( &work->world );
	DG_PutVector( pos, pos, N_PRIMS*N_VERTS );

	// 指定方向伸長
	pos = SCR_POS;
	vec = work->vec;
	for( i=0; i<N_PRIMS*N_VERTS; i++ ){
		_sceVu0ScaleVector( &fvtemp, &work->direction, vec->vw );
		pos->vx+= fvtemp.vx;
		pos->vy+= fvtemp.vy;
		pos->vz+= fvtemp.vz;

		pos++;
		vec++;
	}
	OK_Scr_Mem( prim->pos[clock],  SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos;
	FVECTOR		*vec;
	FVECTOR		fvtemp;
	SVECTOR		svtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i ;
	float	ftemp;
	float	angle;
	float	size;

	DG_ConfigPrim2Tex( prim, tex );

	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE;

	pos        = SCR_POS;
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		pos->vx = WIDTH_HALF  *frnd();
		pos->vy = HEIGHT_HALF *frnd();
		pos->vz = 0;

		pos ++;
	}
	OK_Scr_Mem( work->pos,   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );

	pos        = SCR_POS;
	DG_SetPos2( &work->center, &work->rot );
	DG_PutVector( pos, pos, N_PRIMS*N_VERTS );
	DG_GetPos( &work->world );
	FastInverseMatrix( &work->world_inv, &work->world );

	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	svtemp.vz = 0;
	pos        = SCR_POS;
	vec        = work->vec;
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		ftemp = VEC_MIN + VEC_RND*rnd();
		angle = TPI*rnd();
		vec->vx = ftemp * sinf( angle );
		vec->vy = ftemp * cosf( angle );
		vec->vz = 0.0f;

		vec->vw = DIR_SHIFT_RATIO + rnd()*(1.0f-DIR_SHIFT_RATIO);	// 方向ベクトルの何処に置くかの比

		_sceVu0ScaleVector( &fvtemp, &work->direction, vec->vw );
		pos->vx+= fvtemp.vx;
		pos->vy+= fvtemp.vy;
		pos->vz+= fvtemp.vz;

		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

		ftemp = TPI*rnd();
		size  = SIZE_MIN+SIZE_RND*rnd();
		uvrgbwh1->w  = uvrgbwh0->w  = (int)(size * sinf(ftemp));
		uvrgbwh1->h  = uvrgbwh0->h  = (int)(size * cosf(ftemp));
		uvrgbwh1->r  = uvrgbwh0->r  = COL_R;
		uvrgbwh1->g  = uvrgbwh0->g  = COL_G;
		uvrgbwh1->b  = uvrgbwh0->b  = COL_B;
		uvrgbwh1->a  = 
		uvrgbwh0->a  = 
		uvrgbwh1->pad0  = 
		uvrgbwh0->pad0  = (int)(COL_A_MIN + COL_A*(1.0f-vec->vw)) ;

		pos++;
		vec++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		norm;


	/* 設置中心座標 */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		work->center.vx = (float)GCL_GetNextInt() ;
		work->center.vy = (float)GCL_GetNextInt() ;
		work->center.vz = (float)GCL_GetNextInt() ;
		work->center.vw = 1.0f ;
	}

	/* 法線 */
	if ( GCL_GetOption( 'n' ) != NULL ) {
		norm.vx = (float)GCL_GetNextInt() ;
		norm.vy = (float)GCL_GetNextInt() ;
		norm.vz = (float)GCL_GetNextInt() ;
		norm.vw = 1.0f ;
	}

	OK_DirVecXY( &DG_ZeroVector, &norm, &work->rot );


	/* 光線の絶対方向 */
	if ( GCL_GetOption( 'd' ) != NULL ) {
		work->direction.vx = (float)GCL_GetNextInt() ;
		work->direction.vy = (float)GCL_GetNextInt() ;
		work->direction.vz = (float)GCL_GetNextInt() ;
		work->direction.vw = 1.0f ;
	}

	/* 光線の到達距離 */
	if ( GCL_GetOption( 'l' ) != NULL ) {
		work->length = (float)GCL_GetNextInt() ;
	}

	_sceVu0Normalize( &work->direction, &work->direction );
	_sceVu0ScaleVector( &work->direction, &work->direction, work->length );


	/* 回転スプライト */
	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewSlitLight2( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->map = where;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}
