//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------

/*
	xmenusub.c
	メニュー関連補助ルーチン

	2002/02/18	K.Takabe
	$Id: xmenusub.c,v 1.5 2002/12/05 18:41:01 takaki Exp $
*/

/* ---------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#if 0 //BP_PS2 def PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"def_dma.h"
#include	"libdg.cnf"
#include	"dmapack.h"

#include	"sprite_2d.h"
#include "menu.h"

void	GM_VisibleMenuLife( void );
void	GM_InvisibleMenuLife( void );

#include "BP_BuildDefines.h"

#if BP_USE_NEW_FONT_SYSTEM()
#include "BP_Renderer.h"
#include "BP_Font.h"
#else
#include	"font.h"
#endif

#include "libfs.h"

/* ---------------------------------------------------------------- */
#define SCALE_X(_x)	((_x)*((float)DRAW_WIDTH/VR_WIDTH))
//#define SCALE_X(_x)	((_x)*((float)640.0f/VR_WIDTH))
#define SCALE_Y(_y) ((_y)*((float)DRAW_HEIGHT/VR_HEIGHT))

#define SAFE_ZONE_AMOUNT 0.0f

/* ---------------------------------------------------------------- */
#define	Items		GM_ItemNames
#define	Weapons		GM_WeaponNames

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

/* ---------------------------------------------------------------- */
/* テクスチャリスト */
static int	Item_Textures[] = {
	/* 0 */
	0,				/* IT_None         ("") */
	7120894,		/* IT_Ration       ("ration_alp_ovl") */
	10534805,		/* IT_DummyScope   ("scope_alp_ovl") */
	1938540,		/* IT_Medicine     ("medi_alp_ovl") */
	12883882,		/* IT_Styptic      ("styp_alp_ovl") */
	1938540,		/* IT_Diazepam     ("medi_alp_ovl") */
	15045012,		/* IT_Uniform      ("gol1_alp_ovl") */
	12672479,		/* IT_Jackeet      ("bam_alp_ovl") */
	12672547,		/* IT_Stealth      ("sam_alp_ovl") */
	5791245,		/* IT_MineDetector ("mnd_alp_ovl") */
	2461477,		/* IT_BombSenserA  ("bsn1_alp_ovl") */
	2527013,		/* IT_BombSenserB  ("bsn2_alp_ovl") */
	8412688,		/* IT_NightVision  ("ngl_alp_ovl") */
	8412712,		/* IT_Thermal      ("tgl_alp_ovl") */
	10534805,		/* IT_Scope        ("scope_alp_ovl") */
	10593390,		/* IT_Camera       ("camera_alp_ovl") */
	8109879,		/* IT_CBBox        ("cbx_a_alp_ovl") */
	50325,			/* IT_Tobacco      ("smoke_alp_ovl") */
	16276955,		/* IT_Card         ("acd_alp_ovl") */
	11165220,		/* IT_Shaver       ("shv_alp_ovl") */
	2121210,		/* IT_PHS          ("htl_alp_ovl") */
	10593390,		/* IT_TnkCamera    ("camera_alp_ovl") */
	8175415,		/* IT_CBBoxB       ("cbx_b_alp_ovl") */
	8240951,		/* IT_CBBoxC       ("cbx_c_alp_ovl") */
	8437559,		/* IT_CBBoxWet     ("cbx_f_alp_ovl") */
	10968541,		/* IT_VibSenser    ("aps_alp_ovl") */
	8306487,		/* IT_CBBoxD       ("cbx_d_alp_ovl") */
	8372023,		/* IT_CBBoxE       ("cbx_e_alp_ovl") */
	0,				/* IT_Razor		   ("") *//* 没 */
	15949250,		/* IT_SocomSpprsr  ("scm_sp2_alp_ovl") */
	8633790,		/* IT_AKSpprsr     ("aks_sp2_alp_ovl") */
	10593390,		/* IT_DummyTnkCamera ("camera_alp_ovl") */
	5791201,		/* IT_MugenBandana ("bnd_alp_ovl") */
	8084969,		/* IT_DogTag       ("dog_alp_ovl") */
	7888397,		/* IT_MODisc       ("mod_alp_ovl") */
	9670083,		/* IT_UspSpprsr    ("usp_sp2_alp_ovl") */
	4626812,		/* IT_MugenWig     ("wig2_alp_ovl") */
	4561276,		/* IT_WigA         ("wig1_alp_ovl") */
	4561276,		/* IT_WigB         ("wig1_alp_ovl") */
	4561276,		/* IT_WigC         ("wig1_alp_ovl") */
	4561276,		/* IT_WigD         ("wig1_alp_ovl") */

	15110548,		/* IT_Uniform      ("gol2_alp_ovl") */
};
#if 0
static int	ItemGroup_Textures[] = {
	0,				/* IG_None     ("") */
	0,				/* IG_Cig 		("") */
	0,				/* IG_Card   ("") */
	0,				/* IG_Sensor   ("") */
	0,				/* IG_Spprsr   ("") */
	0,				/* IG_Etc1   ("") */
	0,				/* IG_Goggle   ("") */
	0,				/* IG_Scope    ("") */
	0,				/* IG_Armor    ("") */
	0,				/* IG_Box1     ("") */
	0,				/* IG_Ration ("") */
	0,				/* IG_Medicine ("") */
	0,				/* IG_DogTag  ("") */
	0,				/* IG_Etc2     ("") */
	0,				/* IG_Wig    ("") */
//	0,				/* IG_Disc   ("") */
//	0,				/* IG_Box2     ("") */
//	0,				/* IG_Uniform  ("") */
};
#endif
#if 0
static int	ItemGroup_Colors[] = {
	MENU_BLACK,				/* IG_None     ("") */
	MENU_PINK,				/* IG_Cig 		("") */
	MENU_VIOLET,				/* IG_Card   ("") */
	MENU_YELLOW,				/* IG_Sensor   ("") */
	MENU_CYAN,				/* IG_Spprsr   ("") */
	MENU_ORANGE,				/* IG_Etc1   ("") */
	MENU_DGRAY,				/* IG_Goggle   ("") */
	MENU_ORANGE2,				/* IG_Scope    ("") */
	MENU_WHITE,				/* IG_Armor    ("") */
	MENU_BLUE,				/* IG_Box1     ("") */
	0xc0c000,				/* IG_Ration ("") */
	0xc000c0,				/* IG_Medicine ("") */
	0x204060,				/* IG_DogTag  ("") */
	0x604020,				/* IG_Etc2     ("") */
	0x602040,				/* IG_Wig    ("") */
//	MENU_BLUE,				/* IG_Disc   ("") */
//	0x00c0c0,				/* IG_Box2     ("") */
//	0x0000c0,				/* IG_Uniform  ("") */
};
#endif

static int	Weapon_Textures[] = {
	/* 0 */
	0,				/* WP_None          ("") */
	875609,			/* WP_m92           ("m9_alp_ovl") */
	286254,			/* WP_Usp           ("usp_alp_ovl") */
	89636,			/* WP_Socom         ("scm_alp_ovl") *//* (431973)"scm2_alp_ovl" */
	16473625,		/* WP_Psg1          ("psg_alp_ovl") */
	7757344,		/* WP_Rgb6          ("rgb_alp_ovl") */
	16618748,		/* WP_Nikita        ("niki_alp_ovl") */
	12752808,		/* WP_Stinger       ("stin_alp_ovl") */
	13471647,		/* WP_Claymore      ("cray_alp_ovl") */
	6708708,		/* WP_C4Bomb        ("cfr_alp_ovl") */
	6341084,		/* WP_ChaffGrenade  ("chaff_alp_ovl") */
	4364202,		/* WP_StunGrenade   ("stun_alp_ovl") */
	4480489,		/* WP_Mic           ("dmp_alp_ovl") */
	5660152,		/* WP_Blade         ("hfb_alp_ovl") */
	2579941,		/* WP_ColdSpray     ("cls_alp_ovl") */
	482781,			/* WP_Aks           ("aks_alp_ovl")*//* (13453142)"aks_sp_alp_ovl" */
	12279307,		/* WP_Magazine      ("mag_alp_ovl") */
	14245365,		/* WP_Grenade       ("gre_alp_ovl") */
	4116054,		/* WP_m4            ("m4a_nm_alp_ovl") */
	4625956,		/* WP_Psg1T         ("psg2_alp_ovl") */
	4480489,		/* WP_DemoMic       ("dmp_alp_ovl") */
	9854517,		/* WP_Book 			("wpb_alp_ovl") */

	/* サプレッサー付きテクスチャ */
	431973,			/* WP_Socom         ("scm2_alp_ovl") */
	13453142,		/* WP_Aks           ("aks_sp_alp_ovl") */
	16106966,		/* WP_Usp           ("usp_sp_alp_ovl") */
	5660152,		/* WP_Blade         ("hfb_alp_ovl") */
	10916335,		/* WP_Blade（非殺傷）("hfb2_alp_ovl") */

};
#if 0
static int	WeaponGroup_Textures[] = {
	MENU_BLACK,				/* WG_None    ("") */
	MENU_LGRAY,				/* WG_HandGun ("") */
	MENU_CYAN,				/* WG_Rifle   ("") */
	MENU_PINK,				/* WG_Throw   ("") */
	MENU_VIOLET,			/* WG_Heavy   ("") */
	MENU_ORANGE,			/* WG_Blade   ("") */
	MENU_DGRAY,				/* WG_Mic 	  ("") */
	MENU_ORANGE2,			/* WG_Book    ("") */
	MENU_YELLOW,			/* WG_Set     ("") */
	MENU_BLUE,				/* WG_Spray   ("") */
};
#endif
#if 0
static int	WeaponGroup_Colors[] = {
	MENU_BLACK,				/* WG_None    ("") */
	MENU_LGRAY,				/* WG_HandGun ("") */
	MENU_CYAN,				/* WG_Rifle   ("") */
	MENU_PINK,				/* WG_Throw   ("") */
	MENU_VIOLET,			/* WG_Heavy   ("") */
	MENU_ORANGE,			/* WG_Blade   ("") */
	MENU_DGRAY,				/* WG_Mic 	  ("") */
	MENU_ORANGE2,			/* WG_Book    ("") */
	MENU_YELLOW,			/* WG_Set     ("") */
	MENU_BLUE,				/* WG_Spray   ("") */
};
#endif


