//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	area.c
		ロード情報のセットと履歴管理

	1999/09/01 K.Uehara
	$Id: area.c,v 1.1.1.3 2002/11/19 11:41:45 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "libgv.h"
#include "libfs.h"

#include "game.h"

#include "BP_SaveLoadMGS.h"

//static char	sv_area_directory[ 16 ] = "" ;
#ifdef DEBUG
static char area_directory[ 16 ] = "";
#endif

extern int gBP_SEInternalResidentCategory;

/*----------------------------------------------------------------*/

void BP_CheckResidentAreaHook()
{
   const char * const dirname = ( char * )GM_SaveResidentDir;
   //BP - need to let SE override system know which player voice is in the current
   //resident set: snake or raiden.
   if(  !strcmp( dirname, "r_vr_s" )  //snake
      || !strcmp( dirname, "r_vr_p" )  //pliskin
      || !strcmp( dirname, "r_vr_t" )  //tuxedo snake
      || !strcmp( dirname, "r_vr_1" )  //"previous" snake
      || !strcmp( dirname, "r_vr_sp" ) //snake tales picture show?
      || !strcmp( dirname, "r_tnk0" )  //snake tanker episode
      || !strcmp( dirname, "r_sna_b" ) //snake boss survival
      )
   {
      gBP_SEInternalResidentCategory = 0;   //use snake voice.
   }
   else
   {
      gBP_SEInternalResidentCategory = 1;   //assume raiden voice for all else.
   }
}

/*----------------------------------------------------------------*/

/* 常駐入れ替えシステム */
//static	char	resident_area_directory[ 24 ] = "" ;
int				GM_ChangeResident ;

void 	GM_ChangeResidentArea( char *dirname )
{
	//strcpy( resident_area_directory, dirname ) ;
	/* 同じなら入れ替えない */
    if ( strcmp( ( char * )GM_SaveResidentDir, dirname ) ) GM_ChangeResident = 1 ;
    strcpy( ( char * )GM_SaveResidentDir, dirname ) ;

    BP_CheckResidentAreaHook();
}

/*----------------------------------------------------------------*/

void GM_InitArea( void )
{
}

void GM_SetArea( int id, char *dirname )
{
	char	*save_dir ;
#ifdef DEBUG
	strcpy( area_directory, dirname );
#endif
	GM_SaveArea = GV_StrCode( dirname ) ;
	save_dir = ( char * )( &GM_SaveAreaDir[ 0 ] ) ;
	strcpy( save_dir, dirname ) ;

   //BP - If the trophy system was disabled due to Wrong User, we can re-enable it
   //only when the user returns to the main menu.
   if( !strcmp( dirname, "n_title" ) )
   {
      MGS_SaveStatus_ClearWrongUser();
   }
}

char *GM_GetArea( void )
{
//	return area_directory;
    if ( GM_ChangeResident == 0 ) {
		return ( char * )( &GM_SaveAreaDir[ 0 ] ) ;
	}
//    return resident_area_directory ;
    return ( char * )GM_SaveResidentDir ;
}

#if 0	/* 廃止 */
void GM_SaveAreaDir( void )
{
	strcpy( sv_area_directory, area_directory ) ;
}

char *GM_GetSaveAreaDir( void )
{
	return sv_area_directory ;
}
#endif
