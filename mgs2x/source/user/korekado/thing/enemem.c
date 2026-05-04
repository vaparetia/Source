//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	enemem.c
	敵兵情報記憶システム

	2001/04/30 Y.Korekado
	$Id: enemem.c,v 1.1.1.3 2002/11/19 11:44:27 Yoshizawa1 Exp $
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

#include	"gameheader.h"
#include	"korekado/conv/define.h"
#include	"enemem.h"

#define LINKVAR_TES	(1)
/*----------------------------------------------*/
typedef struct _ListEneMem {
	u_short	c_num ;
	u_short	load_num ;
	ENEMEM	enemem[ MAX_ENEMEM ] ;
} LIST_ENEMEM ;


#ifndef LINKVAR_TES
static LIST_ENEMEM	ListEneMem ;
#endif

//#define EneMem  ((ENEMEM *)&ENEMEM_EneMem[0]) ;

/*----------------------------------------------*/
static int SameData( ENEMEM *same_enm )
{
	ENEMEM *enm ;
	int		i ;
#ifdef LINKVAR_TES
	ENEMEM *EneMem ;

	EneMem = ((ENEMEM *)&ENEMEM_EneMem[0]) ;
	enm = EneMem ;
#else
	enm = &ListEneMem.enemem[0] ;
#endif
	for( i=0; i<MAX_ENEMEM; i++ ) {
		if ( enm->status == ENEMEM_ST_DEAD ) {
			if ( (enm->stage == same_enm->stage) && (enm->alive_name == same_enm->alive_name) ) {
				return i ;
			}
		} else {
			if ( (enm->stage == same_enm->stage) && (enm->name == same_enm->name) ) {
				return i ;
			}
		}
		enm++ ;
	}
	return -1 ;
}

