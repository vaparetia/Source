//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	smoke_mitsukoshi.c
	三越ケムリ
	2001/08/22 S.Okajima
	$Id: smoke_mitsukoshi.c,v 1.1.1.3 2002/11/19 11:47:36 Yoshizawa1 Exp $

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

#include <libutl.h>


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"


#define	SCR_POS		(SCRPAD_ADDR)

//#define	N_VERTS		(16)
#define	N_VERTS		(4)
//#define	N_PRIMS		(0x4000/16/N_VERTS)
#define	N_PRIMS		(1)
#define	N_LOOPS		(8)

#define	DATA_NUM	(N_LOOPS+1)

#define	WIND_SENSIVILITY	(0.1f)
#define	INTERP_RATIO	(0.95f)

/*----------------------------------------------------------------*/
extern int OK_GetLocalWind2( FVECTOR *pos, FVECTOR *output );
extern float	OK_slow_param;
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	FMATRIX		world;
	FVECTOR		wind;
	FVECTOR		dest_pos[DATA_NUM];
	FVECTOR		dest_vec[DATA_NUM];
	FVECTOR		intr_vec[DATA_NUM];
	float		wind_ratio[DATA_NUM];
	float		alpha_max;
	float		height;
	float		wind_sensivility;
	float		angle[N_LOOPS*N_PRIMS*N_VERTS];
	int			life;
	int			life_max;

	int			num;

	DG_PRIM2	*prim ;

} Work ;

