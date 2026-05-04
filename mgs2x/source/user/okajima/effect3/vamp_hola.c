//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vamp_hola.c
	バンプ戦：オーラ

	2001/07/06 S.Okajima
	$Id: vamp_hola.c,v 1.1.1.3 2002/11/19 11:47:38 Yoshizawa1 Exp $
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
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define	DECAY_RATIO		(0.95f)

#define	SCR_POS_D		(SCRPAD_ADDR)
#define	SCR_VEC_D		(SCRPAD_ADDR + 0x2000)
#define	N_VERTS_D		(32)
#define	N_PRIMS_D		(0x2000/16/N_VERTS_D)
#define	N_LOOPS_D		(2)

//#define	FADE_OUT_TIME	(120)
#define	FADE_OUT_TIME	(60)

#define	GRAVI_HEIGHT_MIN	(2000.0f)
#define	GRAVI_HEIGHT_RND	(1000.0f)


#define	G_PARAM			(1000.0f)
#define	VEC_MIN			(0.0f)
#define	VEC_RND			(10.0f)
#define	FIND_NUM		(4)

#define	LIFE_MIN		(0.0f)
#define	LIFE_RND		(180.0f)
#define	LIFE_MAX		(LIFE_MIN+LIFE_RND)
#define	LIFE_HLF		(LIFE_MAX*0.5f)


#define	JOINT_NUM		(22)

#define	POS_VS_UVS	(1 + 2)

#define	DIV_VERTS		(1)
#define	DIV_NORMS		(1)
#define	SCR_DIVISION	( DIV_VERTS + DIV_NORMS )

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

//#define	N_VERTS2		(16)
#define	N_VERTS2		(32)
//関節毎にスクラッチパット使用
#define	N_PRIMS2		( 64 )


#define	PARAM1		( (MAX_FVECTOR - N_VERTS2*POS_VS_UVS) / SCR_DIVISION )
#define	PARAM2		(PARAM1 * 16)

//#define	SCR_POS		((void *)SCRPAD_ADDR)
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCR_POS   + N_VERTS2 * 16)
#define	SCR_VERTS	(SCR_UVS   + N_VERTS2 * 16 * 2)
#define	SCR_NORMS	(SCR_VERTS + PARAM2 * DIV_VERTS)

//#define	NORMS_LIMIT		(0.75f)
#define	NORMS_LIMIT		(1.0f)


//#define	TEX_NAME		(10972307 /*"chi01_alp"*/)
#define	TEX_NAME		(3594043 /*"drop01_msk"*/)
//#define	TEX_NAME		(6715088 /*"rcm_l_msk"*/)
#define	RAND_WIDTH		(80.0f)

#if 1
#define	COL_R			(180)
#define	COL_G			(30)
#define	COL_B			(0)
#define	COL_A			(18)
#else
#define	COL_R			(140)
#define	COL_G			(160)
#define	COL_B			(180)
#define	COL_A			(18)
#endif

#define	SIZE			(80)

#define	HOLA_LIFE	(60)

#define	SCALE_UP	(50)
#define	FREQ		(110)
#define	FREQ2		(273)
#define	FREQ3		(136)
#define	RADIUS_MIN	(400.0f)
#define	RADIUS_ADD	(500.0f)
#define	RADIUS_Y	(1000.0f)
#define	UPPER		(1200.0f)

#define	FADE_IN_MAX	(120)

/* ---------------------------------------------------------------- */
extern void OK_PutSprtHolaUpperCenter( FVECTOR *center );
extern void OK_PutSprtHolaUpper( FVECTOR *pos, FVECTOR *vec );
extern void OK_PutPolyHolaUpperCenter( FVECTOR *center );
extern void OK_PutPolyHolaUpper( FVECTOR *pos, FVECTOR *vec );
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	DG_OBJS		*objs;
	DG_PRIM2	*prim ;
	FMATRIX		*world[JOINT_NUM];
	FVECTOR		*verts[JOINT_NUM];
	FVECTOR		*norms[JOINT_NUM];
	FVECTOR		vec[N_LOOPS_D*N_PRIMS_D*N_VERTS_D];
	FVECTOR		g_center;
	FVECTOR		g_center_before;
	int			name;
	int			map;
	int			n_verts[JOINT_NUM];
	int			n_models;
	int			fade_out_started;
	int			fade_out_count;
	int			count;
} Work ;


