/*
	DirectX用テクスチャ圧縮ルーチン

	2002/03/08	K.Takabe

*/
/*
	現状ではDXT3のみ
	高品質で圧縮するために若干遅い

	一番品質に影響するDXT1圧縮ではまず、クラスタリングによる減色処理で
	２色を選び、さらに圧縮ブロック周辺で使用されている色データを
	減色で得た２色に加えて、その選んだ色の中で一番誤差の少なくなる組み合わせを
	選ぶことで劣化を最小限に抑えるように工夫している。
	恐らくMicrosoft製ライブラリの物よりは若干品質はいいと思われる。

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "my.h"
#include "bmp.h"


/* ---------------------------------------------------------------- */
static int pow_table[ 512 ] ;
static int BitConvertTable_RB[256] ;
static int BitConvertTable_G[256] ;


static int				UseColorNum ;				/* 登録色数 */
static unsigned short	UseColorList[16*10] ;	/* 登録色データ（１６ビット） */
static int				weight_list[16*10] ;	/* 減色時のウェイト記録用 */

/* キーカラーテーブル */
typedef struct _color_list {
	unsigned short	col1 ;			/* 圧縮用第１カラー */
	unsigned short	col2 ;			/* 圧縮用第２カラー */
	RGBA		colors[4] ;			/* 展開後カラーリスト */
	int			len ;				/* キーカラー間の距離 */
	int			error ;				/* 誤差 */
} COLOR_LIST ;
static int			ColorTableListNum ;			/* 色リストテーブル数 */
static COLOR_LIST	ColorListTable[(16*10)*(16*10)] ;	/* 色リストテーブル */


/* ---------------------------------------------------------------- */
/* ディザつきで減色した色を取得する */
static unsigned int add_dithe_color( unsigned int col, int x, int y )
{
	static int	offset_table[16] = {
		 0-8, 8-8, 2-8,10-8,
		12-8, 4-8,14-8, 6-8,
		 3-8,11-8, 1-8, 9-8,
		15-8, 7-8,13-8, 5-8
	};
	int		r, g, b, a, offset ;
	r = ( col >> 16 ) & 0xff ;
	g = ( col >>  8 ) & 0xff ;
	b = ( col       ) & 0xff ;
	x &= 0x03 ;
	y &= 0x03 ;
	offset = x + y * 4 ;
	r += offset_table[ offset ] / 4 ;
	g += offset_table[ ( offset + 1 ) & 15 ] / 8 ;
	b += offset_table[ ( offset + 2 ) & 15 ] / 4 ;
	if ( r < 0 ) r = 0 ;
	if ( r > 255 ) r = 255 ;
	if ( g < 0 ) g = 0 ;
	if ( g > 255 ) g = 255 ;
	if ( b < 0 ) b = 0 ;
	if ( b > 255 ) b = 255 ;
	col = ( r << 16 ) | ( g << 8 ) | b | ( col & 0xff000000 );
	return ( col );
}
/* ---------------------------------------------------------------- */
/* 32bitから16bitへの変換用テーブルの作成 */
static void InitColorConvertTable( void )
{
	int		i, j, min_index, min_diff, diff ;
	int		check_table[256] ;

	for ( i = 0 ; i < 32 ; i++ ){
		check_table[i] = ( i << 3 ) | ( i >> 2 ) ;
	}
	for ( i = 0 ; i < 256 ; i++ ){
		min_index = 0 ;
		min_diff = 0x7fffffff ;
		for ( j = 0 ; j < 32 ; j++ ){
			diff = check_table[j] - i ;
			diff = diff * diff ;
			if ( min_diff > diff ){
				min_index = j ;
				min_diff = diff ;
			}
		}
		BitConvertTable_RB[ i ] = min_index ;
	}
	for ( i = 0 ; i < 64 ; i++ ){
		check_table[i] = ( i << 2 ) | ( i >> 4 ) ;
	}
	for ( i = 0 ; i < 256 ; i++ ){
		min_index = 0 ;
		min_diff = 0x7fffffff ;
		for ( j = 0 ; j < 64 ; j++ ){
			diff = check_table[j] - i ;
			diff = diff * diff ;
			if ( min_diff > diff ){
				min_index = j ;
				min_diff = diff ;
			}
		}
		BitConvertTable_G[ i ] = min_index ;
	}
}
/* ---------------------------------------------------------------- */
/* ３２ビットカラーから１６ビットカラーへ変換（アルファ無視） */
static unsigned short conv_color_32_to_16( unsigned int col32 )
{
	unsigned int	r, g, b;//, a ;
	unsigned short	col16 ;
	r = ( col32 >> 16 ) & 0xff ;
	g = ( col32 >> 8 ) & 0xff ;
	b = col32 & 0xff ;
#if 0
#if 1
	/* 四捨五入処理 */
	r += 4 ; if ( r > 255 ) r = 255 ;
	g += 2 ; if ( g > 255 ) g = 255 ;
	b += 4 ; if ( b > 255 ) b = 255 ;
#endif
	r >>= 3 ;
	g >>= 2 ;
	b >>= 3 ;
#else
	r = BitConvertTable_RB[ r ];
	g = BitConvertTable_G[ g ];
	b = BitConvertTable_RB[ b ];
#endif
	col16 = b | ( g << 5 ) | ( r << 11 ) ;
	return ( col16 );
}
/* １６ビットカラーから３２ビットカラーへ変換（アルファ無視） */
static unsigned int conv_color_16_to_32( unsigned short col16 )
{
	unsigned int	r, g, b;//, a ;
	unsigned int	col32 ;
	r = ( col16 >> 11 ) & 0x1f ;
	g = ( col16 >> 5 ) & 0x3f ;
	b = col16 & 0x1f ;
	r = ( r << 3 ) | ( r >> 2 ) ;
	g = ( g << 2 ) | ( g >> 4 ) ;
	b = ( b << 3 ) | ( b >> 2 ) ;
	col32 = b | ( g << 8 ) | ( r << 16 ) ;
	return ( col32 );
}
/* 代表２色から４色を生成 */
static void make_colors( RGBA *collist, unsigned short col1, unsigned short col2 )
{
	RGBA	col_a, col_b ;
	*(unsigned int*)&col_a = conv_color_16_to_32( col1 );
	*(unsigned int*)&col_b = conv_color_16_to_32( col2 );
	collist[0] = col_a ;
	collist[1] = col_b ;
	collist[2].r = ( (unsigned int)col_a.r * 2 + (unsigned int)col_b.r + 1 ) / 3 ;
	collist[2].g = ( (unsigned int)col_a.g * 2 + (unsigned int)col_b.g + 1 ) / 3 ;
	collist[2].b = ( (unsigned int)col_a.b * 2 + (unsigned int)col_b.b + 1 ) / 3 ;
	collist[3].r = ( (unsigned int)col_a.r + (unsigned int)col_b.r * 2 + 1 ) / 3 ;
	collist[3].g = ( (unsigned int)col_a.g + (unsigned int)col_b.g * 2 + 1 ) / 3 ;
	collist[3].b = ( (unsigned int)col_a.b + (unsigned int)col_b.b * 2 + 1 ) / 3 ;
	//color_2 = (2 * color_0 + color_1 + 1) / 3;
    //color_3 = (color_0 + 2 * color_1 + 1) / 3;
}

