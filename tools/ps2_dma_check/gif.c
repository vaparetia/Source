/*
	gif.c
	GIF解析＆エミュレーションルーチン

	2001/10/19 K.Takabe
	$Id: gif.c,v 1.1 2001/10/22 06:00:49 usr02774 Exp $
*/


#include <stdio.h>
#include "app.h"
#include "dmacheck.h"

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
static int reg_description2num[16] = {
	GS_PRIM,GS_RGBAQ,GS_ST,GS_UV,GS_XYZF2,GS_XYZ2,GS_TEX0_1,GS_TEX0_2,
	GS_CLAMP_1,GS_CLAMP_2,GS_FOG,GS_ERROR,GS_XYZF3,GS_XYZ3,GS_AD,GS_NOP
};
/* ---------------------------------------------------------------- */
/* 文字列テーブル */
static char		*str_gif_flg[4] = {"PACKED ","REGLIST","IMAGE ","ERROR!"};
static char		*str_regname[16] = {
	"PRIM   ","RGBAQ  ","ST     ","UV     ","XYZF2  ","XYZ2   ","TEX0_1 ","TEX0_2 ",
	"CLAMP_1","CLAMP_2","FOG    ","-      ","XYZF3  ","XYZ3   ","A+D    ","NOP    "
};
static char		*str_regs[] = {
	/* 0x00 */
	"PRIM","RGBAQ","ST","UV",
	"XYZF2","XYZ2","TEX0_1","TEX0_2",
	"CLAMP_1","CLAMP_2","FOG","",
	"XYZF3","XYZ3","","",
	/* 0x10 */
	"","","","",
	"TEX1_1","TEX1_2","TEX2_1","TEX2_2",
	"XYOFFSET_1","XYOFFSET_2","PRMODECONT","PRMODE",
	"TEXCLUT","","","",
	/* 0x20 */
	"","","SCANMSK","",
	"","","","",
	"","","","",
	"","","","",
	/* 0x30 */
	"","","","",
	"MIPTBP1_1","MIPTBP1_2","MIPTBP2_1","MIPTBP2_2",
	"","","","TEXA",
	"","FOGCOL","","TEXGLUSH",
	/* 0x40 */
	"SCISSOR_1","SCISSOR_2","ALPHA_1","ALPHA_2",
	"DIMX","DTHE","COLCLAMP","TEST_1",
	"TEST_2","PABE","FBA_1","FBA_2",
	"FRAME_1","FRAME_2","ZBUF_1","ZBUF_2",
	/* 0x50 */
	"BITBLTBUF","TRXPOS","TRXREG","TRXDIR",
	"HWREG","","","",
	"","","","",
	"","","","",
	/* 0x60 */
	"SIGNAL","FINISH","LABEL","",
	"","","","",
	"","","","",
	"","","","",
};
/* ---------------------------------------------------------------- */
/* GIFを初期化 */
void InitGif( GifWork *gif, int flag )
{
	gif->flag = flag ;
	gif->skip_count = 0 ;
}
/* ---------------------------------------------------------------- */
void ExecGif( GifWork *gif, unsigned int data[4] )
{
	int			i ;

	if ( gif->skip_count == 0 ){

		/* GIFタグを要素に分解する */
		gif->nloop = data[0] & 0x7fff ;
		gif->eop = ( data[0] >> 15 ) & 0x0001 ;
		gif->pre = ( data[1] >> 14 ) & 0x0001 ;
		gif->prim = ( data[1] >> 15 ) & 0x03ff ;
		gif->flg = ( data[1] >> 26 ) & 0x0003 ;
		gif->nreg = ( data[1] >> 28 ) & 0x000f ;
		for ( i = 0 ; i < 8 ; i++ ){
			gif->regs[i] = ( data[2] >> ( i*4 ) ) & 0x0f ;
			gif->regs[i+8] = ( data[3] >> ( i*4 ) ) & 0x0f ;
		}

		/* GIFタグ内容の表示 */
		if ( gif->flag & DMA_FLAG_DISP_GIF ){
			printf("  <GIFtag>NLOOP:%3d EOP:%1d PRE:%1d PRIM:%03x FLG:%6s NREG:%d\n",
				   gif->nloop, gif->eop, gif->pre, gif->prim, str_gif_flg[gif->flg], gif->nreg );
			if ( gif->flg == 0 || gif->flag == 1 ){
				/* レジスタリストの表示 */
				printf("          ");
				for ( i = 0 ; i < gif->nreg ; i++ ){
					printf("%s ", str_regname[gif->regs[i]] );
					if ( ( i != ( gif->nreg - 1 ) ) && ( ( i & 3 ) == 3 ) ) printf("\n          ");
				}
				printf("\n");
			}
		}

		/* 転送サイズ計算 */
		switch ( gif->flg ){
		case 0:/* PACKED */
			gif->skip_count = gif->nloop * gif->nreg ;
			break ;
		case 1:/* REGLIST */
			gif->skip_count = ( gif->nloop * gif->nreg + 1 ) / 2 ;
			break ;
		case 2:/* IMAGE */
			gif->skip_count = gif->nloop ;
			break ;
		case 3:/* ERROR!! */
			exit(0);
			break ;
		}
		gif->reg_count = 0 ;
	} else {
		int		reg_num, reg_description ;
		switch ( gif->flg ){
		case 0:/* PACKED */
			/* レジスタ内容の表示 */
			reg_description = gif->regs[ gif->reg_count ];
			reg_num = reg_description2num[ reg_description ];
			if ( reg_num == GS_AD ) reg_num = data[2] ;
			if ( reg_num < 0x63 ){
				if ( gif->flag & DMA_FLAG_DISP_GS ){
					printf("          (%10s) %08x %08x\n", str_regs[reg_num], data[1], data[0] );
				}
			}
			gif->reg_count++ ;
			if ( gif->reg_count >= gif->nreg ){
				gif->reg_count = 0 ;
				gif->nloop-- ;
			}
			break ;
		case 1:/* REGLIST */
			/* 前半 */
			reg_description = gif->regs[ gif->reg_count ];
			reg_num = reg_description2num[ reg_description ];
			if ( reg_num < 0x63 ){
				if ( gif->flag & DMA_FLAG_DISP_GS ){
					printf("          (%10s) %08x %08x\n", str_regs[reg_num], data[1], data[0] );
				}
			}
			gif->reg_count++ ;
			if ( gif->reg_count >= gif->nreg ){
				gif->reg_count = 0 ;
				gif->nloop-- ;
			}
			/* 後半 */
			reg_description = gif->regs[ gif->reg_count ];
			reg_num = reg_description2num[ reg_description ];
			if ( reg_num < 0x63 ){
				if ( gif->flag & DMA_FLAG_DISP_GS ){
					printf("          (%10s) %08x %08x\n", str_regs[reg_num], data[3], data[2] );
				}
			}
			gif->reg_count++ ;
			if ( gif->reg_count >= gif->nreg ){
				gif->reg_count = 0 ;
				gif->nloop-- ;
			}
			break ;
		case 2:
			break ;
		}
		gif->skip_count-- ;
	}

}
