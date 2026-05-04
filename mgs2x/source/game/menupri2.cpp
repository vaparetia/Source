/*
	menupri.c

	メニュー系プリミティブ表示管理デーモン

	1999/08/18 K.Uehara ( original K.Takabe )
	2000/09/13 K.Takabe : new font support

	2001/05/01 M.Kobayashi X のため全面書き換え
	2001/05/09 M.Kobayashi DG_SPRT で書く
	
	$Id: menupri2.cpp,v 1.1.1.3 2002/11/19 11:41:53 Yoshizawa1 Exp $
*/

#include	"gameheader.h"
#include	"../system/libdg/dgconf.h"

#define FTOI12(_f) (DG_FTOI((_f) * 4096.0f))

#define	MAX_STRING	512

const	int	FONT_KIND	= 3;

const	float	fDist = 1000.f;

#define N_PRIMS	(MAX_MENU_PRIMS)
#define N_PACKETS	(1)


#define TRI_CODE		(3678773)	/* "menu" */
#define TEXTURE0		(6852885)	/* "font_alp_ovl" */
#define TEXTURE1		(13364753)	/* "font_mini_alp_ovl" */
#define TEXTURE2		(9005379)	/* "debug_font_alp_ovl" */
#define ICON_TEXTURE0	(11717998)	/* "cd_err_alp_ovl" */

/* ------------------------------------------------------------ */
/*
	プリミティブ管理デーモン
*/

int GM_DebugPrint_Off = 0;	// これ以降DEBUGPRINTをOFFにする。

/* ---------------------------------------------------------------------- */

/* フォントテクスチャ情報記録用構造体 */
typedef struct {
	float	fTexWidth;		/* テクスチャサイズ */
	float	fTexHeight;		/* テクスチャサイズ */
	int		width ;			/* フォントの幅 */
	int		height ;		/* フォントの高さ */
	int		x_space;		// 文字間
	int		y_space;		// 文字間
	DG_TEX*		pTex;		// テクスチャ
	UINT32		u32TexCode;	// テクスチャコード
	int		x, y;			// カレント位置
	int		xtop;			//
	u_char	r,g,b,a;		// 色
	u_char	u8Flag;			// 文字位置フラグ
	char*	pPitch;			// ピッチテーブルへ
} FONT_INFO ;

/* アイコンテクスチャ情報記録用構造体 */
typedef struct {
	int		tex_width ;		/* テクスチャ幅 */
	int		tex_height ;	/* テクスチャの高さ */
	int		offset_x ;		/* 配置オフセット */
	int		offset_y ;		/* 配置オフセット */
	int		x_num ;			/* アイコン分割数 */
	int		y_num ;			/* アイコン分割数 */
} ICON_INFO ;

/* MENU_Printf処理実行アクター用ワーク */
typedef	ALIGN16_DECL(struct)	{
	FONT_INFO		font_info[FONT_KIND] ;
	ICON_INFO		icon_info[1] ;
	
} WORK  ;

/* ワークの実体 */
static WORK menuprimwork;		/* 常駐させるためワークはスタティックに確保 */

/* 各フォントデータのピッチデータ */
static char	font_pitch[][96] = {
	{
		/*  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  */
		6, 4,12,12,12,12,12, 4, 5, 5,11,11, 4,11, 4,12,11,11,11,11,11,11,11,11,11,11, 4, 4, 7,11, 7,12,
		/*  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [ \\  ]  ^  _  */
		12,12,12,12,12,12,12,12,12, 4,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12, 5,10, 5, 6,12,
		/*  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~     */
		11,11,11,11,11,11,11,11,11, 4,11,11, 6,12,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,
	},
	{
		/*  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  */
		9, 4, 5,10,10,10,10, 2, 4, 4,10,10, 3,10, 3, 6, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 3, 3, 6,10, 6,10,
		/*  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [ \\  ]  ^  _  */
		10,10,10,10,10,10,10,10,10, 3,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10, 4,10, 4, 4,10,
		/*  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~     */
		10,10,10,10,10,10,10,10,10, 3,10,10, 3,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	},
	{
		/*  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  */
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		/*  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z  [ \\  ]  ^  _  */
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		/*  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~     */
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	}
};


