//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_frm.c
	フレームパケット処理

	2000/07/26	K.Takabe
	$Id: demo_frm.c,v 1.1.1.3 2002/11/19 11:45:08 Yoshizawa1 Exp $

*/
/*
	DEMO_FRAME*	DM_GetFrameData( int no )
	int		no ;		検索するカメラ番号

	現在再生中のデモのカメラを取得する

	--------------------------------
	＜パケット別処理ルーチン用関数＞

	void DM_Packet_Frame( DM_WORK *work, DEMO_OBJECT *packet )
	フレームパケット処理
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
/* フレームパケット処理 */
void DM_Packet_Frame( DM_WORK *work, DEMO_FRAME *packet )
{
   DM_FRAME	*frame ;

   if ( !DEMO_PACKET_IS_ENDIAN_SWAP( packet->header ) )
   {
      DEMO_PACKET_SET_ENDIAN_SWAP( packet->header );

      BP_LE_SwapSInt_Inp( &packet->no );
      BP_LE_SwapFloat_Inp( &packet->screen );
      BP_LE_SwapFloat_Inp( &packet->z_rotate );
      BP_LE_SwapSInt_Inp( &packet->chanl );
      BP_LE_SwapFloatArray_Inp( &packet->camera_pos.vx, 4 );
      BP_LE_SwapFloatArray_Inp( &packet->camera_trg.vx, 4 );
   }

	/* フレーム管理構造体取得 */
	frame = &work->frm_list[ work->n_frame++ ] ;
	ASSERT( work->n_frame < DM_MAX_FRAME );

	/* 検索に必要なデータと実体へのポインタをコピー */
	frame->id = DEMO_PACKET_ID( packet->header );
	frame->no = packet->no ;
	frame->chanl = packet->chanl ;
	frame->frame_ptr = packet ;
}

/* ---------------------------------------------------------------- */
	/*
		現在再生中のデモのカメラを取得する
	*/
DEMO_FRAME*	DM_GetFrameData( int no )
{
	DM_WORK		*work ;
	DM_FRAME	*frame ;
	int			i ;

	/* デモ管理構造体へのポインタを取得 */
	if ( ( work = DM_GetDemoWork() ) == NULL ) return ( NULL );

	/* カメラ番号で検索 */
	frame = work->frm_list ;
	for ( i = work->n_frame ; i > 0 ; frame++, i-- ){
		if ( frame->no == no ) return ( frame->frame_ptr );
	}

	return ( NULL );
}
