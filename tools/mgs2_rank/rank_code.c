/*
	汎用ランキングエンコーダ
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
	コンフィグレーション
*/

#define	DATA_LENGTH		55
#define SUM_LENGTH		5
#define CRC_LENGTH		6
#define SEED_LENGTH		6
/* 6 Bit の倍数にする */

typedef struct {
	int data;		// 32bitまで
	int len;		// 32まで
} INPUT_DATA;

#define VAL_BITS   (DATA_LENGTH+SUM_LENGTH+CRC_LENGTH)
#define TOTAL_BITS (VAL_BITS+SEED_BITS)

#define ONE_ENC_BITS 6
#define ENC_BYTES	(TOTAL_BITS/ONE_ENC_BITS)
#define SEED_BITS	SEED_LENGTH

#define BITS_MAX	TOTAL_BITS

static int seed_pos[ SEED_BITS + 1 ] = { 6, 16, 21, 24, 46, 59, BITS_MAX };

/* old */
//static char code[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!?";
#ifdef VER001117
/* ver.001117 */
//static char code[65] = "!$%&*+-:0123456789<=>?@ABCEFGHJKLMNPQRTUVWXY[]^abdefghijkmnqrtyz";
#endif
#ifdef VER001118
/* ver.001118 */
static char code[65] = "!$%&*+-:0123456789<=>?@AZCEFGHJKLMNPQRTUVWXY[]^abdefghijkmnqrtyz";
#endif

typedef struct {
	char bits[ BITS_MAX ];
	int count;
} BITS_WORK;

static void init_bits_work( BITS_WORK *work )
{
	int i;

	work->count = 0;
	for( i = 0; i < BITS_MAX; i++ ){
		work->bits[ i ] = 0;
	}
}

static void set_bit( BITS_WORK *work, int bit )
{
	work->bits[ work->count ] = bit;
	work->count ++;
}

static void set_bits( BITS_WORK *work, int value, int len )
{
	unsigned int mask;
	int i;

	mask = 1 << ( len - 1 );
	for( i = 0; i < len; i++ ){
		set_bit( work, ( mask & value ) ? 1 : 0 );
		mask = mask >> 1;
	}
}

static int get_bits( BITS_WORK *work, int start, int len )
{
	int i;
	int c;

	c = 0;
	for( i = start; i < start + len; i++ ){
		c = ( c << 1 ) | work->bits[ i ];
	}
	return c;
}

static int get_bits_byte( BITS_WORK *work, int pos )
{
	int i, ep, res;

	if( pos >= work->count ){
		return -1;
	}

	ep = pos + 8;
	if( ep > work->count ){
		ep = work->count;
	}

	res = 0;
	for( i = pos; i < ep; i++ ){
		res = ( res << 1 ) | work->bits[ i ];
	}
	return res;
}

// rand

#define SEED_OFFSET		0x43456345

static unsigned long rand_seed = 1;

int my_rand( void )
{
//	next = next * 1103515245L + 12245;
	rand_seed = 2100005341UL * rand_seed + 3321;
	return ( rand_seed >> 16 );
}

// CRC

/* 16 bit CRC 計算 */

//#define CRCPOLY 0xEDB88320
#define CRCPOLY 0x8408

static unsigned long calc_crc_bits( BITS_WORK *work )
{
	unsigned long i, j;
	unsigned short r;
	int pos;
	
	r = 0xFFFFU;

	pos = 0;

	for( ;; ){
		int c;
		if( ( c = get_bits_byte( work, pos ) ) < 0 ){
			break;
		}
		pos += 8;
		r ^= c;
		for( j = 8; j > 0; j-- ){
			if( r & 1 ){
				r = ( r >> 1 ) ^ CRCPOLY;
			} else {
				r >>= 1;
			}
		}
	}
	r = r ^ 0xFFFFU;
	return ( r >> 8 ) ^ ( r & 0xff );
}

static unsigned long calc_sum_bits( BITS_WORK *work )
{
	long sum;
	long i;
	int pos;

	sum = 0;
	pos = 0;

	for( ;; ){
		long j, c;

		if( ( c = get_bits_byte( work, pos ) ) < 0 ){
			break;
		}
		pos += 8;

		for( j = 0; j < 2; j++ ){
			sum += ( c & 0x0f );
			sum += ( c >> 4 );
		}
	}
	return ( sum & 0xff );
}

/* ------------------------------------------------------------- */

static void make_table( char *table, int len )
{
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
			t = my_rand() % len;
			c = table[ t ];
			table[ t ] = table[ i ];
			table[ i ] = c;
		}
	}
}

/* ------------------------------------------------------------- */

