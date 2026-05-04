#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<math.h>
#include	<ctype.h>
#include	"quat.h"
#include	"util.h"

typedef	struct	{
    short	vx ;
    short	vy ;
    short	vz ;
	short   pad;
} SVECTOR ;
typedef struct {
	short	x ;
	short	y ;
	short	z ;
	short	w ;
}SQUAT ;

#define		MAX_NAME_LEN	(64)
#define		MAX_N_MODELS	(64)
#define		MAX_FRAMES (1024)
#define ABS(a) ( (a >= 0) ? a : -a )



/* プロトタイプ宣言 */
void MatrixRotZYX( FMATRIX *mat, SVECTOR *rot );
/* 指定したビット長の数字を入れる(MAX bit=16) */
void PushData( int bit, int n );
/* 指定したビット位置から数字を取り出す(MAX bit=16) */
unsigned int PopDataU( int position, int bit );
/* データ記憶位置をショート単位に揃える */
void DataPositionFix( void );
/* ＰＳ用に有効桁数を落として補間処理を行う */
void QuatSquad( FVECTOR *res, FVECTOR *_q1, FVECTOR *_a, FVECTOR *_b, FVECTOR *_q2, float t );
/* 次のキーフレームを検索する */
int	GetNextKeyFrame( SVECTOR *rots, int now, int max_length );
/* 前のキーフレームを検索する */
int	GetPrevKeyFrame( SVECTOR *rots, int now, int max_length );
/* 補間用クォータニオンを計算する */
void MakeKeyParam( SVECTOR *rots, FVECTOR *data, FVECTOR *correct, int max_length );
/* 現在のデータで一定以上の角度差が発生する場合に追加するキーフレーム番号を返す */
int CheckRotData( SVECTOR *rots, FVECTOR *data, FVECTOR *correct, int max_length );
/* 補間間隔が１２７フレーム以上ある場合に途中でキーを入れて全て１２７フレーム以内に収める */
void CheckKeyLength( SVECTOR *rots, FVECTOR *data, FVECTOR *correct, int max_length );
/* 現在のモーションキーから逆算したＭｔｄファイル用モーションデータを作成 */
void MakeDebugData( SVECTOR *rots, FVECTOR *data, FVECTOR *correct, int max_length, SVECTOR *debug );
/* 念のため大きさをチェックしてバッファへ書き込む */
int WriteMotionVectorData( int data );
/* 移動データをデータバッファに登録する */
void MakeMoveData( SVECTOR *center, int length );


#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif

typedef struct {
	int model_units;
	int motion_length;
} MOTION_FILE_HEAD;

typedef struct {
	long	name_id ;			/* ファイル名ＩＤ */
	long	flag ;				/* 各種フラグ情報 */
	long	motion_base_tick ;	/* モーションのフレームベースカウント（１／３００秒単位） */
	long	motion_length ;		/* モーションフレーム数 */
	short	*archives_data ;	/* アーカイブデータへのポイント */
	long	archives_size ;		/* アーカイブデータのサイズ（ハーフワード単位） */
	long	rot_units ;			/* 可動関節ユニット数 */
	long	pad ;				/* リザーブ */
	long	move_offset ;		/* 移動量データのオフセット */
	long	turn_offset ;		/* 方向転換情報のオフセット */
	long	fix_offset ;		/* 関節固定情報のオフセット */
	long	rots_offset[NULL_ARRAY] ;		/* 関節回転情報のオフセット */
} MT3_FILE_HEADER ;

#define MT3_FLAG_MOVE_PART	(0x00000001)
#define MT3_FLAG_ROT_PART	(0x00000002)
#define MT3_FLAG_FIX_PART	(0x00000004)
#define MT3_FLAG_TURN_PART	(0x00000008)

typedef struct {
	SQUAT	rot, c_rot ;
	short	length ;
} MOTION_KEY_DATA ;

/*	グローバル	*/
char		*fname ;
int		N_MODELS ;
int		ConvMode = 1;
int		ConvQuality = 10 ;
SVECTOR	*center ;
SVECTOR	*center2 ;
SVECTOR	*LocalRots;
SVECTOR	*LocalRots2;
int		MotionDataPos;
int		MotionDataBitPos;
unsigned short	*MotionData;
unsigned long	MotionIndex[MAX_N_MODELS+1];
MOTION_FILE_HEAD MotionInfo;
int		TotalKeyCount ;

