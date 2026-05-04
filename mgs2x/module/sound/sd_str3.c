#include	<stdio.h>
#include	<kernel.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	<sif.h>
#include	<sifcmd.h>
#include	<sifrpc.h>
//BP_PS2 #include	"../jsifman/jsifman.h"
#include	<libsd.h>
#include	"sd_debug.h"
#include	"sd_incl.h"
#include	"sd_ext.h"

#include "mgs_type.h"

#include "BP_EndianSupport.h"
#include "BP_SoundSupport.h"

/*========================================================================*/
/*使用するDMAチャンネルの登録*/
#define AC3_DMA_CH	0
#define LNR_DMA_CH	1
/*使用するコア番号の登録*/
#define AC3_CORE	SD_CORE_0
#define LNR_CORE	SD_CORE_1

/*8bitバッファ基本サイズ*/
#define LNR8_BUF_SIZE	0x400	//0x400
/*8bitバッファ->16bitバッファ転送数*/
#define LNR8_BUF_NUM	((LNR8_LOAD_SIZE*2)/LNR8_BUF_SIZE)	//0x10000/0x800=0x20 //0x8000/0x400=0x20
/*１回のEE読み込み量*/
#define EE_TRANS_SIZE		0x4000
#define LNR8_LOAD_SIZE	EE_TRANS_SIZE
/*16bitバッファ基本サイズ*/
#define LNR16_BUF_SIZE	(LNR8_BUF_SIZE*2)
/*16bitバッファを幾つ持つか*/
#define LNR16_BUF_NUM	2

/*SPU転送バッファ(サウンドRAM)サイズ*/
//#define STR_CHANNEL_SIZE	(LNR16_BUF_SIZE*LNR16_BUF_NUM)
/*１割り込みの再生サイズ(48000KHz)*/
//#define LNR8_INT_SIZE		0x200	//Int =5mSEC
#define LNR8_INT_SIZE		0x100	//Int.=2.5mSEC
/*========================================================================*/
extern int	id_SdEELoad;
extern int gBP_SdEELoadThreadWakeupCount;

#ifdef BP_360
int gBP_MTAChannel = -1;//lnr8 streams not allowed on X360.
#else
int gBP_MTAChannel = 1; //lnr8 streams are always channel 1.
#endif

extern unsigned long se_pant[0x41];
//EEからの読み込み関数
extern int EEOpen (unsigned int sd_code);
extern int EERead (int fd, unsigned char *ptr, int idx, int len);
/*----- EE読み込みアドレス -----*/
extern struct	EE_ADDR ee_addr[2];
extern int		str2_wait_fg[2];	//再生開始待ち
/*----- sd_set完了待ち -----*/
extern int		str2_iop_load_set[2];

/*----- 外部参照あり -----*/
unsigned int	lnr8_status;
unsigned int	lnr8_load_code;
unsigned int	lnr8_counter;
unsigned int	lnr8_stop_fg;
//int			lnr8_wait_fg;	//再生開始待ち
unsigned int	lnr8_first_load;	//最初のデータロード中
int			lnr8_read_disable;
int			lnr8_fp;
unsigned int	lnr8_play_counter;

unsigned int	str3_pause_fg;

/*----- ストリーミング読み込み -----*/
unsigned int	lnr8_wave_size, lnr8_unload_size, lnr8_unplay_size;
static unsigned int bp_lnr8_play_size;
extern unsigned char	eeload2_buf[2][2][EE_TRANS_SIZE]	//BP_PS2 __attribute__((aligned(4)))
;//VAGストリーミングと共用
//unsigned char	lnr8_buf[2][LNR8_LOAD_SIZE]	__attribute__((aligned(4)));
static unsigned char	*lnr8_trans_buf;
int				lnr8_read_status[LNR8_BUF_NUM];
unsigned int	lnr8_read_idx;
unsigned int	lnr8_play_idx;
static unsigned int	lnr8_play_ofst;
unsigned int	lnr8_mute_fg;

unsigned int	lnr8_off_ctr;

/*----- ストリーミング再生 -----*/
signed short	lnr16_buf[2][LNR16_BUF_SIZE/2]	//BP_PS2 __attribute__((aligned(8)))
;

#define BP_LPCM_PLAYBACK_BUF_SIZE (LNR8_BUF_SIZE * LNR8_BUF_NUM)

unsigned int	spu_lnr16_idx;
unsigned int	lnr16_next_ofst;

unsigned short	lnr_volume;				//音量
unsigned int	lnr_keyoffs;

int ac3_fg;
int ac3_clr;
//void			*ee_lnr_load_addr;

static void lnr_trans_init(void);
static void BP_DecodeFromEE();
static void lnr_trans(signed short *ptr16, const signed char *ptr8, unsigned int sz);

/*-------------------------------------------------------------------*/
/*  ストリーミング・キーOFF                                          */
/*-------------------------------------------------------------------*/
static
void lnr_tr_off(void)
{
	lnr_keyoffs = 1;
}

/*-------------------------------------------------------------------*/
/*  SPU2レジスタ・キュー書き込み                                     */
/*-------------------------------------------------------------------*/
void lnr_spuwr(void) {
	if (lnr_keyoffs) {
#if 1 //BP_PS2
      BP_StopDirectOutputStream(gBP_MTAChannel);
      BP_SetDirectOutputStreamVolume_Direct( gBP_MTAChannel,0, 0.f );
#else
		sceSdBlockTrans( LNR_DMA_CH, SD_TRANS_MODE_STOP, 0, 0 );
		BP_sceSdSetParam( LNR_CORE|SD_P_BVOLL , 0 ) ;	//サウンドデータ入力ボリューム設定
		BP_sceSdSetParam( LNR_CORE|SD_P_BVOLR , 0 ) ;
#endif
		lnr_keyoffs = 0;
///*DEBUG*/PRINTF(("\n...Streaming Stoped.(lnr8_counter=%x)\n", lnr8_counter[1]));
	}
}
void ac3_spuwr(void) {

	if (lnr_keyoffs) {
#if 1 //BP_PS2
      BP_TODO_BREAK;
#else
		sceSdBlockTrans( AC3_DMA_CH, SD_TRANS_MODE_STOP, 0, 0 );
		sceSdSetCoreAttr( SD_C_SPDIF_MODE, // これをすると次のAC3再生でノイズが出る
		SD_SPDIF_MEDIA_DVD | SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_PROHIBIT );
#endif
		lnr_keyoffs = 0;
	}
}

