//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	stg_smoke_back.c
	スティンガーミサイル後部
	2000/07/19 S.Okajima
	$Id: stg_smoke_back.c,v 1.1.1.3 2002/11/19 11:50:37 Yoshizawa1 Exp $

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

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

#include	"gameheader.h"
#include	"libmt.h"



#define	BACK_SHIFT			(1200.0f)
#define	DECAY_RATIO			(0.97f)

#define	RGB_RND_MAX		(64)
#define	RGB_RND_MIN		(48)
#define	ALPHA			(16)
#define	ALPHA_SUB		(1)

#define	TEX_PER			(2.5f)
#define	WIDTH_ADD_PER	(0.05f)

#define N_PRIMS		(2)
#define N_VERTS		(16)

#define	SCR_POS0	(SCRPAD_ADDR)
#define	SCR_POS1	(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS0	(SCRPAD_ADDR + 0x2000)
#define	SCR_UVS1	(SCRPAD_ADDR + 0x3000)


#define	WIN_MIN				(10.0f)
#define	WIN_MAX				(250.0f)

#define	SIZE_RND		(1000.0f)
#define	SIZE_MIN		(3000.0f)

#define	LIFE			(40)
#define	LIFE_STATIC		(10)

#define	ANGLE_STEP		(TPI*0.0001f)

#define	RAISE			(60000-4000)

#define	INIT_RADIUS0		(1000.0f)
#define	INIT_RADIUS1		(500.0f)
#define	SPEED				(300.0f)

