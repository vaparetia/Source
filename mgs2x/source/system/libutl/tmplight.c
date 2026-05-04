//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 
   tmplight.c
   テンプライトの呼び出し

   1999/07/27/ H.Tanaka
   version.  1999/08/18
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

#include	"gameheader.h"
#include	"def_dma.h"


void  Big_TmpLight2(
FVECTOR  *pos,
float    r_range,
float    e_range,
int      color,
int      flag
)
{
    if((flag < 1) || (3 < flag))
    {
	return ;
    }
    
    flag <<= 8 ;
    
    DG_SetTmpLight2(pos,r_range,e_range,color,flag) ;
}
