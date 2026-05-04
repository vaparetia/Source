/*
	GV headers

		1999/05/26	K.Uehara
		$Id: libgv.h,v 1.13 2002/12/11 09:13:46 takaki Exp $
*/

#ifndef __LIBGV_H__
#define __LIBGV_H__

/* コンパイルマクロ */
#define	__SCRATCHPAD_USE_VIRTUAL_ALLOC__	(FALSE)		// VirtualAllocExを使用(ScratchPad)

/* I64`マクロのため */
#include "mgs_type.h"


#ifdef __cplusplus
#define class class_flag
#define this this_ptr
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
  ユーティリティマクロ
*/

#define NORM16( a )	( ( (u_int)(a) + 15 ) & ~0x0f )

/*
	ロードデータ初期化ルーチン
*/
typedef	int	( *GV_LOADFUNC )( void *, int, int );
typedef void ( *GV_FREEFUNC )( void * );

#define LOADER_CALL_RESIDENT	0x0100
#define LOADER_SPEC_MASK		0x00FF

enum loadfunc_mode {
	GV_INIT_NOCACHE = 0,
	GV_INIT_CACHE = 1,
	GV_INIT_RESIDENT = 2,
	GV_INIT_RESIDENT_AGAIN = 3,
};

#define GV_CACHEID_RESIDENT		0x80000000

/*----------------------------------------------------------------*/

/*
	EFFECTの制限サイズ
*/

//#define GV_MEM_EFFECT_LIMIT_SIZE	( 128*1024 )
#define GV_MEM_EFFECT_LIMIT_SIZE	( 1024*1024 )

/* ---------------------------------------------------- */

/*
	プロセスのワークヘッダ
*/
typedef struct _GV_ACT {
	struct _GV_ACT	*next ;		/* リンクリスト		*/
	void ( *act )( struct _GV_ACT * );
	unsigned int class;			/* クラスフラグ */
	unsigned int name;			/* Wait用識別子 */

	struct _GV_ACT	*prev ;		/* リンクリスト		*/
	void ( *die )( struct _GV_ACT * );
	GV_FREEFUNC free;			/* メモリ解放関数 */
	unsigned short prio;		/* 優先度(system actor 用) */
	unsigned short kill;		/* killレベルフラグ */

#ifndef GOLD_VERSION
   char *filename;				/* デバッグ用 */
   unsigned int runtime;		/* デバッグ用 */
   unsigned int count;			/* デバッグ用 */
#endif
#ifdef DEBUG
   int res1;					/* デバッグ用(予約) */
#endif

} GV_ACT;

typedef struct _GV_ACT_EX {
	GV_ACT actor;
	struct _GV_ACT_EX *child;	/* 子ども */
	struct _GV_ACT_EX *c_prev;	/* 子どものなかでのリンクリスト */
	struct _GV_ACT_EX *c_next;	/* 子どものなかでのリンクリスト */
	int ( *sigfunc )( void *work, int signal, int value );
} GV_ACT_EX;

#ifdef KP_WINDOWS
typedef struct _GV_ACT_EX_RD {
	GV_ACT_EX actor_ex;

	int ( *release_func )( void *work );	// ResetDevice時解放処理関数
	int ( *create_func )( void *work );		// ResetDevice時再作成関数
} GV_ACT_EX_RD;
#endif

#define	GV_SIGNAL_SYSTEM	0xFF00
#define GV_SIGNAL_KILL		0xFF01
#define GV_SIGNAL_DIE       0xFF02    /* メッセージの送り元が死のうとしている */

/*
	プロセスの優先順位
*/

enum	{
	GV_ACTOR_DAEMON,		/* システムデーモン	*/
	GV_ACTOR_MANAGER,		/* ゲーム進行プロセス */
	GV_ACTOR_ASSIST,		/* ゲーム補助プロセス */

	GV_ACTOR_PREV2,			/* メインより前に実行されるキャラクタ（ポーズ時も動作）*/
	GV_ACTOR_PREV,			/* メインより前に実行されるキャラクタ */

	GV_ACTOR_PLAYER,		/* 通常キャラクタ (プレイヤー) */
	GV_ACTOR_USER,			/* 通常キャラクタ */
	GV_ACTOR_CAMERA,		/* カメラ(ポーズ時も動作) */

	GV_ACTOR_AFTER,			/* メインより後に実行されるキャラクタ */
	GV_ACTOR_EFFECT,		/* エフェクト */
	GV_ACTOR_AFTER2,		/* メインより後に実行されるキャラクタ（ポーズ時も動作）*/

	GV_ACTOR_DAEMON2,		/* システムデーモン２ */
	GV_ACTOR_LEVEL_MAX
} ;

#define GV_PRIO_MAX			0xffff

/* プロセスクラス */

#define GV_CLASS_SYSTEM		0x00008000
#define GV_CLASS_CHARA		0x00004000
#define GV_CLASS_OBJECT		0x00002000
#define GV_CLASS_EFFECT		0x00001000

#define GV_CLASS_EX			0x00000080	// GV_ACT_EX
#define GV_CLASS_PARENT		0x00000040	// 子どもを持つ
#define GV_CLASS_CHILD_TOP	0x00000020	// CHILD TOP
#define GV_CLASS_CHILD		0x00000010	// CHILD

#define GV_CLASS_FOLLOW		0x00000100	// 親から殺された
#define GV_CLASS_KILLALL	0x00000200	// ステージ切替えなどで殺された

#define GV_CLASS_WAITING	0x000F0000	// 待ち状態
#define GV_CLASS_WAIT_MESG	0x00010000	// メッセージ待ち
#define GV_CLASS_WAIT_USER0	0x00020000	//  ユーザー使用 / Mapきりかえ(game/map.c)
#define GV_CLASS_WAIT_USER1	0x00040000	//  ユーザー使用（予約）
#define GV_CLASS_WAIT_USER	0x00080000	//  ユーザー使用（汎用）

