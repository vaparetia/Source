/*===================================================================
 *  Title           : VZ003 Sound Sub Routine
 *  version         : 0.10
 *  Code            : VZ003
 *  Filename        : SD_MAIN.C
 *  Creator         : K.C.E OSAKA - K.Muraoka
 *  First edition   : 1995/06/01
 *===================================================================*/

/*=======================================================================*/
/*===================== INCLUDE FILES ===================================*/
/*=======================================================================*/
#include	<stdio.h>
#include	<kernel.h>
#include	<sys/types.h>
#include	<sif.h>
#include	<sifcmd.h>
#include	<sifrpc.h>
#include	<timerman.h>
//BP_PS2 #include	"../jsifman/jsifman.h"
#include	<libsd.h>
#include "sd_debug.h"
#include "sd_incl.h"
#include "sd_ext.h"

#include "mgs_type.h"

#include "BP_EndianSupport.h"
#include "BP_FileSupport.h"
#include "BP_SoundEffectOverride.h"
#include "BP_SoundSupport.h"

//#define BP_DISABLE_MIDI_BGM
//#define BP_SE_CODE_PRINT_DEBUG

extern int   BP_SpewSfxPlayed;

//extern struct	SETBL se_tbl[];
extern struct SETBL *jo_header;
extern unsigned char *jo_data;
extern struct SETBL *se_header;
extern unsigned char *se_exp_table;
extern unsigned char *se_data;
extern int	id_SdMain, id_SdEELoad;
extern int gBP_SdEELoadThreadWakeupCount;

/* for STR 1/2*/
extern unsigned int	str2_status[2];
extern unsigned int	str2_effect[2];
extern unsigned int	str2_load_code[2];
extern unsigned int	str2_counter[2];
extern unsigned int	str2_stop_fg[2];
extern int				str2_wait_fg[2];//再生開始待ち
extern unsigned int	str2_first_load[2];	//最初のデータロード中
//int				str2_read_disable[2];
extern unsigned int	str2_mono_fg[2];	//1=モノラル
extern unsigned short	str2_pitch[2];	//周波数レジスタ値
extern int           str2_game_flag[2];
extern int           str2_top_pos[2];
extern int           str2_bp_surround_type[2];
extern int		str2_iop_load_set[2];
extern unsigned int	str2_pause_fg[2];
extern unsigned int	str3_pause_fg;
/* for STR 3(LNR8)*/
extern unsigned int	lnr8_status;
extern unsigned int	lnr8_load_code;
extern unsigned int	lnr8_counter;
extern unsigned int	lnr8_stop_fg;
extern unsigned int	lnr8_first_load;	//最初のデータロード中
//int					lnr8_read_disable;

extern unsigned int	str2_play_counter[2];
extern unsigned int	lnr8_play_counter;
extern int ac3_fg;

extern struct EE_ADDR ee_addr[2];
/*******************/
int 				sd_set_cli (int sound_code);
int					SePlay(int sound_code);
void sd_set (int sound_code);

float bp_cli_float_param = 0.f;
int bp_cli_int_param = 0;
void * bp_cli_ptr_param = NULL;

extern int BP_EnableMidiBgm;
extern int BP_EnableSfx;

/*************************************************************************
* Modules for Client Tasks
**************************************************************************/

#define GM_STREAM_CHANNEL_0		0x00000010
#define GM_STREAM_CHANNEL_1		0x00000020
#define	GM_STREAM_FLAG_3D		0x00000080	// 音声ストリーム3Dフラッグ

int bp_get_str2_stream_surround_type( const int top_pos, const int flag, const int is_stereo )
{
   //General rule being applied here:
   //* streams that are cinematic or music should use stereo upmixed to 8-channel so all speakers are filled.
   //* other stereo streams (typically ambient / processed voice or voice with music)
   //  are spread to FL, C, FR channels.
   //* mono codec streams (incl. mindvoice) get stereo volumes piped only to center channel.
   //* mono environmental streams get normal 3D pan treatment. (it's too loud to use for codec streams)

   //TODO: this does not properly handle game over streams or various other special cases where these rules
   //don't work with the flag set in GM_VoxStream().  Need to add a table of special cases most likely.
   //For now, these streams will play with stereo->8-channel upmix assuming flag is 0.

   // On Vita, we always use stereo..

   //Apply general rules.
   switch( flag )
   {
   case GM_STREAM_CHANNEL_0:
      //codec
   case GM_STREAM_CHANNEL_1:
      //mindvoice
      return is_stereo ? BP_SURROUND_VOICE_TYPE_FLFCFR : BP_SURROUND_VOICE_TYPE_CENTER;
   case GM_STREAM_FLAG_3D:
      //in-game vox stream
      return BP_SURROUND_VOICE_TYPE_SURROUND;
   }

   //default: music or other full-surround-spread stereo stream.
   return BP_SURROUND_VOICE_TYPE_STEREO;
}

/*---------------------- Set Sound Code ---------------------------------*/
#if 1
int bp_iop_sd_set_cli (int sound_code)
#else
int sd_set_cli (int sound_code)
#endif
{
	sd_set((unsigned int)sound_code);
	return(0);
}
/*--------------------- Save Sound Code ---------------------------------*/
//int	get_sng_code(void)
//{
//	if( (sng_play_code == 0xFFFFFFFFL) || (sng_play_code == 0) ) return (0);
//	else return(sng_play_code);
//}
/*---------------------- Set Play S.E.Code --------------------------*/

//#define BP_FILTER_SE

static inline int bp_filter_sound( int se_code )
{
#ifdef BP_FILTER_SE
   return se_code == SD_S_O2DAMAGE || se_code == SD_S_ISEL02;
#else
   (void)se_code;
   return 1;
#endif
}

