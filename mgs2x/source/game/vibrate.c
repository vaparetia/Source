//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
   vibrate.c
   パッド振動君
   
   1999/12/20 M.Sonoyama
   $Id: vibrate.c,v 1.3 2002/11/23 11:01:04 Yoshizawa1 Exp $
   */
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#include "BP_Misc.h"

#if defined(BP_VITA)
extern void MENU_Item_Panel_PulseAnim();
#endif

#define TYPE_FILE		0x10
#define	TYPE_SCENARIO	TYPE_FILE
#define TYPE_PROGRAM	0x20

#define WITH_DISPVIB	0x100

#define	HIGH_END		0x04
#define	LOW_END			0x08

#define VIBRATION_1		1
#define VIBRATION_2		2
#define VIBRATION_AP_SENSOR 1024

#define	MAX_VIBRATIONS	(16)

static	int		N_Vibrations ;
static	void	*VibrationActors[ MAX_VIBRATIONS ] ;

typedef	struct	{
	int		value ;
	u_char	*script ;
	u_char	*script_def ;
	float	time ;
} VibSet ;

typedef struct {
    GV_ACT_EX 	actor ;
    u_short 	type ;
	u_short		end_flag ;
	VibSet		vibset[ 2 ] ;
	int			fulltime ;
	float		scale ;
	int			*ctrl ;
	int			local_ctrl ;
	int			time ;
	int			nNo ;
} Work;

extern	int	DM_FrameSkip ;

#if 0
static	void	DumpScript( u_char *script )
{
	while( 1 ) {
		printf( "(%d %d)", script[ 0 ], script[ 1 ] ) ;
		if ( script[ 0 ] == 0 && script[ 1 ] == 0 ) break ;
		script += 2 ;
	}
	printf( "\n" ) ;
}
#endif

static long64 get_next( VibSet *v, int dtime, float scale )
{
    u_char *p ;

    p = v->script ;
    v->value = p[ 0 ] ;
    v->time = ( float )( p[ 1 ] * dtime ) ;
    if( v->value == 0 && p[ 1 ] == 0 ) {
		return 0 ;
    }
    v->script = p + 2 ;
    return 1;
}

static	int	get_fulltime( VibSet *v, int dtime )
{
	u_char *p ;
	int		time ;

    p = v->script ;
	if ( p == NULL ) return 0 ;
	time = 0 ;
	while( p[ 0 ] != 0 || p[ 1 ] != 0 ) {
		time += p[ 1 ] * dtime ;
		p += 2 ;
	}
	return time ;
}

static	void	get_fulltime2( Work *work, int dtime )
{
	int		time1, time2 ;

	time1 = get_fulltime( &work->vibset[ 0 ], dtime ) ;
	time2 = get_fulltime( &work->vibset[ 1 ], dtime ) ;
	if ( time1 > time2 ) work->fulltime = time1 ;
	else				 work->fulltime = time2 ;
}

static	int		Flag( Work *work, int flag )
{
	int			*ctrl ;

	ctrl = work->ctrl ;
	if ( ctrl == NULL ) return 0 ;
	return ( *ctrl & flag ) ;
}

static	inline	void	SetFlag( Work *work, int flag )
{
	if ( work->ctrl != NULL ) *( work->ctrl ) |= flag ;
}

static	inline	void	ResetFlag( Work *work, int flag )
{
	if ( work->ctrl != NULL ) *( work->ctrl ) &= ~flag ;
}

static	inline	void	PutFlag( Work *work, int flag )
{
	if ( work->ctrl != NULL ) *( work->ctrl ) = flag ;
}

static	void	ResetScript( Work *work )
{
	work->vibset[ 0 ].script = work->vibset[ 0 ].script_def ;
	work->vibset[ 1 ].script = work->vibset[ 1 ].script_def ;				
	work->vibset[ 0 ].time = 0.0F ;
	work->vibset[ 1 ].time = 0.0F ;
	work->type &= ~( HIGH_END | LOW_END ) ;
	work->time = 0 ;
}

