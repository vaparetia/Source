/*
	conv.c
	テクスチャα合成
	$Id: conv.c,v 1.5 2001/12/04 10:18:50 usr02774 Exp $

 */


#include <stdio.h>
#include <stdlib.h>

#include <MDU_tex.h>

/* ---------------------------------------------------------------- */
	/*
		テクスチャα合成
	*/
int Conv( char *out_filename, char *in_filename1, char *in_filename2 )
{
	MDU_Tex	*tex1, *tex2 ;
	int			x, y, i, j ;
	RGBQUAD		*pal1, *pal2 ;
	int			*index1, *index2 ;

	if ( ( tex1 = MDU_LoadBmp( in_filename1 ) ) == NULL ) return ( -1 );
	if ( in_filename2 == NULL ){
		pal1 = tex1->clut.data ;
		for ( j = 0 ; j < tex1->n_colors ; j++ ){
			pal1[ j ].alpha = 128 ;
		}
		MDU_SaveBmp( tex1, out_filename );
		printf("set alpha all 128\n");
		return ( 0 );
	}
	if ( ( tex2 = MDU_LoadBmp( in_filename2 ) ) == NULL ) return ( -1 );

	//if ( tex1->n_colors > 16 || tex2->n_colors > 16 ) return ( -1 );
	if ( ( tex1->n_colors * tex2->n_colors ) > 256 ){
		printf("color over!!\n");
		return ( -1 );
	}
	if ( tex1->width != tex2->width ) return ( -1 );
	if ( tex1->height != tex2->height ) return ( -1 );

	pal1 = tex1->clut.data ;
	pal2 = tex2->clut.data ;
	for ( i = 0 ; i < tex2->n_colors ; i++ ){
		for ( j = 0 ; j < tex1->n_colors ; j++ ){
			pal1[ i * tex1->n_colors + j ].r = pal1[ j ].r ;
			pal1[ i * tex1->n_colors + j ].g = pal1[ j ].g ;
			pal1[ i * tex1->n_colors + j ].b = pal1[ j ].b ;
			pal1[ i * tex1->n_colors + j ].alpha = pal2[ i ].r / 2 ;
		}
	}

	index1 = tex1->data.data ;
	index2 = tex2->data.data ;
	for ( y = 0 ; y < tex1->height ; y++ ){
		for ( x = 0 ; x < tex1->width ; x++ ){
			*index1 = *index1 | ( *index2 * tex1->n_colors ) ;
			index1++ ;
			index2++ ;
		}
	}

	tex1->n_colors = tex1->n_colors * tex2->n_colors ;
	tex1->flag |= MDU_TEX_FLAG_COLOR256 ;
	MDU_SaveBmp( tex1, out_filename );
	return ( 0 );
}

typedef struct {
	unsigned char	r, g, b, a ;
} RGBA ;


int	GetNearPalette( RGBQUAD *pal, int n_pal, int r, int g, int b, int a )
{
	int		max_len_col = 0x7fffffff ;
	int		near_col ;
	int		i, rr, gg, bb, aa, len ;

	for ( i = 0 ; i < n_pal ; i++ ){
		/* αについてチェック */
		aa = a - pal[ i ].alpha ;
		if ( aa < 0 ) aa = -aa ;
		/* カラーについてチェック */
		rr = r - pal[ i ].r ;
		gg = g - pal[ i ].g ;
		bb = b - pal[ i ].b ;
		//len = rr * rr + gg * gg + bb * bb + aa * aa ;
		len = ( rr * rr + gg * gg + bb * bb ) * a + ( aa * aa ) * ( 128 - a ) ;
		if ( max_len_col > len ){
			near_col = i ;
			max_len_col = len ;
		}
	}
	return ( near_col );
}

