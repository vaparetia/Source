//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	codecitp.c
		無線／解釈部
	2000/07/17	K.Uehara
	$Id: codecitp.c,v 1.1.1.3 2002/11/19 11:45:00 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "gameheader.h"
#include "codec_config.h"
#include "codec.h"

#include "libfs.h"
#include "font.h"
#include "mts.h"

#include "cdc_face.h"
#include "cdc_load.h"
#include "cdc_mind.h"
#include "memcall.h"

#define _codecitp_c_
#include "cdc_hair.h"

#include "bp_threading.h"

#ifdef PSX2
#ifndef _DEBUG_
#define DBG(...)
#else
#define DBG(...)  printf(__VA_ARGS__)
#endif
#endif
#ifdef KP_XBOX
#define DBG
#endif

#define DEF_SPAN_TIME  60
#define DEF_SPAN_BIAS  10


int cdcFaceCamera(int side,
		  float far, float zoom,
		  float heading, float pan,
		  float pitch, float gain);

int cdcTraceCameraSwitch(int side, int sw);

int cdcFaceLimit(int side, float zoom,
		 float right, float left, float upper, float lower);

int cdcFaceControlSpeed(int side, int spin_delay, int zoom_delay);

/*
	GCLのフォーマットとして記録されているスクリプトを解釈し,
	表示部にリクエストを送る。
*/

static CODEC_ITP_PARAM *itp_param;


static char *select_tmp[10];  /* 最大 10 項目まで(実際は多分無理) */
static int   bug_models[10];  /* 最大 10 個までのモデルを入れ換える(バグり) */


/* ---------------------------------------------------------------------- */
/*
	表示部へのリクエスト関数
*/

static void ReqMesgAndWait( char *mesg, int who )
{
	itp_param->request = CODEC_REQ_MESG_WAIT;
	itp_param->req[ 0 ] = who;
	itp_param->req[ 1 ] = ( int )mesg;

	BP_SleepThread();

	DBG("ReqMesgAndWait() is finish.\n");
}

static void ReqVoxStart( int vox )
{
  printf("[[REQ]] Vox Start\n");
	itp_param->request = CODEC_REQ_VOX_START;
	itp_param->req[ 0 ] = vox;

	BP_SleepThread();
}

static void ReqMovieStart( int vox )
{
  printf("[[REQ]] Movie Start\n");
	itp_param->request = CODEC_REQ_MOVIE_START;
	itp_param->req[ 0 ] = vox;

	BP_SleepThread();
}

/* ---------------------------------------------------------------------- */
/*
	コマンド関数
*/

static int MovieCmd(char * top )
{
  int mov;

  mov = GCL_GetNextInt();
  ReqMovieStart(mov);

  {
    /* 次にかかれているブロックを実行 */
    int type, value;
    GCL_GetNextValue( GCL_NextStr(), &type, &value );
    ASSERT( type == GCL_BLOCK );
    
    GCL_ExecBlock( ( char * )value, NULL );
  }
  
  return GCL_OK;
}

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


static int TalkCmd( char *top )
{
	int who, face;

	who = GCL_GetNextInt();
	face = GCL_GetNextInt();

	if( GCL_GetOption( 'm' ) != NULL ){
		while( GCL_NextStr() != NULL ){
			char *mes;
			mes = GCL_GetNextString();
			ReqMesgAndWait( mes, who );
		}
	}
 DBG("TalkCmd() is success.\n");
	return GCL_OK;
}

