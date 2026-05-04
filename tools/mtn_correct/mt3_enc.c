#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gte.h"
#include "quat.h"
#include "motion.h"


/* ------------------------------------------------------------ */
/* 次のキーフレームを検索する */
static GetNextKeyFrame( KEY_INFO *key, int now, int length, int flag );
/* 前のキーフレームを検索する */
static GetPrevKeyFrame( KEY_INFO *key, int now, int length, int flag );
/* ２次補間パラメータ用クォータニオンを計算（実際にキーのあるところのみ計算） */
static void MakeCorrectQuat( FVECTOR *c_rots, FVECTOR *org_quat, KEY_INFO *key, int length, int flag );
/* 現在のデータで一定以上の角度差が発生する場合に追加するキーフレーム番号を返す */
static int CheckErrorAngle( FVECTOR *rots, FVECTOR *c_rots, KEY_INFO *key, int length, int flag, float error_angle );
/* 補間間隔が１２７フレーム以上ある場合に途中でキーを入れて全て１２７フレーム以内に収める */
static void CheckKeyLength( KEY_INFO *key, int length, int flag );
/* キーにしたがって圧縮したデータを展開したモーションを生成 */
static void MakeNewQuat( FVECTOR *out, FVECTOR *rots, FVECTOR *c_rots, KEY_INFO *key, int length, int flag );


/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */

void MTN_Mt3Encode( MTN_MOTION *out_motion, MTN_MOTION *in_motion, int flag, float error_angle )
{
	int		i, j, ret ;
	int		length_plus, length ;
	KEY_INFO	*key ;
	FVECTOR		*in_rots, *out_rots, *c_rots, *correct_rots ;

	length = in_motion->header.motion_length ;
	length_plus = in_motion->header.motion_length + 1 ;

	/* 圧縮に最低限必要なキーを設定する */
	key = in_motion->minfo ;
	key[ 0 + 1 ].key = 1 ;
	key[ length ].key = 1 ;
	for ( i = 0 ; i < in_motion->header.motion_joints ; i++ ){
		key = &in_motion->jinfo[ i * length_plus ] ;
		key[ 0 + 1 ].key = 1 ;
		key[ length ].key = 1 ;
	}
	/* コピーする */
	MTN_CopyMotion( out_motion, in_motion );


	/* 指定された誤差で関節のキーを自動生成する */
	correct_rots = malloc( sizeof(FVECTOR) * length_plus );
	for ( i = 0 ; i < out_motion->header.motion_joints ; i++ ){
		key = &out_motion->jinfo[ i * length_plus ] ;
		in_rots = &in_motion->rots[ i * length_plus ] ;
		out_rots = &out_motion->rots[ i * length_plus ] ;
		c_rots = correct_rots ;

		/* 指定誤差以内に納まるようにキーを生成する */
		do {
			MakeCorrectQuat( c_rots, in_rots, key, length, in_motion->header.flags );
			ret =  CheckErrorAngle( in_rots, c_rots, key, length, in_motion->header.flags, error_angle );
			if ( ret != -1 ){
				key[ ret ].key = 1 ;
				continue ;
			}
			break ;
		} while ( ret != -1 );
		/* 最大キー長さが１２７に納まるようにキーを短くする */
		CheckKeyLength( key, length, in_motion->header.flags );
		MakeCorrectQuat( c_rots, in_rots, key, length, in_motion->header.flags );
		/* 完成したキーで再生した場合のモーションデータを作成 */
		MakeNewQuat( out_rots, in_rots, c_rots, key, length, in_motion->header.flags );
	}

	free( c_rots );

}

