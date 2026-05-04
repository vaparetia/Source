//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	depend_arms.c
	状況依存武器
	
	2001/05/23 T.Shibata
	
	$Id: depend_arms4.c,v 1.1.1.3 2002/11/19 11:48:31 Yoshizawa1 Exp $

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

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

typedef	struct	{
	GV_ACT_EX	actor;
	FMATRIX		world;
	FMATRIX		world_ctrg;
	FMATRIX		lights[2];
	EFTCONTROL	control;
	
	FVECTOR		rots[ 8 ] ;
	int			name;
	int			now_arms;
	int			arms_id;
	DG_OBJS		*objs;
	DG_OBJS		*objs_ctrg;
} Work;

enum {
	ARMS_KIND_USP = 0,
	ARMS_KIND_USP_SP,
	ARMS_KIND_SCM,
	ARMS_KIND_SCM_SP,
	
	ARMS_KIND_HAND,
};

static FVECTOR	DependArmsCtrgShift[ARMS_KIND_HAND] = {
	{  17.5f, -115.9f,  -1.2f, 1.0f },
	{  17.5f, -115.9f,  -1.2f, 1.0f },
	{  17.5f,  -86.0f,  14.0f, 1.0f },
	{  17.5f,  -86.0f,  14.0f, 1.0f },
};

enum {
	MSG_DPNDARMS_ARMS_FIRE = 0,
	MSG_DPNDARMS_ARMS_INV,
	MSG_DPNDARMS_ARMS_VIS,
};

extern void *NewArmsEffectUsp( int name , int flags , FMATRIX *world );
extern void *NewArmsEffectSocom( int name , int flags , FMATRIX *world );

#define	EFFECT_FLAGS_AMO	(0x01)
#define	EFFECT_FLAGS_LINE	(0x02)
#define	EFFECT_FLAGS_REC	(0x04)
#define	EFFECT_FLAGS_ON_TMP	(0x08)
#define EFFECT_FLAGS_ON_SPARK (0x10)
#define EFFECT_FLAGS_SAA_LEFT (0x20)
#define EFFECT_FLAGS_NO_MZL (0x40)

static void DpndArmsCallFire( int id, FMATRIX *world, int flags )
{
	switch( id ){
	  case ARMS_KIND_USP:
		NewArmsEffectUsp( 0, flags, world );
		break;
	  case ARMS_KIND_SCM:
		NewArmsEffectSocom( 0, flags, world );
		break;
	  case ARMS_KIND_SCM_SP:
		NewArmsEffectSocom( 0, flags|EFFECT_FLAGS_NO_MZL, world );
		break;
	  case ARMS_KIND_USP_SP:
		NewArmsEffectUsp( 0, flags|EFFECT_FLAGS_NO_MZL, world );
		break;
	  default:
		break;
	}
}


static inline void _RotTrans( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	lqc2				vf7,0x30(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddaz.xyzw		ACC, vf6,vf8
	vmaddw.xyzw			vf8, vf7,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) :"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVectorW(in, out, 1 ) ;
#endif
}

static void Act( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	DM_EftControlMatrix( &work->control, &work->world ) ;
	DG_COPY_MAT( &work->world_ctrg, &work->world );
	_RotTrans( (FVECTOR*)work->world_ctrg.m[3], &work->world, &DependArmsCtrgShift[work->arms_id] );
	DG_GetLightMatrix( &work->control.mov, work->lights );

	//PRINT_PFVEC(0,(FVECTOR*)work->world_ctrg.m[3]);
	//AN_Test_Eye( (FVECTOR*)work->world_ctrg.m[3], 2 );
	mes_num=GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
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
		  case MSG_DPNDARMS_ARMS_INV:
			DG_InvisibleObjs( work->objs );
			DG_InvisibleObjs( work->objs_ctrg );
			break;
		  case MSG_DPNDARMS_ARMS_VIS:
			DG_VisibleObjs( work->objs );
			DG_VisibleObjs( work->objs_ctrg );
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
	DG_DequeueObjs( work->objs_ctrg );
	DG_FreeObjs( work->objs_ctrg );
}


