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
#include "BP_SoundSupport.h"

/*========================================================================*/
/*使用するDMAチャンネルの登録*/
#define		STR_DMA_CH	1
//#define		STR_DMA_CH	1
/*使用するコア番号の登録*/
#define		STR_CORE	SD_CORE_1
/*使用するチャンネル(ステレオ)の登録*/
#define		STR_CH0_L	20
#define		STR_CH0_R	21
#define		STR_CH1_L	22
#define		STR_CH1_R	23
/*エンベロープ・リリース*/
#define		STR_FADE_OFF	13
#define		STR_CUT_OFF		7

/*使用するチャンネルbit*/
#define STR_CH0_BIT			((1<<STR_CH0_L)|(1<<STR_CH0_R))
#define STR_CH1_BIT			((1<<STR_CH1_L)|(1<<STR_CH1_R))
/*ストリーミングデータ・ヘッダサイズ*/
#define STR_HEADERSIZE		0x800
/*転送バッファ(メインRAM)数*/
#define TRANS_BUF_NUM		((EE_TRANS_SIZE*2)/STR_CHANNEL_SIZE)
/*１回のEE読み込み量*/
#define EE_TRANS_SIZE		0x4000
/*SPU再生バッファサイズ、このサイズの４倍がサウンド(SPU)RAMの確保サイズとなる*/
#define SPU_PLAY_BUF_SIZE	0x800
/*再生バッファ(サウンドRAM)を幾つ持つか*/
#define SPU_PLAY_BUF_NUM	2
/*SPU転送バッファ(サウンドRAM)サイズ*/
#define STR_CHANNEL_SIZE	(SPU_PLAY_BUF_SIZE*SPU_PLAY_BUF_NUM)
/*１割り込みの再生サイズ(44100KHz)*/
#define STR_INT_SIZE		0x80
/*========================================================================*/
extern int	id_SdEELoad;
extern int gBP_SdEELoadThreadWakeupCount;
extern int BP_EnableVagStreams;

extern unsigned long se_pant[0x41];
//EEからの読み込み関数
extern int EEOpen (unsigned int sd_code);
extern int EERead (int fd, unsigned char *ptr, int idx, int len);
/*----- EE読み込みアドレス -----*/
extern struct EE_ADDR ee_addr[2];

/*----- 外部参照あり -----*/
unsigned int	str2_status[2];
unsigned int	str2_effect[2];
unsigned int	str2_load_code[2];
unsigned int	str2_counter[2];
unsigned int	str2_stop_fg[2];
int				str2_wait_fg[2];	//再生開始待ち
unsigned int	str2_first_load[2];	//最初のデータロード中
int				str2_read_disable[2];
unsigned int	str2_mono_fg[2];			//1=モノラル
unsigned short	str2_pitch[2];				//周波数レジスタ値
int            str2_game_flag[2];
int            str2_top_pos[2];
int            str2_bp_surround_type[2];
int				str2_fp[2];
unsigned int	str2_play_counter[2];
unsigned int	str2_pause_fg[2];
/*-------------------------*/
unsigned short	str2_master_vol = 0x3FFF;	//マスターVolume
unsigned short	str2_master_pitch = 0x1000;	//マスターPitch

/*----- ストリーミング読み込み -----*/
unsigned int	str2_unload_size[2];
unsigned char	eeload2_buf[2][2][EE_TRANS_SIZE]	//BP_GCC __attribute__((aligned(4)))
;
unsigned char	*str2_trans_buf[2];
int				str2_read_status[2][TRANS_BUF_NUM];
unsigned long	str2_play_idx[2];
unsigned int	str2_mute_fg[2];
int				str2_iop_load_set[2];

int				ee2_buf_idx[2];
/*----- ストリーミング再生 -----*/
unsigned int	str2_l_r_fg[2];
unsigned int	spu_str2_start_ptr_l[2];
unsigned int	spu_str2_start_ptr_r[2];
unsigned int	str2_play_offset[2];
unsigned int	str2_unplay_size[2];		//未再生サイズx
unsigned int	spu_str2_idx[2];
unsigned int	str2_next_idx[2];
unsigned int	str2_off_ctr[2];
unsigned int	str2_keyoffs;//キーOFF bit値
unsigned short	str2_volume[2];				//音量

unsigned char dummy_data[0x800];

static STR_TIME str2_play_counter_work[ 2 ];

static int BP_str2_sceSdVoiceTrans( short channel, unsigned short mode, unsigned char *m_addr, unsigned int _s_addr, unsigned int size )
{
   //Sanitize the VAG data before transferring it.  Some JP stream data has a magic vag pattern near the end of the data which would not
   //have been reached on PS2 because of a VAG_LOOPEND in the preceding VAG packet, but out playback ignores the markers so the data
   //gets played as static.
   BP_SanitizeVAG( m_addr, size );
   return BP_sceSdVoiceTrans( channel, mode, m_addr, _s_addr, size );
}

/*-------------------------------------------------------------------*/
/*  ストリーミング・キーOFF                                          */
/*-------------------------------------------------------------------*/
void str2_tr_off(int ch)
{
	if (ch == 0) str2_keyoffs |= STR_CH0_BIT;
	else str2_keyoffs |= STR_CH1_BIT;
}

