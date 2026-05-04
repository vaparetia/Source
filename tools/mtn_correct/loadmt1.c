#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gte.h"
#include "motion.h"


/* ------------------------------------------------------------ */

typedef struct _mgs_motion{
	int			active_flag ;
	int			max_units ;
	int			max_length ;
	SVECTOR		*center ;
	SVECTOR		*local_rots ;
} MGS_MOTION ;

/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */

/* 指定された数のshort型データをリトルエンディアンに揃える */
static void ChangeLittleEndianShort( unsigned short *src, int num )
{
	unsigned short tmp;
	unsigned char  *dst;

	dst = (unsigned char *) src;
	while ( num-- ){
		tmp = *src++;
		*dst++ = tmp & 0xff;
		*dst++ = (tmp >> 8) & 0xff;
	}
}
/* 指定された数のlong型データをリトルエンディアンに揃える */
static void ChangeLittleEndianLong( unsigned long *src, int num )
{
	unsigned long tmp;
	unsigned char  *dst;

	dst = (unsigned char *) src;
	while ( num-- ){
		tmp = *src++;
		*dst++ = tmp & 0xff;
		tmp >>= 8;
		*dst++ = tmp & 0xff;
		tmp >>= 8;
		*dst++ = tmp & 0xff;
		tmp >>= 8;
		*dst++ = tmp & 0xff;
	}
}

/* 指定された数のリトルエンディアンデータをshort型に揃える */
static void RecoverLittleEndianShort( unsigned short *dst, int num )
{
	unsigned short tmp;
	unsigned char  *src;

	src = (unsigned char *) dst;
	while ( num-- ){
		tmp = *src++;
		tmp |= (*src++) << 8;
		*dst++ = tmp;
	}
}
/* 指定された数のリトルエンディアンデータをlong型に揃える */
static void RecoverLittleEndianLong( unsigned long *dst, int num )
{
	unsigned long tmp;
	unsigned char  *src;

	src = (unsigned char *) dst;
	while ( num-- ){
		tmp = *src++;
		tmp |= (*src++) << 8;
		tmp |= (*src++) << 16;
		tmp |= (*src++) << 24;
		*dst++ = tmp;
	}
}

/* ---------------------------------------------------------------------- */
/* 指定された数のshort型データをビッグエンディアンに揃える */
static void ChangeBigEndianShort( unsigned short *src, int num )
{
	unsigned short tmp;
	unsigned char  *dst;

	dst = (unsigned char *) src;
	while ( num-- ){
		tmp = *src++;
		*dst++ = (tmp >> 8) & 0xff;
		*dst++ = tmp & 0xff;
	}
}
/* 指定された数のlong型データをビッグエンディアンに揃える */
static void ChangeBigEndianLong( unsigned long *src, int num )
{
	unsigned long tmp;
	unsigned char  *dst;

	dst = (unsigned char *) src;
	while ( num-- ){
		tmp = *src++;
		*dst++ = ( tmp >> 24 ) & 0xff;
		*dst++ = ( tmp >> 16 ) & 0xff;
		*dst++ = ( tmp >> 8 ) & 0xff;
		*dst++ = tmp & 0xff;
	}
}

/* 指定された数のビッグエンディアンデータをshort型に揃える */
static void RecoverBigEndianShort( unsigned short *dst, int num )
{
	unsigned short tmp;
	unsigned char  *src;

	src = (unsigned char *) dst;
	while ( num-- ){
		tmp = (*src++) << 8 ;
		tmp |= (*src++) ;
		*dst++ = tmp;
	}
}
/* 指定された数のビッグエンディアンデータをlong型に揃える */
static void RecoverBigEndianLong( unsigned long *dst, int num )
{
	unsigned long tmp;
	unsigned char  *src;

	src = (unsigned char *) dst;
	while ( num-- ){
		tmp = (*src++) << 24 ;
		tmp |= (*src++) << 16 ;
		tmp |= (*src++) << 8 ;
		tmp |= (*src++) ;
		*dst++ = tmp;
	}
}

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */


/* ＭＧＳ中間モーションデータの読み込み */
static int MGS_LoadMt1Data( MGS_MOTION *motion, char *name )
{
	FILE	*fp ;
	char	*data_buffer ;
	long	info[2] ;
	short	*data ;
	int		i, j, size ;

	if ( ( fp = fopen( name, "rb" ) ) == NULL ){
		return ( -1 );
	}
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	size -= sizeof( long ) * 2 ;
	data_buffer = malloc( size );
	fread( info, sizeof( long ), 2, fp );
	fread( data_buffer, 1, size, fp );
	fclose( fp );

	data = (short*) data_buffer  ;

	RecoverBigEndianLong( info, 2 );
	motion->max_units = info[0] ;
	motion->max_length = info[1] ;
	RecoverBigEndianShort( data, ( motion->max_length + 1 ) * ( motion->max_units + 1 ) * 4 );
	motion->center = (SVECTOR*)data ;
	motion->local_rots = &motion->center[ motion->max_length + 1 ] ;

	{/* 移動値の補正 */
		SVECTOR	start = motion->center[0];
		for ( i = 0 ; i <= motion->max_length ; i++ ){
			/* ＸＺ座標のスタート地点を原点にする */
			motion->center[i].vx -= start.vx ;
			motion->center[i].vz -= start.vz ;
		}
		
	}

	motion->active_flag = 1 ;

	return ( 0 );
}

/* ＭＧＳ中間モーションデータの開放 */
static void MGS_FreeMotion( MGS_MOTION *motion )
{
	motion->active_flag = 0 ;
	free( motion->center );
}

/* ---------------------------------------------------------------------- */
MTN_MOTION* MT_LoadMt1( char *filename )
{
	MGS_MOTION	mt1 ;
	MTN_MOTION	*motion ;
	KEY_INFO	*key ;
	int		i, j, size ;

	/* ＭＴ１ファイルの読み込み */
	MGS_LoadMt1Data( &mt1, filename );

	motion = malloc( sizeof(MTN_MOTION) );
	memset( motion, 0, sizeof(MTN_MOTION) );

	/* 各種設定コピー */
	motion->header.flags = 0 ;
	motion->header.motion_length = mt1.max_length ;
	motion->header.motion_joints = mt1.max_units ;
	motion->header.motion_tick = 5 ;
	motion->header.error_angle = 16 ;

	/* 移動量の読み込み */
	size = sizeof(FVECTOR) * ( mt1.max_length + 1 ) ;
	motion->header.move_size = size ;
	motion->move = malloc( size );
	for ( i = 0 ; i <= mt1.max_length ; i++ ){
		motion->move[ i ].vx = mt1.center[ i ].vx ;
		motion->move[ i ].vy = mt1.center[ i ].vy ;
		motion->move[ i ].vz = mt1.center[ i ].vz ;
		motion->move[ i ].vw = 1.0f ;
	}

	/* 関節角度の読み込み */
	size = sizeof(FVECTOR) * ( mt1.max_length + 1 ) * mt1.max_units ;
	motion->header.rots_size = size ;
	motion->rots = malloc( size );
	for ( i = 0 ; i < mt1.max_units ; i++ ){
		SVECTOR		*rots ;
		FVECTOR		*quat ;
		rots = &mt1.local_rots[ i * ( mt1.max_length + 1 ) ] ;
		quat = &motion->rots[ i * ( motion->header.motion_length + 1 ) ] ;
		for ( j = 0 ; j <= mt1.max_length ; j++ ){
			FVECTOR	vec ;
			vec.vx = M_PI * rots->vx / 2048.0 ;
			vec.vy = M_PI * rots->vy / 2048.0 ;
			vec.vz = M_PI * rots->vz / 2048.0 ;
			MT_EulerToQuatXYZ( quat, &vec );
			MT_QuatNormalize( quat, quat );
			rots++ ;
			quat++ ;
		}
	}

	/* 関節移動量の領域確保＆初期化 */
	motion->header.trans_size = 0 ;
	motion->trans = NULL ;	/* 使わない */

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

	/* ＭＴ１用メモリの開放 */
	MGS_FreeMotion( &mt1 );

	return ( motion ) ;
}

