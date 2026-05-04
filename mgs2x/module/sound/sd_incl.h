
/*===================================================================
 *  Title           : "METALGEAR SOLID" SOUND
 *                  : マクロ定義
 *  version         : 1.00
 *
 *  Code            : mj001
 *  Filename        : SD_INCL.H
 *  Creator         : K.C.E JAPAN - K.Muraoka
 *  First edition   : 1996/11/13
 *
 *  Note            : 
 *                  : 
 *===================================================================*/

#define SD_INT_05

#define	STEREO 2

/*--------------------- ＳＰＵチャンネル割当て ----------------------*/
#define SPU_CORE_NUM		2	//コアの数(SPU2=2)
/* IRQトラック以外のチャンネル */
#define USE_TRACK_NUM		48	//IRQトラックは未使用

/* 内蔵ＢＧＭと効果音のチャンネル */
#define INTERNAL_TRACK_NUM	(SNG_TRACK_NUM+SE_TRACK_NUM)

/* 内蔵ＢＧＭのチャンネル */
#define SNG_TRACK_NUM		(16*2)	//16Track * 2Song

/* 効果音のチャンネル */
#define SE_START_TRACK		SNG_TRACK_NUM
#define SE_TRACK_NUM		12
#define SE_CORE1_OFFSET		(SNG_TRACK_NUM-24)

/* ストリーミング（ＢＧＭ）のチャンネル */
//#define STR_TRACK_NUM		2

/*--------------------- ＳＰＵ設定 ----------------------------------*/
#define S_PITCH			0x1000		/*周波数(48000Hz)*/

/*--------------------- メインＲＡＭ割当て --------------------------*/
#define SNG_DATA_SIZE	0x20000 //0x10000
#define WAVEHEADERSIZE	0x2000
#define SEHEADERSIZE	0x1000
#define SE_EXP_TABLE_SIZE	0x800	/* 2000/03/13 SEコード変換テーブル*/
#define SE_DATA_SIZE	0x6000
#define CDLOAD_BUF_SIZE	0x10000		//0x18000 /*2001/01/19*/

/*--------------------- ＳＮＧドライバ ------------------------------*/
#define ENV_CUT_OFF			7		/*音カット時のリリースレジスタの値*/
#define SEQ_DRC				0x0048	/*ｼｰｹﾝｽ制御 ﾄﾞﾗﾑ開始ｺｰﾄﾞ*/
#define SNG_NOTE_SIZE		4
#define SNG_VOL				0x10000	/*ＢＧＭフェード割合の最大値*/

/*--------------------- ＳＮＧ制御コード ----------------------------*/
#define	REST_SET	0xf2
#define	KYU_SET		0xf2
#define	TIE_SET		0xf3
#define	SNO_SET		0xd2			/* 音色セット               */

#define	FXS_SET		0xDA	// 2000/01/18
#define	FXE_SET		0xDB	// 2000/01/18

#define	SWP_SET		0xe4			/* ベンド・セット           */
#define L1_S		0xe7			/* loop1 start				*/
#define L1_E		0xe8			/* loop1 end				*/
#define L2_S		0xe9			/* loop2 start				*/
#define L2_E		0xea			/* loop2 end				*/
#define L3_S		0xeb			/* loop2 start				*/
#define L3_E		0xec			/* loop2 end				*/
#define KAKKO_S		0xed			/* kakko start				*/
#define KAKKO_E		0xee			/* kakko end				*/

/*--------------------- 音コード送信タスクへのエラーメッセージ ------*/

//#define ERR_STR_OPEN		-1
//#define ERR_VOX_OPEN		-2
//#define ERR_VOX_NOBLOCK		-3
//#define ERR_SE_OPEN			-4
//#define ERR_SNG_OPEN		-5

//#define SD_TASK_ACTIVE		0x80	/*Started Sound Task*/

/*--------------------- ファイル読み込み種別 ------------------------*/

#define				KIND_STR	1
#define				KIND_WVX	2
#define				KIND_MDX	3
#define				KIND_EFX	4
#define				KIND_PAK	5

