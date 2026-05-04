/* sd_ioset.c サウンド・エディター Ｉ／Ｏプログラム */

#include	<stdio.h>
#include	<kernel.h>
#include	<sys/types.h>
#include	<sif.h>
#include	<sifcmd.h>
#include	<sifrpc.h>
#include	<libsd.h>

#include "sd_debug.h"
#include "sd_incl.h"
#include "sd_ext.h"

#include "BP_EndianSupport.h"
#include "BP_SoundSupport.h"

extern void 		swpadset(int xfreq);
/*extern unsigned char	wavs;*/
extern unsigned int 	freq_tbl[];
extern unsigned char	vol_tbl[128];
extern void		sfx_send(void);
extern void		graph_key_on(int mtrack);
extern void		graph_key_off(int mtrack);
extern void	 	graph_level_set(int mtrack, int y);
extern void 		midi_keyon(unsigned char note_w);
extern void		midi_keyoff_check(unsigned int note_w);
extern void 		midi_keyoff(unsigned char note_w);
extern void 		midi_all_off(void);

extern unsigned char *se_data;

/************************************************************************/
/*						関数定義										*/
/************************************************************************/

/*
void			dspwr(unsigned int reg, unsigned int n);
*/
void			keyon(void), keyoff(void);
void			tone_set(unsigned int n);
void			freq_set(unsigned int freq);
void			pan_set2(unsigned char x);
void			sound_off();
void			drum_set(unsigned int n);

/************************************************************************/
/*  title:  ＰＳＸレジスタ・キュー書き込み								*/
/************************************************************************/
void set_vp_vol(int i) {
   int core, voice;
	if (i < 24) {
		BP_sceSdSetParam( SD_CORE_0|(i<<1)|SD_VP_VOLL , spu_tr_wk[i].vol_l );
		BP_sceSdSetParam( SD_CORE_0|(i<<1)|SD_VP_VOLR , spu_tr_wk[i].vol_r );
      core = 0;
      voice = i;
	} else {
		BP_sceSdSetParam( SD_CORE_1|((i-24)<<1)|SD_VP_VOLL , spu_tr_wk[i].vol_l );
		BP_sceSdSetParam( SD_CORE_1|((i-24)<<1)|SD_VP_VOLR , spu_tr_wk[i].vol_r );
      core = 1;
      voice = i - 24;
	}
   BP_SoundSupport_SurroundSoundSetVoicePanVol( core, voice, spu_tr_wk[i].bp_pan3d, spu_tr_wk[i].bp_vol3d );
   BP_SoundSupport_SurroundSoundSetVoiceType( core, voice, spu_tr_wk[i].bp_surround_type );
   BP_SoundSupport_ApplyVoiceVolume( core, voice );
}
void off_vp_vol(int i) {
   int core, voice;
	if (i < 24) {
		BP_sceSdSetParam( SD_CORE_0|(i<<1)|SD_VP_VOLL , 0 );
		BP_sceSdSetParam( SD_CORE_0|(i<<1)|SD_VP_VOLR , 0 );
      core = 0;
      voice = i;
	} else {
		BP_sceSdSetParam( SD_CORE_1|((i-24)<<1)|SD_VP_VOLL , 0 );
		BP_sceSdSetParam( SD_CORE_1|((i-24)<<1)|SD_VP_VOLR , 0 );
      core = 1;
      voice = i - 24;
	}
   BP_SoundSupport_SurroundSoundSetVoicePanVol( core, voice, 0.f, 0.f );
   BP_SoundSupport_ApplyVoiceVolume( core, voice );
}

int sng_pause_save = 0;
void spuwr(void) {
	int				i;
/*--------------------- Key Off -----------------------------------------*/
	if (keyoffs[0]) {
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_KOFF , keyoffs[0] );
		keyoffs[0] = 0;
	}
	if (keyoffs[1]) {
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_KOFF , keyoffs[1] );
		keyoffs[1] = 0;
	}
/*--------------------- Reverb Off --------------------------------------*/
	if (rev_off_bit[0]) {
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_VMIXEL , rev_bit_data[0] );
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_VMIXER , rev_bit_data[0] );
		rev_off_bit[0] = 0;
	}
	if (rev_off_bit[1]) {
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_VMIXEL , rev_bit_data[1] );
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_VMIXER , rev_bit_data[1] );
		rev_off_bit[1] = 0;
	}
