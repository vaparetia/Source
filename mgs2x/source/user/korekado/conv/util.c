//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	util.c
	ユーティリティー

	1997/07/07 Y.Korekado
	$Id: util.c,v 1.1.1.3 2002/11/19 11:44:05 Yoshizawa1 Exp $
	
	他の人のと、かぶっているものが多数あるはず。
	後に統合する。
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <string.h>
#endif

#include	"libgv.h"
#include	"libmt.h"
#include	"libgcl.h"
#include	"libutl.h"
#include	"gameheader.h"
#include	"define.h"
#include	"camera.h"

#include	"korekado/conv/korekado.x"

//#define M_PI 3.14159265358979323846264338327950288419716939937510f

void KR_AlertCount(void )
{
	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return ;
	
	GM_AlertCount++ ;
	if ( GM_AlertCount > GM_MAX_RESULT_COUNT ) GM_AlertCount = GM_MAX_RESULT_COUNT ;
}

void KR_KillCount(void )
{
	extern int VR_KillCount ;

	GM_KillCount++ ;
	VR_KillCount++ ;

	SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_ENEMY_KILL ) ;
	if ( GM_KillCount > GM_MAX_RESULT_COUNT ) GM_KillCount = GM_MAX_RESULT_COUNT ;
}

void KR_ClearingCount( void )
{
	GM_ClearingCount++ ;
printf("rkoekroekroekrokerokeokroek[%d]\n",GM_ClearingCount);
	if ( GM_ClearingCount > GM_MAX_RESULT_COUNT ) GM_ClearingCount = GM_MAX_RESULT_COUNT ;
}

/* 場所から床傾き計算 */
int	ENE_GetGRotFromPos( FVECTOR *pos, SVECTOR *rot, float z, HZX_GROUP_ID hzx_id, int seg_type, int flr_type )
{
    FVECTOR	head, vec ;
    float	hh, diff ;
    int		turn, flag ;	
	float		levels[ 2 ] ;
	HZX_HZD		flr[ 2 ] ;

    flag = HZX_LevelHazardCheck( hzx_id, pos, seg_type, flr_type ) ;
	if ( flag != 0 ) {
		HZX_GetLevelHeight( levels ) ; 
		HZX_GetLevelHazard( flr, NULL ) ;
	}
	if ( !( 1 & flag ) ) return 0 ;

    DG_SetPos2( pos, rot ) ;
    head.vx = head.vy = 0.0F ;
    head.vz = z ; 
    DG_PutVector( &head, &head, 1 ) ;
    HZX_SlopeFloorLevel( &hh, &head, flr ) ;
//printf(" hh=%f head=%f floor.y=%f!!\n",hh, head.vz, levels[0]);
    diff = hh - levels[ 0 ] ;
    vec.vx = diff ; 
    vec.vz = z ;
    turn = - GV_VecDir2( &vec ) ;
	if ( turn < -2048 ) turn += 4096 ;

    return turn ;
}

