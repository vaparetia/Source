#include	<stdio.h>
#include	<kernel.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	<sif.h>
#include	<sifcmd.h>
#include	<sifrpc.h>
#include	"../usbfs/usbfs.h"
//BP_PS2 #include	"../jsifman/jsifman.h"
#include	<libsd.h>
#include	"sd_debug.h"
#include	"sd_incl.h"
#include	"sd_ext.h"

#include "mgs_type.h"
#include "BP_EndianSupport.h"
#include "BP_FileSupport.h"
#include "BP_SoundSupport.h"
#include "libfs.h"

char gCurrStageSoundDir[FILENAME_MAX] = "";

//#define WAVSNUM 0x7B		/* 常駐波形の数 1999/11/30 K.Muraoka */<--不要

/*#define MGS */
#define HDD

/*---- 外部宣言 ----*/
extern struct SETBL *jo_header;
extern struct SETBL *se_header;
extern unsigned char *se_exp_table;
extern unsigned char *cdload_buf;
//extern unsigned char cdload_buf[CDLOAD_BUF_SIZE];
extern unsigned char sng_data[SNG_DATA_SIZE];
extern unsigned char wave_header[WAVEHEADERSIZE];
extern int			str2_read_disable[2];
extern int			str2_iop_load_set[2];

/*---- ワークエリア ----*/
unsigned int	gsize=0;
int				pak_read_fg;

int				pak_cd_read_fg = 0;

/*----- EE読み込みアドレス -----*/
struct EE_ADDR ee_addr[2];

/*---- プロトタイプ宣言 ----*/
#if 1 //BP_PS2
void *         PcmOpen (unsigned long sd_code, int kind);
void *         PcmReadAsync (void * fd, void *ptr, int len);
int				PcmLseek ( void * fd, unsigned int offset, int fg);
int				PcmClose (void * fd);
#else
int				PcmOpen (unsigned long sd_code, int kind);
int				PcmRead (int fd, void *ptr, int len);
int				PcmLseek ( int fd, unsigned int offset, int fg);
int				PcmClose (int fd);
#endif
int				LoadWaveFile(void);
void			WaveCdLoad(void);
void			WaveSpuTrans(void);
char			num2char(unsigned long value);
void			code2name (unsigned long code, unsigned char *ptr);
void			str_cat(char *str1, char *str2);

#if 0
#define open( n, m )	pcOpen( n, m )
#define lseek( a, b, c )	pcLseek( a, b, c )
#define read( a, b, c )	pcRead( a, b, c )
#define close( a )		pcClose( a )
#endif

/*========================================================================
*   LOAD PAK FILE
*-------------------------------------------------------------------------
*   OUT: = 0  : Complete
*        Else : Error
=========================================================================*/
struct PAK_HEADER {
	unsigned int		offset;
	unsigned int		code;
};
struct PAK_HEADER	pak_header[0x100];
int		save_wvx1=0xFFFFFFFF, save_wvx2=0xFFFFFFFF, save_efx=0xFFFFFFFF, save_mdx=0xFFFFFFFF;
//int		save_pak=0xFFFFFFFF;

