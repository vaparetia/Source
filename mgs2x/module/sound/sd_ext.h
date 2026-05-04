/*===================================================================
 *  Title           : "METALGEAR SOLID" SOUND
 *                  : -  ワークエリア外部参照 -
 *  version         : 1.00
 *
 *  Code            : mj001
 *  Filename        : SD_EXT.H
 *  Creator         : K.C.E JAPAN - K.Muraoka
 *  First edition   : 1996/11/13
 *
 *  Note            : 
 *                  : 
 *===================================================================*/

/*---------------------- 波形ロード ---------------------------------*/

extern int				wave_loaded;

/*--------------------- ＳＮＧ再生 ----------------------------------*/

//extern unsigned char	*sng_data;			/*ＳＮＧデータの先頭へのポインタ*/
extern unsigned long	sng_load_code, sng_play_code;
extern int				sng_status;	/*ＳＮＧ再生状況*/
#if 1 //BP_PS2
extern void *        bp_sng_fp;
#else
extern int				sng_fp;
#endif
extern int				sng_pause_fg;
extern int				int_pause_fg;

extern long				sng_fadein_time;
extern long				sng_fade_time[SNG_TRACK_NUM];
extern long				sng_fade_value[SNG_TRACK_NUM];
extern long				sng_fadein_fg;
extern int				sng_fout_fg;
extern int				sng_fout_term[SPU_CORE_NUM];
extern int				sng_master_vol[SNG_TRACK_NUM];

//extern int					sng_tempo_move;	//1999/12/22 for Mixer Fader
extern int				sng_kaihi_fg;

extern long				sng_syukan_vol;
extern int				sng_syukan_fg;

extern long				sng_mic_vol;
extern int				sng_mic_fg;

extern int					sd_sng_code_buf[0x10];		/*音コード一時保存*/
extern int					sd_code_set, sd_code_read;

extern int				vox_on_vol;
/*--------------------- For PCM Read ------------------------------------*/

//extern int				sd_alloc_size;
//extern int				act_fg;
//extern char				pcm_path[0x80];

//extern int				pcm_file_status[3];

/*--------------------- Sound Mono/Stereo Flag --------------------------*/
extern int				sound_mono_fg;
/*--------------------- For Sound Effect --------------------------------*/

//extern unsigned char	*cdload_buf;
extern unsigned long	se_load_code;
extern unsigned long	se_save_code2;
//extern unsigned char	*se_data;			/*ＳＥデータの先頭へのポインタ*/
//extern struct SETBL	*se_header;
extern struct SEPLAYTBL	se_playing[SE_TRACK_NUM];
extern struct SEPLAYTBL	se_request[SE_TRACK_NUM];
extern int				se_pan[SE_TRACK_NUM];
extern int				se_vol[SE_TRACK_NUM];
extern float         se_bp_angle[SE_TRACK_NUM];
extern int				se_bp_surround_type[SE_TRACK_NUM];
#if 1 //BP_PS2
extern void *        bp_se_fp;
#else
extern int				se_fp;
#endif
extern int				se_rev_on;
extern int				stop_jizoku_se;
extern int				stop_jizoku_se2;

/*---------------------- For Wave Load ------------------------------*/
extern unsigned long	spu_wave_start_ptr;
//extern unsigned char	*wave_header;
extern unsigned long	wave_load_status;
extern unsigned long	wave_load_code;
extern unsigned long	wave_save_code;
extern unsigned long	wave_unload_size;
extern unsigned long	wave_load_size;
#if 1 //BP_PS2
extern void *        bp_wave_fp;
#else
extern int				wave_fp;
#endif
extern unsigned char	*wave_load_ptr;
extern unsigned long	bp_base_spu_load_offset;
extern unsigned long	spu_load_offset;

/*--------------------- From PSKERO ------------------------------------*/
/*extern unsigned long		sound_code;*/
extern unsigned long		mdata1,mdata2,mdata3,mdata4;
extern unsigned long		key_fg;
extern unsigned long		mtrack;
extern unsigned char		*mptr;
extern unsigned long 		song_end[SPU_CORE_NUM];
extern struct SPU_TRACK_REG	spu_tr_wk[INTERNAL_TRACK_NUM];
extern struct WAVE_W		*voice_tbl;
extern struct WAVE_W	drum_tbl[0x40];
extern struct SOUND_W		sound_w[SNG_TRACK_NUM+SE_TRACK_NUM], *sptr;
//extern unsigned char		wavs;			/* ドラム以外の波形の数 */

extern unsigned long		keyons[SPU_CORE_NUM];
extern unsigned long		keyoffs[SPU_CORE_NUM];
extern unsigned long		keyd[SPU_CORE_NUM];

extern struct CORE_EFCT_WK core_efct_wk;

/*--------------------- 各トラック用ワーク -----------------------------*/

