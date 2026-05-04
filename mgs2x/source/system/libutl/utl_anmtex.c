//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	utl_anmtex.c
	アニメーションテクスチャ展開処理

	2000/06/02 K.Takabe
	$Id: utl_anmtex.c,v 1.2 2002/12/18 05:27:43 takaki Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libutl.h"

#include	"utl_anmtex.h"

#include "BP_EndianSupport.h"

/* ------------------------------------------------------------ */
/* データバッファ操作関連 */
/* 指定したビット長の数字を入れる(MAX bit=8) */
static inline void PushData( int bit, int n, unsigned char *datas, int *data_bit_pos )
{
	int bit_offset,short_offset;
	unsigned int work,data,mask;

	bit_offset = (*data_bit_pos) & 0x07 ;
	short_offset = (*data_bit_pos) >> 3 ;
	mask = ((unsigned int)1 << bit) - 1;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 8 );
	data = ( (unsigned int)( n & mask ) ) << bit_offset ;
	work &= ~( ( (unsigned int)mask ) << bit_offset ) ;
	work |= data;
	datas[ short_offset   ] = work & 0xff;
	datas[ short_offset+1 ] = (work >> 8) & 0xff;
	*data_bit_pos += bit;
}
static inline void PushData1(  int n, unsigned char *datas, int *data_bit_pos )
{
	int bit_offset,short_offset;
	unsigned int work,data,mask;

	bit_offset = (*data_bit_pos) & 0x07 ;
	short_offset = (*data_bit_pos) >> 3 ;
	mask = 0x01 ;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 8 );
	data = ( (unsigned int)( n & mask ) ) << bit_offset ;
	work &= ~( ( (unsigned int)mask ) << bit_offset ) ;
	work |= data;
	datas[ short_offset   ] = work & 0xff;
	datas[ short_offset+1 ] = (work >> 8) & 0xff;
	*data_bit_pos += 1;
}
static inline void PushData4(  int n, unsigned char *datas, int *data_bit_pos )
{
	int bit_offset,short_offset;
	unsigned int work,data,mask;

	bit_offset = (*data_bit_pos) & 0x07 ;
	short_offset = (*data_bit_pos) >> 3 ;
	mask = 0x0f ;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 8 );
	data = ( (unsigned int)( n & mask ) ) << bit_offset ;
	work &= ~( ( (unsigned int)mask ) << bit_offset ) ;
	work |= data;
	datas[ short_offset   ] = work & 0xff;
	datas[ short_offset+1 ] = (work >> 8) & 0xff;
	*data_bit_pos += 4;
}
static inline void PushData8(  int n, unsigned char *datas, int *data_bit_pos )
{
	int bit_offset,short_offset;
	unsigned int work,data,mask;

	bit_offset = (*data_bit_pos) & 0x07 ;
	short_offset = (*data_bit_pos) >> 3 ;
	mask = 0xff ;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 8 );
	data = ( (unsigned int)( n & mask ) ) << bit_offset ;
	work &= ~( ( (unsigned int)mask ) << bit_offset ) ;
	work |= data;
	datas[ short_offset   ] = work & 0xff;
	datas[ short_offset+1 ] = (work >> 8) & 0xff;
	*data_bit_pos += 8;
}
/* 指定したビット長の数字を入れる(MAX bit=8) */
static inline int PopData( int bit, unsigned char *datas, int *data_bit_pos )
{
	int bit_offset,short_offset;
	unsigned int work,mask;

	bit_offset = (*data_bit_pos) & 0x07 ;
	short_offset = (*data_bit_pos) >> 3 ;
	mask = ((unsigned int)1 << bit) - 1;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 8 );
	work >>= bit_offset ;
	work &= mask ;
	*data_bit_pos += bit;
	return ( work );
}
static inline int PopData1( unsigned char *datas, int *data_bit_pos )
{
	int bit_offset,short_offset ;
	unsigned int work ;

	bit_offset = (*data_bit_pos) & 0x07 ;
	short_offset = (*data_bit_pos) >> 3 ;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 8 );
	work >>= bit_offset ;
	work &= 0x01 ;
	*data_bit_pos += 1;
	return ( work );
}
static inline int PopData4( unsigned char *datas, int *data_bit_pos )
{
	int bit_offset, short_offset ;
	unsigned int work ;

	bit_offset = (*data_bit_pos) & 0x07 ;
	short_offset = (*data_bit_pos) >> 3 ;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 8 );
	work >>= bit_offset ;
	work &= 0x0f ;
	*data_bit_pos += 4;
	return ( work );
}
static inline int PopData8( unsigned char *datas, int *data_bit_pos )
{
	int bit_offset, short_offset ;
	unsigned int work ;

	bit_offset = (*data_bit_pos) & 0x07 ;
	short_offset = (*data_bit_pos) >> 3 ;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 8 );
	work >>= bit_offset ;
	work &= 0xff ;
	*data_bit_pos += 8;
	return ( work );
}
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* 展開処理 */
static int Decompress( unsigned char *out_buffer, unsigned char *in_buffer, int expand_size )
{
    int	i, len, size, use_size ;
    unsigned char	status, *refer_buffer ;

    /* ラインデータ展開 */
	size = 0 ;
	use_size = 0 ;
	while ( size < expand_size ){
		status = *in_buffer++ ;
		use_size++ ;
		//if ( status == 0x80 ) break ;	/* 終端チェック */
		len = status & 0x7f ;
		if ( status & 0x80 ){
			/* 圧縮データ展開 */
			int		offset ;
			offset = (unsigned char)*in_buffer++ ;
			refer_buffer = out_buffer - offset ;
			for ( i = 0 ; i < len ; i++ ){
				*out_buffer++ = *refer_buffer++ ;
			}
			use_size += 1 ;
		} else {
			/* 無圧縮データ列展開 */
			for ( i = 0 ; i < len ; i++ ){
				*out_buffer++ = *in_buffer++ ;
			}
			use_size += len ;
		}
		size += len ;
	}
	return ( use_size );
}

