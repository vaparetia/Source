#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gte.h"
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
	MTN_MOTION	*motion ;
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

	return ( motion );
}

/* ------------------------------------------------------------ */
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
