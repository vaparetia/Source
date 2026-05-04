/*===================================================================
 *  Title           : "METALGEAR SOLID" SOUND
 *                  : 内蔵音源ドライバ
 *  version         : 1.00
 *  Code            : mj001
 *  Filename        : SD_DRV.C
 *  Creator         : K.C.E.JAPAN - K.Muraoka
 *  First edition   : 1996/11/07
 *===================================================================*/
#include	<stdio.h>
#include	<kernel.h>
#include	<sys/types.h>
#include	<sif.h>
#include	<sifcmd.h>
#include	<sifrpc.h>
#include	<timerman.h>
#include	<libsd.h>
#include	"sd_debug.h"
#include	"sd_incl.h"
#include	"sd_ext.h"
//#include	"g_sound.h"

#include "mgs_type.h"
#include "BP_AudioStream.h"
#include "BP_SoundSupport.h"

/*--------------------*/
/*--- Song Control ---*/
/*--------------------*/
/*SONG PAUSE*/
#define SNG_PAUSEON		0x01FFFF01
#define SNG_PAUSEOFF	0x01FFFF02
/*SONG FADE IN*/
#define SNG_FIN_S		0x01FFFF03
#define SNG_FIN_M		0x01FFFF04
#define SNG_FIN_L		0x01FFFF05
/*SONG FADE OUT & PAUSE*/
#define SNG_FOUTP_SS	0x01FFFF06
#define SNG_FOUTP_S		0x01FFFF07
#define SNG_FOUTP_M		0x01FFFF08
#define SNG_FOUTP_L		0x01FFFF09
/*SONG FADE OUT & STOP*/
#define SNG_FOUTS_SS	0x01FFFF0A
#define SNG_FOUTS_S		0x01FFFF0B
#define SNG_FOUTS_M		0x01FFFF0C
#define SNG_FOUTS_L		0x01FFFF0D
/*Kaihi MODE*/
#define SNG_KAIHI_01	0x01FFFF10
#define SNG_KAIHI_RESET	0x01FFFF11
#define SNG_KAIHI_RESET2	0x01FFFF12
/*Syukan MODE*/
#define SNG_SYUKAN_ON	0x01FFFF20
#define SNG_SYUKAN_OFF	0x01FFFF21
/*MICROPHONE MODE*/
#define SNG_MIC_ON		0x01FFFF22
#define SNG_MIC_OFF		0x01FFFF23

/*SONG PAUSE*/
#define INT_PAUSEON		0x01FFFF24
#define INT_PAUSEOFF	0x01FFFF25

/*SONG STOP*/
#define SNG_STOP		0x01FFFFFF
/* 1999/06/03 拡張SE停止 */
#define SE_EXP_STOP		0x01FFFFFD
/* 1999/06/03 全SE停止 */
#define SE_ALL_STOP		0x01FFFFFE
/*SONG PLAY CODE*/
#define SNG_PLAY_01		0x01000001	/*アクション曲*/
#define SNG_PLAY_02		0x01000002	/*潜入曲*/
#define SNG_PLAY_03		0x01000003
#define SNG_PLAY_04		0x01000004
#define SNG_PLAY_05		0x01000005
#define SNG_PLAY_06		0x01000006
#define SNG_PLAY_07		0x01000007
#define SNG_PLAY_08		0x01000008

/*---------------------- 外部宣言 -----------------------------------*/
extern void		sng_off();
extern void		se_off(int i);
extern void		se_off_exp();
extern void		se_off_all();
extern void		sng_pause();
extern void		sng_pause_off();
extern int		sound_sub(void);
extern void		spuwr(void);
extern int 		MemSpuTransWithNoLoop(int i);
extern void		WaveSpuTrans(void);

//extern struct	SETBL se_tbl[];
extern unsigned char sng_data[SNG_DATA_SIZE];
extern int		se_tracks;
extern int		id_SdMain;

extern unsigned int	str2_status[2];
extern unsigned int	str2_load_code[2];
extern unsigned int	lnr8_status;
extern unsigned int	lnr8_load_code;

extern unsigned int pak_sng_code;	// 2000/11/17

/*--------------------- プロトタイプ宣言 ----------------------------*/
void	sng_track_init(struct SOUND_W * ptr);
void	sng_adrs_set(unsigned long play_code);
void	SngFadeIn(unsigned long kind);
int		SngFadeOutP(unsigned long kind);
int		SngFadeOutS(unsigned long kind);
int		SngKaihiP(void);
void	SngKaihiReset(void);
void	SngKaihiReset2(void);
void	SngFadeWkSet(void);
void	SngFadeInt(void);
void	SngTempoInt(void);
void	se_adrs_set(int idx);
void	sd_efct_set (void);