/* 代表２色から3色を生成 */
static void make_colors3( RGBA *collist, unsigned short col1, unsigned short col2 )
{
	RGBA	col_a, col_b ;
	*(unsigned int*)&col_a = conv_color_16_to_32( col1 );
	*(unsigned int*)&col_b = conv_color_16_to_32( col2 );
	collist[0] = col_a ;
	collist[1] = col_b ;
	collist[2].r = ((unsigned int)col_a.r  + (unsigned int)col_b.r) / 2 ;
	collist[2].g = ((unsigned int)col_a.g  + (unsigned int)col_b.g) / 2 ;
	collist[2].b = ((unsigned int)col_a.b  + (unsigned int)col_b.b) / 2 ;

	collist[3].r = 0;
	collist[3].g = 0;
	collist[3].b = 0;

}

/* ３２ビットのベタ用近似カラーを求める */
static int make_bestcolor_from_beta( unsigned short *col, unsigned short *base )
{
	int		r, g, b, rr, gg, bb ;
	int		i, j, k, count = 0 ;
	r = ( *base >> 11 ) & 0x1f ;
	g = ( *base >> 5 ) & 0x3f ;
	b = *base & 0x1f ;
	for ( i = -1 ; i < 2 ; i++ ){
		for ( j = -1 ; j < 2 ; j++ ){
			for ( k = -1 ; k < 2 ; k++ ){
				rr = r + i ;
				gg = g + j ;
				bb = b + k ;
				if ( rr > 31 ) continue ;
				if ( rr <  0 ) continue ;
				if ( gg > 63 ) continue ;
				if ( gg <  0 ) continue ;
				if ( bb > 31 ) continue ;
				if ( bb <  0 ) continue ;
				*col++ = bb | ( gg << 5 ) | ( rr << 11 ) ;
				count++ ;
			}
		}
	}
	return ( count );
}


