//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	poly_hola_upper.c
	オーラ

	2001/07/08 S.Okajima
	$Id: poly_hola_upper.c,v 1.1.1.3 2002/11/19 11:47:34 Yoshizawa1 Exp $
*/




#ifdef PSX2 ///
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

#if 1
#define	COL_R			(200)
#define	COL_G			(90)
#define	COL_B			(40)
#define	COL_A			(64)
#else
#define	COL_R			(0)
#define	COL_G			(0)
#define	COL_B			(0)
#define	COL_A			(160)
#endif

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(0x4000/16/N_VERTS)
#define	N_TOTAL		(N_PRIMS*N_VERTS)

#define	FADE_OUT_TIME	(300)

#define	SIZE			(100.0f)

#define	VEC_MAX		(40.0f)
#define	PARAM		(1000.0f)
#define	CORE_RADIUS	(10.0f)

#define	DECAY_RATIO	(0.95f)
#define	INF_RATIO	(0.5f)

#define	LIFE_MAX	(60)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			nothing_flag;
	int			nothing_count;

	int			fade_out_started;
	int			fade_out_count;

	int			now_num;
	int			clock;

	int			col_dat[LIFE_MAX];
	int			life[N_TOTAL];
	FVECTOR		vec[N_TOTAL];

	DG_PRIM2	*prim;
} Work ;

static Work *OK_POLY_HOLA_UPPER_WORK = NULL;
static FVECTOR *OK_PolyHolaUpperCenter = NULL;

/* ---------------------------------------------------------------- */
static int ReceiveSignal( void *pwork, int signal, int value )
{
	Work *work = pwork;

	switch(signal){
	  case 0:
		work->fade_out_started = 1;
		work->fade_out_count = FADE_OUT_TIME;
		break;
	  default:
		return GV_DefaultSignalFunc( pwork, signal, value );// Must do this!!!
	}
	return 0;
}

/*----------------------------------------------------------------*/
static void Act( Work *work )
{
	FVECTOR		*sc_pos;
	FVECTOR		*sc_pos2;
	int			clock;
	int			*life;
	int			*col_dat;
	int			k;
	int			num;
	float		fade_ratio;
	float		scale;
	float		len;
	DG_PRIM2	*prim ;
	FVECTOR		fvtemp;
	DG_PRIM2_UVRGB	*uvrgb;

	prim = work->prim;
	if( OK_PolyHolaUpperCenter==NULL ){
//printf("ok:COME::::::::::::::::::::::::::::::::::::::::::::\n");
#if 0
		Dummy.vx = 0.0f;
		Dummy.vy = 1000000.0f;
		Dummy.vz = 0.0f;
		OK_PolyHolaUpperCenter = &Dummy;
#else
		DG_InvisiblePrim2( prim );
#endif
		return;
	}

//printf("%f %f %f\n",OK_PolyHolaUpperCenter->vx,OK_PolyHolaUpperCenter->vy,OK_PolyHolaUpperCenter->vz);


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
			GV_DestroyActor( work ) ;
			DG_InvisiblePrim2( prim );
			return;
		}
	}else{
		fade_ratio = 1.0f;
	}


	if( work->nothing_flag ){
		DG_InvisiblePrim2( prim );
		return;
	}

	num = 0;
	sc_pos2 = work->vec;
	life    = work->life;
	col_dat = work->col_dat;
	uvrgb = prim->uvrgb[clock];
	OK_Mem_Scr( SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), N_TOTAL );
	sc_pos  = SCR_POS;
	for ( k = 0 ; k < N_TOTAL ; k++ ){
		if( ++(*life) < LIFE_MAX ){
			if( (k+1)&0x02 ){
				uvrgb->a = 0;
			}else{
				uvrgb->a = col_dat[(*life)];
			}
			_sceVu0AddVector( sc_pos,  sc_pos,  sc_pos2 );
			_sceVu0SubVector( &fvtemp, OK_PolyHolaUpperCenter, sc_pos );
//			fvtemp.vy = 0.0f;
			len = _sceVu0InnerProduct( &fvtemp, &fvtemp );
			if( len > CORE_RADIUS ){
				scale = PARAM/len;
				if( scale > INF_RATIO ) scale = INF_RATIO;
				_sceVu0ScaleVector( &fvtemp, &fvtemp, scale );
				_sceVu0AddVector( sc_pos2, sc_pos2, &fvtemp );
			}
			_sceVu0ScaleVector( sc_pos2, sc_pos2, DECAY_RATIO );
//			sc_pos2->vy-= P_GRAVITY*0.25f;
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


	if( num>=N_TOTAL ){
		work->nothing_count++;
		if( work->nothing_count > 4 ){
			work->nothing_flag = 1;
			work->nothing_count= 0;
//printf("work->nothing_flag:%d\n",work->nothing_flag);
		}
	}


}


void OK_PutPolyHolaUpperCenter( FVECTOR *center )
{
	OK_PolyHolaUpperCenter = center;
}

void OK_PutPolyHolaUpper( FVECTOR *pos, FVECTOR *vec )
{
	Work *work;
	FVECTOR	*sc_pos1;
	FVECTOR	*sc_pos2;
	FVECTOR	fvtemp;
	int	num;
	int	*life;


	if( OK_POLY_HOLA_UPPER_WORK == NULL ){
		void *NewPolyHolaUpper( void );
		NewPolyHolaUpper();
	}
	if( OK_POLY_HOLA_UPPER_WORK == NULL ) return; // それでもやっぱり起動できなかった
	work = OK_POLY_HOLA_UPPER_WORK;

	work->nothing_flag = 0;

	num = work->now_num;
	life = work->life;
	life+= num;
	if( (*life)>= LIFE_MAX ){
		sc_pos1 = work->prim->pos[work->clock];
		sc_pos2 = work->vec;
		sc_pos1+= num;
		sc_pos2+= num;

		(*life) = 0;
		DG_COPY_VEC( sc_pos1, pos );
		_sceVu0Normalize( &fvtemp, vec );
		_sceVu0ScaleVector( &fvtemp, &fvtemp, VEC_MAX*rnd() );

		sc_pos2->vx = fvtemp.vz;
		sc_pos2->vy = fvtemp.vy;
		sc_pos2->vz =-fvtemp.vx;

		work->now_num++;
		if( work->now_num>= N_TOTAL ) work->now_num = 0;
	}

}

/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_PolyHolaUpperCenter = NULL;
	OK_POLY_HOLA_UPPER_WORK = NULL;
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 2, 0, 1, 0x00 ) );

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i=0; i<N_TOTAL/4; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
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
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
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

	work->nothing_count= 0;
	work->nothing_flag = 1;
	work->now_num = 0;

	col_dat = work->col_dat;
	for ( j=0; j<LIFE_MAX; j++ ){
		(*col_dat++) = (int)((float)COL_A * sinf( PI * (float)j / (float)(LIFE_MAX-1) )   );
	}


	life    = work->life;
	for ( j = 0 ; j < N_TOTAL ; j++ ){
		(*life++) = LIFE_MAX;
	}


//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
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


void *NewPolyHolaUpper( void )
{
	Work		*work ;

	OPERATOR() ;

//	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), GV_PRIO_MAX ) ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GV_SetActorSignalFunc( work, ReceiveSignal ) ;
	}
	OK_POLY_HOLA_UPPER_WORK = work;
	return (void *)work ;
}