MT3_FILE_HEADER	*Mt3FileHeader ;

/* ------------------ プログラムサブルーチン ------------------ */
/*	MOTIONS構造体の初期化	*/
static	void	InitMOTIONS( void )
{
    int		i ;

	LocalRots = malloc( sizeof(SVECTOR) * MAX_N_MODELS * MAX_FRAMES );
	LocalRots2 = malloc( sizeof(SVECTOR) * MAX_N_MODELS * MAX_FRAMES );
	MotionData = malloc( sizeof(short) * 64 * 1024 );
    center = ( SVECTOR * )malloc( sizeof( SVECTOR ) * MAX_FRAMES ) ;
    center2 = ( SVECTOR * )malloc( sizeof( SVECTOR ) * MAX_FRAMES ) ;
	Mt3FileHeader = malloc( sizeof( MT3_FILE_HEADER ) + sizeof( long ) * MAX_N_MODELS );

	fprintf( stderr, "Motion Init OK !!\n") ;
}

static int LoadMotionData( char *fname )
{
    FILE	*fp ;
    int		i, j ;

    fp = fopen( fname, "rb" ) ;
    if ( fp == NULL ){
		fprintf( stderr, "Can not open file <%s> !!\n", fname ) ;
		return (1);
	}
	{/* ヘッダー部分の書き込み */
		fread( &MotionInfo, sizeof(MOTION_FILE_HEAD), 1, fp );
		RecoverBigEndianLong( &MotionInfo, sizeof(MOTION_FILE_HEAD)/4 );
		printf("units:%d length:%d\n",MotionInfo.model_units, MotionInfo.motion_length);
	}
	{/*  */
		fread( center, sizeof(SVECTOR), MotionInfo.motion_length+1, fp );
		RecoverBigEndianShort( center, ( sizeof(SVECTOR) * ( MotionInfo.motion_length + 1 ) ) / 2 );
	}
	{/*  */
		fread( LocalRots, sizeof(SVECTOR), (MotionInfo.motion_length+1) * MotionInfo.model_units, fp );
		RecoverBigEndianShort( LocalRots, ( sizeof(SVECTOR) * ( (MotionInfo.motion_length+1) * MotionInfo.model_units ) ) / 2 );
	}
    fclose( fp ) ;
	return (0);
}