/*========================================================================
*   START STR STREAM
*   ファイルの最初から再生
*-------------------------------------------------------------------------
*   OUT: = 0  : Complete
*        Else : Error
=========================================================================*/
int lnr8_err_fg = 0;
#if 0
void vag_check(void)
{
	char *p = &eeload2_buf[1][lnr8_read_idx][0];
	int i;
	int ctr = 0;

	for (i=0; i<LNR8_LOAD_SIZE; i += 0x10) {
		if ((p[1]==2 ) || (p[1]==1)) ctr++;
	}
	if (ctr == LNR8_LOAD_SIZE/0x10) {
		lnr8_err_fg = 1;
		PRINTF(("ERROR(LNR8):This Voice File is VAG.\n"));
	}
}
#endif

int StartLnrEEStream(void)
{
	int				read_size, j;

#if 1 //BP
   //Because we perform decoding at EERead time, need to initialize this earlier than they did on PS2.
   lnr_trans_init();
#endif
	lnr8_read_disable = 0;
/*----- 前ストリームのクローズ -----*/
	lnr_tr_off();	/*STR ENV Off*/
/*----- ストリームのオープン -----*/
	if ((lnr8_fp = EEOpen(lnr8_load_code)) <0) {
		PRINTF(("StartLnrEEStream:File Open Error(%x)\n", lnr8_load_code));
		lnr8_fp = lnr8_first_load = lnr8_load_code = 0;
		str2_iop_load_set[1] = 0;//sd_set完了待ちフラグ、str2と共用
		return(-1);
	}
/*----- 各種情報設定 -----*/
	lnr8_wave_size = lnr8_unplay_size = lnr8_unload_size = ee_addr[1].sz;
	lnr_volume = (unsigned short)0x7F;
/*----- 最初のデータ読み込み -----*/
	lnr8_read_idx = 0;
	read_size = EERead(lnr8_fp, &eeload2_buf[1][lnr8_read_idx][0], lnr8_read_idx, LNR8_LOAD_SIZE);
   BP_DecodeFromEE( LNR8_LOAD_SIZE );

///*DEBUG*/	vag_check();
	for (j=0;j<(LNR8_BUF_NUM/2);j++) lnr8_read_status[(lnr8_read_idx*(LNR8_BUF_NUM/2))+j] = 1;
	lnr8_read_idx++;
	if (lnr8_unload_size > read_size) lnr8_unload_size -= read_size;
	else lnr8_unload_size = 0;
	for (j=0;j<(LNR8_BUF_NUM/2);j++) lnr8_read_status[(lnr8_read_idx*(LNR8_BUF_NUM/2))+j] = 0;

/*----- ワークエリア設定 -----*/
	if (ac3_fg) {
//		BP_sceSdSetParam( AC3_CORE|SD_P_BVOLL , 0 ) ;	//サウンドデータ入力ボリューム設定
//		BP_sceSdSetParam( AC3_CORE|SD_P_BVOLR , 0 ) ;
		ac3_clr = 1;
PRINTF(("...StrAC-3 Start.\n"));
	} else {
PRINTF(("...StrLNR8 Start.\n"));
	}
	lnr8_trans_buf = &eeload2_buf[1][0][0];
//	str3_pause_fg = 0; //2000/08/22
	return(0);
}


/*========================================================================
*	ストリーミングＣＤ読み込み
=========================================================================*/

static void BP_DecodeFromEE( int decodeSize )
{
   //BP - immediately decode the copied lpcm8 data into our large(r) playback buffer
   //after a read.  We bypass the tiny lnr16_buf they used on ps2.
   int j,k;
   signed short decodeBuf[LNR8_BUF_SIZE];
   //Both channels are double buffered.  lnr_8_read_idx (0 or 1) tells us whether to
   //take the first or second half of the buffer.  Divide by sizeof(short) to convert
   //to short index rather than byte offset.
   int bufIndex = lnr8_read_idx*BP_LPCM_PLAYBACK_BUF_SIZE/sizeof(short);
   short* baseDstBuffer = (short*)BP_GetDirectOutputStreamPlaybackBuffer(gBP_MTAChannel);
   short* dstBuffer = baseDstBuffer + bufIndex;

   memset( dstBuffer, 0, LNR8_LOAD_SIZE * 2 );
   for(j=0;j<decodeSize / LNR8_BUF_SIZE;++j)
   {
      const char * decodeSrc = &eeload2_buf[1][lnr8_read_idx][j*LNR8_BUF_SIZE];
      lnr_trans( decodeBuf, decodeSrc, LNR8_BUF_SIZE );

      //Two consecutive packets of:
      //First 512 bytes = 256 left channel samples
      //Second 512 bytes = 256 right channel samples
      //Reinterleave into 1 sample left, 1 sample right...
      
      for( k=0; k < 0x200; ++k )
      {
         int src_l = (k&0xff)+((k&0x100)<<1);
         int src_r = src_l + 0x100;
         
         *dstBuffer++ = decodeBuf[src_l];
         *dstBuffer++ = decodeBuf[src_r];
      }
   }
   if( (unsigned char*)dstBuffer > (unsigned char*)baseDstBuffer+BP_LPCM_PLAYBACK_BUF_SIZE*2 )
      BP_BREAK;

//   printf("BP_DecodeFromEE %d: %d\n", lnr8_read_idx, (unsigned char*)baseDstBuffer+BP_LPCM_PLAYBACK_BUF_SIZE*2 - (unsigned char*)dstBuffer );
}

