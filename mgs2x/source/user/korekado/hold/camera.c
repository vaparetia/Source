//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	camera.c
	カメラ設置

	2001/01/11 Y.Korekado
	$Id: camera.c,v 1.1.1.3 2002/11/19 11:44:15 Yoshizawa1 Exp $

	2001/10/22 PAL
*/

/*----------------------------------------------------------------
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

#include	"korekado/conv/korekado.x"
#include	"korekado/conv/define.h"
#include	"korekado/enemy/enemy.h"
#include	"holdene.h"

/*----------------------------------------------------------------*/
#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)
#define HEAD_MODEL_NAME	(13028291)	/* field_camera_head_Y180 */
#define BASE_MODEL_NAME	(12628037)	/* field_camera_base_Y180 */


#define C4_CAMERA_NAME1	(4869005)	/* 解体Ｃ４付カメラ右 */
#define C4_CAMERA_NAME2	(4869311)	/* 解体Ｃ４付カメラ左 */
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT		actor ;
	OBJECT		body[ 2 ] ;

	CONTROL		*trg_ctrl ;	/* ターゲットのコントロール */

	FVECTOR		pos ;
	SVECTOR		rot ;
	SVECTOR		head_rot ;
	int			name ;
	int			lit ;
	int			*flag ;
	short		think ;
	short		num ;
	int			lamp ;
	u_int		time ;
	int			*status2 ;
	FMATRIX		m ;
	int			where ;
} Work ;

enum {
	HEAD_OBJ,
	BASE_OBJ
} ;

enum {
	TARGET_TRACE,
	PLAYER_DETECT
} ;
static OBJECT	*CameraObj[2] ;

#define		CM_LAMP_FLAGS_ACTV		(0x80000001)
#define		CM_LAMP_FLAGS_SLEEP		(0x80000002)
/*----------------------------------------------------------------*/

static void HeadMarkCall( work )
Work *work ;
{
	static FVECTOR Front = { 0.0, 333.0, 0.0 } ;

	DG_SetPos( &(work->body[ HEAD_OBJ ].objs->world) ) ;
	DG_MovePos( &Front ) ;
	DG_GetPos( &work->m ) ;

	HeadMarkRun(&work->m, 0 ) ;
}

OBJECT *HLD_GetCameraObject( int num )
{
	return CameraObj[num] ;
}


static void HeadRot( Work *work, FVECTOR *pos )
{
	FVECTOR vec ;

	_sceVu0SubVector(  &vec, pos, &work->pos ) ;
	_FVecToRotXY( &vec, &work->head_rot ) ;
	work->head_rot.vx -= 1024 ;
}


static void SetHeadObject( work )
Work *work ;
{
	if ( *work->status2 & (HLD_STATUS2_SLEEPING|HLD_STATUS2_ACTIVE) ) return ;
	HeadRot( work,&work->trg_ctrl->mov ) ;
	DG_SetPos2( &work->pos, &work->head_rot ) ;
	DG_PutObjs( work->body[ HEAD_OBJ ].objs ) ;
}

static void EyeInfo( work )
Work *work ;
{
	if ( GM_PlayerPosition.vy < -15000 ) return ;
	if ( GM_PlayerPosition.vy > -12000 ) return ;
	if ( GM_PlayerPosition.vz > -12000 ) return ;
	if ( GM_PlayerPosition.vz < -15000 ) return ;

	if ( work->num == 0 ) {
		if ( GM_PlayerPosition.vx > 0 ) return ; 
		if ( GM_PlayerPosition.vx < -7000 ) return ; 
		SET_FLAG( *(work->flag), HLD_STATUS_CAMERA2_DETECT ) ;
	} else {
		if ( GM_PlayerPosition.vx > 7000 ) return ; 
		if ( GM_PlayerPosition.vx < 0 ) return ; 
		SET_FLAG( *(work->flag), HLD_STATUS_CAMERA1_DETECT ) ;
	}
	HeadMarkCall( work ) ;
	work->think = PLAYER_DETECT ;

	SET_FLAG( GM_GameStatus, GM_STATUS_DETECT ) ;
}

/*----------------------------------------------------------------*/
static void PlayerDetect( work )
Work *work ;
{
	HeadRot( work, &GM_PlayerPosition ) ;
	DG_SetPos2( &work->pos, &work->head_rot ) ;
	DG_PutObjs( work->body[ HEAD_OBJ ].objs ) ;
}
/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	switch( work->think ) {
		case TARGET_TRACE :
			SetHeadObject( work ) ;
			EyeInfo( work ) ;
		break ;
		case PLAYER_DETECT :
			PlayerDetect( work ) ;
		break ;
	}

	if ( work->num == 0 ) {
		if ( (work->time%4096) == 2048  )	work->lamp = CM_LAMP_FLAGS_SLEEP ;
		else if( (work->time%4096) == 0  )	work->lamp = CM_LAMP_FLAGS_ACTV ;
	} else {
		if ( (work->time%4096) == 2048 )		work->lamp = CM_LAMP_FLAGS_ACTV ;
		else if( (work->time%4096) == 0  )	work->lamp = CM_LAMP_FLAGS_SLEEP ;
	}
	
	work->time ++ ;
}