#ifdef __cplusplus
extern "C" {
#endif

char	*font_pitch0 = font_pitch[0] ;
char	*font_pitch1 = font_pitch[1] ;
char	*font_pitch2 = font_pitch[2] ;


#ifdef __cplusplus
}
#endif


/* ---------------------------------------------------------------------- */
/*
	Initialize
*/

static int GetResources( WORK* pw )
{
	GM_ResetMenuPrimManager();
	return 0;
}



/* ---------------------------------------------------------------------- */
/*
	スタートアップ
*/
void *GM_StartMenuPrimManager( void )
{
	WORK* pw;
	
	pw = &menuprimwork;
	GV_ZeroMemory( pw, sizeof( menuprimwork ) );

	if ( GetResources( pw ) < 0 ) {
		HANGUP();
	}
	return (void *)pw ;	// ACTOR ではなくなったのでほんとはあぶない
}


void GM_ResetMenuPrimManager( void )
{	// Xbox追加 テクスチャを設定しなおす
	WORK* pw = &menuprimwork;

	FONT_INFO* pi;

	/* 標準フォント */
	pi = &pw->font_info[0];
	pi->u32TexCode = TEXTURE0;
	pi->pTex = DG_GetTexture(TEXTURE0);
	pi->width = 12;
	pi->height = 16;
	pi->x_space = 1;
	pi->y_space = 1;
	pi->pPitch = font_pitch[0];
	pi->fTexWidth = 384.f;
	pi->fTexHeight = 48.f;

	/* ミニフォント */
	pi = &pw->font_info[1];
	pi->u32TexCode = TEXTURE1;
	pi->pTex = DG_GetTexture(TEXTURE1);
	pi->width = 10;
	pi->height = 14;
	pi->x_space = 1;
	pi->y_space = 1;
	pi->pPitch = font_pitch[1];
	pi->fTexWidth = 320.f;
	pi->fTexHeight = 42.f;
	
	/* デバッグ用フォント */
	pi = &pw->font_info[2];
	pi->u32TexCode = TEXTURE2;
	pi->pTex = DG_GetTexture(TEXTURE2);
	pi->width = 8;
	pi->height = 16;
	pi->x_space = 0;
	pi->y_space = 1;
	pi->pPitch = font_pitch[2];
	pi->fTexWidth = 256.f;
	pi->fTexHeight = 48.f;

	/* アイコン関連の設定 */

	// その他イニシャライズ
	int i;
	for( pi = &pw->font_info[0], i = 0 ; i < FONT_KIND; i++, pi++ ){
		pi->x = pi->y = 0;
		__MENU_ResetColor( i );
	}
}



/* ---------------------------------------------------------------------- */
/*
	外部関数からの呼出しインターフェース
*/

void *MENU_OpenPrim( void )
{
	return NULL;
}

void MENU_ClosePrim( void )
{
}

/* ------------------------------------------------------------ */
/*
	画面へのprintf
*/

void __MENU_Color( int which, int r, int g, int b, int a)
{
	FONT_INFO* pi = &menuprimwork.font_info[ which ];

	pi->r = r ;
	pi->g = g ;
	pi->b = b ;
	pi->a = a;
}

void __MENU_ResetColor( int which )
{
	__MENU_Color( which, 255, 255, 255, 255 );
}

void __MENU_Locate( int which, int x, int y, int flag = MENU_MODE_NORMAL)
{
	FONT_INFO *pi = &menuprimwork.font_info[ which ];

	pi->xtop = pi->x = x ;
	pi->y = y ;
	pi->u8Flag = (u_char)flag;
}

void __MENU_SetAlphaMode( long64 alpha )
{
}


