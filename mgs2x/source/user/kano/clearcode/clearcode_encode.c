//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

  クリアコードエンコーダ 可変長対応
  ついでにデコーダも書いとく

  2002/08/05 M.Kobayashi

  $Id: clearcode_encode.c,v 1.4 2002/12/04 10:29:00 takaki Exp $

 */

#include "clearcode.h"
#ifdef TOOL
#include <math.h>
#endif

#ifdef KP_WINDOWS
#include <xtl.h>
#endif

#define MAGIC_NUMBER	0xbad0deed

// clearcode.c からコピー
/* 7 bit CRC 計算
   110bitのデータであるので、7bitのCRC符号があれば充分である。
   計算は、1+x^1+x^2+x^5+x^7を使った。
   既約多項式ではあるが、原始多項式かどうか疑問？ */

#define CRCPOLY		0x74
static unsigned int calc_crc_code( void *data, int len )
{
	int i, j;
	unsigned int r;
	char *p;

	r = 0;
	p = data;

	for( i = len; i > 0; i-=8 ){
		r ^= *( p ++ );
		for( j = (i>=8 ? 8 : i) ; j > 0; j-- ){
			if( r & 1 ){
				r = ( r >> 1 ) ^ CRCPOLY;
			} else {
				r >>= 1;
			}
		}
	}
	return r;
}
static unsigned int rand_seed;
static unsigned int clr_rand( void )
{
	rand_seed = 2100005341U * rand_seed + 3321;
	return rand_seed;
}

static void make_table( u_char *table, int len )
{	// 入れ替えテーブル作成
	int i;
	int j;

	/* 初期値設定 */
	for( i = 0; i < len; i++ ){
		table[ i ] = i;
	}

	/* シャッフル４回 */
	for( j = 0; j < 4; j++ ){
		for( i = 0; i < len; i++ ){
			int c;
			int t;
			t = clr_rand() % len;
			c = table[ t ];
			table[ t ] = table[ i ];
			table[ i ] = c;
		}
	}
}


//////////////////////////////
// CODE_BIT_DATA ユーティリティ

static void CODE_BIT_DATA_Add( CODE_BIT_DATA* p, u_int data, int len )
{	// 後ろに追加
	int pos = p->bitlen;
	p->bitlen += len;
	
	while( len > 0 ) {
		int bytepos = pos / 8;
		int bitpos = pos & 7;
		int writelen = ( len > 8 - bitpos ) ? (8 - bitpos ) : len ;
		u_int mask = (( 1 << writelen ) - 1) << bitpos;

		p->data[ bytepos ] &= ~mask;
		p->data[ bytepos ] |= ( mask & ( data << bitpos ) );

		data >>= writelen;
		len -= writelen;
		pos += writelen;
	}
}

#if 0	// いまのところ使ってない
static u_int CODE_BIT_DATA_Get( CODE_BIT_DATA* p,int len )
{	// 後ろから取得
	int pos = p->bitlen;
	u_int data = 0;
	p->bitlen -= len;
	
	ASSERT( pos >= len );
	
	while( len > 0 ) {
		int bytepos = (pos - 1) / 8;
		int bitpos = (pos - 1) & 7;
		int readlen = ( len > bitpos + 1 ) ? ( bitpos + 1) : len ;
		u_int mask = (( 1 << readlen ) - 1) << (bitpos + 1 - readlen) ;

		data <<= readlen;
		data |= ( p->data[ bytepos ] & mask ) >> ( bitpos + 1 - readlen );
		len -= readlen;
		pos -= readlen;
	}
	return data;
}
#endif


static void CODE_BIT_DATA_SetBit( CODE_BIT_DATA* p, int pos, int bit )
{	// pos 位置のビットを bit にする (bit = 0 or 1)
	int shift;
	int mask;
	
	ASSERT( p->bitlen > pos );
	shift = pos & 7;
	pos /= 8;
	mask = 1 << shift;
	p->data[ pos ] = (p->data[ pos ] & ~mask) | ( bit << shift );
}

static int CODE_BIT_DATA_GetBit( CODE_BIT_DATA* p, int pos )
{	// pos 位置のビットを取得する
	int shift;
	int mask;
	
	ASSERT( p->bitlen > pos );
	shift = pos & 7;
	pos /= 8;
	mask = 1 << shift;
	return  (p->data[ pos ] & mask) >> shift;
}

static void make_perm( u_char* perm, int pool, int len, u_int seed )
{	// 0 ~ pool の中から len 取り出す順列をランダムに作る
	int i;
	int j;
	rand_seed = seed;
	for( i = 0 ; i < len; i++ ) {
		int offset;
		int ctr;
		perm[ i ] = clr_rand() % ( pool - i );
		offset = 0;
		ctr = 0;
		do {
			offset = ctr;
			ctr = 0;
			for( j = 0 ; j < i; j++ ){
				if( perm[ j ] <= perm[ i ] + offset) {
					++ctr;
				}
			}
		} while( offset != ctr );
		perm[ i ] += offset;
	}
#if 0	
	for( i = 0 ; i < len; i++ ) {
		printf("%d ", perm[ i ] );
	}
	printf("\n");
#endif	
}

