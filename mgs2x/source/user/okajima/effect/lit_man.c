//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	lit_man.c
	プログラム制御光源位置アドレス格納バッファ

	1999/10/15 S.Okajima
	$Id: lit_man.c,v 1.1.1.3 2002/11/19 11:47:07 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#ifdef PSX2
#include	"def_dma.h"
#endif

#include	"lit_man.h"



typedef	struct	{
	GV_ACT_EX	actor ;
} Work ;

Work	*OK_LIGHT_MAN_WORK = NULL;

static void Act( Work *work )
{
	int i;

	for( i=0; i<MAX_LIGHT_NUM; i++ ){
		ok_lit_pos_sorted[i] = NULL;
	}

	ok_lit_data_num=0;
	for( i=0; i<MAX_LIGHT_NUM; i++ ){
		if( ok_lit_pos[i] != NULL ){
			ok_lit_pos_sorted[ok_lit_data_num] = ok_lit_pos[i];
			ok_lit_rot_sorted[ok_lit_data_num] = ok_lit_rot[i];
			ok_lit_data_num++;
		}
	}

	for( i=ok_lit_data_num; i<MAX_LIGHT_NUM; i++ ){
		ok_lit_pos_sorted[i] = NULL;
		ok_lit_rot_sorted[i] = NULL;
	}



//printf("\n");
}

static void Die( Work *work )
{
	OK_LIGHT_MAN_WORK = NULL;
}

static int GetResources( Work *work )
{
	int i;

	ok_lit_data_num=0;
	for( i=0; i<MAX_LIGHT_NUM; i++ ){
		ok_lit_pos[i]        = NULL;
		ok_lit_rot[i]        = NULL;
		ok_lit_pos_sorted[i] = NULL;
		ok_lit_rot_sorted[i] = NULL;
	}

	return (0);
}



void *NewDynamicLightPosManager( void )
{
	Work		*work ;

	OPERATOR() ;

	if( OK_LIGHT_MAN_WORK!=NULL ) return (void *)1;

	work = (Work *)GV_NewEffect( GV_ACTOR_PREV, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	OK_LIGHT_MAN_WORK = work;
	return (void *)work ;
}


void OK_RemoveDynamicLight( FVECTOR *pos )
{
	int i;

//printf("OK_RemoveDynamicLight\n");

	for( i=0; i<MAX_LIGHT_NUM; i++ ){
		if( ok_lit_pos[i] == pos ){
//printf("OK_RemoveDynamicLight:Success\n");
			ok_lit_pos[i] =  NULL;
			ok_lit_rot[i] =  NULL;
			return;
		}
	}
}

/* rot は 光の強度の計算に用いる */
void OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot )
{
	int i;

	if( OK_LIGHT_MAN_WORK==NULL ) NewDynamicLightPosManager();

//printf("OK_SetDynamicLight\n");
	for( i=0; i<MAX_LIGHT_NUM; i++ ){
		if( ok_lit_pos[i] == pos ) return;	/* 二重登録は弾く */
	}

	for( i=0; i<MAX_LIGHT_NUM; i++ ){
		if( ok_lit_pos[i] == NULL ){
//printf("OK_SetDynamicLight:Success\n");
			ok_lit_pos[i] =  pos;
			ok_lit_rot[i] =  rot;
			return;
		}
	}
}
