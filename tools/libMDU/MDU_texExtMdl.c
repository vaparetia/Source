/*
   MDU_texExtMdl.c

   .mdlからテクスチャを抽出

   by M.Sonoyama 1999.Sep.～ 
   $Id: MDU_texExtMdl.c,v 1.20 2002/12/26 07:08:11 usr01363 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <SFDLINUX.h>

//#include "METALGEAR.h"
#include "MDU_tex.h"
#include "MDU_util.h"

static	int	N_Textures ;
static	MDU_Tex	*Textures[ MAX_TEXTURES ] ;
static	char	TexNames[ MAX_TEXTURES ][ 256 ] ;

typedef struct {
    u_char	b, g, r, alpha ;
} RGBQ ;

typedef struct {
    u_char	b, g, r ;
} RGBT ;

static	int	N_Colors ;
static  int decrease_level;


/* .mdlからテクスチャを抽出 */

/* 真面目に考える必要が無いことに気づき、非常に単純なものを作成 */
static int ShiftColor(int c)
{
    int cc=c;
    int lvl;

    lvl=(8-decrease_level);

    while(lvl<8){
	int nc;
	int mask;

	nc=c>>lvl;
	mask=0xff>>lvl;
	cc&=~mask;
	cc|=nc;
	lvl+=(8-decrease_level);
    }

    return cc;
}

static void DecreaseColor(HP3DPICTURE hP3DPicture)
{
    BITMAPINFOHEADER 	*bih ;
    RGBT		*rgb ;
    u_char		*alpha, *rgbpad ;
    int			i, w, pad, pada ;

    bih = ( BITMAPINFOHEADER * )P3DPicture( hP3DPicture )->hDIB ;

    pad = 4 - bih->biWidth * 3 % 4 ;
    if ( pad == 4 ) pad = 0 ;

    pada = 4 - bih->biWidth % 4 ;
    if ( pada == 4 ) pada = 0 ;

    rgb = ( RGBT * )( ( u_int )bih + ( u_int )bih->biSize ) ;
    alpha = ( u_char * )( ( u_int )bih + sizeof( BITMAPINFOHEADER ) 
			 + bih->biWidth * bih->biHeight * sizeof( RGBT ) 
			 + pad * bih->biHeight * sizeof( u_char ) ) ;
    w = 0 ; 

#if 0
    printf("%d %d %d %d %d %d , %p,%p,%p\n",
	   bih->biSize,bih->biWidth,bih->biHeight,pad,sizeof(RGBT),sizeof( BITMAPINFOHEADER ),
	   bih,rgb,alpha);
#endif

    for ( i = 0; i < bih->biWidth * bih->biHeight; i ++ ) {
	rgb->r=ShiftColor(rgb->r);
	rgb->g=ShiftColor(rgb->g);
	rgb->b=ShiftColor(rgb->b);
	*alpha=ShiftColor(*alpha);

	rgb++;
	alpha++;

	if ( ++ w == bih->biWidth ) {
	    rgbpad = ( u_char * )rgb ; rgbpad += pad ;
	    rgb = ( RGBT * )rgbpad ;
	    alpha += pada ;
	    w = 0 ;
	}
    }

#if 0
    printf("%p %p\n",rgb,alpha);
#endif

}

