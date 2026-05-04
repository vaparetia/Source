//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_demo.c
	�猀薜梴鴷ル獌捗雿鶗諢憧薔�

	2001/07/23 K.Takabe
	$Id: pdr_demo.c,v 1.1.1.3 2002/11/19 11:51:24 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

#include "pdray.h"
#include "r_common.h"

/* ---------------------------------------------------------------- */
extern void *NewOozeBlood( DG_OBJS *objs, int model_id );
extern void TAKABE_OozeBloodSetColor( void *_work, float r, float g, float b );
extern void TAKABE_OozeBloodClear( void *_work, float scale );
extern void *NewOozeBloodDemo( int name, DG_OBJS *objs, int model_id );
/* ---------------------------------------------------------------- */
	/*
		稒心瑜目伙由□伙□民件
	*/
void *NewRayOozeBloodDemo( int name, DG_OBJS *objs )
{
	void	*work ;
	work = NewOozeBloodDemo( name, objs, 2729025/* "pdray_def_mt" */ );
	if ( work != NULL ){
		TAKABE_OozeBloodSetColor( work, 0.5f*1.8f, 0.9f*1.8f, 0.9f*1.8f );
	}
	return ( work );
}
void PDRAY_OozeBloodAddDemo( int name, int joint_num, float x, float y, float z, float range )
{
    GV_MSG	msg ;
	int		buffer[8] ;
	msg.address = name ;
	msg.message = buffer ;
	buffer[0] = 0 ;
	buffer[1] = joint_num ;
	buffer[2] = x ;
	buffer[3] = y ;
	buffer[4] = z ;
	buffer[5] = range ;
	msg.message_len = 6 ;
	GV_SendMessage( &msg );
}
/* ---------------------------------------------------------------- */