static void Act( Work *work )
{
	int		force, through ;
    long64 	dtime, ret ;
	float	scale ;
	VibSet	*v ;
	float	skip, dskip ;

	if ( Flag( work, VAR_FLAG_PLAY_STOP ) ) {
		PutFlag( work, VAR_FLAG_IDLE ) ;
		GV_DestroyActor( work ) ;
		return ;
	}

	if( work->type & TYPE_FILE ) {
		dtime = 1 ;
	} else {
		dtime = 2 ;
	}

	scale = work->scale ;
	force = Flag( work, VAR_FLAG_FORCE ) ;
	through = Flag( work, VAR_FLAG_PLAY_SKIP ) ;

	dskip = ( float )( dtime + DM_FrameSkip ) * scale ;

	if ( !( work->type & HIGH_END ) && ( work->type & VIBRATION_1 ) ) {
		v = &work->vibset[ 0 ] ;
		if( v->time <= 0.0F ) {
			skip = 0.0F - v->time ;
			ret = get_next( v, dtime, scale ) ;
			v->time -= skip ;
			if ( ret == 0 ) work->type |= HIGH_END ;
		}
		v->time -= dskip ;
//		GM_PadVibration1 += v->value ;
		if ( !through ) 
      {
			if ( force ) 
            GM_SetForceVibration1( 0, v->value ) ;
			else		 
            GM_SetVibration1( 0, v->value ) ;
#ifdef KP_WINDOWS
			if ( work->type & WITH_DISPVIB ) {
				if ( force ) GM_SetForceDispVibration1( v->value ) ;
				else		 GM_SetDispVibration1( v->value ) ;
			}
#endif
		}
	}
	if ( !( work->type & LOW_END ) && ( work->type & VIBRATION_2 ) ) {
		v = &work->vibset[ 1 ] ;
		if( v->time <= 0.0F ) {
			skip = 0.0F - v->time ;
			ret = get_next( v, dtime, scale ) ;
			v->time -= skip ;
			if ( ret == 0 ) work->type |= LOW_END ;
		}
		v->time -= dskip ;
//		GM_PadVibration2 += v->value ;
		if ( !through ) 
      {
			if ( force ) 
            GM_SetForceVibration2( 0, v->value ) ;
			else		 
            GM_SetVibration2( 0, v->value ) ;
#ifdef KP_WINDOWS
			if ( work->type & WITH_DISPVIB ) {
				if ( force ) GM_SetForceDispVibration2( v->value ) ;
				else		 GM_SetDispVibration2( v->value ) ;
			}
#endif
		}
	}

	if ( Flag( work, VAR_FLAG_RESTART ) ) {
		ResetScript( work ) ;
		ResetFlag( work, VAR_FLAG_RESTART ) ;
		return ;
	}

	if ( ( work->type & ( HIGH_END | LOW_END ) ) == work->end_flag ) {
		if ( Flag( work, VAR_FLAG_LOOP ) ) {
			ResetScript( work ) ;
			return ;
		} else {
			PutFlag( work, VAR_FLAG_IDLE ) ;
		}
		GV_DestroyActor( work );
	}
	work->time ++ ;
}

static	void	Die( Work *work )
{
	VibrationActors[ work->nNo ] = NULL ;
	N_Vibrations -- ;
	//printf( "[%d] vib die\n", GV_Time ) ;
}

