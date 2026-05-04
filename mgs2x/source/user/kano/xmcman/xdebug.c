/*
  XMCMAN デバッグ用
  2002/04/10 M.Kobayashi
  $Id: xdebug.c,v 1.1.1.3 2002/11/19 11:43:46 Yoshizawa1 Exp $

 */

#include	"gameheader.h"
#include	"xmcman.h"

#define DISP_CHANL			4
#define L2D_PRIORITY		0
#define L2D_STRCODE			0x0096a917		/* photo_save */
#define CODE_DEFAULTACTION		0x0008a3fb		/* DefaultAction */

typedef struct MCX_DB_WORK {
	MCX_MAN	man;
} MCX_DB_WORK;



void* NewMcxDebug( void )
{
	MCX_DB_WORK* pw;
	void* l2d_data;
	int	l2d_handle;	

	if( ( pw = (MCX_DB_WORK*) GV_Malloc( sizeof( MCX_DB_WORK ) ) ) == NULL ) {
		return NULL;
	}

	if( ( l2d_data = GV_GetCache( GV_CacheID( L2D_STRCODE,'o') ) ) ==NULL ) {
		HANGUP();
	}
	l2d_handle = L2D_SetupLayout(l2d_data,DISP_CHANL,L2D_PRIORITY,SPR_FLAG_PRIV );
	ASSERT( l2d_handle >= 0 );
//	L2D_EvokeAction( l2d_handle, CODE_DEFAULTACTION );
	
	MCX_InitManager( &pw->man, l2d_handle, NULL, 'a' );
	
	return pw;
}
