//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	randamizer.c
	ランダムテーブル
	2000/12/26 S.Okajima
	$Id: randamizer.c,v 1.1.1.3 2002/11/19 11:47:42 Yoshizawa1 Exp $

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
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

#define	RANDAM_FIELD_NUM	(0x4000 / 4)
#define	MASK				(0xfffffffc)

//-------------------------------
ALIGN16_PRE int		ALIGN16_POST	OK_RandamField_irnd[RANDAM_FIELD_NUM];
ALIGN16_PRE float	ALIGN16_POST	OK_RandamField_rnd[RANDAM_FIELD_NUM];
ALIGN16_PRE float	ALIGN16_POST	OK_RandamField_frnd[RANDAM_FIELD_NUM * 2];
//-------------------------------

static int InitializeFlag_irnd;
static int InitializeFlag_rnd;
static int InitializeFlag_frnd;

static void OK_Randamize_irnd( void )
{
	int	i;
	int	*p_randam;

	p_randam = SCRPAD_ADDR;
	i = RANDAM_FIELD_NUM;
	while( --i >= 0 ){
		(*p_randam) = irnd();
		p_randam++;
	}
	OK_Scr_Mem( OK_RandamField_irnd, SCRPAD_ADDR, sizeof(int), RANDAM_FIELD_NUM );
}
static void OK_Randamize_rnd( void )
{
	int	i;
	float	*p_randam;

	p_randam = SCRPAD_ADDR;
	i = RANDAM_FIELD_NUM;
	while( --i >= 0 ){
		(*p_randam) = rnd();
		p_randam++;
	}
	OK_Scr_Mem( OK_RandamField_rnd, SCRPAD_ADDR, sizeof(float), RANDAM_FIELD_NUM );
}
static void OK_Randamize_frnd( void )
{
	int	i;
	float	*p_randam;

	p_randam = SCRPAD_ADDR;
	i = RANDAM_FIELD_NUM;
	while( --i >= 0 ){
		(*p_randam) = frnd();
		p_randam++;
	}
	OK_Scr_Mem( OK_RandamField_frnd, SCRPAD_ADDR, sizeof(float), RANDAM_FIELD_NUM );

   // AS(JM) - Copy it twice so that we can always just return a pointer
   OK_Scr_Mem( &( OK_RandamField_frnd[ RANDAM_FIELD_NUM ] ), SCRPAD_ADDR, sizeof(float), RANDAM_FIELD_NUM );
}
//-------------------------------
void OK_irnd_to_scr( void *dst, int num )
{
	int	i;
	int		*p_randam;

	if( !InitializeFlag_irnd ){
		InitializeFlag_irnd = 1;
		OK_Randamize_irnd();
	}

	if( num <=0 ) return;

	i = ((irnd()>>8)%RANDAM_FIELD_NUM) & MASK;
	p_randam = (int *)dst;
	if( RANDAM_FIELD_NUM - i >= num ){
		OK_Mem_Scr(  p_randam,                     &OK_RandamField_irnd[i], sizeof(int), num );
	}else{
		OK_Mem_Scr(  p_randam,                     &OK_RandamField_irnd[i], sizeof(int), RANDAM_FIELD_NUM - i );
		OK_Mem_Scr( &p_randam[RANDAM_FIELD_NUM-i],  OK_RandamField_irnd,    sizeof(int), i+num - RANDAM_FIELD_NUM );
	}
}
void OK_rnd_to_scr( void *dst, int num )
{
	int	i;
	float	*p_randam;

	if( !InitializeFlag_rnd ){
		InitializeFlag_rnd = 1;
		OK_Randamize_rnd();
	}

	if( num <=0 ) return;

	i = ((irnd()>>8)%RANDAM_FIELD_NUM) & MASK;
	p_randam = (float *)dst;
	if( RANDAM_FIELD_NUM - i >= num ){
		OK_Mem_Scr(  p_randam,                     &OK_RandamField_rnd[i], sizeof(float), num );
	}else{
		OK_Mem_Scr(  p_randam,                     &OK_RandamField_rnd[i], sizeof(float), RANDAM_FIELD_NUM - i );
		OK_Mem_Scr( &p_randam[RANDAM_FIELD_NUM-i],  OK_RandamField_rnd,    sizeof(float), i+num - RANDAM_FIELD_NUM );
	}
}
void OK_frnd_to_scr( void *dst, int num )
{
	int	i;
	float	*p_randam;

	if( !InitializeFlag_frnd ){
		InitializeFlag_frnd = 1;
		OK_Randamize_frnd();
	}

	if( num <=0 ) return;

	i = ((irnd()>>8)%RANDAM_FIELD_NUM) & MASK;
	p_randam = (float *)dst;
	if( RANDAM_FIELD_NUM - i >= num ){
		OK_Mem_Scr(  p_randam,                     &OK_RandamField_frnd[i], sizeof(float), num );
	}else{
		OK_Mem_Scr(  p_randam,                     &OK_RandamField_frnd[i], sizeof(float), RANDAM_FIELD_NUM - i );
		OK_Mem_Scr( &p_randam[RANDAM_FIELD_NUM-i],  OK_RandamField_frnd,    sizeof(float), i+num - RANDAM_FIELD_NUM );
	}
}

float const *OK_frnd_to_ptr( int num )
{
   int	i;
   float	*p_randam;

   if( !InitializeFlag_frnd ){
      InitializeFlag_frnd = 1;
      OK_Randamize_frnd();
   }

   if( num <=0 ) return OK_RandamField_frnd;

   i = ((irnd()>>8)%RANDAM_FIELD_NUM) & MASK;

   return &OK_RandamField_frnd[i];
}