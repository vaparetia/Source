/*
	$Id: loadtpk.c,v 1.1 2002/06/19 05:48:55 usr02774 Exp $
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "loadinc.h"

static int	tex_mode[4] = { 0x14, 0x13, 0x02, 0x00 };	/* モード値－＞ＰＳＭ値変換テーブル */

int			MaxTexpack = 0 ;
TEX_INFO	TexpackList[ 16 ] ;

/* ---------------------------------------------------------------- */
static int GetDataSize( int width, int height, int format )
{
	int		size ;
	switch ( format ){
	case TEXPACK_FORMAT_A8R8G8B8:
	case TEXPACK_FORMAT_A8G8G8R8:
		size = width * height * 4 ;
		break ;
	case TEXPACK_FORMAT_A4R4G4B4:
	case TEXPACK_FORMAT_A1R5G5B5:
	case TEXPACK_FORMAT_A1G5G5R5:
	case TEXPACK_FORMAT_V8U8:
		size = width * height * 2 ;
		break ;
	case TEXPACK_FORMAT_PAL4:
	case TEXPACK_FORMAT_PAL4AGBR:
		size = width * height / 2 ;
		break ;
	case TEXPACK_FORMAT_PAL8:
	case TEXPACK_FORMAT_PAL8AGBR:
		size = width * height ;
		break ;
	case TEXPACK_FORMAT_DXT1:
		size = width * height / 2 ;
		break ;
	case TEXPACK_FORMAT_DXT3:
	case TEXPACK_FORMAT_DXT5:
		size = width * height ;
		break ;
	}
	return ( size );
}
/* ---------------------------------------------------------------- */
static int LoadTpkData( FILE *fp, int code, int size )
{
	TEXPACK_HEADER	*tmp_header ;
	TEX_INFO		*tpk_info ;
	TEXPACK_INFO	*tex_datas ;
	int				i, j, image_size, clut_size ;
	unsigned long64	tex ,tex2 ;

	/* テンポラリのワークに一度全部読み込む */
	tmp_header = malloc( size );
	fread( tmp_header, 1, size, fp );

	tex_datas = (TEXPACK_INFO*)( (char*)tmp_header + (int)tmp_header->info );

	tpk_info = &TexpackList[ MaxTexpack ] ;
	tpk_info->width = tmp_header->width ;
	tpk_info->height = tmp_header->height ;
	tpk_info->n_textures = tmp_header->n_info ;
	tpk_info->mode = tmp_header->format ;
	tpk_info->mip_level = tmp_header->mip_level ;
	/* デフォルトではミップマップレベルを１に設定しておく */
	if ( tpk_info->mip_level == 0 ) tpk_info->mip_level = 1 ;

	/* 記憶領域の確保 */
	tpk_info->textures = malloc( sizeof( DG_TEX ) * tpk_info->n_textures );
	tpk_info->image_size = image_size = GetDataSize( tpk_info->width, tpk_info->height, tmp_header->format );
	/* イメージデータ領域の確保 */
	tpk_info->image_data = malloc( image_size );
	memset( tpk_info->image_data, 0, image_size );
	/* パレット領域の確保 */
	tpk_info->clut_data = NULL ;
	clut_size = 0 ;
	tpk_info->clut_size = clut_size * tpk_info->n_textures * 4 ;
	if ( clut_size != 0 ){
		tpk_info->clut_data = malloc( tpk_info->clut_size );
		memset( tpk_info->clut_data, 0, tpk_info->clut_size );
	}

	{/* ２のｎ乗とするサイズを求める */
		unsigned int	twe = 0, the = 0, tw, th ;
		tw = 1 ; i = tpk_info->width - 1 ;
		while( i > 0 ){ i >>= 1 ; tw <<= 1 ; twe++ ; }
		th = 1 ; i = tpk_info->height - 1 ;
		while( i > 0 ){ i >>= 1 ; th <<= 1 ; the++ ; }
		tpk_info->w_exp = twe ;
		tpk_info->h_exp = the ;
	}

	{/* テクセルデータのコピー */
		int		*src, *dst ;
		int		count = 0 ;

		dst = tpk_info->image_data ;
		src = (int*)( (char*)tmp_header->texel_addr + (int)tmp_header );
		for ( i = image_size / 4 ; i > 0 ; i-- ){
			*dst = *src ;
			dst++ ;
			src++ ;
		}
	}

#if 0
	/* パレットデータのコピー */
	if ( ( tmp_header->mode & 7 ) <= 1 ){
		int		*src, *dst, x, y, offset ;
		TexData2	*tex_data ;

		tex_data = tex_datas ;
		for ( i = 0 ; i < tpk_info->n_textures ; i++, tex_data++ ){
			src = (int*)( (int)(tex_data->clut_addr) + (int)tmp_header );
			dst = (int*)tpk_info->clut_data + i * clut_size ;
			for ( j = 0 ; j < clut_size ; j++ ){
				offset = j ;
				dst[ offset ] = *src ;
				src++ ;
			}
		}
	}
#endif

	{/* テクスチャ情報の作成 */
		/* ベースとなるテクスチャ設定値を生成 */
		tex = 0 ;/* ベースアドレス */
		tex |= ( tpk_info->width / 64 ) << 14 ;	/* TBW */
		//tex |= tex_mode[ ptk_info->mode & 7 ] << 20 ;	/* PSM */
		tex |= (unsigned long64)tpk_info->w_exp << 26 ;	/* TW */
		tex |= (unsigned long64)tpk_info->h_exp << 30 ;	/* TH */
		tex |= (unsigned long64)1 << 34 ;	/* TCC(RGBA) */
		tex |= (unsigned long64)0 << 35 ;	/* TFX(MODULATE) */
		tex |= (unsigned long64)0 << 37 ;	/* CBP */
		tex |= (unsigned long64)0 << 51 ;	/* CPSM(PSMCT32) */
		tex |= (unsigned long64)0 << 55 ;	/* CSM(CSM1) */
		tex |= (unsigned long64)0 << 56 ;	/* CSA */
		tex |= (unsigned long64)0 << 61 ;	/* CLD(???) */

		tex2 = 0;	/* TEX2レジスタ */
		tex2 |= tex_mode[ 1 ] << 20 ;		/* PSM TEX2は 8bit mode 固定 */
		tex2 |= (unsigned long64)0 << 37 ;	/* CBP */
		tex2 |= (unsigned long64)0 << 51 ;	/* CPSM(PSMCT32) */
		tex2 |= (unsigned long64)0 << 55 ;	/* CSM(CSM1) */
		tex2 |= (unsigned long64)0 << 56 ;	/* CSA */
		tex2 |= (unsigned long64)4 << 61 ;	/* CLD(???) */
	}

	{/* ヘッダー及び各テクスチャ情報をコピーする */
		TEXPACK_INFO	*src_tex ;
		DG_TEX		*texture ;
		float		tex_width, tex_height ;
		int			u_max, u_min, v_max, v_min ;
		unsigned long64	clut_base, clut_offset ;

		tex_width = (float)tpk_info->width ;
		tex_height = (float)tpk_info->height ;
		//clut_offset = clut_offset_data[ tmp_header->mode & 7 ] ;
		clut_base = 0 ;/* 注）ＣＬＵＴベースアドレス／４を意味する */
		tpk_info->max_x = 0 ; tpk_info->max_y  = 0 ;

		texture = tpk_info->textures ;
		src_tex = tex_datas ;
		for ( i = tpk_info->n_textures ; i > 0 ; i--, src_tex++, texture++ ){
			u_min = src_tex->x ;
			v_min = src_tex->y ;
			u_max = src_tex->x + src_tex->w ;
			v_max = src_tex->y + src_tex->h ;
			if ( tpk_info->max_x < u_max ) tpk_info->max_x = u_max ;
			if ( tpk_info->max_y < v_max ) tpk_info->max_y = v_max ;
			//tpk_info->max_x = ( tpk_info->max_x + 0x3 ) & ~0x3 ;	/* 念のため４の倍数に補正 */
			//tpk_info->max_y = ( tpk_info->max_y + 0x3 ) & ~0x3 ;	/* 念のため４の倍数に補正 */
			memset( texture, 0, sizeof(DG_TEX) );
			texture->tex_id = src_tex->id ;
			texture->flag = src_tex->flag ;
			texture->texinfonum = MaxTexpack ;
			texture->u_offset = ( (float)src_tex->x + 0.5F ) / tex_width ;
			texture->v_offset = ( (float)src_tex->y + 0.5F ) / tex_height ;
			texture->u_scale = ( (float)src_tex->w - 1.0F ) / tex_width ;
			texture->v_scale = ( (float)src_tex->h - 1.0F ) / tex_height ;
			/* GIF設定 */
			//texture->tex_trans.giftag.tag = SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7 );
			//texture->tex_trans.giftag.regs = 0x0fffeeee ;
			texture->tex_trans.clamp.reg = SCE_GS_CLAMP_1 ;
			texture->tex_trans.tex2.reg = SCE_GS_TEX2_1 ;
			texture->tex_trans.tex0.reg = SCE_GS_TEX0_1 ;
			texture->tex_trans.alpha.reg = SCE_GS_ALPHA_1 ;
			//texture->tex_trans.pad[0].reg = SCE_GS_NOP ;
			texture->tex_trans.clamp.data = SCE_GS_SET_CLAMP( 2, 2, u_min, u_max-1, v_min, v_max-1 );
			texture->tex_trans.tex2.data = tex2 ;
			texture->tex_trans.tex0.data = tex ;
			if ( src_tex->flag & TEXINFO_FLAG_DECAL ){
				texture->tex_trans.tex0.data |= (unsigned long64)1 << 35 ;	/* TFX(DECAL) */
				printf("decal flag on(%d)\n", src_tex->id );
			}
			/* αモード設定 */
			switch ( src_tex->flag & TEXINFO_FLAG_BLEND_MASK ){
			case  TEXINFO_FLAG_BLEND_NONE:
				printf("check texture (%d)\n", src_tex->id);
				texture->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 0, 0, 0, 0 );
				break ;
			case  TEXINFO_FLAG_BLEND_HLF:
				printf("check texture half (%d)\n", src_tex->id);
				texture->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 );
				break ;
			case TEXINFO_FLAG_BLEND_ADD:/* add */
				printf("check texture add (%d)\n", src_tex->id);
				texture->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 );
				break ;
			case TEXINFO_FLAG_BLEND_SUB:/* sub */
				printf("check texture sub (%d)\n", src_tex->id);
				texture->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 );
				break ;
			case TEXINFO_FLAG_BLEND_AMODE:/* amode */
				texture->tex_trans.alpha.data = (unsigned long64)( ( src_tex->flag & TEXINFO_FLAG_AMODE_MASK ) >> 24 );
				printf("check texture blend (%d) %04x\n", src_tex->id, texture->tex_trans.alpha.data );
				break ;
			}
			/* テクスチャモード設定 */
