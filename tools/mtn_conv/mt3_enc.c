#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gte.h"
#include "quat.h"
#include "motion.h"

//#include <windows.h>
//#include "win3d.h"


/* ------------------------------------------------------------ */
/* 次のキーフレームを検索する */
static int GetNextKeyFrame( KEY_INFO *key, int now, int length, int flag );
/* 前のキーフレームを検索する */
static int GetPrevKeyFrame( KEY_INFO *key, int now, int length, int flag );
/* 次のキーフレームを検索する */
static int GetNextTransKeyFrame( KEY_INFO *key, int now, int length, int flag );
/* 前のキーフレームを検索する */
static int GetPrevTransKeyFrame( KEY_INFO *key, int now, int length, int flag );
/* ２次補間パラメータ用クォータニオンを計算（実際にキーのあるところのみ計算） */
static void MakeCorrectQuat( FVECTOR *c_rots, FVECTOR *org_quat, KEY_INFO *key, int length, int flag );
/* ２次補間パラメータ用微分値を計算 */
static void MakeCorrectMove( FVECTOR *dmov, FVECTOR *org_mov, int length, int flag );
/* 現在のデータで一定以上の角度差が発生する場合に追加するキーフレーム番号を返す */
static int CheckErrorAngle( FVECTOR *rots, FVECTOR *c_rots, KEY_INFO *key, int length, int flag, float error_angle );
/* 現在のデータで一定以上の差が発生する場合に追加するキーフレーム番号を返す */
static int CheckErrorMove( FVECTOR *mov, FVECTOR *dmov, KEY_INFO *key, int length, int flag, float error_len );
/* 現在のデータで一定以上の差が発生する場合に追加するキーフレーム番号を返す */
static int CheckErrorTrans( FVECTOR *mov, FVECTOR *dmov, KEY_INFO *key, int length, int flag, float error_len );
/* 補間間隔が１２７フレーム以上ある場合に途中でキーを入れて全て１２７フレーム以内に収める */
static void CheckKeyLength( KEY_INFO *key, int length, int flag );
/* 補間間隔が１２７フレーム以上ある場合に途中でキーを入れて全て１２７フレーム以内に収める */
static void CheckTransKeyLength( KEY_INFO *key, int length, int flag );
/* キーにしたがって圧縮したデータを展開したモーションを生成 */
static void MakeNewQuat( FVECTOR *out, FVECTOR *rots, FVECTOR *c_rots, KEY_INFO *key, int length, int flag );
/* そのフレームの角度変化量を取得（微分） */
static float GetAngleDiff( FVECTOR *rots, int now, int length, int flag );
/* 次のキーフレームのフレーム数を求める */
static int GetNextKeyWidth( KEY_INFO *key, int now, int length, int flag );
/* 前のキーフレームのフレーム数を求める */
static int GetPrevKeyWidth( KEY_INFO *key, int now, int length, int flag );

static unsigned short FtoMF( float f );
static float MFtoF( unsigned short mf );

/* ------------------------------------------------------------ */
float HermiteInterpolate( float q0, float q1, float d0, float d1, float t );
float HermiteInterpolate2( FVECTOR q, float t );

/* ------------------------------------------------------------ */
static void SetupQuat( FVECTOR *quat, int length )
{
	int		i ;
		float	res ;
	FVECTOR	*q1, *q2 ;
	for ( i = 1 ; i <= length ; i++ ){
		q1 = &quat[i-1] ;
		q2 = &quat[i] ;
		res = q1->vx * q2->vx + q1->vy * q2->vy + q1->vz * q2->vz + q1->vw * q2->vw ;
		if ( res < 0.0f ){
			q2->vx = - q2->vx ;
			q2->vy = - q2->vy ;
			q2->vz = - q2->vz ;
			q2->vw = - q2->vw ;
		}
	}
}

/* ------------------------------------------------------------ */