static int BlockCmd( char *top )
{
  int command, block, freq, face;

  command = GCL_GetNextInt();
  DBG( "command = %x\n", command );
  block = GCL_GetNextInt();
  DBG( "block = %x\n", block );
  freq = GCL_GetNextInt();
  DBG( "freq = %d\n", freq );
  face = GCL_GetNextInt();
  DBG( "face = %X\n", face );

  DBG("face file = 0x%08x\n", face);
  ASSERT(0 != face);

  itp_param->request = CODEC_REQ_FACE_LOAD;
  itp_param->req[0] = face;

  BP_SleepThread();
  DBG("BlockCmd\n");
  {
    int type, value;
    GCL_GetNextValue( GCL_NextStr(), &type, &value );
    ASSERT( type == GCL_BLOCK );
    GCL_ExecBlock( ( char * )value, NULL );
  }
  return GCL_OK;
}


/* 表示顔変更コマンド */
static int FaceCmd(char * top)
{
  int side, evm_id, spk_id;

  DBG("[CODEC] entering FaceCmd()...\n");

  side   = GCL_GetNextInt(); /* 表示窓サイド    */
  evm_id = GCL_GetNextInt(); /* EVM モデルの ID */
  spk_id = GCL_GetNextInt(); /* 話者 ID         */

  itp_param->request = CODEC_REQ_FACE_SET;
  itp_param->req[0] = side;
  itp_param->req[1] = evm_id;
  itp_param->req[2] = spk_id;

  BP_SleepThread();
  DBG("FaceCmd\n");

  return GCL_OK;
}

/* 髪の毛 */
static int HairCmd(char * top)
{
  int side;
  int sample_num = -1;
  int hit_param = -1;
  int ambient = 0;
  int model = -1;
  int bound = -1;
  int nowind = 0;

  /* 表示サイド */
  side = GCL_GetNextInt();
  if(NULL != GCL_GetOption('n')) model      = GCL_GetNextInt();
  if(NULL != GCL_GetOption('d')) sample_num = GCL_GetNextInt();
  if(NULL != GCL_GetOption('b')) bound      = GCL_GetNextInt();
  if(NULL != GCL_GetOption('o')) hit_param  = GCL_GetNextInt();
  if(NULL != GCL_GetOption('q')) ambient    = GCL_GetNextInt();
  if(NULL != GCL_GetOption('W')) nowind     = 1;

  itp_param->request = CODEC_REQ_HAIR_SET;
  itp_param->req[0]  = side;
  itp_param->req[1]  = model;
  itp_param->req[2]  = sample_num;
  itp_param->req[3]  = nowind;

  cdcHair_bound     = bound;
  cdcHair_hit_param = hit_param;
  cdcHair_ambient   = ambient;
 
  BP_SleepThread();
  DBG("HairCmd\n");

  return GCL_OK;
}

/* その他付属物 */
static int ObjectCmd(char * top)
{
  int side;
  int model;
  int spk_id;
  int obj_num;
  int skel_num;

  side     = GCL_GetNextInt(); /* 表示サイド                             */
  model    = GCL_GetNextInt(); /* モデル名                               */
  spk_id   = GCL_GetNextInt(); /* 話者名(モーションを持たせるために使用) */
  obj_num  = GCL_GetNextInt(); /* オブジェクト番号                       */
  skel_num = GCL_GetNextInt(); /* 基準となる位置となる関節の番号         */

  printf("side: %d model: %08x spk_id: %08x obj_num:%d, skel_num: %d\n",
	 side, model, spk_id, obj_num, skel_num);

  itp_param->request = CODEC_REQ_OBJECT_SET;
  itp_param->req[0]  = side;
  itp_param->req[1]  = model;

  /* オブジェクトに与えるモーションを識別するために使用する */ 
  itp_param->req[2]  = spk_id + 2 + obj_num;

  itp_param->req[3]  = skel_num;

  BP_SleepThread();

  return GCL_OK;
}

/* キャンセル時モーション設定 */
static int CancelMtnCmd(char * top)
{
  int side, mtn_id, mftime;

  DBG("[CODEC] entering CancelMtnCmd()...\n");
  side = GCL_GetNextInt();     /* 表示サイド             */
  mtn_id = GCL_GetNextInt();   /* モーションデータ ID    */
  mftime = GCL_GetNextInt();   /* 補間時間 (1/300秒単位) */

  /*
   * モーションの設定部
   */

  return GCL_OK;
}