/*===================================================================
<<< 内蔵ＢＧＭ処理フロー >>>
---------------------------------------------------------------------
1:音コード受付:SdSet()
  ･･･sd_sng_codeに音コードをセットする

2:音コードチェック:SdInt()->IntSdMain()
  ･･･制御コードならば、IntSdMain()で処理する
  ･･･ＢＧＭ再生ならば、前ＢＧＭを終了させ、sng_status=1

3:ＢＧＭロード処理:SdMain()->LoadBgmData()
  ･･･ＢＧＭデータをロードし、sng_status=2

4:ＢＧＭ再生開始:SdInt()->IntSdMain()
  ･･･ＢＧＭワークを初期化し、sng_status=3

5:ＢＧＭ再生:SdInt()->IntSdMain()
  ･･･ＢＧＭを終了する時、sng_status=4
=====================================================================*/
/*-------------------------------------------------------------------*/
/*  内蔵音源ドライバ割り込み処理メイン                               */
/*-------------------------------------------------------------------*/
void	IntSdMain(void)
{
	int			sd_sng_code;		/*音コード一時保存*/
	int			i;

//PRINTF(("%x", sng_status));
/*---------------------- ＢＧＭ音コードチェック ---------------------*/
	if (sd_sng_code_buf[sd_code_read]) {
		sd_sng_code = sd_sng_code_buf[sd_code_read];
		sd_sng_code_buf[sd_code_read] = 0;
		sd_code_read = (sd_code_read+1) & 0xF;
//PRINTF(("SngCode=%x\n", sd_sng_code));
	} else {
		sd_sng_code = 0;
	}

	if (sd_sng_code) {
		switch (sd_sng_code) {
/*----- ポーズ -----*/
		case SNG_PAUSEON:
			sng_pause_fg = 1;
         for(i=0;i<BP_DOSTREAM_COUNT;++i)
         {
            BP_AudioStreamPause( i, 1, BP_AUDIOSTREAM_PAUSE_GLOBAL );
         }
			sng_pause();
PRINTF(("SongPauseOn\n"));
			break;
/*----- ポーズ解除 -----*/
		case SNG_PAUSEOFF:
			sng_pause_off();
			sng_pause_fg = 0;
         for(i=0;i<BP_DOSTREAM_COUNT;++i)
         {
            BP_AudioStreamPause( i, 0, BP_AUDIOSTREAM_PAUSE_GLOBAL );
         }
PRINTF(("SongPauseOff\n"));
			break;
/*----- ポーズ -----*/
		case INT_PAUSEON:
			int_pause_fg = 1;
			sng_pause();
PRINTF(("InternalSoundPauseOn\n"));
			break;
/*----- ポーズ解除 -----*/
		case INT_PAUSEOFF:
			sng_pause_off();
			int_pause_fg = 0;
PRINTF(("InternalSoundPauseOff\n"));
			break;
/*----- フェードイン -----*/
		case SNG_FIN_S:
		case SNG_FIN_M:
		case SNG_FIN_L:
			if (sng_play_code != 0xFFFFFFFFL) {
				sng_fout_fg = 0;
/*				if (sng_status == 0) sng_fadein_fg = sd_sng_code;*/
				if (sng_status <= 2) sng_fadein_fg = sd_sng_code;
				else SngFadeIn(sd_sng_code);
//1999/12/22 for Mixer Fader
//				sng_tempo_move = sng_kaihi_fg = 0;
				sng_kaihi_fg = 0;

			}
PRINTF(("SongFadein\n"));
			break;
/*----- フェードアウト＆ポーズ -----*/
		case SNG_FOUTP_SS:
		case SNG_FOUTP_S:
		case SNG_FOUTP_M:
		case SNG_FOUTP_L:
			SngFadeOutP(sd_sng_code);
PRINTF(("SongFadeout&Pause\n"));
			break;
/*----- フェードアウト＆停止 -----*/
		case SNG_FOUTS_SS:
		case SNG_FOUTS_S:
		case SNG_FOUTS_M:
		case SNG_FOUTS_L:
			SngFadeOutS(sd_sng_code);
PRINTF(("SongFadeout&Stop\n"));
			break;
/*----- 回避モード・スタート -----*/
		case SNG_KAIHI_01:
			SngKaihiP();
PRINTF(("SongKaihiMode\n"));
			break;
/*----- 回避モード・リセット1 -----*/
		case SNG_KAIHI_RESET:
			SngKaihiReset();//前半トラックをONし、後半トラックをOFF
PRINTF(("SongKaihiReset\n"));
			break;
/*----- 回避モード・リセット2 -----*/
		case SNG_KAIHI_RESET2:
			SngKaihiReset2();//前半トラックをOFFし、後半トラックをON
PRINTF(("SongKaihiReset\n"));
			break;
/*----- 主観＆マイクモード -----*/
		case SNG_SYUKAN_ON:
			sng_syukan_fg = 1;
//PRINTF(("SongSyukanMode On\n"));
			break;
		case SNG_SYUKAN_OFF:
			sng_syukan_fg = 0;
//PRINTF(("SongSyukanMode Off\n"));
			break;
		case SNG_MIC_ON:
			sng_mic_fg = 1;
			break;
		case SNG_MIC_OFF:
			sng_mic_fg = 0;
			break;
/*----- 停止 -----*/
		case SE_EXP_STOP:	/* 1999/06/03 拡張SE停止*/
			se_off_exp();
			break;
		case SE_ALL_STOP:	/* 1999/06/03 全SE停止*/
			se_off_all();
			break;
		case SNG_STOP:		/* BGM停止 */
//			sng_play_code = 0;			//2000/12/07 K.Muraoka
			sng_play_code=0xFFFFFFFFL; 
			sng_off();
			sng_status = 0;
PRINTF(("SongStop\n"));
			break;

//以下、SNG再生開始のタイミングと同期する為、sd_cliから引越してきたオートメーション
/*--------------------- イントロループ・スキップ --------------------*/
	case 0xFF0000FFL:
		skip_intro_loop = 1;
		break;
/*--------------------- オートメーション・フェーズ設定 --------------*/
	case 0xFF000101:
		auto_phase_fg = 1;
		break;
	case 0xFF000102:
		auto_phase_fg = 2;
		break;
	case 0xFF000103:
		auto_phase_fg = 3;
		break;
	case 0xFF000104:
		auto_phase_fg = 4;
		break;
	case 0xFF000105:
		auto_phase_fg = 5;
		break;
	case 0xFF000106:
		auto_phase_fg = 6;
		break;
	case 0xFF000107:
		auto_phase_fg = 7;
		break;
	case 0xFF000108:
		auto_phase_fg = 8;
		break;

/*----- ソング開始 -----*/
		case SNG_PLAY_01:
		case SNG_PLAY_02:
		case SNG_PLAY_03:
		case SNG_PLAY_04:
		case SNG_PLAY_05:
		case SNG_PLAY_06:
		case SNG_PLAY_07:
		case SNG_PLAY_08:
			if (sng_play_code != sd_sng_code) {
//				if (sng_status >= 2) {	//2000/10/04 危険曲はロード中も再生を続ける
					if (sng_data[0]<(sd_sng_code&0xF)) {
						PRINTF(("ERROR:SNG PLAY CODE(%x/%x)\n", sd_sng_code, sng_data[0]));;
//PRINTF(("%x:%x:%x:%x\n", sng_data[0], sng_data[1], sng_data[2], sng_data[3]));
//PRINTF(("%x:%x:%x:%x\n", sng_data[4], sng_data[5], sng_data[6], sng_data[7]));
//PRINTF(("%x:%x:%x:%x\n", sng_data[8], sng_data[9], sng_data[10], sng_data[11]));
//PRINTF(("%x:%x:%x:%x\n", sng_data[12], sng_data[13], sng_data[14], sng_data[15]));
						sd_sng_code = 0;
					} else {
						sng_status = 2;
						if (sng_play_code == 0xFFFFFFFFL) {
							for (i=0;i<SNG_TRACK_NUM;i++) {
								sng_fade_time[i] = 0;
								sng_fade_value[i] = 0;
								sng_master_vol[i] = 0;
							}
							sng_fout_term[0] = sng_fout_term[1] = 0;
						}
						sng_play_code = sd_sng_code;
/*						sd_sng_code = 0;*/
						sng_off();
						sng_pause_fg = int_pause_fg = 0;
//1999/12/22 for Mixer Fader
//						sng_tempo_move = sng_kaihi_fg = 0;
						sng_kaihi_fg = 0;
//2000/09/30
						auto_env_pos = auto_env_pos2 = auto_phase_fg = 0;
						skip_intro_loop = 0;
					}
//				} else PRINTF(("sng_status=%x\n", sng_status));
			} else PRINTF(("SameSongHasAlreadyPlayed\n"));
			break;
/*----- ソング・ロード（pakファイルのロードはここを通らない） -----*/
		default:
			if (sng_load_code != sd_sng_code) {
				sng_load_code = sd_sng_code;
//				sng_play_code = 0;	//2000/10/04 危険曲はロード中も再生を続ける
//				sng_status = 1;	//2000/10/04 危険曲はロード中も再生を続ける
//				sng_off();			//2000/10/04 危険曲はロード中も再生を続ける
#if 1 //BP_PS2
            BP_TODO_BREAK;
#else
				BP_WakeupThread(id_SdMain);
#endif
			}
			break;
		}
/*		sd_sng_code = 0;	1998/04/25 BGM用音コードをリングバッファにする*/
	}
/*----- ソング・ロード（pakファイルの場合） -----*/
	if (pak_sng_code) {
		sng_load_code = pak_sng_code;

      bp_sng_load_substatus = BP_SNG_LOAD_SUBSTATUS_LOAD;

		pak_sng_code = 0;
	}
/*---------------------- ＢＧＭ再生 ---------------------------------*/
	switch (sng_status) {
	case 1:		//2000/10/04 危険曲はロード中も再生を続ける
		break;
	case 2:
//DEBUG ファイルロード中にＢＧＭ再生コードが発行された時の対処
/*DEBUG*/if (sng_load_code || pak_load_status) {
///*DEBUG*/	PRINTF(("Can't Start BGM Play While Loading!! (LoadCode=%x / PakStat=%x)\n", sng_load_code, pak_load_status));
/*DEBUG*/	break;
/*DEBUG*/		}
		if (sng_play_code && (sng_play_code != 0xFFFFFFFF)) {

//			if (wave_load_status >= 1) {
//PRINTF(("WaitForStartSongPlayWhileLoadWave(wave_load_status=%x)\n", wave_load_status));
//				break;	/*1998/03/14 波形ロード中は再生開始しない*/
//			}

			sng_adrs_set(sng_play_code);
			SngFadeWkSet();
			if (fader_off_fg) {
				for (i=0; i<SNG_TRACK_NUM; i++) {
					mix_fader[i].now_vol = mix_fader[i].dst_vol = 0;
					mix_fader[i].int_vol = 0;
/*2001/04/25*/		mix_fader[i].now_frq = mix_fader[i].last_frq = 0;
				}
				fader_off_fg = 0;
PRINTF(("*** SNG:FADER OFF ***\n"));
			} else {
				for (i=0; i<SNG_TRACK_NUM; i++) {
					mix_fader[i].now_vol = mix_fader[i].dst_vol = 0xFFFF;
					mix_fader[i].int_vol = 0;
/*2001/04/25*/		mix_fader[i].now_frq = mix_fader[i].last_frq = 0;
				}
PRINTF(("*** SNG:FADER ON ***\n"));
			}
			sng_status = 3;
		}
		break;
	case 3:
			SngFadeInt();
			SngTempoInt();
//			for(mtrack = 0; mtrack < SNG_TRACK_NUM/2; mtrack++){
			for(mtrack = 0; mtrack < SNG_TRACK_NUM; mtrack++){
				if (mtrack < 24) {
					keyd[0] = 1<<mtrack;
					keyd[1] = 0;
					if ((song_end[0] & keyd[0]) != 0) continue;
				} else {
					keyd[0] = 0;
					keyd[1] = 1<<(mtrack-24);
					if ((song_end[1] & keyd[1]) != 0) continue;
				}
				sptr = &sound_w[mtrack];

				if(sptr->mpointer == 0) {
					song_end[0] |= keyd[0];
					song_end[1] |= keyd[1];
					continue;
				}

				mptr = sptr->mpointer;
				if (sound_sub() != 0) {
					song_end[0] |= keyd[0];
					song_end[1] |= keyd[1];
					sptr->mpointer = 0;
				} else {
					sptr->mpointer = mptr;
				}
			}
			if ( (song_end[0] == 0x00FFFFFF)
				&& (song_end[1] == 0x000000FF) ) sng_status = 4;
//PRINTF(("song_end=%x:%x\n", (unsigned int)song_end[0], (unsigned int)song_end[1]));

		if (fx_sound_code >= 2) fx_sound_code = 0;	// 2000/01/17
		if (skip_intro_loop >= 2) skip_intro_loop = 0;	// 2000/01/18
		break;
	case 4:
		sng_off();
		sng_play_code = 0;
		sng_status = 2;
		break;
	}
/*---------------------- 効果音再生 ---------------------------------*/
	for(mtrack = SE_START_TRACK; mtrack < INTERNAL_TRACK_NUM; mtrack++){
		/* 再生スタート */
		if (se_tracks < 2) {	/* ２Tr以上の効果音は、全てのTrセットまで待つ */
			if (se_request[mtrack-SNG_TRACK_NUM].code) {
/*PRINTF(("DRV:SE Requested(%x)\n", se_request[mtrack-SNG_TRACK_NUM].code));*/
				se_off(mtrack-SNG_TRACK_NUM);
				se_adrs_set(mtrack-SNG_TRACK_NUM);
				continue;
			}
		}
		/* 再生中 */
		keyd[0] = 0;
		keyd[1] = 1<<(mtrack-24);
		if ((song_end[1] & keyd[1]) != 0) continue;

		sptr = &sound_w[mtrack];
		if(sptr->mpointer == 0) {
			song_end[1] |= keyd[1];
			continue;
		}

		mptr = sptr->mpointer;
		if (sound_sub() != 0) {
			song_end[1] |= keyd[1];
			sptr->mpointer = 0;
		} else {
			sptr->mpointer = mptr;
		}
	}
	if (stop_jizoku_se >= 2) stop_jizoku_se = 0;	/*1998/01/27 K.Muraoka*/
	if (stop_jizoku_se2 >= 2) stop_jizoku_se2 = 0;	/*2001/07/25 K.Muraoka*/

/*----- SPU2へ内蔵音源用波形転送 -----*/
/* 2000/01/14 */
	if ((BP_sceSdVoiceTransStatus(DMA_CH, SD_TRANS_STATUS_CHECK)) == 1) { //DMA ch.=0
		if ( (wave_load_status == WAV_LOAD_STATUS_SPU_TRANS) || (wave_load_status == 4) ) {
			WaveSpuTrans();
#if 0 //BP - handled by BP_SdMainLoopIteration()
			BP_WakeupThread(id_SdMain);
#endif
		} else {
			for(mtrack = SE_START_TRACK; mtrack < INTERNAL_TRACK_NUM; mtrack++){
				keyd[0] = 0;
				keyd[1] = 1<<(mtrack-24);
				if (MemSpuTransWithNoLoop(mtrack)) break;
			}
		}
	}
/*DEBUG*/	else PRINTF((">"));
/* 2000/01/14 Add End */

	spuwr();
	sd_efct_set();
}