#ifdef KP_WINDOWS
#define GV_CLASS_EX_RD		0x00100000	// GV_ACT_EX_RD
#endif

#define GV_CLASS_MESSAGE_KILL	0x10000000	// killメッセージで死ぬ

#define GV_CHECK_WAIT( _this, _mask ) (((GV_ACT*)_this)->class & (_mask))

/* kill level */

#define GV_KILL_SYSTEM_M		0x01
#define GV_KILL_STAGE_M			0x02
#define GV_KILL_USER_M			0x04

#define GV_KILL_LEVEL_SYSTEM	(GV_KILL_SYSTEM_M)
		// システムが切り替わるときに終了処理
#define GV_KILL_LEVEL_MANAGER	(GV_KILL_SYSTEM_M|GV_KILL_STAGE_M)
		// ステージが切り替わるときに終了処理(GameOver時も残る)
#define GV_KILL_LEVEL_NORMAL	(GV_KILL_SYSTEM_M|GV_KILL_STAGE_M|GV_KILL_USER_M)
		// ステージが切り替わるときに終了処理

/* ポーズレベル設定 */

#define GV_PAUSE_NOSTOP		   0x00
#define GV_PAUSE_STOP		   0x01
#define GV_PAUSE_PAUSE		   0x02
#define GV_PAUSE_MENU		   0x04
#define GV_PAUSE_READERROR	   0x08
#define GV_PAUSE_DISCERROR	   0x10
#define GV_PAUSE_DEBUG        0x20  //BP_PAUSE - added by Jack

#define GV_LEVEL_NORMAL		(GV_PAUSE_STOP|GV_PAUSE_PAUSE|GV_PAUSE_MENU|GV_PAUSE_READERROR \
							 |GV_PAUSE_DISCERROR|GV_PAUSE_DEBUG)
#define GV_LEVEL_STOP		(GV_PAUSE_STOP|GV_PAUSE_READERROR|GV_PAUSE_DISCERROR|GV_PAUSE_DEBUG)
#define GV_LEVEL_NOSTOP		GV_PAUSE_NOSTOP

/* ---------------------------------------------------- */

/* メッセージ関連 */

/* メッセージ構造体 (16Byte前提に作っているところあり) */

typedef struct {
	int address;		/* 宛先 */
	short _len;			/* 内部使用 */
	short _flag;		/* 内部使用 */
	int *message;		/* メッセージ列へのポインタ */
	int message_len;	/* メッセージの数 */
} GV_MSG;

#define MAX_MESSAGE_PARAM	512		/* 1Frameで必要なメッセージ列のワーク */
#define MAX_MESSAGES		128		/* 最大１フレームで飛ぶメッセージの数 */

#define GV_MSG_RECEIVED	0x01
#define GV_MSG_ACT_KILL	0x80

#define GV_MESSAGE_KILL		0xFFFFFF

/* ---------------------------------------------------- */

/* パッド関連 */

typedef struct {
	short type;					/* ペリフェラルタイプ */

	/* デジタルデータ */
#ifndef KP_WINDOWS
	unsigned short status;		/* 状態 */
	unsigned short press;		/* press trigger */
	unsigned short release;		/* release trigger */
#else
	DWORD	status;			/* 状態 */
	DWORD	press;			/* press trigger */
	DWORD	release;		/* release trigger */
#endif

	short dir;					/* 十字キー入力方向 */
	short analog_input;			/* アナログ入力の有無 */

	unsigned char right_dx;		/* アナログ */
	unsigned char right_dy;
	unsigned char left_dx;
	unsigned char left_dy;

	unsigned char pressure[ 12 ];		/* 感圧情報へのポインタ */

	int	flag ;			/* パッド状態フラグ */
} GV_PAD;

/* パッド状態フラグ */
enum {
    GV_PAD_RELEASE_PRG =	0x00000001,
    GV_PAD_RELEASE_SCN =	0x00000002,
    GV_PAD_MASK_PRG =		0x00000004,
    GV_PAD_MASK_SCN =		0x00000008,	
    GV_PAD_PRESS_PRG =		0x00000010,
    GV_PAD_PRESS_SCN =		0x00000020,
	GV_PAD_RELEASE_SYSTEM = 0x00000100,
	GV_PAD_PAD_DEMO =		0x00001000,
	GV_PAD_SEPARATE_STICK =	0x00002000, /*右スティックとデジタルキーを非同期*/
	GV_PAD_DISCONNECT	=   0x00004000,	// XBOX 用パッド抜けてるフラグ
} ;

#define	GV_PAD_RELEASE 	(GV_PAD_RELEASE_PRG|GV_PAD_RELEASE_SCN|GV_PAD_RELEASE_SYSTEM)
#define	GV_PAD_MASK		(GV_PAD_MASK_PRG|GV_PAD_MASK_SCN)

/* パッドタイプ */

#define GV_PAD_DIGITAL		0	// DIGITAL
#define GV_PAD_ANALOG		1	// ANALOG ( SLPH-1150 )
#define GV_PAD_DUALSHOCK	2	// DUALSHOCK
#define GV_PAD_DUALSHOCK2	3	// DUALSHOCK2

// Used for custom Vita control schemes.
#define BP_VITA_CONTROL_SCHEME_ORIG 0

// The first overhaul of the controls
#define BP_VITA_CONTROL_SCHEME_V1 1

// Additional tweaks to the first overhaul
#define BP_VITA_CONTROL_SCHEME_V2 2

/* ボタンマクロ */

#define PAD_UDLR		(0x0000f000)
#define PAD_ABXY		(0x000000f0)
#define PAD_LR			(0x0000000f)

#define PAD_U			(0x00001000)
#define PAD_D			(0x00004000)
#define PAD_L			(0x00008000)
#define PAD_R			(0x00002000)
#define PAD_A			(0x00000020)
#define PAD_B			(0x00000040)
#define PAD_X			(0x00000010)
#define PAD_Y			(0x00000080)

