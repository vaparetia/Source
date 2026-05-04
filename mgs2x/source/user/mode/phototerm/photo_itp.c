//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  photo_itp.c
  写真評価インタプリタ

  2001/05/08  Y.Kira
  $Id: photo_itp.c,v 1.1.1.3 2002/11/19 11:45:19 Yoshizawa1 Exp $
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

#include "gameheader.h"
#include "g_struct.h"
#include "g_define.h"
#include "libdg.h"
#include "font.h"
#include "libfs.h"

#define  _photo_itp_c_
#include "photo_itp.h"
#include "photo_syntax.h"

#include "bp_threading.h"

#include "mts.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"

#ifdef DEBUG
#define DBG   printf
#else
#define DBG
#endif /* DEBUG */


static PHOTO_ITP_PARAM  * itp_param;

/* ---------------------------------------------------------------------- */
/*
	表示部へのリクエスト関数
*/

static void ReqMesgAndWait( char *mesg )
{
  printf("mesg = %p\n", mesg);
  itp_param->request = PHOTO_REQ_MESG_WAIT;
  itp_param->req[ 0 ] = ( int )mesg;
  
  BP_SleepThread();
}

static void ReqVoxStart( int vox )
{
  printf("[[REQ]] Vox Start: %x\n", vox);
  itp_param->request = PHOTO_REQ_VOX_START;
  itp_param->req[ 0 ] = vox;
  
  BP_SleepThread();
}

/* ---------------------------------------------------------------------- */
/*
	コマンド関数
*/

static int VoxCmd( char *top )
{
  /* 音声ストリーム再生要求 */
  int vox;
  
  vox = GCL_GetNextInt();
  
  ReqVoxStart( vox );
  
  {
    /* 次にかかれているブロックを実行 */
    int type, value;
    GCL_GetNextValue( GCL_NextStr(), &type, &value );
    ASSERT( type == GCL_BLOCK );
    
    GCL_ExecBlock( ( char * )value, NULL );
  }
  
  return GCL_OK;
}

/*
 * メッセージの表示
 */
static int TalkCmd( char *top )
{
  int who, face;
  
  who = GCL_GetNextInt();
  face = GCL_GetNextInt();
  
  if( GCL_GetOption( 'm' ) != NULL ){
    while( GCL_NextStr() != NULL ){
      char *mes;
      mes = GCL_GetNextString();
      ReqMesgAndWait( mes );
    }
  }
  return GCL_OK;
}

/*
 * リピート命令
 */
static int RepeatCmd(char * top)
{
  int ret;
  int type, value;
  
  GCL_GetNextValue( GCL_NextStr(), &type, &value);
  ASSERT(type == GCL_BLOCK);
  do {
    ret = GCL_ExecBlock((char *)value, NULL);
  } while(ret);

  return GCL_OK;
}

/*
 * 写真評価の入口
 */
static int BlockCmd(char * top)
{
  int command, block;

  command = GCL_GetNextInt();
  DBG("command = %x\n", command);

  block   = GCL_GetNextInt();   /* $s:写真ブロック */
  DBG("block = %x\n", block);

  /* 無線と違って顔データは無い */
  {
    int type, value;

    GCL_GetNextValue(GCL_NextStr(), &type, &value);
    ASSERT(type == GCL_BLOCK);
    GCL_ExecBlock((char *)value, NULL);
  }

  return GCL_OK;
}

/*
 * 表示コマ番号の指定
 */
static int PhotoFrame(char * top)
{
  int frame_num;

  frame_num = GCL_GetNextInt();   /* コマ番号 */
  itp_param->request = PHOTO_REQ_SET_FRAME;
  itp_param->req[0] = frame_num;
  BP_SleepThread();

  return GCL_OK;
}


static int EraseCmd(char * top)
{
  itp_param->request= PHOTO_REQ_SET_FRAME;
  itp_param->req[0] = -1;   /* 負の値であれば、画像の消去 */
  BP_SleepThread();
  return GCL_OK;
}


/*
 * 表示されているコマ番号の取得
 */
static int NowFrame(char * top)
{
  itp_param->request = PHOTO_REQ_GET_FRAME;
  BP_SleepThread();

  GCL_Status = (PHOTO_ACK == itp_param->result) ? itp_param->res[0] : -1;
  return GCL_OK;
}

/*
 * 乱数の発生
 */
static int RandCmd(char * top)
{
  int rnd_max;

  rnd_max = GCL_GetNextInt();
  GCL_Status = (BP_PS2_rand() >> 16) % rnd_max;

  DBG("RAND = %d\n", GCL_Status);

  return GCL_OK;
}

/*
 * パペットをロードし、表示する
 */
static int LoadPuppet(char * top)
{
  int layout, initact;

  layout  = GCL_GetNextInt();
  initact = GCL_GetNextInt();
  itp_param->request = PHOTO_REQ_LOAD_PUPPET;
  itp_param->req[0] = layout;
  itp_param->req[1] = initact;
  BP_SleepThread();
  return GCL_OK;
}