/*----------------------------------------------*/
int ENEMEM_GclRenewMemory( void )
{
	ENEMEM *enm ;
	int stage, time, load_num, i ;

printf(" ENEMEM_GclRenewMemory\n" ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	stage = GCL_GetNextInt() ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	time = GCL_GetNextInt() ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	load_num = GCL_GetNextInt() ;


	enm = ((ENEMEM *)&ENEMEM_EneMem[0]) ;

printf("stage[%d] time[%d] load_num[%d]\n",stage, time, load_num ) ;

	for( i=0; i<MAX_ENEMEM; i++ ) {
printf("enm->stage[%d] status[%d]\n",enm->stage,enm->status ) ;
		if ( enm->stage != stage ) {	/* 現ステージ以外のデータを更新 */
			/* この時点でenm->loadnumに「０」がはいっていることはないはず！！ */
printf("enm->loadnum [%d] LOADSUB[%d] GM_ResetLoadCount[%d]\n",enm->loadnum,LOADSUB(enm->loadnum, load_num), GM_ResetLoadCount ) ;
			if ( LOADSUB(enm->loadnum, load_num) >= GM_ResetLoadCount ) {
				if ( enm->stage ){
//					ASSERT( enm->loadnum != 0 ) ;
				}
				enm->stage = 0 ;	/* リセット */
			} else if ( (enm->status == ENEMEM_ST_SLEEP) || (enm->status == ENEMEM_ST_FAINT) ||
						(enm->status == ENEMEM_ST_DEAD) ) {
				enm->time -= time ;
printf(" [%d] enmt->time[%d]\n",i,enm->time ) ;
				if ( enm->time < 0 ) {
printf(" wakeup stage[%d] name[%d] status[%d]\n",enm->stage,enm->name,enm->status ) ;
					enm->time = 0 ;
					enm->status = ENEMEM_ST_NORMAL ;
				}
			}
		} else {
			if ( (enm->status == ENEMEM_ST_DEAD) ) {
				/* 死体だけは更新,更新後そのステージで発生死体を登録すること*/
				enm->time -= time ;
printf(" [%d] enmt->time[%d]\n",i,enm->time ) ;
				if ( enm->time < 0 ) {
printf(" now stage[%d] name[%d] status[%d]\n",enm->stage,enm->name,enm->status ) ;
					enm->time = 0 ;
					enm->status = ENEMEM_ST_NORMAL ;
				}
			}
		}
		enm ++ ;
	}
	return 0 ;
}

int ENEMEM_GclSet( void )
{
	ENEMEM enm ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.stage = GCL_GetNextInt() ;
printf("enemem:stage[%d]\n",enm.stage) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.hzx_id = GM_GetHzxGroupID( GM_GetMapID(GCL_GetNextInt()) ) ;
printf("enemem:hzx_id[%d]\n",enm.hzx_id) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.name = GCL_GetNextInt() ;
printf("enemem:name[%d]\n",enm.name) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.x = (float)GCL_GetNextInt() ;
printf("enemem:x[%f]\n",enm.x) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.y = (float)GCL_GetNextInt() ;
printf("enemem:y[%f]\n",enm.y) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.z = (float)GCL_GetNextInt() ;
printf("enemem:z[%f]\n",enm.z) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.dir = GCL_GetNextInt() ;
printf("enemem:dir[%d]\n",enm.dir) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.status = GCL_GetNextInt() ;
printf("enemem:status[%d]\n",enm.status) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.pose = GCL_GetNextInt() ;
printf("enemem:pose[%d]\n",enm.pose) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.time = GCL_GetNextInt() ;
	enm.time = COUNT_VMODE(enm.time) ;
printf("enemem:time[%d]\n",enm.time) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.faint = GCL_GetNextInt() ;
printf("enemem:faint[%d]\n",enm.faint) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.life = GCL_GetNextInt() ;
printf("enemem:life[%d]\n",enm.life) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.pbreak = GCL_GetNextInt() ;
printf("enemem:pbreak[%d]\n",enm.pbreak) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.item = GCL_GetNextInt() ;
printf("enemem:item[%d]\n",enm.item) ;

	enm.loadnum = 0 ;	/* 必ず反映 */
	ENEMEM_Save( &enm ) ;

	return 0 ;
}

int ENEMEM_GclReset( void )
{
	int stage, name ;
	ENEMEM *enm ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	stage = GCL_GetNextInt() ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	enm = ENEMEM_Load( stage, name ) ;
	if ( enm == NULL ) return 0 ;
	enm->stage = 0 ;
	
	return 1 ;
}

int ENEMEM_GclAllReset( void )
{
	ENEMEM *enm ;
	int i ;

	enm = ((ENEMEM *)&ENEMEM_EneMem[0]) ;
	for( i=0; i<MAX_ENEMEM; i++ ) {
		enm->stage = 0 ;
		enm++ ;
	}

	ENEMEM_CurrentNum = 0 ;

	return 1 ;
}

int ENEMEM_GclGet( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int stage, name ;
	ENEMEM *enm ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	stage = GCL_GetNextInt() ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	enm = ENEMEM_Load( stage, name ) ;
	if ( enm == NULL ) return 0 ;

	/* マップID */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->hzx_id );

	/* X座標 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->x );

	/* Y座標 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->y );

	/* Z座標 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->z );

	/* 方向 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->dir );

	/* ステータス */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->status );

	/* ポーズ */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->pose );

	/* タイム */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->time );

	/* 気絶値 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->faint );

	/* ライフ値 */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->life );

	/* 部位ダメージ */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->pbreak );

	/* アイテム */
	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref );
	GCL_SetVarRef( &ref, 0, (int)enm->item );

	return 1 ;
}

