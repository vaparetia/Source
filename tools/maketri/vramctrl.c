/*
	$Id: vramctrl.c,v 1.1 2000/01/12 11:54:16 usr10478 Exp $
 */
/*                                                      */
/* ＰＳ２ＶＲＡＭデータ形式コントロールユーティリティー [[Data format control utility]] */
/*                                                      */
/* 1999.3.29 K.Takabe                                   */
/*                                                      */

/*
	ＰＳ２のＶＲＡＭデータはカラーモードによって特殊なアドレス変換が
	行われるが、現段階では仕様が公開されていない為、独自解析した結果を元に
	作成されている。

	＜用語定義＞

	ブロック：
	　ＶＲＡＭイメージ転送基準となるベースアドレスを先頭とした
	　連続した６４ワードの領域。ＶＲＡＭ上では連続した領域であるが
	　実機のデータ転送時に連続してデータを転送することは出来ない。
	　（データ転送機構による分散書き込みの為）
	　各カラーモードにおける格納データフォーマットは
	　　３２ビットカラー＞８×８テクセル
	　　１６ビットカラー＞１６×８テクセル
	　　８ビットインデックス＞１６×１６テクセル
	　　４ビットインデックス＞３２×１６テクセル
	　となっている。

	ブロックセット：
	　３２ブロックを一塊としたＶＲＡＭ記憶領域単位。各カラーモード間の
	　独立性が保証される最小単位の為、別モードのテクスチャ格納先頭ベース
	　アドレスを同一ブロックセット内に収めてはならない。

	ブロック座標：
	　各ブロックを敷き詰めた時の２次元座標位置

	ブロックアドレス：
	　ＶＲＡＭ上でのブロック格納位置＝ベースアドレス

	ブロックイメージ：
	　ブロック内のデータ格納イメージ。カラーモードによってデータの並びが異なる
	　為、転送時の指定カラーモードに合わせてイメージの変換を行う必要がある。

[[
PS2 is done for VRAM data address translation is a special color modes,
because at this stage has not been published specifications, are created based on the results of its own analysis.

<Term definition>

Block:
VRAM area 64 consecutive words beginning with the base as the reference image transfer.
VRAM on the transfer of data when data is transferred continuously to a contiguous area can not be real.
(Data transfer mechanism for writing distributed)
Stored data format for each color mode.
Bit color 32> 8 × 8 texels
Bit color 16> 16 × 8 texels
Bit index 8> 16 × 16 texels
Bit index 4> 32 × 16 texels
Has become.

Block Sets:
32 blocks and one block VRAM storage unit.
The smallest unit is guaranteed for the independence between each color mode,
another mode <no> texture storage location head base address same a block set should not be housed in the

Blocks Coordinates:
During the two-dimensional coordinates of each block paved

Block Address:
VRAM block storage location on-base

Block image:
In the block image data storage. Sequence data for different color modes,
it is necessary to convert color images to fit the specified transfer mode.
]]


=======================================================================
[[4 bit block image data sequence (Y coordinate hex - X coordinate hex)]]
４ビットブロックイメージデータ配列（Ｙ座標１６進－Ｘ座標１６進）
    <  28   24   20   16   12    8    4    0>
+00: 2-1C 0-18 2-14 0-10 2-0C 0-08 2-04 0-00  ; ３２×４テクセルが１６ワード内に右のように並ぶ [[32x4 texel 16 words in a line]]
+01: 2-1D 0-19 2-15 0-11 2-0D 0-09 2-05 0-01
+02: 2-1E 0-1A 2-16 0-12 2-0E 0-0A 2-06 0-02
+03: 2-1F 0-1B 2-17 0-13 2-0F 0-0B 2-07 0-03
+04: 2-18 0-1C 2-10 0-14 2-08 0-0C 2-00 0-04
+05: 2-19 0-1D 2-11 0-15 2-09 0-0D 2-01 0-05
+06: 2-1A 0-1E 2-12 0-16 2-0A 0-0E 2-02 0-06
+07: 2-1B 0-1F 2-13 0-17 2-0B 0-0F 2-03 0-07

+08: 3-1C 1-18 3-14 1-10 3-0C 1-08 3-04 1-00
+09: 3-1D 1-19 3-15 1-11 3-0D 1-09 3-05 1-01
+0A: 3-1E 1-1A 3-16 1-12 3-0E 1-0A 3-06 1-02
+0B: 3-1F 1-1B 3-17 1-13 3-0F 1-0B 3-07 1-03
+0C: 3-18 1-1C 3-10 1-14 3-08 1-0C 3-00 1-04
+0D: 3-19 1-1D 3-11 1-15 3-09 1-0D 3-01 1-05
+0E: 3-1A 1-1E 3-12 1-16 3-0A 1-0E 3-02 1-06
+0F: 3-1B 1-1F 3-13 1-17 3-0B 1-0F 3-03 1-07

+10: 6-18 4-1C 6-10 4-14 6-08 4-0C 6-00 4-04  ; 次の３２×４テクセルは１６ワード内に右のように並ぶ [[The following 32 × 4 texels are right in line as 16 words]]
+01: 6-19 4-1D 6-11 4-15 6-09 4-0D 6-01 4-05
+02: 6-1A 4-1E 6-12 4-16 6-0A 4-0E 6-02 4-06
+03: 6-1B 4-1F 6-13 4-17 6-0B 4-0F 6-03 4-07
+04: 6-1C 4-18 2-14 4-10 6-0C 4-08 6-04 4-00
 .
 .
 .

以降３２×８テクセル単位で上記の３２ワードのパターンで配置される [[Since the above 32 × 8 texels are arranged in a pattern of 32 words]]

=======================================================================
８ビットブロックイメージデータ配列（Ｙ座標１６進－Ｘ座標１６進）
[[8 bit block image data sequence (Y coordinate hex - X coordinate hex)]]

    <  24   16    8    0>
+00: 2-0C 0-08 2-04 0-00  ; １６×４テクセルが１６ワード内に右のように並ぶ [[16x4 texel 16 words in a line]]
+01: 2-0D 0-09 2-05 0-01
+02: 2-0E 0-0A 2-06 0-02
+03: 2-0F 0-0B 2-07 0-03
+04: 2-08 0-0C 2-00 0-04
+05: 2-09 0-0D 2-01 0-05
+06: 2-0A 0-0E 2-02 0-06
+07: 2-0B 0-0F 2-03 0-07

+08: 3-0C 1-08 3-04 1-00
+09: 3-0D 1-09 3-05 1-01
+0A: 3-0E 1-0A 3-06 1-02
+0B: 3-0F 1-0B 3-07 1-03
+0C: 3-08 1-0C 3-00 1-04
+0D: 3-09 1-0D 3-01 1-05
+0E: 3-0A 1-0E 3-02 1-06
+0F: 3-0B 1-0F 3-03 1-07

+00: 6-08 4-0C 6-00 4-04  ; 次の１６×４テクセルは１６ワード内に右のように並ぶ [[The following 16 × 4 texel is right in line as 16 words]]
+01: 6-09 4-0D 6-01 4-05
+02: 6-0A 4-0E 6-02 4-06
+03: 6-0B 4-0F 6-03 4-07
+04: 6-0C 4-00 6-04 4-00
 .
 .
 .

以降１６×８テクセル単位で上記の３２ワードのパターンで配置される [[Since the above 16 × 8 texels are arranged in a pattern of 32 words]]

=======================================================================
１６ビットブロックイメージデータ配列（Ｙ座標１６進－Ｘ座標１６進）
[[16 bit block image data sequence (Y coordinate hex - X coordinate hex)]]
    <  16    0>
+00: 0-08 0-00  ; １６×１テクセルが８ワード内に右のように並ぶ [[16x1 texel 8 words in a line]]
+01: 0-09 0-01
+02: 0-0A 0-02
+03: 0-0B 0-03
+04: 0-0C 0-04
+05: 0-0D 0-05
+06: 0-0E 0-06
+07: 0-0F 0-07

+08: 1-08 1-00
 .
 .
 .

=======================================================================


*/



