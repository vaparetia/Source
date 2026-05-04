//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  brk_hazard.c
  共有箱型ハザード
  
  2000/03/30 T. Morita
  2000/10/16 1.14 T.Morita
  $Id: brk_hzd_ini.c,v 1.1.1.3 2002/11/19 11:45:31 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

#include "brk_hazard.h"

int      BRK_HZD_n_hzd ;
HZD_BOX *BRK_HZD_Pool[BRK_N_HZD] ;

void BRK_InitSystem()
{
    int i ;

    /* ダイナミックハザード の 初期化も済ます */
    BRK_HZD_n_hzd = 0 ;
    for( i=0; i<BRK_N_HZD ; i++ )
	BRK_HZD_Pool[i] = NULL ;
}