/* ２色間の距離（の二乗）を求める */
static int len_color( RGBA *col1, RGBA *col2 )
{
	int		r, g, b, len ;
	r = col1->r - col2->r ;
	g = col1->g - col2->g ;
	b = col1->b - col2->b ;
#if 0
	len = r * r + g * g + b * b ;
#else
	len = pow_table[ r + 256 ] + pow_table[ g + 256 ] + pow_table[ b + 256 ] ;
#endif
	return ( len );
}
/* 誤差を算出 */
static int error_size( RGBA *base, RGBA *col, int num )
{
	int		error, max_error, total_error, select ;
	int		i, j, r, g, b ;

	total_error = 0 ;
	for ( i = 0 ; i < num ; i++ ){
		max_error = 0x7fffffff ;
		select = 0 ;
		r = 256 - col[ i ].r ;
		g = 256 - col[ i ].g ;
		b = 256 - col[ i ].b ;
		for ( j = 0 ; j < 4 ; j++ ){
#if 0
			error = len_color( &base[ j ], &col[ i ] );
#else
			error = pow_table[ base[ j ].r + r ] + pow_table[ base[ j ].g + g ] + pow_table[ base[ j ].b + b ] ;
#endif
			if ( max_error > error ){
				max_error = error ;
				select = j ;
			}
		}
		total_error += max_error ;
	}

	return ( total_error );
}

//..一番近い色のインデックス得る (0-3)
static int color_select( RGBA *base, RGBA *col )
{
	int		error, max_error, select ;
	int		j, r, g, b ;

	max_error = 0x7fffffff ;
	select = 0 ;
	for ( j = 0 ; j < 4 ; j++ ){
		r = base[ j ].r - col->r ;
		g = base[ j ].g - col->g ;
		b = base[ j ].b - col->b ;
		error = r * r + g * g + b * b ;
		if ( max_error > error ){
			max_error = error ;
			select = j ;
		}
	}

	return ( select );
}

//..一番近い色のインデックス得る (0-3)
//..透過色の場合3を返す
static int color_selectPenetration( RGBA *base, RGBA *col )
{
	int		error, max_error, select ;
	int		i=0, j, r, g, b ;
	
	if( col[i].r == 0 && col[i].g == 0 && col[i].b == 0)
		return(3);//..透過色ビット
	
	max_error = 0x7fffffff ;
	select = 0 ;
	for ( j = 0 ; j < 3 ; j++ ){
		r = base[ j ].r - col->r ;
		g = base[ j ].g - col->g ;
		b = base[ j ].b - col->b ;
		error = r * r + g * g + b * b ;
		if ( max_error > error ){
			max_error = error ;
			select = j ;
		}
	}

	return ( select );
}




/* ---------------------------------------------------------------- */
/* 色近似高速化用テーブルの初期化 */
void InitPowTable( void )
{
	int		i ;
	for ( i = -256 ; i < 256 ; i++ ){
		pow_table[ i + 256 ] = i * i ;
	}
}
/* 色のブレンド */
void MakeColorBlend2( RGBA *res, RGBA *col0, RGBA *col1 )
{
	unsigned int	r, g, b, a ;
	r = ( col0->r + col1->r ) / 2 ;
	g = ( col0->g + col1->g ) / 2 ;
	b = ( col0->b + col1->b ) / 2 ;
	a = ( col0->a + col1->a ) / 2 ;
	res->r = r ;
	res->g = g ;
	res->b = b ;
	res->a = a ;
}
void MakeColorBlend4( RGBA *res, RGBA *col0, RGBA *col1, RGBA *col2, RGBA *col3 )
{
	unsigned int	r, g, b, a ;
	r = ( col0->r + col1->r + col2->r + col3->r ) / 4 ;
	g = ( col0->g + col1->g + col2->g + col3->g ) / 4 ;
	b = ( col0->b + col1->b + col2->b + col3->b ) / 4 ;
	a = ( col0->a + col1->a + col2->a + col3->a ) / 4 ;
	res->r = r ;
	res->g = g ;
	res->b = b ;
	res->a = a ;
}
void MakeColorBlend2Weight( RGBA *res, RGBA *col0, int weight0, RGBA *col1, int weight1 )
{
	unsigned int	r, g, b, a ;
	int				weight = weight0 + weight1 ;
	r = ( col0->r * weight0 + col1->r * weight1 ) / weight ;
	g = ( col0->g * weight0 + col1->g * weight1 ) / weight ;
	b = ( col0->b * weight0 + col1->b * weight1 ) / weight ;
	a = ( col0->a * weight0 + col1->a * weight1 ) / weight ;
#if 0
	r >>= 3 ; r = ( r << 3 ) | ( r >> 2 ) ;
	g >>= 2 ; g = ( g << 2 ) | ( g >> 4 ) ;
	b >>= 3 ; b = ( b << 3 ) | ( b >> 2 ) ;
#endif
	res->r = r ;
	res->g = g ;
	res->b = b ;
	res->a = a ;
}
/* キーカラーの候補を作成 */
void MakeColorListTable( RGBA col[], int num, int beta_flag )
{
	int		i, j ;
	unsigned short		col16 ;
	//unsigned int		col32 ;

	/* 使用している色を調べる */
	UseColorNum = 0 ;
	for ( i = 0 ; i < num ; i++ ){
		col16 = conv_color_32_to_16( *(int*)&col[i] );		/* １６ビットカラーへ変換 */
		/* 既に登録済みかどうかをチェック */
		for ( j = 0 ; j < UseColorNum ; j++ ){
			if ( UseColorList[ j ] == col16 ) break ;
		}
		if ( j == UseColorNum ){
			UseColorList[ UseColorNum++ ] = col16 ;
		}
	}
	/* 単色の場合は１６ビット時に精度が落ちる可能性があるため、補間用カラーも加えておく */
	if ( UseColorList[0] != 0 && UseColorNum < 2 ){
		UseColorNum +=  make_bestcolor_from_beta( &UseColorList[ UseColorNum ], &UseColorList[ 0 ] );
	}
#if 1
	/* カラーのソート */
	for ( i = UseColorNum-1 ; i > 0 ; i-- ){
		for ( j = 0 ; j < i ; j++ ){
			if ( UseColorList[ j ] < UseColorList[ j + 1 ] ){
				unsigned short	tmp ;
				tmp = UseColorList[ j ] ;
				UseColorList[ j ] = UseColorList[ j + 1 ];
				UseColorList[ j + 1 ] = tmp ;
			}
		}
	}
#endif
	/* カラーの展開パターンを作成 */
	ColorTableListNum = 0 ;
	for ( i = 0 ; i < UseColorNum ; i++ ){
		for ( j = i ; j < UseColorNum ; j++ ){
			COLOR_LIST	*list ;
			list = &ColorListTable[ ColorTableListNum++ ] ;
			/* １ビットアルファは使用しないことを前提としているので注意！ */
			if ( UseColorList[ i ] > UseColorList[ j ] ){
				list->col1 = UseColorList[ i ] ;
				list->col2 = UseColorList[ j ] ;
			} else {
				list->col1 = UseColorList[ j ] ;
				list->col2 = UseColorList[ i ] ;
			}
			if ( list->col1 == list->col2 ){
				list->col2 = 0 ;
			}
			make_colors( list->colors, list->col1, list->col2 );
			list->len = len_color( &list->colors[0], &list->colors[1] );
		}
	}
}