#define	SPREAD_RATIO		(0.04f)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;

	int			now_num;
	int			size_add;
	float		size;
	float		size_randam;
	float		angle[N_PRIMS*N_VERTS];
	FVECTOR		poly_vec[N_PRIMS*N_VERTS];
	FVECTOR		pos;

	FVECTOR		back_vec;

	FVECTOR		init_step;

	int			alpha_step;

	int			life;

	HZX_GROUP_ID	map_id;

} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	int	i;
	int		clock;
	int		count;
	float	ftemp0;
	float	*angle=NULL;
	FVECTOR	outer_force;
	FVECTOR	*poly_vec;
	FVECTOR	*pos;
	FVECTOR	*pos_before;

	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	count          = 0;
	angle          = work->angle;
	poly_vec = work->poly_vec;

	OK_Mem_Scr( SCR_POS0, work->prim->pos[clock],     sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Mem_Scr( SCR_POS1, work->prim->pos[1-clock],   sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Mem_Scr( SCR_UVS0, work->prim->uvrgb[clock],   sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Mem_Scr( SCR_UVS1, work->prim->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	pos            = SCR_POS0;
	pos_before     = SCR_POS1;
	uvrgbwh        = SCR_UVS0;
	uvrgbwh_before = SCR_UVS1;
	if( work->life > LIFE - LIFE_STATIC ){
		for( i=0; i<N_PRIMS*N_VERTS; i++ ){
			_sceVu0ScaleVector( poly_vec, poly_vec, DECAY_RATIO );

			ftemp0 = _sceVu0InnerProduct( poly_vec, &work->back_vec );
			_sceVu0ScaleVector( &outer_force, &work->back_vec, ftemp0 );
			_sceVu0SubVector( &outer_force, poly_vec, &outer_force ) ;
			_sceVu0ScaleVector( &outer_force, &outer_force, SPREAD_RATIO );
			_sceVu0AddVector( poly_vec, poly_vec, &outer_force ) ;
			_sceVu0AddVector( pos, pos_before, poly_vec ) ;
//AN_Test_Eye2( pos, 2 );
			(*angle) += ANGLE_STEP;
			uvrgbwh->w = (int)(cosf( (*angle)*TPI ) * work->size);
			uvrgbwh->h = (int)(sinf( (*angle)*TPI ) * work->size);

			poly_vec++;
			angle++;
			pos++;
			pos_before++;
			uvrgbwh++;
			uvrgbwh_before++;
		}
	}
   else
   {
		int const adjustedTime = GV_Time >> AS_IsRunningAt30Fps();
      int const adjustedFrameNumber = 8 >> AS_IsRunningAt30Fps();
      
      for( i=0; i<N_PRIMS*N_VERTS; i++ ){
			_sceVu0AddVector( pos, pos_before, poly_vec ) ;

			if( uvrgbwh_before->a < ALPHA_SUB ){
				uvrgbwh->a = 0;
				uvrgbwh->w = 0;
				uvrgbwh->h = 0;
				uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = 0 ;
				count++;
			}else{
				uvrgbwh->a = uvrgbwh_before->a;
            if((adjustedTime + i) % adjustedFrameNumber == 0)
            {
               uvrgbwh->a -= ALPHA_SUB;
            }
				(*angle) += ANGLE_STEP;
				uvrgbwh->w = (int)(cosf( (*angle)*TPI ) * work->size);
				uvrgbwh->h = (int)(sinf( (*angle)*TPI ) * work->size);
			}

			poly_vec++;
			angle++;
			pos++;
			pos_before++;
			uvrgbwh++;
			uvrgbwh_before++;
		}
	}
	OK_Scr_Mem( work->prim->pos[clock],    SCR_POS0, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( work->prim->pos[1-clock],  SCR_POS1, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( work->prim->uvrgb[clock],  SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( work->prim->uvrgb[1-clock],SCR_UVS1, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	if( count >= N_PRIMS*N_VERTS  &&  work->life < 0){
		GV_DestroyActor( work ) ;
	}

	work->life--;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center )
{
	float	*angle;
	FVECTOR	fvtemp;
	FVECTOR	*poly_vec;
	SVECTOR	svtemp;
	FVECTOR	*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i ;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	svtemp.vz = 0;
	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	angle   = work->angle;
	pos      = SCR_POS0;
	uvrgbwh  = SCR_UVS0;
	poly_vec = work->poly_vec;
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		svtemp.vx = irnd()%4096;
		svtemp.vy = irnd()%2048;
		fvtemp.vz = INIT_RADIUS1 * rnd();
		DG_SetPos2( &work->pos, &svtemp );
		DG_PutVector( &fvtemp, pos, 1 );
//AN_Test_Eye2( pos, 2 );
		_sceVu0SubVector( poly_vec, pos, center ) ;
		_sceVu0Normalize( poly_vec, poly_vec );
		_sceVu0ScaleVector( poly_vec, poly_vec, SPEED*rnd() );


		uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;

		(*angle) = rnd();
		uvrgbwh->w = (int)(cosf( (*angle)*TPI ) * work->size);
		uvrgbwh->h = (int)(sinf( (*angle)*TPI ) * work->size);
		uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = irnd() % (RGB_RND_MAX - RGB_RND_MIN) + RGB_RND_MIN;
		uvrgbwh->a = (int)((float)ALPHA * (rnd() + 1.0f) * 0.5f) ;

		pos ++;
		uvrgbwh ++ ;
		angle++;
		poly_vec++;
	}


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS0, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS0, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, FVECTOR *vector )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		fvtemp;

	work->map_id = GM_GetHzxGroupID( GM_CurrentStageMap );
	work->now_num = 0;

	work->life = LIFE;
	_sceVu0Normalize( &work->back_vec, vector );
	_sceVu0ScaleVector( &fvtemp, &work->back_vec, BACK_SHIFT );
	_sceVu0AddVector( &work->pos, pos, &fvtemp ) ;

	work->size        = SIZE_RND*rnd() + SIZE_MIN;
	work->size_add    = (int)(work->size * WIDTH_ADD_PER);
	work->size_randam = work->size * 0.5f;

	tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
//	tex = DG_GetTexture( GV_StrCode( "rcm_l_msk" ) );

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, pos );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

/*
pos    :発生位置（へのポインタ）
vector :飛翔方向ベクトル（へのポインタ） 
*/
void *NewSTG_SmokeBack( FVECTOR *pos, FVECTOR *vector )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, vector ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

