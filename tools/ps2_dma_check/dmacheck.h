/*
	dmacheck.h
	PS2 DMA tag checker

	2000/06/29 K.Takabe
	$Id: dmacheck.h,v 1.2 2001/10/22 06:00:49 usr02774 Exp $
*/

#ifndef __DMA_CHECK_H__
#define __DMA_CHECK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
	/*
		GIF関連
	*/
/* GIFレジスタ一覧 */
enum {
	GS_PRIM			= 0x00,
	GS_RGBAQ		= 0x01,
	GS_ST			= 0x02,
	GS_UV			= 0x03,
	GS_XYZF2		= 0x04,
	GS_XYZ2			= 0x05,
	GS_TEX0_1		= 0x06,
	GS_TEX0_2		= 0x07,
	GS_CLAMP_1		= 0x08,
	GS_CLAMP_2		= 0x09,
	GS_FOG			= 0x0a,
	GS_XYZF3		= 0x0c,
	GS_XYZ3			= 0x0d,
	GS_TEX1_1		= 0x14,
	GS_TEX1_2		= 0x15,
	GS_TEX2_1		= 0x16,
	GS_TEX2_2		= 0x17,
	GS_XYOFFSET_1	= 0x18,
	GS_XYOFFSET_2	= 0x19,
	GS_PRMODECONT	= 0x1a,
	GS_PRMODE		= 0x1b,
	GS_TEXCLUT		= 0x1c,
	GS_SCANMSK		= 0x22,
	GS_MIPTBP1_1	= 0x34,
	GS_MIPTBP1_2	= 0x35,
	GS_MIPTBP2_1	= 0x36,
	GS_MIPTBP2_2	= 0x37,
	GS_TEXA			= 0x3b,
	GS_FOGCOL		= 0x3d,
	GS_TEXGLUSH		= 0x3f,
	GS_SCISSOR_1	= 0x40,
	GS_SCISSOR_2	= 0x41,
	GS_ALPHA_1		= 0x42,
	GS_ALPHA_2		= 0x43,
	GS_DIMX			= 0x44,
	GS_DTHE			= 0x45,
	GS_COLCLAMP		= 0x46,
	GS_TEST_1		= 0x47,
	GS_TEST_2		= 0x48,
	GS_PABE			= 0x49,
	GS_FBA_1		= 0x4a,
	GS_FBA_2		= 0x4b,
	GS_FRAME_1		= 0x4c,
	GS_FRAME_2		= 0x4d,
	GS_ZBUF_1		= 0x4e,
	GS_ZBUF_2		= 0x4f,
	GS_BITBLTBUF	= 0x50,
	GS_TRXPOS		= 0x51,
	GS_TRXREG		= 0x52,
	GS_TRXDIR		= 0x53,
	GS_HWREG		= 0x54,
	GS_SIGNAL		= 0x60,
	GS_FINISH		= 0x61,
	GS_LABEL		= 0x62,
	GS_NOP			= 0xff,
	GS_AD			= -2,
	GS_ERROR		= -1,
};

/* GIF動作ワーク */
typedef struct _gif_work{
	int				flag ;				/* 各種フラグ */
	int				nloop ;
	int				eop ;
	int				pre ;
	int				prim ;
	int				flg ;
	int				nreg ;
	unsigned char	regs[16] ;
	int				skip_count ;		/* 読み飛ばす回数（データ部分） */
	int				reg_count ;			/* 実行レジスタカウント */
	int				pad ;
} GifWork ;


/* ---------------------------------------------------------------- */
	/*
		VIF関連
	*/

/* VIFCODE一覧 */
enum {
	VIFCODE_NOP			= 0x00,
	VIFCODE_STCYCL		= 0x01,
	VIFCODE_OFFSET		= 0x02,
	VIFCODE_BASE		= 0x03,
	VIFCODE_ITOP		= 0x04,
	VIFCODE_STMOD		= 0x05,
	VIFCODE_MSKPATH3	= 0x06,
	VIFCODE_MARK		= 0x07,
	VIFCODE_FLUSHE		= 0x10,
	VIFCODE_FLUSH		= 0x11,
	VIFCODE_FLUSHA		= 0x13,
	VIFCODE_MSCAL		= 0x14,
	VIFCODE_MSCALF		= 0x15,
	VIFCODE_MSCNT		= 0x17,
	VIFCODE_STMASK		= 0x20,
	VIFCODE_STROW		= 0x30,
	VIFCODE_STCOL		= 0x31,
	VIFCODE_MPG			= 0x4a,
	VIFCODE_DIRECT		= 0x50,
	VIFCODE_DIRECTHL	= 0x51,
	VIFCODE_UNPACK		= 0x60,
};

