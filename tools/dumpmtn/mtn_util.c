#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "quat.h"
#include "motion.h"


/* ------------------------------------------------------------ */
/* 複製したものを作成 */
MTN_MOTION* MTN_DuplicateMotion( MTN_MOTION *org_motion )
{
	MTN_MOTION	*motion ;
	int			size ;
	void		*ptr ;

	motion = malloc( sizeof(MTN_MOTION) );
	memset( motion, 0, sizeof(MTN_MOTION) );

	motion->header = org_motion->header ;

	if ( ( size = motion->header.move_size ) != 0 ){
		motion->move = ptr = malloc( size );
		memcpy( motion->move, org_motion->move, size );
	}
	if ( ( size = motion->header.rots_size ) != 0 ){
		motion->rots = ptr = malloc( size );
		memcpy( motion->rots, org_motion->rots, size );
	}
	if ( ( size = motion->header.trans_size ) != 0 ){
		motion->trans = ptr = malloc( size );
		memcpy( motion->trans, org_motion->trans, size );
	}
	if ( ( size = motion->header.minfo_size ) != 0 ){
		motion->minfo = ptr = malloc( size );
		memcpy( motion->minfo, org_motion->minfo, size );
	}
	if ( ( size = motion->header.jinfo_size ) != 0 ){
		motion->jinfo = ptr = malloc( size );
		memcpy( motion->jinfo, org_motion->jinfo, size );
	}
	if ( ( size = motion->header.sound_size ) != 0 ){
		motion->sound = ptr = malloc( size );
		memcpy( motion->sound, org_motion->sound, size );
	}
	if ( ( size = motion->header.anime_size ) != 0 ){
		motion->anime = ptr = malloc( size );
		memcpy( motion->anime, org_motion->anime, size );
	}

	return ( motion );
}

/* 内容のコピー */
void MTN_CopyMotion( MTN_MOTION *dest, MTN_MOTION *org_motion )
{
	int			size ;

	dest->header = org_motion->header ;

	if ( ( size = org_motion->header.move_size ) != 0 ){
		memcpy( dest->move, org_motion->move, size );
	}
	if ( ( size = org_motion->header.rots_size ) != 0 ){
		memcpy( dest->rots, org_motion->rots, size );
	}
	if ( ( size = org_motion->header.trans_size ) != 0 ){
		memcpy( dest->trans, org_motion->trans, size );
	}
	if ( ( size = org_motion->header.minfo_size ) != 0 ){
		memcpy( dest->minfo, org_motion->minfo, size );
	}
	if ( ( size = org_motion->header.jinfo_size ) != 0 ){
		memcpy( dest->jinfo, org_motion->jinfo, size );
	}
	if ( ( size = org_motion->header.sound_size ) != 0 ){
		memcpy( dest->sound, org_motion->sound, size );
	}
	if ( ( size = org_motion->header.anime_size ) != 0 ){
		memcpy( dest->anime, org_motion->anime, size );
	}
}