/*-----------------------------------------------------------------------*/
	for (i = 0; i < INTERNAL_TRACK_NUM; i++) {
/*--------------------- Volume ------------------------------------------*/
		if ((sng_pause_fg) || (int_pause_fg)) {	// if Pause On
			sng_pause_save = 1;
			if (i < SNG_TRACK_NUM) {
/* BGM */		off_vp_vol(i);
			} else {
				if (i < (SNG_TRACK_NUM+SE_TRACK_NUM) ) {

/* SE */			if ( ((se_playing[i-SNG_TRACK_NUM].code&0xFFF) >= 0xF0) && ((se_playing[i-SNG_TRACK_NUM].code&0xFFF) <= 0xFF) ) {
						if (spu_tr_wk[i].vol_fg) {
							set_vp_vol(i);
							spu_tr_wk[i].vol_fg = 0;
						}
					} else {
						off_vp_vol(i);
					}

				} else {
/* その他 */		off_vp_vol(i);
				}
			}
		} else {	//Pause Off
			if ((spu_tr_wk[i].vol_fg) || (sng_pause_save)) {
				set_vp_vol(i);
				spu_tr_wk[i].vol_fg = 0;
				sng_pause_save = 0;
			}
		}
/*--------------------- Frequency ---------------------------------------*/
		if (spu_tr_wk[i].pitch_fg) {
			if (i < 24) {
				BP_sceSdSetParam( SD_CORE_0|(i<<1)|SD_VP_PITCH , spu_tr_wk[i].pitch );
			} else {
				BP_sceSdSetParam( SD_CORE_1|((i-24)<<1)|SD_VP_PITCH , spu_tr_wk[i].pitch );
			}
			spu_tr_wk[i].pitch_fg = 0;
		}
/*--------------------- Wave Address ------------------------------------*/
		if (spu_tr_wk[i].addr_fg) {
			if (i < 24) {
				BP_sceSdSetAddr ( SD_CORE_0|(i<<1)|SD_VA_SSA , spu_tr_wk[i].addr+spu_wave_start_ptr );
			} else {
				BP_sceSdSetAddr ( SD_CORE_1|((i-24)<<1)|SD_VA_SSA , spu_tr_wk[i].addr+spu_wave_start_ptr );
///*DEBUG*/	PRINTF(("Ch%x:WaveAddr=%x\n", i, (unsigned int)(spu_tr_wk[i].addr+spu_wave_start_ptr) ));
			}
			spu_tr_wk[i].addr_fg = 0;
		}
/*--------------------- Envelope ----------------------------------------*/
		if (spu_tr_wk[i].env1_fg || spu_tr_wk[i].env2_fg || spu_tr_wk[i].env3_fg) {
			if (i < 24) {
				BP_sceSdSetParam( SD_CORE_0|(i<<1)|SD_VP_ADSR1 , 
				  SD_ADSR1(
				  	spu_tr_wk[i].a_mode,
				  	spu_tr_wk[i].ar,
				  	spu_tr_wk[i].dr,
				  	spu_tr_wk[i].sl
				  )
				);
				BP_sceSdSetParam( SD_CORE_0|(i<<1)|SD_VP_ADSR2 , 
				  SD_ADSR2(
				  	(spu_tr_wk[i].s_mode),
				  	spu_tr_wk[i].sr,
				  	spu_tr_wk[i].r_mode,
				  	spu_tr_wk[i].rr
				  )
				);
			} else {
				BP_sceSdSetParam( SD_CORE_1|((i-24)<<1)|SD_VP_ADSR1 , 
				  SD_ADSR1(
				  	spu_tr_wk[i].a_mode,
				  	spu_tr_wk[i].ar,
				  	spu_tr_wk[i].dr,
				  	spu_tr_wk[i].sl
				  )
				);
				BP_sceSdSetParam( SD_CORE_1|((i-24)<<1)|SD_VP_ADSR2 , 
				  SD_ADSR2(
				  	(spu_tr_wk[i].s_mode),
				  	spu_tr_wk[i].sr,
				  	spu_tr_wk[i].r_mode,
				  	spu_tr_wk[i].rr
				  )
				);
			}
			spu_tr_wk[i].env1_fg = spu_tr_wk[i].env2_fg = spu_tr_wk[i].env3_fg = 0;
		}
/*-----------------------------------------------------------------------*/
	}
