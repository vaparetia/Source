//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	プロセス管理ルーチン群

	1999/03/23 K.Uehara
	$Id: actor.c,v 1.9 2002/11/29 15:12:04 takaki Exp $
*/

#ifdef PSX2
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#endif // PSX2
#ifdef KP_XBOX
#include	<xtl.h>
#endif

//#include	"mts.h"

#include	"libgv.h"
#include	"libgv.cnf"

#include "BP_Debug.h"

#ifndef GOLD_VERSION
//#define ADD_ACTOR_CPU_MARKERS 1
#endif

#if 1 //BP_PS2
#define excep_printf printf
#else
int excep_printf( const char *format, ... );
#endif

/*----------------------------------------------------------------*/

	/*
		現在の PAUSE レベル
	*/
int		GV_PauseLevel ;

/*----------------------------------------------------------------*/

	/*
		各優先順位のリンクリスト
	*/
typedef	struct	{
	GV_ACT		start ;		/* 最初のプロセス	*/
	GV_ACT		end ;		/* 最後のプロセス	*/
	int 		pause_level ;	/* PAUSE レベル		*/
} AList ;


static	AList	ActorList[ GV_ACTOR_LEVEL_MAX ] ;

static struct {
	short pause;
} levels[ GV_ACTOR_LEVEL_MAX ] = {
// GV_ACTOR_DAEMON		/* システムデーモン	*/
	{ GV_LEVEL_NOSTOP },

// GV_ACTOR_MANAGER,		/* ゲーム進行プロセス	*/
	{ GV_LEVEL_NOSTOP },

// GV_ACTOR_ASSIST,			/* ゲーム補助プロセス	*/
	{ GV_LEVEL_STOP },

//	GV_ACTOR_PREV2,			/* メインより前に実行されるキャラクタ（ポーズ時も動作）*/
	{ GV_LEVEL_STOP },
//	GV_ACTOR_PREV,			/* メインより前に実行されるキャラクタ */
	{ GV_LEVEL_NORMAL },

//	GV_ACTOR_PLAYER,		/* 通常キャラクタ (プレイヤー) */
	{ GV_LEVEL_NORMAL },
//	GV_ACTOR_USER,			/* 通常キャラクタ */
	{ GV_LEVEL_NORMAL },
//	GV_ACTOR_CAMERA,		/* カメラ(ポーズ時も動作) */
	{ GV_LEVEL_STOP & (~GV_PAUSE_DEBUG ) },

//	GV_ACTOR_AFTER,			/* メインより後に実行されるキャラクタ */
	{ GV_LEVEL_NORMAL },
//	GV_ACTOR_EFFECT,		/* エフェクト */
	{ GV_LEVEL_NORMAL },
//	GV_ACTOR_AFTER2,		/* メインより後に実行されるキャラクタ（ポーズ時も動作）*/
	{ GV_LEVEL_STOP & (~GV_PAUSE_DEBUG) },

// GV_ACTOR_DAEMON2,		/* システムデーモン２	*/
	{ GV_LEVEL_NOSTOP }
};

/*----------------------------------------------------------------*/

void GV_InitActorSystem( void )
{
	AList		*list ;
	int		i ;

	list = ActorList ;
	for ( i = 0; i < GV_ACTOR_LEVEL_MAX; i++ ){
		GV_ACT		*start, *end ;

		start = &( list->start ) ;
		end = &( list->end ) ;
		start->prev = NULL ;
		start->next = end ;
		end->prev = start ;
		end->next = NULL ;

		start->act = start->die = NULL ;
		end->act = end->die = NULL ;

		start->class = 0;
		end->class = 0;

		start->prio = 0;
		end->prio = GV_PRIO_MAX;

		list->pause_level = levels[ i ].pause ;
		list ++ ;
	}
	GV_PauseLevel = 0 ;
}

/*----------------------------------------------------------------*/

#ifdef DEBUG
#ifdef PSX2
#include <eekernel.h>
#endif	// PSX2