/* 継続フラグ１ビット＋データ４ビットで記録 */
static int Decompress2_16( unsigned char *out_buffer, unsigned char *in_buffer, int expand_size )
{
	int		size, data, bit ;
	int		read_bit_offset ;

	data = 0 ;
	size = 0 ;
	read_bit_offset = 0 ;
	while ( size < expand_size ){
		bit = PopData1( in_buffer, &read_bit_offset );
		if ( bit == 0 ){
			data = PopData4( in_buffer, &read_bit_offset );
			*out_buffer = data ;
		} else {
			*out_buffer = data ;
		}

		bit = PopData1( in_buffer, &read_bit_offset );
		if ( bit == 0 ){
			data = PopData4( in_buffer, &read_bit_offset );
			*out_buffer++ |= data << 4 ;
		} else {
			*out_buffer++ |= data << 4 ;
		}
		size++ ;
	}

	return ( ( read_bit_offset + 7 ) / 8 );
}
/* 継続フラグ１ビット＋データ８ビットで記録 */
static int Decompress2_256( unsigned char *out_buffer, unsigned char *in_buffer, int expand_size )
{
	int		size, data, bit ;
	int		read_bit_offset ;

	data = 0 ;
	size = 0 ;
	read_bit_offset = 0 ;
	while ( size < expand_size ){
		bit = PopData( 1, in_buffer, &read_bit_offset );
		if ( bit == 0 ){
			data = PopData( 8, in_buffer, &read_bit_offset );
			*out_buffer++ = data ;
		} else {
			*out_buffer++ = data ;
		}
		size++ ;
	}

	return ( ( read_bit_offset + 7 ) / 8 );
}