static void BP_Trans0( int index )
{
   int bufIndex = index*LNR8_BUF_SIZE;
   short* baseDstBuffer = (short*)BP_GetDirectOutputStreamPlaybackBuffer(gBP_MTAChannel);
   short* dstBuffer = baseDstBuffer + bufIndex;
   memset( dstBuffer, 0, LNR8_BUF_SIZE * sizeof( short ) );

   if( (unsigned char*)dstBuffer + LNR8_BUF_SIZE * sizeof( short ) > (unsigned char*)baseDstBuffer+BP_LPCM_PLAYBACK_BUF_SIZE*2 )
      BP_BREAK;
//   printf("BP_Trans0 %d: %d\n", index, (unsigned char*)baseDstBuffer+BP_LPCM_PLAYBACK_BUF_SIZE*2 - ((unsigned char*)dstBuffer + LNR8_BUF_SIZE * sizeof( short )) );
}

static void LnrEELoad(void)
{
	int				read_size, i, j, status;

//	if ((lnr8_status < 4) || (lnr8_status > 6)) return;
	for  (i=0; i<2; i++) {
		if (lnr8_unload_size) {
			status = 0;
			for (j=0;j<(LNR8_BUF_NUM/2);j++) {
				status |= lnr8_read_status[(lnr8_read_idx*(LNR8_BUF_NUM/2))+j];//0-0x0F/0x10-0x1F
			}
			if (status == 0) {
#if 0 //BP_PS2
/*DEBUG*/WaitVblankStart();	//EEからのロードでも何故か必要
/*DEBUG*/WaitVblankEnd();	//<---これもないと不安定
#endif
				if (lnr8_unload_size > LNR8_LOAD_SIZE) {
					read_size = EERead (lnr8_fp, &eeload2_buf[1][lnr8_read_idx][0], lnr8_read_idx, LNR8_LOAD_SIZE);
					if (read_size) {
                  BP_DecodeFromEE( LNR8_LOAD_SIZE );
						for (j=0;j<(LNR8_BUF_NUM/2);j++) lnr8_read_status[(lnr8_read_idx*(LNR8_BUF_NUM/2))+j] = 1;
						lnr8_read_idx = (lnr8_read_idx+1) & 1;
						lnr8_unload_size -= LNR8_LOAD_SIZE;
///*DEBUG*/PRINTF(("UnloadSize=%x\n", lnr8_unload_size ));
					}
				} else {
					read_size = EERead (lnr8_fp, &eeload2_buf[1][lnr8_read_idx][0], lnr8_read_idx, LNR8_LOAD_SIZE);//端数は切り捨てられるので最後まで読む
					if (read_size) {
                  BP_DecodeFromEE( lnr8_unload_size );
						for (j=0;j<(LNR8_BUF_NUM/2);j++) lnr8_read_status[(lnr8_read_idx*(LNR8_BUF_NUM/2))+j] = 1;
						lnr8_read_idx = (lnr8_read_idx+1) & 1;
						lnr8_unload_size = 0;
					}
///*DEBUG*/PRINTF(("Str2:LoadEnd=%x\n", lnr8_unload_size ));
					break;
				}
			}
		}
	}
}
void lnr_load(void)
{
	switch (lnr8_status) {
	case 1:
/*if (lnr8_load_code[1]) PRINTF(("Code=%x\n", lnr8_load_code[1]));*/
//		if ((ee_addr[1].set_ctr >= 2)
//			|| (ee_addr[1].set_ctr && (ee_addr[1].sz <= LNR8_LOAD_SIZE) )) { //バッファが埋ってからスタート
		if (ee_addr[1].set_ctr >= 1) {
			if (StartLnrEEStream() != 0) lnr8_status = 0;
			else {
				lnr8_status = 2;
			}
		}
/*DEBUG*/ else PRINTF(("WaitForLNR8DATA(status=1 ctr=%x)\n", ee_addr[1].set_ctr));
		break;
	case 2:
		if (ee_addr[1].set_ctr >= 2) {
			LnrEELoad();
			if (ac3_clr && ac3_fg) {		//2001/07/03
PRINTF(("Wait for ac3_clr = 0\n"));
			} else {
				lnr8_status++;
			}
		}
/*DEBUG*/ else PRINTF(("WaitForLNR8DATA(status=2 ctr=%x)\n", ee_addr[1].set_ctr));
	case 3:
	case 4:
	case 5:
	case 6:
		LnrEELoad();
		break;
	case 7:
		break;
	case 8:
//		PcmClose(lnr8_fp[1]);
		ee_addr[1].set_ctr = 0;
		ee_addr[1].read_ctr = 0;
		lnr8_fp = lnr8_status = lnr8_load_code = 0;
		str2_iop_load_set[1] = 0;	//2001/09/09 ロード残りがあれば、捨てる
		PRINTF(("***StrLNR8 Terminate***\n"));
		break;
	}
}

/*========================================================================
*   TRANSFER BGM DATA TO SPU
=========================================================================*/
signed short lnr_val[2];
static void lnr_trans_init(void)
{
	lnr_val[0] = lnr_val[1] = 0;
}
void lnr_trans_0(signed short *ptr16,unsigned int sz)
{
	int				i;
	for (i=0; i<sz; i++) {
		*(ptr16+i) = 0;
	}
}

static inline signed short dec_16(unsigned char val8)
{
	signed short	val16;
	int		shift;

	shift = val8 & 7;					//スケール(b2-b0)を取り出す
	val16 = val8 & 0x78;				//差分(b6-b3)を取り出し、val16に代入

	if (shift != 7) {
		val16 |= 0x80;					//スケールが7以外の時、差分の最上位bitをON(=1)に戻す
		val16 = (val16 << 7) >> shift;	//差分を上位に揃え、そこからスケールに合わせて右シフトする
	} else {							//スケールが7の時
		val16 = (val16 << 8) >> shift;	//差分を上位に揃え、そこからスケールに合わせて右シフトする
	}
	if (val8 & 0x80) val16 = -val16;	//符号がマイナスの時、2の補数をとる

	return (val16);
}