unsigned int size_x;	//wvx、efxの読み込みサイズ
unsigned int pak_sng_code;	// 2000/11/17
unsigned int	pak_jochuu_wave = 0xFF;	// 2001/08/03 0xFF= Dummy File No.
void LoadPakFile()
{
	struct PAK_HEADER	* const p = pak_header;

   printf("BP: LoadPakFile(): %d, %d\n", pak_load_status, bp_pak_load_substatus );
	switch (pak_load_status) {
	case PAK_LOAD_STATUS_LOAD_WVX_1:
// (2000/07/26)ゲームでは同じ音コードでもファイルが異なる仕様なのでコメントアウト
//		if (pak_load_code == save_pak) {
//			pak_load_code = 0;
//			pak_load_status = 0;
//			PRINTF(("LoadPakFile:Same pak is already Loaded.(%x)\n", (unsigned int)pak_load_code));
//			break;
//		}
      //BP - added support for asynchronous loading.
      //This code is not run on a separate processor for us!
      switch( bp_pak_load_substatus )
      {
      case BP_PAK_LOAD_SUBSTATUS_LOAD_HEADER:
         {
      		if (pak_read_fg) {
#if 1 //BP_PS2
               BP_BREAK;
#else
			      PRINTF(("ERROR:PAK File Already Opened.\n"));
			      if (pak_fp) {
				      PcmClose(pak_fp);
				      pak_fp = 0;
			      }
			      pak_read_fg = 0;
#endif
      		}
            /*--------------------- Open PAK File ------------------------------*/
            bp_pak_fp = PcmOpen (pak_load_code, KIND_PAK);
#if 0 //BP_PS2
		      if (pak_fp = <0)
            {
			      WaitVblankStart();	//pak_load_statusがEEに確実に伝わるよう、Waitする
			      WaitVblankEnd();	//2000/10/31
			      WaitVblankStart();	//*
			      WaitVblankEnd();	//*
			      WaitVblankStart();	//*
			      WaitVblankEnd();	//*
			      WaitVblankStart();	//*
			      WaitVblankEnd();	//*
			      pak_load_code = 0;
			      pak_fp = 0;
			      pak_load_status = 0;
			      PRINTF(("LoadPakFile:File Open Error(%x)\n", (unsigned int)pak_load_code));
			      break;
		      } else {
      //			save_pak = pak_load_code;
		      }
#endif
		      pak_read_fg = 1;
		      bp_pak_op = PcmReadAsync (bp_pak_fp, (char *)pak_header, sizeof(pak_header));
            bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_WAITING_HEADER;
         }
         //fallthrough
      case BP_PAK_LOAD_SUBSTATUS_WAITING_HEADER:
         {
            //BP - wait for header read to complete.
            if( !BP_TryFinishFileOp( bp_pak_op ) )
            {
               break;
            }
            bp_pak_op = NULL;
            bp_pak_load_substatus = 0;
            {
               int i;
               for( i=0; i < sizeof(pak_header) / sizeof( *pak_header ); ++i )
               {
                  BP_LE_SwapUInt_Inp( &pak_header[i].offset );
                  BP_LE_SwapUInt_Inp( &pak_header[i].code );
               }
            }
//p[].offsetが0ならば、エラー
#if 0
if (p[0].offset) PRINTF( ( "PAK wvx1 :offset=%X size=%X*800H\n", p[0].offset, p[0].code );
else PRINTF( "***ERROR*** PAK wvx1 :offset=%X size=%X*800H\n", p[0].offset, p[0].code );
if (p[1].offset) PRINTF( "PAK wvx2 :offset=%X size=%X*800H\n", p[1].offset, p[1].code );
else printf( "***ERROR*** PAK wvx2 :offset=%X size=%X*800H\n", p[1].offset, p[1].code );
if (p[2].offset) PRINTF( "PAK efx :offset=%X size=%X*800H\n", p[2].offset, p[2].code );
else printf( "***ERROR*** PAK efx :offset=%X size=%X*800H\n", p[2].offset, p[2].code );
if (p[3].offset) printf( "PAK mdx :offset=%X size=%X*800H\n", p[3].offset, p[3].code );
else printf( "***ERROR*** PAK mdx :offset=%X size=%X*800H\n", p[3].offset, p[3].code );
#endif
//波形ファイル1読み込み
		      if (p[0].offset) {
			      if ( (p[0].code != save_wvx1)&&(p[0].code != save_wvx2)&&(p[0].code != pak_jochuu_wave) ) {
				      size_x = (p[1].offset-p[0].offset)*0x800;//wvxサイズ
				      save_wvx1 = p[0].code;
      //				if ((p[0].code==0x1F) || (p[0].code==0x2F) || (p[0].code==0x5F) || (p[0].code==0x7F) ) pak_jochuu_wave = p[0].code;
				      if ((p[0].code & 0xF) == 0xF) pak_jochuu_wave = p[0].code; //2002/04/16
				      wave_load_code = 0xFEFFFFFE;	//dummy
				      wave_load_status = WAV_LOAD_STATUS_LOAD;
                  if( bp_wav_load_substatus )
                     BP_BREAK;
                  bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOAD;
				      pak_load_status = PAK_LOAD_STATUS_WAITING_WVX_1;
			      } else {
				      PcmLseek(bp_pak_fp, (p[1].offset-p[0].offset)*0x800, SEEK_CUR);
				      pak_load_status = PAK_LOAD_STATUS_LOAD_WVX_2;
				      PRINTF(("PAK Load:wvx1 Skip!(wvx=%x)\n", p[0].code));
			      }
		      } else {
			      pak_load_status = PAK_LOAD_STATUS_CLOSE;
      //			pak_load_status = 3;
      //			PRINTF(("PAK Load:No wvx1 data... Skipped!\n"));
		      }
         }
         break; //bp_pak_load_substatus 2
      default:
         BP_BREAK;
      }  //switch (bp_pak_load_substatus)
		break; //pak_load_status 1
	case PAK_LOAD_STATUS_WAITING_WVX_1://波形ロード1完了待ち
		if (wave_load_status == WAV_LOAD_STATUS_NONE) {
			pak_load_status = PAK_LOAD_STATUS_LOAD_WVX_2;
		}
			break;
	case PAK_LOAD_STATUS_LOAD_WVX_2:
//波形ファイル2読み込み
		if (p[1].offset) {
			if ( (p[1].code != save_wvx1)&&(p[1].code != save_wvx2)&&(p[1].code != 0xFF)) {
				size_x = (p[2].offset-p[1].offset)*0x800;//wvxサイズ
				save_wvx2 = p[1].code;
				wave_load_code = 0xFEFFFFFF;	//dummy
				wave_load_status = WAV_LOAD_STATUS_LOAD;
            if( bp_wav_load_substatus )
               BP_BREAK;
            bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOAD;
				pak_load_status = PAK_LOAD_STATUS_WAITING_WVX_2;
			} else {
				PcmLseek(bp_pak_fp, (p[2].offset-p[1].offset)*0x800, SEEK_CUR);
				pak_load_status = PAK_LOAD_STATUS_LOAD_SE;
            bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_LOAD_SE;
				PRINTF(("PAK Load:wvx2 Skip!(wvx=%x)\n", p[1].code));
			}
		} else {
			pak_load_status = PAK_LOAD_STATUS_CLOSE;
//			pak_load_status = 5;
//			PRINTF(("PAK Load:No wvx2 data... Skipped!\n"));
		}
		break;
	case PAK_LOAD_STATUS_WAITING_WVX_2://波形ロード2完了待ち
		if (wave_load_status == WAV_LOAD_STATUS_NONE)
      {
         pak_load_status = PAK_LOAD_STATUS_LOAD_SE;
         bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_LOAD_SE;
      }
      else
      {
   		break;
      }
      //fallthrough
//効果音ファイルのロード
	case PAK_LOAD_STATUS_LOAD_SE:
      switch( bp_pak_load_substatus )
      {
      case BP_PAK_LOAD_SUBSTATUS_LOAD_SE:
         {
            int skip_load = 0;
		      if (p[2].offset)
            {
			      if ((p[2].code != save_efx) && (p[2].code != 0xFF))
               {
      /*DEBUG*/PRINTF(("Start EFX READ (%x)\n", p[2].code));
				      save_efx = p[2].code;

                  //BP - the SE loading is "inlined" here so this stale code setting se_load_code to a dummy value
                  //has been removed since it would have side effects now that we're loading this step asyncrhonously.
                  //(This was presumably intended to allow LoadSeFile to perform this step but it's not doing that)
				      //se_load_code = 0x02FFFFFF;	//dummy

                  //BP - read se_exp_table.
				      bp_pak_op = PcmReadAsync (bp_pak_fp, se_exp_table, SE_EXP_TABLE_SIZE);//0x800 常駐SEをロードすると、拡張波形テーブル(.ztb)が壊れる
                  bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_WAITING_SEEXP;
			      }
               else
               {
				      PcmLseek(bp_pak_fp, (p[3].offset-p[2].offset)*0x800, SEEK_CUR);
				      PRINTF(("PAK Load:efx Skip!(efx=%x)\n", p[2].code));
                  skip_load = 1;
			      }
		      }
            else
            {
			      PRINTF(("PAK Load:No efx data...Skipped!\n"));
               skip_load = 1;
		      }
            if( skip_load )
            {
               pak_load_status = PAK_LOAD_STATUS_LOAD_BGM;
               break;
            }
         } //bp_pak_load_substatus 1
         //fallthrough
      case BP_PAK_LOAD_SUBSTATUS_WAITING_SEEXP:
         {
            //BP - wait for se_exp_table read to complete.
            if( !BP_TryFinishFileOp( bp_pak_op ) )
            {
               break;
            }
            //BP - read jo_header or se_header.
            if ((se_exp_table[0]==0xFE) && (se_exp_table[1]==0xFE)) {	//常駐SEファイルのテーブル(.ztb)は、0xFEで埋められている
               bp_pak_op = PcmReadAsync (bp_pak_fp, jo_header, ((p[3].offset-(p[2].offset+1))*0x800));
            } else {
               bp_pak_op = PcmReadAsync (bp_pak_fp, se_header, ((p[3].offset-(p[2].offset+1))*0x800));
            }
            //				PcmRead (pak_fp, se_exp_table, ((p[3].offset-p[2].offset)*0x800));//LoadSEFileはファイルサイズを間違うので使わない

            //BP removed (see above)
            //se_load_code = 0x0;

            bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_WAITING_JO_SE;
         }
         //fallthrough
      case BP_PAK_LOAD_SUBSTATUS_WAITING_JO_SE:
         {
            //BP - wait for jo_header or se_header read to complete.
            if( !BP_TryFinishFileOp( bp_pak_op ) )
            {
               break;
            }
            bp_pak_op = NULL;
            bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_NONE;
            pak_load_status = PAK_LOAD_STATUS_LOAD_BGM;
         }
         break;
      default:
         BP_BREAK;
      } //switch (bp_pak_load_substatus)
		break;
	case PAK_LOAD_STATUS_LOAD_BGM:
//ＢＧＭファイルのロード
		if (p[3].offset) {//ＢＧＭファイル
			if ((p[3].code != save_mdx)&&(p[3].code != 0xFF)) {
/*DEBUG*/PRINTF(("Start MDX READ (%x)\n", p[3].code));

/* 2000/11/17 */
					save_mdx = p[3].code;
					pak_sng_code = 0x010000FF;	//dummy
					pak_load_status = PAK_LOAD_STATUS_WAIT_BGM_LOAD_START;
//				if (sd_sng_code_buf[sd_code_set] == 0) {
//					save_mdx = p[3].code;
//					sd_sng_code_buf[sd_code_set] = 0x010000FF;	//dummy
//					sd_code_set = (sd_code_set+1) & 0xF;
//					pak_load_status = 7;
//				} else {
//					PRINTF(("***TooMuchBGMSoundCode(LoadPakFile)***\n"));
//					break;
//				}

			} else {
				pak_load_status = PAK_LOAD_STATUS_CLOSE;
				PRINTF(("PAK Load:mdx Skip!(mdx=%x)\n", p[3].code));
				break;
			}
		} else {
			pak_load_status = PAK_LOAD_STATUS_CLOSE;
			PRINTF(("PAK Load:No mdx data...Skipped!\n"));
			break;
		}
		break;
	case PAK_LOAD_STATUS_WAIT_BGM_LOAD_START://ＢＧＭロード開始待ち
		if (pak_sng_code) break;	//pak_sng_codeが0になるまで(=sng_load_codeがセットされるまで)待つ
//		if (sng_load_code == 0) break;
		else pak_load_status = PAK_LOAD_STATUS_WAIT_BGM_LOAD;
		break;
	case PAK_LOAD_STATUS_WAIT_BGM_LOAD://ＢＧＭロード完了待ち
		if (sng_load_code) break;
		else pak_load_status = PAK_LOAD_STATUS_CLOSE;
		break;
	case PAK_LOAD_STATUS_CLOSE:
//終了処理
		pak_read_fg = 0;
		PcmClose(bp_pak_fp);
		bp_pak_fp = 0;
		pak_load_status = PAK_LOAD_STATUS_NONE;
      bp_pak_load_substatus = BP_PAK_LOAD_SUBSTATUS_NONE;
/*DEBUG*/PRINTF(("Complete PAK LOAD \n"));
		break;
	}
/*DEBUG*/	PRINTF(("pak_load_status=%x(%x)\n", pak_load_status, pak_read_fg));
}
/*========================================================================
*   LOAD SE FILE
*-------------------------------------------------------------------------
*   OUT: = 0  : Complete
*        Else : Error
=========================================================================*/
int LoadSeFile(void)
{
   printf("BP: LoadSeFile(): %d\n", bp_se_load_substatus );
   switch( bp_se_load_substatus )
   {
   case BP_SE_LOAD_SUBSTATUS_LOAD_SEEXP:
      {
	      if (bp_se_fp) {
		      PRINTF(("ERROR:SE File Already Opened.\n"));
/*DEBUG*/	/* *((long*)1)=0;*/
		      PcmClose(bp_se_fp);
		      bp_se_fp = 0;
	      }
/*--------------------- Open WAVE File ------------------------------*/
         bp_se_fp = PcmOpen (se_load_code, KIND_EFX);
#if 0 //BP_PS2
	      if ((se_fp) <0) {
		      se_load_code = 0;
		      se_fp = 0;
		      PRINTF(("LoadSeFile:File Open Error(%x)\n", (unsigned int)se_load_code));
		      return(-1);
	      }
#endif
/*--------------------- Load SE Table & Data ------------------------*/
         if( bp_pak_op )
            BP_BREAK;

         //BP - read se_exp_table.
	      bp_pak_op = PcmReadAsync (bp_se_fp, se_exp_table, SE_EXP_TABLE_SIZE);//0x800 常駐SEをロードすると、拡張波形テーブル(.ztb)が壊れる
         bp_se_load_substatus = BP_SE_LOAD_SUBSTATUS_WAITING_SEEXP;
      }
      //fallthrough
   case BP_SE_LOAD_SUBSTATUS_WAITING_SEEXP:
      {
         //BP - wait for se_exp_table read to complete.
         if( !BP_TryFinishFileOp( bp_pak_op ) )
         {
            break;
         }

         //BP - read jo_header or se_header.
	      if ((se_exp_table[0]==0xFE) && (se_exp_table[1]==0xFE)) {	//常駐SEファイルのテーブル(.ztb)は、0xFEで埋められている
		      bp_pak_op = PcmReadAsync (bp_se_fp, jo_header, SEHEADERSIZE+SE_DATA_SIZE);//0x1000+0x6000
	      } else {
		      bp_pak_op = PcmReadAsync (bp_se_fp, se_header, SEHEADERSIZE+SE_DATA_SIZE);//0x1000+0x6000
	      }
         bp_se_load_substatus = BP_SE_LOAD_SUBSTATUS_WAITING_JO_SE;
      }
      //fallthrough
   case BP_SE_LOAD_SUBSTATUS_WAITING_JO_SE:
      {
         //BP - wait for jo_header or se_header read to complete.
         if( !BP_TryFinishFileOp( bp_pak_op ) )
         {
            break;
         }
         bp_pak_op = NULL;
         PcmClose(bp_se_fp);
         se_load_code = 0;
         bp_se_fp = 0;
         bp_se_load_substatus = BP_SE_LOAD_SUBSTATUS_NONE;
      }
      break;
   default:
      BP_BREAK;
   }
   return(0);
}

/*-------------------------------------------------------------------*/
/* ＳＮＧデータのＣＤ読み込みを開始									 */
/*-------------------------------------------------------------------*/
int LoadSngData()
{
//BP removed /*DEBUG*/unsigned int	sz;
/* PRINTF(("SNG Load (Main-1)\n"));*/
   printf("BP: LoadSngData(): %d, %d\n", bp_sng_load_substatus );
   switch( bp_sng_load_substatus )
   {
   case BP_SNG_LOAD_SUBSTATUS_LOAD:
      {
/*--------------------- ＳＮＧファイルのオープン --------------------*/
         bp_sng_fp = PcmOpen (sng_load_code, KIND_MDX);
#if 0 //BP_PS2
	      if ((sng_fp) <0) {
		      sng_load_code = 0;
		      sng_fp = 0;
		      PRINTF(("LoadSngData:File Open Error(%x)\n", (unsigned int)sng_load_code));
      /*		reply_sound (&reply_sng, ERR_SNG_OPEN, sng_load_code);*/
		      return(-1);
	      }
#endif
/*--------------------- ＳＮＧファイル読み込み ----------------------*/
         if( bp_pak_op )
            BP_BREAK;
      	bp_pak_op = PcmReadAsync (bp_sng_fp, sng_data, SNG_DATA_SIZE);
//BP removed /*DEBUG*/PRINTF(("SNG DATA SIZE = %X\n", sz));
         bp_sng_load_substatus = BP_SNG_LOAD_SUBSTATUS_WAITING_LOAD;
      }
      //fallthrough
   case BP_SNG_LOAD_SUBSTATUS_WAITING_LOAD:
      {
         //BP - wait for sng read to complete.
         if( !BP_TryFinishFileOp( bp_pak_op ) )
         {
            break;
         }
         //PRINTF(("SNG NUM=%x\n", sng_data[0]));
         bp_pak_op = NULL;
         PcmClose(bp_sng_fp);
         sng_load_code = 0;		//1999/12/13 K.Muraoka
         bp_sng_fp = 0;
         //PRINTF(("CompletedLoadSong(%x)\n", (unsigned int)sng_load_code));
         bp_sng_load_substatus = BP_SNG_LOAD_SUBSTATUS_NONE;
      }
      break;
   default:
      BP_BREAK;
   }
	return(0);
}

/*========================================================================
* 音色テーブルのセット
* 注意：
*ドラム設定は常駐(.wvxファイルの最初の4Byteが0)ファイルにのみ含まれるものとする
=========================================================================*/

void set_voice_tbl(struct WAVE_W *p, unsigned int size, unsigned int offset) {
	unsigned int	i;
	int				drum_on=0;
	signed int		addr_new, addr_old=-1;

PRINTF(("size=%x\n", size));
	for (i = (offset/0x10); i < (size/0x10); i++) {
//ドラムか音色かチェックする
		addr_new = (signed int)( BP_LE_SwapUInt( p[i].addr_le ) );
		if (drum_on == 0) {
			if (addr_new < (addr_old)) {
				drum_on = 1;
				break;
			}
			else addr_old = addr_new;
		}
	}
	memcpy( (char *)((u_int)voice_tbl+offset), p, i*0x10 );
	if(drum_on) memcpy( drum_tbl, &p[i], size-(i*0x10) );
}

/*========================================================================
*   LOAD WAVE FILE
*-------------------------------------------------------------------------
*   OUT: = 0  : SoundRAM Data
*         -1  : Error
*          1  : IOP Memory Streaming Data
=========================================================================*/

int LoadWaveFile(void)
{
   if( wave_load_status != WAV_LOAD_STATUS_LOAD )
      BP_BREAK;

   switch( bp_wav_load_substatus )
   {
   case BP_WAV_LOAD_SUBSTATUS_LOAD:
      {
         unsigned int	first_read_size;
      /*DEBUG*/PRINTF(("Start LoadWave(.wvx)File\n"));

#if 1 //BP_PS2
         if( bp_wave_fp )
            BP_BREAK;
#else
	      if (wave_fp) {
		      PRINTF(("ERROR:Wave File Already Opened.\n"));
/*DEBUG*/	/* *((long*)1)=0;*/
		      PcmClose(wave_fp);
		      wave_fp = 0;
      	}
#endif
/*--------------------- Open WAVE File ----------------------------------*/
         bp_wave_fp = PcmOpen (wave_load_code, KIND_WVX);
#if 0 //BP_PS2
	      if (wave_fp <0) {
		      wave_fp = 0;
		      PRINTF(("LoadWaveFile:File Open Error(%x)\n", (unsigned int)wave_load_code));

			      WaitVblankStart();	//pak_load_statusがEEに確実に伝わるよう、Waitする
			      WaitVblankEnd();	//2000/10/31
			      WaitVblankStart();	//*
			      WaitVblankEnd();	//*
			      WaitVblankStart();	//*
			      WaitVblankEnd();	//*
			      WaitVblankStart();	//*
			      WaitVblankEnd();	//*

		      wave_load_code = 0;
		      return(-1);
	      }
#endif
/*--------------------- Load WAVE Table --------------------------------*/
	      if (wave_load_code < 0xFEFFFFFE)
         {
		      first_read_size = CDLOAD_BUF_SIZE;
///*DEBUG*/PRINTF(("<<<1>>>\n"));
	      }
         else
         {
		      if (size_x < CDLOAD_BUF_SIZE)
            {
			      first_read_size = size_x;
///*DEBUG*/PRINTF(("<<<2>>>\n"));
		      }
            else
            {
			      first_read_size = CDLOAD_BUF_SIZE;
///*DEBUG*/PRINTF(("<<<3>>>\n"));
      		}
      	}
         if( bp_pak_op )
            BP_BREAK;
	      bp_pak_op = PcmReadAsync (bp_wave_fp, cdload_buf, first_read_size);
         bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOADING_TABLE;
      } //BP_WAV_LOAD_SUBSTATUS_LOAD
      //fallthrough
   case BP_WAV_LOAD_SUBSTATUS_LOADING_TABLE:
      {
         unsigned int	offset, size;
         unsigned int	mem_str_idx;	// 2000/02/10
         //BP - wait for wav table load to complete.
         if( !BP_TryFinishFileOp( bp_pak_op ) )
         {
            break;
         }
         bp_pak_op = NULL;

//	wavs = WAVSNUM;		/* 常駐波形の数 */
	      offset = ((unsigned long)cdload_buf[0]) << 24; //音色テーブルオフセットAddr.
	      offset |= ((unsigned long)cdload_buf[1]) << 16;
	      offset |= ((unsigned long)cdload_buf[2]) << 8;
	      offset |= (unsigned long)cdload_buf[3];
	      size = ((unsigned long)cdload_buf[4]) << 24; //ヘッダーサイズ
	      size |= ((unsigned long)cdload_buf[5]) << 16;
	      size |= ((unsigned long)cdload_buf[6]) << 8;
	      size |= (unsigned long)cdload_buf[7];
/*DEBUG*/	PRINTF(("SUP HEADER OFFSET=%x:HEADER SIZE=%x\n", (unsigned int)offset, (unsigned int)size));
      	wave_load_ptr = &cdload_buf[0x10];
//	dst_ptr = &wave_header[offset];
//	memcpy(dst_ptr, wave_load_ptr, size);
//音色テーブル・セット
      	set_voice_tbl((struct WAVE_W *)wave_load_ptr, size, offset);
/*--------------------- Load SPU Data ----------------------------------*/
	      wave_load_ptr += size;
	      spu_load_offset = ((unsigned long)*wave_load_ptr) << 24;
	      spu_load_offset |= ((unsigned long)*(wave_load_ptr+1) ) << 16;
	      spu_load_offset |= ((unsigned long)*(wave_load_ptr+2) ) << 8;
	      spu_load_offset |= (unsigned long)*(wave_load_ptr+3);
         bp_base_spu_load_offset = spu_load_offset;
	      wave_unload_size = ((unsigned long)*(wave_load_ptr+4) ) << 24;
	      wave_unload_size |= ((unsigned long)*(wave_load_ptr+5) ) << 16;
	      wave_unload_size |= ((unsigned long)*(wave_load_ptr+6) ) << 8;
	      wave_unload_size |= (unsigned long)*(wave_load_ptr+7);
///*DEBUG*/	PRINTF(("CDREAD BUFFER ADDRESS=%x\n", (unsigned int)wave_load_ptr));
/*DEBUG*/	PRINTF(("SPU OFFSET=%x:SIZE=%x\n", (unsigned int)spu_load_offset, (unsigned int)wave_unload_size));
	      wave_load_ptr += 0x10;
	      if (wave_unload_size > (CDLOAD_BUF_SIZE-(size+0x20))) {
		      wave_load_size = CDLOAD_BUF_SIZE-(size+0x20);
	      } else {
		      wave_load_size = wave_unload_size;
	      }
	      wave_unload_size -= wave_load_size;
/*--------------------- 効果音ワーク初期化 ----------------------------*/
      	wave_save_code = wave_load_code;

      	if (offset >= 0x1000)
         {
            //BP - iop / memory stream.
            //Changed this to load all at once (don't need to go through cdload_buf)
		      memcpy(mem_str_buf, wave_load_ptr, wave_load_size);
		      mem_str_idx = wave_load_size;

            if (mem_str_idx + wave_unload_size > MEM_STR_BUF_SIZE)
            {
               BP_BREAK;
            }
            if( wave_unload_size )
            {
               bp_pak_op = PcmReadAsync( bp_wave_fp, &mem_str_buf[mem_str_idx], wave_unload_size );
               wave_unload_size = 0;
               bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOADING_WAVS;
            }
            else
            {
               PcmClose(bp_wave_fp);
               bp_wave_fp = 0;
               wave_load_code = 0;
               bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_NONE;
               //BP - done loading IOP waveform data; no further processing needed.
               return(1);
            }
      	}
         else
         {
            //BP - need to transfer buffered waveform data to SPU before loading more.
            //Further wav loading will be done via WaveCdLoad().
            bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_NONE;
		      return(0);
      	}
         bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOADING_WAVS;
      } //BP_WAV_LOAD_SUBSTATUS_LOADING_TABLE
      //fallthrough
   case BP_WAV_LOAD_SUBSTATUS_LOADING_WAVS:
      {
         //BP - wait for iop wav load to complete.
         if( !BP_TryFinishFileOp( bp_pak_op ) )
         {
            break;
         }
         bp_pak_op = NULL;

         PcmClose(bp_wave_fp);
         bp_wave_fp = 0;
         wave_load_code = 0;
         bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_NONE;
         //BP - done loading IOP waveform data; no further processing needed.
         return(1);
      }
      break;
   default:
      BP_BREAK;
   }//switch (bp_wav_load_substatus)

   //still in WAV_LOAD_STATUS_LOAD.
   return 2;
}

/*========================================================================
*   LOAD WAVE DATA FROM CD-ROM (波形分割転送用)
=========================================================================*/
void WaveCdLoad(void) {
#if 0 //BP_PS2
/*DEBUG*/WaitVblankEnd();	//2000/01/24 ネットワークではファイルリードが
/*DEBUG*/WaitVblankStart();	//SPU転送を追い越す症状がある為、ウェイトする
/*DEBUG*/WaitVblankEnd();	//CD/DVDアクセス時は無効にする事
/*DEBUG*/WaitVblankStart();
/*DEBUG*/WaitVblankEnd();
/*DEBUG*/WaitVblankStart();
/*DEBUG*/WaitVblankEnd();
#endif
   if( wave_load_status != WAV_LOAD_STATUS_CD_LOAD )
      BP_BREAK;

   printf("BP: WaveCdLoad: %d\n", bp_wav_load_substatus );
   switch( bp_wav_load_substatus )
   {
   case BP_WAV_LOAD_SUBSTATUS_LOAD:
      {
         //BP - can't trivially bypass cdload_buf because WaveSpuTrans() copies from it.
         //This is a multi-step load.
         if( bp_pak_op )
            BP_BREAK;

	      if (wave_unload_size > CDLOAD_BUF_SIZE)
         {
            printf("BP: WaveCdLoad READ %d (%d)\n", CDLOAD_BUF_SIZE, wave_unload_size );
		      bp_pak_op = PcmReadAsync (bp_wave_fp, cdload_buf, CDLOAD_BUF_SIZE);
		      wave_load_ptr = cdload_buf;
		      wave_load_size = CDLOAD_BUF_SIZE;
		      wave_unload_size -= CDLOAD_BUF_SIZE;
            bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOADING_WAVS;
	      }
         else
         {
		      if (wave_unload_size)
            {
               printf("BP: WaveCdLoad READ LAST %d\n", wave_unload_size );
			      bp_pak_op = PcmReadAsync (bp_wave_fp, cdload_buf, wave_unload_size );
			      wave_load_ptr = cdload_buf;
			      wave_load_size = wave_unload_size;
			      wave_unload_size = 0;
               bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOADING_WAVS;
		      }
            else
            {
               //BP - final SPU transfer is complete so we don't need to do any more CD loading.
               printf("BP: WaveCdLoad COMPLETE\n" );
               //Decode the entire range of SPU memory at once to prevent discontinuities at transfer boundaries.
               BP_DecodeSpuMemory( spu_wave_start_ptr+bp_base_spu_load_offset, spu_load_offset - bp_base_spu_load_offset );
			      wave_load_status = WAV_LOAD_STATUS_NONE;
               bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_NONE;
			      PcmClose(bp_wave_fp);
      PRINTF(("Complete Load Wave:%x\n", (unsigned int)wave_load_code));
			      bp_wave_fp = 0;
			      wave_load_code = 0;	/*1999/12/13 K.Muraoka /Status Read for EE */
               break;
		      }
	      }
      }
      //fallthrough
   case BP_WAV_LOAD_SUBSTATUS_LOADING_WAVS:
      {
         //BP - wait for spu wav load to complete.
         if( !BP_TryFinishFileOp( bp_pak_op ) )
         {
            break;
         }
         bp_pak_op = NULL;
         //Next step is to load into SPU memory.
         wave_load_status = WAV_LOAD_STATUS_SPU_TRANS;
         //WavSpuTrans() will set substatus back into BP_WAV_LOAD_SUBSTATUS_LOAD.
         bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_NONE;
      }
      break;
   default:
      BP_BREAK;
   }
}

/*========================================================================
*   TRANSFER S.E. DATA TO SPU
=========================================================================*/

void WaveSpuTrans(void)
{
	/* 結局使わない 1998/03/17*/
	int	size;

	if (wave_load_status == WAV_LOAD_STATUS_SPU_TRANS)
   {
      printf("BP: WaveSpuTrans offset %d sz %d\n", spu_wave_start_ptr+spu_load_offset, wave_load_size );
      //BP - disable decoding ADPCM data in SPU memory on the fly.  We must decode all at once
      //when the entire body of waveform data is done transferring.
      gBP_DecodeSpuMemoryOnTransfer = 0;
		size = BP_sceSdVoiceTrans(
			0,
//			SD_TRANS_MODE_WRITE | SD_TRANS_BY_IO, //libsdのバグが治るまで
			SD_TRANS_MODE_WRITE | SD_TRANS_BY_DMA,
			wave_load_ptr,
			(spu_wave_start_ptr+spu_load_offset),
			(unsigned int)wave_load_size
			);
      //BP - re-enable
      gBP_DecodeSpuMemoryOnTransfer = 1;
		if (size < 0) PRINTF(("ERROR:SPU TRANSFER.\n"));
		else {
			PRINTF(("SPU Trans Base=%x Offset=%x Size=%x\n", (u_int)spu_wave_start_ptr, (u_int)spu_load_offset, size));
			spu_load_offset += wave_load_size;
		}
		wave_load_status = WAV_LOAD_STATUS_CD_LOAD;
      if( bp_wav_load_substatus )
         BP_BREAK;
      bp_wav_load_substatus = BP_WAV_LOAD_SUBSTATUS_LOAD;
	}
}

/*========================================================================
*   Convert Sound Code to File Name
=========================================================================*/

void code2name (unsigned long code, unsigned char *ptr)
{
/* SNG Load & Play (0x01000000 - 0x0100FFFF) */
	if (( code >= 0x01000000L) && (code <= 0x0100FFFFL)) {
		*(ptr) = 's';
		*(ptr+1) = 'g';
		*(ptr+2)  = num2char ( (code>>20)&0xF );
		*(ptr+3)  = num2char ( (code>>16)&0xF );
		*(ptr+4)  = num2char ( (code>>12)&0xF );
		*(ptr+5)  = num2char ( (code>>8)&0xF );
		*(ptr+6)  = num2char ( (code>>4)&0xF );
		*(ptr+7)  = num2char ( code&0xF );
		*(ptr+8)  = '.';
		*(ptr+9)  = 'm';
		*(ptr+10)  = 'd';
		*(ptr+11)  = 'x';
		*(ptr+12)  = '\0';
		return;
	}
/* SE Load & Play (0x02000000 - 0x0200FFFF) */
	if (( code >= 0x02000000L) && (code <= 0x0200FFFFL)) {
		*(ptr) = 's';
		*(ptr+1) = 'e';
		*(ptr+2)  = num2char ( (code>>20)&0xF );
		*(ptr+3)  = num2char ( (code>>16)&0xF );
		*(ptr+4)  = num2char ( (code>>12)&0xF );
		*(ptr+5)  = num2char ( (code>>8)&0xF );
		*(ptr+6)  = num2char ( (code>>4)&0xF );
		*(ptr+7)  = num2char ( code&0xF );
		*(ptr+8)  = '.';
		*(ptr+9)  = 'e';
		*(ptr+10)  = 'f';
		*(ptr+11)  = 'x';
		*(ptr+12)  = '\0';
		return;
	}
/* STR Load & Play (0xF0000000 - 0xF0FFFFFF) */
	if (( code >= 0xF0000000L) && (code <= 0xF0FFFFFFL)) {
		*(ptr) = 'v';
		*(ptr+1) = 'c';
		*(ptr+2)  = num2char ( (code>>20)&0xF );
		*(ptr+3)  = num2char ( (code>>16)&0xF );
		*(ptr+4)  = num2char ( (code>>12)&0xF );
		*(ptr+5)  = num2char ( (code>>8)&0xF );
		*(ptr+6)  = num2char ( (code>>4)&0xF );
		*(ptr+7)  = num2char ( code&0xF );
		*(ptr+8)  = '.';
		*(ptr+9)  = 'p';
		*(ptr+10)  = 'c';
		*(ptr+11)  = 'm';
		*(ptr+12)  = '\0';
	}
/* WAVE Load (0xFE000000 - 0xFE00FFFF)*/
	if (( code >= 0xFE000000L) && (code <= 0xFE00FFFFL)) {
		*(ptr) = 'w';
		*(ptr+1) = 'v';
		*(ptr+2)  = num2char ( (code>>20)&0xF );
		*(ptr+3)  = num2char ( (code>>16)&0xF );
		*(ptr+4)  = num2char ( (code>>12)&0xF );
		*(ptr+5)  = num2char ( (code>>8)&0xF );
		*(ptr+6)  = num2char ( (code>>4)&0xF );
		*(ptr+7)  = num2char ( code&0xF );
		*(ptr+8)  = '.';
		*(ptr+9)  = 'w';
		*(ptr+10)  = 'v';
		*(ptr+11)  = 'x';
		*(ptr+12)  = '\0';
	}
	if (( code >= 0xFE800000L) && (code <= 0xFEFFFFFFL)) {
#ifdef USA_VERSION
		*(ptr) = 'u';
		*(ptr+1) = 's';
#else
	#ifdef EUROPE_VERSION
		*(ptr) = 'e';
		*(ptr+1) = 'u';
	#else
		*(ptr) = 'p';
		*(ptr+1) = 'k';
	#endif
#endif
//		*(ptr) = 'p';
//		*(ptr+1) = 'k';
		*(ptr+2)  = num2char ( (code>>20)&0x7 );
		*(ptr+3)  = num2char ( (code>>16)&0xF );
		*(ptr+4)  = num2char ( (code>>12)&0xF );
		*(ptr+5)  = num2char ( (code>>8)&0xF );
		*(ptr+6)  = num2char ( (code>>4)&0xF );
		*(ptr+7)  = num2char ( code&0xF );
		*(ptr+8)  = '.';
		*(ptr+9)  = 's';
		*(ptr+10)  = 'd';
		*(ptr+11)  = 'x';
		*(ptr+12)  = '\0';
	}
}

/*========================================================================
*   Convert Numeric to Character
=========================================================================*/

char num2char(unsigned long value)
{
	value &= 0x0F;
	if (value <= 9) value += '0';
	else value += ('a'- 0x0A);
	return((char)value);
}

void str_cat(char *str1, char *str2)
{
	int		i;

	for (i=0;i<0x100;i++){
		if(str1[i] == '\0') break;
	}
	if (i < 0x100) strcpy(&str1[i], str2);
}

/*========================================================================
*   PCM FILE OPEN
*-------------------------------------------------------------------------
*   IN: (sd_code)= Sound Code
*   OUT: File Hadle (-1 = Error)
=========================================================================*/
char *sd_path_cd1[] = {
		 "\\DUMMY\\",
#if 0
		  "sim:/usr/local/develop/mgs2sd/sim_iop/vox1/",
		  "sim:/usr/local/develop/mgs2sd/sim_iop/wvx1/",
		  "sim:/usr/local/develop/mgs2sd/sim_iop/mdx1/",
		  "sim:/usr/local/develop/mgs2sd/sim_iop/efx1/"
#else
		  "host0:./sound/vox1/",
		  "host0:./sound/wvx1/",
		  "host0:./sound/mdx1/",
		  "host0:./sound/efx1/",
		  "host0:./sound/sdx1/"
#endif
};
//#ifndef HDD
//CdlFILE		file_dummy, file_str, file_wvx, file_mdx, file_efx;
//CdlFILE		*sd_fp_cd1[] = { &file_dummy, &file_str, &file_wvx, &file_mdx, &file_efx };
//#endif

void BP_BuildSoundPakFilename( char * path, int code )
{
   const char * const regionFolder =
#if 1
   //JP and EU sound paks match US ones so we just keep one copy of them in the repository
   //and in the archives.
      "us";
#else
      FS_BP_GetCurrentRegionFolder();
#endif
   sprintf( path, "%s/stage/%s/pk%06x.sdx", regionFolder, gCurrStageSoundDir, code );
}

/* CDから読み取るためのワーク */
static int pakcd_pos;
///*DEBUG*/ int rd_sd;
#if 1 //BP_PS2
void * PcmOpen (unsigned long sd_code, int kind)
{
   char file_name[0x80] = "";
   void * fd;

   if (pak_read_fg) {	//パックファイルロード中
      return(bp_pak_fp);
   }

   if( kind != KIND_PAK && kind != KIND_WVX )
      BP_TODO_BREAK;

   BP_BuildSoundPakFilename( file_name, sd_code );

   fd = BP_OpenFile( file_name, NULL );
   return fd;
}
#else
int PcmOpen (unsigned long sd_code, int kind)
{
	int		fd;
	unsigned char file_name[0x80];
	unsigned char name[16];

	if (pak_read_fg) {	//パックファイルロード中
		return(pak_fp);
	}
#if 1	// add K.Uehara
	if( pak_cd_read_fg ){
		// CDMODE
		cdOpen();
		pakcd_pos = pak_load_code;
PRINTF(( "pak cd read start %d\n", pakcd_pos ));
		return 1;
	}
#endif
	if ( (kind != KIND_PAK) || (path_name[0] == 0) ) strcpy(file_name, sd_path_cd1[kind]);
	else strcpy(file_name, path_name);
	code2name(sd_code, name);
	str_cat(file_name, name);
/*DEBUG*/if (!pak_read_fg) PRINTF(("HDD Open:FileName=%s\n", file_name));
	cdOpen();//2002/09/10
	fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		printf("PcmOpen Error(%x)\n", fd);
		cdClose();
	}
///*DEBUG*/rd_sd = 0;
	return(fd);
}
#endif