/* 継続フラグ１ビット＋データ４ビットで記録 */
static int Decompress3_16( unsigned char *out_buffer, unsigned char *in_buffer, int expand_size, unsigned char *compress_buffer2 )
{
	int		size, data, bit, offset ;
	int		read_bit_offset, read_bit_offset2 ;

	offset = 0 ;
	offset = Decompress( compress_buffer2, in_buffer, ( expand_size + 3 ) / 4 ) ;
	data = 0 ;
	size = 0 ;
	read_bit_offset = offset * 8 ;
	read_bit_offset2 = 0 ;
	while ( size < expand_size ){
		bit = PopData1( compress_buffer2, &read_bit_offset2 );
		if ( bit == 0 ){
			data = PopData4( in_buffer, &read_bit_offset );
			*out_buffer = data ;
		} else {
			*out_buffer = data ;
		}

		bit = PopData1( compress_buffer2, &read_bit_offset2 );
		if ( bit == 0 ){
			data = PopData4( in_buffer, &read_bit_offset );
			*out_buffer++ |= data << 4 ;
		} else {
			*out_buffer++ |= data << 4 ;
		}
		size++ ;
	}

	return ( ( read_bit_offset + 7 ) / 8 );
}

static int Decompress3_256( unsigned char *out_buffer, unsigned char *in_buffer, int expand_size, unsigned char *compress_buffer2 )
{
	int		size, data, bit, offset ;
	int		read_bit_offset, read_bit_offset2 ;

	offset = 0 ;
	offset = Decompress( compress_buffer2, in_buffer, ( expand_size + 7 ) / 8 ) ;
	data = 0 ;
	size = 0 ;
	read_bit_offset = offset * 8 ;
	read_bit_offset2 = 0 ;
	while ( size < expand_size ){
		bit = PopData1( compress_buffer2, &read_bit_offset2 );
		if ( bit == 0 ){
			data = PopData8( in_buffer, &read_bit_offset );
			*out_buffer++ = data ;
		} else {
			*out_buffer++ = data ;
		}
		size++ ;
	}

	return ( ( read_bit_offset + 7 ) / 8 );
}


/* ４ビットランレングス */
#define GET_HALF_LOW( _data )	( ( _data ) & 0x0f )
#define GET_HALF_HIGH( _data )	( ( ( _data ) >> 4 ) & 0x0f )
#define GET_HALF_DATA( _buffer, _offset )	( ((_offset)&1) ? GET_HALF_HIGH( (_buffer)[ (_offset) / 2 ] ) : GET_HALF_LOW( (_buffer)[ (_offset) / 2 ] ) )
static int Decompress4_16( unsigned char *out_buffer, unsigned char *in_buffer, int expand_size )
{
	int		size, data, len, i ;
	int		read_bit_offset, write_bit_offset ;

	data = 0 ;
	size = 0 ;
	read_bit_offset = 0 ;
	write_bit_offset = 0 ;
	expand_size *= 2 ;
	while ( size < expand_size ){
		len = PopData4( in_buffer, &read_bit_offset );
		if ( len & 0x08 ){
			/* 圧縮部展開 */
			data = PopData4( in_buffer, &read_bit_offset );
			len = ( len & 0x07 ) + 3 ;
			for ( i = 0 ; i < len ; i++ ){
				PushData4( data, out_buffer, &write_bit_offset );
			}
			size += len ;
		} else {
			/* 無圧縮部展開 */
			len = ( len & 0x07 ) + 1 ;
			for ( i = 0 ; i < len ; i++ ){
				data = PopData4( in_buffer, &read_bit_offset );
				PushData4( data, out_buffer, &write_bit_offset );
			}
			size += len ;
		}
	}

	return ( ( read_bit_offset + 7 ) / 8 );
}


/* ８ビットランレングス */
static int Decompress4_256( unsigned char *out_buffer, unsigned char *in_buffer, int expand_size )
{
	int		size, data, len, i ;
	unsigned char	*top_ptr = in_buffer ;

	data = 0 ;
	size = 0 ;
	while ( size < expand_size ){
		len = *in_buffer++ ;
		if ( len & 0x80 ){
			/* 圧縮部展開 */
			data = *in_buffer++ ;
			len = ( len & 0x7f ) + 3 ;
			for ( i = 0 ; i < len ; i++ ){
				*out_buffer++ = data ;
			}
			size += len ;
		} else {
			/* 無圧縮部展開 */
			len = ( len & 0x7f ) + 1 ;
			for ( i = 0 ; i < len ; i++ ){
				*out_buffer++ = *in_buffer++ ;
			}
			size += len ;
		}
	}

	return ( (int)in_buffer - (int)top_ptr );
}