#if 0
extern unsigned int	str2_status[2];
void vag_check(signed char *p, unsigned int sz)
{
	int i;
	int ctr = 0;

	for (i=0; i<sz; i += 0x10) {
		if ((p[1]==2 ) || (p[1]==1)) ctr++;
	}
	if (ctr == sz/0x10) {
		lnr8_err_fg = 1;
		PRINTF(("ERROR(LNR8):This Voice File is VAG.status=%x/%x/%x\n", str2_status[0], str2_status[1], lnr8_status));
	}
//	else PRINTF(("OK(LNR8):This Voice File is LNR8.status=%x\n", lnr8_status));
}
#endif

static void lnr_trans(signed short *ptr16, const signed char *ptr8, unsigned int sz)
{
	int				i, j;
	signed short	val_l, val_r;
	signed long		val_m;

///*DEBUG*/PRINTF(("sz=%x\n", sz ));

///*DEBUG*/ vag_check(ptr8, sz);

   if( sz != LNR8_BUF_SIZE )
      BP_BREAK;

	if (sound_mono_fg) {
/*MONO Mix*/
		val_l = lnr_val[0];
		val_r = lnr_val[1];
		for (j=0; j<sz; j+=0x200) {
			for (i=0; i<0x100; i++) {
				val_l = dec_16(*ptr8) + val_l;
				ptr8++;
				val_r = dec_16(*ptr8) + val_r;
				ptr8++;
				val_m = (signed long)val_l + (signed long)val_r;
				*(ptr16+i+j) = *(ptr16+i+j+0x100) = (signed short)(val_m / 2);
			}
			lnr_val[0] = val_l;
			lnr_val[1] = val_r;
		}
	} else {
/*STEREO*/
		val_l = lnr_val[0];
		val_r = lnr_val[1];
		for (j=0; j<sz; j+=0x200) {
			for (i=0; i<0x100; i++) {
//				val_l = *(ptr16+i+j) = (((signed short)(((*ptr8) & 0xF8)<<8)) >> ((*ptr8) & 7)) + val_l;
				val_l = *(ptr16+i+j) = dec_16(*ptr8) + val_l;
				ptr8++;
//				val_r = *(ptr16+i+j+0x100) = (((signed short)(((*ptr8) & 0xF8)<<8)) >> ((*ptr8) & 7)) + val_r;
				val_r = *(ptr16+i+j+0x100) = dec_16(*ptr8) + val_r;
				ptr8++;
			}
			lnr_val[0] = val_l;
			lnr_val[1] = val_r;
		}
	}
}

#define LNR8_FADE_CNT	0x02
#define LNR8_FADE_INIT	0x01
unsigned int	lnr8_fade_vol;

int				lnr8_clock_offset;
static STR_TIME lnr8_play_counter_work;

static void bp_set_lnr8_paused( int paused )
{
   if( lnr8_mute_fg != paused)
   {
      lnr8_mute_fg = paused;
      BP_SetDirectOutputStreamPaused( gBP_MTAChannel, paused );
   }
}