enum {
	TEX_WINDOW1,
	TEX_WINDOW2,
	TEX_DEFAULT,
	TEX_NO_USE,
	TEX_BULLET,
	TEX_FUNA,
	TEX_EMPTY,
	MAX_OTHER_TEX_LIST
};
static int Other_Textures[] = {
	12738100,			/* TEX_WINDOW1 "win_alp_ovl" */
	//2529661,			/* TEX_WINDOW2 "win2_alp_ovl" */
	0,					/* TEX_WINDOW2 "win2_alp_ovl" */
	0,					/* TEX_DEFAULT ??? */
	14450094,			/* TEX_NO_USE "no_use_alp_ovl" */
	2645473,			/* TEX_BULLET "blt_alp_ovl" */
	5607725, 			/* TEX_FUNA "funa_alp_ovl" */
	3400858,			/* TEX_EMPTY "empty_alp_ovl" */
	0
};

/* ---------------------------------------------------------------- */
/* 符号なし８ビットから符号なし１６ビットへ拡張 */
#define PUBEXT(_d) ({ u_long128 __tmp ; asm ("pextlb %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* １６ビットから８ビットへパッキング */
#define PUBPAC(_d) ({ u_long128 __tmp ; asm ("ppacb %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* ３２ビットから１６ビットへパッキング */
#define PUHPAC(_d) ({ u_long128 __tmp ; asm ("ppacw %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* １６ビット乗算（符号付き） */
#define PHMUL(_a,_b) ({ u_long128 __tmp ; asm ("pmulth $0,%1,%2;pmfhl.sh %0":"=r"(__tmp):"r"(_a),"r"(_b) ); __tmp ;})

/* ---------------------------------------------------------------- */
/* 薬きょう表示用スプライト描画プリミティブ */
typedef struct {
	DG_DMATAG		dmatag_tex ;	/* テクスチャ設定パケット転送タグ */
	DG_DMATAG		dmatag_sprt ;	/* スプライト本体転送タグ */
	DG_GIFTAG		sprt_tag ;		/* スプライトプリミティブ用GIFタグ */
	DG_MENU_SPRT	sprt[MAX_DRAW_BULLET] ;			/* スプライトプリミティブ */
} PRIM_BULLET ;

/* 単一のテクスチャ設定付きスプライト描画プリミティブ */
typedef struct {
	DG_DMATAG		dmatag_tex ;	/* テクスチャ設定パケット転送タグ */
	DG_DMATAG		dmatag_sprt ;	/* スプライト本体転送タグ */
	DG_GIFTAG		sprt_tag ;		/* スプライトプリミティブ用GIFタグ */
	DG_MENU_SPRT	sprt ;			/* スプライトプリミティブ */
} PRIM_SPRITE ;

/* パネルウィンドウ表示用プリミティブ集合 */
typedef struct {
	/* 背景ウィンドウ */
	PRIM_SPRITE		window_prim ;
	/* アイコンウィンドウ */
	PRIM_SPRITE		icon_prim ;
	/* 使用禁止マーク */
	PRIM_SPRITE		mark_prim ;
	/* タイトルウィンドウ */
	PRIM_SPRITE		title_prim ;
} PRIM_PANEL ;

/* グループウィンドウ表示用プリミティブ集合 */
typedef struct {
	/* 背景ウィンドウ */
	PRIM_SPRITE		window_prim ;
	PRIM_SPRITE		color_prim ;
} PRIM_GROUP ;

/* 説明文描画用プリミティブ */
typedef struct {
	DG_DMATAG		dmatag ;		/* 本体転送タグ */
	DG_GIFTAG		gif_tag ;		/* ＧＳ設定用GIFタグ */
	struct _explain_setup{
		DG_GSREG	texflush ;		/* テクスチャキャッシュのフラッシュ */
		//DG_GSREG	tex1 ;			/* テクスチャ設定 */
#ifndef HIGHRESO_FFI
		DG_GSREG	xyoffset ;		/* ハイレゾ化のためのオフセット設定 */
#endif
		DG_GSREG	tex0 ;			/* テクスチャ設定 */
		DG_GSREG	clamp ;			/* クランプ設定 */
		DG_GSREG	alpha ;			/* アルファ設定 */
	} data ;
	/* テクスチャ描画 */
	DG_GIFTAG		sprt_tag ;		/* スプライトプリミティブ用GIFタグ */
	//DG_MENU_SPRT	win_sprt ;		/* スプライトプリミティブ */
	DG_MENU_SPRT	mes_sprt ;		/* スプライトプリミティブ */
} PRIM_EXPLAIN ;




/* プリミティブ表示コントロール構造体 */
typedef struct {
	DG_DMAPACK		*dmapack ;
	int				tri_id ;			/* 使用TRI */
	DG_TEXTURE_LIST	*tex_list ;			/* TRIデータポインタ */
	int				current_offset ;	/* 書き込みデータ最終位置オフセット（1qword単位） */
	int				end_offset ;		/* 書き込みデータ限界位置オフセット（1qword単位） */
	u_long128		*buffer[2] ;		/* 転送バッファ */
	u_long128		*current_buffer ;	/* 最終書き込み位置アドレス */
} PRIM_CONTROL ;


typedef struct {
	GV_ACT	actor ;
	PRIM_CONTROL	panel_ctrl ;	/* パネル表示用 */
	PRIM_CONTROL	group_ctrl ;	/* グループ表示用 */
	PRIM_CONTROL	explain_ctrl ;	/* 説明分表示用（テクスチャ転送パケットなども含む） */

#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_VRAMINFO  bp_vinfo;
   unsigned int      bp_tex;
#else
   FONT_VRAMINFO	vinfo;			/* フォント展開ワーク */
   void			*vram ;			/* フォント展開領域 */
   int				vram_size ;

   /* XBOX追加 */
   DG_TEX_LIN		*tex ;			/* フォント展開用テクスチャ */
#endif
   char			*last_message ;	/* フォント展開領域に書き込まれているメッセージへのポインタ */

	int				current_menu_type ;		/* 現在選択中のメニュータイプ（0:old 1:new） */
	void			*item_change_ptr ;		/* アイテム装備メニューアクターのワーク */
	void			*weapon_change_ptr ;	/* 武器装備メニューアクターのワーク */

} Work ;

#if defined(BP_VITA)

typedef struct SIconAnimState 
{
   float time;
   int currValue;
   int enablePulse;
   int doPulse;
   int itemOverrideTexId;
   int origTexId;
   int timeToSwapBackIcon;
   DG_DMAPACK_COLOR iconColor;
} SIconAnimState;

float const kMaxTime = PI / 2.0f;
float const kTimeStep = 0.05 * (PI / 2.0f);
int const kMaxPulseSize = 21; // Icon will shrink and grow over 30 pixels.
int const kMaxIconSwapTime = 25;
static SIconAnimState g_IconAnimState;

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
extern void *NewItemChangeVita( int );
extern void *NewWeaponChangeVita( int );
extern void BP_PlayAPSensorPing();

#else
extern void *NewItemChange( int );
extern void *NewItemChange2( int );
extern void *NewWeaponChange( int );
extern void *NewWeaponChange2( int );
#endif

static Work	*work_ptr = NULL ;
static DG_TEX	*ItemTexList[MAX_ITEMS+1] ;
static DG_TEX	*WeaponTexList[MAX_WEAPONS+5] ;
static DG_TEX	*OtherTexList[MAX_OTHER_TEX_LIST];

/*------------------------------------------------------------------*/
	/*
		メニュー取り付きフナムシ用管理ワーク
	*/
static int	disp_ration_flag ;
static int	disp_ration_x ;
static int	disp_ration_y ;
static int	disp_ration_alpha ;
static int	shipworm_offset_x = 0 ;
static int	shipworm_offset_y = 0 ;

int MENU_GetRationPanelInfo( int *x, int *y, int *alpha )
{
	*x = disp_ration_x ;
	*y = disp_ration_y ;
	*alpha = disp_ration_alpha ;
	return ( disp_ration_flag );
}
void MENU_SetShipwormOffset( int x, int y )
{
	shipworm_offset_x = x ;
	shipworm_offset_y = y ;
}

/*------------------------------------------------------------------*/
	/*
		外部プログラムからのメニュー表示関連制御
	*/
static int	menu_disp_status ;
int MENU_GetMenuDispStatus( void )
{
	return ( menu_disp_status );
}

void MENU_SetMenuDispStatus( int status )
{
	menu_disp_status |= status ;
}
void MENU_ResetMenuDispStatus( int status )
{
	menu_disp_status &= ~status ;
}

#if defined(BP_VITA)
//----------------------------------------------------------------------------
void MENU_Item_Panel_ResetAnim(int isEnabled)
{
   if (!isEnabled)
   {
      g_IconAnimState.time = 0.0f;
      g_IconAnimState.currValue = 0;
      g_IconAnimState.enablePulse = 0;
      g_IconAnimState.doPulse = 0;
      g_IconAnimState.iconColor = DG_MakeDmaPackColorFromInt(0x80808080);
      g_IconAnimState.origTexId = -1;
      g_IconAnimState.itemOverrideTexId = -1;
      g_IconAnimState.timeToSwapBackIcon = 0;
   }

   g_IconAnimState.enablePulse = isEnabled;
}

//----------------------------------------------------------------------------
void MENU_Item_Panel_PulseAnim()
{
   // We need to add this additional check here in case the stage is started and the AP sensor was initially equipped 
   // without the user actually equipping it explicitly.
   if (!g_IconAnimState.enablePulse && GM_Item == IT_VibSensor)
      g_IconAnimState.enablePulse = 1;

   if (g_IconAnimState.enablePulse)
   {
      g_IconAnimState.doPulse = 1;
      g_IconAnimState.time = kMaxTime;

      if (GM_Item == IT_VibSensor)
      {
         BP_PlayAPSensorPing();
      }
   }
}
//----------------------------------------------------------------------------
void MENU_Item_Panel_Swap_Texture(int newTexID)
{
   g_IconAnimState.itemOverrideTexId = newTexID;
}

