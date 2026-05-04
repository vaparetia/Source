//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	corpcom.c
	死体管理

	1999/09/13 Y.Korekado
	$Id: corpcom.c,v 1.1.1.3 2002/11/19 11:44:07 Yoshizawa1 Exp $
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
#include	"gameheader.h"
#include	"enemy.h"

#include	"libutl.h"
/* ---------------------------------------------------------------- */
#define	LIMITED_CORP	(1)
#define MAX_CORP		(10)

/* ---------------------------------------------------------------- */
typedef struct {
	NEWCORP	*n_start ;
	NEWCORP	*n_end ;
	int		n_num ;
	int		n_id ;
	u_long64	uniq_name_buff ;

	OLDCORP	*o_start ;
	OLDCORP	*o_end ;
	int		o_num ;
	int		o_id ;
} CORPSYS	;

static CORPSYS	Corp ;

/* ---------------------------------------------------------------- */

static	inline	void	MatToVec( w, v )
FMATRIX	*w ;
FVECTOR	*v ;
{
	v->vx = w->m[3][0] ;
	v->vy = w->m[3][1] ;
	v->vz = w->m[3][2] ;
}

/* ---------------------------------------------------------------- */
void	CP_InitCorpSystem( void )
{
	Corp.n_start = NULL ;
	Corp.n_end = NULL ;
	Corp.n_num = 0 ;
	Corp.n_id = 0 ;
	Corp.o_start = NULL ;
	Corp.o_end = NULL ;
	Corp.o_num = 0 ;
	Corp.o_id = 0 ;
	
	Corp.uniq_name_buff = 0 ;
}


int	GetUniqName( void )
{
	int i;

	for( i=0; i<64; i++ ){
		
		if( !(Corp.uniq_name_buff & (I64(1)<<i)) ) {
			Corp.uniq_name_buff |= (I64(1)<<i) ;
printf("corp: uniq name [%d] \n",i ) ;
			return i  ;
		}
	}

printf("corp: uniq name Err \n" ) ;
	return i  ;
}
void	FreeUniqName( u_int i )
{
printf("corp: uniq name delet [%d] \n",i ) ;
	Corp.uniq_name_buff &= ~(I64(1)<<i) ;
}

/* ---------------------------------------------------------------- */
int CP_NewCorpNum( void )
{
	return Corp.n_num ;
}
int CP_OldCorpNum( void )
{
	return Corp.o_num ;
}


//****************さとよし追加******************
NEWCORP* CP_GetCorpN_start( void )
{
	return Corp.n_start ;
}


void	CP_SetCorp( corp, ctrl, zadd, uniq_id, entk )
NEWCORP	*corp ;
CONTROL	*ctrl ;
int		*zadd ;
int		uniq_id ;
ENETHINK *entk ;
{
	corp->ctrl = ctrl ;
	corp->zadd = zadd ;
	corp->id = Corp.n_id++ ;
	corp->uniq_id = uniq_id ;
	corp->count = 0 ;
	corp->flag = 0 ;
	corp->entk = entk ;

	corp->before = NULL ;
	corp->next = NULL ;
}

void	CP_PutCorp( corp )
NEWCORP	*corp ;
{
	if ( Corp.n_start == NULL ) {
		Corp.n_start = corp ;
	}
	corp->before = Corp.n_end ;
	if ( corp->before != NULL ) {
		corp->before->next = corp ;
	}
	corp->next = NULL ;
	Corp.n_end = corp ;
	Corp.n_num ++ ;
#ifdef LIMITED_CORP
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( Corp.n_num > MAX_CORP ) {
			NEWCORP	*rotten ;
			int n, i ;
			
			n = Corp.n_num - MAX_CORP - 1 ;
			rotten = Corp.n_start ;
			for ( i=0; i<n; i++ ) {
				ASSERT( rotten->next != NULL ) ;
				rotten = rotten->next ;
			}
			rotten->count = 0 ;
		}
	}
#endif

printf(" New Corp num[%d] \n",Corp.n_num );
}

static	void	FreeCorp( corp ) 
NEWCORP	*corp ;
{
	if ( corp->before != NULL ) {
		corp->before->next = corp->next ;
	} else {
		Corp.n_start = corp->next ;
	}
	if ( corp->next != NULL ) {
		corp->next->before = corp->before ;
	} else {
		Corp.n_end = corp->before ;
	}
	Corp.n_num -- ;
printf(" New Corp Free num[%d] \n",Corp.n_num );
}
void	CP_FreeCorp( corp )
NEWCORP	*corp ;
{
	NEWCORP	*cur ;
	
	cur = Corp.n_start ;
	/* 登録されているかチェック */
	while ( cur != NULL ) {
		if ( cur == corp ) {
			FreeCorp( corp ) ;
			return ;
		}
		cur = cur->next ;
	}
}

