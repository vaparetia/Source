//-----------------------------------------------------------------------------
// libdg.h
// ディスプレイジェネレータライブラリヘッダ
//
// 1999/07/07 K.Takabe
// 2001/03/22 F.Miyauchi (xbox)
//-----------------------------------------------------------------------------

// xbox 版では、
//
// windowの生成→DirectXの初期化等々もサポートする。
// vector, matrix に関しては include/mgs_type.h の FVECTOR, FMATRIX を使用する。
// ※計算自体は D3DX を利用する。

// <その他メモ>
//
// <PRIM2 関係>
// ・XBOXでは、描画途中でのカリング方向変更(PS2はuv.fで行っている)は不可能。
//   SetRenderStateでカリングフラグを変更しなければならないため、非実用的…。
// ・SCE_GS_SET_ALPHA に関して。dst に対する fix 値演算不可？
// ・PRIM2のアンチエイリアスに関して。
//   d3dDevice->SetRenderState(D3DRS_EDGEANTIALIAS, TRUE);
//   でアンチエイリアスONに出来るが、PS2とは相容れない感じ…
//

#ifndef __LIBDGX_H__
#define __LIBDGX_H__

#error THIS FILE SHOULD NOT BE INCLUDED

#ifdef WIN32
// 非標準の拡張機能使用(構造体の内部にサイズ0の配列がある)警告抑制 
#pragma warning( disable : 4200 )
#endif

#include "mgsx_type.h"
#include "fmt_kmdx.h"  // モデルフォーマット 
#include "fmt_cm2x.h"  // テクスチャフォーマット 
#include "fmt_lit.h"  // 光源フォーマット 
#include "vertex.h"   // 頂点タイプ 

// πの値 
#ifndef M_PI
#define M_PI  D3DX_PI
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------------------- */
/*
	ＸＢＯＸ関連
*/
/* 頂点シェーダー情報 */
typedef struct _dg_vertexshader{
	struct _dg_vertexshader		*next ;	/* リンク用 */
	u_int		handle ;		/* 取得ハンドル */
	u_short		size ;			/* プログラムサイズ */
	short		load_addr ;		/* プログラムロードアドレス *//* -1なら動的ロード */
	void		*prog_addr ;	/* プログラム実体 */
} DG_VERTEXSHADER ;

/* ピクセルシェーダー情報 */
typedef struct {
	u_int		handle ;		/* 取得ハンドル */
	void		*prog_addr ;	/* プログラム実体 */
	int			pad[2] ;
} DG_PIXELSHADER ;

/* 頂点フォーマット */
typedef struct {
#if 0 //BP_RENDER
	D3DVERTEXATTRIBUTEFORMAT	faf ;	/* 頂点フォーマット属性 */
#else
   int faf;
#endif
} DG_VERTEXFORMAT ;

/* 頂点ストリーム関連 *//* 最大２ストリーム分のみ確保 */
typedef struct {
	int					max_stream ;
#if 0 //BP_RENDER
	D3DSTREAM_INPUT		input[2] ;
#endif
} DG_VERTEXSTREAM ;

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PS2/SCE互換関連
//-----------------------------------------------------------------------------	

// スクラッチパッド(16K)
#if 0
extern unsigned char *SCRPAD_ADDR;
#else
#define SCRPAD_ADDR	((unsigned char *)SCRATCH_ADDRESS)
#endif	
	

#define SCE_GS_ALPHA_MASK    (0x00000000ffffffffL)
#if 0
// PS2半透明指定互換用マクロ (in eestruct.h)
#define SCE_GS_SET_ALPHA_1 SCE_GS_SET_ALPHA
#define SCE_GS_SET_ALPHA_2 SCE_GS_SET_ALPHA
#define SCE_GS_SET_ALPHA(a, b, c, d, fix) \
	((u_long64)(a)       | ((u_long64)(b) << 2)     | ((u_long64)(c) << 4) | \
	((u_long64)(d) << 6) | ((u_long64)(fix) << 32))

#define SCE_GS_SET_CLAMP_1	SCE_GS_SET_CLAMP
#define SCE_GS_SET_CLAMP_2	SCE_GS_SET_CLAMP
#define SCE_GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv) \
	((u_long64)(wms)         | ((u_long64)(wmt) << 2) | \
	((u_long64)(minu) << 4)  | ((u_long64)(maxu) << 14) | \
	((u_long64)(minv) << 24) | ((u_long64)(maxv) << 34))

// パラメータ部分だけ取り出す	
#define SCE_GS_ALPHA_MASK    (0x00000000ffffffffL)
// fix 値を取り出す 
#define SCE_GS_ALPHA_FIX(a)  ((u_long64)(a) >> 32)

typedef ALIGN16_DECL(struct) _dg_gsreg {
	u_long64 data;
	//u_long64 reg;
} DG_GSREG ;
#endif
typedef ALIGN16_DECL(struct) _dg_gsregx {
	u_long64 data;
	//u_long64 reg;
} DG_GSREGX ;


/*----------------------------------------------------------------*/
/* ＜ＬＩＢＤＧ汎用定義＞ */

/* ＰＳ２ＤＭＡタグ構造体 */
typedef ALIGN16_DECL(struct) _dg_dmatag{
	u_int	qwc ;
	void	*addr ;
	u_int	vifcode[2] ;
} DG_DMATAG  ;

/* ＰＳ２ＧＳレジスタ（Ａ＋Ｄ）構造体 */
typedef ALIGN16_DECL(struct) _dg_gsreg {
	u_long64	data ;
	u_long64	reg ;
} DG_GSREG  ;

/* ＰＳ２ＧＩＦタグ構造体 */
typedef ALIGN16_DECL(struct) _dg_giftag{
	u_long64	tag ;
	u_long64	regs ;
} DG_GIFTAG  ;

/* ＰＳ２ＶＩＦデータ構造体（1qword分） */
typedef ALIGN16_DECL(struct) _dg_vifcode {
	u_int	vifcode[4] ;
} DG_VIFCODE  ;




//-----------------------------------------------------------------------------
// チャンネル関係
//-----------------------------------------------------------------------------

/* ＜描画環境関連＞ */

	/*
		描画環境設定構造体
	*/
typedef ALIGN16_DECL(struct) _dg_drawenv {
	u_long64			gif_tag ;
	u_long64			reg_list ;
	struct {
		sceGsFrame		frame1 ;
		u_long64			frame1_addr ;
		sceGsZbuf		zbuf1 ;
		u_long64			zbuf1_addr ;
		sceGsXyoffset	xyoffset1 ;
		u_long64			xyoffset1_addr ;
		sceGsScissor	scissor1 ;
		u_long64			scissor1_addr ;
		sceGsPrmodecont	prmodecont ;
		u_long64			prmodecont_addr ;
		sceGsColclamp	colclamp ;
		u_long64			colclamp_addr ;
		sceGsTest		test1 ;
		u_long64			test1_addr ;
		sceGsPabe		pabe ;
		u_long64			pabe_addr ;
		sceGsTexa		texa ;
		u_long64			texa_addr ;
		sceGsDthe		dthe ;
		u_long64			dthe_addr ;
		sceGsDimx		dimx ;
		u_long64			dimx_addr ;
		sceGsClamp		clamp ;
		u_long64			clamp_addr ;
	} datas ;
} DG_DRAWENV  ;
typedef ALIGN16_DECL(struct) _dg_drawoffset {
	DG_GIFTAG		gif_tag ;
	struct {
		sceGsXyoffset	xyoffset1 ;
		u_long64			xyoffset1_addr ;
	} datas ;
} DG_DRAWOFFSET  ;
typedef ALIGN16_DECL(struct) _dg_drawfog {
	DG_GIFTAG		gif_tag ;
	struct {
		sceGsFogcol		fogcol ;
		u_long64			fogcol_addr ;
	} datas ;
} DG_DRAWFOG  ;

// オブジェクトキューサイズ 
#define DG_MAX_USR_OBJQUEUE  (8)
	
/*
  オブジェクトキューバッファ
*/
typedef struct _dg_obj_buffer{
	int         max_queue;          /* 使用可能な最大数 */
	int			n_queue;			/* 現在使用中のキューの数 */
    void        **queue;            /* キューバッファへのポインタ */
    int         id;                 /* 識別ＩＤ */
} DG_OBJ_BUFFER;

/*
  オブジェクトキュー
*/
typedef struct {
    /*
	  新管理方式構造体
    */
    DG_OBJ_BUFFER    objs_buffer;           /* 通常オブジェクト用キューバッファ */
    DG_OBJ_BUFFER    evmobj_buffer;         /* マルチウェイトエンベロープ用キューバッファ */
    DG_OBJ_BUFFER    prim2_buffer;          /* 新プリミティブ用キューバッファ */
    /* 以下拡張 */
    DG_OBJ_BUFFER    comdl_buffer;          /* 共有モデルオブジェクト用キューバッファ */
    DG_OBJ_BUFFER    shdwwrite_buffer;      /* 影投影オブジェクト用キューバッファ */
    DG_OBJ_BUFFER    spot_buffer;           /* 投影スポットオブジェクト用キューバッファ */
    DG_OBJ_BUFFER    particle_buffer;       /* パーティクルオブジェクト用キューバッファ */	
    DG_OBJ_BUFFER    user_buffer[DG_MAX_USR_OBJQUEUE]; /* 拡張用リザーブバッファ（プラグインで使用） */
} DG_OBJ_QUEUE;

/*
  チャンネルデータ
*/
typedef ALIGN16_DECL(struct)    _dg_chanl {
    /*
	  カメラデータ
    */
    FMATRIX eye_pers;        /* カメラ透視変換マトリクス (= pers * eye_inv) */
    FMATRIX eye_inv;         /* カメラ逆行列 */
    FMATRIX eye;             /* カメラ行列 */
    FMATRIX pers;            /* 透視変換マトリクス */
    FMATRIX raise_pers;      /* 透視変換マトリクス */
    FMATRIX raise_eye_pers;  /* カメラ透視変換マトリクス */
	FMATRIX	xpers ;				/* DirectX用透視変換マトリクス */
	FMATRIX	eye_xpers ;			/* DirectX用透視変換マトリクス×カメラ逆行列マトリクス */
	FMATRIX	raise_xpers ;		/* DirectX用透視変換マトリクス */
	FMATRIX	raise_eye_xpers ;	/* DirectX用透視変換マトリクス×カメラ逆行列マトリクス */
    float  screen;          /* カメラ→スクリーンキョリ(default = 2.0)        */

    /*
	  オブジェクト管理情報
    */
    int             flag;                         /* チャンネルフラグ */
    int             group_id;                     /* 有効グループＩＤ */
    DG_OBJ_QUEUE    *obj_queue;                   /* 処理対象オブジェクトキュー */
    int             n_stage;                      /* チャンネル処理ステージ数 */
    void (**stage_list)(struct _dg_chanl *, int); /* チャンネル処理ステージ関数リスト */

    /*
	  描画環境など
    */
    int            width;              /* 描画幅（最大512） */
    int            height;             /* 描画高さ（最大256） */
    int            offset_x;
    int            offset_y;
    DG_DRAWENV       draw_env[2];    /* デフォルト描画環境設定 */
    DG_DRAWOFFSET    draw_offset[2]; /* ハイレゾ化に必要な環境設定 */
    int            bg_clear_flag;      /* 背景クリアフラグ */
    int            chanl_num;          /* チャンネル番号 */
    int            high_reso;          /* オフセット変更によるハイレゾ化有効フラグ */
} DG_CHANL ;


/*
  チャンネル処理ユニット
*/
typedef void (*QueFunc)(DG_CHANL *, int);

/*
  チャンネル、オブジェクトキューのバッファサイズ
*/
enum {
    DG_CHANL_MAIN,
    DG_CHANL_SUB1,
    DG_CHANL_SUB2,
    DG_CHANL_SUB3,
    DG_CHANL_MENU,
    DG_MAX_CHANLS
};

/*
  番号→チャンネル構造体
*/
#define DG_Chanl(_chanl) (DG_Chanls + _chanl)
	
//-----------------------------------------------------------------------------
// 光源関連
//-----------------------------------------------------------------------------

#define	MAX_TMPLIGHTS	(16)

// 一時光源バッファ（ダブルバッファの片方） 
typedef	struct tagDG_TLIGHT {
#if 0
	LIT_DEF         TmpLightDef;
#else
	LIT_DEF_HEADER	TmpLightDef;	/* VCだとエラーが出るので */
#endif
	LIT_GRP         TmpLightGrp[3];
	LIT_POINT       points[MAX_TMPLIGHTS];
	LIT_SPOT        spots[MAX_TMPLIGHTS];
	LIT_BLACKPOINT  blacks[MAX_TMPLIGHTS];
} DG_TLIGHT;

//-----------------------------------------------------------------------------

	
// 関節モデルの最大ユニット数 
#define DG_MAX_JOINTS        (128)


/*----------------------------------------------------------------*/
	/*
		イメージ転送構造体
	*/
typedef ALIGN16_DECL(struct) _dg_loadimage {
	DG_DMATAG		dmatag0 ;			/* レジスタ設定値転送ＤＭＡタグ */
	DG_GIFTAG		giftag0 ;			/* レジスタ設定 */
	DG_GSREG		bitbltbuf ;
	DG_GSREG		trxpos ;
	DG_GSREG		trxreg ;
	DG_GSREG		trxdir ;
	DG_GIFTAG		giftag1 ;
	DG_DMATAG		dmatag1 ;			/* イメージ転送ＤＭＡタグ */
} DG_LOADIMAGE  ;

	/*
		ＶＲＡＭ内イメージ転送構造体
	*/
typedef ALIGN16_DECL(struct) _dg_moveimage {
	DG_DMATAG		dmatag ;			/* レジスタ設定値転送ＤＭＡタグ */
	DG_GIFTAG		giftag ;			/* レジスタ設定 */
	DG_GSREG		bitbltbuf ;
	DG_GSREG		trxpos ;
	DG_GSREG		trxreg ;
	DG_GSREG		trxdir ;
} DG_MOVEIMAGE  ;

	/*
		イメージ取り込み構造体（システム専用）
	*/
typedef ALIGN16_DECL(struct) _dg_storeimage {
	DG_DMATAG		dmatag ;			/* レジスタ設定値転送ＤＭＡタグ */
	DG_GIFTAG		giftag ;			/* レジスタ設定 */
	DG_GSREG		bitbltbuf ;
	DG_GSREG		trxpos ;
	DG_GSREG		trxreg ;
	DG_GSREG		finish ;
	DG_GSREG		trxdir ;
} DG_STOREIMAGE  ;


//-----------------------------------------------------------------------------
// テクスチャ関連
//-----------------------------------------------------------------------------