#define INT1SEC				(200)	/*およそ１秒間の割り込み回数*/

/*-------------------------------------------------------------------*/
/*                     波形定義用ワーク                              */
/*-------------------------------------------------------------------*/
struct WAVE_W {
unsigned long	addr_le;
char			sample_note;
char			sample_tune;
unsigned char	a_mode;
unsigned char	ar;
unsigned char	dr;
unsigned char	s_mode;
unsigned char	sr;
unsigned char	sl;
unsigned char	r_mode;
unsigned char	rr;
unsigned char	pan;
unsigned char	dec_vol;
};
/*-------------------------------------------------------------------*/
/*                     ＳＰＵレジスタ書き込み用ワーク                */
/*-------------------------------------------------------------------*/
struct	SPU_TRACK_REG {
		unsigned short	vol_l;	/*VOL(L)*/
		unsigned short	vol_r;	/*VOL(R)*/
		int				vol_fg;

		unsigned short	pitch;	/*P(L)/P(R)*/
		int				pitch_fg;

		unsigned long	addr;	/*Wave Address*/
		int				addr_fg;

		long			a_mode;	/*Attack Mode*/
		unsigned short	ar;		/*Attack Rate*/
		unsigned short	dr;		/*Decay Rate*/
		int				env1_fg;

		long			s_mode;	/*Sustain Mode*/
		unsigned short	sr;		/*Sustain Rate*/
		unsigned short	sl;		/*Sustain Level*/
		int				env2_fg;

		long			r_mode;	/*Sustain Mode*/
		unsigned short	rr;		/*Sustain Rate*/
		int				env3_fg;