#if 0
			if ( src_tex->flag & TEXINFO_FLAG_EMAP ){
				printf("emap texture (%d)\n", src_tex->id );
				//texture->flag |= TRI_TEXTAG_FLAG_EMAP ;
			}
			if ( src_tex->flag & TEXINFO_FLAG_SMAP ){
				printf("smap texture (%d)\n", src_tex->id );
				//texture->flag |= TRI_TEXTAG_FLAG_SMAP ;
			}
			if ( src_tex->flag & TEXINFO_FLAG_BMAP ){
				printf("bmap texture (%d)\n", src_tex->id );
				//texture->flag |= TRI_TEXTAG_FLAG_BMAP ;
			}
			/* テクスチャ固有ベクトルデータ設定 */
			if ( texture->flag & TRI_TEXTAG_FLAG_BMAP ){
				/* バンプマッピングテクスチャ時の設定（テクスチャシフト量パラメータ） */
				int		t_size ;
				t_size = 1 << tpk_info->w_exp ;
				texture->tex_trans.vec1.vx = -1.0f / (float)t_size ;
				t_size = 1 << tpk_info->h_exp ;
				texture->tex_trans.vec1.vy = -1.0f / (float)t_size ;
			} else {
				/* 環境マップテクスチャ（バンプマッピング以外）時の設定 */
				texture->tex_trans.vec1.vx = -0.4999f * texture->u_scale ;
				texture->tex_trans.vec1.vy = -0.4999f * texture->v_scale ;
				texture->tex_trans.vec1.vz = 1.0f ;
				texture->tex_trans.vec1.vw = 1.0f ;
				texture->tex_trans.vec2.vx = 0.5f * texture->u_scale + texture->u_offset ;
				texture->tex_trans.vec2.vy = 0.5f * texture->v_scale + texture->v_offset ;
				texture->tex_trans.vec2.vz = 0.0f ;
				texture->tex_trans.vec2.vw = 0.0f ;
			}
