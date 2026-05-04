/*===========================================================================*/
/*=                                                                         =*/
/*=                     <<<<<<<<< sd_sub1.c >>>>>>>>>                       =*/
/*=         ＰＳＸ版サウンドドライバー・シーケンス制御ルーチン              =*/
/*=                                                                         =*/
/*===========================================================================*/
#include "sd_debug.h"
#include "sd_incl.h"
#include "sd_ext.h"

//#define BP_SPEW_MIDI

#define	ENDOFMEASURE  0xFE  /* 小節ＥＮＤ     */
#define	ENDOFBLOCK    0xFE  /* ブロックＥＮＤ */
#define	ENDOFSONG     0xFF  /* ソングＥＮＤ   */
/*---------------------------------------------------------------------------*/
/*                                  外部定義                                 */
/*---------------------------------------------------------------------------*/
extern void	     drum_set(unsigned int n);
extern void	     freq_set(unsigned int note_tune);
extern void	     vol_set(unsigned char vol_data);
extern void	     keyoff(void);
extern void	     keyon(void);
/*extern void      opmwr(unsigned char reg, unsigned char n);*/
extern unsigned char get_gate(unsigned char gate);
extern unsigned char get_vel(unsigned char vol);
/*extern void	     graph_level_set(int mtrack, int y);*/
/*extern void	     midi_note_set(void);*/

extern void		por_compute(void); /* H.Inoue */
extern void 	dspwr(unsigned long reg, unsigned long n);

/* 制御コード・セット */

extern void rest_set(void),   sno_set(void),  pan_set(void),  pan_move(void);
extern void vib_set(void),    rdm_set(void),  lp1_start(void),lp1_end(void);
extern void lp2_start(void),  lp2_end(void),  tempo_set(void),tempo_move(void);
extern void trans_set(void),  tre_set(void),  vol_chg(void),  vol_move(void);
extern void vib_change(void), por_set(void),  sws_set(void),  detune_set(void);
extern void swp_set(void),    tie_set(void),  echo_set1(void),echo_set2(void);
extern void kakko_start(void),kakko_end(void),env_set(void),  pm_set(void);
extern void jump_set(),       no_cmd(void);
extern void ads_set(void),    srs_set(void),  rrs_set(void);
extern void eon_set(void),    eof_set(void);
extern void l3s_set(void),l3e_set(void),svl_set(void),svp_set(void),use_set(void);
extern void block_end(void);
extern void ofs_set(void);
// 2000/01/17
extern void fxs_set(void), fxe_set(void), xon_set(void);
// 2000/01/18
extern void vol_i_move(void);
// 2000/01/26
extern void at1_set(void), at2_set(void), at3_set(void), at4_set(void), at5_set(void);
// 2000/01/14
extern void mno_set(void);
// 2000/07/07
extern void at6_set(void), at7_set(void), at8_set(void);
// 2000/07/17
extern void flg_set(void);

/*---------------------------------------------------------------------------*/
/*                       制御コード・プログラム・テーブル                    */
/*---------------------------------------------------------------------------*/
void (*cntl_tbl[0x80])(void) = {
/* 0x80 */
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
/* 0x90 */
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
/* 0xA0 */
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
/* 0xB0 */
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
/* 0xC0 */
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,no_cmd,
	no_cmd,no_cmd,no_cmd,no_cmd,no_cmd, at6_set, at7_set, at8_set,
/* 0xD0 */
	tempo_set,tempo_move,sno_set,svl_set,svp_set,vol_chg,vol_move,ads_set,
	srs_set,rrs_set,fxs_set,fxe_set,ofs_set,pan_set,pan_move,trans_set,
/* 0xE0 */
	detune_set,vib_set,vib_change,rdm_set,swp_set,sws_set,por_set,lp1_start,
/* 0xE8 */
	lp1_end,lp2_start,lp2_end,l3s_set,l3e_set,kakko_start,kakko_end,xon_set,
/* 0xF0 */
    vol_i_move,env_set,rest_set,tie_set,echo_set1,echo_set2,eon_set,eof_set,
/* 0xF8 */
	at1_set,at2_set,at3_set,at4_set,at5_set,mno_set,flg_set,block_end
};

const char * bp_cntl_tbl_debug_name[0x80] = {
   /* 0x80 */
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   /* 0x90 */
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   /* 0xA0 */
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   /* 0xB0 */
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   /* 0xC0 */
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd","no_cmd",
   "no_cmd","no_cmd","no_cmd","no_cmd","no_cmd"," at6_set"," at7_set"," at8_set",
   /* 0xD0 */
   "tempo_set","tempo_move","sno_set","svl_set","svp_set","vol_chg","vol_move","ads_set",
   "srs_set","rrs_set","fxs_set","fxe_set","ofs_set","pan_set","pan_move","trans_set",
   /* 0xE0 */
   "detune_set","vib_set","vib_change","rdm_set","swp_set","sws_set","por_set","lp1_start",
   /* 0xE8 */
   "lp1_end","lp2_start","lp2_end","l3s_set","l3e_set","kakko_start","kakko_end","xon_set",
   /* 0xF0 */
   "vol_i_move","env_set","rest_set","tie_set","echo_set1","echo_set2","eon_set","eof_set",
   /* 0xF8 */
   "at1_set","at2_set","at3_set","at4_set","at5_set","mno_set","flg_set","block_end"
};

