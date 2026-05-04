//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   emma_equip.c
   エマ専用 装備品キャラ

   2001/04/10 T. Morita
   $Id: emma_equip.c,v 1.1.1.3 2002/11/19 11:45:56 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "../../kano/attachment/attachments.h"
#include "../include/util.h"

enum frt_equop_t
{
    EMA_EQUIP_IDCARD  = 0x0001,
    EMA_EQUIP_GLASSES = 0x0002,
    EMA_EQUIP_HAIR_DRY = 0x0004,
    EMA_EQUIP_HAIR_WET = 0x0008,
    EMA_EQUIP_HAIR_WRT = 0x0010,
} ;

typedef struct work_t
{
    GV_ACT_EX actor ;
    OBJECT   *body  ;
    int       name  ;
    int       flag  ;
    DG_OBJS  *glasses ;
} Work ;

static void Die( Work *work )
{
    if ( work->glasses )
	DG_DequeueObjs( work->glasses ), DG_FreeObjs( work->glasses ) ;
}


static void Act( Work *work )
{
    if ( work->flag & EMA_EQUIP_GLASSES )
    {
	_sceVu0CopyMatrix( &work->glasses->world,
			   &work->body->objs->objs[HUMAN21_ATAMA].world ) ;

	/* マップの設定*/
	work->glasses->group_id = work->body->objs->group_id ;

	/* 非表示フラグの設定*/
	if ( (work->body->evmobj ? work->body->evmobj->flag & DG_EVMOBJ_INVISIBLE : 1 ) &&
	     (work->body->objs->flag & DG_FLAG_INVISIBLE) )
	    DG_InvisibleObjs( work->glasses ) ;
	else
	    DG_VisibleObjs( work->glasses ) ;
    }
}

static int GetResources( Work *work, int name, OBJECT *body, int flag )
{
    void *child ;
    extern void *NewEvmHairModel_called2( int name, int model_name, int sample_num,
					  OBJECT *target,
					  unsigned char *objnum,
					  int n_objnum,
					  FVECTOR *x,
					  SVECTOR *r,
					  float oval_param,
					  int collision_flag,
					  unsigned char *collision_objs,
					  int visible_flag,int light_flag,
					  int boundmodel_name ) ;

    unsigned char hair_objnum[]={ 11,12, };
    unsigned char id_objnum[]={ 2, };
    unsigned char hair_colobjs[]={ 11,12, };
    unsigned char id_colobjs[]={ 2,3,7,11,12, };

    work->name = name ;
    work->flag = flag ;
    work->body = body ;

    /* エマ 髪の毛 */
    if ( flag & EMA_EQUIP_HAIR_DRY )
    {
	if ( !(child = NewEvmHairModel_called2( GV_StrCode("エマ髪の毛"),
						GV_StrCode("ema_hair_mh"),17,
						body,
						hair_objnum,sizeof(hair_objnum)/sizeof(hair_objnum[0]),
						NULL,NULL,
						1.0f,
						sizeof(hair_colobjs)/sizeof(hair_colobjs[0]),
						hair_colobjs,
						0,0,GV_StrCode("ema_bounding"))) )
	    PERROR( "NewEvmHairModel_called2 Failed for DryHair : NewEmmaEquip\n" ) ;
	GV_SetActorChild( work, child ) ;
    }

    if ( flag & (EMA_EQUIP_HAIR_WRT|EMA_EQUIP_HAIR_WET) )
    {
	int smp ;

	smp = flag & EMA_EQUIP_HAIR_WRT ? 22 : 17 ;
	if ( !(child = NewEvmHairModel_called2( GV_StrCode("エマぬれ髪の毛"),
						GV_StrCode("ema_hair_swim_mh"),smp,
						body,
						hair_objnum,sizeof(hair_objnum)/sizeof(hair_objnum[0]),
						NULL,NULL,
						1.0f,
						sizeof(hair_colobjs)/sizeof(hair_colobjs[0]),
						hair_colobjs,
						0,0,GV_StrCode("ema_bounding"))) )
	    PERROR( "NewEvmHairModel_called2 Failed for WetHair : NewEmmaEquip\n" ) ;
	GV_SetActorChild( work, child ) ;
    }

    /* エマ IDカード */
    if ( flag & EMA_EQUIP_IDCARD )
    {
	if ( !(child = NewEvmHairModel_called2( GV_StrCode("エマＩＤ"),
						GV_StrCode("ema_idrope_mh_mt"),18,
						body,
						id_objnum,
						sizeof(id_objnum)/sizeof(id_objnum[0]),
						NULL,NULL,
						1.0f,
						sizeof(id_colobjs)/sizeof(id_colobjs[0]),
						id_colobjs,
						0,0,GV_StrCode("ema_bounding"))) )
	    PERROR( "NewEvmHairModel_called2 Failed for IDCard : NewEmmaEquip\n" ) ;
	GV_SetActorChild( work, child ) ;
    }

    /* エマ めがね */
    if ( flag & EMA_EQUIP_GLASSES )
    {
	work->glasses = DG_MakeObjs( GV_GetCache( GV_CacheID( 11219402/*ema_glasses_mt*/, 'k' )),
				   DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
	DG_QueueObjs( work->glasses ) ;
	DG_SetLightMatrix( work->glasses, body->objs->light ) ;
    }

    /* 初期化する */
    Act( work ) ;

    return 0 ;
}

void *NewEmmaEquip( int name, OBJECT *body, int flag )
{
    Work *work ;

    work = (Work *)GV_NewActorPrio( GV_ACTOR_USER, sizeof(Work), 0x10 ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResources( work, name, body, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

void *NewEmmaScnEquip( int name, int where )
{
	CONTROL *ctrl ;
	OBJECT  *body ;
	int      man, flag ;

	flag = GCL_GetOptionValue( 'f', 6 ) ; /*デフォルトは 乾いた髪の毛 眼鏡*/
	man  = GCL_GetOptionValue( 'n', 0 ) ;
	if ( man == 0 ) {
		printf( "emma_equip.c : No name option !!!!!\n" ) ;
		return NULL ;
	}

	ctrl = GM_SearchWhere( man ) ;
	if ( ctrl == NULL         ) {
		printf( "emma_equip.c : There is no such name<%d>!!!!!\n", man ) ;
		return NULL ;
	}
	body = ctrl->object ;
	if ( ctrl->object == NULL ) {
		printf( "emma_equip.c : There is no body in control<%d>!!!!\n", man ) ;
		return  NULL ;
	}
	return NewEmmaEquip( name, body, flag ) ;	
}
