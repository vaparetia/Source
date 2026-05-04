//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	enefind.c
	敵兵の発見物管理

	2000/06/09 Y.Korekado
	$Id: enefind.c,v 1.1.1.3 2002/11/19 11:41:48 Yoshizawa1 Exp $
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
///#include	"libutl.h"

ENEFINDLISTLIST	GM_EneFindListList ;
ENEFINDLIST		GM_EneFindList ;

/* ---------------------------------------------------------------- */
void	GM_InitEneFindSystem( void )
{
	GM_EneFindListList.start = &GM_EneFindList ;
	GM_EneFindListList.end = &GM_EneFindList ;
	GM_EneFindListList.num = 1 ;
	GM_EneFindListList.unique_id = 1 ;
	
	GM_EneFindList.start = NULL ;
	GM_EneFindList.end = NULL ;
	GM_EneFindList.num = 0 ;
	GM_EneFindList.unique_id = 0 ;
	GM_EneFindList.listtype = EF_LIST_TYPE_ONE ;
	GM_EneFindList.id = 0 ;
}
/* ---------------------------------------------------------------- */
ENEFINDLIST	*GM_GetEneFindList( int id )
{
	ENEFINDLIST		*efl ;
	
	efl = GM_EneFindListList.start ;
	while( efl != NULL ) {
		if( efl->id == id ) return efl ;
		efl = efl->next ;
	}
	
	return NULL ;
}
void	GM_InitEneFindList( ENEFINDLIST *efl, int type )
{
	efl->start = NULL ;
	efl->end = NULL ;
	efl->num = 0 ;
	efl->unique_id = 0 ;
	efl->listtype = type ;
	efl->id = 0 ;
}

void	GM_PutEneFindList( ENEFINDLIST *efl )
{
	ENEFINDLISTLIST	*efll ;

	efll = &GM_EneFindListList ;
	
	if ( efll->start == NULL ) {
		efll->start = efl ;
	} else {
		efll->end->next = efl ;
	}
	efl->next = NULL ;
	efl->id = efll->unique_id ;
	efll->end = efl ;
	efll->num ++ ;
	efll->unique_id ++ ;
#if 1
	if( efll->unique_id > 60000 ) efll->unique_id = 0 ;
#else
	ASSERT( efll->unique_id < 10000 ) ;
#endif

//printf(" Ene Find List Num[%d] id[%d]\n", efll->num, efl->id ) ;
}

void	GM_FreeEneFindList( ENEFINDLIST *efl )
{
	ENEFINDLISTLIST *efll ;
	ENEFINDLIST 	*list, *cur ;
	
	ASSERT( efl != NULL ) ;

	efll = &GM_EneFindListList ;
	list = efll->start ;

	if ( list == efl ) {
		efll->start = efl->next ;
		efll->num -- ;
		if( efl->next == NULL ) {
			efll->end = NULL ;
		}
		return ;
	}

	while ( (cur = list->next) != NULL ) {
		if ( cur == efl ) {
			efll->num -- ;
			list->next = efl->next ;
			if( efl->next == NULL ) {
				efll->end = list ;
			}
			return ;
		}
		list = cur ;
	}
}

/* --- リンク型 --------------------------------------------------- */
ENEFIND	*GM_GetEneFindFromList( ENEFINDLIST	*efl, int id )
{
	ENEFIND		*ef ;

	ASSERT( efl != NULL ) ;

	ef = efl->start ;
	while( ef != NULL ) {
		if( ef->id == id ) return ef ;
		ef = ef->next ;
	}
	
	return NULL ;
}

void	GM_PutEneFindToList( ENEFINDLIST *efl, ENEFIND *ef )
{
	ASSERT( efl != NULL ) ;

	if ( efl->start == NULL ) {
		efl->start = ef ;
	} else {
		efl->end->next = ef ;
	}
	ef->next = NULL ;
	ef->id = efl->unique_id ;
	efl->end = ef ;
	efl->num ++ ;
	efl->unique_id ++ ;
#if 1
	if( efl->unique_id > 60000 ) efl->unique_id = 0 ;
#else
	ASSERT( efl->unique_id < 10000 ) ;
#endif

//printf("Ene Find ListID[%d] Num[%d] id[%d] type[%x]\n", efl->id, efl->num, ef->id, ef->type ) ;
}

void	GM_FreeEneFindFromList( ENEFINDLIST *efl, ENEFIND *ef )
{
	ENEFIND 	*list, *cur ;
//printf("Free Call id[%d] num [%d] ef_id[%d]\n",efl->id, efl->num, ef->id ) ;

	ASSERT( efl != NULL ) ;
	list = efl->start ;
	ASSERT( list != NULL ) ;

	if ( ef == NULL ) {
		printf("NULLNULL\n");
		return ;
	}


#ifdef DEBUG_MODE
if (0){
	extern void AN_Test_Eye2( FVECTOR *, int );
	AN_Test_Eye2( &ef->pos, 16 );
}
#endif

	if ( list == ef ) {
		efl->start = ef->next ;
		efl->num -- ;
		if( ef->next == NULL ) {
			efl->end = ef ;
		}
		return ;
	}

	while ( (cur = list->next) != NULL ) {
		if ( cur == ef ) {
			list->next = ef->next ;
			efl->num -- ;
			if( efl->end == cur ) {
				efl->end = list ;
			}
			return ;
		}
		list = cur ;
	}
}

/* ---------------------------------------------------------------- */
int GM_EneFindNum( void )
{
	return GM_EneFindList.num ;
}

void	GM_SetEneFind( ENEFIND *ef, FVECTOR *pos, int addr, int type )
{
	ef->pos = *pos ;
#if 1	//00.12.07
	{
		HZX_GROUP_ID group_id ;
		int	z ;

		z = HZX_Zone1( addr ) ;
		group_id = HZX_ZoneGroupID( addr ) ;
		
		if ( HZX_InsideZone( group_id, pos, z ) ) {
			ef->zoneaddr = addr ;
		} else {
			int g, z ;
			HZX_Pos2Zone( pos, &g, &z );
			ef->zoneaddr = HZX_AddressNo( g, z, z ) ;
printf("enefind.c: out of zone[%x] > [%x]\n",addr,ef->zoneaddr );
		}
	}
#else
	{
		HZX_GROUP_ID group_id ;
		int	z ;

		z = HZX_Zone1( addr ) ;
		group_id = HZX_ZoneGroupID( addr ) ;
		
		if ( !HZX_InsideZone( group_id, pos, z ) ) {
printf("enefind.c: out of zone[%x]!!\n",addr );
		}
	}
	ef->zoneaddr = addr ;
#endif

	ef->type = type ;
	ef->next = NULL ;
}

ENEFIND	*GM_GetEneFind( int id )
{
	return GM_GetEneFindFromList( &GM_EneFindList, id ) ;
}

void	GM_PutEneFind( ENEFIND *ef )
{
	GM_PutEneFindToList( &GM_EneFindList, ef ) ;
}

void	GM_FreeEneFind( ENEFIND *ef )
{
	GM_FreeEneFindFromList( &GM_EneFindList, ef ) ;
}