#define PAD_L1			(0x00000004)
#define PAD_R1			(0x00000008)
#define PAD_L2			(0x00000001)
#define PAD_R2			(0x00000002)

#define PAD_STA			(0x00000800)
#define PAD_SEL			(0x00000100)

#define PAD_AL			(0x00000200)
#define PAD_AR			(0x00000400)

//BP_PAD - useful pad defines so things are less confusing!
#define PS2_PAD_CROSS      PAD_B
#define PS2_PAD_CIRCLE     PAD_A
#define PS2_PAD_SQUARE     PAD_Y
#define PS2_PAD_TRIANGLE   PAD_X

#define PS2_PAD_PRESS_CROSS      PAD_PRESS_B
#define PS2_PAD_PRESS_CIRCLE     PAD_PRESS_A
#define PS2_PAD_PRESS_SQUARE     PAD_PRESS_Y
#define PS2_PAD_PRESS_TRIANGLE   PAD_PRESS_X

//BP_PAD - useful pad defines so things are less confusing!


#ifdef KP_WINDOWS
#define PAD_EX1			(0x00010000)
#define PAD_EX2			(0x00020000)
#define PAD_EX3			(0x00040000)

#define PAD_RU			(0x00100000)
#define PAD_RD			(0x00400000)
#define PAD_RL			(0x00800000)
#define PAD_RR			(0x00200000)
#endif

#ifndef KP_WINDOWS
#define PAD_PRESS_R		0
#define PAD_PRESS_L		1
#define PAD_PRESS_U		2
#define PAD_PRESS_D		3
#define PAD_PRESS_X		4
#define PAD_PRESS_A		5
#define PAD_PRESS_B		6
#define PAD_PRESS_Y		7
#define PAD_PRESS_L1	8
#define PAD_PRESS_R1	9
#define PAD_PRESS_L2	10
#define PAD_PRESS_R2	11
#else
enum
{
	PAD_PRESS_R,	//	0
	PAD_PRESS_L,	//	1
	PAD_PRESS_U,	//	2
	PAD_PRESS_D,	//	3
	PAD_PRESS_X,	//	4
	PAD_PRESS_A,	//	5
	PAD_PRESS_B,	//	6
	PAD_PRESS_Y,	//	7
	PAD_PRESS_L1,	//	8
	PAD_PRESS_R1,	//	9
	PAD_PRESS_L2,	//	10
	PAD_PRESS_R2,	//	11
	/* (注)追加の際にはwpad_config_equ.hにも追加する事 */

	MAX_PAD_PRESS,	
} ;

#endif

/* analog_input */
#define	GV_PAD_ANALOG_BUTTON	0x0001
#define	GV_PAD_ANALOG_R_USE		0x0002
#define	GV_PAD_ANALOG_L_USE		0x0004
#define	GV_PAD_ANALOG_INPUT		0x0007

#define GV_PAD_ANALOG_CHANGE	0x0008
#define GV_PAD_ANALOG_R_CHANGE	(1<<(PAD_PRESS_R +4))
#define GV_PAD_ANALOG_L_CHANGE	(1<<(PAD_PRESS_L +4))
#define GV_PAD_ANALOG_U_CHANGE	(1<<(PAD_PRESS_U +4))
#define GV_PAD_ANALOG_D_CHANGE	(1<<(PAD_PRESS_D +4))
#define GV_PAD_ANALOG_X_CHANGE	(1<<(PAD_PRESS_X +4))
#define GV_PAD_ANALOG_A_CHANGE	(1<<(PAD_PRESS_A +4))
#define GV_PAD_ANALOG_B_CHANGE	(1<<(PAD_PRESS_B +4))
#define GV_PAD_ANALOG_Y_CHANGE	(1<<(PAD_PRESS_Y +4))
#define GV_PAD_ANALOG_L1_CHANGE	(1<<(PAD_PRESS_L1+4))
#define GV_PAD_ANALOG_R1_CHANGE	(1<<(PAD_PRESS_R1+4))
#define GV_PAD_ANALOG_L2_CHANGE	(1<<(PAD_PRESS_L2+4))
#define GV_PAD_ANALOG_R2_CHANGE	(1<<(PAD_PRESS_R2+4))
#define GV_PAD_ANALOG_ALLCHANGE	0xfff8

#define GV_PAD_ANALOG_PRESS(_p)	(1<<((_p)+4)) /* PAD_PRESS_?? を使うため*/


/* パッド遊び幅 */
#ifdef BP_VITA

/* included these from g_define.h */
extern int GM_GameStatus;
#  define STATE_PAD_DEMO_REDEFINE 0x40000000

#  define ANALOG_MARGIN_DEMO     (48)
#  define ANALOG_MARGIN_F_DEMO   (48.0F)
#  define ANALOG_MARGIN_VITA     (32)
#  define ANALOG_MARGIN_F_VITA   (32.0F)

inline int get_analog_margin()
{
   if (GM_GameStatus & STATE_PAD_DEMO_REDEFINE)
   {
      return ANALOG_MARGIN_DEMO;
   }
   return ANALOG_MARGIN_VITA;
}
inline float get_analog_margin_f()
{
   if (GM_GameStatus & STATE_PAD_DEMO_REDEFINE)
   {
      return ANALOG_MARGIN_F_DEMO;
   }
   return ANALOG_MARGIN_F_VITA;
}

#  define ANALOG_MARGIN    get_analog_margin()
#  define ANALOG_MARGIN_F  get_analog_margin_f()

#else

#  define ANALOG_MARGIN    (48)
#  define ANALOG_MARGIN_F  (48.0F)

#endif


/* その他コンビネーションマクロ */

extern int BP_Pad_GetOkAssignment();
extern int BP_Pad_GetCancelAssignment();

#ifdef KP_XBOX	// PAD_A はＢボタンのこと
#define PAD_OK			( PAD_B | PAD_STA )
#define PAD_CANCEL		( PAD_A | PAD_SEL )
#define PAD_DEMO_CANCEL	( PAD_B | PAD_STA )
#else	
#define PAD_OK			( BP_Pad_GetOkAssignment() )
#define PAD_CANCEL		( BP_Pad_GetCancelAssignment() )
#define PAD_DEMO_CANCEL	( PAD_B | PAD_STA )
#endif	