static void CODE_BIT_DATA_RandAdd( CODE_BIT_DATA* p, u_int data, int len, u_int seed )
{	// ランダムビット組込み
	// ビット列を適当にばらして入れる
	u_char perm[ CODE_MAX_BYTE * 8 ];
	CODE_BIT_DATA dst;
	int i;
	int n;
	
	// 順列生成{ (p->bitlen + len) P len }
	make_perm( perm, p->bitlen + len, len, seed );

	// 組込み
	GV_ZeroMemory( dst.data, CODE_MAX_BYTE );
	dst.bitlen = p->bitlen + len;
	
	for( i = 0 ; i < len ; i++ ) {
		// まずフラグとして１を書き込む
		CODE_BIT_DATA_SetBit( &dst, perm[ i ], 1 );
	}

	for( i = 0, n = 0 ; i < dst.bitlen ; i++ ) {
		// もとのデータを入れる
		if( CODE_BIT_DATA_GetBit( &dst, i ) == 0 ) {
			CODE_BIT_DATA_SetBit( &dst, i, CODE_BIT_DATA_GetBit( p, n ) );
			++n;
		}
	}
	for( i = 0 ; i < len ; i++ ) {
		// 最後に data を入れる
		CODE_BIT_DATA_SetBit( &dst, perm[ i ], data & 1 );
		data >>= 1;
	}

	*p = dst;
}

static u_int CODE_BIT_DATA_RandGet( CODE_BIT_DATA* p, int len, u_int seed )
{	// ランダムビット抜き取り
	u_char perm[ CODE_MAX_BYTE * 8 ];
	CODE_BIT_DATA dst;	// 抜き取った後
	CODE_BIT_DATA test;	// フラグ用
	u_int data;
	int i;
	int n;

	ASSERT( p->bitlen >= len );
	
	// 順列生成{ p->bitlen P len }
	make_perm( perm, p->bitlen, len, seed );

	// 
	GV_ZeroMemory( dst.data, CODE_MAX_BYTE );
	GV_ZeroMemory( test.data, CODE_MAX_BYTE );
	dst.bitlen = p->bitlen - len;
	test.bitlen = p->bitlen;

	// データ抜き取り＆フラグ作成
	data = 0;
	for( i = len - 1 ; i >= 0 ; --i ) {
		data <<= 1;
		data |= CODE_BIT_DATA_GetBit( p, perm[ i ] );
		CODE_BIT_DATA_SetBit( &test, perm[ i ], 1 );
	}

	// データを詰める
	for( i = 0, n = 0 ; i < p->bitlen ; i++ ) {
		if( CODE_BIT_DATA_GetBit( &test, i ) == 0 ) {
			CODE_BIT_DATA_SetBit( &dst, n, CODE_BIT_DATA_GetBit( p, i ) );
			++n;
		}
	}
	
	*p = dst;
	return data;
}

static void CODE_BIT_DATA_RandXor( CODE_BIT_DATA* p, int seed )
{	// ランダム反転
	int i;

	rand_seed = seed;
	
	for( i = 0; i < p->bitlen ; i += 8 ){
		u_char mask = (u_char)clr_rand();
		p->data[i / 8] ^= mask;
	}
}

static void CODE_BIT_DATA_RandSwap( CODE_BIT_DATA* p, int seed, int encode )
{	// ランダム入れ替え
	
	static u_char table[ CODE_MAX_BYTE * 8 ];
	int i;

	CODE_BIT_DATA result;
	GV_ZeroMemory( &result, sizeof( result ) );

	rand_seed = seed;
	
	make_table( table, p->bitlen );

	for( i = 0; i < p->bitlen; i++ ){
		int index0 = i / 8;
		int shift0 = i & 7;
		int index1 = table[i] / 8;
		int shift1 = table[i] & 7;
		if( encode ) {
			int bit0 = (p->data[ index0 ] >> shift0) & 1;
			result.data[ index1 ] |= ( bit0 << shift1 );
		} else {
			int bit0 = (p->data[ index1 ] >> shift1) & 1;
			result.data[ index0 ] |= ( bit0 << shift0 );
		}
	}
	memcpy( p->data, result.data, CODE_MAX_BYTE );
}

static void CODE_BIT_DATA_CleanTail( CODE_BIT_DATA* p )
{	// 余分な部分を０クリアする
	int shift = p->bitlen & 7;
	int pos = p->bitlen / 8;
	int mask = (1 << shift) - 1;

	p->data[ pos ] &= mask;
	for( ++pos ; pos < CODE_MAX_BYTE ; ++pos ) {
		p->data[ pos ] = 0;
	}
}