#include <stdio.h>
#include <stdlib.h>

/* ブロック変換テーブル [[Block translation table]] */
static int block_table1[ 8 ] = { 0,1,4,5,16,17,20,21 };
static int block_table2[ 4 ] = { 0,2,8,10 };


/* ３２ビットカラーモードアドレス変換 [[32-bit color mode address translation]] */
int GetImageBlockNumber32bit( int x, int y, int width )
{
	int		hx, hy, lx, ly, block_set_x ;
	/* ブロック座標に変換 [[Transform coordinate to block]] */
	x = x / 8 ;
	y = y / 8 ;
	/* ブロック座標を上位下位に変換 [[Coordinates into the upper and lower blocks]] */
	hx = x / 8 ; lx = x & 7 ;
	hy = y / 4 ; ly = y & 3 ;
	/* 横方向最大ブロックセット数の算出 [[Calculation of the maximum number of horizontal blocks set]] */
	block_set_x = width / 64 ;
	/* 座標に相当するブロックアドレスを返す [[Return the block address corresponding to the coordinates]] */
	return ( block_table1[ lx ] + block_table2[ ly ] + ( hx + hy * block_set_x ) * 32 );
}
/* ３２ビットカラーイメージ＝＞３２ビットカラーイメージ変換 [[Convert 32 bit color image to 32 bit color image]] */
void ConvertBlockImage32to32( void *src, void *dst )
{
	int		x, y, i ;
	unsigned int	*data ;
	unsigned int	*color ;
	int		offset, shift ;

	data = dst ;
	color = src ;
	/* 0でクリア [[Clear 0]] */
	for ( i = 0 ; i < 64 ; i++ ) data[ i ] = 0 ;
	/* ８×８の３２ビットブロックイメージを３２ビットブロックイメージに変換（何もしない） [[8x8 32-bit block images into 32-bit block images (nothing)]] */
	for ( y = 0 ; y < 8 ; y++ ){
		for ( x = 0 ; x < 8 ; x++ ){
			/* 格納オフセット位置＆シフト量を算出 [[Store offset and calculate the position shift]] */
			offset = y * 8 ;
			offset += x ;
			/* データの書き込み [[Write data]] */
			data[ offset ] |= *color ;
			color++ ;
		}
	}
}