/* 光源色設定(主光源) */
static int LightColCmd(char * top)
{
  int side, r, g, b;

  DBG("[CODEC] entering LightColCmd()...\n");
  side = GCL_GetNextInt();   /* 表示サイド */
  r    = GCL_GetNextInt();   /* R          */
  g    = GCL_GetNextInt();   /* G          */
  b    = GCL_GetNextInt();   /* b          */

  if(codecSetFaceLightColor(side, r, g, b)) return GCL_ERROR;
  return GCL_OK;
}

/* Ambient 色設定 */
static int AmbientColCmd(char * top)
{
  int side, r, g, b;

  DBG("[CODEC] entering AmbientColCmd()...\n");
  side = GCL_GetNextInt();   /* 表示サイド */
  r    = GCL_GetNextInt();   /* R          */
  g    = GCL_GetNextInt();   /* G          */
  b    = GCL_GetNextInt();   /* b          */

  if(codecSetFaceAmbient(side, r, g, b)) return GCL_ERROR;
  return GCL_OK;
}

/* 光源方向設定 */
static int LightVecCmd(char * top)
{
  int side, x, y, z;

  DBG("[CODEC] entering LightVecCmd()...\n");
  side = GCL_GetNextInt();   /* 表示サイド */
  x    = GCL_GetNextInt();   /* X          */
  y    = GCL_GetNextInt();   /* Y          */
  z    = GCL_GetNextInt();   /* Z          */
  DBG("side[%d]  (%d, %d, %d)\n", side, x, y, z);

  if(codecSetFaceLightVector(side, (float)x, (float)y, (float)z))
    return GCL_ERROR;

  return GCL_OK;
}

static int DefMtnCmd(char * top)
{
  int side, motion;

  DBG("codecitp.c: DefMtnCmd()\n");
  side = GCL_GetNextInt();
  motion = GCL_GetNextInt();
  DBG("--<1>------\n");

  itp_param->request = CODEC_REQ_DEF_MTN_SET;
  itp_param->req[0] = side;
  itp_param->req[1] = motion;

  DBG("--<2>------\n");

  BP_SleepThread();
  DBG("--<3>------\n");
  return GCL_OK;
}

static int MindVoiceCmd(char * top)
{
  int vox_list[4];
  int side;
  int i;

  side = GCL_GetNextInt();
  for(i = 0; i < 4; i++) vox_list[i] = GCL_GetNextInt();
  CDC_SetupMindVoice(side, vox_list);
  DBG("Setup MindVoice\n");
  return GCL_OK;
}

static int RandCmd(char * top)
{
  int rnd_max;

  rnd_max = GCL_GetNextInt();
  
  GCL_Status = ( BP_PS2_rand() >> 16 ) % rnd_max;

  DBG("RAND = %d\n", GCL_Status);

  return 0;
}

static int PreFreqCmd(char * top)
{
  GCL_Status = codecGetLastResponsedFreq();
  DBG("Pre Freq = %d\n", GCL_Status);
  return 0;
  
}
static int SetFreqCmd(char * top)
{
  int freq;

  freq = GCL_GetNextInt();

  itp_param->request = CODEC_REQ_FREQ_SET;
  itp_param->req[0]  = freq;

  BP_SleepThread();

  return 0;
}


static int CallCountCmd(char * top)
{
  GCL_Status = codecGetResponseCount();
  DBG("Call Count = %d\n", GCL_Status);
  return 0;
  
}

static int DispEnableCmd(char * top)
{
  itp_param->request = CODEC_REQ_DISP_CTRL;
  itp_param->req[0] = 1;
  BP_SleepThread();
  return GCL_OK;
}