/* テクスチャフラグ（ＸＢＯＸ版ではフォーマット変更のためこの値は無意味なので注意） */
/* ※ CM2ファイルフォーマットから引っ張ってきて名前を変えたもの  */
#define DG_TEX_COLOR_MASK (0x00000003) // テクスチャカラーモード設定値マスク 
#define DG_TEX_USE_AMODE  (0x00000004) // アルファブレンドモード指定フラグ 
#define DG_TEX_BLEND_MODE (0x00000030) // アルファブレンドモード設定値マスク 
#define DG_TEX_DECAL      (0x00000100) // 強制シェーディング禁止フラグ 
#define DG_TEX_FLAG_EMAP  (0x00010000) // 環境マッピングフラグ 
#define DG_TEX_FLAG_BMAP  (0x00020000) // バンプマッピングフラグ 
#define DG_TEX_FLAG_SMAP  (0x00040000) // アニメ影マップフラグ 
#define DG_TEX_AMODE_MASK (0xff000000) // アルファブレンドモード直接指定設定値 
#define DG_TEX_FREE_BIT   (0x00f7fec8)       // 空ビット 
#define DG_TEX_FLAG_ALL   (~DG_TEX_FREE_BIT) // 使用ビット 
/* ＸＢＯＸではテクスチャのフラグはfmt_cm2x.h内のTEXINFO_FLAG_*になっているので注意 */
/* 特にマッピング設定などがピット指定ではなく番号指定になっている */

/* テクスチャパラメータ (PS2とは違う) */
typedef struct _dg_tex_trans {
	DG_GSREG alpha; // SCE_GS_SET_ALPHA の値が入る 
	DG_GSREG clamp;
	DG_GSREG tex2;
	DG_GSREG tex0;
	FVECTOR vec1; /* パラメータ格納用 */
	FVECTOR vec2; /* パラメータ格納用 */
	// XBOX追加 
	LPDIRECT3DTEXTURE9 ptex; // テクスチャ 
#if 0 //BP_RENDER
   LPDIRECT3DPALETTE8 ppal ;
#else
   void *ppal;
#endif
	int			pad[ 2 ];
} DG_TEX_TRANS;
	
/* テクスチャ情報 */
typedef struct _dg_tex {
    float               u_offset;   /* 使用テクスチャの開始Ｕ座標 */
    float               v_offset;   /* 使用テクスチャの開始Ｖ座標 */
    float               u_scale;    /* 使用テクスチャのスケール補正値 */
    float               v_scale;    /* 使用テクスチャのスケール補正値 */
    unsigned int        tex_id;     /* テクスチャＩＤ */
    unsigned int        tri_id;     /* テクスチャアーカイバＩＤ */
    int                 flag;       /* テクスチャフラグ */
#if 0
	int					texinfonum ;/* xti内での使用XTI_TEXINFO番号 */
#else
	unsigned short		texinfonum ;/* xti内での使用XTI_TEXINFO番号 */
	unsigned short		clut_num ;	/* CLUTエントリ番号 */
#endif
    DG_TEX_TRANS        tex_trans;  /* テクスチャパラメータ転送パケット */
} DG_TEX;

/* 線形テクスチャ（ＸＢＯＸ専用＆DG_DMAPACKでのみ使用可能） */
typedef struct _dg_tex_lin {
	u_short				width ;		/* 横幅 */
	u_short				height ;	/* 縦幅 */
	u_short				format ;	/* 画像フォーマット */
	u_short				flag ;		/* 各種フラグ */
	void				*image ;	/* イメージデータ */
	void				*palette ;	/* パレットデータ */
	IDirect3DBaseTexture9 * d3dtexture ;
#if 0 //BP_RENDER
   D3DPalette			d3dpalette ;
#else
   void *d3dpalete;
#endif

} DG_TEX_LIN ;
enum {
	DG_TEXLIN_FORMAT_A8R8G8B8,		/* ３２ビットテクスチャ */
	DG_TEXLIN_FORMAT_A1R5G5B5,		/* １６ビットテクスチャ */
	DG_TEXLIN_FORMAT_A4R4G4B4,		/* １６ビットテクスチャ */
	DG_TEXLIN_FORMAT_P8,			/* ８ビットパレットテクスチャ（実際には８ビット輝度テクスチャ） */
	DG_TEXLIN_FORMAT_R5G6B5,		/* １６ビットアルファ無し */
	DG_TEXLIN_FORMAT_L8,			/* ８ビット輝度テクスチャ */
	DG_TEXLIN_FORMAT_A8,			/* ８ビットアルファテクスチャ */
	DG_TEXLIN_FORMAT_X1R5G5B5,		/* １６ビットテクスチャアルファ無し２ */
};

/* ユーザー生成テクスチャ（現在まだ未使用） */
typedef struct _dg_usertex {
	DG_TEX				tex ;		/* ＤＧシステム用テクスチャ構造体 */
	/* 生成テクスチャ固有情報 */
	u_short				width ;			/* 横幅 */
	u_short				height ;		/* 縦幅 */
	u_long64				format ;		/* 画像フォーマット */
	void				*image ;		/* イメージデータ */
	void				*palette ;		/* パレットデータ */
	IDirect3DBaseTexture9 * d3dtexture ;	/* 生成したD3DTexture8オブジェクト */
#if 0 //BP_RENDER
   D3DPalette			d3dpalette ;  /* 生成したD3DPalette8パレットオブジェクト */
#else
   void *d3dpalete;
#endif
} DG_USERTEX ;

	/*
		部分テクスチャ転送パケット
	*/
typedef ALIGN16_DECL(struct) _dg_tex_loadreplace {
	DG_TEX			*org_tex ;			/* 置き換え先テクスチャ */
	DG_TEX			*src_tex ;			/* 置き換えテクスチャ */
	LPDIRECT3DTEXTURE9	org_ptex ;			/* パラメータ書き換え復元用 */
	DG_TEX_LIN		*tex ;				/* 置き換え用テクスチャ */
	FVECTOR			org_vec1 ;			/* パラメータ書き換え復元用 */
	FVECTOR			org_vec2 ;			/* パラメータ書き換え復元用 */
	void			*texel ;			/* 転送元テクスチャデータ */
	void			*clut ;				/* 転送元ＣＬＵＴデータ */
	int				mode ;				/* 展開テクスチャのモード */
	int				pitch ;				/* 生成するテクスチャのピッチ */
	int				width ;
	int				height ;
	/* 接続制御関連 */
	struct _dg_texture_list		*tex_list ;	/* 対象ＴＲＩ */
	struct _dg_tex_loadreplace	*prev ;		/* リンク */
	struct _dg_tex_loadreplace	*next ;		/* リンク */
	int				pad ;
} DG_TEX_LOADREPLACE  ;

	/*
		部分テクスチャ移動パケット
	*/
typedef ALIGN16_DECL(struct) _dg_tex_movereplace {
	DG_TEX			*org_tex ;			/* 置き換え先テクスチャ */
	DG_TEX			*src_tex ;			/* 置き換えテクスチャ */
	LPDIRECT3DTEXTURE9	org_ptex ;			/* パラメータ書き換え復元用 */
	int				pad2 ;
	FVECTOR			org_vec1 ;			/* パラメータ書き換え復元用 */
	FVECTOR			org_vec2 ;			/* パラメータ書き換え復元用 */
	/* 接続制御関連 */
	struct _dg_texture_list		*tex_list ;	/* 対象ＴＲＩ */
	struct _dg_tex_movereplace	*prev ;		/* リンク */
	struct _dg_tex_movereplace	*next ;		/* リンク */
	int				pad ;
} DG_TEX_MOVEREPLACE  ;

typedef ALIGN16_DECL(struct) _dg_tex_packet {
	DG_LOADIMAGE	texel0 ;			/* 第１テクセルデータ（最大５０４ＫＢ） */
	DG_LOADIMAGE	texel1 ;			/* 第２テクセルデータ（最大５０４ＫＢ） */
	DG_LOADIMAGE	clut ;				/* ＣＬＵＴデータ */
	DG_DMATAG		rettag ;			/* 終端（RETタグ） */
} DG_TEX_LOAD  ;
typedef DG_TEX_LOAD DG_TEX_PACKET  ;

typedef struct _dg_texpack {
	XTI_TEXINFO		*texinfo ;		/* テクスチャ固有情報 */
	void			*data ;			/* 確保したデータ領域 */
   IDirect3DBaseTexture9 *d3d_texture ;	/* DirectXテクスチャ配列 */
} DG_TEXPACK ;

/* テクスチャファイル管理構造体 */
typedef ALIGN16_DECL(struct) _dg_texture_list {
	DG_TEX_PACKET	tex_packet[2];      /* テクスチャデータ転送ＤＭＡパケット */
	u_long64	load_tex0;          /* ＶＲＡＭに読み込まれた際のベースアドレス設定値 */
	int				code;               /* 読み込んだテクスチャアーカイバのコードＩＤ */
	int				load_tex_base;      /* ＶＲＡＭに読み込まれた際のベースアドレス */
	int				load_clut_base;     /* ＶＲＡＭに読み込まれた際のベースアドレス */
	TRI_FILEHEADER  *header;
	DG_TEX			*textures;			/* 各テクスチャの情報 */
	int				*texture_image;     /* テクスチャイメージアドレス */
	int				*clut_image;		/* ＣＬＵＴイメージアドレス */
	int				*tmp_clut_image;	/* 一時的ＣＬＵＴイメージアドレス(要8qwordアラインアドレス) */
	int				tex_size;			/* デバッグ用 */
	short			flag;				/* テクスチャ制御用フラグ */
	short			prio;				/* テクスチャプライオリティ(＝描画プライオリティ) */
	DG_TEX_LOADREPLACE *load_link;/* テクスチャ部分入れ替えパケットリンク(追加読み込み) */
	DG_TEX_MOVEREPLACE *move_link;/* テクスチャ部分入れ替えパケットリンク(ＶＲＡＭ内移動) */
#if BP_RENDER
   D3DPalette		*d3dpalette ;		/* 生成したD3DPalette8パレットオブジェクト */
#else
   void *d3dpalette;
#endif
	int				pad[1];
	LPDIRECT3DTEXTURE9  ptex;           /* DirectX8用テクスチャクラスへのポインタ */
	DG_TEXPACK		texpack[] ;
} DG_TEXTURE_LIST ;

	
//-----------------------------------------------------------------------------
// オブジェクト関連
//-----------------------------------------------------------------------------

/* 処理フラグ */
enum {
    DG_FLAG_TEXT            = 0x0000,    /* テクスチャ座標あり（必須）（現在は無視）*/
    DG_FLAG_TRANS           = 0x0000,    /* 表示座標計算（必須）（現在は無視）      */
    DG_FLAG_BOUND           = 0x0000,    /* 単位バウンディング（現在は無視）        */
    DG_FLAG_GBOUND          = 0x0000,    /* 全体バウンディング（現在は無視）        */
    DG_FLAG_AMBIENT         = 0x0000,    /* アンビエントを固有設定（現在は無視）    */
    DG_FLAG_IRTEXTURE       = 0x0000,    /* 赤外線ゴーグル反応モデル（現在は無視）  */
    DG_FLAG_ENVMAP          = 0x0000,    /* Enviroment Mapping（現在は無視）        */

    DG_FLAG_PAINT           = 0x0001,    /* 点光源計算によるプレシェイド使用        */
    DG_FLAG_SHADE           = 0x0002,    /* 光源マトリクスによる平行光源シェーディング */
    DG_FLAG_NOFOG           = 0x0004,    /* フォグ効果なし（DG_FLAG_SHADE使用時のみ）  */
    DG_FLAG_AUTOREPAINT     = 0x0008,    /* プレシェイドの自動再計算あり        */
    DG_FLAG_ONEPIECE        = 0x0010,    /* 一体型モデル            */
    DG_FLAG_FINISHCALC      = 0x0020,    /* オブジェクトマトリクス計算済み（旧名） */
    DG_FLAG_FINISHEDCALC    = 0x0020,    /* オブジェクトマトリクス計算済み（新名） */
    DG_FLAG_QUATROT         = 0x0040,    /* 関節回転をオイラー角ではなくクォータニオンで指定 */
    DG_FLAG_PLUGINDRAW      = 0x0080,    /* 通常描画フェーズでの描画キャンセル（プラグイン用） */
    DG_FLAG_IRREACTION      = 0x0100,    /* 赤外線センサーに反応 */
	DG_FLAG_SEMITRANS       = 0x0200,    /* 強制５０％半透明描画（objs->flagでのみ有効）*/
	DG_FLAG_FOGPARAM        = 0x0400,    /* フォグパラメータ指定（DG_FLAG_SHADE使用時のみ）*/
    DG_FLAG_LATTERDRAW      = 0x0800,    /* 後回し描画（マルチテクスチャモデルのみ対応） */
    DG_FLAG_INVISIBLE       = 0x3000,    /* 不可視属性（随時変更可能）    */
    DG_FLAG_INVISIBLE0      = 0x1000,    /* 不可視属性（随時変更可能）（チャンネル０のみ）    */
    DG_FLAG_INVISIBLE1      = 0x2000,    /* 不可視属性（随時変更可能）（チャンネル１のみ）    */
    DG_FLAG_INVISIBLE2      = 0x4000,    /* 不可視属性（随時変更可能）（チャンネル２のみ）    */
    DG_FLAG_INVISIBLE3      = 0x8000,    /* 不可視属性（随時変更可能）（チャンネル３のみ）    */

    /* システム内部で使用するフラグ */
    DG_FLAG_MULTITEX        = 0x00010000,    /* マルチテクスチャモデル */
	DG_FLAG_DELAYED			= 0x00020000,	/* メモリの１フレーム遅らせ開放フラグ */
	// TODO:POINLIGHTフラグ 
	//DG_FLAG_POINTLIGHT      = 0x00020000,    /* ポイントライトに反応 */
	//DG_FLAG_POINTLIGHT2     = 0x00040000,    /* ポイントライトに反応(ピクセルシェーダ版) */
	DG_FLAG_POINTLIGHT      = 0x00100000,    /* ポイントライトに反応 */
	DG_FLAG_POINTLIGHT2     = 0x00200000,    /* ポイントライトに反応(ピクセルシェーダ版) */
	
