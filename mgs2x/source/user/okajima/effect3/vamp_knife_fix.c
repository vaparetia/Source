//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vamp_knife_fix.c
	バンプ戦：ナイフ縛り

	2001/06/24 S.Okajima
	$Id: vamp_knife_fix.c,v 1.1.1.3 2002/11/19 11:47:38 Yoshizawa1 Exp $
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

#define	FADE_OUT_TIME	(30)

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

/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
#define	N_VERTS2		(32)
//#define	N_VERTS2		(16)
//関節毎にスクラッチパット使用
#define	N_PRIMS2		( JOINT_NUM )


#define	PARAM1		( (MAX_FVECTOR - N_VERTS2*POS_VS_UVS) / SCR_DIVISION )
#define	PARAM2		(PARAM1 * 16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCR_POS   + N_VERTS2 * 16)
#define	SCR_VERTS	(SCR_UVS   + N_VERTS2 * 16 * 2)
#define	SCR_NORMS	(SCR_VERTS + PARAM2 * DIV_VERTS)


#if 0
#define	TEX_NAME		(6715088 /*"rcm_l_msk"*/)
#define	RAND_WIDTH		(0.0f)
#define	P_ALPHA_MAX		(255)
#define	P_RGB_MAX		(255)
#define	SIZE			(20)
#define	SHIFT_UP		((float)SIZE*1.5f)
#else
#define	TEX_NAME		(3594043 /*"drop01_msk"*/)
#define	RAND_WIDTH		(80.0f)
#ifdef KP_XBOX
#define	P_ALPHA_MAX		(64)  /* これくらい濃くしないと、うまく出ない */
#else
#define	P_ALPHA_MAX		(8)
#endif
#define	P_RGB_MAX		(8)
#define	SIZE			(80)
#define	SHIFT_UP		((float)SIZE*0.75f)
#endif

#define	F_ALPHA		((float)P_ALPHA_MAX)



#define	SHIFT_ROT		(256.0f)

#define	NORMS_LIMIT		(0.5f)
#define	LENGTH_LIMIT	(8000.0f)
#define	LENGTH_MIN		(5000.0f)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;
	int			name;
	int			map;

	DG_OBJS		*objs;
	FVECTOR		t_pos;
	FVECTOR		l_pos;

	int			n_models;
	int			n_now;

	int			fade_out_started;
	int			fade_out_count;

	DG_PRIM2	*prim_d ;
	DG_PRIM2	*prim ;
	FMATRIX		*world[JOINT_NUM];
	FVECTOR		*verts[JOINT_NUM];
	FVECTOR		vec[N_LOOPS_D*N_PRIMS_D*N_VERTS_D];
	int			n_verts[JOINT_NUM];

	void		*kirari_work;

	FMATRIX		knife_world;

} Work ;


