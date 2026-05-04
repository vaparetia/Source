//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ストリーミング字幕コントロールドライバ

	2000/09/11 K.Uehara
	$Id: codeccap.c,v 1.1.1.3 2002/11/19 11:45:00 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<eekernel.h>
#include	<eeregs.h>
#include	<libgraph.h>
#endif
#ifdef KP_XBOX
#include <xtl.h>
#endif

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"stream.h"
#include	"g_define.h"

#include	"strctrl.h"
#include	"font.h"
#include	"linkvar.h"

#include "BP_FileSupport.h"

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

typedef struct _codeccap_Work {
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

static void EndianSwapCaption( CAPTION *pCaption )
{
   BP_LE_SwapSInt_Inp( &pCaption->start_count );
   BP_LE_SwapSInt_Inp( &pCaption->end_count );
   BP_LE_SwapSInt_Inp( &pCaption->name );
   BP_LE_SwapSInt_Inp( &pCaption->len );
}

static void BP_FixupCaption( CAPTION *pCaption, const int top_pos );

static void init_caption( Work *work )
{
	int size;

	size = BP_LE_SwapSInt( *( int * )( work->data ) );
	//	printf( "cap size = %d\n", size );
	work->caption = ( CAPTION * )( work->data + sizeof( int ) );
   EndianSwapCaption( work->caption );

   BP_FixupCaption( work->caption, work->ctrl->top_pos );

	work->font_top = ( char * )work->caption + size;
	// font_set_top_addr( FONT_TYPE_VOX, work->font_top + sizeof( int ) );
	work->status = STAT_START_WAIT;
}

static void next_caption( Work *work )
{
	work->caption = ( CAPTION * )( ( char * )work->caption + work->caption->len );
   EndianSwapCaption( work->caption );

   BP_FixupCaption( work->caption, work->ctrl->top_pos );

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

	if( ( type & 0xFFFF0000 ) != 0 ){
		/* 多言語対応 */
		if( ( type >> 16 ) != GM_Language ){
			return NULL;
		}
	}

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
	NULL, 0, NewStreamCodecCapDriver,
};

int GM_StreamCodecCaptionDriverInit( void )
{
	GM_StreamAddDriver( &driver, CHANK_TYPE_CODEC_CAP );
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

#if defined(BP_360)

static const SBPRadioCapOverride skRadioCapOverrides_jp[] =
//static const SBPRadioCapOverride skRadioCapOverride_vc000501_jp[] =
{
   { "vc000501", 7,   2521,   4318,   2521,   4290 },
   { "vc000501", 7,   4722,   6324,   4830,   6180 },
   { "vc000501", 7,   6352,   7118,   6210,   6960 },
//};
//static const SBPRadioCapOverride skRadioCapOverride_vc000601_jp[] =
//{
   { "vc000601", 7,     27,   1127,     27,   1230 },
   { "vc000601", 7,   1156,   1587,   1320,   1680 },
   { "vc000601", 7,   1630,   2145,   1830,   2280 },
   { "vc000601", 7,   2159,   4708,   2310,   4770 },
   { "vc000601", 7,   4820,   5196,   4920,   5310 },
//};
//static const SBPRadioCapOverride skRadioCapOverride_vc212771_jp[] =
//{
   { "vc212771", 7,     13,   1991,     13,   2100 },
   { "vc212771", 7,   2117,   3969,   2340,   4170 },
   { "vc212771", 7,   4082,   5893,   4380,   6180 },
   { "vc212771", 7,   5990,   7438,   6360,   7830 },
   { "vc212771", 7,   7537,   8163,   7980,   8610 },
   { "vc212771", 7,   8275,   9417,   8730,   9840 },
   { "vc212771", 7,   9515,   9710,   9990,  10110 },
   { "vc212771", 7,   9780,   9988,  10260,  10380 },
//};
//static const SBPRadioCapOverride skRadioCapOverride_vc264891_jp[] =
//{
   { "vc264891", 7,     27,   2311,     27,   2100 },
   { "vc264891", 7,   2507,   3858,   2310,   3450 },
   { "vc264891", 7,   3998,   5363,   3630,   4980 },
//};
//static const SBPRadioCapOverride skRadioCapOverride_vc264971_jp[] =
//{
   { "vc264971", 7,     27,   2437,     27,   2232 },
   { "vc264971", 7,   2563,   4234,   2400,   3990 },
//};
//static const SBPRadioCapOverride skRadioCapOverride_vc264771_jp[] =
//{
   { "vc264771", 7,     27,   2228,     27,   1980 },
   { "vc264771", 7,   2340,   3677,   2100,   3420 },
   { "vc264771", 7,   3775,   4819,   3540,   4560 },
   { "vc264771", 7,   4931,   6379,   4710,   6150 },
//};
//static const SBPRadioCapOverride skRadioCapOverride_vc265061_jp[] =
//{
   { "vc265061", 7,     27,   2158,     27,   2010 },
   { "vc265061", 7,   2312,   3858,   2190,   3690 },
   { "vc265061", 7,   3984,   6310,   3870,   6180 },
   { "vc265061", 7,   6450,   8302,   6330,   8148 },
   { "vc265061", 7,   8414,   9946,   8304,   9660 },
   { "vc265061", 7,  10058,  11423,   9840,  11130 },
};

static const SBPRadioCapOverride skRadioCapOverrides_us[] =
//static const SBPRadioCapOverride skRadioCapOverride_vc265061_us[] =
{
   { "vc265061", 1,     13,   1434,     13,   1440 },
   { "vc265061", 1,   1462,   2186,   1530,   2160 },
   { "vc265061", 1,   2229,   3622,   2247,   3480 },
   { "vc265061", 1,   3678,   4458,   3555,   4311 },
   { "vc265061", 1,   4527,   5530,   4380,   5340 },
   { "vc265061", 1,   5586,   6825,   5460,   6660 },
   { "vc265061", 1,   6910,   8428,   6750,   8100 },
   { "vc265061", 1,   8512,   9306,   8235,   8970 },
   { "vc265061", 1,   9390,  10560,   9030,  10140 },
   { "vc265061", 1,  10657,  11506,  10200,  11070 },
   { "vc265061", 3,     13,   1434,     13,   1440 },
   { "vc265061", 3,   1462,   2186,   1530,   2160 },
   { "vc265061", 3,   2229,   3622,   2247,   3480 },
   { "vc265061", 3,   3678,   4458,   3555,   4311 },
   { "vc265061", 3,   4527,   5530,   4380,   5340 },
   { "vc265061", 3,   5586,   6825,   5460,   6660 },
   { "vc265061", 3,   6910,   8428,   6750,   8100 },
   { "vc265061", 3,   8512,   9306,   8235,   8970 },
   { "vc265061", 3,   9390,  10560,   9030,  10140 },
   { "vc265061", 3,  10657,  11506,  10200,  11070 },
   { "vc265061", 2,     13,   1434,     13,   1440 },
   { "vc265061", 2,   1462,   2186,   1530,   2160 },
   { "vc265061", 2,   2229,   3622,   2247,   3480 },
   { "vc265061", 2,   3678,   4458,   3555,   4311 },
   { "vc265061", 2,   4527,   5530,   4380,   5340 },
   { "vc265061", 2,   5586,   6825,   5460,   6660 },
   { "vc265061", 2,   6910,   8428,   6750,   8100 },
   { "vc265061", 2,   8512,   9306,   8235,   8970 },
   { "vc265061", 2,   9390,  10560,   9030,  10140 },
   { "vc265061", 2,  10657,  11506,  10200,  11070 },
   { "vc265061", 4,     13,   1434,     13,   1440 },
   { "vc265061", 4,   1462,   2186,   1530,   2160 },
   { "vc265061", 4,   2229,   3622,   2247,   3480 },
   { "vc265061", 4,   3678,   4458,   3555,   4311 },
   { "vc265061", 4,   4527,   5530,   4380,   5340 },
   { "vc265061", 4,   5586,   6825,   5460,   6660 },
   { "vc265061", 4,   6910,   8428,   6750,   8100 },
   { "vc265061", 4,   8512,   9306,   8235,   8970 },
   { "vc265061", 4,   9390,  10560,   9030,  10140 },
   { "vc265061", 4,  10657,  11506,  10200,  11070 },
   { "vc265061", 5,     13,   1434,     13,   1440 },
   { "vc265061", 5,   1462,   2186,   1530,   2160 },
   { "vc265061", 5,   2229,   3622,   2247,   3480 },
   { "vc265061", 5,   3678,   4458,   3555,   4311 },
   { "vc265061", 5,   4527,   5530,   4380,   5340 },
   { "vc265061", 5,   5586,   6825,   5460,   6660 },
   { "vc265061", 5,   6910,   8428,   6750,   8100 },
   { "vc265061", 5,   8512,   9306,   8235,   8970 },
   { "vc265061", 5,   9390,  10560,   9030,  10140 },
   { "vc265061", 5,  10657,  11506,  10200,  11070 },
};

//end X360

#elif defined(BP_PS3)

static const SBPRadioCapOverride skRadioCapOverrides_jp[] =
//static const SBPRadioCapOverride skRadioCapOverride_vc000501_jp[] =
{
   { "vc000501", 7,   2521,   4318,   2521,   4290 },
   { "vc000501", 7,   4722,   6324,   4830,   6180 },
   { "vc000501", 7,   6352,   7118,   6210,   6960 },
//};
//static const SBPRadioCapOverride skRadioCapOverride_vc000601_jp[] =
//{
   { "vc000601", 7,     27,   1127,     27,   1230 },
   { "vc000601", 7,   1156,   1587,   1320,   1680 },
   { "vc000601", 7,   1630,   2145,   1830,   2280 },
   { "vc000601", 7,   2159,   4708,   2310,   4770 },
   { "vc000601", 7,   4820,   5196,   4920,   5310 },
//};
//static const SBPRadioCapOverride skRadioCapOverride_vc212771_jp[] =
//{
   { "vc212771", 7,     13,   1991,     13,   2100 },
   { "vc212771", 7,   2117,   3969,   2340,   4170 },
   { "vc212771", 7,   4082,   5893,   4380,   6180 },
   { "vc212771", 7,   5990,   7438,   6360,   7830 },
   { "vc212771", 7,   7537,   8163,   7980,   8610 },
   { "vc212771", 7,   8275,   9417,   8730,   9840 },
   { "vc212771", 7,   9515,   9710,   9990,  10110 },
   { "vc212771", 7,   9780,   9988,  10260,  10380 },
};

static const SBPRadioCapOverride skRadioCapOverrides_us[] =
//static const SBPRadioCapOverride skRadioCapOverride_vc265061_us[] =
{
   { "vc265061", 1,     13,   1434,     13,   1500 },
   { "vc265061", 1,   1462,   2186,   1539,   2220 },
   { "vc265061", 1,   2229,   3622,   2310,   3540 },
   { "vc265061", 1,   3678,   4458,   3570,   4320 },
   { "vc265061", 1,   4527,   5530,   4440,   5370 },
   { "vc265061", 1,   5586,   6825,   5520,   6690 },
   { "vc265061", 1,   6910,   8428,   6750,   8130 },
   { "vc265061", 1,   8512,   9306,   8250,   9000 },
   { "vc265061", 1,   9390,  10560,   9150,  10290 },
   { "vc265061", 1,  10657,  11506,  10362,  11190 },
   { "vc265061", 3,     13,   1434,     13,   1500 },
   { "vc265061", 3,   1462,   2186,   1539,   2220 },
   { "vc265061", 3,   2229,   3622,   2310,   3540 },
   { "vc265061", 3,   3678,   4458,   3570,   4320 },
   { "vc265061", 3,   4527,   5530,   4440,   5370 },
   { "vc265061", 3,   5586,   6825,   5520,   6690 },
   { "vc265061", 3,   6910,   8428,   6750,   8130 },
   { "vc265061", 3,   8512,   9306,   8250,   9000 },
   { "vc265061", 3,   9390,  10560,   9150,  10290 },
   { "vc265061", 3,  10657,  11506,  10362,  11190 },
   { "vc265061", 2,     13,   1434,     13,   1500 },
   { "vc265061", 2,   1462,   2186,   1539,   2220 },
   { "vc265061", 2,   2229,   3622,   2310,   3540 },
   { "vc265061", 2,   3678,   4458,   3570,   4320 },
   { "vc265061", 2,   4527,   5530,   4440,   5370 },
   { "vc265061", 2,   5586,   6825,   5520,   6690 },
   { "vc265061", 2,   6910,   8428,   6750,   8130 },
   { "vc265061", 2,   8512,   9306,   8250,   9000 },
   { "vc265061", 2,   9390,  10560,   9150,  10290 },
   { "vc265061", 2,  10657,  11506,  10362,  11190 },
   { "vc265061", 4,     13,   1434,     13,   1500 },
   { "vc265061", 4,   1462,   2186,   1539,   2220 },
   { "vc265061", 4,   2229,   3622,   2310,   3540 },
   { "vc265061", 4,   3678,   4458,   3570,   4320 },
   { "vc265061", 4,   4527,   5530,   4440,   5370 },
   { "vc265061", 4,   5586,   6825,   5520,   6690 },
   { "vc265061", 4,   6910,   8428,   6750,   8130 },
   { "vc265061", 4,   8512,   9306,   8250,   9000 },
   { "vc265061", 4,   9390,  10560,   9150,  10290 },
   { "vc265061", 4,  10657,  11506,  10362,  11190 },
   { "vc265061", 5,     13,   1434,     13,   1500 },
   { "vc265061", 5,   1462,   2186,   1539,   2220 },
   { "vc265061", 5,   2229,   3622,   2310,   3540 },
   { "vc265061", 5,   3678,   4458,   3570,   4320 },
   { "vc265061", 5,   4527,   5530,   4440,   5370 },
   { "vc265061", 5,   5586,   6825,   5520,   6690 },
   { "vc265061", 5,   6910,   8428,   6750,   8130 },
   { "vc265061", 5,   8512,   9306,   8250,   9000 },
   { "vc265061", 5,   9390,  10560,   9150,  10290 },
   { "vc265061", 5,  10657,  11506,  10362,  11190 },
};

//end PS3

#endif

static void BP_FixupCaption( CAPTION *pCaption, const int top_pos )
{
#if defined(BP_360) || defined(BP_PS3)
   //BP_CODE_DATA_PATCH
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