////////////////

void ClearCodeEncodeEx( CODE_BIT_DATA* pDst, const CODE_BIT_DATA* pSrc, int rnd_code )
{
	int crc_code;
	int seed;

	// seed 決定
	rnd_code &= 0x1f;
	seed=(unsigned int)(1<<rnd_code)-1;
	
	// CRC 計算
	*pDst = *pSrc;
	CODE_BIT_DATA_CleanTail( pDst );
	crc_code = calc_crc_code( pDst->data, pDst->bitlen ) & 0x7f;
//	CODE_BIT_DATA_Add( pDst, crc_code, CODE_CRC_LEN );	// データに追加
	CODE_BIT_DATA_RandAdd( pDst, crc_code, CODE_CRC_LEN, seed );	// データに追加
	
	// rand 反転
	CODE_BIT_DATA_RandXor( pDst, seed );

	// rand 入れ替え
	CODE_BIT_DATA_RandSwap( pDst, seed, TRUE );

//	CODE_BIT_DATA_Add( pDst, rnd_code, CODE_RND_LEN );	// データに追加
	CODE_BIT_DATA_RandAdd( pDst, rnd_code, CODE_RND_LEN,  MAGIC_NUMBER );	// データに追加

	CODE_BIT_DATA_CleanTail( pDst );
}

int ClearCodeDecodeEx( CODE_BIT_DATA* pDst, const CODE_BIT_DATA* pSrc )
{
	int rnd_code;
	int crc_code;
	int seed;
	*pDst = *pSrc;
	
	if( pDst->bitlen < CODE_RND_LEN + CODE_CRC_LEN ) {
		printf("invalid data length\n");
		return FALSE;
	}
	
	// seed 取り出し
//	rnd_code = CODE_BIT_DATA_Get( pDst, CODE_RND_LEN );
	rnd_code = CODE_BIT_DATA_RandGet( pDst, CODE_RND_LEN, MAGIC_NUMBER );
	seed=(unsigned int)(1<<rnd_code)-1;
	
	// rand 入れ替え
	CODE_BIT_DATA_RandSwap( pDst, seed, FALSE );

	// rand 反転
	CODE_BIT_DATA_RandXor( pDst, seed );

	// CRC 取り出し
//	crc_code = CODE_BIT_DATA_Get( pDst, CODE_CRC_LEN );	
	crc_code = CODE_BIT_DATA_RandGet( pDst, CODE_CRC_LEN, seed );	
	
	// CRC 計算
	CODE_BIT_DATA_CleanTail( pDst );
	if( crc_code != ( calc_crc_code( pDst->data, pDst->bitlen ) & 0x7f ) ) {
		printf("crc error\n" );
		return FALSE;
	}
	
	CODE_BIT_DATA_CleanTail( pDst );
	
	return TRUE;
}

int ClearCodeAddRedundantBit( CODE_BIT_DATA* p, int dst_len, int dst_radix, int seed )
{	// 変換後桁数を満たすまでランダムビットを付加する
	// 必要な桁数を返す

	// -> 固定値を付加することにした
	
	int n, m;
	int addlen;
	
	n = (int)( (double)p->bitlen * (log( 2.0 ) / log( (double)dst_radix )) + 1.0 );	// 26進の必要桁数
	if( dst_len > n ) n = dst_len;
	m = (int)( (double)n * ( log( (double)dst_radix ) / log ( 2.0 ) ) ); // 2進で表現できる最大桁数
	addlen = m - p->bitlen;
	
//	CODE_BIT_DATA_Add( p, irnd(), addlen );
//	CODE_BIT_DATA_Add( p, 0xffffffff, addlen );
	CODE_BIT_DATA_Add( p, 2100005341U * seed + 3321 , addlen );

	return n;
}

void ClearCodeChangeRadix( u_char* pDst, int dst_len, int dst_radix,
						   const u_char* pSrc, int src_len, int src_radix ) 
{	// radix_from 進数から radix_to 進数にする
	// 最大 256 進数
	
	u_char s[ CODE_MAX_BYTE ];
	
	int i,j;
	unsigned int t;
	int	srcmax;

	ASSERT( src_radix <= 256 && dst_radix <= 256 );
			
	memcpy( s, pSrc, src_len );

	j = 0;
	srcmax = src_len - 1;
	while( srcmax >= 0 ) {
		t=0;
		for( i = srcmax; i>=0; i-- ){
			t= (t * src_radix ) + s[i];
			s[i]= t / dst_radix;
			t %= dst_radix;
			
			if( i == srcmax && s[i] == 0 ) --srcmax;
		}
		
		ASSERT( j < dst_len );
		pDst[j++]=t;
	}

//	printf("required buffer length %d\n", j );

	while( j < dst_len ) {
		pDst[j++] = 0;
	}
#if 0
#ifdef DEBUG_MODE
	if( dst_radix > 32 ) return;
			
	printf("ClearCode = ");

	for(i=0;i<j;i++){
		printf("%c",pDst[i]+'A');
	}

	printf("\n");
#endif
#endif	
}