/*--------------------- Reverb On ---------------------------------------*/
	if (rev_on_bit[0]) {
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_VMIXEL , rev_bit_data[0] );
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_VMIXER , rev_bit_data[0] );
		rev_on_bit[0] = 0;
	}
	if (rev_on_bit[1]) {
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_VMIXEL , rev_bit_data[1] );
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_VMIXER , rev_bit_data[1] );
		rev_on_bit[1] = 0;
	}
/*--------------------- Key On ------------------------------------------*/
	if (keyons[0]) {
		BP_sceSdSetSwitch( SD_CORE_0|SD_S_KON , keyons[0] );
//PRINTF(("<KON0:%x>", (unsigned int)keyons[0]));
		keyons[0] = 0;
	}
	if (keyons[1]) {
		BP_sceSdSetSwitch( SD_CORE_1|SD_S_KON , keyons[1] );
//PRINTF(("<KON1:%x>", (unsigned int)keyons[1]));
		keyons[1] = 0;
	}
}
/************************************************************************/
/*  title:  サウンドＯＦＦ												*/
/*	in:     無し														*/
/*	out:    無し														*/
/*	function: 全トラックのキーＯＦＦ									*/
/************************************************************************/

void sound_off()
{
	int			i;
//	int		 key_no;

	for (i=0; i<INTERNAL_TRACK_NUM; i++ ) {
		spu_tr_wk[i].rr = ENV_CUT_OFF;
		spu_tr_wk[i].env3_fg = 1;
/* ポーズなので、終了フラグを立てない 1999/08/24
		key_no = spu_ch_tbl[i];
		song_end |= key_no;
*/
	}
	keyoffs[0] = 0x00FFFFFF;
	keyoffs[1] = 0x003FFFFF;
	spuwr();
}


void sng_off()
{
	int			i;

	for (i=0; i<SNG_TRACK_NUM; i++ ) {
		spu_tr_wk[i].rr = ENV_CUT_OFF;
		spu_tr_wk[i].env3_fg = 1;
	}
	song_end[0] |= 0x00FFFFFF;
	song_end[1] |= 0x000000FF;
	keyoffs[0] |= 0x00FFFFFF;
	keyoffs[1] |= 0x000000FF;
/*	spuwr();*/
}

void se_off(int i)
{
	spu_tr_wk[SNG_TRACK_NUM+i].rr =0; /*ENV_CUT_OFF*/
	spu_tr_wk[SNG_TRACK_NUM+i].env3_fg = 1;
	song_end[1] |= (1<<((SNG_TRACK_NUM-24)+i));
	keyoffs[1] |= (1<<((SNG_TRACK_NUM-24)+i));
	sound_w[SNG_TRACK_NUM+i].mpointer = 0;
/*	spuwr();*/
}
/* 拡張SEの停止 */
void se_off_exp()
{
	int	i;
	for(i = SNG_TRACK_NUM; i < INTERNAL_TRACK_NUM; i++){
		if ((sound_w[i].mpointer >= se_data) && (sound_w[i].mpointer < se_data+SE_DATA_SIZE)) {
			se_off(i-SNG_TRACK_NUM);
		}
	}
/*DEBUG*/PRINTF(("CUT EXPANDED SE\n"));
}


/* 全SEの停止 */
void se_off_all()
{
	int	i;
	for(i = SNG_TRACK_NUM; i < INTERNAL_TRACK_NUM; i++){
		if (sound_w[i].mpointer) {
			se_off(i-SNG_TRACK_NUM);
		}
	}
/*DEBUG*/PRINTF(("CUT ALL SE\n"));
}

void sng_pause()
{
//2001/08/09 K.Muraoka
//	BP_sceSdSetParam( SD_CORE_0|SD_P_MVOLL , 0 ) ;	//Main Volume=off
//	BP_sceSdSetParam( SD_CORE_0|SD_P_MVOLR , 0 ) ;
//	BP_sceSdSetParam( SD_CORE_1|SD_P_MVOLL , 0 ) ;
//	BP_sceSdSetParam( SD_CORE_1|SD_P_MVOLR , 0 ) ;
}

