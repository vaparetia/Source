//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   padctrl.c
   パッドコントロール
   
   2000/03/14 M.Sonoyama
   $Id: padctrl.c,v 1.1.1.3 2002/11/19 11:41:54 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

extern	void	NewStartScenarioDemo( void ) ;
extern	void	NewEndScenarioDemo( void ) ;

/* シナリオコマンド "パッド操作" */
void	NewPadControl( void )
{
    int		which ;
    
    if ( GCL_GetOption( 'w' ) != NULL ) { /* which */
		which = GCL_GetNextInt() ;
    } else {
		which = 0 ;
    }
    if ( GCL_GetOption( 'r' ) != NULL ) { /* release */
		GV_PadReleaseOnScn( which ) ;
		NewStartScenarioDemo() ;
    } else if ( GCL_GetOption( 'm' ) != NULL ) { /* mask */
		GV_PadMaskOnScn( which, GCL_GetNextInt() ) ;
    } else if ( GCL_GetOption( 'p' ) != NULL ) { /* press */
		GV_PadPressScn( which, GCL_GetNextInt() ) ;
    } else if ( GCL_GetOption( 'c' ) != NULL ) { /* cancel */
		if ( GCL_NextStr() == NULL ) {
			GV_PadMaskCancelScn( which, 0 ) ;
			NewEndScenarioDemo() ;
		} else {
			int		mode ;

			mode = GCL_GetNextInt() ;
			GV_PadMaskCancelScn( which, mode ) ;
			if ( mode == 0 || ( mode & 1 ) ) {
				NewEndScenarioDemo() ;
			}
		}

    }
}

/* シナリオコマンド”パッドチェック” */
int	NewPadCheck( void )
{
    int			which ;
	GV_PAD		*pad ;

	if ( GCL_GetOption( 'd' ) != NULL ) pad = GV_PadDataDirect ;
	else							    pad = GV_PadData ;
    which = 0 ;
    if ( GCL_GetOption( 'w' ) != NULL ) {
		which = GCL_GetNextInt() ;
    }
    if ( GCL_GetOption( 'p' ) != NULL ) {
		return pad[ which ].press ;
    } else if ( GCL_GetOption( 's' ) != NULL ) {
		return pad[ which ].status ;
    } else if ( GCL_GetOption( 'r' ) != NULL ) {
		return pad[ which ].release ;
	} else if ( GCL_GetOption( 'n' ) != NULL ) {
		return ( pad[ which ].flag & GV_PAD_RELEASE ) ? 1 : 0 ;
	}
    return 0 ;
}
