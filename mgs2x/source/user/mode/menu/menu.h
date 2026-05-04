/*
	menu.h
	装備メニュー関連ヘッダ

	2000/10/07	K.Takabe
	$Id: menu.h,v 1.2 2002/12/05 18:42:02 takaki Exp $

*/

#ifndef __MENU_H__
#define __MENU_H__

#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------------------- */
	/*
		描画関連定義
	*/

#define VR_WIDTH		(512)		/* 仮想座標系における表示範囲 */
#define	VR_HEIGHT		(384)		/* 仮想座標系における表示範囲 */

#define	PAD_IT_CHANGE	(PAD_L2)	/* アイテムメニューボタン */
#define	PAD_WP_CHANGE	(PAD_R2)	/* 武器メニューボタン */

#define	QUICK_TIME		GM_MENU_QUICK_TIME

#define	OPEN_TIME_G		(8)		/* グループオープン時間 */
#define	OPEN_TIME_L		(8)		/* リストオーブン時間 */
#define OPEN_TIME_E		(5)		/* 説明分オープン時間 */
#define	DISP_GROUPS		(5)		/* 一度に表示可能なグループ数 */
#define	DISP_LISTS		(5)		/* 一度に表示可能なリスト数 */
//#define	UPPER_MAX		(5)						/* 上方向最大表示グループ数（選択行含む） */
#define	UPPER_MAX		(4)						/* 上方向最大表示グループ数（選択行含む） */
#define	LOWER_MAX		(DISP_GROUPS-UPPER_MAX)	/* 下方向最大表示グループ数 */
#define U_SPACE			(8)
#define D_SPACE			(8)
#define L_SPACE			(8*TARGET_ASPECT_X)
#define R_SPACE			(8*TARGET_ASPECT_X)

#define	STEP_X		(4*TARGET_ASPECT_X)			/* パネル間の幅 */
#define	STEP_Y		(12-6)	/* パネル間の幅 */

//#define	PW			((VR_WIDTH-R_SPACE-L_SPACE-(STEP_X*(DISP_LISTS+1)))/DISP_LISTS)		/* パネルの幅 */
#define	PW			(72) //(((VR_WIDTH-R_SPACE-L_SPACE-(STEP_X*(DISP_LISTS+1)))/DISP_LISTS)*TARGET_ASPECT_X)		/* パネルの幅 */


#define	PH			(60) //(((VR_HEIGHT-U_SPACE-D_SPACE-(STEP_Y*(DISP_GROUPS+1)))/DISP_GROUPS)-6)	/* パネルの高さ */
#define	IT_PX		(16) //((VR_WIDTH-R_SPACE-(PW+STEP_X)*(DISP_LISTS))-95)					/* アイテム表示位置 */
#define	WP_PX		(VR_WIDTH - PW - IT_PX) //((L_SPACE+(PW+STEP_X)*(DISP_LISTS-1)+STEP_X)+95)					/* 武器表示位置 */
#define	PY			(VR_HEIGHT - PH - 10) //((U_SPACE+(PH+STEP_Y)*(DISP_GROUPS-1)+STEP_Y))+16)				/* 装備パネル表示位置 */
#define TITLE_HEIGHT	(14)	/* タイトルの高さ */

#define GW			(24*TARGET_ASPECT_X)							/* グループ表示パネルの幅 */
#define	GY			(PY-((PH+STEP_Y)*LOWER_MAX))	/* グループの表示中心位置? */

#define	SHIFT_TIME	(6)

/* テキスト表示位置（パネル左上からのオフセット） */
#ifdef PSX2
#define	TXT_SHIFT_X	((332-256+8+8)*TARGET_ASPECT_X)
//#define	TXT_SHIFT_X	(332-256+8+8)
#else
#define	TXT_SHIFT_X	((int)((332-256+8+8)*DRAW_WIDTH/512))
#endif
#define	TXT_SHIFT_Y	(PH-TITLE_HEIGHT+2)
#define	NUM_TXT_SHIFT_Y	(PH-TITLE_HEIGHT*2+2)


