/*
	$Id: loadcm2.c,v 1.7 2001/02/01 07:48:08 usr02774 Exp $
 */
/*                                              */
/* ＣＭ２形式テクスチャ読み込みユーティリティー [[Texture read format utility]] */
/*                                              */
/* 1999.03.29 K.Takabe                         	*/
/* 1999.11.10 M.Matsuzaki                  		*/
/*                                              */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "loadinc.h"

#if 0
/* ツール用テクスチャ格納イメージ生成ルーチン有効化 [[Enable tool for store image generation routine]] */
#define IMAGE_PREVIEW
#endif

static int	texel_size[4] = {4,8,16,32};				/* モード値－＞テクセルビット数変換 [[Mode value -> texture bit number conversion]] */
static int	clut_entry_size[4] = {16,256,0,0};			/* モード値－＞ＣＬＵＴエントリー数変換 [[Mode value -> CLUT entry number conversion]] */
static int	tex_mode[4] = { 0x14, 0x13, 0x02, 0x00 };	/* モード値－＞ＰＳＭ値変換テーブル [[Mode value -> PSM value conversion table]] */
//static int	clut_offset_data[4] = { 1, 16, 0, 0 };		/* モード値－＞ＣＬＵＴベースアドレス差分値変換テーブル [[Mode value -> CLUT base address difference value conversion table]] */
static int	clut_offset_data[4] = { 4, 16, 0, 0 };		/* モード値－＞ＣＬＵＴベースアドレス差分値変換テーブル [[Mode value -> CLUT base address difference value conversion table]] */
static int	trans_offset_data[4][32] = {
	{0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,16,8,24,32,48,40,56,64,80,72,88,96,112,104,120,128,144,136,152,160,176,168,184,192,208,200,216,224,240,232,248},
	{0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};
static unsigned char gamma_table[256] ;

int			MaxTextures = 0 ;
TEX_INFO	TextureList[ 16 ] ;
#ifdef IMAGE_PREVIEW
void		*rgba_color_image[ 16 ] ;	/* プレビュー用イメージデータ [[Preview image data]] */
#endif

static void ConvertImage( TexData2 *tex_data,
						 unsigned char *org_image, unsigned char *clut_data,
						 unsigned char *dst_image, int width, int mode );


/* ---------------------------------------------------------------- */
static int ConvertGamma( int data, float gamma_inv )
{
	float	a ;

	if ( data == 0 ) return ( 0 );
	a = (float)( data & 0xff ) / 255.0 ;
	a = exp( gamma_inv * log( a ) ) ;
	return ( (int)( a * 255 ) );
}
static void MakeGammaTable( float gamma )
{
	float	a ;
	int		i ;

	a = 1 / gamma ;
	for ( i = 0 ; i < 256 ; i++ ){
		gamma_table[i] = (unsigned char)ConvertGamma( i, a );
	}
	
}

/* ---------------------------------------------------------------- */
static int LoadCm2Data( FILE *fp, int code, int size )
{
	CM2_FILEHEADER	*tmp_header ;
	TEX_INFO		*info ;
	TexData2		*tex_datas ;
	int				i, j, image_size, clut_size ;
	unsigned long64	tex ,tex2 ;

   /* テンポラリのワークに一度全部読み込む [[Temporary for work once read all the time]] */
	tmp_header = malloc( size );
	fread( tmp_header, 1, size, fp );

	tex_datas = malloc( sizeof(TexData2) * tmp_header->n_textures ) ;

	info = &TextureList[ MaxTextures ] ;
	info->width = tmp_header->width ;
	info->height = tmp_header->height ;
	info->n_textures = tmp_header->n_textures ;
	info->mode = tmp_header->mode ;

	if ( tmp_header->magic != NEW_CM2_MAGIC ){/* テクスチャ情報の変換 [[Texture information conversion]] */
		TexData	*org_tex ;
		TexData2	*new_tex ;
		int			flag ;

		flag = ( tmp_header->mode & 3 ) | 0x8 ;
		org_tex = (TexData*)&tmp_header[1] ;
		new_tex = tex_datas ;
		for ( i = 0 ; i < tmp_header->n_textures ; i++ ){
			new_tex[i].strcode = org_tex[i].strcode ;
			new_tex[i].flag = flag ;
			new_tex[i].n_colors = org_tex[i].n_colors ;
			new_tex[i].width = org_tex[i].width ;
			new_tex[i].height = org_tex[i].height ;
			new_tex[i].pix_x = org_tex[i].pix_x ;
			new_tex[i].pix_y = org_tex[i].pix_y ;
			new_tex[i].clut_x = org_tex[i].clut_x ;
			new_tex[i].clut_y = org_tex[i].clut_y ;
			new_tex[i].clut_addr = org_tex[i].clut_addr ;
		}
	} else {
		TexData2	*org_tex ;
		TexData2	*new_tex ;

		org_tex = (TexData2*)&tmp_header[1] ;
		new_tex = tex_datas ;
		for ( i = 0 ; i < tmp_header->n_textures ; i++ ){
			*new_tex++ = *org_tex++ ;
		}
	}

   /* 記憶領域の確保 [[Allocate storage]] */
	info->textures = malloc( sizeof( DG_TEX ) * info->n_textures );
	info->image_size = image_size = info->width * info->height * texel_size[ tmp_header->mode & 7 ] / 8 ;
#ifdef IMAGE_PREVIEW
	rgba_color_image[ MaxTextures ] = malloc( info->width * info->height * 4 );
	memset( rgba_color_image[ MaxTextures ], 0, info->width * info->height * 4 );
#endif
	/* イメージデータ領域の確保 */
	info->image_data = malloc( image_size );
	memset( info->image_data, 0, image_size );
	/* パレット領域の確保 [[Palette area to ensure]] */
	info->clut_data = NULL ;
	clut_size = clut_entry_size[ info->mode & 7 ] ;
	info->clut_size = clut_size * info->n_textures * 4 ;
	if ( clut_size != 0 ){
		info->clut_data = malloc( info->clut_size );
		memset( info->clut_data, 0, info->clut_size );
	}

	{/* ２のｎ乗とするサイズを求める [[The power and size is determined by 2 n]] */
		unsigned int	twe = 0, the = 0, tw, th ;
		tw = 1 ; i = info->width - 1 ;
		while( i > 0 ){ i >>= 1 ; tw <<= 1 ; twe++ ; }
		th = 1 ; i = info->height - 1 ;
		while( i > 0 ){ i >>= 1 ; th <<= 1 ; the++ ; }
		info->w_exp = twe ;
		info->h_exp = the ;
	}

	{/* テクセルデータのコピー [[Copy texture data]] */
		int		*src, *dst ;
		int		count = 0 ;

		dst = info->image_data ;
		src = (int*)( (int)tmp_header->pixdata + (int)tmp_header );
		for ( i = image_size / 4 ; i > 0 ; i-- ){
			*dst = *src ;
			dst++ ;
			src++ ;
		}
	}

#if 0
   /* パレットデータのコピー [[Copying palette data]] */
	if ( ( tmp_header->mode & 7 ) <= 1 ){
		int		*src, *dst, x, y, offset ;
		TexData2	*tex_data ;

		tex_data = tex_datas ;
		for ( i = 0 ; i < tmp_header->n_textures ; i++, tex_data++ ){
			src = (int*)( (int)(tex_data->clut_addr) + (int)tmp_header );
			dst = (int*)info->clut_data + i * clut_size ;
			for ( j = 0 ; j < clut_size ; j++ ){
				offset = trans_offset_data[ tmp_header->mode & 7 ][ j / 8 ] + ( j & 7 ) ;
				dst[ offset ] = *src ;
				((char*)&dst[offset] )[3] = 128 ;
				src++ ;
			}
		}
	}
#else
	if ( ( tmp_header->mode & 7 ) <= 1 ){
		int		*src, *dst, x, y, offset ;
		TexData2	*tex_data ;

		tex_data = tex_datas ;
		for ( i = 0 ; i < info->n_textures ; i++, tex_data++ ){
			src = (int*)( (int)(tex_data->clut_addr) + (int)tmp_header );
			dst = (int*)info->clut_data + i * clut_size ;
			for ( j = 0 ; j < clut_size ; j++ ){
				offset = j ;
				dst[ offset ] = *src ;
				src++ ;
			}
		}
	}
#endif

	{/* テクスチャ情報の作成 [[ Creating the texture]] */
		/* ベースとなるテクスチャ設定値を生成 [[Underlying texture generation settings]] */
		tex = 0 ;/* ベースアドレス [[Base address]] */
		tex |= ( info->width / 64 ) << 14 ;	/* TBW */
		tex |= tex_mode[ info->mode & 7 ] << 20 ;	/* PSM */
		tex |= (unsigned long64)info->w_exp << 26 ;	/* TW */
		tex |= (unsigned long64)info->h_exp << 30 ;	/* TH */
		tex |= (unsigned long64)1 << 34 ;	/* TCC(RGBA) */
		tex |= (unsigned long64)0 << 35 ;	/* TFX(MODULATE) */
		tex |= (unsigned long64)0 << 37 ;	/* CBP */
		tex |= (unsigned long64)0 << 51 ;	/* CPSM(PSMCT32) */
		tex |= (unsigned long64)0 << 55 ;	/* CSM(CSM1) */
		tex |= (unsigned long64)0 << 56 ;	/* CSA */
		tex |= (unsigned long64)0 << 61 ;	/* CLD(???) */

		tex2 = 0;	/* TEX2レジスタ [[Register]] */
		tex2 |= tex_mode[ 1 ] << 20 ;		/* PSM TEX2は 8bit mode 固定 [[Fixed]] */
		tex2 |= (unsigned long64)0 << 37 ;	/* CBP */
		tex2 |= (unsigned long64)0 << 51 ;	/* CPSM(PSMCT32) */
		tex2 |= (unsigned long64)0 << 55 ;	/* CSM(CSM1) */
		tex2 |= (unsigned long64)0 << 56 ;	/* CSA */
		tex2 |= (unsigned long64)4 << 61 ;	/* CLD(???) */
	}

	{/* ヘッダー及び各テクスチャ情報をコピーする [[Copy the header information for each texture and]] */
		TexData2	*src_tex ;
		DG_TEX		*texture ;
		float		tex_width, tex_height ;
		int			u_max, u_min, v_max, v_min ;
		unsigned long64	clut_base, clut_offset ;

		tex_width = (float)info->width ;
		tex_height = (float)info->height ;
		clut_offset = clut_offset_data[ tmp_header->mode & 7 ] ;
		clut_base = 0 ;/* 注）ＣＬＵＴベースアドレス／４を意味する [[Note) means CLUT base address/4]] */
		info->max_x = 0 ; info->max_y  = 0 ;

		texture = info->textures ;
		src_tex = tex_datas ;
		for ( i = info->n_textures ; i > 0 ; i--, src_tex++, texture++ ){
			u_min = src_tex->pix_x ;
			v_min = src_tex->pix_y ;
			u_max = src_tex->pix_x + src_tex->width ;
			v_max = src_tex->pix_y + src_tex->height ;
			if ( info->max_x < u_max ) info->max_x = u_max ;
			if ( info->max_y < v_max ) info->max_y = v_max ;
			memset( texture, 0, sizeof(DG_TEX) );
			texture->tex_id = src_tex->strcode ;
			texture->flag = 0 ;
#if 0
			texture->u_offset = (float)src_tex->pix_x / tex_width ;
			texture->v_offset = (float)src_tex->pix_y / tex_height ;
			texture->u_scale = (float)src_tex->width / tex_width ;
			texture->v_scale = (float)src_tex->height / tex_height ;
#else
			texture->u_offset = ( (float)src_tex->pix_x + 0.5F ) / tex_width ;
			texture->v_offset = ( (float)src_tex->pix_y + 0.5F ) / tex_height ;
			texture->u_scale = ( (float)src_tex->width - 1.0F ) / tex_width ;
			texture->v_scale = ( (float)src_tex->height - 1.0F ) / tex_height ;
#endif
			/* GIF設定 [[Settings]] */
			texture->tex_trans.giftag.tag = SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 );
			texture->tex_trans.giftag.regs = 0x0fffeeee ;
			texture->tex_trans.clamp.reg = SCE_GS_CLAMP_1 ;
			texture->tex_trans.tex2.reg = SCE_GS_TEX2_1 ;
			texture->tex_trans.tex0.reg = SCE_GS_TEX0_1 ;
			texture->tex_trans.alpha.reg = SCE_GS_ALPHA_1 ;
			texture->tex_trans.pad[0].reg = SCE_GS_NOP ;
			texture->tex_trans.clamp.data = SCE_GS_SET_CLAMP( 2, 2, u_min, u_max-1, v_min, v_max-1 );
			texture->tex_trans.tex2.data = tex2 ;
			texture->tex_trans.tex0.data = tex ;
			if ( src_tex->flag & CM2_TEX_DECAL ){
				texture->tex_trans.tex0.data |= (unsigned long64)1 << 35 ;	/* TFX(DECAL) */
				printf("decal flag on(%d)\n", src_tex->strcode);
			}
			/* αモード設定 [[Alpha mode setting]] */
			if ( !( src_tex->flag & CM2_TEX_USE_AMODE ) ){
				switch ( ( src_tex->flag & CM2_TEX_BLEND_MODE ) >> 4 ){
				case 0:/* half */
					printf("check texture half (%d)\n", src_tex->strcode);
					texture->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 );
					//texture->alpha = (unsigned long64)0x00000044 ;
					//texture->alpha = (unsigned long64)0x00000064 | ((unsigned long64)0x0040 << 32 );
					break ;
				case 1:/* add */
					printf("check texture add (%d)\n", src_tex->strcode);
					texture->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 );
					//texture->alpha = (unsigned long64)0x00000048 ;
					break ;
				case 2:/* sub */
					printf("check texture sub (%d)\n", src_tex->strcode);
					texture->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 );
					//texture->alpha = (unsigned long64)0x00000042 ;
					break ;
				case 3:/* blend */
					printf("check texture blend (%d)\n", src_tex->strcode);
					texture->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 );
					//texture->alpha = (unsigned long64)0x00000044 ;
					break ;
				}
			} else {
				printf("check texture immediate (%d)\n", src_tex->strcode);
				texture->tex_trans.alpha.data = (unsigned long64)( ( src_tex->flag & CM2_TEX_AMODE_MASK ) >> 24 );
				//texture->alpha = (unsigned long64)( ( src_tex->flag & CM2_TEX_AMODE_MASK ) >> 24 ) ;
			}
			/* テクスチャモード設定 [[Texture mode setting]] */
			if ( src_tex->flag & CM2_TEX_FLAG_EMAP ){
				printf("emap texture (%d)\n", src_tex->strcode );
				texture->flag |= TRI_TEXTAG_FLAG_EMAP ;
			}
			if ( src_tex->flag & CM2_TEX_FLAG_SMAP ){
				printf("smap texture (%d)\n", src_tex->strcode );
				texture->flag |= TRI_TEXTAG_FLAG_SMAP ;
			}
			if ( src_tex->flag & CM2_TEX_FLAG_BMAP ){
				printf("bmap texture (%d)\n", src_tex->strcode );
				texture->flag |= TRI_TEXTAG_FLAG_BMAP ;
			}
			/* テクスチャ固有ベクトルデータ設定 [[Texture specific vector data set]] */
			if ( texture->flag & TRI_TEXTAG_FLAG_BMAP ){
				/* バンプマッピングテクスチャ時の設定（テクスチャシフト量パラメータ）[[Setting at bump mapping texture (texture shift parameter)]] */
				int		t_size ;
				t_size = 1 << info->w_exp ;
				texture->tex_trans.vec1.vx = -1.0f / (float)t_size ;
				t_size = 1 << info->h_exp ;
				texture->tex_trans.vec1.vy = -1.0f / (float)t_size ;
			} else {
				/* 環境マップテクスチャ（バンプマッピング以外）時の設定 [[Environment map texture (non-bump mapping) settings]] */
				texture->tex_trans.vec1.vx = -0.4999f * texture->u_scale ;
				texture->tex_trans.vec1.vy = -0.4999f * texture->v_scale ;
				texture->tex_trans.vec1.vz = 1.0f ;
				texture->tex_trans.vec1.vw = 1.0f ;
				texture->tex_trans.vec2.vx = 0.5f * texture->u_scale + texture->u_offset ;
				texture->tex_trans.vec2.vy = 0.5f * texture->v_scale + texture->v_offset ;
				texture->tex_trans.vec2.vz = 0.0f ;
				texture->tex_trans.vec2.vw = 0.0f ;
			}
