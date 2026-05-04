//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_spray.c
	C4説明シーン用スプレー  *NewDemoDollSpray
	2001/06/23 K.Sigeno
	$Id: demo_spray.c,v 1.1.1.3 2002/11/19 11:49:12 Yoshizawa1 Exp $
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include "gameheader.h"
#include "libutl.h"

void *NewIceSpray_mng( int name , int where  );

typedef	struct _Work {
	GV_ACT_EX	actor ;
} Work ;
static void Act(Work *work)
{
}
static void Die(Work *work)
{
}
static int GetResources( work,name,where )
int	name ;
int	where ;
{
	CONTROL	*ctrl ;
	GCL_GetOption( 'n' ) ;
	ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
	GV_SetActorChild( work ,
		NewIceSpray_mng(  &ctrl->object->objs->objs[HUMAN21_HIDARI_TE].world , 
		&DG_ZeroVector , 0 )) ;
	GV_SetActorMessageKill( work, name );

   //BP - Fix "error C4716: 'GetResources' : must return a value"
   return 1;
}
void *NewDemoDollSpray( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