/* ---------------------------------------------------------------- */
/* バッファ間XORをとる */
static void XorBuffer( unsigned char *out_image, unsigned char *in_image, unsigned char *before_image, int size )
{
	while ( size > 0 ){
		*out_image++ = *in_image++ ^ *before_image++ ;
		size-- ;
	}
}
/* ---------------------------------------------------------------- */
static void SetupCompressBuffer( unsigned char *image, unsigned char *before_image, int size, int type )
{
	switch ( type ){
	case 0:/* そのまま */
		break ;
	case 1:/* 前フレームとのXOR */
		XorBuffer( image, image, before_image, size );
		break ;
	}
}
static void DecompressBuffer( unsigned char *image, unsigned char *compress_buffer, int size, int mode, int type )
{
	switch ( type ){
	case 0:/*  */
		Decompress( image, compress_buffer, size );
		break ;
	case 1:/*  */
		switch ( mode ){
		case 0:
			Decompress2_16( image, compress_buffer, size );
			break ;
		case 1:
			Decompress2_256( image, compress_buffer, size );
			break ;
		}
		break ;
	case 2:/*  */
		switch ( mode ){
		case 0:
			Decompress3_16( image, compress_buffer, size, SCRPAD_ADDR );
			break ;
		case 1:
			Decompress3_256( image, compress_buffer, size, SCRPAD_ADDR );
			break ;
		}
		break ;
	case 3:/*  */
		switch ( mode ){
		case 0:
			Decompress4_16( image, compress_buffer, size );
			break ;
		case 1:
			Decompress4_256( image, compress_buffer, size );
			break ;
		}
		break ;
	}
}

static int UTL_LoadInitAnmTex( void *buf )
{
   int frameIndex;
   ANMTEX_HEADER *pHeader = (ANMTEX_HEADER *) buf;

   if ( BP_LE_SwapSInt( pHeader->flag_le ) & ANMTEX_HEADER_FLAG_ENDIANSWAPPED )
   {
      return 1;
   }

   // Time to endian swap!

   pHeader->flag_le |= BP_LE_SwapSInt( ANMTEX_HEADER_FLAG_ENDIANSWAPPED );

   BP_LE_SwapSInt_Inp( &( pHeader->format_id ) );
   BP_LE_SwapSInt_Inp( &( pHeader->width ) );
   BP_LE_SwapSInt_Inp( &( pHeader->height ) );
   BP_LE_SwapSInt_Inp( &( pHeader->frames ) );
   BP_LE_SwapSInt_Inp( &( pHeader->pad[0] ) );
   BP_LE_SwapSInt_Inp( &( pHeader->pad[1] ) );
   BP_LE_SwapSInt_Inp( &( pHeader->pad[2] ) );

   for ( frameIndex = 0; frameIndex < pHeader->frames; ++frameIndex )
   {
      ANMTEX_INFO *pInfo = pHeader->info + frameIndex;

      BP_LE_SwapSInt_Inp( &( pInfo->compress_flag ) );
      BP_LE_SwapSInt_Inp( &( pInfo->data_offset ) );
      BP_LE_SwapSInt_Inp( &( pInfo->compress_size ) );
      BP_LE_SwapSInt_Inp( &( pInfo->palette_offset ) );
   }

   // Endian swap palette data
   if( pHeader->frames > 0 )
   {
      unsigned char * pDataPtr = (unsigned char *)( (int)pHeader + sizeof(ANMTEX_HEADER) + sizeof(ANMTEX_INFO) * pHeader->frames );
      unsigned int * pPaletteDataCur = (unsigned int*)pDataPtr;
      unsigned int * pPaletteDataEnd = (unsigned int*)(pDataPtr + pHeader->info[0].data_offset);

      while( pPaletteDataCur < pPaletteDataEnd )
      {
         unsigned int color = BP_LE_SwapSInt(*pPaletteDataCur);

         // Swap R/B
         color = (color & 0xFF00FF00) | ((color & 0x00FF0000) >> 16) | ((color & 0x000000FF) << 16);

         *pPaletteDataCur++ = color;
      }
   }
   return 1;
}

/* ---------------------------------------------------------------- */
	/*
		アニメテクスチャ再生ワーク初期化
	*/
