//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ストリーミング字幕コントロールドライバ

	2000/09/11 K.Uehara
	$Id: caption.c,v 1.1.1.3 2002/11/19 11:41:46 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<eekernel.h>
#include	<eeregs.h>
#include	<libgraph.h>
#include	<string.h>
#endif

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"libgcl.h"
#include	"stream.h"
#include	"g_define.h"
#include	"linkvar.h"

#include	"strctrl.h"
#include	"font.h"
#include	"jimaku.h"

#include "BP_BuildDefines.h"
#include "BP_EndianSupport.h"

#ifdef	PSX2
typedef char byte;
#endif

#include "BP_FileSupport.h"

#define MAX_DATA_SIZE	(64*1024) //BP increased from 5K to 64K

int GM_StreamCaptionCurrentName;
int GM_StreamCaptionCurrentHandler;

/* ---------------------------------------------------------------------- */
/*
	字幕データの構造
*/

typedef struct {
	int start_count;
	int end_count;
	int name;
#if BP_USE_NEW_FONT_SYSTEM()
   short len;
   short lang;
#else
	int len;
#endif
	char string[ 0 ];	// len;
} CAPTION;

typedef struct {
	int start_count;
	int name;
	int value;
	int option_plus_lang;
} ACTION;

static void EndianSwapCaption( CAPTION *caption )
{
   BP_LE_SwapSInt_Inp( &caption->start_count );
   BP_LE_SwapSInt_Inp( &caption->end_count );
   BP_LE_SwapSInt_Inp( &caption->name );
#if BP_USE_NEW_FONT_SYSTEM()
   BP_LE_SwapSShort_Inp( &caption->len );
   BP_LE_SwapSShort_Inp( &caption->lang );
#else
   BP_LE_SwapSInt_Inp( &caption->len );
#endif
}

static void EndianSwapAction( ACTION *action )
{
   BP_LE_SwapSInt_Inp( &action->start_count );
   BP_LE_SwapSInt_Inp( &action->name );
   BP_LE_SwapSInt_Inp( &action->value );
   BP_LE_SwapSInt_Inp( &action->option_plus_lang );
}

typedef struct _caption_Work {
	GV_ACT_EX actor;
	int type;
	GM_STREAM_CONTROL *ctrl;

	int status;
	CAPTION *caption;
	CAPTION *caption_end;
	ACTION *action;
	void *font_top;

#if BP_USE_NEW_FONT_SYSTEM()
	int end_count;
   short *level;
   int level_now_tick;

	unsigned char* data;
#else
	unsigned char data[ MAX_DATA_SIZE ];
#endif
} Work;

enum {
	STAT_START_WAIT,
	STAT_DISPLAY,
};

static Work *caption_work = NULL;

static void BP_FixupCaption( CAPTION *pCaption, const int top_pos );

static void swap_captions( Work *work )
{
   CAPTION *c = work->caption;

   while ( c != work->caption_end )
   {
      EndianSwapCaption( c );
      BP_FixupCaption(c, work->ctrl->top_pos);

      c = ( CAPTION * )( ( char * )c + c->len );
   }
}

static void swap_actions( Work *work )
{
   ACTION *a = work->action;

   while ( (char *) a < (char *) work->font_top )
   {
      EndianSwapAction( a );
      ++a;
   }
}

static void next_caption( Work *work )
{
#if BP_USE_NEW_FONT_SYSTEM()
   do {
      work->caption = ( CAPTION * )( ( char * )work->caption + work->caption->len );
      //printf( "next %d %d\n", work->caption->start_count, work->caption->end_count );
      if( work->caption >= work->caption_end ){
         //printf( "caption end\n" );
         work->caption = NULL;
         return;
      }
   } while( work->caption->lang != GM_Language );
#else
   work->caption = ( CAPTION * )( ( byte * )work->caption + work->caption->len );
   //printf( "next %d %d\n", work->caption->start_count, work->caption->end_count );
   if( work->caption >= work->caption_end ){
      //printf( "caption end\n" );
      work->caption = NULL;
   }
#endif
}