#endif

/*------------------------------------------------------------------*/

/* プリミティブコントロールの確保 */
static void MakePrimControl( PRIM_CONTROL *prim_ctrl, int buffer_size, int prio, int tri_id )
{
	DG_DMAPACK	*dmapack ;
	u_long128	*buffer ;

	/* プリミティブを確保（DG_DMAPACKを使用） */
	dmapack = prim_ctrl->dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, prio );
	DG_QueueDmapack( dmapack );
	/* プリミティブバッファの確保 */
	buffer = GV_Malloc( sizeof(u_long128) * buffer_size * 2 );
	prim_ctrl->buffer[0] = &buffer[0] ;
	prim_ctrl->buffer[1] = &buffer[buffer_size] ;
	/* テクスチャパケットの取得 */
	prim_ctrl->tri_id = tri_id ;
	prim_ctrl->tex_list = DG_GetTextureList( tri_id );
#if 0
	/* DMAPACKにバッファを設定 */
	dmapack->packet[0] = prim_ctrl->buffer[0] ;
	dmapack->packet[1] = prim_ctrl->buffer[1] ;
#endif
	dmapack->autopacket = buffer ;

	/* その他初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->end_offset = buffer_size ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ 0 ] ;
}

/* プリミティブコントロールの開放 */
static void FreePrimControl( PRIM_CONTROL *prim_ctrl )
{
	DG_DequeueDmapack( prim_ctrl->dmapack );
	DG_FreeDmapack( prim_ctrl->dmapack );
	GV_DelayedFree( prim_ctrl->buffer[0] );
}

static void ResetMenuSafeZone()
{
   MENU_SetSafeZoneTrim(0.0f);
}

static void SetMenuSafeZone()
{
   float const safeZone = SAFE_ZONE_AMOUNT;
   MENU_SetSafeZoneTrim(safeZone);
}

static void SetViewMapping(PRIM_CONTROL *prim_ctrl)
{
   float const safeZone = SAFE_ZONE_AMOUNT;
   float const ooSafeZone = 1.0f - safeZone;
   prim_ctrl->current_buffer = DG_SetDmapackViewMapping(prim_ctrl->current_buffer, safeZone*DRAW_WIDTH, safeZone*DRAW_HEIGHT, ooSafeZone*DRAW_WIDTH, ooSafeZone*DRAW_HEIGHT);
}

static void ResetViewMapping(PRIM_CONTROL *prim_ctrl)
{
   prim_ctrl->current_buffer = DG_SetDmapackViewMapping(prim_ctrl->current_buffer, 0.0f, 0.0f, DRAW_WIDTH, DRAW_HEIGHT);
}

/* 終端タグの書き込み */
static void ClosePrimControl( PRIM_CONTROL *prim_ctrl )
{
   ResetViewMapping(prim_ctrl);
	DG_SetDmapackEnd( prim_ctrl->current_buffer );
	/* 追加書き込みをする為終端後アドレスは記録しない（次に上書きできるように） */
}

/* プリミティブコントロールの初期化 */
static void InitPrimControl( PRIM_CONTROL *prim_ctrl )
{
	int		size ;

	if ( prim_ctrl->current_offset >= ( prim_ctrl->end_offset - 1 ) ){
		printf("menu_sub.c: prim buffer over error!!!\n");
	}
	/* 書き込み位置の初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ 0 ] ;
	/* 終端タグの書き込み */

   ClosePrimControl( prim_ctrl );
}

#if 0
/* バッファから指定したサイズのメモリを確保 */
static void *GetPrimBuffer( PRIM_CONTROL *prim_ctrl, int size )
{
	void	*buff_addr ;
	if ( ( prim_ctrl->current_offset + size ) > prim_ctrl->end_offset ) return ( NULL );
	buff_addr = prim_ctrl->current_buffer ;
	prim_ctrl->current_buffer += size ;
	prim_ctrl->current_offset += size ;
	/* 先に終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );
	return ( buff_addr );
}
#endif

/* ---------------------------------------------------------------- */

/* スプライト描画プリミティブの設定 */
/* 初期設定 */
static void SetSprite( PRIM_SPRITE *sprt )
{
#if 0
	/* DMAタグ＆GIFタグの設定 */
	sprt->dmatag_sprt.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DG_MENU_SPRT)+SIZEOF_QWORD(DG_GIFTAG) );
	sprt->dmatag_sprt.vifcode[0] = SCE_VIF1_SET_NOP(0);
	sprt->dmatag_sprt.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_MENU_SPRT)+SIZEOF_QWORD(DG_GIFTAG) ,0 );
	sprt->sprt_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_SPRITE ;
	*(u_short*)&sprt->sprt_tag |= 1 ;	/* GIFパケット転送データを１に設定 */
	sprt->sprt.prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
#endif
}
/* テクスチャ＆ＵＶ設定 */
static void SetSpriteTex( PRIM_SPRITE *sprt, DG_TEX *tex )
{
#if 0
	/* テクスチャ設定 */
	if ( tex != NULL ){
		sprt->dmatag_tex.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
		sprt->dmatag_tex.addr = &tex->tex_trans ;
		sprt->dmatag_tex.vifcode[0] = SCE_VIF1_SET_NOP(0);
		sprt->dmatag_tex.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_TEX_TRANS) ,0 );
	} else {
#if 0
		sprt->sprt.prim &= ~SCE_GS_PRIM_TME ;
		sprt->dmatag_tex.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
		sprt->dmatag_tex.vifcode[0] = SCE_VIF1_SET_NOP(0);
		sprt->dmatag_tex.vifcode[1] = SCE_VIF1_SET_NOP(0);
#else
		sprt->sprt.prim &= ~SCE_GS_PRIM_TME ;
		sprt->dmatag_tex.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
		sprt->dmatag_tex.addr = &DG_SystemTexture[2].tex_trans ;
		sprt->dmatag_tex.vifcode[0] = SCE_VIF1_SET_NOP(0);
		sprt->dmatag_tex.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_TEX_TRANS) ,0 );
#endif
	}