static void DumpChild( GV_ACT *this, int level )
{
	if( this->class & GV_CLASS_PARENT ){
		GV_ACT_EX *cp;
		cp = ( ( GV_ACT_EX * )this )->child;
		for( ; cp != NULL; cp = cp->c_next ){
			printf( "%*s%08X[%08X:%08X] %s\n", level * 2 + 1, ""
					, cp, cp->c_prev, cp->c_next
					, cp->actor.filename );
			if( cp->actor.class & GV_CLASS_PARENT ){
				DumpChild( (GV_ACT * )cp, level + 1 );
			}
		}
	}
}

void GV_DumpActorSystem( int mode )
{
	AList		*list ;
	int		i, pause, sum, num;

	pause = GV_PauseLevel ;
	list = ActorList ;
	printf( "\n--DumpActorSystem--\n" );
	sum = num = 0;
	for ( i = 0; i < GV_ACTOR_LEVEL_MAX; i++ ){
		printf( "Lv %d Pause %X\n", i, list->pause_level );
		{
			GV_ACT		*this, *next ;
			this = &( list->start ) ;
			do {
				next = this->next ;
				if( this->act != NULL ){
					int c;
					if( this->count > 0 ){
						c = this->runtime * 100 / this->count;
					} else {
						c = 0;
					}
					this->runtime = this->count = 0;
					if( mode ){
						if( c < 100 ){
							sum += c;
							num ++;
							continue;
						}
					}
					printf( "%c%c%04d.%02d:P%04X:K%02X:%c%08X %08X %s"
							, ( this->class & GV_CLASS_WAITING ) ? 'W':' '
							, ( this->class & GV_CLASS_MESSAGE_KILL ) ? 'k':' '
							, c / 100, c % 100
							, this->prio
							, this->kill
							, ( this->class & GV_CLASS_EX ) ? 'E':' '
							, this
							, this->act
							, this->filename );

					if( this->class & ( GV_CLASS_WAITING | GV_CLASS_MESSAGE_KILL ) ){
						printf( " n%08X", this->name );
					}

					printf( "\n" );

					DumpChild( this, 1 );

					WAIT_HSYNC( 10 ) ;
				}
			} while ( ( this = next ) != NULL ) ;
		}
		list ++ ;
	}
	if( sum > 0 ){
		printf( "other %d tasks total %04d.%02d\n", num, sum / 100, sum % 100 );
	}
}

#endif

void GV_ExecActorSystem( void )
{
	AList		*list ;
	int		i, pause ;

	list = ActorList ;

	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {
		void		( *act )( GV_ACT * ) ;
		GV_ACT		*this, *next ;

		/*
			現在の PAUSE レベル未満のプロセスは停止
		*/
		pause = GV_PauseLevel ;

      //BP_PAUSE - make debug pause act like regular pause otherwise cinemas don't pause correctly
      if( (GV_PauseLevel & GV_PAUSE_DEBUG) && (list != &ActorList[GV_ACTOR_CAMERA]) )  // NOTE: Update cameras so tweak system menu applies real time
      {
         pause |= GV_PAUSE_STOP;
      }
      //BP_PAUSE - make debug pause act like regular pause otherwise cinemas don't pause correctly

		if ( ( list->pause_level & pause ) == 0 ) {
			this = list->start.next;
			do {
				next = this->next ;
				if( this->class & GV_CLASS_WAITING ) continue;
				if ( ( act = this->act ) != NULL ) {
#ifdef DEBUG_MODE
					int intime, outtime;
					intime = GET_HSYNC_TIMER();
#endif
#ifdef DEBUG
               MARK( this->filename );
#endif
#if defined(ADD_ACTOR_CPU_MARKERS)
               BP_Debug_PushCPUMarker(this->filename);
#endif
					( *act )( this ) ;
#if defined(ADD_ACTOR_CPU_MARKERS)
               BP_Debug_PopCPUMarker();
#endif
#if 0
					if( GV_CheckMemory() == 0 ){
						printf( "mem err in %s\n", this->filename );
						HANGUP();
					}
#endif
#ifdef DEBUG_MODE
					if( next && next->prev == this ){
						// これ以外の時は解放された
						outtime = GET_HSYNC_TIMER();
						this->runtime += DIFF_HSYNC_TIMER( outtime, intime );
						this->count++;
					}
#endif
				}
			} while ( ( this = next ) != NULL ) ;
		}
		list ++ ;
	}
}

