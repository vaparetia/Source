//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	setjump.c ジャンプポイント設定コマンド
	void *NewSetJumpPos(void)
	2001/02/06 K.Sigeno
	$Id: setjump.c,v 1.1.1.3 2002/11/19 11:49:07 Yoshizawa1 Exp $
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



#include	"libdg.h"
#include	"libdg.cnf"
#include	"gameheader.h"
#include	"libutl.h"

#include	"korekado/enemy/enemy.h"
#include	"../tng/tng.h"

typedef	struct	{
	GV_ACT_EX	actor ;
	R_INTRPT	r_intrpt ;
#ifdef DEBUG_MODE
	FVECTOR viewpos[2] ;
#endif
}Work ;

static void Act(Work *work){
#ifdef DEBUG_MODE
//	viewpos[0] = up_pos;
//	viewpos[1] = down_pos;
extern	int	GM_Debug2PMode ;
	return ;

	if(GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE ){
		NewLineView( work->viewpos, (1), 255, 255, 0 ); 
	}
#endif

}
static void Die(Work *work){
return ;
	GM_FreeRouteIntrpt( &work->r_intrpt );
}
static int GetResources( Work *work )
{
	FVECTOR		up_pos ,down_pos;
	HZX_GROUP_ID hzx_id ;
	int		up_addr,down_addr ,z, dir ;

	return 0 ;

	/* 座標取得 */
    if ( GCL_GetOption( 'u' ) != NULL ) {
		up_pos.vx = (float)GCL_GetNextInt() ;
		up_pos.vy = (float)GCL_GetNextInt() ;
		up_pos.vz = (float)GCL_GetNextInt() ;
	} 
    if ( GCL_GetOption( 'd' ) != NULL ) {
		down_pos.vx = (float)GCL_GetNextInt() ;
		down_pos.vy = (float)GCL_GetNextInt() ;
		down_pos.vz = (float)GCL_GetNextInt() ;
	}

#ifdef DEBUG_MODE
	work->viewpos[0] = up_pos;
	work->viewpos[1] = down_pos;
#endif


	hzx_id = HZX_GetHzxIDbyZone( 0, &up_pos, &z ) ;
	up_addr = HZX_AddressNo( GV_GetNo( hzx_id ), z, z ) ;
	hzx_id = HZX_GetHzxIDbyZone( 0, &down_pos, &z ) ;
	down_addr = HZX_AddressNo( GV_GetNo( hzx_id ), z, z ) ;

	dir = (int) GV_VecDir2FromTo( &up_pos,&down_pos ) ;

	HZX_SetNearFlag( up_addr, down_addr, HZX_ROOT_RINTRPT ) ;
	GM_SetRouteIntrpt( &work->r_intrpt, up_addr, down_addr,
		&up_pos ,dir, (ROOT_INTRPT_NOENE|ROOT_INTRPT_JUMP), ZONE_INTRPT_OBJ ) ;
	GM_PutRoteIntrpt( &work->r_intrpt) ;
	return 0 ;
}

void *NewSetJumpPos(void)
{
	Work		*work ;
	OPERATOR() ;
//    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA|GV_CLASS_WAITING, 
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
		sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0  ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