/* 圧縮に必要な１６ビットカラー２つを求める */
void SelectBlockColor( unsigned short select_col[2], RGBA col[] )
{
	int		max_error, select_list ;
	int		i;
	COLOR_LIST		*list, *current ;

	/* 一番誤差の少ないリストを検索 */
	max_error = 0x7fffffff ;
	select_list = 0 ;
	current = ColorListTable ;
	for ( i = 0 ; i < ColorTableListNum ; i++ ){
		list = &ColorListTable[ i ] ;
		list->error = error_size( list->colors, col, 16 );
		if ( ( max_error > list->error ) ||
			( ( max_error == list->error ) && ( list->len < current->len ) ) ){
			max_error = list->error ;
			select_list = i ;
			current = list ;
		}
	}

	/* 最終的に選択した色を返す */
	select_col[0] = ColorListTable[ select_list ].col1 ;
	select_col[1] = ColorListTable[ select_list ].col2 ;

	return ;
}

/* 色成分をDXT1で圧縮したデータを求める */
void EncodeDXT_Color( unsigned short data[4], unsigned short base_col[2], RGBA col[16] )
{
	RGBA	colors[4] ;
	int		i, j ;

	/* キーカラーの登録 */
	data[0] = base_col[0] ;
	data[1] = base_col[1] ;

	/* ４色を生成 */
	make_colors( colors, base_col[0], base_col[1] );

	data[2] = 0 ;
	data[3] = 0 ;
	/* ブロック内の色を一番近い色インデックスを求める */
	for ( i = 0 ; i < 8 ; i+=4 ){
		for ( j = 0 ; j < 4 ; j++ ){
			int		index = i + j ;
			data[2] |= color_select( colors, &col[ index ] ) << ( index * 2 ) ;
		}
	}
	for ( i = 8 ; i < 16 ; i+=4 ){
		for ( j = 0 ; j < 4 ; j++ ){
			int		index = i + j ;
			data[3] |= color_select( colors, &col[ index ] ) << ( ( index - 8 ) * 2 ) ;
		}
	}
	//data[2] = 0x5500 ;	/* debug */
	//data[3] = 0xffaa ;	/* debug */
}