/* RANDOM LFO TABLE */
char rdm_tbl[0x81]={
 0x9F,0x3C,0xB2,0x52,0xAF,0x45,0xC7,0x89,0x10,0x7F,0xE0,0x9D,0xDC,0x1F,0x61,0x16,
 0x39,0xC9,0x9C,0xEB,0x57,0x08,0x66,0xF8,0x5A,0x24,0xBF,0x0E,0x3E,0x15,0x4B,0xDB,
 0xAB,0xF5,0x31,0x0C,0x43,0x02,0x55,0xDE,0x41,0xDA,0xBD,0xAE,0x19,0xB0,0x48,0x57,
 0xBA,0xA3,0x36,0x0B,0xF9,0xDF,0x17,0xA8,0x04,0x0C,0xE0,0x91,0x18,0x5D,0xDD,0xD3,
 0x28,0x8A,0xF2,0x11,0x59,0x6F,0x06,0x0A,0x34,0x2A,0x79,0xAC,0x5E,0xA7,0x83,0xC6,
 0x39,0xC1,0xB4,0x3A,0x3F,0xFE,0x4F,0xEF,0x1F,0x00,0x30,0x99,0x4C,0x28,0x83,0xED,
 0x8A,0x2F,0x2C,0x66,0x3F,0xD6,0x6C,0xB7,0x49,0x22,0xBC,0x65,0xFA,0xCF,0x02,0xB1,
 0x46,0xF0,0x9A,0xD7,0xE2,0x0F,0x11,0xC5,0x74,0xF6,0x7A,0x2C,0x8F,0xFB,0x19,0x6A,
 0xE5
};
/*
char rdm_tbl[0x200]={
 0x9F,0x3C,0xB2,0x52,0xAF,0x45,0xC7,0x89,0x10,0x7F,0xE0,0x9D,0xDC,0x1F,0x61,0x16,
 0x39,0xC9,0x9C,0xEB,0x57,0x08,0x66,0xF8,0x5A,0x24,0xBF,0x0E,0x3E,0x15,0x4B,0xDB,
 0xAB,0xF5,0x31,0x0C,0x43,0x02,0x55,0xDE,0x41,0xDA,0xBD,0xAE,0x19,0xB0,0x48,0x57,
 0xBA,0xA3,0x36,0x0B,0xF9,0xDF,0x17,0xA8,0x04,0x0C,0xE0,0x91,0x18,0x5D,0xDD,0xD3,
 0x28,0x8A,0xF2,0x11,0x59,0x6F,0x06,0x0A,0x34,0x2A,0x79,0xAC,0x5E,0xA7,0x83,0xC6,
 0x39,0xC1,0xB4,0x3A,0x3F,0xFE,0x4F,0xEF,0x1F,0x00,0x30,0x99,0x4C,0x28,0x83,0xED,
 0x8A,0x2F,0x2C,0x66,0x3F,0xD6,0x6C,0xB7,0x49,0x22,0xBC,0x65,0xFA,0xCF,0x02,0xB1,
 0x46,0xF0,0x9A,0xD7,0xE2,0x0F,0x11,0xC5,0x74,0xF6,0x7A,0x2C,0x8F,0xFB,0x19,0x6A,
 0xE5,0x75,0x51,0x4A,0xFF,0xB8,0x7F,0x62,0xDB,0x4F,0x14,0xE8,0xC2,0xD6,0x62,0xB9,
 0xC2,0xAD,0xA6,0xD5,0x29,0xF7,0x14,0x2E,0x6C,0x18,0x8E,0xA4,0xC1,0xA0,0x7E,0x32,
 0xA9,0xBE,0xE6,0x15,0xB6,0x7D,0xF8,0xA0,0xE6,0x12,0x7A,0x67,0x47,0xD2,0x03,0x58,
 0xFD,0xD1,0x9E,0x31,0x9B,0x50,0x54,0xA5,0xEC,0x72,0x70,0xD5,0xB6,0x9B,0x82,0x7C,
 0xA1,0xC5,0x72,0xB9,0xA8,0x33,0x8B,0xEB,0x59,0x9E,0x29,0x87,0x8E,0x36,0xAD,0x86,
 0xEA,0xE4,0xD4,0x37,0x92,0xAE,0x45,0xF0,0x6E,0x24,0xD0,0xF3,0xCB,0x95,0x71,0xE8,
 0xC6,0x38,0xED,0x71,0x44,0x22,0x2B,0x60,0xC0,0x26,0xF9,0x53,0x5A,0xC4,0x2D,0x74,
 0xC8,0x50,0x8D,0xF7,0xAC,0x01,0x79,0xBD,0x52,0x10,0x5E,0x67,0xB0,0x01,0xA6,0xFF,

 0x5F,0xCE,0x54,0x64,0x5C,0xE7,0xBA,0x32,0x96,0xFC,0x00,0xE9,0xCC,0x77,0xDC,0x60,
 0xB5,0x92,0x4D,0x84,0x1C,0x68,0xEE,0x4E,0xAF,0xAB,0x07,0x87,0x9F,0x93,0xBF,0x88,
 0xF1,0xCD,0x80,0xD7,0x4A,0xE5,0x20,0x4E,0x63,0x44,0xDA,0x78,0x8F,0x07,0xF3,0x40,
 0x98,0x6B,0x37,0xA2,0xF4,0x21,0x1E,0x96,0x13,0x7E,0x09,0xEA,0x3D,0x27,0x86,0x4D,
 0xBE,0x8B,0xCD,0xD0,0xFB,0xA3,0x1B,0x97,0x2A,0xBC,0x1A,0x8C,0x73,0x4C,0x46,0x6F,
 0xDE,0x26,0x8C,0xB8,0x33,0xAA,0x0E,0xBB,0x21,0xD8,0x89,0x3B,0x97,0x1D,0xBB,0x23,
 0x48,0x90,0x69,0xFA,0xC9,0x76,0xCE,0xE7,0x1B,0x7B,0x1A,0x2B,0x78,0xC0,0x03,0x08,
 0x6B,0xCF,0xCC,0xB3,0x6D,0xA9,0xB2,0x3A,0x16,0xA1,0x56,0x95,0x0B,0x17,0xF1,0x12,
 0x99,0x13,0xD8,0x81,0x56,0x38,0xF5,0x3E,0xD1,0xB1,0xE3,0xE1,0xB3,0x2E,0x84,0xEF,
 0x5B,0x04,0x64,0x5D,0xB5,0x0A,0x5B,0x1D,0x94,0x06,0xD9,0xE4,0x0D,0xFD,0x6D,0xA2,
 0x4B,0xF6,0x65,0xA5,0xCB,0xAA,0x5F,0xE3,0x70,0x1E,0xF2,0x93,0x82,0x63,0x27,0xEE,
 0xE2,0x1C,0x98,0x23,0xCA,0x0D,0xEC,0xD9,0x05,0xF4,0x3B,0x53,0x9D,0x73,0x3C,0xC3,
 0xD4,0xFE,0xE1,0xDD,0x61,0x69,0x90,0x2D,0x2F,0x8D,0x51,0xC4,0xA4,0x7C,0x43,0x88,
 0x34,0x5C,0x85,0xB4,0x7D,0x25,0x30,0x58,0x85,0x68,0x35,0xC7,0x47,0x20,0x05,0x35,
 0x09,0x25,0x80,0x55,0xCA,0x6A,0x91,0xFC,0xC8,0xD3,0x0F,0xA7,0xC3,0x9A,0x6E,0x94,
 0x9C,0x42,0xDF,0x77,0x42,0x40,0xD2,0x76,0x3D,0x41,0x81,0x75,0xB7,0x49,0xE9,0x7B
};
*/
/*---------------------------------------------------------------------------*/
/*                                  関数定義                                 */
/*---------------------------------------------------------------------------*/
   int 		 sound_sub(void);
   void	 	 note_set(void);
   int		 tx_read(void);
   void 	 note_compute(void);
   void 	 swpadset(int xfreq);
   void 	 vol_compute(void);
   void		 keych(void);
   void		 bendch(void);
   void		 tempo_ch(void);
   int		 random(void);
   void 	 note_cntl(void);
   void		 volxset(unsigned char depth);
/* int 		 vib_generate(signed char cnt); */
   int		 vib_compute(void);
/* unsigned char tre_generate(void); */
   void		 pan_generate(void);
	void	fader_automation1(void), fader_automation2(void);/* フェーダーオートメーション */

/*---------------------------------------------------------------------------*/
/*                                マクロ定義                                 */
/*---------------------------------------------------------------------------*/

#define GET_VEL	  *(mptr)
#define GET_GATE  *(mptr+1)
#define GET_STEP  *(mptr+2)
#define GET_NOTE  *(mptr+3)