static void init_caption( Work *work, int offset )
{
   int is_bp_cap;
	int size;

   unsigned char* currentPtr = work->data;

   is_bp_cap = (BP_LE_SwapSInt( *(int*)(currentPtr) ) == 'BCAP') ? 1 : 0;

   if( is_bp_cap )
   {
      currentPtr += 4;
   }

   size = BP_LE_SwapSInt( *( int * )( currentPtr ) );
//printf( "CAPTION OFFSET = %d\n", offset );
//printf( "cap size = %d\n", size );
	work->caption = ( CAPTION * )( currentPtr + sizeof( int ) );
	work->font_top = ( char * )work->caption + size;

#if BP_USE_NEW_FONT_SYSTEM()
   //This stream driver should not be servicing old format caption packets.
   //The new code is not compatible with the MGS2 packets.
   XASSERT( is_bp_cap, "Only BP caption data supported!" );

   work->level = NULL;
   work->action = NULL;
   work->level_now_tick = 0;
   if( offset != 0 ){
      int ofs_base;
      int ofs_level;

      ofs_base = ( offset & 0xFFFF );
      if( ( ofs_level = ( offset >> 16 ) ) > 0 ){
         work->level = ( char * )work->font_top - ofs_base - ofs_level;
      }
      if( ofs_base > 0 ){
         work->action = ( ACTION* )(( char * )work->font_top - ofs_base );
      }
      work->caption_end = ( char * )work->font_top - ofs_base - ofs_level;
   } else {
      work->caption_end = work->font_top;
   }

   swap_captions( work );
   if ( work->action )
   {
      swap_actions( work );
   }

   if( work->caption == work->caption_end ){
      work->caption = NULL;
   } else {
      if( work->caption->lang != GM_Language ){
         next_caption( work );
      }
   }

   if( !is_bp_cap )
      font_set_top_addr( FONT_TYPE_VOX, ( char *) work->font_top + sizeof( int ) );
#else
   XASSERT( !is_bp_cap, "Only non-BP caption data supported!" );
	if( offset > 0 ){
		work->action = ( ACTION* )(( byte * )work->caption + size - offset);
		work->caption_end = ( void * )work->action;
	} else {
		work->action = NULL;
		work->caption_end = work->font_top;
	}

   swap_captions( work );
   if ( work->action )
   {
      swap_actions( work );
   }

   if( work->caption == work->caption_end ){
		work->caption = NULL;
	}
	font_set_top_addr( FONT_TYPE_VOX, ( byte *) work->font_top + sizeof( int ) );
	work->status = STAT_START_WAIT;
#endif
}

extern char* BP_GetOverrideString(char* inputString);

static void Act( Work *work )
{
	/* 字幕パケットの取得 */
GET:
	if( work->caption == NULL && work->action == NULL ){
		/* 取得処理 */
		void *data;
		void *stream_h = work->ctrl->stream_h;

		if( ( data = FS_StreamGetData( stream_h, work->type ) ) != NULL ){
			int size;
			size = FS_StreamGetSize( stream_h, data );
         XASSERT(size < MAX_DATA_SIZE, "Caption data too big!");
			memcpy( work->data, data, size );
			init_caption( work, FS_STREAM_GET_OPTION( data ) );

			FS_StreamFreeData( stream_h, data );
		} else if( GM_IS_STREAM_END( work->ctrl )
#if BP_USE_NEW_FONT_SYSTEM()
         && ( work->status == STAT_START_WAIT )
#endif
         )
      {
			GV_DestroyActor( work );
			return;
		}
	}
#if BP_USE_NEW_FONT_SYSTEM()
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

				GM_StreamCaptionCurrentHandler = work->ctrl->id;
				GM_StreamCaptionCurrentName = work->caption->name;

				if( !( GM_Configuration & GM_CONFIG_CAPTION_OFF ) ){
					GM_JimakuShow( work->ctrl->id, BP_GetOverrideString(work->caption->string) );
				}
			}
			break;
		  case STAT_DISPLAY:
			/* 表示中 */
			if( work->caption->end_count <= work->ctrl->tick ){

				GM_JimakuHide();

//				printf( "END\n" );
				work->status = STAT_START_WAIT;

				GM_StreamCaptionCurrentHandler = work->ctrl->id;
				GM_StreamCaptionCurrentName = 0;

				next_caption( work );
			}
			break;
		}
		if( work->caption == NULL ){
			goto GET;
		}
	}