/* ---------------------------------------------------------------- */
static int ReceiveSignal( void *pwork, int signal, int value )
{
	Work *work = pwork;

	switch(signal){
	  case 0:
		work->fade_out_started = 1;

#if 0
		if( value < 0 ){
			work->fade_out_count = FADE_OUT_TIME;
		}else{
			work->fade_out_count = DIRECT_TICK( value );
		}
#else
		work->fade_out_count = FADE_OUT_TIME;
#endif
		break;
	  default:
		return GV_DefaultSignalFunc( pwork, signal, value );// Must do this!!!
	}
	return 0;
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
#ifdef PSX2 ///
	/* スプライト用 */
#endif
	int i, n, m, o;
	int	max_num;
	int	clock;
	float	fade_alpha;
	float	scale;
	float	angle;
	float	angle2;
	float	angle3;
	float	radius;
	FVECTOR	direction;
	FVECTOR	fvtemp;
	FVECTOR	*buff;
	FVECTOR *from;
	FVECTOR *pos;
	FVECTOR	*vec;
	int		*n_verts;
	FVECTOR	**verts;
	FVECTOR	**norms;
	FMATRIX	**world;
//	FMATRIX	fmat;


	n = work->count%FREQ;
	m = work->count%FREQ2;
	o = work->count%FREQ3;
	angle  = TPI * n / FREQ;
	angle2 = TPI * m / FREQ2;
	angle3 = TPI * o / FREQ3;
	radius = RADIUS_MIN + RADIUS_ADD * (1.0f + (sinf( angle ) + sinf( angle2 ))*0.5f);

	DG_COPY_VEC( &fvtemp, (FVECTOR *)work->world[12]->m[3] );
	fvtemp.vx+= radius * sinf(angle);
//	fvtemp.vy+= UPPER + (RADIUS_MIN + RADIUS_ADD) * sinf( angle3 );
	fvtemp.vy+= UPPER + RADIUS_Y * sinf( angle3 );
	fvtemp.vz+= radius * cosf(angle);

	_sceVu0SubVector( &direction, &fvtemp, &work->g_center_before ) ;
	_sceVu0ScaleVector( &fvtemp, &direction, 0.25f );
	_sceVu0AddVector( &fvtemp, &fvtemp, &work->g_center_before ) ;
	DG_COPY_VEC( &work->g_center_before, &work->g_center );
	DG_COPY_VEC( &work->g_center, &fvtemp ) ;
//	work->g_center.vy-= GV_VecLen3F( &direction )*0.5f;
	OK_PutPolyHolaUpperCenter( &work->g_center );

//printf("dd:%f %f %f\n",direction.vx,direction.vy,direction.vz);


//AN_Test_Eye2( &work->g_center, 2 );



//	DG_InvisibleObjs(GM_PlayerBody->objs) ;


	prim   = work->prim;

	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim );


	DG_SwitchBuffPrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	clock = prim->buffer_clock;




	if( work->fade_out_started ){
		work->fade_out_count--;
		if( work->fade_out_count < 0 ){
			work->fade_out_count = 0;
			GV_DestroyActor( work ) ;
			DG_InvisiblePrim2( prim );
			return;
		}
	}else{
		work->fade_out_count++;
		if( work->fade_out_count > FADE_OUT_TIME ){
			work->fade_out_count = FADE_OUT_TIME;
		}
	}
	if( work->fade_out_count > FADE_OUT_TIME ){
		work->fade_out_count = FADE_OUT_TIME;
	}else if( work->fade_out_count < 0 ){
		work->fade_out_count = 0;
	}
	fade_alpha = (float)(COL_A * work->fade_out_count / FADE_OUT_TIME);




	direction.vx = 0.0f;
	direction.vy = 0.0f;
	direction.vz =-1.0f;
	direction.vw = 1.0f;
	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &direction, &direction, 1 );

	scale = 1.0f + SCALE_UP * (1.0f + sinf( TPI*(float)(work->count%FREQ)/(float)FREQ ))*0.5f;

	vec      = work->vec;
	n_verts  = work->n_verts;
	pos      = prim->pos[ clock ];
	uvrgbwh  = prim->uvrgb[ clock ];
	world    = work->world;
	verts    = work->verts;
	norms    = work->norms;
	i = work->n_models ; 
	max_num = N_PRIMS2*N_VERTS2;
	while( --i>=0 ){
		OK_Mem_Scr( (void *)SCR_VERTS, *verts,  sizeof(FVECTOR), *n_verts) ;
		OK_Mem_Scr( (void *)SCR_NORMS, *norms,  sizeof(FVECTOR), *n_verts) ;

		DG_SetPos( *world );
		DG_RotVector( (void *)SCR_NORMS, (void *)SCR_NORMS, *n_verts );

		from = (void *)SCR_VERTS;
		buff = (void *)SCR_NORMS;
		n    = *n_verts;
		while ( -- n >= 0 ) {
			buff->vw = DG_FABS(_sceVu0InnerProduct( buff, &direction ));

/*
			pos->vx += frnd()*RAND_WIDTH;
			pos->vy += frnd()*RAND_WIDTH;
			pos->vz += frnd()*RAND_WIDTH;
*/

			buff->vw = (NORMS_LIMIT - buff->vw) / NORMS_LIMIT;
			buff->vw*= buff->vw;
//			uvrgbwh->a = (int)(fade_alpha * (buff->vw - NORMS_LIMIT) / (1.0f - NORMS_LIMIT) );
//			uvrgbwh->a = (int)(fade_alpha * (NORMS_LIMIT - buff->vw) / NORMS_LIMIT );
			uvrgbwh->a = (int)(fade_alpha * buff->vw );
//			uvrgbwh->a = (int)(fade_alpha);

			DG_PutVector( from, pos, 1 );
//			_sceVu0ScaleVector( buff, buff, scale );
//			_sceVu0AddVector( pos, pos, buff );



			if( ((irnd()>>8)&127)==0 ){
				OK_PutPolyHolaUpper( pos, buff );
			}


			uvrgbwh++;
			pos++;
			if( --max_num <= 0 ) break;

			buff ++ ;
			from ++ ;
		}
		if( max_num <= 0 ) break;

		n_verts++;
		world++;
		verts++;
		norms++;
	}