/* VIF動作ワーク */
typedef struct _vif_work{
	int				flag ;				/* 各種フラグ */
	int				code ;				/* VIFCODE */
	int				cmd ;				/* 実行中コマンド */
	int				num ;				/*  */
	int				immediate ;			/*  */
	int				cmd_index ;			/* インデックス化したVIFコマンド */
	int				skip_count ;		/* 読み飛ばす回数（データ部分） */
	int				buffer_fill_count ;	/* バッファ読み込み数 */
	/*  */
	unsigned int	addr ;				/* VU書き込みアドレス */
	int				count ;				/* 残り書き込み数 */
	int				unpack_vn ;			/* UNPACK用書き込みベクトル次数 */
	int				unpack_vl ;			/* UNPACK用転送元データ要素のビット幅 */
	/*  */
	GifWork			gif_work ;			/* VIF1用 */
	unsigned int	buffer[4] ;			/* データ読み込みバッファ */
} VifWork ;

/* ---------------------------------------------------------------- */
	/*
		DMA関連
	*/

/* DMAタグID */
enum {
	DMATAG_REFE, DMATAG_CNT, DMATAG_NEXT, DMATAG_REF,
	DMATAG_REFS, DMATAG_CALL, DMATAG_RET, DMATAG_END
};

/* ワーク内の各種フラグ */
enum {
	DMA_FLAG_TERMINATED		= 0x00000001,	/* 終了フラグ */
	DMA_FLAG_EXEC_VIF		= 0x00000002,	/* VIFCODE実行フラグ */
	DMA_FLAG_TTE			= 0x00000004,	/* タグ転送フラグ */
	DMA_FLAG_EXEC_GIF		= 0x00000008,	/* GIF実行フラグ */
	DMA_FLAG_EXEC_GS		= 0x00000010,	/* GS実行フラグ（未サポート） */
	DMA_FLAG_DISP_TAG		= 0x00010000,	/* タグ内容表示フラグ */
	DMA_FLAG_DISP_VIF		= 0x00020000,	/* VIFCODE表示フラグ */
	DMA_FLAG_DISP_GIF		= 0x00040000,	/* GIFタグ内容表示フラグ */
	DMA_FLAG_DISP_GS		= 0x00080000,	/* GSパケット内容表示フラグ */
	DMA_FLAG_SKIP_DATA		= 0x08000000,	/* DMA内容スキップフラグ（デバッグ用） */
	/* VIF固有フラグ */
	VIF_FLAG_STOP			= 0x10000001,	/* 停止フラグ */
	/* GIF固有フラグ */
	GIF_FLAG_STOP			= 0x20000001,	/* GIFタグ内容表示フラグ */
	GIF_FLAG_STOP_GS		= 0x20000002,	/* GSパケット内容表示フラグ */
};

/* DMA動作ワーク */
typedef struct _dma_work{
	int				flag ;				/* 各種フラグ */
	int				qwc ;				/* 転送サイズ */
	unsigned int	tadr ;				/* 実行中転送タグアドレス */
	unsigned int	madr ;				/* データ転送アドレス */
	int				id ;				/* 最終実行TAGID */
	int				stack_count ;		/* スタックカウント */
	unsigned int	dma_stack[2] ;		/* スタック */
	GifWork			gif_work ;			/* GIF用 */
	VifWork			vif_work ;			/* VIF0,VIF1用 */
	unsigned int	buffer[4] ;			/* 読み込みバッファ */
} DmaWork ;


/* ---------------------------------------------------------------- */



/* DMAタグを解析 */
void CheckDmaTag( unsigned int *tag, char *output );
/* DMA初期化 */
void InitDma( DmaWork *dma, int mode );
/* DMA実行開始 */
void SendDma( DmaWork *dma, unsigned int addr );
/* DMAタグを実行 */
void ExecDma( DmaWork *dma );

/* VIFCODEを解析 */
void CheckVifcode( unsigned int data, char *output );
/* VIFを初期化 */
void InitVif( VifWork *vif, int flag );
/* VIFを実行 */
void ExecVif( VifWork *vif, unsigned int data );


#ifdef __cplusplus
}
#endif

#endif