/* テクスチャを１枚抽出 */
static	int	ExtractTexture( name, hP3DPicture )
char		*name ;
HP3DPICTURE 	hP3DPicture ;
{
    BITMAPHEADER_PAD		bh ;
    BITMAPINFOHEADER 	*bihp, *bih ;
    RGBT		*rgb ;
    u_char		*alpha, alp, *rgbpad ;
    RGBQ		*clut, *rgbq ;
    char		out[ 256 ], *p ;
    MDU_Tex		*tex ;
    int			i, j, c, w, pad, pada, alpha16 ;
    u_int		*index ;

#if 0
    printf("%s : %p\n",P3DPicture( hP3DPicture )->szName,P3DPicture( hP3DPicture )->szName);
#endif

    bihp = ( BITMAPINFOHEADER * )P3DPicture( hP3DPicture )->hDIB ;

    bih = &bh.info ;
    memcpy( bih, bihp, sizeof( BITMAPINFOHEADER ) ) ;

    /* ２４ビットＢＭＰと256色に対応(VRS用) */
    if ( bih->biBitCount != 24 && bih->biBitCount != 8) {
	printf( "%s : invalid bit mode %d\n", name, bih->biBitCount ) ;
	return -1;
    }
    tex = MDU_FindTexture( 0 ) ;
    if ( tex == NULL ) return -1;
    MDU_AllocTex( tex, bih->biWidth, bih->biHeight ) ;
    /* 色数をカウント */
    alpha16 = 0 ;
    decrease_level=0;

check_color_start :
    pad = 4 - bih->biWidth * 3 % 4 ;
    if ( pad == 4 ) pad = 0 ;

    pada = 4 - bih->biWidth % 4 ;
    if ( pada == 4 ) pada = 0 ;

    rgb = ( RGBT * )( ( u_int )bihp + ( u_int )bihp->biSize ) ;
    alpha = ( u_char * )( ( u_int )bihp + sizeof( BITMAPINFOHEADER ) 
			 + bih->biWidth * bih->biHeight * sizeof( RGBT ) 
			 + pad * bih->biHeight * sizeof( u_char ) ) ;
    N_Colors = 0 ; w = 0 ; 
    clut = ( RGBQ * )tex->clut.data ;
    index = ( u_int * )tex->data.data ;
    index += tex->width * ( bih->biHeight - 1 ) ;

    if ( bih->biBitCount == 8 ) {
	u_char	*rgbpad ;
	int	used ;

	/* 特別よん */
	printf( "biBitCount == 8\n" ) ;
	rgbq = ( RGBQ * )rgb ;
	used = bih->biClrUsed ;
	for ( i = 0; i < 256; i ++ ) {
	    if ( used != 0 && i == used ) break ;
	    clut->r = rgbq->r ;
	    clut->g = rgbq->g ;
	    clut->b = rgbq->b ;
	    clut->alpha = 128 ;
	    clut ++ ; rgbq ++ ;
	}
	rgbpad = ( u_char * )( ( ( u_int )rgbq + 3 ) / 4 * 4 ) ;
//	rgbpad = ( u_char * )( ( ( u_int )rgbq + 3 ) / 4 * 4 + 8 - bih->biWidth * 3 ) ;
	for ( i = 0; i < bih->biWidth * bih->biHeight; i ++ ) {
	    *( index ++ ) = *( rgbpad ++ ) ;
	    if ( ++ w == bih->biWidth ) {
		rgbpad += pad ;
		index -= bih->biWidth * 2 ;
		w = 0 ;
	    }
	}
	N_Colors = 256 ;
	goto mdu_tex_setting ;
    }


    for ( i = 0; i < bih->biWidth * bih->biHeight; i ++ ) {
	c = 0 ;
	if ( MDU_FindString( name, "_alp" ) >= 0 ) {
	    if ( alpha16 == 1 ) {
		alp = *alpha >> 4 ; alp &= 0xf ;
		alp = alp * 128 / 15 ;
		alpha ++ ;
	    } else {
		alp = *( alpha ++ ) ;
		if ( MDU_FindString( name, "_alpx2" ) < 0 ) {
		    alp = alp * 128 / 255 ;
		}
	    }
	} else {
	    alp = 128 ;
	}

	for ( j = 0; j < N_Colors; j ++ ) {
	    if ( rgb->r == clut[ j ].r &&
		 rgb->g == clut[ j ].g &&
		 rgb->b == clut[ j ].b && 
		 alp == clut[ j ].alpha ) {
		c = 1 ;
		*( index ++ ) = j ;
		break ;
	    }
	}
	if ( c == 0 ) {
	    if ( N_Colors == 256 ) {
		if ( MDU_FindString( name, "_alp" ) >= 0 &&
		     alpha16 == 0 ) {
		    /* アルファ１６段階モードで変換しなおす */
		    printf( "%s : alpha -> 16 levels\n", name ) ;
		    alpha16 = 1 ;
		    goto check_color_start ;
		}
#if 0
		printf( "%s : too many colors\n", name ) ;
		MDU_ClearTexture( tex ) ;

		return -1;
#else
		decrease_level++;
		DecreaseColor(hP3DPicture);
		goto check_color_start ;
#endif
	    } else {
		clut[ N_Colors ].r = rgb->r ;
		clut[ N_Colors ].g = rgb->g ;
		clut[ N_Colors ].b = rgb->b ;
		clut[ N_Colors ].alpha = alp ;
		*( index ++ ) = N_Colors ;
		N_Colors ++ ;
	    }
	}
	rgb ++ ; 
	if ( ++ w == bih->biWidth ) {
	    rgbpad = ( u_char * )rgb ; rgbpad += pad ;
	    rgb = ( RGBT * )rgbpad ;
	    alpha += pada ;
	    w = 0 ;
	    index -= bih->biWidth + tex->width ;
	}
    }
mdu_tex_setting :
    if(decrease_level>0){
	printf("Error : %s : Decrease Color : %d\n",name,decrease_level);
    }

    tex->id = MDU_GetStrCode( name ) ;
    tex->data.dx = tex->data.dy = 0 ;
    tex->clut.dx = tex->clut.dy = 0 ;
    tex->clut.h = 1 ;
    if ( N_Colors <= 16 ) {
	tex->n_colors = tex->clut.w = 16 ;
	tex->flag = MDU_TEX_FLAG_CLUT_EXIST | MDU_TEX_FLAG_COLOR16 ;
	tex->data.w = tex->width / 2 ;
	tex->data.h = tex->height ;
    } else {
	tex->n_colors = tex->clut.w = 256 ;
	tex->flag = MDU_TEX_FLAG_CLUT_EXIST | MDU_TEX_FLAG_COLOR256 ;
	tex->data.w = tex->width ;
	tex->data.h = tex->height ;
    }

    MDU_SetTransTexture( tex, name ) ;
    MDU_SetMaskTexture( tex, name ) ;

    Textures[ N_Textures ] = tex ;
    strcpy( out, name ) ;
    p = out + strlen( out ) ;
    strcpy( p, ".bmp\0" ) ;
    strcpy( TexNames[ N_Textures ], out ) ;
    N_Textures ++ ;
    // MDU_SaveBmp( tex, out ) ;

    return 0;
}