/* ---------------------------------------------------------------- */
static void GetWind( FVECTOR *pos, FVECTOR *wind )
{
	OK_GetLocalWind2( pos, wind );
//	wind->vy = DG_FABS( wind->vy );
	wind->vy*= 0.5f;
	_sceVu0ScaleVector( wind, wind, OK_slow_param*WIND_SENSIVILITY );
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2	*prim ;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*sc_pos;
	FVECTOR		*dest_pos;
	FVECTOR		*dest_vec;
	FVECTOR		*intr_vec0;
	FVECTOR		*intr_vec1;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	FVECTOR		wind;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int			i,j;
	int			clock;
	int			itemp;
	float		alpha;
	float		life_ratio;
	float		inter_ratio;
	float		before_len;
	float		len;
	float		limit_size;
	float		limit_size_min;
	float		*angle;
	float		*wind_ratio;

	GetWind( (FVECTOR *)work->world.m[3], &wind );
	_sceVu0ScaleVector( &fvtemp0, &work->wind,        INTERP_RATIO );
	_sceVu0ScaleVector( &fvtemp1, &wind,       1.0f - INTERP_RATIO );
	_sceVu0AddVector( &work->wind, &fvtemp0, &fvtemp1 );



	prim = work->prim;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	dest_pos = work->dest_pos;
	dest_vec = work->dest_vec;
	wind_ratio = work->wind_ratio;
	for( i=0; i<DATA_NUM; i++ ){
		_sceVu0AddVector( dest_pos, dest_pos, dest_vec );
		_sceVu0ScaleVector( &fvtemp0, &work->wind, *wind_ratio );
//		_sceVu0AddVector( dest_vec, dest_vec, &fvtemp0 );
		_sceVu0AddVector( dest_pos, dest_pos, &fvtemp0 );

//if( work->life == 1 ) printf("dest_pos:%d::%f %f %f\n",i,dest_pos->vx,dest_pos->vy,dest_pos->vz);
		wind_ratio++;
		dest_pos++;
		dest_vec++;
	}

	work->dest_vec[work->num].vy -= P_GRAVITY*0.2f;

	dest_pos  = work->dest_pos;
	intr_vec0 = work->intr_vec;
	//次の点までのベクトル、そのまま
	_sceVu0SubVector( intr_vec0, &dest_pos[1], &dest_pos[0] );
	_sceVu0ScaleVector( intr_vec0, intr_vec0, 1.0f/(float)(N_PRIMS*N_VERTS) );
	DG_COPY_VEC( &fvtemp0, intr_vec0 );
	dest_pos++;
	intr_vec0++;
	for( i=1; i<DATA_NUM-1; i++ ){
		_sceVu0SubVector( intr_vec0, &dest_pos[1], &dest_pos[0] );
		_sceVu0ScaleVector( intr_vec0, intr_vec0, 1.0f/(float)(N_PRIMS*N_VERTS) );

		//次の点までのベクトル、との中間地点
		_sceVu0AddVector( intr_vec0, intr_vec0, &fvtemp0 );
		_sceVu0ScaleVector( intr_vec0, intr_vec0, 0.5f );

		DG_COPY_VEC( &fvtemp0, intr_vec0 );
		dest_pos++;
		intr_vec0++;
	}
#if 0
	//次の点までのベクトル、そのまま
	_sceVu0SubVector( intr_vec0, &dest_pos[1], &dest_pos[0] );
	_sceVu0ScaleVector( intr_vec0, intr_vec0, 1.0f/(float)(N_PRIMS*N_VERTS) );
#else
	DG_COPY_VEC( intr_vec0, &fvtemp0 );
#endif

#if 0
if(GV_Time%60==0){
	dest_pos  = work->dest_pos;
	printf("0:%f %f %f\n",dest_pos->vx,dest_pos->vy,dest_pos->vz);	dest_pos++;
	printf("1:%f %f %f\n",dest_pos->vx,dest_pos->vy,dest_pos->vz);	dest_pos++;
	printf("2:%f %f %f\n",dest_pos->vx,dest_pos->vy,dest_pos->vz);	dest_pos++;
	printf("3:%f %f %f\n",dest_pos->vx,dest_pos->vy,dest_pos->vz);	dest_pos++;
	printf("4:%f %f %f\n",dest_pos->vx,dest_pos->vy,dest_pos->vz);	dest_pos++;
	printf("5:%f %f %f\n",dest_pos->vx,dest_pos->vy,dest_pos->vz);	dest_pos++;
	printf("6:%f %f %f\n",dest_pos->vx,dest_pos->vy,dest_pos->vz);	dest_pos++;
	printf("7:%f %f %f\n",dest_pos->vx,dest_pos->vy,dest_pos->vz);	dest_pos++;
}
#endif

#if 1

	life_ratio = (float)work->life / (float)work->life_max;
	if( life_ratio < 0.0f ) life_ratio = 0.0f;

	alpha = work->alpha_max * sinf( PI * life_ratio );
	if( alpha < 0.0f ) alpha = 0.0f;

	limit_size_min = work->height * (2.0f / (float)(N_LOOPS*N_PRIMS*N_VERTS));
	limit_size = limit_size_min*8.0f;

	intr_vec0 = work->intr_vec;
	intr_vec1 = work->intr_vec;
	intr_vec1++;;
	sc_pos    = prim->pos[  clock];
	uvrgbwh   = prim->uvrgb[clock];
	angle     = work->angle;
	DG_COPY_VEC( &fvtemp0, &DG_ZeroVector );
	before_len = 0.0f;
	for( i=0; i<N_LOOPS; i++ ){
		pos0 = SCR_POS;
		pos1 = SCR_POS;
		pos1++;
		DG_COPY_VEC( pos0, &fvtemp0 );
		if( pos0->vy < before_len ) pos0->vy = before_len;
		for ( j=1; j<N_PRIMS*N_VERTS; j++ ){
			inter_ratio = (float)j * (1.0f / (float)(N_PRIMS*N_VERTS));

			_sceVu0ScaleVector( &fvtemp0, intr_vec0, 1.0f-inter_ratio );
			_sceVu0ScaleVector( &fvtemp1, intr_vec1,      inter_ratio );
			_sceVu0AddVector( &fvtemp0, &fvtemp0, &fvtemp1 );

			_sceVu0AddVector( pos1, pos0, &fvtemp0 );
			len = GV_VecLen3F( &fvtemp0 )*3.0f;
			if( len < limit_size_min ) len = limit_size_min;
			itemp = (int)(alpha * (limit_size - len) / limit_size);
//			if( pos1->vy < len ) pos1->vy = len;
//if(GV_Time%300==0)printf("%f %f\n",pos0->vy,before_len);
			if( pos0->vy < before_len ) pos0->vy = before_len;
			before_len = len;

			if( itemp < 1 ){
//			if( 0 ){
				uvrgbwh->a = 0;
				uvrgbwh->w = 0;
				uvrgbwh->h = 0;
			}else{
				uvrgbwh->a = itemp;
//				uvrgbwh->a = 255;
				uvrgbwh->w = (int)(len * sinf( *angle ));
				uvrgbwh->h = (int)(len * cosf( *angle ));
			}
			if(j&1){
				(*angle)+= PI*0.004f;
			}else{
				(*angle)-= PI*0.004f;
			}

			angle++;
			pos0++;
			pos1++;
			uvrgbwh++;
		}
		DG_COPY_VEC( &fvtemp0, pos0 );
//if(i==0)printf("%x\n",pos1);
		OK_Scr_Mem( sc_pos, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS);
		sc_pos += N_PRIMS*N_VERTS;
		intr_vec0++;;
		intr_vec1++;;
	}

#endif

	if( work->life-- < 0 ) GV_DestroyActor( work ) ;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int col )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*sc_uvr0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*sc_uvr1 ;	/* スプライト用 */
	int		i,j ;
	float	width;
	float	*angle;
	int		col_r;
	int		col_g;
	int		col_b;
	int		count;

	col_r = ((col>>24)&255);
	col_g = ((col>>16)&255);
	col_b = ((col>> 8)&255);

	width = work->height * (1.0f / (float)(N_PRIMS*N_VERTS/2));

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


	//-------------------------------
	count = N_LOOPS*N_PRIMS*N_VERTS;
	sc_uvr0 = prim->uvrgb[0];
	sc_uvr1 = prim->uvrgb[1];
	angle   = work->angle;
	for ( i=0; i<N_LOOPS*2; i++ ){
		uvrgbwh = SCR_POS ;
		for ( j=0; j<N_PRIMS*N_VERTS/2; j++ ){
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			(*(angle++)) = TPI*rnd();
//			local_width = width * (0.25f + 0.75f*(float)(count)*(1.0f / (float)(N_LOOPS*N_PRIMS*N_VERTS)));
//			local_width = 100.0f;
//			uvrgbwh->w = (int)(local_width * sinf( angle ));
//			uvrgbwh->h = (int)(local_width * cosf( angle ));

			uvrgbwh->r = col_r;
			uvrgbwh->g = col_g;
			uvrgbwh->b = col_b;

			uvrgbwh->a = 0;

			count--;
			uvrgbwh++ ;
		}
		OK_Scr_Mem( sc_uvr0, SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS*N_PRIMS/2 ) ;
		OK_Scr_Mem( sc_uvr1, SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS*N_PRIMS/2 ) ;
		sc_uvr0+= N_VERTS*N_PRIMS/2;
		sc_uvr1+= N_VERTS*N_PRIMS/2;
	}
}