static DG_OBJS *InitItemObjs( int code, FMATRIX *root, FMATRIX *light, FVECTOR *rots )
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
	if ( def->n_x_models > 1 && def->n_x_models < 8 ) {
		objs->rots = rots ;
	}
	return objs;
}

static DG_OBJS *InitOneItemObjs( int code, FMATRIX *root, FMATRIX *light )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( code, 'k' ) ) ;
	if(!def){ return NULL; }
	objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
	if(!objs) return NULL;
	if(DG_QueueObjs( objs )<0) return NULL;
	if(root) objs->root = root;
	if(light) DG_SetLightMatrix( objs, light );

	return objs;
}

static int GetResources( Work *work, int ng_mdl, int flags )
{
	int		code_ctrg = 0;

	switch( PL_EquipWeaponCode ){
	  case MDL_USP_SUB:
		work->arms_id = ARMS_KIND_USP;
		work->now_arms = PL_EquipWeaponCode;
		code_ctrg = MDL_USP_AMO;
		break;
	  case MDL_SOCOM_SUB:
		work->arms_id = ARMS_KIND_SCM;
		work->now_arms = PL_EquipWeaponCode;
		code_ctrg = MDL_SOCOM_AMO;
		break;
	  case MDL_SOCOM_SP_SUB:
		work->arms_id = ARMS_KIND_SCM_SP;
		work->now_arms = PL_EquipWeaponCode;
		code_ctrg = MDL_SOCOM_AMO;
		break;
	  case MDL_USP_SP_SUB:
		work->arms_id = ARMS_KIND_USP_SP;
		work->now_arms = PL_EquipWeaponCode;
		code_ctrg = MDL_USP_AMO;
		break;
	  default:
		work->now_arms = -1;
		break;
	}
	if( work->now_arms == -1 ){
		switch( ng_mdl ){
		  case 0:
			if ( GM_PlayerStateFlag & PL_SOCOM_SPPRSR_ATTACHED ){
				work->arms_id = ARMS_KIND_SCM_SP;
				work->now_arms = MDL_SOCOM_SP_SUB;
			}else{
				work->arms_id = ARMS_KIND_SCM;
				work->now_arms = MDL_SOCOM_SUB;
			}
			code_ctrg = MDL_SOCOM_AMO;
			break;
		  case 1:
			//ＵＳＰ
			if ( GM_PlayerStateFlag & PL_USP_SPPRSR_ATTACHED ){
				work->arms_id = ARMS_KIND_USP_SP;
				work->now_arms = MDL_USP_SP_SUB;
			}else{
				work->arms_id = ARMS_KIND_USP;
				work->now_arms = MDL_USP_SUB;
			}
			code_ctrg = MDL_USP_AMO;
			break;
		}
	}
	
	work->objs = InitItemObjs( work->now_arms, &work->world, work->lights, work->rots );
	if( !work->objs ) {
		printf("ERR!! NO MDL <depend_arms4.c> code[%d]\n",work->now_arms);
		printf("平野さんを呼んで次の文を加えてください。多分直ります\n");
		printf("weapon	m92_rai/m92_rai_sub.kms\n\n");
		return -1;
	}
	if( code_ctrg ){
		work->objs_ctrg = InitOneItemObjs( code_ctrg, &work->world_ctrg, work->lights );
		if( !work->objs_ctrg ) {
			printf("ERR!! NO MDL <depend_arms4.c> code[%d]\n", code_ctrg );
			return -1;
		}
	}
    return 0;
}

void *NewDependArms_Kill( int name, int ng_mdl, int flags )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ));

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		DM_EftControlAddList( name, &work->control );
		if ( GetResources( work, ng_mdl, flags ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
	}

	return (void *)work ;

}
