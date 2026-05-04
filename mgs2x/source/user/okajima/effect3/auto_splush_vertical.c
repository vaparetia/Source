//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	auto_splush_vertical.c
	のびーるポリゴン

	2001/05/31 S.Okajima
	$Id: auto_splush_vertical.c,v 1.1.1.3 2002/11/19 11:47:27 Yoshizawa1 Exp $
*/

#ifdef PSX2 ///
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

#include	"../../mode/demo/eft_con.h"

#define	RAISE			(0)

#define	COL_R		(128)
#define	COL_G		(192)
#define	COL_B		(255)
#define	COL_A		(128)


//#define	SCR_WRK_LEN	(0x3f00)
#define	SCR_WRK_LEN	(0x1000)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + SCR_WRK_LEN)

#define N_VERTS			(32)
#define N_POLYS			(N_VERTS/4)
#define N_PRIMS			(SCR_WRK_LEN / 16 / N_VERTS)
#define N_LOOPS			(1)

#define INIT_PHASE		(16)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		dummy;
	FVECTOR		*pos;
	float		width;
	float		intense;
	int			cycle;
	int			cont_flag;
	int			initial_life;

	int			n_prims;
	DG_PRIM2	*prim ;

} Work ;


static int InitVerts( Work *work, int num, int clock, float intense )
{
	FVECTOR		*center;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*pos2;
	float	angle;
	float	radius;

	center = SCR_POS;

	DG_COPY_VEC( center, work->pos );
	center->vy = GM_WaterLevel;

	radius = work->width*rnd() * 2.0f;
	angle  = TPI*rnd();
	center->vx+= radius*sinf( angle );
	center->vz+= radius*cosf( angle );


	pos0 = work->prim->pos[   clock ];
	pos1 = work->prim->pos[ 1-clock ];
	pos0+= num*4;
	pos1+= num*4;

	pos2 = pos0;
/*
	DG_COPY_VEC( pos2++, center );
	DG_COPY_VEC( pos2++, center );
	DG_COPY_VEC( pos2++, center );
	DG_COPY_VEC( pos2++, center );
*/
	pos0->vx = pos0->vz;
	pos0->vy = GM_WaterLevel;
	(pos0++)->vw = (pos1++)->vw = center->vx;	// 中心を格納しておく
	pos0->vx = pos0->vz;
	pos0->vy = GM_WaterLevel;
	(pos0++)->vw = (pos1++)->vw = center->vy;
	pos0->vx = pos0->vz;
	pos0->vy = GM_WaterLevel;
	(pos0++)->vw = (pos1++)->vw = center->vz;
	pos0->vx = pos0->vz;
	pos0->vy = GM_WaterLevel;
	(pos0++)->vw = intense;
	(pos1++)->vw = intense + P_GRAVITY;	// 次のフレームに使用する速度をづらしておく

//printf("%f\n",intense);

	return 1;
}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2 *prim;
	int	count;
	int	clock;
	int	i, j;
	FVECTOR	*before_pos;
	FVECTOR	*pos;
	FVECTOR	*sc_pos0;
	FVECTOR	*sc_pos1;
	FVECTOR	*sc_temp0;
	FVECTOR	*sc_temp1;
	FVECTOR	*sc_temp2;
	DG_PRIM2_UVRGB	*uvrgb;	/* スプライト用 */

	prim = work->prim;

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;


	sc_temp0 = sc_temp1 = sc_temp2 = SCR_TMP;
	sc_temp1+= 1;
	sc_temp2+= 2;

	sc_temp1->vx = work->width;
	sc_temp1->vy = 0;
	sc_temp1->vz = 0;
	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( sc_temp1, sc_temp1, 1 );


	count = N_LOOPS*work->n_prims*N_POLYS;
	before_pos = prim->pos[1-clock];
	pos        = prim->pos[  clock];
	uvrgb      = prim->uvrgb[clock];
	for ( j=0 ; j < N_LOOPS ; j++ ){
		OK_Mem_Scr( SCR_POS, before_pos, sizeof(FVECTOR), N_VERTS*work->n_prims ) ;
		sc_pos0 = SCR_POS;
		sc_pos1 = SCR_POS;
		for ( i = 0 ; i < work->n_prims*N_POLYS ; i++ ){
			sc_temp0->vx = (sc_pos0++)->vw;	// 中心座標
			sc_temp0->vy = (sc_pos0++)->vw;
			sc_temp0->vz = (sc_pos0++)->vw;

			sc_pos0->vw += P_GRAVITY;
			sc_pos1->vy+= sc_pos0->vw;
			if( sc_pos1->vy <= GM_WaterLevel ){
				(uvrgb++)->a = 0;
				(uvrgb++)->a = 0;
				(uvrgb++)->a = 0;
				(uvrgb++)->a = 0;
				count--;

				sc_pos1+= 4;

			}else{
				(uvrgb++)->a = COL_A;
				(uvrgb++)->a = COL_A;
				(uvrgb++)->a = COL_A;
				(uvrgb++)->a = COL_A;

				sc_pos1->vx = sc_temp0->vx + sc_temp1->vx;
//				sc_pos1->vy+= sc_pos0->vw;
				sc_pos1->vz = sc_temp0->vz + sc_temp1->vz;
				sc_pos1++;

				sc_pos1->vx = sc_temp0->vx - sc_temp1->vx;
				sc_pos1->vy+= sc_pos0->vw;
				sc_pos1->vz = sc_temp0->vz - sc_temp1->vz;
				sc_pos1++;

				sc_pos1->vx = sc_temp0->vx + sc_temp1->vx;
				sc_pos1->vy = GM_WaterLevel;
				sc_pos1->vz = sc_temp0->vz + sc_temp1->vz;
				sc_pos1++;

				sc_pos1->vx = sc_temp0->vx - sc_temp1->vx;
				sc_pos1->vy = GM_WaterLevel;
				sc_pos1->vz = sc_temp0->vz - sc_temp1->vz;
				sc_pos1++;
			}
			(sc_pos0++)->vw += P_GRAVITY;	// 次のフレーム分




		}
		OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*work->n_prims ) ;
		before_pos += N_VERTS*work->n_prims;
		pos        += N_VERTS*work->n_prims;
	}

