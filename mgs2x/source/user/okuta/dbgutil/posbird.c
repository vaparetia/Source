//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    kmtest.c
    カモメさんテスト
    2001/05/01 Masafumi Okuta
    $Id: posbird.c,v 1.1.1.3 2002/11/19 11:47:49 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
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
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"
#include	"../../okajima/etc/ok_util.h"

#ifdef DEBUG_MODE
#include	"debugmenu.h"	
#endif

void* NewPositionBird( name, where)
int		name;		// 名前
int		where;		// マップ
{
    return NULL;
}