      float       bp_pan3d;
      float       bp_vol3d;
      int         bp_surround_type;

};
/*-------------------------------------------------------------------*/
/*                     効果音セット用ワーク                          */
/*-------------------------------------------------------------------*/
struct SETBL {
	unsigned char	pri;
	unsigned char	tracks;
	unsigned char	kind;		/* 0:BGM用、1:SE用のパンテーブルを参照*/
	unsigned char	character;	/* 0:音声以外、1:スネーク音声、2:、3:、4:...*/
	unsigned int   addr_le[3];
};
struct SEPLAYTBL {
	unsigned char	pri;
	unsigned char	kind;		/* 0:BGM用、1:SE用のパンテーブルを参照*/
	unsigned char	character;	/* 0:音声以外、1:スネーク音声、2:、3:、4:...*/
	unsigned char	*addr;
	unsigned long	code;
   float          bp_angle;
   int            bp_surround_type;
};
struct CORE_EFCT_WK {
	unsigned char	mode;	//エフェクト種類
	unsigned short	depth;	//エフェクトデプス
	unsigned char	status;	//1=設定待ち、2=リセット待ち
};
/*-------------------------------------------------------------------*/
/*                     各トラック用ワーク                            */
/*-------------------------------------------------------------------*/
struct SOUND_W  {
unsigned char	*mpointer;	/* 音データ・ポインタ */
/*-------------------------------------------------------------------*/
unsigned char	ngc;		/* ステップ・カウンター */
unsigned char	ngo;		/* キーオフ・カウンター */
unsigned char	ngs;		/* ステップ・タイム */
unsigned char	ngg;		/* ゲート・タイム */
/*-------------------------------------------------------------------*/
unsigned char	lp1_cnt;	/* ループ・カウンター */
unsigned char	lp2_cnt;
signed int	  	lp1_vol;	/* ループ音量 */
signed int	  	lp2_vol;
signed int	  	lp1_freq;	/* ループ周波数 */
signed int	  	lp2_freq;
unsigned char 	*lp1_addr;	/* ループ・アドレス */
unsigned char 	*lp2_addr;
unsigned char 	*lp3_addr;
/*-------------------------------------------------------------------*/
unsigned char	kakfg;		/* kakko flag */
unsigned char	*kak1ptr;	/* kakko start address */
unsigned char	*kak2ptr;	/* kakko return address */
/*-------------------------------------------------------------------*/
unsigned char	pvoc;		/* ボリューム移動カウンター */
int				pvod;		/* ボリューム移動データ */
int				pvoad;		/* ボリューム移動１割り込み当りの変化量 */
unsigned int	pvom;		/* ボリューム移動メモリ */
unsigned char	vol;		/* ボリューム (velosity) */
/*-------------------------------------------------------------------*/
char			panc;		/* パン・カウンター */
int				pand;		/* パン・データ */
int         	panad;		/* パン移動加算値 */
int				panm;		/* パン・メモリ */
char			panf;		/* パン・フェーズ */
char			panoff;		/* パン・off flag */
char            panmod;
/*-------------------------------------------------------------------*/
unsigned char	swpc;		/* スィープ・カウンター */
unsigned char	swphc;		/* スィープ・ホールド・カウンター */
unsigned int	swpd;		/* スィープ深さデータ*/
int	    		swpad;		/* スィープ１割り込み当りの変化量 */
unsigned int	swpm;		/* スィープ・メモリ	（目的値） */
/*-------------------------------------------------------------------*/
unsigned char	swsc;		/* スィープ・カウンター */
unsigned char	swshc;		/* スィープ・ホールド・カウンター */
/*-------------------------------------------------------------------*/
char			swsk;		/* スィープ・フラグ */
int				swss;		/* スィープ変化量　（半音単位） */
/*-------------------------------------------------------------------*/
unsigned char	vibhc;		/* ビブラート・ホールド・カウンタ */
unsigned int	vib_tmp_cnt;/* ビブラート・カウンター (H.Inoue) */
unsigned char	vib_tbl_cnt;/* ビブラート・テーブル・カウンタ (H.Inoue) */
unsigned char	vib_tc_ofst;/* ビブラート・テーブル・カウンタ・オフセット (H.Inoue) */
unsigned char	vibcc;		/* ビブラート・チェンジ・カウンタ */
unsigned int	vibd;		/* ビブラート深さ */
unsigned int	vibdm;		/* ビブラート深さメモリ */
unsigned char	vibhs;		/* ビブラート・ホールド・ステップ数 */
unsigned char	vibcs;		/* ビブラート・チェンジ・ステップ数 */
unsigned char	vibcad;		/* ビブラート１ステップ当りの移動量 */
unsigned int	vibad;		/* ビブラート１ステップ当りの変化量 (vch)*/
/*-------------------------------------------------------------------*/
unsigned int	rdmc;		/* ＬＦＯカウンター */
unsigned int	rdmo;		/* ＬＦＯオフセット・ポインター */
unsigned char	rdms;		/* ＬＦＯ速さデータ */
unsigned int	rdmd;		/* ＬＦＯ data */
/*-------------------------------------------------------------------*/
signed char		trec;		/* トレモロ・カウンタ */
unsigned char	trehc;		/* トレモロ・ホールド・カウンター */
char			tred;		/* トレモロ深さ */
unsigned char	trecad;		/* テレモロ１割り込み当りの変化量 */
unsigned char	trehs;		/* トレモロ・ホールド・ステップ数 */
/*-------------------------------------------------------------------*/
unsigned int	snos;		/* 音色ナンバー */
/*-------------------------------------------------------------------*/
int				ptps;		/* パート・トランスポーズ値 */
/*-------------------------------------------------------------------*/
unsigned long	dec_vol;	/* PSX 音量補正値 */
/*-------------------------------------------------------------------*/
signed int		tund;		/* デチューン・データ */
/*-------------------------------------------------------------------*/
unsigned int  	tmpd;		/* テンポ・カウンタ */
unsigned char	tmp;		/* テンポ・データ */
unsigned int  	tmpad;		/* テンポ１割り込み当りの変化量 */
unsigned char	tmpc;		/* テンポ移動カウンター */
unsigned int  	tmpw;		/* テンポ移動値 */
unsigned char	tmpm;		/* テンポ・メモリー（目的）値 */
/*-------------------------------------------------------------------*/
unsigned int	rest_fg;	/* 休符フラグ */
/*-------------------------------------------------------------------*/
signed char		macro;		/* チューニング・データ（音程） */
unsigned char		micro;		/* チューニング・データ（チューン） */
/*-------------------------------------------------------------------*/
unsigned short	rrd;		/* release depth */
/*-------------------------------------------------------------------*/
/* 2000/01/17 */
unsigned int	fx_time_base;	/* FX wait タイムベース */
unsigned char	fx_mode;		/* FX wait モード 0-31:別トラックからの起動、255:音コードによる起動 */
unsigned char	fx_ngc;			/* FX wait ステップカウンタ*/
unsigned char	*fx_addr;		/* FXスタートアドレス */
unsigned char	fx_play_fg;		/* FX再生中フラグ */
unsigned char	fx_on;			/* 別トラックのFX再生on */
/*-------------------------------------------------------------------*/
/* 2000/01/26 */
unsigned int	auto_pos;	/*位置/フェーズ・データ保存*/
unsigned char	auto_mode;	/* 0=エンベロープモード、1=タイマーモード */
unsigned char	auto1_vol;	/* オートメーション1音量 */
unsigned char	auto2_vol;	/* オートメーション2音量 */
unsigned char	auto3_vol;	/* オートメーション3音量 */
unsigned char	auto4_vol;	/* オートメーション4音量 */
unsigned char	auto5_vol;	/* オートメーション5音量 */
/*2000/07/07*/
unsigned char	auto6_vol;	/* オートメーション6音量 */
unsigned char	auto7_vol;	/* オートメーション7音量 */
unsigned char	auto8_vol;	/* オートメーション8音量 */

unsigned char	auto1_tim;	/* オートメーション1位置/タイマー */
unsigned char	auto2_tim;	/* オートメーション2位置/タイマー */
unsigned char	auto3_tim;	/* オートメーション3位置/タイマー */
unsigned char	auto4_tim;	/* オートメーション4位置/タイマー */
unsigned char	auto5_tim;	/* オートメーション5位置/タイマー */
unsigned char	auto6_tim;	/* オートメーション6位置/タイマー */
unsigned char	auto7_tim;	/* オートメーション7位置/タイマー */
unsigned char	auto8_tim;	/* オートメーション8位置/タイマー */
/*-------------------------------------------------------------------*/
};
/*--- Mixing Fader ---*/ //1999/12/22
struct MIX_FADER {
	signed int	int_vol;
	unsigned int	now_vol;
	unsigned int	dst_vol;

