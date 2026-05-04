/*
	codecitp.c
		無線／解釈部
	2000/07/17	K.Uehara
	$Id: codecitp.c,v 1.1.1.3 2002/11/19 11:45:06 Yoshizawa1 Exp $
*/
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

#include "gameheader.h"
#include "codec_config.h"
#include "codec.h"

#include "libfs.h"
#include "font.h"
#include "mts.h"

#include "cdc_face.h"
#include "cdc_load.h"
#include "cdc_mind.h"

#define _codecitp_c_
#include "cdc_hair.h"

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)  printf(args)
#endif

int cdcFaceCamera(int side,
		  float far, float zoom,
		  float heading, float pan,
		  float pitch, float gain);

int cdcFaceLimit(int side, float zoom,
		 float right, float left, float upper, float lower);

int cdcFaceControlSpeed(int side, int spin_delay, int zoom_delay);

int MemcallGetRes(int num);
int MemcallGetFreq(int num);
/*
	GCLのフォーマットとして記録されているスクリプトを解釈し,
	表示部にリクエストを送る。
*/

static CODEC_ITP_PARAM *itp_param;

/* ---------------------------------------------------------------------- */
/*
	表示部へのリクエスト関数
*/

static void ReqMesgAndWait( char *mesg )
{
	itp_param->request = CODEC_REQ_MESG_WAIT;
	itp_param->req[ 0 ] = ( int )mesg;

	SleepThread();

	DBG("ReqMesgAndWait() is finish.\n");
}

static void ReqVoxStart( int vox )
{
  printf("[[REQ]] Vox Start\n");
	itp_param->request = CODEC_REQ_VOX_START;
	itp_param->req[ 0 ] = vox;

	SleepThread();
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
 DBG("TalkCmd() is success.\n");
	return GCL_OK;
}

static int BlockCmd( char *top )
{
  int command, block, freq, face;
  void * face_data;

  command = GCL_GetNextInt();
  DBG( "command = %x\n", command );
  block = GCL_GetNextInt();
  DBG( "block = %x\n", block );
  freq = GCL_GetNextInt();
  DBG( "freq = %d\n", freq );
  face = GCL_GetNextInt();
  DBG( "face = %X\n", face );

  itp_param->request = CODEC_REQ_FACE_LOAD;
  itp_param->req[0] = face;

  SleepThread();
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
  
  SleepThread();
  DBG("FaceCmd\n");

  return GCL_OK;
}

/* 髪の毛 */
static int HairCmd(char * top)
{
  int side, tmp;
  int sample_num = -1;
  int hit_param = -1;
  int ambient = 0;
  int model = -1;
  int bound = -1;
  
  /* 表示サイド */
  side = GCL_GetNextInt();
  if(NULL != GCL_GetOption('n')) model      = GCL_GetNextInt();
  if(NULL != GCL_GetOption('d')) sample_num = GCL_GetNextInt();
  if(NULL != GCL_GetOption('b')) bound      = GCL_GetNextInt();
  if(NULL != GCL_GetOption('o')) hit_param  = GCL_GetNextInt();
  if(NULL != GCL_GetOption('q')) ambient    = GCL_GetNextInt();


  itp_param->request = CODEC_REQ_HAIR_SET;
  itp_param->req[0]  = side;
  itp_param->req[1]  = model;
  itp_param->req[2]  = sample_num;

  cdcHair_bound     = bound;
  cdcHair_hit_param = hit_param;
  cdcHair_ambient   = ambient;
 
  SleepThread();
  DBG("HairCmd\n");

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

  SleepThread();
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
  
  GCL_Status = ( rand() >> 16 ) % rnd_max;

  DBG("RAND = %d\n", GCL_Status);

  return 0;
}

static int PreFreqCmd(char * top)
{
  GCL_Status = codecGetLastResponsedFreq();
  DBG("Pre Freq = %d\n", GCL_Status);
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
  SleepThread();
  return GCL_OK;
}