/*========================================================================
*   PCM FILE READ
*-------------------------------------------------------------------------
*   IN: (fd)     = File Handle
*       (ptr)   = Buffer Address
*       (len)    = Length
*   OUT: Read Size
=========================================================================*/
#if 1 //BP_PS2
void * PcmReadAsync (void * fd, void *ptr, int len)
{
   void * bp_op;
   len = (len+0x7FF) & 0xFFFFF800;	//2000/07/13 0x800単位で読む

   bp_op = BP_ReadFileAsync( fd, ptr, len );
   return bp_op;
}
#else
int PcmRead (int fd, void *ptr, int len)
{
	int			err;

	len = (len+0x7FF) & 0xFFFFF800;	//2000/07/13 0x800単位で読む

#if 1	// add K.Uehara
	if( pak_cd_read_fg ){
		// CDMODE
		cdRead( pakcd_pos, ptr, len );
		pakcd_pos += len / 0x800;
		return len;
	}
#endif

	err = read(fd, ptr, len);
///*DEBUG*/PRINTF(("rd_sd=%x/read=%x\n", rd_sd, err));
///*DEBUG*/rd_sd += err;
	if (err <= 0) printf("PcmRead Error(%x:size=%x)\n", err, len);
	return(err);
}
#endif

#if 1 //BP_PS2
int PcmLseek ( void * fd, unsigned int offset, int fg)
{
   BP_SeekFile( fd, offset, fg );
   return 0;   //return value not used by MGS2
}
#else
int PcmLseek ( int fd, unsigned int offset, int fg)
{
	int			err;
#if 1	// add K.Uehara
	if( pak_cd_read_fg ){
		// CDMODE
		if( fg == SEEK_CUR ){
			pakcd_pos += offset / 0x800;
//printf( "seek to %d\n", pakcd_pos );
		}
		return fd;
	}
#endif

	err = lseek(fd, offset, fg);
	if (err < 0) {
		printf("CD Seek Error(%x)\n", err);
		return(err);
	} else return(fd);
}
#endif

