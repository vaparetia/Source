#include	<stdio.h>
#include	<stdlib.h>
#include	<kernel.h>
#include	<sys/types.h>
#include	<sif.h>
#include	<sifcmd.h>
#include	<sifrpc.h>
#include	<timerman.h>
#include	<libsd.h>
#include	"../cdbios/cdbios.h"
#include	"sd_debug.h"
#include	"sd_incl.h"
#include	"sd_ext.h"

#include "mgs_type.h"

#include "BP_SoundSupport.h"
#include "BP_UnifiedSDX.h"

/*---- 定数宣言 -----*/
#define	REVERB_DEPTH 	0x2000

/* 外部宣言 */
extern void	IntSdMain(void);
extern int	id_SdMain;
extern int	LoadSngData(unsigned long code);
extern int	LoadSeFile(void);
extern int	LoadWaveFile(void);
extern void	WaveCdLoad(void);
extern void	init_sng_work(void);
extern void sd_set_status( void );
extern void sd_send_status( void );
extern void str2_load(void);
extern void	str2_int(void);
extern void lnr_load(void);
extern void	lnr_int(void);

extern unsigned int	str2_status[2];
extern unsigned int	lnr8_status;
extern int ac3_fg;

/* プロトタイプ宣言 */
void sd_init (void);
int SdMain (void);
int SdEELoad (void);
int SdSet (void);
int SdInt (void);
void SdTerm (void);

void keyOn (unsigned long ch);
void keyOff (unsigned long  ch);
void KeyOffStr(void);
void KeyOffStr2(void);
int sd_mem_alloc(void);
void StrFadeIn(unsigned long fade_time);

/* ワークエリア定義 */
//int				str_close_fg, str_stop_fg;
//int				str_pause_wait;
//int				sd_debug_mode;

/*---------------------------------------------------------------*/
// サウンド・メインタスク（低優先）
/*---------------------------------------------------------------*/
#if 0 //BP_PS2
int SdMain (void)
{
PRINTF(("StartThread:SdMain\n"));
	while (1) {
		if (pak_read_fg == 0) BP_SleepThread();	//pak_read_fgがonの時、連続読み込みする
		else {
			WaitVblankStart();
			WaitVblankEnd();
		}
/* PAKファイルのロード */
		if (pak_load_status) LoadPakFile();
/* ＳＮＧデータ読み込み */
//2000/10/04 危険曲はロード中も再生を続ける
		if (sng_load_code != 0) LoadSngData(sng_load_code);
//		if (sng_status == 1) {
//			if (LoadSngData(sng_load_code) != 0) sng_status = 0;
//			else sng_status = 2;
//		}
/* 効果音ロード */
		if (se_load_code) {
			LoadSeFile();
		}
/* 波形ロード */
#ifndef MGS
		switch (wave_load_status) {
		case 1:
			if (LoadWaveFile() != 0) wave_load_status = 0;
			else wave_load_status = 2;
			break;
		case 2:
			break;
		case 3:
			WaveCdLoad();
			break;
		}
#endif
/* EEへステータス送信 */
//		sd_set_status();	/*Comment 2000/05/19 SdInt内で行う*/
//		sd_send_status();
	}
	SdTerm();
	BP_ExitThread();
}

/*---------------------------------------------------------------*/
// サウンド・EEからのロードタスク（低優先）
/*---------------------------------------------------------------*/
int SdEELoad (void)
{
PRINTF(("StartThread:SdEELoad\n"));
	while (1) {
		BP_SleepThread();
/* ストリーミング・ロード */
		if (lnr8_status) lnr_load();
		if (str2_status[0] || str2_status[1]) str2_load();
	}
	BP_ExitThread();
}