extern struct SOUND_W		*sptr;
extern unsigned long		rev_on_bit[SPU_CORE_NUM];
extern unsigned long		rev_off_bit[SPU_CORE_NUM];
extern unsigned long		rev_bit_data[SPU_CORE_NUM];

/*--- Mixing Fader -----*/	//1999/12/22
extern struct MIX_FADER	mix_fader[SNG_TRACK_NUM+SE_TRACK_NUM];

/*--- FX Play ---*/	// 2000/01/18
extern int				fx_sound_code;

/*--- Skip Intro Loop ---*/	// 2000/01/18
extern int				skip_intro_loop;

/*--- Mixer Automation -----*/	// 2000/01/26 
extern unsigned int		auto_phase_fg;
extern unsigned char		auto_env_pos;
//2000/07/07
extern unsigned char		auto_env_pos2;

/*--- Memory Streaming -----*/// 2000/02/10 
extern int					mem_str_fg;
extern char				mem_str_buf[MEM_STR_BUF_SIZE];
extern struct MEMSTR_W	mem_str_w[SE_TRACK_NUM];

/*--- Streaming ---*/
//extern int		vox_rev_on;
extern int		lnr8_fg;

/*--- Vox Automation -----*/	// 2000/06/29
extern struct VOX_FADER vox_fader[2];

/*--- 内蔵BGM(SNG)フェーダー音量=0でスタート ---*/
extern int		fader_off_fg;

/*--- パックファイルのロード ---*/
extern int				pak_load_status;
extern int           bp_pak_load_substatus;
extern int           bp_se_load_substatus;
extern int           bp_sng_load_substatus;
extern int           bp_wav_load_substatus;
#if 1 //BP_PS2
extern void *        bp_pak_fp;
extern void *        bp_pak_op;
#else
extern int				pak_fp;
#endif
extern unsigned int	pak_load_code;
extern int				pak_read_fg;
extern int				pak_cd_read_fg;

/*--- 各種フラグ ---*/	//2000/07/17
extern int				fg_syukan_off[SNG_TRACK_NUM+SE_TRACK_NUM];
extern int				fg_rev_set[SNG_TRACK_NUM+SE_TRACK_NUM];

/*--- パス名 ---*/	//2000/07/18
extern char			path_name[0x80];

//BP added enum for ease of understandability
enum
{
   PAK_LOAD_STATUS_NONE                = 0,
   PAK_LOAD_STATUS_LOAD_WVX_1          = 1,
   PAK_LOAD_STATUS_WAITING_WVX_1       = 2,
   PAK_LOAD_STATUS_LOAD_WVX_2          = 3,
   PAK_LOAD_STATUS_WAITING_WVX_2       = 4,
   PAK_LOAD_STATUS_LOAD_SE             = 5,
   PAK_LOAD_STATUS_LOAD_BGM            = 6,
   PAK_LOAD_STATUS_WAIT_BGM_LOAD_START = 7,
   PAK_LOAD_STATUS_WAIT_BGM_LOAD       = 8,
   PAK_LOAD_UNIFIED                    = 9,
   PAK_LOAD_STATUS_CLOSE               = 10
};

enum
{
   BP_PAK_LOAD_SUBSTATUS_NONE          = 0,
   BP_PAK_LOAD_SUBSTATUS_LOAD_HEADER   = 1,
   BP_PAK_LOAD_SUBSTATUS_WAITING_HEADER= 2,
   BP_PAK_LOAD_SUBSTATUS_LOAD_SE       = 3,
   BP_PAK_LOAD_SUBSTATUS_WAITING_SEEXP = 4,
   BP_PAK_LOAD_SUBSTATUS_WAITING_JO_SE = 5
};

enum
{
   BP_SE_LOAD_SUBSTATUS_NONE           = 0,
   BP_SE_LOAD_SUBSTATUS_LOAD_SEEXP     = 1,
   BP_SE_LOAD_SUBSTATUS_WAITING_SEEXP  = 2,
   BP_SE_LOAD_SUBSTATUS_WAITING_JO_SE  = 3
};

enum
{
   BP_SNG_LOAD_SUBSTATUS_NONE          = 0,
   BP_SNG_LOAD_SUBSTATUS_LOAD          = 1,
   BP_SNG_LOAD_SUBSTATUS_WAITING_LOAD  = 2
};

enum
{
   BP_WAV_LOAD_SUBSTATUS_NONE          = 0,
   BP_WAV_LOAD_SUBSTATUS_LOAD          = 1,
   BP_WAV_LOAD_SUBSTATUS_LOADING_TABLE = 2,
   BP_WAV_LOAD_SUBSTATUS_LOADING_WAVS  = 3,
};

enum
{
   WAV_LOAD_STATUS_NONE                = 0,
   WAV_LOAD_STATUS_LOAD                = 1,
   WAV_LOAD_STATUS_SPU_TRANS           = 2,
   WAV_LOAD_STATUS_CD_LOAD             = 3
};