/*
 * パペットのデフォルトアクション指定
 */
static int DefAction(char * top)
{
  int action, tired, tired_time;

  action = GCL_GetNextInt();     /* アクション名     */

  
  tired = -1;
  tired_time = 0;
  if(NULL != GCL_GetOption('t'))
    {
      tired  = GCL_GetNextInt();     /* 疲れアクション名 */
      tired_time = GCL_GetNextInt(); /* 疲れるまでの時間 */
    }

  itp_param->request = PHOTO_REQ_SET_DEFACT;
  itp_param->req[0]  = action;
  itp_param->req[1]  = tired;
  itp_param->req[2]  = tired_time;

  BP_SleepThread();

  return GCL_OK;
}

/*
 * パペットアニメを指定する
 */
static int Puppet(char * top)
{
  int action;
  int a_time;
  int sound;

  action = GCL_GetNextInt();
  a_time = -1;
  sound = -1;
  if(NULL != GCL_GetOption('t')) a_time = GCL_GetNextInt();
  if(NULL != GCL_GetOption('s')) sound = GCL_GetNextInt();

  if((a_time < 0) && (NULL != GCL_GetOption('l'))) a_time = 0;

  itp_param->request = PHOTO_REQ_SET_ACT;
  itp_param->req[0]  = action;
  itp_param->req[1]  = a_time;
  itp_param->req[2]  = sound;  /* SE 同時発生 */

  BP_SleepThread();

  return GCL_OK;
}

static int KeyWait(char * top)
{
  itp_param->request = PHOTO_REQ_KEY_WAIT;
  BP_SleepThread();
  return GCL_OK;
}

static int WaitCmd(char * top)
{
  int t;

  t = GCL_GetNextInt();   /* 待ち時間 */
  itp_param->request = PHOTO_REQ_TIME_WAIT;
  itp_param->req[0] = t;
  BP_SleepThread();
  return GCL_OK;
}

static int PhotoSave(char * top)
{
  int num;

  num = GCL_GetNextInt();  /* 何枚目をセーブするかを得る */

  itp_param->request = PHOTO_REQ_PHOTO_SAVE;
  itp_param->req[0] = num;
  BP_SleepThread();
  return GCL_OK;
}

static int PhotoSE(char * top)
{
  int num;

  num = GCL_GetNextInt();   /* SE 番号を取得 */
  itp_param->request = PHOTO_REQ_SOUND;
  itp_param->req[0] = num;
  BP_SleepThread();
  return GCL_OK;
}

/* ------------------------------------------------------------------------ */
#define ITP_PRI	61

/*
 * 命令と処理関数の対応表
 */
static GCL_COMMANDLIST Commands[] = {
  { ITP_block,      BlockCmd },   /* block   : begin block              */
  { ITP_talk,       TalkCmd },    /* talk    : talk                     */
  { ITP_repeat,     RepeatCmd },  /* repeat  : repeat block             */
  { ITP_vox,        VoxCmd },     /* vox     : vox stream               */
  { ITP_photoframe, PhotoFrame }, /* photoframe: set photo frame number */
  { ITP_nowframe,   NowFrame },   /* nowframe: now frame number         */
  { ITP_rand,       RandCmd },    /* rand    : random                   */
  { ITP_defaction,  DefAction },  /* defaction: puppet default action   */
  { ITP_puppet,     Puppet },     /* puppet  : puppet animation         */
  { ITP_loadpuppet, LoadPuppet }, /* loadpuppet: load puppet            */
  { ITP_keywait,    KeyWait },    /* keywait : key waiting              */
  { ITP_wait,       WaitCmd },    /* wait    : time wait                */
  { ITP_photoerase, EraseCmd },   /* photoerase: erase picture          */
  { ITP_photosave,  PhotoSave },  /* photosave: save photograph         */
  { ITP_sound,      PhotoSE },    /* sound   : sound effect by script   */
};

/* ------------------------------------------------------------------------ */

static GCL_COMMANDDEF photo_commands = GCL_COMMANDS(Commands);

static void ItpMain(int id, void * arg)
{
  PHOTO_ITP_PARAM * param = arg;
  GCL_ENVIRONMENT env;
  itp_param = param;

  /*
   * 初期化
   */
  printf( "Photo ItpMain: id: %d\n", id );

  GCL_SaveCurrentEnvironment(&env);
  GCL_LoadScript(param->data);
  GCL_AddCommMulti(&photo_commands);
  font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop());

  param->request = PHOTO_REQ_NOREQ;
  GCL_ExecScript();

  /*
   * 終了処理
   */
  GCL_DelCommMulti( &photo_commands );
  GCL_RestoreEnvironment( &env );
  font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop());
  param->request = PHOTO_REQ_END;
}

int PHOTO_ItpInit(PHOTO_ITP_PARAM * param)
{
  param->request = -1;
  param->id = MTS_NewThread("PHOTOITP",  ItpMain, ITP_PRI,
			    param->stack, param->stacksize, param);
  return 0;
}