    /* 拡張プラグインで使用されるもの */
    DG_FLAG_PLUGINMASK      = 0xff000000,
    DG_FLAG_SHADOWMAKE      = 0x01000000,    /* 影生成フラグ（要 DG_FLAG_SHADE） */
    DG_FLAG_SHADOWWRITE     = 0x02000080,    /* 影投影モデルフラグ（要 DG_FLAG_SHADE|DG_FLAG_ONEPIECE） */
    DG_FLAG_OPTCMF          = 0x04000080,    /* 光学迷彩処理（要 DG_FLAG_SHADE） */
    DG_FLAG_SHADOWVOL       = 0x08000080,    /* シャドーボリュームモデル */
	DG_FLAG_TEST			= 0x80000000,    /* テスト用フラグ */
};
/* マルチテクスチャタイプ */
enum {
	DG_MULTITEX_NORMAL = 0,			/* シングルテクスチャ */
	DG_MULTITEX_NORMAL2,			/* 通常２パステクスチャ */
	DG_MULTITEX_NORMAL3,			/* 通常３バステクスチャ */
	DG_MULTITEX_EMAP,				/* ＭＧＳ２標準環境マップ */
	DG_MULTITEX_SMAP,				/* ＭＧＳ２標準トゥーンシェードマップ */
	DG_MULTITEX_BMAP,				/* ＭＧＳ２標準バンプマップ */
	DG_MULTITEX_EMAP2,				/* ＭＧＳ２重ね用環境マップ（減算＋環境マップの２テクスチャ） */
	DG_MULTITEX_EMAP3,				/* ＭＧＳ２重ね用環境マップ（アルファ＋環境マップの２テクスチャ） */
	DG_MULTITEX_EMAP4,				/* ＭＧＳ２標準環境マップ（２テクスチャ） */
	DG_MULTITEX_EXT4,				/* 拡張４ */
	DG_MULTITEX_MAXNUM,				/* 個数カウント用 */
};

/* シングルテクスチャ用頂点データフォーマット */
typedef struct _dg_vertex_kmss {
	short		vx, vy, vz ;	/* 頂点データ */
	short		wt ;			/* ウェイトデータ */
	short		nx, ny, nz ;	/* 法線 */
	short		f ;				/* フラグ（未使用） */
	short		u0, v0 ;		/* テクスチャ座標 */
} DG_VERTEX_KMSS ;
typedef struct _dg_vertex_kmsm {
	short		vx, vy, vz ;	/* 頂点データ */
	short		wt ;			/* ウェイトデータ */
	short		nx, ny, nz ;	/* 法線 */
	short		f ;				/* フラグ（未使用） */
	short		u0, v0 ;		/* テクスチャ座標 */
	short		u1, v1 ;		/* テクスチャ座標 */
	short		u2, v2 ;		/* テクスチャ座標 */
} DG_VERTEX_KMSM ;

// 頂点バッファ情報 
typedef struct _dg_vbuf_info {
	unsigned int            rgba_ofs;     // RGBデータへのオフセット 
	unsigned int            size;         // ロックする頂点バッファのサイズ 
	unsigned int            stride;       // 頂点一つのサイズ 
	unsigned int            fvf;          // VertexShaderに渡すフラグ 
	LPDIRECT3DVERTEXBUFFER9 vbuf;         // VertexBuffer
	LPDIRECT3DINDEXBUFFER9  ibuf;         // IndexBuffer
	void                    *index;       // 描画時に使用するインデックス情報先頭 
} DG_VBUF_INFO;

// インデックスバッファ使用時に必要になる情報
// パケット数だけ確保する。
// 本当はEVM/KMSのフォーマットを変えるべきかも…。 
typedef struct _dg_index_info {
	DG_VBUF_INFO   *vbuf_info; // 頂点バッファの情報 
	unsigned short min;        // 最小のインデックス値 
	unsigned short num;        // 使用する頂点の数 
	unsigned short start;      // インデックスバッファの使用開始位置 
	unsigned short prims;      // 描画するプリミティブ数 
} DG_INDEX_INFO;

	
/* パケット（＝ＤＭＡ転送単位）情報管理構造体 */
typedef struct _dg_obj_packet {
	int             flag;           /* パケットのフラグ */
    void            *data_ptr;      /* オリジナルＤＭＡデータへのアドレス（RETタグ）未使用 */
	unsigned short	n_verts ;		/* 総頂点数 */
	unsigned short	n_indices ;		/* 総インデックス数 */
	unsigned short	pad ;			/* リザーブ */
    unsigned char   verts_offset;   /* 次の頂点へのオフセット（qword単位） */
    unsigned char   norms_offset;   /* 次の法線へのオフセット（qword単位） */
    unsigned char   uvs_offset[3];  /* 次のＵＶへのオフセット（qword単位） */
    unsigned char   rgbs_offset;    /* 次のカラーへのオフセット（qword単位） */
    DG_TEX_TRANS    *tex_ptr[3];    /* テクスチャ設定パケット転送タグへのポインタ */

	// XBOX
	DG_MDLPACK              *mdlpack;     /* mdlpackへのポインタ */
	int						mtex_type ;		/* マルチテクスチャタイプ */
} DG_OBJ_PACKET;

/* オブジェクトユニット（＝関節）管理構造体 */
typedef ALIGN16_DECL(struct) _DG_OBJ {
    FMATRIX      world;          /* ワールド座標 */
    FMATRIX      screen;         /* スクリーン座標 */
    FMATRIX      inv_mat;        /* エンベロープ参照先への相対マトリクス */
    int         flag;           /* パケットフラグ */
    int         sort_z;         /* 半透明ソート用Ｚ値（符号付き）*/
    short       bound_mode;     /* バウンディング結果（ bound.c ）*/
    short       parent;         /* 親オブジェクト番号（model->parentの値） */
    short       n_packs;        /* オブジェクトパケット数（model->n_packsの値） */
    short       mdl_type;       /* モデルタイプ（model->typeの値）*/
    int         vanime_flag;    /* 頂点アニメーション用フラグ */
    FMATRIX      *light;         /* 固有光源（ FMATRIX×２ ）（半透明オブジェクトのみ有効） */
    DG_MDL      *model;         /* モデルデータ（ロードデータ）*/
    SVECTOR     *verts;         /* 頂点配列バッファ */
    SVECTOR     *norms;         /* 法線配列バッファ */
	TVECTOR_S   *uvs[3];        /* ＵＶ配列バッファ（最大３セット）*/
    SVECTOR     *rgbs;          /* プリシェードバッファ */
    DG_OBJ_PACKET   *packets;   /* パケット情報（そのオブジェクトユニットに含まれるパケット数分） */
    int         fog;            /* 固定フォグ値（システム内部で使用）*/
    void        *matrix_addr;   /* パケットメモリ上に構成したマトリクスパラメータへのポインタ（システム内部） */
    FVECTOR        bound_min;   /* バウンディング最小値（model->lx,ly,lzの値）*/
    FVECTOR        bound_max;   /* バウンディング最大値（model->ux,uy,uzの値）*/
    FVECTOR        trans;       /* 親オブジェクトからのオフセット（model->tx,ty,tzの値）*/

	// XBOX追加 
	DG_VBUF_INFO *vbuf_info;  /* 頂点バッファ情報 */
	void		*vbuff ;		/* 頂点バッファアドレス */
	u_short		*index ;		/* インデックスバッファアドレス */
	int			stride ;		/* 頂点バッファのストライド */
} DG_OBJ ;

/* オブジェクト（＝モデル）管理構造体 */
typedef ALIGN16_DECL(struct) _DG_OBJS {
    FMATRIX      world;          /* ワールド座標 */
    FMATRIX      *root;          /* 親座標系（指定された場合のみ）*/
    DG_DEF      *def;           /* モデルデータ（ロードデータ）*/
    struct _DG_OBJS    *next;   /* 次の関連オブジェクトへのポインタ */
    unsigned int       flag;    /* 処理フラグ（ DG_FLAG_* ）*/
    int         group_id;       /* 表示グループ */
    int         shadow_id;      /* 影グループ */
    short       n_models;       /* ユニット数 */
    short       chanl;          /* 登録オブジェクトキュー番号（現在未使用） */
    short       bound_mode;     /* バウンディング結果（ bound.c ）*/
    short       pad1;           /*                 */
    FMATRIX      *light;         /* 固有光源（ FMATRIX×２ ）*/
    FVECTOR      *rots;          /* 回転関節データ（指定された場合のみ）*/
    FVECTOR      *movs;          /* 横すべり関節（指定された場合のみ）*/
    SVECTOR     *rgb_buff;      /* プリシェード用カラーバッファ先頭アドレス */
    int         tri_id;         /* テクスチャ入れ替え用ＴＲＩ */
    LIT_DEF     *fix_light;     /* プリシェードに使用したライトへのポインタ（自動再計算用）*/
	float		fog_param[2] ;	/* オブジェクト固有フォグパラメータ（DG_FLAG_FOGPARAM指定時のみ有効） */
    void        *extend_data;   /* 拡張ワークへのポインタ（主にプラグイン関連）*/
    char        *fname;         /* デバッグ用 */

	struct _DG_OBJS	*low ;		/* ローモデルへのポインタ */
	DG_VBUF_INFO *vbuf_info;    /* 頂点バッファ情報(XBOX追加) */
	//DG_ObjsCallback exec_func;  /* OBJS描画用コールバック(!=NULLならシステム描画しない) */
	void (*exec_func)(DG_CHANL *, int, struct _DG_OBJS *, void *);
	int         pad2[3] ;
	
	FVECTOR      bound_min;      /* バウンディング最小値（def->lx,ly,lzの値）*/
    FVECTOR      bound_max;      /* バウンディング最大値（def->ux,uy,uzの値）*/
    FVECTOR      trans;          /* 中心からのオフセット（def->tx,ty,tzの値）*/
    DG_OBJ      objs[0];        /* 物体ハンドラのユニット */
} DG_OBJS ;

/* オブジェクト描画コールバック */
typedef void (*DG_ObjsCallback)(DG_CHANL *cp, int which, struct _DG_OBJS *objs, void *data);

/* 可視化 */
static inline void DG_VisibleObjs(DG_OBJS *objs)
{
	objs->flag &= ~DG_FLAG_INVISIBLE;
}

static inline void DG_VisibleObjsChanl(DG_OBJS *objs, int chanl)
{
	objs->flag &= ~(DG_FLAG_INVISIBLE0 << chanl);
}

/* 不可視化 */
static inline void DG_InvisibleObjs(DG_OBJS *objs)
{
	objs->flag |= DG_FLAG_INVISIBLE;
}

static inline void DG_InvisibleObjsChanl(DG_OBJS *objs, int chanl)
{
	objs->flag |= (DG_FLAG_INVISIBLE0 << chanl);
}

// ユーザ定義コールバックへのポインタ 
static inline void DG_SetObjsCallback(DG_OBJS *objs, DG_ObjsCallback exec_func, void *data)
{
	objs->exec_func = exec_func;
	objs->extend_data = data;
}

static inline void DG_SetObjsCallbackFunc(DG_OBJS *objs, DG_ObjsCallback exec_func)
{
	objs->exec_func = exec_func;
}

// ユーザ定義データへのポインタ
// ※オリジナルソースでは光学迷彩プラグインで使っている。
//   Xbox ではユーザ定義コールバックでも使えるようにする。 
static inline void DG_SetObjsExtendData(DG_OBJS *objs, void *data)
{
	objs->extend_data = data;
}

//-----------------------------------------------------------------------------
// マルチウェイトエンベロープオブジェクト関連
//-----------------------------------------------------------------------------

// 処理フラグ 
enum {
	DG_EVMOBJ_IRREACTION    = 0x0010,    /* 赤外線センサーに反応 */
	DG_EVMOBJ_SEMITRANS     = 0x0020,    /* 強制５０％半透明描画 */
	DG_EVMOBJ_LATTERDRAW    = 0x0080,    /* 後回し描画 */
	DG_EVMOBJ_INVISIBLE     = 0x0300,    /* 不可視属性（随時変更可能）*/
	DG_EVMOBJ_INVISIBLE0    = 0x0100,    /* 不可視属性（随時変更可能）（チャンネル０のみ）*/
	DG_EVMOBJ_INVISIBLE1    = 0x0200,    /* 不可視属性（随時変更可能）（チャンネル１のみ）*/
	DG_EVMOBJ_INVISIBLE2    = 0x0400,    /* 不可視属性（随時変更可能）（チャンネル２のみ）*/
	DG_EVMOBJ_INVISIBLE3    = 0x0800,    /* 不可視属性（随時変更可能）（チャンネル３のみ）*/
};

typedef struct _dg_vertex_evm {
	short		vx, vy, vz, vw ;		/* 頂点データ */
	short		nx, ny, nz ;		/* 法線 */
	short		u0, v0, q0 ;	/* テクスチャ座標 */
	short		u1, v1, q1 ;	/* テクスチャ座標 */
	short		u2, v2, q2 ;	/* テクスチャ座標 */
	short		weight[4] ;		/*  */
	short		index[4] ;		/*  */
} DG_VERTEX_EVM ;
// パケット管理構造体 
typedef struct _dg_evmpack {
	short         flag;             /* 処理フラグ */
	short         max_mats;         /* 最大使用スケルトン数 */
	short         n_verts;          /* パケットに含まれる頂点数 */
	short         n_step;           /* データのステップ数（1qword単位） */
	unsigned char n_uv_step[3];     /* 各ＵＶデータのステップ数（1qword単位） */
	char          mtex_type ;		/* マルチテクスチャタイプ */
	void          *tex0_ptr;        /* テクスチャ設定パケット転送タグへのポインタ */
	void          *tex1_ptr;        /* テクスチャ設定パケット転送タグへのポインタ */
	void          *tex2_ptr;        /* テクスチャ設定パケット転送タグへのポインタ */
	long64          matrix_list;      /* 使用スケルトン番号リスト */

	// XBOX追加 
	//unsigned short n_prims;         /* プリミティブ数 */
	//unsigned short min_index;       /* 描画で使用される最小の頂点インデックス */
	//unsigned short start_index;     /* 描画開始インデックス */
	//DG_INDEX_INFO  *index;
} DG_EVMPACK;

	/*
		マルチウェイトエンベロープオブジェクト管理構造体
	*/
typedef ALIGN16_DECL(struct) _dg_evmobj {
	FMATRIX       world;          /* 設置中心マトリクス */
	FMATRIX       *root;          /* 接続先マトリクス */
	FMATRIX       *light;         /* ライトマトリクスへのポインタ */
	EVM_DEF      *def;           /* モデルデータへのポインタ */
	int          flag;           /* 各種処理フラグ */
	int          chanl;          /* チャンネル */
	int          group_id;       /* 表示グループ */
#if 0
	void         *next;          /* 将来の拡張用 */
#else
	DG_VERTEX_EVM	*vbuff ;	/* 頂点バッファアドレス */
#endif
	FVECTOR		*skel_bounds ;		/* 各スケルトンのバウンディング(min,max)*n_skeleton */
	int          n_skeleton;     /* スケルトン数 */
	int          n_packet;       /* パケットの総数 */
	int          use_buffer;     /* マトリクスバッファ切り替え */
	//unsigned int vs_handle;      /* 頂点シェーダハンドル */	
	FMATRIX       *matrix[2];     /* スケルトンのマトリクス配列（ダブルバッファ） */
	void         *verts_ptr;     /* 頂点データへのポインタ */
	void         *norms_ptr;     /* 法線データへのポインタ */
	void         *uvs0_ptr;      /* ＵＶ０データへのポインタ */
	void         *uvs1_ptr;      /* ＵＶ１データへのポインタ */
	void         *uvs2_ptr;      /* ＵＶ２データへのポインタ */
	void         *weight_ptr;    /* ウェイト値データへのポインタ */
	DG_EVMPACK	 *packs;         /* パケット管理構造体 */
} DG_EVMOBJ ;