#else
			switch ( src_tex->flag & TEXINFO_FLAG_MAPMASK ){
			case TEXINFO_FLAG_EMAP:
				printf("emap texture (%d)\n", src_tex->id );
				/* 環境マップテクスチャ（バンプマッピング以外）時の設定 */
				texture->tex_trans.vec1.vx = -0.4999f * texture->u_scale ;
				texture->tex_trans.vec1.vy = -0.4999f * texture->v_scale ;
				texture->tex_trans.vec1.vz = 1.0f ;
				texture->tex_trans.vec1.vw = 1.0f ;
				texture->tex_trans.vec2.vx = 0.5f * texture->u_scale + texture->u_offset ;
				texture->tex_trans.vec2.vy = 0.5f * texture->v_scale + texture->v_offset ;
				texture->tex_trans.vec2.vz = 0.0f ;
				texture->tex_trans.vec2.vw = 0.0f ;
				break ;
			case TEXINFO_FLAG_SMAP:
				printf("smap texture (%d)\n", src_tex->id );
				/* 環境マップテクスチャ（バンプマッピング以外）時の設定 */
				texture->tex_trans.vec1.vx = -0.4999f * texture->u_scale ;
				texture->tex_trans.vec1.vy = -0.4999f * texture->v_scale ;
				texture->tex_trans.vec1.vz = 1.0f ;
				texture->tex_trans.vec1.vw = 1.0f ;
				texture->tex_trans.vec2.vx = 0.5f * texture->u_scale + texture->u_offset ;
				texture->tex_trans.vec2.vy = 0.5f * texture->v_scale + texture->v_offset ;
				texture->tex_trans.vec2.vz = 0.0f ;
				texture->tex_trans.vec2.vw = 0.0f ;
				break ;
			case TEXINFO_FLAG_BMAP:
				printf("bmap texture (%d)\n", src_tex->id );
				{/* バンプマッピングテクスチャ時の設定（テクスチャシフト量パラメータ） */
					int		t_size ;
					t_size = 1 << tpk_info->w_exp ;
					texture->tex_trans.vec1.vx = -1.0f / (float)t_size ;
					t_size = 1 << tpk_info->h_exp ;
					texture->tex_trans.vec1.vy = -1.0f / (float)t_size ;
				}
				break ;
			default:
				texture->tex_trans.vec1.vx = texture->u_scale ;
				texture->tex_trans.vec1.vy = texture->v_scale ;
				texture->tex_trans.vec1.vz = 1.0f ;
				texture->tex_trans.vec1.vw = 1.0f ;
				texture->tex_trans.vec2.vx = texture->u_offset ;
				texture->tex_trans.vec2.vy = texture->v_offset ;
				texture->tex_trans.vec2.vz = 0.0f ;
				texture->tex_trans.vec2.vw = 0.0f ;
				break ;
			}