/*-------------------------------------------------------------------*/
/* ソング・フェードイン                                              */
/*-------------------------------------------------------------------*/
void SngFadeIn(unsigned long kind)
{
	int			i;
	
	switch (kind) {
	case SNG_FIN_S:sng_fadein_time = SNG_VOL / (INT1SEC); break;
	case SNG_FIN_M:sng_fadein_time = SNG_VOL / (INT1SEC*3); break;
	case SNG_FIN_L:sng_fadein_time = SNG_VOL / (INT1SEC*5); break;
	}
	if (sng_fadein_time == 0) sng_fadein_time = 1;

	for (i=0;i<SNG_TRACK_NUM;i++) sng_fade_time[i] = 0;
	sng_fout_term[0] = sng_fout_term[1] = 0;
}

/*-------------------------------------------------------------------*/
/* ソング・フェードアウト＆ポーズ                                    */
/*-------------------------------------------------------------------*/
/* OUT: 0 = Completed                                                */
/*     -1 = BGM Stopped                                              */
/*-------------------------------------------------------------------*/
int SngFadeOutP(unsigned long kind)
{
	long	fade_time;
	int		i;

	if ((sng_status)  && (sng_fout_term[0] != 0x00FFFFFF) && (sng_fout_term[1] != 0x000000FF)) {

		switch (kind) {
		case SNG_FOUTP_SS:fade_time = SNG_VOL / (INT1SEC/2); break;
		case SNG_FOUTP_S:fade_time = SNG_VOL / (INT1SEC); break;
		case SNG_FOUTP_M:fade_time = SNG_VOL / (INT1SEC*3); break;
		case SNG_FOUTP_L:fade_time = SNG_VOL / (INT1SEC*5); break;
		}
		if (fade_time == 0) fade_time = 1;

		for (i=0;i<SNG_TRACK_NUM;i++) {
			if (i < 24) {
				if ((sng_fout_term[0]&(1<<i)) == 0) {
					sng_fade_time[i] = fade_time;
				}
			} else {
				if ((sng_fout_term[1]&(1<<(i-24))) == 0) {
					sng_fade_time[i] = fade_time;
				}
			}
		}
		sng_fadein_time = 0;
		return(0);
	} else {
		return (-1);
	}
}

