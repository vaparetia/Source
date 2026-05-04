//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cnctobj2.c
	コネクトオブジェクト

	2001/02/07 Y.Korekado
	$Id: cnctobj2.c,v 1.1.1.3 2002/11/19 11:44:02 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewConnectObject2( name, where )
設置場所を親側で制御可能
----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"

#include	"korekado/conv/korekado.x"
#include	"cnctobj.h"

/*----------------------------------------------------------------*/
#ifdef DEBUG_MODE
//#define DEBUGDATA	(1)
#endif

#define	BODY_FLAG			(DG_FLAG_ONEPIECE)
#define	BODY_FLAG_SHADOW	(DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define PRIO	0x40	/* 敵兵より後に処理 */

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	CNCTOBJ		*cnct ;
} Work ;
/*----------------------------------------------------------------*/
void CNCT_CnctObjNum( CNCTOBJ *cnct, int num )
{
	cnct->cnct_num = num ;
}

void CNCT_CnctObjShift( CNCTOBJ *cnct, FVECTOR *shift )
{
	cnct->shift.vx = shift->vx ;
	cnct->shift.vy = shift->vy ;
	cnct->shift.vz = shift->vz ;
}

void CNCT_CnctObjRot( CNCTOBJ *cnct, SVECTOR *rot )
{
	cnct->rot.vx = rot->vx ;
	cnct->rot.vy = rot->vy ;
	cnct->rot.vz = rot->vz ;
}

void CNCT_CnctObjObject( CNCTOBJ *cnct, OBJECT *obj )
{
	cnct->cnct_obj = obj ;
}
/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	CNCTOBJ *cnct ;

	cnct = work->cnct ;

	DG_SetPos( &cnct->cnct_obj->objs->objs[cnct->cnct_num].world ) ;
	DG_MovePos( &cnct->shift ) ;
	DG_RotatePosZYX( &cnct->rot ) ;

	DG_PutObjs( cnct->body.objs ) ;
	DG_SetLightMatrix( cnct->body.objs, cnct->cnct_obj->objs->light );

#ifdef DEBUGDATA
	DEBUG_Locate( 40 + ( 22 ), 115, 0 );
	DEBUG_Printf( "num [%d]", cnct->cnct_num );
	DEBUG_Locate( 40 + ( 22 ), 135, 0 );
	DEBUG_Printf("add[%x]",cnct->cnct_obj );
	DEBUG_Locate( 40 + ( 22 ), 155, 0 );
	DEBUG_Printf("add[%x]",cnct->body.objs->root );
#endif
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &work->cnct->body ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, cnct, cnct_obj, cnct_num, shift, rot, model )
Work	*work ;
CNCTOBJ *cnct ;
OBJECT	*cnct_obj ;
int		cnct_num ;
FVECTOR	*shift ;
SVECTOR	*rot ;
int		model ;
{

	OBJECT		*body ;
	
	work->cnct = cnct ;

	cnct->cnct_obj = cnct_obj ;
	cnct->cnct_num = cnct_num ;
	cnct->shift = ( shift != NULL )? *shift : DG_ZeroVector ;
	cnct->rot = ( rot != NULL )? *rot : DG_ZeroSVector ;

	body = &cnct->body ;
	GM_InitObject( body, model, BODY_FLAG );
	if ( body->objs == NULL ) return -1 ;

//GM_ConfigObjectRoot( body, cnct->cnct_obj, cnct->cnct_num ) ;

	return 0 ;
}

	/*じれじえ
		シナリオ呼び出し
	*/
void *NewConnectObject2( CNCTOBJ *cnct, OBJECT *cnct_obj, int cnct_num, FVECTOR *shift, SVECTOR *rot, int model )
{
	Work		*work ;

	OPERATOR() ;

#ifdef DEBUGDATA
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
#else
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
#endif
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, cnct, cnct_obj, cnct_num, shift, rot, model ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