/*========================================================================
*   PCM FILE CLOSE
*-------------------------------------------------------------------------
*   IN: (fd)     = File Handle
*   OUT: = 0  : Complete
*        Else : Error
=========================================================================*/
#if 1 //BP_PS2
int PcmClose( void * fd )
{
   if( pak_cd_read_fg ){
      if (pak_read_fg == 0) {	//パックファイル読み込み中でなければ
         PRINTF(("CD Close\n"));
         BP_CloseFile( fd );
      }
   }
   return 0;
}
#else
int PcmClose (int fd)
{
	int			err = 0;
#if 1	// add K.Uehara
	if( pak_cd_read_fg ){
		if (pak_read_fg == 0) {	//パックファイル読み込み中でなければ
PRINTF(("CD Close\n"));
			cdClose();
		}
		return err;
	}
#endif
	if (pak_read_fg == 0) {	//パックファイル読み込み中でなければ
PRINTF(("HDD Close\n"));
		err = close(fd);
			cdClose();	//2002/09/09
		if (err < 0) printf("PcmClose Error(%x)\n", err);
	}
	return(err);
}
#endif
/*========================================================================
*   EEからの読み込み関数
=========================================================================*/
/*----- EEファイルのオープン -----*/
//   IN: (sd_code)= Sound Code
//   OUT: File Hadle (-1 = Error)
int EEOpen (unsigned int sd_code)
{
	int				fd = 0;
	struct EE_ADDR	*p;
	if ( (sd_code&0xFF000000) == 0xF5000000 ) {	//ストリーミング1
		fd = 1;
	} else {
		if ( (sd_code&0xFF000000) == 0xF4000000 ) {	//ストリーミング2
			fd = 2;
		} else {
			PRINTF(("ERROR:EEOpen:(SdCode=%x)\n", sd_code));
         BP_BREAK;
		}
	}
	p = &ee_addr[fd-1];
//	if (p->base == 0) {
//		PRINTF(("ERROR:EEOpen:NoBaseAddress!!(SdCode=%x)\n", sd_code));
//	} else {
//		p->ofst[0]= p->ofst[1] = 0;
//		p->read_ctr = 0;
PRINTF(("EE Open:No=%x(fd=%x)\n", sd_code, (fd-1) ));
//	}
	return(fd);
}