/*-------------------------------------------------------------------*/
/*  SPU2レジスタ・キュー書き込み                                     */
/*-------------------------------------------------------------------*/
void str2_spuwr(void) {
	if (str2_keyoffs) {
		BP_sceSdSetSwitch( STR_CORE|SD_S_KOFF , str2_keyoffs );
		str2_keyoffs = 0;
	}
}
/*========================================================================
*   START STR STREAM
*   ファイルの最初から再生
*-------------------------------------------------------------------------
*   OUT: = 0  : Complete
*        Else : Error
=========================================================================*/
#if 0
int vag_err_fg = 0;
void vag_check_(int ch)
{
	char *p = &eeload2_buf[ch][ee2_buf_idx[ch]][0];
	int i;
	int ctr = 0;

	for (i=0; i<EE_TRANS_SIZE; i += 0x10) {
		if ((p[1]==2 ) || (p[1]==1)) ctr++;
	}
	if (ctr != EE_TRANS_SIZE/0x10) {
		vag_err_fg = 1;
		PRINTF(("ERROR(VAG):This Voice File is LNR8.\n"));
	}
}
#endif
static
int StartEEStream(int ch)
{
	int				read_size, j;

	str2_read_disable[ch] = 0;
/*----- 前ストリームのクローズ -----*/
	str2_tr_off(ch);	/*STR ENV Off*/
/*----- ストリームのオープン -----*/
	if ((str2_fp[ch] = EEOpen(str2_load_code[ch])) <0) {
		PRINTF(("StartEEStream:File Open Error(%x)\n", str2_load_code[ch]));
		str2_fp[ch] = str2_first_load[ch] = str2_load_code[ch] = 0;
		str2_iop_load_set[ch] = 0;
		return(-1);
	}
/*----- 各種情報設定 -----*/
	str2_unplay_size[ch] = str2_unload_size[ch] = ee_addr[ch].sz;
PRINTF(("str2_unload_size[%x]=%x\n", ch, str2_unload_size[ch]));
	str2_volume[ch] = (unsigned short)0x7F;
/*----- 最初のデータ読み込み -----*/
	ee2_buf_idx[ch] = 0;
#if 1
	read_size = EERead(str2_fp[ch], &eeload2_buf[ch][ee2_buf_idx[ch]][0], ee2_buf_idx[ch], EE_TRANS_SIZE);
///*DEBUG*/	vag_check_(ch);
#else
	read_size = EE_TRANS_SIZE;
#endif
	
	for (j=0;j<(TRANS_BUF_NUM/2);j++) str2_read_status[ch][(ee2_buf_idx[ch]*(TRANS_BUF_NUM/2))+j] = 1;
	ee2_buf_idx[ch]++;
	if (str2_unload_size[ch] > read_size) str2_unload_size[ch] -= read_size;
	else str2_unload_size[ch] = 0;
	for (j=0;j<(TRANS_BUF_NUM/2);j++) str2_read_status[ch][(ee2_buf_idx[ch]*(TRANS_BUF_NUM/2))+j] = 0;

//	if (str2_unload_size[ch]) {
/*----- ２番目のデータ読み込み -----*/
//		read_size = EERead(str2_fp[ch], &eeload2_buf[ch][ee2_buf_idx[ch]][0], ee2_buf_idx[ch], EE_TRANS_SIZE);
//		for (j=0;j<(TRANS_BUF_NUM/2);j++) str2_read_status[ch][(ee2_buf_idx[ch]*(TRANS_BUF_NUM/2))+j] = 1;
//		ee2_buf_idx[ch] = (ee2_buf_idx[ch]+1) & 1;
//		if (str2_unload_size[ch] > read_size) str2_unload_size[ch] -= read_size;
//		else str2_unload_size[ch] = 0;
//	}
/*----- ワークエリア設定 -----*/
	str2_trans_buf[ch] = &eeload2_buf[ch][0][0];
//	str2_pause_fg[ch] = 0;	//2001/08/22 K.Muraoka
	return(0);
}


/*========================================================================
*	ストリーミングＣＤ読み込み
=========================================================================*/

