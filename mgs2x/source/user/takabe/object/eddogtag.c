//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	eddogtag.c
	エンディングデモ用ユーザードックタグテクスチャ生成

	2001/07/24 K.Takabe
	$Id: eddogtag.c,v 1.3 2002/12/25 14:43:44 takaki Exp $

*/
/*

chara ユーザードックタグ[NewUserDogtagSet] $s:name \
	-tri_id $s:ＴＲＩファイルＩＤ \
	-debug \
	-flag $w:フラグ
// このキャラを起動することでそのステージにおいてエンディング用ドックタグの
// バンプマップテクスチャをユーザー情報に書き換えます。このキャラ自体は
// モデルを表示する機能を持っているわけではないのでデモ上で普通にモデルを
// 出すことで効果を確認することが出来ます。
// -tri_idオプションはステージ構築時にドックタグ用テクスチャを別のＴＲＩと
// 一緒にパックした場合に使用します。デフォルトは"dogtag_mh_mt"です。
// -debug オプションは強制的にユーザー情報を設定します。デモの確認時などに
// 使用してください。但し、このオプションを残したままゲームに組み込むのは
// ＜絶対＞にやらないようお願いします。
// フラグは現在未使用

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"

#include "BP_Renderer.h"

/* ---------------------------------------------------------------- */
/* 特製フォントデータ（スタティックデータ約１０ＫＢ弱ぐらい） */
#include "dogfont.h"
/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/


/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/
#define TRI_ID			(6086754)		/* "dogtag_mh_mt" */
#define TEXTURE_ID		(9049700)		/* "dgtg_bmap" */
#define TEX_DEPTH		(4)


#define FONT_WIDTH		(8)
#define FONT_HEIGHT		(8)
#define FONT_SPACE		(1)
#define FONT_PITCH		(2)

//BP - switch to remove / modify elements in dogtag to avoid TCR 011 (BAS Personal Information) issues
#define BP_DOGTAG_TCR_011

/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct eddogtag_Work
{
	GV_ACT_EX	actor ;

	int			name ;
	int			map ;

	int			tri_id ;
	int			tex_id ;
	DG_TEX					*tex ;
	int						width ;
	int						height ;

	char        *texel_image ;
	int			*clut_image ;

	int						x, y ;

} Work ;

/* ---------------------------------------------------------------- */

static char	*blood_list[5] =
{
   "?",
   "A",
   "B",
   "AB",
   "O"
};

static char *sex_list[2] =
{
   "MALE",
   "FEMALE"
};

