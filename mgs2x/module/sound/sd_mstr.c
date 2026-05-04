#include	<stdio.h>
#include	<kernel.h>
#include	<sys/types.h>
#include	<sif.h>
#include	<sifcmd.h>
//#include	<sifrpc.h>
#include	<libsd.h>
//#include	<libspu2.h>
#include	"sd_debug.h"
#include	"sd_incl.h"
#include	"sd_ext.h"

#include "BP_EndianSupport.h"
#include "BP_SoundSupport.h"

#define SPU_PLAY_BUF_SIZE	0x800	/*SPU再生バッファのサイズ(BGM)*/

extern void	keyon(void);

void init_mem_str_w(void)
{
	int i;

	for (i=0;i<SE_TRACK_NUM;i++) mem_str_w[i].status=0;
}

//==========================================================
// メモリストリーミング
//----------------------------------------------------------
// IN:i= トラックNo.
//==========================================================

int MemSpuTransWithNoLoop(int i)
{
	int		dma_fg = 0;
	unsigned int	play_now;
	struct MEMSTR_W	*p;
	unsigned short env;

	i -= SNG_TRACK_NUM;
	p = &mem_str_w[i];
/*--------------------- Transfer BGM Data -------------------------------*/
	switch (p->status&0xF) {
#if 1 //BP_PS2
      //Circumvent this entire state sequence because we can just play back the waveform from non-SPU RAM.
      //Updates are not consistent enough for us that the original code would work all the time in any case.
   case 2:
      //Initialize playback.
      if( p->bp_override_wav_addr )
      {
         p->wave_size = p->unplay_size = p->bp_override_wav_size;
         p->play_offset = p->bp_override_wav_addr;
      }
      else
      {
         p->wave_size = p->unplay_size = BP_LE_SwapUInt(voice_tbl[(p->snos)+1].addr_le) - BP_LE_SwapUInt(voice_tbl[p->snos].addr_le);
         p->play_offset = &mem_str_buf[BP_LE_SwapUInt(voice_tbl[p->snos].addr_le)];//転送元アドレス・オフセット
      }

      //以下２行で、VAGデータを書き換えている
      *( ((p->play_offset)+ 0x11) ) = 0; //波形データに含まれるループデータを削除
      *( (p->play_offset)+((p->wave_size)-0x1F) ) = 1; //波形エンドでループOff
      //English memory streams have the magic PS2 vag end marker at the end-- remove it to prevent an audible pop.
      BP_SanitizeVAG( (unsigned char*)p->play_offset, p->wave_size );
      //Okay to send now.
      BP_PlayMemStream( SD_CORE_1, i+SE_CORE1_OFFSET, (unsigned char*)p->play_offset, p->wave_size );

      //Prevent normal SSA from being set for this voice.
      spu_tr_wk[i+SNG_TRACK_NUM].addr = 0;
      spu_tr_wk[i+SNG_TRACK_NUM].addr_fg = 0;

      keyon();
      ++(p->status);
      dma_fg = 1;
      break;
   case 3:
      {
         //Check play position and volume envelope to see if we should end.
         play_now = BP_GetMemStreamPosition(SD_CORE_1, i+SE_CORE1_OFFSET);
         env = BP_sceSdGetParam(SD_CORE_1 | SD_VP_ENVX | ( (i+SE_CORE1_OFFSET) <<1));
         if ( env == 0 || play_now >= p->wave_size )
         {
            ++(p->status);
         }
      }
      break;
   case 4:
      //Just in case there's any significance to 7 being the last state...
      p->status = 7;
      break;
   case 7:
      break;
#else //BP_PS2
	case 2:
		p->wave_size = p->unplay_size = BP_LE_SwapUInt(voice_tbl[(p->snos)+1].addr_le) - BP_LE_SwapUInt(voice_tbl[p->snos].addr_le);
		p->play_offset = &mem_str_buf[BP_LE_SwapUInt(voice_tbl[p->snos].addr_le)];//転送元アドレス・オフセット

//以下２行で、VAGデータを書き換えている
		*( ((p->play_offset)+ 0x11) ) = 0; //波形データに含まれるループデータを削除
		*( (p->play_offset)+((p->wave_size)-0x1F) ) = 1; //波形エンドでループOff

//		*((p->play_offset)+1) |= 4;//ループスタート設定
		BP_sceSdSetAddr( SD_CORE_1|( (i+SE_CORE1_OFFSET)<<1 )|SD_VA_LSAX, MEM_SPU_START_PTR+(i*0x1000) );
		BP_sceSdVoiceTrans( DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
			(unsigned char*)p->play_offset,
			MEM_SPU_START_PTR+(i*0x1000), (unsigned int)SPU_PLAY_BUF_SIZE );
		p->play_offset += SPU_PLAY_BUF_SIZE;	//転送元アドレス・オフセット
		p->unplay_size -= SPU_PLAY_BUF_SIZE;	//転送残りサイズ
		++(p->status);
		dma_fg = 1;
		break;
	case 3:
		if ( (p->unplay_size == 0) || (p->unplay_size&0x80000000) ) ++(p->status);
		else {
			if (p->unplay_size > SPU_PLAY_BUF_SIZE) {
				if ( *(p->play_offset+(SPU_PLAY_BUF_SIZE-0xF)) != 1)
					*(p->play_offset+(SPU_PLAY_BUF_SIZE-0xF)) |= 3;//ループエンド設定
			}
			BP_sceSdVoiceTrans( DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
				(unsigned char*)p->play_offset,
				(MEM_SPU_START_PTR+(i*0x1000)+SPU_PLAY_BUF_SIZE),
				(unsigned int)SPU_PLAY_BUF_SIZE );
			p->play_offset += SPU_PLAY_BUF_SIZE;	//転送元アドレス・オフセット
			p->unplay_size -= SPU_PLAY_BUF_SIZE;	//転送残りサイズ
			++(p->status);
			dma_fg = 1;
		}
		break;
	case 4:
		BP_sceSdSetAddr ( SD_CORE_1|( (i+SE_CORE1_OFFSET) <<1)|SD_VA_SSA , MEM_SPU_START_PTR+(i*0x1000) );
		keyon();
		p->next_idx = SPU_PLAY_BUF_SIZE;	//転送先ワークインデックス(0,SPU_PLAY_BUF_SIZE)
		++(p->status);

		if ( (p->unplay_size == 0) || (p->unplay_size&0x80000000) ) {
			++(p->status);
		}
		break;
	case 5:
/*--- 転送途中でKeyOffされた(ENV値=0)時は、転送を止める ---*/
		env = BP_sceSdGetParam(SD_CORE_1 | SD_VP_ENVX | ( (i+SE_CORE1_OFFSET) <<1));
//PRINTF(("env=%x\n", env));
		if ( env == 0) {
			++(p->status);
		}

		play_now = BP_sceSdGetAddr(SD_VA_NAX | SD_CORE_1 | ( (i+SE_CORE1_OFFSET) << 1));
		play_now = (play_now - (MEM_SPU_START_PTR+(i*0x1000)));
		if (play_now >= (SPU_PLAY_BUF_SIZE*2)) {
			PRINTF(("ERROR:MemoryStreamingAddress(%x)\n", play_now));
			break;
		}
		if ( p->next_idx==(play_now&SPU_PLAY_BUF_SIZE) ) {
/*--- 次の波形を転送 ---*/
			dma_fg = 1;
			if (play_now >= SPU_PLAY_BUF_SIZE) { /*後半再生中は前半分を転送*/
//PRINTF(("MAE"));
//				*((p->play_offset)+1) |= 4;//ループスタート設定
				BP_sceSdVoiceTrans( DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
					(unsigned char*)p->play_offset,
					(MEM_SPU_START_PTR+(i*0x1000)),
					(unsigned int)SPU_PLAY_BUF_SIZE );
				p->next_idx = 0;
			} else {								/*前半再生中は後半分を転送*/
//PRINTF(("ATO"));
				if (p->unplay_size > SPU_PLAY_BUF_SIZE) {
					if ( *(p->play_offset+(SPU_PLAY_BUF_SIZE-0xF)) != 1)
						*(p->play_offset+(SPU_PLAY_BUF_SIZE-0xF)) |= 3;//ループエンド設定
				}
				BP_sceSdVoiceTrans( DMA_CH, SD_TRANS_MODE_WRITE|SD_TRANS_BY_DMA,
					(unsigned char*)p->play_offset,
					(MEM_SPU_START_PTR+(i*0x1000)+SPU_PLAY_BUF_SIZE),
					(unsigned int)SPU_PLAY_BUF_SIZE );
				p->next_idx = SPU_PLAY_BUF_SIZE;
			}
			p->play_offset = p->play_offset+SPU_PLAY_BUF_SIZE;
/*--- 波形データ完了チェック ---*/
			if (p->unplay_size > SPU_PLAY_BUF_SIZE) {
				p->unplay_size -= SPU_PLAY_BUF_SIZE;
//PRINTF(("(%x)\n", p->unplay_size));
			} else {
//PRINTF(("...Complete\n"));
				++(p->status);
			}
		} else {
//PRINTF(("-%x : %x\n", p->next_idx, play_now));
		}
		break;
	case 6:
		++(p->status);
		break;
	case 7:
		break;
#endif //BP_PS2
	}
	return (dma_fg);
}