void StrEELoad(int ch)
{
	int				read_size, i, j, status;

//	if ((str2_status[ch] < 4) || (str2_status[ch] > 6)) return;
//	if (str2_status[ch] > 6) return;

	for  (i=0; i<2; i++) {
		if (str2_unload_size[ch]) {
			status = 0;
			for (j=0;j<(TRANS_BUF_NUM/2);j++) {
				status |= str2_read_status[ch][(ee2_buf_idx[ch]*(TRANS_BUF_NUM/2))+j];//0-3/4-7
///*DEBUG*/ if (j == 0) PRINTF(("idx=%x    ", (ee2_buf_idx[ch]*(TRANS_BUF_NUM/2))+j ));
			}
			if (status == 0) {
#if 0 //BP_PS2
/*DEBUG*/WaitVblankStart();	//EEからのロードでも何故か必要
/*DEBUG*/WaitVblankEnd();	//<---これもないと不安定
#endif
				if (str2_unload_size[ch] > EE_TRANS_SIZE) {
					read_size = EERead (str2_fp[ch], &eeload2_buf[ch][ee2_buf_idx[ch]][0], ee2_buf_idx[ch], EE_TRANS_SIZE);
					if (read_size) {
///*DEBUG*/PRINTF(( "%x: %x: %x\n", str2_fp[ch], &eeload2_buf[ch][ee2_buf_idx[ch]][0], ee2_buf_idx[ch] ));
						for (j=0;j<(TRANS_BUF_NUM/2);j++) str2_read_status[ch][(ee2_buf_idx[ch]*(TRANS_BUF_NUM/2))+j] = 1;
						ee2_buf_idx[ch] = (ee2_buf_idx[ch]+1) & 1;
						str2_unload_size[ch] -= EE_TRANS_SIZE;
///*DEBUG*/PRINTF(("UnloadSize=%x(read=%x)\n", str2_unload_size[ch], read_size  ));
					}
				} else {
					read_size = EERead (str2_fp[ch], &eeload2_buf[ch][ee2_buf_idx[ch]][0], ee2_buf_idx[ch], EE_TRANS_SIZE);//端数は切り捨てられるので最後まで読む
					if (read_size) {
						for (j=0;j<(TRANS_BUF_NUM/2);j++) str2_read_status[ch][(ee2_buf_idx[ch]*(TRANS_BUF_NUM/2))+j] = 1;
						ee2_buf_idx[ch] = (ee2_buf_idx[ch]+1) & 1;
						str2_unload_size[ch] = 0;
					}
///*DEBUG*/PRINTF(("Str2:LoadEnd=%x\n", str2_unload_size[ch] ));
					break;
				}
			}
		}
	}
}
void str2_load(void)
{
	int ch;
//PRINTF(("status=%x\n", str2_status[1]));
	for (ch=0; ch<2; ch++) {
		switch (str2_status[ch]) {
		case 1:
/*if (str2_load_code) PRINTF(("Code=%x\n", str2_load_code));*/
//			if (ee_addr[ch].set_ctr == 1){
//				if ((str2_fp[ch] = EEOpen(str2_load_code[ch])) <0) {
//					PRINTF(("StartEEStream:File Open Error(%x)\n", str2_load_code[ch]));
//					str2_fp[ch] = str2_first_load[ch] = str2_load_code[ch] = 0;
//					return;
//				}
//				EERead(str2_fp[ch], &eeload2_buf[ch][ee2_buf_idx[ch]][0], ee2_buf_idx[ch], EE_TRANS_SIZE);
//			}
//			if ((ee_addr[ch].set_ctr >= 2)
//				|| (ee_addr[ch].set_ctr && (ee_addr[ch].sz <= EE_TRANS_SIZE) )) { //バッファが埋ってからスタート
			if (ee_addr[ch].set_ctr >= 1){
				if (StartEEStream(ch) != 0) str2_status[ch] = 0;
				else {
					str2_l_r_fg[ch] = 0;
					str2_status[ch] = 2;
				}
			} else PRINTF(("Waiting for EE Data Trans.(0)\n"));
			break;
		case 2:
			if (ee_addr[ch].set_ctr >= 2) {
				StrEELoad(ch);
				str2_status[ch]++;
			} else {
				if (str2_unload_size[ch] == 0) str2_status[ch]++;
				else PRINTF(("Waiting for EE Data Trans.(1)\n"));
			}
		case 3:
		case 4:
		case 5:
		case 6:
			StrEELoad(ch);
			break;
		case 7:
			break;
		case 8:
//			PcmClose(str2_fp[ch]);
			ee_addr[ch].set_ctr = 0;
			ee_addr[ch].read_ctr =0;
			str2_fp[ch] = str2_status[ch] = str2_load_code[ch] = 0;
			str2_iop_load_set[ch] = 0;	//2001/09/09 ロード残りがあれば、捨てる

			PRINTF(("***STR Terminate(ch=%x)***\n", ch));
			break;
		}
	}
}
/*========================================================================
*   TRANSFER BGM DATA TO SPU
=========================================================================*/
int vag_err_fg = 0;
#if 0
int vag_check_int(int ch)
{
	char *p = (u_char*)(str2_trans_buf[ch]+str2_play_offset[ch]);
	int i;
	int ctr = 0;

	for (i=0; i<SPU_PLAY_BUF_SIZE; i += 0x10) {
		if ((p[1]==2 ) || (p[1]==1)) ctr++;
		else PRINTF(("%x:", (p[1]&0xFF)));
	}
	if (ctr != SPU_PLAY_BUF_SIZE/0x10) {
		vag_err_fg = 1;
		PRINTF(("ERROR(VAG):This Voice File is LNR8.(%x)\n", ctr));
		return(0);
	} else return(1);
}
#endif

static void bp_set_voice_surround_params( const int ch )
{
   unsigned int vol;
   float fvol;
   
   vol = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*0x3fff)/0x7F)*str2_master_vol)/0x3FFF;
   vol &= 0x7fff;
   fvol = BP_PS2VolToFVol(vol);

   if( str2_mono_fg[ch] )
   {
      //In this case, the two voices contain the same data.
      //They mix an additional 1/3 volume (of the same signal) from the second channel
      //into the same speakers as the first channel.
      BP_SoundSupport_SurroundSoundSetVoicePanVol( STR_CORE, (STR_CH0_L+ch*2), vox_fader[ch].set_bp_angle, fvol );
      BP_SoundSupport_SurroundSoundSetVoicePanVol( STR_CORE, (STR_CH0_R+ch*2), vox_fader[ch].set_bp_angle, fvol / 3 );
   }
   else
   {
      //In this case, the 3D parameters *should* be ignored because we *should* have set a surround voice type
      //that spreads the stereo signal.  Set them same as mono anyway.
      BP_SoundSupport_SurroundSoundSetVoicePanVol( STR_CORE, (STR_CH0_L+ch*2), vox_fader[ch].set_bp_angle, fvol );
      BP_SoundSupport_SurroundSoundSetVoicePanVol( STR_CORE, (STR_CH0_R+ch*2), vox_fader[ch].set_bp_angle, fvol / 3 );
   }
   BP_SoundSupport_SurroundSoundSetVoiceType( STR_CORE, (STR_CH0_L+ch*2), str2_bp_surround_type[ch] );
   BP_SoundSupport_SurroundSoundSetVoiceType( STR_CORE, (STR_CH0_R+ch*2), str2_bp_surround_type[ch] );
   BP_SoundSupport_ApplyVoiceVolume( STR_CORE, STR_CH0_L+ch*2 );
   BP_SoundSupport_ApplyVoiceVolume( STR_CORE, STR_CH0_R+ch*2 );
}