/* パッドデータ実体 */
enum {
	GV_PAD_1,
	GV_PAD_2,
	GV_PAD_1_DIRECT,
	GV_PAD_2_DIRECT,
	GV_PAD_MAX
};

/* ---------------------------------------------------- */

/* Windows用入力手続き */

#ifdef KP_WINDOWS
void	GV_SetWindowsInputConfigulationMode(BOOL subject) ;	// 入力モードを設定(通常/主観)

int		GV_GetWindowsSubjectMove(void) ;	// 主観移動モード設定取得
#endif

/* ---------------------------------------------------- */

/* メモリ関連 */

enum {
	GV_MEMORY_STATIC,
	GV_MEMORY_DYNAMIC
};

/* メモリシステム番号 */

enum {
	GV_PACKET_MEMORY0,
	GV_PACKET_MEMORY1,
	GV_NORMAL_MEMORY,
	MAX_MEMSYS
};

/* ---------------------------------------------------- */
/*
	変数宣言
*/

extern int GV_Time;
#define GV_Clock	(GV_Time&1)

extern int GV_PauseLevel;

extern GV_PAD 	GV_PadData[ GV_PAD_MAX ];
extern GV_PAD	GV_PadDataDirect[ GV_PAD_MAX ] ;

#define GV_PauseLevelNoXMB  (GV_PauseLevel & ~GV_PAUSE_DEBUG)  //BP_PAUSE - exclude XMB from pause check (which currently sets GV_PAUSE_DEBUG)


extern int GV_PadControlDisabled(int control);

/* ---------------------------------------------------- */
/*
	No.とビットとの相互変換
*/

/* Noからビットを得る */
EXTERN_INLINE int GV_GetBit( int no )
{
    return 0x40000000 >> no ;
}

/* ビットからNoを得る */
EXTERN_INLINE int GV_GetNo( int id )
{
#ifdef BP_PSX2_ASM
    u_long64	bit64, id64 ;

    bit64 = id;
    asm volatile ( "plzcw %0,%1" : "=r"(id64) : "r"(bit64) ) ;
    /* 上からの連続したビット数 - 1 */
    return ( int )( id64 & 0xffffffff ) ;
#else
	int i;
	for(i = 0; i < 31; i++){
		if (id & 0x40000000) return i;
		id <<= 1;
	}
	return 31;
#endif	
}

/* ビットからNoを削除 */
EXTERN_INLINE unsigned int GV_DeletBit( unsigned int bit, int no )
{
	unsigned int	deletbit ;
	
	deletbit = GV_GetBit( no ) ;
	bit &= ~deletbit ;
	
	return bit ;
}


/* ---------------------------------------------------- */
/*
	トレース用マクロ
*/

#define KP_CTASSERT(cond)    extern int KP_CTASSERT( int compile_time_assertion_failed[ ((cond) ? 1 : -1) ] )

#ifdef KP_WINDOWS
#ifdef ERROR
#undef ERROR
#endif
#endif

#ifdef DEBUG
extern char *GV_DebugMes;
#define	WARNING(f,v)
// BP_PS2 #define	ERROR(f,v)
#define	ASSERT(c) if ( !(c) ) GV_Assert( __FILE__, __LINE__ ) ;
#define  XASSERT(c,...) ASSERT(c)
#define	OPERATOR()
#define	MARK(n)		( GV_DebugMes = (char*)n )
#define HANGUP()	BP_BREAK
#define Debug( a )	a
#else
#define	WARNING(f,v)
//#define	ERROR(f,v)
#define	ASSERT(c)
#define  XASSERT(c,...)
#define	OPERATOR()
#define	MARK(n)
#define HANGUP() BP_BREAK
#define Debug( a )
#endif

/* ---------------------------------------------------------------------- */
/*
	エラー表示マクロ
*/

#define GV_ERROR_NO_MEMORY		0x00000001
#define GV_ERROR_MESSAGE_FULL	0x00000002
#define GV_ERROR_MAX_OBJ_QUEUE	0x00000004
#define GV_ERROR_MAX_PRIM_QUEUE	0x00000008
#define GV_ERROR_MAX_MISC_QUEUE	0x00000010
#define GV_ERROR_NAVI_NO_GOAL	0x00000020
#define GV_ERROR_DOUBLE_MESG	0x00000040
#define GV_ERROR_JUST_ON_WALL	0x00000080
#define	GV_ERROR_NARROW_POS		0x00000100
#define	GV_ERROR_PL_MESG_FAIL	0x00000200	/* プレイヤーに強制系のメッセージを送ったが無効にされた */
#define	GV_ERROR_WORLD_BOTTOM	0x00000400	/* 永久落下キャラがいる */
#define GV_ERROR_TEXTURE_OVER	0x00000800	/* テクスチャの１ＭＢオーバー */
#define GV_ERROR_NO_INIT_PRIM2	0x00001000	/* DG_PRIM2の未初期化使用警告 */

#define GV_ERROR_CHAR			"MSOPQGDWNFBTI"		// 下位ビットから順番(gamed.cで表示)

#ifdef DEBUG_MODE

extern unsigned int GV_ErrorFlag;		// gvd.c

#define GV_ERROR( _a )	( GV_ErrorFlag |= (_a) )

#else

#define GV_ERROR( _a )

#endif


/* ---------------------------------------------------- */
/*
	インラインマクロ
*/
#if 0 //BP_PS2
//#ifdef PSX2
#define GV_ActorEX( _act ) \
{ \
	ASSERT( sizeof( *_act ) == sizeof( GV_ACT_EX ) ); \
	(_act)->actor.class |= GV_CLASS_EX; \
	*( long128 * )&( (_act)->child ) = 0; \
	(_act)->sigfunc = GV_DefaultSignalFunc; \
}
#else
#define GV_ActorEX( _act ) \
{ \
	ASSERT( sizeof( *_act ) == sizeof( GV_ACT_EX ) ); \
	(_act)->actor.class |= GV_CLASS_EX; \
	(_act)->child = (_act)->c_prev = (_act)->c_next = NULL; \
	(_act)->sigfunc = GV_DefaultSignalFunc; \
}
#endif