static inline void DG_VisibleEvmChanl(DG_EVMOBJ *evmobj, int chanl)
{
	evmobj->flag &= ~(DG_EVMOBJ_INVISIBLE0 << chanl);
}

static inline void DG_InvisibleEvmChanl(DG_EVMOBJ *evmobj, int chanl)
{
	evmobj->flag |= DG_EVMOBJ_INVISIBLE0 << chanl;
}

//-----------------------------------------------------------------------------
// 頂点アニメーション関連
//-----------------------------------------------------------------------------

	/*
		頂点アニメ管理ワーク
	*/
typedef struct _dg_v_anime{
	void    *mem[2];			/* 取得メモリアドレスポインタ */
	int     flag;				/* 初期化フラグ記憶 */
	DG_OBJ  *obj;				/* 初期化オブジェクト */
	void    *verts_top[2];	/* 頂点バッファの先頭アドレス *//* XBOXでは頂点バッファへのアドレスになる */
	void    *norms_top[2];	/* 法線バッファの先頭アドレス *//* XBOXでは未使用 */
	void    *uvs_top[2];	/* ＵＶバッファの先頭アドレス *//* XBOXでは未使用 */
	int     buffer_clock;	/* バッファ切り替え用クロック */
	int		pad[1] ;
} DG_VERTS_ANIME;

enum {
	DG_VANIME_VERTS = 0x0001,	/* 頂点アニメーションフラグ */
	DG_VANIME_NORMS = 0x0002,	/* 法線アニメーションフラグ */
	DG_VANIME_UVS   = 0x0004,	/* ＵＶアニメーションフラグ */
	DG_VANIME_SINGLE = 0x8000,	/* シングルバッファ作成 */

	/* DG_SUBPACK構造体で使用する */
	DG_VANIME_DELVERTS = 0x0010,	/* 頂点アニメーションフラグ */
	DG_VANIME_DELNORMS = 0x0020,	/* 法線アニメーションフラグ */
	DG_VANIME_DELUVS   = 0x0040,	/* ＵＶアニメーションフラグ */
};

static inline void DG_SwitchVAnimeBuffer( DG_VERTS_ANIME *v_anime )
{
    if(!(v_anime->flag & DG_VANIME_SINGLE)){
		v_anime->buffer_clock = 1 - v_anime->buffer_clock;
    }
}