/* ------------------------------------------------------------ */
/* 次のキーフレームを検索する */
static GetNextKeyFrame( KEY_INFO *key, int now, int length, int flag )
{
	int	i ;
	for ( i = now + 1 ; i <= length  ; i++ ){
		if ( key[ i ].key ) return ( i ) ;
	}
	if ( flag & MTN_FLAG_LOOP ){
		return ( 1 ) ;
	} else {
		return ( length );
	}
}
/* 前のキーフレームを検索する */
static GetPrevKeyFrame( KEY_INFO *key, int now, int length, int flag )
{
	int	i ;
	for ( i = now - 1 ; i >= 1 ; i-- ){
		if ( key[ i ].key ) return ( i ) ;
	}
	if ( flag & MTN_FLAG_LOOP ){
		return ( length ) ;
	} else {
		return ( 1 );
	}
}
/* ------------------------------------------------------------ */
/* ２次補間パラメータ用クォータニオンを計算（実際にキーのあるところのみ計算） */
static void MakeCorrectQuat( FVECTOR *c_rots, FVECTOR *org_quat, KEY_INFO *key, int length, int flag )
{
	int		i, prev_key, next_key ;
	FVECTOR	*prev, *now, *next ;
	now = org_quat + 1 ;
	for ( i = 1 ; i <= length ; ){
		prev = &org_quat[ prev_key = GetPrevKeyFrame( key, i, length, flag ) ];
		next = &org_quat[ next_key = GetNextKeyFrame( key, i, length, flag ) ];
		MT_QuatQCompA( &c_rots[i], prev, &org_quat[i], next );
		if ( i == length ) break ;
		if ( i > next_key ) break ;
		i = next_key ;
	}
}
/* 現在のデータで一定以上の角度差が発生する場合に追加するキーフレーム番号を返す */
static int CheckErrorAngle( FVECTOR *rots, FVECTOR *c_rots, KEY_INFO *key, int length, int flag, float error_angle )
{
	int prev_key, next_key, i, j, width ;
	FVECTOR	q1, q2 ;
	float	t, c, base_cos ;
	float	max_diff_angle = 1.0, angle ;
	int		max_diff_index = -1 ;

	base_cos = cos( error_angle );
	for ( i = 0 ; i < length ; ){
		next_key = GetNextKeyFrame( key, i, length, flag );
		width = next_key - i ;
		for ( j = 1 ; j <= width ; j++ ){
			t = (float)j / (float)width ;
			MT_QuatSquad( &q1, &rots[i], &c_rots[i], &c_rots[next_key], &rots[next_key], t );
			c = MT_QuatDot( &rots[i+j], &q1 );
			if ( c < 0.0f ) c = -c ;
			/* ここで角度差をチェック */
			if ( c < base_cos ){
				if ( c < max_diff_angle ){
					max_diff_angle = c ;
					max_diff_index = i + j ;
				}
			}
		}
		i = next_key ;
	}
	return ( max_diff_index );
}

/* 補間間隔が１２７フレーム以上ある場合に途中でキーを入れて全て１２７フレーム以内に収める */
static void CheckKeyLength( KEY_INFO *key, int length, int flag )
{
	int		next_key, i, width ;

	for ( i = 1 ; i < length ; ){
		next_key = GetNextKeyFrame( key, i, length, flag );
		width = next_key - i ;
		while ( width > 127 ){
			key[ i + 127 ].key = 1 ;
			i += 127 ;
			width -= 127 ;
		}
		i = next_key ;
	}
}

/* キーにしたがって圧縮したデータを展開したモーションを生成 */
static void MakeNewQuat( FVECTOR *out, FVECTOR *rots, FVECTOR *c_rots, KEY_INFO *key, int length, int flag )
{
	int prev_key, next_key, i, j, width ;
	FVECTOR	q1, q2 ;
	float	t, c, base_cos ;

	for ( i = 0 ; i < length ; ){
		next_key = GetNextKeyFrame( key, i, length, flag );
		width = next_key - i ;
		for ( j = 1 ; j <= width ; j++ ){
			t = (float)j / (float)width ;
			MT_QuatSquad( &out[i+j], &rots[i], &c_rots[i], &c_rots[next_key], &rots[next_key], t );
		}
		if ( i == length ) break ;
		i = next_key ;
	}
	return ;
}