/*---------------------------------------------------------------*/
// サウンド割り込みタスク（高優先）
/*---------------------------------------------------------------*/
//int pad_k_fg;
int	send_timer = 0;
int	sd_int_sw = 0;	//2.5mSEC割り込み対応（AC-3ストリーミング以外は、5mSEC処理にする）
int SdInt (void)
{
/* SPU2を初期化する */
	sd_init();
PRINTF(("StartThread:SdInt\n"));
	while (1) {
		BP_SleepThread();

		lnr_int();			/* STR 3 (LNR8/AC-3) 再生 */
		if (sd_int_sw) {
			IntSdMain();	/* SNG/SE再生 */
			str2_int();		/* STR 1/2 再生 */
			sd_set_status();/* EEへステータス送信 */
			sd_send_status();
		}
		++sd_int_sw;
		sd_int_sw &= 1;
/*-----------------------------------------------------------------------*/
	}
	BP_ExitThread();
}
#endif

extern void LoadPakFile();

#if defined(BP_VITA)
void LoadPakFileVita()
{
   extern unsigned int pak_sng_code;

   if (!pak_load_status)
      return;

   int result = BP_LoadPakFileFromArchive(pak_load_code);
   if (result)
   {
      pak_load_status = PAK_LOAD_STATUS_NONE;
      bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_NONE;
      pak_sng_code = 0;
      sng_load_code = 0;
   }
   else
   {
      pak_load_status = PAK_LOAD_UNIFIED;
      pak_sng_code = 0x010000ff; // dummy value to tell the sound system we are loading song data.
   }
}
#endif

void BP_SdMainLoopIteration()
{
//   if (pak_read_fg == 0) return;	//pak_read_fgがonの時、連続読み込みする

   /* PAKファイルのロード */
#if defined(BP_VITA)
   LoadPakFileVita();
   return;
#else
   if (pak_load_status) 
   {
      LoadPakFile();
   }
#endif

   /* ＳＮＧデータ読み込み */
   //2000/10/04 危険曲はロード中も再生を続ける
   if (sng_load_code != 0) 
   {
     LoadSngData(sng_load_code);
   }

   //		if (sng_status == 1) {
   //			if (LoadSngData(sng_load_code) != 0) sng_status = 0;
   //			else sng_status = 2;
   //		}
   /* 効果音ロード */
   if (se_load_code) {
      LoadSeFile();
   }

   /* 波形ロード */
#ifndef MGS
   if( wave_load_status )
   {
      printf("BP - wave_load_status %d\n", wave_load_status );
      switch (wave_load_status)
      {
      case WAV_LOAD_STATUS_LOAD:
         {
            int ret = LoadWaveFile();
            switch( ret )
            {
            case 0:
               {
                  //BP - load complete; need to transfer to SPU memory.
                  wave_load_status = WAV_LOAD_STATUS_SPU_TRANS;
               }
               break;
            case 1:
               {
                  //BP - load complete; IOP waveforms so no transfer needed.
                  wave_load_status = WAV_LOAD_STATUS_NONE;
               }
               break;
            case 2:
               {
                  //BP - still loading.
               }
               break;
            default:
               BP_BREAK;
            }
         }
         break;
      case WAV_LOAD_STATUS_SPU_TRANS:
         break;
      case WAV_LOAD_STATUS_CD_LOAD:
         WaveCdLoad();
         break;
      default:
         BP_BREAK;
      }
   }
#endif
}

int gBP_SdEELoadThreadWakeupCount = 0;

void BP_SdEELoadLoopIteration()
{
   while( gBP_SdEELoadThreadWakeupCount > 0 )
   {
      if (lnr8_status) lnr_load();
      if (str2_status[0] || str2_status[1]) str2_load();
      --gBP_SdEELoadThreadWakeupCount;
   }
}

int	send_timer = 0;
int	sd_int_sw = 0;	//2.5mSEC割り込み対応（AC-3ストリーミング以外は、5mSEC処理にする）