	unsigned int	now_pan;

	unsigned int	now_frq;
	unsigned int	last_frq;
};

/*--- Memory Streaming ---*/	//2000/02/08
#define MEM_STR_BUF_SIZE	0x80000	//0xA0000	2001/01/25
#define STR_SPU_START		0x5020
#define STR2_SPU_START		0x7020
#define MEM_SPU_START_PTR	(STR_SPU_START+0x4000)
struct MEMSTR_W {
	int				status;
	unsigned int	wave_size;
	unsigned int	unplay_size;
	unsigned int	snos;
	unsigned char	*play_offset;
	unsigned int	next_idx;
   const void *   bp_override_wav_addr;
   int            bp_override_wav_size;
};

/*--- EE読み込みアドレス -----*/
struct EE_ADDR {
	void			*base;
	unsigned int	ofst[2];
	unsigned int	set_ctr;
	unsigned int	read_ctr;
	unsigned int	sz;
};

/*--- 音声用Mixing Fader ---*/ //2000/06/29
struct VOX_FADER {
	unsigned int	set_vol;
	unsigned int	play_vol;
	unsigned int	set_pan;
	unsigned int	play_pan;
   float          set_bp_angle;
   float          play_bp_angle;
};
/*--- 使用DMAチャンネル -----*/
#define DMA_CH 0	//"0"を即値で使用している箇所もあり、要注意。

/*--- 再生カウンタ更新用 (add by K.Uehara) -----*/

typedef struct {
	unsigned long long start;
	int offset;
} STR_TIME;

void reset_play_counter( STR_TIME *st, int offset );
int get_play_counter( STR_TIME *st );

