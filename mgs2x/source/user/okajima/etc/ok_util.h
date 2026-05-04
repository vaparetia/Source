/*
	ok_util.c
	おかじの便利集ヘッダ
	2000/01/18 S.Okajima
	$Id: ok_util.h,v 1.1.1.3 2002/11/19 11:47:41 Yoshizawa1 Exp $
*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

extern float OK_RectLen( FVECTOR *pos0, FVECTOR *pos1 );
extern void OK_Print3D_perse( FVECTOR *pos, char *strings, CVECTOR col );
extern DG_PRIM2 *OK_FreePrim2( DG_PRIM2 *prim );
extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern void OK_DirVecXY_Y( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern void OK_IntSmoother( int *itemp1, int *itemp2, float ratio );
extern void OK_FloatSmoother( float *ftemp1, float *ftemp2, float ratio );
extern void OK_DirectionSmoother( SVECTOR *svect1, SVECTOR *svect2, float ratio );
extern int	OK_CheckCrossBoundaryOuter( FVECTOR *from, FVECTOR *to, FVECTOR *bound_min, FVECTOR *bound_max, FVECTOR *hit );
extern int	OK_CheckCrossBoundaryInner( FVECTOR *from, FVECTOR *to, FVECTOR *bound_min, FVECTOR *bound_max, FVECTOR *hit );
extern void OK_Mem_Scr( void *dst, void *src, int size, int num );
extern void OK_Scr_Mem( void *dst, void *src, int size, int num );
extern void OK_FloorToBoundary( HZX_FLR *flr, FVECTOR *fv );
extern void OK_NoFloorInit( FVECTOR *fv, float *fl );
extern int	OK_StepCheckHzd( FVECTOR *now, FVECTOR *before, FVECTOR *vec, float decay, float margin, HZX_GROUP_ID map_id );
extern int  OK_CheckPos_Floor( HZX_FLR *flr, FVECTOR *pos );
extern int  OK_CheckFloorEdge( FVECTOR *ans_vec, FVECTOR *pos, float check_length, HZX_FLR *flr );
extern int  OK_CheckTrap( CONTROL *ctrl, int trap_id );
extern int	OK_CheckBoundOnCamera( FVECTOR *fvec0, FVECTOR *fvec1, int cam_num );
extern void OK_ScaleVector( FVECTOR *output_v, FVECTOR *input_v, float scale, int num );
extern int	OK_PutFloorLight( FVECTOR *center, float floor_height, float radius, CVECTOR col );
extern void OK_Print3D_perse( FVECTOR *pos, char *strings, CVECTOR col );

extern int		OK_RandamField_irnd[];
extern float	OK_RandamField_rnd[];
extern float	OK_RandamField_frnd[];
extern void OK_irnd_to_scr( void *dst, int num );
extern void OK_rnd_to_scr( void *dst, int num );
extern void OK_frnd_to_scr( void *dst, int num );
extern float const *OK_frnd_to_ptr( int num );

#ifdef __GNUC__
#define OK_printf( fmt, args... )	 printf("OKP:"fmt, ##args )
#define oprintf( fmt, args... )	 printf("OKP:"fmt, ##args )
#else
#define OK_printf	printf
#define oprintf		printf
#endif


#define	OK_SHIFT_UPPER	( 32 )
#define OK_DISPLAY_INFORMATION_TIME0 (10)
#define OK_DISPLAY_INFORMATION_TIME1 (120)
#define OK_DISPLAY_INFORMATION_TIME2 (30)

#define OK_MENU_COL_R (196)
#define OK_MENU_COL_G (196)
#define OK_MENU_COL_B (196)

#define OK_MENU_FULL_COL_R (196)
#define OK_MENU_FULL_COL_G (64)
#define OK_MENU_FULL_COL_B (64)
#define OK_MENU_COL_A (128)


#define	OK_SPLUSH_MOUNTAIN_MAX	(16)


//-------------------------------
static inline int OK_ItemVisionCheck( void )
{
	if( (GM_Item == IT_NightVision)
	 || (GM_Item == IT_Thermal)
	  ){
		return 1;
	}
	return 0;
}


