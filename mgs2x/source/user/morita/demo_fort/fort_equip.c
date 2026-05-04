//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_equip.c
   フォーチュン専用 装備品キャラ

   2000/10/03 T. Morita
   $Id: fort_equip.c,v 1.1.1.3 2002/11/19 11:45:57 Yoshizawa1 Exp $
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
    FRT_EQUIP_MAGPACK= 0x0001,
    FRT_EQUIP_COAT   = 0x0002,
} ;

typedef struct work_t
{
    GV_ACT_EX actor ;
    int       name  ;
    OBJECT   *body  ;
    DG_OBJS  *gun   ;

    ATTACHMENT_ARGUMENT2 equipB[1] ;
} Work ;

static void Die( Work *work )
{
    if ( work->gun )
	DG_DequeueObjs( work->gun ), DG_FreeObjs( work->gun ) ;
}

static void Act( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    if ( work->gun )
    {
	for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	    switch( msg->message[0] )
	    {
	    case 0:
		DG_VisibleObjs( work->gun ) ;
		break ;
	    case 1:
		DG_InvisibleObjs( work->gun ) ;
		break ;
	    }	
	_sceVu0CopyMatrix( &work->gun->world, &work->body->objs->objs[HUMAN21_MIGI_TE].world ) ;
    }
}

static int GetResources( Work *work, int name, OBJECT *body, int flag, int wpname )
{
    int     numB = 0 ;
    DG_DEF *def ;
    DG_OBJ *obj ;
    void   *child ;

    work->name = name ;
    work->body = body ;

    if ( wpname != 1 && wpname != 0 )
    {
	if ( !(def = GV_GetCache( GV_CacheID( wpname, 'k' ) )) )
	    PERROR( "Cannot find Weapon KMS-Model( Not in data.cnf ) : NewFortEquip\n" ) ;
	if ( !(work->gun = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Cannot create Weapon DG_OBJS( Maybe no memory ) : NewFortEquip\n" ) ;
	DG_QueueObjs( work->gun ) ;

	/* スリング */
	{
	    void *NewFortJointParts( int id, int where,
				     FMATRIX *world1, FVECTOR *pos1,
				     FMATRIX *world2, FVECTOR *pos2 ) ;
	    FVECTOR FortShoulder[2]  = { { 109.3f,  208.4f,  49.2f, 1.0f },
					 { 134.0f,  205.0f,  43.0f, 1.0f } } ;
	    FVECTOR SlingLnrFront[2] = { {   8.5f,-1010.0f, 244.0f, 1.0f },
					 {  33.5f,-1010.0f, 244.0f, 1.0f } } ;
	    FVECTOR FortWaist[2]     = { {-137.7f,  -39.8f,  29.8f, 1.0f },
					 {-119.9f,  -57.1f,  28.4f, 1.0f } } ;
	    FVECTOR SlingLnrRear[2]  = { {  55.0f,   48.5f, 132.0f, 1.0f },
					 {  55.0f,   73.5f, 132.0f, 1.0f } } ;

	    obj = body->objs->objs ;
	    GV_SetActorChild( work,
			      NewFortJointParts( GV_StrCode( "for_lnr_sling" ),
						 GM_CurrentMap,
						 &work->gun->world, SlingLnrFront,
						 &obj[HUMAN21_MUNE].world, FortShoulder ) ) ;
	    GV_SetActorChild( work,
			      NewFortJointParts( GV_StrCode( "for_lnr_sling" ),
						 GM_CurrentMap,
						 &work->gun->world, SlingLnrRear,
						 &obj[HUMAN21_MUNE].world, FortWaist ) ) ;
	}
    }

    /* 装備品Ｂ フォーチュン マガジンパック */
    if ( flag & FRT_EQUIP_MAGPACK )
    {
	static FVECTOR pos  = { 110.0f,    7.0f, 55.0f, 1.0f } ;
	static FVECTOR aim  = {  33.0f, -121.0f, 44.0f, 1.0f } ;
	static FVECTOR leng = {   0.0f, -222.3f,  0.0f, 1.0f } ;

	work->equipB[numB].model_name = 5331692 /*GV_StrCode( "for_mag" )*/ ;
	work->equipB[numB].v          = &leng ;
	work->equipB[numB].r          = &DG_ZeroSVector ;
	work->equipB[numB].objnum     = HUMAN21_KOSHI ;
	work->equipB[numB].x          = &pos ;
	work->equipB[numB].objnum2    = HUMAN21_HIDARI_ASHI1 ;
	work->equipB[numB].p          = &aim ;
	numB++ ;
    }
    if ( !(child = NewAttachments_called( body, NULL, 0, work->equipB, numB, NULL, 0 ) ) )
	PERROR( "NewAttachment_called Failed for MAGPACK : NewFortEquip\n" ) ;
    GV_SetActorChild( work, child ) ;

    /* コート */
    if ( flag & FRT_EQUIP_COAT )
    {
	void *NewEvmHairModel_Demo2( int name,int model_name,int sample_num,
				     OBJECT *target,FVECTOR *x,SVECTOR *r,
				     float oval_param, int visible_flag,
				     int light_flag, int boundmodel_name ) ;
#define BOUNDMODEL_NAME 3846528
#define CHARA_NAME      GV_StrCode( "コートシミュレーション" )

	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( CHARA_NAME, GV_StrCode( "for_parts_coat_mh" ), 0,
						 body, &DG_ZeroVector, &DG_ZeroSVector,
						 1.3f, 0, 0,
						 BOUNDMODEL_NAME ) ) ;

	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( CHARA_NAME, GV_StrCode( "for_parts_rsode_mh" ), 1,
						 body, &DG_ZeroVector, &DG_ZeroSVector,
						 1.4f, 0, 0,
						 BOUNDMODEL_NAME ) ) ;

	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( CHARA_NAME, GV_StrCode( "for_parts_lsode_mh" ), 2,
						 body, &DG_ZeroVector, &DG_ZeroSVector,
						 1.4f, 0, 0,
						 BOUNDMODEL_NAME ) ) ;

	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( CHARA_NAME, GV_StrCode( "for_hair_coat_mh" ), 3,
						 body, &DG_ZeroVector, &DG_ZeroSVector,
						 1.0f, 0, 0,
						 BOUNDMODEL_NAME ) ) ;
    }

    return 0 ;
}

void *NewFortEquip( int name, OBJECT *body, int flag, int wpname )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResources( work, name, body, flag, wpname ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
