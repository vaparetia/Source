//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	c_box_dust.c
	ダンボールの壊れ破片
	2000/10/19 S.Okajima
	$Id: c_box_dust.c,v 1.1.1.3 2002/11/19 11:47:02 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

#define	FRAGMENT_NAME1	(13925923)	/* cbx_a_frg1_cm.kms */
//#define	FRAGMENT_NAME2	(13958691)	/* cbx_a_frg2_cm.kms */
//#define	FRAGMENT_NAME3	(13991459)	/* cbx_a_frg3_cm.kms */

#define	N_PARTS		(4)

#define	DUST_GRAVITY	( P_GRAVITY * 0.5f )
#define	DECAY_VECTOR	( 0.96f )

#define	COMDL_ALPHA		( 128 )

#define	INIT_SPEED		( 50.0f )
#define	RAND_SPEED		( 50.0f )

#define	LIFE_TIME		( 120 )

extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int			life;

	DG_COMDL	*comdl ;
	FVECTOR		vec[N_PARTS];
	SVECTOR		rot[N_PARTS];
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	j;
	int	alpha;
	DG_COMDL_POS	*comdl_pos;
	FVECTOR			*vec;
	SVECTOR			*rot;

	work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );

	if( work->life > LIFE_TIME/4 ){
		alpha = COMDL_ALPHA; 
	}else{
		alpha = COMDL_ALPHA * work->life / (LIFE_TIME/4); 
	}

	comdl_pos = work->comdl->pos;
	vec       = work->vec;
	rot       = work->rot;
	for ( j=0; j<N_PARTS; j++ ){
		DG_SetPos( &comdl_pos->world );
		DG_RotatePos( rot );
		DG_GetPos( &comdl_pos->world );
		_sceVu0AddVector( (FVECTOR *)&comdl_pos->world.m[3], (FVECTOR *)&comdl_pos->world.m[3], vec ) ;
		_sceVu0ScaleVector( vec, vec, DECAY_VECTOR );
		vec->vy += DUST_GRAVITY;
		comdl_pos->color.vw = alpha ;
		vec++;
		rot++;
		comdl_pos++;
	}

	work->life--;
	if( work->life <= -1 ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	if( work->comdl != NULL ){
		DG_DequeueComdlObjs( work->comdl );
		DG_FreeComdl( work->comdl );
		work->comdl = NULL;
	}
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, FVECTOR *force )
{
	FVECTOR		*vec;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	FVECTOR		randam_sphere;
	DG_DEF		*def ;
	DG_COMDL_POS	*comdl_pos;
	SVECTOR		*rot;
	SVECTOR		svtemp;
	int			j;
	int			itemp;
	int			col_r;
	int			col_g;
	int			col_b;

	work->life = LIFE_TIME;

	DG_COPY_VEC( &fvtemp0, force ) ;
	fvtemp0.vy = (fvtemp0.vy > 0.0f)? fvtemp0.vy: 0.0f;
	_sceVu0Normalize( &fvtemp0, &fvtemp0 );
	_sceVu0ScaleVector( &fvtemp0, &fvtemp0, INIT_SPEED );

	//      ambient                        parallel
	col_r = ((int)DG_ColorMatrix.m[3][0] + (int)DG_ColorMatrix.m[0][0])/2;
	col_g = ((int)DG_ColorMatrix.m[3][1] + (int)DG_ColorMatrix.m[0][1])/2;
	col_b = ((int)DG_ColorMatrix.m[3][2] + (int)DG_ColorMatrix.m[0][2])/2;
#if 0
	itemp = (col_r > col_g)? col_r : col_g;
	itemp = (itemp > col_b)? itemp : col_b;
	col_r = 127 + 128 * col_r / itemp;
	col_g = 127 + 128 * col_g / itemp;
	col_b = 127 + 128 * col_b / itemp;
#else
	itemp = (col_r > col_g)? col_r : col_g;
	itemp = (itemp > col_b)? itemp : col_b;
	if( itemp == 0 ) itemp = 1;
	col_r = 255 * col_r / itemp;
	col_g = 255 * col_g / itemp;
	col_b = 255 * col_b / itemp;
#endif

	randam_sphere.vx = 0.0f;
	randam_sphere.vy = RAND_SPEED;
	randam_sphere.vz = 0.0f;
	svtemp.vz = 0;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( FRAGMENT_NAME1, 'k' ) ) ;
	if( def==NULL ) return -1;
	work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_DEFAULT|DG_COMDL_SEMITRANS, N_PARTS, 0 );
	if( work->comdl==NULL ) return -1;
	DG_QueueComdlObjs( work->comdl );

	comdl_pos = work->comdl->pos;
	vec = work->vec;
	rot = work->rot;
	for ( j=0; j<N_PARTS; j++ ){
		rot->vx = irnd()%128-64;
		rot->vy = irnd()%128-64;
		rot->vz = irnd()%128-64;
		DG_SetPos2( pos, rot ) ;
		DG_GetPos( &comdl_pos->world );

		svtemp.vx = irnd()%2048;
		svtemp.vy = irnd()%4096;
		DG_SetPos2( &DG_ZeroVector, &svtemp ) ;
		DG_RotVector( &randam_sphere, &fvtemp1, 1 );
		_sceVu0AddVector( &fvtemp0, &fvtemp0, &fvtemp1 );
		_sceVu0ScaleVector( vec, &fvtemp0, rnd()*0.5f+0.5f );
		_sceVu0AddVector( (FVECTOR *)&comdl_pos->world.m[3], (FVECTOR *)&comdl_pos->world.m[3], vec ) ;

		comdl_pos->color.vx = col_r;
		comdl_pos->color.vy = col_g;
		comdl_pos->color.vz = col_b;
		comdl_pos->color.vw = COMDL_ALPHA ;

		vec++;
		rot++;
		comdl_pos++;
	}
	return 0 ;
}


void *NewBoxBreakDust( FVECTOR *pos, FVECTOR *force )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, pos, force ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