/* ------------------------------------------------------------ */
/* 有効ビット数を返す */
static int GetEffectiveBit( int data );
/* 指定したビット長の数字を入れる(MAX bit=16) */
static void PushData( int bit, int n, unsigned short *datas, int *data_bit_pos );
/* データ記憶位置をショート単位に揃え、そのオフセットを返す */
static int DataPositionFix( int *data_bit_pos );
/* 念のため大きさをチェックしてバッファへ書き込む */
static int WriteMotionVectorData( int data, unsigned short *datas, int *data_bit_pos );
/* 移動データをデータバッファに登録する */
static void MakeMoveData( FVECTOR *center, int length, unsigned short *datas, int *data_bit_pos );
/* 関節回転データをデータバッファに登録する */
static void MakeRotsData( FVECTOR *rot, KEY_INFO *key, int length, int flag, unsigned short *datas, int *data_bit_pos );
/* 関節固定データをバッファに登録する */
static void MakeFixData( FVECTOR *move, KEY_INFO *key, int n_joints, int length, unsigned short *datas, int *data_bit_pos );
/* 移動軌跡データをバッファに登録する */
static void MakeRootData( FVECTOR *move, KEY_INFO *key, int length, unsigned short *datas, int *data_bit_pos );
/* 文字列コードの取得 */
static unsigned short MGS_GetStrCode( char *str );
/* ------------------------------------------------------------ */
/* ＭＴ３形式でのデータ保存（圧縮後ＭＴＮからＭＴ３を生成する） */
void MT3_Mt3Save( MTN_MOTION *motion, char *filename, int flag )
{
	unsigned short	*datas ;
	MT3_FILE_HEADER	*mt3_header ;
	KEY_INFO		*key ;
	FVECTOR			*rots ;
	int				data_bit_pos ;
	int				length, n_joints, m_flag ;
	int				i, data_size, header_size ;
	FILE			*fp ;

	length = motion->header.motion_length ;
	n_joints = motion->header.motion_joints ;
	m_flag = motion->header.flags ;

	header_size = sizeof( MT3_FILE_HEADER ) + sizeof( long ) * n_joints ;
	mt3_header = malloc( header_size );
	memset( mt3_header, 0, header_size );
	datas = malloc( 1024 * 1024 );
	memset( datas, 0, 1024 * 1024 );

	mt3_header->name_id = MGS_GetStrCode( filename ) ;
	mt3_header->flag = MT3_FLAG_ROT_PART | MT3_FLAG_MOVE_PART | MT3_FLAG_ROOT_PART | MT3_FLAG_FIX_PART ;
	mt3_header->motion_base_tick = motion->header.motion_tick ;
	mt3_header->motion_length = length ;
	mt3_header->archives_data = (void*)( header_size ) ;
	mt3_header->rot_units = n_joints ;
	mt3_header->fix_offset = 0 ;

	switch ( m_flag & MTN_FLAG_TYPE_MASK ){
	  case MTN_FLAG_TYPE_01:
		mt3_header->flag |= MT3_FLAG_TYPE_01 ;
		break ;
	}
	if ( m_flag & MTN_FLAG_TURN ) mt3_header->flag |= MT3_FLAG_TURN_FLAG ;


	data_bit_pos = 0 ;

	/* 移動量データの生成 */
	mt3_header->move_offset = DataPositionFix( &data_bit_pos );
	MakeMoveData( motion->move, length, datas, &data_bit_pos );

	/* 各関節回転データの生成 */
	for ( i = 0 ; i < n_joints ; i++ ){
		key = &motion->jinfo[ ( length + 1 ) * i ] ;
		rots = &motion->rots[ ( length + 1 ) * i ] ;
		mt3_header->rots_offset[ i ] = DataPositionFix( &data_bit_pos );
		MakeRotsData( rots, key, length, m_flag, datas, &data_bit_pos );
	}

	/* 移動軌跡データの生成 */
	mt3_header->root_offset = DataPositionFix( &data_bit_pos );
	MakeRootData( motion->move, motion->minfo, length, datas, &data_bit_pos );

	/* 移動軌跡データの生成 */
	mt3_header->fix_offset = DataPositionFix( &data_bit_pos );
	MakeFixData( motion->move, motion->jinfo, n_joints, length, datas, &data_bit_pos );
	if ( mt3_header->fix_offset == DataPositionFix( &data_bit_pos ) ){
		mt3_header->flag &= ~MT3_FLAG_FIX_PART ;
	}

	data_size = DataPositionFix( &data_bit_pos );

	mt3_header->archives_size = data_size ;
	/* ファイルへの書き出し */
	if ( ( fp = fopen( filename, "wb" ) ) != NULL ){
		/* ヘッダ部分 */
		fwrite( mt3_header, 1, header_size, fp );
		/* 圧縮データ部分 */
		fwrite( datas, sizeof(short), data_size, fp );
		fclose( fp );
	}

	free( mt3_header );
	free( datas );

}

