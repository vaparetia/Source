//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	interupt.c
	ゾーン遮断管理
	
	2000/07/05 Y.Korekado
	$Id: interupt.c,v 1.1.1.3 2002/11/19 11:41:51 Yoshizawa1 Exp $			   
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

/* ---------------------------------------------------------------- */
typedef struct {
	R_INTRPT	*r_start ;
	R_INTRPT	*r_end ;
	int		r_num ;
	int		r_uniq_id ;
	
	Z_INTRPT	*z_start ;
	Z_INTRPT	*z_end ;
	int		z_num ;
	int		z_uniq_id ;
} INTRPTSYS	;

static	INTRPTSYS	IntrptSys ;
/* ---------------------------------------------------------------- */
void	GM_InitInterruptSystem( void )
{
	IntrptSys.r_start = NULL ;
	IntrptSys.r_end = NULL ;
	IntrptSys.r_num = 0 ;
	IntrptSys.r_uniq_id = 0 ;

	IntrptSys.z_start = NULL ;
	IntrptSys.z_end = NULL ;
	IntrptSys.z_num = 0 ;
	IntrptSys.z_uniq_id = 0 ;
}

/* ---------------------------------------------------------------- */
R_INTRPT	*GM_GetRIntrptCheckStatus( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr, int status )
{
	R_INTRPT	*res ;
	HZX_ZONE_ADD	next_addr, tmp_addr ;
	int mapno, z ;

	mapno = HZX_ZoneMapNo( from_addr ) ;
	z = HZX_Zone1( from_addr ) ;
	from_addr = HZX_AddressNo( mapno, z, z ) ;

	mapno = HZX_ZoneMapNo( to_addr ) ;
	z = HZX_Zone1( to_addr ) ;
	to_addr = HZX_AddressNo( mapno, z, z ) ;

//printf("start from_addr=0x%x to_addr=0x%x\n",from_addr,to_addr);
	tmp_addr = -1 ;
	while ( from_addr != to_addr ) {
		next_addr = HZX_NextZoneCrossGroup( from_addr, to_addr ) ;

		if ( HZX_Zone1( next_addr ) == 255 || tmp_addr == next_addr) {
			printf(" GM_GetRIntrptZ2Z Err !!!!!\n");
			return NULL ;
		}
		tmp_addr = from_addr ;

//printf("intrpt:from_addr=0x%x next_addr=0x%x to_addr=0x%x\n",from_addr,next_addr,to_addr);
		if ( (res=GM_GetRIntrpt( next_addr, from_addr ) ) != NULL ) {
			if ( res->status & status ) return res ; 
		}
		from_addr = next_addr ;
	}
	
	return NULL ;
}

#if 1



R_INTRPT	*GM_GetRIntrptZ2Z( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr )
{
	R_INTRPT	*res ;
	HZX_ZONE_ADD	next_addr, tmp_addr ;
	int mapno, z ;

	mapno = HZX_ZoneMapNo( from_addr ) ;
	z = HZX_Zone1( from_addr ) ;
	from_addr = HZX_AddressNo( mapno, z, z ) ;

	mapno = HZX_ZoneMapNo( to_addr ) ;
	z = HZX_Zone1( to_addr ) ;
	to_addr = HZX_AddressNo( mapno, z, z ) ;

//printf("start from_addr=0x%x to_addr=0x%x\n",from_addr,to_addr);
	tmp_addr = -1 ;
	while ( from_addr != to_addr ) {
		next_addr = HZX_NextZoneCrossGroup( from_addr, to_addr ) ;

		if ( HZX_Zone1( next_addr ) == 255 || tmp_addr == next_addr) {
			printf(" GM_GetRIntrptZ2Z Err !!!!!\n");
			return NULL ;
		}
		tmp_addr = from_addr ;

//printf("intrpt:from_addr=0x%x next_addr=0x%x to_addr=0x%x\n",from_addr,next_addr,to_addr);
		if ( (res=GM_GetRIntrpt( next_addr, from_addr ) ) != NULL ) return res ; 
		from_addr = next_addr ;
	}
	
	return NULL ;
}
#else
R_INTRPT	*GM_GetRIntrptZ2Z( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr )
{
	R_INTRPT	*res ;
	int next, from, to, tmp ;
	HZX_GROUP_ID id ;
	HZX_ZONE_ADD	addr1, addr2 ;

	id = GM_GetBit( HZX_ZoneMapNo( from_addr ) ) ;

	from = HZX_Zone1( from_addr ) ;
	to = HZX_Zone1( to_addr ) ;

//printf("start from=0x%x to=0x%x\n",from,to);
	tmp = -1 ;
	while ( from != to ) {
		next = HZX_NextZone( id, from, to ) ;
		if ( next==tmp ) {
			printf(" GM_GetRIntrptZ2Z Err !!!!!\n");
			return NULL ;
		}
		tmp = from ;
//printf("next=0x%x from=0x%x to=0x%x\n",next,from,to);
		addr1 = HZX_Address( id, from, from ) ;
		addr2 = HZX_Address( id, next, next ) ;
		if ( (res=GM_GetRIntrpt( addr1, addr2 ) ) != NULL ) return res ; 
		from = next ;
	}
	
	return NULL ;
}
#endif