static void output_bits( char *result, BITS_WORK *work, int seed, int *pos, int len )
{
	int c, n, j;
	int i, sp, sm;
	char *p;

	p = result;
	c = n = 0;
	j = 0;
	
	sp = 0;
	sm = 1;

	for( i = 0; i < work->count + len; i++ ){
		if( i == pos[ sp ] ){
			sp++;
			c = ( c << 1 ) | ( ( seed & sm ) ? 1 : 0 );
			sm = ( sm << 1 );
		} else {
			c = ( c << 1 ) | work->bits[ j ];
			j++;
		}
		n++;
		if( n == ONE_ENC_BITS ){
			*( p ++ ) = code[ c ];
			n = 0;
			c = 0;
		}
	}
	*p = '\0';
}

/* ------------------------------------------------------------- */

static void swap_bits_enc( BITS_WORK *work )
{
	BITS_WORK dst;
	int i;
	static char table[ BITS_MAX ];

	make_table( table, work->count );

	init_bits_work( &dst );

	for( i = 0; i < work->count; i++ ){
		dst.bits[ table[ i ] ] = work->bits[ i ];
	}
	memcpy( work->bits, dst.bits, work->count * sizeof( char ) );
}

static void swap_bits_dec( BITS_WORK *work )
{
	BITS_WORK dst;
	int i;

	static char table[ BITS_MAX ];

	make_table( table, work->count );

	init_bits_work( &dst );

	for( i = 0; i < work->count; i++ ){
		dst.bits[ i ] = work->bits[ table[ i ] ];
	}
	memcpy( work->bits, dst.bits, work->count * sizeof( char ) );
}

/* ------------------------------------------------------------- */

static void encode( char *result, INPUT_DATA *input, int num, int seed )
{
	unsigned long value;
	int mask;
	int i;
	int crc, sum;
	BITS_WORK Work, *work;

	seed = seed & ( ( 1 << SEED_LENGTH ) - 1 );

	rand_seed = seed + SEED_OFFSET;
	mask = ( my_rand() << 14 ) + my_rand();

	work = &Work;

	init_bits_work( work );

	for( i = 0; i < num; i++ ){
		int data;
		data = input->data ^ mask;
printf( "data = %x %x %x\n", input->data, data, mask );
		set_bits( work, data, input->len );
		input ++;
	}

	sum = calc_sum_bits( work );
	set_bits( work, sum, SUM_LENGTH );
	crc = calc_crc_bits( work );
	set_bits( work, crc, CRC_LENGTH );
printf( "SUM = %x CRC = %x\n", sum, crc );

	swap_bits_enc( work );

	output_bits( result, work, seed, seed_pos, SEED_BITS );
}

/* ------------------------------------------------------------- */

static void get_data( BITS_WORK *work, char *string, int len )
{
	char *p;
	int i, n;
	p = string;
	n = 0;
	for( i = 0; i < len; i++ ){
		char *res;
		if( ( res = strchr( code, *p ) ) != NULL ){
			int c, j, m;

			c = res - code;
			m = ( 1 << ( ONE_ENC_BITS - 1 ) );

			for( j = 0; j < ONE_ENC_BITS; j++ ){
				work->bits[ n ] = ( ( c & m ) ? 1 : 0 );
				m = m >> 1;
				n++;
			}
		} else {
			printf( "ILLEGAL CODE '%c'!!\n", *p );
			exit( 1 );
		}
		p++;
	}
	work->count = n;
}

static int get_enc_bits( BITS_WORK *work, BITS_WORK *org, int *pos, int len )
{
	int i, n, m;
	int seed;
	int sp;
	
	sp = 0;
	seed = 0;
	n = 0;
	m = 1;

	for( i = 0; i < BITS_MAX; i++ ){
		if( i == pos[ sp ] ){
			if( org->bits[ i ] != 0 ){
				seed = seed | m;
			}
			m = m << 1;
			sp ++;
		} else {
			work->bits[ n ] = org->bits[ i ];
			n ++;
		}
	}
	return seed;
}