/* ---------------------------------------------------------------- */
static int ReceiveSignal( void *pwork, int signal, int value )
{
	Work *work = pwork;

	switch(signal){
	  case 0:
		work->fade_out_started = 1;
		work->fade_out_count = FADE_OUT_TIME;
		GV_CallChildSignalFunc( work, 0, 0 );
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
	DG_PRIM2	*prim_d ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int i, k, n;
	int	max_num;
	int	clock;
	int	count_d;
	int	fade_alpha;
	int	count_total;
	float	data_0;
	float	data_1;
	float	len;
	float	floor_height;
	FVECTOR	l_pos;
	FVECTOR	t_pos;
	FVECTOR	direction;
	FVECTOR	diff;
	FVECTOR	cam_back;
	FVECTOR *from;
	FVECTOR *pos;
	FVECTOR	*sc_pos0;
	FVECTOR	*sc_pos1;
	FVECTOR	*sc_vec;
	FVECTOR	*pos_d;
	FVECTOR	*vec;
	int		*n_verts;
	FVECTOR	**verts;
	FMATRIX	**world;

//AN_Test_Eye2( work->t_pos, 2 );
//AN_Test_Eye2( work->l_pos, 2 );

	prim   = work->prim;
	prim_d = work->prim_d;

	if( work->fade_out_started ){
		if( work->kirari_work != NULL ){
			GV_DestroyOtherActor( work->kirari_work );
			work->kirari_work = NULL;
		}

		fade_alpha = P_ALPHA_MAX * work->fade_out_count / FADE_OUT_TIME;
		work->fade_out_count--;
		if( work->fade_out_count < 0 ){
			work->fade_out_count = 0;
			GV_DestroyActor( work ) ;
			DG_InvisiblePrim2( prim );
			DG_InvisiblePrim2( prim_d );
			return;
		}
	}else{
		fade_alpha = P_ALPHA_MAX;
	}


	cam_back.vx = 0.0f;
	cam_back.vy = 0.0f;
	cam_back.vz = -10000.0f;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &cam_back, &cam_back, 1 );


	DG_COPY_VEC( &l_pos, &work->l_pos );
	DG_COPY_VEC( &t_pos, &work->t_pos );
	floor_height = t_pos.vy + SHIFT_UP;
	_sceVu0SubVector( &direction, &t_pos, &l_pos ) ;
	direction.vy = 0.0f;
	_sceVu0Normalize( &direction, &direction );
	_sceVu0ScaleVector( &direction, &direction, 2.0f );
	data_1 = t_pos.vy - l_pos.vy;

//	t_pos.vy+= GRAVI_HEIGHT_MIN + GRAVI_HEIGHT_RND*rnd();


	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim );
	 //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	clock = prim->buffer_clock;

	DG_VisiblePrim2( prim_d );
	DG_SwitchBuffPrim2( prim_d );
	GM_GroupPrim2( prim_d, GM_CurrentStageMap ) ;

	count_total = N_LOOPS_D*N_PRIMS_D*N_VERTS_D;
	uvrgbwh = prim_d->uvrgb[  clock];
	sc_pos0 = prim_d->pos[1-clock];
	sc_pos1 = prim_d->pos[  clock];
	sc_vec  = work->vec;
	for( k=0; k<N_LOOPS_D; k++ ){
		OK_Mem_Scr( SCR_POS_D, sc_pos0, sizeof(FVECTOR), N_PRIMS_D*N_VERTS_D) ;
		OK_Mem_Scr( SCR_VEC_D, sc_vec,  sizeof(FVECTOR), N_PRIMS_D*N_VERTS_D) ;
		pos_d = SCR_POS_D;
		vec   = SCR_VEC_D;
		for( i=0; i<N_PRIMS_D*N_VERTS_D; i++ ){
			_sceVu0SubVector( &diff, &t_pos, pos_d );
			len = _sceVu0InnerProduct( &diff, &diff ) ;
			if( len < 10000.0f ){
				DG_COPY_VEC( pos_d, &t_pos );
				vec->vw =-1.0f;
			}else{
				len = G_PARAM / len;
				_sceVu0ScaleVector( &diff, &diff, len );
				diff.vw =-1.0f;	// vec->vw は カウンタ
				_sceVu0AddVector( vec, vec, &diff );
			}
			if( vec->vw < 0.0f ){
				count_total--;
				uvrgbwh->a = 0;
			}else if( vec->vw < LIFE_HLF ){
				uvrgbwh->a = (int)((float)fade_alpha * vec->vw / LIFE_HLF);
			}else{
				uvrgbwh->a = fade_alpha;
			}

			_sceVu0AddVector( pos_d, pos_d, vec );
			pos_d->vy = floor_height;

			pos_d++;
			vec++;
			uvrgbwh++;
		}
		OK_Scr_Mem( sc_pos1, SCR_POS_D, sizeof(FVECTOR), N_PRIMS_D*N_VERTS_D) ;
		OK_Scr_Mem( sc_vec,  SCR_VEC_D, sizeof(FVECTOR), N_PRIMS_D*N_VERTS_D) ;
		sc_pos0+= N_PRIMS_D*N_VERTS_D;
		sc_pos1+= N_PRIMS_D*N_VERTS_D;
		sc_vec += N_PRIMS_D*N_VERTS_D;
	}


	count_d  = N_LOOPS_D*N_PRIMS_D*N_VERTS_D;
	pos_d    = prim_d->pos[clock];
	vec      = work->vec;
	n_verts  = work->n_verts;
	pos      = prim->pos[ clock ];
	uvrgbwh  = prim->uvrgb[ clock ];
	world    = work->world;
	verts    = work->verts;
	i = work->n_models; 
	while( --i>=0 ){
		OK_Mem_Scr( (void *)SCR_VERTS, *verts,  sizeof(FVECTOR), *n_verts) ;

		from = (void *)SCR_VERTS;
		n    = *n_verts;

		max_num = N_VERTS2;

		DG_SetPos( *world );
		while ( -- n >= 0 ) {
			if( (irnd()>>8)&1 ){
				DG_PutVector( from, pos, 1 );
				data_0  = pos->vy - l_pos.vy;
				pos->vx = l_pos.vx + (pos->vx - l_pos.vx) * (data_1) / (data_0) + frnd()*RAND_WIDTH;
				pos->vy = floor_height;
				pos->vz = l_pos.vz + (pos->vz - l_pos.vz) * (data_1) / (data_0) + frnd()*RAND_WIDTH;
				if( !work->fade_out_started ){
					if(n < work->n_now){
						while( count_d > 0 ){
							if(vec->vw < 0.0f){
								DG_COPY_VEC( pos_d, pos );
#if 0
								len = TPI*rnd();
								vec->vx = sinf(len) + direction.vx;
//								vec->vy = 1.0f + rnd();
								vec->vy = 0.0f;
								vec->vz = cosf(len) + direction.vz;
								_sceVu0ScaleVector( vec, vec, VEC_MIN + VEC_RND*rnd() );
#else
								DG_COPY_VEC( vec, &DG_ZeroVector );
#endif
								vec->vw = LIFE_MIN + LIFE_RND*rnd();
								count_d--;
								vec++;
								pos_d++;
								break;
							}else{
								count_d--;
								vec++;
								pos_d++;
							}
						}
					}
				}
				uvrgbwh->a = fade_alpha;
				uvrgbwh++;
				pos++;
				if( --max_num <= 0 ) break;
			}
			from ++ ;
		}

		while ( --max_num >= 0 ) {
			uvrgbwh->a = 0;
			uvrgbwh++;

			DG_COPY_VEC( pos, &cam_back );

			pos++;		/* skip させる */
		}

		n_verts++;
		world++;
		verts++;
	}

	work->n_now++;
	if( work->n_now >= work->n_models ) work->n_now = 0;


