//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_cmd.c
	コマンドパケット処理

	2000/07/26	K.Takabe
	$Id: demo_cmd.c,v 1.1.1.3 2002/11/19 11:45:08 Yoshizawa1 Exp $

*/
/*

	--------------------------------
	＜パケット別処理ルーチン用関数＞

	void DM_Packet_Command( DM_WORK *work, DEMO_COMMAND *packet )
	コマンドパケット処理

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

#include <mgs_type.h>
#include <libgv.h>
#include <libdg.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>
#include <camera.h>

#include "libdemo.h"


/* ---------------------------------------------------------------- */
/* コマンドパケット処理 */
void DM_Packet_Command( DM_WORK *work, DEMO_COMMAND *packet )
{
   if ( !DEMO_PACKET_IS_ENDIAN_SWAP( packet->header ) )
   {
      DEMO_PACKET_SET_ENDIAN_SWAP( packet->header );

      BP_LE_SwapSInt_Inp( &packet->command );
      BP_LE_SwapSInt_Inp( &packet->id );
      BP_LE_SwapSInt_Inp( &packet->param1 );
      BP_LE_SwapSInt_Inp( &packet->param2 );
   }
   
   switch ( packet->command ){
	  case DEMO_COMMAND_OBJECT_VISIBLE:
//		printf("command: object visible (%d,%d,%d)\n", packet->id, packet->param1, packet->param2 );
		DM_SetObjectInvisible( packet->id, packet->param1 );
		break ;
	  case DEMO_COMMAND_OBJECT_DESTROY:
//		printf("command: object destroy (%d,%d,%d)\n", packet->id, packet->param1, packet->param2 );
		DM_DestroyObject( packet->id );
		break ;
	  case DEMO_COMMAND_EFFECT_DESTROY:
//		printf("command: effect destroy (%d,%d,%d)\n", packet->id, packet->param1, packet->param2 );
		break ;
	  case DEMO_COMMAND_CAMERA_CREATE:
#ifdef PSX2
#ifdef DEBUG_MODE
		/* デバッグ用コード */
		/* スロー再生・逆転再生時に連続呼びしないように対処ｓ */
		if ( (dm_debug.mode1 >= 1) && (dm_debug.n_camera != 0) ) break;

		dm_debug.n_camera = 1;
#endif
#endif

		printf("command: camera create (%d,%d,%d)\n", packet->id, packet->param1, packet->param2 );
		{
			void	*child_work ;
			child_work = NewDemoCamera( DEMO_PACKET_ID( packet->header ), 7502707/* デモカメラ */, packet->param1 );
			if ( child_work != NULL ) GV_SetActorChild( work->actor, child_work );
		}
		break ;
	  default:
		break ;
	}
}

