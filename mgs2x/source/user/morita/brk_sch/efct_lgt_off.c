//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   efct_lgt_off.c 
   投光器

   1999/12/26 T.Morita
   $Id: efct_lgt_off.c,v 1.1.1.3 2002/11/19 11:45:45 Yoshizawa1 Exp $
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


int NewCom_DestroyLight()
{
    IVECTOR buf ;
    FVECTOR pos ;

    if ( GCL_GetOption( 'p' ) != NULL )
    {
	GCL_GetNextIV( (int *)&buf ) ;
	vu0_IV0toFV( &buf, &pos ) ;

	DG_DestroyLightSphere( &pos, GCL_GetOptionValue( 'r', 4000 ) ) ;
	return 1 ;
    }

    return 0 ;
}
