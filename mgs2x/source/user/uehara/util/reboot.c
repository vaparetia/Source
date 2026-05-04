//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#include <math.h>
#include <string.h>
#endif

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "gameheader.h"
#include "libutl.h"

extern int sd_set_cli ( int );

int NewGclReboot( void )
{
#ifdef PSX2
	extern int main_argc;
	extern char **main_argv;
	char *file;
	
	if( main_argc < 2 ){
		file = main_argv[ 0 ];
	} else {
		file = main_argv[ 1 ];
	}
	if( GCL_GetOption( 'f' ) != NULL ){
		file = GCL_GetNextString();
	}

	GM_Reboot( file );
#endif

	return 0;
}

int NewGetConfig( void )
{
#ifdef PSX2
	extern int main_argc;
	extern char **main_argv;
	char *mes;

	if( main_argc < 3 ){
		return 0;
	}

#if 0 //BP_PS2
	mes = main_argv[ 2 ];

	GM_Configuration = strtol( mes, &mes, 16 );
	GM_Language = strtol( mes, &mes, 16 );
	GM_ScrAdjX = strtol( mes, &mes, 16 );
	GM_ScrAdjY = strtol( mes, &mes, 16 );
#endif

	printf( "GetConfig %08X %04X %04X %04X"
			 , GM_Configuration, GM_Language, GM_ScrAdjX, GM_ScrAdjY );

	/* 表示アジャスト値をセーブしていたものに変更 */
	DG_SetDisplayOffset(GM_ScrAdjX,GM_ScrAdjY,0);

#if 0 // BP FIX divorcing options
	/* サウンドの設定 */
	if(GM_Configuration & GM_CONFIG_CUTSCENES_LETTERBOXED){
		/* モノラル */
#ifdef PSX2
		sd_set_cli(SD_MONORAL);
#else
		GM_SdSet( SD_MONORAL );
#endif		
	}
	else{
		/* ステレオ */
#ifdef PSX2		
		sd_set_cli(SD_STEREO);
#else		
		GM_SdSet( SD_STEREO );
#endif
	}
#endif // BP FIX divorcing options

	{
		int NewGclLangUpdate(void);
		NewGclLangUpdate();
	}
	return 1;
#else
	return 0;
#endif
}
