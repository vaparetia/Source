/*
	debug.c
	デバッグ関連ルーチン

	2001/07/24 K.Takabe
	$Id: debug.c,v 1.5 2002/11/23 11:36:52 Yoshizawa1 Exp $

*/
/*

	----------------------------------------------------------------

	----------------------------------------------------------------
	----------------------------------------------------------------

*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "libgv.h"
#include "libdg.cnf"
#include "libdg.h"
#include "def_dma.h"

#include	"debugmenu.h"




/* ---------------------------------------------------------------- */
#ifdef DEBUG_MODE
int DG_TextureOverlapCheck ;
static GM_DEBUG_MENU debug_texturecheck = {
  class:	"LIBDG",
  menu:		"TEX-OVERLAP CHK",
  max:		2,
  items:	( char *[] ){ "ON", "OFF" },
  values:	( int [] ){ 1, 0 },
  target:	&DG_TextureOverlapCheck,
  type:		GM_DEBUG_MENU_FLAG,
};
static void DebugAct( GV_ACT_EX *debug_work )
{
}
void *NewDGDebugActor( void )
{
	GV_ACT_EX *work ;

	OPERATOR();
	work = GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, sizeof(GV_ACT_EX), 0 );
	if ( work != NULL ){
		GV_SetActor( work, DebugAct, NULL );
		GV_ActorEX( work );
		GM_AddDebugMenu( &debug_texturecheck );
	}
	return ( work );
}
#endif





/* ---------------------------------------------------------------- */