extern void DG_MakeAnimVertsBuffer( DG_VERTS_ANIME *v_anime, DG_OBJ *obj, int flag );
extern void DG_RestartAnimVerts( DG_VERTS_ANIME *v_anime );
extern void DG_StopAnimVerts( DG_VERTS_ANIME *v_anime );
extern void DG_FreeAnimVertsBuffer( DG_VERTS_ANIME *v_anime );
extern void DG_RegistCommonVertex( FVECTOR *verts, int n_verts );
extern void DG_RefineStripVertex( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonVertex2( FVECTOR *verts, int n_verts );
extern void DG_RefineStripVertex2( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonNormal( FVECTOR *verts, int n_verts );
void DG_RegistCommonNormal2(FVECTOR *verts, int n_verts);
extern void DG_RefineStripNormal( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonUV( FVECTOR *verts, int n_verts );
extern void DG_RefineStripUV( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RefineStripUV2( DG_VERTS_ANIME *v_anime, short *index );


//-----------------------------------------------------------------------------
// 共有モデルオブジェクト関連
//-----------------------------------------------------------------------------

enum {
	DG_COMDL_DEFAULT =			0x00000,	/* デフォルトフラグ */
	DG_COMDL_SEMITRANS =		0x00002,	/* 半透明（＝オーバーレイ属性）フラグ */
	DG_COMDL_NOFOG =			0x00004,	/* フォグ無効フラグ */
	DG_COMDL_INVISIBLE =		0x00030,	/* 不可視フラグ */
	DG_COMDL_INVISIBLE0 =		0x00010,	/* 不可視フラグ（チャンネル０のみ） */
	DG_COMDL_INVISIBLE1 =		0x00020,	/* 不可視フラグ（チャンネル１のみ） */
	DG_COMDL_INVISIBLE2 =		0x00040,	/* 不可視フラグ（チャンネル２のみ） */
	DG_COMDL_INVISIBLE3 =		0x00080,	/* 不可視フラグ（チャンネル３のみ） */
};

typedef struct _dg_comdl_packet{
	//PS2依存 
	//DG_DMATAG	verts_tag;		/* 頂点転送用ＤＭＡタグ */
	//DG_DMATAG	norms_tag;		/* 法線転送用ＤＭＡタグ */
	//DG_DMATAG	uvs_tag;		/* ＵＶ転送用ＤＭＡタグ */
	//DG_DMATAG	datas_tag;		/* ワークデータ転送用ＤＭＡタグ(STCYCL,UNPACKR) */
	struct _dg_comdl_packet_data {
		unsigned int n_verts;		/* ＶＵ１に渡すデータ（処理頂点数） */
		unsigned int data_offset;	/* ＶＵ１に渡すデータ（バッファ先頭から使用許可領域までのオフセット＝10） */
		unsigned int pad1;			/* 現在未使用 */
		unsigned int pad2;			/* 現在未使用 */
		//PS2依存 
		//DG_GIFTAG	giftag0;		/* クリップポリゴン用描画設定ＧＩＦタグ */
		//DG_GSREG	prim;			/* クリップポリゴン用描画設定プリミティブ（prim） */
		//DG_GIFTAG	giftag1;		/* クリップポリゴン用描画ＧＩＦタグ */
		//DG_GIFTAG	giftag2;		/* 描画設定ＧＩＦタグ */
		//DG_GSREG	clamp;			/* 描画設定プリミティブ（clamp） */
		//DG_GSREG	tex2;			/* 描画設定プリミティブ（tex2） */
		//DG_GSREG	tex0;			/* 描画設定プリミティブ（tex0） */
		//DG_GSREG	alpha;			/* 描画設定プリミティブ（alpha） */
		//DG_GIFTAG	giftag3;		/* 描画ＧＩＦタグ */
	} dg_comdl_packet_data;		/* ワークデータ */
} DG_COMDL_PACKET;

typedef struct _dg_comdl_pos{
	FMATRIX		world;
	IVECTOR		color;
} DG_COMDL_POS;

typedef struct _dg_comdl {
	int				flag;			/* 各種フラグ */
	int				n_objs;			/* オブジェクト表示数 */
	int				group_id;		/* グループＩＤ */
	int				chanl;			/* 処理チャンネル */
	int				tri_id;			/* 使用テクスチャ */
	//int				pad[3];
#if 0
	int				pad[2] ;
	//LPDIRECT3DTEXTURE9  ptex;      /* Xbox追加/テクスチャ */
	DG_TEX			*tex ;
	DG_MDLPACK          *mdl_pack;  /* Xbox追加/モデル実体 */
	u_long64              alpha;      /* Xbox追加/alpha指定 */
	void			*vbuff ;
	//DG_INDEX_INFO       *index;     /* Xbox追加/インデックスバッファ情報 */	
	DG_COMDL_PACKET	packet[2];		/* 初期化パケット */
	DG_COMDL_POS	pos[0];			/* 各オブジェクトの座標及び色 */
#else
	void			*vbuff ;
	DG_TEX			*tex ;
	DG_MDLPACK		*mdl_pack ;
	DG_COMDL_PACKET	packet[2];		/* 初期化パケット */
	DG_COMDL_POS	pos[0];			/* 各オブジェクトの座標及び色 */
#endif
} DG_COMDL;


//-----------------------------------------------------------------------------
// 新プリミティブ関連
//-----------------------------------------------------------------------------

enum {
    /* プリミティブタイプ */
    DG_PRIM2_LINE =         0x00000001,    /* ラインストリップ */
    DG_PRIM2_POLY =         0x00000002,    /* ポリゴンストリップ */
    DG_PRIM2_CULLPOLY =     0x00000003,    /* カリング付きポリゴンストリップ */
    DG_PRIM2_SPRT =         0x00000004,    /* 通常スプライト */
    DG_PRIM2_RSPRT =        0x00000005,    /* 回転スプライト */

	/* Xbox新設プリミティブタイプ */
	DG_PRIM2_VLINE =        0x00000006,    /* 頂点バッファ直接使用版ラインリスト */
	DG_PRIM2_VTRIANGLE =    0x00000007,    /* 頂点バッファ直接使用版トライアングルリスト */
	DG_PRIM2_VPOLY =        0x00000008,    /* 頂点バッファ直接使用版トライアングルストリップ */

	DG_PRIM2_IVLINE =       0x00000009,    /* 頂点・インデクスバッファ直接使用版ラインリスト */
	DG_PRIM2_IVTRIANGLE =   0x0000000a,    /* 頂点・インデクスバッファ直接使用版トライアングルリスト */
	DG_PRIM2_IVPOLY =       0x0000000b,    /* 頂点・インデクスバッファ直接使用版トライアングルストリップ */

	/* 制御関連 */
    DG_PRIM2_TYPEMASK =     0x0000000f,    /* プログラム内部で使用 */
	DG_PRIM2_NOBUFFER =		0x00000100,		/* 頂点用メモリを確保しない（DG_SetPrim2Buffer()の呼び出し必須） */
    DG_PRIM2_SINGLEBUFF =   0x00000200,    /* シングルバッファフラグ（メモリ節約） */
    DG_PRIM2_ON_CAMERA =    0x00000400,    /* カメラからの相対座標による指定 */
    DG_PRIM2_BOUNDCHECK =   0x00000800,    /* バウンディングチェックの許可 */
    DG_PRIM2_INVISIBLE =    0x00003000,    /* 非表示フラグ */
    DG_PRIM2_INVISIBLE0 =   0x00001000,    /* 非表示フラグ（チャンネル０のみ） */
    DG_PRIM2_INVISIBLE1 =   0x00002000,    /* 非表示フラグ（チャンネル１のみ） */
    DG_PRIM2_INVISIBLE2 =   0x00004000,    /* 非表示フラグ（チャンネル２のみ） */
    DG_PRIM2_INVISIBLE3 =   0x00008000,    /* 非表示フラグ（チャンネル３のみ） */
    /* プリミティブ属性 */
    DG_PRIM2_SHADE =        0x00010000,    /* シェーディングＯＮ（スプライトでは禁止） */
    DG_PRIM2_TEX =          0x00020000,    /* テクスチャＯＮ */
    DG_PRIM2_FOG =          0x00040000,    /* フォグＯＮ */
    DG_PRIM2_ALPHA =        0x00080000,    /* アルファブレンドＯＮ */
    DG_PRIM2_ANTIALIASING = 0x00100000,    /* アンチエイリアシングＯＮ（使用注意！） */
    DG_PRIM2_SHADOWVOL =    0x00200000,    /* シャドウボリューム */
	/* XBOX拡張 */
	DG_PRIM2_CW = 			0x01000000,    /* 時計回り */
	DG_PRIM2_CCW = 			0x02000000,    /* 時計回り */
	DG_PRIM2_FRAMETEX =		0x10000000,		/* フレームバッファテクスチャ使用フラグ */
	DG_PRIM2_MAKEDINDEX =	0x20000000,		/* インデックス作成済みフラグ */
	/* デバッグ用 */
	DG_PRIM2_DEBUGCHECKED =	0x80000000,		/* デバッグ用チェック済みフラグ */ /* Xbox では無視 */
};

#if 0	// つかわない？ M.K
typedef struct _dg_prim2_param{
    FMATRIX      screen;         /* 透視変換マトリクス */
    int         n_verts;        /* 頂点数 */
    int         flag;           /* 処理フラグ */
    int         pad0;
    int         pad1;
} DG_PRIM2_PARAM;               /* サブパケットデータ */
#endif

/* ライン・ポリゴン用頂点要素 */
typedef struct _dg_prim2_uvrgb {
#if 0
	unsigned short r, g, b, a;     /* 頂点カラー */
#else
	unsigned char r, r_pad, g, g_pad, b, b_pad, a, a_pad;        /* スプライトカラー */
#endif
	unsigned short u, v, q;        /* ＵＶＱ */
	unsigned short f;              /* フラグ(描画キック：0x0fff、頂点キックのみ：0x8fff) */
    /* ＜補足＞DG_PRIM2_CULLPOLYの時のフラグ設定
       両面表示：0x0fff 右回り表示：0x0020 左回り表示：0x0000 非表示：0x8fff */
} DG_PRIM2_UVRGB;

/* スプライト用頂点要素 */
typedef struct _dg_prim2_wh {
#if 0
	unsigned short r, g, b, a;        /* スプライトカラー */
#else
	unsigned char r, r_pad, g, g_pad, b, b_pad, a, a_pad;        /* スプライトカラー */
#endif
    unsigned short u0, v0, q0, f0;    /* 左上ＵＶＱ(f0は無視) */
    short          w, h, pad0, pad1;  /* スプライトサイズ */
    unsigned short u1, v1, q1, f1;    /* 右下ＵＶＱ(f1は無視) */
} DG_PRIM2_UVRGBWH;

/* パケット管理構造体 */
typedef struct _dg_prim2_packet {
	int				sort_z;        /* ソート用Ｚ値 */
	void			*next_addr;    /* ソート用リスト構造ポインタ */
	void			*prim;         /* DG_PRIM2へのポインタ */
	short			type;          /* プリミティブタイプ */
	unsigned short	flag;          /* ローカルフラグ */
	short			n_verts;       /* 転送頂点数 */
	u_short			n_indices;       /* DG_PRIM2_V* 用。DrawPrimitiveに渡すプリミティブ個数 */
	
	FVECTOR			*pos_addr;     /* 頂点座標アドレス */
	void			*uvrgb_addr;   /* 頂点要素アドレス(ＵＶ、ＲＧＢなど) */

	/* XBOX 追加 */
	void			*vbuff ;		/* 頂点バッファアドレス（毎セットアップ毎に動的に取られる） */
	void			*ibuff ;		/* インデックスバッファ（ポリゴン及びラインのときのみ初期化時に確保） */
	int				flag2 ;			/* 拡張フラグ */
	int				pad[2] ;
	//u_short			d3dtype;		// D3DPRIMITIVETYPE IBuffer/VBuffer タイプ用 
	//u_short			MinIndex;		// VBuffer 最小値・始点 IBuffer/VBuffer タイプ用 
	//u_short			NumVertices;	// VBuffer 使用頂点数 IBuffer タイプ用 
	//u_short			StartIndex;		// IBuffer 始点 IBuffer タイプ用 

	// XBOX追加 
    //unsigned int    index_size;    /* インデックスバッファをロックするサイズ */
	//unsigned int    size;          /* 頂点バッファ全体のサイズ */
	//unsigned short  verts_type;    /* 頂点バッファタイプ */
	//short           verts_buf_id;  /* 頂点バッファのID(割り当てが無い場合(-1)) */

} DG_PRIM2_PACKET;

/* 新プリミティブオブジェクト */
typedef ALIGN16_DECL(struct) _dg_prim2 {
	FMATRIX			world;         /* マトリクス */
	FMATRIX			screen;        /* 透視変換マトリクス = world * eye_pers */
	FVECTOR			bound_min;     /* バウンディング最小値(DG_PRIM2_BOUNDCHECK指定時のみ) */
	FVECTOR			bound_max;     /* バウンディング最大値(DG_PRIM2_BOUNDCHECK指定時のみ) */
	FMATRIX			*root;         /* 接続マトリクスへのポインタ */
	int				flag;          /* 処理フラグ */
	int				group_id;      /* 表示グループ */
	short			chanl;         /* 使用チャンネル */
	short			type;          /* プリミティブタイプ */
	short			n_prims;       /* パケットの個数 */
	short			packet_verts;  /* １プリミティブの頂点数 */
	int				buffer_clock;  /* ダブルバッファ切り替え用 */
	FVECTOR			*pos[2];       /* 頂点座標配列 */
	void			*uvrgb[2];     /* ＵＶ、ＲＧＢ値他頂点データ配列 */
//    DG_PRIM2_PARAM  prim_param[2]; /* プリミティブパラメータ */
	DG_TEX_TRANS	tex_trans;     /* テクスチャパラメータ転送パケット */
	DG_PRIM2_PACKET	*packet[2];    /* パケットデータ */
	int				raise;         /* ソート用優先上げ値 */
	int				shadowID;      /* 影パレットID */
	char			*fname;        /* デバッグ用 */

	/* Xbox追加 */
	void			*vbuff ;		/*  */
	void			*ibuff ;		/*  */
	//void (*exec_func)(DG_CHANL *, int, struct _dg_prim2_packet *, void *);
	//void *extend_data;
} DG_PRIM2 ;

#define DG_MAX_SHADOWPALLET	16
extern void DG_SetShadowPallet(int shadowID, DWORD dwSrcBlend, DWORD dwDestBlend, DWORD dwBlendOp, D3DCOLOR rgba);

typedef void (*DG_Prim2Callback)(DG_CHANL *cp, int which, struct _dg_prim2_packet *packet, void *data);

#if 0
static inline void DG_SetPrim2Callback(DG_PRIM2 *prim2, DG_Prim2Callback exec_func, void *data)
{
	prim2->exec_func = exec_func;
	prim2->extend_data = data;
}
static inline void DG_SetPrim2CallbackFunc(DG_PRIM2 *prim2, DG_Prim2Callback exec_func)
{
	prim2->exec_func = exec_func;
}
static inline void DG_SetPrim2ExtendData(DG_PRIM2 *prim2, void *data)
{
	prim2->extend_data = data;
}
#endif

/* パケットバッファ切り替え */
static inline void DG_SwitchBuffPrim2(DG_PRIM2 *prim)
{
    prim->buffer_clock = 1 - prim->buffer_clock;
}

/* Xbox新設 */
static inline DG_PRIM2_PACKET *DG_GetActivePacketPrim2(DG_PRIM2 *prim)
{
	return prim->packet[prim->buffer_clock];
}

/* 可視化 */
static inline void DG_VisiblePrim2(DG_PRIM2 *prim)
{
    prim->flag &= ~DG_PRIM2_INVISIBLE;
}
static	inline	void	DG_VisiblePrim2Chanl( DG_PRIM2 *prim, int chanl )
{
	prim->flag &= ~( DG_PRIM2_INVISIBLE0 << chanl ) ;
}

/* 不可視化 */
static inline void DG_InvisiblePrim2(DG_PRIM2 *prim)
{
    prim->flag |= DG_PRIM2_INVISIBLE;
}
static	inline	void	DG_InvisiblePrim2Chanl( DG_PRIM2 *prim, int chanl )
{
	prim->flag |= DG_PRIM2_INVISIBLE0 << chanl ;
}

/* パケット優先値設定 */
static inline void DG_RaisePrim2(DG_PRIM2 *prim, int raise)
{
    prim->raise = raise;
}

/* アルファブレンディングモードの設定 */
static inline void DG_SetPrim2Alpha(DG_PRIM2 *prim, u_long64 alpha)
{
	prim->tex_trans.alpha.data = alpha;
}

//-----------------------------------------------------------------------------
// その他のオブジェクト関連
//-----------------------------------------------------------------------------

typedef struct _dg_spot {
	FMATRIX       world;          /*  */
    FMATRIX       screen;         /* 投影マトリクス */
    FMATRIX       *root;          /*  */
    int          flag;           /* フラグ */
    unsigned int color;          /* 色 */
    float        angle;          /* 影響角度  */
    float        range;          /* 影響距離 */
    int          shadow_id;      /* 影グループ */
    void         *objs;          /* 平行投影影用オブジェクトへのポインタ */
    int    	     pad[1];
} DG_SPOT;

/*----------------------------------------------------------------*/
/* ＜ＬＩＢＤＧプラグイン管理機構＞ (add 2001/05/09 M.K) */

/* 処理フェーズレベル */
enum {
	DG_PLUGIN_PHASE_FIRST		= 0x01,	/* 描画開始前 */
	DG_PLUGIN_PHASE_NORMAL		= 0x02,	/* 不透明モデル描画後 */
	DG_PLUGIN_PHASE_AFTER		= 0x04,	/* 半透明モデル描画後 */
	DG_PLUGIN_PHASE_LAST		= 0x08,	/* 最終描画 */
	DG_PLUGIN_PHASE_END1		= 0x10,	/* リザーブ */
} ;

/* プライオリティ設定 */
enum {
	DG_PLUGIN_PRIO_NORMAL		= 0,		/* デフォルトプライオリティ */
};

/* 初期化フラグ */
enum {
	DG_PLUGIN_FLAG_OBJBUFFER	= 0x0001,	/* オブジェクトバッファ使用 */
	DG_PLUGIN_FLAG_ENABLE0		= 0x0010,	/* チャンネル０で動作 */
	DG_PLUGIN_FLAG_ENABLE1		= 0x0020,	/* チャンネル１で動作 */
	DG_PLUGIN_FLAG_ENABLE2		= 0x0040,	/* チャンネル２で動作 */
	DG_PLUGIN_FLAG_ENABLE3		= 0x0080,	/* チャンネル３で動作 */
	DG_PLUGIN_FLAG_ENABLEMENU	= 0x0100,	/* チャンネル４（メニュー用）で動作 */
	DG_PLUGIN_FLAG_ENABLE		= 0x00f0,	/* 全チャンネルで動作 */
};

typedef void (*DG_PluginCallback)( struct _dg_chanl *cp, int which, struct _dg_obj_buffer *obj_buffer, int status );

/* プラグイン管理構造体 */
typedef struct _dg_plugin {
	struct _dg_plugin	*prev ;
	struct _dg_plugin	*next ;
	int		flag ;							/* 初期化フラグ */
	int		object_id ;						/* オブジェクト固有ＩＤ */
	int		phase ;							/* 実行フェーズ指定 */
	int		priority ;						/* 同フェーズ内での優先レベル */
	struct _dg_obj_buffer	*obj_buffer ;	/* 確保オブジェクトバッファ */
	DG_PluginCallback		exec_func ;		/* 実行コールバック関数 */
} DG_PLUGIN ;


/*----------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// チャンネル処理ユニットタイプ
//-----------------------------------------------------------------------------

enum {
	DG_SCREEN_CHANL,
	DG_BOUND_CHANL,
	DG_PRIM_CHANL,
	DG_CHAIN_CHANL,
	DG_CHANL_UNIT_MAX,
};

//-----------------------------------------------------------------------------
// DG_DisplayStatus用定義
//-----------------------------------------------------------------------------

enum {
	DG_STATE_IR_MODE = 0x00000001,    /* 赤外線モード */
};

//-----------------------------------------------------------------------------
// DG常駐頂点シェーダ/ピクセルシェーダ
//-----------------------------------------------------------------------------

// A ... Additional light
// W ... 1 Weight
// L ... Light
// P ... Point light
enum {
	DG_VSHADER_OBJ,
	DG_VSHADER_OBJ_L,
	DG_VSHADER_OBJ_W,
	DG_VSHADER_OBJ_WL,
	DG_VSHADER_OBJ_A,
	DG_VSHADER_OBJ_AL,
	DG_VSHADER_OBJ_AW,
	DG_VSHADER_OBJ_AWL,
	DG_VSHADER_OBJ_P,
	DG_VSHADER_OBJ_NUM,
};

enum {
	DG_PSHADER_NONE,
	DG_PSHADER_POINTLIGHT,
	//DG_PSHADER_NORMAL,
	DG_PSHADER_TEST,
	DG_PSHADER_NUM,
};

extern DWORD DG_PixelShaderHandle[DG_PSHADER_NUM];

//-----------------------------------------------------------------------------
// ＬＩＢＤＧ定義グローバル変数
//-----------------------------------------------------------------------------

extern int          DG_Clock;                /* バッファ切り替えクロック */
extern int          DG_TickCount;            /* 起動からの総VSyncカウント数 */
extern int          DG_FrameRate;            /* フレームレート調整 */
extern int          DG_CurrentField;         /* odd or even */
extern DG_CHANL     DG_Chanls[];             /* チャンネル構造体 */
extern DG_OBJ_QUEUE DG_ObjQueue;             /* オブジェクトキュー構造体 */
//extern u_long128    *DG_CurrentDmaAddr;      /* カレントＤＭＡ先頭バッファ */
extern CVECTOR      DG_FogColor;             /* 遠景色 */
extern CVECTOR		DG_FogColorMaster ;		/**/
extern float        DG_FogParam1;            /* フォグ計算パラメータ */
extern float        DG_FogParam2;            /* フォグ計算パラメータ */
extern FVECTOR      DG_FogParam;             /* Xbox新設/フォグ計算パラメータのコピー*/
extern int          DG_FrameCount;           /*  */
extern int          DG_PrivilegeMode;        /* 特権モードフラグ */
extern int          DG_LastWhich;            /* 最終更新バッファ */
extern int          DG_DisplayStatus;        /* 各種画面表示状態 */
extern int          DG_ActiveShadowFlag;     /* 影チャンネル有効フラグ */
extern int			DG_CurrentBackBuffer ;		/* 使用中バックバッファ番号 */

extern long64         DG_UnDrawFrameCount;
#define DG_UNDRAW_MAX (0x7fff0000)

extern SVECTOR      DG_ZeroSVector;
extern IVECTOR      DG_ZeroIVector;
extern FVECTOR       DG_ZeroVector;
extern FMATRIX       DG_UnitMatrix;
extern FMATRIX       DG_ZeroMatrix;
extern FMATRIX       DG_LightMatrix;
extern FMATRIX       DG_ColorMatrix;
extern FMATRIX       DG_AdditionalLightMatrix;
extern FMATRIX       DG_AdditionalColorMatrix;

extern int                DG_MaxTextures;
//extern DG_TEXTURE_LIST    DG_TextureList[];
extern DG_TEXTURE_LIST	*DG_TextureList[] ;	// new version
extern DG_TEX			DG_SystemTexture[] ;

// XBOX / Windows
#define DG_POINTLIGHT_MAX     (1)
extern int DG_AdditionalLightFlag;
extern int DG_PointLightFlag;
extern FVECTOR DG_PointLightPos[DG_POINTLIGHT_MAX];
extern FVECTOR DG_PointLightCol[DG_POINTLIGHT_MAX];
extern FVECTOR DG_PointLightParam[DG_POINTLIGHT_MAX];

extern LPDIRECT3D9        g_pD3D;
extern LPDIRECT3DDEVICE9  g_pd3dDevice;
extern LPDIRECT3DTEXTURE9 DG_ShadowTexture;
extern D3DPRESENT_PARAMETERS g_d3dpp;

//-----------------------------------------------------------------------------
// マクロ関係
//-----------------------------------------------------------------------------

// float -> DWORD
static inline DWORD DG_FtoDW(float f) {return *((DWORD *)&f);}

// ベクトルのコピー 
#define DG_COPY_VEC(a, b) (*(FVECTOR *)(a) = *(FVECTOR *)(b))
// マトリクスのコピー 
#define DG_COPY_MAT(a, b) (*(FMATRIX *)(a) = *(FMATRIX *)(b))
// 汎用128ビットコピー 
typedef struct tagDG_ULONG128 {
	u_long64 l, m;
} DG_ULONG128;
#define DG_COPY128(a, b) {*(DG_ULONG128 *)(a) = *(DG_ULONG128 *)(b);}
// 汎用64ビットコピー 
#define DG_COPY64(a, b)  {*(u_long64 *)(a) = *(u_long64 *)(b);}
// float --> int 変換。PSX2での内部関数呼び出し回避用 
#define DG_FTOI(_f)      ((int)(_f))

// PS2でのFPU直叩き系命令が続く 
//#define DG_MAX(_a, _b)   ((_a) > (_b) ? (_a) : (_b))
//#define DG_MIN(_a, _b)   ((_a) < (_b) ? (_a) : (_b))
//#define DG_SQRT(_m)      ((float)sqrt(_m))
static float inline DG_MAX( float a, float b )
{
	float	c ;
	__asm{
		movss		xmm0, a
		maxss		xmm0, b
		movss		c, xmm0
	}
	return ( c );
}
static float inline DG_MIN( float a, float b )
{
	float	c ;
	__asm{
		movss		xmm0, a
		minss		xmm0, b
		movss		c, xmm0
	}
	return ( c );
}
static float inline DG_SQRT( float a )
{
	float	b ;
	__asm{
		sqrtss		xmm0, a
		movss		b, xmm0
	}
	return ( b );
}
//#define DG_RSQRT(_m)     (1.0f / (float)sqrt(_m))
static float inline DG_RSQRT( float a )
{
	static float	var_0_5 = 0.5f ;
	static float	var_1_5 = 1.5f ;
	float	b ;
	__asm{
		movss		xmm0, a
		rsqrtss		xmm1, xmm0		; SSEで近似値を求める
		mulss		xmm0, xmm1		; ニュートンラフソン法で精度を上げる
		mulss		xmm0, xmm1
		mulss		xmm0, xmm1
		mulss		xmm0, var_0_5
		mulss		xmm1, var_1_5
		subss		xmm1, xmm0		; 最終精度の結果を取得
		movss		b, xmm1
	}
	return ( b );
}
//#define DG_FABS(_x)      ((_x) > 0.0f ? (_x) : - (_x))
static float inline DG_FABS( float x ) {
	__asm { and x,7fffffffh };
	return (x) ;
}


// PS2/アキュムレータ演算 
// ※適当なので 
extern float DG_ACC; // アキュムレータ 
#define	DG_ADDA(_x,_y)	{DG_ACC = (_x) + (_y);} /* ACC = x + y */
#define	DG_SUBA(_x,_y)	{DG_ACC = (_x) - (_y);} /* ACC = x - y */
#define	DG_MULA(_x,_y)	{DG_ACC = (_x) * (_y);} /* ACC = x * y */
#define	DG_MADDA(_x,_y)	{DG_ACC += (_x) * (_y);}/* ACC += x * y */
#define	DG_MSUBA(_x,_y)	{DG_ACC -= (_x) * (_y);}/* ACC -= x * y */
static inline float DG_MADD(float _x, float _y) {return DG_ACC + _x * _y;}
static inline float DG_MSUB(float _x, float _y) {return DG_ACC - _x * _y;}

// プリミティブ座標設定用ユーティリティ 
//   x, y は画面座標 
//   u, v はテクスチャ座標 
#define DG_POS_X(_x) (((_x) + 2048 - DRAW_WIDTH /2) << 4)
#define DG_POS_Y(_y) (((_y) + 2048 - DRAW_HEIGHT/2) << 4)
#define DG_POS_U(_u) ((_u) << 4)
#define DG_POS_V(_v) ((_v) << 4)


//-----------------------------------------------------------------------------
// バウンディングチェック用
//-----------------------------------------------------------------------------

// clipw 命令エミュレーション用フラグ(VU User's Manual参照) '
enum {
	CLIP_X0_FLAG = (1 << 0),  // x > + w
	CLIP_X1_FLAG = (1 << 1),  // x < - w
	CLIP_Y0_FLAG = (1 << 2),  // y > + w
	CLIP_Y1_FLAG = (1 << 3),  // y < - w
	CLIP_Z0_FLAG = (1 << 4),  // z > + w
	CLIP_Z1_FLAG = (1 << 5),  // z < - w

	// 上のフラグをまとめた定義 
	CLIP_X_FLAG = (CLIP_X0_FLAG|CLIP_X1_FLAG),
	CLIP_Y_FLAG = (CLIP_Y0_FLAG|CLIP_Y1_FLAG),
	CLIP_Z_FLAG = (CLIP_Z0_FLAG|CLIP_Z1_FLAG),
	CLIP_XY_FLAG = (CLIP_X_FLAG|CLIP_Y_FLAG),
	CLIP_YZ_FLAG = (CLIP_Y_FLAG|CLIP_Z_FLAG),
	CLIP_ZX_FLAG = (CLIP_Z_FLAG|CLIP_X_FLAG),
	CLIP_FLAG = (CLIP_X_FLAG|CLIP_Y_FLAG|CLIP_Z_FLAG),
};


//-----------------------------------------------------------------------------
// 2Dプリミティブ(DirectX新設)関連
//-----------------------------------------------------------------------------

// 2次元スプライト 
typedef struct tagDG_SPRT {
	float x, y, w, h;
	float u0, v0;
	float u1, v1;
	float z;
	D3DCOLOR rgba;
} DG_SPRT;

// 2次元ライン 
typedef struct tagDG_LINE {
	float x0, y0;
	D3DCOLOR rgba0;
	float x1, y1;
	D3DCOLOR rgba1;
	float z;
} DG_LINE;

//-----------------------------------------------------------------------------
// パーティクル(ポイントスプライト)(DirectX新設)関連
//-----------------------------------------------------------------------------

enum {
    DG_PARTICLE_SCALEENABLE = 0x00000001,  /* スケール計算ON */

	/* 制御関連 */
    //DG_PARTICLE_TYPEMASK =     0x0000000f,    /* プログラム内部で使用 */
    //DG_PARTICLE_SINGLEBUFF =   0x00000200,    /* シングルバッファフラグ（メモリ節約） */
    //DG_PARTICLE_ON_CAMERA =    0x00000400,    /* カメラからの相対座標による指定 */
    //DG_PARTICLE_BOUNDCHECK =   0x00000800,    /* バウンディングチェックの許可 */
    DG_PARTICLE_INVISIBLE =    0x00003000,    /* 非表示フラグ */
    DG_PARTICLE_INVISIBLE0 =   0x00001000,    /* 非表示フラグ（チャンネル０のみ） */
    DG_PARTICLE_INVISIBLE1 =   0x00002000,    /* 非表示フラグ（チャンネル１のみ） */
    DG_PARTICLE_INVISIBLE2 =   0x00004000,    /* 非表示フラグ（チャンネル２のみ） */
    DG_PARTICLE_INVISIBLE3 =   0x00008000,    /* 非表示フラグ（チャンネル３のみ） */

    /* プリミティブ属性 */
    //DG_PARTICLE_SHADE =        0x00010000,    /* シェーディングＯＮ（スプライトでは禁止） */
    DG_PARTICLE_TEX =          0x00020000,    /* テクスチャＯＮ */
    //DG_PARTICLE_FOG =          0x00040000,    /* フォグＯＮ */
    //DG_PARTICLE_ALPHA =        0x00080000,    /* アルファブレンドＯＮ */
};

typedef struct tagDG_PARTICLE {
	MATRIX         world;
	int            chanl;
	int            flag;
	float          size;         // サイズ 
	float          size_min;     // 最小サイズ 
	float          scale_a, scale_b, scale_c; // D3DRS_POINTSCALE_A to C
	int            buffer_clock; // バッファ切り替えクロック 
	int            n_verts_max;  // 最大頂点数 
	int            n_verts[2];   // 頂点数 
	DG_TEX         *tex;         // テクスチャ 
	DG_POINTVERTEX *point[2];    // 頂点データ 
} DG_PARTICLE;

//-----------------------------------------------------------------------------
// 外部公開関数プロトタイプ
//-----------------------------------------------------------------------------


// 2d_prim.cpp
void DG_Draw2DPrim(void);
void DG_Draw2DSprt(DG_SPRT *pSprt, int nSprt);
void DG_Draw2DLine(DG_LINE *pLine, int nLine);
void DG_Set2DSprtTex(unsigned int code);

// chain.cpp
void DG_InitPixelShader(void);
void DG_InitObjVertexShader(void);
void DG_ChainChanl(DG_CHANL *cp, int which);
void DG_SortChainChanl(DG_CHANL *cp, int which);

/* xchain2.c */
extern void DG_Chain2Chanl(DG_CHANL *cp, int which);
extern void DG_Chain2ChanlLatter(DG_CHANL *cp, int which);
extern void DG_InitMultiTexObjVertexShader(void);

// xopt_cmf.c
void DG_AddPluginOptcmf( void );

// chanl.cpp
void DG_SetDrawEnv(DG_CHANL *cp, int x, int y, int w, int h);
void DG_ChangeDrawLimit(DG_CHANL *cp, int x1, int y1, int x2, int y2);
void *DG_PopDefaultDrawEnv(DG_CHANL *cp, void *addr);
DG_OBJ_BUFFER *DG_MakeUserObjectBuffer(int max, int id);
void DG_FreeUserObjectBuffer(DG_OBJ_BUFFER *obj_buff);
void DG_InitChanlSystem(int flag);
void DG_ResetChanlSystem(int flag);
void DG_DrawChanlSystem(int which);
void DG_ClearChanlSystem(int which);
void DG_UnDrawChanlSystem(int which);
void DG_SortChanlSystem(int which);

int DG_QueueObjs(DG_OBJS *objs);
void DG_DequeueObjs(DG_OBJS *objs);
DG_OBJS* DG_SearchQueueObjs(DG_OBJS *objs);
int DG_QueueShdwwriteObjs(DG_OBJS *objs);
void DG_DequeueShdwwriteObjs(DG_OBJS *objs);
int DG_QueueSpotObjs(DG_SPOT *objs);
void DG_DequeueSpotObjs(DG_SPOT *objs);
int DG_QueueEvmObj(DG_EVMOBJ *evmobj);
void DG_DequeueEvmObj(DG_EVMOBJ *evmobj);
int DG_QueuePrim2(DG_PRIM2 *objs);
void DG_DequeuePrim2(DG_PRIM2 *objs);
int DG_QueueParticle(DG_PARTICLE *objs);
void DG_DequeueParticle(DG_PARTICLE *objs);
int DG_QueueUserObject(DG_OBJ_BUFFER *obj_buff, void *objs);
void DG_DequeueUserObject(DG_OBJ_BUFFER *obj_buff, void *objs);
int DG_SetPrivilegeMode(int mode);
extern int DG_SetReverseDrawOrderMode( int mode );
void DG_StopMainChanlSystem(void);
void DG_RestartMainChanlSystem(void);


// comdl.cpp
void DG_InitComdlVertexShader(void);
void DG_AddPluginComdl(void);
void DG_DeletePluginComdl(void);
int DG_QueueComdlObjs(DG_COMDL *comdl);
void DG_DequeueComdlObjs(DG_COMDL *comdl);
DG_COMDL *DG_MakeComdl(DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl);
void DG_FreeComdl(DG_COMDL *comdl);

/* xd3d.c */
DWORD DG_GetVisibilityTestID(void);
void DG_BeginVisibilityTest(void);
void DG_EndVisibilityTest(DWORD Index);
void DG_GetVisibilityTestResultBlock(DWORD Index, UINT* pResult, ULONGLONG* pTimeStamp);

void DG_InitRenderState(void);
#if 0 //BP_RENDER
void DG_CreatePalette(D3DPALETTESIZE Size, LPDIRECT3DPALETTE8 *ppPalette);
void DG_SetPalette(DWORD Stage, LPDIRECT3DPALETTE8 pPalette);
#endif
void DG_SetFSAA(int type);
void DG_SetDxFogColor(void);
void DG_InitTextureStageState(int Stage);
void DG_Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
			  D3DCOLOR Color, float Z, DWORD Stencil);
void DG_BeginScene(void);
void DG_EndScene(void);
#ifdef KP_XBOX
void DG_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect,
				void *pUnused0, void *pUnused1);
#else
void DG_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect,
				HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
#endif


//void DG_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
inline void DG_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	IDirect3DDevice9_SetRenderState( g_pd3dDevice, State, Value );
}
void DG_SetTransform(D3DTRANSFORMSTATETYPE State, CONST MATRIX *pMatrix);
void DG_CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
						   IDirect3DVertexBuffer9** ppVertexBuffer);