#ifdef IMAGE_PREVIEW
			ConvertImage( src_tex, 
						 info->image_data,  (unsigned char*)( (int)(src_tex->clut_addr) + (int)tmp_header ),
						 rgba_color_image[ MaxTextures ], info->width,
						 info->mode & 7 ) ;
#endif
			//clut_base += clut_offset ;
		}
	}

	free( tex_datas );
	free( tmp_header );

	MaxTextures++ ;

	return (MaxTextures-1);
}

#ifdef IMAGE_PREVIEW
static void ConvertImage( TexData2 *tex_data,
						 unsigned char *org_image, unsigned char *clut_data,
						 unsigned char *dst_image, int width, int mode )
{
	int x, y, i, j ;
	static packsize_table[4] = {4,8,16,32};

	
	for ( y = 0 ; y < tex_data->height ; y++ ){
		unsigned char	*src, *dst ;
		src = org_image + ( ( tex_data->pix_x + ( tex_data->pix_y + y ) * width ) * packsize_table[ mode ] / 8 ) ;
		dst = dst_image + ( ( tex_data->pix_x + ( tex_data->pix_y + y ) * width ) * 4 ) ;
      /* 横１ラインを処理する [[Side 1 line processing]] */
		switch ( mode ){
		  case 0:
         /* ４ビットインデックステクスチャ展開 [[4 bit index texture expand]] */
			for ( x = 0 ; x < tex_data->width ; x+=2 ){
				int index ;
				index = *src & 0x0f ;
				dst[0] = clut_data[ index * 4 + 0 ] ;
				dst[1] = clut_data[ index * 4 + 1 ] ;
				dst[2] = clut_data[ index * 4 + 2 ] ;
				dst[3] = clut_data[ index * 4 + 3 ] ;
				dst[0] = gamma_table[ dst[0] ] ;
				dst[1] = gamma_table[ dst[1] ] ;
				dst[2] = gamma_table[ dst[2] ] ;
				dst += 4 ;
				index = ( *src >> 4 ) & 0x0f ;
				dst[0] = clut_data[ index * 4 + 0 ] ;
				dst[1] = clut_data[ index * 4 + 1 ] ;
				dst[2] = clut_data[ index * 4 + 2 ] ;
				dst[3] = clut_data[ index * 4 + 3 ] ;
				dst[0] = gamma_table[ dst[0] ] ;
				dst[1] = gamma_table[ dst[1] ] ;
				dst[2] = gamma_table[ dst[2] ] ;
				dst += 4 ;
				src++ ;
			}
			break ;
		  case 1:
			/* ８ビットインデックステクスチャ展開 [[8 bit index texture expand]] */
			for ( x = 0 ; x < tex_data->width ; x++ ){
				int index ;
				index = *src ;
				dst[0] = clut_data[ index * 4 + 0 ] ;
				dst[1] = clut_data[ index * 4 + 1 ] ;
				dst[2] = clut_data[ index * 4 + 2 ] ;
				dst[3] = clut_data[ index * 4 + 3 ] ;
				dst[0] = gamma_table[ dst[0] ] ;
				dst[1] = gamma_table[ dst[1] ] ;
				dst[2] = gamma_table[ dst[2] ] ;
				dst += 4 ;
				src++ ;
			}
			break ;
		  case 2:
			/* １６ビットカラーテクスチャ展開 [[16 bit color texture expand]] */
			for ( x = 0 ; x < tex_data->width ; x++ ){
				int color, r, g, b, a ;
				color = *(unsigned short*)src ;
				r = color & 0x1f ;
				g = ( color >> 5 ) & 0x1f ;
				b = ( color >> 10 ) & 0x1f ;
				a = ( color & 0x8000 ) ? 0 : 128 ;
				dst[0] = r ;
				dst[1] = g ;
				dst[2] = b ;
				dst[3] = a ;
				dst[0] = gamma_table[ dst[0] ] ;
				dst[1] = gamma_table[ dst[1] ] ;
				dst[2] = gamma_table[ dst[2] ] ;
				dst += 4 ;
				src += 2 ;
			}
			break ;
		  case 3:
         /* ３２ビットカラーテクスチャ展開 [[32 bit color texture expand]] */
			for ( x = 0 ; x < tex_data->width ; x++ ){
				dst[0] = src[ 0 ] ;
				dst[1] = src[ 1 ] ;
				dst[2] = src[ 2 ] ;
				dst[3] = src[ 3 ] ;
				dst[0] = gamma_table[ dst[0] ] ;
				dst[1] = gamma_table[ dst[1] ] ;
				dst[2] = gamma_table[ dst[2] ] ;
				dst += 4 ;
				src += 4 ;
			}
			break ;
		}
	}

}
#endif