int				str2_mono_offset[2];
unsigned int	str2_mono_next[2];
int				mute2_l_r_fg[2];
int				str2_clock_offset[2];
int Str2SpuTrans(int ch)
{
	int		dma_fg = 0;
	int		end_fg = 0;
	unsigned int voll, volr;

	if (ch > 1) {
		PRINTF(("ERROR:STREAMING CHANNEL(%x)\n", ch));
		return(0);
	}
	if ((str2_stop_fg[ch]) && (str2_status[ch] >= 3 ) ) {	//ストリーミング停止要求
		switch (str2_stop_fg[ch]) {
		case 1:
			BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_ADSR1 , SD_ADSR1( 0, 0, 0xF, 0xF) );
			BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_ADSR2 , SD_ADSR2( 0, 0, 0, STR_CUT_OFF));
			BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_ADSR1 , SD_ADSR1( 0, 0, 0xF, 0xF) );
			BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_ADSR2 , SD_ADSR2( 0, 0, 0, STR_CUT_OFF));
			str2_first_load[ch] = 0;
			str2_status[ch] = 8;
			break;
		case 2:
			BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_ADSR1 , SD_ADSR1( 0, 0, 0xF, 0xF) );
			BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_ADSR2 , SD_ADSR2( 0, 0, 0, STR_FADE_OFF));
			BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_ADSR1 , SD_ADSR1( 0, 0, 0xF, 0xF) );
			BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_ADSR2 , SD_ADSR2( 0, 0, 0, STR_FADE_OFF));
			break;
		}
		str2_tr_off(ch);
		str2_stop_fg[ch] = 0;
PRINTF(("EE STR Stopped.(ch=%x)\n", ch));
	} else {
		if ((str2_stop_fg[ch]) && (str2_status[ch] >= 1 ) ) {	//ストリーミング停止要求
			str2_first_load[ch] = 0;
			str2_status[ch] = 8;
			str2_stop_fg[ch] = 0;
PRINTF(("EE STR Stopped Before Play Start.(ch=%x)\n", ch));
		}
	}
/*--------------------- Transfer BGM Data -------------------------------*/
	switch (str2_status[ch]) {
	case 1:
	case 2:
//		PRINTF(("STATUS=%x UNLOAD=%x\n", str2_status[ch], str2_unload_size[ch]));
		break;
	case 3:
		if (str2_l_r_fg[ch] == 0) {
			str2_play_idx[ch] = 0;
			str2_play_offset[ch] = 0;
			spu_str2_start_ptr_l[ch] = (STR_SPU_START+ch*0x2000);
         BP_SoundSupport_InitADPCMStreamingBuffer( ch, 0, spu_str2_start_ptr_l[ch], STR_CHANNEL_SIZE );

///*DEBUG*/	vag_check_int(ch);
			BP_sceSdSetAddr(STR_CORE|( (STR_CH0_L+ch*2)<<1 )|SD_VA_LSAX, (STR_SPU_START+ch*0x2000) );
			BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
				(u_char*)str2_trans_buf[ch], spu_str2_start_ptr_l[ch],(u_int)SPU_PLAY_BUF_SIZE );
			if (!str2_mono_fg[ch]) {
				str2_play_offset[ch] = SPU_PLAY_BUF_SIZE;
				str2_unplay_size[ch] -= SPU_PLAY_BUF_SIZE;
			}
			str2_l_r_fg[ch] = 1;
		} else {
			spu_str2_start_ptr_r[ch] = (STR_SPU_START+ch*0x2000)+STR_CHANNEL_SIZE;
         BP_SoundSupport_InitADPCMStreamingBuffer( ch, 1, spu_str2_start_ptr_r[ch], STR_CHANNEL_SIZE );
///*DEBUG*/	vag_check_int(ch);
			BP_sceSdSetAddr(STR_CORE|( (STR_CH0_R+ch*2)<<1 )|SD_VA_LSAX, (STR_SPU_START+ch*0x2000)+STR_CHANNEL_SIZE );
			BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
				(u_char*)(str2_trans_buf[ch]+str2_play_offset[ch]), spu_str2_start_ptr_r[ch],(u_int)SPU_PLAY_BUF_SIZE );
			str2_play_offset[ch] += SPU_PLAY_BUF_SIZE;
			str2_unplay_size[ch] -= SPU_PLAY_BUF_SIZE;
			if (!str2_mono_fg[ch]) {
				str2_read_status[ch][str2_play_idx[ch]] = 0;
				str2_play_idx[ch]++;
			}
			str2_l_r_fg[ch] = 0;
			str2_counter[ch] += SPU_PLAY_BUF_SIZE;
			++str2_status[ch];
		}
		dma_fg = 1;
		break;
	case 4:
		if ( (str2_unplay_size[ch] == 0) || (str2_unplay_size[ch]&0x80000000) ) ++str2_status[ch];
		if (str2_l_r_fg[ch] == 0) {
			*(str2_trans_buf[ch]+(str2_play_offset[ch]+(SPU_PLAY_BUF_SIZE-0xF))) |= 1;
///*DEBUG*/	vag_check_int(ch);
			BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
				(u_char*)(str2_trans_buf[ch]+str2_play_offset[ch]), (spu_str2_start_ptr_l[ch]+SPU_PLAY_BUF_SIZE), (u_int)SPU_PLAY_BUF_SIZE );
			if (!str2_mono_fg[ch]) {
				str2_play_offset[ch] += SPU_PLAY_BUF_SIZE;
				str2_unplay_size[ch] -= SPU_PLAY_BUF_SIZE;
			}
			str2_l_r_fg[ch] = 1;
		} else {
			*(str2_trans_buf[ch]+(str2_play_offset[ch]+(SPU_PLAY_BUF_SIZE-0xF))) |= 1;
///*DEBUG*/	vag_check_int(ch);
			BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
				(u_char*)(str2_trans_buf[ch]+str2_play_offset[ch]), (spu_str2_start_ptr_r[ch]+SPU_PLAY_BUF_SIZE), (u_int)SPU_PLAY_BUF_SIZE );
			str2_play_offset[ch] += SPU_PLAY_BUF_SIZE;
			str2_unplay_size[ch] -= SPU_PLAY_BUF_SIZE;
			str2_read_status[ch][str2_play_idx[ch]] = 0;
			str2_play_idx[ch]++;
			str2_l_r_fg[ch] = 0;
			str2_counter[ch] += SPU_PLAY_BUF_SIZE;
			++str2_status[ch];
		}
		dma_fg = 1;
		break;
	case 5:
		if (str2_first_load[ch]) str2_first_load[ch] = 0;
		if (str2_wait_fg[ch]) break;
		if (sng_pause_fg || str2_pause_fg[ch]) break;	//2001/01/05 K.Muraoka