static int DispDisableCmd(char * top)
{
  itp_param->request = CODEC_REQ_DISP_CTRL;
  itp_param->req[0] = 0;
  BP_SleepThread();
  return GCL_OK;
}

static int FdExitCmd(char * top)
{
  return GCL_OK;
}

/* 顔カメラアングル設定 */
static int FaceCamera(char * top)
{
  int side, i;
  int param[6];
  float zfar, zoom;
  float heading, pan;
  float gain, pitch;

  /* 基本的にカメラの注視点は、頭の位置(HUMAN21_ATAMA)を基準に決定される。*/
  side    = GCL_GetNextInt();    /* 表示顔サイド   */

  for(i = 0; i < 6; i++)
    param[i] = GCL_GetNextInt();

  zfar     = (float)param[0] / 1000.0F;    /* カメラ距離     */
  zoom    = (float)param[1] / 1000.0F;    /* カメラズーム値 */

  /* カメラ左右パラメータ */
  heading = (float)param[2];              /* 左右カメラ角   */
  pan     = (float)param[3] / 1000.0F;    /* 左右平行移動量 */
  
  /* カメラ上下パラメータ */
  pitch   = (float)param[4];              /* 上下カメラ角   */
  gain    = (float)param[5] / 1000.0F;    /* 上下平行移動量 */

  /* heading と pitch の値を、2π = 4096 から [rad] に変換 */
  heading = (heading * 2.0F * (float)M_PI) / 4096.0F;
  pitch   = (pitch * 2.0F * (float)M_PI) / 4096.0F;
  printf("heading = %8.3f   pitch = %8.3f\n", heading, pitch);

  /* カメラ設定 */
  cdcFaceCamera(side, zfar, zoom, heading, pan, pitch, gain);
  
  return GCL_OK;
}

/*
 * 顔操作リミッタ設定
 */
static int FaceLimit(char * top)
{
  int side, i;
  int param[5];
  float zoom, right, left, upper, lower;

  side = GCL_GetNextInt();  /* 操作対象の顔 */
  for(i = 0; i < 5; i++)
    param[i] = GCL_GetNextInt();  /* パラメータ取得 */

  zoom = (float)param[0] / 1000;
  right = ((float)param[1] * (float)M_PI) / 2048.0F;
  left  = ((float)param[2] * (float)M_PI) / 2048.0F;
  upper = ((float)param[3] * (float)M_PI) / 2048.0F;
  lower = ((float)param[4] * (float)M_PI) / 2048.0F;

  cdcFaceLimit(side, zoom, right, left, upper, lower);

  return GCL_OK;
}

static int FaceDelay(char * top)
{
  int side;
  int spin_delay;
  int zoom_delay;

  side = GCL_GetNextInt();
  spin_delay = GCL_GetNextInt();
  zoom_delay = GCL_GetNextInt();
  cdcFaceControlSpeed(side, spin_delay, zoom_delay);

  return GCL_OK;
}



/* メモリー呼び出しリスト追加 */
static int MemRegist(char * top)
{
  int area;  /* 登録箇所 */
  int freq;  /* 周波数(通信中の周波数を使うか?) */
  int name_id;

  area = GCL_GetNextInt();  /* 登録箇所         */
  name_id = GCL_GetNextInt();
  freq = GCL_GetNextInt();  /* 登録周波数       */
  MemCallSetName(area, freq, name_id);

#if 0
    if(NULL != GCL_NextStr())
      name = GCL_GetNextString();  /* 名前文字列リソース */
    MemCallSetName(area, freq, name);
#endif

  return GCL_OK;
}

/* 登録済メモリー呼出 指定項目周波数取得 */
static int GetMemFreq(char * top)
{
  int area = GCL_GetNextInt();  /* エリア番号を得る */

  GCL_Status = MemCallGetFreq(area);

  return GCL_OK;
}

