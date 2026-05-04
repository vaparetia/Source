//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ダミーストリーミングドライバ

	2000/07/04 K.Uehara
	$Id: dumstream.c,v 1.1.1.3 2002/11/19 11:41:48 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<eekernel.h>
#include	<eeregs.h>
#include	<libgraph.h>
#endif

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"stream.h"
#include	"g_define.h"

#include	"strctrl.h"

#include "BP_EndianSupport.h"

typedef struct {
	GV_ACT_EX actor;
	int type;
	GM_STREAM_CONTROL *ctrl;
} Work;

void GM_StreamDummyAct( void *_work )
{
	/* ひたすらデータが来たら解放する */
	void *p;
	Work *work = _work;

	do {
		if( ( p = FS_StreamGetData( work->ctrl->stream_h, work->type ) ) != NULL ){
			FS_StreamFreeData( work->ctrl->stream_h, p );
		} else {
			if( work->ctrl->state >= GM_STREAM_STATE_READ_END ){
				/* 読みきってデータがないので終了 */
				GV_DestroyActor( work );
			}
			break;
		}
	} while( FS_STREAM_GET_TIME( p ) < work->ctrl->tick );
}

void *NewStreamDummyDriver( GM_STREAM_CONTROL *ctrl, int type )
{
	Work *work;

	if( ( work = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM, sizeof( Work ), 0xf2 ) ) != NULL ){
		GV_SetActor( &work->actor, GM_StreamDummyAct, NULL );
		GV_ActorEX( &work->actor );

		work->type = type;
		work->ctrl = ctrl;
	}
	return work;
}
