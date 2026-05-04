//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	message.c
		メッセージシステム管理ルーチン

	1999/07/13 K.Uehara
	$Id: message.c,v 1.1.1.3 2002/11/19 11:42:44 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef PSX2
#include "mgs_type.h"
#endif
#ifdef KP_XBOX
#include "mgsx_type.h"
#endif
#include "libgv.h"
#include "libgv.cnf"

/* ---------------------------------------------------------- */
/*
	メッセージバッファ
	(01/08/23 サイズを変更)
*/

typedef struct {
	GV_MSG messages[ MAX_MESSAGES ];	// 16ByteAlign
	int num;
	int work[ MAX_MESSAGE_PARAM ];
	int *ptr;
	int use;
	int dummy;
} MESSAGE_LIST;

static ALIGN16_PRE MESSAGE_LIST message_list[ 2 ] ALIGN16_POST;
static int which_buffer;

/* ---------------------------------------------------------- */
/*
	内部使用関数
*/

static inline void slide_message_forward( GV_MSG *ptr, int n_ptr )
{
	ptr += n_ptr;

	while( -- n_ptr >= 0 ){
#ifdef BP_PSX2_ASM
		asm volatile( "
				lq $8,-16(%0)
				sq $8,0(%0)
			" :: "r"(ptr) : "$8", "memory" );
#else
		*ptr = *(ptr - 1);
#endif		
		ptr --;
	}
}

static inline void *set_message_param( MESSAGE_LIST *list, int *message, int message_len )
{
	int *p, *res;
	int i;

	if( list->use + message_len >= MAX_MESSAGE_PARAM ){
		return NULL;
	}
	res = p = list->ptr;
	for( i = message_len; i > 0; i-- ){
		*( p ++ ) = *( message ++ );
	}
	list->ptr = p;

	return res;
}

/* ---------------------------------------------------------------------- */
/*
	Message Wakeup処理
*/

#define NC_COM_KILL 1
#define NC_COM_MESG	0

static void name_check( void *this )
{
	GV_ACT *act = ( GV_ACT * )this;
	int *n;

	if( ( act->class & ( GV_CLASS_MESSAGE_KILL | GV_CLASS_WAIT_MESG ) ) == 0 ){
		return;
	}

	for( n = ( int * )SCRPAD_ADDR; ; n += 2 ){
		if( *n == 0 ) break;

		if( *( n + 1 ) == NC_COM_MESG ){
			if( GV_CHECK_WAIT( act, GV_CLASS_WAIT_MESG ) ){
				if( act->name == *n ){
					GV_WakeupActor( act, GV_CLASS_WAIT_MESG );
					break;
				}
			}
		} else if( *( n + 1 ) == NC_COM_KILL ){
			if( GV_CHECK_WAIT( act, GV_CLASS_MESSAGE_KILL ) ){
				if( act->name == *n ){
					GV_DestroyActor( act );
					break;
				}
			}
		}
	}
}

static void GV_MesgWakeupActor( MESSAGE_LIST *list )
{
	int *names = ( int * )SCRPAD_ADDR;
	int num;

	if( ( num = list->num ) > 0 ){
		GV_MSG *msg;
		msg = list->messages;
		for( ;; ){
			int address;

			address = msg->address;
			*names = address;
			names++;
			if( msg->_flag & GV_MSG_ACT_KILL ){
				*names = NC_COM_KILL;
			} else {
				*names = NC_COM_MESG;
			}
			names++;
			
			num -= msg->_len;
			msg += msg->_len;
			if( num <= 0 ){
				break;
			}
		}
		*( names ++ ) = 0;
		GV_CheckActorAll( name_check );
	}
}

/* ---------------------------------------------------------- */
/*
	操作関数
*/

void GV_InitMessageSystem( void )
{
	/* 初期化 */

	message_list[ 0 ].num = 0;
	message_list[ 1 ].num = 0;
	which_buffer = 0;
}

void GV_ClearMessageSystem( void )
{
	MESSAGE_LIST *list;

	/* 各フレームの頭で呼び出される。ダブルバッファのクリア */

#ifdef DEBUG_MODE
	{
		int num;
		/* RECEIVEされていないMESSAGEを表示 */
		list = message_list + which_buffer;
		if( ( num = list->num ) > 0 ){
			GV_MSG *msg;

			msg = list->messages;

			for( ;; ){
				if( !( msg->_flag & ( GV_MSG_RECEIVED | GV_MSG_ACT_KILL ) ) ){
//					printf( "WARNING : MESSAGE FAIL %X %X\n", msg->address
//						   , msg->message[ 0 ] );
				}
				num -= msg->_len;
				msg += msg->_len;
				if( num <= 0 ){
					break;
				}
			}
		}
	}
#endif

	list = message_list + which_buffer;
	list->num = 0;
	list->ptr = list->work;
	list->use = 0;

	which_buffer = 1 - which_buffer;

	GV_MesgWakeupActor( message_list + which_buffer );
}

int GV_SendMessage( GV_MSG *send )
{
	/* メッセージをリストに登録 */
	MESSAGE_LIST *list;
	int num;

	/*
		バッファサイズチェック
	*/
	{
		list = message_list + ( 1 - which_buffer );
		num = list->num;
		if( num >= MAX_MESSAGES ){
			printf( "message bufffer full\n" );
			GV_ERROR( GV_ERROR_MESSAGE_FULL );
			return -1;
		}
		list->num = num + 1;
	}
	/*
		宛先別に連続するようにメッセージを並べる。
		宛先の最後に追加する
	*/
	{
		GV_MSG *msg;
		int address, length, flag;
		int *param;

		param = set_message_param( list, send->message, send->message_len );
		if( param == NULL ){
			/* 登録せずに終了 */
			printf( "message param full\n" );
			GV_ERROR( GV_ERROR_MESSAGE_FULL );
			list->num = num;
			return -1;
		}
		msg = list->messages;
		address = send->address;
		ASSERT( address != 0 ) ;
		length = 0;
		flag = 0;
		for( ; num > 0; num -- ){
			if( msg->address == address ){
				length = msg->_len;
				flag = msg->_flag;
				/* それ以降のMESGを下にスライド */
				slide_message_forward( msg, num );
				break;
			}
			msg ++;
		}
		msg->address = address;
		msg->message_len = send->message_len;
		msg->message = param;
		msg->_len = length + 1;
		msg->_flag = flag;
		if( param[ 0 ] == GV_MESSAGE_KILL ){
			msg->_flag |= GV_MSG_ACT_KILL;
		}
	}
	return 0;
}

static inline int msg_check( int madd, int cadd )
{
	if( ( madd & 0xFF000000 ) == 0 ){
		if( madd == ( cadd & 0x00FFFFFF ) ){
			return 1;
		}
	} else if( madd == cadd ){
		return 1;
	}
	return 0;
}
	
int GV_ReceiveMessage( int address, GV_MSG **msg_ptr )
{
	MESSAGE_LIST *list;
	GV_MSG *msg;
	int num;

	if( GV_PauseLevel != 0 ) return 0;
		/* なんらかのポーズ状態ではメッセージを受け取れない */

	list = message_list + which_buffer;
	if( ( num = list->num ) == 0 ) return 0;
	msg = list->messages;

	/*
		address宛のメッセージを検索
		存在すれば受信メッセージ数を返す
	*/

	if( ( address & 0xFF000000 ) == 0 ){
		// 受け取り手が番号なし
		while( -- num >= 0 ){
			if( msg->address == address ){
				*msg_ptr = msg;
				msg->_flag = GV_MSG_RECEIVED;
				return msg->_len;
			}
			msg ++;
		}
	} else {
		// 受け取り手が番号あり
//		GV_MSG *buf = ( void * )SCRPAD_ADDR;
		static ALIGN16_PRE GV_MSG buf[ MAX_MESSAGES ] ALIGN16_POST;
		GV_MSG *top = buf;
		int len = 0;

		while( num > 0 ){
			if( msg_check( msg->address, address ) ){
				msg->_flag = GV_MSG_RECEIVED;
				memcpy( top, msg, sizeof( GV_MSG ) * msg->_len );
				top += msg->_len;
				len += msg->_len;
			}
			num -= msg->_len;
			msg += msg->_len;
		}
		if( len > 0 ){
			*msg_ptr = buf;
			return len;
		}
	}
	return 0;
}