//BP - return 1 if we need to wake up SdEELoad thread, causing lnr_load to be called once.
static int lnrSpuTrans(void)
{
	int		dma_fg = 0;
	int		end_fg = 0;
	int		read_wait = 0;

	if ((lnr8_stop_fg) && (lnr8_status >= 3 ) ) {	//ストリーミング停止要求
		lnr8_status = 8;
		lnr_tr_off();
		lnr8_stop_fg = 0;
PRINTF(("StrLNR8 Stopped.\n"));
	}
//if (lnr8_status) PRINTF(("status=%x\n", lnr8_status));
/*--------------------- Transfer BGM Data -------------------------------*/
	switch (lnr8_status) {
	case 2:
//PRINTF(("SizeOFlnr16_buf=%x\n", sizeof(lnr16_buf)));
		dma_fg = 1;
		break;
	case 3:
		lnr8_play_idx = 0;	//eeload2_buf[1]を0x20分割したブロックのidx
		lnr8_play_ofst = 0;	//eeload2_buf[1]オフセットカウンタ
#if 1 //BP_PS2
      //Don't manually step this stuff here.  We've already decoded a whole
      //playback buffer's worth of data.
#else
		lnr_trans_init();
		lnr_trans(&lnr16_buf[0][0], lnr8_trans_buf, LNR8_BUF_SIZE);//8->16bit変換
		lnr8_play_ofst = LNR8_BUF_SIZE;	//eeload2_buf[1]オフセットカウンタ更新
		lnr8_unplay_size -= LNR8_BUF_SIZE;	//8bitデータ残り再生サイズ更新
		lnr8_read_status[lnr8_play_idx] = 0;//8bitデータ読み込み済フラグをクリア
		lnr8_play_idx++;					//int8_bufブロックidx更新
		lnr8_counter += LNR8_BUF_SIZE/2;		//EEへ返す「転送済カウンタ」更新
#endif
		++lnr8_status;
		dma_fg = 1;
		break;
	case 4:
		if ( (lnr8_unplay_size == 0) || (lnr8_unplay_size&0x80000000) ) ++lnr8_status;
#if 1 //BP_PS2
      //Don't manually step this stuff here.  We've already decoded a whole
      //playback buffer's worth of data.
#else
		lnr_trans(&lnr16_buf[1][0], lnr8_trans_buf+lnr8_play_ofst, LNR8_BUF_SIZE);//8->16bit変換
		lnr8_play_ofst += LNR8_BUF_SIZE;
		lnr8_unplay_size -= LNR8_BUF_SIZE;
		lnr8_read_status[lnr8_play_idx] = 0;
		lnr8_play_idx++;
		lnr8_counter += LNR8_BUF_SIZE/2;
#endif
		++lnr8_status;
		dma_fg = 1;
		break;
	case 5:
		if (lnr8_first_load) lnr8_first_load = 0;
		if (str2_wait_fg[1]) break;	//再生開始待ちはstr2と共用する
		if (sng_pause_fg || str3_pause_fg) break;	//2001/01/18 K.Muraoka str2と共通
		lnr8_fade_vol = LNR8_FADE_INIT;
#if 1 //BP_PS2
      {
         DirectOutputStreamInitData soundInit = { 0 };
         soundInit.channel = gBP_MTAChannel;
         soundInit.trackCount = 1;
         soundInit.channelCount = 2;
         soundInit.totalStreamSize = lnr8_wave_size * sizeof(short);
         soundInit.playbackBufferSize = BP_LPCM_PLAYBACK_BUF_SIZE*2;
         soundInit.freq = 48000;
         soundInit.totalStreamSamples = 0;
         soundInit.format = BP_DOSTREAM_FORMAT_PCM;
         soundInit.streamType = BP_DOSTREAM_TYPE_DIRECT;

#ifdef BP_X360
         //The buffers necessary for this format's playback don't exist on X360 anymore!
         //All these streams should have been replaced with X360 format data.
         BP_BREAK;
#else
         //Only X360 cares about streamTop; it uses it to identify streams by name to decide
         //whether they're music or not for XMP cooperation support.  No point doing the work
         //to pass along the streamTop from the game to here for any other platform.
         soundInit.streamTop = 0;
#endif
         BP_InitDirectOutputStream(&soundInit);
         bp_lnr8_play_size = lnr8_play_counter = lnr8_play_idx = 0;
         BP_ContinuePlayDirectOutputStream(gBP_MTAChannel);
         BP_SetDirectOutputStreamVolume_Direct( gBP_MTAChannel, 0, (float)lnr8_fade_vol/LNR8_FADE_CNT );
      }
#else
		BP_sceSdSetParam( LNR_CORE|SD_P_BVOLL , 0x7fff*lnr8_fade_vol/LNR8_FADE_CNT ) ;	//サウンドデータ入力ボリューム設定
		BP_sceSdSetParam( LNR_CORE|SD_P_BVOLR , 0x7fff*lnr8_fade_vol/LNR8_FADE_CNT ) ;
		sceSdBlockTrans(LNR_DMA_CH, SD_TRANS_MODE_WRITE | SD_BLOCK_LOOP, (u_char *)&lnr16_buf, sizeof(lnr16_buf));
#endif
      lnr8_fade_vol++;

		lnr8_clock_offset = 0;
		reset_play_counter( &lnr8_play_counter_work, lnr8_clock_offset );	// Add by K.Uehara

		lnr16_next_ofst = LNR16_BUF_SIZE;
		++lnr8_status;
		if ( (lnr8_unplay_size == 0) || (lnr8_unplay_size&0x80000000) ) {
PRINTF(("lnrSpuTrans:status=4 --> 6(%x)\n", lnr8_unplay_size));
			lnr8_off_ctr = (lnr8_wave_size / LNR8_INT_SIZE);
			lnr8_mute_fg = 0;
			++lnr8_status;
		}
		break;
	case 6:
#if 1 //BP_PS2
      //stream playback state completely replaced.
      if (lnr8_fade_vol <= LNR8_FADE_CNT) {
         BP_SetDirectOutputStreamVolume_Direct( gBP_MTAChannel, 0, (float)lnr8_fade_vol/LNR8_FADE_CNT );
         lnr8_fade_vol++;
      }
      //Get stream position and divide by two to convert to per-stereo-channel units.
      spu_lnr16_idx = BP_GetDirectOutputStreamPosition(gBP_MTAChannel) / 2;
      {
         //See how many read packets we need to knock off.
         int bp_lnr8_unread_idx = spu_lnr16_idx / BP_LPCM_PLAYBACK_BUF_SIZE;
         int new_lnr8_play_idx = spu_lnr16_idx / LNR8_BUF_SIZE;
         //handle pause flags
         if(sng_pause_fg || str3_pause_fg)
         {
            bp_set_lnr8_paused( 1 );
            break;
         }

         if( lnr8_read_status[lnr8_play_idx] == 0 )
         {
            //We've underflowed.  Pause the stream.
            bp_set_lnr8_paused( 1 );
         }
         else
         {
            //still playing back from the alternate buffer.
            bp_set_lnr8_paused( 0 );
         }

         while( lnr8_play_idx != new_lnr8_play_idx )
         {
            //Mark this packet as consumed.
            lnr8_read_status[lnr8_play_idx] = 0;

            //Zero out the memory just played; it could be possible to overread the area of valid data during fadeout
            //and they handled this in state 7 using lnr_trans_0().
            BP_Trans0( lnr8_play_idx );

            if (lnr8_unplay_size > LNR8_BUF_SIZE) {
               lnr8_unplay_size -= LNR8_BUF_SIZE;
               ///*DEBUG*/PRINTF(("%x:%x ", lnr8_unplay_size, lnr8_counter[1]));
               ///*DEBUG*/PRINTF(("lnr8_counter=%x\n", lnr8_counter[1]));
               //update lnr8_play_counter: time into playback, measured in 1/200ths of a second
               bp_lnr8_play_size += LNR8_BUF_SIZE;
               //stereo, lpcm16.  Convert from bytes->samples->seconds->ticks.
               lnr8_play_counter = bp_lnr8_play_size / sizeof(short) / (48000/200);
//               printf("DG %d, lnr8 %d\n", DG_TickCount, lnr8_play_counter );

            } else {
               //start fadeout.
               lnr8_off_ctr = ((LNR8_BUF_SIZE*LNR8_BUF_NUM) / LNR8_INT_SIZE);
               ///*DEBUG*/PRINTF(("...Streaming Stoped.(lnr8_counter=%x)\n", lnr8_counter[1]));
               lnr8_status = 7; //fadeout state.
            }

            lnr8_counter += LNR8_BUF_SIZE/2;

            //Next packet.
            ++lnr8_play_idx;

            // This fixes the case where (new_lnr8_play_idx == LNR8_BUF_NUM) as is the case with
            // MGSTWO-754: PS3: MGS2: 4160 NA: (webdemo) Shell-2-Core 1F-Air Purification Room; The game crashes immediately after the scene where President Johnson dies.
            if( lnr8_play_idx == new_lnr8_play_idx)
               break;

            if( lnr8_play_idx == LNR8_BUF_NUM )
            {
               lnr8_play_idx = 0;
            }
         }
         dma_fg = 1;
      }
#else
		if (lnr8_fade_vol <= LNR8_FADE_CNT) {
			BP_sceSdSetParam( LNR_CORE|SD_P_BVOLL , 0x7fff*lnr8_fade_vol/LNR8_FADE_CNT ) ;	//サウンドデータ入力ボリューム設定
			BP_sceSdSetParam( LNR_CORE|SD_P_BVOLR , 0x7fff*lnr8_fade_vol/LNR8_FADE_CNT ) ;
			lnr8_fade_vol++;
		}
		spu_lnr16_idx = sceSdBlockTransStatus(LNR_DMA_CH, 0);
		spu_lnr16_idx = (spu_lnr16_idx & 0xFFFFFF) - (u_int)&lnr16_buf;
///*DEBUG*/PRINTF(("lnr16idx=%x : lnr16ofst=%x\n", spu_lnr16_idx , lnr16_next_ofst));
/*DEBUG:再生中アドレスを表示*/
		if (( spu_lnr16_idx >= LNR16_BUF_SIZE*2 )||( spu_lnr16_idx == 0x80000000 )) {
			PRINTF(("ERROR:LNR8StreamingAddress(status=6 %x)\n", spu_lnr16_idx));
			break;
		}
		if (lnr8_mute_fg == 0) {
//			lnr8_play_counter++;
			lnr8_play_counter = get_play_counter( &lnr8_play_counter_work );
		}
		if ( lnr16_next_ofst==(spu_lnr16_idx&LNR16_BUF_SIZE) ) {
			if (sng_pause_fg || str3_pause_fg) goto Lnr8Pause0;	//2001/01/18 K.Muraoka str2と共通
			dma_fg = 1;
			if ( lnr8_read_status[lnr8_play_idx] != 0 ) {		//EEからのデータがある時
/*-----------------------------------------------------------------------*/
				if (lnr8_mute_fg != 0){
					reset_play_counter( &lnr8_play_counter_work, lnr8_clock_offset+((LNR8_BUF_SIZE/2)/(48000/200)) );	// Add by K.Uehara
					lnr8_clock_offset = 0;
					lnr8_mute_fg = 0;
///*DEBUG*/PRINTF(("lnr8_play_counter=%x\n", lnr8_play_counter));
				}
				lnr_trans(&lnr16_buf[(~(spu_lnr16_idx>>11))&1][0], lnr8_trans_buf+lnr8_play_ofst, LNR8_BUF_SIZE);//8->16bit変換

				lnr8_play_ofst += LNR8_BUF_SIZE;
				if (lnr8_play_ofst == (LNR8_BUF_SIZE*LNR8_BUF_NUM)) lnr8_play_ofst = 0;

				if (lnr8_unplay_size > LNR8_BUF_SIZE) {
					lnr8_unplay_size -= LNR8_BUF_SIZE;
///*DEBUG*/PRINTF(("%x:%x ", lnr8_unplay_size, lnr8_counter[1]));
///*DEBUG*/PRINTF(("lnr8_counter=%x\n", lnr8_counter[1]));
				} else {
					lnr8_off_ctr = ((LNR8_BUF_SIZE*LNR8_BUF_NUM) / LNR8_INT_SIZE);
					lnr8_play_ofst = 0;
///*DEBUG*/PRINTF(("...Streaming Stoped.(lnr8_counter=%x)\n", lnr8_counter[1]));
					++lnr8_status;
				}

				lnr8_read_status[lnr8_play_idx] = 0;
				lnr8_play_idx++;
				if (lnr8_play_idx == LNR8_BUF_NUM) lnr8_play_idx = 0;

				lnr8_counter += LNR8_BUF_SIZE/2;

				lnr16_next_ofst = (lnr16_next_ofst+LNR16_BUF_SIZE)&((LNR16_BUF_SIZE*LNR16_BUF_NUM)-1);
				
			} else {
/*--------------------- Mute Volume While Retry CD Read -----------------*/
				read_wait = 1;	//2001/09/20
/*DEBUG*/PRINTF(("EE Lnr8 READ Retry\n"));
Lnr8Pause0:
				if (lnr8_mute_fg == 0) {
					lnr8_clock_offset = get_play_counter( &lnr8_play_counter_work );
					lnr8_mute_fg = 1;
				}
				lnr_trans_0(&lnr16_buf[(~(spu_lnr16_idx>>11))&1][0], LNR8_BUF_SIZE);//16bit無音セット
				lnr16_next_ofst = (lnr16_next_ofst+LNR16_BUF_SIZE)&((LNR16_BUF_SIZE*LNR16_BUF_NUM)-1);

//2001/09/20 unload_sizeが0の時、リード待ちでここに来るのはエラーなので、プレイカウンタを進める
				if ((lnr8_unload_size == 0) && (read_wait)) {
					lnr8_off_ctr = ((LNR8_BUF_SIZE*LNR8_BUF_NUM) / LNR8_INT_SIZE);
					lnr8_play_ofst = 0;
					lnr8_counter += LNR8_INT_SIZE;
					lnr8_play_counter = lnr8_clock_offset;
/*DEBUG*/PRINTF(("...Streaming Stoped(read wait error).(lnr8_counter=%x)\n", lnr8_counter));
					++lnr8_status;
				}
//2001/09/20 ここまで

			}
		}
#endif //BP_PS2
		break;
	case 7:
#if 1 //BP_PS2
      if ((lnr8_off_ctr&0x80000000)) {
         lnr_tr_off();	//一つ前のステータスの完了処理で行なうべき？
         ++lnr8_status;
      }
#else
		spu_lnr16_idx = sceSdBlockTransStatus(LNR_DMA_CH, 0);
		spu_lnr16_idx = (spu_lnr16_idx & 0xFFFFFF) - (u_int)(&lnr16_buf);
		if ( (spu_lnr16_idx >= LNR16_BUF_SIZE*2) || (spu_lnr16_idx&0x80000000) ) {
			PRINTF(("ERROR:LNR8StreamingAddress(status=7 idx=%x)\n", spu_lnr16_idx));
			lnr_tr_off();	//一つ前のステータスの完了処理で行なうべき？
			++lnr8_status;
			break;
		}
		if ( lnr16_next_ofst==(spu_lnr16_idx&LNR16_BUF_SIZE) ) {
			lnr_trans_0(&lnr16_buf[(~(spu_lnr16_idx>>11))&1][0], LNR8_BUF_SIZE);//16bit無音セット
			lnr16_next_ofst = (lnr16_next_ofst+LNR16_BUF_SIZE)&((LNR16_BUF_SIZE*LNR16_BUF_NUM)-1);
		}
		lnr8_counter += LNR8_INT_SIZE;
//		lnr8_play_counter++;
		lnr8_play_counter = get_play_counter( &lnr8_play_counter_work );
//		if ((lnr8_off_ctr-- == 0xFFFFFFFF)) { /*2001/09/04*/

		if ((lnr8_off_ctr&0x80000000)) {
			lnr_tr_off();	//一つ前のステータスの完了処理で行なうべき？
			++lnr8_status;
		}
#endif
		lnr8_off_ctr--;
		break;
	case 8:
		lnr8_counter += LNR8_INT_SIZE;
		end_fg = 1;
		break;
	}
	return (dma_fg | end_fg);
}