/* 床の高さ計算 */
float	KR_GetGRotFromPos( FVECTOR *pos, HZX_GROUP_ID hzx_id, int seg_type, int flr_type )
{
    int		flag ;	
	float		levels[ 2 ] ;

    flag = HZX_LevelHazardCheck( hzx_id, pos, seg_type, flr_type ) ;
	if ( flag != 0 ) {
		HZX_GetLevelHeight( levels ) ; 
	}
	if ( !( 1 & flag ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	if ( !( 2 & flag ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;

	return  levels[ 0 ] ;
}

float	KR_InnerProduct( FVECTOR *p, FVECTOR *v1, FVECTOR *v2 )
{
    FVECTOR	d1, d2 ;
    float f ;

    _sceVu0SubVector( &d1, p, v1 ) ;
    _sceVu0SubVector( &d2, p, v2 ) ;
    _sceVu0Normalize( &d1, &d1 ) ;
    _sceVu0Normalize( &d2, &d2 ) ;
    f = _sceVu0InnerProduct( &d1, &d2) ;

	return f ;
}

/* カメラの法線との内積 */
float	KR_InnerProductInCamera( FVECTOR *pos )
{
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/
	return KR_InnerProduct( &cam->position, &cam->target, pos ) ;
}


int	KR_CameraDis( pos )
FVECTOR		*pos ;
{
	GM_CameraSet	*cam ;
	FVECTOR		vec ;
	float dis ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
    dis = GV_VecLen3F( &vec ) ;
#if 0 //yano 0除算回避
	dis /= cam->angle/2 ;
#else
	if( cam->angle == 0.0f )cam->angle = 0.00001f;
	dis /= cam->angle/2 ;	
#endif

//printf("cam_dis[%f] [%f][%f][%f] [%f][%f][%f] \n",dis,
//	pos->vx,pos->vy,pos->vz, cam->target.vx, cam->target.vy, cam->target.vz ) ;	
	
	return (int)dis ;
}

void KR_UnsetAllObjsFlag( DG_OBJS *objs, int flag )
{
	int i ;
	
	UNSET_FLAG( objs->flag, flag ) ;
	for( i=0; i<objs->n_models; i++ ) {
		UNSET_FLAG( objs->objs[i].flag, flag ) ;
	}
}

void KR_SetAllObjsFlag( DG_OBJS *objs, int flag )
{
	int i ;
	
	SET_FLAG( objs->flag, flag ) ;
	for( i=0; i<objs->n_models; i++ ) {
		SET_FLAG( objs->objs[i].flag, flag ) ;
	}
}

/* 移動量 */
float	KR_MotionStepSpeed( MOTION_CONTROL *mt_ctrl )
{
	float speed ;

//	speed = _FVecLen2( &my_ctrl->mt3_ctrl->move->step ) ;
	speed = _FVecLen2( &mt_ctrl->step ) ;
	return speed ;
}

int	KR_MotionTime( MT3_CONTROL *mt3_ctrl )
{
	float time ;

	time = ( mt3_ctrl->play_time ) / mt3_ctrl->motion_time_base ;

	return (int)time ;
}

int	KR_LeftMotion( MT3_CONTROL *mt3_ctrl )
{
	float left ;

	left = ( mt3_ctrl->motion_total_time - mt3_ctrl->play_time ) / mt3_ctrl->motion_time_base ;

	return (int)left ;
}

int KR_MapConnect( int map1, int map2 )
{
	int	disp_map ;

//printf("KR_MapConnect map1[%x] map2[%x] \n",map1,map2 ) ;

	/* 同じマップ */
	if ( map1 & map2 ) return 1 ;

	/* 指定チャンネルに表示中のマップＩＤを返す */
	disp_map = GM_GetMapIDfromChanlDisp( 0 ) ;

	/* どちらも表示マップ */
	if ( (disp_map & map1) && (disp_map & map2) ) return 1 ;

	return 0 ;
}

void KR_FindCameraCall( OBJECT *body, FVECTOR *pos, int map ) 
{
	extern void *NewSubCameraControl( FVECTOR *pos, FVECTOR *target,
							float init_angle, float max_angle, float add_angle,
							u_int life, u_int wait_time, int map  ) ;
	static	FVECTOR	shift = { 0.0f, 400.0f, 200.0f } ;
	FMATRIX	m ;
	FVECTOR	vec ;

	DG_SetPos( &BODYWORLD( body, HUMAN21_ATAMA ) ) ;
	DG_MovePos( &shift ) ;
	DG_GetPos( &m ) ;
	KR_FMatToFvec( &m, &vec ) ;

	NewSubCameraControl( &vec, pos, 2.0f, 8.0f, 0.4f, 180, 60, map  ) ;
}

/* トラップ進入チェック */
int	KR_CheckTrap( CONTROL *ctrl, int trap_id )
{
    u_int	*inside ;
    int		n_inside ;

    n_inside = ctrl->evt.n_inside ;
    inside = ctrl->evt.inside ;
    while ( -- n_inside >= 0 ) {
		if ( ( *inside & 0x00ffffff ) == 
			( trap_id & 0x00ffffff ) ) return ( *inside ) ;
		inside ++ ;
    }
    return 0 ;
}


/* 床傾き計算 */
#if 0
int	KR_GetGRot( ctrl, z )
CONTROL		*ctrl ;
float		z ;
#else
int	KR_GetGRot( CONTROL *ctrl, float z )
#endif
{
    FVECTOR	head, vec ;
    float	hh, diff ;
    int		turn ;	

//printf(" get grot start !!\n");
    if ( ( ctrl->level[ 0 ] == NULL ) ) return 0 ;

    DG_SetPos2( &(ctrl->mov), &(ctrl->rot) ) ;
    head.vx = head.vy = 0.0F ;
    head.vz = z ; 
    DG_PutVector( &head, &head, 1 ) ;
    HZX_SlopeFloorLevel( &hh, &head, ctrl->level[ 0 ] ) ;
//printf(" hh=%f head=%f floor.y=%f!!\n",hh, head.vz, ctrl->levels[0]);
    diff = hh - ctrl->levels[ 0 ] ;
    vec.vx = diff ; 
    vec.vz = z ;
    turn = - GV_VecDir2( &vec ) ;
    if ( turn < -2048 ) turn += 4096 ;

    return turn ;
}


void	KR_DestroyActor( w )
void	**w ;
{
	if ( *w == NULL ) return ;
	GV_DestroyActor( *w ) ;
	*w = NULL ;
}

int	KR_RandU( n )
int	n ;
{
	return irnd()%n ;
}

int	KR_RandS( n )
int	n ;
{
	return n - (irnd()%(n*2)) ;
}

int	KR_GetDownDir( ctrl, dir )
CONTROL *ctrl ;
int		dir ;
{
	int wall_dir, y1,y2,d1,d2, res ;

	if ( ctrl->n_touches > 0 ) return -1 ;

	/* 一番近い壁に対して６０度の傾きを求める */
	wall_dir = GV_VecDir2( &ctrl->vecs[ 0 ] ) ;

	if ( _DiffDirAbs( dir, wall_dir ) > 680 ) return -1 ;

	y1 = 4095 & (wall_dir + 680) ;
	y2 = 4095 & (wall_dir - 680) ;
	d1 = _DiffDirAbs( dir, y1 ) ;
	d2 = _DiffDirAbs( dir, y2 ) ;

	res = ( d1 < d2 ) ? y1 : y2 ;

	return  res ;
}

void KR_MemCopy( void *dst, void *src, int size, int num )
{
	extern void UTL_StartSprToMem( void *dst, void *src, int size );
	extern void UTL_EndSprToMem( void );

	UTL_StartSprToMem( dst, src, size * num / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}

void KR_GroupObject( object, map )
OBJECT	*object ;
int		map ;
{
 	GM_CurrentMap = map ;
    object->map_name = map ;
    GM_GroupObjs( object->objs, map ) ;
}

void	_CopyObjectWorld( from, to )
DG_OBJS	*from, *to ;
{
	int	i, min ;

	_sceVu0CopyMatrix( &to->world, &from->world ) ;

	min = (from->n_models < to->n_models)?from->n_models:to->n_models ;
	for( i=0; i<min; i++ ) {
//		_sceVu0CopyMatrix( &to->objs[i].world, &from->objs[i].world ) ;
to->objs[i].world=from->objs[i].world;
	}
}

/*floatのrot値からintへ四捨五入*/
/*2002.08.15 sigeno*/
#ifdef PSX2
//PSX2
#define F_ROT_TO_INT(a) ( ((int)(a)) & 4095)
#else
//XBOX
#define F_ROT_TO_INT(a) ( ((int)(a+0.5f)) & 4095)
#endif



int MatToYRot( mat )
FMATRIX	*mat ;
{
	FVECTOR	res, x, y ;
	int	dir ;

#define _ABSf(x) ((x>=0.0f)?(x):(-(x)))
	if ( _ABSf(mat->m[0][1]) > _ABSf(mat->m[2][1]) ){
		y.vx = mat->m[2][0] ;
		x.vx = mat->m[2][2] ;
	} else {
		x.vx = mat->m[0][0] ;
		y.vx = -mat->m[0][2] ;
	}
	MT_Atan2X4( &res, &y, &x );
	dir = res.vx * 2048 / (float)M_PI ;

	return dir ;
}


int MatToXRot( mat )
FMATRIX	*mat ;
{
	FVECTOR	res, x, y ;
	int	dir ;

	x.vx = mat->m[1][1] ;
	y.vx = -mat->m[2][1] ;

	MT_Atan2X4( &res, &y, &x );
	dir = res.vx * 2048 / (float)M_PI ;

	return dir ;
/*
	static FVECTOR	f={0.0F, 0.0F, 1000.0F} ;
	FVECTOR	from, to, vec ;
	int	res ;
	
	KR_FMatToFvec( &BODYWORLD( entk->act->body, HUMAN21_ATAMA ), &from ) ;
	DG_SetPos( &BODYWORLD( entk->act->body, HUMAN21_ATAMA ) ) ;
	DG_PutVector( &f, &to, 1 ) ;
	_sceVu0SubVector(  &vec, &to, &from ) ;
	res = _FVecDirX( &vec ) ;
*/
}

void	KR_FMatToFvec( FMATRIX	*mat, FVECTOR *vec ) {
	vec->vx = mat->m[ 3 ][ 0 ] ;
	vec->vy = mat->m[ 3 ][ 1 ] ;
	vec->vz = mat->m[ 3 ][ 2 ] ;
}

void	KR_FvecToMat( FVECTOR *vec, FMATRIX	*mat ) {
	mat->m[ 3 ][ 0 ] = vec->vx ;
	mat->m[ 3 ][ 1 ] = vec->vy ;
	mat->m[ 3 ][ 2 ] = vec->vz ;
}

void	_ScaleMatrix( mat, vec )
FMATRIX		*mat ;
FVECTOR		*vec ;
{
	mat->m[0][0] *= vec->vx ;
	mat->m[1][0] *= vec->vx ;
	mat->m[2][0] *= vec->vx ;

	mat->m[0][1] *= vec->vy ;
	mat->m[1][1] *= vec->vy ;
	mat->m[2][1] *= vec->vy ;

	mat->m[0][2] *= vec->vz ;
	mat->m[1][2] *= vec->vz ;
	mat->m[2][2] *= vec->vz ;

//printf("scalemat> [%f][%f][%f]\n",mat->m[0][0],mat->m[1][1],mat->m[2][2] );
}

void	_RotMatrixZYX( FMATRIX *mat, SVECTOR *rot )
{
	float		rot_x, rot_y, rot_z ;
	int			tmp ;

	tmp = rot->vz ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_z = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vx ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_x = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vy ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_y = (float)tmp * (float)M_PI / 2048.0F ;
	_sceVu0UnitMatrix( mat );
	_sceVu0RotMatrixZ( mat, mat, rot_z );
	_sceVu0RotMatrixX( mat, mat, rot_x );
	_sceVu0RotMatrixY( mat, mat, rot_y );

	mat->m[ 3 ][ 0 ] = 0 ;
	mat->m[ 3 ][ 1 ] = 0 ;
	mat->m[ 3 ][ 2 ] = 0 ;
}

void	_FRotQuat( quat, frot )
FVECTOR		*quat ;
FVECTOR		*frot ;
{
	FMATRIX	mat ;
	SVECTOR	rot ;

	rot.vx = (short)frot->vx ;
	rot.vy = (short)frot->vy ;
	rot.vz = (short)frot->vz ;

    _RotMatrixZYX( &mat, &rot ) ;
	MT_MatToQuat( quat, &mat ) ;
}

void	_RotQuat( quat, rot )
FVECTOR		*quat ;
SVECTOR		*rot ;
{
	FMATRIX	mat ;

    _RotMatrixZYX( &mat, rot ) ;
	MT_MatToQuat( quat, &mat ) ;
}


static inline void _FvecToSvec( FVECTOR *fvec, SVECTOR *svec )
{
	svec->vx = (short)(fvec->vx) ;
	svec->vy = (short)(fvec->vy) ;
	svec->vz = (short)(fvec->vz) ;
}
static inline void _SvecToFvec( SVECTOR *svec,FVECTOR *fvec )
{
	fvec->vx = (float)(svec->vx) ;
	fvec->vy = (float)(svec->vy) ;
	fvec->vz = (float)(svec->vz) ;
}

float		_FVecLen2( vec )
FVECTOR		*vec ;
{
	FVECTOR		tmp ;

	tmp.vx = vec->vx * vec->vx ;
	tmp.vz = vec->vz * vec->vz ;
	return bp_sqrtf( tmp.vx + tmp.vz ) ;   //BP_MATH - emulate PS2 sqrtf
}

int		_FVecLen3( vec )
FVECTOR		*vec ;
{
	FVECTOR		tmp ;

	tmp.vx = vec->vx * vec->vx ;
	tmp.vy = vec->vy * vec->vy ;
	tmp.vz = vec->vz * vec->vz ;
	return (int)bp_sqrtf( tmp.vx + tmp.vy + tmp.vz ) ; //BP_MATH - emulate PS2 sqrtf
}

int		_FVecDir2( vec )
FVECTOR		*vec ;
{
	float	f ;
	int		r ;

	f = atan2f( vec->vx, vec->vz ) ;
#if 0
	r = (int) (2048.0f * f / ( float )M_PI) ;
	r &= 4095 ;
#else
	r = F_ROT_TO_INT((2048.0f * f / ( float )M_PI)) ;
#endif	
// 	printf("x=%f z=%f  f = %f r = %d\n",vec->vx, vec->vz, f, r ) ;
	return r ;
}

int		_FVecDirX( vec )
FVECTOR		*vec ;
{
	float	f, d_xz ;
	int		r  ;

	d_xz = (float)_FVecLen2( vec ) ;


	f = atan2f( d_xz, vec->vy ) ;
#if 0
	r = (int) (2048.0f * f / ( float )M_PI) ;
	r &= 4095 ;
#else
	r = F_ROT_TO_INT((2048.0f * f / ( float )M_PI)) ;
#endif
	return r ;
}

int		_FVecDirZ( vec )
FVECTOR		*vec ;
{
	float	f, d_xz ;
	int		r  ;
	FVECTOR		tmp ;

	tmp.vx = vec->vx * vec->vx ;
	tmp.vy = vec->vy * vec->vy ;
	d_xz = bp_sqrtf( tmp.vx + tmp.vy ) ;   //BP_MATH - emulate PS2 sqrtf

	f = atan2f( d_xz, vec->vy ) ;
#if 0
	r = (int) (2048.0f * f / ( float )M_PI) ;
	r &= 4095 ;
#else
	r = F_ROT_TO_INT((2048.0f * f / ( float )M_PI)) ;
#endif
	return r ;
}

void	_FVecToRotXY( vec, rot )
FVECTOR		*vec ;
SVECTOR		*rot ;
{
	rot->vx = (short)_FVecDirX( vec ) ;
	rot->vy = (short)_FVecDir2( vec ) ;
	rot->vz = 0 ;
}

void	_FVecToRotYZ( vec, rot )
FVECTOR		*vec ;
SVECTOR		*rot ;
{
	rot->vx = 0 ;
	rot->vy = (short)_FVecDir2( vec ) ;
	rot->vz = (short)_FVecDirZ( vec ) ;
}

int		_FVecTrgDir2( FVECTOR *pos, FVECTOR *trg )
{
	FVECTOR	vec ;
	int		dir ;

	_sceVu0SubVector(  &vec, trg, pos ) ;
	dir = _FVecDir2( &vec ) ;

	return dir ;
}

/* 水平方向距離 */
int		KR_FVecTrgDis2( FVECTOR *pos, FVECTOR *trg )
{
	FVECTOR	vec ;
	int		dis ;

	_sceVu0SubVector(  &vec, trg, pos ) ;
	dis = _FVecLen2( &vec ) ;

	return dis ;
}
int		_FVecTrgDis( FVECTOR *pos, FVECTOR *trg )
{
	FVECTOR	vec ;
	int		dis ;

	_sceVu0SubVector(  &vec, trg, pos ) ;
	dis = _FVecLen3( &vec ) ;

	return dis ;
}

float _RsinF( d )
int	d ;
{
	float f, t ;

	f = ( float )M_PI * d / 2048.0F ;
	t = sinf( f ) ;
//	printf("f = %f sinf =%f\n",f,t );
	return t ;
}

float _RcosF( d )
int	d ;
{
	float f, t ;

	f = ( float )M_PI * d / 2048.0F ;
	t = cosf( f ) ;
//printf("f = %f cosf =%f\n",f,t );
	return t ;
}

int		_DiffDirAbs( from, to )
int		from, to ;
{
	int		diff ;

	diff = 4095 & ( to - from ) ;
	return ( diff <= 2048 ) ? diff : 4096 - diff ;
}

int		_DiffDisAbs( from, to )
FVECTOR		*from, *to ;
{
	FVECTOR	vec ;
	int		diff ;

	_sceVu0SubVector(  &vec, from, to ) ;
	diff = _FVecLen3( &vec ) ;
	return ( diff < 0 ) ? -diff : diff ;
}

int	_PosInRangeXZ( pos, center, range )
FVECTOR	*pos ;
FVECTOR	*center ;
int		range ;
{
	float		dx, dz ;

	dx = pos->vx - center->vx ;
	dz = pos->vz - center->vz ;
	if ( (int)dx > -range && (int)dx < range && (int)dz > -range && (int)dz < range ) return 1 ;
	
	return 0 ;
}

/* 巡回セールスマン問題(Traveling Salesman Problem)、厳密解 */
#define MAX_DIM	7+1
#define PENALTY	255

int TSP_map[MAX_DIM-1][MAX_DIM-1] ;
static int min_path[ MAX_DIM ] ;
static int true_min[ MAX_DIM ] ;
static int min_path_point ;
static int min_cost ;

static void TspSearch(int dim, int now_town, int *no_path, int no_path_count, int now_cost)
{
	int new_cost ;
	int i, j;
	int new_no_path[ MAX_DIM-1 ][ MAX_DIM-2 ] ;

	if(no_path_count == 1) {
		new_cost = now_cost + TSP_map[now_town][no_path[0]] ;
		if(TSP_map[now_town][no_path[0]] == 0)	new_cost += PENALTY;
		if(new_cost < min_cost) {
			min_cost = new_cost;
			min_path[min_path_point] = no_path[0];
			memcpy(true_min, min_path, sizeof(int) * (dim + 1));
		}
		return;
	}

	for(i = 0; i < no_path_count; i++) {
		new_cost = now_cost + TSP_map[now_town][no_path[i]];
		if(TSP_map[now_town][no_path[i]] == 0)
			new_cost += PENALTY;
		if(new_cost >= min_cost) {
			continue;
		}

		for(j = 0; j < no_path_count; j++) {
			if(i > j)
				new_no_path[i][j] = no_path[j];
			else if(i < j)
				new_no_path[i][j - 1] = no_path[j];
		}
		min_path[min_path_point] = no_path[i];
		min_path_point++;
		TspSearch(dim, no_path[i], new_no_path[i], no_path_count - 1, new_cost);
		min_path_point--;
	}
}

int TSP( dim, min_route )
int dim ;
int *min_route ;
{
	int no_path[MAX_DIM];
	int i ;

for( i=0; i<MAX_DIM; i++ ) {
	min_path[ i ] = 9999 ;
	true_min[ i ] = 9999 ;
}

	min_path[0] = 0;
	min_path_point = 1;
	min_cost = PENALTY * MAX_DIM ;

	for( i=1; i<dim; i++ ) {
		no_path[i - 1] = i;
	}

	TspSearch( dim, 0, no_path, dim - 1, 0) ;
	true_min[ dim ] = 0 ;	/* 最後は元に戻る */

	memcpy(min_route, true_min, sizeof(int) * (dim + 1));

{
	int j ;
	
	printf("dim = %d\n",dim);
	for( i=0; i<dim; i++ ) {
		for( j=0; j<dim; j++ ) {
			printf("[%d]",TSP_map[i][j]);
		}
		printf("\n");
	}

	printf("cost %d\n", min_cost);
	printf("path ");
	for(i = 0; i < dim; i++)
		printf("%2d->", true_min[i]);
	printf("%2d\n", true_min[i]);
	
	return min_cost ;
}

}

int	ENE_PreShade( objs, id )
DG_OBJS		*objs ;
int			id ;
{
#if 0
	int	i ;
	DG_LITS_OLD	*lights ;
	DG_LIT		*lit ;

	lights = (DG_LITS_OLD*)GV_GetCache( GV_CacheID( id, 'l' ) ) ;
	lit = GV_Malloc( sizeof(DG_LIT) * lights->n_lights );
	for ( i = 0 ; i < lights->n_lights ; i++ ){
		lit[i].point.vx = lights->lights[i].point.vx ;
		lit[i].point.vy = lights->lights[i].point.vy ;
		lit[i].point.vz = lights->lights[i].point.vz ;
		lit[i].color = lights->lights[i].color ;
		lit[i].r_range = lights->lights[i].r_range ;
		lit[i].e_range = lights->lights[i].e_range ;
		lit[i].force = 
		    ( (float)lit[i].color.r * 0.30F
		     + (float)lit[i].color.g * 0.59F
		     + (float)lit[i].color.b * 0.11F ) / 255.0F ;
	}
	DG_MakePreshade( objs, lit  );
	GV_Free( lit ) ;
#else
	LIT_DEF	*def ;

	def = (LIT_DEF*)GV_GetCache( GV_CacheID( id, 'l' ) );
	ASSERT( def != NULL ) ;
	return DG_MakePreshade( objs, def );
#endif
}

/*--- GCL ----------------------*/
void	ENE_ExecProc( int proc_id, GCL_ARGS *arg )
{
	if ( proc_id != 0 ) {
		GCL_ExecProc( proc_id, arg ) ;
	}
}

int	ENE_GCL_GetFV( char *ptr, FVECTOR *fvec )
{
	int	vec[ 3 ] ;

	if ( GCL_NextStr() != NULL ){
//		GCL_GetIV( ptr, vec ) ;
		GCL_GetNextIV( vec ) ;
		fvec->vx = (float)vec[0] ;
		fvec->vy = (float)vec[1] ;
		fvec->vz = (float)vec[2] ;
		return 0 ;
	}
	
	return -1 ;
}

int	ENE_GCL_GetFVs( FVECTOR *fvec )
{
	int	vec[ 3 ], num ;

	num = 0 ;
	while ( GCL_NextStr() != NULL ){
		GCL_GetNextIV( vec ) ;
		fvec->vx = (float)vec[0] ;
		fvec->vy = (float)vec[1] ;
		fvec->vz = (float)vec[2] ;
		
		fvec++ ;
		num++ ;
	}

	return num ;
}


int	ENE_GCL_GetFVandM( FVECTOR *fvec, int *map )
{
	int	vec[ 3 ], num, mapname ;

	num = 0 ;
	while ( GCL_NextStr() != NULL ){
		GCL_GetNextIV( vec ) ;
		fvec->vx = (float)vec[0] ;
		fvec->vy = (float)vec[1] ;
		fvec->vz = (float)vec[2] ;
if ( GCL_NextStr() == NULL ) {
	extern void	HZX_Pos2Zone( FVECTOR *, int *, int * ) ;
	int g, z ;
	HZX_Pos2Zone( fvec, &g, &z );
	*map = GM_GetBit( g ) ;
} else {
		mapname = GCL_GetNextInt( ) ;
		*map = GM_GetHzxGroupID( GM_GetMapID( mapname ) );
}
		fvec++ ;
		map++ ;
		num++ ;
	}

	return num ;
}

int ENE_GclCallProcs( char *top )
{
	int i, id;
	char *p ;

	for( i = 0; GCL_NextStr() != NULL; i++ ){
		id = GCL_GetNextInt( ) ;
		p = GCL_NextStr() ;
		GCL_ExecProc( id, NULL );
		GCL_SetNextStr( p ) ;
	}

	return i;
}

int ENE_GclGetInt( char *top, int *buff ) 
{
	int i ;
	
#if 1
	for( i = 0; GCL_NextStr() != NULL; i++ ){
		*buff = GCL_GetNextInt( ) ;
printf("buff=%d\n",*buff);	
		buff++ ;
	}
#else
	i = 0 ;
	while ( top != NULL ) {
		*buff = GCL_GetInt( top ) ;
		buff++ ;
		i++ ;
		top = GCL_NextStr() ;
	}
#endif
printf("iiiiiiiiiii=%d\n",i);	
	return i ;
}

int ENE_GclGetProc( proc, argv )
int *proc ;
int	*argv ;
{
	int	argc, i ;

	*proc = 0 ;
	argc = 0 ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	*proc = GCL_GetNextInt( ) ;

	for( i = 0; GCL_NextStr() != NULL; i++, argc++ ){
		*argv++ = GCL_GetNextInt( ) ;
	}
	
	return argc ;
}


void KR_ProcCallSendName( int proc, int name )
{
	GCL_ARGS arg ;
	int	argv ;

	argv = name ;
	arg.argc = 1 ;
	arg.argv = &argv ;
	ENE_ExecProc( proc, &arg ) ;
}
