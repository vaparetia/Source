/*----------------------------------------------------------
  util.h
  各種計算ユーティリティ

  1999/07/07 M.Sonoyama
  $Id: util.h,v 1.1.1.3 2002/11/19 11:42:46 Yoshizawa1 Exp $			  
-----------------------------------------------------------*/

#ifndef __LIBGV_UTIL_H__
#define	__LIBGV_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void	GV_ItoFVector( FVECTOR *, SVECTOR * ) ;
extern	void	GV_FtoIVector( SVECTOR *, FVECTOR * ) ;
extern	void	GV_IVtoFV( void *, void *, int ) ;
extern	void	GV_FVtoIV( void *, void *, int ) ;
extern	int	GV_VecLen3( SVECTOR * ) ;
extern	float	GV_VecLen3F( FVECTOR * ) ;
extern	float	GV_VecLen3F2( FVECTOR *, FVECTOR * ) ;
extern	void	GV_LenVec3( SVECTOR *, SVECTOR *, int, int ) ;
extern	void	GV_SubVec3( SVECTOR *, SVECTOR *, SVECTOR * ) ;
extern	void	GV_LenVec3F( FVECTOR *, FVECTOR *, float, float ) ;
extern	void	GV_SubVec3F( FVECTOR *, FVECTOR *, FVECTOR * ) ;
extern	short	GV_VecDir2( FVECTOR * ) ;
extern	short	GV_VecDir2X( FVECTOR * ) ;
extern	short	GV_VecDir2FromTo( FVECTOR *from, FVECTOR *to ) ;
extern	void	GV_VecToRot( FVECTOR *, SVECTOR * ) ;
extern	void	GV_DirVec3( SVECTOR *, int, FVECTOR * ) ;
extern	int	GV_DiffDirU( int, int ) ;
extern	int	GV_DiffDirS( int, int ) ;
extern	int	GV_DiffDirAbs( int, int ) ;
extern	float	GV_PointToVectorLen( FVECTOR *, FVECTOR *, FVECTOR * ) ;
extern	void	GV_GetShiftPos( FVECTOR *, FVECTOR *, SVECTOR *, float, float, float ) ;

extern	int	GV_NearExp2( int, int ) ;
extern	int	GV_NearExp4( int, int ) ;
extern	int	GV_NearExp8( int, int ) ;
extern	int	GV_NearExp16( int, int ) ;
extern	int	GV_NearPhase( int, int ) ;
extern	int	GV_NearRange( int, int, int ) ;
extern	int	GV_NearSpeed( int, int, int ) ;
extern	int	GV_NearTime( int, int, int ) ;

extern	int	GV_NearExp2P( int, int ) ;
extern	int	GV_NearExp4P( int, int ) ;
extern  int	GV_NearExp8P( int, int ) ;
extern  int	GV_NearExp16P( int, int ) ;
extern  int	GV_NearExpNP( int, int, int ) ;
extern	int	GV_NearRangeP( int, int, int ) ;
extern	int	GV_NearSpeedP( int, int, int ) ;
extern	int	GV_NearTimeP( int, int, int ) ;

//#ifdef PAL
extern  int		GV_NearExp8PPAL( int, int ) ;
extern	void	GV_NearExp8PVPAL( void *, void *, int ) ;
//#endif

extern void	GV_NearExp2V( void *, void *, int ) ;
extern void	GV_NearExp4V( void *, void *, int ) ;
extern void	GV_NearExp8V( void *, void *, int ) ;
extern void	GV_NearExp16V( void *, void *, int ) ;
extern void	GV_NearRangeV( void *, void *, void *, int ) ;
extern void	GV_NearTimeV( void *, void *, int, int ) ;
extern void	GV_NearSpeedV( void *, void *, int, int ) ;

extern void	GV_NearExp2PV( void *, void *, int ) ;
extern void	GV_NearExp4PV( void *, void *, int ) ;
extern void	GV_NearExp8PV( void *, void *, int ) ;
extern void	GV_NearExp16PV( void *, void *, int ) ;
extern void	GV_NearExpNPV( void *, void *, int, int ) ;
extern void	GV_NearTimePV( void *, void *, int, int ) ;
extern void	GV_NearTimePV2( void *, void *, int, int ) ;
extern void	GV_NearSpeedPV( void *, void *, int, int ) ;

/*-----------------------------------------------------------*/

extern float	GV_NearExp2F( float, float ) ;
extern float	GV_NearExp4F( float, float ) ;
extern float	GV_NearExp8F( float, float ) ;
extern float	GV_NearExp16F( float, float ) ;
extern float	GV_NearExpNF( float, float, float ) ;
extern float	GV_NearRangeF( float, float, float ) ;
extern float	GV_NearSpeedF( float, float, float ) ;
extern float	GV_NearTimeF( float, float, int ) ;

extern void	GV_NearExp2VF( void *, void *, int ) ;
extern void	GV_NearExp4VF( void *, void *, int ) ;
extern void	GV_NearExp8VF( void *, void *, int ) ;
extern void	GV_NearExp16VF( void *, void *, int ) ;
extern void	GV_NearExpNVF( void *, void *, float, int ) ;
extern void	GV_NearRangeVF( void *, void *, void *, int ) ;
extern void	GV_NearTimeVF( void *, void *, int, int ) ;

extern	void	GV_MatToVec( FMATRIX *, FVECTOR * ) ;
extern	void	GV_VecToMat( FVECTOR *, FMATRIX * ) ;

extern	float	GV_VecLen3F( FVECTOR * ) ;
extern	void	GV_LenVec3F( FVECTOR *, FVECTOR *, float, float ) ;

/*-----------------------------------------------------------*/

//extern	GM_CameraSystemWork	GM_Camera ;
//extern	GM_SnakeCameraWork	GM_SnakeCamera ;

/*-----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif /* __LIBGV_UTIL_H__ */