/* １６ビットカラーモードアドレス変換 [[16-bit color mode address translation]] */
int GetImageBlockNumber16bit( int x, int y, int width )
{
	int		hx, hy, lx, ly, block_set_x ;
	/* ブロック座標に変換 [[Transform coordinate to block]] */
	x = x / 16 ;
	y = y / 8 ;
	/* ブロック座標を上位下位に変換 [[Coordinates into the upper and lower blocks]] */
	hx = x / 4 ; lx = x & 3 ;
	hy = y / 8 ; ly = y & 7 ;
	/* 横方向最大ブロックセット数の算出 [[Calculation of the maximum number of horizontal blocks set]] */
	block_set_x = width / 64 ;
	/* 座標に相当するブロックアドレスを返す [[Return the block address corresponding to the coordinates]] */
	return ( block_table2[ lx ] + block_table1[ ly ] + ( hx + hy * block_set_x ) * 32 );
}
/* １６ビットカラーイメージ＝＞３２ビットカラーイメージ変換 [[Convert 16-bit color image to 32-bit color image]] */
void ConvertBlockImage16to32( void *src, void *dst )
{
	int		x, y, i ;
	unsigned int	*data ;
	unsigned short	*color ;
	int		offset, shift ;

	data = dst ;
	color = src ;
	/* 0でクリア [[Clear 0]] */
	for ( i = 0 ; i < 64 ; i++ ) data[ i ] = 0 ;
	/* １６×８の１６ビットブロックイメージを３２ビットブロックイメージに変換 [[Convert 16x8 16-bit block image to 32-bit block image]] */
	for ( y = 0 ; y < 8 ; y++ ){
		for ( x = 0 ; x < 16 ; x++ ){
			/* 格納オフセット位置＆シフト量を算出 [[Calculates the storage offset and the amount of shift]] */
			offset = y * 8 ;
			offset += x & 7 ;
			shift = ( x & 0x08 ) * 2 ;
			/* データの書き込み [[Write data]] */
			data[ offset ] |= *color << shift ;
			color++ ;
		}
	}
}

