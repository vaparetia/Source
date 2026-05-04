//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	radarctrl.c
	レーダー用パラメータデータ管理ルーチン

	1999/11/11 K.Uehara
	$Id: radarctrl.c,v 1.1.1.3 2002/11/19 11:41:54 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

static struct {
	struct _radar_ctrl *next;
} top = { NULL };

static FVECTOR center;

void GM_RadarSetCenter( FVECTOR *vec )
{
	center = *vec;
}

FVECTOR *GM_RadarGetCenter( void )
{
	return &center;
}

static void add_link( RADAR_CTRL *rctrl )
{
	RADAR_CTRL *p;

	p = ( RADAR_CTRL * )&top;

	rctrl->next = p->next;
	p->next = rctrl;
}

static void remove_link( RADAR_CTRL *rctrl )
{
	RADAR_CTRL *p, *prev;

	p = ( RADAR_CTRL * )&top;
	prev = p;

	for( ; p != NULL; p = p->next ){
		if( p == rctrl ){
			prev->next = p->next;
			return;
		}
		prev = p;
	}
	ASSERT( FALSE );
}

extern int SIG_InitRadarSprite(RADAR_CTRL *);
extern void SIG_FreeRadarSprite(RADAR_CTRL *);

void GM_InitRadarControl( RADAR_CTRL *rctrl, FVECTOR *pos, int flag, int map )
{
	if(SIG_InitRadarSprite(rctrl)){
#ifdef DEBUG_MODE
		printf("RADAR SPRITE NO MEMORY!!\n");
#endif
		return ;
	}
#ifdef DEBUG_MODE
	printf("RADAR SPRITE CREATE!!\n");
#endif
	add_link( rctrl );
	rctrl->flag = flag;
	rctrl->map = map;
	rctrl->pos = pos;
	rctrl->same_floor_rate = DEFAULT_SAME_FLOOR_RATE;
	if(rctrl->flag & RADAR_SIGHT){
		printf("SET SIGHT RADAR !!!\n ");
	}else {
		printf("UNSET SIGHT RADAR !!!\n ");
	}
}

void GM_FreeRadarControl( RADAR_CTRL *rctrl )
{
	SIG_FreeRadarSprite(rctrl) ;
	remove_link( rctrl );
}

void GM_InitRadarSystem( void )
{
	top.next = NULL;
}

RADAR_CTRL *GM_RadarControlGetTop( void )
{
	return top.next;
}