#ifdef KP_WINDOWS
#define GV_ActorEXRD( _act ) \
{ \
	ASSERT( sizeof( *_act ) == sizeof( GV_ACT_EX_RD ) ); \
	(_act)->actor_ex.actor.class |= GV_CLASS_EX_RD; \
	(_act)->release_func = NULL; \
	(_act)->create_func = NULL; \
}
#endif

#define	GV_ConvVec3( _from, _to ) \
{ \
	  (_to)->vx = (_from)->vx ; \
	  (_to)->vy = (_from)->vy ; \
	  (_to)->vz = (_from)->vz ; \
}

#define	GV_InitVec3( _v, _x, _y, _z ) \
{ \
	  (_v)->vx = _x ; \
	  (_v)->vy = _y ; \
	  (_v)->vz = _z ; \
}

#define	GV_SetVec3( _v, _x, _y, _z )	GV_InitVec3( _v, _x, _y, _z ) 

/* ---------------------------------------------------- */

/*
	関数プロトタイプ
*/

// in gvd.c

void GV_StartDaemon( void );
void GV_KillDaemon( void );
void GV_ResetPacketMemory( void );

void GV_ResetMemory( void );
void GV_ResetSystem( void );

// in actor.c 

void GV_InitActorSystem( void );
void GV_ExecActorSystem( void );
void GV_DestroyActorSystem( int kill );
void GV_DestroyActorSystemAll( void );
void GV_KillActorSystem( void );

void GV_InsertActor( int level, void *work );
void GV_InsertActorPriority( int level, void *work, int prio );
void *GV_CreateActor( int level, int class, int size, int prio );
void GV_DestroyActorQuick( void *this );
void GV_DestroyActor( void *this );
void GV_DestroyOtherActor( void *target );
void GV_DestroyOtherActorQuick( void *target );
GV_ACT	*GV_SearchActor( void *target ) ;

void GV_CheckActorAll( void (*func)( void *act ) );

void GV_SetActorChild( void *parent, void *child );
void GV_CallChildSignalFunc( void *this, int signal, int value );
int GV_CallParentSignalFunc( void *this, int signal, int value );
void GV_DestroyChild( void *this );
int GV_DefaultSignalFunc( void *work, int signal, int value );

#ifdef KP_WINDOWS
void GV_SetActorResetDeviceFunc( void *this, void *release_func, void *create_func );
int GV_ActorResetDeviceRelease( void *this );
int GV_ActorResetDeviceCreate( void *this );
void GV_ActorResetDeviceReleaseAll(void);
void GV_ActorResetDeviceCreateAll(void);
#endif

#ifndef GOLD_VERSION
void GV_SetNamedActor( void *this, void *act, void *die, char *name );
#else
void GV_SetActor( void *this, void *act, void *die );
#endif

#ifdef DEBUG
void GV_DumpActorSystem( int mode );
#endif

int GV_CheckActorList( void );	// for EXCEPTION check

// 互換関数
#define GV_NewActor( _l, _s ) GV_CreateActor( _l, GV_CLASS_OBJECT, _s, 0 )
#define GV_NewActorPrio( _l, _s, _p ) GV_CreateActor( _l, GV_CLASS_OBJECT, _s, _p )
#define GV_NewEffect( _l, _s ) GV_CreateActor( _l, GV_CLASS_EFFECT, _s, 0 )
#define GV_NewEffectPrio( _l, _s, _p ) GV_CreateActor( _l, GV_CLASS_EFFECT, _s, _p )

// in memory.c

void GV_InitMemoryBlock( int id, int type, void *top, int size );
void GV_InitMemorySystemAll( void );
void GV_FreeMemory( int id, void *ptr );
#ifndef KP_WINDOWS
void GV_ZeroMemory( void *to, int size );
#else
#define	GV_ZeroMemory( to_, size_ )		ZeroMemory((to_), (size_))
#endif
void GV_ResizeMemory( int id, void *ptr, int size );

void GV_Free( void *addr );
void GV_Resize( void *addr, int size );
void GV_CleanMemorySystem( void );
void GV_FreeMemory2( int which, void *addr );
void GV_DelayedFree( void *addr );
int GV_GetMaxFreeMemory( int id );
int GV_GetFreeMemorySize( int id );

#ifdef DEBUG_MODE
void GV_DumpMemory( int id );
void GV_DumpMemorySize( int id );
void GV_SetLoadedMemory( void );
void GV_CheckMemoryLeak( void );
#endif
int GV_CheckMemory( void );	// for EXCEPTION check

#ifdef DEBUG_MODE
/*
	デバッグ用呼出元記録型メモリ確保関数
*/
#ifdef KP_WINDOWS	// (#ifdef KP_XBOX)
void *GV_AllocMemoryD( int id, void **ptr, int size, int align, char *fname ,int line);
void *GV_MallocD( int size, char *fname ,int line);
void *GV_MallocLoadD( int size, char *fname  ,int line);
void *GV_MallocActD( int size, char *fname ,int line );
#else
void *GV_AllocMemoryD( int id, void **ptr, int size, int align, char *fname );
void *GV_MallocD( int size, char *fname );
void *GV_MallocLoadD( int size, char *fname );
void *GV_MallocActD( int size, char *fname );
#endif

#ifdef KP_WINDOWS	// (#ifdef KP_XBOX)

#define GV_AllocMemory( _id, _ptr, _size, _align ) \
	GV_AllocMemoryD( (_id), (_ptr), (_size), (_align), __FILE__ ,__LINE__)