void DG_CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format,
						  D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer);
void DG_SetRenderTarget(IDirect3DSurface9* pRenderTarget,
						IDirect3DSurface9* pNewZStencil);
//void DG_SetTexture(DWORD Stage, IDirect3DBaseTexture8 *pTexture);
extern void DG_SetTexture( int stage, DG_TEX_TRANS *tex_trans );
extern void DG_SetTextureDirect( int stage, IDirect3DBaseTexture9 *pTexture );
void DG_GetRenderTarget(IDirect3DSurface9** ppRenderTarget);
void DG_GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface);
void DG_CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage,
					  D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture);
void DG_SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
void DG_DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
void DG_DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
						CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
void DG_DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices,
							 UINT StartIndex, UINT PrimitiveCount);
void DG_DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex,
							   UINT NumVertices, UINT PrimitiveCount,
							   CONST void* pIndexData, D3DFORMAT IndexDataFormat,
							   CONST void* pVertexStreamZeroData,
							   UINT VertexStreamZeroStride);
void DG_DrawVertices( D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount );
void DG_DrawIndexedVertices( D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST WORD *pIndexData );
void DG_DrawVerticesUP( D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount,
					   CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride );
void DG_DrawIndexedVerticesUP( D3DPRIMITIVETYPE PrimitiveType,
							  UINT VertexCount,
							  CONST void* pIndexData,
							  CONST void* pVertexStreamZeroData,
							  UINT VertexStreamZeroStride );
void DG_SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT Stride);
void DG_SetIndices(IDirect3DIndexBuffer9* pIndexData, UINT BaseVertexIndex);

void DG_SetAlphaMode(u_long64 type);
void DG_SetSemitransAdd(void);
void DG_SetSemitransSub(void);
void DG_SetSemitransNormal(void);
void DG_SetSemitransShadow(void);

//void DG_CreatePixelShader(int id, DWORD* pHandle);
//void DG_CreateVertexShader(int id, const DWORD* pDecl, DWORD* pHandle);
void	DG_CreatePixelShader( DG_PIXELSHADER *shader, void *code );
void	DG_CreateVertexShader( DG_VERTEXSHADER *shader, void *code, const DWORD* pDecl );
void	DG_MakePixelShader( DG_PIXELSHADER *shader, void *code );
void DG_SetVertexShaderConstant(DWORD Register,	CONST void* pConstantData,
								DWORD  ConstantCount);
void DG_SetVertexShaderConstantMatrix(DWORD Register,
									  CONST MATRIX *Matrix,
									  DWORD nMatrix);
void DG_SetPixelShaderConstant(DWORD Register,	CONST void* pConstantData,
								DWORD  ConstantCount);
void DG_ResetShader(void);
void DG_ResetVertexShader(void);
void DG_ResetPixelShader(void);
void DG_SetVertexShader(DWORD Handle);
void DG_SetPixelShader( DG_PIXELSHADER *shader );
//void DG_SetPixelShader(DWORD Handle);
void DG_DeleteVertexShader(DWORD Handle);
void DG_DeletePixelShader(DWORD Handle);
extern void	DG_MakePixelShader( DG_PIXELSHADER *shader, void *code );
extern void DG_FreePixelShader( DG_PIXELSHADER *shader );


/* xdgd.cpp */
float DG_GetHSyncTimer(void);
float DG_GetMsecTimer(void);
int DG_GetFrameTimer(void);
void DG_SetDrawMark( void );

