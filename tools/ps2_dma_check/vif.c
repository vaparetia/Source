/*
	vif.c
	VIF解析＆エミュレーションルーチン

	2000/06/30 K.Takabe
	$Id: vif.c,v 1.3 2002/12/21 02:41:46 usr02774 Exp $
*/

#include <stdio.h>
#include "app.h"
#include "dmacheck.h"

/* VIFCODEからインデックスに変換するためのテーブル */
static int	vifcode_list[21] = {
	VIFCODE_NOP,VIFCODE_STCYCL,VIFCODE_OFFSET,VIFCODE_BASE,
	VIFCODE_ITOP,VIFCODE_STMOD,VIFCODE_MSKPATH3,VIFCODE_MARK,
	VIFCODE_FLUSHE,VIFCODE_FLUSH,VIFCODE_FLUSHA,VIFCODE_MSCAL,
	VIFCODE_MSCALF,VIFCODE_MSCNT,VIFCODE_STMASK,VIFCODE_STROW,
	VIFCODE_STCOL,VIFCODE_MPG,VIFCODE_DIRECT,VIFCODE_DIRECTHL,VIFCODE_UNPACK
};


/* ---------------------------------------------------------------- */
/* VIFCODEを解析 */
void CheckVifcode( unsigned int data, char *output )
{
	static char	*vifcode_CMD[22] = {
		"NOP     ","STCYCL  ","OFFSET  ","BASE    ",
		"ITOP    ","STMOD   ","MSKPATH3","MARK    ",
		"FLUSHE  ","FLUSH   ","FLUSHA  ","MSCAL   ",
		"MSCALF  ","MSCNT   ","STMASK  ","STROW   ",
		"STCOL   ","MPG     ","DIRECT  ","DIRECTHL",
		"UNPACK  ","UNPACKR "
	};
	static char	*unpack_mode_vn[] = { "S-","V2-","V3-","V4-" };
	static char	*unpack_mode_vl[] = { "32","16","8","5" };
	int		cmd, num, imm, intrp, i, cmd_num ;

	/* VIFCODEを要素に分解 */
	intrp = ( data >> 31 ) & 1 ;
	cmd = ( data >> 24 ) & 0x7f ;
	num = ( data >> 16 ) & 0xff ;
	imm = ( data >> 0 ) & 0xffff ;

	/* VIFCODEをインデックスに変換 */
	for ( i = 0 ; i < 20 ; i++ ){
		if ( cmd == vifcode_list[i] ) break ;
	}
	cmd_num = i ;

	/* 解析文字列作成 */
	if ( cmd_num < 20 ){
		/* VIF_UNPACK以外 */

		sprintf( output, "%cVIFCODE:%s NUM:%3d IMM:%6d",
				 intrp ? '*' : ' ',
				 vifcode_CMD[cmd_num],
				 num,
				 imm );

	} else {
		/* VIF_UNPACKの時 */

		/* 相対／絶対アドレス指定チェック */
		if ( imm & 0x8000 ) cmd_num++ ;

		sprintf( output, "%cVIFCODE:%s NUM:%3d IMM:%6d ( ADDR:%08x %s%s%s %s )",
				 intrp ? '*' : ' ',
				 vifcode_CMD[cmd_num],
				 num,
				 imm, 

				 imm & 0x3ff,
				 unpack_mode_vn[(cmd>>2)&3],
				 ( imm & 0x4000 ) ? "U" : "",
				 unpack_mode_vl[(cmd)&3],
				 ( cmd & 0x10 ) ? "MASKED" : "" );

	}

}