/*SPU2チャンネルparam書き込み*/
		if (sound_mono_fg) {
			voll = volr = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*se_pant[0x40-vox_fader[ch].set_pan])/0x7F)*str2_master_vol)/0x3FFF;
         if( !BP_EnableVagStreams )
         {
            voll = volr = 0;
         }
			BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLL , voll );
			BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLR , volr );
			BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLL , voll/3 );
			BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLR , volr/3 );
			vox_fader[ch].play_vol = vox_fader[ch].set_vol;
			vox_fader[ch].play_pan = vox_fader[ch].set_pan;
PRINTF(("VOX(SOUND_MONO): voll=%x:volr=%x\n", voll, volr));
		} else {
			if (str2_mono_fg[ch]) {	//波形データがMONOの時
				voll = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*se_pant[0x40-vox_fader[ch].set_pan])/0x7F)*str2_master_vol)/0x3FFF;
				volr = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*se_pant[vox_fader[ch].set_pan])/0x7F)*str2_master_vol)/0x3FFF;
            if( !BP_EnableVagStreams )
            {
               voll = volr = 0;
            }
				BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLL , voll );
				BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLR , volr );
				BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLL , voll/3 );
				BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLR , volr/3 );
				vox_fader[ch].play_vol = vox_fader[ch].set_vol;
				vox_fader[ch].play_pan = vox_fader[ch].set_pan;
PRINTF(("VOX(MONO): voll=%x:volr=%x\n", voll, volr));
			} else {	//STEREO
//2001/04/10 ステレオ音声は、音量だけ変える（パン変更は無効）
//				voll = volr = ((unsigned int)((str2_volume[ch]*se_pant[0x3F])/0x7F)*str2_master_vol)/0x3FFF;
				voll = volr = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*se_pant[0x3F])/0x7F)*str2_master_vol)/0x3FFF;
            if( !BP_EnableVagStreams )
            {
               voll = volr = 0;
            }

				BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLL ,  voll);
				BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLR , (unsigned int)0 );
				BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLL , (unsigned int)0 );
				BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLR , volr );
//2001/04/10
				vox_fader[ch].play_vol = vox_fader[ch].set_vol;
				vox_fader[ch].play_pan = vox_fader[ch].set_pan;

PRINTF(("VOX(STEREO): voll=%x:volr=%x\n", voll, volr));
			}
		}
      bp_set_voice_surround_params( ch );
      vox_fader[ch].play_bp_angle = vox_fader[ch].set_bp_angle;
		BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_PITCH , ((unsigned int)str2_pitch[ch]*str2_master_pitch)/0x1000 );
		BP_sceSdSetAddr ( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VA_SSA , (STR_SPU_START+ch*0x2000) );
		BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_ADSR1 , SD_ADSR1( 0, 0, 0xF, 0xF) );
		BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_ADSR2 , SD_ADSR2( 0, 0, 0, ENV_CUT_OFF));
		BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_PITCH , ((unsigned int)str2_pitch[ch]*str2_master_pitch)/0x1000 );
		BP_sceSdSetAddr ( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VA_SSA , (STR_SPU_START+ch*0x2000)+STR_CHANNEL_SIZE );
		BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_ADSR1 , SD_ADSR1( 0, 0, 0xF, 0xF) );
		BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_ADSR2 , SD_ADSR2( 0, 0, 0, ENV_CUT_OFF));
/*KON*/	if (ch == 0) {
			if (str2_effect[0]) {
				rev_bit_data[1] |= STR_CH0_BIT;
				rev_on_bit[1] |= STR_CH0_BIT;
			} else {
				rev_bit_data[1] &= ~STR_CH0_BIT;
				rev_off_bit[1] |= STR_CH0_BIT;
			}
			BP_sceSdSetSwitch( STR_CORE|SD_S_KON , STR_CH0_BIT);
		} else {
			if (str2_effect[1]) {
				rev_bit_data[1] |= STR_CH1_BIT;
				rev_on_bit[1] |= STR_CH1_BIT;
			} else {
				rev_bit_data[1] &= ~STR_CH1_BIT;
				rev_off_bit[1] |= STR_CH1_BIT;
			}
			BP_sceSdSetSwitch( STR_CORE|SD_S_KON , STR_CH1_BIT);
		}
		str2_clock_offset[ch] = 0;
		reset_play_counter( &str2_play_counter_work[ ch ], str2_clock_offset[ch] );	// Add by K.Uehara
///*DEBUG*/PRINTF(("KON:Vol=%x, Pitch=%x CH=%x\n",  ((unsigned int)str2_volume[ch]*str2_master_vol)/0x3FFF,((unsigned int)str2_pitch[ch]*str2_master_pitch)/0x1000, ch ));
		spu_str2_idx[ch] = mute2_l_r_fg[ch] = 0;
		str2_next_idx[ch] = SPU_PLAY_BUF_SIZE;

		if(str2_mono_fg[ch]) str2_mono_next[ch] = str2_next_idx[ch];	//2001/09/27

		str2_mute_fg[ch] = 0;
		++str2_status[ch];
		if ( (str2_unplay_size[ch] == 0) || (str2_unplay_size[ch]&0x80000000) ) {
PRINTF(("Str2SpuTrans:status=4 --> 6(%x)\n", str2_unplay_size[ch]));
			str2_off_ctr[ch] = (STR_CHANNEL_SIZE / STR_INT_SIZE) - 1;
			++str2_status[ch];
		}
		break;
	case 6:
/*エンベロープ値=0ならストリーミング終える*/
		if ( BP_sceSdGetParam( STR_CORE | ((STR_CH0_L+ch*2)<<1) | SD_VP_ENVX ) == 0 ) {
			str2_off_ctr[ch] = 0xFFFFFFFF;
			++str2_status[ch];
			PRINTF(("Str:Envelope Stopped\n"));
			break;
		}

      if ((vox_fader[ch].set_vol != vox_fader[ch].play_vol) || (vox_fader[ch].set_pan != vox_fader[ch].play_pan) || (vox_fader[ch].set_bp_angle != vox_fader[ch].play_bp_angle)) {
			if (sound_mono_fg) {
				voll = volr = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*se_pant[0x20])/0x7F)*str2_master_vol)/0x3FFF;
            if( !BP_EnableVagStreams )
            {
               voll = volr = 0;
            }
				BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLL , voll );
				BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLR , volr );
				BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLL , voll/3 );
				BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLR , volr/3 );
				vox_fader[ch].play_vol = vox_fader[ch].set_vol;
				vox_fader[ch].play_pan = vox_fader[ch].set_pan;
PRINTF(("VOX(SOUND MONO): voll=%x:volr=%x\n", voll, volr));
			} else {
				if (str2_mono_fg[ch]) {	//波形データがMONOの時
					voll = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*se_pant[0x40-vox_fader[ch].set_pan])/0x7F)*str2_master_vol)/0x3FFF;
					volr = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*se_pant[vox_fader[ch].set_pan])/0x7F)*str2_master_vol)/0x3FFF;
               if( !BP_EnableVagStreams )
               {
                  voll = volr = 0;
               }
					BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLL , voll );
					BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLR , volr );
					BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLL , voll/3 );
					BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLR , volr/3 );
					vox_fader[ch].play_vol = vox_fader[ch].set_vol;
					vox_fader[ch].play_pan = vox_fader[ch].set_pan;
PRINTF(("VOX(MONO): voll=%x:volr=%x\n", voll, volr));

//2001/04/10 ステレオ音声は、音量だけ変える（パン変更は無効）
				} else {	//STEREO
					voll = volr = ((unsigned int)((((str2_volume[ch]*vox_fader[ch].set_vol)/0x3F)*se_pant[0x3F])/0x7F)*str2_master_vol)/0x3FFF;
               if( !BP_EnableVagStreams )
               {
                  voll = volr = 0;
               }
					BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLL ,  voll);
					BP_sceSdSetParam( STR_CORE|((STR_CH0_L+ch*2)<<1)|SD_VP_VOLR , (unsigned int)0 );
					BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLL , (unsigned int)0 );
					BP_sceSdSetParam( STR_CORE|((STR_CH0_R+ch*2)<<1)|SD_VP_VOLR , volr );
					vox_fader[ch].play_vol = vox_fader[ch].set_vol;
					vox_fader[ch].play_pan = vox_fader[ch].set_pan;
PRINTF(("VOX(STEREO): voll=%x:volr=%x\n", voll, volr));
				}
			}
         bp_set_voice_surround_params( ch );
         vox_fader[ch].play_bp_angle = vox_fader[ch].set_bp_angle;
		}
		spu_str2_idx[ch] = BP_sceSdGetAddr(SD_VA_NAX | STR_CORE | ((STR_CH0_L+ch*2)<< 1));
