//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cnctobj.c
	コネクトオブジェクト

	2000/04/03 Y.Korekado
	$Id: cnctobj.c,v 1.1.1.3 2002/11/19 11:44:02 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewConnectObject( name, where )

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
/*----------------------------------------------------------------*/
#ifdef DEBUG_MODE
//#define __DEBUGDATA	(1)
#endif
//#define	BODY_FLAG			(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)
#define	BODY_FLAG			(DG_FLAG_ONEPIECE)
#define	BODY_FLAG_SHADOW	(DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define PRIO	0x40	/* 敵兵より後に処理 */

/*----------------------------------------------------------------*/
typedef	ALIGN16_DECL(struct)	{
	GV_ACT_EX	actor ;
	OBJECT		body ;
	OBJECT		*cnct_obj ;

	int			cnct_num ;
	FVECTOR		shift ;
	SVECTOR		rot ;
} Work  ;

/*----------------------------------------------------------------*/
#ifdef __DEBUGDATA
static	void	debug_data( work )
Work		*work ;
{
	static	int	pAD=0, vAL=1 ;
	int	v ;

	v=0 ;
	if( GV_PadData[ 1 ].status & PAD_L2 ){
		if( GV_PadData[ 1 ].press & PAD_R ) if ( ++pAD>=6 ) pAD = 0 ; 
		if( GV_PadData[ 1 ].press & PAD_L ) if ( --pAD<0 ) pAD = 5 ; 
		if( GV_PadData[ 1 ].press & PAD_X ) if ( (vAL*=10) >=1000 ) vAL = 1 ; 

		if( GV_PadData[ 1 ].press & PAD_U ){
			v=1*vAL ;
		} else if( GV_PadData[ 1 ].press & PAD_D ){
			v=-1*vAL ;
		}
		
		switch( pAD ) {
			case 0 :
			work->rot.vx += v ;
			break ;
			case 1 :
			work->rot.vy += v ;
			break ;
			case 2 :
			work->rot.vz += v ;
			break ;
			case 3 :
			work->shift.vx += (float)v ;
			break ;
			case 4 :
			work->shift.vy += (float)v ;
			break ;
			case 5 :
			work->shift.vz += (float)v ;
			break ;
		}
		DEBUG_Locate( 40 + ( 22 ), 115, 0 );
		DEBUG_Printf( "rot = [%d][%d][%d]", work->rot.vx,work->rot.vy,work->rot.vz );
		DEBUG_Locate( 40 + ( 22 ), 135, 0 );
		DEBUG_Printf( "shi = [%6.1f][%6.1f][%6.1f]", work->shift.vx,work->shift.vy,work->shift.vz );


	}
}
#endif
/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
#ifdef __DEBUGDATA
debug_data(work ) ;
#endif

	DG_SetPos( &work->cnct_obj->objs->objs[work->cnct_num].world ) ;
	DG_MovePos( &work->shift ) ;
	DG_RotatePosZYX( &work->rot ) ;

	DG_PutObjs( work->body.objs ) ;
	DG_SetLightMatrix( work->body.objs, work->cnct_obj->objs->light );

}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, cnct_obj, cnct_num, shift, rot, model )
Work	*work ;
OBJECT	*cnct_obj ;
int		cnct_num ;
FVECTOR	*shift ;
SVECTOR	*rot ;
int		model ;
{

	OBJECT		*body ;

	work->cnct_obj = cnct_obj ;
	work->cnct_num = cnct_num ;
	work->shift = ( shift != NULL )? *shift : DG_ZeroVector ;
	work->rot = ( rot != NULL )? *rot : DG_ZeroSVector ;

	body = &work->body ;
	GM_InitObject( body, model, BODY_FLAG );
	if ( body->objs == NULL ) return -1 ;
//	GM_ConfigObjectRoot( body, work->cnct_obj, work->cnct_num ) ;

	return 0 ;
}

	/*じれじえ
		シナリオ呼び出し
	*/
void *NewConnectObject( OBJECT *cnct_obj, int cnct_num, FVECTOR *shift, SVECTOR *rot, int model )
{
	Work		*work ;

	OPERATOR() ;

#ifdef __DEBUGDATA
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
#else
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
#endif
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, cnct_obj, cnct_num, shift, rot, model ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