#endif
}
/* 座標設定 */
static void SetSpritePos( DG_MENU_SPRT *sprt, int x1, int y1, int x2, int y2 )
{
#if 0
#ifdef PSX2
	static FVECTOR	scale = { (float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT, 
								(float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT };
	static FVECTOR	offset = { 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2,
								 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2 };
	IVECTOR			tmp ;
	tmp.vx = x1 ;
	tmp.vy = y1 ;
	tmp.vz = x2 ;
	tmp.vw = y2 ;
	asm volatile("
		lqc2			vf1,0x00(%0)	# 
		lqc2			vf2,0x00(%1)	# 
		lqc2			vf3,0x00(%2)	# 
		vitof0.xyzw		vf1,vf1			# 整数から浮動小数点へ
		vmula.xyzw		ACC,vf2,vf1		# スケール乗算
		vmaddw.xyzw		vf1,vf3,vf0		# オフセット加算
		vftoi4.xyzw		vf1,vf1			# 浮動小数点から整数へ
		sqc2			vf1,0x00(%0)	# 
	"::"r"(&tmp),"r"(&scale),"r"(&offset):"memory" );
	DG_SET_XY1( sprt, tmp.vx, tmp.vy );
	DG_SET_XY2( sprt, tmp.vz, tmp.vw );
#else
	x1 = ( (float)DRAW_WIDTH/VR_WIDTH * x1 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	y1 = ( (float)DRAW_HEIGHT/VR_HEIGHT * y1 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	x2 = ( (float)DRAW_WIDTH/VR_WIDTH * x2 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	y2 = ( (float)DRAW_HEIGHT/VR_HEIGHT * y2 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	DG_SET_XY1( sprt, x1, y1 );
	DG_SET_XY2( sprt, x2, y2 );
#endif
#endif
}
/* ＵＶ設定 */
static void SetSpriteUv( DG_MENU_SPRT *sprt, int u1, int v1, int u2, int v2 )
{
#if 0
	/* UV設定 */
	DG_SET_UV1( sprt, u1, v1 );
	DG_SET_UV2( sprt, u2, v2 );
#endif
}
/* 色設定 */
inline static void SetSpriteCol( DG_MENU_SPRT *sprt, int col )
{
#if 0
	//DG_SET_RGBA1( sprt, r, g, b, a );
	*(int*)&sprt->rgba1.r = col ;
#endif
}
/* ---------------------------------------------------------------- */
/* 初期設定 */
static void SetBullet( PRIM_BULLET *bullet )
{
#if 0
	int		i ;
	/* DMAタグ＆GIFタグの設定 */
	bullet->dmatag_sprt.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DG_MENU_SPRT)*MAX_DRAW_BULLET+SIZEOF_QWORD(DG_GIFTAG) );
	bullet->dmatag_sprt.vifcode[0] = SCE_VIF1_SET_NOP(0);
	bullet->dmatag_sprt.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_MENU_SPRT)*MAX_DRAW_BULLET+SIZEOF_QWORD(DG_GIFTAG) ,0 );
	bullet->sprt_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_SPRITE ;
	*(u_short*)&bullet->sprt_tag |= MAX_DRAW_BULLET ;	/* GIFパケット転送データを１に設定 */
	for ( i = 0 ; i < MAX_DRAW_BULLET ; i++ ){
		bullet->sprt[i].prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
	}
#endif
}

/* ---------------------------------------------------------------- */
/* パネルプリミティブを指定位置に表示 */
static void PutPanelPrim( int x, int y, int col1, int col2, int col3, DG_TEX *icon_tex, DG_TEX *mark_tex, int flag, int enable_flash)
{
	PRIM_CONTROL	*prim_ctrl ;
	PRIM_PANEL		*panel ;
//	PRIM_SPRITE		*sprt ;
	DG_TEX			*tex ;
	int				tx, ty, tw, th, w, h ;
	float			x0, x1, y0, y1 ;
	int             xy_changed = 0 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->panel_ctrl ;

   SetViewMapping(prim_ctrl);

	/* タイトルスプライトの設定 */
	tex = OtherTexList[ TEX_WINDOW1 ] ;
	//DG_GetTexelInfo( &tw, &th, &tx, &ty, tex );					/* テクスチャサイズ取得 */
	x0 = SCALE_X( x );
	y0 = SCALE_Y( y + PH - TITLE_HEIGHT );
	x1 = SCALE_X( x + PW );
	y1 = SCALE_Y( y + PH );

	prim_ctrl->current_buffer = DG_SetDmapackTex( prim_ctrl->current_buffer, tex );
	prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
													 x0, y0, 0.0f, 0.0f,
													 x1, y1, 1.0f, 1.0f,
													 DG_MakeDmaPackColorFromInt(0x00000000),
													 tex );
	/* 背景ウィンドウスプライトの設定 */
	tex = OtherTexList[ TEX_WINDOW1 ];
	//DG_GetTexelInfo( &tw, &th, &tx, &ty, tex );					/* テクスチャサイズ取得 */
	x0 = SCALE_X( x );
	y0 = SCALE_Y( y );
	x1 = SCALE_X( x + PW );
	y1 = SCALE_Y( y + PH );

   
   prim_ctrl->current_buffer = DG_SetDmapackTex( prim_ctrl->current_buffer, tex );
	prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
													 x0, y0, 0.0f, 0.0f,
													 x1, y1, 1.0f, 1.0f,
													 DG_MakeDmaPackColorFromInt(col1),
													 tex );
	/* アイコンスプライトの設定 */
	if ( icon_tex != NULL && !( ( mark_tex == OtherTexList[ TEX_FUNA ] ) && ( GM_ItemNum( IT_Ration ) == 0 ) ) )
   {
		xy_changed = 1 ; /* xyの補正が入った */
		tex = icon_tex ;					/* テクスチャ取得 */
		DG_GetTexelInfo( &tw, &th, &tx, &ty, tex );					/* テクスチャサイズ取得 */

      if (!enable_flash)
      {
         tw = tw * TARGET_ASPECT_X;
         h = th / 2 ;												/* サイズ用に取得 */
      }
      else
      {
#if defined(BP_VITA)
         tw = (int)((tw + g_IconAnimState.currValue) * TARGET_ASPECT_X);
         h = (int)((th  + g_IconAnimState.currValue) / 2.0f);
#else
         tw = tw * TARGET_ASPECT_X;
         h = th / 2 ;												/* サイズ用に取得 */
#endif
      }

      w = tw / 2 ;												/* サイズ用に取得 */

		x = (x + PW / 2 + 8) ;						/* 背景ウィンドウの中央座標を計算 */
		y = y + ( PH - TITLE_HEIGHT ) / 2 + 2 ;		/* 背景ウィンドウの中央座標を計算 */
		x0 = SCALE_X( (x-3) - w );
		y0 = SCALE_Y( y - h );
		x1 = SCALE_X( (x-3) + w );
		y1 = SCALE_Y( y + h );
		/* テクスチャ取得（フナムシマーク） */
		if ( ( mark_tex == OtherTexList[ TEX_FUNA ] ) &&
			( GM_ItemNum( IT_Ration ) == 0 ) ){
         // BP_WARNING: sprt was never initialized, so we have garbage pointer to garbage
            //			SetSpriteCol( &sprt->sprt, 0 );										/* 色設定 */
         BP_RENDER_TODO_BREAK;
		}

      //This fixes the item selection highlight bug
		prim_ctrl->current_buffer = DG_SetDmapackTex( prim_ctrl->current_buffer, tex );

      if (enable_flash)
      {
#if defined(BP_VITA)
        prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
            x0, y0, 0.0f, 0.0f,
            x1, y1, 1.0f, 1.0f,
            g_IconAnimState.iconColor,
            tex );
#else
         prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
            x0, y0, 0.0f, 0.0f,
            x1, y1, 1.0f, 1.0f,
            DG_MakeDmaPackColorFromInt(col2),
            tex );
#endif
      }
      else
      {
         prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
            x0, y0, 0.0f, 0.0f,
            x1, y1, 1.0f, 1.0f,
            DG_MakeDmaPackColorFromInt(col2),
            tex );
      }

		
	}

	/* マークスプライトの設定 */
	if ( flag & PANEL_FLAG_NO_USE )
   {
		tex = mark_tex ;					/* テクスチャ取得 */
		DG_GetTexelInfo( &tw, &th, &tx, &ty, tex );					/* テクスチャサイズ取得 */
      tw = tw * TARGET_ASPECT_X;

      w = tw / 2 ;												/* サイズ用に取得 */
		h = th / 2 ;												/* サイズ用に取得 */
		if ( xy_changed == 0 ){ /* 補正が入っていないので入れる */
			x = x + PW / 2 + 8 ;						/* 背景ウィンドウの中央座標を計算 */
			y = y + ( PH - TITLE_HEIGHT ) / 2 + 2 ;		/* 背景ウィンドウの中央座標を計算 */
		}
		if ( mark_tex == OtherTexList[ TEX_FUNA ] ){	/* テクスチャ取得（フナムシマーク） */
			x += shipworm_offset_x ;
			y += shipworm_offset_y ;
		}


		x0 = SCALE_X( (x-2) - w );
		y0 = SCALE_Y( y - h );
		x1 = SCALE_X( (x-2) + w );
		y1 = SCALE_Y( y + h );
		/* テクスチャ取得（フナムシマーク） */
		if ( ( mark_tex == OtherTexList[ TEX_FUNA ] ) &&
			( GM_ItemNum( IT_Ration ) == 0 ) ){
         // BP_WARNING: sprt was never initialized, so we have garbage pointer to garbage
			//SetSpriteCol( &sprt->sprt, 0 );										/* 色設定 */
         BP_RENDER_TODO_BREAK;
		}
		prim_ctrl->current_buffer = DG_SetDmapackTex( prim_ctrl->current_buffer, tex );
		prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
														 x0, y0, 0.0f, 0.0f,
														 x1, y1, 1.0f, 1.0f,
														 DG_MakeDmaPackColorFromInt((col2&0xff000000)|0x002e24e8),
														 tex );
	}

	ClosePrimControl( prim_ctrl );

#if defined(BP_VITA)
   // This function is called each render frame so just increment the time step here.
   if (g_IconAnimState.enablePulse && g_IconAnimState.doPulse)
   {
      float t = BP_SinFast(g_IconAnimState.time);

      g_IconAnimState.currValue = (int)(t * kMaxPulseSize);
      g_IconAnimState.iconColor.r = 255;
      g_IconAnimState.iconColor.g = (int)(t * 255);
      g_IconAnimState.iconColor.b = (int)(t * 255);

      g_IconAnimState.time -= kTimeStep;
      if (g_IconAnimState.time <= 0.0f)
      {
         g_IconAnimState.doPulse = 0;
         g_IconAnimState.iconColor = DG_MakeDmaPackColorFromInt(0x80808080);
      }
   }
   else
   {
      g_IconAnimState.currValue = 0;
      g_IconAnimState.iconColor = DG_MakeDmaPackColorFromInt(0x80808080);
   }
#endif
}
/* ---------------------------------------------------------------- */
/* 薬きょう表示プリミティブを指定位置に表示 */
static void PutBulletPrim( int x, int y, int n_bullet, int n_max_bullet, int n_last_bullet, int flag )
{
	PRIM_CONTROL	*prim_ctrl ;
	PRIM_BULLET		*bullet ;
	DG_TEX			*tex ;
	int				tx, ty, tw, th, i ;
	int				bw, bh ;
	float			x0, x1, y0, y1 ;

	if ( work_ptr == NULL ) return ;
	if ( n_max_bullet > MAX_DRAW_BULLET ) n_max_bullet = MAX_DRAW_BULLET ;
	if ( n_bullet > MAX_DRAW_BULLET ) n_bullet = MAX_DRAW_BULLET ;
	prim_ctrl = &work_ptr->panel_ctrl ;

   SetViewMapping(prim_ctrl);

	tex = OtherTexList[ TEX_BULLET ];
	DG_GetTexelInfo( &tw, &th, &tx, &ty, tex );					/* テクスチャサイズ取得 */
   tw = tw*TARGET_ASPECT_X;
   bw = tw ; bh = th ;
	x += PW - bw ;
	y += PH - TITLE_HEIGHT + ( TITLE_HEIGHT - bh ) / 2 ;
	prim_ctrl->current_buffer = DG_SetDmapackTex( prim_ctrl->current_buffer, tex );
	for ( i = 0 ; i < MAX_DRAW_BULLET ; i++ ){
		x0 = SCALE_X( x );
		y0 = SCALE_Y( y );
		x1 = SCALE_X( x + bw );
		y1 = SCALE_Y( y + bh );
		if ( i >= n_max_bullet && i >= n_bullet ){
			/* 表示しない */
		} else if ( i >= n_bullet ){
			prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
														  x0, y0, 0.0f, 0.0f,
														  x1, y1, 1.0f, 1.0f,
														  DG_MakeDmaPackColorFromInt(0x803e380c),	/* 色設定 */
														  tex );
		} else if ( i < n_last_bullet ){
			prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
														  x0, y0, 0.0f, 0.0f,
														  x1, y1, 1.0f, 1.0f,
														  DG_MakeDmaPackColorFromInt(0x803c4dc4),	/* 色設定 */
														  tex );
		} else {
			prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
														  x0, y0, 0.0f, 0.0f,
														  x1, y1, 1.0f, 1.0f,
														  DG_MakeDmaPackColorFromInt(0x808c946f),	/* 色設定 */
														  tex );
		}
		x -= bw ;
	}

	ClosePrimControl( prim_ctrl );
}
/* ---------------------------------------------------------------- */
static void PutGroupPrim( int x, int y, int col )
{
	PRIM_CONTROL	*prim_ctrl ;
	float			x0, x1, y0, y1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->group_ctrl ;

   SetViewMapping(prim_ctrl);

	/* 背景ウィンドウスプライトの設定 */
	x0 = SCALE_X( x );
	y0 = SCALE_Y( y );
	x1 = SCALE_X( x + GW );
	y1 = SCALE_Y( y + PH );
	prim_ctrl->current_buffer = DG_SetDmapackBox( prim_ctrl->current_buffer,
													 x0, y0,
													 x1, y1,
													 DG_MakeDmaPackColorFromInt(col & 0xff000000) );

	x0 = SCALE_X( x + 4 );
	y0 = SCALE_Y( y + PH - TITLE_HEIGHT + 3 );
	x1 = SCALE_X( x + GW - 4 );
	y1 = SCALE_Y( y + PH - 3 );
	prim_ctrl->current_buffer = DG_SetDmapackBox( prim_ctrl->current_buffer,
													 x0, y0,
													 x1, y1,
													 DG_MakeDmaPackColorFromInt(col) );

	ClosePrimControl( prim_ctrl );
}