static int trim( DG_SPRT* psprtTop, DG_SPRT* psprtEnd, int xw, u_char u8Flag )
{
	int ret;
	int shift;

	ret = 0;
	if( psprtTop != NULL && psprtTop < psprtEnd && xw > 0 ){
		switch( u8Flag & 3 ){
		case MENU_MODE_LEFT:
		default:
			return 0;
		case MENU_MODE_CENTER:
			shift = - ( xw / 2 );
			ret = -xw;
			break;
		case MENU_MODE_RIGHT:
			shift = -xw;
			ret = -xw * 2;
			break;
		}
		for( ; psprtTop < psprtEnd; psprtTop ++ ){
			psprtTop->x += (float)shift;
		}
	}
	return ret;
}

void __MENU_Print( int which, char* pStr )
{
	int		x, y, w, h, u, v;
	int		nStr;
	int wsum;
	FONT_INFO*	pi;
	DG_SPRT sprt[MAX_STRING];


#ifdef DEBUG_MODE
	if( which == 2 && ( ! GM_DebugModeEnable || GM_DebugPrint_Off ) ){
		return;
	}
#endif
	/* モード、フラグによるフォント設定 */
	pi = &menuprimwork.font_info[ which ] ;
	if( pi->pTex == NULL ) {
		GM_ResetMenuPrimManager();
		return;
	}

	x = pi->x; y = pi->y;
	w = 0 ;
	h = pi->height ;
	wsum = 0;
	nStr = 0;

	DG_SPRT* pSprt;
	DG_SPRT* pSprtTop;
	pSprt = pSprtTop = sprt;
	
	for(; *pStr != '\0'; pStr++ ){
		int c = *pStr;
		if ( c == '\n' ) {
			if( pi->u8Flag != MENU_MODE_LEFT && wsum > 0 ){
				trim( pSprtTop, pSprt, wsum, pi->u8Flag );
			}
			pSprtTop = pSprt;
			wsum = 0;
			pi->y = y += h;
			x = pi->xtop;
			continue;
		}
		if ( c > ' ' && c < ( ' ' + 96 ) ){
			c -= ' ';
			w = pi->pPitch[ c ];
			u = ( ( c & 31 ) * pi->width );
			v = ( ( c / 32 ) * pi->height );
		} else{
			x += pi->pPitch[ 0 ] + pi->x_space;
			wsum += pi->pPitch[ 0 ] + pi->x_space;
			continue;
		}

		pSprt->x = (float)x - 0.5f;	// 0.5f は座標補正
		pSprt->y = (float)y - 0.5f;	// 0.5f は座標補正
		pSprt->w = (float)w;
		pSprt->h = (float)pi->height;
		pSprt->z = 1.f;

		pSprt->u0 =  u / pi->fTexWidth * pi->pTex->u_scale + pi->pTex->u_offset;
		pSprt->v0 =  v / pi->fTexHeight * pi->pTex->v_scale + pi->pTex->v_offset;
		pSprt->u1 = (u + w) / pi->fTexWidth * pi->pTex->u_scale + pi->pTex->u_offset;
		pSprt->v1 = (v + pi->height) / pi->fTexHeight * pi->pTex->v_scale + pi->pTex->v_offset;

///		pSprt->rgba = D3DCOLOR_RGBA( pi->r, pi->g, pi->b,  pi->a ); 
		pSprt->rgba = D3DCOLOR_RGBA( pi->r, pi->g, pi->b,  0x80 ); 

		pSprt++;
		x += w + pi->x_space;
		wsum += w + pi->x_space;
		nStr++;
		if( nStr >= MAX_STRING ) {
			printf(__FILE__ ": too many strings\n");
			return;
		}
	}

	if( pi->u8Flag != MENU_MODE_LEFT && wsum > 0 ){
		trim( pSprtTop, pSprt, wsum, pi->u8Flag );
		pi->y = y + h + pi->y_space ;
	} else {
		pi->x = x ;
	}
	DG_Set2DSprtTex(pi->u32TexCode);
	DG_Draw2DSprt(sprt, nStr);
}