/*----------------------------------------------------------------*/

void GV_InsertActor( int level, void *work )
{
	GV_ACT		*this, *next, *prev ;

	ASSERT( level >= 0 && level < GV_ACTOR_LEVEL_MAX ) ;
	this = (GV_ACT *)work ;

	prev = &( ActorList[ level ].start ) ;
	next = prev->next;

	// 各タスクリストの先頭に追加する

	next->prev = this;
	this->next = next;
	this->prev = prev;
	prev->next = this;

	this->prio = 0;
	this->act = this->die = NULL ;
}

void GV_InsertActorPriority( int level, void *work, int prio )
{
	GV_ACT		*this, *next, *prev ;

	ASSERT( level >= 0 && level < GV_ACTOR_LEVEL_MAX ) ;
	this = (GV_ACT *)work ;

	this->prio = prio;
	prev = &( ActorList[ level ].start ) ;

	for( next = prev->next; next != NULL; next = next->next ){
		if( next->prio >= prio ){
			next->prev = this;
			this->next = next;
			this->prev = prev;
			prev->next = this;

			this->act = this->die = NULL ;
			return;
		}
		prev = next;
	}
	HANGUP();
}

void *GV_CreateActor( int level, int class, int size, int prio )
{
	GV_ACT		*this ;

	if( class & GV_CLASS_EFFECT ){
		int rest;
		rest = GV_GetFreeMemorySize( GV_NORMAL_MEMORY );
		if( rest < GV_MEM_EFFECT_LIMIT_SIZE ){
			printf( "EFFECT MEMORY LIMIT %d\n", rest );
			return NULL;
		}
	}

	this = (GV_ACT *)GV_MallocAct( size ) ;
	if ( this != NULL ) {
		GV_ZeroMemory( this, size ) ;
		GV_SetActorFreeFunc( this, GV_Free );
		GV_SetActorClass( this, class );
		if( class == GV_CLASS_SYSTEM ){
			GV_SetActorKillLevel( this, GV_KILL_LEVEL_MANAGER );
		} else {
			GV_SetActorKillLevel( this, GV_KILL_LEVEL_NORMAL );
		}
		GV_InsertActorPriority( level, this, prio );
	}
	return (void *)this ;
}

#ifndef GOLD_VERSION

void GV_SetNamedActor( void *this, void *act, void *die, char *name )
{
	GV_ACT		*actor ;

	actor = (GV_ACT *)this ;
	actor->act = (void (*)( struct _GV_ACT * ))act ;
	actor->die = (void (*)( struct _GV_ACT * ))die ;
	actor->filename = name;
	actor->runtime = actor->count = 0;
}

#else

void GV_SetActor( void *this, void *act, void *die )
{
	GV_ACT		*actor ;

	actor = (GV_ACT *)this ;
	actor->act = (void (*)( struct _GV_ACT * ))act ;
	actor->die = (void (*)( struct _GV_ACT * ))die ;
}

#endif

/*----------------------------------------------------------------*/
/*
	親子関係管理ルーチン
*/

/*
	デフォルトのSignalFunc
*/

int GV_DefaultSignalFunc( void *work, int signal, int value )
{
	if( signal == GV_SIGNAL_KILL ){
		( ( GV_ACT * )work )->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		return 1;
	}
	return 0;
}