#define GV_Malloc( _size ) \
	GV_MallocD( (_size), __FILE__ ,__LINE__ )
#define GV_MallocLoad( _size ) \
	GV_MallocLoadD( (_size), __FILE__  ,__LINE__)
#define GV_MallocAct( _size ) \
	GV_MallocActD( (_size), __FILE__  ,__LINE__)

#else

#define GV_AllocMemory( _id, _ptr, _size, _align ) \
	GV_AllocMemoryD( (_id), (_ptr), (_size), (_align), __FILE__ )
#define GV_Malloc( _size ) \
	GV_MallocD( (_size), __FILE__ )
#define GV_MallocLoad( _size ) \
	GV_MallocLoadD( (_size), __FILE__ )
#define GV_MallocAct( _size ) \
	GV_MallocActD( (_size), __FILE__ )

#endif
#else
void *GV_AllocMemory( int id, void **ptr, int size, int align );
void *GV_Malloc( int size );
#define GV_MallocLoad	GV_Malloc
#define GV_MallocAct	GV_Malloc
#endif

// in pad.c
void GV_InitPadSystem( void );
void GV_ResetPadSystem( void );
void GV_UpdatePadSystem( void );
int	GV_GetPadOrigin( void ) ;
void GV_OriginPadSystem( int org );

void GV_SetPadVibration1( int which, int value );
void GV_SetPadVibration2( int which, int value );

extern void GV_PadSetDefaultKeyConf( void ) ;/* キーコンフィグをデフォルトに */
extern void GV_PadSetKeyConf( int a, int b ) ;/* キーコンフィグを設定するaのボタンをbに変える。a,bともにGV_PRESS_???を指定する*/
extern int  GV_PadGetStatus( int a ) ; /* キーコンフィグ後のstatusに変える。GV_PADのstatusは既にこの関数を通っている */
#ifdef KP_XBOX
extern void GV_SetChangePressure( int press_key, int type ) ;
#endif

#ifdef DEBUG_MODE
int GV_PadHangupCheck( void );
#endif

void	GV_PadReleaseOnSystem( int ) ;
void	GV_PadReleaseOffSystem( int ) ;
void	GV_PadReleaseOn( int ) ;
void	GV_PadReleaseOff( int ) ;
void	GV_PadReleaseOnScn( int ) ;
void	GV_PadMaskOn( int, int ) ;
void	GV_PadMaskOff( int ) ;
void	GV_PadMaskOnScn( int, int ) ;
void	GV_PadPressScn( int, int ) ;
void	GV_PadMaskCancelScn( int, int ) ;

void	GV_PadSeparateOnScn( int which ) ;/* アナログとデジタル方向キーの分離*/
void	GV_PadSeparateOffScn( int which ) ;

void	GV_PadSetDemoData( int, int, void * ) ;

void GV_SetPressureChangeFlag( short flag ) ;
void GV_InitPressureChangeFlag( void ) ;

#ifdef KP_XBOX
void GV_ResetPressureChangeFlag( short flag ) ;

void GV_ChangePadPortMapping( int prevport, int newport );
int	GV_GetXPadType( int port );
#endif
int GV_GetXPadPort( int port );


// in debug.c
void GV_Assert( char *file, int line );

// in strcode.c
int GV_StrCode( char const *string );

// in cache.c

int GV_CacheID( int root_id, int spec );
int GV_CacheID2( char *root_name, int spec );
int GV_CacheID3( char *name );

void *GV_GetCache( int id );
void *GV_GetCacheMarkSwapped( int id, int *pWasSwapped );
void GV_ExchangeCache( int id1, int id2 ) ;

void GV_SetLoader( int spec, GV_LOADFUNC init );

void GV_ResetLoader( void );
void GV_InitCacheSystem( void );
void GV_FreeCacheSystem( void );
int GV_LoadInit( void *data, int name, int cache_mode );

// in resident.c

void GV_InitResidentMemory( void );
void GV_SetSystemResident( void );
void GV_ResetResidentMemory( void );

void *GV_AllocResidentMemory( int size, int id );
void *GV_AllocResidentMemoryAligned( int size, int id, int align );
void GV_ReinitResidentData( void );
int GV_GetResidentDataSize( void *ptr );

// in message.c

void GV_InitMessageSystem( void );
void GV_ClearMessageSystem( void );
int GV_SendMessage( GV_MSG *send );
int GV_ReceiveMessage( int address, GV_MSG **msg_ptr );

/*
	インライン関数
*/

#ifndef __GV_INLINE_C__
#define __GV_INLINE__

#include "inline.c"

#else
// インライン関数プロトタイプ
// in inline.c 

void GV_SetActorPriority( void *this, int prio );
void GV_SetActorFreeFunc( void *this, GV_FREEFUNC free );
void GV_InitActor( int level, void *work, GV_FREEFUNC free );	// 互換用
void GV_SetActorKillLevel( void *this, int kill_level );
void GV_SetActorClass( void *this, int class );
void GV_PauseOnActorSystem( int pause );
void GV_PauseOffActorSystem( int pause );
void GV_PauseToggleActorSystem( int pause );
void GV_SetActorSignalFunc( void *this, int (*func)( void *work, int signal, int value ) );
void GV_SleepActor( void *this, int waitflag );
void GV_WakeupActor( void *this, int waitflag );
void GV_WaitMessage( void *this, int name );
void GV_ChangeActFunc( void *this, void *func );
int GV_IsFollowDestroy( void *this ) ;
int	GV_IsStageDestroy( void *this ) ;
int GV_CallSignalFunc( void *work, int signal, int value );
void GV_SetActorMessageKill( void *this, int name );
void GV_ExitPad(void);

#endif

#ifndef GOLD_VERSION
/* デバッグ時にプロセスのソースファイル名を登録する */
#define GV_SetActor( _w, _a, _d )	GV_SetNamedActor( _w, _a, _d, __FILE__ )
#endif

/* ---------------------------------------------------- */

/* デバッグ用USBキーボード関連 */

