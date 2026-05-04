/*
   MDU_texInit.c
   
   テクスチャ関連初期化
   
   by M.Sonoyama 1999 Aug.～
   $Id: MDU_texInit.c,v 1.17 2002/08/26 11:22:09 usr01363 Exp $
   
   Konami Computer Entertainment Japan West   
   */

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<sys/types.h>

#include	"MDU_tex.h"
#include	"MDU_util.h"

static	MDU_Tex	MDU_TexDatas[ MAX_TEXTURES ] ;

/* テクスチャダンプ */
static	void	DumpData( tex )
MDU_Tex		*tex ;
{
#if 0
    int		i ;
    u_int	*index ;

    i = tex->width * tex->height ;
    index = ( u_int * )tex->data.data ;
    while( -- i >= 0 ) printf( "%d.", *( index ++ ) ) ;
    printf( "\n" ) ;
#endif
}

static	void	DumpClut( tex )
MDU_Tex		*tex ;
{
    int		i ;
    RGBQUAD2	*rgb ;
    
    if ( tex->flag & MDU_TEX_FLAG_COLOR256 ) {
		printf( "256 colors\n" ) ;
		i = 256 ;
    } else {
		printf( "16 colors\n" ) ;
		i = 16 ;
    }
    rgb = ( RGBQUAD2 * )tex->clut.data ;
    while( -- i >= 0 ) {
		printf( "%d %d %d %d\n", rgb->r, rgb->g, rgb->b, rgb->alpha ) ;
		rgb ++ ;
    }
}

void		MDU_DumpTexture( tex )
MDU_Tex		*tex ;
{
    printf( "id :	%d\n", tex->id ) ;
    printf( "flag :	%x\n", tex->flag ) ;
    printf( "w, h :	%d %d\n", tex->width, tex->height ) ;
    printf( "n_colors 	%d\n", tex->n_colors ) ;
    DumpClut( tex ) ;
    DumpData( tex ) ;
}

/* アルファ値設定 */
/* ＣＬＵＴありのみ対応 */
void		MDU_SetAlphaTexture( tex, alpha )
MDU_Tex		*tex ;
int		alpha ;
{
    RGBQUAD2	*rgb ;
    int		i ;

    if ( !( tex->flag & MDU_TEX_FLAG_CLUT_EXIST ) ) return ;
    rgb = ( RGBQUAD2 * )( tex->clut.data ) ;
    for ( i = 0; i < tex->n_colors; i ++, rgb ++ ) {
		/* マスク処理に注意 */
		if ( rgb->r == 0 && rgb->g == 0 && rgb->b == 0 && 
			rgb->alpha == 0 ) continue ;
		rgb->alpha = alpha ;
    }
}

/* マスク処理 */
void		MDU_SetMaskTexture( tex, name )
MDU_Tex		*tex ;
char		*name ;
{
    RGBQUAD2	*clut ;
    int		n ;

    if ( MDU_FindString( name, "msk" ) < 0 ) return ;
    clut = ( RGBQUAD2 * )tex->clut.data ;
    n = tex->n_colors ;
    while( -- n >= 0 ) {
		if ( clut->r == 0 &&
			clut->g == 0 &&
			clut->b == 0 ) {
			clut->alpha = 0 ;
		}
		clut ++ ;
    }
}

void		MDU_SetMaskTextureEx( tex, sw )
MDU_Tex		*tex ;
int		sw ;
{
    RGBQUAD2	*clut ;
    int		n ;

    clut = ( RGBQUAD2 * )tex->clut.data ;
    n = tex->n_colors ;
    while( -- n >= 0 ) {
		if ( clut->r == 0 &&
			clut->g == 0 &&
			clut->b == 0 ) {
			if ( sw == 1 ) clut->alpha = 0 ;
			else           clut->alpha = 128 ;
		}
		clut ++ ;
    }
}