/* ------------------------------------------------------------ */
/* 指定関節数で指定したフレーム数分の空モーションデータを生成 */
MTN_MOTION* MTN_AllocMotion( int n_joints, int frame )
{
	MTN_MOTION	*motion ;
	KEY_INFO	*key ;
	FVECTOR		vec, zero_quat ;
	int		i, j, size ;

	motion = malloc( sizeof(MTN_MOTION) );
	memset( motion, 0, sizeof(MTN_MOTION) );

	/* 各種設定コピー */
	motion->header.flags = 0 ;
	motion->header.motion_length = frame ;
	motion->header.motion_joints = n_joints ;
	motion->header.motion_tick = 5 ;		/* モーションタイムベース設定 5/300 = 1/60sec */
	motion->header.error_angle = 16 ;		/* モーション圧縮許容角度 */

	/* 移動量の初期化 */
	size = sizeof(FVECTOR) * ( frame + 1 ) ;
	motion->header.move_size = size ;
	motion->move = malloc( size );
	for ( i = 0 ; i <= motion->header.motion_length ; i++ ){
		motion->move[ i ].vx = 0.0f ;
		motion->move[ i ].vy = 0.0f ;
		motion->move[ i ].vz = 0.0f ;
		motion->move[ i ].vw = 1.0f ;
	}

	/* 関節角度の初期化 */
	size = sizeof(FVECTOR) * ( frame + 1 ) * n_joints ;
	motion->header.rots_size = size ;
	motion->rots = malloc( size );
	vec.vx = 0.0f ;
	vec.vy = 0.0f ;
	vec.vz = 0.0f ;
	//MT_EulerToQuatXYZ( &zero_quat, &vec );
	//MT_QuatNormalize( &zero_quat, &zero_quat );
	for ( i = 0 ; i < n_joints ; i++ ){
		FVECTOR		*quat ;
		quat = &motion->rots[ i * ( motion->header.motion_length + 1 ) ] ;
		for ( j = 0 ; j <= motion->header.motion_length ; j++ ){
			*quat = zero_quat ;
			quat++ ;
		}
	}

	/* 関節移動量の初期化 */
	size = sizeof(FVECTOR) * ( frame + 1 ) * n_joints ;
	motion->header.trans_size = size ;
	motion->trans = malloc( size );
	for ( i = 0 ; i < n_joints ; i++ ){
		FVECTOR		*trans ;
		trans = &motion->trans[ i * ( motion->header.motion_length + 1 ) ] ;
		for ( j = 0 ; j <= motion->header.motion_length ; j++ ){
			trans->vx = 0.0f ;
			trans->vy = 0.0f ;
			trans->vz = 0.0f ;
			trans->vw = 1.0f ;
			trans++ ;
		}
	}

	/* キーフレーム情報の領域確保＆初期化 */
	size = sizeof(KEY_INFO) * ( motion->header.motion_length + 1 ) ;
	size = ( size + 15 ) & 0xfffff0 ;
	motion->header.minfo_size = size ;
	motion->minfo = malloc( size );
	memset( motion->minfo, 0, size );
	/* 圧縮に最低限必要なキーを設定する */
	key = motion->minfo ;
	key[ 0 + 1 ].key = 1 ;
	key[ motion->header.motion_length ].key = 1 ;
	/* キーフレーム情報の領域確保＆初期化 */
	size = sizeof(KEY_INFO) * ( motion->header.motion_length + 1 ) ;
	size = size * motion->header.motion_joints ;
	size = ( size + 15 ) & 0xfffff0 ;
	motion->header.jinfo_size = size ;
	motion->jinfo = malloc( size );
	memset( motion->jinfo, 0, size );
	/* 圧縮に最低限必要なキーを設定する */
	for ( i = 0 ; i < motion->header.motion_joints ; i++ ){
		key = &motion->jinfo[ i * ( motion->header.motion_length + 1 ) ] ;
		key[ 0 + 1 ].key = 1 ;
		key[ motion->header.motion_length ].key = 1 ;
	}

	return ( motion ) ;
}
/* ------------------------------------------------------------ */
/* モーションデータメモリを開放 */
void MTN_FreeMotion( MTN_MOTION *motion )
{
	if ( motion->move != NULL ) free( motion->move );
	if ( motion->rots != NULL ) free( motion->rots );
	if ( motion->trans != NULL ) free( motion->trans );
	if ( motion->minfo != NULL ) free( motion->minfo );
	if ( motion->jinfo != NULL ) free( motion->jinfo );
	if ( motion->sound != NULL ) free( motion->sound );
	if ( motion->anime != NULL ) free( motion->anime );
	free( motion );
}