#else
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

				GM_StreamCaptionCurrentHandler = work->ctrl->id;
				GM_StreamCaptionCurrentName = work->caption->name;

				if( !( GM_Configuration & GM_CONFIG_CAPTION_OFF ) ){
					GM_JimakuShow( work->ctrl->id, work->caption->string );
				}
			}
			break;
		  case STAT_DISPLAY:
			/* 表示中 */
			if( work->caption->end_count <= work->ctrl->tick ){

				GM_JimakuHide();

//				printf( "END\n" );
				work->status = STAT_START_WAIT;

				GM_StreamCaptionCurrentHandler = work->ctrl->id;
				GM_StreamCaptionCurrentName = 0;

				next_caption( work );
			}
			break;
		}
		if( work->caption == NULL ){
			goto GET;
		}
	}
#endif

	if( work->action != NULL ){
		// アクション処理
		if( work->action->start_count <= work->ctrl->tick ){
			int proc;
			proc = STR_ACTION_PROC( work->ctrl );
			if( proc != 0 ){
				GCL_ARGS arg;
				int argv[ 3 ];
				argv[ 0 ] = work->ctrl->id;
				argv[ 1 ] = work->action->name;
				argv[ 2 ] = work->action->value;
				arg.argc = 3;
				arg.argv = argv;
				GCL_ExecProc( proc, &arg );
			}
         {
            do 
            {
               work->action++;
               if ((void *)work->action >= work->font_top)
               {
                  work->action = NULL;
               }
               else
               {
                  int lang = (work->action->option_plus_lang >> 24);
                  if (lang == GM_LANG_DEFAULT || lang == GM_Language)
                  {
                     break;
                  }
               }
            }
            while (work->action != NULL);
         }
		}
	}

#if BP_USE_NEW_FONT_SYSTEM()
   if( work->level != NULL ){
      // work->level は、1/20 単位。
      if( work->level_now_tick + abs( *work->level ) * 15 <= work->ctrl->tick ){
         work->level_now_tick += abs( *work->level ) * 15;
         work->level++;
         if( *work->level == 0 ){
            work->level = NULL;
         }
      }
   }
#endif
}

static void Die( Work *work )
{
	GM_StreamCaptionCurrentHandler = 0;
	GM_StreamCaptionCurrentName = 0;

	if( work->status == STAT_DISPLAY ){
		GM_JimakuHide();
	}

	if( caption_work == work ){
		caption_work = NULL;
	}

#if BP_USE_NEW_FONT_SYSTEM()
   if( work->data )
      free(work->data);
#endif
}