/* 
	色成分をDXT1で圧縮したデータを求める 
	RGB == 0なら透過色として扱う

*/
void EncodeDXT1_ColorPenetration( unsigned short data[4], unsigned short base_col[2], RGBA col[16] )
{
	RGBA	colors[4] ;
	int		i, j ,alpha_flg=0;
	unsigned short tmp_col;	

	for(i=0; i<16; i++){
		if( col[i].r == 0 && col[i].g == 0 && col[i].b == 0){
			alpha_flg = TRUE;
		}
	}
	
	if(alpha_flg == TRUE){
		if(base_col[0]>base_col[1]){
			unsigned short tmp_col;
			tmp_col=base_col[0];
			base_col[0] = base_col[1];
			base_col[1] = tmp_col;
		}
	}else{
		if(base_col[0]<base_col[1]){
			tmp_col=base_col[0];
			base_col[0] = base_col[1];
			base_col[1] = tmp_col;
		}
	}	



	/* キーカラーの登録 */
	data[0] = base_col[0] ;
	data[1] = base_col[1] ;
	
	if(alpha_flg==TRUE){
		//..透過色あり
		/* ３色を生成 */
		make_colors3( colors, base_col[0], base_col[1] );

		data[2] = 0 ;
		data[3] = 0 ;
		/* ブロック内の色を一番近い色インデックスを求める */
		for ( i = 0 ; i < 8 ; i+=4 ){
			for ( j = 0 ; j < 4 ; j++ ){
				int		index = i + j ;
				data[2] |= color_selectPenetration( colors, &col[ index ] ) << ( index * 2 ) ;
			}
		}
		for ( i = 8 ; i < 16 ; i+=4 ){
			for ( j = 0 ; j < 4 ; j++ ){
				int		index = i + j ;
				data[3] |= color_selectPenetration( colors, &col[ index ] ) << ( ( index - 8 ) * 2 ) ;
			}
		}
	}else{
		/* ４色を生成 */
		make_colors( colors, base_col[0], base_col[1] );
	
		data[2] = 0 ;
		data[3] = 0 ;
		/* ブロック内の色を一番近い色インデックスを求める */
		for ( i = 0 ; i < 8 ; i+=4 ){
			for ( j = 0 ; j < 4 ; j++ ){
				int		index = i + j ;
				data[2] |= color_select( colors, &col[ index ] ) << ( index * 2 ) ;
			}
		}
		for ( i = 8 ; i < 16 ; i+=4 ){
			for ( j = 0 ; j < 4 ; j++ ){
				int		index = i + j ;
				data[3] |= color_select( colors, &col[ index ] ) << ( ( index - 8 ) * 2 ) ;
			}
		}
	}
	//data[2] = 0x5500 ;	/* debug */
	//data[3] = 0xffaa ;	/* debug */
}



/* アルファ成分をDXT3で圧縮したデータを求める */
void EncodeDXT_AlphaDXT3( unsigned short data[4], RGBA col[16] )
{
	static int	offset_table[16] = {
		 0-8, 8-8, 2-8,10-8,
		12-8, 4-8,14-8, 6-8,
		 3-8,11-8, 1-8, 9-8,
		15-8, 7-8,13-8, 5-8
	};
	int		i, j, a ;

	/*  */
	for ( i = 0 ; i < 16 ; i+=4 ){
		data[ i / 4 ] = 0 ;
		for ( j = 0 ; j < 4 ; j++ ){
			int		index = i + j ;
			a = col[ index ].a ;
#if 1
			if ( a != 0 ){
				a += offset_table[ index ];
				if ( a > 255 ) a = 255 ;
				if ( a < 1 ) a = 1 ;
			}
#endif
			data[ i / 4 ] |= ( (unsigned int)a >> 4 ) << ( j * 4 ) ;
		}
	}
}