void *NewPadVibrationScn( int name, int where )
{
    Work 	*work;
	int		file ;

	/* シナリオ起動はNULLを返さない */
	if ( N_Vibrations >= MAX_VIBRATIONS ) return ( void * )1 ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) );
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		file = GCL_GetOptionValue( 'v', 0 ) ;
		ASSERT( file != 0 ) ;
		work->type = TYPE_SCENARIO | VIBRATION_1 | VIBRATION_2 ;
		//work->vibset[ 0 ].script = ( u_char * )GV_GetCache( GV_CacheID( file, 'v' ) ) ;
		work->vibset[ 0 ].script = GM_FindVibData( file ) ;
		ASSERT( work->vibset[ 0 ].script != NULL ) ;
		work->vibset[ 0 ].time = 0.0F;

		work->vibset[ 1 ].script = work->vibset[ 0 ].script ;
		while( get_next( &work->vibset[ 1 ], 1, 1.0F ) ) ;
		work->vibset[ 1 ].script += 2 ;
		work->vibset[ 1 ].time = 0.0F;

		work->end_flag = HIGH_END | LOW_END ;

		work->vibset[ 0 ].script_def = work->vibset[ 0 ].script ;
		work->vibset[ 1 ].script_def = work->vibset[ 1 ].script ;

		work->scale = 1.0F ;
		work->ctrl = NULL ;

		VibrationActors[ N_Vibrations ] = work ;
		work->nNo = N_Vibrations ;
		N_Vibrations ++ ;
    }
    return ( void * )work;
}

void *NewPadVibration( char *script, int type )
{
    Work *work;

	if ( N_Vibrations >= MAX_VIBRATIONS ) return NULL ;

#if defined(BP_VITA)
   if (type & VIBRATION_AP_SENSOR)
   {
      MENU_Item_Panel_PulseAnim();
   }
   
   return NULL;
#endif

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) );
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->vibset[ ( type & 3 ) - 1 ].script = script;

      work->type = TYPE_PROGRAM | ( type & 3 ) ;

#ifdef KP_WINDOWS
		work->type |= WITH_DISPVIB ;	// 画面振動(代替機能)
#endif
		work->vibset[ ( type & 3 ) - 1 ].time = 0.0F ;
		work->end_flag = ( ( type & ( VIBRATION_1 | VIBRATION_2 ) ) << 2 ) ;

      if ( BP_IsPAL() == TRUE )
		   work->scale = 60.0F / 50.0F ;
      else
		   work->scale = 1.0F ;

      work->ctrl = &work->local_ctrl ;
		work->local_ctrl = VAR_FLAG_PLAYING | ( type & ( VAR_FLAG_LOOP | VAR_FLAG_FORCE ) ) ;
		work->vibset[ 0 ].script_def = work->vibset[ 0 ].script ;
		work->vibset[ 1 ].script_def = work->vibset[ 1 ].script ;

		VibrationActors[ N_Vibrations ] = work ;
		work->nNo = N_Vibrations ;
		N_Vibrations ++ ;
    }
    return ( void * )work;
}