/*---------------------------------------------------------------------------*/
/*  	title:  音割り込み処理（各ポート）			  	                     */
/*		in:     無し						                                 */
/*		out:    0 = トラック・データ継続中			                         */
/*		        1 = トラックＥＮＤ                                           */
/*---------------------------------------------------------------------------*/
int sound_sub(void)
{
//	int				tempo_move;

	sptr->tmpd += sptr->tmp;	/* テンポ・カウンタ更新 */

//1999/12/12 for Mixer Fader
//	if (mtrack < SNG_TRACK_NUM) {
//		if (sng_tempo_move) {	/* 回避モード・テンポ移動中 */
//			tempo_move = sng_tempo_move >> 5;
//			if (sptr->tmp > tempo_move) sptr->tmpd -= tempo_move;
//		}
//	}
#if 1	/* 2000/01/17 */
	if( (sptr->tmpd >= 0x100) && (sptr->fx_time_base) ) {
		--sptr->fx_ngc;
		if (sptr->fx_ngc == 0) {
			sptr->fx_ngc = sptr->fx_time_base;
			if (sptr->fx_mode == 0xFF) {	// サウンドコードによるFXスタート
				if (fx_sound_code) {
					sptr->fx_play_fg = 1;
					mptr = sptr->fx_addr;
					sptr->ngc = 1;
					fx_sound_code++;
				}
			} else {
				if (sound_w[(sptr->fx_mode)&0x1F].fx_on) { //トラック指定によるFXスタート
					sptr->fx_play_fg = 1;
					mptr = sptr->fx_addr;
					sptr->ngc = 1;
					sound_w[(sptr->fx_mode)&0x1F].fx_on = 0;
				}
			}
		}
	}
	if ( (sptr->fx_time_base) && (!sptr->fx_play_fg) ) {
/* FX待機中 */
		sptr->tmpd &= 0xFF;
		goto fx_end;
	}

	key_fg=0;
	if(sptr->tmpd >= 0x100){	/* ステップ・カウンタ更新 ？ */
		sptr->tmpd &= 0xFF;
		--sptr->ngc;			/*ステップ・カウンタ更新 */
		if(sptr->ngc == 0){		/* 音符ＥＮＤ ？ */
			if(tx_read() != 0){	/* テキスト読み込み */
				keyoff();		/* トラックＥＮＤならキーオフ */
				return(1);
			}
		}else{
			keych();			/* キーオフ・チェック */
		}
		tempo_ch();				/* テンポ移動チェック */
		bendch();				/* スィープ・チェック */
		vol_compute();			/* 音量データ計算 */
/* 2000/01/26 */
		fader_automation1();	/* フェーダーオートメーション */
	}
	else{
		note_cntl();			/* set pan move, sweep, vib */
	}
/* 2000/01/26 */
	fader_automation2();
/* 2000/02/14 */
//	if (key_fg != 0) keyon();
	if (key_fg != 0) {
		if (sptr->snos < 0x100) keyon();
		else mem_str_w[mtrack-SE_START_TRACK].status = 2;
	}
fx_end:
#else
	key_fg=0;
	if(sptr->tmpd >= 0x100){	/* ステップ・カウンタ更新 ？ */
		sptr->tmpd &= 0xFF;
		--sptr->ngc;			/*ステップ・カウンタ更新 */
		if(sptr->ngc == 0){		/* 音符ＥＮＤ ？ */
			if(tx_read() != 0){	/* テキスト読み込み */
				keyoff();		/* トラックＥＮＤならキーオフ */
				return(1);
			}
		}else{
			keych();			/* キーオフ・チェック */
		}
		tempo_ch();				/* テンポ移動チェック */
		bendch();				/* スィープ・チェック */
		vol_compute();			/* 音量データ計算 */
	} else {
		note_cntl();			/* set pan move, sweep, vib */
	}
	if (key_fg != 0) keyon();
#endif
	return(0);
}
/*---------------------------------------------------------------------------*/
/*  	title:  音割り込み処理（テキスト読み込み）			                 */
/*		in:     無し						                                 */
/*		out:    0 = トラック・データ継続中			                         */
/*		        1 = トラックＥＮＤ、ブランク・データ                         */
/*---------------------------------------------------------------------------*/
int tx_read()
{
    int				read_fg;
	int				loop_count = 0;

    read_fg=1;		  	/* タイ、休符、ノート・データがくるまで、読み込む */

    while(read_fg){

/*無限ループ対処 '93/03/02 takamine-------------------------------------*/

		loop_count++;
		if( loop_count == 256 ){  /*２５６個制御コードが続いたら強制終了*/
			return(1);
		}

/*----------------------------------------------------------------------*/

		mdata1 = GET_NOTE;	/* ノート・データのロード */

		if(mdata1 == 0) return(1);

		mdata2 = GET_STEP;	/* ステップ・データのロード */
		mdata3 = GET_GATE;	/* ゲート・データのロード */
		mdata4 = GET_VEL;	/* ベロシティ・データのロード */

/*PRINTF(("DATA=%x, %x, %x, %x\n",mdata1, mdata2, mdata3, mdata4));*/
#ifdef BP_SPEW_MIDI
      PRINTF(("%p: DATA=%2x, %2x, %2x, %2x   %s\n",mptr, mdata1, mdata2, mdata3, mdata4, (mdata1>=0x80) ? bp_cntl_tbl_debug_name[mdata1-0x80] : "note_set"));
#endif
		 mptr += SNG_NOTE_SIZE;
/*--------------------- 制御コード --------------------------------------*/
		 if(mdata1 >= 0x80) {
			(*cntl_tbl[mdata1-0x80])();   /* テーブル・ジャンプ */
			if((mdata1 == REST_SET)||(mdata1 == TIE_SET)||
			   (mdata1 == 0xFF)){      /* Block Data End (H.Inoue) */
				read_fg=0;             /* 休符、タイなら読み込み終了 */
			}
/* Add 1996/11/19 K.Muraoka */
			if (mdata1 == ENDOFSONG) return(1);
/* 2000/01/17 */
			if ((mdata1 == FXS_SET) || (mdata1 == FXE_SET)) read_fg=0;
		 } else {

/* 1996/10/23 K.Muraoka) Comment Out*/
//				if(mdata1 >= 0x60) keyoff();  /* for MIDI */

			if((sptr->ngg <= 99)&&(mdata4 != 0)){
				key_fg=1;             /* gate<100,vel>0 のデータならキーオンする */
			}
/********************** ノート・データ・セット **************************/
			sptr->rest_fg=0;   /* 休符フラグのリセット */
			note_set();	   /* ノート・データ・セット */
			read_fg=0;	   /* テキスト読み込み終了 */
		 }
	}
	return(0);
}
/*---------------------------------------------------------------------------*/
/*  	title:  ノート・データ・セット					                     */
/*		in:    mdata1 = ノート・データ				                         */
/*		       mdata2 = ステップ・データ			                         */
/*		       mdata3 = ゲート・データ 				                         */
/*		       mdata4 = ベロシティ		 		                             */
/*		out:   無し						                                     */
/*---------------------------------------------------------------------------*/
void note_set()
{
	unsigned int  x;

	sptr->ngs = mdata2;		/* ステップ・タイム・データの保存 */
	sptr->ngg = mdata3;		/* ゲート・タイム・データの保存 */
	sptr->vol = mdata4 & 0x7F;	/* 出力音量データの保存 */

	note_compute();			/* ノート・データの計算 */

	sptr->ngc = sptr->ngs;		/* ステップ・タイムをカウンターにロード */
	x = (sptr->ngg*sptr->ngc)/100;  /* ゲートを百分率からステップ数に変換 */
	if(x == 0){			/* ゲートの最小データは 1 */
		x = 1;
	}
	sptr->ngo = (unsigned char)x;	/* ステップ・カウンタのセット */
}
/*------------------------------------------*/
/*     ＧＡＩＮリリース ＡＤＳＲ初期化      */
/*------------------------------------------*/
void adsr_reset()
{
	spu_tr_wk[mtrack].rr = sptr->rrd;
	spu_tr_wk[mtrack].env3_fg = 1;
}
/*--------------------------------------------------------------------------*/
/*                               音符データ処理								*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*  	title:  ノート・データ計算											*/
/*		in:     mdata1 = ノート・データ										*/
/*		out:    無し														*/
/*--------------------------------------------------------------------------*/
void note_compute()
{
	int	     x;
	unsigned int swp_ex;
/*----------------------*/
/*   ノートFREQ計算     */
/*----------------------*/
/* 1996/10/23 K.Muraoka */
	if(mdata1 >= SEQ_DRC){		/* ドラム音コード */
		drum_set(mdata1);	/* ドラム音セット */
/* 1996/10/23 K.Muraoka */
		x = 36;		        /* ノートＮｏ．セット */
	}else{
/*--------------------- ノート、デチューン・データを求める ------------------*/
		x = (int)mdata1;	/* ノートＮｏ．セット */
	}
	x += (int)sptr->ptps;		         /* トランスポーズ値加算 */
	x = (x << 8) + sptr->tund;	         /* デチューン値加算 */
	x = x + sptr->lp1_freq + sptr->lp2_freq; /* ループ周波数加算 */
	while (x >= 0x6000) x -= 0x6000;	 /* ノートの最大値は 0x5FFF */
	swp_ex = sptr->swpd;			 /* 前のノート・データの保存 */
	sptr->swpd = ((unsigned int)x) << 8;		 /* ノート・データの保存 */
/*------------------------- 制御関連カウンタの初期化 ------------------------*/
	sptr->vibhc=sptr->vibcc=0;               /* ビブラート・ワーク (H.Inoue) */
	sptr->vib_tmp_cnt=sptr->vib_tbl_cnt=0;   /* ビブラート・ワーク (H.Inoue) */
	sptr->vibd = 0;
	sptr->trec = sptr->trehc = 0;		 /* トレモロ・ワーク */
	adsr_reset();
/*----------------------*/
/*   スイープ初期化     */
/*----------------------*/
	sptr->swpc = sptr->swsc;
	if(sptr->swpc != 0){
/*------------------------------ スィープ処理 -------------------------------*/
		sptr->swphc = sptr->swshc;	  /* ホールド・カウンタ・セット */
		if(sptr->swsk == 0){
/*------------------- 次の音にむかってスィープ・スタート(sws) ---------------*/
			x = sptr->swpd >> 8;		  /* ノート・データ目的値 */
			if(sptr->swss > 0x7F00){
			    sptr->swpd += (0x10000 - (sptr->swss & 0xFFFF)) << 8;
			}else
			    sptr->swpd -= sptr->swss << 8; /* ノート・データ - 変化量 */
		     /* x = sptr->swss + sptr->swpd; */
			swpadset(x);		  /* １割込当たりの変化量を求める(H.Inoue) */
		}
		else{
/*------------------------- ポルタメント・スタート(por) ---------------------*/
		     /*	x = sptr->swpd; (H.Inoue) *//* ノート・データ目的値 */
			sptr->swpm = sptr->swpd >> 8;  /* ノート・データ目的値 */
			sptr->swpd = swp_ex;	  /* 前のノート・データをポップする */
		}
	     /*	swpadset(x); (H.Inoue) */	  /* １割込当たりの変化量を求める */
	}
	x = sptr->swpd >> 8;				  /*  <x> 出力ノートＮｏ．*/
	freq_set((unsigned int)x);	/* ノート、デチューンを周波数に変換する */
}
/*--------------------------------------------------------------------------*/
/*  	title:  １割り込み当りのノート（周波数）変化量を求める				*/
/*		in:     xfreq = ノート・データ（目的値）							*/
/*		out:    無し														*/
/*		function:															*/
/*			１．ノート・データの範囲チェック								*/
/*			２．ノート（目的ｰ現在）／変化時間								*/
/*		etc:    <note_compute>, <bendch> から周波数設定時に呼ばれる			*/
/*--------------------------------------------------------------------------*/
void swpadset(int xfreq)
{
	register unsigned int flame_dat;

	if(sptr->swpc != 0){		    /* スィープ、ポルタメント中 ？ */
		flame_dat=(sptr->swpc<<8);  /* 割り込み数計算 (H.Inoue) */
		flame_dat/=sptr->tmp;	    /* 割り込み数=(Move_step*100H)/Tempo (H.Inoue) */

		if(xfreq<0){		    /* ノート・データの範囲は 0 ~ 0x5FFF */
			xfreq=0;
		}
		else{
			if(xfreq >= 0x6000) xfreq=0x5FFF;
		}
		sptr->swpm = xfreq;	    /* スィープ（目的値）セット */
		xfreq = (xfreq << 8) - sptr->swpd; /* 目的 ｰ 現在 */
		if(xfreq < 0){
		     /*	xfreq = -xfreq/sptr->swpc; */	/* １ステップ当りの変化量 */
		     xfreq = -xfreq/flame_dat; 	/* １割り込み当りの変化量 (H.Inoue) */
			 sptr->swpad = -xfreq;
		}
		else{
		     /*	sptr->swpad = xfreq/sptr->swpc; *//* １ステップ当りの変化量 */
		     sptr->swpad = xfreq/flame_dat;  /* １割り込み当りの変化量 (H.Inoue) */
		}
	}
}
/*--------------------------------------------------------------------------*/
/*  	title:  音量データ計算												*/
/*		in:     無し														*/
/*		out:    無し														*/
/*		function:															*/
/*			１．音量移動 on/off チェック＆セット							*/
/*			２．トレモロ on/off チェック＆セット							*/
/*		etc:    <sound_sub> から音量設定時に呼ばれる。						*/
/*--------------------------------------------------------------------------*/
void vol_compute()
{
	unsigned int depth;

/* voly */
	if(sptr->pvoc != 0){		  /* ボリューム移動中？ */
		if((--sptr->pvoc) == 0){
			sptr->pvod = (int)sptr->pvom << 8;  /*音量移動完了*/
		}else{
			sptr->pvod += sptr->pvoad;	    /*音量移動中*/
		}
	}
	if(sptr->vol != 0){
/* trey */
		if(sptr->tred != 0){	 /* トレモロ中？ */
			if(sptr->trehs == sptr->trehc){
				sptr->trec += sptr->trecad; /*トレモロ中*/
/* treset */
				if(sptr->trec < 0){
					depth = sptr->tred * -sptr->trec; /*トレモロ変化量*/
				}else{
					if(sptr->trec == 0) depth = 1;
					else depth = sptr->tred * sptr->trec;
				}
			}else{		 /*トレモロ・ホールド中*/
				++sptr->trehc;
				depth = 0;	 /*トレモロ変化無し */
			}
		}else{
			depth = 0;	 	 /*トレモロＯＦＦ*/
		}
		volxset((unsigned char)(depth >> 8));	/* 音量出力 */
	}
	pan_generate();
}
/*--------------------------------------------------------------------------*/
/*  	title:  パン・データ制御											*/
/*		in:     無し														*/
/*		out:    無し														*/
/*		function:															*/
/*			１．パン移動 on/off チェック＆セット							*/
/*		etc:    <vol_compute> から音量設定時に呼ばれる。					*/
/*--------------------------------------------------------------------------*/
void pan_generate(void)
{
/* pany */
	if(sptr->panc != 0){	 /* パン移動 on ？ */
/* addws */
		if((--sptr->panc) == 0){
			sptr->pand = sptr->panm;   /*音量移動完了*/
		}else{
			sptr->pand += sptr->panad; /*音量移動中*/
		}
		sptr->panf = (unsigned char)(sptr->pand >> 8);
	}
}
/*--------------------------------------------------------------------------*/
/*  	title:  キーオフ・チェック											*/
/*		in:     無し														*/
/*		out:    無し														*/
/*		function:															*/
/*			１．キーオフ on/off チェック＆セット							*/
/*			２．スィープ、ビブラート、ＬＦＯセット							*/
/*		etc:    <sound_sub> から呼ばれる。									*/
/*--------------------------------------------------------------------------*/
void key_cut_off(void)
{
	if (sptr->rrd > ENV_CUT_OFF) {
		spu_tr_wk[mtrack].rr = ENV_CUT_OFF;
		spu_tr_wk[mtrack].env3_fg = 1;
	}
}