#ifdef USBKBD

#include "usbkbd.h"

#define MAX_KEYCODE	3

typedef struct {
	unsigned char mask;
	unsigned char status[ 3 ];
	unsigned char press[ 4 ];
	unsigned char release[ 4 ];
} GV_KEY;

extern GV_KEY GV_KeyData;

EXTERN_INLINE int GV_KeyCheck( unsigned char *key, int code )
{
	int i;
	for( i = 0; i < MAX_KEYCODE; i++ ){
		if( key[ i ] == code ){
			return 1;
		}
	}
	return 0;
}

#define KEY_MASK( _mask )	( GV_KeyData.mask & (_mask) )

#define KEY_PRESS( _code )		GV_KeyCheck( GV_KeyData.press, _code )
#define KEY_STATUS( _code )		GV_KeyCheck( GV_KeyData.status, _code )
#define KEY_RELEASE( _code )	GV_KeyCheck( GV_KeyData.release, _code )

#define IS_ANYKEY_PRESS()	( GV_KeyData.press[ 0 ] != 0 )

#else

#define KEY_MASK( _mask )		0
#define KEY_PRESS( _code )		0
#define KEY_STATUS( _code )		0
#define KEY_RELEASE( _code )	0
#define IS_ANYKEY_PRESS()		0

#endif

/* ---------------------------------------------------------- */

/* デバッグ関連マクロ */

/*
	インラインマクロ
*/

/*
	キャッシュ制御など
*/

#ifdef PSX2

#if 1 //BP_PS2
#define GV_PREFECH( _ptr )	
#define GV_UNCACHE( _ptr )	( _ptr )
#define GV_UNCACHEA( _ptr )	( _ptr )
#else
#define GV_PREFECH( _ptr )	asm ( "pref 0,0(%0)":: "r"(_ptr) )
#define GV_UNCACHE( _ptr )	( typeof(_ptr) )( ( unsigned int )_ptr | 0x20000000 )
#define GV_UNCACHEA( _ptr )	( typeof(_ptr) )( ( unsigned int )_ptr | 0x30000000 )
#endif

#endif	// PSX2

#ifdef KP_XBOX
#define GV_PREFECH( _ptr )	
#define GV_UNCACHE( _ptr )	( _ptr )
#define GV_UNCACHEA( _ptr )	( _ptr )

#endif	// KP_XBOX

/*
	デバッグ用HSYNCタイマーユーティリティ
*/
#ifdef PSX2
#ifndef _EEREGS_H
#include <eeregs.h>
#endif
#endif	// PSX2

#ifdef PSX2

#if 1 //BP_PS2
extern void INIT_HSYNC_TIMER(void);
extern u_int GET_HSYNC_TIMER(void);
extern void RESET_HSYNC_TIMER(void);
#else
#define INIT_HSYNC_TIMER()	( *T1_MODE = T_MODE_CLKS_M | T_MODE_CUE_M )
#define GET_HSYNC_TIMER()	( *T1_COUNT )
#define RESET_HSYNC_TIMER()	( *T1_COUNT = 0 )
#endif

#endif
#ifdef KP_XBOX

extern void INIT_HSYNC_TIMER(void);
extern u_int GET_HSYNC_TIMER(void);
extern void RESET_HSYNC_TIMER(void);

#endif
#define DIFF_HSYNC_TIMER( a, b )	( ( 0x00010000 + (a) - (b) ) & 0xffff )

EXTERN_INLINE void WAIT_HSYNC( u_int a )
{
	u_int t;
	t = GET_HSYNC_TIMER();
	for( ;; ){
		if( DIFF_HSYNC_TIMER( GET_HSYNC_TIMER(), t ) > a ){
			break;
		}
	}
}

/*
	デバッグ用マーカー
	COPの命令アドレスブレークポイントレジスタを使用
	break.hのハードウエアブレイクポイントとは共存しない

    Debug Marker
    Use the instructions Adoresuburekupointorejisuta COP
    Does not co-exist and break.h Hadoueabureikupointo

*/

EXTERN_INLINE void GV_MARK_REG( int value )
{
#ifdef PSX2	

#if 0 //BP_ASM - this is PS2 some kind of breakpoint signalling for debug code
	asm volatile( "mtiab %0" :: "r"( value ) );
#endif

#endif	
}

/*
	デバッグ用パフォーマンスカウンタユーティリティ
*/

#if 1 //BP_DEBUG

#define GV_SET_PRFC_ICACHE()	
#define GV_GET_PRFC_ICACHE()	0

#define GV_SET_PRFC_DCACHE()	
#define GV_GET_PRFC_DCACHE()	0

#define GV_SET_PRFC_CLOCK()		
#define GV_GET_PRFC_CLOCK()	0

#else

#define _PRFC_MASK( _ch, _no ) (0x80000000|((((_no)<<5)|(3<<3))<<((_ch)*10)))

#ifdef PSX2
#define _SET_PRFC( _ch, _no )\
{\
	int _m;\
	_m = _PRFC_MASK(_ch,_no);\
	asm __volatile__( "mtps %0,0; mtpc $0,%1; sync.p;" : : "r"(_m), "n"(_ch) );\
}

#define _GET_PRFC( _ch )\
({\
	int _v;\
	asm __volatile__( "mtps $0,0; mfpc %0,%1; sync.p;" : "=r"(_v) : "n"(_ch) );\
	_v;\
})
#else
#define _SET_PRFC( _ch, _no )
#define _GET_PRFC( _ch ) 0
#endif

#define GV_SET_PRFC_ICACHE()	_SET_PRFC( 0, 6 )
#define GV_GET_PRFC_ICACHE()	_GET_PRFC( 0 )

#define GV_SET_PRFC_DCACHE()	_SET_PRFC( 1, 6 )
#define GV_GET_PRFC_DCACHE()	_GET_PRFC( 1 )