/*-------------------------------------------------------------------*/
/* ソング・フェードアウト＆停止                                      */
/*-------------------------------------------------------------------*/
/* OUT: 0 = Completed                                                */
/*     -1 = BGM Stopped                                              */
/*-------------------------------------------------------------------*/
int SngFadeOutS(unsigned long kind)
{
	long	fade_time;
	int		i;

	if ( ((sng_status) && (sng_fout_term[0]!=0x00FFFFFF) && (sng_fout_term[1]!=0x000000FF))
		|| ((sng_status) && (sng_fadein_time)) ) {
		switch (kind) {
		case SNG_FOUTS_SS:fade_time = SNG_VOL / (INT1SEC/2); break;
		case SNG_FOUTS_S:fade_time = SNG_VOL / (INT1SEC); break;
		case SNG_FOUTS_M:fade_time = SNG_VOL / (INT1SEC*3); break;
		case SNG_FOUTS_L:fade_time = SNG_VOL / (INT1SEC*5); break;
		}
		if (fade_time == 0) fade_time = 1;

		for (i=0;i<SNG_TRACK_NUM;i++) {
			if (i <24) {
				if ((sng_fout_term[0]&(1<<i)) == 0) {
					sng_fade_time[i] = fade_time;
				}
			} else {
				if ((sng_fout_term[1]&(1<<(i-24))) == 0) {
					sng_fade_time[i] = fade_time;
				}
			}
		}
		sng_fadein_time = 0;
		sng_play_code=0xFFFFFFFFL; 
PRINTF(("SNG FADEOUT START(status=%x)\n", sng_status));
		return(0);
	} else {
		sng_fadein_time = 0;		// 2000/12/07 K.Muraoka
		sng_play_code=0xFFFFFFFFL; 
		sng_off();
		sng_status = 0;
PRINTF(("SNG FADEOUT CANCELED(status=%x) & SONG STOP\n", sng_status));
		return(-1);
	}
}