/* ---------------------------------------------------------------- */

void MENU_PutPagePrim(int x, int y, int alpha, int col)
{
   PRIM_CONTROL	*prim_ctrl ;
   float			x0, x1, y0, y1 ;

   if ( work_ptr == NULL ) return ;
   /* プリミティブを１枚分確保 */
   prim_ctrl = &work_ptr->group_ctrl ;

   x0 = SCALE_X( x );
   y0 = SCALE_Y( y );
   x1 = SCALE_X( x + 5 );
   y1 = SCALE_Y( y + 16 );
   col |= alpha << 24;
   prim_ctrl->current_buffer = DG_SetDmapackBox( prim_ctrl->current_buffer,
                                        x0, y0,
                                        x1, y1,
                                        DG_MakeDmaPackColorFromInt(col) );

   ClosePrimControl( prim_ctrl );
}

/* ---------------------------------------------------------------- */
/* バッファから指定したサイズのメモリを確保 */
static void PutBackPanelPrim( int x, int y, int col )
{
	PRIM_CONTROL	*prim_ctrl ;
	float			x0, x1, y0, y1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->panel_ctrl ;

   SetViewMapping(prim_ctrl);

	{/* アルファ値をコントロール */
		int	a ;
		a = col >> 24 ;
		a = ( a * 109 ) / 128 ;
		col = ( col & 0xffffff ) | ( a << 24 );
	}

	/* 背景ウィンドウスプライトの設定 */
	x0 = SCALE_X( x );
	y0 = SCALE_Y( y );
	x1 = SCALE_X( x + PW );
	y1 = SCALE_Y( y + PH );
	prim_ctrl->current_buffer = DG_SetDmapackBox( prim_ctrl->current_buffer,
													 x0, y0,
													 x1, y1,
													 DG_MakeDmaPackColorFromInt(col) );

	ClosePrimControl( prim_ctrl );
}
/* バッファから指定したサイズのメモリを確保 */
static void PutShipwormPanelPrim( int x, int y, int col )
{
	PRIM_CONTROL	*prim_ctrl ;
	PRIM_SPRITE		*sprt ;
	DG_TEX			*tex ;
	int				tx, ty, tw, th, w, h ;
	float			x0, x1, y0, y1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->panel_ctrl ;

   SetViewMapping(prim_ctrl);

	tex = OtherTexList[ TEX_FUNA ];	/* テクスチャ取得（フナムシマーク） */
	DG_GetTexelInfo( &tw, &th, &tx, &ty, tex );					/* テクスチャサイズ取得 */
   tw = tw * TARGET_ASPECT_X;
	w = tw / 2 ;												/* サイズ用に取得 */
	h = th / 2 ;												/* サイズ用に取得 */

	/* 背景ウィンドウスプライトの設定 */
	x0 = SCALE_X( x - w );
	y0 = SCALE_Y( y - h );
	x1 = SCALE_X( x + w );
	y1 = SCALE_Y( y + h );
	prim_ctrl->current_buffer = DG_SetDmapackTex( prim_ctrl->current_buffer, tex );
	prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
													 x0, y0, 0.0f, 0.0f,
													 x1, y1, 1.0f, 1.0f,
													 DG_MakeDmaPackColorFromInt(col),
													 tex );

	ClosePrimControl( prim_ctrl );
}
/* ---------------------------------------------------------------- */
/* パネルをアイテム用として表示する */
void MENU_PutItemPanel( int x, int y, int type, int alpha1, int alpha2, int flag )
{
	DG_TEX	*tex, *mark_tex ;
	int		col1, col2, col3, pos_x, pos_y ;

//	if ( GM_PlayerStatus & PLAYER_DEBUG ) return ;
#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_DISPLAY_OFF ) return ;
#endif

#if defined(BP_VITA)
   if (g_IconAnimState.itemOverrideTexId != -1 && g_IconAnimState.timeToSwapBackIcon == 0)
   {
      // Change the current item icon to the specified item override for a specified amount of time.
      g_IconAnimState.origTexId = type;
      g_IconAnimState.timeToSwapBackIcon = 1;
      tex = ItemTexList[g_IconAnimState.itemOverrideTexId];
   }
   else if (g_IconAnimState.timeToSwapBackIcon != 0)
   {
      if (g_IconAnimState.timeToSwapBackIcon == kMaxIconSwapTime)
      {
         tex = ItemTexList[g_IconAnimState.origTexId];
         g_IconAnimState.timeToSwapBackIcon = 0;
         g_IconAnimState.origTexId = -1;
         g_IconAnimState.itemOverrideTexId = -1;
      }
      else
      {
         g_IconAnimState.timeToSwapBackIcon++;
         tex = ItemTexList[g_IconAnimState.itemOverrideTexId];
      }
   }
   else
   {
      tex = ItemTexList[type];
   }
#else
   //tex_id = Item_Textures[ type ] ;
	tex = ItemTexList[ type ] ;
#endif

	mark_tex = OtherTexList[ TEX_NO_USE ] ;		/* テクスチャ取得（通常使用禁止マーク） */
	if ( type == IT_Ration && menu_disp_status & MENU_DISP_SHIPWORM ){
		flag |= PANEL_FLAG_NO_USE ;
		mark_tex = OtherTexList[ TEX_FUNA ];	/* テクスチャ取得（フナムシマーク） */
	}

   SetMenuSafeZone();

	if ( alpha1 < 1 ) alpha1 = 1 ;
	if ( alpha1 > 128 ) alpha1 = 128 ;
	if ( alpha2 < 1 ) alpha2 = 1 ;
	if ( alpha2 > 128 ) alpha2 = 128 ;
	switch ( flag & PANEL_FLAG_TYPEMASK ){
	  default:
	  case 0:	/* 通常表示時 */
		col1 = 0x00201810 | ( alpha1 << 24 );	/* ウィンドウカラー */
		col2 = 0x00808080 | ( alpha2 << 24 );	/* アイコン */
		col3 = 0x00000000 | ( (alpha2/2) << 24 );	/* タイトルカラー */
		break ;
	  case 1:	/* 選択中アクティブ時 */
		{
			int t = GV_Time & 63, t2 ;
			if ( t > 31 ) t = 64 - t ;
			t2 = t + 32 ;
			t = t * 2 + 32 ;
			//col1 = (t2) | (t2<<8) | (t<<16) | ( alpha1 << 24 );
			col1 = (42*3/2) | (53*3/2<<8) | (60*3/2<<16) | ( alpha1 << 24 ) ;
			//col1 = 0x808080 | ( alpha1 << 24 ) ;
			col2 = 0x00808080 | ( alpha2 << 24 );	/* アイコン */
			col3 = 0x00000000 | ( (alpha2/2) << 24 );	/* タイトルカラー */
		}
		break ;
	  case 2:	/* 選択中非アクティブ時 */
		col1 = 0x00201810 | ( alpha1 << 24 );
		col2 = 0x00404040 | ( alpha2 << 24 );	/* アイコン */
		col3 = 0x00000000 | ( (alpha2/2) << 24 );	/* タイトルカラー */
		break ;
	}
	if ( ( flag & PANEL_FLAG_TYPEMASK ) != 0 ){
		int		back_col = -1 ;
		if ( !( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX ) ){
			if ( GM_Item == IT_None ){
				if ( type == IT_None )		back_col = ((alpha1)<<24)|MENU_CURRENT_COL ;
				if ( type == GM_ItemPrev )	back_col = ((alpha1)<<24)|MENU_PREV_COL ;
			} else {
				if ( type == GM_Item )		back_col = ((alpha1)<<24)|MENU_CURRENT_COL ;
				if ( type == IT_None )		back_col = ((alpha1)<<24)|MENU_PREV_COL ;
			}
		} else {
			if ( type == GM_Item )			back_col = ((alpha1)<<24)|MENU_CURRENT_COL ;
			if ( type == GM_ItemPrev )		back_col = ((alpha1)<<24)|MENU_PREV_COL ;
		}
		if ( back_col != -1 ){
			PutBackPanelPrim( x, y, ((alpha1)<<24)|back_col );
			col1 = back_col ;
		}
	}
	if ( ( flag & PANEL_FLAG_TYPEMASK ) == 1 )
   {
      PutBackPanelPrim( x, y, ((alpha1)<<24)|MENU_SELECT_COL );
   }
	PutPanelPrim( x, y, col1, col2, col3, tex, mark_tex, flag, 1 );

	MENU_SetAlphaMode( 0, 1, 0, 1, 0 );
	/* アイテム名表示 */
	if ( alpha2 != 0 ){
		pos_x = SCALE_X( x );
		pos_y = SCALE_Y( y + TXT_SHIFT_Y );
		//pos_y = ( y + TXT_SHIFT_Y ) * DRAW_HEIGHT / VR_HEIGHT ;
		MENU_S_Locate( pos_x + TXT_SHIFT_X, pos_y, 1 );
      if ( !BP_Area_EU() )
      {
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
   		MENU_S_Color( 200, 200, 200, alpha2 );
      }
      else
      {
//#else
		   MENU_S_Color( 160, 160, 160, alpha2 );
      }
//#endif
		//if ( ItemMenuWork->flag == MENU_WIN_OPEN && type == IT_Magazine ) {
		//	MENU_Printf( "%s %d\n", Items[ type ], MagazinePage ) ;
		//} else {
			MENU_S_Printf( "%s\n", Items[ type ] ) ;
		//}
	}
	/* 所持数表示 */
	if ( alpha1 != 0 ){
		pos_x = DG_FTOI( SCALE_X( x ) );
		pos_y = ( y + NUM_TXT_SHIFT_Y ) * DRAW_HEIGHT / VR_HEIGHT ;
		MENU_S_Locate( pos_x + TXT_SHIFT_X, pos_y, 1 ) ;
      if ( !BP_Area_EU() )
      {
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
   		MENU_S_Color( 200, 200, 200, alpha1 );
      }
      else
      {
//#else
		   MENU_S_Color( 160, 160, 160, alpha2 );
      }
//#endif
		if ( type == IT_Card ){
			MENU_S_Printf( "Lv.%d\n", GM_ItemNum( type ) ) ;
		} else if ( type == IT_DogTag ){
			MENU_S_Printf( "%d\n", GM_ItemNum( type ) ) ;
		} else if ( GM_ItemMaxNum( type ) != 1 && !( GM_ItemTypes[ type ] & IT_TYPE_CBBOX ) ) {
			MENU_S_Printf( "%d/%d\n", GM_ItemNum( type ), GM_ItemMaxNum( type ) ) ;
		}
	}
	/* レーション特別処理（メニュー取り付きフナムシ用） */
	if ( type == IT_Ration ){
		disp_ration_x = x + PW / 2 + 8 ;
		disp_ration_y = y + ( PH - TITLE_HEIGHT ) / 2 + 2 ;
		disp_ration_alpha = alpha1 ;
		disp_ration_flag = 1 ;
	}

   ResetMenuSafeZone();
}