#ifdef KP_XBOX
// クロック計測マクロ
#define GV_SET_PRFC_CLOCK()		_SET_PRFC( 0, 1 )
EXTERN_INLINE unsigned int GV_GET_PRFC_CLOCK( void )
{
	LARGE_INTEGER clock;
	__asm {
		rdtsc;
		mov clock.LowPart,eax;
		mov clock.HighPart,edx;
	}
	return clock.LowPart;
}
#else
#define GV_SET_PRFC_CLOCK()		_SET_PRFC( 0, 1 )
#define GV_GET_PRFC_CLOCK()		_GET_PRFC( 0 )
#endif

#endif

/*
	負荷計測用マクロ
*/

#if 0

enum {
	GV_PROFILE_DRAW_START,
	GV_PROFILE_DRAW_END = GV_PROFILE_DRAW_START + 32,

	GV_PROFILE_ACT_START,
	GV_PROFILE_CHANL_START,
	GV_PROFILE_CHANL_END = GV_PROFILE_CHANL_START + 31,
	GV_PROFILE_ACT_END,
	GV_PROFILE_MAX
};



extern unsigned int GV_ProfileClock;
extern unsigned short GV_ProfileLog[ 2 ][ GV_PROFILE_MAX ];	/* in gvd.c */
extern unsigned short GV_ChanlProfileNum[ 2 ];
extern unsigned short GV_DrawProfileNum[ 2 ];

#define GV_PROFILE( a ) ( GV_ProfileLog[ GV_ProfileClock ][ (a) ] = (unsigned short)GET_HSYNC_TIMER() )
#define GV_PROFILE_START( a ) \
{\
	GV_ProfileLog[ GV_ProfileClock ][ (a) ] = (unsigned short)GET_HSYNC_TIMER();\
}

#define GV_PROFILE_CHANL_START() \
{ \
	GV_ProfileLog[ GV_ProfileClock ][ GV_PROFILE_CHANL_START ] = (unsigned short)GET_HSYNC_TIMER(); \
	GV_ChanlProfileNum[ GV_ProfileClock ] = 0; \
}

#define GV_PROFILE_CHANL() \
{\
	int t = GET_HSYNC_TIMER();\
	if( GV_ProfileLog[GV_ProfileClock][ GV_PROFILE_CHANL_START \
									   + GV_ChanlProfileNum[ GV_ProfileClock ] ] != (unsigned short)t ){\
		GV_ChanlProfileNum[ GV_ProfileClock ] ++; \
		GV_ProfileLog[GV_ProfileClock][ GV_PROFILE_CHANL_START \
									   + GV_ChanlProfileNum[ GV_ProfileClock ] ] = (unsigned short)t;\
	}\
}

// FOR XBOX
#define GV_PROFILE_ACT_END() \
{\
	GV_ProfileLog[ GV_ProfileClock ][ GV_PROFILE_ACT_END ] = (unsigned short)GET_HSYNC_TIMER(); \
}

#define GV_PROFILE_DRAW( _n )\
{\
	GV_ProfileLog[GV_ProfileClock][ GV_PROFILE_DRAW_START + (_n) ] = (unsigned short)GET_HSYNC_TIMER();\
}

#define GV_PROFILE_DRAW_END( _n )\
{\
	GV_DrawProfileNum[ GV_ProfileClock ] = _n;\
}

// FOR XBOX
#define GV_PROFILE_DRAW_CLK( _n, _c )\
{\
	GV_ProfileLog[_c][ GV_PROFILE_DRAW_START+(_n) ] = (unsigned short)GET_HSYNC_TIMER();\
}
#define GV_PROFILE_DRAW_END_CLK( _n, _c )\
{\
	GV_DrawProfileNum[ _c ] = _n;\
}

#define GV_PROFILE_FLIP()	( GV_ProfileClock = 1 - GV_ProfileClock )

#else

#define GV_PROFILE( a )
#define GV_PROFILE_START( a )
#define GV_PROFILE_CHANL_START()
#define GV_PROFILE_CHANL()
#define GV_PROFILE_DRAW_END( _n )
#define GV_PROFILE_FLIP()

#define GV_PROFILE_ACT_END()
#define GV_PROFILE_DRAW_CLK( _n, _c )

#endif



#ifdef KP_XBOX

// FLOAT STACK CHECK

EXTERN_INLINE int GV_FLOAT_STACK( void )
{
	unsigned short control;
	__asm {
		fstsw	control;
	}
	return ( int )( ( control >> 11 ) & 0x07 );
}
#if 0
EXTERN_INLINE int GV_FLOAT_OPE_ON( void )
{
	// 浮動小数点系のエラーを受けるようにする。
	int value = 0x270;
	__asm {
		fnclex;
		fldcw	value
	}
}
EXTERN_INLINE int GV_FLOAT_OPE_OFF( void )
{
	int value = 0x27B;
	__asm {
		fnclex;
		fldcw	value
	}
}
#endif

#endif

#ifdef KP_XBOX

#ifndef KP_WINDOWS

#define SCRATCH_ADDRESS		((void*)0x70000000)

#else	// for Windows

#if __SCRATCHPAD_USE_VIRTUAL_ALLOC__
#define SCRATCH_ADDRESS		((void*)0x70000000)		// 仮想メモリ確保による直接参照
#else
#include "libgv.cnf"
extern char *ScratchAddr[SCRATCH_SIZE>>2];
#define SCRATCH_ADDRESS		((void *)ScratchAddr)
#endif

#endif // for Windows

extern void *GV_NoiseBlock;

#else //BP

#include "libgv.cnf"

#if BP_VITA
__thread
#endif
extern unsigned char* BP_ScratchPadGV;

extern void GV_AS_EnableScratchpadFromThisThread();
#define GV_AS_GetScratchpadAddress() (BP_ScratchPadGV)
#define SCRPAD_ADDR	(GV_AS_GetScratchpadAddress()) //BP
#define GV_AS_GetDGLocalWork() ( BP_ScratchPadGV + 16*1024 )

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#undef class
#undef this
#endif

#endif	/* __LIBGV_H__ */