void BP_SdIntLoopIteration()
{
   lnr_int();			/* STR 3 (LNR8/AC-3) 再生 */
   if (sd_int_sw) {
      IntSdMain();	/* SNG/SE再生 */
      str2_int();		/* STR 1/2 再生 */
      sd_set_status();/* EEへステータス送信 */
      sd_send_status();
   }
   ++sd_int_sw;
   sd_int_sw &= 1;
}

/*=======================================================================*/
/*===================== SUBROUTUNES =====================================*/
/*=======================================================================*/

/*========================================================================
*   INITIALIZE SPU & SOUND WORK
=========================================================================*/

#if 1 //BP_PS2
void bp_iop_sd_init (void)
#else
void sd_init (void)
#endif
{
	sceSdEffectAttr	r_attr;
	int				i;

#if 1 //BP_PS2
   //Just include the stuff we want-- reverb initialization.
   for( i = 0; i < 2; i++ )
   {
      r_attr.depth_L  = 0;
      r_attr.depth_R  = 0;
      r_attr.mode = SD_REV_MODE_HALL | SD_REV_MODE_CLEAR_WA;
      BP_sceSdSetEffectAttr( i, &r_attr ); //Reverb=off
      /*Reverb On */
      BP_sceSdSetCoreAttr( i|SD_C_EFFECT_ENABLE, 1 );
      BP_sceSdSetParam( i|SD_P_EVOLL , REVERB_DEPTH);
      BP_sceSdSetParam( i|SD_P_EVOLR , REVERB_DEPTH);
      BP_sceSdSetSwitch( i|SD_S_VMIXEL , 0 );
      BP_sceSdSetSwitch( i|SD_S_VMIXER , 0 );
      rev_bit_data[i] = 0;
   }
#else
	for( i = 0; i < 2; i++ )
	{
		BP_sceSdSetParam( i|SD_P_MVOLL , 0 ) ;	//Main Volume=off
		BP_sceSdSetParam( i|SD_P_MVOLR , 0 ) ;
		BP_sceSdSetSwitch( i|SD_S_PMON , 0 );	//Pitch LFO=off
		BP_sceSdSetSwitch( i|SD_S_NON , 0 );	//NOISE=off
/*Reverb Reset*/
		if (i == 0) {
			BP_sceSdSetParam(i|SD_P_MMIX, 0x0F0C);	//サウンドデータ入力OFF、コア外部入力ON
			BP_sceSdSetAddr( i|SD_A_EEA, 0x01FFFF ); //エフェクト作業エリア終端アドレス
		} else {
			BP_sceSdSetParam(i|SD_P_MMIX, 0x0FCC);	//サウンドデータ入力ON、コア0入力ON
			BP_sceSdSetAddr( i|SD_A_EEA, 0x1FFFFF );
		}
		r_attr.depth_L  = 0;
		r_attr.depth_R  = 0;
		r_attr.mode = SD_REV_MODE_HALL | SD_REV_MODE_CLEAR_WA;
		sceSdSetEffectAttr( i, &r_attr ); //Reverb=off
/*Reverb On */
		BP_sceSdSetCoreAttr( i|SD_C_EFFECT_ENABLE, 1 );
		BP_sceSdSetParam( i|SD_P_EVOLL , REVERB_DEPTH);
		BP_sceSdSetParam( i|SD_P_EVOLR , REVERB_DEPTH);
		BP_sceSdSetSwitch( i|SD_S_VMIXEL , 0 );
		BP_sceSdSetSwitch( i|SD_S_VMIXER , 0 );
		rev_bit_data[i] = 0;
/*メインボリュームを最大に設定*/
		BP_sceSdSetParam( i|SD_P_MVOLL , 0x3fff ) ;
		BP_sceSdSetParam( i|SD_P_MVOLR , 0x3fff ) ;
	}
/*Core1:外部入力ボリューム (=Core0のミキシング出力)*/
	BP_sceSdSetParam( 1|SD_P_AVOLL , 0x7fff);
	BP_sceSdSetParam( 1|SD_P_AVOLR , 0x7fff);

///*Core0 サウンドデータ入力ボリューム*/
	BP_sceSdSetParam( 0|SD_P_BVOLL , 0);
	BP_sceSdSetParam( 0|SD_P_BVOLR , 0);
///*Core1 サウンドデータ入力ボリューム*/
	BP_sceSdSetParam( 1|SD_P_BVOLL , 0);
	BP_sceSdSetParam( 1|SD_P_BVOLR , 0);

/*=======================================================================
・メディア（カテゴリコード）
        SD_SPDIF_MEDIA_DVD      メディアがDVDの場合
        SD_SPDIF_MEDIA_CD       メディアがCDの場合（デフォルト）
・出力する種類
        SD_SPDIF_OUT_OFF        SPDIFに何も出力しない
        SD_SPDIF_OUT_PCM        アナログと同じ音をPCMで出力（デフォルト）
        SD_SPDIF_OUT_BITSTREAM  Core0の入力ブロックに入力されたデータを、
                                ビットストリームとして出力する
・デジタル録音
        SD_SPDIF_COPY_NORMAL    通常モード（１世代録音可・デフォルト）
        SD_SPDIF_COPY_PROHIBIT  デジタル録音禁止
------------------------------------------------------------------------*/
	BP_sceSdSetCoreAttr( SD_C_SPDIF_MODE,
	SD_SPDIF_MEDIA_DVD | SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_PROHIBIT );
//	SD_SPDIF_MEDIA_DVD | SD_SPDIF_OUT_BITSTREAM | SD_SPDIF_COPY_PROHIBIT );

////	SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_NORMAL ); //<--デジタルコピー可
////	SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_PROHIBIT ); //<--デジタルコピー不可

/*DEBUG*/
	PRINTF(("Effect 0 Address = %x - %x\n", 
		sceSdGetAddr(SD_CORE_0 | SD_A_ESA),
		sceSdGetAddr(SD_CORE_0 | SD_A_EEA) ));
	PRINTF(("Effect 1 Address = %x - %x\n", 
		sceSdGetAddr(SD_CORE_1 | SD_A_ESA),
		sceSdGetAddr(SD_CORE_1 | SD_A_EEA) ));
#endif
/*SPU RAMのアロケート*/
// 00 0000 +------+
//         |005020| システム領域
// 00 5020 +------+
//         |001000| CD/DVDストリーミング1再生エリア（左）
// 00 6020 +------+
//         |001000| CD/DVDストリーミング1再生エリア（右）
// 00 7020 +------+
//         |001000| ストリーミング2再生エリア（左）
// 00 8020 +------+
//         |001000| ストリーミング2再生エリア（右）
// 00 9020 +------+
//         |      | IOPメモリストリーミング再生エリア(12Ch)
// 01 5220 +------+
//         |00ADE0| リバーブエリア(CORE0)
// 02 0000 +------+
//         |      |
//         |150000| 常駐波形エリア(BGM:E0000+SE:70000)
//         |      |
// 17 0000 +------+
//         |      |
//         |085220| 入替波形エリア
//         |      |
// 1F 5220 +------+
//         |00ADE0| リバーブエリア(CORE1)
// 1F FFFF +------+
	spu_wave_start_ptr = 0x020000;

/*Initialize Work*/
	init_sng_work();
	wave_load_status = WAV_LOAD_STATUS_NONE;
	for (i = 0; i < SE_TRACK_NUM; i++) {
		se_playing[i].code = 0;
	}
/* Mix Fader */	//99/12/22
	for (i=0;i<(SNG_TRACK_NUM/2); i++) {
		mix_fader[i].now_vol = 0xFFFF;
		mix_fader[i].dst_vol = 0xFFFF;
		mix_fader[i].int_vol = 0;
		mix_fader[i].now_pan = 0x20;
/*2001/04/25*/	mix_fader[i].now_frq = mix_fader[i].last_frq = 0;
	}
	for (i=(SNG_TRACK_NUM/2);i<SNG_TRACK_NUM; i++) {
		mix_fader[i].now_vol = 0;
		mix_fader[i].dst_vol = 0;
		mix_fader[i].int_vol = 0;
		mix_fader[i].now_pan = 0x20;
/*2001/04/25*/	mix_fader[i].now_frq = mix_fader[i].last_frq = 0;
	}
	vox_fader[0].set_vol = vox_fader[1].set_vol = 0x3F;
	vox_fader[0].set_pan = vox_fader[1].set_pan = 0x20;

   BP_SoundSupport_SetMasterVolume(1.f);
}

