
/*===================================================================
 *  Title           : "METALGEAR SOLID" SOUND
 *                  : -  ワークエリア設定 -
 *  version         : 1.00
 *
 *  Code            : mj001
 *  Filename        : SD_WK.H
 *  Creator         : K.C.E JAPAN - K.Muraoka
 *  First edition   : 1996/11/13
 *
 *  Note            : 
 *                  : 
 *===================================================================*/
#include	<libsd.h>
#include	"sd_debug.h"
#include	"sd_incl.h"
/*---------------------- 波形ロード ---------------------------------*/

int				wave_loaded;

/*--------------------- ＳＮＧ再生 ----------------------------------*/

//unsigned char		*sng_data;			/*ＳＮＧデータの先頭へのポインタ*/
unsigned long		sng_load_code, sng_play_code;
int					sng_status;	/*ＳＮＧ再生状況*/
#if 1 //BP_PS2
void *            bp_sng_fp;
#else
int					sng_fp;
#endif
int					sng_pause_fg;
int					int_pause_fg;

long				sng_fadein_time;
long				sng_fade_time[SNG_TRACK_NUM];
long				sng_fade_value[SNG_TRACK_NUM];
long				sng_fadein_fg;
int					sng_fout_fg;
int					sng_fout_term[SPU_CORE_NUM];
int					sng_master_vol[SNG_TRACK_NUM];

//int					sng_tempo_move;	//1999/12/22 for Mixer Fader
int					sng_kaihi_fg;

long				sng_syukan_vol;
int					sng_syukan_fg;

long				sng_mic_vol;
int					sng_mic_fg;

int					sd_sng_code_buf[0x10];		/*音コード一時保存*/
int					sd_code_set, sd_code_read;

int				vox_on_vol;

/*--------------------- For PCM Read ------------------------------------*/

//int				sd_alloc_size;
//int				act_fg = 1;
//char			pcm_path[0x80];

//int				pcm_file_status[3];

/*--------------------- Sound Mono/Stereo Flag --------------------------*/
int				sound_mono_fg;
/*--------------------- For Sound Effect --------------------------------*/

unsigned long	se_load_code;
unsigned long	se_save_code2;
//unsigned char	*se_data;
//struct SETBL	*se_header;
struct SEPLAYTBL	se_playing[SE_TRACK_NUM];
struct SEPLAYTBL	se_request[SE_TRACK_NUM];
int				se_pan[SE_TRACK_NUM];
int				se_vol[SE_TRACK_NUM];
float          se_bp_angle[SE_TRACK_NUM];
int				se_bp_surround_type[SE_TRACK_NUM];
#if 1 //BP_PS2
void *         bp_se_fp;
#else
int				se_fp;
#endif
int				se_rev_on;
int				stop_jizoku_se;
int				stop_jizoku_se2;

/*---------------------- For Wave Load ------------------------------*/
unsigned long	spu_wave_start_ptr;
//unsigned char	*wave_header;
unsigned long	wave_load_status;
unsigned long	wave_load_code;
unsigned long	wave_save_code;
unsigned long	wave_unload_size;
unsigned long	wave_load_size;
#if 1 //BP_PS2
void *         bp_wave_fp;
#else
int				wave_fp;
#endif
unsigned char	*wave_load_ptr;
unsigned long	bp_base_spu_load_offset;
unsigned long	spu_load_offset;

/*--------------------- From PSKERO ------------------------------------*/
/*unsigned long		sound_code;*/
unsigned long		mdata1,mdata2,mdata3,mdata4;
unsigned long		key_fg;
unsigned long		mtrack;
unsigned char		*mptr;
unsigned long 		song_end[SPU_CORE_NUM];
struct SPU_TRACK_REG	spu_tr_wk[INTERNAL_TRACK_NUM];
struct WAVE_W	*voice_tbl;
struct WAVE_W	drum_tbl[0x40];
//unsigned char		wavs;			/* ドラム以外の波形の数 */

unsigned long		keyons[SPU_CORE_NUM];
unsigned long		keyoffs[SPU_CORE_NUM];
unsigned long		keyd[SPU_CORE_NUM];

struct CORE_EFCT_WK core_efct_wk;

/*--------------------- 各トラック用ワーク -----------------------------*/
struct SOUND_W		sound_w[SNG_TRACK_NUM+SE_TRACK_NUM];
struct SOUND_W		*sptr;
unsigned long		rev_on_bit[SPU_CORE_NUM];
unsigned long		rev_off_bit[SPU_CORE_NUM];
unsigned long		rev_bit_data[SPU_CORE_NUM];
/*--------------------- Work for IRQ Track ------------------------------*/
unsigned long	irq_addr;
long			spu_irq_counter;
unsigned long	blank_data_addr;

/*--- Mixing Fader -----*/	//1999/12/22
struct MIX_FADER	mix_fader[SNG_TRACK_NUM+SE_TRACK_NUM]; //SEはワークだけ用意（メモリ破壊防止）

/*--- FX Play ---*/	// 2000/01/18
int				fx_sound_code;

/*--- Skip Intro Loop ---*/	// 2000/01/18
int				skip_intro_loop;

/*--- Mixer Automation -----*/	// 2000/01/26 
unsigned int		auto_phase_fg;
unsigned char		auto_env_pos;
//2000/07/07
unsigned char		auto_env_pos2;

/*--- Memory Streaming -----*/// 2000/02/10 
int					mem_str_fg;
char 				mem_str_buf[MEM_STR_BUF_SIZE];
struct MEMSTR_W	mem_str_w[SE_TRACK_NUM];

/*--- Streaming ---*/
//int		vox_rev_on;
int		lnr8_fg;

/*--- Vox Automation -----*/	// 2000/06/29
struct VOX_FADER vox_fader[2];

/*--- 内蔵BGM(SNG)フェーダー音量=0でスタート ---*/
int		fader_off_fg;

/*--- パックファイルのロード ---*/
int				pak_load_status;
int            bp_pak_load_substatus;
int            bp_se_load_substatus;
int            bp_sng_load_substatus;
int            bp_wav_load_substatus;
#if 1 //BP_PS2
void *         bp_pak_fp;
void *         bp_pak_op;
#else
int				pak_fp;
#endif
unsigned int	pak_load_code;
int				pak_read_fg;

/*--- 各種フラグ ---*/	//2000/07/17
int				fg_syukan_off[SNG_TRACK_NUM+SE_TRACK_NUM];
int				fg_rev_set[SNG_TRACK_NUM+SE_TRACK_NUM];

/*--- パス名 ---*/	//2000/07/18
char			path_name[0x80] //BP_GCC__attribute__((aligned(16)))
;
