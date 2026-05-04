//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_pkt.c
	デモ用パケットストリーム解析処理

	2000/07/26	K.Takabe
	$Id: demo_pkt.c,v 1.1.1.3 2002/11/19 11:45:09 Yoshizawa1 Exp $

*/
/*
	void DM_ExecDemoStream( DM_WORK *work, void *packet_stream_ptr, int exec_flag )
	DM_WORK		*work ;					デモ管理構造体
	void		*packet_stream_ptr ;	処理するデモのパケットストリーム
	int			exec_flag ;				フレーム実行フラグ（0:NOSKIPのみ実行 1:全て実行）

	デモデータのパケットストリームを処理する

	--------------------------------
	＜パケット別処理ルーチン用関数＞

	void DM_Packet_TerminateSequence( DM_WORK *work, DEMO_PACKET *packet )
	シーン終端パケット処理

	void DM_Packet_TerminateFrame( DM_WORK *work, DEMO_PACKET *packet )
	フレーム終端パケット処理

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	/*
		各種マクロ定義
	*/

/* ---------------------------------------------------------------- */
extern DM_PACKET_FUNC	PacketExecFunctions[] ;

/* ---------------------------------------------------------------- */
	/*
		デモデータのパケットストリームを処理する
	*/
void DM_ExecDemoStream( DM_WORK *work, void *packet_stream_ptr, int exec_flag )
{
	DEMO_PACKET		*packet ;
	int				packet_type ;

	/* フレーム処理の初期化 */
	work->n_frame = 0 ;		/* フレームリスト初期化 */
	//work->n_motion = 0 ;	/* モーションリスト初期化(K.Takabe Changed 2000/19/21) */

#ifdef PSX2
#ifdef DEBUG_MODE
	/* パケット情報デバッグ用のリストの初期化 */
	memset(dm_debug.lpsPacketList, 0x00, sizeof(short) * 8 * 2);
#endif
#endif

	/* パケットストリームの解析 */
	work->frame_end = 0 ;
	while ( work->frame_end == 0 ){
		packet = packet_stream_ptr ;
		packet_stream_ptr = (void*)((int)packet_stream_ptr + DEMO_PACKET_SIZE( *packet ) ) ;

		/* パケットスキップのチェック */
		if ( exec_flag == 0 && !( DEMO_PACKET_PACKET_TYPE(*packet )& DEMO_PACKET_NOSKIP ) ) continue ;

		/* 各パケット毎の処理 */
		packet_type = DEMO_PACKET_PACKET_TYPE_MASKED( *packet );

		if ( packet_type < DEMO_PACKET_END ){
#ifdef PSX2
#ifdef DEBUG_MODE
			(dm_debug.lpsPacketList[packet_type][0])++;
			dm_debug.lpsPacketList[packet_type][1] += packet->size;
			dm_debug.lpsPacketList[DEMO_PACKET_END][0] += packet->size;
#endif
#endif

			( *PacketExecFunctions[ packet_type ] )( work, packet );
		}
	}
}

/* ---------------------------------------------------------------- */
/* シーン終端パケット処理 */
void DM_Packet_TerminateSequence( DM_WORK *work, DEMO_PACKET *packet )
{
	/* デモシーケンスの終端処理（使わないかも） */
	work->sequence_end = 1 ;
}

/* ---------------------------------------------------------------- */
/* フレーム終端パケット処理 */
void DM_Packet_TerminateFrame( DM_WORK *work, DEMO_PACKET *packet )
{
	/* １フレーム分のパケットストリーム終端 */
	work->frame_end = 1 ;
}