/*-------------------------------------------------------------------*/
/* 回避モード移行＆ポーズ                                            */
/*-------------------------------------------------------------------*/
/* OUT: 0 = Completed                                                */
/*     -1 = BGM Stopped                                              */
/*-------------------------------------------------------------------*/
#define KAIHI_FO_RYTHM	(SNG_VOL/(INT1SEC*15))
/*#define KAIHI_FO_RYTHM	(SNG_VOL/(INT1SEC*8))*/
#define KAIHI_FO_CHORD	(SNG_VOL/(INT1SEC*8))
/*#define KAIHI_FO_CHORD	(SNG_VOL/(INT1SEC*3))*/

int SngKaihiP(void)
{
#if 1
	int		i;

	if (!sng_kaihi_fg) {
		for (i=0; i<(SNG_TRACK_NUM/2); i++) {
			mix_fader[i].dst_vol = 0;
			mix_fader[i].int_vol = (signed int)(mix_fader[i].dst_vol-mix_fader[i].now_vol)/(INT1SEC*6);
		}
		for (i=(SNG_TRACK_NUM/2); i<SNG_TRACK_NUM; i++) {
			mix_fader[i].dst_vol = 0xFFFF;
			mix_fader[i].int_vol = (signed int)(mix_fader[i].dst_vol-mix_fader[i].now_vol)/(INT1SEC*5);
		}
		sng_kaihi_fg = 1;
	} else {
		for (i=0; i<(SNG_TRACK_NUM/2); i++) {
			mix_fader[i].dst_vol = 0xFFFF;
			mix_fader[i].int_vol = (signed int)(mix_fader[i].dst_vol-mix_fader[i].now_vol)/(INT1SEC/2);
		}
		for (i=(SNG_TRACK_NUM/2); i<SNG_TRACK_NUM; i++) {
			mix_fader[i].dst_vol = 0;
			mix_fader[i].int_vol = (signed int)(mix_fader[i].dst_vol-mix_fader[i].now_vol)/(INT1SEC*2);
		}
		sng_kaihi_fg = 0;
	}
	return(0);
#else
	int		i;

	if ((sng_status) && (sng_fout_term[0] != 0x00FFFFFF) && (sng_fout_term[1] != 0x000000FF)) {
		sng_fade_time[2] = KAIHI_FO_RYTHM;
		sng_fade_time[3] = KAIHI_FO_RYTHM;
		for (i=4;i<SNG_TRACK_NUM;i++) {
			if (i <24) {
				if ((sng_fout_term[0]&(1<<i)) == 0) {
					sng_fade_time[i] = KAIHI_FO_CHORD;
				}
			} else {
				if ((sng_fout_term[1]&(1<<(i-24))) == 0) {
					sng_fade_time[i] = KAIHI_FO_CHORD;
				}
			}
		}
		sng_fadein_time = 0;
		sng_kaihi_fg = 1;
		return(0);
	} else {
		return (-1);
	}
#endif
}
/*回避モード・リセット*/
void SngKaihiReset(void)
{
	int		i;

	for (i=0; i<(SNG_TRACK_NUM/2); i++) {
		mix_fader[i].now_vol = 0xFFFF;
		mix_fader[i].dst_vol = 0xFFFF;
		mix_fader[i].int_vol = 0;
	}
	for (i=(SNG_TRACK_NUM/2); i<SNG_TRACK_NUM; i++) {
		mix_fader[i].now_vol = 0;
		mix_fader[i].dst_vol = 0;
		mix_fader[i].int_vol = 0;
	}
	sng_kaihi_fg = 0;
}
void SngKaihiReset2(void)
{
	int		i;

	for (i=0; i<(SNG_TRACK_NUM/2); i++) {
		mix_fader[i].now_vol = 0;
		mix_fader[i].dst_vol = 0;
		mix_fader[i].int_vol = 0;
	}
	for (i=(SNG_TRACK_NUM/2); i<SNG_TRACK_NUM; i++) {
		mix_fader[i].now_vol = 0xFFFF;
		mix_fader[i].dst_vol = 0xFFFF;
		mix_fader[i].int_vol = 0;
	}
	sng_kaihi_fg = 0;
}
/*-------------------------------------------------------------------*/
/* ソングフェード・再生スタート時のワーク初期化                      */
/*-------------------------------------------------------------------*/
void	SngFadeWkSet(void)
{
	int			i;

	if (sng_fadein_fg == 0) {
		sng_fadein_time = 0;
		for (i=0;i<SNG_TRACK_NUM;i++) sng_fade_time[i] = 0;
		for (i=0;i<SNG_TRACK_NUM;i++) sng_fade_value[i] = 0;
	} else {
		switch (sng_fadein_fg) {
		case SNG_FIN_S:
		case SNG_FIN_M:
		case SNG_FIN_L:
			SngFadeIn(sng_fadein_fg);
			for (i=0;i<SNG_TRACK_NUM;i++) sng_fade_value[i] = SNG_VOL;
			sng_fadein_fg = 0;
		}
	}
	sng_fout_term[0] = sng_fout_term[1] = 0;
	sng_fout_fg = 0;			/* For FadeOut&Continue Pause */
}


/*-------------------------------------------------------------------*/
/* ソングフェード・割り込み処理                                      */
/*-------------------------------------------------------------------*/
/* 乗算値 (Max.0xFFFF - Min.0x0000)                                  */
/*-------------------------------------------------------------------*/

/*#define	SYUKAN_MINUS_VOL	0x8000*/
#define	SYUKAN_MINUS_VOL	0x5000	//11/16
#define	MIC_MINUS_VOL	0x9000		//7/16
#define	VOX_MINUS_VOL	0x6000		//10/16