/*========================================================================
*   TERMINATE SPU
=========================================================================*/

void SdTerm (void) {
#if 1 //BP_PS2
   BP_TODO_BREAK;
#else
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_KOFF , 0xFFFFFF );
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_KOFF , 0xFFFFFF );
#endif
}

/*========================================================================
*   KEY OFF PCM CHANNEL
*-------------------------------------------------------------------------
*   IN: (ch)= Channel No.
=========================================================================*/
void keyOff (unsigned long  ch)
{
	if (ch < 24) {
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_KOFF , (1<<ch) );
	} else {
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_KOFF , (1<<(ch-24)) );
	}
}

/*========================================================================
*   KEY ON PCM CHANNEL
*-------------------------------------------------------------------------
*   IN: (ch)= Channel No.
=========================================================================*/
void keyOn (unsigned long ch)
{
	if (ch < 24) {
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_KON , (1<<ch) );
	} else {
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_KON , (1<<(ch-24)) );
	}
}


/*========================================================================
* サウンドで使用するメイン・メモリをアロケートする
=========================================================================*/
//extern struct SETBL se_tbl[];

unsigned char sng_data[SNG_DATA_SIZE];
unsigned char wave_header[WAVEHEADERSIZE];

unsigned char jo_data_area[0x1000+SE_DATA_SIZE];	// 0x1000=SETBL*0x100 テーブルSize
struct SETBL *jo_header;
unsigned char *jo_data;

