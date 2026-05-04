/*
    util.c
    便利関数
    2001/04/04 Masafumi Okuta
    $Id: util.c,v 1.1.1.3 2002/11/19 11:48:02 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

int MAO_BoundCheck(FVECTOR vecPos, FVECTOR vec1, FVECTOR vec2);
void MAO_DbgPlayerPosDump(void);


int MAO_BoundCheck(FVECTOR vecPos, FVECTOR vec1, FVECTOR vec2)
{
    if ( vecPos.vx >= vec1.vx && 
	 vecPos.vy >= vec1.vy && 
	 vecPos.vz >= vec1.vz && 
	 vecPos.vx <= vec2.vx && 
	 vecPos.vy <= vec2.vy && 
	 vecPos.vz <= vec2.vz)
	return 1;
    return 0;
}


void MAO_DbgPlayerPosDump(void)
{
#ifdef DEBUG_MODE
    if ( (GV_PadData[ 0 ].press & PAD_L1) ){
	printf("{ %8.1ff, %8.1ff, %8.1ff }, // \n", GM_PlayerPosition.vx, GM_PlayerPosition.vy, GM_PlayerPosition.vz);
    }
#endif
}