//printf("count:%d\n",count);
	if( count <= 0 ){
		GV_DestroyActor( work );
		DG_InvisiblePrim2( work->prim );
		return;
	}

	if( !work->cont_flag ){
		work->initial_life--;
		if( work->initial_life >= 0 ){
			for(i=0; i<2; i++){
				if( work->cycle >= 0 ) InitVerts( work, work->cycle, clock, work->intense * (0.5f+0.5f*rnd())  );
				work->cycle--;
			}
		}
	}else{
		InitVerts( work, work->cycle, clock, work->intense * (0.5f+0.5f*rnd())  );
		work->cycle--;
		if( work->cycle < 0 ) work->cycle = N_LOOPS*work->n_prims*N_POLYS-1;
	}

}


/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	int		i;
	float	change_u0,change_u1;
	float	change_v0,change_v1;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	pos0   = prim->pos[ 0 ];
	pos1   = prim->pos[ 1 ];
	for ( i = 0 ; i < N_LOOPS*work->n_prims*N_POLYS ; i++ ){
		(pos0++)->vw = (pos1++)->vw = 0.0f;
		(pos0++)->vw = (pos1++)->vw = 0.0f;
		(pos0++)->vw = (pos1++)->vw = 0.0f;
		(pos0++)->vw = (pos1++)->vw = 0.0f;

		change_u0 = (float)((irnd()>>8)&1);
		change_u1 = 1.0f - change_u0;
		change_v0 = (float)((irnd()>>8)&1);
		change_v1 = 1.0f - change_v0;

		uvrgb1->u = uvrgb0->u = FTOI12( change_u0 * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( change_v0 * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x8fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( change_u1 * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( change_v0 * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x8fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( change_u0 * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( change_v1 * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x0fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( change_u1 * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( change_v1 * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x0fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = 0 ;
		uvrgb0++;
		uvrgb1++;
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->initial_life = INIT_PHASE;

	work->cycle = N_LOOPS*work->n_prims*N_POLYS-1;

//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 13081966 /*"chi03_msk"*/ );
//	tex = DG_GetTexture( 8617636 /*"col128_alp"*/ );
//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS*work->n_prims, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

/* ---------------------------------------------------------------- */
/*
・pos  の保持は必要無い（移動無し）
・flag ０：一定時間で消滅  １：ずっと存在
*/

void *NewAutoSplushVertical_Hold( FVECTOR *pos, float width, float intense, int flag )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->cont_flag = flag;
		work->dummy.vx = pos->vx;
		work->dummy.vy = GM_WaterLevel;
		work->dummy.vz = pos->vz;
		work->pos       = &work->dummy;
		work->width     = width;
		work->intense   = intense;

		work->n_prims = N_PRIMS;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------- */
/*
・pos  のポインタはずっと参照
・flag ０：一定時間で消滅  １：ずっと存在
*/

void *NewAutoSplushVertical( FVECTOR *pos, float width, float intense, int flag )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->cont_flag = flag;
		work->pos       = pos;
		work->width     = width;
		work->intense   = intense;

		work->n_prims = N_PRIMS;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewAutoSplushVertical_EftCtrl( int con_name, float width, float intense )
{
	EFTCONTROL	*ctrl;
	ctrl = DM_GetEftControl( con_name );
	if( !ctrl ) return NULL;
	return NewAutoSplushVertical( &ctrl->mov, width, intense, 1 );
}

void *NewAutoSplushVertical_Scn( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		if ( GCL_GetOption( 'p' ) != NULL ){
			work->dummy.vx = (float)GCL_GetNextInt();
			work->dummy.vy = GM_WaterLevel;
			work->dummy.vz = (float)GCL_GetNextInt();
		}else{
			printf("auto_splush_vertical.c::p\n");
			ASSERT(0)
		}

		if ( GCL_GetOption( 'w' ) != NULL ){
			work->width = (float)GCL_GetNextInt();
		}else{
			printf("auto_splush_vertical.c::w\n");
			ASSERT(0)
		}

		if ( GCL_GetOption( 'i' ) != NULL ){
			work->intense = (float)GCL_GetNextInt();
		}else{
			printf("auto_splush_vertical.c::i\n");
			ASSERT(0)
		}

		work->cont_flag = 0;
		work->pos    = &work->dummy;

		work->n_prims = 2;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