/* アルファ成分をDXT5で圧縮したデータを求める */
void EncodeDXT_AlphaDXT5( unsigned short data[4], RGBA col[16] )
{
	int		i, j, k, a, max_a, min_a, zero_flag, one_flag ;
	int		alpha[8] ;
	unsigned short	*data_ptr ;
	int				shift ;

	/* αの最大最小を検索 */
	max_a = -1 ;
	min_a = 256 ;
	zero_flag  = 0 ;
	one_flag = 0 ;
	for ( i = 0 ; i < 16 ; i+=4 ){
		data[ i / 4 ] = 0 ;
		for ( j = 0 ; j < 4 ; j++ ){
			int		index = i + j ;
			a = col[ index ].a ;
#if 0
			if ( a == 0 ){
				zero_flag = 1 ;
			} else if ( a == 255 ){
				one_flag = 1 ;
			} else {
				if ( a > max_a ) max_a = a ;
				if ( a < min_a ) min_a = a ;
			}
#else
			/* 現状では補間８αモードのみ対応 */
			if ( a > max_a ) max_a = a ;
			if ( a < min_a ) min_a = a ;
#endif
		}
	}
	if ( 1 ){
		/* ８αモード */
		alpha[0] = max_a ;
		alpha[1] = min_a ;
		alpha[2] = ( 6 * alpha[0] + 1 * alpha[1] + 3 ) / 7 ;
		alpha[3] = ( 5 * alpha[0] + 2 * alpha[1] + 3 ) / 7 ;
		alpha[4] = ( 4 * alpha[0] + 3 * alpha[1] + 3 ) / 7 ;
		alpha[5] = ( 3 * alpha[0] + 4 * alpha[1] + 3 ) / 7 ;
		alpha[6] = ( 2 * alpha[0] + 5 * alpha[1] + 3 ) / 7 ;
		alpha[7] = ( 1 * alpha[0] + 6 * alpha[1] + 3 ) / 7 ;
	} else {
		/* ６αモード */
		alpha[0] = min_a ;
		alpha[1] = max_a ;
		alpha[2] = ( 4 * alpha[0] + 1 * alpha[1] + 2 ) / 5 ;
		alpha[3] = ( 3 * alpha[0] + 2 * alpha[1] + 2 ) / 5 ;
		alpha[4] = ( 2 * alpha[0] + 3 * alpha[1] + 2 ) / 5 ;
		alpha[5] = ( 1 * alpha[0] + 4 * alpha[1] + 2 ) / 5 ;
		alpha[6] = 0 ;
		alpha[7] = 255 ;
	}
	data[0] = alpha[0] | ( alpha[1] << 8 ) ;
	data[ 1 ] = 0 ;
	data[ 2 ] = 0 ;
	data[ 3 ] = 0 ;
	data_ptr = data + 1 ;
	shift = 0 ;
	for ( i = 0 ; i < 16 ; i+=4 ){
		for ( j = 0 ; j < 4 ; j++ ){
			int		min_diff, min_index, diff ;
			int		index = i + j ;
			a = col[ index ].a ;
			/* 一番近いαを検索 */
			min_diff = 65536 ;
			min_index = 0 ;
			for ( k = 0 ; k < 8 ; k++ ){
				diff = a - alpha[ k ] ;
				if ( diff < 0 ) diff = -diff ;
				if ( diff < min_diff ){
					min_index = k ;
					min_diff = diff ;
				}
			}
			*( data_ptr + 0 ) |= min_index << shift ;
			*( data_ptr + 1 ) |= ( min_index << shift ) >> 16 ;
			shift += 3 ;
			if ( shift >= 16 ){
				shift -= 16 ;
				data_ptr++ ;
			}
		}
	}
}

/* クラスタリングによる減色処理 */
void LessenColor( RGBA *res, RGBA *col, int current_num, int res_num )
{
	int		len, min_len, min_index_a, min_index_b ;
	int		i, j, tmp, w ;

	for ( i = 0 ; i < current_num ; i++ ) weight_list[ i ] = 1 ;
	for ( i = 0 ; i < current_num ; i++ ) res[ i ] = col[ i ] ;

	while ( current_num > res_num ){
		min_len = 0x7fffffff ;
		min_index_a = min_index_b = 0 ;
		for ( i = 0 ; i < current_num - 1 ; i++ ){
			for ( j = i+1 ; j < current_num ; j++ ){
				len = len_color( &res[ i ], &res[ j ] );
				if ( len < min_len ){
					min_len = len ;		
					min_index_a = i ;
					min_index_b = j ;	//..色の距離最小の組み合わせ
				}
			}
		}
		if ( min_index_a > min_index_b ){
			tmp = min_index_a ;
			min_index_a = min_index_b ;
			min_index_b = tmp ;
		}
		w = weight_list[ min_index_a ] + weight_list[ min_index_b ] ;
		MakeColorBlend2Weight( &res[ min_index_a ],
							  &res[ min_index_a ], weight_list[ min_index_a ] + w / 2,
							  &res[ min_index_b ], weight_list[ min_index_b ] + w / 2 );
		weight_list[ min_index_a ] += weight_list[ min_index_b ] ;
		weight_list[ min_index_a ] = 1 ;	/* ウェイト無視モード */
		for ( i = min_index_b ; i < current_num ; i++ ){
			res[ i ] = res[ i + 1 ] ;
			weight_list[ i ] = weight_list[ i + 1 ] ;
		}
		current_num-- ;
	}
	
}