/*
	child を parentの子どもにする。
*/
void GV_SetActorChild( void *parent, void *child )
{
	GV_ACT_EX *p, *c;

	if( child == NULL ) return;

	ASSERT( ( ( GV_ACT * )parent )->class & GV_CLASS_EX );
	ASSERT( ( ( GV_ACT * )child )->class & GV_CLASS_EX );

	p = parent;
	c = child;

	c->actor.class |= GV_CLASS_CHILD;
	if( c->sigfunc == NULL ){
		c->sigfunc = GV_DefaultSignalFunc;
	}

	if( p->child == NULL ){
		/* 親の設定 */
		p->actor.class |= GV_CLASS_PARENT;
		p->child = c;
		/* 子の設定 */
		c->actor.class |= GV_CLASS_CHILD_TOP;
		c->c_prev = p;
	} else {
		/* 子の先頭として登録する */
		p->child->actor.class &= ~GV_CLASS_CHILD_TOP;
		p->child->c_prev = c;

		c->actor.class |= GV_CLASS_CHILD_TOP;
		c->c_prev = p;
		c->c_next = p->child;

		p->child = c;
	}
}

void GV_CallChildSignalFunc( void *this, int signal, int value )
{
	GV_ACT_EX *p;

	p = this;
	ASSERT( p->actor.class & GV_CLASS_EX );

	/* 子全員に通知 */
	p = p->child;
	for( ; p != NULL; p = p->c_next ){
		if( p->sigfunc != NULL ){
			( *p->sigfunc )( p, signal, value );
		}
	}
}

int GV_CallParentSignalFunc( void *this, int signal, int value )
{
	GV_ACT_EX *p;

	p = this;
	ASSERT( p->actor.class & GV_CLASS_EX );
	ASSERT( p->actor.class & GV_CLASS_CHILD );

	/* 親を探す */
	for( ; !( p->actor.class & GV_CLASS_CHILD_TOP ); p = p->c_prev ){
		;
	}
	p = p->c_prev;
	return ( *p->sigfunc )( p, signal, value );
}

void GV_DestroyChild( void *this )
{
	GV_ACT_EX *p;
	p = this;

	if( !( p->actor.class & GV_CLASS_EX )
		|| !( p->actor.class & GV_CLASS_CHILD ) ){
			return;
	}

	p->actor.class &= ~GV_CLASS_CHILD;

	if( p->actor.class & GV_CLASS_CHILD_TOP ){
		GV_ACT_EX *parent;
		parent = p->c_prev;
		if( ( parent->child = p->c_next ) != NULL ){
			p->c_next->c_prev = parent;
			p->c_next->actor.class |= GV_CLASS_CHILD_TOP;
		}
	} else {
		if( ( p->c_prev->c_next = p->c_next ) != NULL ){
			p->c_next->c_prev = p->c_prev;
		}
	}
}

/*----------------------------------------------------------------*/
/*
	終了処理
*/

static void Destroy( void *this )
{
	GV_ACT		*actor, *next, *prev ;
	void		( *die )( GV_ACT * ) ;
	void		( *free )( void * ) ;

	actor = (GV_ACT *)this ;
	/* このアクターをリストから切り放す */
	next = actor->next ;
	prev = actor->prev ;
	ASSERT( next != NULL && prev != NULL ) ;
	next->prev = prev ;
	prev->next = next ;
	actor->next = actor->prev = NULL;

	/* 終了処理関数をコール */
	if ( ( die = actor->die ) != NULL ) {
		( *die )( actor );
	}
	/* 解放関数が指定されていたら解放 */
	if( ( free = actor->free ) != NULL ){
		( *free )( actor );
	}
}

static void DestroyChild( void *this )
{
	int class;

	/* 子プロセス,親プロセスの終了処理 */

	class = ( ( GV_ACT * )this )->class;
	if( class & GV_CLASS_PARENT ){
		/* 親の場合は、子にkillシグナルを通知(死ぬかどうかは子次第) */
		GV_CallChildSignalFunc( this, GV_SIGNAL_KILL, 0 );
	}
	if( class & GV_CLASS_CHILD ){
		/* 子の場合は親から切り離される */
		GV_DestroyChild( this );
	}
}

void GV_DestroyActorQuick( void *this )
{
	ASSERT( this != NULL );

	DestroyChild( this );

	/* その場(これが呼ばれたアクター内)でDestroy処理 */
	Destroy( this );
}

extern void BP_SpecialCase_DemoActorDie(void* this);