R_INTRPT	*GM_GetRIntrptOne( HZX_ZONE_ADD from )
{
	INTRPTSYS	*itrptsys ;
	R_INTRPT	*cur ;

	itrptsys = &IntrptSys ;
	cur = itrptsys->r_start ;

	while( cur != NULL ) {
		if( cur->zone1 == from || cur->zone2 == from ) return cur ;
		cur = cur->next ;
	}
	
	return NULL ;
}

R_INTRPT	*GM_GetRIntrpt( HZX_ZONE_ADD from, HZX_ZONE_ADD to )
{
	INTRPTSYS	*itrptsys ;
	R_INTRPT	*cur ;

	itrptsys = &IntrptSys ;
	cur = itrptsys->r_start ;

	while( cur != NULL ) {
		if( cur->zone1 == from && cur->zone2 == to ) return cur ;
		if( cur->zone2 == from && cur->zone1 == to ) return cur ;
		cur = cur->next ;
	}
	
	return NULL ;
}

void	GM_SetRouteIntrpt( R_INTRPT *r_intrpt, HZX_ZONE_ADD zone1, HZX_ZONE_ADD zone2,
							FVECTOR *pos, int dir, int status, int kind )
{
	r_intrpt->zone1 = zone1 ;
	r_intrpt->zone2 = zone2 ;

	r_intrpt->pos = *pos ;
	r_intrpt->dir = dir ;
	r_intrpt->status = status ;
	r_intrpt->kind = kind ;
	r_intrpt->id = 0 ;
	r_intrpt->callback = NULL ;
	r_intrpt->work = NULL ;
}

void	GM_SetRIntrptCallBack( void *ptr, R_INTRPT *r_intrpt, RINTRPT_CALLBACK func )
{
	r_intrpt->callback = func ;
	r_intrpt->work = ptr ;
}

void	GM_PutRoteIntrpt( R_INTRPT *r_intrpt )
{
	INTRPTSYS	*itrptsys ;

	itrptsys = &IntrptSys ;

	if ( itrptsys->r_start == NULL ) {
		itrptsys->r_start = r_intrpt ;
	} else {
		itrptsys->r_end->next = r_intrpt ;
	}
	r_intrpt->next = NULL ;
	r_intrpt->id = itrptsys->r_uniq_id ;
	itrptsys->r_end = r_intrpt ;
	itrptsys->r_num ++ ;
	itrptsys->r_uniq_id ++ ;

	/* ゾーンのnear_flag にフラグを立てる */
	HZX_SetNearFlag( r_intrpt->zone1, r_intrpt->zone2, HZX_ROOT_RINTRPT ) ;

	ASSERT( itrptsys->r_uniq_id < 10000 ) ;
printf(" Put Route Interrupt num[%d] ",itrptsys->r_num);
printf("zone0[%x] zone1[%x]\n",r_intrpt->zone1,r_intrpt->zone2 ) ;
}

void	GM_FreeRouteIntrpt( R_INTRPT *r_intrpt )
{
	INTRPTSYS	*itrptsys ;
	R_INTRPT	*list, *cur ;

	ASSERT( r_intrpt != NULL ) ;

	HZX_UnsetNearFlag( r_intrpt->zone1, r_intrpt->zone2, HZX_ROOT_RINTRPT ) ;

	itrptsys = &IntrptSys ;
	list = itrptsys->r_start ;

	if ( list == r_intrpt ) {
		itrptsys->r_start = r_intrpt->next ;
		itrptsys->r_num -- ;
		if( r_intrpt->next == NULL ) {
			itrptsys->r_end = NULL ;
		}
		return ;
	}

	while ( (cur = list->next) != NULL ) {
		if ( cur == r_intrpt ) {
			list->next = r_intrpt->next ;
			if( r_intrpt->next == NULL ) {
				itrptsys->r_end = list ;
			}
			return ;
		}
		list = cur ;
	}
}