void sng_pause_off()
{
//2001/08/09 K.Muraoka
//	BP_sceSdSetParam( SD_CORE_0|SD_P_MVOLL , 0x3FFF ) ;	//Main Volume=on
//	BP_sceSdSetParam( SD_CORE_0|SD_P_MVOLR , 0x3FFF ) ;
//	BP_sceSdSetParam( SD_CORE_1|SD_P_MVOLL , 0x3FFF ) ;
//	BP_sceSdSetParam( SD_CORE_1|SD_P_MVOLR , 0x3FFF ) ;
}

/************************************************************************/
/*  title:  キーＯＮ													*/
/*	in:     無し														*/
/*	out:    無し														*/
/*	function: 各トラックのキーＯＮ										*/
/************************************************************************/

void keyon()
{
	keyons[0] |= keyd[0];				  /* キーＯＮビットのセット */
	keyons[1] |= keyd[1];
}

/************************************************************************/
/*  title:  キーＯＦＦ													*/
/*	in:     無し														*/
/*	out:    無し														*/
/*	function: 各トラックのキーＯＦＦ									*/
/************************************************************************/

void keyoff()
{
	keyoffs[0] |= keyd[0];			/* キーＯＦＦビットのセット */
	keyoffs[1] |= keyd[1];
}

/************************************************************************/
/*  title:  音色セット													*/
/*	in:     音色番号													*/
/*	out:    無し														*/
/************************************************************************/

void tone_set(unsigned int n)
{
	unsigned int	reg;
	struct WAVE_W	*p;
//PRINTF(("no=%x\n", n));

/* 2000/02/14 */
//	if (n < 0x100) p = &voice_tbl[n];
//	else p = &drum_tbl[n-0x100];
	if (n < 0x200) p = &voice_tbl[n];
	else p = &drum_tbl[n-0x200];

//PRINTF(("addr=%x\n", p->addr));
	reg = (mtrack << 4) + 4;	/* 音色セット・レジスタ・アドレス */
	spu_tr_wk[mtrack].addr = BP_LE_SwapUInt( p->addr_le );
	spu_tr_wk[mtrack].addr_fg = 1;
	sptr->macro = p->sample_note; /* チューニング （ノート）*/
	sptr->micro = p->sample_tune; /* チューニング （チューン）*/

/*Attack & Decay*/
   if (p->a_mode) spu_tr_wk[mtrack].a_mode = SD_ADSR_A_EXP;
	else spu_tr_wk[mtrack].a_mode = SD_ADSR_A_LINEAR;
    spu_tr_wk[mtrack].ar = 127 - (p->ar & 0x7F);
    spu_tr_wk[mtrack].dr = 15 - (p->dr & 0xF);
	spu_tr_wk[mtrack].env1_fg = 1;

/*Sustain*/
	switch (p->s_mode) {
	case 0: spu_tr_wk[mtrack].s_mode = SD_ADSR_S_LINEAR_DEC;break;
	case 1: spu_tr_wk[mtrack].s_mode = SD_ADSR_S_EXP_DEC;break;
	case 2: spu_tr_wk[mtrack].s_mode = SD_ADSR_S_LINEAR_INC;break;
	default: spu_tr_wk[mtrack].s_mode = SD_ADSR_S_EXP_INC;break;
	}
	spu_tr_wk[mtrack].sr = 127 - (p->sr & 0x7F);
	spu_tr_wk[mtrack].sl = (p->sl & 0xF);
	spu_tr_wk[mtrack].env2_fg = 1;

/*Release*/
	if (p->r_mode) spu_tr_wk[mtrack].r_mode = SD_ADSR_R_EXP;
	else spu_tr_wk[mtrack].r_mode = SD_ADSR_R_LINEAR;
	spu_tr_wk[mtrack].rr = sptr->rrd = 31- (p->rr & 0x1F);
	spu_tr_wk[mtrack].env3_fg = 1;

	if(sptr->panmod==0)pan_set2(p->pan);
	sptr->dec_vol = p->dec_vol;	/* マイナス音量セット */
}

/************************************************************************/
/*  title:  オート・パン・データ・セット				*/
/*	in:     パン・データ						*/
/*	out:    無し							*/
/*	function: 音色セット時にパン・データの初期値をロードする。	*/
/*  etc:    主にドラムのオート・パンで使用する。			*/
/************************************************************************/

