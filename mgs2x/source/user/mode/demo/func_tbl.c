//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	func_tbl.c
	デモ用各種関数テーブルファイル

	2000/07/26	K.Takabe
	$Id: func_tbl.c,v 1.1.1.3 2002/11/19 11:45:10 Yoshizawa1 Exp $

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
	/*
		パケット処理関数テーブル
	*/
DM_PACKET_FUNC	PacketExecFunctions[] = {
	(DM_PACKET_FUNC)DM_Packet_TerminateSequence,	/* DEMO_PACKET_TERMINATE_SEQUENCE */
	(DM_PACKET_FUNC)DM_Packet_TerminateFrame,		/* DEMO_PACKET_TERMINATE_FRAME */
	(DM_PACKET_FUNC)DM_Packet_Command,				/* DEMO_PACKET_COMMAND */
	(DM_PACKET_FUNC)DM_Packet_Frame,				/* DEMO_PACKET_FRAME */
	(DM_PACKET_FUNC)DM_Packet_Object,				/* DEMO_PACKET_OBJECT */
	(DM_PACKET_FUNC)DM_Packet_Motion,				/* DEMO_PAKCET_MOTION */
	(DM_PACKET_FUNC)DM_Packet_Effect,				/* DEMO_PAKCET_EFFECT */
	(DM_PACKET_FUNC)DM_Packet_MotionPack,			/* DEMO_PAKCET_MOTION_PACK */
};

