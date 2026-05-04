//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "gameheader.h"
#include "game.h"

void	*NewUnusedChara( int name, int map )
{
	printf( "This Chara is NOT SUPPORTED now\n" );
	return NULL;
}
