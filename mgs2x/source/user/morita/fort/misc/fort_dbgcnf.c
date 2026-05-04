//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    fort_dbgcnf.c

    フォーチュンデバック
    2001/05/08 Masafumi Okuta
    $Id: fort_dbgcnf.c,v 1.1.1.3 2002/11/19 11:46:17 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"
#include "debugmenu.h"	

int FRT_DbgOffense = 1 ;


static char * dbg_offense_tems[]  = { "ON", "OFF" } ;
static int    dbg_offense_values[] = { 1, 0 } ;
static GM_DEBUG_MENU dbg_offense = 
{
	NULL,
	"FORTUNE",
	"OFFENCE-MODE",
	dbg_offense_tems,
	dbg_offense_values,
	(int*)&FRT_DbgOffense, /* target */
	0x00000001, /* mask */
	NULL, /*func*/
	0, /*strid*/
	0, /*types */
	2, /*max*/
	0,0
};

int FRT_DbgStatus = 0 ;
static char * dbg_status_tems[]  = { "OFF", "ON" } ;
static int    dbg_status_values[] ={ 0, 1 } ;
static GM_DEBUG_MENU dbg_status = 
{
	NULL,
    "FORTUNE",
    "STATUS",
	dbg_status_tems,
	dbg_status_values,
	&FRT_DbgStatus,
	0x00000001, /* mask */
	NULL, /*func*/
	0, /*strid*/
	0, /*types */
	2, /*max*/
	0,0
};

int FRT_DbgAgingTest = 0 ;
static char * dbg_aging_tems[]  = { "OFF", "ON" } ;
static int    dbg_aging_values[] ={ 0, 1 } ;
static GM_DEBUG_MENU dbg_aging = 
{
	NULL,
    "FORTUNE",
    "AGING-TEST",
	dbg_aging_tems,
	dbg_aging_values,
    &FRT_DbgAgingTest,
	0x00000001, /* mask */
	NULL, /*func*/
	0, /*strid*/
	0, /*types */
	2, /*max*/
	0,0
};

void FRT_InitDbgConfiguration()
{
    GM_AddDebugMenu( &dbg_offense ) ;
    GM_AddDebugMenu( &dbg_status ) ;
    GM_AddDebugMenu( &dbg_aging ) ;
}