/* パネルを武器用として表示する */
void MENU_PutWeaponPanel( int x, int y, int type, int alpha1, int alpha2, int flag )
{
	DG_TEX	*tex, *mark_tex ;
	int		col1, col2, col3, pos_x, pos_y ;

//	if ( GM_PlayerStatus & PLAYER_DEBUG ) return 
#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_DISPLAY_OFF ) return ;
#endif
	//tex_id = Weapon_Textures[ type ] ;
	tex = WeaponTexList[ type ] ;
	mark_tex = OtherTexList[ TEX_NO_USE ] ;		/* テクスチャ取得（通常使用禁止マーク） */
	if ( GM_WeaponNum( type ) == 0 ){
		flag |= PANEL_FLAG_NO_USE ;
		mark_tex = OtherTexList[ TEX_EMPTY ] ;		/* テクスチャ取得（通常使用禁止マーク） */
	}

   SetMenuSafeZone();

	if ( alpha1 < 0 ) alpha1 = 0 ;
	if ( alpha1 > 128 ) alpha1 = 128 ;
	if ( alpha2 < 0 ) alpha2 = 0 ;
	if ( alpha2 > 128 ) alpha2 = 128 ;
	switch ( flag & PANEL_FLAG_TYPEMASK ){
	  default:
	  case 0:	/* 通常表示時 */
		col1 = 0x00201810 | ( alpha1 << 24 );	/* ウィンドウカラー */
		col2 = 0x00808080 | ( alpha2 << 24 );	/* アイコン */
		col3 = 0x00000000 | ( (alpha2/2) << 24 );	/* タイトルカラー */
		break ;
	  case 1:	/* 選択中アクティブ時 */
		{
			int t = GV_Time & 63, t2 ;
			if ( t > 31 ) t = 64 - t ;
			t2 = t + 32 ;
			t = t * 2 + 32 ;
			//col1 = (t2) | (t2<<8) | (t<<16) | ( alpha1 << 24 );
			col1 = (42) | (53<<8) | (60<<16) | ( alpha1 << 24 ) ;
			col2 = 0x00808080 | ( alpha2 << 24 );	/* アイコン */
			col3 = 0x00000000 | ( (alpha2/2) << 24 );	/* タイトルカラー */
		}
		break ;
	  case 2:	/* 選択中非アクティブ時 */
		col1 = 0x00201810 | ( alpha1 << 24 );
		col2 = 0x00404040 | ( alpha2 << 24 );	/* アイコン */
		col3 = 0x00000000 | ( (alpha2/2) << 24 );	/* タイトルカラー */
		break ;
	}
	if ( ( flag & PANEL_FLAG_TYPEMASK ) != 0 ){
		int		back_col = -1 ;
		if ( !( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX ) ){
			if ( GM_Weapon == WP_None ){
				if ( type == WP_None )			back_col = ((alpha1)<<24)|MENU_CURRENT_COL ;
				if ( type == GM_WeaponPrev )	back_col = ((alpha1)<<24)|MENU_PREV_COL ;
			} else {
				if ( type == GM_Weapon )		back_col = ((alpha1)<<24)|MENU_CURRENT_COL ;
				if ( type == WP_None )			back_col = ((alpha1)<<24)|MENU_PREV_COL ;
			}
		} else {
			if ( type == GM_Weapon )			back_col = ((alpha1)<<24)|MENU_CURRENT_COL ;
			if ( type == GM_WeaponPrev )		back_col = ((alpha1)<<24)|MENU_PREV_COL ;
		}
		if ( back_col != -1 ){
			PutBackPanelPrim( x, y, ((alpha1)<<24)|back_col );
			col1 = back_col ;
		}
	}
	if ( ( flag & PANEL_FLAG_TYPEMASK ) == 1 ) PutBackPanelPrim( x, y, ((alpha1)<<24)|MENU_SELECT_COL );
	PutPanelPrim( x, y, col1, col2, col3, tex, mark_tex, flag, 0 );

	/* アイテム名表示 */
	if ( alpha2 != 0 ){
		pos_x = DG_FTOI( SCALE_X( x ) );
		pos_y = DG_FTOI( (float)( y + TXT_SHIFT_Y ) * ( (float)DRAW_HEIGHT / (float)VR_HEIGHT ) );
		MENU_S_Locate( pos_x + TXT_SHIFT_X, pos_y, 1 );
      if ( !BP_Area_EU() )
      {
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
   		MENU_S_Color( 200, 200, 200, alpha2 );
      }
      else
      {
//#else
		   MENU_S_Color( 160, 160, 160, alpha2 );
      }
//#endif
		if ( ( GM_WeaponTypes[ type ] & WP_TYPE_MAGAZINE ) && !GM_CheckPlayerStatus( PLAYER_MENU_OPEN ) ) 
      {
			//MENU_S_Printf( "%s %2d\n", Weapons[ type ], GM_Magazine ) ;
			{/* 弾倉装填数表示 */
				int	max_bullet, n_last_bullet = 0 ;
				switch ( type ){
				  default:
				  case WP_m92:
				  case WP_Usp:
					max_bullet = 15 ;
					break ;
				  case WP_Socom:
					max_bullet = 12 ;
					break ;
				  case WP_Famas:
					max_bullet = 25 ;
					n_last_bullet = 3 ;
					break ;
				  case WP_Psg1:
					max_bullet = 20 ;
					break ;
				  case WP_Psg1T:
					max_bullet= 5 ;
					break ;
				  case WP_Aks:
				  case WP_m4:
					max_bullet = 30 ;
					break ;
				  case WP_Rgb6:
					max_bullet= 6 ;
					break ;
				}
				PutBulletPrim( x, y, GM_Magazine, max_bullet, n_last_bullet, 0 );
			}
		} 
      else 
      {
			MENU_S_Printf( "%s\n", Weapons[ type ] ) ;
		}
	}
	/* 所持数表示 */
	if ( alpha1 != 0 ){
		if ( !( GM_WeaponTypes[ type ] & WP_TYPE_ONLY_ONE ) ) {
			pos_x = DG_FTOI( SCALE_X( x ) );
			pos_y = DG_FTOI( (float)( y + NUM_TXT_SHIFT_Y ) * ( (float)DRAW_HEIGHT / (float)VR_HEIGHT ) );
			MENU_S_Locate( pos_x + TXT_SHIFT_X, pos_y, 1 ) ;
         if ( !BP_Area_EU() )
         {
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
   			MENU_S_Color( 200, 200, 200, alpha1 );
         }
         else
         {
//#else
			   MENU_S_Color( 160, 160, 160, alpha2 );
         }
//#endif
			MENU_S_Printf( "%d/%d\n", GM_WeaponNum( type ), GM_WeaponMaxNum( type ) ) ;
		}
	}

   ResetMenuSafeZone();
}

/* ---------------------------------------------------------------- */
/* アイテムグループパネルを表示 */
void MENU_PutItemGroup( int x, int y, int type, int alpha, int col )
{
	//int		tex_id ;
	//int		col ;

	//tex_id = ItemGroup_Textures[ type ] ;
	if ( alpha < 0 ) alpha = 0 ;
	if ( alpha > 128 ) alpha = 128 ;
	//col = 0x00808080 | ( alpha << 24 );
	//col = ItemGroup_Colors[ type ] | ( alpha << 24 );
	col |= ( alpha << 24 );
	PutGroupPrim( x, y, col );
}

/* 武器グループパネルを表示 */
void MENU_PutWeaponGroup( int x, int y, int type, int alpha, int col )
{
	//int		tex_id ;
	//int		col ;

	//tex_id = WeaponGroup_Textures[ type ] ;
	if ( alpha < 0 ) alpha = 0 ;
	if ( alpha > 128 ) alpha = 128 ;
	//col = 0x00808080 | ( alpha << 24 );
	//col = WeaponGroup_Colors[ type ] | ( alpha << 24 );
	col |= ( alpha << 24 );
	PutGroupPrim( x, y, col );
}

