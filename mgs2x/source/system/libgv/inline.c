/*
	inline.c

	libgvのインライン関数

	libgv.hの中でincludeされる
	
	最適化をかけないときのことを考慮して
	実体を作っておく
*/

#define __GV_INLINE_C__

#ifndef __GV_INLINE__

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#include	"libgv.h"

#undef extern 
#undef inline
#define extern
#define inline

#endif

EXTERN_INLINE void GV_SetActorPriority( void *this, int prio )
{
	( ( GV_ACT * )this )->prio = prio;
}

EXTERN_INLINE void GV_SetActorFreeFunc( void *this, GV_FREEFUNC free )
{
	( ( GV_ACT * )this )->free = free;
}

EXTERN_INLINE void GV_SetActorKillLevel( void *this, int kill_level )
{
	( ( GV_ACT * )this )->kill = kill_level;
}

EXTERN_INLINE void GV_SetActorClass( void *this, int class )
{
	( ( GV_ACT * )this )->class = class;
}

EXTERN_INLINE void GV_InitActor( int level, void *work, GV_FREEFUNC free )
{
	GV_InsertActor( level, work );
	GV_SetActorFreeFunc( work, free );
}

EXTERN_INLINE void GV_PauseOnActorSystem( int pause )
{
	GV_PauseLevel |= pause;
}

EXTERN_INLINE void GV_PauseOffActorSystem( int pause )
{
	GV_PauseLevel &= ~pause;
}

EXTERN_INLINE void GV_PauseToggleActorSystem( int pause )
{
	GV_PauseLevel ^= pause;
}

EXTERN_INLINE void GV_SetActorSignalFunc( void *this
								 , int (*func)( void *work, int signal, int value ) )
{
	GV_ACT_EX *p;

	p = (GV_ACT_EX*)this;
	ASSERT( p->actor.class & GV_CLASS_EX );
	p->sigfunc = func;
}

EXTERN_INLINE void GV_SleepActor( void *this, int waitflag )
{
	GV_ACT *act = ( GV_ACT * )this;
	act->class |= waitflag;
}

EXTERN_INLINE void GV_WakeupActor( void *this, int waitflag )
{
	GV_ACT *act = ( GV_ACT * )this;
	act->class &= ~waitflag;
}

EXTERN_INLINE void GV_WaitMessage( void *this, int name )
{
	GV_ACT *act = ( GV_ACT * )this;

	if( GV_PauseLevel == 0 ){
		act->name = name;
		act->class |= GV_CLASS_WAIT_MESG;
	}
}

EXTERN_INLINE void GV_SetActorMessageKill( void *this, int name )
{
	GV_ACT *act = ( GV_ACT * )this;
	act->name = name;
	act->class |= GV_CLASS_MESSAGE_KILL;
}

EXTERN_INLINE int GV_IsFollowDestroy( void *this )
{
	return ( ( ( GV_ACT * )this )->class & ( GV_CLASS_FOLLOW | GV_CLASS_KILLALL ) );
}

EXTERN_INLINE int GV_IsStageDestroy( void *this ) 
{
	return ( ( ( GV_ACT * )this )->class & GV_CLASS_KILLALL ) ;
}

EXTERN_INLINE int GV_CallSignalFunc( void *work, int signal, int value )
{
	GV_ACT_EX *p = (GV_ACT_EX*)work;

	return ( *p->sigfunc )( p, signal, value );
}

EXTERN_INLINE void GV_ChangeActFunc( void *this, void *func )
{
	( ( GV_ACT * )this )->act = ( void (*)(GV_ACT*) )func;
}