static	void	MakeMOTIONS( void ) 
{
    int		i, j ;
    unsigned short	length ;
    char	*name ;
    float	time ;
	SVECTOR	*rots_base, *debug_base ;
	FVECTOR	*RotData ;
	FVECTOR	*RotCorrectData ;
    
	/* モーション長計算 */
    length = MotionInfo.motion_length ;
	N_MODELS = MotionInfo.model_units ;

	/* メモリ確保 */
	memset( MotionData, 0, 64*1024 );
	RotData = malloc( MAX_FRAMES * sizeof( FVECTOR ) );
	RotCorrectData = malloc( MAX_FRAMES * sizeof( FVECTOR ) );

    /*	中心移動データをデバッグ用に保存	*/
    for ( i = length; i >= 0; i -- ) {
		center2[ i ] = center[ i ] ;
    }
    /*	中心移動データを差分データに	*/
    for ( i = length; i > 0; i -- ) {
		center[ i ].vx -= center[ i - 1 ].vx ;
		center[ i ].vz -= center[ i - 1 ].vz ;
    }
    center ++ ;

	Mt3FileHeader->name_id = 0 ;
	Mt3FileHeader->flag = MT3_FLAG_ROT_PART | MT3_FLAG_MOVE_PART ;
	Mt3FileHeader->motion_base_tick = 5 ;/* 1/60秒データ */
	Mt3FileHeader->motion_length = length ;
	Mt3FileHeader->archives_data = (void*)( sizeof( MT3_FILE_HEADER ) + sizeof( long ) * N_MODELS ) ;
	Mt3FileHeader->rot_units = N_MODELS ;
	Mt3FileHeader->fix_offset = 0 ;

	TotalKeyCount = 0 ;
	MotionDataPos = 0 ;
	MotionDataBitPos = 0 ;

	Mt3FileHeader->move_offset = MotionDataPos ;
	MakeMoveData( center, length );

	for ( i = 0 ; i < N_MODELS ; i++ ){
		int	total_key = 0 ;
		int ret ;

		rots_base = &LocalRots[ i * ( length + 1 ) ] ;
		debug_base = &LocalRots2[ i * ( length + 1 ) ] ;
		/* 回転データをクォータニオンに変換 */
		for ( j = 1 ; j <= length ; j++ ){
			FVECTOR euler_rot ;
			euler_rot.vx = M_PI * rots_base[j].vx / 2048.0 ;
			euler_rot.vy = M_PI * rots_base[j].vy / 2048.0 ;
			euler_rot.vz = M_PI * rots_base[j].vz / 2048.0 ;
			MT_EulerToQuatXYZ( &RotData[ j ], &euler_rot );
			MT_QuatNormalize( &RotData[ j ], &RotData[ j ] );
			RotCorrectData[ j ] = RotData[ j ] ;
		}
		if ( 1 ){
			/* ループモーション */
			RotData[0] = RotData[length] ;
			RotData[length+1] = RotData[1] ;
			RotData[length+2] = RotData[2] ;
		} else {
			/* 非ループモーション */
			RotData[0] = RotData[1] ;
			RotData[length+1] = RotData[length] ;
			RotData[length+2] = RotData[length] ;
		}
		/*
			0 (1frame)              --- 移動量算出＆補間パラメータ算出用ダミーフレーム
			1~length(length frame)  --- メインモーションデータ（モーションデータとなるデータ）
			length+1(1frame)        --- 最終キーフレーム用データ（モーションデータに含まない）
			length+2(1frame)        --- 補間パラメータ算出用ダミーフレーム
		*/

		/* キーフレームチェック */
		rots_base[1].pad = 1 ;
		TotalKeyCount += 2 ;
		total_key = 2 ;

		do {
			MakeKeyParam( rots_base, RotData, RotCorrectData, length );
			ret = CheckRotData( rots_base, RotData, RotCorrectData, length );
			if ( ret != -1 ){
				/* 一番差のある場所にキーを追加 */
				rots_base[ ret ].pad = 1 ;
				TotalKeyCount++ ;
				total_key++ ;
				continue ;
			}
			break ;
		} while ( ret != -1 );
		CheckKeyLength( rots_base, RotData, RotCorrectData, length );
		/* 決定されたキーからデバッグ用のモーションデータを生成 */
		MakeKeyParam( rots_base, RotData, RotCorrectData, length );
		MakeDebugData( rots_base, RotData, RotCorrectData, length, debug_base );
		printf("joint %d: %d keys\n", i, total_key );

		{/* 決定したキーデータを駄長ビットを切り捨てながらメモリに保存 */
			int		count = 0, old_key_num = 1, len ;
			SQUAT	rot, c_rot ;

			Mt3FileHeader->rots_offset[i] = MotionDataPos ;
			for ( j = 1 ; j <= (length+1) ; j++ ){
				if ( ( rots_base[j].pad == 1 ) || ( j == ( length + 1 ) ) ){
					len = j - old_key_num ;
					rot.x = RotData[j].vx * 2047 ;
					rot.y = RotData[j].vy * 2047 ;
					rot.z = RotData[j].vz * 2047 ;
					rot.w = RotData[j].vw * 2047 ;
					c_rot.x = RotCorrectData[j].vx * 2047 ;
					c_rot.y = RotCorrectData[j].vy * 2047 ;
					c_rot.z = RotCorrectData[j].vz * 2047 ;
					c_rot.w = RotCorrectData[j].vw * 2047 ;
					if ( rot.x!=c_rot.x || rot.y!=c_rot.y || rot.z!=c_rot.z || rot.w!=c_rot.w ){
						PushData( 8, len );/* 先頭キー以外は必ず１以上の値になるはず */
						WriteMotionVectorData( rot.x );
						WriteMotionVectorData( rot.y );
						WriteMotionVectorData( rot.z );
						WriteMotionVectorData( rot.w );
						WriteMotionVectorData( c_rot.x );
						WriteMotionVectorData( c_rot.y );
						WriteMotionVectorData( c_rot.z );
						WriteMotionVectorData( c_rot.w );
					} else {
						PushData( 8, len|0x80 );/* 先頭キー以外は必ず１以上の値になるはず */
						WriteMotionVectorData( rot.x );
						WriteMotionVectorData( rot.y );
						WriteMotionVectorData( rot.z );
						WriteMotionVectorData( rot.w );
					}
					old_key_num = j ;
					count++ ;
				}
			}
			DataPositionFix();
		}

	}
	Mt3FileHeader->archives_size = MotionDataPos ;

	printf("Total key count : %d\n", TotalKeyCount );
	printf("Total data size %d\n", MotionDataPos * 2 );

	free( RotData );
	free( RotCorrectData );

}