void DG_SetBgColor(unsigned int color);
void DG_ResetSystem(void);
void DG_ResetTexture(void);
#ifdef KP_XBOX
void DG_StartDaemon(void);
#else
void DG_StartDaemon(HINSTANCE hInstance); // windows環境ではhInstanceが必要 
#endif
void DG_KillDaemon(void);
extern void DG_SetDrawMark( void );
extern void DG_SetDrawStartMark( void );
extern void DG_SetDrawEndMark( void );
extern void DG_DrawSync( void );

// evmobjs.cpp
void DG_InitEvmVertexShader(void);
void DG_ChainEvmChanl(DG_CHANL *cp, int which);
void DG_ChainEvmChanlLatter(DG_CHANL *cp, int which);
DG_EVMOBJ *DG_MakeEvmObj(EVM_DEF *def, int flag, int chanl);
void DG_FreeEvmObj(DG_EVMOBJ *evmobj);
void DG_WriteEvmMdlPaketUV(int tri_code, EVM_PACK *pack);
void DG_ConnectObjs(DG_OBJS *parent, DG_OBJS *child);
void DG_DisconnectObjs(DG_OBJS *parent, DG_OBJS *child);


/* xframe.c */
void DG_SetFogColor(int r, int g, int b);
void DG_SetFogParam(float fNear, float fFar);
void DG_SetClipParam(float fNear, float fFar);
void DG_InitFrameSystem(void);
void DG_StartFrame(void);
void DG_EndFrame(void);
void DG_ViewScreenMatrix(FMATRIX *res,
						 float scrz,               // スクリーンまでの距離 
						 float ax, float ay,       // アスペクト比 
						 float cx, float cy,       // スクリーンの中心座標 
						 float zmin, float zmax,   // Zバッファ最小値/最大値 
						 float nearz, float farz); // ニアクリップ面のZ,ファークリップ面のZ 
void DG_MakeCameraMatrix(MATRIX *mat, VECTOR *vecFrom, VECTOR *vecTo);
void DG_SetCamera(DG_CHANL *chanl, MATRIX *mat, float fScreen);
void DG_SetCamera2(DG_CHANL *chanl, VECTOR *vecFrom, VECTOR *vecTo, float screen);
void DG_FrameChanl(DG_CHANL *cp, int which);
void DG_AddCurrentGroup(int id);
void DG_DeleteCurrentGroup(int id);
void DG_AddCurrentGroup2(int chanl, int id);
void DG_DeleteCurrentGroup2(int chanl, int id);
extern void DG_MakeStencilMask( void );

// light.cpp
void DG_PointLightEnable(void);
void DG_PointLightDisable(void);
void DG_SetPointLightStatus(int flag);
void DG_SetPointLightCol(int n, VECTOR *v);
void DG_SetPointLightPos(int n, VECTOR *v);
void DG_ClearPointLight(int n);

void DG_SetAdditionalLightVec(int n, VECTOR *v);
void DG_SetAdditionalLightCol(int n, VECTOR *v);
void DG_SetAdditionalLightPow(int n, float power);
void DG_SetAdditionalAmbient(VECTOR *v);
void DG_SetAdditionalLightStatus(int flag);
void DG_AdditionalLightEnable(void);
void DG_AdditionalLightDisable(void);

void DG_ClearTmpLight(void);
int DG_SetTmpLight(FVECTOR *point, float r_range, float e_range);
int DG_SetTmpLight2(FVECTOR *point, float r_range, float e_range, int color, int flag);
int		DG_SetTmpSpotLight( FVECTOR *point, FVECTOR *dir, float range, float umbra, float penumbra, int color, int flag );
void DG_InitLightSystem(void);
void DG_SetAmbient(int r, int g, int b);
void DG_SetMainLightDir(int x, int y, int z);
void DG_SetMainLightCol(int r, int g, int b);
void DG_ResetFixedLight(void);
void DG_SetFixedLightMap(LIT_DEF *light, int map_id);
void DG_SetDxMainLight(MATRIX *light, MATRIX *color);
void DG_SetDxLight(MATRIX *light, MATRIX *color);
int DG_GetLightMatrix(VECTOR *pos, MATRIX *light);
int DG_GetLightMatrixFix(VECTOR *pos, MATRIX *light);
void DG_SetLightMatrix(DG_OBJS *objs, MATRIX *light);
extern void DG_SwitchLightSphere( FVECTOR *pos, float r_range, int flag );
extern void DG_SwitchLightBound( FVECTOR *max, FVECTOR *min, int flag );
extern void DG_DestroyLightSphere( FVECTOR *pos, float r_range );
extern void DG_DestroyLightBound( FVECTOR *max, FVECTOR *min );
extern void DG_LightReshadeChanl( DG_CHANL *cp, int which );


// loader.cpp
void DG_InitKmsVertexBuffer(void);
void DG_ReleaseVertexBuffer(DG_VBUF_INFO *info);
void DG_ReleaseKmsVertexBuffer(void);
int DG_LoadInitKms(void *buf, int id);
int DG_LoadInitLt2(void *buf, int id);
int DG_LoadInitEvm(void *buf, int id);
int DG_LoadInitCvd(void *buf, int id) ; //T.Morita added 2002.02.05
int DG_LoadInitCv2(void *buf, int id) ; //T.Morita added 2002.02.05
int DG_LoadInitZar(void *buf, int id);
void DG_UnloadKms( void *buf );
void DG_UnloadKms( void *buf );

// objs.cpp
void DG_FreeObjs(DG_OBJS *objs);
//int DG_SetVertexBuffer(int flag, DG_OBJ_PACKET *pkt, DG_MDLPACK *mdlpack);
void DG_WriteMdlPaketUV(int tri_code, DG_MDLPACK *pack);

#ifdef DEBUG_MODE
DG_OBJS *DG_MakeObjsD(DG_DEF *def, int flag, int chanl, char *fname);
DG_OBJS *DG_MakeObjs2D(DG_DEF *def, int flag, int chanl,
					   DG_MDL **mdl_list, int n_list, char *fname );
#define DG_MakeObjs(_d,_f,_c) DG_MakeObjsD(_d,_f,_c, __FILE__)
#define DG_MakeObjs2(_d,_f,_c,_m,_n) DG_MakeObjs2D(_d,_f,_c,_m,_n, __FILE__)
#else
DG_OBJS *DG_MakeObjs(DG_DEF *def, int flag, int chanl);
DG_OBJS *DG_MakeObjs2(DG_DEF *def, int flag, int chanl, DG_MDL **mdl_list, int n_list);
#endif // DEBUG_MODE
void DG_SetFogParamObjs( DG_OBJS *objs, float var_near, float var_far );

// particle.cpp
void DG_InitParticleVertexBuffer(void);
void DG_ReleaseParticleVertexBuffer(void);
void DG_ConfigParticleTex(DG_PARTICLE *p, DG_TEX *tex);
void DG_ParticleChanl(DG_CHANL *cp, int which);
void DG_SetParticleSize(DG_PARTICLE *p, float size);
void DG_SetParticleSizeMin(DG_PARTICLE *p, float size);
void DG_SetParticleScaleParam(DG_PARTICLE *p, float a, float b, float c);
void DG_SetParticleNum(DG_PARTICLE *p, int n_verts);
DG_POINTVERTEX *DG_GetParticleBuf(DG_PARTICLE *p);
void DG_SwitchBuffParticle(DG_PARTICLE *p);

DG_PARTICLE *DG_MakeParticle(int flag, int n_verts, int chanl);
void DG_FreeParticle(DG_PARTICLE *p);

// pos.cpp
void DG_SinCos( float *s, float *c, float rot );
void DG_RotMatrixX( FMATRIX *res, FMATRIX *mat, float rot_x );
void DG_RotMatrixY( FMATRIX *res, FMATRIX *mat, float rot_y );
void DG_RotMatrixZ( FMATRIX *res, FMATRIX *mat, float rot_z );
void DG_InversMatrix( FMATRIX *res, FMATRIX *mat );
void DG_GetPos(MATRIX *world);
void DG_SetPos(MATRIX *world);
void DG_SetPos2(VECTOR *mov, SVECTOR *rot);
void DG_MovePos(VECTOR *mov);
void DG_RotatePos(SVECTOR *rot);
void DG_RotatePosZYX(SVECTOR *rot);
void DG_ScalePos(VECTOR *scale);
void DG_PutObjs(DG_OBJS *objs);
void DG_PutVector(VECTOR *from, VECTOR *to, int n);
void DG_RotVector(VECTOR *from, VECTOR *to, int n);
void DG_ReflectMatrix(VECTOR *pole, MATRIX *m1, MATRIX *m2);
void DG_ReflectVector(VECTOR *pole, VECTOR *vec1, VECTOR *vec2);
int DG_PointCheckOne(VECTOR *vec, int flag);
void DG_TransPersOneChanl(VECTOR *res, VECTOR *pos, int chanl);
void DG_TransPersOne(VECTOR *res, VECTOR *pos);
int DG_BoundCheckFlag(MATRIX *world, VECTOR *bound_max, VECTOR *bound_min, int flag);
int DG_BoundCheck(MATRIX *world, VECTOR *bound_max, VECTOR *bound_min);
int	DG_ObjsBoundCheckChanl(DG_OBJS	*objs, int chanl);
int	DG_ObjBoundCheckChanl(DG_OBJS *objs, int joint, int chanl);


// pshade.cpp
int DG_MakePreshade(DG_OBJS *objs, LIT_DEF *lit_def);
int DG_MakePreshadeParts(DG_OBJS *objs, LIT_DEF *lit_def);
void DG_FreePreshade(DG_OBJS *objs);
void DG_TempPreshadeRGB(DG_OBJ *obj, LIT_DEF *lit_def, int which, int start_flag);
void DG_TmpLightPreshadeChanl(DG_CHANL *cp, int which);
void DG_RefreshVAnimeChanl(DG_CHANL *cp, int which);

// prim2.cpp
void DG_InitPrim2VertexBuffer(void);
void DG_ReleasePrim2VertexBuffer(void);
void DG_CleanPrim2VertexBuffer(void);

DG_PRIM2VERTEX *DG_LockPrimPack(DG_PRIM2_PACKET *packet);
void DG_UnlockPrimPack(DG_PRIM2_PACKET *packet);

/* prim2.cpp */
extern void DG_InitPrim2VertexShader(void);

//int DG_WritePrimPacks(DG_PRIM2_PACKET *packet, DG_CHANL *cp, int which);
extern void DG_BP_WritePrimInitPacket( DG_CHANL *cp );
extern void DG_BP_WritePrimLocalParam( DG_PRIM2 *prim );
extern int DG_BP_WritePrimPacksAndPendRender( DG_CHANL *viewport, DG_PRIM2_PACKET *packet, int clone );
extern void DG_BP_WritePrimPacksFinalize();

void DG_Prim2Chanl(DG_CHANL *cp, int which);
DG_PRIM2 *DG_MakePrim2(int flag, int n_prims, int n_verts, int chanl);
void DG_FreePrim2(DG_PRIM2 *prim);
void DG_ConfigPrim2Tex(DG_PRIM2 *prim, DG_TEX *tex);

// plugin.c (add 2001/05/09 M.K)
extern void DG_InitPluginSystem( void );
extern DG_PLUGIN *DG_SearchPlugin( int object_id );
extern void DG_MakePlugin( DG_PLUGIN *plugin, int id, int flag, int phase, int prio, void *func, int n_queue );
extern void DG_FreePlugin( DG_PLUGIN *plugin );
extern void DG_AddPlugin( DG_PLUGIN *add_plugin );
extern void DG_DeletePlugin( DG_PLUGIN *del_plugin );
extern void DG_PluginStartChanl( DG_CHANL *cp, int which );
extern void DG_PluginEndChanl( DG_CHANL *cp, int which );
extern void DG_PluginChanl( DG_CHANL *cp, int which );

static inline DG_PRIM2 *GM_MakePrim2Chanl(int type, int n_prims, int n_verts, int chanl)
{
#if 0
	DG_PRIM2 *prim;
	extern int GM_CurrentMap;
	static inline void GM_GroupPrim2(DG_PRIM2 *, int);

	prim = DG_MakePrim2(type, n_prims, n_verts, chanl);
	if (prim != NULL) {
		DG_QueuePrim2(prim);
		if (chanl != DG_CHANL_MENU) {
			GM_GroupPrim2(prim, GM_CurrentMap);
		}
	}
#endif
	// 仮(GM_CurrentMap, GM_GroupPrim2 をとりあえず外したもの) 
	DG_PRIM2 *prim;
	prim = DG_MakePrim2(type, n_prims, n_verts, chanl);
	if (prim != NULL) {
		DG_QueuePrim2(prim);
	}
	
	return prim;
}

// 機能追加型 DG_MakePrim2() (3Dチャンネル固定) 
static inline DG_PRIM2 *GM_MakePrim2(int type, int n_prims, int n_verts)
{
	return GM_MakePrim2Chanl(type, n_prims, n_verts, 0);
}

// 機能追加型 DG_FreePrim2() 
static inline void GM_FreePrim2(DG_PRIM2 *prim)
{
	if (prim != NULL) {
		DG_DequeuePrim2(prim);
		DG_FreePrim2(prim);
	}
}

#ifdef DEBUG_MODE
// デバッグ用 
#define GM_MakePrim2(_t, _np, _nv) GM_MakePrim2D(_t, _np, _nv, __FILE__)

DG_PRIM2 *DG_MakePrim2D(int flag, int n_prims, int n_verts, int chanl, char *fname);
DG_PRIM2 *GM_MakePrim2ChanlD(int type, int n_prims, int n_verts, int chanl, char *fname);
DG_PRIM2 *GM_MakePrim2D(int type, int n_prims, int n_verts, char *fname);
#endif

// screen.cpp
void DG_ScreenChanl(DG_CHANL *cp, int which);

// shadow.cpp
void DG_SpotChainChanl(DG_CHANL *cp, int which);
void DG_ShadowChainChanl(DG_CHANL *cp, int which);
void DG_InitShadowVertexShader(void);