void __MENU_Printf( int which, char *fmt, ... )
{
	va_list			args ;
	char			buffer[ 256 ];

	va_start( args, fmt );
	_vsnprintf( buffer, 256, fmt, args );

	__MENU_Print( which, buffer );

	va_end( args );
}

// 可変マクロが使えなくてめんどうなので入り口を作ってしまう
void MENU_Printf( char *fmt, ... ){
	va_list			args ;
	char			buffer[ 256 ];

	va_start( args, fmt );
	_vsnprintf( buffer, 256, fmt, args );

	__MENU_Print( 0, buffer );

	va_end( args );
}

void MENU_S_Printf( char *fmt, ... ){
	va_list			args ;
	char			buffer[ 256 ];

	va_start( args, fmt );
	_vsnprintf( buffer, 256, fmt, args );

	__MENU_Print( 1, buffer );

	va_end( args );
}

void DEBUG_Printf( char *fmt, ... ){
	va_list			args ;
	char			buffer[ 256 ];

	va_start( args, fmt );
	_vsnprintf( buffer, 256, fmt, args );

	__MENU_Print( 2, buffer );

	va_end( args );
}




#if 0
/* ---------------------------------------------------------------- */
	/*
		文字と同じ感覚でアイコンを表示する
	*/
void MENU_PutIcon( int type, int x, int y, int ptn, int col, int flag )
{
	MENU_PRIM *prim;
	MENU_SPRT_INIT *init;
	sceGifTag *gstag;
	MENU_SPRT *sprt ;
	ICON_INFO	*icon_info ;
	int			w, h, u, v ;

	/* モード、フラグによるフォント設定 */
	icon_info = &menuprimwork.icon_info[ 0 ] ;

	/* パケットを取得 */
	/* 初期化パケット */
	prim = MENU_OpenPrim();

	MENU_NEW_TAG( gstag, prim );
	MENU_NEW_PRIM( init, prim );

	/* 初期化パケット生成 */
	DG_SET_GIFTAG( gstag, .FLG = SCE_GIF_PACKED, .NREG = 1, .NLOOP = 4
				   , .REGS0 = GS_REGS_AD );
	init->tex2 = icon_info->tex2 ;
	init->tex0 = icon_info->tex0 ;
	init->prim = icon_info->prim ;
	init->rgba.reg = SCE_GS_RGBAQ ;
	init->rgba.data = col ;

	/* 頂点、ＵＶの設定パケットタグ */
	MENU_NEW_TAG( gstag, prim );
	DG_SET_GIFTAG( gstag, .FLG = SCE_GIF_REGLIST, .NREG = 4, .NLOOP = 1
				   , .REGS0 = GS_REGS_UV
				   , .REGS1 = GS_REGS_XYZ2
				   , .REGS2 = GS_REGS_UV
				   , .REGS3 = GS_REGS_XYZ2 );

	MENU_NEW_PRIM( sprt, prim );

	x = DG_POS_X( x );
	y = DG_POS_X( y );
	w = ( icon_info->tex_width / icon_info->x_num ) << 4 ;
	h = ( icon_info->tex_height / icon_info->y_num ) << 4 ;
	u = icon_info->offset_x * 16 + w * ( ptn % icon_info->x_num ) + 8 ;
	v = icon_info->offset_y * 16 + h * ( ptn / icon_info->x_num ) + 8 ;
	DG_SET_XY1( sprt, x, y );
	DG_SET_XY2( sprt, x + w, y + h );
	DG_SET_UV1( sprt, u, v );
	DG_SET_UV2( sprt, u + w, v + h );

	MENU_ClosePrim();
}

/* ---------------------------------------------------------------- */
	/*
		メニュー文字プリミティブ内に任意のサイズのＧＩＦパケット領域を確保する
	*/
void* MENU_PutPacket( int size )
{
	MENU_PRIM	*prim;
	void		*packet_addr ;

	prim = MENU_OpenPrim();

	packet_addr = MENU_NEXT_PRIM( prim, size );

	MENU_ClosePrim();

	return ( packet_addr );
}
#endif