static	void	Die( work )
Work		*work ;
{
	extern	void	DG_FreePreshade( DG_OBJS * ) ;

    DG_FreePreshade( work->body[ HEAD_OBJ ].objs ) ;
    DG_FreePreshade( work->body[ BASE_OBJ ].objs ) ;
    GM_FreeObject( &work->body[ BASE_OBJ ] ) ;
    GM_FreeObject( &work->body[ HEAD_OBJ ] ) ;
}
/*----------------------------------------------------------------*/
static	int	GetResources( Work	*work, int where, FVECTOR *pos, int dir, int trg_name, int *flag, int num, int *status2 )
{
	extern void *NewCmeraLamp( FMATRIX *world, int *flags ) ;

	work->where = where ;
	work->name = 0 ;
	work->num = num ;
	work->flag = flag ;
	work->status2 = status2 ;

	GM_CurrentMap = where ;	

	GM_InitObject( &work->body[ HEAD_OBJ ], HEAD_MODEL_NAME, BODY_FLAG );
	GM_InitObject( &work->body[ BASE_OBJ ], BASE_MODEL_NAME, BODY_FLAG );
    GM_GroupObjs( work->body[ HEAD_OBJ ].objs, GM_CurrentMap ) ;
    GM_GroupObjs( work->body[ BASE_OBJ ].objs, GM_CurrentMap ) ;

	NewCmeraLamp( &(work->body[ HEAD_OBJ ].objs->objs[0].world), &work->lamp ) ;
	work->time = 0 ;

//	SET_FLAG( work->body[ HEAD_OBJ ].objs->flag, DG_FLAG_INVISIBLE ) ;

#if 0
	{
		static a=0;
		
		if ( a ){
			work->pos.vx = -5250.0 ;
			work->pos.vy = -14000.0 ;
			work->pos.vz = -2000.0 ;
		} else {
			work->pos.vx = 5250.0 ;
			work->pos.vy = -14000.0 ;
			work->pos.vz = -2000.0 ;
			a++ ;
		}
	}
	work->rot = DG_ZeroSVector ;

#else
	work->pos = *pos ;
	work->rot.vx = work->rot.vz = 0 ;
	work->rot.vy =dir ;
#endif

	work->pos.vy += 145.0f ;/* 秘密の値 */

//	NewMakeObjectBoundHazard( GM_GetHzxGroupID( where ), work->body[ 0 ].objs, 0, 0 ) ;

	{
		LIT_DEF	*def ;
		int	grp_id ;
			
//		def = (LIT_DEF*)GV_GetCache( GV_StrCode("w04c") ) ;
		
		grp_id = GM_GetDGGroupID( where ) ;
		def = DG_GetFixedLight( grp_id ) ;
		DG_MakePreshade( work->body[BASE_OBJ].objs, def );
		DG_MakePreshade( work->body[HEAD_OBJ].objs, def );
	}


//	work->trg_ctrl = GM_SearchWhere( 3637087 ) ;
printf("trg_name = %d\n",trg_name ) ;
	work->trg_ctrl = GM_SearchWhere( trg_name ) ;


	work->head_rot = work->rot ;
	HeadRot( work,&work->trg_ctrl->mov ) ;
	DG_SetPos2( &work->pos, &work->rot ) ;
	DG_PutObjs( work->body[ BASE_OBJ ].objs ) ;

	DG_SetPos2( &work->pos, &work->head_rot ) ;
	DG_PutObjs( work->body[ HEAD_OBJ ].objs ) ;

#if 0
	if ( (work->lit = GCL_GetOptionValue( 'l', 0 )) != 0 ) {
		ENE_PreShade( work->body[ HEAD_OBJ ].objs, work->lit ) ;
		ENE_PreShade( work->body[ BASE_OBJ ].objs, work->lit ) ;
	}
#endif

	work->think = TARGET_TRACE ;

	CameraObj[num] = &(work->body[ HEAD_OBJ ]) ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		extern int	C4MAN_Regist( int name, int *map, DG_OBJS *objs, FMATRIX *world, FVECTOR *shift, SVECTOR *rot ) ;
		int	c4name ;
		
		c4name = (num)?C4_CAMERA_NAME2:C4_CAMERA_NAME1 ;
		C4MAN_Regist( c4name, &work->where, work->body[ HEAD_OBJ ].objs, &work->body[ HEAD_OBJ ].objs->world, &DG_ZeroVector, &DG_ZeroSVector ) ;
	}

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewHoldCamera( int where, FVECTOR *pos, int dir, int name, int *flag, int num,
				int *status2 )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, where, pos, dir, name, flag, num, status2 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