/* プレイヤー名登録 */
static int PlayerName(char * top)
{
  int area = GCL_GetNextInt();  /* プレイヤ名として登録する文字列の StrCode */
  
  itp_param->request = CODEC_REQ_SET_PLAYER;
  itp_param->req[ 0 ] = area;
  
  BP_SleepThread();

  return GCL_OK;
}

/* 心の声許可 */
static int MindEnable(char * top)
{
  itp_param->request = CODEC_REQ_MIND_MASK;
  itp_param->req[0] = 0;  /* 0 で許可 */
  BP_SleepThread();
  return GCL_OK;
}

/* 心の声禁止 */
static int MindDisable(char * top)
{
  itp_param->request = CODEC_REQ_MIND_MASK;
  itp_param->req[0] = 1;  /* 非0 で禁止 */
  BP_SleepThread();
  return GCL_OK;
}

/* メニュー選択 */
static int SelMenu(char * top)
{
  int i;
  itp_param->request = CODEC_REQ_SELECT;
  itp_param->req[0] = GCL_GetNextInt();   /* 項目数 */
  itp_param->req[1] = 0;   /* デフォルトで選択されている項目 */

  /* デフォルトの項目が指定されている場合 */
  if(NULL != GCL_GetOption('s'))
    itp_param->req[1] = GCL_GetNextInt();

  if(NULL != GCL_GetOption('m'))
    {
      for(i = 0; i < itp_param->req[0]; i++)
	select_tmp[i] = GCL_GetNextString();
      
      itp_param->req[2] = (int)select_tmp;
      
      BP_SleepThread();
      
      if(!itp_param->result)
	GCL_Status = itp_param->res[0];
      else
	GCL_Status = -1;

      printf("Select answer = %d\n", GCL_Status);
    }
  
  return GCL_OK;
}

/* セーブモード起動 */
static int SaveMode(char * top)
{
  itp_param->request = CODEC_REQ_SAVE_MODE;
  BP_SleepThread();
  GCL_Status = itp_param->result;
  printf("<CMD> savemode: GCL_Status = %d\n", GCL_Status);
  return GCL_OK;
}


static int WaitCmd(char * top)
{
  int t;

  t = GCL_GetNextInt();    /* 待ち時間 (1/300) 単位 */
  itp_param->request = CODEC_REQ_TIME_WAIT;
  itp_param->req[0]  = t;
  BP_SleepThread();
  return GCL_OK;
}

static int VibrationCmd(char * top)
{
  int type;
  int v_time;

  type = GCL_GetNextInt();   /* 振動タイプの取得 */
  v_time = 0;
  if(NULL != GCL_GetOption('t'))
    v_time = GCL_GetNextInt();

  itp_param->request = CODEC_REQ_VIBRATION;
  itp_param->req[0] = type;
  itp_param->req[1] = v_time;
  BP_SleepThread();
  return GCL_OK;
}

static int EarthQuakeCmd(char * top)
{
  int sw;
  int q_time;
  int amp;

  sw = GCL_GetNextInt();     /* 地震スイッチの値 */

  q_time = 0;
  if(NULL != GCL_GetOption('t'))
    q_time = GCL_GetNextInt();   /* 地震の持続時間 */

  amp = 0;
  if(NULL != GCL_GetOption('a'))
    amp = GCL_GetNextInt();      /* 地震の振幅 */

  itp_param->request = CODEC_REQ_EARTH_Q;
  itp_param->req[0] = sw;
  itp_param->req[1] = q_time;
  itp_param->req[2] = amp;
  BP_SleepThread();
  return GCL_OK;
}

/*
 * バグり時に、顔が突如入れ替わったりする表示にする
 */