/* ------------------------------------------------------------ */
/* データバッファ操作関連 */
/* 有効ビット数を返す */
static int GetEffectiveBit( int data )
{
	unsigned int mask,bit,sign;
	bit = 16;
	mask = 0x0008000;
	sign = data & mask ;
	do {
		sign >>= 1;
		mask >>= 1;
		if ( sign != (unsigned int)(data & mask) ) return ( bit );
	} while ( --bit );
	return ( bit );
}
/* 指定したビット長の数字を入れる(MAX bit=16) */
static void PushData( int bit, int n, unsigned short *datas, int *data_bit_pos )
{
	int pos,bit_offset,short_offset;
	unsigned int work,data,mask;

	bit_offset = (*data_bit_pos) & 0x0f;
	short_offset = (*data_bit_pos) >> 4;
	mask = ((unsigned int)1 << bit) - 1;
	work = datas[ short_offset ] + ( (unsigned int)datas[ short_offset+1 ] << 16 );
	data = ((unsigned int)( n & mask)) << bit_offset;
	work |= data;
	datas[ short_offset   ] = work & 0xffff;
	datas[ short_offset+1 ] = (work >> 16) & 0xffff;
	*data_bit_pos += bit;
}
/* データ記憶位置をショート単位に揃え、そのオフセットを返す */
static int DataPositionFix( int *data_bit_pos )
{
	int tmp;
	tmp = *data_bit_pos + 15;
	*data_bit_pos = tmp & 0xfffffff0;
	return ( tmp / 16 ) ;
}

/* 念のため大きさをチェックしてバッファへ書き込む */
static int WriteMotionVectorData( int data, unsigned short *datas, int *data_bit_pos )
{
	if ( data < -2047 ) data = -2047 ;
	if ( data > 2047 ) data = 2047 ;
	PushData( 12, data, datas, data_bit_pos );
}

/* 移動データをデータバッファに登録する */
static void MakeMoveData( FVECTOR *center, int length, unsigned short *datas, int *data_bit_pos )
{
	SVECTOR *vec;
	int i,pos,ave_y;
	int max_bit_x,max_bit_y,max_bit_z,tmp;

	vec = malloc( sizeof(SVECTOR) * length );
	/* Ｙの平均を求める */
	ave_y = 0 ;
	for ( i = 1 ; i <= length ; i++ ){
		ave_y += center[i].vy ;
	}
	ave_y /= length ;

	/* Ｙを平均からの差分にし、ＸＺ軸の座標を移動量に変換 */
	for ( i = 1 ; i <= length ; i++ ){
		vec[i-1].vy = (int)center[i].vy - ave_y ;
		vec[i-1].vx = (int)center[i].vx - (int)center[i-1].vx ;
		vec[i-1].vz = (int)center[i].vz - (int)center[i-1].vz ;
	}

	/* ＸＹＺ各々の必要ビット数を求める */
	max_bit_x = max_bit_y = max_bit_z = 0;
	for ( i = 0 ; i < length ; i++ ){
		if ( max_bit_x < (tmp = GetEffectiveBit( vec[i].vx ))) max_bit_x = tmp;
		if ( max_bit_y < (tmp = GetEffectiveBit( vec[i].vy ))) max_bit_y = tmp;
		if ( max_bit_z < (tmp = GetEffectiveBit( vec[i].vz ))) max_bit_z = tmp;
	}

	{/* データ生成部分 */
		/* 初期化データ書き込み */
		PushData( 16, ave_y, datas, data_bit_pos );/* Ｙの平均 */
		PushData( 4, max_bit_x, datas, data_bit_pos );
		PushData( 4, max_bit_y, datas, data_bit_pos );
		PushData( 4, max_bit_z, datas, data_bit_pos );
		PushData( 4, 0, datas, data_bit_pos );/* パディング */
		/* データ本体書き込み */
		for ( i = 0 ; i < length ; i++ ){
			PushData( max_bit_x, vec[i].vx, datas, data_bit_pos );
			PushData( max_bit_y, vec[i].vy, datas, data_bit_pos );
			PushData( max_bit_z, vec[i].vz, datas, data_bit_pos );
		}
	}

	free(vec);
}

