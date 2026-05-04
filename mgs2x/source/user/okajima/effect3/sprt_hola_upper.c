//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sprt_hola.c
	オーラ

	2001/07/08 S.Okajima
	$Id: sprt_hola_upper.c,v 1.1.1.3 2002/11/19 11:47:37 Yoshizawa1 Exp $
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
#define	P_RGB_MAX		(128)
#define	P_ALPHA_MAX		(64)
#else
#define	P_RGB_MAX		(16)
#define	P_ALPHA_MAX		(64)
#endif

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(32)
//#define	N_VERTS		(16)
#define	N_PRIMS		(0x4000/16/N_VERTS)
#define	N_TOTAL		(N_PRIMS*N_VERTS)

#define	FADE_OUT_TIME	(120)

#define	SIZE			(15.0f)
//#define	SIZE			(100.0f)

#define	VEC_MAX		(20.0f)
#define	PARAM		(500.0f)
#define	CORE_RADIUS	(40.0f)

#define	LIFE_MAX	(60)

#define	DECAY_RATIO	(0.95f)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			fade_out_started;
	int			fade_out_count;

	int			now_num;
	int			clock;

	int			col_dat[LIFE_MAX];
	int			life[N_TOTAL];
	FVECTOR		vec[N_TOTAL];

	DG_PRIM2	*prim;
} Work ;

static Work *OK_SPRT_HOLA_UPPER_WORK = NULL;
static FVECTOR *OK_HolaUpperCenter = NULL;
static FVECTOR Dummy;

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
	FVECTOR	*sc_pos;
	FVECTOR	*sc_pos2;
	int	clock;
	int	*life;
	int	*col_dat;
	int	k;
	float		fade_ratio;
	float		len;
	DG_PRIM2	*prim ;
	FVECTOR		fvtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	if( OK_HolaUpperCenter==NULL ){
		Dummy.vx = 0.0f;
		Dummy.vy = 1000000.0f;
		Dummy.vz = 0.0f;
		OK_HolaUpperCenter = &Dummy;
	}

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
			GV_DestroyActor( work ) ;
			DG_InvisiblePrim2( prim );
			return;
		}
	}else{
		fade_ratio = 1.0f;
	}

	sc_pos2 = work->vec;
	life    = work->life;
	col_dat = work->col_dat;
	uvrgbwh = prim->uvrgb[clock];
	OK_Mem_Scr( SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), N_TOTAL );
	sc_pos  = SCR_POS;
	for ( k = 0 ; k < N_TOTAL ; k++ ){
		if( ++(*life) < LIFE_MAX ){
			uvrgbwh->w = uvrgbwh->h = SIZE;
			uvrgbwh->a = col_dat[(*life)];
			_sceVu0AddVector( sc_pos,  sc_pos,  sc_pos2 );
			_sceVu0SubVector( &fvtemp, OK_HolaUpperCenter, sc_pos );
//			fvtemp.vy = 0.0f;
			len = _sceVu0InnerProduct( &fvtemp, &fvtemp );
			if( len > CORE_RADIUS ){
				_sceVu0ScaleVector( &fvtemp, &fvtemp, PARAM/len );
				_sceVu0AddVector( sc_pos2, sc_pos2, &fvtemp );
			}
			_sceVu0ScaleVector( sc_pos2, sc_pos2, DECAY_RATIO );
			sc_pos2->vy-= P_GRAVITY*0.1f;
		}else{
			uvrgbwh->a = uvrgbwh->w = uvrgbwh->h = 0;
		}
		uvrgbwh++;
		sc_pos++;
		sc_pos2++;
		life++;
	}
	OK_Scr_Mem( prim->pos[  clock], SCR_POS, sizeof(FVECTOR), N_TOTAL );



}


void OK_PutSprtHolaUpperCenter( FVECTOR *center )
{
	OK_HolaUpperCenter = center;
}

void OK_PutSprtHolaUpper( FVECTOR *pos, FVECTOR *vec )
{
	Work *work;
	FVECTOR	*sc_pos1;
	FVECTOR	*sc_pos2;
	FVECTOR	fvtemp;
	int	num;
	int	*life;


	if( OK_SPRT_HOLA_UPPER_WORK == NULL ){
		void *NewSprtHolaUpper( void );
		NewSprtHolaUpper();
	}
	if( OK_SPRT_HOLA_UPPER_WORK == NULL ) return; // それでもやっぱり起動できなかった
	work = OK_SPRT_HOLA_UPPER_WORK;

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
	OK_HolaUpperCenter = NULL;
	OK_SPRT_HOLA_UPPER_WORK = NULL;
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_TOTAL ; j++ ){
		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

#if 0
		angle = TPI*rnd();
		uvrgbwh1->w = uvrgbwh0->w = (int)(cosf( angle ) * (float)SIZE);
		uvrgbwh1->h = uvrgbwh0->h = (int)(sinf( angle ) * (float)SIZE);
#else
		uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = SIZE;
#endif

		uvrgbwh1->r = uvrgbwh0->r = P_RGB_MAX;
		uvrgbwh1->g = uvrgbwh0->g = P_RGB_MAX;
		uvrgbwh1->b = uvrgbwh0->b = P_RGB_MAX;
		uvrgbwh1->a = uvrgbwh0->a = 0;

		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			j;
	int			*col_dat;
	int			*life;

	work->now_num = 0;

	col_dat = work->col_dat;
	for ( j=0; j<LIFE_MAX; j++ ){
		(*col_dat++) = (int)((float)P_ALPHA_MAX * sinf( PI * (float)j / (float)(LIFE_MAX-1) )   );
	}


	life    = work->life;
	for ( j = 0 ; j < N_TOTAL ; j++ ){
		(*life++) = LIFE_MAX;
	}


//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
//	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex );
	work->clock = prim->buffer_clock;

	work->fade_out_started = 0;

	return 0 ;
}


void *NewSprtHolaUpper( void )
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
	OK_SPRT_HOLA_UPPER_WORK = work;
	return (void *)work ;
}