//printf("%d\n",max_num);
	while ( --max_num >= 0 ) {
		(uvrgbwh++)->a = 0;
	}


	work->count++;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim   = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;
//	float	size;
//	float	angle;

	DG_ConfigPrim2Tex( prim, tex );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 2, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS2 ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
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

			uvrgbwh1->r = uvrgbwh0->r = COL_R;
			uvrgbwh1->g = uvrgbwh0->g = COL_G;
			uvrgbwh1->b = uvrgbwh0->b = COL_B;
			uvrgbwh1->a = uvrgbwh0->a = COL_A;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS2 * N_VERTS2 );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS2 * N_VERTS2 );
}

static int GetResources( Work *work )
{
	CV2_DEF		*cvd_def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		*vec;
	FVECTOR		*fvtemp0;
	FVECTOR		*fvtemp1;
	int		i,j ;

	work->fade_out_count = 0;
	work->n_models = work->objs->n_models < JOINT_NUM ? work->objs->n_models : JOINT_NUM ; 

//	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	tex = DG_GetTexture( TEX_NAME );
	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( prim );

	i = N_LOOPS_D*N_PRIMS_D*N_VERTS_D;
	vec = work->vec;
	while( --i>=0 ){
		vec->vw =-1.0f;
		vec++;
	}

	/* モデルの共有頂点データ取得 */
	cvd_def = GV_GetCache( GV_CacheID( 3075579 /*"sna_skl3"*/, 'c' ) );


	i = work->n_models ; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
		if( work->n_verts[i] > PARAM1 ){
			work->n_verts[i] = PARAM1;
		}
		work->world[i]   = &work->objs->objs[i].world;
		fvtemp0 = work->verts[i]   = cvd_def->models[i].verts;
		fvtemp1 = work->norms[i]   = cvd_def->models[i].norms;
		for( j=0; j<work->n_verts[i]; j++ ){
			if( _sceVu0InnerProduct( fvtemp0, fvtemp1 ) < 0.0f ){
				_sceVu0ScaleVector( fvtemp1, fvtemp1, -1.0f );
			}
			fvtemp0++;
			fvtemp1++;
		}
	}







	work->fade_out_started = 0;

#if 0
#if 1
	{
		extern void *NewSprtHola( FVECTOR *edge0, FVECTOR *edge1 );
		NewSprtHola(
			(FVECTOR *)work->world[ 5]->m[3],
			(FVECTOR *)work->world[ 3]->m[3] );
		NewSprtHola(
			(FVECTOR *)work->world[ 4]->m[3],
			(FVECTOR *)work->world[ 8]->m[3] );
		NewSprtHola(
			(FVECTOR *)work->world[ 7]->m[3],
			(FVECTOR *)work->world[ 9]->m[3] );
	}
#else
	{
		extern void *NewStripHola( FVECTOR *edge0, FVECTOR *edge1 );
		NewStripHola(
			(FVECTOR *)work->world[ 5]->m[3],
			(FVECTOR *)work->world[ 3]->m[3] );
		NewStripHola(
			(FVECTOR *)work->world[ 4]->m[3],
			(FVECTOR *)work->world[ 8]->m[3] );
		NewStripHola(
			(FVECTOR *)work->world[ 7]->m[3],
			(FVECTOR *)work->world[ 9]->m[3] );
	}
#endif
#endif

	DG_COPY_VEC( &work->g_center_before, (FVECTOR *)work->world[12]->m[3] );
	DG_COPY_VEC( &work->g_center, &work->g_center_before ) ;


	return (0);
}


/* ---------------------------------------------------------------- */
void *NewVampHola( DG_OBJS *objs )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->objs = objs;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GV_SetActorSignalFunc( work, ReceiveSignal ) ;
	}
	return (void *)work ;
}