static	void	SaveMOTIONS( char *fname )
{
    FILE	*fp ;
    char	out[ 256 ] ;
    int		i, j, len ;

    strcpy( out, fname ) ;
	out[strlen(fname)-1] = '3';
    
    fp = fopen( out, "wb" ) ;
    if ( fp == NULL ) return ;

	{/* ヘッダー部分の書き込み */
		len = ( sizeof(MT3_FILE_HEADER) + sizeof( long ) * Mt3FileHeader->rot_units ) / 4 ;
		ChangeLittleEndianLong( Mt3FileHeader, len );
		fwrite( Mt3FileHeader, 4, len, fp );
		RecoverLittleEndianLong( Mt3FileHeader, len );
	}
	{/* アーカイブデータの書き込み */
		ChangeLittleEndianShort( MotionData, Mt3FileHeader->archives_size );
		fwrite( MotionData, sizeof(short), Mt3FileHeader->archives_size, fp );
	}

    fclose( fp ) ;
}

/*	mtd形式でセーブ		*/
static	void	SaveMOTIONS2( char *fname )
{
    FILE	*fp ;
    char	out[ 256 ] ;
    int		i, j ;

    strcpy( out, fname ) ;
	out[strlen(fname)-1] = 'd';
    
    fp = fopen( out, "wb" ) ;
    if ( fp == NULL ) return ;

	{/* ヘッダー部分の書き込み */
		MOTION_FILE_HEAD tmp ;
		tmp = MotionInfo ;
		ChangeBigEndianLong( &tmp, sizeof(MOTION_FILE_HEAD)/4 );
		fwrite( &tmp, sizeof(MOTION_FILE_HEAD), 1, fp );
	}
	{/*  */
		ChangeBigEndianShort( center2, ( sizeof(SVECTOR) * ( MotionInfo.motion_length + 1 ) ) / 2 );
		fwrite( center2, sizeof(SVECTOR), MotionInfo.motion_length+1, fp );
	}
	{/*  */
		ChangeBigEndianShort( LocalRots2, ( sizeof(SVECTOR) * ( (MotionInfo.motion_length+1) * MotionInfo.model_units ) ) / 2 );
		fwrite( LocalRots2, sizeof(SVECTOR), (MotionInfo.motion_length+1) * MotionInfo.model_units, fp );
	}

    fclose( fp ) ;
}

/*-------------------------------*/
/* ------------------ メインプログラム ------------------ */

int
main( int argc, char **argv )
{
	int data_num[256],num_count = 0;
	int i,mode = -1;
	char *data[256];

	/* ヘルプの表示 */
	if ( argc < 2 ){
		fprintf( stderr, "New version motion converter 2.10 ( mt1 > mt3 )   by Takabe\n");
		fprintf( stderr, "Usage: mt3conv2 [-options] mt1-file(*.mt1) \n");
		fprintf( stderr, "Option    -d ... Make debug data (*.mtd)\n");
		fprintf( stderr, "          -D ... Only make debug data (*.mtd)\n");
		fprintf( stderr, "          -q ... Quality(0.1DEG)\n");
		return (0);
	}
	/* オプションチェック */
	if ( argc > 256 ) argc = 256;
	ConvMode = 1 ;/* 変換モード（デフォルトは通常変換のみ） */
	for (i=1;i<argc;i++){
		if (argv[i][0] == '-'){
			if ( argv[i][1] == 'd' ){
				ConvMode |= 2 ;/* 通常変換＆デバッグデータ生成 */
			}
			if ( argv[i][1] == 'D' ){
				ConvMode &= ~1 ;/* デバッグデータオンリー */
				ConvMode |= 2 ;/* デバッグデータオンリー */
			}
			if ( argv[i][1] == 'q' ){
				if ( argv[i][2] != '\0' ){
					ConvQuality = atoi( &argv[i][2] ) ;
				}
				printf("Convert Quality = %f DEG\n", (float)ConvQuality / 10.0 );
			}
		} else {
			data[num_count++] = argv[i];
		}
	}

	return (ProgMain( num_count, data ));
}

