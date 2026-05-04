//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   no_use.c
   武器・アイテムの装備不可定義

   2001/04/17	M.Sonoyama
   $Id: no_use.c,v 1.1.1.3 2002/11/19 11:41:54 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

int		*GM_NoUseWeaponsFromWeapon[] = {
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL
} ;

static	int		Psg1NoUseI[] = 		{ 9, IT_Scope, IT_Camera, IT_TnkCamera,
									  IT_CBBox, IT_CBBoxB, IT_CBBoxC, IT_CBBoxD, IT_CBBoxE,
									  IT_CBBoxWet } ;
#define	Psg1TNoUseI			Psg1NoUseI
#define	StingerNoUseI		Psg1NoUseI
#define	ColdSprayNoUseI		Psg1NoUseI
#define	MicNoUseI			Psg1NoUseI
#define	NikitaNoUseI		Psg1NoUseI

int		*GM_NoUseItemsFromWeapon[] = {
	NULL, NULL, NULL, NULL, Psg1NoUseI, 
	NULL, NikitaNoUseI, StingerNoUseI, NULL, NULL, 
	NULL, NULL, MicNoUseI, NULL, ColdSprayNoUseI,
	NULL, NULL, NULL, NULL, Psg1TNoUseI, 
	MicNoUseI, NULL
} ;

static	int		ScopeNoUseW[] = 	{ 7, WP_Psg1, WP_Psg1T, WP_Stinger, 
										 WP_ColdSpray, WP_Mic, WP_Nikita, WP_DemoMic } ;
#define	CameraNoUseW		ScopeNoUseW
#define	CBBoxNoUseW			ScopeNoUseW

int		*GM_NoUseWeaponsFromItem[] = {
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, ScopeNoUseW, 
	CameraNoUseW, CBBoxNoUseW, NULL, NULL, NULL, 
	NULL, CameraNoUseW, CBBoxNoUseW, CBBoxNoUseW, CBBoxNoUseW, 
	NULL, CBBoxNoUseW, CBBoxNoUseW, NULL, NULL, 
	NULL, NULL, NULL, NULL,	NULL,
	NULL, NULL, NULL, NULL,	NULL,
	NULL,
} ;

int		*GM_NoUseItemsFromItem[] = {
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,	NULL,
	NULL,
} ;

/*----------------------------------------------------------------*/

static	int		Exist( int *list, int this )
{
	int			i ;

	if ( list == NULL ) return 0 ;
	for ( i = 1; i < list[ 0 ] + 1; i ++ ) {
		if ( list[ i ] == this ) return 1 ;
	}
	return 0 ;
}

int		GM_CheckNoUseWeaponFromWeapon( int from, int to )
{
	return Exist( GM_NoUseWeaponsFromWeapon[ from ], to ) ;
}

int		GM_CheckNoUseWeaponFromItem( int from, int to )
{
	return Exist( GM_NoUseWeaponsFromItem[ from ], to ) ;
}

int		GM_CheckNoUseItemFromWeapon( int from, int to )
{
	return Exist( GM_NoUseItemsFromWeapon[ from ], to ) ;
}

int		GM_CheckNoUseItemFromItem( int from, int to )
{
	return Exist( GM_NoUseItemsFromItem[ from ], to ) ;
}
