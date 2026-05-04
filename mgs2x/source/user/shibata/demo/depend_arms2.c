//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	depend_arms2.c
	状況依存武器
	
	2001/05/23 T.Shibata
	
	$Id: depend_arms2.c,v 1.1.1.3 2002/11/19 11:48:31 Yoshizawa1 Exp $

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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"
#include	"../../mode/demo/eft_con.h"



typedef	struct	{
	GV_ACT_EX	actor;
	FMATRIX		world;
	FMATRIX		lights[2];
	EFTCONTROL	control;
	DG_OBJS		*objs;
	int			name;
	int			now_arms;
	int			arms_id;
} Work;

enum {
	ARMS_KIND_AKS,
	ARMS_KIND_AKS_SP,
};

enum {
	MSG_DPNDARMS_ARMS_FIRE = 0,
	MSG_DPNDARMS_ARMS_INV,
	MSG_DPNDARMS_ARMS_VIS,
};


#define	EFFECT_FLAGS_AMO	(0x01)
#define	EFFECT_FLAGS_LINE	(0x02)
#define	EFFECT_FLAGS_REC	(0x04)
#define	EFFECT_FLAGS_ON_TMP	(0x08)
#define EFFECT_FLAGS_ON_SPARK (0x10)
#define EFFECT_FLAGS_SAA_LEFT (0x20)
#define EFFECT_FLAGS_NO_MZL (0x40)
#if 0
static void DpndArmsCallFire( int id, FMATRIX *world, int flags )
{
	switch( id ){
	  case ARMS_KIND_AKS:
		NewArmsEffectSocom( 0, flags, world );
		break;
	  case ARMS_KIND_AKS_SP:
		NewArmsEffectSocom( 0, flags|EFFECT_FLAGS_NO_MZL, world );
		break;
	  default:
		break;
	}
}
#endif
static void Act( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	DM_EftControlMatrix( &work->control, &work->world ) ;
	DG_GetLightMatrix( &work->control.mov, work->lights );

	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
#if 0
		  case MSG_DPNDARMS_ARMS_FIRE:
			{
				int flags = 0;
				if(msg->message[1]) flags |= EFFECT_FLAGS_AMO;
				if(msg->message[2]) flags |= EFFECT_FLAGS_LINE;
				if(msg->message[3]) flags |= EFFECT_FLAGS_REC;
				if(msg->message[4]) flags |= EFFECT_FLAGS_ON_TMP;
				if(msg->message[5]) flags |= EFFECT_FLAGS_ON_SPARK;
				//if(msg->message[6]) flags |= EFFECT_FLAGS_SAA_LEFT;
				DpndArmsCallFire( work->arms_id, &work->world, flags );
			}
			break;
#endif
		  case MSG_DPNDARMS_ARMS_INV:
			DG_InvisibleObjs( work->objs );
			break;
		  case MSG_DPNDARMS_ARMS_VIS:
			DG_VisibleObjs( work->objs );
			break;
		  default:
			printf("depend_arms.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}
	//GV_WaitMessage( work, work->name );
}

static void Die( Work *work )
{
    DM_EftControlDelList( &work->control );
	DG_DequeueObjs( work->objs );
	DG_FreeObjs( work->objs );	
}


static DG_OBJS *InitItemObjs( int code, FMATRIX *root, FMATRIX *light )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( code, 'k' ) ) ;
	if(!def){ return NULL; }
	objs = DG_MakeObjs( def, DG_FLAG_SHADE, 0 );
	if(!objs) return NULL;
	if(DG_QueueObjs( objs )<0) return NULL;
	if(root) objs->root = root;
	if(light) DG_SetLightMatrix( objs, light );
	return objs;
}

static int GetResources( Work *work, int flags )
{
	if ( GM_PlayerStateFlag & PL_AK_SPPRSR_ATTACHED ){
		work->arms_id = ARMS_KIND_AKS_SP;
		work->now_arms = MDL_AKS_RAI_SP_SUB;
	}else{
		work->arms_id = ARMS_KIND_AKS;
		work->now_arms = MDL_AKS_RAI_SUB;
	}
	
	work->objs = InitItemObjs( work->now_arms, &work->world, work->lights );
	if( !work->objs ) {
		printf("ERR!! NO MDL <depend_arms2.c> code[%d]\n",work->now_arms);
//		printf("平野さんを呼んで次の文を加えてください。多分直ります\n");
//		printf("weapon	m92_rai/m92_rai_sub.kms\n\n");
		return -1;
	}
	
    return 0;
}

void *NewDependArms_Aks( int name, int flags )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ));

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		DM_EftControlAddList( name, &work->control );
		if ( GetResources( work, flags ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
	}

	return (void *)work ;
}