int ENEMEM_GclCopy( void )
{
	int stage, name ;
	ENEMEM *from_enm, enm ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	stage = GCL_GetNextInt() ;
printf("enemem: copy form stage[%d]\n",stage) ;
	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;
printf("enemem: copy form name[%d]\n",name) ;

	from_enm = ENEMEM_Load( stage, name ) ;
	if ( from_enm == NULL ) return 0 ;

	enm = *from_enm ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.stage = GCL_GetNextInt() ;
printf("enemem: copy to stage[%d]\n",enm.stage) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.name = GCL_GetNextInt() ;
printf("enemem: copy to name[%d]\n",enm.name) ;





	/* マップID */
	if ( GCL_NextStr() == NULL ) return -1 ;
	enm.hzx_id = GM_GetHzxGroupID( GM_GetMapID(GCL_GetNextInt()) ) ;
printf("enemem: copy to map[%d]\n",enm.hzx_id) ;

	/* X座標 */
	if ( GCL_NextStr() != NULL ) {
		enm.x = GCL_GetNextInt() ;
		/* Y座標 */
		if ( GCL_NextStr() != NULL ) {
			enm.y = GCL_GetNextInt() ;
			/* Z座標 */
			if ( GCL_NextStr() != NULL ) {
				enm.z = GCL_GetNextInt() ;
			}
		}
	}

	enm.loadnum = 0 ;	/* 必ず反映 */
	ENEMEM_Save( &enm ) ;

	return 1 ;
}

void ENEMEM_Set( ENEMEM *enm, int stage, int hzx_id, int name, FVECTOR *pos, int dir,
				int status, int pose, int time, int item,
				int life, int faint, int pbreak, int alive_name )
{
	enm->stage = stage ;
	enm->hzx_id = hzx_id ;
	enm->name = name ;
	enm->alive_name = alive_name ;
	enm->x = (float)pos->vx ;
	enm->y = (float)pos->vy ;
	enm->z = (float)pos->vz ;
	enm->dir = (short)dir ;
	enm->status = (u_char)status ;
	enm->time = (u_int)time ;
	enm->pose = (u_char)pose ;
	enm->life = (short)life ;
	enm->faint = (short)faint ;
	enm->pbreak = (short)pbreak ;
	enm->item = (u_char)item ;
	enm->loadnum = (u_char)GM_GlobalLoadCount ;
}

void ENEMEM_Save( ENEMEM *save_enm )
{
	ENEMEM *enm ;
	int n, l ;

	ENEMEM *EneMem ;

	EneMem = ((ENEMEM *)&ENEMEM_EneMem[0]) ;

	n = SameData( save_enm ) ;

#ifdef LINKVAR_TES
	if ( n < 0 ) {
		if ( (n = ENEMEM_CurrentNum+1) >= MAX_ENEMEM ) n = 0 ;

		enm = EneMem + n ;
printf("enm[%x] EneMem[%x] ENEMEM_CurrentNum [%d] n[%d]\n",enm, EneMem, ENEMEM_CurrentNum,n ) ;
printf("enemem:enm->loadnum[%d] GM_GlobalLoadCount[%d]\n",enm->loadnum,GM_GlobalLoadCount) ;
		l = LOADSUB(enm->loadnum, GM_GlobalLoadCount) ;
		if ( l != 0 ) {
//			ASSERT ( l > GM_ResetLoadCount ) ;
		}
		ENEMEM_CurrentNum = n ;
	} else {
		enm = EneMem + n ;
	}
#else

	if ( n < 0 ) {
		if ( (n = ListEneMem.c_num+1) >= MAX_ENEMEM ) n = 0 ;
		enm = &ListEneMem.enemem[n] ;
printf("enemem:enm->loadnum[%d] GM_GlobalLoadCount[%d]\n",enm->loadnum,GM_GlobalLoadCount) ;
		l = LOADSUB(enm->loadnum, GM_GlobalLoadCount) ;
		if ( l != 0 ) {
			ASSERT ( l > ListEneMem.load_num ) ;
		}
		ListEneMem.c_num = n ;
	} else {
		enm = &ListEneMem.enemem[n] ;
	}
#endif

	*enm = *save_enm ;
printf("enemem save:n[%d] add[%x] stage[%d] name[%d]\n",n,enm,enm->stage,enm->name) ;

printf("enemem:stage[%d]\n",enm->stage) ;
printf("enemem:name[%d]\n",enm->name) ;
printf("enemem:alive_name[%d]\n",enm->alive_name) ;
printf("enemem:hzx_id[%d]\n",enm->hzx_id) ;
printf("enemem:x[%f]\n",enm->x) ;
printf("enemem:y[%f]\n",enm->y) ;
printf("enemem:z[%f]\n",enm->z) ;
printf("enemem:dir[%d]\n",enm->dir) ;
printf("enemem:status[%d]\n",enm->status) ;
printf("enemem:pose[%d]\n",enm->pose) ;
printf("enemem:time[%d]\n",enm->time) ;
printf("enemem:life[%d]\n",enm->life) ;
printf("enemem:faint[%d]\n",enm->faint) ;
printf("enemem:pbreak[%d]\n",enm->pbreak) ;
printf("enemem:item[%d]\n",enm->item) ;


}

