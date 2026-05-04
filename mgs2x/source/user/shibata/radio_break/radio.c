//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

   無線機壊れ呼出口
	2000/05/17 T.Shibata

	$Id: radio.c,v 1.1.1.3 2002/11/19 11:48:47 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

//#include	"def_dma.h"
//#include	"utl_dma.h"

#include	"../util/ts_util.h"

#define		LIFE_TIME		(180)

#define	FRASH_R (600.0f)
#define	FRASH_E (FRASH_R*2.0f)

// extern
extern void *NewTs_Spark( FVECTOR *center,FVECTOR *, float );
extern void *NewRadio_Comdl( FVECTOR *center, FVECTOR *force );
extern void *NewTs_Min_Smoke( FVECTOR *center, FVECTOR *force );
extern void *NewTs_Min_Fog( FVECTOR *center );

typedef struct
{
	GV_ACT_EX		actor;
	int				timer;
	FVECTOR			*center;
} Work ;

#if 0
static inline void _AddVec( FVECTOR *output, FVECTOR *v0, FVECTOR *v1 )
{

	asm volatile ("
	lqc2		vf9 ,0x00(%1)
	lqc2		vf10,0x00(%2)
    vadd        vf8 ,vf9, vf10
	sqc2		vf8 ,0x00(%0)
	": : "r"(output), "r"(v0), "r"(v1) );
}
#endif

static void Act(Work *work)
{
	FVECTOR fvtemp;
#if 1
	if(!(irnd()%21)){
		DG_COPY_VEC(&fvtemp,work->center);
		fvtemp.vy += 1000.0f;
		NewTs_Spark( work->center, NULL, -1.0f );

		GM_SeSetMode( SD_E_HIBANA02, work->center, GM_SEMODE_BOMB ) ;

		//NewTs_Min_Fog( &fvtemp );

#if 1
		DG_SetTmpLight(&fvtemp,FRASH_R,FRASH_E);
#else
		DG_SetTmpLight2(&fvtemp,FRASH_R,FRASH_E,0x00ff00ff,LIT_FLAG_BGONLY|LIT_FLAG_CHARAONLY)
#endif
	}
#else
	if(!(work->timer)){
		DG_COPY_VEC(&fvtemp,work->center);
		fvtemp.vy += 1000.0f;
		NewTs_Spark( &fvtemp, NULL, -1.0f );
	}
#endif
	//AN_Test_Eye2( work->center, 2 );
	if(++work->timer > LIFE_TIME){
		//printf("kill radio break\n");
		
		GV_DestroyActor( work );
	}
}

static void Die(Work *work)
{

}

static int GetResources( Work *work,FMATRIX *mat, FVECTOR *force )
{
	/* DataInit */
	work->timer = 0;
	work->center = (FVECTOR*)&mat->m[3];
#if 0
	{
		FVECTOR fvtemp;
		FVECTOR	w_for = { 0.0f, 0.0f, -1.0f, 0.0f };
		
		DG_COPY_VEC(&fvtemp,work->center);
		fvtemp.vy += 1000.0f;
		
		DG_SetTmpLight(&fvtemp,FRASH_R,FRASH_E);
		NewTs_Spark( &fvtemp, NULL, -1.0f );	
		NewRadio_Comdl( &fvtemp, &w_for );
		fpu_DivVectorScaler(&w_for,&w_for,-1.0f);
		//NewTs_Min_Smoke( &fvtemp, &w_for );
		
		NewTs_Min_Fog( &fvtemp );
	}
#else
		NewTs_Spark( work->center, NULL, -1.0f );	
		//NewRadio_Comdl( work->center, force );
		NewTs_Min_Fog( work->center );
		DG_SetTmpLight( work->center,FRASH_R,FRASH_E);
	
#endif
	
	GM_SeSetMode( SD_E_ECODBR01, work->center, GM_SEMODE_BOMB ) ;
	GM_SeSetMode( SD_E_HIBANA02, work->center, GM_SEMODE_BOMB ) ;

	return (0);
}

/*
    mat:	matrix
*/

void *NewRadio_break( FMATRIX *mat, FVECTOR *force )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		if(GetResources( work, mat, force ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