/* 説明分関連定義 */
#if 0
#  ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
#    define EXPLAIN_VRAM_WIDTH	( 384 )		/* フォント展開領域幅 */
#    define EXPLAIN_VRAM_MALLOC_WIDTH	( 384 )		/* フォント展開領域幅 *//* XBOX拡張 */
#  else
#    define EXPLAIN_VRAM_WIDTH	( 400 )		/* フォント展開領域幅 */
#    define EXPLAIN_VRAM_MALLOC_WIDTH	( 512 )		/* フォント展開領域幅 *//* XBOX拡張 */
#  endif
#else
#  define EXPLAIN_VRAM_WIDTH ( BP_Area_EU() ? 400 : 384 )
#  define EXPLAIN_VRAM_MALLOC_WIDTH ( BP_Area_EU() ? 512 : 384 )
#endif
#define EXPLAIN_VRAM_HEIGHT	( 230 )		/* フォント展開領域高さ */
#define EXPLAIN_TEX_FMT		( SCE_GS_PSMT4 )		/* 説明文テクスチャフォーマット */
#define	EXPLAIN_TEX_BASE	(TEXTURE_TOP_PAGE())	/* 説明文テクスチャベースアドレス */
#define	EXPLAIN_TEX_WIDTH	( ( EXPLAIN_VRAM_WIDTH  + 127 ) &0xff80 )	/* 説明文テクスチャVRAM幅 */
#define	EXPLAIN_TEX_HEIGHT	( ( EXPLAIN_VRAM_HEIGHT + 127 ) &0xff80 )	/* 説明文テクスチャVRAM高さ */
#define EXPLAIN_CLUT_FMT	( SCE_GS_PSMCT32 )		/* 説明文ＣＬＵＴフォーマット */
#define	EXPLAIN_CLUT_BASE	(TEXTURE_TOP_PAGE()+EXPLAIN_TEX_WIDTH*(EXPLAIN_TEX_HEIGHT/128)*64)	/* ＣＬＵＴベース */

/* 説明分表示領域 */
//#define	EXP_WIN_X	(128)	/* 旧方式用 */
#define	EXP_WIN_IX	(128)		/* 旧方式用 *//* アイテム説明文用 */
#define	EXP_WIN_WX	(64)		/* 旧方式用 *//* 武器説明文用 */
#define	EXP_WIN_Y	(96)		/* 旧方式用 */
#define	EXP_WIN2_IX	(128+12)		/* 新方式用 *//* アイテム説明文用 */
#define	EXP_WIN2_WX	(64-12)		/* 新方式用 *//* 武器説明文用 */
#define	EXP_WIN2_Y	(96-(PY-GY)+24)/* 新方式用 */
#define	EXP_WIN_W	(320*TARGET_ASPECT_X)
#define	EXP_WIN_H	(192)
#define EXP_WIN_EDGE_X	( 3 )	/* 説明ウィンドウ枠の太さ */
#define EXP_WIN_EDGE_Y	( 2 )	/* 説明ウィンドウ枠の太さ */
#define EXP_TAG_W1	( 80 )		/* タグ部分の長辺 */
#define EXP_TAG_W2	( 90 )		/* タグ部分の短辺 */
#define EXP_TAG_H	( 14 )		/* タグ部分の高さ */

/* 説明文リソースＩＤ */
#define RESOURCE_ITEM_MENU_EXPLAIN		(2)	/* "" */
#define RESOURCE_WEAPON_MENU_EXPLAIN	(1)	/* "" */

/* ---------------------------------------------------------------- */
	/*
		アイテムパネル表示フラグ
	*/
enum {
	PANEL_FLAG_NORMAL		= 0x0000,	/* 通常表示 */
	PANEL_FLAG_ACTIVE		= 0x0001,	/* アクティブ表示 */
	PANEL_FLAG_NOACTIVE		= 0x0002,	/* 非アクディブ表示 */
	PANEL_FLAG_TYPEMASK		= 0x0003,
	/* 各種ビットフラグ */
	PANEL_FLAG_NO_USE		= 0x0004,	/* 使用禁止マーク */
	//PANEL_FLAG_EMPTY		= 0x0008,	/* 弾薬空マーク */
	PANEL_FLAG_CURRENT		= 0x0010,	/* 現在装備中マーク */
	PANEL_FLAG_PREV			= 0x0020,	/* クイックチェンジ対象装備マーク */
};

/* 薬きょう表示最大 */
#define MAX_DRAW_BULLET		(32)
/* ---------------------------------------------------------------- */
/* 装備メニュー関連ステータス */
extern int		MENU_AutoSelectItem ;
extern int		MENU_AutoSelectWeapon ;
extern int		MENU_StatusFlag ;
enum {
	MENU_STATUS_MIC_EXPLAIN2		= 0x00000001,	/* エイムズマイク説明フラグ */
};
/* ---------------------------------------------------------------- */
	/*
		外部からのメニュー表示制御用
	*/
enum {
	MENU_DISP_SHIPWORM			= 0x0001,
};

/* ---------------------------------------------------------------- */
	/*
		テクスチャ名などの定数
	*/

#define MENU_TRI			(9018481)	/* "menuicon" */
#if 0
#define TEX_WINDOW1			(12738100)	/* "win_alp_ovl" */
#define TEX_WINDOW2			(2529661)	/* "win2_alp_ovl" */
#define TEX_DEFAULT			(7120894)
#define TEX_MARK1			(14450094)	/* "no_use_alp_ovl" */
#define TEX_BULLET			(2645473)	/* "blt_alp_ovl" */
#define TEX_FUNA			(5607725)	/* "funa_alp_ovl" */
#endif
/* ---------------------------------------------------------------- */
	/*
		グループカラー定義
	*/