void MakeMipmap( MDU_Tex *low_tex, MDU_Tex *org_tex )
{
	char	*temp_buffer ;
	RGBA	*image ;
	int		width, height, i ;

	width = org_tex->width ;
	height = org_tex->height ;

	MDU_AllocTex( low_tex, org_tex->width / 2, org_tex->height / 2 );
	low_tex->n_colors = org_tex->n_colors ;
	low_tex->flag = org_tex->flag ;
	low_tex->width = org_tex->width / 2 ;
	low_tex->height = org_tex->height / 2 ;
	{
		RGBQUAD			*src_pal, *dst_pal ;
		src_pal = org_tex->clut.data ;
		dst_pal = low_tex->clut.data ;
		for ( i = 0 ; i < org_tex->n_colors ; i++ ){
			dst_pal[ i ] = src_pal[ i ] ;
		}
	}

	image = malloc( org_tex->width * org_tex->height * 4 * 2 );
	memset( image, 0, org_tex->width * org_tex->height * 4 * 2 );

	{
		int		x, y, index, n ;
		unsigned int	*img ;
		RGBQUAD			*pal ;
		img = org_tex->data.data ;
		pal = org_tex->clut.data ;
		for ( y = 0 ; y < org_tex->height ; y++ ){
			for ( x = 0 ; x < org_tex->width ; x++ ){
				n = x + y * org_tex->width ;
				index = img[ n ] ;
				image[ n ].r = pal[ index ].r ;
				image[ n ].g = pal[ index ].g ;
				image[ n ].b = pal[ index ].b ;
				image[ n ].a = pal[ index ].alpha ;
			}
		}
	}

	{
		RGBA	*upper_image ;
		RGBA	*current_image ;
		int				i, x, y, w, pitch ;

		pitch = org_tex->width ;
		upper_image = image ;
		current_image = &image[ org_tex->width * org_tex->height ] ;
		for ( i = 1 ; i < 4 ; i++ ){
			for ( y = 0 ; y < ( org_tex->height >> i ) ; y++ ){
				for ( x = 0 ; x < ( org_tex->width >> i ) ; x++ ){
					int		r, g, b, a, base ;
					base = x * 2 + y * 2 * pitch ;
					r  = upper_image[ base + 0 + pitch * 0 ].r ;
					r += upper_image[ base + 1 + pitch * 0 ].r ;
					r += upper_image[ base + 0 + pitch * 1 ].r ;
					r += upper_image[ base + 1 + pitch * 1 ].r ;
					g  = upper_image[ base + 0 + pitch * 0 ].g ;
					g += upper_image[ base + 1 + pitch * 0 ].g ;
					g += upper_image[ base + 0 + pitch * 1 ].g ;
					g += upper_image[ base + 1 + pitch * 1 ].g ;
					b  = upper_image[ base + 0 + pitch * 0 ].b ;
					b += upper_image[ base + 1 + pitch * 0 ].b ;
					b += upper_image[ base + 0 + pitch * 1 ].b ;
					b += upper_image[ base + 1 + pitch * 1 ].b ;
					a  = upper_image[ base + 0 + pitch * 0 ].a ;
					a += upper_image[ base + 1 + pitch * 0 ].a ;
					a += upper_image[ base + 0 + pitch * 1 ].a ;
					a += upper_image[ base + 1 + pitch * 1 ].a ;
					r >>= 2 ;
					g >>= 2 ;
					b >>= 2 ;
					a >>= 2 ;
					current_image[ x + y * pitch ].r = r ;
					current_image[ x + y * pitch ].g = g ;
					current_image[ x + y * pitch ].b = b ;
					current_image[ x + y * pitch ].a = a ;
				}
			}
			upper_image = current_image ;
			current_image = &current_image[ y * pitch ] ;
		}
	}

	{
		int		x, y, index, base, n ;
		unsigned int	*img ;
		RGBQUAD	*pal ;
		pal = org_tex->clut.data ;
		img = low_tex->data.data ;
		for ( y = 0 ; y < low_tex->height ; y++ ){
			for ( x = 0 ; x < low_tex->width ; x++ ){
				base = x + ( y + org_tex->height ) * width ;
				index = GetNearPalette( pal, org_tex->n_colors,
									  image[ base ].r,
									  image[ base ].g,
									  image[ base ].b,
									  image[ base ].a );
				image[ base ].r = pal[ index ].r ;
				image[ base ].g = pal[ index ].g ;
				image[ base ].b = pal[ index ].b ;
				image[ base ].a = pal[ index ].alpha ;
				n = x + y * low_tex->width ;
				img[ n ] = index ;
			}
		}
	}

}


int Conv2( char *out_filename, char *in_filename1 )
{
	MDU_Tex	*tex1 ;
	MDU_Tex	mip_tex ;

	if ( ( tex1 = MDU_LoadBmp( in_filename1 ) ) == NULL ) return ( -1 );

	MakeMipmap( &mip_tex, tex1 );

	MDU_SaveBmp( &mip_tex, out_filename );
	return ( 0 );
}
