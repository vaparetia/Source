//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dive_flow.c
	突入水飛沫：ガス状

	2001/07/12 S.Okajima
	$Id: dive_flow.c,v 1.1.1.3 2002/11/19 11:47:29 Yoshizawa1 Exp $
*/



#ifdef PSX2	///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>


#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(0x4000/16/N_VERTS)
#define	N_TOTAL		(N_PRIMS*N_VERTS)

#define	FADE_OUT_TIME	(30)

#define	SIZE			(100.0f)

#define	VEC_MAX		(100.0f)
#define	PARAM		(2000.0f)
#define	CORE_RADIUS	(10.0f)

#define	DECAY_RATIO	(0.995f)

#define	LIFE_PART	(90)
#define	LIFE_MAX	(120)


/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern void *NewRippleStrip( FVECTOR *center, float radius, int life );

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			fade_out_started;
	int			fade_out_count;

	int			now_num;
	int			clock;

	int			main_life;
	int			col;

	int			col_dat[LIFE_MAX];
	int			life[N_TOTAL];
	FVECTOR		vec[N_TOTAL];
	FVECTOR		center;
	float		radius;
	float		intense;

	DG_PRIM2	*prim;
} Work ;

/*----------------------------------------------------------------*/
static void Act( Work *work )
{
	FVECTOR		*sc_pos;
	FVECTOR		*sc_pos1;
	FVECTOR		*sc_pos2;
	int			clock;
	int			*life;
	int			*col_dat;
	int			k, loop;
	int			num;
	float		fade_ratio;
	float		angle;
	float		radius;
	DG_PRIM2	*prim ;
	FVECTOR		fv_keep;
	DG_PRIM2_UVRGB	*uvrgb;


	prim = work->prim;
	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	work->clock = clock = prim->buffer_clock;

	if( work->fade_out_started ){
		fade_ratio = (float)work->fade_out_count / (float)FADE_OUT_TIME;
		work->fade_out_count--;
		if( work->fade_out_count < 0 ){
			extern void *NewRippleStrip( FVECTOR *center, float radius, int life );
			NewRippleStrip( &work->center, 30000.0f, 600 );
			GV_DestroyActor( work ) ;
			DG_InvisiblePrim2( prim );
			return;
		}
	}else{
		fade_ratio = 1.0f;
	}

	num = 0;
	fv_keep.vw = -1.0f;
	sc_pos2 = work->vec;
	life    = work->life;
	col_dat = work->col_dat;
	uvrgb = prim->uvrgb[clock];
	OK_Mem_Scr( SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), N_TOTAL );
	sc_pos  = SCR_POS;
	for ( k = 0 ; k < N_TOTAL ; k++ ){
		if( ++(*life) < LIFE_MAX ){
			_sceVu0AddVector( sc_pos,  sc_pos,  sc_pos2 );
			if( (k+1)&0x02 ){
				uvrgb->a = 0;
			}else if( sc_pos->vy < GM_WaterLevel ){
				if( uvrgb->a!=0 ){
					DG_COPY_VEC( &fv_keep, sc_pos );
					fv_keep.vw = 1.0f;
					uvrgb->a = 0;
				}
			}else{
				uvrgb->a = (int)((float)col_dat[(*life)]*fade_ratio);
			}
			sc_pos2->vy+= P_GRAVITY;
			_sceVu0ScaleVector( sc_pos2, sc_pos2, DECAY_RATIO );
		}else{
			num++;
			uvrgb->a = 0;
		}
		uvrgb++;
		sc_pos++;
		sc_pos2++;
		life++;
	}
	OK_Scr_Mem( prim->pos[  clock], SCR_POS, sizeof(FVECTOR), N_TOTAL );

	if( !work->fade_out_started ){
		loop = ((irnd()>>8)&3)+1;
		for( k=0; k<loop*4; k++ ){
			num = work->now_num;
			life = work->life;
			life+= num;
			if( (*life)>= LIFE_MAX ){
				sc_pos1 = prim->pos[work->clock];
				sc_pos2 = work->vec;
				sc_pos1+= num;
				sc_pos2+= num;

				(*life) = 0;
				DG_COPY_VEC( sc_pos1, &work->center );
				angle = TPI*rnd();
				radius = work->radius*rnd();
				sc_pos1->vx+= radius*sinf( angle );
				sc_pos1->vy+= work->radius*0.1f*rnd();
				sc_pos1->vz+= radius*cosf( angle );

#if 1
				angle = TPI*rnd();
				radius = work->intense*rnd()*0.25f;
				sc_pos2->vx = radius*sinf( angle );
				sc_pos2->vy = work->intense*rnd();
				sc_pos2->vz = radius*cosf( angle );
#else
				sc_pos2->vx = 0.0f;
				sc_pos2->vy = radius*rnd()*0.1f;
				sc_pos2->vz = 0.0f;
#endif

				work->now_num++;
				if( work->now_num>= N_TOTAL ) work->now_num = 0;
			}else{
				break;
			}
		}
	}

	if( work->main_life > 0 ){
		work->main_life--;
		if( work->main_life <= 0 ){
			work->fade_out_started = 1;
			work->fade_out_count = FADE_OUT_TIME;
		}
	}

//	if( fv_keep.vw > 0.0f ) NewRippleStrip( &fv_keep, 6000.0f + 1000.0f*rnd(), 200 );

}


/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i;
	int	col_r;
	int	col_g;
	int	col_b;

	col_r = ((work->col>>24)&255);
	col_g = ((work->col>>16)&255);
	col_b = ((work->col>> 8)&255);

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 2, 0, 1, 0x00 ) );

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i=0; i<N_TOTAL/4; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = col_r ;
		uvrgb0->g = uvrgb1->g = col_g ;
		uvrgb0->b = uvrgb1->b = col_b ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = 0 ;
		uvrgb0->g = uvrgb1->g = 0 ;
		uvrgb0->b = uvrgb1->b = 0 ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = 0 ;
		uvrgb0->g = uvrgb1->g = 0 ;
		uvrgb0->b = uvrgb1->b = 0 ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = col_r ;
		uvrgb0->g = uvrgb1->g = col_g ;
		uvrgb0->b = uvrgb1->b = col_b ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			j;
	int			*col_dat;
	int			*life;
	float		col_a;

	work->now_num = 0;

	col_a = (float)(work->col&255);

	col_dat = work->col_dat;
	for ( j=0; j<LIFE_PART; j++ ){
		(*col_dat++) = (int)(col_a);
	}
	for (    ; j<LIFE_MAX; j++ ){
		(*col_dat++) = (int)(col_a * (float)(LIFE_MAX - j) / (float)(LIFE_MAX - LIFE_PART) );
	}


	life    = work->life;
	for ( j = 0 ; j < N_TOTAL ; j++ ){
		(*life++) = LIFE_MAX;
	}


//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex );
	work->clock = prim->buffer_clock;

	work->fade_out_started = 0;

	return 0 ;
}


void *NewDivingFlow( FVECTOR *center, float radius, float intense, int col, int life )
{
	Work		*work ;

	OPERATOR() ;

//	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), GV_PRIO_MAX ) ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, center );
		work->radius  = radius;
		work->intense = intense;
		work->col = col;
		work->main_life = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