/* ---------------------------------------------------------------- */
static void VifExec_NOP( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_STCYCL( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_OFFSET( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_BASE( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_ITOP( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_STMOD( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_MSKPATH3( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_MARK( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_FLUSHE( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_FLUSH( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_FLUSHA( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_MSCAL( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_MSCALF( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_MSCNT( VifWork *vif )
{
	vif->skip_count = 0 ;
}
static void VifExec_STMASK( VifWork *vif )
{
	vif->skip_count = 1 ;
	vif->buffer_fill_count = 0 ;
}
static void VifExec_STROW( VifWork *vif )
{
	vif->skip_count = 4 ;
	vif->buffer_fill_count = 0 ;
}
static void VifExec_STCOL( VifWork *vif )
{
	vif->skip_count = 4 ;
	vif->buffer_fill_count = 0 ;
}
static void VifExec_MPG( VifWork *vif )
{
	if ( vif->num == 0 ) vif->num = 256 ;
	vif->skip_count = vif->num * 2 ;
	vif->buffer_fill_count = 0 ;
}
static void VifExec_DIRECT( VifWork *vif )
{
	if ( vif->num == 0 ) vif->num = 256 ;
	vif->skip_count = vif->immediate * 4 ;
	vif->buffer_fill_count = 0 ;
}
static void VifExec_DIRECTHL( VifWork *vif )
{
	if ( vif->num == 0 ) vif->num = 256 ;
	vif->skip_count = vif->immediate * 4 ;
	vif->buffer_fill_count = 0 ;
}
static void VifExec_UNPACK( VifWork *vif )
{
}

static void (*vif_exec_list[])( VifWork * ) =  {
	VifExec_NOP,
	VifExec_STCYCL,
	VifExec_OFFSET,
	VifExec_BASE,
	VifExec_ITOP,
	VifExec_STMOD,
	VifExec_MSKPATH3,
	VifExec_MARK,
	VifExec_FLUSHE,
	VifExec_FLUSH,
	VifExec_FLUSHA,
	VifExec_MSCAL,
	VifExec_MSCALF,
	VifExec_MSCNT,
	VifExec_STMASK,
	VifExec_STROW,
	VifExec_STCOL,
	VifExec_MPG,
	VifExec_DIRECT,
	VifExec_DIRECTHL,
	VifExec_UNPACK,
};

/* ---------------------------------------------------------------- */
/* VIFを初期化 */
void InitVif( VifWork *vif, int flag )
{
	vif->flag = flag ;
	vif->skip_count = 0 ;
	vif->buffer_fill_count = 0 ;
	InitGif( &vif->gif_work, flag );
}

/* VIFを実行 */
void ExecVif( VifWork *vif, unsigned int data )
{
	int		i ;

	/* 動作チェック */
	if ( vif->flag & VIF_FLAG_STOP ) return ;

	if ( vif->skip_count == 0 ){
		/* VIFCODEを要素に分解 */
		vif->code = data ;
		vif->cmd = ( data >> 24 ) & 0x7f ;
		vif->num = ( data >> 16 ) & 0xff ;
		vif->immediate = ( data >> 0 ) & 0xffff ;

		/* VIFCODEをインデックスに変換 */
		for ( i = 0 ; i < 20 ; i++ ){
			if ( vif->cmd == vifcode_list[i] ) break ;
		}
		vif->cmd_index = i ;

		if ( vif->flag & DMA_FLAG_DISP_VIF ){
			char	out_buffer[256] ;
			CheckVifcode( vif->code, out_buffer );
			printf("  <%08x> %s\n", vif->code, out_buffer );
		}

		if ( vif->cmd_index < 20 ){
			/* VIF_UNPACK以外 */
			(*vif_exec_list[vif->cmd_index])( vif );

		} else {
			/* 不整コードチェック */
			if ( ( vif->cmd & 0x60 ) != 0x60 ){
				printf("UNPACK: vertex format error!!\n");
				vif->flag |= VIF_FLAG_STOP ;
				return ;
			}

			/* VIF_UNPACKの時 */
			vif->count = vif->num ;
			vif->addr = vif->immediate & 0x3ff ;
			vif->unpack_vn = ( ( vif->cmd >> 2 ) & 0x03 ) + 1 ;
			vif->unpack_vl = 32 >> ( vif->cmd & 0x03 ) ;
			if ( ( vif->cmd & 0x0f ) != 0x0f ){
				vif->skip_count = ( vif->num * vif->unpack_vn * vif->unpack_vl + 31 ) / 32 ;
				if ( vif->unpack_vl == 4 ){
					printf("UNPACK: vertex format error!!\n");
					vif->flag |= VIF_FLAG_STOP ;
					return ;
				}
			} else {
				vif->skip_count = ( vif->num * 16 + 31 ) / 32 ;
			}

		}

	} else {
		/* データ転送 */
		vif->buffer[ vif->buffer_fill_count ] = data ;
		vif->buffer_fill_count++ ;
		if ( vif->buffer_fill_count >= 4 ){
			if ( ( vif->cmd == VIFCODE_DIRECT ) || ( vif->cmd == VIFCODE_DIRECTHL ) ){
				/* GIFへの転送 */
				ExecGif( &vif->gif_work, vif->buffer );
			}
			vif->buffer_fill_count = 0 ;
		}
		vif->skip_count-- ;
	}
}

