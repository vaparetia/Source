//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gclutil.c
		gclユーティリティ関数

	2000/07/14 K.Uehara
	$Id: gclutil.c,v 1.1.1.3 2002/11/19 11:51:36 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#include <math.h>
#endif

#include "gameheader.h"
#include "libutl.h"

extern int BP_COsContext_GetLanguage_MGS();

int NewGclAssert( void )
{
	if( GCL_GetNextInt() == 0 ){
		char *p;
		if( ( p = GCL_GetNextString() ) != NULL ){
			printf( "GCL:ASSERT %s\n", p );
		}
		GCL_ASSERT( FALSE );

#ifdef DEBUG_MODE
		HANGUP();
#endif
	}
	return 0;
}

int NewGclSin( void )
{
	int rad;

	rad = GCL_GetNextInt();

	return ( int )( vu0_SinS( rad ) * 4096 );
}

int NewGclCos( void )
{
	int rad;

	rad = GCL_GetNextInt();

	return ( int )( vu0_CosS( rad ) * 4096 );
}

int NewGclAtan( void )
{
	int x, y;
	x = GCL_GetNextInt();
	y = GCL_GetNextInt();
	return ( int )( atan2f( y, x ) * 2048.0F / M_PI ) & 4095;
}
int NewGclInsideChk4P( void )
{
	int x0, y0,x1, y1,x2, y2,x3, y3,x4, y4;
	x0 = GCL_GetNextInt();
	y0 = GCL_GetNextInt();
	x1 = GCL_GetNextInt();
	y1 = GCL_GetNextInt();
	x2 = GCL_GetNextInt();
	y2 = GCL_GetNextInt();
	x3 = GCL_GetNextInt();
	y3 = GCL_GetNextInt();
	x4 = GCL_GetNextInt();
	y4 = GCL_GetNextInt();
	if (((y1-y0)*(x2-x1)-(y2-y1)*(x1-x0)) > 0 ) return 0 ;
	if (((y2-y0)*(x3-x2)-(y3-y2)*(x2-x0)) > 0 ) return 0 ;
	if (((y3-y0)*(x4-x3)-(y4-y3)*(x3-x0)) > 0 ) return 0 ;
	if (((y4-y0)*(x1-x4)-(y1-y4)*(x4-x0)) > 0 ) return 0 ;
	return 1 ;
}
int NewGclDiffDirABS( void )
{
	int from , to , diff ;
	from = GCL_GetNextInt();
	to = GCL_GetNextInt();
	diff = 4095 & ( to - from ) ;
	diff = ( diff <= 2048 ) ? diff : diff - 4096 ;
	return (( diff < 0 ) ? -diff : diff );
}

int NewGclAbs( void )
{
	int value;
	value = GCL_GetNextInt();
	return (value > 0) ? value : -value;
}

int NewGclVecLen( void )
{
	float x0, y0, z0, x1, y1, z1, r;

	x0 = ( float )GCL_GetNextInt();
	y0 = ( float )GCL_GetNextInt();
	z0 = ( float )GCL_GetNextInt();
	x1 = ( float )GCL_GetNextInt();
	y1 = ( float )GCL_GetNextInt();
	z1 = ( float )GCL_GetNextInt();

	DG_MULA( x0 - x1, x0 - x1 );
	DG_MADDA( y0 - y1, y0 - y1 );
	r = DG_MADD( z0 - z1, z0 - z1 );

	return ( int )DG_SQRT( r );
}

int NewGclLangUpdate( void )
{
   //BP - GM_Language is set directly as a variable by a gcl script on startup.
   //PS2 / EU ignores the system setting, defaulting to English in the scripting
   //and allowing the player to change it in the in-game options.
   //We have no easy way to circumvent this in the EU build.
   //This function is called after initialization as well as any time it is changed
   //by the in-game options, however, so we can put a hook in here to override it
   //with the PS3 / X360 system setting.
   short bp_lang = (short)BP_COsContext_GetLanguage_MGS();
   printf( "SET LANG = %d\n", GM_Language );
   GM_Language = bp_lang;
   printf( "BP OVERRIDE LANG = %d\n", GM_Language );
	GCL_SetProcSelectNo( GM_Language );
	return 0;
}

int NewGclWhile( void )
{
	void *p;
	p = GCL_NextStr();

	while( GCL_GetInt( p ) ){
		int type;
		void *block;
		GCL_GetNextValue( GCL_NextStr(), &type, &block );
		GCL_ExecBlock( block, NULL );
	}
	return 0;
}

#if 0

int NewGclTest( void )
{
	void *res;
	int a;

	res = GCL_GetNextResource();
printf( "RES = %X\n", res );
dump( res, 128 );

	GCL_SetArgTop( res );

	a = GCL_GetNextInt();
	printf( "A = %d\n", a );
	a = GCL_GetNextInt();
	printf( "A = %d\n", a );
	printf( "%s\n", GCL_GetNextString() );
	a = GCL_GetNextInt();
	printf( "A = %d\n", a );
}

#endif
