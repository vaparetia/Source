/*
	ストリーミング字幕コントロールドライバ

	2000/09/11 K.Uehara
	$Id: codeccap.c,v 1.1.1.3 2002/11/19 11:45:06 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<eekernel.h>
#include	<eeregs.h>
#include	<libgraph.h>

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"stream.h"
#include	"g_define.h"

#include	"strctrl.h"
#include	"font.h"

#define MAX_DATA_SIZE	4096

/* ---------------------------------------------------------------------- */
/*
	字幕データの構造
*/

typedef struct {
	int start_count;
	int end_count;
	int name;
	int len;
	char string[ 0 ];	// len;
} CAPTION;

typedef struct {
	GV_ACT_EX actor;

	GM_STREAM_CONTROL *ctrl;
	int type;

	int status;
	CAPTION *caption;
	void *font_top;

	void *jimaku_work;	// test;

	unsigned char data[ MAX_DATA_SIZE ];
} Work;

enum {
	STAT_START_WAIT,
	STAT_DISPLAY,
};


static Work * now_work = NULL;



static void init_caption( Work *work )
{
	int size;

	size = *( int * )( work->data );
	//	printf( "cap size = %d\n", size );
	work->caption = ( CAPTION * )( work->data + sizeof( int ) );
	work->font_top = ( void * )work->caption + size;
	font_set_top_addr( FONT_TYPE_VOX, work->font_top + sizeof( int ) );
	work->status = STAT_START_WAIT;
}

static void next_caption( Work *work )
{
	work->caption = ( CAPTION * )( ( void * )work->caption + work->caption->len );
//printf( "next %d %d\n", work->caption->start_count, work->caption->end_count );
	if( ( void * )work->caption >= work->font_top ){
//printf( "caption end\n" );
		work->caption = NULL;
	}
}

static void Act( Work *work )
{
	/* 字幕パケットの取得 */
GET:
	if( work->caption == NULL ){
		/* 取得処理 */
		void *data;
		void *stream_h = work->ctrl->stream_h;

		if( ( data = FS_StreamGetData( stream_h, work->type ) ) != NULL ){
			int size;
			size = FS_StreamGetSize( stream_h, data );
			memcpy( work->data, data, size );
			FS_StreamFreeData( stream_h, data );

			init_caption( work );
		} else if( GM_IS_STREAM_END( work->ctrl ) ){
			GV_DestroyActor( work );
			return;
		}
	}
	if( work->caption != NULL ){
		/* 表示処理 */
		switch( work->status ){
		  case STAT_START_WAIT:
			/* 表示開始待ち */
			if( work->caption->start_count <= work->ctrl->tick ){
//				printf( "SHOW %d %d\n", work->caption->start_count, work->caption->end_count );
//				printf( "tick = %d\n", work->ctrl->tick );
//				dump( work->caption->string, strlen( work->caption->string ) );

				work->status = STAT_DISPLAY;
//				work->jimaku_work = NewJimakuDirect( work->caption->string );
			}
			break;
		  case STAT_DISPLAY:
			/* 表示中 */
			if( work->caption->end_count <= work->ctrl->tick ){

//				GV_DestroyActor( work->jimaku_work );
//				work->jimaku_work = NULL;

//				printf( "END\n" );
				work->status = STAT_START_WAIT;
				next_caption( work );
			}
			break;
		}
		if( work->caption == NULL ){
			goto GET;
		}
	}
}

static void Die( Work *work )
{
//	if( work->jimaku_work != NULL ){
//		GV_DestroyActor( work->jimaku_work );
//	}
	if(work == now_work) now_work = NULL;
}

/* 現在の表示ステータスを得る */
int GM_GetCodecCapStatus(void)
{
  Work * work = now_work;
  int ret = 0;
  if(NULL != work)
    if(work->caption != NULL) 
      switch(work->status)
	{
	case STAT_START_WAIT: ret = 0;  break;
	case STAT_DISPLAY:    ret = 1;  break;
	}

  return ret;
}

void *NewStreamCodecCapDriver( GM_STREAM_CONTROL *ctrl, int type )
{
	Work *work;

	if( ( work = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 0xf2 ) ) != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );

		work->ctrl = ctrl;
		work->caption = NULL;
		work->type = type;
		work->jimaku_work = NULL;
	}
	now_work = work;
	return work;
}

/* ---------------------------------------------------------------------- */
/*
	ドライバ登録 
*/

static GM_STREAM_DRIVER driver = {
	driver: NewStreamCodecCapDriver,
};

int GM_StreamCodecCaptionDriverInit( void )
{
	GM_StreamAddDriver( &driver, CHANK_TYPE_CODEC_CAP );
	return 0;
}