/* ---------------------------------------------------------------- */
/* フナムシパネルを表示する */
void MENU_PutShipwormPanel( int x, int y )
{
	PutShipwormPanelPrim( x, y, 0x80808080 );
}
/* ---------------------------------------------------------------- */
static void Font2Texture( DG_TEX_LIN *tex, char *font_vram, unsigned short *pal )
{
#if 0
	/* 16bit(A4R4G4B4) */
	unsigned short	*dst ;
	unsigned char	*src ;
	int				i, j, index ;
	dst = tex->image ;
	src = font_vram ;
	for ( j = 0 ; j < tex->height ; j++ ){
		for ( i = 0 ; i < tex->width ; i += 2 ){
			index = *src & 0x0f ;
			*dst++ = pal[ index ];
			index = ( *src >> 4 ) & 0x0f ;
			*dst++ = pal[ index ];
			src++ ;
		}
	}
#else
	/* ８ビットアルファ */
	unsigned char	*dst ;
	unsigned char	*src ;
	static unsigned char	pal2alpha[4] = {0,255*1/3,255*2/3,255};
	int				i, j, index ;
	dst = tex->image ;
	src = font_vram ;
	for ( j = 0 ; j < tex->height ; j++ ){
		for ( i = 0 ; i < EXPLAIN_VRAM_WIDTH ; i += 2 ){
			index = *src & 0x0f ;
			*dst++ = pal2alpha[ index ];
			index = ( *src >> 4 ) & 0x0f ;
			*dst++ = pal2alpha[ index ];
			src++ ;
		}
		dst += EXPLAIN_VRAM_MALLOC_WIDTH - EXPLAIN_VRAM_WIDTH ;
	}
#endif

#if 1 //BP def KP_WINDOWS
	DG_LinerTextureSetImageDirty(tex) ;	// image変更内容を適用させる
#endif

}
/* 説明分テキストを展開する */
void MENU_CreateExplainText( char *message )
{
   if ( work_ptr == NULL ) return ;
   /* 重複変更チェック */
   if ( work_ptr->last_message == message ) return ;
   work_ptr->last_message = message ;

   {
#if BP_USE_NEW_FONT_SYSTEM()
      BP_FONT_DRAWINFO dr;

      BP_font_begin_render_texture(&work_ptr->bp_vinfo, 1);
      
      BP_font_open_drawinfo(&dr, &work_ptr->bp_vinfo);
      
      BP_font_set_color(&dr, 255, 255, 255, 128);
      BP_font_set_refnum( GM_ItemNum( IT_Card ) );

#ifdef ENGLISH
      if ( !BP_Area_EU() )
      {
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
         dr.xtop = 0 ;
         dr.ytop = 0 ;
      }
      else
      {
//#else
         dr.xtop = BP_FONT_CONVERT_ORIGINAL_X(1);
         dr.ytop = 0 ;
      }
//#endif
#else
      dr.xtop = BP_FONT_CONVERT_ORIGINAL_X(12);
      dr.ytop = 0 ;
#endif

      BP_font_draw_string(&dr, message);

      BP_font_end_render_texture(&work_ptr->bp_vinfo);

#else
      FONT_DRAWINFO dr;

      GV_ZeroMemory( work_ptr->vram, work_ptr->vram_size );
      font_open_drawinfo( &dr, &work_ptr->vinfo );

#ifdef ENGLISH
#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
      dr.xtop = 0 ;
      dr.ytop = 0 ;
#else
      dr.xtop = 1 ;
      dr.ytop = 0 ;
#endif
#else
      dr.xtop = 12 ;
      dr.ytop = 0 ;
#endif

      font_set_refnum( GM_ItemNum( IT_Card ) );
      font_draw_string( &dr, message );
      {/* XBOX用テクスチャ用に展開 */
         unsigned short	pal[4] = {
            0x0ccc, 0x5ccc, 0xaccc, 0xfccc
         };
         Font2Texture( work_ptr->tex, work_ptr->vram, pal );
      }
#endif
   }

}

/* 説明文を表示する */
void MENU_PutExplainText( int x, int y, int type )
{
	PRIM_CONTROL	*prim_ctrl ;
	PRIM_EXPLAIN	*prim ;
	float			x0, x1, y0, y1, u0, v0, u1, v1 ;
	u_int			col ;

	if ( work_ptr == NULL ) return ;

	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->explain_ctrl ;

   SetViewMapping(prim_ctrl);
   SetMenuSafeZone();

	/* 背景ウィンドウプリミティブ設定 */
	MENU_SetExplainWindowPacket( NULL, x, y );

	/* 説明文描画プリミティブ設定 */
#if BP_USE_NEW_FONT_SYSTEM()
   prim_ctrl->current_buffer = DG_SetDmapackTextureDynamic01( prim_ctrl->current_buffer, NULL, work_ptr->bp_tex );

   u0 = ( 0.5f ) / BP_GetDynamicTextureWidth(work_ptr->bp_tex);
   v0 = ( 0.5f ) / BP_GetDynamicTextureHeight(work_ptr->bp_tex);
   u1 = ( BP_FONT_CONVERT_ORIGINAL_X(EXPLAIN_VRAM_WIDTH) + 0.5f ) / BP_GetDynamicTextureWidth(work_ptr->bp_tex);
   v1 = ( BP_FONT_CONVERT_ORIGINAL_Y(EXPLAIN_VRAM_HEIGHT) + 0.5f ) / BP_GetDynamicTextureHeight(work_ptr->bp_tex);

#else
	prim_ctrl->current_buffer = DG_SetDmapackTexLin( prim_ctrl->current_buffer, work_ptr->tex );

   u0 = ( 0.5f ) / work_ptr->tex->width ;
   v0 = ( 0.5f ) / work_ptr->tex->height ;
   u1 = ( EXPLAIN_VRAM_WIDTH - 0.5f ) / work_ptr->tex->width ;
   v1 = ( EXPLAIN_VRAM_HEIGHT - 0.5f ) / work_ptr->tex->height ;


#endif

	prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	/* 座標設定 */
#ifdef ENGLISH
   if ( !BP_Area_EU() )
   {
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
	   x0 = SCALE_X( x + 6 );
	   y0 = SCALE_Y( y + EXP_WIN_EDGE_Y - 4 );
	   x1 = SCALE_X( x + EXP_WIN_W - 12 );
	   y1 = SCALE_Y( y + EXP_WIN_H + EXP_WIN_EDGE_Y - 4 );
   }
   else
   {
//#else
	   x0 = SCALE_X( x + 12 );
	   y0 = SCALE_Y( y + EXP_WIN_EDGE_Y - 4 );
	   x1 = SCALE_X( x + EXP_WIN_W - 24 );
	   y1 = SCALE_Y( y + EXP_WIN_H + EXP_WIN_EDGE_Y - 4 );
   }
//#endif
#else
	x0 = SCALE_X( x );
	y0 = SCALE_Y( y + EXP_WIN_EDGE_Y - 4 );
	x1 = SCALE_X( x + EXP_WIN_W );
	y1 = SCALE_Y( y + EXP_WIN_H + EXP_WIN_EDGE_Y - 4 );
#endif

   prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
												   x0, y0, u0, v0,
												   x1, y1, u1, v1, DG_MakeDmaPackColorFromInt(0x60606060) );

	ClosePrimControl( prim_ctrl );

	{/* 種別タグの表示 */
		static char		*ExplainTagName[] = {"EQUIP","WEAPON"} ;
		int		pos_x, pos_y ;

		pos_x = ( x + EXP_WIN_EDGE_X ) * DRAW_WIDTH / VR_WIDTH ;
		pos_y = ( y + EXP_WIN_H ) * DRAW_HEIGHT / VR_HEIGHT ;
		MENU_SetAlphaMode( 0, 1, 0, 1, 0 );
		MENU_Locate( pos_x , pos_y, 0 );
		MENU_Color( 200, 200, 200, 128 );
		MENU_Printf( "%s\n", ExplainTagName[ type ] ) ;
	}

   ResetMenuSafeZone();

	/* メニューライフゲージ表示 */
	GM_VisibleMenuLife() ;
}


void MENU_PutBoxPrim(int x, int y, int w, int h, int alpha, int col)
{
   if (work_ptr != NULL)
   {
      /* プリミティブを１枚分確保 */
      PRIM_CONTROL *prim_ctrl = &work_ptr->panel_ctrl;

      float			x0, x1, y0, y1 ;

      MENU_SetAlphaMode( 0, 1, 0, 1, 0 );

      col |= alpha << 24;

      /* 背景ウィンドウスプライトの設定 */
      x0 = SCALE_X( x );
      y0 = SCALE_Y( y );
      x1 = SCALE_X( x + w );
      y1 = SCALE_Y( y + h );
      prim_ctrl->current_buffer = DG_SetDmapackBox( prim_ctrl->current_buffer,
         x0, y0,
         x1, y1,
         DG_MakeDmaPackColorFromInt(col) );

      ClosePrimControl( prim_ctrl );
   }
}

void MENU_PutTextPrim(int x, int y, char *text, int r, int g, int b, int a, int flag)
{
   if (work_ptr != NULL)
   {
      MENU_SetAlphaMode( 0, 1, 0, 1, 0 );
      /* アイテム名表示 */
      MENU_S_Locate( x*DRAW_WIDTH/VR_WIDTH, y*DRAW_HEIGHT/VR_HEIGHT, flag );
      MENU_S_Color( r, g, b, a );
      MENU_S_Printf("%s\n", text);
      ClosePrimControl(&work_ptr->panel_ctrl);
   }
}