static int BugFaceCmd(char * top)
{
  int side;    /* 表示サイド */
  int spk_id;  /* 話者 ID    */
  int span_time; /* 最低間隔 */
  int span_bias; /* 誤差値   */
  int cnt;     /* モデルの数 */
  char * pos;

  side   = GCL_GetNextInt();   /* 表示サイドの取得 */
  spk_id = GCL_GetNextInt();   /* 話者 ID の取得   */

  span_time = DEF_SPAN_TIME;
  span_bias = DEF_SPAN_BIAS;

  /*
   * 時間間隔の設定
   */
  if(NULL != GCL_GetOption('s'))
    {
      span_time = GCL_GetNextInt();
      span_bias = GCL_GetNextInt();
    }

  cnt = 0;
  if(NULL != GCL_GetOption('f'))
    for(cnt = 0; NULL != (pos = GCL_NextStr()); cnt++)
      bug_models[cnt] = GCL_GetInt(pos);
  bug_models[cnt] = -1;

  itp_param->request = CODEC_REQ_BUG_FACE;
  itp_param->req[0] = ((side & 0xff) << 24) | (spk_id & 0xffffff);
  itp_param->req[1] = (int)bug_models;  /* 顔モデルのリスト。-1 で終了  */
  itp_param->req[2] = span_time;        /* 切替え間隔 */
  itp_param->req[3] = span_bias;        /* 間隔にランダムに上乗せする
					   時間の最大 */
  BP_SleepThread();
  return GCL_OK;
}


/*
 * 任意ノイズ発生コマンド
 */
static int NoiseCmd(char * top)
{
  int side;
  int sw;
  int t;

  side = GCL_GetNextInt();   /* ノイズを操作するサイド */
  sw   = GCL_GetNextInt();   /* ノイズの ON/OFF        */
  t = 0;

  if(NULL != GCL_GetOption('t'))
    t = GCL_GetNextInt();

  itp_param->request = CODEC_REQ_NOISE;
  itp_param->req[0] = side;
  itp_param->req[1] = sw;
  itp_param->req[2] = t;

  BP_SleepThread();

  return GCL_OK;
}

static int CloseEyeMotion(char * top)
{
  int side;
  int name;

  side = GCL_GetNextInt();   /* 表示サイド         */
  name = GCL_GetNextInt();   /* 目閉じモーション名 */

  /* 目閉じモーションを設定すると、自動的に目パチを行う…はず */
  codecSetEyeCloseMotion(side, name);

  return GCL_OK;
}

static int VibPlay(char * top)
{
  int vib_name;

  vib_name = GCL_GetNextInt();  /* 振動名称 */
  itp_param->request = CODEC_REQ_VIBPLAY;
  itp_param->req[0] = vib_name;

  BP_SleepThread();

  return GCL_OK;
}

static int BugEyes(char * top)
{
  int side, sw;

  side = GCL_GetNextInt();  /* 適用サイド */
  sw = GCL_GetNextInt();    /* 許可 / 禁止フラグ */
  itp_param->request = CODEC_REQ_BUGEYES;
  itp_param->req[0] = side;
  itp_param->req[1] = sw;
  BP_SleepThread();

  return GCL_OK;
}

static int TraceCamera(char * top)
{
  int side, sw;

  side = GCL_GetNextInt();  /* 適用サイド */
  sw = GCL_GetNextInt();    /* 許可 / 禁止フラグ */
  cdcTraceCameraSwitch(side, sw);
  return GCL_OK;
}

   
/* ---------------------------------------------------------------------- */
/*
	解釈部メイン
*/

#define ITP_PRI	61

