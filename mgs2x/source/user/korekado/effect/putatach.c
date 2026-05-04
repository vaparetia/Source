//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	NewDemoAttachments.c
	デモ用装備品

	2000/04/19 Y.Korekado
	$Id: putatach.c,v 1.1.1.3 2002/11/19 11:44:06 Yoshizawa1 Exp $
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
#include	"libmt.h"
#include	"libutl.h"

#include	"gameheader.h"
#include	"korekado/conv/define.h"
#include	"../../kano/attachment/attachments.h"

#if 1
#include	"../../kano/attachment/ene_arg.h"
#include	"../../kano/attachment/sna_arg.h"
#include	"../../kano/attachment/org_arg.h"
#include	"../../kano/attachment/crg_arg.h"
#include	"../../kano/attachment/org_plt_arg.h"
#else
#include	"ene_arg.h"
#include	"sna_arg.h"
#include	"org_arg.h"
#endif
/*----------------------------------------------------------------*/
/* arg2 0～15bit*/
#define	MAX_ATARG2	16
/* arg3 16～31bit*/
#define	MAX_ATARG3	16

typedef struct {
	ATTACHMENT_ARGUMENT2	*arg2 ;
	ATTACHMENT_ARGUMENT3	*arg3 ;
} ATTACH_ARGS ;

static	ATTACH_ARGS attach_args[]= {
	{ ene_arg2,	ene_arg3 },
	{ NULL, sna_arg3 },
	{ org_arg2, org_arg3 },
	{ crg_arg2, crg_arg3 },
	{ org_plt_arg2, org_plt_arg3 },
} ;

enum {
	ARG_ID_ENE,
	ARG_ID_SNA,
	ARG_ID_ORG,
	ARG_ID_CRG,
	ARG_ID_ORG_PLT,
};

#define USE_ACT_EX
/*----------------------------------------------------------------*/

typedef	struct	{
#ifdef USE_ACT_EX
	GV_ACT_EX	actor ;
#else
	GV_ACT	actor ;
#endif
	OBJECT		*body ;
	int			flag ;

	void	*attachments ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
}
static	void	Die( work )
Work		*work ;
{
#ifdef USE_ACT_EX
	if ( !(GV_IsFollowDestroy( work )) ) {
		if( work->attachments != NULL ) GV_DestroyActor( work->attachments ) ;
		printf("DemoAttachments: own destroy \n");
	} else printf("DemoAttachments: parent destroy \n");
#else
	if( work->attachments != NULL ) GV_DestroyActor( work->attachments ) ;
#endif
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, parent, body, at_id, flag )
Work	*work ;
void	*parent ;
OBJECT	*body ;
int		at_id ;
int		flag ;
{
	ATTACHMENT_ARGUMENT2	at_arg2[MAX_ATARG2], *arg2 ;
	ATTACHMENT_ARGUMENT3	at_arg3[MAX_ATARG3], *arg3 ;
	int at2_num, at3_num, i ;

	work->body = body ;
	work->flag = flag ;

	work->attachments = NULL ;
	at2_num = at3_num = 0 ;

	arg2 = attach_args[ at_id ].arg2 ;
	for ( i=0; i<MAX_ATARG2; i++ ) {
		if ( flag & (1<<i) ) {
printf(" arg2 i[%d]\n",i ) ;
			at_arg2[ at2_num++ ] = arg2[ i ] ;
		}
	}

	arg3 = attach_args[ at_id ].arg3 ;
	for ( i=0; i<MAX_ATARG3; i++ ) {
		if ( flag & (1<<(i+MAX_ATARG2)) ) {
printf(" arg3 i[%d]\n",i ) ;
			at_arg3[ at3_num++ ] = arg3[ i ] ;
		}
	}

	if ( at3_num+at2_num > 0 ) {
#ifdef USE_ACT_EX
		//printf("shibashiba\n");
        if(!parent) parent = work;
		GV_SetActorChild( parent, 
			(work->attachments = NewAttachments_called(body,NULL,0,at_arg2,at2_num,at_arg3,at3_num)) ) ;
#else
		work->attachments = NewAttachments_called(body,NULL,0,at_arg2,at2_num,at_arg3,at3_num) ;
#endif
	} else {
		return -1 ;
	}
printf("DemoAttachments: attach[%d] flag [%x] \n", at_id, flag);
	
	return 0 ;
}
/*----------------------------------------------------------------*/
void		*NewPutAttachments( parent, body, at_id, flag )
void	*parent ;
OBJECT	*body ;
int		at_id ;
int		flag ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
#ifdef USE_ACT_EX
		GV_ActorEX( &( work->actor ) ) ;
#endif
		if ( GetResources( work, parent, body, at_id, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