static int DispDisableCmd(char * top)
{
  itp_param->request = CODEC_REQ_DISP_CTRL;
  itp_param->req[0] = 0;
  SleepThread();
  return GCL_OK;
}

static int FdExitCmd(char * top)
{
}

/* 顔カメラアングル設定 */
static int FaceCamera(char * top)
{
  int side, i;
  int param[6];
  float far, zoom;
  float heading, pan;
  float gain, pitch;

  /* 基本的にカメラの注視点は、頭の位置(HUMAN21_ATAMA)を基準に決定される。*/
  side    = GCL_GetNextInt();    /* 表示顔サイド   */

  for(i = 0; i < 6; i++)
    param[i] = GCL_GetNextInt();

  far     = (float)param[0] / 1000.0F;    /* カメラ距離     */
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
  cdcFaceCamera(side, far, zoom, heading, pan, pitch, gain);
  
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
  int res;   /* 名前のリソース番号 */
  int freq;  /* 周波数(通信中の周波数を使うか?) */

  area = GCL_GetNextInt();  /* 登録箇所         */
  res  = GCL_GetNextInt();  /* 名前リソース番号 */
  freq = GCL_GetNextInt();  /* 登録周波数       */

  MemcallSetName(area, res, freq);

  return GCL_OK;
}


/* 登録済メモリー呼出 指定項目人名リソース番号取得 */
static int GetMemRes(char * top)
{
  int area = GCL_GetNextInt();  /* エリア番号を得る */
  GCL_Status = MemcallGetRes(area);
  return GCL_OK;
}

/* 登録済メモリー呼出 指定項目周波数取得 */
static int GetMemFreq(char * top)
{
  int area = GCL_GetNextInt();  /* エリア番号を得る */

  GCL_Status = MemcallGetFreq(area);

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

  { 0x003490c5, FaceCmd },       /* face: change face model               */
  { 0x00359192, HairCmd },       /* hair: option hair model               */

  { 0x00ede73a, CancelMtnCmd },  /* cancelmtn: set motion at cancel       */
  { 0x00bf1560, LightColCmd },   /* lightcol: set main lighting color     */
  { 0x00abc569, AmbientColCmd }, /* ambientcol: set ambient color         */
  { 0x00bf6017, LightVecCmd },   /* lightvec:   set main lighting vector  */
  { 0x00099515, MindVoiceCmd },  /* mindvoice:  set mind voice set        */

  { 0x006c54ba, FaceCamera },    /* facecamera: set face camera angle     */
  { 0x005f6325, FaceLimit },     /* facelimit: set face angle spin limit  */
  { 0x00dd5e29, FaceDelay },     /* facedelay: set face control delay time */

  { 0x00f576fe, MemRegist },     /* memregist: regist memory call list    */
  { 0x00f29af3, GetMemRes },     /* getmemres: get registed name resource */
  { 0x004d912f, GetMemFreq },    /* getmemfreq: get registed freq.        */

  { 0x003a9224, RandCmd },       /* rand: randome                         */
  { 0x0084f1fb, PreFreqCmd },    /* prefreq: get precall frequency        */
  { 0x00cf8612, CallCountCmd },  /* callcount: get responsed call counter */
  { 0x0001E658, VoxCmd },	 /* vox: vox stream                       */

  { 0x0084c3bc, DefMtnCmd },     /* defmtn: setup default motion          */
  { 0x00cb4ddc, DispEnableCmd},  /* dispen: face display enable.          */
  { 0x00cb4dc1, DispDisableCmd}, /* dispds: face display disable.         */

  { 0x00746e66, FdExitCmd },     /* fdexit: exit codec mode with fadeout  */
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
	param->id = MTS_NewThread( "CODECITP", ItpMain, ITP_PRI
							   , param->stack, param->stacksize, param );

	return 0;
}