void	SngFadeInt(void)
{
	unsigned long		vol1, vol2, vol3, vol4;
	int					i, fout_on=0, fin_off=0;

	if (sng_status >= 3) {
		for (i=0;i<SNG_TRACK_NUM; i++) fout_on |= sng_fade_time[i];
		if (fout_on) {
/*フェードアウト・マイナス音量計算*/
			for (i=0;i<SNG_TRACK_NUM; i++) {
				if (sng_fade_time[i]) {
					sng_fade_value[i] += sng_fade_time[i];
					if (SNG_VOL <= sng_fade_value[i]) {
						if (i < 24) sng_fout_term[0] |= (1<<i);
						else sng_fout_term[1] |= (1<<(i-24));
						sng_fade_value[i] = SNG_VOL;
						sng_fade_time[i] = 0;
					}
					if ((sng_fout_term[0] == 0x00FFFFFF) &&
						(sng_fout_term[1] == 0x000000FF)) {
						if (sng_play_code == 0xFFFFFFFFL) {
							sng_status=4;
						} else {
							sng_fout_fg = 1; /* For FadeOut&Continue Pause */
						}
					} else {
						sng_fout_fg = 0; /* For FadeOut&Continue Pause */
					}
				}
			}
		} else {
/*音声再生時マイナス音量計算*/
			if ((str2_load_code[0] && str2_status[0])||(str2_load_code[1] && str2_status[1]) ||(lnr8_load_code && lnr8_status)) {
				if (vox_on_vol < VOX_MINUS_VOL) {
					vox_on_vol += (VOX_MINUS_VOL/(INT1SEC/2));
					if (vox_on_vol > VOX_MINUS_VOL) vox_on_vol = VOX_MINUS_VOL;
				}
			} else {
				if (vox_on_vol) {
					vox_on_vol -= (VOX_MINUS_VOL/INT1SEC);
					if (vox_on_vol < 0) vox_on_vol = 0;
				}
			}
/*フェードインマイナス音量計算*/
			if (sng_fadein_time) {
				for (i=0;i<SNG_TRACK_NUM; i++) {
					if (sng_fade_value[i] <= sng_fadein_time) sng_fade_value[i] = 0;
					else sng_fade_value[i] -= sng_fadein_time;
					fin_off |= sng_fade_value[i];
				}
				if (!fin_off) {
					sng_fadein_time = 0;
				}
			}
		}

/*主観＆マイクモード時マイナス音量計算*/
		if (sng_syukan_fg) {
			if (sng_syukan_vol < SYUKAN_MINUS_VOL) {
				sng_syukan_vol += (SYUKAN_MINUS_VOL/(INT1SEC/2));
				if (sng_syukan_vol > SYUKAN_MINUS_VOL) sng_syukan_vol = SYUKAN_MINUS_VOL;
			}
		} else {
			if (sng_syukan_vol) {
				sng_syukan_vol -= (SYUKAN_MINUS_VOL/INT1SEC);
				if (sng_syukan_vol < 0) sng_syukan_vol = 0;
			}
		}
		if (sng_mic_fg) {
			if (sng_mic_vol < MIC_MINUS_VOL) {
				sng_mic_vol += (MIC_MINUS_VOL/(INT1SEC/2));
				if (sng_mic_vol > MIC_MINUS_VOL) sng_mic_vol = MIC_MINUS_VOL;
			}
		} else {
			if (sng_mic_vol) {
				sng_mic_vol -= (MIC_MINUS_VOL/INT1SEC);
				if (sng_mic_vol < 0) sng_mic_vol = 0;
			}
		}
		if (sng_mic_vol > sng_syukan_vol) vol4 = sng_mic_vol;
		else vol4 = sng_syukan_vol;

		for (i=0;i<SNG_TRACK_NUM; i++) {
			vol1 = SNG_VOL;	//ＢＧＭフェード割合の最大値
//2000/07/17 Add fg_syukan_off
//			if ( (sng_syukan_vol >= vox_on_vol)&&(fg_syukan_off[i] == 0) ) vol3 = sng_syukan_vol;
//2001/04/10 Add MIC MODE
//			if ( (sng_syukan_vol >= vox_on_vol)&&(fg_syukan_off[i] == 0) ) vol3 = vol4;
//2001/08/27 修正(sng_syukan_vol ==> vol4に変更)
			if ( (vol4 >= vox_on_vol)&&(fg_syukan_off[i] == 0) ) vol3 = vol4;
			else vol3 = vox_on_vol;

			if (sng_fade_value[i] > vol3) {
				vol2 = sng_fade_value[i];
			} else {
				vol2 = vol3;
			}
			if (vol1 < vol2) {
				vol1 = 0;
			} else {
				vol1 -= vol2;
			}
/* Add Mix Fader */	//99/12/22
			if (mix_fader[i].now_vol != mix_fader[i].dst_vol) {
				mix_fader[i].now_vol += mix_fader[i].int_vol;
				if (mix_fader[i].int_vol >= 0) {
					if (mix_fader[i].now_vol > mix_fader[i].dst_vol) {
						mix_fader[i].now_vol = mix_fader[i].dst_vol;
						mix_fader[i].int_vol = 0;
					}
				} else {
					if ((signed int)mix_fader[i].now_vol < (signed int)mix_fader[i].dst_vol) {
						mix_fader[i].now_vol = mix_fader[i].dst_vol;
						mix_fader[i].int_vol = 0;
					}
				}
			}
			vol1 = (vol1 * mix_fader[i].now_vol)/(0x10000-1);
//if(i==0)
//PRINTF(("%x\n", mix_fader[i].now_vol));
			sng_master_vol[i] = vol1;
		}
	}
}

/*-------------------------------------------------------------------*/
/* ソング・テンポ・フェード割り込み処理                              */
/*-------------------------------------------------------------------*/
#define	MOVE_TEMPO_VALUE	0x10
void	SngTempoInt(void)
{
// 1999/12/22 for Mixer Fader
//		if (sng_kaihi_fg) {
//			if (sng_tempo_move < (MOVE_TEMPO_VALUE*0x10)) {
//				++sng_tempo_move;
//			}
//		} else {
//			if (sng_tempo_move) {
//				if (sng_tempo_move <= MOVE_TEMPO_VALUE) sng_tempo_move = 0;
//				else sng_tempo_move -= MOVE_TEMPO_VALUE;
//			}
//		}
}