/* メインプログラム */
int ProgMain( int argc, char **argv )
{
	int i;

	InitMOTIONS();

	for (i=0;i<argc;i++){
		if ( LoadMotionData( argv[i] ) ) continue;
		MakeMOTIONS() ;
		/* 通常データ出力 */
		if ( ConvMode & 1 ) SaveMOTIONS( argv[i] ) ;
		/* デバッグ用データ出力 */
		if ( ConvMode & 2 ) SaveMOTIONS2( argv[i] ) ;
		fprintf( stderr, "Convert OK!\n");
	}

	return (0);
}

/* ------------------ プログラムサブルーチン ------------------ */
/* --------- utils --------- */
void MatrixRotZYX( FMATRIX *mat, SVECTOR *rot )
{
	float		sx, cx, sy, cy, sz, cz ;
	float		vx, vy, vz ;

	sy = -mat->m[ 0 ][ 2 ] ;
	cy = sqrt( 1.0 * 1.0 - sy * sy ) ;
	if ( cy < 0.0000001f ) {
		vy = ( sy > 0.0 ) ? M_PI/2 : M_PI/2*3 ;
		vx = atan2( - mat->m[ 2 ][ 1 ], mat->m[ 1 ][ 1 ] ) ;
		vz = 0 ;
	} else {
		vy = atan2( sy, cy ) ;
		sx = mat->m[ 1 ][ 2 ] / cy ;
		cx = mat->m[ 2 ][ 2 ] / cy ;
		vx = atan2( sx, cx ) ;
		sz = mat->m[ 0 ][ 1 ] / cy ;
		cz = mat->m[ 0 ][ 0 ] / cy ;
		vz = atan2( sz, cz ) ;
	}
	rot->vx = (short)( ( vx / M_PI ) * 2048 ) ;
	rot->vy = (short)( ( vy / M_PI ) * 2048 ) ;
	rot->vz = (short)( ( vz / M_PI ) * 2048 ) ;
}

/* データバッファ操作関連 */
/* 指定したビット長の数字を入れる(MAX bit=16) */
void PushData( int bit, int n )
{
	int pos,bit_offset,short_offset;
	unsigned int work,data,mask;

	bit_offset = MotionDataBitPos & 0x0f;
	short_offset = MotionDataBitPos >> 4;
	mask = (1 << bit) - 1;
	work = MotionData[ short_offset ] + MotionData[ short_offset+1 ]*65536;
	data = ((unsigned int)(n & mask)) << bit_offset;
	work |= data;
	MotionData[ short_offset   ] = work & 0xffff;
	MotionData[ short_offset+1 ] = (work >> 16) & 0xffff;
	MotionDataBitPos += bit;
	MotionDataPos = MotionDataBitPos / 16;
}
/* 指定したビット位置から数字を取り出す(MAX bit=16) */
unsigned int PopDataU( int position, int bit )
{
	int pos,bit_offset,short_offset;
	unsigned int work,data,mask;

	bit_offset = position & 0x0f;
	short_offset = position >> 4;
	mask = (1 << bit) - 1;
	work = MotionData[ short_offset ] + MotionData[ short_offset+1 ]*65536;
	work >>= bit_offset;
	data = work & mask;

	return (data);
}
/* データ記憶位置をショート単位に揃える */
void DataPositionFix( void )
{
	int tmp;
	tmp = MotionDataBitPos + 15;
	MotionDataBitPos = tmp & 0xfffffff0;
	MotionDataPos = tmp / 16 ;
}