void GV_DestroyActor( void *this )
{
	ASSERT( this != NULL );

	{
		GV_ACT *act = ( GV_ACT * ) this;
		if( act->prev == NULL || act->next == NULL ){
			printf( "WARN !! DESTROY ERR !!\n" );
			return;
		}
	}

   BP_SpecialCase_DemoActorDie(this);

	DestroyChild( this );

	/* 次のターンでDestroy処理 */
	( (GV_ACT *)this )->act = ( void (*)( GV_ACT * ) )Destroy;
	GV_WakeupActor( this, GV_CLASS_WAITING );
}

GV_ACT			*GV_SearchActor( void *target )
{
	AList		*list ;
	int		i ;

	ASSERT( target != NULL );

	list = ActorList ;
	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {
		GV_ACT		*this, *next ;

		this = &( list->start ) ;
		do {
			next = this->next ;
			if ( this == target ){
				return this ;
			}
		} while ( ( this = next ) != NULL ) ;
		list ++ ;
	}
	return NULL ;
}

void GV_DestroyOtherActor( void *target )
{
	/*
	   アクターリストを検索して、存在するかどうか確認してから
	   Destroy する
	*/
	GV_ACT	*this ;

	if ( target == NULL ) return ;
	this = GV_SearchActor( target ) ;
	if ( this != NULL ) GV_DestroyActor( this ) ;
#if 0	/* 旧 */
	AList		*list ;
	int		i ;

	ASSERT( target != NULL );

	list = ActorList ;
	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {
		GV_ACT		*this, *next ;

		this = &( list->start ) ;
		do {
			next = this->next ;
			if ( this == target ){
				GV_DestroyActor( this ) ;
				return;
			}
		} while ( ( this = next ) != NULL ) ;
		list ++ ;
	}
#endif
}

void	GV_DestroyOtherActorQuick( void *target )
{
	GV_ACT		*this ;

	if ( target == NULL ) return ;
	this = GV_SearchActor( target ) ;
	if ( this != NULL ) GV_DestroyActorQuick( this ) ;
}

void GV_DestroyActorSystem( int kill )
{
	AList		*list ;
	int		i ;

	list = ActorList ;
	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {
		GV_ACT		*this, *next ;

		/*
			指定された KILL レベル以下のプロセスは停止
		*/
		this = &( list->start ) ;

		do {
			next = this->next ;
			if ( this->kill & kill ) {
				this->class |= GV_CLASS_KILLALL;
				GV_DestroyActor( ( void * )this ) ;
			}
		} while ( ( this = next ) != NULL ) ;
		list ++ ;
	}
}

void GV_DestroyActorSystemAll( void )
{
	AList		*list ;
	int		i ;

	list = ActorList ;
	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {
		GV_ACT		*this, *next ;

		/*
			有無を言わさずに全プロセスは停止
		*/
		this = &( list->start ) ;

		do {
			next = this->next ;
			this->class |= GV_CLASS_KILLALL;
			GV_DestroyActor( ( void * )this ) ;
		} while ( ( this = next ) != NULL ) ;
		list ++ ;
	}
}


/* ---------------------------------------------------------------------- */
/*
	ユーティリティ
*/

/*
	すべてのACTORにたいして、func を実行する
*/
void GV_CheckActorAll( void (*func)( void *act ) )
{
	AList		*list ;
	int		i ;

	list = ActorList ;
	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {
		GV_ACT		*this, *next ;

		this = list->start.next;
		do {
			next = this->next ;
			( *func )( ( void * )this );
		} while ( ( this = next ) != NULL ) ;
		list ++ ;
	}
}


/*
	ActorSystem終了処理
*/
void GV_KillActorSystem( void )
{
	GV_DestroyActorSystemAll() ;	// 終了処理発効
	GV_ExecActorSystem() ;			// 終了処理は次の処理の時に行われる。
}

/*
	ACTOR LIST の整合性チェック(デバッグ用)
*/
#ifndef MGS_MASTER