/* 関節回転データをデータバッファに登録する */
static void MakeRotsData( FVECTOR *rots, KEY_INFO *key, int length, int flag, unsigned short *datas, int *data_bit_pos )
{
	FVECTOR		*c_rots ;
	int			i, len, old_key_num ;

	old_key_num = 0 ;
	c_rots = malloc( sizeof(FVECTOR) * ( length + 1 ) );
	MakeCorrectQuat( c_rots, rots, key, length, flag );
	for ( i = 1 ; i <= length ; i++ ){
		if ( key[i].key ){
			len = i - old_key_num ;
			PushData( 8, len, datas, data_bit_pos );
			WriteMotionVectorData( rots[i].vx * 2047, datas, data_bit_pos );
			WriteMotionVectorData( rots[i].vy * 2047, datas, data_bit_pos );
			WriteMotionVectorData( rots[i].vz * 2047, datas, data_bit_pos );
			WriteMotionVectorData( rots[i].vw * 2047, datas, data_bit_pos );
			WriteMotionVectorData( c_rots[i].vx * 2047, datas, data_bit_pos );
			WriteMotionVectorData( c_rots[i].vy * 2047, datas, data_bit_pos );
			WriteMotionVectorData( c_rots[i].vz * 2047, datas, data_bit_pos );
			WriteMotionVectorData( c_rots[i].vw * 2047, datas, data_bit_pos );
			old_key_num = i ;
		}
	}
	free( c_rots );
}

/* 移動軌跡データをバッファに登録する */
static void MakeRootData( FVECTOR *move, KEY_INFO *key, int length, unsigned short *datas, int *data_bit_pos )
{
	SVECTOR *vec;
	int			i, len, old_key ;

	vec = malloc( sizeof(SVECTOR) * length );

	old_key = 0 ;
	for ( i = 1 ; i <= length ; i++ ){
		if ( key[i].key ){
			len = i - old_key ;
			PushData( 16, len, datas, data_bit_pos );
			PushData( 16, (int)move[i].vx - (int)move[old_key].vx, datas, data_bit_pos );
			PushData( 16, (int)move[i].vy                        , datas, data_bit_pos );
			PushData( 16, (int)move[i].vz - (int)move[old_key].vz, datas, data_bit_pos );
			old_key = i ;
		}
	}

	free(vec);
}

/* 関節固定データをバッファに登録する */
static void MakeFixData( FVECTOR *move, KEY_INFO *key, int n_joints, int length, unsigned short *datas, int *data_bit_pos )
{
	SVECTOR *vec;
	int			i, j, len, old_key, old_joint ;

	old_key = 1 ;
	old_joint = -1 ;
	for ( i = 1 ; i <= length ; i++ ){
		for ( j = 0 ; j < n_joints ; j++ ){
			if ( key[ j * ( length + 1 ) + i ].fix ) break ;
		}
		if ( j == n_joints ) j = -1 ;
		if ( i == 1 ) old_joint = j ;
		if ( old_joint != j ){
			PushData( 16, i - old_key, datas, data_bit_pos );
			PushData( 16, old_joint, datas, data_bit_pos );
			old_key = i ;
			old_joint = j ;
		}
	}
	PushData( 16, i - old_key, datas, data_bit_pos );
	PushData( 16, old_joint, datas, data_bit_pos );

}

/* 文字列コードの取得 */
static unsigned short MGS_GetStrCode( char *str )
{
	unsigned short	code = 0 ;
	while ( *str != 0 && *str != '.' ){
		code = ( (code<<5) | ( code>>11) ) & 0xffff ;
		if ( *str == ':' ) code = - *str ;
		if ( *str == '\\' ) code = - *str ;
		if ( *str == '/' ) code = - *str ;
		code = ( code ) + (unsigned char)*str++ ;
	}
	return ( code & 0xffff );
}