/* ------------------ プログラムサブルーチン ------------------ */
/* ＰＳ用に有効桁数を落として補間処理を行う */
void QuatSquad( FVECTOR *res, FVECTOR *_q1, FVECTOR *_a, FVECTOR *_b, FVECTOR *_q2, float t )
{
	FVECTOR		q1, q2, a, b ;
	volatile int			x, y, z, w ;
	x = _q1->vx * 2047 ; q1.vx = (float)x / 2047.0 ;
	y = _q1->vy * 2047 ; q1.vy = (float)y / 2047.0 ;
	z = _q1->vz * 2047 ; q1.vz = (float)z / 2047.0 ;
	w = _q1->vw * 2047 ; q1.vw = (float)w / 2047.0 ;
	x = _q2->vx * 2047 ; q2.vx = (float)x / 2047.0 ;
	y = _q2->vy * 2047 ; q2.vy = (float)y / 2047.0 ;
	z = _q2->vz * 2047 ; q2.vz = (float)z / 2047.0 ;
	w = _q2->vw * 2047 ; q2.vw = (float)w / 2047.0 ;
	x = _a->vx * 2047 ; a.vx = (float)x / 2047.0 ;
	y = _a->vy * 2047 ; a.vy = (float)y / 2047.0 ;
	z = _a->vz * 2047 ; a.vz = (float)z / 2047.0 ;
	w = _a->vw * 2047 ; a.vw = (float)w / 2047.0 ;
	x = _b->vx * 2047 ; b.vx = (float)x / 2047.0 ;
	y = _b->vy * 2047 ; b.vy = (float)y / 2047.0 ;
	z = _b->vz * 2047 ; b.vz = (float)z / 2047.0 ;
	w = _b->vw * 2047 ; b.vw = (float)w / 2047.0 ;
	MT_QuatSquad( res, &q1, &a, &b, &q2, t );
}

/* 次のキーフレームを検索する */
int	GetNextKeyFrame( SVECTOR *rots, int now, int max_length )
{
	int		i ;
	for ( i = now + 1 ; i <= max_length ; i++ ){
		if ( rots[i].pad == 1 ) return ( i );
	}
	if ( now <= max_length ) return ( max_length + 1 );
	return ( max_length + 2 ) ;
}

/* 前のキーフレームを検索する */
int	GetPrevKeyFrame( SVECTOR *rots, int now, int max_length )
{
	int		i ;
	for ( i = now - 1 ; i > 0 ; i-- ){
		if ( rots[i].pad == 1 ) return ( i );
	}
	return ( 0 ) ;
}

/* 補間用クォータニオンを計算する */
void MakeKeyParam( SVECTOR *rots, FVECTOR *data, FVECTOR *correct, int max_length )
{
	int now_key, prev_key, next_key, j, k, width ;
	for ( j = 1 ; j <= (max_length+1) ; ){
		now_key = j ;
		prev_key = GetPrevKeyFrame( rots, j, max_length );
		next_key = GetNextKeyFrame( rots, j, max_length );
		MT_QuatQCompA( &correct[now_key], &data[prev_key], &data[now_key], &data[next_key] );
		j = next_key ;
	}
}

/* 現在のデータで一定以上の角度差が発生する場合に追加するキーフレーム番号を返す */
int CheckRotData( SVECTOR *rots, FVECTOR *data, FVECTOR *correct, int max_length )
{
	int now_key, prev_key, next_key, i, j, width ;
	FVECTOR	q1, q2 ;
	float	t, c, base_cos ;
	float	max_diff_angle = 1.0, angle ;
	int		max_diff_index = -1 ;

	base_cos = cos( M_PI*( (float)ConvQuality / 10.0 )/180.0 );
	for ( i = 1 ; i <= max_length ; ){
		now_key = i ;
		next_key = GetNextKeyFrame( rots, i, max_length );
		width = next_key - now_key ;
		for ( j = 0 ; j < width ; j++ ){
			t = (float)j / (float)width ;
			MT_QuatSquad( &q1, &data[now_key], &correct[now_key], &correct[next_key], &data[next_key], t );
			c = MT_QuatDot( &data[now_key+j], &q1 );
			/* ここで角度差をチェック */
			if ( c < base_cos ){ 
				if ( c < max_diff_angle ){
					max_diff_angle = c ;
					max_diff_index = now_key + j ;
				}
				//return ( ( now_key + next_key ) / 2 ); /* 失敗 */
			}
		}
		i = next_key ;
	}
	return ( max_diff_index );
}

/* 補間間隔が１２７フレーム以上ある場合に途中でキーを入れて全て１２７フレーム以内に収める */
void CheckKeyLength( SVECTOR *rots, FVECTOR *data, FVECTOR *correct, int max_length )
{
	int now_key, prev_key, next_key, i, j, width ;

	for ( i = 1 ; i <= max_length ; ){
		now_key = i ;
		next_key = GetNextKeyFrame( rots, i, max_length );
		width = next_key - now_key ;
		while ( width > 127 ){
			rots[ now_key + 127 ].pad = 1 ;
			now_key += 127 ;
			width -= 127 ;
			TotalKeyCount++ ;
		}
		i = next_key ;
	}
}

