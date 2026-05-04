/*
	main.c
	モーションファイルユーティリティプログラム

	2000/09/29 K.Takabe
	$Id: main.c,v 1.1 2001/07/14 11:04:57 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "gte.h"
#include "quat.h"
#include "motion.h"

MTN_MOTION *MTN_SamplingRateConvert( MTN_MOTION *org_motion, int new_tick, int flag );
/* ---------------------------------------------------------------- */
extern int ProgMain( char *org_file, char *lip_file, char *out_file );
int		program_mode = 0 ;
int		error_angle = -1 ;
int		error_length = -1 ;
int		trans_flag = -1 ;
int		new_sampling_rate ;
char	*out_path ;

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[1024] ;
	int		mode = -1, error_flag = 0 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
#if 0
			if ( argv[0][1] == 'e' ){
				if ( argc <= 1 ){
					error_flag = 1 ;
					break ;
				}
				program_mode = 1 ;
				error_angle = atoi( argv[1] );
				argc-- ;
				argv++ ;
			}
#endif
		} else {
			switch ( mode ){
			default:/* 入力ファイル取得 */
				if ( get_input_files < 1024 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_input_files < 3 || get_input_files >= 1024 || error_flag ){
		fprintf( stderr, "mtn lip blend   Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: mtn_lip_blend <org_face_mtn> <blend_lip_mtn> <out_mtn> \n" );
		//fprintf( stderr, "(Options)  -e <n> ... error angle set  \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	printf("n_files:%d\n", get_input_files );
	return ProgMain( input_files[0], input_files[1], input_files[2] );
}

/* ---------------------------------------------------------------- */
int ProgMain( char *org_file, char *lip_file, char *out_file )
{
	MTN_MOTION	*org_motion, *lip_motion, *out_motion ;
	int		ret = 0 ;
	int		max_motion_length, n_joints ;
	int		i, j ;
	FVECTOR	org_move, org_rots[128], org_trans[128] ;
	FVECTOR	lip_move, lip_rots[128], lip_trans[128] ;
	FVECTOR	out_move, out_rots[128], out_trans[128] ;
	static blend[] = {
		0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0.1,0.1,1,1,1,0,0,0,0,0,0,0,0,
	};

	//fprintf( stdout, "convert...(%s)\n", *in_files );
	org_motion = MTN_OpenMTNFile( org_file );
	if ( org_motion == NULL ) return ( -1 );
	lip_motion = MTN_OpenMTNFile( lip_file );
	if ( lip_motion == NULL ) return ( -1 );

	max_motion_length = ( org_motion->header.motion_length > lip_motion->header.motion_length ) ? 
		org_motion->header.motion_length : lip_motion->header.motion_length ;
	n_joints = org_motion->header.motion_joints ;

	out_motion = MTN_AllocMotion( n_joints, max_motion_length );
	//out_motion = MTN_DuplicateMotion( org_motion );
	if ( out_motion == NULL ) return ( -1 );

	for ( i = -1 ; i < max_motion_length ; i++ ){
		MTN_GetMotionData( org_motion, i, &org_move, org_trans, org_rots );
		MTN_GetMotionData( lip_motion, i, &lip_move, lip_trans, lip_rots );

		for ( j = 0 ; j < n_joints ; j++ ){
			GTE_InterpolateVector( &out_trans[j], &org_trans[j], &lip_trans[j], blend[j] );
			MT_QuatSlerp( &out_rots[j], &org_rots[j], &lip_rots[j], blend[j] );
		}

		MTN_SetMotionData( out_motion, i, &org_move, out_trans, out_rots );
	}

	MTN_SaveMTNFile( out_file, out_motion );

	MTN_FreeMotion( out_motion );
	MTN_FreeMotion( lip_motion );
	MTN_FreeMotion( org_motion );

	return ( 0 );
}