// store.c
extern void DG_StoreChanl( DG_CHANL *cp, int which );
/* xtext.c */
extern void DG_InitTextureCache( void );
extern int DG_LoadInitTriBuf( void *buf, int code, DG_TEXTURE_LIST *list );
extern int DG_GetTextureListSize( void *buf );
extern int DG_LoadInitTri( void *buf, int code );
extern void *DG_FreeTextureCache( void *buf );
extern void DG_SetTextureTmpClut( int which );
extern void DG_ResetTextureTmpClut( void );
extern DG_TEX* DG_GetTexture( unsigned int code );
extern DG_TEX* DG_GetTexture2( unsigned int tri_code, unsigned int code );
extern DG_TEXTURE_LIST* DG_GetTextureList( unsigned int code );
extern int	DG_SearchTriFromKms( DG_DEF *def );
extern int	DG_SearchTriFromTex( DG_TEX *org );
extern void DG_GetTexelInfo( int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex );
extern DG_TEX_MOVEREPLACE *DG_MakeMoveReplacePacket( int tri_code, int dst_tex_id, int src_tex_id );
extern void DG_FreeMoveReplacePacket( DG_TEX_MOVEREPLACE *tex_replace );
extern DG_TEX_LOADREPLACE *DG_MakeLoadReplacePacket( int tri_code, int dst_tex_id, void *texel, void *clut );
extern void DG_FreeLoadReplacePacket( DG_TEX_LOADREPLACE *tex_replace );
extern void DG_SetMoveReplaceTexture( DG_TEX_MOVEREPLACE *tex_replace );
extern void DG_ResetMoveReplaceTexture( DG_TEX_MOVEREPLACE *tex_replace );
extern void DG_SetLoadReplaceTexture( DG_TEX_LOADREPLACE *tex_replace );
extern void DG_ResetLoadReplaceTexture( DG_TEX_LOADREPLACE *tex_replace );

#if 0//BP
// text.cpp
LPDIRECT3DTEXTURE8 DG_GetDxTexture(int code);
int DG_LoadInitBmp(void *buf, int code);
int DG_LoadInitXti(void *buf, int code);
#endif

// patch.h
#define __PATCH_H__
typedef struct _dg_patch_vert{
	FVECTOR		pos ;			/* 頂点座標 */
	FVECTOR		pos_ds ;		/* 頂点座標Ｓ軸方向変化量 */
	FVECTOR		pos_dt ;		/* 頂点座標Ｔ軸方向変化量 */
	FVECTOR		uv ;			/* テクスチャ座標 */
} DG_PATCH_VERT ;

typedef struct _dg_patch_parts{
	int				v_index[4] ;	/* 参照頂点へのインデックス */
	int				parts_index[4] ;	/* 隣接するパッチ曲面パーツのインデックス */
	FVECTOR			max, min ;		/* バウンディングチェックサイズ */
	int				flag ;			/* 処理フラグ */
	int				lod_level ;		/*  */
	int				edge_flag ;		/* エッジフラグ */

	int             n_prims;        /* Xbox追加: プリミティブ数 */
	int             n_verts;        /* Xbox追加: 頂点数 */	
	LPDIRECT3DINDEXBUFFER9 ibuf;    /* Xbox追加: インデックスバッファ */
	int				pad[1] ;		/*  */
} DG_PATCH_PARTS ;

typedef struct _dg_patch {
	FMATRIX			world ;				/* ワールドマトリクス */
	FMATRIX			*root ;				/* 付随マトリクス */
	FMATRIX			*light ;			/* 光源マトリクス */
	int				flag ;				/* 各種フラグ */
	int				n_verts ;			/* 管理頂点数 */
	int				n_patch ;			/* オブジェクト表示数 */
	int				group_id ;			/* グループＩＤ */
	int				chanl ;				/* 処理チャンネル */
	int				tri_id ;			/* 使用テクスチャ */
	float			lod_z_bias ;		/* ＬＯＤ補正距離 */
	int				lod_level_bias ;	/* ＬＯＤ補正レベル */
	int				buffer_clock ;		/* 頂点ダブルバッファ切り替えクロック */
	DG_TEX			*tex ;				/* テクスチャ構造体へのポインタ */
	DG_PATCH_VERT	*verts[2] ;			/* 共有頂点データ */
	DG_PATCH_PARTS	*parts ;			/* パッチ曲面データ配列 */
	int				max_level ;			/* 最大分割数制限（０が１６分割、４で分割無し） */
	FVECTOR			max,min ;			/* 全体バウンディング（未使用） */
	FMATRIX			envmap_correct ;	/* 環境マップ補正マトリクス */
#if 1	// バンプ実験 2001/06/13 M.Kobayashi
	// patch.cpp に #ifdef BUMP_TEST で改造してある 
	LPDIRECT3DTEXTURE9	pTexBump;		// バンプ用テクスチャ 
	LPDIRECT3DTEXTURE9	pTexLight;		// ディフューズ・スペキュラ用テクスチャ 
	FVECTOR			vecS;				// タンジェントスペース記述 
	FVECTOR			vecT;				// タンジェントスペース記述 
	FVECTOR			vecBumpLight;		// バンプ用光源方向 
	FVECTOR			vecBumpCol;			// バンプ用光源色 
#endif	
} DG_PATCH ;


/* DG_PATCH.flag 用フラグ */
enum {
	DG_PATCH_INVISIBLE			= 0x0003,		/* 不可視属性（全チャンネル） */
	DG_PATCH_INVISIBLE0			= 0x0001,		/* 不可視属性（チャンネル０のみ） */
	DG_PATCH_INVISIBLE1			= 0x0002,		/* 不可視属性（チャンネル１のみ） */
	DG_PATCH_INVISIBLE2			= 0x0004,		/* 不可視属性（チャンネル２のみ） */
	DG_PATCH_INVISIBLE3			= 0x0008,		/* 不可視属性（チャンネル３のみ） */
	DG_PATCH_SINGLE				= 0x0010,		/* シングルバッファモード（静止オブジェクト用）＜未実装＞ */
	DG_PATCH_NOCHECK			= 0x0020,		/* 各パッチ曲面のＬＯＤ、バウンディングチェック */
												/* （他のパッチ曲面オブジェクトと共有する場合に使用する） */
	DG_PATCH_FOGBLACK			= 0x0040,		/* 強制的にフォグカラーを黒に（加算半透明テクスチャに有効） */
	DG_PATCH_SEMITRANS			= 0x0080,		/* 強制５０％半透明 */
	DG_PATCH_DUMMY				= 0x0000,		/*  */
	DG_PATCH_NO_WRAP			= 0x80000000,	/* テクスチャリピート無し（ＸＢＯＸ用拡張） */

	DG_PATCH_TYPEMASK			= 0xff00,		/*  */
	DG_PATCH_NORMAL				= 0x0000,		/* 通常曲面サーフェス */
 	DG_PATCH_REFLECTPLANE		= 0x0100,		/* 反射平面型（Ｙ軸０固定・水面反射エミュレーション） */
	DG_PATCH_BUMPMAP			= 0x0200,		/* ＸＺ軸固定ＵＶ摂動バンプマップ曲面サーフェス */
 	DG_PATCH_REFLECTPLANE2		= 0x0300,		/* 反射平面型（Ｙ軸０固定・水面反射エミュレーション） */
};

/* ---------------------------------------------------------------- */

void DG_InitPatchVertexShader(void);
extern void DG_AddPluginPatch( void );
extern void DG_DeletePluginPatch( void );
void DG_ReleasePatchVertexBuffer( void );

extern void DG_QueuePatchObjs( DG_PATCH *patch );
extern void DG_DequeuePatchObjs( DG_PATCH *patch );
extern DG_PATCH* DG_MakePatch( int flag, int n_patch, int n_verts );
extern void DG_FreePatch( DG_PATCH *patch );

extern DG_PATCH* DG_MakePatchMesh( int flag, int n_vert_s, int n_verts_t );
extern void DG_ConfigPatchLOD( DG_PATCH *patch, int level );
extern void DG_SetupPatchMeshBounding( DG_PATCH *patch );


/* ---------------------------------------------------------------- */

// Xbox新規追加
// コールバックオブジェクト
//
// 自分でオリジナルの描画関数を定義したい時に使用して下さい。
// PS2 の dmapack の代わりみたいなもの。
//

enum {
	DG_CALLBACK_NORMAL = 0x0001, // 通常チャンネルで使用 
	DG_CALLBACK_MENU = 0x0002, // メニュー専用チャンネルで使用

	DG_CALLBACK_INVISIBLE0 = 0x0010, /* 不可視属性（チャンネル０のみ） */
	DG_CALLBACK_INVISIBLE1 = 0x0020, /* 不可視属性（チャンネル１のみ） */
	DG_CALLBACK_INVISIBLE2 = 0x0040, /* 不可視属性（チャンネル２のみ） */
	DG_CALLBACK_INVISIBLE3 = 0x0080, /* 不可視属性（チャンネル３のみ） */
	DG_CALLBACK_INVISIBLE  = 0x0003, /* 不可視属性（全チャンネル） */
	DG_CALLBACK_INVISIBLEMENU = 0x0100, /* メニューチャンネルで非表示 */
};

// コールバック実行フェーズ 
enum {
	DG_CALLBACK_PHASE_FIRST = DG_PLUGIN_PHASE_FIRST,
	DG_CALLBACK_PHASE_NORMAL = DG_PLUGIN_PHASE_NORMAL,
	DG_CALLBACK_PHASE_AFTER = DG_PLUGIN_PHASE_AFTER,
	DG_CALLBACK_PHASE_LAST = DG_PLUGIN_PHASE_LAST
};

typedef struct _dg_callback_obj {
	int  flag;          /* 各種フラグ */
	short phase;         /* 実行フェーズ */
	short pri;           /* プライオリティ */
	void *data;         /* コールバックに渡すデータ */
	void (*func)(DG_CHANL *, int, struct _dg_callback_obj *, void *);
} DG_CALLBACK_OBJ;

typedef void (*DG_CallbackObjFunc)(DG_CHANL *, int, DG_CALLBACK_OBJ *, void *data);

void DG_AddPluginCallbackObj(void);
void DG_DeletePluginCallbackObj(void);
int DG_QueueCallbackObj(DG_CALLBACK_OBJ *obj);
void DG_DequeueCallbackObj(DG_CALLBACK_OBJ *obj);
void DG_FreeCallbackObj(DG_CALLBACK_OBJ *obj);
DG_CALLBACK_OBJ *DG_MakeCallbackObj(DG_CallbackObjFunc func, int flag, int phase,
									int pri, void *data);
void DG_SetCallbackObj(DG_CALLBACK_OBJ *obj, DG_CallbackObjFunc func, int flag,
					   int phase, int pri, void *data);

/* xvbuffer */
extern void DG_InitDynamicVertexBufferSystem( void );
extern void DG_ResetDynamicVertexBuffer( int which );
extern void *DG_AllocDynamicVertexBuffer( int size, int num );
extern void DG_SetVertexBuffer( int stream, void *addr, int size );
extern void *DG_NewDynamicVertexBuffer( int size, int num );

/* ximage.c */
extern void DG_LoadImage( void *addr, int which, int x, int y, int w, int h, int mode );
extern void DG_StoreImage( void *addr, int which, int x, int y, int w, int h, int mode );
extern DG_TEX_LIN *DG_MakeLinerTexture( int width, int height, int format );
extern void DG_FreeLinerTexture( DG_TEX_LIN *tex );
extern void DG_SetLinerTexture( DG_TEX_LIN *tex );
extern void DG_MakeLinerTexture2( DG_TEX_LIN *tex, int width, int height, int format, void *addr );

/* xdmactrl.c */
#ifndef DEBUG_MODE
extern void DG_OpenDmaTask( void );
#else
extern void _DG_OpenDmaTask( char *fname );
#define DG_OpenDmaTask() _DG_OpenDmaTask( __FILE__ )
#endif
extern void DG_CloseDmaTask( void );
extern void DG_DmaStart( int which );
extern void DG_DmaClear( int which );
extern void DG_DmaSkip( int which );

/* xvshader.c */
extern void DG_ResetVertexShader(void);
extern void DG_MakeVertexShader( DG_VERTEXSHADER *shader, void *code, const DWORD* pDecl );
extern void DG_ClearVertexShader( void );
extern int DG_LoadVertexShader( DG_VERTEXSHADER *shader );
extern int DG_LoadVertexShader2( int addr, DG_VERTEXSHADER *shader );
extern void DG_SelectVertexShader( DG_VERTEXSHADER *shader, DG_VERTEXFORMAT *vformat );
extern void DG_MakeVertexFormat( DG_VERTEXFORMAT *format, const DWORD* pDecl );
extern void DG_SetVertexStream( DG_VERTEXFORMAT *format );

/* xdgmem.c */
extern void DG_InitMemorySystem( int static_size, int dynamic_size, int push_buff_size );
extern void DG_ResetMemorySystem( int which );
extern void *DG_AllocDynamicVertexBuffer( int size, int num );
extern void *DG_OpenPushBufferMemory( D3DPushBuffer *pushbuffer );
extern int DG_ClosePushBufferMemory( D3DPushBuffer *pushbuffer );
extern int DG_GetPushBufferFreeSpace( void );
extern void *DG_AllocLocalVideoMemory( int size );
extern void *DG_AllocLocalVideoMemoryAlign( int size, int align );
extern void DG_FreeLocalVideoMemory( void *addr );
extern void DG_DelayedFreeLocalVideoMemory( void *addr );

static inline int check_buffer_full( void )
{
	extern int			DG_PacketMemoryFree ;			/* カレントパケットメモリの空き領域 */
	extern int			DG_PacketOverFlag ;				/* そのフレーム内でのバッファーオーバー通知用 */
	if ( DG_PacketOverFlag ) return ( -1 );
	//if ( DG_PacketMemoryFree < ( 128 * 1024 ) ){
		if ( DG_GetPushBufferFreeSpace() < ( 32 * 1024 ) ){
			DG_PacketOverFlag = 1 ;
			return ( -1 );
		}
	//}
	return ( 0 );
}
#define DG_ISCHECK_PACKETBUFFER()	(check_buffer_full())



/* xdma_ctrl.c */
extern void DG_DmaClear( int which );
extern void DG_DmaStart( int which );
extern void DG_DmaSkip( int which );
extern int DG_DmaCheckEnd( void );

/* xstore.c */
extern void		DG_StoreChanl( DG_CHANL *cp, int which );
extern void DG_SetFrameStore( void *store_addr, int mode );

/* xalpha.c */
extern void DG_InitAlphaState( void );

/* xdmapack.c *//* 残りはdmapack.h内に記述 */
extern void *DG_DrawVerticesDirect( u_int type, int v_size, int count );
extern void DG_DrawVerticesDirectEnd( void );



/* ---------------------------------------------------------------- */
	/*
		頂点バッファ切り替え
	*/
static	inline	void	DG_SwitchBuffPatch( DG_PATCH *patch )
{
	patch->buffer_clock = 1 - patch->buffer_clock ;
}

// スクラッチスタックダミー 
#define DG_SetScratchStack( addr )
#define DG_ResetScratchStack()


#ifdef __cplusplus
}
#endif

#include "dmapack.h"
#include "prims.h"
#include "m_prim2.h" /* 2002/02/14 T.Morita */


#define DG_GetMdlFormat( _def ) ((_def)->data_format & MGS_MODEL_FORMAT_MASK ) /* 2002/03/25 M.Kobayashi */

#endif /* __LIBDGX_H__*/