#define MENU_RGB( _r, _g, _b )	( (_r)|((_g)<<8)|((_b)<<16) )
#define MENU_LGRAY		MENU_RGB(80,80,80)
#define MENU_PINK		MENU_RGB(107,73,97)
#define MENU_VIOLET		MENU_RGB(77,38,98)
#define MENU_BLUE		MENU_RGB(32,37,64)
#define MENU_YELLOW		MENU_RGB(97,88,29)
#define MENU_CYAN		MENU_RGB(53,83,90)
#define MENU_ORANGE		MENU_RGB(97,48,38)
#define MENU_DGRAY		MENU_RGB(37,37,37)
#define MENU_ORANGE2	MENU_RGB(89,64,32)
#define MENU_BLACK		MENU_RGB(0,0,0)
#define MENU_WHITE		MENU_RGB(192,192,192)

//#define COLOR_DEBUG
#ifndef COLOR_DEBUG
#define MENU_SELECT_COL			MENU_RGB(32,0,141)
#define MENU_CURRENT_COL		MENU_RGB(32,98,99)
#define MENU_PREV_COL			MENU_RGB(2,62,58)
#define MENU_GRP_CURRENT_COL	MENU_RGB(207,127,13)
#define MENU_GRP_PREV_COL		MENU_RGB(101,103,87)
#else
extern int	MENU_select_col ;
extern int	MENU_current_col ;
extern int	MENU_prev_col ;
extern int	MENU_grp_current_col ;
extern int	MENU_grp_prev_col ;
#define MENU_SELECT_COL			MENU_select_col
#define MENU_CURRENT_COL		MENU_current_col
#define MENU_PREV_COL			MENU_prev_col
#define MENU_GRP_CURRENT_COL	MENU_grp_current_col
#define MENU_GRP_PREV_COL		MENU_grp_prev_col
#endif

/* ---------------------------------------------------------------- */

/* 選択パネル管理構造体 */
typedef struct {
	SPR_OBJ		*root ;		/* ルートプリミティブ（ダミー） */
	SPR_OBJ		*window ;	/* アイコン背景用プリミティブ */
	SPR_OBJ		*icon ;		/* アイコン表示用プリミティブ */
	SPR_OBJ		*title ;	/* タイトル背景用プリミティブ */
	char		*string ;	/* タイトル表示文字列 */
	int			n_count, max_count ;	/* 保持数＆最大保持数 */
	int			current_tex_id ;	/* アイコンに設定中のテクスチャＩＤ */
	int			flag ;
} PANEL_PRIM ;

typedef struct {
	SPR_OBJ		*root ;		/* ルートプリミティブ */
	SPR_OBJ		*window ;	/* テクスチャ表示プリミティブ */
	int			current_tex_id ;	/* 現在設定中のテクスチャＩＤ */
} GROUP_PRIM ;


extern int PL_PAD_ITEMUSE ;/* raiden/pl_pad.c */


extern void MENU_SetShipwormOffset( int x, int y );
extern int MENU_GetMenuDispStatus( void );
extern void MENU_SetMenuDispStatus( int status );

extern void MENU_PutItemPanel( int x, int y, int type, int alpha1, int alpha2, int flag );
extern void MENU_PutWeaponPanel( int x, int y, int type, int alpha1, int alpha2, int flag );
extern void MENU_PutItemGroup( int x, int y, int type, int alpha, int col );
extern void MENU_PutWeaponGroup( int x, int y, int type, int alpha, int col );
extern void MENU_PutShipwormPanel( int x, int y );
extern void MENU_PutPagePrim( int x, int y, int alpha, int col );

extern void MENU_CreateExplainText( char *message );
extern void MENU_PutExplainText( int x, int y, int type );

extern void MENU_PutBoxPrim(int x, int y, int w, int h, int alpha, int color);
extern void MENU_PutTextPrim(int x, int y, char *text, int r, int g, int b, int a, int flag);

extern void *NewMenuPrimControl( void );

extern int MENU_SetExplainWindowPacket( void *buffer, int x, int y );

/* common.c */
extern int MENU_QuickChangeWeapon( void );
extern int MENU_QuickChangeItem( void );
extern int MENU_ChangeWeapon( int weapon );
extern int MENU_ChangeItem( int item );
extern int MENU_DefaultCheckWeapon( void );
extern int MENU_DefaultCheckItem( void );
extern void MENU_UpdateItemExplainMessage( int num );
extern void MENU_UpdateWeaponExplainMessage( int num );

extern void MENU_SetSafeZoneTrim(float safeZoneTrim);

extern void MENU_ForceClose();

#ifdef __cplusplus
}
#endif

#endif