ENEMEM *ENEMEM_Load( int stage, int name )
{
	ENEMEM *enm ;
	int i, l ;
	ENEMEM *EneMem ;

	EneMem = ((ENEMEM *)&ENEMEM_EneMem[0]) ;

printf("enemem:stage[%d] name[%d]\n",stage,name) ;

#ifdef LINKVAR_TES
	enm = EneMem ;
#else
	enm = &ListEneMem.enemem[0] ;
#endif

	for( i=0; i<MAX_ENEMEM; i++ ) {
//	for( i=0; i<10; i++ ) {
//printf("i[%d]:add[%x] stage[%d] name[%d] \n",i,enm,enm->stage,enm->name ) ;
		if ( (enm->stage == stage) && (enm->name == name) ) {
			/* 指定回数以内だったら */
			l = LOADSUB(enm->loadnum, GM_GlobalLoadCount) ;
printf("		[%d]:stage[%d] name[%d] loadnum[%d] GM_GlobalLoadCount[%d] L[%d]/[%d]\n",
				i,enm->stage,enm->name,enm->loadnum,GM_GlobalLoadCount,l,GM_ResetLoadCount) ;
			if ( l <= GM_ResetLoadCount || (enm->loadnum==0)) {
				return enm ;
			}
		}
		enm++ ;
	}

	return NULL ;
}

ENEMEM *ENEMEM_DeadLoad( int stage, int alive_name )
{
	ENEMEM *enm ;
	int i, l ;
	ENEMEM *EneMem ;

	EneMem = ((ENEMEM *)&ENEMEM_EneMem[0]) ;

printf("enemem: dead stage[%d] alive_name[%d]\n",stage,alive_name) ;

#ifdef LINKVAR_TES
	enm = EneMem ;
#else
	enm = &ListEneMem.enemem[0] ;
#endif

	for( i=0; i<MAX_ENEMEM; i++ ) {
		if ( (enm->stage == stage) && (enm->alive_name == alive_name) ) {
			/* 指定回数以内だったら */
			l = LOADSUB(enm->loadnum, GM_GlobalLoadCount) ;
printf("		[%d]:stage[%d] name[%d] loadnum[%d] GM_GlobalLoadCount[%d] L[%d]/[%d]\n",
				i,enm->stage,enm->name,enm->loadnum,GM_GlobalLoadCount,l,GM_ResetLoadCount) ;
			if ( l <= GM_ResetLoadCount || (enm->loadnum==0)) {
				return enm ;
			}
		}
		enm++ ;
	}

	return NULL ;
}

#if 0
/*いらんかも*/
void ENEMEM_Init( int load_num )
{
	ENEMEM *enm ;
	int i ;


	enm = &ListEneMem.enemem[0] ;
	for( i=0; i<MAX_ENEMEM; i++ ) {
		enm->stage = 0 ;
	}

	ListEneMem.c_num = 0 ;
	ASSERT( load_num < MAX_LOAD_MEM ) ;
	ListEneMem.load_num = load_num ;
}
#endif