int		sd_print_fg;
/*extern int se_max_num;*/
int	se_tracks;
int SePlay(int sound_code) {
	int					priority;
	int					i, j;
	unsigned int		idx;
	int					se_code, se_idx;
	struct SEPLAYTBL	se_tmp;
   int               bp_se_track_count;
   void *            bp_se_tracks[2] = { 0 };

/*	if ((sound_code&0xFF) > se_max_num) return (-1);*/
/*---------------------- 効果音完了チェック -------------------------*/

	j = song_end[1] >> (SNG_TRACK_NUM-24);
	for (i = 0; i < SE_TRACK_NUM; i++) {
		if (j&1) {
			se_playing[i].code = 0;
			se_playing[i].pri = 0;
			se_playing[i].character = 0;
		}
		j >>= 1;
	}
/*---------------------- 効果音ヘッダを一時ワークへコピー -----------*/
//	se_code = sound_code & 0x1FF;	//常駐を0x100個、入替を0x100個に拡張する
	se_code = sound_code & 0x7FF;	// 2000/04/04

   if( !BP_EnableSfx )
   {
      return -1;
   }

   if( !bp_filter_sound( se_code ) )
   {
      return -1;
   }

   if( BP_SpewSfxPlayed )
   {
      printf("SePlay: 0x%03x\n", se_code );
   }
	se_tmp.code = sound_code;
   se_tmp.bp_angle = bp_cli_float_param;
   //BP - .kind indicates whether a system SE or not.  System sounds should be front speakers only;
   //general sound effects can be assumed to be full surround.
	if (se_code < 0x100) {
//		se_tracks = se_tbl[se_code].tracks;
//		se_tmp.character = se_tbl[se_code].character;
		se_tracks = jo_header[se_code].tracks;
		se_tmp.character = jo_header[se_code].character;
      se_tmp.kind = jo_header[se_code].kind;

#if defined(BP_VITA)
      se_tmp.bp_surround_type = BP_SURROUND_VOICE_TYPE_STEREO;
#else
      se_tmp.bp_surround_type = se_tmp.kind ? BP_SURROUND_VOICE_TYPE_SURROUND : BP_SURROUND_VOICE_TYPE_FLFCFR;
#endif
	} else {
		se_idx = se_exp_table[se_code-0x100];
		if (se_idx == 0xFF) {
			PRINTF(("ERR:se_exp_table offset(%x)\n", (se_code&0x7FF) ));
			return(-1);
		}
///*DEBUG*/else if (sd_print_fg) PRINTF(("SE Internal Code = %x\n", se_idx));
		se_tracks = se_header[se_idx].tracks;
		se_tmp.character = se_header[se_idx].character;
      se_tmp.kind = se_header[se_idx].kind;
#if defined(BP_VITA)
      se_tmp.bp_surround_type = BP_SURROUND_VOICE_TYPE_STEREO;
#else
      se_tmp.bp_surround_type = se_tmp.kind ? BP_SURROUND_VOICE_TYPE_SURROUND : BP_SURROUND_VOICE_TYPE_FLFCFR;
#endif
	}

   //BP - check override here, so we can override number of tracks and initialize up to 2 tracks properly.
   bp_se_track_count = BP_Override_SoundEffect( se_code, &bp_se_tracks[0], &bp_se_tracks[1] );
   if( bp_se_track_count )
   {
      se_tracks = bp_se_track_count;
   }

	if (se_tracks > 3) {
/*DEBUG*/PRINTF(("SE:TooManyTracks(%x):sound_code=%x\n", se_tracks, (se_code&0x7FF) ));
		se_tracks = 3;
	}

	for (idx=0; idx<se_tracks; idx++) {
		if (se_code < 0x100) {
			se_tmp.pri = jo_header[se_code].pri;
			se_tmp.kind = jo_header[se_code].kind;
			se_tmp.addr = jo_data+(BP_LE_SwapUInt(jo_header[se_code].addr_le[idx]));
		} else {
			se_tmp.pri = se_header[se_idx].pri;
			se_tmp.kind = se_header[se_idx].kind;
			se_tmp.addr = se_data+(BP_LE_SwapUInt(se_header[se_idx].addr_le[idx]));
		}
/*---------------------- 優先チェック -------------------------------*/
		priority = 0x100;				/* Highest Priority + 1*/
		j = 0;

		for (i=0; i<SE_TRACK_NUM; i++) {
/*----- 再生中の同じ効果音を探す -----*/
			if (((se_playing[i].code&0x7FF) == (se_tmp.code&0x7FF)) && (se_request[i].code==0) ){
//			if (((se_playing[i].code&0x1FF) == se_code) && (se_request[i].code==0) ){
				priority = 0;
				j = i;
				break;
			} else {
/*複数トラックＳＥが二重で鳴るバグに対応 2000/04/12*/
				if ( ((se_request[i].code&0x7FF) == (se_tmp.code&0x7FF)) && (idx == 0) ){
//				if ( ((se_request[i].code&0x1FF) == se_code) && (idx == 0) ){
					se_tracks = 0;
					return(0); //同じＳＥがリクエストされていたら、再度セットする必要なし
				}
//				if (se_tracks == 1) {
//					if ((se_request[i].code&0x1FF) == se_code) {
//						priority = 0;
//						j = i;
//						break;
//					}
//				}
			}
/*----- 再生中の同じ人物の音声を探す -----*/
			if (se_tmp.character) {
// 2001/05/09
				if (((se_playing[i].character)==se_tmp.character) && (se_request[i].code != se_tmp.code)) {
//				if ( (se_playing[i].character) == se_tmp.character ){
					priority = 0;
					j = i;
					break;
// 2001/05/09
//				} else {
//					if ((se_request[i].character) == se_tmp.character ) {
//						priority = 0;
//						j = i;
//						break;
//					}
				}
			}
		}

		if (j != i) {
/*----- 停止中のトラックを探す -----*/
			for (i=0; i<SE_TRACK_NUM; i++) {
				if ( (se_playing[i].code==0) && (se_request[i].code==0) ) {
					priority = 0;
					j = i;
					break;
				}
			}
			if (j != i) {
/*----- 優先の一番低い効果音を探す -----*/
				for (i=0; i<SE_TRACK_NUM; i++) {
					if (se_request[i].code == 0 ) {
						if (se_playing[i].pri <= priority) {
							priority = se_playing[i].pri;
							j = i;
						}
					} else {
						if (se_request[i].pri <= priority) {
							priority = se_request[i].pri;
							j = i;
						}
					}
				}
/*DEBUG*/	if ((se_tmp.pri) >= (priority)) PRINTF(("***SE Priority:%x > (%x or %x)***\n", (unsigned int)(se_tmp.code&0x7FF), (unsigned int)(se_playing[j].code&0x7FF), (unsigned int)(se_request[j].code&0x7FF) ));
///*DEBUG*/	if ((se_tmp.pri%100) >= (priority%100)) PRINTF(("***SE Priority:%x > (%x or %x)***\n", (unsigned int)(se_tmp.code&0x7FF), (unsigned int)(se_playing[j].code&0x7FF), (unsigned int)(se_request[j].code&0x7FF) ));
			}
		}
/*効果音のセット*/
		if ((se_tmp.pri >= priority) && (j < SE_TRACK_NUM) ) {
			se_request[j].pri = se_tmp.pri;
			se_request[j].kind = se_tmp.kind;
			se_request[j].character = se_tmp.character;
/*キャラクター音声は、Track00にセットする事(1998/06/18)*/
//2001/05/09			se_tmp.character = 0;

			se_request[j].addr = se_tmp.addr;
			se_request[j].code = se_tmp.code;
         se_request[j].bp_angle = se_tmp.bp_angle;
         se_request[j].bp_surround_type = se_tmp.bp_surround_type;

         if( bp_se_track_count )
         {
            //override this midi program.
            se_request[j].addr = bp_se_tracks[idx];
         }

         if( BP_SpewSfxPlayed )
         {
            printf("      : %d: [%d][%d]\n", idx, (j+SNG_TRACK_NUM)/24,(j+SNG_TRACK_NUM)%24 );
         }

/*1998/03/15 常駐効果音の再生スタート時に停止フラグがONならリセットする*/
			if (se_tmp.pri == 0xFF) {
				stop_jizoku_se = 0;
				stop_jizoku_se2 = 0;
			}
		} else {
/*DEBUG*/PRINTF(("SECan'tPlay:LowPriority(code=%x:pri=%x:j=%x)\n", (unsigned int)(se_tmp.code&0x7FF), se_tmp.pri, j ));
		}
	}
	se_tracks = 0;
	return(0);
}