int UTL_InitAnmtexWork( ANMTEX_WORK *anmtex_work, int tri_id, int tex_id, int anmtex_id, int flag )
{
	ANMTEX_HEADER	*header ;
	DG_TEX	*tex ;
	int		width, height ;

	GV_ZeroMemory( anmtex_work, sizeof(ANMTEX_WORK) );
	anmtex_work->tri_id = tri_id ;
	anmtex_work->tex_id = tex_id ;
	anmtex_work->anmtex_id = anmtex_id ;
	anmtex_work->flag = flag ;
	anmtex_work->last_set_buffer = -1 ;

	anmtex_work->tex = tex = DG_GetTexture2( tri_id, tex_id );

#ifdef KP_WINDOWS
	anmtex_work->last_frame_pcntr = DG_FramePresentCount-1 ;
#endif

	/* アニメテクスチャを取得する */
	header = anmtex_work->anmtex_header = GV_GetCache( GV_CacheID( anmtex_id, 'r' ) );
	if ( header == NULL ){
		printf("%s:anmtex not found!!(%d)\n", __FILE__, anmtex_id );
		return ( -1 );
	}

   UTL_LoadInitAnmTex( header );

	/* データ先頭ポインタの取得 */
	anmtex_work->data_ptr =
	  (unsigned char*)( (int)header + sizeof(ANMTEX_HEADER) + sizeof(ANMTEX_INFO) * header->frames );

	/* テクスチャ展開バッファを取得 */
	width = anmtex_work->width = header->width ;
	height = anmtex_work->height = header->height ;
	anmtex_work->image_size = ( (BP_LE_SwapSInt( header->flag_le ) & ~ANMTEX_HEADER_FLAG_ENDIANSWAPPED) == 0 ) ? ( width * height / 2 ) : ( width * height ) ;
	anmtex_work->clut_size = ( (BP_LE_SwapSInt( header->flag_le ) & ~ANMTEX_HEADER_FLAG_ENDIANSWAPPED) == 0 ) ? ( 4 * 16 ) : ( 4 * 256 ) ;

	{/* テクスチャ整合チェック */
	}

	/* 展開バッファ確保 */
	anmtex_work->tex_image[0] = GV_Malloc( anmtex_work->image_size * 2 + anmtex_work->clut_size * 2 );
	anmtex_work->tex_image[1] = anmtex_work->tex_image[0] + anmtex_work->image_size ;
	anmtex_work->tex_clut[0] = anmtex_work->tex_image[1] + anmtex_work->image_size ;
	anmtex_work->tex_clut[1] = anmtex_work->tex_clut[0] + anmtex_work->clut_size ;

	/* テクスチャ入れ替えパケット確保 */
//#ifdef PSX2
	anmtex_work->tex_load[0] =
	  DG_MakeLoadReplacePacket( tri_id, tex_id, anmtex_work->tex_image[0], anmtex_work->tex_clut[0] );
	anmtex_work->tex_load[1] =
	  DG_MakeLoadReplacePacket( tri_id, tex_id, anmtex_work->tex_image[1], anmtex_work->tex_clut[1] );
//#endif
#ifndef PSX2
  /* テクスチャ入れ替えパケットに画像フォーマットを設定（4bit or 8bit） */
  if ( anmtex_work->tex_load[0] != NULL ){
	  anmtex_work->tex_load[0]->mode = header->flag & 0x01 ;
  }
  if ( anmtex_work->tex_load[1] != NULL ){
	  anmtex_work->tex_load[1]->mode = header->flag & 0x01 ;
  }
#endif

	return ( 0 );
}
	/*
		アニメテクスチャ再生ワークの破棄
	*/
void UTL_FreeAnmtexWork( ANMTEX_WORK *anmtex_work )
{
	int		i ;
	for ( i = 0 ; i < 2 ; i++ ){
//#ifdef PSX2
		if ( anmtex_work->tex_load[i] != NULL ){
			if ( anmtex_work->last_set_buffer == i ) DG_ResetLoadReplaceTexture( anmtex_work->tex_load[i] );
			DG_FreeLoadReplacePacket( anmtex_work->tex_load[i] );
		}
//#endif
	}
	if ( anmtex_work->tex_image[0] != NULL ) GV_DelayedFree( anmtex_work->tex_image[ 0 ] );
}
	/*
		アニメーションを進ませる
	*/