#endif
			//clut_base += clut_offset ;
		}
	}

	free( tmp_header );

	MaxTexpack++ ;

	return (MaxTexpack-1);
}



/* ---------------------------------------------------------------- */
/* 文字列コードの取得 */
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

/* テクスチャデータを読み込む */
int LoadTpkFile( char *name )
{
	FILE	*fp ;
	int		size, ret ;
	unsigned int	code ;

	if ( ( fp = fopen( name, "rb" ) ) == NULL ){
		fprintf(stderr, "file open error!(%s) \n", name );
		return (-1) ;
	}
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	if ( size == 0 ) return ( -1 );

	code = MGS_GetStrCode( name ) & 0xffff ;
	ret = LoadTpkData( fp, code, size );
	fclose( fp );

	return ( ret );
}

/* テクスチャ情報の取得 */
TEX_INFO *GetTexpackInfo( int num )
{
	if ( num >= MaxTexpack ) return ( NULL ) ;
	return ( &TexpackList[ num ] );
}

/* 読み込んだテクスチャデータを破棄する */
#define safe_free( _p ) { if ( (_p) != NULL ) free( _p ); (_p) = NULL ; }
void ClearTextureData( void )
{
	int		i ;
	for ( i = 0 ; i < MaxTexpack ; i++ ){
		safe_free( TexpackList[ i ].textures );
		safe_free( TexpackList[ i ].image_data );
		safe_free( TexpackList[ i ].clut_data );
	}
	MaxTexpack = 0 ;
}




