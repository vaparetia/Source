//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_status_cmd.c
	GM_VRStatusの操作系コマンドをこのオブジェクトに集中
	2002/07/16 K.Sigeno
	$Id: vr_status_cmd.c,v 1.1.1.3 2002/11/19 11:49:57 Yoshizawa1 Exp $
*/

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

void SetSnakeTalesStatus(void){
	GM_VRStatus |= GM_VR_SNAKETALES ;
}