static char *nationality_list[] = 
{
	"Afghanistan" , 
	"Albania" , 
	"Algeria" , 
	"Andorra" , 
	"Angola" , 
	"Antigua & Barbuda" , /* 5 */ 
	"Argentina" , 
	"Armenia" , 
	"Australia" , 
	"Austria" , 
	"Azerbaijan" , 
	"Bahamas" , 
	"Bahrain" , 
	"Bangladesh" , 
	"Barbados" , 
	"Belarus" , 
	"Belgium" , 
	"Belize" , 
	"Benin" , 
	"Bhutan" , 
	"Bolivia" , 
	"Bosnia & Herzegovina" , /* 21 */
	"Botswana" , 
	"Brazil" , 
	"Brunei" , 
	"Bulgaria" , 
	"Burkina Faso" , 
	"Burundi" , 
	"Cambodia" , 
	"Cameroon" , 
	"Canada" , 
	"Cape Verde" , 
	"Central African Rep." , /* 32 */
	"Chad" , 
	"Chile" , 
	"China" , 
	"Colombia" , 
	"Comoros" , 
	"Congo Rep." , 
	"Congo, Dem. Rep." , 
	"Costa Rica" , 
	"Cote D'ivoire" , 
	"Croatia" , 
	"Cuba" , 
	"Cyprus" , 
	"Czech Rep." , 
	"Denmark" , 
	"Djibouti" , 
	"Dominica" , 
	"Dominican Rep." , 
	"Ecuador" , 
	"Egypt" , 
	"El Salvador" , 
	"Equatorial Guinea" ,  /* 53 */
	"Eritrea" , 
	"Estonia" , 
	"Ethiopia" , 
	"Fiji" , 
	"Finland" , 
	"France" , 
	"Gabon" , 
	"Gambia" , 
	"Georgia" , 
	"Germany" , 
	"Ghana" , 
	"Greece" , 
	"Grenada" , 
	"Guatemala" , 
	"Guinea" , 
	"Guinea-Bissau" , 
	"Guyana" , 
	"Haiti" , 
	"Honduras" , 
	"Hungary" , 
	"Iceland" , 
	"India" , 
	"Indonesia" , 
	"Iran" , 
	"Iraq" , 
	"Ireland" , 
	"Israel" , 
	"Italy" , 
	"Jamaica" , 
	"Japan" , /* 83 */
	"[]{JPN", /*"Kce"*/
	"Jordan" , 
	"Kazakhstan" , 
	"Kenya" , 
	"Kiribati" , 
	"Kuwait" , 
	"Kyrgyzstan" , 
	"Laos" , 
	"Latvia" , 
	"Lebanon" , 
	"Lesotho" , 
	"Liberia" , 
	"Libya" , 
	"Liechtenstein" , 
	"Lithuania" , 
	"Luxembourg" , 
	"Macedonia" , 
	"Madagascar" , 
	"Malawi" , 
	"Malaysia" , 
	"Maldives" , 
	"Mali" , 
	"Malta" , 
	"Marshall Islands" , /* 107 */ 
	"Mauritania" , 
	"Mauritius" , 
	"Mexico" , 
	"Micronesia" , 
	"Moldova" , 
	"Monaco" , 
	"Mongolia" , 
	"Morocco" , 
	"Mozambique" , 
	"Myanmar" , 
	"N. Korea" , 
	"Namibia" , 
	"Nauru" , 
	"Nepal" , 
	"Netherlands" , 
	"New Zealand" , 
	"Nicaragua" , 
	"Niger" , 
	"Nigeria" , 
	"Norway" , 
	"Oman" , 
	"Pakistan" , 
	"Palau" , 
	"Panama" , 
	"Papua New Guinea" ,  /*132*/
	"Paraguay" , 
	"Peru" , 
	"Philippines" , 
	"Poland" , 
	"Portugal" , 
	"Qatar" , 
	"Romania" , 
	"Russia" , 
	"Rwanda" , 
	"S. Africa" , 
	"S. Korea" , 
	"Samoa" , 
	"San Marino" , 
	"Sao Tome & Principe" , /* 146 */
	"Saudi Arabia" , 
	"Senegal" , 
	"Seychelles" , 
	"Sierra Leone" , 
	"Singapore" , 
	"Slovakia" , 
	"Slovenia" , 
	"Solomon Islands" , /* 154 */ 
	"Somalia" , 
	"Spain" , 
	"Sri Lanka" , 
	"St. Kitts & Nevis" , /* 158 */ 
	"St. Lucia" , 
	"St.Vincent&TheGrenadines" , /* 160 */ 
	"Sudan" , 
	"Suriname" , 
	"Swaziland" , 
	"Sweden" , 
	"Switzerland" , 
	"Syria" , 
//					"Taiwan" , 
	"Tajikistan" , 
	"Tanzania" , 
	"Thailand" , 
	"Togo" , 
	"Tonga" , 
	"Trinidad & Tobago" , /* 173 */ 
	"Tunisia" , 
	"Turkey" , 
	"Turkmenistan" , 
	"Tuvalu" , 
	"Uae" , 
	"Uganda" , 
	"Uk" , 
	"Ukraine" , 
	"Uruguay" , 
	"Usa" , /* 183 */
	"Uzbekistan" , 
	"Vanuatu" , 
	"Vatican City" , 
	"Venezuela" , 
	"Vietnam" , 
	"Yemen" , 
	"Yugoslavia" , 
	"Zambia" , 
	"Zimbabwe" , 
	"?" /* 193 */
};