/* ------------------------------------------------------------ */
/* モーションファイルからモーションデータを取得 */
MTN_MOTION* MTN_OpenMTNFile( char *filename )
{
	FILE		*fp ;
	MTN_MOTION	*motion ;
	KEY_INFO	*key ;
	int			size ;
	void		*ptr ;
	int			i ;

	if ( ( fp = fopen( filename, "rb" ) ) == NULL ){
		return NULL ;
	}

	motion = malloc( sizeof(MTN_MOTION) );
	memset( motion, 0, sizeof(MTN_MOTION) );

	fread( &motion->header, 1, sizeof(MTN_FILE_HEADER), fp );

	if ( ( size = motion->header.move_size ) != 0 ){
		motion->move = ptr = malloc( size );
		fread( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.rots_size ) != 0 ){
		motion->rots = ptr = malloc( size );
		fread( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.trans_size ) != 0 ){
		motion->trans = ptr = malloc( size );
		fread( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.minfo_size ) != 0 ){
		motion->minfo = ptr = malloc( size );
		fread( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.jinfo_size ) != 0 ){
		motion->jinfo = ptr = malloc( size );
		fread( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.sound_size ) != 0 ){
		motion->sound = ptr = malloc( size );
		fread( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.anime_size ) != 0 ){
		motion->anime = ptr = malloc( size );
		fread( ptr, 1, size, fp );
	}

	/* 圧縮に最低限必要なキーを設定する */
	key = motion->minfo ;
	key[ 0 + 1 ].key = 1 ;
	key[ motion->header.motion_length ].key = 1 ;
	/* 圧縮に最低限必要なキーを設定する */
	for ( i = 0 ; i < motion->header.motion_joints ; i++ ){
		key = &motion->jinfo[ i * ( motion->header.motion_length + 1 ) ] ;
		key[ 0 + 1 ].key = 1 ;
		key[ motion->header.motion_length ].key = 1 ;
	}

	fclose( fp );
	return ( motion );
}
/* モーションデータをファイルへ書き込む */
void MTN_SaveMTNFile( char *filename, MTN_MOTION *motion )
{
	FILE		*fp ;
	int			size ;
	void		*ptr ;

	if ( ( fp = fopen( filename, "wb" ) ) == NULL ){
		return NULL ;
	}

	fwrite( &motion->header, 1, sizeof(MTN_FILE_HEADER), fp );

	if ( ( size = motion->header.move_size ) != 0 ){
		ptr = motion->move ;
		fwrite( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.rots_size ) != 0 ){
		ptr = motion->rots ;
		fwrite( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.trans_size ) != 0 ){
		ptr = motion->trans ;
		fwrite( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.minfo_size ) != 0 ){
		ptr = motion->minfo ;
		fwrite( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.jinfo_size ) != 0 ){
		ptr = motion->jinfo ;
		fwrite( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.sound_size ) != 0 ){
		ptr = motion->sound ;
		fwrite( ptr, 1, size, fp );
	}
	if ( ( size = motion->header.anime_size ) != 0 ){
		ptr = motion->anime ;
		fwrite( ptr, 1, size, fp );
	}

	fclose( fp );
}
/* ------------------------------------------------------------ */
/* 指定フレームのモーションデータを取得 */
void MTN_GetMotionData( MTN_MOTION *motion, int frame, FVECTOR *mov, FVECTOR *trans, FVECTOR *rots )
{
	int		i ;

	if ( motion == NULL ) return ;
	if ( frame >= motion->header.motion_length ) return ;
	if ( frame < -1 ) return ;

	/* 移動量取得 */
	if ( mov != NULL ){
		*mov = motion->move[ frame + 1 ] ;
	}
	/* 関節移動量取得 */
	if ( trans != NULL ){
		for ( i = 0 ; i < motion->header.motion_joints ; i++ ){
			trans[i] = motion->trans[ ( motion->header.motion_length + 1 ) * i + frame + 1 ] ;
		}
	}
	/* 関節回転量取得 */
	if ( rots != NULL ){
		for ( i = 0 ; i < motion->header.motion_joints ; i++ ){
			rots[i] = motion->rots[ ( motion->header.motion_length + 1 ) * i + frame + 1 ] ;
		}
	}
}
/* 指定フレームのモーションデータを設定 */
void MTN_SetMotionData( MTN_MOTION *motion, int frame, FVECTOR *mov, FVECTOR *trans, FVECTOR *rots )
{
	int		i ;

	if ( motion == NULL ) return ;
	if ( frame >= motion->header.motion_length ) frame = motion->header.motion_length - 1 ;
	if ( frame < -1 ) frame = -1 ;

	/* 移動量設定 */
	if ( mov != NULL ){
		motion->move[ frame + 1 ] = *mov ;
	}
	/* 関節移動量設定 */
	if ( trans != NULL ){
		for ( i = 0 ; i < motion->header.motion_joints ; i++ ){
			motion->trans[ ( motion->header.motion_length + 1 ) * i + frame + 1 ] = trans[i] ;
		}
	}
	/* 関節回転量設定 */
	if ( rots != NULL ){
		for ( i = 0 ; i < motion->header.motion_joints ; i++ ){
			motion->rots[ ( motion->header.motion_length + 1 ) * i + frame + 1 ] = rots[i] ;
		}
	}
}
/* ------------------------------------------------------------ */