void pan_set2(unsigned char x)
{
	if (sptr->panoff == 0){		/* オート・パンがＯＦＦでなければ・・・ */
	     /* sptr->panf = x; */	/* パン・データのセット */
	     /*	sptr->pand = (unsigned int)(x << 8); */
		sptr->panf = x*2;  	/* *2 (H.Inoue) */
		sptr->pand = (unsigned int)(x*2 << 8);/* *2 (H.Inoue) */
	}
}

/************************************************************************/
/*  title:  パン移動／音量セット										*/
/*	in:     音量データ													*/
/*	out:    無し														*/
/*	function: パン・データを左右の音量に振り分け、出力する。			*/
/************************************************************************/

	unsigned long pant[41] = {
#if 1
0x0000, 0x0141, 0x0352, 0x06F5, 0x0A7A, 0x0DE3, 0x0F5D, 0x12F7, 0x1617, 0x1919,
0x1A66, 0x1D40, 0x1FFC, 0x229C, 0x2404, 0x2677, 0x28CE, 0x2B07, 0x2D24, 0x2E44,
0x2FFF, 0x31A3, 0x329D, 0x344B, 0x35DB, 0x374F, 0x3811, 0x3958, 0x3A82, 0x3B90,
0x3C81, 0x3CE3, 0x3DAA, 0x3E55, 0x3EF1, 0x3F25, 0x3F87, 0x3FCB, 0x3FF3, 0x3FFF, 0x3FFF
#else
		    0,  2,  4,  7, 10, 13, 16, 20, 24, 28,
		   32, 36, 40, 45, 50, 55, 60, 65, 70, 75,
		   80, 84, 88, 92, 96,100,104,107,110,112,
		  114,116,118,120,122,123,124,125,126,127,127
#endif
	 };

	unsigned long se_pant[0x41] = {
#if 1
//pan 50 - 0 - 50
//0x0000, 0x1E5E, 0x1EE2, 0x1FCD, 0x20B2, 0x2197, 0x21FB, 0x22F5, 0x23D3, 0x24AC, 0x250B, 0x25E2, 0x26B6, 0x2786, 0x2852, 0x28C4,
//0x298D, 0x2A54, 0x2B16, 0x2B81, 0x2C3F, 0x2CFB, 0x2DB4, 0x2E1A, 0x2ECE, 0x2F80, 0x302E, 0x307C, 0x3125, 0x31CD, 0x3271, 0x32CB,
//0x335B, 0x33E6, 0x343C, 0x34D6, 0x356C, 0x3601, 0x3641, 0x36CF, 0x375E, 0x37E6, 0x3831, 0x38B8, 0x3939, 0x39B8, 0x39FF, 0x3A7A,
//0x3AF1, 0x3B67, 0x3BA8, 0x3C16, 0x3C86, 0x3CF2, 0x3D58, 0x3D85, 0x3DEA, 0x3E4B, 0x3EB3, 0x3EDB, 0x3F36, 0x3F8D, 0x3FE0, 0x3FFF, 0x3FFF
//pan 100 - 0 - 100
0x0000, 0x00C5, 0x024D, 0x04CF, 0x0688, 0x08F3, 0x0A61, 0x0CB7, 0x0E51, 0x0FAC, 0x11E3, 0x1369, 0x1589, 0x16FF, 0x18D5, 0x1A3C,
0x1B9C, 0x1D87, 0x1EA8, 0x207E, 0x21BF, 0x22CD, 0x2485, 0x25B0, 0x2751, 0x2845, 0x29D1, 0x2ADD, 0x2BE4, 0x2D2C, 0x2E23, 0x2F7A,
0x2FFF, 0x3082, 0x31C0, 0x3297, 0x33A2, 0x346A, 0x352B, 0x3634, 0x36CD, 0x37C1, 0x3863, 0x3940, 0x39BF, 0x3A4C, 0x3B0A, 0x3B76,
0x3C1F, 0x3C8E, 0x3CF6, 0x3D72, 0x3DCB, 0x3E3F, 0x3E88, 0x3EE5, 0x3F17, 0x3F4A, 0x3F89, 0x3FA8, 0x3FD2, 0x3FE6, 0x3FF9, 0x3FFF, 0x3FFF
#else
		  0,   2,   4,      /*0*/
		  6,   8,  10,      /*10*/
		 14,  18,  22,      /*24*/
		 28,  34,  40,      /*40*/
		 46,  52,  58,  64, /*60*/
		 70,  76,  82,      /*80*/
		 88,  94, 100,      /*100*/
		106, 112, 118,      /*120*/
		124, 130, 136,      /*140*/
		142, 148, 154, 160, /*160*/

		166, 172, 178,      /*172*/
		183, 188, 193,      /*196*/
		198, 203, 208,      /*210*/
		213, 217, 221,      /*222*/
		224, 227, 230, 233, /*232*/
		236, 238, 240,      /*240*/
		242, 244, 246,      /*246*/
		248, 249, 250,      /*250*/
		251, 252, 253,      /*253*/
		254, 254, 255, 255, /*255*/
		255
#endif
		};