static GCL_COMMANDLIST Commands[] = {
  { 0x0057c8d1, BlockCmd },
  { 0x003B91EB, TalkCmd },

  { 0x00c89dee, MindEnable },    /* minden: mind voice enable             */
  { 0x00c89dd3, MindDisable },   /* mindds: mind voice disbale            */

  { 0x0009658c, MovieCmd },      /* movie: IPU stream movie play          */
  { 0x00409c51, SelMenu },       /* selectmenu: select menu               */
  { 0x008bfa17, SaveMode },      /* savemode: evoke save mode             */

  { 0x00f271f8, PlayerName },    /* player: set player name               */
  { 0x003490c5, FaceCmd },       /* face: change face model               */
  { 0x00359192, HairCmd },       /* hair: option hair model               */
  { 0x0056a1b8, ObjectCmd },     /* object: other object model(ex:glasses)*/

  { 0x00ede73a, CancelMtnCmd },  /* cancelmtn: set motion at cancel       */
  { 0x00bf1560, LightColCmd },   /* lightcol: set main lighting color     */
  { 0x00abc569, AmbientColCmd }, /* ambientcol: set ambient color         */
  { 0x00bf6017, LightVecCmd },   /* lightvec:   set main lighting vector  */
  { 0x00099515, MindVoiceCmd },  /* mindvoice:  set mind voice set        */

  { 0x006c54ba, FaceCamera },    /* facecamera: set face camera angle     */
  { 0x00ec568b, TraceCamera },   /* tracecamera: trace camera switch      */
  { 0x005f6325, FaceLimit },     /* facelimit: set face angle spin limit  */
  { 0x00dd5e29, FaceDelay },     /* facedelay: set face control delay time */

  { 0x00f576fe, MemRegist },     /* memregist: regist memory call list    */
  { 0x004d912f, GetMemFreq },    /* getmemfreq: get registed freq.        */

  { 0x003a9224, RandCmd },       /* rand: randome                         */
  { 0x0084f1fb, PreFreqCmd },    /* prefreq: get precall frequency        */
  { 0x0074f2a2, SetFreqCmd },    /* setfreq: set display frequency        */
  { 0x00cf8612, CallCountCmd },  /* callcount: get responsed call counter */
  { 0x0001E658, VoxCmd },	 /* vox: vox stream                       */

  { 0x0084c3bc, DefMtnCmd },     /* defmtn: setup default motion          */
  { 0x00cb4ddc, DispEnableCmd},  /* dispen: face display enable.          */
  { 0x00cb4dc1, DispDisableCmd}, /* dispds: face display disable.         */

  { 0x003d1194, WaitCmd },       /* wait: time wait                       */

  { 0x00c513b8, VibrationCmd },  /* vib: vibration switch                 */
  { 0x004ade41, EarthQuakeCmd }, /* earthq: screen earth quake            */

  { 0x00a4aa35, BugFaceCmd },    /* bugface; bug face                     */

  { 0x001932cc, NoiseCmd },      /* noise: noise command                  */

  { 0x00746e66, FdExitCmd },     /* fdexit: exit codec mode with fadeout  */
  { 0x006b11d5, CloseEyeMotion },/* eyeclose: set eye close motion        */
  { 0x0059daf1, VibPlay },       /* vibplay : vibration file play         */
  { 0x00a48a83, BugEyes },       /* bugeyes: bug eyes                     */
};

static GCL_COMMANDDEF codec_commands = GCL_COMMANDS( Commands );

static void ItpMain( int id, void *arg )
{
	CODEC_ITP_PARAM *param = arg;
	GCL_ENVIRONMENT env;

	itp_param = param;

	/*
		初期化
	*/

   printf( "Codec ItpMain: id: %d\n", id );
	GCL_SaveCurrentEnvironment( &env );
	GCL_LoadScript( param->data );
	GCL_AddCommMulti( &codec_commands );
	font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop() );

	param->request = CODEC_REQ_NOREQ;
	

	GCL_ExecScript();
	/*
		終了処理
	*/

	GCL_DelCommMulti( &codec_commands );
	GCL_RestoreEnvironment( &env );
	font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop() );

	param->request = CODEC_REQ_END;
}

int CODEC_ItpInit( CODEC_ITP_PARAM *param )
{
	param->request = -1;
	param->id = MTS_NewThread( "CODECITP", ItpMain, ITP_PRI,
				   param->stack, param->stacksize, param );

	return 0;
}