/*========================================================================
*   SNGに対する音コードをリングバッファに積む
=========================================================================*/
void set_sng_code_buf(int sound_code)
{
	if (sd_sng_code_buf[sd_code_set] == 0) {
		sd_sng_code_buf[sd_code_set] = sound_code;
/*DEBUG*/ /*PRINTF(("SdCode=%x(idx:%x)\n", sd_sng_code_buf[sd_code_set], sd_code_set));*/
		sd_code_set = (sd_code_set+1) & 0xF;
	} else {
		PRINTF(("***TooMuchBGMSoundCode(%x)***\n", sound_code));
	}
}
/*========================================================================
*   SET SOUND CODE
=========================================================================*/

/*int		se_stop_fg;	1999/04/08 VR mode only*/

int	str_playing_fg[2];	//2001/09/11 ストリーミング情報設定が多重で来た時は受け付けない

void sd_set (int sound_code)
{
/*--------------------- PRINTF DEBUG --------------------------------*/
#if 0
	mts_set_exception_func(sd_print_debug);
	sd_code_buf[sd_code_idx]= sound_code;
	sd_code_idx = (sd_code_idx+1)&7;
#endif
/*-------------------------------------------------------------------*/
//	1998/07/10 K.Muraoka
	if (sound_code == 0xFF000000) {
#ifdef BP_SE_CODE_PRINT_DEBUG
		sd_print_fg = 1;
#endif
		goto SdSetEnd;
	}
	if (sound_code == 0xFF000001) {
		sd_print_fg = 0;
		goto SdSetEnd;
	}
//	2000/01/04 K.Muraoka
/*DEBUG*/if (sound_code == 0xFF000002) {
/*DEBUG*/PRINTF(("\n\n*** FADER VOLUME STATUS ***\n"));
/*DEBUG*/PRINTF(("CH00=%2x:CH01=%2x:CH02=%2x:CH03=%2x",
/*DEBUG*/mix_fader[0].now_vol, mix_fader[1].now_vol, mix_fader[2].now_vol, mix_fader[3].now_vol));
/*DEBUG*/PRINTF(("CH04=%2x:CH05=%2x:CH06=%2x:CH07=%2x\n", 
/*DEBUG*/mix_fader[4].now_vol, mix_fader[5].now_vol, mix_fader[6].now_vol, mix_fader[7].now_vol));
/*DEBUG*/PRINTF(("CH08=%2x:CH09=%2x:CH0A=%2x:CH0B=%2x", 
/*DEBUG*/mix_fader[8].now_vol, mix_fader[9].now_vol, mix_fader[0xA].now_vol, mix_fader[0xB].now_vol));
/*DEBUG*/PRINTF(("CH0C=%2x:CH0D=%2x:CH0E=%2x:CH0F=%2x\n", 
/*DEBUG*/mix_fader[0xC].now_vol, mix_fader[0xD].now_vol, mix_fader[0xE].now_vol, mix_fader[0xF].now_vol));
/*DEBUG*/PRINTF(("CH10=%2x:CH11=%2x:CH12=%2x:CH13=%2x", 
/*DEBUG*/mix_fader[0x10].now_vol, mix_fader[0x11].now_vol, mix_fader[0x12].now_vol, mix_fader[0x13].now_vol));
/*DEBUG*/PRINTF(("CH14=%2x:CH15=%2x:CH16=%2x:CH17=%2x\n", 
/*DEBUG*/mix_fader[0x14].now_vol, mix_fader[0x15].now_vol, mix_fader[0x16].now_vol, mix_fader[0x17].now_vol));
/*DEBUG*/PRINTF(("CH18=%2x:CH19=%2x:CH1A=%2x:CH1B=%2x", 
/*DEBUG*/mix_fader[0x18].now_vol, mix_fader[0x19].now_vol, mix_fader[0x1A].now_vol, mix_fader[0x1B].now_vol));
/*DEBUG*/PRINTF(("CH1C=%2x:CH1D=%2x:CH1E=%2x:CH1F=%2x\n", 
/*DEBUG*/mix_fader[0x1C].now_vol, mix_fader[0x1D].now_vol, mix_fader[0x1E].now_vol, mix_fader[0x1F].now_vol));
/*DEBUG*/PRINTF(("\n*** FADER PAN STATUS ***\n"));
/*DEBUG*/PRINTF(("CH00=%2x:CH01=%2x:CH02=%2x:CH03=%2x",
/*DEBUG*/mix_fader[0].now_pan, mix_fader[1].now_pan, mix_fader[2].now_pan, mix_fader[3].now_pan));
/*DEBUG*/PRINTF(("CH04=%2x:CH05=%2x:CH06=%2x:CH07=%2x\n", 
/*DEBUG*/mix_fader[4].now_pan, mix_fader[5].now_pan, mix_fader[6].now_pan, mix_fader[7].now_pan));
/*DEBUG*/PRINTF(("CH08=%2x:CH09=%2x:CH0A=%2x:CH0B=%2x", 
/*DEBUG*/mix_fader[8].now_pan, mix_fader[9].now_pan, mix_fader[0xA].now_pan, mix_fader[0xB].now_pan));
/*DEBUG*/PRINTF(("CH0C=%2x:CH0D=%2x:CH0E=%2x:CH0F=%2x\n", 
/*DEBUG*/mix_fader[0xC].now_pan, mix_fader[0xD].now_pan, mix_fader[0xE].now_pan, mix_fader[0xF].now_pan));
/*DEBUG*/PRINTF(("CH10=%2x:CH11=%2x:CH12=%2x:CH13=%2x", 
/*DEBUG*/mix_fader[0x10].now_pan, mix_fader[0x11].now_pan, mix_fader[0x12].now_pan, mix_fader[0x13].now_pan));
/*DEBUG*/PRINTF(("CH14=%2x:CH15=%2x:CH16=%2x:CH17=%2x\n", 
/*DEBUG*/mix_fader[0x14].now_pan, mix_fader[0x15].now_pan, mix_fader[0x16].now_pan, mix_fader[0x17].now_pan));
/*DEBUG*/PRINTF(("CH18=%2x:CH19=%2x:CH1A=%2x:CH1B=%2x", 
/*DEBUG*/mix_fader[0x18].now_pan, mix_fader[0x19].now_pan, mix_fader[0x1A].now_pan, mix_fader[0x1B].now_pan));
/*DEBUG*/PRINTF(("CH1C=%2x:CH1D=%2x:CH1E=%2x:CH1F=%2x\n", 
/*DEBUG*/mix_fader[0x1C].now_pan, mix_fader[0x1D].now_pan, mix_fader[0x1E].now_pan, mix_fader[0x1F].now_pan));
/*DEBUG*/}

/*DEBUG*/	if (sd_print_fg) {
/*DEBUG*/			if ( (sound_code&0xFF000000)==0x00000000 ){
/*DEBUG*/				PRINTF(("SeCode=%x vol=%x pan=%x\n", (unsigned int)sound_code&0xFFF,(unsigned int)((sound_code&0x3F000)>>12), (unsigned int)((sound_code&0xFC0000)>>18) ));
/*DEBUG*/			} else {
/*DEBUG*/				PRINTF(("SdCode=%x\n", (unsigned int)sound_code));
/*DEBUG*/			}
/*DEBUG*/	}

/*--------------------- 効果音再生 (0x00xx xxxx) --------------------*/
	if ( (sound_code&0xFF000000)==0x00000000 ){
//		if ((sound_code&0x1FF) == 0) goto SdSetEnd;
		if ((sound_code&0x7FF) == 0) goto SdSetEnd;	// 2000/04/04
//		if (se_stop_fg) goto SdSetEnd;	/*for INTEGRAL VR Mode*/
		SePlay(sound_code);
		goto SdSetEnd;
	}
/*--------------------- ＳＮＧ再生 (0x01xx xxxx) --------------------*/
	if ( (sound_code&0xFF000000)==0x01000000 ){
#if 1 /*1997/05/27 0 = BGMを鳴らさない(VTR録画用)*/
#ifndef BP_DISABLE_MIDI_BGM
      if( BP_EnableMidiBgm )
      {
   		set_sng_code_buf(sound_code);
      }
#endif
#endif
		goto SdSetEnd;
	}
/*--------------------- ＳＥロード (0x02xx xxxx) --------------------*/
	if ( (sound_code&0xFF000000)==0x02000000 ){
		se_load_code = sound_code;
      if( bp_se_load_substatus )
         BP_BREAK;
      bp_se_load_substatus = BP_SE_LOAD_SUBSTATUS_LOAD_SEEXP;
#if 1 //BP_PS2
      BP_TED_BREAK;
#else
		BP_WakeupThread(id_SdMain);
#endif
		goto SdSetEnd;
	}
/*--------------------- ファイルロード・パス名GET (0x8xxx xxxx) -----------*/
	if ( (sound_code&0xE0000000)==0x80000000 ){
#if 1 //BP_PS2
      BP_TED_BREAK;
#else
		SIF_RV_QUEUE	*que;

		que = sif_get_mem(path_name, (unsigned char *)(sound_code & 0x1FFFFFFF), sizeof(path_name) );
		while (1) {
			if (que->id & 0x80000000) break;
		}
		sif_rv_release_queue(que);
#endif
		PRINTF(("Path name = %s\n", path_name));
		goto SdSetEnd;
	}

/*---------------------  STR Load & Play (0xF0xx xxxx) --------------*/
	if ( (sound_code&0xFF000000)==0xF0000000 ){
		if (lnr8_status) lnr8_stop_fg = 1;	//STR3 停止
		str2_stop_fg[0] = 0;
		ee_addr[0].sz = (sound_code & 0xFFFFFF) <<4;
		goto SdSetEnd;
	}

/*--- ストリーミング1EE(読み込み)オフセットアドレス設定 (0xF1xx xxxx) --*/
	if ( (sound_code&0xFF000000)==0xF1000000 ){
#if 1 //BP_PS2 (see sdstream.c for the other end of this)
      ee_addr[0].ofst[0] = (unsigned int)bp_cli_ptr_param;
      if( (sound_code & 0x00FFFFFF) != (ee_addr[0].ofst[0] & 0x0FFFFFFF)>>4)
         BP_BREAK;   //sanity check
#else
		ee_addr[0].ofst[0] = sound_code & 0x00FFFFFF;
#endif
		ee_addr[0].set_ctr++;
		str2_iop_load_set[0] = 1;
#if 1 //BP_PS2
      ++gBP_SdEELoadThreadWakeupCount;
#else
		BP_WakeupThread(id_SdEELoad);
#endif
		goto SdSetEnd;
	}

/*--------------------- ストリーミング2/3スタート (0xF2xx xxxx) --------*/
	if ( (sound_code&0xFF000000)==0xF2000000 ){
		ee_addr[1].sz = (sound_code & 0xFFFFFF) <<4;
		goto SdSetEnd;
	}

/*--------------------- ストリーミング2/3オフセットアドレス設定 (0xF3xx xxxx) --------*/
	if ( (sound_code&0xFF000000)==0xF3000000 ){
//		ee_addr[1].ofst[(ee_addr[1].set_ctr)&1] = sound_code & 0x00FFFFFF;
#if 1 //BP_PS2 (see sdstream.c for the other end of this)
      ee_addr[1].ofst[0] = (unsigned int)bp_cli_ptr_param;
      if( (sound_code & 0x00FFFFFF) != (ee_addr[1].ofst[0] & 0x0FFFFFFF)>>4)
         BP_BREAK;   //sanity check
#else
		ee_addr[1].ofst[0] = sound_code & 0x00FFFFFF;
#endif
		ee_addr[1].set_ctr++;
//PRINTF(("ctr1=%x\n", ee_addr[1].set_ctr));
		str2_iop_load_set[1] = 1;
#if 1 //BP_PS2
      //BP_SdEELoadLoopIteration() handles this
      ++gBP_SdEELoadThreadWakeupCount;
#else
		BP_WakeupThread(id_SdEELoad);
#endif
		goto SdSetEnd;
	}

/*--------------------- ストリーミング2/3情報設定 (0xF4xx xxxx) --------*/
	if ( (sound_code&0xFF000000)==0xF4000000 ){

		if (str_playing_fg[1]) goto SdSetEnd;	//2001/09/11
		str_playing_fg[1] = 1;

//0xF4 kc ff ff ・・・k:0=VAG(Reverb off) 1=8bitPCM 2=AC-3、c=Ch.Num、ffff=Freq
//					  k:8=VAG(Reverb on)
		switch (sound_code &0x00F00000) {

		case 0x00000000:

//		case 0x00800000:
// 特定の音声にリバーブをかける（0xF48xxxxx）
//			if ((sound_code &0x00F00000) == 0x00800000) str2_effect[1] = 1;
//			else str2_effect[1] = 0;

			if (lnr8_status) lnr8_stop_fg = 1;//LNR8 STR 停止
			if ((sound_code & 0x000F0000) == 0x10000) str2_mono_fg[1] = 1;
			else str2_mono_fg[1] = 0; //STEREO
			str2_pitch[1] = ((sound_code & 0xFFFF)*0x1000)/48000;
			str2_stop_fg[1] = 0;
         str2_game_flag[1] = bp_cli_int_param;
         str2_top_pos[1] = (int)bp_cli_ptr_param;
         str2_bp_surround_type[1] = bp_get_str2_stream_surround_type( str2_top_pos[1], str2_game_flag[1], str2_mono_fg[1] == 0 );

			str2_first_load[1] = str2_load_code[1] = sound_code;
			str2_play_counter[1] = str2_counter[1] = 0;

			ee_addr[1].set_ctr = ee_addr[1].read_ctr = 0;
			str2_pause_fg[1] = 0;	//2001/08/27 K.Muraoka
			str2_status[1] = 1;

/*DEBUG*/	PRINTF(("*** Start Str2(VAG) mono=%x, pitch=0x%x ***\n", str2_mono_fg[1], str2_pitch[1]));

			break;

		case 0x00100000:	// 8bit PCM Streaming
			ac3_fg = 0;
			if (str2_status[0]) str2_stop_fg[0] = 1;//STR1 停止
			if (str2_status[1]) str2_stop_fg[1] = 1;//STR2 停止
			lnr8_stop_fg = 0;

			lnr8_first_load = lnr8_load_code = sound_code;
			ee_addr[1].set_ctr = ee_addr[1].read_ctr = 0;
			str3_pause_fg = 0; //2000/08/27
			lnr8_status = 1;
			lnr8_play_counter = lnr8_counter = 0;

/*DEBUG*/	PRINTF(("*** Start LNR8 Stream ***\n"));

			break;

		case 0x00200000:	// AC-3 Streaming
         BP_BREAK;   //BP - no AC3 streams!
			ac3_fg = 1;
			if (str2_status[0]) str2_stop_fg[0] = 1;//STR1 停止
			if (str2_status[1]) str2_stop_fg[1] = 1;//STR2 停止
			lnr8_stop_fg = 0;

			lnr8_first_load = lnr8_load_code = sound_code;
			ee_addr[1].set_ctr = ee_addr[1].read_ctr = 0;
			lnr8_status = 1;
			lnr8_play_counter = lnr8_counter = 0;

/*DEBUG*/	PRINTF(("*** Start AC-3 Stream ***\n"));

			break;

		default:
			break;
		}
#if 1 //BP_PS2
      //BP_SdEELoadLoopIteration() handles this
      ++gBP_SdEELoadThreadWakeupCount;
#else
		BP_WakeupThread(id_SdEELoad);
#endif
		goto SdSetEnd;
	}

/*----------- ストリーミング1(EE読み込み)情報設定 (0xF5xx xxxx) --------*/
	if ( (sound_code&0xFF000000)==0xF5000000 ){

		if (str_playing_fg[0]) goto SdSetEnd;	//2001/09/11
		str_playing_fg[0] = 1;

//0xF5 kc ff ff

// 特定の音声にリバーブをかける（0xF58xxxxx）
//		if ((sound_code &0x00F00000) == 0x00800000) str2_effect[0] = 1;
//		else str2_effect[0] = 0;

		if ((sound_code & 0x000F0000) == 0x10000) str2_mono_fg[0] = 1;
		else str2_mono_fg[0] = 0; //STEREO
		str2_pitch[0] = ((sound_code & 0xFFFF)*0x1000)/48000;
      str2_game_flag[0] = bp_cli_int_param;
      str2_top_pos[0] = (int)bp_cli_ptr_param;
      str2_bp_surround_type[0] = bp_get_str2_stream_surround_type( str2_top_pos[0], str2_game_flag[0], str2_mono_fg[0] == 0 );
/*DEBUG*/	PRINTF(("*** Start Str1(VAG) mono=%x, pitch=0x%x ***\n", str2_mono_fg[0], str2_pitch[0]));

		if (lnr8_status) lnr8_stop_fg = 1;
		str2_first_load[0] = str2_load_code[0] = sound_code;
		str2_play_counter[0] = str2_counter[0] = 0;

		ee_addr[0].set_ctr = ee_addr[0].read_ctr = 0;
		str2_pause_fg[0] = 0;	//2001/08/27 K.Muraoka
		str2_status[0] = 1;
#if 1 //BP_PS2
      ++gBP_SdEELoadThreadWakeupCount;
#else
		BP_WakeupThread(id_SdEELoad);
#endif
		goto SdSetEnd;
	}

/*--- Mixing FREQ/PAN/VOLUME Fader (0xF8xxxxxx) -----------------*/
// 1111 1000 cccc cfff ffff pppp ppvv vvvv
	if ( (sound_code & 0xFF000000) == 0xF8000000L ) {
		int	i = (sound_code & 0x00F80000) >> 19;
		mix_fader[i].now_frq = ((sound_code&0x7F000)-0x40000) >> 7;
		mix_fader[i].now_pan = (sound_code&0xFC0) >> 6;
		mix_fader[i].dst_vol = ((sound_code&0x3F)<<10)+((sound_code&0x3F)<<4)+((sound_code&0x3F)>>2); //max.=0xFFFF
		mix_fader[i].now_vol = mix_fader[i].dst_vol;
		mix_fader[i].int_vol = 0;
///*DEBUG*/ if (sd_print_fg) PRINTF(("Fader:CH=%x:FRQ=%x:Pan=%x:Vol=%x\n", i, mix_fader[i].now_frq, mix_fader[i].now_pan, mix_fader[i].now_vol));
		goto SdSetEnd;
	}

/*--------------------- Core1 エフェクト設定(0xF9xx xxxx) -----------------*/
// 1111 1001 | 0000 mmmm | 0ddd dddd | dddd dddd = m:Mode d:Depth

	if ( (sound_code&0xFF000000)==0xF9000000 ){
//		switch ((sound_code>>16)&0xF) {
//		case 0:core_efct_wk.mode = SD_REV_MODE_OFF;break;
//		case 1:core_efct_wk.mode = SD_REV_MODE_ROOM;break;
//		case 2:core_efct_wk.mode = SD_REV_MODE_STUDIO_A;break;
//		case 3:core_efct_wk.mode = SD_REV_MODE_STUDIO_B;break;
//		case 4:core_efct_wk.mode = SD_REV_MODE_STUDIO_C;break;
//		case 5:core_efct_wk.mode = SD_REV_MODE_HALL;break;
//		case 9:core_efct_wk.mode = SD_REV_MODE_PIPE;break;
//		default: goto SdSetEnd;
//		}
		core_efct_wk.mode = SD_REV_MODE_HALL;

		core_efct_wk.depth = sound_code&0xFFFF;//depth(-8000 - +7FFF)
		core_efct_wk.status = 1;
		goto SdSetEnd;
	}

/*--- オートメーション・エンベロープ位置設定(0xFA0000XX) ---*/
	if ((sound_code & 0xFF000000) == 0xFA000000) {
		switch (sound_code & 0x00000F00) {
		case 0x000:
			auto_env_pos = sound_code & 0xFF;
			break;
		case 0x100:
			auto_env_pos2 = sound_code & 0xFF;
			break;
		default:
			break;
		}
		goto SdSetEnd;
	}

/*--- Mixing PAN/VOLUME Fader (0xFBxxxxxx) -----------------*/
	if ( (sound_code & 0xFF000000) == 0xFB000000L ) {
		if (sound_code <= 0xFB1F3F3FL) {
			int	i = (sound_code & 0x001F0000) >> 16;
			mix_fader[i].now_pan = (sound_code&0x3F00) >> 8;
			mix_fader[i].dst_vol = ((sound_code&0x3F)<<10)+((sound_code&0x3F)<<4)+((sound_code&0x3F)>>2); //max.=0xFFFF
			mix_fader[i].now_vol = mix_fader[i].dst_vol;
			mix_fader[i].int_vol = 0;
///*DEBUG*/ if (sd_print_fg) PRINTF(("Fader:Pan=%x:Vol=%x\n", mix_fader[i].now_pan, mix_fader[i].now_vol));

		} else {
/* Vox Fader */
			if ( ((sound_code & 0x00FF0000) == 0x00FE0000)
				|| ((sound_code & 0x00FF0000) == 0x00FF0000) ) {
				int i = (sound_code&0x00010000) >> 16;
				vox_fader[i].set_vol = sound_code & 0x3F;
				vox_fader[i].set_pan = (sound_code&0x3F00) >> 8;
            vox_fader[i].set_bp_angle = bp_cli_float_param;
///*DEBUG*/ if (sd_print_fg) PRINTF(("VoxFader:Code=%x:Ch=%x:Pan=%x:Vol=%x\n", sound_code, i, vox_fader[i].set_pan, vox_fader[i].set_vol));
			}
		}
		goto SdSetEnd;
	}

/*--- Mixing PAN Fader (0xFC000000 - 0xFC1F3FFF) ---*/
	if (( sound_code >= 0xFC000000L) && (sound_code <= 0xFC1F3FFFL)) {
		int	i = (sound_code & 0x001F0000) >> 16;
		mix_fader[i].now_pan = (sound_code&0x3F00) >> 8;

///*DEBUG*/if (sd_print_fg) PRINTF(("Fader:Pan=%x\n", mix_fader[i].now_pan ));

		goto SdSetEnd;
	}

/*--- Mixing Fader (0xFD000000 - 0xFD1F3FFF) ---*/
	if (( sound_code >= 0xFD000000L) && (sound_code <= 0xFD1F3FFFL)) {
		int	i = (sound_code & 0x001F0000) >> 16;
		mix_fader[i].dst_vol = ((sound_code&0x3F00)<<2)+((sound_code&0x3F00)>>4)+((sound_code&0x3F00)>>10); //max.=0xFFFF
		if (mix_fader[i].dst_vol == mix_fader[i].now_vol) { //現在値と同じフェーダー値を設定した時
			 mix_fader[i].int_vol = 0;
		} else {	//現在値と異なるフェーダー値を設定した時
			if (sound_code&0xFF) {	//タイマーが0以外の時
				mix_fader[i].int_vol=(signed int)(mix_fader[i].dst_vol-mix_fader[i].now_vol)/((sound_code&0xFF)*10);
				if (mix_fader[i].int_vol == 0) mix_fader[i].int_vol = 1;
			} else {	//タイマーが0の時
				mix_fader[i].now_vol = mix_fader[i].dst_vol;
				mix_fader[i].int_vol = 0;
			}
		}
///*DEBUG*/ PRINTF(("***Fader(ch=%x):Vol=%x:Dst=%x:Ctr=%x***\n", i, mix_fader[i].now_vol, mix_fader[i].dst_vol, (sound_code&0xFF) ));
		goto SdSetEnd;
	}

/*--------------------- 波形ロード (0xFExx xxxx) --------------------*/
	if ( (sound_code&0xFF000000)==0xFE000000 ){
		pak_cd_read_fg = 0;
		if (sound_code < 0xFE800000) {
// 以下のコメントを解除すると、EE側のsd_statusリードで誤動作を起こす2000/10/31
//			if ((wave_save_code != sound_code) && (se_save_code2 != sound_code)) {
			wave_load_code = sound_code;
			wave_load_status = WAV_LOAD_STATUS_LOAD;
         if( bp_wav_load_substatus )
            BP_BREAK;
         bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOAD;
//			}
		} else {	//パックファイルのロード
			pak_load_code = sound_code;
			pak_load_status = 1;
         if( bp_pak_load_substatus )
            BP_BREAK;
         bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_LOAD_HEADER;
		}
#if 1 //BP_PS2
      BP_TED_BREAK;
#else
		BP_WakeupThread(id_SdMain);	//wave_load_codeがセットされない場合もステータス書き込みする
#endif
		goto SdSetEnd;
	}
#if 1 // add K.Uehara
/*--------------------- CD波形ロード (0xCxxx xxxx) --------------------*/
	if ( (sound_code&0xF0000000)==0xC0000000 ){
		// CDからの波形ファイルのロード

		pak_load_code = ( sound_code & 0x0FFFFFFF );
		pak_load_status = 1;
      if( bp_pak_load_substatus )
         BP_BREAK;
      bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_LOAD_HEADER;
		pak_cd_read_fg = 1;
#if 1 //BP_PS2
      //handled by BP_SdMainLoopIteration
#else
		BP_WakeupThread(id_SdMain);	//wave_load_codeがセットされない場合もステータス書き込みする
#endif
		goto SdSetEnd;
	}
#endif
/*--------------------- Control Code --------------------------------*/
/*--------------------- (0xFF000000 - 0xFF0000FF) -------------------*/
	switch (sound_code) {
/*--------------------- SE Stop Mode on (Integral VR Mode) ----------*/
/*
	case 0xFF000003L:
		se_stop_fg = 1;
		 break;
*/
/*--------------------- SE Stop Mode off (Integral VR Mode) ---------*/
/*
	case 0xFF000004L:
		se_stop_fg = 0;
		 break;
*/
/*--------------------- Monoral Mode on -----------------------------*/
	case 0xFF000005L:
		sound_mono_fg = 1;
		break;
/*--------------------- Monoral Mode off ----------------------------*/
	case 0xFF000006L:
		sound_mono_fg = 0;
		break;
/*--------------------- S.E. Reverb on ------------------------------*/
	case 0xFF000007L:
		se_rev_on = 1;
		break;
/*--------------------- S.E. Reverb off -----------------------------*/
	case 0xFF000008L:
		se_rev_on = 0;
		break;

//	case 0xFF000009L:

/*--------------------- STR1 off ------------------------------------*/
	case 0xFF00000AL:
		if (str2_status[0]) str2_stop_fg[0] = 1;
		str2_iop_load_set[0] = 0;
		str_playing_fg[0] = 0;	//2001/09/11
		break;
/*--------------------- STR1 Wait -----------------------------------*/
	case 0xFF00000BL:
		str2_wait_fg[0] = 1;
/*DEBUG*/PRINTF(("*** STR WAIT ***\n"));
		break;
/*--------------------- STR1 Wait Off (Start) -----------------------*/
	case 0xFF00000CL:
		str2_wait_fg[0] = 0;
/*DEBUG*/PRINTF(("*** STR WAIT OFF (START) ***\n"));
		break;
/*--------------------- STR1 Fade Out -------------------------------*/
	case 0xFF00000DL:
		if (str2_status[0]) str2_stop_fg[0] = 2;
		break;

/*--------------------- STR2 off ------------------------------------*/
	case 0xFF00000EL:
		if (str2_status[1]) str2_stop_fg[1] = 1;
		if (lnr8_status) lnr8_stop_fg = 1;
		str2_iop_load_set[1] = 0;
		str_playing_fg[1] = 0;	//2001/09/11
		break;
/*--------------------- STR2 Wait -----------------------------------*/
	case 0xFF00000FL:
		str2_wait_fg[1] = 1;
/*DEBUG*/PRINTF(("*** STR2 WAIT ***\n"));
		break;
/*--------------------- STR2 Wait Off (Start) -----------------------*/
	case 0xFF000010L:
		str2_wait_fg[1] = 0;
/*DEBUG*/PRINTF(("*** STR WAIT OFF (START) ***\n"));
		break;
/*--------------------- STR2 Fade Out -------------------------------*/
	case 0xFF000011L:
		if (str2_status[1]) str2_stop_fg[1] = 2;
		if (lnr8_status) lnr8_stop_fg = 1;
		break;

/* case 0xFF000012L: */

/* case 0xFF000013L: */

/*--------------------- 内蔵BGM(SNG)フェーダー音量=0でスタート ------*/
	case 0xFF000014L:
		fader_off_fg = 1;
/*DEBUG*/PRINTF(("*** SET Volume Fader =0 for SNG ***(SdCode=%x)\n", sound_code));
		break;

/*--------------------- STR1 Reverb on -----------------------------*/
	case 0xFF000015L:
			rev_on_bit[1] |= ( 3 << (SE_CORE1_OFFSET+SE_TRACK_NUM) );
			rev_bit_data[1] |= (3<< (SE_CORE1_OFFSET+SE_TRACK_NUM) );
		break;
/*--------------------- STR1 Reverb off ----------------------------*/
	case 0xFF000016L:
			rev_off_bit[1] |= ( 3 << (SE_CORE1_OFFSET+SE_TRACK_NUM) );
			rev_bit_data[1] &= ~(3<< (SE_CORE1_OFFSET+SE_TRACK_NUM) );
		break;
/*--------------------- STR2 Reverb on -----------------------------*/
	case 0xFF000017L:
			rev_on_bit[1] |= ( 0xC << (SE_CORE1_OFFSET+SE_TRACK_NUM) );
			rev_bit_data[1] |= (0xC<< (SE_CORE1_OFFSET+SE_TRACK_NUM) );
		break;
/*--------------------- STR2 Reverb off ----------------------------*/
	case 0xFF000018L:
			rev_off_bit[1] |= ( 0xC << (SE_CORE1_OFFSET+SE_TRACK_NUM) );
			rev_bit_data[1] &= ~(0xC<< (SE_CORE1_OFFSET+SE_TRACK_NUM) );
		break;

/*--------------------- All STR Pause ON ---------------------------*/
	case 0xFF000019L:
			str2_pause_fg[0] = str2_pause_fg[1] = str3_pause_fg = 1;
		break;
/*--------------------- All STR Pause OFF --------------------------*/
	case 0xFF00001AL:
			str2_pause_fg[0] = str2_pause_fg[1] = str3_pause_fg = 0;
		break;

/*--------------------- STR2[0] Pause ON ---------------------------*/
	case 0xFF00001BL:
			str2_pause_fg[0] = 1;
		break;
/*--------------------- STR2[0] Pause OFF --------------------------*/
	case 0xFF00001CL:
			str2_pause_fg[0] = 0;
		break;

/*--------------------- STR2[1] Pause ON ---------------------------*/
	case 0xFF00001DL:
			str2_pause_fg[1] = 1;
		break;
/*--------------------- STR2[1] Pause OFF --------------------------*/
	case 0xFF00001EL:
			str2_pause_fg[1] = 0;
		break;

/*--------------------- 持続効果音ストップ -------------------------*/
	case 0xFF0000FDL:
		stop_jizoku_se2 = 1;
		break;
/*--------------------- 持続効果音ストップ -------------------------*/
	case 0xFF0000FEL:
		stop_jizoku_se = 1;
		break;
/*--------------------- イントロループ・スキップ -------------------*/
	case 0xFF0000FFL:
		set_sng_code_buf(sound_code);//2000/09/30
//		skip_intro_loop = 1;
		break;
/*--------------------- FXトラック再生 --------------------*/
	case 0xFF000100L:
		fx_sound_code = 1;
		break;
/*--------------------- オートメーション・フェーズ設定 --------------*/
/* 2000/01/26 */
	case 0xFF000101:
		set_sng_code_buf(sound_code);//2000/09/30
//		auto_phase_fg = 1;
		break;
	case 0xFF000102:
		set_sng_code_buf(sound_code);//2000/09/30
//		auto_phase_fg = 2;
		break;
	case 0xFF000103:
		set_sng_code_buf(sound_code);//2000/09/30
//		auto_phase_fg = 3;
		break;
	case 0xFF000104:
//		set_sng_code_buf(sound_code);//2000/09/30
		auto_phase_fg = 4;
		break;
	case 0xFF000105:
		set_sng_code_buf(sound_code);//2000/09/30
//		auto_phase_fg = 5;
		break;
	case 0xFF000106:
		set_sng_code_buf(sound_code);//2000/09/30
//		auto_phase_fg = 6;
		break;
	case 0xFF000107:
		set_sng_code_buf(sound_code);//2000/09/30
//		auto_phase_fg = 7;
		break;
	case 0xFF000108:
		set_sng_code_buf(sound_code);//2000/09/30
//		auto_phase_fg = 8;
		break;

	case 0xFFFFFFEC:	/* Stop IRQ */
//		SpuSetIRQ(SpuOff);
		break;
	case 0xFFFFFFED:	/* Restart IRQ */
//		SpuSetIRQ(SpuOn);
		break;
	case 0xFFFFFFFD:	/* Stop STR */
//		StrFadeOutStop(INT1SEC/2);
		break;

	case 0xFFFFFFFF:
	  {
#if 1 //BP_PS2
        BP_TODO_BREAK;
#else
		extern void sd_exit( void );
		sd_exit();
#endif
	  }
		break;
	default:
		break;
	}
/*-----------------------------------------------------------------------*/
SdSetEnd:
	return;
}