void vol_set(unsigned long vol_data)
{
	unsigned int	pan;

	if(vol_data >= sptr->dec_vol) vol_data -= sptr->dec_vol;
	else vol_data = 0;			/* 音量最小限度チェック */
/*--------------------- 音量セット ----------------------------------*/
	if ((sptr->panmod == 2) && (mtrack<SNG_TRACK_NUM)) {
/*------------------------------------------------------------*/
/*--- フェーダー情報からパン位置を取得するＢＧＭ扱いのＳＥ ---*/
/*--- 2000/01/20                                           ---*/
/*------------------------------------------------------------*/
		pan = mix_fader[mtrack].now_pan;
		if (sound_mono_fg) pan = 0x20;
		++vol_data;	//最大音量を出す為に
		spu_tr_wk[mtrack].vol_r = (((vol_data * se_pant[pan]) / 0x7F)*sng_master_vol[mtrack])/0x10000;
		spu_tr_wk[mtrack].vol_l = (((vol_data * se_pant[0x40-pan]) /0x7F)*sng_master_vol[mtrack])/0x10000;
//		spu_tr_wk[mtrack].vol_r = (((vol_data * se_pant[pan]) >> 7)*sng_master_vol[mtrack])/0x10000;
//		spu_tr_wk[mtrack].vol_l = (((vol_data * se_pant[0x40-pan]) >> 7)*sng_master_vol[mtrack])/0x10000;
		spu_tr_wk[mtrack].vol_fg = 1;
      //BP: midi instrument notes should be "stereo spread" when in surround mode.
      spu_tr_wk[mtrack].bp_surround_type = BP_SURROUND_VOICE_TYPE_STEREO;
	} else {
/*----------------------------------*/
/*--- ＢＧＭ或いはシステム系ＳＥ ---*/
/*----------------------------------*/
		if ((mtrack<SNG_TRACK_NUM) || (se_playing[mtrack-SNG_TRACK_NUM].kind==0) ) {
//			if(vol_data >= sptr->dec_vol) vol_data -= sptr->dec_vol;
//			else vol_data = 0;			/* 音量最小限度チェック */
			pan = sptr->pand >> 8;		/* パン * 音量 */
			if (pan > 40) pan = 40;
/*1998/04/10 MONO対応*/ if (sound_mono_fg) pan = 20;
			++vol_data;	//最大音量を出す為に
         //BP: midi notes should be "stereo spread" and UI sfx should be just in FL/C/FR speakers.
			if (mtrack<SNG_TRACK_NUM) {
				spu_tr_wk[mtrack].vol_r = (((vol_data * pant[pan])/0x7F)*sng_master_vol[mtrack])/0x10000;
				spu_tr_wk[mtrack].vol_l = (((vol_data * pant[40-pan])/0x7F)*sng_master_vol[mtrack])/0x10000;
//				spu_tr_wk[mtrack].vol_r = (((vol_data * pant[pan])>>7)*sng_master_vol[mtrack])/0x10000;
//				spu_tr_wk[mtrack].vol_l = (((vol_data * pant[40-pan])>>7)*sng_master_vol[mtrack])/0x10000;
            spu_tr_wk[mtrack].bp_surround_type = BP_SURROUND_VOICE_TYPE_STEREO;
			} else {
				spu_tr_wk[mtrack].vol_r = (vol_data * pant[pan])/0x7F;
				spu_tr_wk[mtrack].vol_l = (vol_data * pant[40-pan])/0x7F;
//				spu_tr_wk[mtrack].vol_r = (vol_data * pant[pan])>>7;
//				spu_tr_wk[mtrack].vol_l = (vol_data * pant[40-pan])>>7;

#if defined(BP_VITA)
            spu_tr_wk[mtrack].bp_surround_type = BP_SURROUND_VOICE_TYPE_STEREO;
#else
            spu_tr_wk[mtrack].bp_surround_type = BP_SURROUND_VOICE_TYPE_FLFCFR;
#endif
			}
			spu_tr_wk[mtrack].vol_fg = 1;
		} else {
/*----------------------------------------------*/
/*--- サウンドコードでパン位置を取得するＳＥ ---*/
/*----------------------------------------------*/
//			if(vol_data >= sptr->dec_vol) vol_data -= sptr->dec_vol;
//			else vol_data = 0;			/* 音量最小限度チェック */
			vol_data = ((vol_data+1) * (se_vol[mtrack-SNG_TRACK_NUM])) >> 6;
			pan = se_pan[mtrack-SNG_TRACK_NUM];
/*1998/04/10 MONO対応*/ if (sound_mono_fg) pan = 0x20;
			++vol_data;	//最大音量を出す為に
			spu_tr_wk[mtrack].vol_r = (vol_data * se_pant[pan]) >> 7;
			spu_tr_wk[mtrack].vol_l = (vol_data * se_pant[0x40-pan]) >> 7;
			spu_tr_wk[mtrack].vol_fg = 1;
         //BP added
         spu_tr_wk[mtrack].bp_pan3d = se_bp_angle[mtrack-SNG_TRACK_NUM];
         spu_tr_wk[mtrack].bp_vol3d = (float)vol_data / 127;
         spu_tr_wk[mtrack].bp_surround_type = se_bp_surround_type[mtrack-SNG_TRACK_NUM];
		}
	}
}

