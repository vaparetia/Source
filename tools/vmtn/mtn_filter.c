/* #include <windows.h> */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gte.h"
#include "quat.h"
#include "motion.h"

#define _mtn_filter_c_
#include "mtn_filter.h"

#define SMOOTH_TIME	(4)
void MotionSmooth( MTN_MOTION *motion )
{
  int			i, j, k, c ;
  FVECTOR		*src, *dst, tmp_vec, vec ;
  FVECTOR		zero_vec = {0,0,0,1};
  MTN_MOTION	*motion0 ;
  
  motion0 = MTN_DuplicateMotion( motion );
  
  if ( motion->move != NULL ){
    src = &motion0->move[ 0 ] ;
    dst = &motion->move[ 0 ] ;
    for ( j = 0 ; j <= motion->header.motion_length ; j++ ){
      vec = zero_vec ;
      for ( k = 0 ; k < SMOOTH_TIME ; k++ ){
	c = j + k - SMOOTH_TIME/2 ;
	if ( c < 0 ) c = 0 ;
	if ( c > motion->header.motion_length ) c = motion->header.motion_length ;
	tmp_vec = src[ c ];
	vec.vx += tmp_vec.vx / SMOOTH_TIME ;
	vec.vy += tmp_vec.vy / SMOOTH_TIME ;
	vec.vz += tmp_vec.vz / SMOOTH_TIME ;
      }
      dst[ j ] = vec ;
    }
  }
  
  if ( motion->trans != NULL ){
    for ( i = 0 ; i < motion->header.motion_joints ; i++ ){
      src = &motion0->trans[ i * ( motion->header.motion_length + 1 ) ] ;
      dst = &motion->trans[ i * ( motion->header.motion_length + 1 ) ] ;
      for ( j = 0 ; j <= motion->header.motion_length ; j++ )
	{
	  vec = zero_vec ;
	  for ( k = 0 ; k < SMOOTH_TIME ; k++ ){
	    c = j + k - SMOOTH_TIME/2 ;
	    if ( c < 0 ) c = 0 ;
	    if ( c > motion->header.motion_length ) c = motion->header.motion_length ;
	    tmp_vec = src[ c ];
	    vec.vx += tmp_vec.vx / SMOOTH_TIME ;
	    vec.vy += tmp_vec.vy / SMOOTH_TIME ;
	    vec.vz += tmp_vec.vz / SMOOTH_TIME ;
	  }
	  dst[ j ] = vec ;
	}
    }
  }
  
  if ( motion->rots != NULL ){
    for ( i = 0 ; i < motion->header.motion_joints ; i++ ){
      src = &motion0->rots[ i * ( motion->header.motion_length + 1 ) ] ;
      dst = &motion->rots[ i * ( motion->header.motion_length + 1 ) ] ;
      for ( j = 0 ; j <= motion->header.motion_length ; j++ )
	{
	  vec = zero_vec ;
	  for ( k = 0 ; k < SMOOTH_TIME ; k++ ){
	    c = j + k - SMOOTH_TIME/2 ;
	    if ( c < 0 ) c = 0 ;
	    if ( c > motion->header.motion_length ) c = motion->header.motion_length ;
	    tmp_vec = src[ c ];
	    MT_QuatSlerp( &tmp_vec, &zero_vec, &tmp_vec, 1.0f / SMOOTH_TIME );
	    MT_QuatMul( &vec, &vec, &tmp_vec );
	  }
	  dst[ j ] = vec ;
	}
    }
  }
  
  MTN_FreeMotion( motion0 );
}