static	void	*NewPadVibration2EX( int file, int playtime, int actorlevel )
{
    Work 	*work;
printf("NewPadVibration2EX\n") ;

	if ( N_Vibrations >= MAX_VIBRATIONS ) return NULL ;

    work = ( Work * )GV_NewActor( actorlevel, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		ASSERT( file != 0 ) ;
		work->type = TYPE_FILE | VIBRATION_1 | VIBRATION_2 ;
		//work->vibset[ 0 ].script = ( u_char * )GV_GetCache( GV_CacheID( file, 'v' ) ) ;
		work->vibset[ 0 ].script = GM_FindVibData( file ) ;

		VibrationActors[ N_Vibrations ] = work ;
		work->nNo = N_Vibrations ;
		N_Vibrations ++ ;

#ifdef DEBUG_MODE
		if ( work->vibset[ 0 ].script == NULL ) {
			printf( "warning : vibfile %d not found\n", file ) ;
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( work->vibset[ 0 ].script == NULL ) {
//			printf( "warning : vibfile %d not found\n", file ) ;
			GV_DestroyActor( work ) ;
			return NULL ;
		}
//		ASSERT( work->vibset[ 0 ].script != NULL ) ;
#endif
		work->vibset[ 0 ].time = 0.0F ;
		work->vibset[ 1 ].script = work->vibset[ 0 ].script ;
		while( get_next( &work->vibset[ 1 ], 1, 1.0F ) ) ;
		work->vibset[ 1 ].script += 2 ;
		work->vibset[ 1 ].time = 0.0F ;
		work->end_flag = HIGH_END | LOW_END ;

		if ( playtime > 0 ) {
			get_fulltime2( work, 1 ) ;
//			work->scale = ( float )playtime / ( float )work->fulltime ;
			work->scale = ( float )work->fulltime / ( float )playtime ;
		} else {

         if ( BP_IsPAL() == TRUE )
      		work->scale = 60.0F / 50.0F ;
         else
      		work->scale = 1.0F ;
		}
		work->vibset[ 0 ].script_def = work->vibset[ 0 ].script ;
		work->vibset[ 1 ].script_def = work->vibset[ 1 ].script ;
		work->ctrl = NULL ;

    }
    return ( void * )work;
}

void	*NewPadVibration2( int file, int playtime )
{
#if defined(BP_VITA)
   // No vibration support for Vita.
   return NULL;
#else
	return NewPadVibration2EX( file, playtime, GV_ACTOR_AFTER ) ;
#endif
}

void	*NewPadVibrationRadio( int file )
{
#if defined(BP_VITA)
	return NULL;
#else
   return NewPadVibration2EX( file, 0, GV_ACTOR_MANAGER ) ;
#endif
}

void	*NewPadVibration3( u_char *script, int *ctrl )
{
    Work 	*work;
	if ( N_Vibrations >= MAX_VIBRATIONS ) return NULL ;

#if defined(BP_VITA)
   // No vibration support for Vita.
   return NULL;
#endif

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->type = TYPE_FILE | VIBRATION_1 | VIBRATION_2 ;
		work->vibset[ 0 ].script = script ;
		work->vibset[ 0 ].time = 0.0F ;
		work->vibset[ 1 ].script = work->vibset[ 0 ].script ;
		while( get_next( &work->vibset[ 1 ], 1, 1.0F ) ) ;
		work->vibset[ 1 ].script += 2 ;
		work->vibset[ 1 ].time = 0.0F ;
		work->end_flag = HIGH_END | LOW_END ;

      if ( BP_IsPAL() == TRUE )
		   work->scale = 60.0F / 50.0F ;
      else
		   work->scale = 1.0F ;

      work->ctrl = ctrl ;
		work->vibset[ 0 ].script_def = work->vibset[ 0 ].script ;
		work->vibset[ 1 ].script_def = work->vibset[ 1 ].script ;
//DumpScript( work->vibset[ 0 ].script ) ;
//DumpScript( work->vibset[ 1 ].script ) ;

		VibrationActors[ N_Vibrations ] = work ;
		work->nNo = N_Vibrations ;
		N_Vibrations ++ ;
    }

    return ( void * )work;
}

/*----------------------------------------------------------------*/

static	int		AliveVibrationAct( void *this )
{
	int			i ;
	
	if ( this == NULL ) return 0 ;
	for ( i = 0; i < MAX_VIBRATIONS; i ++ ) {
		if ( this == VibrationActors[ i ] ) return 1 ;
	}
	return 0 ;
}

/*----------------------------------------------------------------*/

void	GM_SetVibrationScale( void *actor, float scale )
{
	Work		*work ;

	if ( AliveVibrationAct( actor ) == 0 ) {
		printf( "vibrate act is not exist!\n" ) ;
		return ;
	}
	work = ( Work * )actor ;
	work->scale = scale ;
}

/*----------------------------------------------------------------*/

void	GM_StartVibrationSystem( void )
{	
	int		i ;

	N_Vibrations = 0 ;
	for ( i = 0; i < MAX_VIBRATIONS; i ++ ) {
		VibrationActors[ i ] = NULL ;
	}
}
