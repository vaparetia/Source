/*======================================================================*/
/*=                                                                    =*/
/*=                   <<<<<<<<< sd_sub2.c >>>>>>>>>                    =*/
/*=         サウンド・エディター音プログラム 制御コード・セット        =*/
/*=                                                                    =*/
/*======================================================================*/

#include	<libsd.h>
#include "sd_debug.h"
#include "sd_incl.h"
#include "sd_ext.h"

#include "BP_SoundEffectOverride.h"

#define	SE_ELEVATOR	(0x0BE)

/************************************************************************/
/*                      外部定義                                        */
/************************************************************************/
extern void		     tone_set(unsigned int n);
extern void		     keyoff(void),rroff(void);
extern void		     dspwr(unsigned long reg, unsigned long data);
/************************************************************************/
/*                      関数定義                                        */
/************************************************************************/
void	rest_set(),		sno_set(),		pan_set(),		pan_move();
void	vib_set(),  	rdm_set(),  	lp1_start(),    lp1_end();
void	lp2_start(),	lp2_end(),  	tempo_set(),    tempo_move();
void	trans_set(),	tre_set(),  	vol_chg(),      vol_move();
void	vib_change(),	por_set(),  	sws_set(),      detune_set();
void	swp_set(),      tie_set(),      echo_set1(),    echo_set2();
void	kakko_start(),	kakko_end(),	env_set(),      pm_set();
void	jump_set(),     no_cmd();
void    ads_set(void),  srs_set(void),  rrs_set(void);
void	eon_set(void),	eof_set(void);
void    l3s_set(void),l3e_set(void),svl_set(void),svp_set(void),use_set(void);
void    block_end(void);
void    ofs_set(void);
// 2000/01/17
void	fxs_set(void), fxe_set(void), xon_set(void);
// 2000/01/18
void vol_i_move(void);
void at1_set(void), at2_set(void), at3_set(void), at4_set(void), at5_set(void);
// 2000/02/14
void mno_set(void);
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（休符セット KYU）			*/
/*----------------------------------------------------------------------*/
void rest_set()
{
	sptr->rest_fg = 1;	/* 休符フラグ on */
	keyoff();		/* キーオフ */
	sptr->ngs = mdata2;	/* step time */
	sptr->ngg = 0;		/* gate time */
	sptr->vol = 0;		/* velosity  */

	sptr->ngc = sptr->ngs;	/* ステップ・カウンタのセット */
	sptr->ngo = 0;		/* ゲート・オフ・セット */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（タイ・セット TIE）			*/
/*----------------------------------------------------------------------*/
void tie_set()
{
	unsigned int	x;

	sptr->rest_fg = 1;		   /* 休符フラグ on */
	sptr->ngs = mdata2;		   /* step time */
	sptr->ngg = mdata3;								   	   /* gate time */

	sptr->ngc = sptr->ngs;		   /* ステップ・カウンタのセット */
	x = (sptr->ngg * sptr->ngc) / 100; /* 百分率をステップ数に変換 */
	if(x == 0){			   /* ゲートの最小限は１ */
		x = 1;
	}
	sptr->ngo = (unsigned char)x;	   /* ゲート・データ・セット */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（音色セット SNO）                        */
/*----------------------------------------------------------------------*/
void sno_set()
{
	sptr->snos = mdata2;   /* 音色Ｎｏ．セーブ */
	keyoff();              /* クリック防止の為のキーＯＦＦ */
	tone_set(mdata2);      /* 音色セット */
}
void svl_set()
{
	sptr->snos = mdata2;   /* 音色Ｎｏ．セーブ */
	keyoff();              /* クリック防止の為のキーＯＦＦ */
	tone_set(mdata2);      /* 音色セット */
}
void svp_set()
{
	sptr->snos = mdata2;   /* 音色Ｎｏ．セーブ */
	keyoff();              /* クリック防止の為のキーＯＦＦ */
	tone_set(mdata2);      /* 音色セット */
}
void ofs_set()	/*1999/06/01 for MGS2*/
{
	
	spu_tr_wk[mtrack].addr += (mdata2*0x1000)+(mdata3*0x10);
	spu_tr_wk[mtrack].addr_fg = 1;
}

void use_set()
{
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（パンセット PAN）                        */
/*----------------------------------------------------------------------*/
void pan_set()
{
    sptr->panmod =mdata2;
    sptr->panf = mdata3+20;
    sptr->pand = sptr->panf << 8;
    sptr->panc = 0;		/* パン移動クリア */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（パン移動セット PAM）                    */
/*----------------------------------------------------------------------*/
void pan_move()
{
    int pan_data;
    unsigned char pand_shift;

    sptr->panc =mdata2; /* 移動ステップ数 */
    pand_shift= (unsigned char)(mdata3 + 20); /* 移動目的値 */
    sptr->panm = (unsigned int)(pand_shift << 8); /* 移動目的値 */
    pan_data = (int)(pand_shift - sptr->panf); /* 目的値 - 現在値 */
    if(pan_data < 0){/* 右へ移動 */
        sptr->panad = -(((-pan_data)<<8)/mdata2);
        if(sptr->panad < -0x7f0) sptr->panad=-0x7f0;
    }else{ /* 左へ移動 */
        sptr->panad = ((pan_data<<8)/mdata2);
        if(sptr->panad > 0x7f0) sptr->panad=0x7f0;
    }

}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（ビブラートセット VIB）                  */
/*----------------------------------------------------------------------*/
void vib_set()
{
	sptr->vibhs = mdata2;		/* ホールド・カウンタ */
	sptr->vibcad = mdata3;		/* 速さ */
/* (H.Inoue)==> */
	if(sptr->vibcad<0x40){
		if(sptr->vibcad<0x20){
			sptr->vib_tc_ofst=1;	/* 01-1F */
			sptr->vibcad=(sptr->vibcad<<3);
		}
		else{
			sptr->vib_tc_ofst=2;	/* 20-3F */
			sptr->vibcad=(sptr->vibcad<<2);
		}
	}
	else{
		if(sptr->vibcad<0x80){
			sptr->vib_tc_ofst=4;	/* 40-7F */
			sptr->vibcad=(sptr->vibcad<<1);
		}
		else{
			if(sptr->vibcad!=0xff){
				sptr->vib_tc_ofst=8;  /* 80-FE */
			}
			else{
				sptr->vib_tc_ofst=16; /* FF  For SE */
			}
		}
	}
	sptr->vibd = (unsigned int)mdata4 << 8;	  /* vibdm:深さ＝上位正数値 */
	sptr->vibdm = (unsigned int)mdata4 << 8;  /* vibdm:深さ＝下位少数値 */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（ビブラートチェンジ VIM）		*/
/*----------------------------------------------------------------------*/
void vib_change()
{
	sptr->vibcs = mdata2;		    /* チェンジ・カウンタ */
	sptr->vibad = (unsigned int)(sptr->vibdm/mdata2); /* １ステップの変化量 */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（ＬＦＯセット RDM）			*/
/*----------------------------------------------------------------------*/
void rdm_set()
{
	sptr->rdms = mdata2;                 /* 速さ */
	sptr->rdmd = (mdata3 << 8) + mdata4; /* 深さ(2bytes) */
	sptr->rdmc = 0;	                     /* カウンタ */
	sptr->rdmo = 0;	                     /* ＬＦＯテーブル・オフセット */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（ループ）                                */
/*----------------------------------------------------------------------*/
void lp1_start()
{
	sptr->lp1_addr = mptr;		    /* スタート・アドレス・セーブ */
	sptr->lp1_cnt = 0;		    /* カウンタ・クリア */
	sptr->lp1_vol = sptr->lp1_freq = 0; /* 変化音量、周波数クリア */

}

void lp1_end()
{
	if (mtrack < SNG_TRACK_NUM) {
/*イントロループ・スキップ*/
		if ((skip_intro_loop) && (mdata2 == 0)) {	// 2000/01/18
			sptr->lp1_vol = 0;
			sptr->lp1_freq = 0;
			skip_intro_loop++;
			goto lp1_end_term;
		}
	} else {
/*常駐効果音のストップ*/
		if ((stop_jizoku_se) && (mdata2 == 0)) {
			sptr->lp1_vol = 0;
			sptr->lp1_freq = 0;
			stop_jizoku_se++; /*1998/01/27 K.Muraoka*/
			goto lp1_end_term;
		}
		if ((stop_jizoku_se2) && (mdata2 == 0)) {
			if ((se_playing[mtrack-SNG_TRACK_NUM].code & 0xFFF) != SE_ELEVATOR) {
				sptr->lp1_vol = 0;
				sptr->lp1_freq = 0;
				stop_jizoku_se2++;
				goto lp1_end_term;
			}
		}
	}
	if(((++sptr->lp1_cnt) != mdata2) || (mdata2 == 0)){ /* ループ終了でなければ．．． */
		sptr->lp1_vol += (signed int)((signed char)mdata3); /* ループ音量加算 */
		sptr->lp1_freq += (signed int)((signed char)mdata4) << 3; /* ループ周波数加算 */
		mptr = sptr->lp1_addr; /* アドレス再セット */
	} else {
		sptr->lp1_vol = 0;
		sptr->lp1_freq = 0;
	}
lp1_end_term:
   (void)(0); //BP
}
/*----------------------------------------------------------------------*/
void lp2_start()
{
	sptr->lp2_addr = mptr;		    /* スタート・アドレス・セーブ */
	sptr->lp2_cnt = 0;		    /* カウンタ・クリア */
	sptr->lp2_vol = sptr->lp2_freq = 0; /* 変化音量、周波数クリア */
}

void lp2_end()
{
	if(((++sptr->lp2_cnt) != mdata2) || (mdata2 == 0)){
						  /* ループ終了でなければ．．． */
		sptr->lp2_vol += (signed int)((signed char)mdata3);
						  /* ループ音量加算 */
		sptr->lp2_freq += (signed int)((signed char)mdata4) << 3;
						  /* ループ周波数加算 */
		mptr = sptr->lp2_addr;		  /* アドレス再セット */
	}
}
/*----------------------------------------------------------------------*/
void l3s_set()
{
	sptr->lp3_addr = mptr;		    /* スタート・アドレス・セーブ */

}

void l3e_set()   /* (H.Inoue) */
{
    if(sptr->lp3_addr){
        mptr = sptr->lp3_addr;  /* アドレス再セット */
    }else{
        block_end();            /* 終了 */
    }
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（テンポ・セット TPS）			*/
/*----------------------------------------------------------------------*/
void tempo_set()
{
	sptr->tmp = mdata2;
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（テンポ移動セット TPM）                  */
/*----------------------------------------------------------------------*/
void tempo_move()
{
	register signed int tmp_data;

	sptr->tmpc = mdata2;			    /* 移動ステップ数 */
	sptr->tmpm = mdata3;			    /* 移動目的値 */
	sptr->tmpw = (unsigned int)sptr->tmp << 8;
	tmp_data = (int)(sptr->tmpm - sptr->tmp);
	if(tmp_data < 0){		    /* だんだん遅くなる */
	    if(tmp_data<-127) tmp_data=-127;
		sptr->tmpad = (int)(-(((int)(-tmp_data) << 8) / sptr->tmpc));
		if(sptr->tmpad < -0x7f0) sptr->tmpad = -0x7f0;/* For New Press (H.Inoue) */
	}else{					    /* だんだん速くなる */
	    if(tmp_data>127) tmp_data=127;
		sptr->tmpad = (int)(((int)tmp_data << 8) / sptr->tmpc);
		if(sptr->tmpad > 0x7f0) sptr->tmpad = 0x7f0;/* For New Press (H.Inoue) */
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（トランスポーズ・セット TRN）			*/
/*----------------------------------------------------------------------*/
void trans_set()
{
	sptr->ptps = (signed int)((signed char)mdata2);	  /* トランスポーズ値セット */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（トレモロ・セット TRS）			*/
/*----------------------------------------------------------------------*/
void tre_set()
{
	sptr->trehs = mdata2;		/* ホールド・ステップ数 */
	sptr->trecad = mdata3;		/* トレモロ速さ */
	sptr->tred = mdata4;		/* トレモロ深さ */
}
/*
{
	sptr->trehs = mdata2;
	sptr->treptr = tbl_ptr + (unsigned int)(mdata3 * 0x80);
	sptr->trecs = mdata4;
}
*/
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（音量セット VOL）                        */
/*----------------------------------------------------------------------*/
void vol_chg()
{
	sptr->pvod = (int)mdata2 << 8;	/* トラック音量 */
	sptr->pvoc = 0;			/* 音量移動クリア */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（音量移動 VOM）	                        */
/*----------------------------------------------------------------------*/
void vol_move()
{
	signed int vol_data;

	sptr->pvoc = mdata2;		    /* 移動ステップ数 */
	sptr->pvom = mdata3;	        /* 移動目的値 */
	vol_data = (int)mdata3 << 8;
	vol_data -= sptr->pvod;		    /* 目的値 - 現在値 */
	if(vol_data<0){	                /* だんだん小さくなる */
		sptr->pvoad = -(-vol_data / sptr->pvoc);
		if(sptr->pvoad < -0x7f0) sptr->pvoad=-0x7f0;/* For New Press (H.Inoue) */
	}else{				/* だんだん大きくなる */
		sptr->pvoad = vol_data / sptr->pvoc;
		if(sptr->pvoad > 0x7f0) sptr->pvoad=0x7f0;/* For New Press (H.Inoue) */
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（ポルタメント POR）						*/
/*----------------------------------------------------------------------*/
void por_set()
{
	sptr->swshc = 0;		/* ホールド時間 = 0 */
	sptr->swsc = mdata2;	/* ポルタメント速さ */
	if(mdata2==0){
	    sptr->swsk=0;		/* スィープ */
    }else{
	    sptr->swsk=1;		/* ポルタメント */
    }
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（スィープ SWS）                          */
/*----------------------------------------------------------------------*/
void sws_set()
{
	sptr->swsk = 0;			    /* ポルタメント OFF */
	sptr->swshc = mdata2;	  	/* ホールド・カウンタ・セット */
	sptr->swsc = mdata3;		/* スィープ速さ */
	sptr->swss = mdata4 << 8;	/* スィープ深さ */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（デチューン DTS）                        */
/*----------------------------------------------------------------------*/
void detune_set()
{
	sptr->tund = (signed int)((signed char)mdata2) << 2; /* デチューン値 */
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット(ダミー SWP)                              */
/*----------------------------------------------------------------------*/
void swp_set()
{
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（エコー）                                */
/*----------------------------------------------------------------------*/
void echo_set1()
{
/*
	SpuReverbAttr		r_attr;

	SpuSetReverb(SPU_OFF);
	SpuReserveReverbWorkArea(SPU_OFF);

	r_attr.mask = (SPU_REV_MODE);
	switch (mdata2) {
	case 1: r_attr.mode = SPU_REV_MODE_ROOM; break;
	case 2: r_attr.mode = SPU_REV_MODE_STUDIO_A; break;
	case 3: r_attr.mode = SPU_REV_MODE_STUDIO_B; break;
	case 4: r_attr.mode = SPU_REV_MODE_STUDIO_C; break;
	case 5: r_attr.mode = SPU_REV_MODE_HALL; break;
	case 6: r_attr.mode = SPU_REV_MODE_SPACE; break;
	case 7:
		r_attr.mask |= (SPU_REV_DELAYTIME | SPU_REV_FEEDBACK);
		r_attr.mode = SPU_REV_MODE_ECHO;
		r_attr.delay = mdata3;
		r_attr.feedback = mdata4;
		break;
	case 8:
		r_attr.mask |= (SPU_REV_DELAYTIME | SPU_REV_FEEDBACK);
		r_attr.mode = SPU_REV_MODE_DELAY;
		r_attr.delay = mdata3;
		r_attr.feedback = mdata4;
		break;
	case 9: r_attr.mode = SPU_REV_MODE_PIPE; break;
	default: r_attr.mode = SPU_REV_MODE_OFF; break;
	}

	SpuSetReverbModeParam(&r_attr);

	if ( (SpuReserveReverbWorkArea(SPU_ON)) == SPU_OFF ) {
		PRINTF(("ERR:Can'tReservedReverbWorkArea"));
	} else {
		SpuSetReverb(SPU_ON);
	}
*/
}

void echo_set2()
{
/*
	SpuReverbAttr		r_attr;

	r_attr.mask = (SPU_REV_DEPTHL | SPU_REV_DEPTHR);
	r_attr.depth.left =  ((short)mdata2)<<8;
	r_attr.depth.right = ((short)mdata3)<<8;

	SpuSetReverbDepth(&r_attr);
*/
}

void eon_set(void)
{
	if ((mtrack >= SNG_TRACK_NUM) && (mtrack < INTERNAL_TRACK_NUM)) {
		if ((!se_playing[mtrack-SNG_TRACK_NUM].kind) //システムＳＥ
			|| fg_rev_set[mtrack]) {				//トラック内の情報でリバーブセット
			rev_on_bit[1] |= keyd[1];
			rev_bit_data[1] |= keyd[1];
		}
	} else {	//for BGM (1999/12/08)
		rev_on_bit[0] |= keyd[0];
		rev_on_bit[1] |= keyd[1];
		rev_bit_data[0] |= keyd[0];
		rev_bit_data[1] |= keyd[1];
	}
}

void eof_set(void)
{
	if ((mtrack >= SNG_TRACK_NUM) && (mtrack < INTERNAL_TRACK_NUM)) {
		if ((!se_playing[mtrack-SNG_TRACK_NUM].kind)	//システムＳＥ
			|| fg_rev_set[mtrack]) {				//トラック内の情報でリバーブセット
			rev_off_bit[1] |= keyd[1];
			rev_bit_data[1] &= ~keyd[1];
		}
	} else {	// for BGM (1999/12/08)
		rev_off_bit[0] |= keyd[0];
		rev_off_bit[1] |= keyd[1];
		rev_bit_data[0] &= ~keyd[0];
		rev_bit_data[1] &= ~keyd[1];
	}
}

/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（カッコ）                                */
/*----------------------------------------------------------------------*/
void kakko_start()
{
	sptr->kak1ptr = mptr;	/* データ・アドレス・セーブ */
	sptr->kakfg = 0;							/* カッコ・フラグ初期化 */
}

void kakko_end()
{
	switch(sptr->kakfg){
	case 0:			       /* カッコ・エンド */
		++sptr->kakfg;	       /* カッコ・フラグ = 1 */
		break;
	case 1:			       /* カッコ・コール */
		++sptr->kakfg;	       /* カッコ・フラグ = 2 */
		sptr->kak2ptr = mptr;  /* データ・アドレス・セーブ */
		mptr = sptr->kak1ptr;  /* スタート・アドレス・セット */
		break;
	case 2:
		--sptr->kakfg;	       /* カッコ・リターン */
		mptr = sptr->kak2ptr;  /* スタート・アドレス・セット */
		break;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（エンベロープモード・セット ENV）		*/
/*----------------------------------------------------------------------*/
void env_set()	/*1999/06/01 for MGS2*/
{
/*Attack Mode*/
	if (mdata2) spu_tr_wk[mtrack].a_mode = SD_ADSR_A_EXP;
	else spu_tr_wk[mtrack].a_mode = SD_ADSR_A_LINEAR;
	spu_tr_wk[mtrack].env1_fg = 1;

/*Sustain Mode*/
	switch (mdata3) {
	case 0: spu_tr_wk[mtrack].s_mode = SD_ADSR_S_LINEAR_DEC;break;
	case 1: spu_tr_wk[mtrack].s_mode = SD_ADSR_S_EXP_DEC;break;
	case 2: spu_tr_wk[mtrack].s_mode = SD_ADSR_S_LINEAR_INC;break;
	default: spu_tr_wk[mtrack].s_mode = SD_ADSR_S_EXP_INC;break;
	}
	spu_tr_wk[mtrack].env2_fg = 1;

/* Release Mode */
	if (mdata4) spu_tr_wk[mtrack].r_mode = SD_ADSR_R_EXP;
	else spu_tr_wk[mtrack].r_mode = SD_ADSR_R_LINEAR;
	spu_tr_wk[mtrack].env3_fg = 1;
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（エンベロープ・セット）                  */
/*----------------------------------------------------------------------*/
void ads_set()
{
	spu_tr_wk[mtrack].a_mode = SD_ADSR_A_LINEAR;/*リニアモード固定*/
	spu_tr_wk[mtrack].ar = 127 - (mdata2 & 0x7F);
	spu_tr_wk[mtrack].dr = 15 - (mdata3 & 0xF);
	spu_tr_wk[mtrack].sl = (mdata4 & 0xF);
	spu_tr_wk[mtrack].env1_fg = 1;
/*
	if (mdata2 == 0) {
		spu_tr_wk[mtrack].a_mode = SPU_VOICE_LINEARIncN;
	} else {
		spu_tr_wk[mtrack].a_mode = SPU_VOICE_EXPIncN;
	}
	spu_tr_wk[mtrack].ar = 127 - (mdata3 & 0x7F);
	spu_tr_wk[mtrack].dr = 15 - (mdata4 & 0xF);
	spu_tr_wk[mtrack].env1_fg = 1;
*/
}

void srs_set()
{
	spu_tr_wk[mtrack].s_mode = SD_ADSR_S_LINEAR_DEC;/*リニアモード固定*/
	spu_tr_wk[mtrack].sr = 127 - (mdata2 & 0x7F);
/*	spu_tr_wk[mtrack].rr = 31-(mdata3 & 0x1F);
*/
	spu_tr_wk[mtrack].env2_fg = 1;
/*
	switch (mdata2) {
	case 0: spu_tr_wk[mtrack].s_mode = SPU_VOICE_LINEARDecN;break;
	case 1: spu_tr_wk[mtrack].s_mode = SPU_VOICE_EXPDecN;break;
	case 2: spu_tr_wk[mtrack].s_mode = SPU_VOICE_LINEARIncN;break;
	default: spu_tr_wk[mtrack].s_mode = SPU_VOICE_EXPIncN;break;
	}
	spu_tr_wk[mtrack].sr = 127 - (mdata3 & 0x7F);
	spu_tr_wk[mtrack].sl = (mdata4 & 0xF);
	spu_tr_wk[mtrack].env2_fg = 1;
*/
}

void rrs_set()
{
	spu_tr_wk[mtrack].r_mode = SD_ADSR_R_LINEAR;/*リニアモード固定*/
	sptr->rrd = spu_tr_wk[mtrack].rr = 31 - (mdata2 & 0x1F);
	spu_tr_wk[mtrack].env3_fg = 1;
/*
	if (mdata2 == 0) {
		spu_tr_wk[mtrack].r_mode = SPU_VOICE_LINEARDecN;
	} else {
		spu_tr_wk[mtrack].r_mode = SPU_VOICE_EXPDecN;
	}
	sptr->rrd = spu_tr_wk[mtrack].rr = 31 - (mdata3 & 0x1F);
	spu_tr_wk[mtrack].env3_fg = 1;
*/
}

/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（ピッチ・モジュレーション・セット PMS）	*/
/*----------------------------------------------------------------------*/
void pm_set()
{
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（ジャンプ・セット JMP）                  */
/*----------------------------------------------------------------------*/
void jump_set()
{
}
/*----------------------------------------------------------------------*/
/*  title:  BLOCK ENDコード・セット                                     */
/*----------------------------------------------------------------------*/
void block_end(void)
{
    keyoffs[0] |= keyd[0];
    keyoffs[1] |= keyd[1];
}
/*----------------------------------------------------------------------*/
/*  title:  FXトラックスタート                                          */
/*----------------------------------------------------------------------*/
/* 2000/01/17 */
void fxs_set()
{
	sptr->fx_mode = mdata2;
	sptr->fx_time_base = mdata3;
	sptr->fx_addr = mptr;
}
/*----------------------------------------------------------------------*/
/*  title:  FXトラックエンド                                            */
/*----------------------------------------------------------------------*/
/* 2000/01/17 */
void fxe_set()
{
	mptr = sptr->fx_addr;
	sptr->fx_play_fg = 0;
}
/*----------------------------------------------------------------------*/
/*  title:  別トラックのFXを起動                                        */
/*----------------------------------------------------------------------*/
/* 2000/01/17 */
void xon_set()
{
	sptr->fx_on = 1;
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（トラック指定音量移動 VOI）              */
/*----------------------------------------------------------------------*/
void vol_i_move()
{
PRINTF(("*** ERROR: SoundData(voi):mtrack=%x\n", mtrack));
#if 0	//MGS2では使用しない
	signed int vol_data;
	struct SOUND_W *p;

	p = &sound_w[mdata2];
	p->pvoc = mdata3;		    /* 移動ステップ数 */
	if (p->pvoc == 0) p->pvoc = 1;
	p->pvom = mdata4;	        /* 移動目的値 */
	vol_data = (int)mdata4 << 8;
	vol_data -= p->pvod;		    /* 目的値 - 現在値 */
	if(vol_data<0){	                /* だんだん小さくなる */
		p->pvoad = -(-vol_data / p->pvoc);
		if(p->pvoad < -0x7f0) p->pvoad=-0x7f0;/* For New Press (H.Inoue) */
	}else{				/* だんだん大きくなる */
		p->pvoad = vol_data / p->pvoc;
		if(p->pvoad > 0x7f0) p->pvoad=0x7f0;/* For New Press (H.Inoue) */
	}
#endif
}
// 2000/01/26
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（オートメーション設定 1）                */
/*----------------------------------------------------------------------*/
void at1_set()
{
	if ( (sptr->auto_pos!=auto_env_pos) && (sptr->auto_pos!=auto_phase_fg) ) {
		mix_fader[mtrack].dst_vol = ((unsigned int)(mdata2)<<8)+ mdata2; //max.=0xFFFF
		mix_fader[mtrack].now_vol = mix_fader[mtrack].dst_vol;
		mix_fader[mtrack].int_vol = 0;
	}
	sptr->auto1_vol = mdata2; //max.=0xFF
	sptr->auto1_tim = mdata3;
	sptr->auto_mode = mdata4;
	if (sptr->auto_mode == 1) {
		sptr->auto2_vol = mdata2; //max.=0xFF
		sptr->auto2_tim = 0;
		sptr->auto3_vol = mdata2;
		sptr->auto3_tim = 0;
		sptr->auto4_vol = mdata2;
		sptr->auto4_tim = 0;
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = 0;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 0;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 0;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 0;
	} else {
		sptr->auto2_vol = mdata2;
		sptr->auto2_tim = 255;
		sptr->auto3_vol = mdata2;
		sptr->auto3_tim = 255;
		sptr->auto4_vol = mdata2;
		sptr->auto4_tim = 255;
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = 255;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 255;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 255;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 255;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（オートメーション設定 2）                */
/*----------------------------------------------------------------------*/
void at2_set()
{
	if (sptr->auto_mode == 1) {
		sptr->auto2_vol = mdata2; //max.=0xFF
		sptr->auto2_tim = mdata3;
		sptr->auto3_vol = mdata2;
		sptr->auto3_tim = 0;
		sptr->auto4_vol = mdata2;
		sptr->auto4_tim = 0;
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = 0;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 0;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 0;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 0;
	} else {
		sptr->auto2_vol = mdata2; //max.=0xFF
		sptr->auto2_tim = mdata3;
		sptr->auto3_vol = mdata2;
		sptr->auto3_tim = 255;
		sptr->auto4_vol = mdata2;
		sptr->auto4_tim = 255;
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = 255;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 255;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 255;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 255;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（オートメーション設定 3）                */
/*----------------------------------------------------------------------*/
void at3_set()
{
	if (sptr->auto_mode) {
		sptr->auto3_vol = mdata2; //max.=0xFF
		sptr->auto3_tim = mdata3;
		sptr->auto4_vol = mdata2;
		sptr->auto4_tim = 0;
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = 0;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 0;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 0;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 0;
	} else {
		sptr->auto3_vol = mdata2; //max.=0xFF
		sptr->auto3_tim = mdata3;
		sptr->auto4_vol = mdata2;
		sptr->auto4_tim = 255;
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = 255;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 255;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 255;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 255;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（オートメーション設定 4）                */
/*----------------------------------------------------------------------*/
void at4_set()
{
	if (sptr->auto_mode) {
		sptr->auto4_vol = mdata2; //max.=0xFF
		sptr->auto4_tim = mdata3;
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = 0;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 0;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 0;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 0;
	} else {
		sptr->auto4_vol = mdata2;
		sptr->auto4_tim = mdata3;
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = 255;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 255;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 255;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 255;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（オートメーション設定 5）                */
/*----------------------------------------------------------------------*/
void at5_set()
{
	if (sptr->auto_mode == 1) {
		sptr->auto5_vol = mdata2; //max.=0xFF
		sptr->auto5_tim = mdata3;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 0;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 0;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 0;
	} else {
		sptr->auto5_vol = mdata2;
		sptr->auto5_tim = mdata3;
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = 255;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 255;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 255;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（オートメーション設定 6）                */
/*----------------------------------------------------------------------*/
void at6_set()
{
	if (sptr->auto_mode == 1) {
		sptr->auto6_vol = mdata2; //max.=0xFF
		sptr->auto6_tim = mdata3;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 0;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 0;
	} else {
		sptr->auto6_vol = mdata2;
		sptr->auto6_tim = mdata3;
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = 255;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 255;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（オートメーション設定 7）                */
/*----------------------------------------------------------------------*/
void at7_set()
{
	if (sptr->auto_mode == 1) {
		sptr->auto7_vol = mdata2; //max.=0xFF
		sptr->auto7_tim = mdata3;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 0;
	} else {
		sptr->auto7_vol = mdata2;
		sptr->auto7_tim = mdata3;
		sptr->auto8_vol = mdata2;
		sptr->auto8_tim = 255;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（オートメーション設定 8）                */
/*----------------------------------------------------------------------*/
void at8_set()
{
	sptr->auto8_vol = mdata2; //max.=0xFF
	sptr->auto8_tim = mdata3;
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（音色セット MNO）                        */
/*----------------------------------------------------------------------*/
void mno_set()
{
	if (mtrack >= SE_START_TRACK) {
      int track = mtrack - SE_START_TRACK;
#if 1 //BP added to support precooked waveforms to replace entire sound effects.
      if( mdata2 == 0xff && (mdata3&0xf0) == 0xf0 )
      {
         int wavIndex = (int)mdata4 | ((int)(mdata3&0xf)<<8);
         mem_str_w[track].bp_override_wav_addr = BP_GetWavOverrideAddress( wavIndex, &mem_str_w[track].bp_override_wav_size );
         //Set it to a value that indicates it's a memory stream; snos isn't used for anything else problematic.
         //(memory stream "snos" start at 0x100)
         mem_str_w[track].snos = sptr->snos = 0x100;
         keyoff();
         //manually set ADSR envelope.
         spu_tr_wk[mtrack].a_mode = 0;
         spu_tr_wk[mtrack].ar = 0;
         spu_tr_wk[mtrack].ar = 15;
         spu_tr_wk[mtrack].env1_fg = 1;
         spu_tr_wk[mtrack].s_mode = SD_ADSR_S_LINEAR_DEC;
         spu_tr_wk[mtrack].sr = 127;
         spu_tr_wk[mtrack].sl = 15;
         spu_tr_wk[mtrack].env2_fg = 1;
         spu_tr_wk[mtrack].r_mode = 0;
         spu_tr_wk[mtrack].rr = sptr->rrd = 6;
         spu_tr_wk[mtrack].env3_fg = 1;
         sptr->macro = 0;
         sptr->micro = 0;
      }
      else
      {
         mem_str_w[track].bp_override_wav_addr = 0;
         mem_str_w[track].bp_override_wav_size = 0;
         mem_str_w[track].snos = sptr->snos = mdata2+0x100;   /* 音色Ｎｏ．セーブ */
         keyoff();              /* クリック防止の為のキーＯＦＦ */
         tone_set(sptr->snos);      /* 音色セット */
      }
#endif
	}
}
/*----------------------------------------------------------------------*/
/*  title:  制御コード・セット（各種フラグセット FLG）                  */
/*----------------------------------------------------------------------*/
void flg_set()
{
//主観モード時、音量下げないフラグ
	switch (mdata2) {
	case 0:
		fg_syukan_off[mtrack] = mdata3;
		break;
//SEモードに関わらず、トラック内の情報でリバーブをセットする
	case 1:
		fg_rev_set[mtrack] = mdata3;
		break;
	}
}
/*----------------------------------------------------------------------*/
/*  title:  未定義制御コード・セット                                    */
/*----------------------------------------------------------------------*/
void no_cmd()
{
}
/*---------------------------------------------------------------------------*/