u_int ClearCodeCalcStringsCrc( char* pstr )
{	// 文字列を CRC ルーチンに入れた結果を返す
	char buf[256];
	int i;
	int n;

	// スペースは無視して計算する
	for(i = 0, n = 0 ; i < 256; i++ ) {
		if( pstr[ i ] == ' ') continue;
		if( ( buf[ n++ ] = pstr[ i ] ) == '\0' ) break;
	}

	return calc_crc_code( buf, strlen(buf) );
}

#ifndef KP_WINDOWS
int	ClearCodeGenerate( u_char* pDst, int code_len, int dst_radix,
					   void* pSrcbit, int bitlen, int seed )
#else
int	ClearCodeGenerate( int mode, u_char* pDst, int code_len, int dst_radix,
					   void* pSrcbit, int bitlen, int seed )
#endif
{	// 
	CODE_BIT_DATA src;
	CODE_BIT_DATA crypt;
	memcpy( src.data, pSrcbit, (bitlen + 7) / 8 );
	src.bitlen = bitlen;

	//// システム共通データを書き込む
#ifndef TOOL	
	// プラットフォーム２ビット

#ifndef KP_WINDOWS
	CODE_BIT_DATA_Add( &src, (GM_Configuration2 & GM_CONFIG_PLATFORM) >> GM_CONFIG_PLATFORM_SHIFT, 2 );
	// 仕向け値２ビット
	CODE_BIT_DATA_Add( &src, (GM_Configuration2 & GM_CONFIG_REGION) >> GM_CONFIG_REGION_SHIFT, 2 );

#else	// ↓は、リージョンコード設定がおかしいので、とりあえず対処した物。(By Takaki)

	printf("[GM_CONFIG_PLATFORM:%d]\n", (GM_Configuration2 & GM_CONFIG_PLATFORM) >> GM_CONFIG_PLATFORM_SHIFT) ;
	printf("[GM_CONFIG_REGION:%d]\n", (GM_Configuration2 & GM_CONFIG_REGION) >> GM_CONFIG_REGION_SHIFT) ;
	CODE_BIT_DATA_Add( &src, GM_CONFIG_PLATFORM_PC, 2 );
	// 仕向け値２ビット
	CODE_BIT_DATA_Add( &src, GM_CONFIG_REGION_EU, 2 );
#endif

#else
	CODE_BIT_DATA_Add( &src, 0, 2 );
	CODE_BIT_DATA_Add( &src, 0, 2 );
#endif	

	ClearCodeEncodeEx( &crypt, &src, seed );
	// 余分なビットを埋める
	code_len = ClearCodeAddRedundantBit( &crypt, code_len, dst_radix, seed );
	// 基数変換
	ClearCodeChangeRadix( pDst, code_len, dst_radix,
						  crypt.data, (crypt.bitlen + 7) / 8, 256 );
#ifdef KP_WINDOWS
	// Windows版ではここで出力(外部登録用)
	{
		int		i ;
		char	str_buff[64] ;
		u_char	*dst_ptr ;
		u_char	*src_ptr ;

		ASSERT( code_len < (sizeof(str_buff)-1) ) ;

		// 文字列作成
		dst_ptr = (u_char *)str_buff ;
		src_ptr = pDst ;
		for(i=code_len; i>0; i--, src_ptr++, dst_ptr++)
		{
			*dst_ptr = *src_ptr + (u_char)'A' ;
		}
		*dst_ptr = 0 ;

		// パスワード出力 
		S_MGS2SSaveRankingPassword( mode, (const char *)str_buff) ;
	}
#endif
	return code_len;
}
					   
u_int ClearCodeGetData( CODE_BIT_DATA* p, int pos, int len )
{	// p の pos の位置から len の長さのデータを読む
	// デコーダ用ユーティリティ
	u_int data = 0;

	pos += len;
	ASSERT( pos <= p->bitlen );
	
	while( len > 0 ) {
		int bytepos = (pos - 1) / 8;
		int bitpos = (pos - 1) & 7;
		int readlen = ( len > bitpos + 1 ) ? ( bitpos + 1) : len ;
		u_int mask = (( 1 << readlen ) - 1) << (bitpos + 1 - readlen) ;

		data <<= readlen;
		data |= ( p->data[ bytepos ] & mask ) >> ( bitpos + 1 - readlen );
		len -= readlen;
		pos -= readlen;
	}
	return data;
}