void keych()
{
	int           vib_data,rdm_data,set_fg;

/*---------------------------------------------------------------------------*/
/*                       エンベロープ１ リリース処理                         */
/*---------------------------------------------------------------------------*/

/*--------------------- ノイズ対策 --------------------------------------*/
	if((sptr->ngg < 100) && (sptr->ngc == 1)){
		key_cut_off();			/* ノイズ対策 */
	}
/*--------------------- Key Off -----------------------------------------*/
	if(sptr->ngo != 0){
		--sptr->ngo;
		if(sptr->ngo == 0){
			keyoff();
		}
	}
/*---------------------------------------------------------------------------*/
/*                       エンベロープ１ スイープ処理                         */
/*---------------------------------------------------------------------------*/
/* swpy */
	set_fg=0;			 /* 周波数変化フラグ＆データのクリア */
	if(sptr->swpc != 0){	 /* スィープ on ？ */
		if(sptr->swphc != 0){
			--sptr->swphc;	 /* スィープ・ホールド中 */
		} else {
/* addw */
			if(sptr->swsk == 0){  /* スィープ処理 */
				if((--sptr->swpc) == 0){
					sptr->swpd = sptr->swpm << 8;   /* スィープ移動完了 */
				}else{
					sptr->swpd += sptr->swpad; /* スィープ移動中 */
				}
			} else {		      /* ポルタメント処理 */
				por_compute();
			}
			set_fg=1;
		}
    }
/*---------------------------------------------------------------------------*/
/*                       エンベロープ１ ビブラート処理                       */
/*---------------------------------------------------------------------------*/
    vib_data = 0;
    if(sptr->vibdm != 0){	  /* ビブラート on */
		if(sptr->vibhc != sptr->vibhs){
			++sptr->vibhc;	  /* ビブラート・ホールド中 */
		} else {
			if(sptr->vibcc == sptr->vibcs){
				sptr->vibd = sptr->vibdm; /*ビブラート・チェンジ完了*/
			} else {
				if(sptr->vibcc != 0){
					sptr->vibd += sptr->vibad; /*チェンジ中*/
				} else {
					sptr->vibd = sptr->vibad;  /*チェンジ開始*/
				}
				++sptr->vibcc;
			}
/* vib18 */
	     /*	sptr->vibc += sptr->vibcad;          (H.Inoue) *//*ビブラート・カウンタ更新*/
	     /* sptr->vibcadw2 = 0;                  (H.Inoue) */
	     /*	vib_data = vib_generate(sptr->vibc); (H.Inoue) *//*ビブラート*/
	     /*	set_fg = 1;                          (H.Inoue) */

			sptr->vib_tmp_cnt += sptr->vibcad;
			if(sptr->vib_tmp_cnt>=0x100){
				sptr->vib_tmp_cnt &= 0x00ff;
				vib_data = vib_compute();
				set_fg = 1;
			}
		}
    }
/*---------------------------------------------------------------------------*/
/*                   　   エンベロープ１ ランダム処理  　                    */
/*---------------------------------------------------------------------------*/
    if((rdm_data=random()) != 0){  /* ランダムＬＦＯ on ？ */
        vib_data += rdm_data;      /* ランダム・データ加算 */
        set_fg = 1;
    }
/*2001/04/25 FREQ Fader*/
	if (mix_fader[mtrack].now_frq != mix_fader[mtrack].last_frq) {
		mix_fader[mtrack].last_frq = mix_fader[mtrack].now_frq;
		set_fg = 1;
	}

    if(set_fg != 0){
        freq_set((sptr->swpd >> 8) + vib_data); /*周波数セット*/
    }
}
/*--------------------------------------------------------------------------*/
/*  	title:  ポルタメント変化量計算(Hideto Inoue)						*/
/*		in:     　　無し													*/
/*		out:    　　無し													*/
/*		function: 　無し　													*/
/*--------------------------------------------------------------------------*/
void por_compute()
{
	int          por_freq;
	unsigned int pfreq_h,pfreq_l;

	por_freq= sptr->swpm - (sptr->swpd >> 8);
	if(por_freq<0){
	    por_freq=-por_freq;
	    pfreq_l=por_freq&0x00ff;
	    pfreq_h=por_freq>>8;
	    pfreq_l=(pfreq_l*sptr->swsc)>>8;
	    pfreq_h=(pfreq_h*sptr->swsc);
	    por_freq=pfreq_h+pfreq_l;
	    if(por_freq==0)por_freq=1; /* ０補正 */
	    por_freq=-por_freq;
	}
	else{
	    if(por_freq==0){
	   	sptr->swpc=0;	/* ポルタメント移動終了 */
	    }
	    else{
		pfreq_l=por_freq&0x00ff;
		pfreq_h=por_freq>>8;
	    	pfreq_l=(pfreq_l*sptr->swsc)>>8;
	    	pfreq_h=(pfreq_h*sptr->swsc);
		por_freq=pfreq_h+pfreq_l;
	   	if(por_freq==0)por_freq=1; /* ０補正 */
	    }
	}
	sptr->swpd+=por_freq << 8;
}
/*--------------------------------------------------------------------------*/
/*  	title:  ビブラート波形の生成										*/
/*		in:     <cnt> ビブラート・カウンタ									*/
/*		out:    ビブラート波形 * 深さ										*/
/*		function:															*/
/*			１．三角波の生成												*/
/*			２．ビブラート深さの補正										*/
/*		etc:    <keych> <note_cntl> から呼ばれる。							*/
/*---------------------------------------------------------------------------*/
	unsigned char VIBX_TBL[0x20]={	   /* DEPTH_MAX=半音 */
				0x00,0x20,0x38,0x50,0x68,0x80,0x90,0xA0,
				0xB0,0xC0,0xD0,0xE0,0xE8,0xF0,0xF0,0xF8,

				0xFF,0xF8,0xF4,0xF0,0xE8,0xE0,0xD0,0xC0,
				0xB0,0xA0,0x90,0x80,0x68,0x50,0x38,0x20
		      };
