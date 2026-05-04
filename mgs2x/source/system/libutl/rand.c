//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>

unsigned int random_seed = 1 ;
void init_rnd( int x )
{
    random_seed = x ;
}