//unsigned char se_data_area[HEADERSIZE+SE_DATA_SIZE];
unsigned char se_data_area[SE_EXP_TABLE_SIZE+0x1000+SE_DATA_SIZE];	// 0x1000=SETBL*0x100 テーブルSize
struct SETBL *se_header;
unsigned char *se_data;
unsigned char *se_exp_table;

//unsigned char cdload_buf[CDLOAD_BUF_SIZE];
unsigned char *cdload_buf;
//unsigned char str_header[HEADERSIZE];
int sd_mem_alloc(void)

{
//	int		err;

	voice_tbl = (struct WAVE_W *)wave_header;

/*常駐SEエリア*/
	jo_header = (struct SETBL *)jo_data_area;
	jo_data = (unsigned char *)jo_header+0x1000;

/*拡張SEエリア*/
/* 2000/03/13 for SE Expansion Table
	se_header = (struct SETBL *)se_data_area;
*/
	se_exp_table = (unsigned char *)se_data_area;
	se_header = (struct SETBL *)(se_exp_table+0x800);
	se_data = (unsigned char *)se_header+0x1000;

//ＣＤ読み込みバッファは、EE用読み込みバッファを拝借する 2001/01/25
#if 1 //BP_PS2
   //shared a buffer from cdbios on ps2, and this was the size.
   cdload_buf = (unsigned char*)malloc( 32 * 1024 * 2 );
#else
	cdload_buf = (unsigned char *)cdGetBuffer();
#endif

	return 0;
}

/*=======================================================================*/
/*=====================	End on File =====================================*/
/*===================== DON'T ADD STUFF AFTER THIS ======================*/
/*=======================================================================*/


