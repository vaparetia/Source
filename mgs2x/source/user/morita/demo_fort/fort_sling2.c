//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_sling.c
   フォーチュン専用 スリングキャラ

   2000/10/03 T. Morita
   $Id: fort_sling2.c,v 1.1.1.3 2002/11/19 11:45:57 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

#define PERROR(_s) ({ printf(_s) ; return -1 ; } )

typedef struct work_t
{
    GV_ACT_EX actor ;
} Work ;

static int GetResources( Work *work, OBJECT *body, DG_OBJS *weapon )
{
    void *NewFortJointParts( int id, int where,
			     FMATRIX *world1, FVECTOR *pos1,
			     FMATRIX *world2, FVECTOR *pos2 ) ;
    FVECTOR FortShoulder[2]  = { { 88.3f,   211.4f,  49.0f, 1.0f },
				 { 111.0f,  203.0f,  43.0f, 1.0f } } ;
    FVECTOR SlingLnrFront[2] = { {   8.5f,-1010.0f, 244.0f, 1.0f },
				 {  33.5f,-1010.0f, 244.0f, 1.0f } } ;
    FVECTOR FortWaist[2]     = { {-137.7f,  -39.8f,  29.8f, 1.0f },
				 {-119.9f,  -57.1f,  28.4f, 1.0f } } ;
    FVECTOR SlingLnrRear[2]  = { {  55.0f,   48.5f, 132.0f, 1.0f },
				 {  55.0f,   73.5f, 132.0f, 1.0f } } ;
    DG_OBJ *obj ;

    obj = body->objs->objs ;
    GV_SetActorChild( work,
		      NewFortJointParts( 12441792 /*GV_StrCode( "for_lnr_sling" )*/,
					 GM_CurrentMap,
					 &weapon->world, SlingLnrFront,
					 &obj[HUMAN21_MUNE].world, FortShoulder ) ) ;
    GV_SetActorChild( work,
		      NewFortJointParts( 12441792 /*GV_StrCode( "for_lnr_sling" )*/,
					 GM_CurrentMap,
					 &weapon->world, SlingLnrRear,
					 &obj[HUMAN21_MUNE].world, FortWaist ) ) ;
    return 0 ;
}

void *NewFortSling2( OBJECT *body, DG_OBJS *weapon )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, NULL, NULL ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResources( work, body, weapon ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