static int GetResources( Work *work, int col )
{
	FVECTOR		keep_pos;
	FVECTOR		fvtemp;
	SVECTOR		svtemp;
	FVECTOR		*dest_pos;
	FVECTOR		*dest_vec;
	DG_PRIM2	*prim ;
	DG_TEX		*tex=NULL ;
	int	i;
	float		*wind_ratio;
	float		num_ratio;

	GetWind( (FVECTOR *)work->world.m[3], &work->wind );

	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	svtemp.vz = 0;
	dest_pos   = work->dest_pos;
	dest_vec   = work->dest_vec;
	wind_ratio = work->wind_ratio;
	DG_COPY_VEC( &keep_pos, &DG_ZeroVector );
	for( i=0; i<DATA_NUM; i++ ){
		num_ratio = (float)i / (float)DATA_NUM;
//		fvtemp.vz = (work->height * num_ratio * (1.0f+rnd())) / (float)(work->life);
		fvtemp.vz = (work->height * num_ratio * 0.5f) / (float)(work->life);
		svtemp.vx =-((irnd()>>8)&255);
		svtemp.vy = ((irnd()>>8)&4095);
		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_RotVector( &fvtemp, dest_vec, 1 );
		_sceVu0AddVector( &keep_pos, &keep_pos, dest_vec );
		DG_COPY_VEC( dest_pos, &keep_pos );

//		*wind_ratio = rnd();
		*wind_ratio = num_ratio*0.25f + rnd()*0.75f;

		wind_ratio++;
		dest_pos++;
		dest_vec++;
	}

#if 1
	work->num = (irnd()>>8)%DATA_NUM;
#else
	work->num = DATA_NUM/2 + ((irnd()>>8)%4)-2;
	if( work->num < 0 ){
		work->num = 0;
	}else if( work->num > DATA_NUM-1 ){
		work->num = DATA_NUM-1;
	}
#endif
	work->wind_ratio[work->num]*= -1.0f;

//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 7744599 /*"hamon09_add_alp"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );


//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
//	tex = DG_GetTexture( 12020883 /*"chi02_alp"*/ );

	tex = DG_GetTexture( 13069459 /*"chi03_alp"*/ );

//	tex = DG_GetTexture( 14118035 /*"chi04_alp"*/ );
//	tex = DG_GetTexture( 15166611 /*"chi05_alp"*/ );

	work->alpha_max = (float)( col&255 );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex, col );
	DG_InvisiblePrim2( prim );
//	DG_VisiblePrim2( prim );

	prim->root = &work->world;

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewSmokeMitsukoshi( FVECTOR *center, float height, int col, int life, float wind_sensivility )
{
	Work		*work ;

	OPERATOR() ;
//	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), GV_PRIO_MAX-32 ) ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_MAT( &work->world, &DG_UnitMatrix );
		DG_COPY_VEC( (FVECTOR *)work->world.m[3], center );
		work->height = height;
		work->life = work->life_max = life;
		work->wind_sensivility = wind_sensivility;

		if ( GetResources( work, col ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