static void decode( INPUT_DATA *input, int num, char *string )
{
	static char data[ ENC_BYTES ];
	int seed;
	int mask;
	BITS_WORK org_bit;
	BITS_WORK Work, *work;

printf( "DECODE:\n" );

	get_data( &org_bit, string, sizeof( data ) );

	work = &Work;

	init_bits_work( work );
	seed = get_enc_bits( work, &org_bit, seed_pos, SEED_BITS );
printf( "SEED %d\n", seed );

	rand_seed = seed + SEED_OFFSET;
	mask = ( my_rand() << 14 ) + my_rand();
printf( "mask = %x\n", mask );
	work->count = VAL_BITS;

	swap_bits_dec( work );
	{
		int crc, sum;
		int _crc, _sum;
		int pos1, pos2;

		int i, pos;

		pos = 0;
		for( i = 0; i < num; i++ ){
			int data;
			data = get_bits( work, pos, input->len );
			input->data = ( data ^ mask ) & ( ( 1 << input->len ) - 1 );
			pos += input->len;
			input ++;
		}

		sum = get_bits( work, pos, SUM_LENGTH );
		pos1 = pos;
		pos += SUM_LENGTH;
		crc = get_bits( work, pos, CRC_LENGTH );
		pos2 = pos;
		pos += SUM_LENGTH;

		printf( "V:CRC %X:SUM %X\n", crc, sum );

		work->count = pos1;
		_sum = calc_sum_bits( work ) & ( ( 1 << SUM_LENGTH ) - 1 );

		work->count = pos2;
		_crc = calc_crc_bits( work ) & ( ( 1 << CRC_LENGTH ) - 1 );

		printf( "C:CRC %X:SUM %X\n", _crc, _sum );
	}
}

/* ------------------------------------------------------------- */

static void usage( void )
{
	printf( "Usage:\n" );
	printf( "\trankenc e <score> <flag> <seed>\n" );
	printf( "\trankenc d <code>\n" );
	exit( 1 );
}

static void PrintClearCode( INPUT_DATA *data )
{
	static char GameLevel[5][10] = {
		"VERY EASY",
		"EASY",
		"NORMAL",
		"HARD",
		"VERY HARD",
	};
	static char RadarType[3][10] = {
		"TYPE 1",
		"TYPE 2",
		"OFF",
	};
	int		temp0,temp1,temp2;
	
	temp0 = data[0].data & 0x0007;

	if(temp0 >= 5){ printf("ERR !!\n"); return;};
	printf("GameLevel    :    %s\n",GameLevel[temp0]);

	temp0 = (data[0].data>>3) & 0x0003;
	if(temp0 >= 3){ printf("ERR !!\n"); return;};
	printf("RadarType    :    %s\n",RadarType[temp0]);

	temp0 = (data[0].data>>5) & 0x7fff;
	temp2 = temp0%60;
	temp1 = (temp0/60)%60;
	temp0 = (temp0/3600);

	printf("TotalPlayTime:    %d時間%d分%d秒\n",temp0,temp1,temp2);

	temp0 = (data[0].data>>20) & 0x003f;
	printf("GameOverCount:    %d\n",temp0);


	temp0 = (data[1].data) & 0x00ff;
	printf("AlertCount   :    %d\n",temp0);
 
	temp0 = (data[1].data>>8) & 0x00ff;
	printf("FireCount    :    %d\n",temp0);

	temp0 = (data[1].data>>16) & 0x00ff;
	printf("KillCount    :    %d\n",temp0);

	temp0 = (data[1].data>>24) & 0x001f;
	printf("ダメージ率   :    %d[ダメージ/体力マックス]0<->32\n",temp0);
//[0-2]ゲームレベル（3bit) 0(veasy) <-> 4(vhard)
//[3-4]レーダーモード (2bit)
//[5-19]総プレイ時間(15bit)
//		精度は時分秒。18時間強程度まで計測。それ以上は計測不能として処理。
//		コンティニューでも時間はリセットしない。（ただしコンティニュー画面中はタイマー停止）
//[20-25]ゲームオーバー回数(6bit)
//		コンティニュー回数でもある。0から62回まで計測。それ以上は計測不能として処理。
	


//[0-7]危険モード回数(8bit)
//		危険モードに突入した回数。0から254回まで計測。それ以上は計測不能として処理。
//[8-15]発砲数(8bit)
//		USP、M9ともに発砲した総数を合算(区別の必要なし）。ハッピートリガかそうでないかが判る。
//[16-23]敵抹殺回数(8bit)
//		気絶、麻酔は数えない。殺した敵兵数のみをカウント。256人まで。
//[24-28]クリアした時間帯（5bit）
//		PS2内蔵タイマーより、クリアした時間帯(精度は時間)。何時ぐらいにユーザーが遊んでいるのかを知る手がかりになる。





}

int main( int argc, char *argv[] )
{
	int seed;

	INPUT_DATA input[ 2 ] = {
		{ 0x00000000, 26 },
		{ 0x00000000, 29 },
	};

	if( argc < 2 ){
		usage();
	}

	seed = 43;
	if( argv[ 1 ][ 0 ] == 'e' ){
		char string[ 64 ];

		encode( string, input, 2, seed );
		printf( "RESULT = %s\n", string );
	} else if( argv[ 1 ][ 0 ] == 'd' ){
		int score;
		int flag;
		decode( input, 2, argv[ 2 ] );
		printf( "1 = %x\n", input[ 0 ].data );
		printf( "2 = %x\n", input[ 1 ].data );
		PrintClearCode( input );
	} else {
		usage();
	}
	return 0;
}