/* 抽出したテクスチャを指定ディレクトリにセーブ */
void	MDU_SaveExtractTextures( dir )
char	*dir ;
{
    extern int	MDU_M2K_FindTexture( char * ) ;
    char	fullpath[ 512 ] ;
    int		i ;
    
    for ( i = 0; i < N_Textures; i ++ ) {
	if ( MDU_M2K_FindTexture( TexNames[ i ] ) < 0 ) continue ;
	strcpy( fullpath, dir ) ;
	strcat( fullpath, "/" ) ;	
	strcat( fullpath, TexNames[ i ] ) ;
	MDU_SaveBmp( Textures[ i ], fullpath ) ;
    }
}

/* mdlからテクスチャを抽出 */
int	MDU_ExtractTextureFromMdl( hP3DModel )
HP3DMODEL	hP3DModel ;
{
    HP3DPICTURE   hP3DPicture ;    
    char          szFName[ _MAX_FNAME ] ;
    
    N_Textures = 0 ;
    if ( ( hP3DPicture = P3DGetFirstPicture( hP3DModel ) ) != NULL ) {
	do {
	    if(strlen(P3DPicture( hP3DPicture )->szName)>0){

#if 0
		printf("%s 0x%08x %d %d\n",
		       P3DPicture( hP3DPicture )->szName,
		       (int)(P3DPicture( hP3DPicture )->szName),
		       strlen(P3DPicture( hP3DPicture )->szName),_MAX_FNAME);
#endif

		_splitpath( P3DPicture( hP3DPicture )->szName, NULL, NULL, szFName, NULL ) ;
		if(ExtractTexture( szFName, hP3DPicture )==-1) return -1;
	    }
	} while ( ( hP3DPicture = P3DGetNextPicture( hP3DPicture ) ) != NULL ) ;
    }
    printf( "%d textures\n", N_Textures ) ;

    return 0;
}

/* ＩＤ検索 */
void	MDU_CompTexID( int id )
{
    int		i ;
    int		idd ;

    for ( i = 0; i < N_Textures; i ++ ) {
	idd = MDU_GetStrCode( TexNames[ i ] ) ;
	if ( id < 0 || id == idd ) {
	    fprintf( stderr, "%d : %s\n", idd, TexNames[ i ] ) ;
	}
    }
}