/************************************************************************/
/*                           周波数テーブル                             */
/************************************************************************/

#if 1 //BP_PS2

//BP - stole this updated table from the xmodule (xbox/PC) drop of sd_ioset.c
//The array is indexed in the below by 'note' which is cast to an unsigned char
//in the range 0...127.
//The original array only has 108 entries so certain underflow cases produced
//garbage results.  It's not clear why they had all those commented out entries,
//but it was clearly wrong.
//The new table properly has 128+1 entries which covers the entire range
//0...(127+1) that can be indexed by the function.
unsigned int freq_tbl[]	= {
//c00   c01   d00   d01   e00   f00   f01   g00   g01   a00   a01   b00  1.0594631*/

// 66,   70,   75,   79,   84,   89,   94,  100,  106,  112,  119,  126, /* c00 */
//133,  141,  150,  159,  168,  178,  189,  200,  212,  225,  238,  252, /* c10 */
  267,  283,  300,  318,  337,  357,  378,  401,  425,  450,  477,  505, /* c00 */
  535,  567,  601,  637,  675,  715,  757,  802,  850,  901,  954, 1011, /* c10 */
 1071, 1135, 1202, 1274, 1350, 1430, 1515, 1605, 1701, 1802, 1909, 2022, /* c20 */
 2143, 2270, 2405, 2548, 2700, 2860, 3030, 3211, 3402, 3604, 3818, 4045, /* c30 */
 4286, 4541, 4811, 5097, 5400, 5721, 6061, 6422, 6804, 7208, 7637, 8091, /* c40 */
 8572, 9082, 9622,10194,10800,11442,12122,12844,13608,14416,15276,16182, /* c50 */
/* for オーバーフロー用データ、ｃ０の２ｏｃｔ下のデータ追加 */
17144,18164,19244,                           11,   12,   13,   14,   15, /*c00-3oct*/
   16,   18,   19,   20,   21,   22,   24,   25,   27,   28,   30,   32, /*c00-2oct*/
   33,   35,   38,   40,   42,   45,   47,   50,   53,   56,   60,   63, /*c00-3oct*/
   66,   70,   75,   79,   84,   89,   94,  100,  106,  112,  119,  126, /*c00-2oct*/
  133,  141,  150,  159,  168,  178,  189,  200,  212,  225,  238,  252, /*c00-1oct*/
  267	//Dummy
};

#else