/* ---------------------------------------------------------------- */
/* １文字書き込み */
static void Locate( Work *work, int x, int y )
{
	work->x = x + (56-18) ;
	work->y = y + (-4) ;
}
static int GetStringWidth ( char *string )
{
	int		ch, len = 0 ;
	while ( ( ch = *string++ ) != '\0' ){
		if ( ch < font_start ) continue ;
		if ( ch >= font_end ) continue ;
		if ( ch == '\n' ) break ;
		ch -= font_start ;
		len += font_width_list[ ch ] + FONT_SPACE ;
	}
	return ( len );
}
static int WriteFont( Work *work, int x, int y, int ch, int flag )
{
	unsigned char	*data, *tex_image, d ;
	int				shift, i, j ;

	if ( x < 0 ) return ( 0 );
	if ( y < 0 ) return ( 0 );
	if ( ( x + font_width ) > (work->width+12) ) return (1) ;
	if ( ( y + font_height ) > work->height ) return (1) ;

//printf("%c\n", ch );
	if ( ch < font_start || ch >= font_end ) return (0);
	ch -= font_start ;
	data = &font_data[ font_size * ch ];
	for ( i = 0 ; i < font_height ; i++ ){
		tex_image = &work->texel_image[ x / 2 + ( y + i ) * ( work->width / 2 ) ];
		shift = x & 1 ;
		for ( j = 0 ; j < font_width ; j++ ){
			if ( !( j & 1 ) ){
				d = *data & 0x0f ;
			} else {
				d = *data >> 4 ;
				data++ ;
			}
//printf("%01x", d );
			if ( flag ){
				d = ( d * 171 ) >> 8 ;
			}
			if ( !shift ){
				*tex_image |= d ;
			} else {
				*tex_image |= d << 4 ;
				tex_image++ ;
			}
			shift ^= 1 ;
		}
//printf("\n");
	}

	return ( font_width_list[ ch ] + FONT_SPACE );
}

static void WriteString( Work *work, char *string, int flag )
{
	int		ch ;
	int		x, y, len ;

#ifdef DEBUG_MODE
	printf("%s: X%d Y%d Flag%d :%s\n", __FILE__, work->x, work->y, flag, string);
#endif

	x = work->x ;
	y = work->y ;
	
   switch ( flag & 3 )
   {
	  case 0:
		break ;
	  case 1:
		len = GetStringWidth( string );
		x -= len ;
		break ;
	  case 2:
		len = GetStringWidth( string );
		x -= len / 2 ;
		break ;
	}

	while ( ( ch = *string++ ) != '\0' )
   {
		if ( ch == '\n' ){
			x = work->x ;
			y += FONT_HEIGHT + FONT_PITCH ;
			work->y += FONT_HEIGHT + FONT_PITCH ;
			switch ( flag & 3 ){
			  case 0:
				break ;
			  case 1:
				len = GetStringWidth( string );
				x -= len ;
				break ;
			  case 2:
				len = GetStringWidth( string );
				x -= len / 2 ;
				break ;
			}
			continue ;
		}
		x += WriteFont( work, x, y, ch, flag & 4 );
	}
}

/* ---------------------------------------------------------------- */