static int pointer_check( void *ptr )
{
#ifdef PSX2	/// _program_top がないのでとりあえずとばす	
	extern int _program_top[];	// app.cmdのスタートアドレス
	void *top = _program_top;

	if( ptr == NULL ) return 0;
	if( ( ( int )ptr & 3 ) != 0 || ptr < top || ptr > MEM_BOTTOM ){
		return -1;
	}
#endif	/// 	
	return 1;
}

int GV_CheckActorList( void )
{
	AList		*list ;
	int		i ;
	GV_ACT		*this, *next ;

	list = ActorList ;
	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {

		this = list->start.next;
		do {
			if( pointer_check( this ) <= 0 ) goto broken2;
#ifdef DEBUG
			if( pointer_check( this->filename ) < 0 ) goto broken2;
#endif
			if( pointer_check( this->next ) < 0 ) goto broken1;
			if( pointer_check( this->act ) < 0 ) goto broken1;
			if( pointer_check( this->die ) < 0 ) goto broken1;
			if( pointer_check( this->free ) < 0 ) goto broken1;
			if( pointer_check( this->prev ) < 0 ) goto broken1;
			next = this->next ;
		} while ( ( this = next ) != NULL ) ;
		list ++ ;
	}
	return 1;
broken1:
#ifdef DEBUG
	excep_printf( "act broken %08X : %s\n", this, this->filename );
	if( this->prev != NULL ){
		excep_printf( "act prev   %08X : %s\n", this->prev, this->prev->filename );
	}
	return 0;
#endif
broken2:
	excep_printf( "act broken ALL %X\n", this );
	return 0;
}
#endif

#ifdef KP_WINDOWS
/*
	ResetDevice時の処理関数を設定する
*/
void GV_SetActorResetDeviceFunc( void *this, void *release_func, void *create_func )
{
	GV_ACT_EX_RD *actor;

	ASSERT( ( ( GV_ACT * )this )->class & GV_CLASS_EX_RD );

	actor = this ;

	actor->release_func = release_func;
	actor->create_func  = create_func;
}

/*
	ResetDevice時解放処理を行う
*/
int GV_ActorResetDeviceRelease( void *this )
{
	int		(*func)(void*);
	int		ret ;

	ASSERT( ( ( GV_ACT * )this )->class & GV_CLASS_EX_RD );

	func = ( ( GV_ACT_EX_RD * )this )->release_func ;
	if( func ){
		ret = func(this);
	} else {
		ret = 1 ;
	}

	return(ret) ;
}

/*
	ResetDevice時再確保処理を行う
*/
int GV_ActorResetDeviceCreate( void *this )
{
	int		(*func)(void*);
	int		ret ;

	ASSERT( ( ( GV_ACT * )this )->class & GV_CLASS_EX_RD );

	func = ( ( GV_ACT_EX_RD * )this )->create_func ;
	if( func ){
		ret = func(this);
	} else {
		ret = 1 ;
	}

	return(ret) ;
}

/*
	ResetDevice時解放処理を行う(全Actor対象)
*/
void GV_ActorResetDeviceReleaseAll(void)
{
	AList	*list ;
	int		i ;

	list = ActorList ;
	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {
		GV_ACT		*this, *next ;

		this = list->start.next;
		do {
			next = this->next ;
			if( this->class & GV_CLASS_EX_RD ){
				GV_ActorResetDeviceRelease( this ) ;
			}
		} while ( ( this = next ) != NULL ) ;
		list ++ ;
	}
}

/*
	ResetDevice時再確保処理を行う(全Actor対象)
*/
void GV_ActorResetDeviceCreateAll(void)
{
	AList	*list ;
	int		i ;

	list = ActorList ;
	for ( i = GV_ACTOR_LEVEL_MAX ; i > 0 ; -- i ) {
		GV_ACT		*this, *next ;

		this = list->start.next;
		do {
			next = this->next ;
			if( this->class & GV_CLASS_EX_RD ){
				GV_ActorResetDeviceCreate( this ) ;
			}
		} while ( ( this = next ) != NULL ) ;
		list ++ ;
	}
}
#endif