/* ３２ビットテクスチャイメージの圧縮（DXT5） */
void *CompressImageDXT5( unsigned int *image, int width, int height, void **compress_buff, int *compress_size )
{
	unsigned short		*data, *write ;
	int					i, j, x, y, count, beta_flag, black_flag ;
	unsigned int		col[ 16 * 4 ] ;	/* 余裕をもって確保 */
	unsigned short		select_col[2] ;

	InitPowTable();
	InitColorConvertTable();

	*compress_size = width * height ;
	data = malloc( *compress_size );
	memset( data, 0, *compress_size );
	if ( compress_buff != NULL ) *compress_buff = data ;

	write = data ;
	for ( y = 0 ; y < height ; y+=4 ){
		for ( x = 0 ; x < width ; x+=4 ){
			beta_flag = 0 ;
			black_flag = 0 ;
			/* キーカラーの選定 */
			count = 0 ;
#if 1
			/* 減色によるキーカラー候補選定 */
			for ( j = 0 ; j < 4 ; j++ ){
				for ( i = 0 ; i < 4 ; i++ ){
					int		 index ;
					index = ( x + i ) + ( y + j ) * width ;
					col[ count++ ] = image[ index ];
					//col[ count-1 ] = add_dithe_color( col[ count-1 ], i, j );
					beta_flag += col[ 0 ] ^ image[ index ] ;
					black_flag += image[ index ] ;
				}
			}
			LessenColor( (void*)col, (void*)col, count, 2 ); count = 2 ;
			if ( black_flag == 0 ) beta_flag = 1 ;
#endif
#if 1
			/* 隣接ピクセルを含めてキーカラー候補選定 */
			for ( j = -1 ; j < 5 ; j++ ){
				int		xx, yy ;
				yy = y + j ;
				if ( yy < 0 ) continue ;
				if ( yy >= height ) continue ;
				for ( i = -1 ; i < 5 ; i++ ){
					int		 index ;
					xx = x + i ;
					if ( xx < 0 ) continue ;
					if ( xx >= width ) continue ;
					index = xx + yy * width ;
					col[ count ] = image[ index ];
					//col[ count ] = add_dithe_color( col[ count ], i, j );
					count++ ;
				}
			}
			//LessenColor( (void*)&col[ count ], (void*)col, count, 2 );
			//count += 2 ;
#endif
			/* 総当り検索用データの構築 */
			MakeColorListTable( (RGBA *)col, count, beta_flag );
			/* 圧縮に使用する色を集める */
			for ( j = 0 ; j < 4 ; j++ ){
				for ( i = 0 ; i < 4 ; i++ ){
					int		 index ;
					index = ( x + i ) + ( y + j ) * width ;
					col[ i + j * 4 ] = image[ index ];
				}
			}
			/* 総当りにより一番誤差の少なかった色をキーカラーに選ぶ */
			//..              result(16bit*2) in(RGBA)
			SelectBlockColor( select_col, (void*)col );
			/* DXT3にてアルファ圧縮 */
			EncodeDXT_AlphaDXT5( write, (void*)col );
			write += 4 ;
			/* DXT1にてカラー圧縮（１ビットアルファ未使用） */
			EncodeDXT_Color( write, select_col, (void*)col );
			write += 4 ;
		}
	}

	return ( data );
}

/* ３２ビットテクスチャイメージの圧縮（DXT3） */
void *CompressImageDXT3( unsigned int *image, int width, int height, void **compress_buff, int *compress_size )
{
	unsigned short		*data, *write ;
	int					i, j, x, y, count, beta_flag, black_flag ;
	unsigned int		col[ 16 * 4 ] ;	/* 余裕をもって確保 */
	unsigned short		select_col[2] ;

	InitPowTable();
	InitColorConvertTable();

	*compress_size = width * height ;
	data = malloc( *compress_size );
	memset( data, 0, *compress_size );
	if ( compress_buff != NULL ) *compress_buff = data ;

	write = data ;
	for ( y = 0 ; y < height ; y+=4 ){
		for ( x = 0 ; x < width ; x+=4 ){
			beta_flag = 0 ;
			black_flag = 0 ;
			/* キーカラーの選定 */
			count = 0 ;
#if 1
			/* 減色によるキーカラー候補選定 */
			for ( j = 0 ; j < 4 ; j++ ){
				for ( i = 0 ; i < 4 ; i++ ){
					int		 index ;
					index = ( x + i ) + ( y + j ) * width ;
					col[ count++ ] = image[ index ];
					//col[ count-1 ] = add_dithe_color( col[ count-1 ], i, j );
					beta_flag += col[ 0 ] ^ image[ index ] ;
					black_flag += image[ index ] ;
				}
			}
			LessenColor( (void*)col, (void*)col, count, 2 ); count = 2 ;
			if ( black_flag == 0 ) beta_flag = 1 ;
#endif
#if 1
			/* 隣接ピクセルを含めてキーカラー候補選定 */
			for ( j = -1 ; j < 5 ; j++ ){
				int		xx, yy ;
				yy = y + j ;
				if ( yy < 0 ) continue ;
				if ( yy >= height ) continue ;
				for ( i = -1 ; i < 5 ; i++ ){
					int		 index ;
					xx = x + i ;
					if ( xx < 0 ) continue ;
					if ( xx >= width ) continue ;
					index = xx + yy * width ;
					col[ count ] = image[ index ];
					//col[ count ] = add_dithe_color( col[ count ], i, j );
					count++ ;
				}
			}
			//LessenColor( (void*)&col[ count ], (void*)col, count, 2 );
			//count += 2 ;
#endif
			/* 総当り検索用データの構築 */
			MakeColorListTable( (RGBA *)col, count, beta_flag );
			/* 圧縮に使用する色を集める */
			for ( j = 0 ; j < 4 ; j++ ){
				for ( i = 0 ; i < 4 ; i++ ){
					int		 index ;
					index = ( x + i ) + ( y + j ) * width ;
					col[ i + j * 4 ] = image[ index ];
				}
			}
			/* 総当りにより一番誤差の少なかった色をキーカラーに選ぶ */
			//..              result(16bit*2) in(RGBA)
			SelectBlockColor( select_col, (void*)col );
			/* DXT3にてアルファ圧縮 */
			EncodeDXT_AlphaDXT3( write, (void*)col );
			write += 4 ;
			/* DXT1にてカラー圧縮（１ビットアルファ未使用） */
			EncodeDXT_Color( write, select_col, (void*)col );
			write += 4 ;
		}
	}

	return ( data );
}