/*----- EEファイルの読み込み -----*/
//   IN: (fd)     = File Handle
//       (idx)   = Buffer offset
//       (len)    = Length
//   OUT: Read Size
int EERead (int fd, unsigned char *ptr, int idx, int len)
{
#if 1 //BP_PS2
   struct EE_ADDR	*p;

   if ((fd != 1) && (fd != 2)) {
      PRINTF(("ERROR:EE File Read(fd=%x)\n", fd));
      str2_iop_load_set[fd-1] = 0;
      return (0);
   }
   p = &ee_addr[fd-1];

   if (p->set_ctr <= p->read_ctr) {
      if (p->set_ctr < p->read_ctr) PRINTF(("ERROR:EERead:NoOffset!!(%x:%x)\n", p->set_ctr, p->read_ctr));
      str2_iop_load_set[fd-1] = 0;
      return (0);
   }

#if 1 //BP_PS2 (see bp_cli_ptr_param)
   memcpy( ptr, (unsigned char *)((p->ofst[0])), len );
#else
   memcpy( ptr, (unsigned char *)((p->ofst[0])<<4), len );
#endif

   if (p->set_ctr) p->read_ctr++;

   str2_iop_load_set[fd-1] = 0;
   return (len);

#else
	SIF_RV_QUEUE	*que;
	struct EE_ADDR	*p;
/*DEBUG*/	int				debug_ctr = 0;

	if ((fd != 1) && (fd != 2)) {
		PRINTF(("ERROR:EE File Read(fd=%x)\n", fd));
		str2_iop_load_set[fd-1] = 0;
		return (0);
	}
	p = &ee_addr[fd-1];
//	if (p->base == 0) {
//		PRINTF(("ERROR:EERead:NoBaseAddress!!(fd=%x)\n", (u_int)p->base));
//		return (0);
//	}

	if (p->set_ctr <= p->read_ctr) {
		if (p->set_ctr < p->read_ctr) PRINTF(("ERROR:EERead:NoOffset!!(%x:%x)\n", p->set_ctr, p->read_ctr));
		str2_iop_load_set[fd-1] = 0;
		return (0);
	}

//	if (str2_read_disable[0] && (idx==0)) {
///*DEBUG*/PRINTF(("Str2[0]:Read Disable\n"));
//		return (0);//sd_setされてからStartEEStream
//	}
//	if (str2_read_disable[1] && (idx==1)) {
///*DEBUG*/PRINTF(("Str2[1]:Read Disable\n"));
//		return (0);//までは読み込みしない
//	}

///*DEBUG*/PRINTF(("EEDataOfst=%x(idx=%x)\n", p->ofst[idx], idx));
//	que = sif_get_mem(ptr, (unsigned char *)(p->base+p->ofst[idx]), len );
//	que = sif_get_mem(ptr, (unsigned char *)((p->ofst[idx])<<4), len );
	que = sif_get_mem(ptr, (unsigned char *)((p->ofst[0])<<4), len );
	while (1) {
//		WaitVblankStart();
//		WaitVblankEnd();
///*DEBUG*/PRINTF(("NowLoadingEEData(%x)\n", que->id));
		BP_DelayThread( 10000 );
		if (que->id & 0x80000000) break;
/*DEBUG*/	debug_ctr++;
/*DEBUG*/	if ((debug_ctr&0xFFFF) == 0) {
/*DEBUG*/		PRINTF(("*** EE File Read:WAITING FOR GET MEM ***\n"));
/*DEBUG*/	} 
	}
	if (p->set_ctr) p->read_ctr++;
	sif_rv_release_queue(que);

	str2_iop_load_set[fd-1] = 0;
//PRINTF(("fd=%x\n", fd));
	return (len);
#endif
}

/*=======================================================================*/
/*===================== End on File =====================================*/
/*===================== DON'T ADD STUFF AFTER THIS ======================*/
/*=======================================================================*/