/* ８ビットインデックスモードアドレス変換 [[8-bit index mode address translation]] */
int GetImageBlockNumber8bit( int x, int y, int width )
{
	int		hx, hy, lx, ly, block_set_x ;
	/* ブロック座標に変換 [[Transform coordinate to block]] */
	x = x / 16 ;
	y = y / 16 ;
	/* ブロック座標を上位下位に変換 [[Coordinates into the upper and lower blocks]] */
	hx = x / 8 ; lx = x & 7 ;
	hy = y / 4 ; ly = y & 3 ;
	/* 横方向最大ブロックセット数の算出 [[Calculation of the maximum number of horizontal blocks set]] */
	block_set_x = width / 128 ;
	/* 座標に相当するブロックアドレスを返す [[Return the block address corresponding to the coordinates]] */
	return ( block_table1[ lx ] + block_table2[ ly ] + ( hx + hy * block_set_x ) * 32 );
}
/* ８ビットインデックスイメージ＝＞３２ビットカラーイメージ変換 [[Convert 8-bit index color image to 32 bit color image]] */
void ConvertBlockImage8to32( void *src, void *dst )
{
	int		x, y, i ;
	unsigned int	*data ;
	unsigned char	*index ;
	int		offset, shift ;

	data = dst ;
	index = src ;
	/* 0でクリア [[Clear 0]] */
	for ( i = 0 ; i < 64 ; i++ ) data[ i ] = 0 ;
	/* １６×１６の８ビットブロックイメージを３２ビットブロックイメージに変換 [[Convert 16x16 8-bit block image to 32-bit block image]] */
	for ( y = 0 ; y < 16 ; y++ ){
		for ( x = 0 ; x < 16 ; x++ ){
			/* 格納オフセット位置＆シフト量を算出 [[Calculates the storage offset and the amount of shift]] */
			offset = ( y / 4 ) * 16 ;
			offset += x & 7 ;
			shift = ( x & 0x08 ) * 2 ;
			if ( y & 1 ){
				offset += 8 ;
			}
			if ( y & 2 ){
				offset ^= 0x04 ;
				shift += 8 ;
			}
			if ( y & 4 ){
				offset ^= 0x04 ;
			}
			/* データの書き込み [[Write data]] */
			data[ offset ] |= *index << shift ;
			index++ ;
		}
	}
}



/* ４ビットインデックスモードアドレス変換 [[4-bit index mode address translation]] */
int GetImageBlockNumber4bit( int x, int y, int width )
{
	int		hx, hy, lx, ly, block_set_x ;
	/* ブロック座標に変換 [[Transform coordinate to block]] */
	x = x / 32 ;
	y = y / 16 ;
	/* ブロック座標を上位下位に変換 [[Coordinates into the upper and lower blocks]] */
	hx = x / 4 ; lx = x & 3 ;
	hy = y / 8 ; ly = y & 7 ;
	/* 横方向最大ブロックセット数の算出 [[Calculation of the maximum number of horizontal blocks set]] */
	block_set_x = width / 128 ;
	/* 座標に相当するブロックアドレスを返す [[Return the block address corresponding to the coordinates]] */
	return ( block_table2[ lx ] + block_table1[ ly ] + ( hx + hy * block_set_x ) * 32 );
}
/* ４ビットインデックスイメージ＝＞３２ビットカラーイメージ変換 [[Convert 4-bit index image to 32 bit color image]] */
void ConvertBlockImage4to32( void *src, void *dst )
{
	int		x, y, i ;
	unsigned int	*data ;
	unsigned char	*index ;
	int		palette ;
	int		offset, shift ;

	data = dst ;
	index = src ;
	/* 0でクリア [[Clear 0]] */
	for ( i = 0 ; i < 64 ; i++ ) data[ i ] = 0 ;
	/* ３２×１６の４ビットブロックイメージを３２ビットブロックイメージに変換 */
	for ( y = 0 ; y < 16 ; y++ ){
		for ( x = 0 ; x < 32 ; x++ ){
			/* インデックスの取り出し [[retrieving index]] */
			if ( ( x & 1 ) == 0 ){
				palette = *index & 0x0f ;
			} else {
				palette = ( *index >> 4 ) & 0x0f ;
				index++ ;
			}
			/* 格納オフセット位置＆シフト量を算出 [[Calculates the storage offset and the amount of shift]] */
			offset = ( y / 4 ) * 16 ;
			offset += x & 7 ;
			shift = x & 0x18 ;
			if ( y & 1 ){
				offset += 8 ;
			}
			if ( y & 2 ){
				offset ^= 0x04 ;
				shift += 4 ;
			}
			if ( y & 4 ){
				offset ^= 0x04 ;
			}
			/* データの書き込み [[Write data]] */
			data[ offset ] |= palette << shift ;
		}
	}
}

/* ３２ビットＣＬＵＴ＝＞３２ビットブロックイメージ変換 [[Convert 32-bit CLUT to 32-bit block image]] */
void ConvertBlockImageClut32( void *src, void *dst, int offset, int num )
{
	static int	trans_offset_data[16] = {
		64*0+ 0,64*1+ 0,64*0+16,64*1+16,
		64*0+32,64*1+32,64*0+48,64*1+48,
		64*2+ 0,64*3+ 0,64*2+16,64*3+16,
		64*2+32,64*3+32,64*2+48,64*3+48,
	};
   	unsigned int	*data, *clut ;
	int				i, j ;

	clut = src ;
	data = dst ;
	for ( j = offset ; j < num ; j++ ){
		for ( i = 0 ; i < 16 ; i++ ){
			data[ trans_offset_data[ j ] + i ] = *clut++ ;
		}
	}
}