static void CreateDogtagTexture( Work *work )
{
	{
		int		x, y ;
		char	*texel ;
		texel = work->texel_image ;

      for ( y = 0 ; y < work->height ; y++ )
      {
			for ( x = 0 ; x < work->width ; x+=2 )
         {
				*texel++ = 0 ;
			}
		}
	}

	{
		char		buffer[256] ;
#ifdef BP_DOGTAG_TCR_011 //BP - change 'name' to 'code name' per MGSTWO-2171 to avoid TCR issue.
      Locate( work,  18,  21 ); WriteString( work, "Code Name", 4 );
#else
		Locate( work,  18,  21 ); WriteString( work, "Name", 4 );
#endif
		
      Locate( work,  18,  61 ); WriteString( work, "Nationality", 4 );

		Locate( work,  18,  101 ); WriteString( work, "Sex", 4 );

#ifndef BP_DOGTAG_TCR_011 //BP - remove blood type per MGSTWO-2171 to avoid TCR issue.
		Locate( work, 114+14,  101 ); WriteString( work, "Blood", 4 );
#endif
		Locate( work,  18, 123 ); WriteString( work, "Birthday", 4 );

		sprintf( buffer, "%s", (char*)GM_MyName );
		//sprintf( buffer, "%s", "TEST = NAME.A" );
		//sprintf( buffer, "%s", "WWWWWWWWWWWWWWW" );
		//Locate( work,  28,  48 ); WriteString( work, buffer, 0 );
		{
			int		len ;
			len = GetStringWidth( buffer );
			if ( ( 28 + len ) > 200 ){
				/* 文字が収まんなかった場合 */
				Locate( work, 114-4,  39 ); WriteString( work, buffer, 2 );
			} else {
				/* 通常 */
				Locate( work,  28,  39 ); WriteString( work, buffer, 0 );
			}
		}

		sprintf( buffer, "%s", nationality_list[GM_MyRegionData] );

		{
			int		len ;
			len = GetStringWidth( buffer );
			if ( ( 28 + len ) > 200 ){
				/* 文字が収まんなかった場合 */
				Locate( work, 114-4,  79 ); WriteString( work, buffer, 2 );
			} else {
				/* 通常 */
				Locate( work,  28,  79 ); WriteString( work, buffer, 0 );
			}
		}

#ifdef BP_DOGTAG_TCR_011 //BP - force to Male per MGSTWO-2171 to avoid TCR issue.
      sprintf( buffer, "%s", sex_list[0] );
#else
		sprintf( buffer, "%s", sex_list[GM_MySexData] );
#endif
		Locate( work,  78+7,  101 ); WriteString( work, buffer, 2 );

#ifndef BP_DOGTAG_TCR_011 //BP - remove blood type per MGSTWO-2171 to avoid TCR issue.
		sprintf( buffer, "%s", blood_list[GM_MyBloodData] );
		Locate( work, 200,  101 ); WriteString( work, buffer, 1 );
#endif

#ifdef BP_DOGTAG_TCR_011 //BP - blank out year of birthday per MGSTWO-2171 to avoid TCR issue.
      sprintf( buffer, "%02d/%02d/XXXX", GM_MyMonthData, GM_MyDayData );
#else
		sprintf( buffer, "%02d/%02d/%04d", GM_MyMonthData, GM_MyDayData, GM_MyYearData );
#endif
		Locate( work, 200, 123 ); WriteString( work, buffer, 1 );

      {
         unsigned int bpTexture = BP_AllocDynamicTexture(work->width, work->height, 0);
         BP_UpdateDynamicTextureCLUT16(bpTexture, work->texel_image, work->clut_image);
         BP_ReplaceTexture(bpTexture, work->tex->BP_TextureHandle);
         BP_FreeDynamicTexture(bpTexture);
      }
	}
}


/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	if ( work->texel_image != NULL ) GV_DelayedFree( work->texel_image );
	if ( work->clut_image != NULL ) GV_DelayedFree( work->clut_image );
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_TEX		*tex;
	int			i ;

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;

	work->tri_id = TRI_ID ;
	work->tex_id = TEXTURE_ID ;

	if ( GCL_GetOption( 't' ) != NULL )
   {
		work->tri_id = GCL_GetNextInt();
	}

#ifdef DEBUG_MODE
   strcpy( (char*)GM_MyName, "TAROO KONAMI" );
   GM_MyRegionData = 84;
   GM_MySexData = 1;
   GM_MyBloodData = 3;
   GM_MyYearData = 1987;
   GM_MyMonthData = 12;
   GM_MyDayData = 29;
#endif

	work->tex = tex = DG_GetTexture2( work->tri_id, work->tex_id );
	
   /* メモリ確保 */
	DG_GetTexelInfo( &work->width, &work->height, NULL, NULL, tex );

	work->texel_image = GV_Malloc( work->width * work->height * 8 / TEX_DEPTH );
	work->clut_image = GV_Malloc( sizeof(int) * ( 1 << TEX_DEPTH ) );

	{
      /* 転送テクスチャ初期化 */
		int		*clut, col ;
		clut = work->clut_image ;
		for ( i = 0 ; i < 16 ; i++ )
      {
			col = 63 - i * 4 ;
			clut[ i ] = col | ( col << 8 ) | ( col << 16 ) | ( 0x80 << 24 ) ;
		}
	}

	CreateDogtagTexture( work );

	/* テクスチャ入れ替え開始 */

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewUserDogtagSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	
   work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	
   if ( work != NULL ) 
   {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
	
      if ( GetResources( work, name, where ) < 0 ) 
      {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}