/* 現在のモーションキーから逆算したＭｔｄファイル用モーションデータを作成 */
void MakeDebugData( SVECTOR *rots, FVECTOR *data, FVECTOR *correct, int max_length, SVECTOR *debug )
{
	int now_key, prev_key, next_key, i, j, width ;
	FVECTOR	q1, q2 ;
	float	t, c ;
	FMATRIX	m ;

	for ( i = 1 ; i <= max_length ; ){
		now_key = i ;
		next_key = GetNextKeyFrame( rots, i, max_length );
		width = next_key - now_key ;
#if 0
		{
			int	dx, dy, dz, dw ;
			dx = (int)(data[now_key].x*4096) - (int)(correct[now_key].x*4096) ;
			dy = (int)(data[now_key].y*4096) - (int)(correct[now_key].y*4096) ;
			dz = (int)(data[now_key].z*4096) - (int)(correct[now_key].z*4096) ;
			dw = (int)(data[now_key].w*4096) - (int)(correct[now_key].w*4096) ;
			if ( !( dx | dy | dz | dw ) ){
		printf("%5d %5d %5d %5d ", 
			   (int)(data[now_key].x*4096),
			   (int)(data[now_key].y*4096),
			   (int)(data[now_key].z*4096),
			   (int)(data[now_key].w*4096) );
		printf("%5d %5d %5d %5d\n",
			   (int)(correct[now_key].x*4096),
			   (int)(correct[now_key].y*4096),
			   (int)(correct[now_key].z*4096),
			   (int)(correct[now_key].w*4096) );
			}
		}
#endif
		for ( j = 0 ; j < width ; j++ ){
			t = (float)j / (float)width ;
			//gluQuatSquad_EXT( &data[now_key], &correct[now_key], &correct[next_key], &data[next_key], t, &q1 );
			QuatSquad( &q1, &data[now_key], &correct[now_key], &correct[next_key], &data[next_key], t );
			MT_QuatToMat( &m, &q1 );
			MatrixRotZYX( &m, &debug[now_key+j] );
			debug[now_key+j].pad = rots[now_key+j].pad ;
		}
		i = next_key ;
	}
}

/* 念のため大きさをチェックしてバッファへ書き込む */
int WriteMotionVectorData( int data )
{
	if ( data < -2047 ) data = -2047 ;
	if ( data > 2047 ) data = 2047 ;
	PushData( 12, data );
}

/* 移動データをデータバッファに登録する */
void MakeMoveData( SVECTOR *center, int length )
{
	SVECTOR *vec;
	int i,pos,ave_y;
	int max_bit_x,max_bit_y,max_bit_z,tmp;

	pos = MotionDataPos;
	vec = malloc( sizeof(SVECTOR) * length );
	/* Ｙの平均を求める */
	ave_y = 0;
	for (i=0;i<length;i++){
		ave_y += center[i].vy;
	}
	ave_y /= length ;

	/* Ｙを平均からの差分にする */
	for (i=0;i<length;i++){
		center[i].vy = center[i].vy - ave_y ;
	}

	/* ＸＹＺ各々の必要ビット数を求める */
	max_bit_x = max_bit_y = max_bit_z = 0;
	for (i=0;i<length;i++){
		if ( max_bit_x < (tmp = GetEffectiveBit( center[i].vx ))) max_bit_x = tmp;
		if ( max_bit_y < (tmp = GetEffectiveBit( center[i].vy ))) max_bit_y = tmp;
		if ( max_bit_z < (tmp = GetEffectiveBit( center[i].vz ))) max_bit_z = tmp;
	}

	{/* データ生成部分 */
		PushData( 16, ave_y );/* Ｙの平均 */
		PushData( 4, max_bit_x );
		PushData( 4, max_bit_y );
		PushData( 4, max_bit_z );
		PushData( 4, 0 );/* パディング */
		for (i=0;i<length;i++){
			PushData( max_bit_x, center[i].vx );
			PushData( max_bit_y, center[i].vy );
			PushData( max_bit_z, center[i].vz );
		}
	}
	DataPositionFix();

	free(vec);
}