int	GM_RIntrptCallBack( R_INTRPT *r_intrpt, int mot_name, int mot_num, int order )
{
    int	( *func )( int, int, int, void * ) ;

	func = r_intrpt->callback ;
	if ( func != NULL ) {
		return ( *func )( mot_name, mot_num, order, r_intrpt->work ) ;
	}
	
	return -1 ;
}
/* ---------------------------------------------------------------- */
void	GM_SetZoneIntrpt( Z_INTRPT *z_intrpt, CONTROL *ctrl, HZX_ZONE_ADD zone, int status, int kind )
{
	z_intrpt->ctrl = ctrl ;
	z_intrpt->zone = zone ;

	z_intrpt->status = status ;
	z_intrpt->kind = kind ;
	z_intrpt->id = 0 ;
}

void	GM_PutZoneIntrpt( Z_INTRPT *z_intrpt )
{
	INTRPTSYS	*itrptsys ;

	itrptsys = &IntrptSys ;

	if ( itrptsys->z_start == NULL ) {
		itrptsys->z_start = z_intrpt ;
	} else {
		itrptsys->z_end->next = z_intrpt ;
	}
	z_intrpt->next = NULL ;
	z_intrpt->id = itrptsys->z_uniq_id ;
	itrptsys->z_end = z_intrpt ;
	itrptsys->z_num ++ ;
	itrptsys->z_uniq_id ++ ;
	ASSERT( itrptsys->z_uniq_id < 10000 ) ;
//printf(" Put Route Interrupt num[%d] \n",itrptsys->r_num);
}

void	GM_FreeZoneIntrpt( Z_INTRPT *z_intrpt )
{
	INTRPTSYS	*itrptsys ;
	Z_INTRPT	*list, *cur ;

	ASSERT( z_intrpt != NULL ) ;

	itrptsys = &IntrptSys ;
	list = itrptsys->z_start ;

	if ( list == z_intrpt ) {
		itrptsys->z_start = z_intrpt->next ;
		itrptsys->z_num -- ;
		if( z_intrpt->next == NULL ) {
			itrptsys->z_end = NULL ;
		}
		return ;
	}

	while ( (cur = list->next) != NULL ) {
		if ( cur == z_intrpt ) {
			list->next = z_intrpt->next ;
			if( z_intrpt->next == NULL ) {
				itrptsys->z_end = list ;
			}
			return ;
		}
		list = cur ;
	}
}


/* ゾーンインタラプトリストの先頭アドレスを返す */
Z_INTRPT *GM_GetZoneIntrptList( void )
{
	return IntrptSys.z_start ;
}

/* z_intrpt以降で最初に指定された、ゾーンアドレス、ステータス、種類のゾーンインタラプトを返す */
Z_INTRPT *GM_SearchZoneIntrpt( Z_INTRPT *z_intrpt, HZX_ZONE_ADD zone, int status, int kind )
{
	Z_INTRPT	*cur ;

	cur = z_intrpt ;
	while( cur != NULL ) {
		if( !kind || cur->kind & kind ) {
			if( !status || cur->status & status ) {
				if( cur->ctrl != NULL ) {
					if( cur->ctrl->addr == zone ) return cur ;
				} else {
					if( cur->zone == zone ) return cur ;
				}
			}
		}
		cur = cur->next ;
	}
	
	return NULL ;
}

Z_INTRPT *GM_ZoneIntrpt( HZX_ZONE_ADD zone, int status, int kind )
{
	return GM_SearchZoneIntrpt( IntrptSys.z_start, zone, status, kind ) ;
}

int	GM_ZoneIntrptCheck( HZX_ZONE_ADD this )
{
	HZX_ZON	*z ;

	z = HZX_GetZoneFromAdd( this ) ;
	if ( z->flag & HZX_ZONE_ZINTRPT ) return 1;

	return 0 ;
}

int	GM_GetZIntrptZ2Z( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr )
{
	R_INTRPT	*res ;
	HZX_ZONE_ADD	next_addr, tmp_addr ;
	int mapno, z ;

//printf("GM_GetZIntrptZ2Z: from_addr=0x%x to_addr=0x%x\n",from_addr,to_addr);

	mapno = HZX_ZoneMapNo( from_addr ) ;
	z = HZX_Zone1( from_addr ) ;
	from_addr = HZX_AddressNo( mapno, z, z ) ;

	mapno = HZX_ZoneMapNo( to_addr ) ;
	z = HZX_Zone1( to_addr ) ;
	to_addr = HZX_AddressNo( mapno, z, z ) ;

//printf("start from_addr=0x%x to_addr=0x%x\n",from_addr,to_addr);
	tmp_addr = -1 ;
	while ( from_addr != to_addr ) {
//printf("intrpt:HZX_NextZoneCrossGroup from_addr=0x%x to_addr=0x%x\n",from_addr,to_addr);
		next_addr = HZX_NextZoneCrossGroup( from_addr, to_addr ) ;

		if ( HZX_Zone1( next_addr ) == 255 || tmp_addr == next_addr) {
			printf(" GM_GetZIntrptZ2Z Err from[%x] to[%x] next[%x] tmp[%x]!!!!!\n",
				from_addr, to_addr, tmp_addr, next_addr ) ;
			return NULL ;
		}
		tmp_addr = from_addr ;

//printf("intrpt:from_addr=0x%x next_addr=0x%x to_addr=0x%x\n",from_addr,next_addr,to_addr);
		if ( GM_ZoneIntrptCheck( next_addr ) != 0 ) return next_addr ; 
		from_addr = next_addr ;
	}

	return -1 ;
}

