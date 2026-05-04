//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  crash_debris.c
  散乱コモデル
  2000/03/15 Yuuta Kunibe	
  $Id: crash_debris.c,v 1.1.1.3 2002/11/19 11:44:35 Yoshizawa1 Exp $

*/

#ifdef PSX2
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../../shibata/util/ts_util.h"


/*----------------------------------------------------------*/
/*			extern                              */
/*----------------------------------------------------------*/
extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );
extern void *NewRunningSmoke2( FVECTOR* pos, float size );

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
			       int *rgba, int w, int h, int life, int *mode );


//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
     NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
		       (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )



#define	CMDL_NUM	(100)
#define COLOR		(0)//(128)
#define ALPHA		(128)
#define	GRAVITY		(5.0f)     

#define	LIFE		(90)


typedef	struct	{

    GV_ACT_EX	actor;
    
    DG_COMDL	*comdl;
    int		n_comdl;


    int 	count;

    FVECTOR	vec[CMDL_NUM];

} Work;




static inline void _ScaleMatrix( FMATRIX *out, FMATRIX *world, float r )
{
	FVECTOR temp = { r, r, r, 0.0f };

// yano /*未検証*/
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	vmul.xyzw			vf4, vf4,vf8
	vmul.xyzw			vf5, vf5,vf8
	vmul.xyzw			vf6, vf6,vf8
	sqc2				vf4,0x00(%0)
	sqc2				vf5,0x10(%0)
	sqc2				vf6,0x20(%0)
	": : "r"(out), "r"(world), "r"(&temp) );//:"memory" );
#else
	_sceVu0ScaleVectorXYZ( (FVECTOR*)out->m[0], (FVECTOR*)world->m[0], r );
	_sceVu0ScaleVectorXYZ( (FVECTOR*)out->m[1], (FVECTOR*)world->m[1], r );
	_sceVu0ScaleVectorXYZ( (FVECTOR*)out->m[2], (FVECTOR*)world->m[2], r );
	_sceVu0CopyVector( (FVECTOR*)out->m[3], (FVECTOR*)world->m[3] );
#endif
}


static void Act( Work *work )
{

    int			i;
    DG_COMDL_POS	*comdl_pos;
    FVECTOR		*vec;

        
    work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );

    comdl_pos = work->comdl->pos;
    vec = work->vec;
    for ( i = 0 ; i < CMDL_NUM; i++ ) {
	_sceVu0AddVector( (FVECTOR*)comdl_pos->world.m[3], (FVECTOR*)comdl_pos->world.m[3], vec );
	vec->vy -= GRAVITY;
	vec++;
	comdl_pos++;
    }

    if ( ++work->count >= LIFE ) {
	GV_DestroyActor( work );
    }    
    
}


static void Die( Work *work )
{
    if ( work->comdl ) {
	DG_DequeueComdlObjs( work->comdl );
	DG_FreeComdl( work->comdl );
    }
}


static int GetResources( Work *work, FVECTOR *pos )
{

	int			i;
	DG_DEF			*def;
	DG_COMDL		*comdl;
	DG_COMDL_POS 		*comdl_pos;

	FVECTOR			*vec;
	SVECTOR			rot;



	work->count = 0;


	
	work->n_comdl = CMDL_NUM;

	//def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("shl_frg1"), 'k' ) );
	def = (DG_DEF*)GV_GetCache( GV_CacheID( 14621904, 'k' ) );

	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, CMDL_NUM, 0 );
	if( !comdl ) {
	    return -1;
	}
	DG_QueueComdlObjs( comdl );


	work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );
	    
	
	comdl_pos = comdl->pos;
	vec = work->vec;

	for ( i = 0; i < CMDL_NUM; i++ ) {

	    /* コモデルMATRIX初期化 */
	    _ScaleMatrix( &comdl_pos->world, &DG_UnitMatrix, 4.0f+frnd()*1.0f );
	    DG_SetPos( &comdl_pos->world );
	    rot.vx = irnd()%4096;
	    rot.vy = irnd()%4096;
	    rot.vz = irnd()%4096;
	    DG_RotatePos( &rot );
	    DG_GetPos( &comdl_pos->world );
	    DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], pos );

	    DG_SetPos( &DG_UnitMatrix );
	    rot.vx = irnd()%4096;
	    rot.vy = irnd()%4096;
	    rot.vz = irnd()%4096;
	    DG_RotatePos( &rot );
	    vec->vx = frnd()*300.0f;
	    vec->vy = 0.0f;
	    vec->vz = 0.0f;
	    DG_RotVector( vec, vec, 1 );

	    /* コモデル色初期化 */
	    comdl_pos->color.vx = COLOR;
	    comdl_pos->color.vy = COLOR;
	    comdl_pos->color.vz = COLOR;
	    comdl_pos->color.vw = ALPHA;

	    comdl_pos++;
	    vec++;

	}

	return 0;

}




void *NewCrashDebris( FVECTOR *pos )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;
}