/*DEBUG:再生中アドレスを表示*/
///*DEBUG*/PRINTF(("nax=%x,%x\n", spu_str2_idx[ch], sceSdGetAddr(SD_VA_NAX | STR_CORE | ((STR_CH0_R+ch*2)<< 1)) ));
		spu_str2_idx[ch] -= (STR_SPU_START+ch*0x2000);
		if ( (spu_str2_idx[ch] >= SPU_PLAY_BUF_SIZE*2) || (spu_str2_idx[ch]&0x80000000) ) {
			PRINTF(("ERROR:Str2(ch%x)Address(%x)\n", ch, spu_str2_idx[ch]));
			break;
		}
		if (str2_mute_fg[ch] == 0){
//			str2_play_counter[ch]++; // CHANGE By K.Uehara
			str2_play_counter[ch] = get_play_counter( &str2_play_counter_work[ ch ] );
		}

		if ( (str2_next_idx[ch]==(spu_str2_idx[ch]&SPU_PLAY_BUF_SIZE)) || (str2_l_r_fg[ch]) || (mute2_l_r_fg[ch]) ) {
			dma_fg = 1;
			if ( (str2_mono_fg[ch]) && (str2_mono_next[ch] != str2_next_idx[ch]) ) goto StrPause0;	//2001/09/26 モノラル時は前半・後半に順序よくバッファリングしないとstr2_read_status[]をクリアするタイミングがずれる

			if ((sng_pause_fg | str2_pause_fg[ch]) && (str2_l_r_fg[ch] == 0)) goto StrPause0;	//2001/01/05 K.Muraoka
			if ( (str2_read_status[ch][str2_play_idx[ch]]!=0) && !(mute2_l_r_fg[ch]) ) {
/*-----------------------------------------------------------------------*/
///*DEBUG*/ if (str2_l_r_fg[ch] == 0) PRINTF(("str2_play_offset[ch]=%x:%x\n", str2_play_offset[ch], spu_str2_start_ptr_l[ch]));
				if (str2_mute_fg[ch] != 0){
					reset_play_counter( &str2_play_counter_work[ ch ], str2_clock_offset[ch]+(((SPU_PLAY_BUF_SIZE*28)/16)/(str2_pitch[ch])/200) );	// Add by K.Uehara
					str2_clock_offset[ch] = 0;
					str2_mute_fg[ch] = 0;
				}
				if (str2_l_r_fg[ch]) str2_counter[ch] += SPU_PLAY_BUF_SIZE;
				if (str2_mono_fg[ch]) str2_mono_offset[ch] = 0;
				if (spu_str2_idx[ch] >= SPU_PLAY_BUF_SIZE) { /*後半再生中は前半分を転送*/
					if (str2_l_r_fg[ch] == 0) {
///*DEBUG*/	vag_check_int(ch);
						BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
							(u_char*)(str2_trans_buf[ch]+str2_play_offset[ch]), spu_str2_start_ptr_l[ch], (u_int)SPU_PLAY_BUF_SIZE );
						str2_l_r_fg[ch] = 1;
					} else {
///*DEBUG*/	vag_check_int(ch);
						BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
							(u_char*)(str2_trans_buf[ch]+str2_play_offset[ch]), spu_str2_start_ptr_r[ch], (u_int)SPU_PLAY_BUF_SIZE );
						str2_next_idx[ch] = (str2_next_idx[ch]+SPU_PLAY_BUF_SIZE)&((SPU_PLAY_BUF_SIZE*SPU_PLAY_BUF_NUM)-1);
						str2_l_r_fg[ch] = 0;
						if (!str2_mono_fg[ch]) {
							str2_read_status[ch][str2_play_idx[ch]] = 0;
							str2_play_idx[ch]++;
							if (str2_play_idx[ch] == TRANS_BUF_NUM) str2_play_idx[ch] = 0;
						}
						if (str2_mono_fg[ch]) {
							str2_mono_next[ch] = str2_next_idx[ch];
							str2_mono_offset[ch] = 1;
						}
					}
//					*(str2_trans_buf[ch]+str2_play_offset[ch]+1) |= 4;
				} else {								/*前半再生中は後半分を転送*/
					*(str2_trans_buf[ch]+str2_play_offset[ch]+SPU_PLAY_BUF_SIZE-0xF) |= 1;
					if (str2_l_r_fg[ch] == 0) {
///*DEBUG*/	vag_check_int(ch);
						BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
							(u_char*)(str2_trans_buf[ch]+str2_play_offset[ch]), (spu_str2_start_ptr_l[ch]+SPU_PLAY_BUF_SIZE), (u_int)SPU_PLAY_BUF_SIZE );
						str2_l_r_fg[ch] = 1;
					} else {
						str2_next_idx[ch] = (str2_next_idx[ch]+SPU_PLAY_BUF_SIZE)&((SPU_PLAY_BUF_SIZE*SPU_PLAY_BUF_NUM)-1);
///*DEBUG*/	vag_check_int(ch);
						BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
							(u_char*)(str2_trans_buf[ch]+str2_play_offset[ch]), (spu_str2_start_ptr_r[ch]+SPU_PLAY_BUF_SIZE), (u_int)SPU_PLAY_BUF_SIZE );
						str2_l_r_fg[ch] = 0;
						str2_read_status[ch][str2_play_idx[ch]] = 0;
						str2_play_idx[ch]++;
						if (str2_play_idx[ch] == TRANS_BUF_NUM) str2_play_idx[ch] = 0;

						if(str2_mono_fg[ch]) {
							str2_mono_next[ch] = str2_next_idx[ch];
							str2_mono_offset[ch] = 1;
						}
					}
				}
				
				if (str2_mono_fg[ch]) {
					if (str2_mono_offset[ch]) {
						str2_play_offset[ch] = str2_play_offset[ch]+SPU_PLAY_BUF_SIZE;
						if (str2_play_offset[ch] == (STR_CHANNEL_SIZE*TRANS_BUF_NUM)) str2_play_offset[ch] = 0;
						if (str2_unplay_size[ch] > SPU_PLAY_BUF_SIZE) {
							str2_unplay_size[ch] -= SPU_PLAY_BUF_SIZE;
///*DEBUG*/PRINTF(("%x:%x:%x\n", str2_unplay_size[ch], str2_unload_size[ch], str2_counter[ch]));
						} else {
							str2_off_ctr[ch] = (STR_CHANNEL_SIZE / STR_INT_SIZE) - 1;
							str2_play_offset[ch] = 0;
/*DEBUG*/PRINTF(("...Streaming Stoped.(str2_counter=%x)\n", str2_counter[ch]));
							++str2_status[ch];
						}
					}
				} else {
					str2_play_offset[ch] = str2_play_offset[ch]+SPU_PLAY_BUF_SIZE;
					if (str2_play_offset[ch] ==(STR_CHANNEL_SIZE*TRANS_BUF_NUM)) str2_play_offset[ch] = 0;
					if (str2_unplay_size[ch] > SPU_PLAY_BUF_SIZE) {
						str2_unplay_size[ch] -= SPU_PLAY_BUF_SIZE;
					} else {
						str2_off_ctr[ch] = (STR_CHANNEL_SIZE / STR_INT_SIZE) - 1;
						str2_play_offset[ch] = 0;
						++str2_status[ch];
					}
				}
			} else {
/*--------------------- Mute Volume While Retry CD Read -----------------*/

//2001/09/20 unload_sizeが0の時、リード待ちでここに来るのはエラーなので、プレイカウンタを進める
				if ((str2_unload_size[ch] == 0) && (mute2_l_r_fg[ch] == 0)) {
					str2_off_ctr[ch] = (STR_CHANNEL_SIZE / STR_INT_SIZE) - 1;
					str2_play_offset[ch] = 0;
					str2_counter[ch] += STR_INT_SIZE;
					str2_play_counter[ch] = str2_clock_offset[ch];
/*DEBUG*/PRINTF(("...Streaming Stoped(read wait error).(str2_counter=%x)\n", str2_counter[ch]));
					++str2_status[ch];
					break;
//2001/09/20 ここまで
				} else {	//2001/09/25 未再生データが未ロードデータより大きければ補正する
//					if ( str2_unplay_size[ch] > str2_unload_size[ch] ) {
//						str2_counter[ch] += (str2_unplay_size[ch] - str2_unload_size[ch]);
//						str2_unplay_size[ch] =  str2_unload_size[ch];  //2001/09/25 リードエラー時にunplay_sizeが遅れる事があるので、矯正
//					}
					if ( mute2_l_r_fg[ch]==0 ) PRINTF(("EE READ Retry(ch=%x:unplay=%x:unload=%x:counter=%x)\n", ch, str2_unplay_size[ch], str2_unload_size[ch], str2_counter[ch]));
				}
StrPause0:	//2001/01/05 K.Muraoka
//PRINTF(("status=%x pause=%x/%x\n", str2_status[ch], sng_pause_fg, str2_pause_fg[ch]));
				if (str2_mute_fg[ch] == 0) {
					str2_clock_offset[ch] = get_play_counter( &str2_play_counter_work[ ch ] );
					str2_mute_fg[ch] = 1;
				}
				if (spu_str2_idx[ch] >= SPU_PLAY_BUF_SIZE) { /*後半再生中は前半分を転送*/
					*(dummy_data+1) = 6;
					*(dummy_data+((sizeof(dummy_data))-0xF)) = 2;
					if (mute2_l_r_fg[ch] == 0) {
						BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
							(u_char*)dummy_data, spu_str2_start_ptr_l[ch], (u_int)SPU_PLAY_BUF_SIZE );
						mute2_l_r_fg[ch] = 1;
					} else {
						BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
							(u_char*)dummy_data, spu_str2_start_ptr_r[ch], (u_int)SPU_PLAY_BUF_SIZE );
						str2_next_idx[ch] = (str2_next_idx[ch]+SPU_PLAY_BUF_SIZE)&((SPU_PLAY_BUF_SIZE*SPU_PLAY_BUF_NUM)-1);
						mute2_l_r_fg[ch] = 0;
					}

				} else {								/*前半再生中は後半分を転送*/
					*(dummy_data+1) = 2;
					*(dummy_data+((sizeof(dummy_data))-0xF)) = 3;
					if (mute2_l_r_fg[ch] == 0) {
						mute2_l_r_fg[ch] = 1;
						BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
							(u_char*)dummy_data, (spu_str2_start_ptr_l[ch]+SPU_PLAY_BUF_SIZE), (u_int)SPU_PLAY_BUF_SIZE );
					} else {
						mute2_l_r_fg[ch] = 0;
						str2_next_idx[ch] = (str2_next_idx[ch]+SPU_PLAY_BUF_SIZE)&((SPU_PLAY_BUF_SIZE*SPU_PLAY_BUF_NUM)-1);
						BP_str2_sceSdVoiceTrans( STR_DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
							(u_char*)dummy_data, (spu_str2_start_ptr_r[ch]+SPU_PLAY_BUF_SIZE), (u_int)SPU_PLAY_BUF_SIZE );
					}
				}
			}
		}
		break;
	case 7:
		str2_counter[ch] += STR_INT_SIZE;