/*
	if( GV_Time%60==0 ){
		extern void *NewPointDogtagFlash( FVECTOR *pos );
		void *p0;
		p0 = NewPointDogtagFlash( &work->t_pos );
		GV_SetActorChild( work, p0 ) ;
	}
*/

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim   = OK_FreePrim2( work->prim );
	work->prim_d = OK_FreePrim2( work->prim_d );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

			uvrgbwh1->w = uvrgbwh1->h = uvrgbwh0->w = uvrgbwh0->h = SIZE ;

			uvrgbwh1->r = uvrgbwh0->r = P_RGB_MAX;
			uvrgbwh1->g = uvrgbwh0->g = P_RGB_MAX;
			uvrgbwh1->b = uvrgbwh0->b = P_RGB_MAX;
			uvrgbwh1->a = uvrgbwh0->a = P_ALPHA_MAX;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}
}


/* ---------------------------------------------------------------- */
static void InitPacket3( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*sc_uvrgbwh ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i,k ;

	DG_ConfigPrim2Tex( prim, tex );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );

	//-------------------------------
	sc_uvrgbwh = SCR_POS_D;
	for ( k = 0 ; k < N_PRIMS_D*N_VERTS2 ; k++ ){
		sc_uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		sc_uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		sc_uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		sc_uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		sc_uvrgbwh->q0 = 4096 ;
		sc_uvrgbwh->q1 = 4096 ;
		sc_uvrgbwh->f0 = 0x0fff ;
		sc_uvrgbwh->f1 = 0x0fff ;
		sc_uvrgbwh->w = sc_uvrgbwh->h = SIZE ;
		sc_uvrgbwh->r = P_RGB_MAX;
		sc_uvrgbwh->g = P_RGB_MAX;
		sc_uvrgbwh->b = P_RGB_MAX;
		sc_uvrgbwh->a = P_ALPHA_MAX;
		sc_uvrgbwh ++ ;
	}
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( i = 0 ; i < N_LOOPS_D ; i++ ){
		OK_Scr_Mem( uvrgbwh0, SCR_POS_D, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS_D*N_VERTS_D) ;
		OK_Scr_Mem( uvrgbwh1, SCR_POS_D, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS_D*N_VERTS_D) ;
		uvrgbwh0+= N_PRIMS_D*N_VERTS_D;
		uvrgbwh1+= N_PRIMS_D*N_VERTS_D;
	}
}


static int GetResources( Work *work )
{
	extern OBJECT	*GM_PlayerKageshibariBody ;
	CV2_DEF		*cvd_def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		*vec;
	int		i ;

	work->n_now = 0;

	//work->objs=GM_PlayerBody->objs;
	if ( GM_PlayerKageshibariBody == NULL ) return -1 ;
	work->objs=GM_PlayerKageshibariBody->objs;	/* 園山修正 2001/08/06 */

	work->n_models = work->objs->n_models < JOINT_NUM ? work->objs->n_models : JOINT_NUM ; 

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_models, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * work->n_models * N_VERTS2 );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * work->n_models * N_VERTS2 );

	tex = DG_GetTexture( TEX_NAME );
	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( prim );


	prim = work->prim_d = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS_D*N_PRIMS_D, N_VERTS_D );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_LOOPS_D * N_PRIMS_D * N_VERTS_D );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_LOOPS_D * N_PRIMS_D * N_VERTS_D );

	tex = DG_GetTexture( TEX_NAME );
	InitPacket3( work, prim, tex );
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
		work->verts[i]   = cvd_def->models[i].verts;
		work->world[i]   = &work->objs->objs[i].world;
	}

	work->fade_out_started = 0;

#if 0
	{
		extern void *NewStripDome( FVECTOR *center );
		GV_SetActorChild( work,
			NewStripDome( &work->t_pos )
		);
	}
#endif

	work->kirari_work = NULL;
	DG_COPY_MAT( &work->knife_world, &DG_UnitMatrix );
	DG_COPY_VEC( (FVECTOR *)work->knife_world.m[3], &work->t_pos );
	work->knife_world.m[3][1]+= 5.0f;
	{
		extern void *NewKirariShadowKnife( FMATRIX *world );
		work->kirari_work = NewKirariShadowKnife( &work->knife_world );
		GV_SetActorChild( work, work->kirari_work ) ;
	}

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewVampShadowTrapEffect( FVECTOR *t_pos, FVECTOR *l_pos )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->t_pos, t_pos);
		DG_COPY_VEC( &work->l_pos, l_pos);

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GV_SetActorSignalFunc( work, ReceiveSignal ) ;
	}
	return (void *)work ;
}