/*---------------------------------------------------------------------------*/
int vib_compute()
{
	int vib_data,tbl_data;

	sptr->vib_tbl_cnt+=sptr->vib_tc_ofst;
	sptr->vib_tbl_cnt&=0x3f;
	tbl_data=VIBX_TBL[(sptr->vib_tbl_cnt&0x1f)]&0x00ff;
	if(sptr->vibd<0x8000){			 /* MAX[7F]=半音 */
		vib_data=(sptr->vibd>>7)&0x00fe; /* Depth正数値x2 */
		vib_data=(unsigned int)(vib_data*tbl_data)>>8; /* (Vib_Depth*Table_Data)/100H */
	}
	else{					 /* 半音*DEPTH_DATA */
	     /* vib_data=((sptr->vibd>>8)&0x007f)+4; *//* Depth正数値-128+4 */
	     /*	vib_data=(unsigned int)(vib_data*tbl_data)>>2; *//* (Vib_Depth*Table_Data)/4 */
		vib_data=((sptr->vibd>>8)&0x007f)+2;        /* Depth正数値-128+2 */
		vib_data=(unsigned int)(vib_data*tbl_data)>>1; /* (Vib_Depth*Table_Data)/2 */
	}
	if(sptr->vib_tbl_cnt>=0x20){
		vib_data = -vib_data;
	}
	return(vib_data);
}
/*---------------------------------------------------------------------------*/
int vib_generate(signed char cnt)
{
	signed char vib_char;
	int	    vib_data;

	if(cnt < 0){			        /* マイナス側のカーブ */
		vib_char = (-cnt) << 1;
		if(vib_char < 0){
			vib_char = -vib_char;	/* マイナス側上向きのカーブ */
		}
		*((unsigned char*)&vib_char) >>= 2;
		vib_data = (unsigned char)(sptr->vibd >> 8) * vib_char;
													 /* デプス * カーブ */
		vib_data = -vib_data;
	}else{					/* プラス側のカーブ */
		vib_char = cnt << 1;
		if(vib_char < 0){
			vib_char = -vib_char;	/* プラス側下向きのカーブ */
		}
		*((unsigned char*)&vib_char) >>= 2;
		vib_data = (unsigned char)(sptr->vibd >> 8) * vib_char;
													 /* デプス * カーブ */
	}
	if (sptr->vibdm < 0x8000) {
		vib_data = (signed int)vib_data >> 2;	  /* デプス補正 */
	}
	return(vib_data);
}
/*--------------------------------------------------------------------------*/
/*  	title:  スィープ・チェック											*/
/*		in:     無し														*/
/*		out:    無し														*/
/*		function:															*/
/*			１．次のテキストデータのチェック								*/
/*			２．スィープならば関連ワークをセット							*/
/*		etc:    <sound_sub> から呼ばれる。									*/
/*--------------------------------------------------------------------------*/
void bendch()
{
	int		bend_frq;

	if(sptr->swpc == 0){			/* スィープ中でなければ．．．*/
		mdata1 = GET_NOTE;
		if(mdata1 == SWP_SET){		/* 次のデータはＳＷＰか ？ */
			sptr->swphc = GET_STEP;	/* ホールド・カウンタ */
			sptr->swpc = GET_GATE;	/* 速さカウンタ */
			bend_frq = GET_VEL;	/* 目的ノートＮｏ． */
#ifdef BP_SPEW_MIDI
         PRINTF(("bendch %p: DATA=%2x, %2x, %2x, %2x\n",mptr, mdata1, sptr->swphc, sptr->swpc, bend_frq));
#endif
			mptr += SNG_NOTE_SIZE;

			bend_frq=(bend_frq + sptr->ptps)<<8; /* ノート目的値 */
			bend_frq+=sptr->tund;	 	     /* デチューン値加算 */
			swpadset(bend_frq);		     /* １割込当たりの変化量を求める */
		}
	}
}
/*--------------------------------------------------------------------------*/
/*  	title:  音符制御（ステップ・アップ時を除く毎割り込み）処理			*/
/*		in:     無し														*/
/*		out:    無し														*/
/*		function:															*/
/*			１．毎割込の音量チェック＆セット								*/
/*			２．毎割込のスィープ、ビブラート、ＬＦＯセット  				*/
/*		etc:    <sound_sub> から呼ばれる。									*/
/*---------------------------------------------------------------------------*/
void note_cntl()
{
	unsigned char x;
	int	      rdm_data, vib_data, fset_fg, frq_data;
	unsigned int  depth;
/*---------------------------------------------------------------------------*/
/*                       エンベロープ２ 音量セット処理                       */
/*---------------------------------------------------------------------------*/
/* trry */
	if(sptr->vol != 0){				/* 音量が０でなければ．．． */
		if(sptr->tred != 0){			/* トレモロが on ならば．．． */
			if(sptr->trehs == sptr->trehc){
							/* ホールド時間が終わっていれば．．． */
/* tresetx */						/* トレモロ波形生成 */
				x = (unsigned char)sptr->tmpd;
				sptr->trec += (char)((unsigned int)(sptr->trecad * x) >> 8);
				if(sptr->trec < 0){
					depth = sptr->tred * -sptr->trec; /* 上向きのカーブ */
				}else{
					if(sptr->trec == 0){
						depth = 1;  /* カーブ最大ポイント */
					} else {
						depth = sptr->tred * sptr->trec; /* 下向きカーブ */
					}
				}
				volxset((unsigned char)(depth >> 8));  /* 音量セット */
			}
		}
	}
/*---------------------------------------------------------------------------*/
/*                      　 エンベロープ２ スイープ処理                       */
/*---------------------------------------------------------------------------*/
/* sppy */
	fset_fg = 0;				  /* 周波数セット・フラグ初期化 */
	frq_data = sptr->swpd >> 8;
	if(sptr->swpc != 0){			  /* スィープ on ならば．．． */
		if(sptr->swphc == 0){		  /* ホールド終了ならば．．． */
		     /* swp_data = sptr->swpad; *//* スィープＯＮ*/
/* hokan */
			fset_fg = 1;		  /* 周波数セット・フラグ on */
			if(sptr->swsk == 0){  /* スィープ処理 */
				sptr->swpd+=sptr->swpad;  /* スィープ移動中 (H.Inoue) */
			}
			else{		      /* ポルタメント処理 */
				por_compute();
			}
			frq_data=sptr->swpd >> 8;	  /* (H.inoue) */

		     /*	if(swp_data < 0){                  */
		     /*		i_data = -swp_data & 0xFF; *//* 周波数 下降 */
		     /*		swp_data = -swp_data >> 8; */
		     /*		i_data *= sptr->tmpd;      */
		     /*		swp_data *= sptr->tmpd;    */
		     /*		frq_data -= (swp_data+(i_data>>8)); */
		     /*	}else{                             */
		     /*		i_data = swp_data & 0xFF;  *//* 周波数 上昇 */
		     /*		swp_data = swp_data >> 8;  */
		     /*		i_data *= sptr->tmpd;      */
		     /*		swp_data *= sptr->tmpd;    */
		     /*		frq_data += (swp_data+(i_data>>8)); */
		     /*	}                                  */
		}
	}
/*---------------------------------------------------------------------------*/
/*                       エンベロープ２ ビブラート処理                       */
/*---------------------------------------------------------------------------*/
/* vbby */
	if((sptr->vibd != 0) && (sptr->vibhs == sptr->vibhc)){
						      /* ビブラート on でホールド終了ならば．．． */
	     /*	sptr->vibcadw2 += sptr->vibcadw;       (H.Inoue) *//* ビブラートカーブの現在点 */
	     /*	cnt = (sptr->vibcadw2) + (sptr->vibc); (H.Inoue) */
	     /*	vib_data = vib_generate(cnt);	       (H.Inoue) *//* ビブラートの生成 */
	     /*	frq_data += vib_data;                  (H.Inoue) */
	     /*	fset_fg = 1;			       (H.Inoue) *//* 周波数セット・フラグ on */

		sptr->vib_tmp_cnt += sptr->vibcad;
		if(sptr->vib_tmp_cnt>=0x100){
			sptr->vib_tmp_cnt &= 0x00ff;
			vib_data = vib_compute(); /* ビブラート計算 */
			frq_data += vib_data;
			fset_fg = 1;
		}
	}
/*---------------------------------------------------------------------------*/
/*                   　   エンベロープ２ ランダム処理 　                     */
/*---------------------------------------------------------------------------*/
	if((rdm_data=random()) != 0){;	/* ＬＦＯ 生成 */
		fset_fg = 1;           /* 周波数セット・フラグ on */
		frq_data += rdm_data;  /* 周波数データ + ＬＦＯデータ */
	}

/*2001/04/25 FREQ Fader*/
	if (mix_fader[mtrack].now_frq != mix_fader[mtrack].last_frq) {
		mix_fader[mtrack].last_frq = mix_fader[mtrack].now_frq;
		fset_fg = 1;
	}

	if(fset_fg != 0){
		freq_set(frq_data);    /*周波数セット*/
	}
}
/*---------------------------------------------------------------------------*/
/*  	title:  ＬＦＯ生成					                                 */
/*		in:     無し						                                 */
/*		out:    ＬＦＯデータ					                             */
/*		function:						                                     */
/*			    １．ＬＦＯ on/off チェック			                         */
/*			    ２．ＬＦＯデータのセット			                         */
/*		etc:    <keych> <note_cntl>  から呼ばれる。	Edited by H.Inoue        */
/*---------------------------------------------------------------------------*/
int random()
{
	unsigned int frq_dt;
	signed char	 c_data;

	frq_dt = 0;			           /* ＬＦＯデータ・クリア */
	if(sptr->rdms != 0){           /* ＬＦＯ on ならば．．． */
		sptr->rdmc += sptr->rdms;  /* ＬＦＯ ＯＮ */
		if(sptr->rdmc > 0x100){	   /* ＬＦＯテーブルからデータ・ロード */
			sptr->rdmc &= 0xFF;
			++sptr->rdmo;	       /* オフセット・ポインター更新 */
			sptr->rdmo&=0x7f;      /* TBL_SIZE=81H */
			c_data = rdm_tbl[sptr->rdmo];     /* Low */
			frq_dt = rdm_tbl[sptr->rdmo+1]<<8;/* High */
			frq_dt += (unsigned char)c_data;

			frq_dt = sptr->rdmd & frq_dt; /* ＬＦＯデータ・セット */
		}
	}
	return(frq_dt);
}
/*---------------------------------------------------------------------------*/
/*  	title:  テンポ移動制御				                                 */
/*		in:     無し						                                 */
/*		out:    無し						                                 */
/*		function:						                                     */
/*		        １．テンポ移動 on/off チェック			                     */
/*		        ２．テンポ移動データのセット			                     */
/*		etc:    <sound_sub>  から呼ばれる。			                         */
/*---------------------------------------------------------------------------*/
void tempo_ch()
{
	if(sptr->tmpc != 0){		  /* テンポ移動 on ならば．．． */
		if((--sptr->tmpc) == 0){
			sptr->tmpw = (int)sptr->tmpm << 8;  /* テンポ移動終了 */
		}else{
			sptr->tmpw += sptr->tmpad;	    /* テンポ移動セット */
		}
		sptr->tmp = (unsigned char)(sptr->tmpw >> 8);
	}
}
/*--------------------------------------------------------------------------*/
/*  	title:  音量セット													*/
/*		in:     トレモロ深さ												*/
/*		out:    無し														*/
/*		function:															*/
/*			１．出力音量算出												*/
/*			２．音量出力													*/
/*		etc:    <vol_compute> <note_cntl>  から呼ばれる。					*/
/*--------------------------------------------------------------------------*/
void volxset(unsigned char depth)
{
	int				vol_data, pvod_w;

	vol_data = sptr->vol;			      /* ベロシティ・データ */
	vol_data -= depth;			      /* トレモロ・データを減算 */
	vol_data += sptr->lp1_vol;		      /* ループ音量加算 */
	vol_data += sptr->lp2_vol;
	if ((signed int)vol_data < 0){		      /* 最低音量チェック */
		vol_data = 0;
	} else {
		if (vol_data > 0x7F) vol_data = 0x7F;
	}
	pvod_w = (sptr->pvod >> 8) & 0xFF;	      /* トラック音量データ */

	vol_set((unsigned char)(((pvod_w * vol_data)+0x80) >> 8)); /* 音量出力(0 - 0x7F) */
/*
	sptr->volx = (unsigned char)((pvod_w * vol_data) >> 7);
	vol_set(sptr->volx);
*/
}
/*-------------------------------------------------------------------*/
/* フェーダーオートメーション処理 1                                  */
/*-------------------------------------------------------------------*/
void	fader_automation1(void) {
	unsigned int auto_vol;
	signed int auto_tim;

	if ((sptr->auto_mode == 1) && (auto_phase_fg != sptr->auto_pos)) {
		sptr->auto_pos = auto_phase_fg;
		switch (auto_phase_fg) {
		case 0:
			break;
		case 1:
			auto_vol = ((unsigned int)(sptr->auto1_vol) << 8)+ sptr->auto1_vol; //max.=0xFF
			auto_tim = (unsigned int)(sptr->auto1_tim);
			break;
		case 2:
			auto_vol = ((unsigned int)(sptr->auto2_vol) << 8)+ sptr->auto2_vol;
			auto_tim = (unsigned int)(sptr->auto2_tim);
			break;
		case 3:
			auto_vol = ((unsigned int)(sptr->auto3_vol) << 8)+ sptr->auto3_vol;
			auto_tim = (unsigned int)(sptr->auto3_tim);
			break;
		case 4:
			auto_vol = ((unsigned int)(sptr->auto4_vol) << 8)+ sptr->auto4_vol;
			auto_tim = (unsigned int)(sptr->auto4_tim);
			break;
		case 5:
			auto_vol = ((unsigned int)(sptr->auto5_vol) << 8)+ sptr->auto5_vol;
			auto_tim = (unsigned int)(sptr->auto5_tim);
			break;
		case 6:
			auto_vol = ((unsigned int)(sptr->auto6_vol) << 8)+ sptr->auto6_vol;
			auto_tim = (unsigned int)(sptr->auto6_tim);
			break;
		case 7:
			auto_vol = ((unsigned int)(sptr->auto7_vol) << 8)+ sptr->auto7_vol;
			auto_tim = (unsigned int)(sptr->auto7_tim);
			break;
		case 8:
			auto_vol = ((unsigned int)(sptr->auto8_vol) << 8)+ sptr->auto8_vol;
			auto_tim = (unsigned int)(sptr->auto8_tim);
			break;
		default:
			auto_phase_fg = 0;
			break;
		}
		if (auto_phase_fg) {
			mix_fader[mtrack].dst_vol = auto_vol;	//max.=0xFFFF
			if (mix_fader[mtrack].dst_vol == mix_fader[mtrack].now_vol) { //現在値と同じフェーダー値を設定した時
				 mix_fader[mtrack].int_vol = 0;
			} else {	//現在値と異なるフェーダー値を設定した時
				if (auto_tim) {	//タイマーが0以外の時
//PRINTF(("dst=%x : now=%x : tim=%x\n", mix_fader[mtrack].dst_vol, mix_fader[0].now_vol, auto_tim));
					mix_fader[mtrack].int_vol=(signed int)(mix_fader[mtrack].dst_vol-mix_fader[mtrack].now_vol)/((auto_tim)*10);
					if (mix_fader[mtrack].int_vol == 0) mix_fader[mtrack].int_vol = 1;
//PRINTF(("Fader=%x\n", mix_fader[0].int_vol));
				} else {	//タイマーが0の時
					mix_fader[mtrack].now_vol = mix_fader[mtrack].dst_vol;
					mix_fader[mtrack].int_vol = 0;
				}
			}
		}
	}
}
/*-------------------------------------------------------------------*/
/* フェーダーオートメーション処理 2                                  */
/*-------------------------------------------------------------------*/
void	fader_automation2(void) {
	unsigned int auto_vol;
	signed int auto_tim;

//	if ( ((sptr->auto_pos != auto_env_pos) || (sptr->auto_pos != auto_env_pos2)) && ((sptr->auto_mode == 2) || (sptr->auto_mode == 3)) ) {
	if ( ((sptr->auto_pos != auto_env_pos) && (sptr->auto_mode == 2)) || ((sptr->auto_pos != auto_env_pos2) && (sptr->auto_mode == 3)) ) {
		switch (sptr->auto_mode) {
		case 2:
			sptr->auto_pos = auto_env_pos;
			break;
		case 3:
			sptr->auto_pos = auto_env_pos2;
			break;
		default:
			break;
		}
		if (sptr->auto_pos <= sptr->auto1_tim) {
			auto_vol = ((unsigned int)(sptr->auto1_vol) << 8)+ sptr->auto1_vol; //max.=0xFFFF
		} else if (sptr->auto_pos <= sptr->auto2_tim) {
			auto_vol = (signed int)(sptr->auto_pos-sptr->auto1_tim) * ((signed int)(sptr->auto2_vol-sptr->auto1_vol));
			auto_vol = (signed int)((auto_vol<<8)+auto_vol) / (signed int)(sptr->auto2_tim-sptr->auto1_tim);
			auto_vol += ((unsigned int)(sptr->auto1_vol) << 8)+ sptr->auto1_vol;
		} else if (sptr->auto_pos <= sptr->auto3_tim) {
			auto_vol = (signed int)(sptr->auto_pos-sptr->auto2_tim) * ((signed int)(sptr->auto3_vol-sptr->auto2_vol));
			auto_vol = (signed int)((auto_vol<<8)+auto_vol) / (signed int)(sptr->auto3_tim-sptr->auto2_tim);
			auto_vol += ((unsigned int)(sptr->auto2_vol) << 8)+ sptr->auto2_vol;
		} else if (sptr->auto_pos <= sptr->auto4_tim) {
			auto_vol = (signed int)(sptr->auto_pos-sptr->auto3_tim) * ((signed int)(sptr->auto4_vol-sptr->auto3_vol));
			auto_vol = (signed int)((auto_vol<<8)+auto_vol) / (signed int)(sptr->auto4_tim-sptr->auto3_tim);
			auto_vol += ((unsigned int)(sptr->auto3_vol) << 8)+ sptr->auto3_vol;
		}else if (sptr->auto_pos <= sptr->auto5_tim) {
			auto_vol = (signed int)(sptr->auto_pos-sptr->auto4_tim) * ((signed int)(sptr->auto5_vol-sptr->auto4_vol));
			auto_vol = (signed int)((auto_vol<<8)+auto_vol) / (signed int)(sptr->auto5_tim-sptr->auto4_tim);
			auto_vol += ((unsigned int)(sptr->auto4_vol) << 8)+ sptr->auto4_vol;
		}else if (sptr->auto_pos <= sptr->auto6_tim) {
			auto_vol = (signed int)(sptr->auto_pos-sptr->auto5_tim) * ((signed int)(sptr->auto6_vol-sptr->auto5_vol));
			auto_vol = (signed int)((auto_vol<<8)+auto_vol) / (signed int)(sptr->auto6_tim-sptr->auto5_tim);
			auto_vol += ((unsigned int)(sptr->auto5_vol) << 8)+ sptr->auto5_vol;
		}else if (sptr->auto_pos <= sptr->auto7_tim) {
			auto_vol = (signed int)(sptr->auto_pos-sptr->auto6_tim) * ((signed int)(sptr->auto7_vol-sptr->auto6_vol));
			auto_vol = (signed int)((auto_vol<<8)+auto_vol) / (signed int)(sptr->auto7_tim-sptr->auto6_tim);
			auto_vol += ((unsigned int)(sptr->auto6_vol) << 8)+ sptr->auto6_vol;
		}else if (sptr->auto_pos <= sptr->auto8_tim) {
			auto_vol = (signed int)(sptr->auto_pos-sptr->auto7_tim) * ((signed int)(sptr->auto8_vol-sptr->auto7_vol));
			auto_vol = (signed int)((auto_vol<<8)+auto_vol) / (signed int)(sptr->auto8_tim-sptr->auto7_tim);
			auto_vol += ((unsigned int)(sptr->auto7_vol) << 8)+ sptr->auto7_vol;
		} else {
			auto_vol = ((unsigned int)(sptr->auto8_vol) << 8)+ sptr->auto8_vol;
		}
		auto_tim = 1;	//50mSec.
//PRINTF(("%x:auto_vol=%x\n", mtrack, auto_vol));
		mix_fader[mtrack].dst_vol = auto_vol;	//max.=0xFFFF
		if (mix_fader[mtrack].dst_vol == mix_fader[mtrack].now_vol) { //現在値と同じフェーダー値を設定した時
			 mix_fader[mtrack].int_vol = 0;
		} else {	//現在値と異なるフェーダー値を設定した時
			if (auto_tim) {	//タイマーが0以外の時
//PRINTF(("dst=%x : now=%x : tim=%x\n", mix_fader[mtrack].dst_vol, mix_fader[0].now_vol, auto_tim));
				mix_fader[mtrack].int_vol=(signed int)(mix_fader[mtrack].dst_vol-mix_fader[mtrack].now_vol)/((auto_tim)*10);
				if (mix_fader[mtrack].int_vol == 0) mix_fader[mtrack].int_vol = 1;
//PRINTF(("Fader=%x\n", mix_fader[0].int_vol));
			} else {	//タイマーが0の時
				mix_fader[mtrack].now_vol = mix_fader[mtrack].dst_vol;
				mix_fader[mtrack].int_vol = 0;
			}
		}
	}
}
/*---------------------------------------------------------------------------*/