unsigned int freq_tbl[]	= {
//c00   c01   d00   d01   e00   f00   f01   g00   g01   a00   a01   b00  1.0594631*/

// 66,   70,   75,   79,   84,   89,   94,  100,  106,  112,  119,  126, /* c00 */
//133,  141,  150,  159,  168,  178,  189,  200,  212,  225,  238,  252, /* c10 */
  267,  283,  300,  318,  337,  357,  378,  401,  425,  450,  477,  505, /* c00 */
  535,  567,  601,  637,  675,  715,  757,  802,  850,  901,  954, 1011, /* c10 */
 1071, 1135, 1202, 1274, 1350, 1430, 1515, 1605, 1701, 1802, 1909, 2022, /* c20 */
 2143, 2270, 2405, 2548, 2700, 2860, 3030, 3211, 3402, 3604, 3818, 4045, /* c30 */
 4286, 4541, 4811, 5097, 5400, 5721, 6061, 6422, 6804, 7208, 7637, 8091, /* c40 */
 8572, 9082, 9622,10194,10800,11442,12122,12844,13608,14416,15276,16182, /* c50 */

/* for オーバーフロー用データ、ｃ０の２ｏｃｔ下のデータ追加 */

// 10,   11,   12,   13,   14,   14,   15,   16,                         /*c00-3oct*/
// 16,   18,   19,   20,   21,   22,   24,   25,   27,   28,   30,   32, /*c00-2oct*/
   33,   35,   38,   40,   42,   45,   47,   50,   53,   56,   60,   63, /*c00-3oct*/
   66,   70,   75,   79,   84,   89,   94,  100,  106,  112,  119,  126, /*c00-2oct*/
  133,  141,  150,  159,  168,  178,  189,  200,  212,  225,  238,  252, /*c00-1oct*/

};

#endif

/************************************************************************/
/*  title:  周波数セット												*/
/*	in:     音程（ノート、チューン）データ								*/
/*	out:    無し														*/
/*	function: 音程を周波数に変換し、出力する。							*/
/************************************************************************/
void freq_set(unsigned int note_tune)
{
	unsigned char		pl, ph, note, tune;
//	unsigned int		reg;
	unsigned int		freq;
	unsigned int		*ptr;

/*------------- チューニング・データを加算し、補正する -----------------*/
//	note_tune += (signed int)sptr->micro;						/* tune */

	if ( mtrack < SNG_TRACK_NUM ) {
		note_tune += mix_fader[mtrack].now_frq;
///*DEBUG*/if ( mtrack == 0 ) PRINTF(("note_tune=%x(%x)\n", note_tune, mix_fader[mtrack].now_frq));
	}
	note_tune += (unsigned int)sptr->micro;
	tune = (unsigned char)note_tune;
	note = (unsigned char)(note_tune >> 8) + sptr->macro;	/* note */
#if 0
   //BP_TODO: do some sound tests and see which sounds might sound different
   //after changing the table over.
   if( note & 0x80 )
      BP_BREAK;
#endif
	note &= 0x7F;

/*--------------------- ノートを周波数に変換する -----------------------*/
	ptr = freq_tbl;

	freq = *(ptr+(note+1)) - *(ptr+note);	/* 半音の周波数（差） */
	if ((freq & 0x8000) != 0) freq = 201;	/* B5 --> C6 の処理 */
	pl = (unsigned char)freq;	/* 周波数(1word)を(2byte)に分割して計算 */
	ph = (unsigned char)(freq >> 8);
	freq = ((pl * tune) >> 8) + (ph * tune);   /* チューン * 半音周波数 */

	freq += *(ptr+note);		/* ノートに対応する周波数を<freq>に加算 */

	spu_tr_wk[mtrack].pitch = (unsigned short)freq;
	spu_tr_wk[mtrack].pitch_fg = 1;
}
/************************************************************************/
/*  title:  Ｄ／Ａ ドラム・セット										*/
/*	in:     ドラム番号													*/
/*	out:    無し														*/
/*	function: ドラム番号を音色番号に変換し、音色セットする。			*/
/*  etc:																*/
/* 	  DA DRUM TABLE = {end adrs(2),start adrs(3),vol(1),freq(1),pan(1)}	*/
/************************************************************************/

void drum_set(unsigned int n)
{
//	n = n - SEQ_DRC + wavs;				  /* ドラム番号を音色番号に変換 */

/* 2000/02/14 */
//	n = n - SEQ_DRC + 0x100;				  /* ドラム番号を音色番号に変換 */
	n = n - SEQ_DRC + 0x200;				  /* ドラム番号を音色番号に変換 */
	tone_set(n);										  /* 音色セット */
}