/* ３２ビットテクスチャイメージの圧縮（DXT1） */
void *CompressImageDXT1( unsigned int *image, int width, int height, void **compress_buff, int *compress_size )
{
	unsigned short		*data, *write ;
	int					i, j, x, y, count, beta_flag, black_flag ;
	unsigned int		col[ 16 * 4 ] ;	/* 余裕をもって確保 */
	unsigned short		select_col[2] ;

	InitPowTable();
	InitColorConvertTable();

	*compress_size = width * height / 2;
	data = malloc( *compress_size );
	memset( data, 0, *compress_size );
	if ( compress_buff != NULL ) *compress_buff = data ;

	write = data ;
	
	
	for ( y = 0 ; y < height ; y+=4 ){
		for ( x = 0 ; x < width ; x+=4 ){//..4*4ブロック毎移動
			beta_flag = 0 ;
			black_flag = 0 ;
			/* キーカラーの選定 */
			count = 0 ;
#if 1
			/* 減色によるキーカラー候補選定 */
			for ( j = 0 ; j < 4 ; j++ ){
				for ( i = 0 ; i < 4 ; i++ ){
					int		 index ;
					index = ( x + i ) + ( y + j ) * width ;
					col[ count++ ] = image[ index ];
					//col[ count-1 ] = add_dithe_color( col[ count-1 ], i, j );
					beta_flag += col[0] ^ image[ index ] ;
					black_flag += image[ index ];
				}
			}
			//..            32bit*16   32bit*16  16
			LessenColor( (void*)col, (void*)col, count, 2 ); count = 2 ;
			if ( black_flag == 0 ) beta_flag = 1 ;
#endif
#if 1
			/* 隣接ピクセルを含めてキーカラー候補選定 */
			for ( j = -1 ; j < 5 ; j++ ){
				int		xx, yy ;
				yy = y + j ;
				if ( yy < 0 ) continue ;
				if ( yy >= height ) continue ;
				for ( i = -1 ; i < 5 ; i++ ){
					int		 index ;
					xx = x + i ;
					if ( xx < 0 ) continue ;
					if ( xx >= width ) continue ;
					index = xx + yy * width ;
					col[ count ] = image[ index ];
					//col[ count ] = add_dithe_color( col[ count ], i, j );
					count++ ;
				}
			}
			//LessenColor( (void*)&col[ count ], (void*)col, count, 2 );
			//count += 2 ;
#endif
			/* 総当り検索用データの構築 */
			MakeColorListTable( (RGBA*)col, count, beta_flag );
			/* 圧縮に使用する色を集める */
			for ( j = 0 ; j < 4 ; j++ ){
				for ( i = 0 ; i < 4 ; i++ ){
					int		 index ;
					index = ( x + i ) + ( y + j ) * width ;
					col[ i + j * 4 ] = image[ index ];
				}
			}
			/* 総当りにより一番誤差の少なかった色をキーカラーに選ぶ */
			//..              result(16bit*2) in(RGBA)
			SelectBlockColor( select_col, (void*)col );
#if 0
			/* DXT3にてアルファ圧縮 */
			EncodeDXT_AlphaDXT3( write, (void*)col );
			write += 4 ;
#endif
			/* DXT1にてカラー圧縮（１ビットアルファ使用） */
			//EncodeDXT_Color( write, select_col, (void*)col );
			EncodeDXT1_ColorPenetration( write, select_col, (void*)col );
			write += 4 ;
		}
	}

	return ( data );
}