/*-------------------------------------------------------------------*/
/* サウンド・ワークの初期化											 */
/*-------------------------------------------------------------------*/
void init_sng_work(void)
{
	for(mtrack = 0; mtrack < INTERNAL_TRACK_NUM; mtrack++){
		sptr = &sound_w[mtrack];
		sptr->mpointer = 0;
		sptr->lp1_addr = sptr->lp2_addr = sptr->lp3_addr=0;
		sng_track_init(sptr);
	}
	keyons[0] = keyons[1] = 0;				/* キー・オン・ポート */
	keyoffs[0] = keyoffs[1] = 0;			/* キー・オフ・ポート */
//	sng_load_code = sng_play_code = 0;	/*1999/12/13 K.Muraoka*/
	sng_play_code = 0;
}

/*----------------------------------------------------------------------*/
/*  title:  各種フラグ・リセット (エディタでのFLG制御コード）           */
/*----------------------------------------------------------------------*/
void flg_reset(i)
{
//主観モード時、音量下げないフラグ
	fg_syukan_off[i] = 0;
//SEモードに関わらず、トラック内の情報でリバーブをセットする
	fg_rev_set[i] = 0;
}

/*-------------------------------------------------------------------*/
/* ソングデータ・アドレスセット                                      */
/*-------------------------------------------------------------------*/
void sng_adrs_set(unsigned long play_code)
{
	int				i,pos;
	unsigned long	base, offset;

	play_code &= 0xF;
	base = ((unsigned long)sng_data[(play_code*4)+3])<<24;
	base += ((unsigned long)sng_data[(play_code*4)+2])<<16;
	base += ((unsigned long)sng_data[(play_code*4)+1])<<8;
	base += (unsigned long)sng_data[(play_code*4)];
/*PRINTF(("SNG BASE=%x\n", base));*/
	song_end[0] = 0;
	song_end[1] &= 0x00FFFF00;
/*--------------------- 各トラックのスタートアドレス設定 ----------------*/
    for( i = 0; i < SNG_TRACK_NUM; i++){
        pos     = (i<<2);
        offset  = (sng_data[base+pos+2]<<16);	/* High */
        offset += (sng_data[base+pos+1]<<8);		/* Mid  */
        offset +=  sng_data[base+pos];			/* Low  */
        if(offset){							/* 0=Track Off */
            sound_w[i].mpointer = sng_data + offset;
            sng_track_init(&sound_w[i]);
			flg_reset(i);	// 2001/07/16
        } else {
			if (i < 24) song_end[0] |= (1 << i);
			else song_end[1] |= (1 << (i-24));
        }
	}
/*--------------------- 各ポート共通ワーク初期化 --------------------*/
	keyons[0] = 0;				/* キー・オン・ポート */
	keyons[1] &= 0x00FFFF00;

/*1998.6.15 前曲のsng_off()が効かなくなるので、以下をコメント*/
//	keyoffs &= ~(long)SNG_BITS;				/* キー・オフ・ポート */

/* 2000/01/17 */
	fx_sound_code = 0;
///* 2000/01/26 */ -->ここで初期化するのでは遅い。もっと早いタイミングで行なう事にした(2000/09/30)
//	auto_env_pos = auto_env_pos2 = auto_phase_fg = 0;
//	skip_intro_loop = 0;
}

/*-------------------------------------------------------------------*/
/* 効果音データ・アドレスセット                                      */
/*-------------------------------------------------------------------*/
void se_adrs_set(int idx)
{
	se_playing[idx].code = se_request[idx].code;
	se_playing[idx].pri = se_request[idx].pri;
	se_playing[idx].kind = se_request[idx].kind;
	se_playing[idx].character = se_request[idx].character;
	se_playing[idx].addr = se_request[idx].addr;
   se_playing[idx].bp_angle = se_request[idx].bp_angle;
   se_playing[idx].bp_surround_type = se_request[idx].bp_surround_type;

	se_request[idx].code = 0;
	se_request[idx].pri = 0;
	se_request[idx].character = 0;

	sng_track_init(&sound_w[SNG_TRACK_NUM+idx]);
	flg_reset(SNG_TRACK_NUM+idx);	//2001/07/16 

/* トラック音量 (v)=0000|0000|pppp|ppvv|vvvv|0000|0000|0000 */
//	se_vol[idx] = (se_playing[idx].code & 0x3F00) >> 8;
	se_vol[idx] = (se_playing[idx].code & 0x3F000) >> 12;//常駐を0x100個、入替を0x100個に拡張する
/* パン  (p)=0000|0000|pppp|ppvv|vvvv|0000|0000|0000 */
//    se_pan[idx] = ((se_playing[idx].code>>16)+0x20)&0x3F;
    se_pan[idx] = (se_playing[idx].code>>18)&0x3F;//パン範囲を0-0x3Fに変更(2000/06/29)
//    se_pan[idx] = ((se_playing[idx].code>>18)+0x20)&0x3F;//常駐を0x100個、入替を0x100個に拡張する
/* データアドレス */
    se_bp_angle[idx] = se_request[idx].bp_angle;
    se_bp_surround_type[idx] = se_request[idx].bp_surround_type;

	sound_w[SNG_TRACK_NUM+idx].mpointer = se_playing[idx].addr;
//PRINTF(("PLAY %x:pri=%x;kind=%x,addr=%x\n", se_playing[idx].code, se_playing[idx].pri, se_playing[idx].kind,se_playing[idx].addr));
	song_end[1] &= ~(1<<((SNG_TRACK_NUM-24)+idx));
	keyons[1] &= ~(1<<((SNG_TRACK_NUM-24)+idx));
	keyoffs[1] &= ~(1<<((SNG_TRACK_NUM-24)+idx));

//PRINTF(("Ch=%d:song_end=%x:%x\n", idx, (u_int)song_end[0], (u_int)song_end[1]));
//PRINTF(("      keyons=%x:%x\n", idx, (u_int)keyons[0], (u_int)keyons[1]));

	if (se_playing[idx].kind) {	/*通常ＳＥのみ*/
		if (se_rev_on) {
			rev_on_bit[1] |= (1<<((SNG_TRACK_NUM-24)+idx));
			rev_bit_data[1] |= (1<<((SNG_TRACK_NUM-24)+idx));
		} else {
			rev_off_bit[1] |= (1<<((SNG_TRACK_NUM-24)+idx));
			rev_bit_data[1] &= ~(1<<((SNG_TRACK_NUM-24)+idx));
		}
	}
}

