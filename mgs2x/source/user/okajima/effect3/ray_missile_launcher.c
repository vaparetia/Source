//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ray_missile_launcher.c
	デモ：レイのミサイル呼び出し君

	2001/07/19 S.Okajima
	$Id: ray_missile_launcher.c,v 1.1.1.3 2002/11/19 11:47:35 Yoshizawa1 Exp $
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

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"



#define SIGNAL_RAYMISSILESMOKE			(0x4665dc00)	/* RAYMissileSmoke<<8 */
#define SIGNAL_RAYMISSILESMOKE_STOP		(SIGNAL_RAYMISSILESMOKE|0x01)
//bombgas6_alp.bmp
//light10_msk.bmp




typedef	struct	{
	GV_ACT_EX		actor;
	int				name;
} Work ;

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			GV_CallChildSignalFunc( work, SIGNAL_RAYMISSILESMOKE_STOP, 0 );
			break;
		  default:
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	CheckMesgParam( work );
}


/*----------------------------------------------------------------*/
static void Die( Work *work )
{
}

static int GetResources( Work *work, FMATRIX *world )
{
	void *p;

	extern void *NewRAYMissileSmoke( FMATRIX *parent );
	p = NewRAYMissileSmoke( world );
	if( p!=NULL ){
		GV_SetActorChild( work, p );
		return 0 ;
	}
	return -1 ;
}

/*
デモ用
*/
void *NewRayMissileForDemo( int name, FMATRIX *world )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name;

		if ( GetResources( work, world ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}