//		str2_play_counter[ch]++;	// RETOUCH BY K.Uehara
		str2_play_counter[ch] = get_play_counter( &str2_play_counter_work[ ch ] );
//		if ((str2_off_ctr[ch]-- == 0xFFFFFFFF)) { /*2001/09/04*/

		if ((str2_off_ctr[ch]&0x80000000)) {
			str2_tr_off(ch);
			++str2_status[ch];
		}
		str2_off_ctr[ch]--;
		break;
	case 8:
		str2_counter[ch] += STR_INT_SIZE;
		end_fg = 1;
		break;
	}
	return (dma_fg | end_fg);
}

void str2_int(void)
{
	if ((BP_sceSdVoiceTransStatus(STR_DMA_CH, SD_TRANS_STATUS_CHECK)) == 1) {
		if (Str2SpuTrans(0))
      {
#if 1 //BP_PS2
         //BP - handled by BP_SdEELoadLoopIteration
         ++gBP_SdEELoadThreadWakeupCount;
#else
         BP_WakeupThread(id_SdEELoad);
#endif
      }
		else {
			if (Str2SpuTrans(1))
         {
#if 1 //BP_PS2
            //BP - handled by BP_SdEELoadLoopIteration
            ++gBP_SdEELoadThreadWakeupCount;
#else
            BP_WakeupThread(id_SdEELoad);
#endif
         }
		}
	}
///*DEBUG*/	else PRINTF(("*"));
	str2_spuwr();
}

unsigned char	dummy_data [0x800] = {
/*0x0000 - 0x03FF*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*000*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*010*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*020*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*030*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*040*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*050*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*060*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*070*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*080*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*090*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0A0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0B0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0C0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0D0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0E0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0F0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*100*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*110*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*120*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*130*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*140*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*150*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*160*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*170*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*180*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*190*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1A0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1B0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1C0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1D0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1E0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1F0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*200*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*210*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*220*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*230*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*240*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*250*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*260*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*270*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*280*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*290*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2A0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2B0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2C0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2D0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2E0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2F0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*300*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*310*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*320*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*330*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*340*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*350*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*360*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*370*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*380*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*390*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3A0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3B0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3C0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3D0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3E0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3F0*/
/*0x400 - 0x7FF*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*000*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*010*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*020*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*030*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*040*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*050*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*060*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*070*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*080*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*090*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0A0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0B0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0C0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0D0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0E0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*0F0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*100*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*110*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*120*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*130*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*140*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*150*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*160*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*170*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*180*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*190*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1A0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1B0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1C0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1D0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1E0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*1F0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*200*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*210*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*220*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*230*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*240*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*250*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*260*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*270*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*280*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*290*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2A0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2B0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2C0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2D0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2E0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*2F0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*300*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*310*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*320*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*330*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*340*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*350*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*360*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*370*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*380*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*390*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3A0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3B0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3C0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3D0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*3E0*/
0x0C,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  /*3F0*/
};

