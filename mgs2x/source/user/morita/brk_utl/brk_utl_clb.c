//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_utl_act.c
   壊れ用 汎用ルーチン

   2000/09/14 T. Morita
   $Id: brk_utl_clb.c,v 1.1.1.3 2002/11/19 11:45:50 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_utl.h"
#include "../brk_hzd/brk_hazard.h"


/*

  貫通止め

*/
void BRK_UTL_CallOffenceWhenThrough( TARGET *off, TARGET *def )
{
    off->hit = def->hit ;
    off->damaged |= off->class & TARGET_CHECK_CLASS ;
    if ( off->callback != NULL )
	( *off->callback )( off, def, off->work ) ;
}