/*日本版緊急対処*/
int	GM_GetZIntrptZ2Z2( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr )
{
	R_INTRPT	*res ;
	HZX_ZONE_ADD	next_addr, tmp_addr ;
	int mapno, z ;

//printf("GM_GetZIntrptZ2Z: from_addr=0x%x to_addr=0x%x\n",from_addr,to_addr);

	mapno = HZX_ZoneMapNo( from_addr ) ;
	z = HZX_Zone1( from_addr ) ;
	from_addr = HZX_AddressNo( mapno, z, z ) ;

	mapno = HZX_ZoneMapNo( to_addr ) ;
	z = HZX_Zone1( to_addr ) ;
	to_addr = HZX_AddressNo( mapno, z, z ) ;

//printf("start from_addr=0x%x to_addr=0x%x\n",from_addr,to_addr);
	tmp_addr = -1 ;
	while ( from_addr != to_addr ) {
//printf("intrpt:HZX_NextZoneCrossGroup from_addr=0x%x to_addr=0x%x\n",from_addr,to_addr);
		next_addr = HZX_NextZoneCrossGroup2( from_addr, to_addr ) ;

		if ( HZX_Zone1( next_addr ) == 255 || tmp_addr == next_addr) {
			printf(" GM_GetZIntrptZ2Z Err from[%x] to[%x] next[%x] tmp[%x]!!!!!\n",
				from_addr, to_addr, tmp_addr, next_addr ) ;
			return NULL ;
		}
		tmp_addr = from_addr ;

//printf("intrpt:from_addr=0x%x next_addr=0x%x to_addr=0x%x\n",from_addr,next_addr,to_addr);
		if ( GM_ZoneIntrptCheck( next_addr ) != 0 ) return next_addr ; 
		from_addr = next_addr ;
	}

	return -1 ;
}
/* ---------------------------------------------------------------- */

/* 指定ＣＯＮＴＲＯＬ以外のＺｉｎｔｒｐｔが指定ゾーンにいるかをチェック */
/* ロッカー死体入れでしか使っていません */
Z_INTRPT	*GM_SearchZIntrptExceptThis( HZX_ZONE_ADD zone, int status, int kind, CONTROL *ctrl )
{
	Z_INTRPT	*cur ;

	cur = IntrptSys.z_start ;
	while( cur != NULL ) {
		if( !kind || cur->kind & kind ) {
			if( !status || cur->status & status ) {
				if( cur->ctrl != NULL ) {
					if( cur->ctrl->addr == zone && 
					    cur->ctrl != ctrl ) return cur ;
				} 
			}
		}
		cur = cur->next ;
	}
	return NULL ;
}

/* ---------------------------------------------------------------- */

int	GM_UnSetZoneFlagForScn( )
{
	extern void	HZX_Pos2Zone( FVECTOR *, int *, int * ) ;
	int 	g, z ;
	int 	flag, zoneadd ;
	FVECTOR	pos ;
	HZX_ZON	*zone ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	flag = GCL_GetNextInt() ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	pos.vx = (float)GCL_GetNextInt() ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	pos.vy = (float)GCL_GetNextInt() ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	pos.vz = (float)GCL_GetNextInt() ;

	HZX_Pos2Zone( &pos, &g, &z );
	zoneadd = HZX_AddressNo( g, z, z ) ;

	zone = HZX_GetZoneFromAdd( zoneadd ) ;
	zone->flag &= ~flag ;

	return 0 ;
}

int	GM_SetZoneFlagForScn( )
{
	extern void	HZX_Pos2Zone( FVECTOR *, int *, int * ) ;
	int 	g, z ;
	int 	flag, zoneadd ;
	FVECTOR	pos ;
	HZX_ZON	*zone ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	flag = GCL_GetNextInt() ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	pos.vx = (float)GCL_GetNextInt() ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	pos.vy = (float)GCL_GetNextInt() ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	pos.vz = (float)GCL_GetNextInt() ;

	HZX_Pos2Zone( &pos, &g, &z );
	zoneadd = HZX_AddressNo( g, z, z ) ;

	zone = HZX_GetZoneFromAdd( zoneadd ) ;
	zone->flag |= flag ;

	return 0 ;
}

/* ---------------------------------------------------------------- */


