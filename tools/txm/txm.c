/*
   txm.c : texture manager for MGS on linux ( use libMDU )

   by M.Sonoyama 1999 Aug. ～
   $Id: txm.c,v 1.4 1999/10/26 08:52:25 usr02011 Exp $
 
   Konami Computer Entertainment Japan West CS1
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<sys/types.h>

#include	"MDU_tex.h"
#include	"MDU_util.h"

static	int	ZeroFlag, TransFlag, AlphaMode, BlendValue ;

static	void	Usage( void )
{
    printf( "txm : texture manager for MGS\n" ) ;
    printf( "usage : dar <options & value> -i file1 [file2 ...]\n" ) ;
    printf( "\t supported extensions is [.bmp]\n" ) ;
    printf( "\t <options> :\n" ) ;
    printf( "\t\t -z : +/- nottrans/trans black pixel\n" ) ;
    printf( "\t\t -t : +/- trans/nottrans color pixel\n" ) ;
    printf( "\t\t -a : hlf/add/sub/bld/alp change transparent mode\n" ) ;
    printf( "\t\t -b : blend value < 0 ～ 100 percent >\n" ) ;
    exit( -1 ) ;
}

int	main( int argc, char **argv )
{
    MDU_Tex	*tex ;
    char	*texs, *ext, *v, alp[ 4 ] ;
    int		iv ;

    /* オプション解釈 */
    ZeroFlag = TransFlag = AlphaMode = BlendValue = -1 ;
//    if ( argc < 4 ) Usage() ;
    MDU_ParseOption( argc, argv ) ;
    if ( MDU_GetOption( 'z' ) != NULL ) {
	v = MDU_GetNextValue() ;
	if ( !strcmp( v, "+" ) ) ZeroFlag = 0 ;
	else if ( !strcmp( v, "-" ) ) ZeroFlag = 1 ;
	else	ZeroFlag = -1 ;
    }
    if ( MDU_GetOption( 't' ) != NULL ) {
	v = MDU_GetNextValue() ;
	if ( !strcmp( v, "+" ) ) {
	    AlphaMode = MDU_TEX_FLAG_TRANS_BLD ;
	    BlendValue = 50 ;	    
	} else if ( !strcmp( v, "-" ) ) {
	    TransFlag = 1 ;
	} else	TransFlag = -1 ;
    }
    if ( MDU_GetOption( 'a' ) != NULL ) {
	v = MDU_GetNextValue() ;
	strncpy( alp, v, 3 ) ;
	if ( !strcmp( alp, "add" ) ) AlphaMode = MDU_TEX_FLAG_TRANS_ADD ;
	else if ( !strcmp( alp, "sub" ) ) AlphaMode = MDU_TEX_FLAG_TRANS_SUB ;
	else if ( !strcmp( alp, "bld" ) ) AlphaMode = MDU_TEX_FLAG_TRANS_BLD ;
	else if ( !strcmp( alp, "alp" ) ) {
	    /* フラグを立てるだけ */
	    AlphaMode = -2 ;
	} else if ( !strcmp( alp, "hlf" ) ) {
	    AlphaMode = MDU_TEX_FLAG_TRANS_BLD ;
	    BlendValue = 50 ;
	} else AlphaMode = -1 ;
    }
    if ( MDU_GetOption( 'b' ) != NULL ) {
	iv = MDU_GetNextInt() ;
	if ( iv >= 0 && iv <= 100 ) BlendValue = iv ;
	else BlendValue = -1 ;
    }
    if ( MDU_GetOption( 'i' ) == NULL ) Usage() ;
    texs = MDU_GetNextValue() ;
    /* 実行 */
    while( texs != NULL ) {
	ext = MDU_GetExtension( texs ) ;
	if ( !strcmp( ext, "bmp" ) ) {
	    tex = MDU_LoadBmp( texs ) ;
	} else if ( !strcmp( ext, "pic" ) ) {
	    tex = MDU_LoadPic( texs ) ;
	} else {
	    printf( "%s : unknown extension\n", texs ) ;
	    goto tex_skip ;
	}
	if ( tex == NULL ) {
	    printf( "%s : texture load error\n", texs ) ;
	    goto tex_skip ;	    
	}
	if ( ZeroFlag != -1 ) {
	    if ( ZeroFlag == 0 ) printf( "%s : set black pixel -> nottrans\n", texs ) ;
	    else		 printf( "%s : set black pixel -> trans\n", texs ) ;
	    MDU_SetMaskTextureEx( tex, ZeroFlag ) ;
	}
	if ( TransFlag != -1 ) {
	    printf( "%s : color pixel -> nottrans\n", texs ) ;
	    tex->flag &= ~0x30 ;
	    MDU_SetAlphaTexture( tex, 128 ) ;
	}
	if ( AlphaMode == -2 ) {
	    printf( "%s : set blend mode bit\n", texs ) ;
	    tex->flag &= ~0x30 ;
	    tex->flag |= MDU_TEX_FLAG_TRANS_BLD ;
	}
	if ( AlphaMode >= 0 ) {
	    printf( "%s : set alpha mode 0x%x\n", texs, AlphaMode ) ;
	    tex->flag &= ~0x30 ;
	    tex->flag |= AlphaMode ;
	    MDU_SetAlphaTexture( tex, 128 ) ;
	}
	if ( BlendValue != -1 ) {
	    printf( "%s : set blending value %d\n", texs, BlendValue ) ;
	    MDU_SetAlphaTexture( tex, BlendValue * 128 / 100 ) ;
	}
	MDU_ChangeExtension( texs, "bmp" ) ;
	MDU_SaveBmp( tex, texs ) ;
	MDU_ClearTexture( tex ) ;
tex_skip :	
        texs = MDU_GetNextValue() ;
    }
    exit( 0 ) ;
}
