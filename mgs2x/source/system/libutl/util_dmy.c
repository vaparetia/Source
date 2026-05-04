//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	util_dmy.c
	ライブラリダミーファイル

	1999/08/16 K.Takabe
	$Id: util_dmy.c,v 1.1.1.3 2002/11/19 11:42:59 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libutl.h"