/*========================================================================
*   TRANSFER BGM DATA TO SPU(AC-3)
=========================================================================*/

unsigned int	ac3_idx;	//AC-3データは0x800転送の後、ダミー転送を0x800*2する

int ac3_trans(signed short *ptr16, signed char *ptr8, unsigned int sz)
{
	int		ret;

	if ((ac3_idx%6) < 2) {
		memcpy(ptr16, ptr8, 0x200); //Rチャンネルは使わない
		memcpy(((unsigned char *)ptr16)+0x400, ptr8+0x200, 0x200); //Rチャンネルは使わない
		ret = 1;
	} else {
		memset(ptr16, '\0', 0x200);
		memset(((unsigned char *)ptr16)+0x400, '\0', 0x200);
		ret = 0;
	}
	ac3_idx++;
	return(ret);
}

char	ac3_0_buf[0x1000];

int ac3SpuTrans(void)
{
#if 1 //BP_PS2
   BP_TODO_BREAK;
   return 0;
#else
	int		dma_fg = 0;
	int		end_fg = 0;

	if ((lnr8_stop_fg) && (lnr8_status >= 3 ) ) {	//ストリーミング停止要求
		lnr8_status = 8;
		lnr_tr_off();
		lnr8_stop_fg = 0;
PRINTF(("StrLNR8 Stopped.\n"));
	}
/*--------------------- Transfer BGM Data -------------------------------*/
	switch (lnr8_status) {
	case 2:
		if (ac3_clr) {

			BP_sceSdSetParam( AC3_CORE|SD_P_BVOLL , 0 ) ;	//サウンドデータ入力ボリューム設定
			BP_sceSdSetParam( AC3_CORE|SD_P_BVOLR , 0 ) ;

			/*0x4000-0x4fff: Core0 MEMIN*/
			sceSdVoiceTrans(SD_CORE_0, (SD_TRANS_MODE_WRITE | SD_TRANS_BY_DMA), ac3_0_buf, 0x4000, 0x1000);
			BP_DelayThread(100*1000);	//0.1Sec

			sceSdSetCoreAttr( SD_C_SPDIF_MODE,
			SD_SPDIF_MEDIA_DVD | SD_SPDIF_OUT_BITSTREAM | SD_SPDIF_COPY_PROHIBIT );

			ac3_clr = 0;
		}
		dma_fg = 1;
		break;
	case 3:
		lnr8_play_idx = 0;	//eeload2_buf[1]を0x20分割したブロックのidx
		lnr8_play_ofst = 0;	//eeload2_buf[1]オフセットカウンタ
		ac3_idx = 0;
		ac3_trans(&lnr16_buf[0][0], lnr8_trans_buf, LNR8_BUF_SIZE);//8->16bit変換
		lnr8_play_ofst = LNR8_BUF_SIZE;	//eeload2_buf[1]オフセットカウンタ更新
		lnr8_unplay_size -= LNR8_BUF_SIZE;	//8bitデータ残り再生サイズ更新
		lnr8_read_status[lnr8_play_idx] = 0;//8bitデータ読み込み済フラグをクリア
		lnr8_play_idx++;					//int8_bufブロックidx更新
		lnr8_counter += LNR8_BUF_SIZE/2;		//EEへ返す「転送済カウンタ」更新
		++lnr8_status;
		dma_fg = 1;
		break;
	case 4:
		if ( (lnr8_unplay_size == 0) || (lnr8_unplay_size&0x80000000) ) ++lnr8_status;
		ac3_trans(&lnr16_buf[1][0], lnr8_trans_buf+lnr8_play_ofst, LNR8_BUF_SIZE);//8->16bit変換
		lnr8_play_ofst += LNR8_BUF_SIZE;
		lnr8_unplay_size -= LNR8_BUF_SIZE;
		lnr8_read_status[lnr8_play_idx] = 0;
		lnr8_play_idx++;
		lnr8_counter += LNR8_BUF_SIZE/2;
		++lnr8_status;
		dma_fg = 1;
		break;
	case 5:
		if (lnr8_first_load) lnr8_first_load = 0;
		if (str2_wait_fg[1]) break;	//再生開始待ちはstr2と共用する
		if (sng_pause_fg || str3_pause_fg) break;	//2001/01/18 K.Muraoka str2と共通
		sceSdBlockTrans(AC3_DMA_CH, SD_TRANS_MODE_WRITE | SD_BLOCK_LOOP, (u_char *)&lnr16_buf, sizeof(lnr16_buf));

		lnr8_clock_offset = 0;
		reset_play_counter( &lnr8_play_counter_work, lnr8_clock_offset );	// Add by K.Uehara

		lnr16_next_ofst = LNR16_BUF_SIZE;
		++lnr8_status;
		if ( (lnr8_unplay_size == 0) || (lnr8_unplay_size&0x80000000) ) {
PRINTF(("lnrSpuTrans:status=4 --> 6(%x)\n", lnr8_unplay_size));
			lnr8_off_ctr = (lnr8_wave_size / LNR8_INT_SIZE);
			lnr8_mute_fg = 0;
			++lnr8_status;
		}
		break;
	case 6:
		spu_lnr16_idx = sceSdBlockTransStatus(AC3_DMA_CH, 0);
		spu_lnr16_idx = (spu_lnr16_idx & 0xFFFFFF) - (u_int)&lnr16_buf;
		if (( spu_lnr16_idx >= LNR16_BUF_SIZE*2 )||( spu_lnr16_idx == 0x80000000 )) {
			PRINTF(("ERROR:LNR8StreamingAddress(status=6 %x)\n", spu_lnr16_idx));
			break;
		}
//		if (lnr8_mute_fg == 0) lnr8_play_counter++;
		if (lnr8_mute_fg == 0){
			lnr8_play_counter = get_play_counter( &lnr8_play_counter_work );
		}
		if ( lnr16_next_ofst==(spu_lnr16_idx&LNR16_BUF_SIZE) ) {
			if (sng_pause_fg || str3_pause_fg) goto Ac3Pause0;	//2001/01/18 K.Muraoka str2と共通
			dma_fg = 1;
			if ( lnr8_read_status[lnr8_play_idx] != 0 ) {		//EEからのデータがある時
/*-----------------------------------------------------------------------*/
				if (lnr8_mute_fg != 0){
					reset_play_counter( &lnr8_play_counter_work, lnr8_clock_offset+((LNR8_BUF_SIZE/2)/(48000/200)) );	// Add by K.Uehara
					lnr8_clock_offset = 0;
					lnr8_mute_fg = 0;
				}
				if ( ac3_trans(&lnr16_buf[(~(spu_lnr16_idx>>11))&1][0], lnr8_trans_buf+lnr8_play_ofst, LNR8_BUF_SIZE) ) {
					lnr8_play_ofst += LNR8_BUF_SIZE;
					if (lnr8_play_ofst == (LNR8_BUF_SIZE*LNR8_BUF_NUM)) lnr8_play_ofst = 0;
					if (lnr8_unplay_size > LNR8_BUF_SIZE) {
						lnr8_unplay_size -= LNR8_BUF_SIZE;
					} else {
						lnr8_off_ctr = ((LNR8_BUF_SIZE*LNR8_BUF_NUM) / LNR8_INT_SIZE);
						lnr8_play_ofst = 0;
						++lnr8_status;
					}
					lnr8_read_status[lnr8_play_idx] = 0;
					lnr8_play_idx++;
					if (lnr8_play_idx == LNR8_BUF_NUM) lnr8_play_idx = 0;
					lnr8_counter += LNR8_BUF_SIZE/2;
				}
				lnr16_next_ofst = (lnr16_next_ofst+LNR16_BUF_SIZE)&((LNR16_BUF_SIZE*LNR16_BUF_NUM)-1);
			} else {
/*--------------------- Mute Volume While Retry CD Read -----------------*/
/*DEBUG*/PRINTF(("EE Lnr8 READ Retry\n"));
Ac3Pause0:
				if (lnr8_mute_fg == 0) {
					lnr8_clock_offset = get_play_counter( &lnr8_play_counter_work );
					lnr8_mute_fg = 1;
				}
				lnr_trans_0(&lnr16_buf[(~(spu_lnr16_idx>>11))&1][0], LNR8_BUF_SIZE);//16bit無音セット
				lnr16_next_ofst = (lnr16_next_ofst+LNR16_BUF_SIZE)&((LNR16_BUF_SIZE*LNR16_BUF_NUM)-1);
			}
		}
		break;
	case 7:
		spu_lnr16_idx = sceSdBlockTransStatus(AC3_DMA_CH, 0);
		spu_lnr16_idx = (spu_lnr16_idx & 0xFFFFFF) - (u_int)(&lnr16_buf);
		if ( (spu_lnr16_idx >= LNR16_BUF_SIZE*2) || (spu_lnr16_idx&0x80000000) ) {
			PRINTF(("ERROR:LNR8StreamingAddress(status=7 idx=%x)\n", spu_lnr16_idx));
			lnr_tr_off();	//2001/10/24 AC-3停止
			++lnr8_status;
			break;
		}
		if ( lnr16_next_ofst==(spu_lnr16_idx&LNR16_BUF_SIZE) ) {
			lnr_trans_0(&lnr16_buf[(~(spu_lnr16_idx>>11))&1][0], LNR8_BUF_SIZE);//16bit無音セット
			lnr16_next_ofst = (lnr16_next_ofst+LNR16_BUF_SIZE)&((LNR16_BUF_SIZE*LNR16_BUF_NUM)-1);
		}
		lnr8_counter += LNR8_INT_SIZE;
//		lnr8_play_counter++;
		lnr8_play_counter = get_play_counter( &lnr8_play_counter_work );
//		if ((lnr8_off_ctr-- == 0xFFFFFFFF)) { /*2001/09/04*/

		if ((lnr8_off_ctr&0x80000000)) {
			lnr_tr_off();	//一つ前のステータスの完了処理で行なうべき？
			++lnr8_status;
		}
		lnr8_off_ctr--;
		break;
	case 8:
		lnr8_counter += LNR8_INT_SIZE;
		end_fg = 1;
		break;
	}
	return (dma_fg | end_fg);
#endif
}


void lnr_int(void)
{
	if (ac3_fg) {
		if ((BP_sceSdVoiceTransStatus(AC3_DMA_CH, SD_TRANS_STATUS_CHECK)) == 1) {
			if (ac3SpuTrans()) {
#if 1 //BP_PS2
            //Handled by BP_SdEELoadLoopIteration()
            ++gBP_SdEELoadThreadWakeupCount;
#else
				BP_WakeupThread(id_SdEELoad);
#endif
			}
			ac3_spuwr();
		}
	} else {
		if ((BP_sceSdVoiceTransStatus(LNR_DMA_CH, SD_TRANS_STATUS_CHECK)) == 1) {
			if (lnrSpuTrans()) {
#if 1 //BP_PS2
            //Handled by BP_SdEELoadLoopIteration()
            ++gBP_SdEELoadThreadWakeupCount;
#else
				BP_WakeupThread(id_SdEELoad);
#endif
			}
			lnr_spuwr();
		}
	}
}

void ac3_int(void)
{
}

