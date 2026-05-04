//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
        voice_test.c
	ストリーミング再生のテスト

	2000/07/11 Y.Kira
	$Id: voice_test.c,v 1.1.1.3 2002/11/19 11:43:54 Yoshizawa1 Exp $

*/
/*

シナリオ呼び出しインターフェイス
chara 顔アニメ実験キャラ[NewTestFaceAnimationSet] $s:name \
	-chara	$s:キャラクタ名
mesg 顔アニメ実験キャラ モーション再生[0] $s:モーション名 $w:補間時間
// 実験サンプル（マルチウェイトエンベロープモデル顔アニメキャラ）
// メッセージによりコントロールする（時間は全て1/300秒単位）
// ＜モーション再生コマンド捕捉＞
//  モーション名にはrmtファイルのファイル名を指定
//  


プログラム呼び出しインターフェイス
	void *StreamVoiceTest( int name, int pos );
	int			name ;		キャラ名
	int                     pos ;           tsymbol値

	


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"


/*
 * 各種定数等定義
 */
extern int BP_BASE_TICK(void);
#define TIME_BASE   (BP_BASE_TICK())   /* NTSC and PAL での 1 フレームに相当する時間 */

typedef struct {
  GV_ACT_EX  actor;
  int        name;
  int        pos;
  int        handle;
  int        status;
} Work;


static void Act( Work *work )
{
  if(work->handle >= 0)
    {
      work->status = GM_StreamStatus(work->handle);
      if(work->status == GM_STREAM_STATE_END)
	work->handle = -1;
    }

  /* ボタン押下で再生開始(実験用実装) */
  if(work->handle < 0 && GV_PadData[1].press & PAD_R2)
    {
      printf("Stream voice Play start!!\n");
      work->handle = GM_VoxStream(work->pos, 0);  /* ストリーム再生の開始 */
    }
}

static void Die( Work * work )
{
}

/* -------------------------------------------------------------------------
 * シナリオインタフェース
 * ------------------------------------------------------------------------- */
void * StreamVoiceTest(int name, int map)
{
  Work  * work;
  int pos;

  printf("entering StreamVoiceTest()\n");
  OPERATOR();

  pos = GCL_GetOptionValue( 't', -1 );
  printf("StreamVoiceTest(): pos = %d\n", pos);

  work = (Work *)GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof(Work), 0xf4);
  if(NULL == work) return NULL;

  /* 初期化処理 */
  work->name = name;
  work->pos = pos;  /* pos の値を保存する */
  work->handle = -1;
  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);
 
  return work;
}