int CP_GetCorpID( entk )
ENETHINK	*entk ;
{
	NEWCORP	*cur ;
	EYEINFO		eyei ;
	
	cur = Corp.n_start ;
	while ( cur != NULL ) {
		ENE_SetEyeInfo( &eyei, &cur->ctrl->mov, cur->zadd, 0, &cur->ctrl->hzx_id ) ;
#if 0
		ENE_EyeInfoCheck( entk, &eyei ) ;
#else
		{
			int w_hzx_id ;
			w_hzx_id = entk->ctrl->hzx_id | cur->ctrl->hzx_id ;
			ENE_EyeInfoCheckWhzxid( entk, &eyei, w_hzx_id ) ;
		}
#endif
//printf(" sight[%d] dis[%d] dir[%d] \n",eyei.sight, eyei.dis, eyei.dir ) ;
		if ( eyei.sight == EYE_INFO_SIGHT_IN ) {
			if ( !(cur->flag&CORPS_ST_REPORT) ) {
				if ( (entk->status & ENE_STATUS_ACCDNT_REPO)
					|| (eyei.dis < entk->sense.eye_s) ) { /* 双眼鏡は無視 */
					return cur->uniq_id ;
				}
			}
		}
		cur = cur->next ;
	}
	return -1 ;
}

NEWCORP	*CP_GetCorp( uniq_id )
int	uniq_id ;
{
	NEWCORP	*cur ;

	cur = Corp.n_start ;
	while ( cur != NULL ) {
		if ( cur->uniq_id == uniq_id ) {
			return cur ;
		}
		cur = cur->next ;
	}
	return NULL ;
}

int CP_GetCorpNum( void )
{
	NEWCORP	*cur ;
	int num = 0;

	cur = Corp.n_start ;
	while ( cur != NULL ) {
		num++ ;
		cur = cur->next ;
	}
	return num ;
}
/************************************/
int CP_CaptureCorpKill( void )
{
	NEWCORP	*cur ;

	cur = Corp.n_start ;
	while ( cur != NULL ) {
		if ( cur->flag & CORPS_ST_CAPTURE ) {
			SET_FLAG( cur->flag, CORPS_ST_KILL ) ;
			return 1 ;
		}
		cur = cur->next ;
	}
	return 0 ;
}

ENETHINK *CP_CaptureCorpModelName( void )
{
	NEWCORP	*cur ;

	cur = Corp.n_start ;
	while ( cur != NULL ) {
		if ( cur->flag & CORPS_ST_CAPTURE ) {
			return cur->entk ;
		}
		cur = cur->next ;
	}
	
	return NULL ;
}

/* ---------------------------------------------------------------- */
void	CP_SetOldCorp( corp, pos )
OLDCORP	*corp ;
FVECTOR	*pos ;
{
	corp->pos = *pos ;
	corp->id = Corp.o_id++ ;

	corp->before = NULL ;
	corp->next = NULL ;
}

void	CP_PutOldCorp( corp )
OLDCORP	*corp ;
{
	if ( Corp.o_start == NULL ) {
		Corp.o_start = corp ;
	}
	corp->before = Corp.o_end ;
	if ( corp->before != NULL ) {
		corp->before->next = corp ;
	}
	corp->next = NULL ;
	Corp.o_end = corp ;
	Corp.o_num ++ ;
printf(" Old Corp num[%d] \n",Corp.o_num );
}

static	void	FreeOldCorp( corp ) 
OLDCORP	*corp ;
{
	if ( corp->before != NULL ) {
		corp->before->next = corp->next ;
	} else {
		Corp.o_start = corp->next ;
	}
	if ( corp->next != NULL ) {
		corp->next->before = corp->before ;
	} else {
		Corp.o_end = corp->before ;
	}
	Corp.o_num -- ;
printf(" Old Corp Free num[%d] \n",Corp.o_num );
}
void	CP_FreeOldCorp( corp )
OLDCORP	*corp ;
{
	OLDCORP	*cur ;
	
	cur = Corp.o_start ;
	/* 登録されているかチェック */
	while ( cur != NULL ) {
		if ( cur == corp ) {
			FreeOldCorp( corp ) ;
			return ;
		}
		cur = cur->next ;
	}
}

/* ---------------------------------------------------------------- */