/*========================================================================
*   エフェクト設定
=========================================================================*/
void sd_efct_set (void)
{
//	sceSdEffectAttr	r_attr;

	switch (core_efct_wk.status) {
	case 0: break;
/*----- エフェクトをリセット -----*/
	case 1:
//		BP_sceSdSetParam( SD_CORE_1|SD_P_EVOLL , 0);
//		BP_sceSdSetParam( SD_CORE_1|SD_P_EVOLR , 0);
//		sceSdSetAddr( SD_CORE_1|SD_A_EEA, 0x1FFFFF );
//		r_attr.depth_L  = 0;
//		r_attr.depth_R  = 0;
//		r_attr.mode = core_efct_wk.mode;
//		sceSdSetEffectAttr( SD_CORE_1, &r_attr ); //Reverb=off
//		core_efct_wk.status = 2;
//		break;
/*----- エフェクト出力リスタート -----*/
	case 2:
		BP_sceSdSetParam( SD_CORE_1|SD_P_EVOLL , core_efct_wk.depth);
		BP_sceSdSetParam( SD_CORE_1|SD_P_EVOLR , core_efct_wk.depth);
		core_efct_wk.status = 0;
/*DEBUG*/
//BP_PS2 PRINTF(("Efct:Address = %x - %x / ", sceSdGetAddr(SD_CORE_1 | SD_A_ESA), sceSdGetAddr(SD_CORE_1 | SD_A_EEA) ));
PRINTF(("mode=%x depth=%x\n", core_efct_wk.mode, core_efct_wk.depth));
		break;
	}
}

/*-------------------------------------------------------------------*/
/*  title:  再生時のＳＮＧワーク初期化						 		 */
/*-------------------------------------------------------------------*/

void sng_track_init(struct SOUND_W * ptr)
{
	ptr->rdmd = 0;			/* ＬＦＯ深さ */
	ptr->ngc = 1;			/* キーオフ・カウンター */
	ptr->ngo = 0;			/* キーオフ・カウンター */
	ptr->ngs = 0;			/* ステップ・タイム */
	ptr->ngg = 0;			/* ゲート・タイム */
	ptr->lp1_cnt = 0;		/* ループ・カウンター */
	ptr->lp2_cnt = 0;
	ptr->lp1_vol = 0;		/* ループ音量 */
	ptr->lp2_vol = 0;
	ptr->lp1_freq = 0;		/* ループ周波数 */
	ptr->lp2_freq = 0;
	ptr->pvoc = 0;			/* パート・ボリューム・チェンジ・カウンター */
	ptr->pvod = 0x7F;		/* パート・ボリューム・データ */
	ptr->vol = 0x7F;		/* パート・ボリューム (velosity)*/
	ptr->pand = 0x1400;		/* パン・データ */
	ptr->panc = 0;			/* パン移動カウンタ */
	ptr->panf = 0x14;		/* パン・データ */
	ptr->panoff = 0;		/* drum パン off flag */
	ptr->panmod = 0;		/* パン・モード */
	ptr->swsk = 0;			/* スィープ／ポルタメントＳＷ */
	ptr->swsc = 0;			/* スィープ・カウンター */
	ptr->vibd = 0;			/* ビブラート深さ */
	ptr->vibdm = 0;			/* ビブラート深さ */
	ptr->tred = 0;			/* トレモロ深さ */
	ptr->snos = 0;			/* 音色ナンバー */
	ptr->ptps = 0;			/* パート・トランスポーズ値 */
	ptr->dec_vol = 0;		/* 音量補正値 */
	ptr->tund = 0;			/* デチューン・データ */
	ptr->tmpd = 0x01;		/* テンポ・カウンタ */
	ptr->tmp = 0xFF;		/* テンポ・データ */
	ptr->tmpc = 0;			/* テンポ移動カウンター */
/* 2000/01/17 */
	ptr->fx_time_base = 0;	/* FX wait タイムベース */
	ptr->fx_ngc = 1;		/* FX wait ステップカウンタ*/
	ptr->fx_play_fg = 0;	/* FX再生中フラグ */
	ptr->fx_on = 0;			/* 別トラックのFX再生on */
/* 2000/01/26 */
	ptr->auto_pos = 0xFFFFFFFF;	/*位置データ保存*/
	ptr->auto_mode = 0;	/* 2=エンベロープモード、1=タイマーモード */
	ptr->auto1_vol = 0;	/* オートメーション1音量 */
	ptr->auto2_vol = 0;	/* オートメーション2音量 */
	ptr->auto3_vol = 0;	/* オートメーション3音量 */
	ptr->auto4_vol = 0;	/* オートメーション4音量 */
	ptr->auto5_vol = 0;	/* オートメーション5音量 */
	ptr->auto6_vol = 0;	/* オートメーション6音量 */
	ptr->auto7_vol = 0;	/* オートメーション7音量 */
	ptr->auto8_vol = 0;	/* オートメーション8音量 */
	ptr->auto1_tim = 0;	/* オートメーション1位置/タイマー */
	ptr->auto2_tim = 0;	/* オートメーション2位置/タイマー */
	ptr->auto3_tim = 0;	/* オートメーション3位置/タイマー */
	ptr->auto4_tim = 0;	/* オートメーション4位置/タイマー */
	ptr->auto5_tim = 0;	/* オートメーション5位置/タイマー */
	ptr->auto6_tim = 0;	/* オートメーション5位置/タイマー */
	ptr->auto7_tim = 0;	/* オートメーション5位置/タイマー */
	ptr->auto8_tim = 0;	/* オートメーション5位置/タイマー */
}