/* 半透明処理など */
void		MDU_SetTransTexture( tex, name )
MDU_Tex		*tex ;
char		*name ;
{
    if ( MDU_FindString( name, "_add" ) >= 0 ) {
		tex->flag &= ~0x30 ;
		tex->flag |= MDU_TEX_FLAG_TRANS_ADD ;
    } else if ( MDU_FindString( name, "_sub" ) >= 0 ) {
		tex->flag &= ~0x30 ;
		tex->flag |= MDU_TEX_FLAG_TRANS_SUB ;
    } else if ( MDU_FindString( name, "_hlf" ) >= 0 ) {
		tex->flag &= ~0x30 ;
		tex->flag |= MDU_TEX_FLAG_TRANS_BLD ;
		MDU_SetAlphaTexture( tex, 128 * 50 / 100 ) ;
    } else if ( MDU_FindString( name, "_bld" ) >= 0 ) {
		tex->flag &= ~0x30 ;
		tex->flag |= MDU_TEX_FLAG_TRANS_BLD ;
		{
			int		alpha, c ;
			char	buf[ 4 ] ;
	    
			c = MDU_FindString( name, "_bld" ) ;
			strncpy( buf, name + c + 4, 3 ) ;
			alpha = atoi( buf ) ;
			MDU_SetAlphaTexture( tex, 128 * alpha / 100 ) ;
		}
    } else if ( MDU_FindString( name, "_alp" ) >= 0 ) {
		/* アルファ値はテクスチャに設定されている */
		/* フラグだけセット */
		tex->flag &= ~0x30 ;
		tex->flag |= MDU_TEX_FLAG_TRANS_BLD ;	
    } 
    if ( MDU_FindString( name, "_decal" ) >= 0 ) {
		/* テクスチャの色をそのまま使う */
		tex->flag |= MDU_TEX_FLAG_DECAL ;
    } else {
		tex->flag &= ~MDU_TEX_FLAG_DECAL ;
	}

    /* 環境マッピング属性など */
    if ( MDU_FindString( name, "_emap" ) >= 0 ) {
		tex->flag &= ~(0x00070000) ;
		tex->flag |= MDU_TEX_FLAG_EMAP ;
    } else if ( MDU_FindString( name, "_bmap" ) >= 0 ) {
		tex->flag &= ~(0x00070000) ;
		tex->flag |= MDU_TEX_FLAG_BMAP ;
    } else if ( MDU_FindString( name, "_smap" ) >= 0 ) {
		tex->flag &= ~(0x00070000) ;
		tex->flag |= MDU_TEX_FLAG_SMAP ;
    } else {
		tex->flag &= ~(0x00070000) ;
	}

    if ( MDU_FindString( name, "_mod" ) >= 0 ) {
		/* ＳＣＥ＿ＧＳ＿ＳＥＴ＿ＡＬＰＨＡ用 */
		//	tex->flag &= ~0xf030 ;
		tex->flag &= ~0xf000 ;
		tex->flag |= MDU_TEX_FLAG_ALPHA_REG ;
		{
			int		mode, c, i, shf ;
			char	buf[ 2 ] ;
	    
			shf = 30 ;
			c = MDU_FindString( name, "_mod" ) ;
			for ( i = 4; i < 8; i ++ ) {
				strncpy( buf, name + c + i, 1 ) ;
				mode = atoi( buf ) ;
				tex->flag |= ( ( mode & 0x3 ) << shf ) ;
				shf -= 2 ;
			}
		}
    } else {
		tex->flag &= ~MDU_TEX_FLAG_ALPHA_REG ;		
	}
}

/* テクスチャデータ検索 */
/* id == 0 の時は、空きを探す */
MDU_Tex		*MDU_FindTexture( id )
u_int		id ;
{
    int		i ;
    MDU_Tex	*tex ;

    tex = MDU_TexDatas ;
    for ( i = 0; i < MAX_TEXTURES; i ++ ) {
		if ( tex->id == id ) return tex ;
		tex ++ ;
    }
    printf( "texture full\n" ) ;
    return NULL ;
}

/* テクスチャデータ部の領域確保 */
void		MDU_AllocTex( tex, width, height )
MDU_Tex		*tex ;
u_int		width, height ;
{
    int		offx ;
    /* 無理矢理偶数にする */
    if ( width % 2 ) width ++ ;
    offx = 4 - width % 4 ; if ( offx == 4 ) offx = 0 ;
	//    tex->data.data = MDU_Alloc( sizeof( u_int ) * ( width + offx ) * height ) ;
    tex->data.data = MDU_Alloc( sizeof( u_int ) * width * height ) ;
    tex->clut.data = MDU_Alloc( sizeof( RGBQUAD2 ) * 256 ) ;
    if ( tex->data.data == NULL ||
		tex->clut.data == NULL ) exit( -1 ) ;
    tex->width = width ;
    tex->height = height ;
}

/* テクスチャデータクリア */
void		MDU_ClearTexture( tex )
MDU_Tex		*tex ;
{
    tex->id = 0 ;
    tex->flag = 0 ;
    tex->height = tex->width = 0 ;
    tex->n_colors = 0 ;
    tex->data.dx = tex->data.dy = 0 ;
    tex->clut.dx = tex->clut.dy = 0 ;
    if ( tex->data.data != NULL ) MDU_Free( tex->data.data ) ; 
    if ( tex->clut.data != NULL ) MDU_Free( tex->clut.data ) ; 
    tex->data.data = NULL ;
    tex->clut.data = NULL ;
}

/* テクスチャ初期化 */
void		MDU_TexInit( void )
{
    int		i ;
    MDU_Tex	*tex ;

    tex = MDU_TexDatas ;
    for ( i = 0; i < MAX_TEXTURES; i ++ ) {
		tex->data.data = NULL ;
		tex->clut.data = NULL ;
		MDU_ClearTexture( tex ) ;
		tex ++ ;
	}
}