void MENU_ForceClose()
{
   extern void CommonChangeForceCloseMenu(struct SCommonChangeWork* work);

#if defined(BP_VITA)
   if (!work_ptr)
      return;

   if (work_ptr->item_change_ptr)
   {
      CommonChangeForceCloseMenu(work_ptr->item_change_ptr);
   }

   if (work_ptr->weapon_change_ptr)
   {
      CommonChangeForceCloseMenu(work_ptr->weapon_change_ptr);
   }
#endif
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	/* 現在選択中のメニュータイプ起動チェック */
	if ( work->current_menu_type == 0 ){
		if ( !( GM_Configuration & GM_CONFIG_OLD_TYPE_MENU ) ){
			if ( work->item_change_ptr != NULL ){
				GV_DestroyActor( work->item_change_ptr );
				work->item_change_ptr = NULL ;
			}
			if ( work->weapon_change_ptr != NULL ){
				GV_DestroyActor( work->weapon_change_ptr );
				work->weapon_change_ptr = NULL ;
			}

#if defined(BP_VITA)
			work->item_change_ptr = NewItemChangeVita( 0 );
			work->weapon_change_ptr = NewWeaponChangeVita( 0 );
#else
         work->item_change_ptr = NewItemChange2( 0 );
         work->weapon_change_ptr = NewWeaponChange2( 0 );
#endif
			work->current_menu_type = 1 ;
			printf("%s:change menu type to new\n", __FILE__);
		}
	} else {
		if ( GM_Configuration & GM_CONFIG_OLD_TYPE_MENU ){
			if ( work->item_change_ptr != NULL ){
				GV_DestroyActor( work->item_change_ptr );
				work->item_change_ptr = NULL ;
			}
			if ( work->weapon_change_ptr != NULL ){
				GV_DestroyActor( work->weapon_change_ptr );
				work->weapon_change_ptr = NULL ;
			}

#if defined(BP_VITA)
			work->item_change_ptr = NewItemChangeVita( 0 );
			work->weapon_change_ptr = NewWeaponChangeVita( 0 );
#else
         work->item_change_ptr = NewItemChange( 0 );
         work->weapon_change_ptr = NewWeaponChange( 0 );
#endif
			work->current_menu_type = 0 ;
			printf("%s:change menu type to old\n", __FILE__);
		}
	}

	/* メニューライフゲージＯＦＦ */
	GM_InvisibleMenuLife() ;
	/* 書き込み位置の初期化のみ行う */
	InitPrimControl( &work->panel_ctrl );
	InitPrimControl( &work->group_ctrl );
	InitPrimControl( &work->explain_ctrl );
	/* レーション表示位置関連データ初期化 */
	disp_ration_flag = 0 ;
	/* 武器テクスチャ変更チェック */
	if ( GM_PlayerStateFlag & PL_SOCOM_SPPRSR_ATTACHED ){
		WeaponTexList[ WP_Socom ] = WeaponTexList[ MAX_WEAPONS + 0 ] ;
	}
	if ( GM_PlayerStateFlag & PL_AK_SPPRSR_ATTACHED ){
		WeaponTexList[ WP_Aks ] = WeaponTexList[ MAX_WEAPONS + 1 ] ;
	}
	if ( GM_PlayerStateFlag & PL_GBSCAP_EXIST ){
		ItemTexList[ IT_Uniform ] = ItemTexList[ MAX_ITEMS + 0 ] ;
	}
	if ( GM_PlayerStateFlag & PL_USP_SPPRSR_ATTACHED ){
		WeaponTexList[ WP_Usp ] = WeaponTexList[ MAX_WEAPONS + 2 ] ;
	}
	/* ブレードモード変更 */
	if ( !( GM_PlayerStateFlag & PL_BLADE_MODE_MINEUCHI ) ){
		/* 殺傷可能時 */
		WeaponTexList[ WP_Blade ] = WeaponTexList[ MAX_WEAPONS + 3 ];
	} else {
		/* 非殺傷モード時 */
		WeaponTexList[ WP_Blade ] = WeaponTexList[ MAX_WEAPONS + 4 ];
	}
#ifdef COLOR_DEBUG
	{
		static int	data[5][3] ;
		if ( GV_PadData[0].press & PAD_X ) printf("%s: color debug addr = %08x\n", __FILE__, data );
		MENU_select_col =		MENU_RGB( data[0][0], data[0][1], data[0][2] );
		MENU_current_col =		MENU_RGB( data[1][0], data[1][1], data[1][2] );
		MENU_prev_col =			MENU_RGB( data[2][0], data[2][1], data[2][2] );
		MENU_grp_current_col =	MENU_RGB( data[3][0], data[3][1], data[3][2] );
		MENU_grp_prev_col =		MENU_RGB( data[4][0], data[4][1], data[4][2] );
	}
#endif
}
#ifdef COLOR_DEBUG
int	MENU_select_col ;
int	MENU_current_col ;
int	MENU_prev_col ;
int	MENU_grp_current_col ;
int	MENU_grp_prev_col ;
#endif

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	FreePrimControl( &work->explain_ctrl );
	FreePrimControl( &work->group_ctrl );
	FreePrimControl( &work->panel_ctrl );
#if BP_USE_NEW_FONT_SYSTEM()
   BP_FreeDynamicTexture(work->bp_tex);
#else
	GV_DelayedFree( work->vram );
	DG_FreeLinerTexture( work->tex );
#endif
	work_ptr = NULL ;
	MENU_StatusFlag = 0 ;	/* 一応フラグを初期化しておく */
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	int		i ;
	/* プリミティブメモリ確保 */
	MakePrimControl( &work->panel_ctrl,
					SIZEOF_QWORD(PRIM_PANEL) * ( 5 + 1 ) * ( 5 + 1 ) + 5,
					239, MENU_TRI );
	MakePrimControl( &work->group_ctrl,
					SIZEOF_QWORD(PRIM_GROUP) * ( 5 + 1 ) + 5,
					251, MENU_TRI );
#if 0
	MakePrimControl( &work->explain_ctrl,
					128,
					240, MENU_TRI );
#else
	MakePrimControl( &work->explain_ctrl,
					128,
					251, MENU_TRI );
#endif
	/* テクスチャリストの生成 */
	for ( i = 0 ; i < MAX_ITEMS+1 ; i++ ){
		if ( Item_Textures[ i ] != 0 ){
			ItemTexList[ i ] = DG_GetTexture2( MENU_TRI, Item_Textures[ i ] );
		}
	}
	for ( i = 0 ; i < MAX_WEAPONS+5 ; i++ ){
		if ( Weapon_Textures[ i ] != 0 ){
			WeaponTexList[ i ] = DG_GetTexture2( MENU_TRI, Weapon_Textures[ i ] );
		}
	}
	for ( i = 0 ; i < MAX_OTHER_TEX_LIST ; i++ ){
		if ( Other_Textures[ i ] != 0 ){
			OtherTexList[ i ] = DG_GetTexture2( MENU_TRI, Other_Textures[ i ] );
		}
	}

#if BP_USE_NEW_FONT_SYSTEM()
   work->bp_tex = BP_AllocDynamicTexture(BP_FONT_CONVERT_ORIGINAL_X(EXPLAIN_VRAM_MALLOC_WIDTH), BP_FONT_CONVERT_ORIGINAL_Y(EXPLAIN_VRAM_HEIGHT), 1);
   BP_font_set_vraminfo_texture(&work->bp_vinfo, work->bp_tex, 0, BP_FONT_CONVERT_ORIGINAL_Y(12), FONT_NO_KINSOKU);

#else
	/* フォント展開テクスチャ確保 */
	work->tex = DG_MakeLinerTexture( EXPLAIN_VRAM_MALLOC_WIDTH, EXPLAIN_VRAM_HEIGHT, DG_TEXLIN_FORMAT_A8 );
	/* フォント展開領域確保 */
	work->vram_size = EXPLAIN_VRAM_MALLOC_WIDTH * EXPLAIN_VRAM_HEIGHT / 2 ;
	work->vram = GV_Malloc( work->vram_size );
	//work->vram_size = EXPLAIN_VRAM_MALLOC_WIDTH * EXPLAIN_VRAM_HEIGHT * 2 ;
	//work->vram = work->tex->image ;
	GV_ZeroMemory( work->vram, work->vram_size );
	/* フォント展開領域設定 */
	font_set_vraminfo( &work->vinfo, work->vram, EXPLAIN_VRAM_WIDTH, EXPLAIN_VRAM_HEIGHT, 0, 12, FONT_NO_KINSOKU );
#endif

	/* メニューの起動処理 */
	if ( GM_Configuration & GM_CONFIG_OLD_TYPE_MENU ){

#if defined(BP_VITA)
      MENU_Item_Panel_ResetAnim(0);
		work->item_change_ptr = NewItemChangeVita( 0 );
		work->weapon_change_ptr = NewWeaponChangeVita( 0 );
#else
      work->item_change_ptr = NewItemChange( 0 );
      work->weapon_change_ptr = NewWeaponChange( 0 );
#endif
		work->current_menu_type = 0 ;
	} else {

#if defined(BP_VITA)
      MENU_Item_Panel_ResetAnim(0);
		work->item_change_ptr = NewItemChangeVita( 0 );
		work->weapon_change_ptr = NewWeaponChangeVita( 0 );
#else
      work->item_change_ptr = NewItemChange2( 0 );
      work->weapon_change_ptr = NewWeaponChange2( 0 );
#endif
		work->current_menu_type = 1 ;
	}

	/* 変数初期化 */

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		アイテムメニュー用プリミティブ管理デーモン
	*/
void *NewMenuPrimControl( void )
{
	Work			*work ;

	if ( work_ptr != NULL ) return (NULL);

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	work_ptr = work ;
	return work ;
}

#if defined(BP_VITA)

/*------------------------------------------------------------------*/
extern void UpdateWeaponPanelPropsAfterListFlip(void* work);
extern void UpdateItemPanelPropsAfterListFlip(void* work);

/*------------------------------------------------------------------*/
int BP_DebugMenu_Callback_UpdateWeaponItemPanels(int data)
{
   UpdateWeaponPanelPropsAfterListFlip(work_ptr->weapon_change_ptr);
   UpdateItemPanelPropsAfterListFlip(work_ptr->item_change_ptr);
   return 0;
}

#endif