// return whether texture needs updated
int UTL_ActAnmtex( ANMTEX_WORK *anmtex_work )
{
	ANMTEX_INFO		*info ;
	int				clock ;
   int bp_needsNewUpdate = FALSE;

#ifdef KP_WINDOWS	
	/* Windows版では諸事情により、同じフレームでの処理を行わない様にします */
	/* これによりアニメーションタイミングがずれてしまいます... */
	if( anmtex_work->last_frame_pcntr == DG_FramePresentCount )
   {
      return bp_needsNewUpdate;
   }
#endif

   if ( !( anmtex_work->flag & ANMTEX_FLAG_SLEEP ) )
   {
      // Arm fix:
      // removed DG_Arm_SkipThisFrame check so that an anmtex's interval_frame and SkipThisFrame
      // couldn't get into anti-phase and never update the texture!

      /* バッファクロック切り替え */
      anmtex_work->buffer_clock = 1 - anmtex_work->buffer_clock ;
      clock = anmtex_work->buffer_clock;

      /* テクスチャ展開処理 */
      info = &anmtex_work->anmtex_header->info[ anmtex_work->frame ] ;

      {/* ＣＬＵＴ展開 */
         int		i, j ;
         unsigned int	*src, *dst ;
         src = (unsigned int*)( anmtex_work->data_ptr + info->palette_offset ) ;
         dst = (unsigned int*)( anmtex_work->tex_clut[ clock ] ) ;
         if ( anmtex_work->clut_size == 16*4 ){
            for ( i = anmtex_work->clut_size / 4 ; i > 0 ; i-- ){
               *dst++ = *src++ ;
            }
         } else {
            /* ＸＢＯＸではリニアでＯＫ */
            for ( i = anmtex_work->clut_size / 4 ; i > 0 ; i-- ){
               *dst++ = *src++ ;
            }
         }
      }

      {/* テクセル展開 */
         int		compress_type, setup_type ;
         unsigned char	*compress_data, *texel_data ;

         compress_data = anmtex_work->data_ptr + info->data_offset ;
         texel_data = anmtex_work->tex_image[ clock ] ;
         compress_type = info->compress_flag & 0x0f ;
         setup_type = ( info->compress_flag >> 4 ) & 0x0f ;

         /* 圧縮データ展開 */
         DecompressBuffer( texel_data, compress_data, anmtex_work->image_size,
            BP_LE_SwapSInt( anmtex_work->anmtex_header->flag_le ) & ~ANMTEX_HEADER_FLAG_ENDIANSWAPPED, compress_type );

         /* イメージセットアップ実行 */
         SetupCompressBuffer( texel_data, anmtex_work->tex_image[ 1 - clock ],
            anmtex_work->image_size, setup_type );
      }

      /* テクスチャ入れ替えパケットの切り替え */
      //#ifdef PSX2
      if ( anmtex_work->last_set_buffer == ( 1 - clock ) ){
         DG_ResetLoadReplaceTexture( anmtex_work->tex_load[ 1 - clock ] );
      }
      DG_SetLoadReplaceTexture( anmtex_work->tex_load[ clock ] );

#ifdef KP_WINDOWS
      DG_LinerTextureFlushImage( anmtex_work->tex_load[ clock ]->tex);
      anmtex_work->last_frame_pcntr = DG_FramePresentCount ;
#endif
      //#endif
      anmtex_work->last_set_buffer = clock ;

      anmtex_work->frame ++;
      if ( anmtex_work->frame >= anmtex_work->anmtex_header->frames ){
         if ( anmtex_work->flag & ANMTEX_FLAG_LOOP ){
            anmtex_work->frame = 0 ;
         } else {
            anmtex_work->frame = anmtex_work->anmtex_header->frames - 1 ;
            anmtex_work->flag |= ANMTEX_FLAG_SLEEP ;
         }
      }

      bp_needsNewUpdate = TRUE;
	}

   return bp_needsNewUpdate;
}