/* ---------------------------------------------------------------- */
/* 文字列コードの取得 [[Get the string code]] (Looks like a string hash) */
unsigned short MGS_GetStrCode( char *str )
{
	unsigned short	code = 0 ;
	while ( *str != 0 && *str != '.' ){
		code = ( (code<<5) | ( code>>11) ) & 0xffff ;
		if ( *str == ':' ) code = - *str ;
		code = ( code ) + (unsigned char)*str++ ;
	}
	return ( code & 0xffff );
}

/* テクスチャデータを読み込む [[Read texture data]] */
int LoadCm2File( char *name )
{
	FILE	*fp ;
	int		size, ret ;
	unsigned int	code ;

	MakeGammaTable( 1.6 );

	if ( ( fp = fopen( name, "rb" ) ) == NULL ){
		fprintf(stderr, "file open error!(%s) \n", name );
		return (-1) ;
	}
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	if ( size == 0 ) return ( -1 );

	code = MGS_GetStrCode( name ) & 0xffff ;
	ret = LoadCm2Data( fp, code, size );
	fclose( fp );

	return ( ret );
}

/* テクスチャ情報の取得 [[Get texture info]] */
TEX_INFO *GetTextureInfo( int num )
{
	if ( num >= MaxTextures ) return ( NULL ) ;
	return ( &TextureList[ num ] );
}

/* 読み込んだテクスチャデータを破棄する [[Discard the texture data read]] */
void ClearTextureData( void )
{
	int		i ;
	for ( i = 0 ; i < MaxTextures ; i++ ){
		free( TextureList[ i ].textures );
		free( TextureList[ i ].image_data );
		free( TextureList[ i ].clut_data );
#ifdef IMAGE_PREVIEW
		free( rgba_color_image[ i ] );
#endif
	}
	MaxTextures = 0 ;
}

#ifdef IMAGE_PREVIEW
/* テクスチャプレビューデータを得る [[Get texture preview data]] */
void GetTextureImage( int num, int *width, int *height, void **image_data )
{
	if ( num >= MaxTextures ){
		*image_data = NULL ;
		*width = 0 ;
		*height = 0 ;
	}
	*width = TextureList[ num ].width ;
	*height = TextureList[ num ].height ;
	*image_data = rgba_color_image[ num ] ;
}
#endif