void MTN_Mt3Encode( MTN_MOTION *out_motion, MTN_MOTION *in_motion, int flag, float error_angle )
{
	int		i, j, k, ret ;
	int		length_plus, length ;
	KEY_INFO	*key ;
	FVECTOR		*in_rots, *out_rots, *c_rots, *correct_rots ;

	length = in_motion->header.motion_length ;
	length_plus = in_motion->header.motion_length + 1 ;

	/* メモリ確保 */
	correct_rots = malloc( sizeof(FVECTOR) * length_plus );

	/* 圧縮に最低限必要なキーを設定する */
	key = in_motion->minfo ;
	key[ 0 + 1 ].key = 1 ;
	key[ length ].key = 1 ;
	key[ 0 + 1 ].fix = 1 ;
	key[ length ].fix = 1 ;
	for ( i = 0 ; i < in_motion->header.motion_joints ; i++ ){
		key = &in_motion->jinfo[ i * length_plus ] ;
		key[ 0 + 1 ].key = 1 ;
		key[ length ].key = 1 ;
	}
	/* コピーする */
	MTN_CopyMotion( out_motion, in_motion );

	{/* 移動量圧縮 */
		FVECTOR	*dmov, *mov, *out_mov ;
		int		pf, nf, next_key, width, size ;

		/* 移動量作成 */
		dmov = correct_rots ;
		mov = in_motion->move ;
		key = out_motion->minfo ;
		MakeCorrectMove( dmov, mov, length, in_motion->header.flags );

		/* 指定誤差以内に納まるようにキーを生成する */
		do {
			ret =  CheckErrorMove( mov, dmov, key, length, in_motion->header.flags, in_motion->header.error_length );
			if ( ret != -1 ){
				key[ ret ].key = 1 ;
				continue ;
			}
			break ;
		} while ( ret != -1 );
		CheckKeyLength( key, length, in_motion->header.flags );

		/* 補間計算 */
		size = 0 ;
		out_mov = out_motion->move ;
		for ( i = 0 ; i < length ; ){
			float		t ;
			size += 12 ;
			next_key = GetNextKeyFrame( key, i, length, in_motion->header.flags );
			width = next_key - i ;
			for ( j = 1 ; j <= width ; j++ ){
				t = (float)j / (float)width ;
				out_mov[i+j].vx = HermiteInterpolate( mov[i].vx, mov[next_key].vx,
													 dmov[i].vx * width, dmov[next_key].vx * width, t );
				out_mov[i+j].vy = HermiteInterpolate( mov[i].vy, mov[next_key].vy,
													 dmov[i].vy * width, dmov[next_key].vy * width, t );
				out_mov[i+j].vz = HermiteInterpolate( mov[i].vz, mov[next_key].vz,
													 dmov[i].vz * width, dmov[next_key].vz * width, t );
			}
			i = next_key ;
		}
		//WIN3D_DebugPrintf("move size %d bytes\n", size );
	}

	/* 指定された誤差で関節のキーを自動生成する */
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

	if ( in_motion->header.flags & MTN_FLAG_TRANS ){
		/* 関節の移動量を圧縮 */
		for ( k = 0 ; k < out_motion->header.motion_joints ; k++ ){
			FVECTOR		*in_trans, *out_trans, *d_trans ;
			int			next_key, width ;

			key = &out_motion->jinfo[ k * length_plus ] ;
			in_trans = &in_motion->trans[ k * length_plus ] ;
			out_trans = &out_motion->trans[ k * length_plus ] ;
			d_trans = correct_rots ;

			/* 補間用に変化量を計算 */
			MakeCorrectMove( d_trans, in_trans, length, in_motion->header.flags );

			/* 指定誤差以内に納まるようにキーを生成する */
			do {
				ret =  CheckErrorTrans( in_trans, d_trans, key, length,
									   in_motion->header.flags, in_motion->header.error_length );
				if ( ret != -1 ){
					key[ ret ].tkey = 1 ;
					continue ;
				}
				break ;
			} while ( ret != -1 );
			CheckTransKeyLength( key, length, in_motion->header.flags );

			/* 補間計算 */
			for ( i = 0 ; i < length ; ){
				float		t ;
				next_key = GetNextTransKeyFrame( key, i, length, in_motion->header.flags );
				width = next_key - i ;
				for ( j = 1 ; j <= width ; j++ ){
					t = (float)j / (float)width ;
					out_trans[i+j].vx = HermiteInterpolate( in_trans[i].vx, in_trans[next_key].vx,
														   d_trans[i].vx * width, d_trans[next_key].vx * width, t );
					out_trans[i+j].vy = HermiteInterpolate( in_trans[i].vy, in_trans[next_key].vy,
														   d_trans[i].vy * width, d_trans[next_key].vy * width, t );
					out_trans[i+j].vz = HermiteInterpolate( in_trans[i].vz, in_trans[next_key].vz,
														   d_trans[i].vz * width, d_trans[next_key].vz * width, t );
				}
				i = next_key ;
			}
		}
	}

	free( correct_rots );

}
/* ------------------------------------------------------------ */
/* 次のキーフレームを検索する */
static int GetNextKeyFrame( KEY_INFO *key, int now, int length, int flag )
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
static int GetPrevKeyFrame( KEY_INFO *key, int now, int length, int flag )
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
/* 次のキーフレームを検索する */
static int GetNextTransKeyFrame( KEY_INFO *key, int now, int length, int flag )
{
	int	i ;
	for ( i = now + 1 ; i <= length  ; i++ ){
		if ( key[ i ].tkey ) return ( i ) ;
	}
	if ( flag & MTN_FLAG_LOOP ){
		return ( 1 ) ;
	} else {
		return ( length );
	}
}
/* 前のキーフレームを検索する */
static int GetPrevTransKeyFrame( KEY_INFO *key, int now, int length, int flag )
{
	int	i ;
	for ( i = now - 1 ; i >= 1 ; i-- ){
		if ( key[ i ].tkey ) return ( i ) ;
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
	FVECTOR	*prev, *next ;
	for ( i = 1 ; i <= length ; ){
		prev = &org_quat[ prev_key = GetPrevKeyFrame( key, i, length, flag ) ];
		next = &org_quat[ next_key = GetNextKeyFrame( key, i, length, flag ) ];
		MT_QuatQCompA( &c_rots[i], prev, &org_quat[i], next );
		if ( i == length ) break ;
		if ( i > next_key ) break ;
		i = next_key ;
	}
}
/* ２次補間パラメータ用微分値を計算 */
static void MakeCorrectMove( FVECTOR *dmov, FVECTOR *org_mov, int length, int flag )
{
	int		i, prev_frame, next_frame ;
	float	d ;
	FVECTOR	*prev_mov, *next_mov, prev_tmp, next_tmp ;

	for ( i = 0 ; i <= length ; i++ ){
		prev_frame = i - 1 ;
		next_frame = i + 1 ;
		prev_mov = &org_mov[prev_frame] ;
		next_mov = &org_mov[next_frame] ;
		d = 0.5f ;
		if ( prev_frame < 0 ){
			if ( flag & MTN_FLAG_LOOP ){
				prev_frame += length ;
				prev_tmp.vx = org_mov[ prev_frame ].vx - ( org_mov[ length ].vx - org_mov[ 0 ].vx );
				prev_tmp.vy = org_mov[ prev_frame ].vy - ( org_mov[ length ].vy - org_mov[ 0 ].vy );
				prev_tmp.vz = org_mov[ prev_frame ].vz - ( org_mov[ length ].vz - org_mov[ 0 ].vz );
				prev_mov = &prev_tmp ;
			} else {
				prev_frame = 0 ;
				prev_mov = &org_mov[prev_frame] ;
				d = 1.0f ;
			}
		}
		if ( next_frame > length ){
			if ( flag & MTN_FLAG_LOOP ){
				next_frame -= length ;
				next_tmp.vx = org_mov[ next_frame ].vx + ( org_mov[ length ].vx - org_mov[ 0 ].vx );
				next_tmp.vy = org_mov[ next_frame ].vy + ( org_mov[ length ].vy - org_mov[ 0 ].vy );
				next_tmp.vz = org_mov[ next_frame ].vz + ( org_mov[ length ].vz - org_mov[ 0 ].vz );
				next_mov = &next_tmp ;
			} else {
				next_frame = length ;
				next_mov = &org_mov[next_frame] ;
				d = 1.0f ;
			}
		}
		dmov[i].vx = ( next_mov->vx - prev_mov->vx ) * d ;
		dmov[i].vy = ( next_mov->vy - prev_mov->vy ) * d ;
		dmov[i].vz = ( next_mov->vz - prev_mov->vz ) * d ;
		dmov[i].vw = 1.0f ;
	}
}
/* 現在のデータで一定以上の角度差が発生する場合に追加するキーフレーム番号を返す */
static int CheckErrorAngle( FVECTOR *rots, FVECTOR *c_rots, KEY_INFO *key, int length, int flag, float error_angle )
{
#if 1
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
#else
	int prev_key, next_key, i, j, width ;
	FVECTOR	q1, q2 ;
	float	t, c, base_cos ;
	float	max_diff_angle = 1.0, angle ;
	int		max_diff_index = -1 ;
	float	d0, d1, base, tt ;
	int		p_width, n_width ;

	base_cos = cos( error_angle );
	for ( i = 0 ; i < length ; ){
		next_key = GetNextKeyFrame( key, i, length, flag );
		width = next_key - i ;
		/* 前後のキー位置を取得 */
		p_width = GetPrevKeyWidth( key, i, length, flag );
		n_width = GetNextKeyWidth( key, next_key, length, flag );
		/* １フレーム当たりの平均ｔ変化量を求める */
		if ( p_width != -1 ){
			d0 = 0.5f / (float)p_width + 0.5f / (float)width ;
		} else {
			d0 = 1.0f / (float)width ;
		}
		if ( n_width != -1 ){
			d1 = 0.5f / (float)n_width + 0.5f / (float)width ;
		} else {
			d1 = 1.0f / (float)width ;
		}
		///* 単位時間当たりの変化量に変換 */
		//d0 = 1.0f / d0 / width ;
		//d1 = 1.0f / d1 / width ;
		///* 正規化 */
		d0 = d0 * width ;
		d1 = d1 * width ;
		for ( j = 1 ; j <= width ; j++ ){
			t = (float)j / (float)width ;
			tt =  HermiteInterpolate( 0.0f, 1.0f, d0, d1, t );
			MT_QuatSquad( &q1, &rots[i], &c_rots[i], &c_rots[next_key], &rots[next_key], tt );
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
#endif
	return ( max_diff_index );
}

/* 現在のデータで一定以上の差が発生する場合に追加するキーフレーム番号を返す */
static int CheckErrorMove( FVECTOR *mov, FVECTOR *dmov, KEY_INFO *key, int length, int flag, float error_len )
{
	int prev_key, next_key, i, j, width ;
	FVECTOR	pos ;
	float	t, base_len ;
	float	max_diff_len = 0.0, lenlen ;
	int		max_diff_index = -1 ;

	base_len = error_len * error_len ;
	for ( i = 0 ; i < length ; ){
		next_key = GetNextKeyFrame( key, i, length, flag );
		width = next_key - i ;
		for ( j = 1 ; j <= width ; j++ ){
			t = (float)j / (float)width ;
			pos.vx = HermiteInterpolate( mov[i].vx, mov[next_key].vx,
										dmov[i].vx * width, dmov[next_key].vx * width, t );
			pos.vy = HermiteInterpolate( mov[i].vy, mov[next_key].vy,
										dmov[i].vy * width, dmov[next_key].vy * width, t );
			pos.vz = HermiteInterpolate( mov[i].vz, mov[next_key].vz,
										dmov[i].vz * width, dmov[next_key].vz * width, t );
			pos.vx -= mov[i+j].vx ;
			pos.vy -= mov[i+j].vy ;
			pos.vz -= mov[i+j].vz ;
			lenlen = pos.vx * pos.vx + pos.vy * pos.vy + pos.vz * pos.vz ;
			/* ここで角度差をチェック */
			if ( lenlen >= base_len ){
				if ( lenlen > max_diff_len ){
					max_diff_len = lenlen ;
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

/* 現在のデータで一定以上の差が発生する場合に追加するキーフレーム番号を返す */
static int CheckErrorTrans( FVECTOR *mov, FVECTOR *dmov, KEY_INFO *key, int length, int flag, float error_len )
{
	int prev_key, next_key, i, j, width ;
	FVECTOR	pos ;
	float	t, base_len ;
	float	max_diff_len = 0.0, lenlen ;
	int		max_diff_index = -1 ;

	base_len = error_len * error_len ;
	for ( i = 0 ; i < length ; ){
		next_key = GetNextTransKeyFrame( key, i, length, flag );
		width = next_key - i ;
		for ( j = 1 ; j <= width ; j++ ){
			t = (float)j / (float)width ;
			pos.vx = HermiteInterpolate( mov[i].vx, mov[next_key].vx,
										dmov[i].vx * width, dmov[next_key].vx * width, t );
			pos.vy = HermiteInterpolate( mov[i].vy, mov[next_key].vy,
										dmov[i].vy * width, dmov[next_key].vy * width, t );
			pos.vz = HermiteInterpolate( mov[i].vz, mov[next_key].vz,
										dmov[i].vz * width, dmov[next_key].vz * width, t );
			pos.vx -= mov[i+j].vx ;
			pos.vy -= mov[i+j].vy ;
			pos.vz -= mov[i+j].vz ;
			lenlen = pos.vx * pos.vx + pos.vy * pos.vy + pos.vz * pos.vz ;
			/* ここで角度差をチェック */
			if ( lenlen >= base_len ){
				if ( lenlen > max_diff_len ){
					max_diff_len = lenlen ;
					max_diff_index = i + j ;
				}
			}
		}
		i = next_key ;
	}
	return ( max_diff_index );
}

/* 補間間隔が１２７フレーム以上ある場合に途中でキーを入れて全て１２７フレーム以内に収める */
static void CheckTransKeyLength( KEY_INFO *key, int length, int flag )
{
	int		next_key, i, width ;

	for ( i = 1 ; i < length ; ){
		next_key = GetNextTransKeyFrame( key, i, length, flag );
		width = next_key - i ;
		while ( width > 127 ){
			key[ i + 127 ].tkey = 1 ;
			i += 127 ;
			width -= 127 ;
		}
		i = next_key ;
	}
}


/* キーにしたがって圧縮したデータを展開したモーションを生成 */
static void MakeNewQuat( FVECTOR *out, FVECTOR *rots, FVECTOR *c_rots, KEY_INFO *key, int length, int flag )
{
#if 1
	int prev_key, next_key, i, j, width ;
	FVECTOR	q1, q2 ;
	float	t, c ;

	for ( i = 0 ; i < length ; ){
		next_key = GetNextKeyFrame( key, i, length, flag );
		width = next_key - i ;
#if 0
		WIN3D_DebugPrintf(" rot: %f %f %f %f , %f %f %f %f\n",
						  c_rots[i].vx, c_rots[i].vy, c_rots[i].vz, c_rots[i].vw,
						  c_rots[next_key].vx, c_rots[next_key].vy, c_rots[next_key].vz, c_rots[next_key].vw );
		WIN3D_DebugPrintf("drot: %f %f %f %f , %f %f %f %f\n",
						  rots[i].vx, rots[i].vy, rots[i].vz, rots[i].vw,
						  rots[next_key].vx, rots[next_key].vy, rots[next_key].vz, rots[next_key].vw );
#endif
		for ( j = 1 ; j <= width ; j++ ){
			t = (float)j / (float)width ;
			MT_QuatSquad( &out[i+j], &rots[i], &c_rots[i], &c_rots[next_key], &rots[next_key], t );
		}
		if ( i == length ) break ;
		i = next_key ;
	}
#else
	int next_key, i, j, width ;
	float	t, d0, d1, base, tt ;
	int		p_width, n_width ;

	for ( i = 0 ; i < length ; ){
		next_key = GetNextKeyFrame( key, i, length, flag );
		width = next_key - i ;
		/* 前後のキー位置を取得 */
		p_width = GetPrevKeyWidth( key, i, length, flag );
		n_width = GetNextKeyWidth( key, next_key, length, flag );
		/* １フレーム当たりの平均ｔ変化量を求める */
		if ( p_width != -1 ){
			d0 = 0.5f / (float)p_width + 0.5f / (float)width ;
		} else {
			d0 = 1.0f / (float)width ;
		}
		if ( n_width != -1 ){
			d1 = 0.5f / (float)n_width + 0.5f / (float)width ;
		} else {
			d1 = 1.0f / (float)width ;
		}
		///* 単位時間当たりの変化量に変換 */
		//d0 = 1.0f / d0 / width ;
		//d1 = 1.0f / d1 / width ;
		///* 正規化 */
		d0 = d0 * width ;
		d1 = d1 * width ;
		for ( j = 1 ; j <= width ; j++ ){
			t = (float)j / (float)width ;
			tt =  HermiteInerpolate( 0.0f, 1.0f, d0, d1, t );
			MT_QuatSquad( &out[i+j], &rots[i], &c_rots[i], &c_rots[next_key], &rots[next_key], tt );
		}
		if ( i == length ) break ;
		i = next_key ;
	}
#endif
	return ;
}

/* そのフレームの角度変化量を取得（微分） */
static float GetAngleDiff( FVECTOR *rots, int now, int length, int flag )
{
	FVECTOR	*p, *n ;
	float	samples, c ;
	/* 前後のフレームのクォータニオンを取得 */
	p = &rots[ now - 1 ];
	n = &rots[ now + 1 ];
	samples = 0.5f ;
	if ( now <= 1 ){
		if ( flag & MTN_FLAG_LOOP ){
			p = &rots[ length ] ;
		} else {
			p = &rots[ now ] ;
			samples = 1.0f ;
		}
	}
	if ( now >= length ){
		if ( flag & MTN_FLAG_LOOP ){
			n = &rots[ 1 ] ;
		} else {
			n = &rots[ now ] ;
			samples = 1.0f ;
		}
	}
	c = MT_QuatDot( p, n );
	if ( c < 0.0f ) c = -c ;
	c = acos( c );
	c *= samples ;

	return c ;
}


/* 次のキーフレームのフレーム数を求める */
static int GetNextKeyWidth( KEY_INFO *key, int now, int length, int flag )
{
	int	i ;
	if ( length == 1 ) return ( 1 );
	for ( i = now + 1 ; i <= length  ; i++ ){
		if ( key[ i ].key ) return ( i - now ) ;
	}
	if ( flag & MTN_FLAG_LOOP ){
		return ( GetNextKeyWidth( key, 1, length, flag ) ) ;
	} else {
		return ( -1 );
	}
}
/* 前のキーフレームのフレーム数を求める */
static int GetPrevKeyWidth( KEY_INFO *key, int now, int length, int flag )
{
	int	i ;
	if ( length == 1 ) return ( 1 );
	for ( i = now - 1 ; i >= 1 ; i-- ){
		if ( key[ i ].key ) return ( now - i ) ;
	}
	if ( flag & MTN_FLAG_LOOP ){
		return ( GetPrevKeyWidth( key, length, length, flag ) ) ;
	} else {
		return ( -1 );
	}
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
/* 移動データをデータバッファに登録する（新バージョン） */
static void MakeMoveData2( FVECTOR *center, KEY_INFO *key, int length, int flag, unsigned short *datas, int *data_bit_pos );
/* 関節回転データをデータバッファに登録する */
static void MakeRotsData( FVECTOR *rot, KEY_INFO *key, int length, int flag, unsigned short *datas, int *data_bit_pos );
/* 関節移動データをデータバッファに登録する */
static void MakeTransData( FVECTOR *mov, KEY_INFO *key, int length, int flag, unsigned short *datas, int *data_bit_pos );
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
	if ( m_flag & MTN_FLAG_TRANS ) header_size += sizeof( long ) * n_joints ;
	mt3_header = malloc( header_size );
	memset( mt3_header, 0, header_size );
	datas = malloc( 1024 * 1024 );
	memset( datas, 0, 1024 * 1024 );

	mt3_header->name_id = MGS_GetStrCode( filename ) ;
	//mt3_header->flag = MT3_FLAG_ROT_PART | MT3_FLAG_MOVE_PART | MT3_FLAG_ROOT_PART | MT3_FLAG_FIX_PART ;
	mt3_header->flag = MT3_FLAG_ROT_PART | MT3_FLAG_MOVE2_PART | MT3_FLAG_ROOT_PART | MT3_FLAG_FIX_PART ;
	if ( m_flag & MTN_FLAG_TRANS ) mt3_header->flag |= MT3_FLAG_TRANS_PART ;
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
#if 0
	MakeMoveData( motion->move, length, datas, &data_bit_pos );
#else
	MakeMoveData2( motion->move, motion->minfo, length, m_flag, datas, &data_bit_pos );
#endif

	/* 各関節回転データの生成 */
	for ( i = 0 ; i < n_joints ; i++ ){
		key = &motion->jinfo[ ( length + 1 ) * i ] ;
		rots = &motion->rots[ ( length + 1 ) * i ] ;
		mt3_header->rots_offset[ i ] = DataPositionFix( &data_bit_pos );
		MakeRotsData( rots, key, length, m_flag, datas, &data_bit_pos );
	}
	if ( m_flag & MTN_FLAG_TRANS ){/* 各関節移動量データの生成 */
		for ( i = 0 ; i < n_joints ; i++ ){
			FVECTOR		*trans ;
			key = &motion->jinfo[ ( length + 1 ) * i ] ;
			trans = &motion->trans[ ( length + 1 ) * i ] ;
			mt3_header->rots_offset[ i + n_joints ] = DataPositionFix( &data_bit_pos );
			MakeTransData( trans, key, length, m_flag, datas, &data_bit_pos );
		}
	}

	/* 移動軌跡データの生成 */
	mt3_header->root_offset = DataPositionFix( &data_bit_pos );
	MakeRootData( motion->move, motion->minfo, length, datas, &data_bit_pos );

	/* 関節固定データの生成 */
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
	int max_bit_x,max_bit_y,max_bit_z,tmp, max_y, min_y;

	vec = malloc( sizeof(SVECTOR) * length );
	/* Ｙの平均を求める */
	ave_y = 0 ;
	max_y = -9999999 ;
	min_y = 9999999 ;
	for ( i = 1 ; i <= length ; i++ ){
		ave_y += center[i].vy ;
		if ( center[i].vy > max_y ) max_y = center[i].vy ;
		if ( center[i].vy < min_y ) min_y = center[i].vy ;
	}
	ave_y /= length ;
	ave_y = ( max_y - min_y ) / 2 + min_y ;

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
	//WIN3D_DebugPrintf("mt3 move size %d bytes\n", ( max_bit_x + max_bit_y + max_bit_z ) * length / 8 );

	free(vec);
}

/* 移動データをデータバッファに登録する（新バージョン） */
static void MakeMoveData2( FVECTOR *mov, KEY_INFO *key, int length, int flag, unsigned short *datas, int *data_bit_pos )
{
	int		i, old_key_num, len ;
	FVECTOR	*dmov ;

	/* ワークメモリ確保 */
	dmov = malloc( sizeof(FVECTOR) * ( length + 1 ) );

	/* 曲線の傾きを計算 */
	MakeCorrectMove( dmov, mov, length, flag );

	/* 初期基準座標を書き出す（相対移動量が必要なため０フレーム目のデータが必要） */
	PushData( 16, FtoMF(mov[0].vx), datas, data_bit_pos );
	PushData( 16, FtoMF(mov[0].vy), datas, data_bit_pos );
	PushData( 16, FtoMF(mov[0].vz), datas, data_bit_pos );

	/* キー座標、傾きを書き出す */
	old_key_num = 0 ;
	for ( i = 1 ; i <= length ; i++ ){
		if ( key[i].key ){
			float		t ;
			len = i - old_key_num ;
			/* キー間のフレーム数を設定 */
			PushData( 8, len, datas, data_bit_pos );
			/* キーの座標を設定 */
			PushData( 16, FtoMF(mov[i].vx), datas, data_bit_pos );
			PushData( 16, FtoMF(mov[i].vy), datas, data_bit_pos );
			PushData( 16, FtoMF(mov[i].vz), datas, data_bit_pos );
			/* キーの傾きを設定 */
			PushData( 16, FtoMF(dmov[i].vx), datas, data_bit_pos );
			PushData( 16, FtoMF(dmov[i].vy), datas, data_bit_pos );
			PushData( 16, FtoMF(dmov[i].vz), datas, data_bit_pos );
			old_key_num = i ;
		}
	}

	free( dmov );

}

/* 関節移動データをデータバッファに登録する */
static void MakeTransData( FVECTOR *mov, KEY_INFO *key, int length, int flag, unsigned short *datas, int *data_bit_pos )
{
	int		i, old_key_num, len ;
	FVECTOR	*dmov ;

	/* ワークメモリ確保 */
	dmov = malloc( sizeof(FVECTOR) * ( length + 1 ) );

	/* 曲線の傾きを計算 */
	MakeCorrectMove( dmov, mov, length, flag );

	/* キー座標、傾きを書き出す */
	old_key_num = 0 ;
	for ( i = 1 ; i <= length ; i++ ){
		if ( key[i].tkey ){
			float		t ;
			len = i - old_key_num ;
			/* キー間のフレーム数を設定 */
			PushData( 8, len, datas, data_bit_pos );
			/* キーの座標を設定 */
			PushData( 16, FtoMF(mov[i].vx), datas, data_bit_pos );
			PushData( 16, FtoMF(mov[i].vy), datas, data_bit_pos );
			PushData( 16, FtoMF(mov[i].vz), datas, data_bit_pos );
			/* キーの傾きを設定 */
			PushData( 16, FtoMF(dmov[i].vx), datas, data_bit_pos );
			PushData( 16, FtoMF(dmov[i].vy), datas, data_bit_pos );
			PushData( 16, FtoMF(dmov[i].vz), datas, data_bit_pos );
			old_key_num = i ;
		}
	}

	free( dmov );

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
		if ( key[i].fix ){
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


/* ================================================================ */
/*

	１６ビット浮動小数点フォーマット

	seeeeeffffffffff
	||    |
	||    +- 10bit 仮数部
	||
	|+- 5bit 指数（バイアス値 N の下駄履き指数）
	|
	+- 1bit 符合

*/

/* 指数のバイアス値（2^0となる指数値） */
#define E_BIAS	(8)		/* メタルギアでは大きい値を重視する */

/* 単精度浮動小数点から１６ビット浮動小数点へ変換 */
static unsigned short FtoMF( float f )
{
	unsigned int	data, s_dat, e_dat, f_dat ;
	unsigned short	mf ;

	data = *(unsigned int*)&f ;
	/* 符合取りだし */
	s_dat = ( data & 0x80000000 ) >> 16 ;
	/* 指数取りだし */
	e_dat = ( data & 0x7f800000 ) ;
	if ( e_dat != 0 ){
		if ( e_dat <= ( 127 - E_BIAS ) << 23 ){
			//printf("convert error!!\n");
			return ( 0x0000 | s_dat );
		}
		if ( e_dat > ( 127 - E_BIAS + 31 ) << 23 ){
			//printf("convert error!!\n");
			return ( 0x7fff | s_dat );
		}
		e_dat -= ( 127 - E_BIAS ) << 23 ;
		e_dat >>= 13 ;
	}
	/* 仮数部取りだし */
	f_dat = ( data & 0x007fffff ) >> 13 ;

	/* ビット合成 */
	mf = s_dat | e_dat | f_dat ;

	return ( mf );
}

/* １６ビット浮動小数点から単精度浮動小数点へ変換 */
static float MFtoF( unsigned short mf )
{
	unsigned int	s_dat, e_dat, f_dat ;
	float			f ;

	/* 符合取りだし */
	s_dat = ( mf & 0x8000 ) << 16 ;
	/* 指数取りだし */
	e_dat = ( mf & 0x7c00 ) ;
	if ( e_dat != 0 ){
		e_dat += ( 127 - E_BIAS ) << 10 ;
		e_dat <<= 13 ;
	}
	/* 仮数部取りだし */
	f_dat = ( mf & 0x03ff ) << 13 ;
	/* ビット合成 */
	*(unsigned int*)&f = s_dat | e_dat | f_dat ;

	return ( f );
}