void *NewStreamCaptionDriver( GM_STREAM_CONTROL *ctrl, int type )
{
	Work *work;

	if( ( type & 0xFFFF0000 ) != 0 ){
#if BP_USE_NEW_FONT_SYSTEM()
      //BP - with new font system, ignore all language-specific CAPTION packets.
      //Our new CAPTION packets have language 0.
      return NULL;
#else
		/* 多言語対応 */
		if( ( type >> 16 ) != GM_Language ){
			return NULL;
		}
#endif
	}

   // AS - On Vita due to some sort of difference in timing, the johnny directional mic scene loads the two vox 
   // streams in the opposite order of the other platforms. caption_prio is set to 128 for all captions instead of 
   // being set in data so this hack checks for the stream that overrides the captions and prevents it. I preferred this
   // hack to minimize any side effects since it only effects this particular circumstance.
   {
      extern const char* BP_FindStreamName(const int);
      const char* const pStreamName = BP_FindStreamName(ctrl->top_pos);

      if (pStreamName)
      {
         if (strstr(pStreamName, "vc121101") && caption_work != NULL)
         {
            return NULL; // Do not allow this stream's caption to override the current one.
         }
      }
   }

	if( caption_work != NULL ){
printf( "dup_caption %X %X\n", ctrl->caption_prio, caption_work->ctrl->caption_prio );
		if( ctrl->caption_prio >= caption_work->ctrl->caption_prio ){
			// 優先度が高いので前のドライバをダミーに変更
printf( "set dummy\n" );
			GV_ChangeActFunc( caption_work, GM_StreamDummyAct );
			caption_work = NULL;
		} else {
printf( "no set\n" );
			return NULL;
		}
	}

	if( ( work = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM, sizeof( Work ), 0xf2 ) ) != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );

		work->ctrl = ctrl;
		work->caption = NULL;
		work->type = type;
#if BP_USE_NEW_FONT_SYSTEM()
      work->data = malloc(MAX_DATA_SIZE);
#endif

		caption_work = work;
	}
	return work;
}

/* ---------------------------------------------------------------------- */
/*
	ドライバ登録 
*/

static GM_STREAM_DRIVER driver = {
#ifdef __GNUC__	
	driver: NewStreamCaptionDriver,
#else
	NULL, 0, NewStreamCaptionDriver,
#endif	
};

int GM_StreamCaptionDriverInit( void )
{
	GM_JimakuDaemonStart();
	GM_StreamAddDriver( &driver, CHANK_TYPE_CAPTION );
	caption_work = NULL;
	return 0;
}

/* ---------------------------------------------------------------------- */
// BP - support for caption timing fixups that could not be safely integrated
// into data in time for ship.
// Applies to code-only patch build only! (though it should be harmless in a build
// where the data has already been fixed)

typedef struct _SBPRadioCapOverride
{
   const char * const   mStreamName;
   int                  mLanguage;
   int                  mOrgBegin;
   int                  mOrgEnd;
   int                  mNewBegin;
   int                  mNewEnd;
} SBPRadioCapOverride;

static const SBPRadioCapOverride skRadioCapOverrides_jp[] =
{
   { "t02a2D", 7,   7537,   7899,   7537,   8261 },
   { "t02a2D", 7,   7913,   8261,   8261,   8261 },
};

static const SBPRadioCapOverride skRadioCapOverrides_us[] =
{
   { "dummy", -1,   -1,   -1,   -1,   -1 },
};

static void BP_FixupCaption( CAPTION *pCaption, const int top_pos )
{
#if defined(BP_360) || defined(BP_PS3)

   const SBPRadioCapOverride * const overrides = BP_Area_JP() ? skRadioCapOverrides_jp : skRadioCapOverrides_us;
   const int overridesCount = BP_Area_JP() ? ( sizeof(skRadioCapOverrides_jp)/sizeof(*skRadioCapOverrides_jp) ) : ( sizeof(skRadioCapOverrides_us)/sizeof(*skRadioCapOverrides_us) );

   {
      const char * const fullPath = BP_FindStreamName( top_pos );

      char streamName[FILENAME_MAX];

      const SBPRadioCapOverride * pOverride = overrides;

      strcpy( streamName, strrchr( fullPath, '/' ) + 1 );
      *(strstr( streamName, ".sdt" )) = 0;

      for( pOverride = overrides; pOverride != overrides+overridesCount; ++pOverride )
      {
         if(   !strcmp( pOverride->mStreamName, streamName )
            && pOverride->mLanguage == GM_Language
            && pCaption->start_count == pOverride->mOrgBegin
            && pCaption->end_count == pOverride->mOrgEnd
            )
         {
            pCaption->start_count = pOverride->mNewBegin;
            pCaption->end_count = pOverride->mNewEnd;
         }
      }
   }
#endif
}

