//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	deadline.c
	死体線

	2002/03/12 Y.Korekado
	$Id: deadline.c,v 1.1.1.3 2002/11/19 11:44:27 Yoshizawa1 Exp $
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
#endif

#include	"gameheader.h"
#include	"korekado/conv/define.h"
#include	"korekado/conv/korekado.x"
/*----------------------------------------------------------------*/
#define PRIM_LINE	(1)

#define DEAD_TYPE_NORMAL	(0)
#define DEAD_TYPE_UTUBUSE	(1)
#define DEAD_TYPE_LEAN		(2)

#define	RAISE			(0)
#define PRIO	0x40	/* 敵兵より後に処理 */

#define LINE_WIDTH	(100.0f)
#define HIGHT_LEVEL	(2000.0f)

#define	MAX_POS	(106)		/* 32*3 + 10 = 106 */
#define N_VERTS		(MAX_POS*2)
#define N_PRIMS		(1)

#define	N_VERTS_STRIP	(64)	/* ストリップは最高６４頂点まで */
#define	N_PRIMS_STRIP	((N_VERTS/64) + 1)
//#define	N_LINES_STRIP	((MAX_POS/64) + 1)
#define	N_LINES_STRIP	(3)
#define	MAX_STRIP	(N_VERTS_STRIP * N_PRIMS_STRIP)

#define	N_EFFECT_PRIMS	(32)
#define	N_EFFECT_VERTS	(2)

#if 1
#define	COL_A	(128)
#define	COL_R	(128)
#define	COL_G	(1)
#define	COL_B	(1)
#else
#define	COL_A	(128)
#define	COL_R	(128)
#define	COL_G	(128)
#define	COL_B	(128)
#endif

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

//#define CHECK_FLOOR	(HZX_FLOOR_NO_ENEMY|HZX_FLOOR_IK)
#define CHECK_FLOOR	(HZX_FLOOR_NO_PLAYER )
/*----------------------------------------------------------------*/
#define DL_STATUS_NEAR_SEG	0x00000001	/* もたれる可能性ある壁がある */
#define DL_STATUS_FACE_LEAN	0x00000002	/* 顔がもたれている */

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			hzx_id ;
	OBJECT		*body ;
	FVECTOR		trans_pos[ MAX_STRIP ] ;
	DG_PRIM2	*prim_strip ;
	int			disp_count ;
	int			type ;
	int			lean_dir ;
	int			max_disp_pos ;
	int			status ;

	float		old_floor_level ;
	HZX_HZD		old_flr[ 2 ] ;
	FMATRIX		shadow_mat ;

	DG_PRIM2	*effect_prim ;
} Work ;

/*----------------------------------------------------------------*/
/*
ぺらぺらゴルちゃんのアウトラインリニア座標でございます。

         "POSITION",
*/
#if 0
FVECTOR	pera[MAX_POS] = {
         {0.0,1830.0,0.0},12
         {40.0,1820.0,0.0},12
         {70.0,1790.0,0.0},12
         {80.0,1750.0,0.0},12
         {80.0,1710.0,0.0},12
         {70.0,1660.0,0.0},12
         {50.0,1610.0,0.0},12
         {80.0,1610.0,0.0},12
         {90.0,1580.0,0.0},12
         {190.0,1540.0,0.0},11
         {230.0,1480.0,0.0},8
         {250.0,1390.0,0.0},8
         {250.0,1200.0,0.0},8
         {240.0,1000.0,0.0},
         {220.0,940.0,0.0},
         {220.0,850.0,0.0},
         {210.0,800.0,0.0},
         {170.0,760.0,0.0},
         {160.0,780.0,0.0},
         {170.0,800.0,0.0},
         {140.0,810.0,0.0},
         {150.0,870.0,0.0},
         {140.0,910.0,0.0},
         {170.0,940.0,0.0},
         {150.0,1010.0,0.0},
         {150.0,1200.0,0.0},
         {120.0,1390.0,0.0},
         {170.0,1360.0,0.0},
         {170.0,1260.0,0.0},
         {170.0,1120.0,0.0},
         {190.0,1000.0,0.0},
         {190.0,740.0,0.0},		//31

         {190.0,740.0,0.0},		//32 ストリップの開始位置を前のポイントにする
         {160.0,520.0,0.0},
         {180.0,440.0,0.0},
         {160.0,360.0,0.0},
         {190.0,280.0,0.0},
         {150.0,230.0,0.0},
         {140.0,130.0,0.0},
         {140.0,80.0,0.0},
         {150.0,40.0,0.0},
         {150.0,0.0,0.0},
         {40.0,0.0,0.0},
         {40.0,40.0,0.0},
         {50.0,80.0,0.0},
         {60.0,130.0,0.0},
         {40.0,240.0,0.0},
         {10.0,280.0,0.0},
         {20.0,370.0,0.0},
         {10.0,450.0,0.0},
         {30.0,520.0,0.0},
         {0.0,800.0,0.0},
         {0.0,900.0,0.0},
         {0.0,800.0,0.0},
         {-30.0,520.0,0.0},
         {-10.0,450.0,0.0},
         {-20.0,370.0,0.0},
         {-10.0,280.0,0.0},
         {-40.0,240.0,0.0},
         {-60.0,130.0,0.0},
         {-50.0,80.0,0.0},
         {-40.0,40.0,0.0},
         {-40.0,0.0,0.0},
         {-150.0,0.0,0.0},		

         {-150.0,0.0,0.0},				//32 ストリップの開始位置を前のポイントにする
         {-150.0,40.0,0.0},
         {-140.0,80.0,0.0},
         {-140.0,130.0,0.0},
         {-150.0,230.0,0.0},
         {-190.0,280.0,0.0},
         {-160.0,360.0,0.0},
         {-180.0,440.0,0.0},
         {-160.0,520.0,0.0},
         {-190.0,740.0,0.0},
         {-190.0,1000.0,0.0},
         {-170.0,1120.0,0.0},
         {-170.0,1260.0,0.0},
         {-170.0,1360.0,0.0},
         {-120.0,1390.0,0.0},
         {-150.0,1200.0,0.0},
         {-150.0,1010.0,0.0},
         {-170.0,940.0,0.0},
         {-140.0,910.0,0.0},
         {-150.0,870.0,0.0},
         {-140.0,810.0,0.0},
         {-170.0,800.0,0.0},
         {-160.0,780.0,0.0},
         {-170.0,760.0,0.0},
         {-210.0,800.0,0.0},
         {-220.0,850.0,0.0},
         {-220.0,940.0,0.0},
         {-240.0,1000.0,0.0},
         {-250.0,1200.0,0.0},
         {-250.0,1390.0,0.0},
         {-230.0,1480.0,0.0},
         {-190.0,1540.0,0.0},

         {-190.0,1540.0,0.0},				//32 ストリップの開始位置を前のポイントにする
         {-90.0,1580.0,0.0},
         {-80.0,1610.0,0.0},
         {-50.0,1610.0,0.0},
         {-70.0,1660.0,0.0},
         {-80.0,1710.0,0.0},
         {-80.0,1750.0,0.0},
         {-70.0,1790.0,0.0},
         {-40.0,1820.0,0.0},
         {0.0,1830.0,0.0},		//始点に戻す 32*3 + 10 = 106
} ;
#else
FVECTOR	pera[MAX_POS] = {
	{0.0,165.0,0.0,		12.0},
	{40.0,155.0,0.0,	12.0},
	{70.0,125.0,0.0,	12.0},
	{80.0,85.0,0.0,		12.0},
	{80.0,45.0,0.0,		12.0},
	{70.0,-5.0,0.0,		12.0},
	{50.0,35.0,0.0,		11.0},
	{80.0,35.0,0.0,		11.0},
	{90.0,290.0,0.0,	2.0},
	{148.0,52.0,0.0,	7.0},
	{50.0,-7.0,0.0,		8.0},
	{70.0,-97.0,0.0,	8.0},
	{70.0,-287.0,0.0,	8.0},
	{60.0,-190.0,0.0,	9.0},	//13
	{40.0,-250.0,0.0,	9.0},
	{28.0,-100.0,-57.0,	10.0},
	{18.0,-150.0,-57.0,	10.0},
	{-22.0,-190.0,-57.0,	10.0},
	{-32.0,-170.0,-57.0,	10.0},
	{-22.0,-150.0,-57.0,	10.0},
	{-52.0,-140.0,-57.0,	10.0},
	{-42.0,-80.0,-57.0,	10.0},
	{-52.0,-40.0,-57.0,	10.0},
	{-10.0,-250.0,0.0,	9.0},
	{-30.0,-180.0,0.0,	9.0},
	{-30.0,-287.0,0.0,	8.0},	//25
	{98.0,-98.0,0.0,	7.0},
	{170.0,70.0,0.0,		2.0},
	{170.0,130.0,0.0,	2.0},
	{170.0,-5.0,0.0,		2.0},
	{190.0,-125.0,0.0,	2.0},	//30
	{60.0,-250.0,-28.0,	17.0},	//31

	{60.0,-250.0,-28.0,	17.0},
	{30.0,-470.0,-28.0,	17.0},
	{50.0,-118.0,-35.0,	18.0},
	{30.0,-197.0,-35.0,	18.0},
	{60.0,-277.0,-35.0,	18.0},
	{20.0,-327.0,-35.0,	18.0},
	{10.0,-427.0,-35.0,	18.0},
	{10.0,-100.0,0.0,	19.0},
	{20.0,-140.0,0.0,	19.0},
	{20.0,-180.0,0.0,	19.0},
	{-90.0,-180.0,0.0,	19.0},
	{-90.0,-140.0,0.0,	19.0},
	{-80.0,-100.0,0.0,	19.0},
	{-70.0,-427.0,-35.0,	18.0},
	{-90.0,-317.0,-35.0,	18.0},
	{-120.0,-277.0,-35.0,	18.0},
	{-110.0,-187.0,-35.0,	18.0},
	{-120.0,-108.0,-35.0,	18.0},
	{-100.0,-470.0,-28.0,	17.0},
	{-130.0,-190.0,-28.0,	17.0},
	{0.0,-225.0,0.0,		0.0},
	{130.0,-190.0,-27.0,	13.0},
	{100.0,-470.0,-27.0,	13.0},
	{120.0,-108.0,-35.0,	14.0},
	{110.0,-187.0,-35.0,	14.0},
	{120.0,-277.0,-35.0,	14.0},
	{90.0,-317.0,-35.0,	14.0},
	{70.0,-427.0,-35.0,	14.0},
	{80.0,-100.0,0.0,	15.0},
	{90.0,-140.0,0.0,	15.0},
	{90.0,-180.0,0.0,	15.0},
	{-20.0,-180.0,0.0,	15.0},

	{-20.0,-180.0,0.0,	15.0},
	{-20.0,-140.0,0.0,	15.0},
	{-10.0,-100.0,0.0,	15.0},
	{-10.0,-427.0,-35.0,	14.0},
	{-20.0,-327.0,-35.0,	14.0},
	{-60.0,-277.0,-35.0,	14.0},
	{-30.0,-197.0,-35.0,	14.0},
	{-50.0,-118.0,-35.0,	14.0},
	{-30.0,-470.0,-27.0,	13.0},
	{-60.0,-250.0,-27.0,	13.0},	//73
	{-190.0,-125.0,0.0,	2.0},	//74
	{-170.0,-5.0,0.0,	2.0},
	{-170.0,130.0,0.0,	2.0},
	{-170.0,70.0,0.0,	2.0},
	{-97.0,-97.0,0.0,	3.0},
	{30.0,-287.0,0.0,	4.0},
	{30.0,-180.0,0.0,	5.0},	//80
	{10.0,-250.0,0.0,	5.0},
	{53.0,-40.0,-57.0,	6.0},
	{43.0,-80.0,-57.0,	6.0},
	{53.0,-140.0,-57.0,	6.0},
	{23.0,-150.0,-57.0,	6.0},
	{33.0,-170.0,-57.0,	6.0},
	{23.0,-190.0,-57.0,	6.0},
	{-17.0,-150.0,-57.0,	6.0},
	{-27.0,-100.0,-57.0,	6.0},
	{-40.0,-250.0,0.0,	5.0},
	{-60.0,-190.0,0.0,	5.0},
	{-70.0,-287.0,0.0,	4.0},	//92
	{-70.0,-97.0,0.0,	4.0},
	{-50.0,-7.0,0.0,		4.0},
	{-147.0,53.0,0.0,	7.0},

	{-147.0,53.0,0.0,	7.0},
	{-90.0,290.0,0.0,	2.0},
	{-80.0,35.0,0.0,	11.0},
	{-50.0,35.0,0.0,	11.0},
	{-70.0,-5.0,0.0,	12.0},
	{-80.0,45.0,0.0,	12.0},
	{-80.0,85.0,0.0,	12.0},
	{-70.0,125.0,0.0,	12.0},
	{-40.0,155.0,0.0,	12.0},
	{0.0,165.0,0.0,		12.0},
} ;
#endif

/*----------------------------------------------------------------*/
//#define	MAKESHADOW_PRI	(1)
static void MakeShadowMatrix( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2, FMATRIX *m )
{
	FVECTOR	t1, t2, nrm, nrm2 ;

#ifdef MAKESHADOW_PRI
printf(" v0 [%1.3f][%1.3f][%1.3f][%1.3f]\n",v0->vx, v0->vy, v0->vz, v0->vw ) ;
printf(" v1 [%1.3f][%1.3f][%1.3f][%1.3f]\n",v1->vx, v1->vy, v1->vz, v1->vw ) ;
printf(" v2 [%1.3f][%1.3f][%1.3f][%1.3f]\n",v2->vx, v2->vy, v2->vz, v2->vw ) ;
#endif
	/* ３点から法線を求める */
	_sceVu0SubVector(  &t1, v0, v1 ) ;
	_sceVu0SubVector(  &t2, v0, v2 ) ;
	_sceVu0OuterProduct( &nrm, &t1, &t2 );
    _sceVu0Normalize( &nrm2, &nrm ) ;
#ifdef MAKESHADOW_PRI
printf(" t1 [%1.3f][%1.3f][%1.3f][%1.3f]\n",t1.vx, t1.vy, t1.vz, t1.vw ) ;
printf(" t2 [%1.3f][%1.3f][%1.3f][%1.3f]\n",t2.vx, t2.vy, t2.vz, t2.vw ) ;
printf(" nrm [%1.3f][%1.3f][%1.3f][%1.3f]\n",nrm.vx, nrm.vy, nrm.vz, nrm.vw ) ;
printf(" nrm2 [%1.3f][%1.3f][%1.3f][%1.3f]\n",nrm2.vx, nrm2.vy, nrm2.vz, nrm2.vw ) ;
#endif

	_sceVu0DropShadowMatrix( m, &nrm, nrm2.vx, nrm2.vy, nrm2.vz, 0/*平行光源*/ ) ;
	m->m[3][0] = 0.0 ;
	m->m[3][1] = 0.0 ;
	m->m[3][2] = 0.0 ;

	_sceVu0ApplyMatrix( &t1, m, v0 ) ;
#ifdef MAKESHADOW_PRI
printf(" [%1.3f][%1.3f][%1.3f][%1.3f]\n",m->m[0][0],m->m[0][1],m->m[0][2],m->m[0][3] ) ;
printf(" [%1.3f][%1.3f][%1.3f][%1.3f]\n",m->m[1][0],m->m[1][1],m->m[1][2],m->m[1][3] ) ;
printf(" [%1.3f][%1.3f][%1.3f][%1.3f]\n",m->m[2][0],m->m[2][1],m->m[2][2],m->m[2][3] ) ;
printf(" [%1.3f][%1.3f][%1.3f][%1.3f]\n",m->m[3][0],m->m[3][1],m->m[3][2],m->m[3][3] ) ;
printf("trans t1 [%1.3f][%1.3f][%1.3f][%1.3f]\n",t1.vx, t1.vy, t1.vz, t1.vw ) ;
#endif
	_sceVu0SubVector(  &t1, v0, &t1 ) ;
	m->m[3][0] = t1.vx ;
	m->m[3][1] = t1.vy ;
	m->m[3][2] = t1.vz ;

#ifdef MAKESHADOW_PRI
printf(" [%1.3f][%1.3f][%1.3f][%1.3f]\n",m->m[0][0],m->m[0][1],m->m[0][2],m->m[0][3] ) ;
printf(" [%1.3f][%1.3f][%1.3f][%1.3f]\n",m->m[1][0],m->m[1][1],m->m[1][2],m->m[1][3] ) ;
printf(" [%1.3f][%1.3f][%1.3f][%1.3f]\n",m->m[2][0],m->m[2][1],m->m[2][2],m->m[2][3] ) ;
printf(" [%1.3f][%1.3f][%1.3f][%1.3f]\n",m->m[3][0],m->m[3][1],m->m[3][2],m->m[3][3] ) ;
#endif
}

void	KR_GetLevelsRotFromPos( float *levels, FVECTOR *pos, HZX_GROUP_ID hzx_id, int seg_type, int flr_type, HZX_HZD *flr )
{
    int		flag ;	

    flag = HZX_LevelHazardCheck( hzx_id, pos, seg_type, flr_type ) ;
	if ( flag != 0 ) {
		HZX_GetLevelHeight( levels ) ; 
		HZX_GetLevelHazard( flr, NULL ) ;
	} else {
		flr->attribute = 0 ;
		flr->type = 0 ;
		flr++ ;
		flr->attribute = 0 ;
		flr->type = 0 ;
	}
	if ( !( 1 & flag ) ) levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	if ( !( 2 & flag ) ) levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
}


void	KR_CrossLineXY( FVECTOR *p1, FVECTOR *p2, FVECTOR *pp1, FVECTOR *pp2, FVECTOR *out_pos )
{
	float l ;
	FVECTOR	vec ;

//printf("Cross Line [%f][%f]-[%f][%f] ", p1->vx, p1->vz, p2->vx, p2->vz ) ;
//printf("* [%f][%f]-[%f][%f]\n", pp1->vx, pp1->vz, pp2->vx, pp2->vz ) ;

	l = GV_PointToVectorLen( pp1, p1, p2 ) ;
	_sceVu0SubVector(  &vec, pp2, pp1 ) ;
	vec.vy = 0 ;
    _sceVu0Normalize( &vec, &vec ) ;
    _sceVu0ScaleVector( &vec, &vec, l );
//printf("Len [%f] Vec [%f][%f] ", l, vec.vx, vec.vz ) ;
    _sceVu0AddVector( out_pos, pp1, &vec ) ;
//printf("Cross Point [%f][%f] ", out_pos->vx, out_pos->vz ) ;
}

/* XZ平面３角形面積 */
static float KR_TriangleAreaXZ( FVECTOR *p1, FVECTOR *p2, FVECTOR *p3 )
{
    return ((p2->vx - p1->vx)*(p3->vz - p1->vz) - (p2->vz - p1->vz)*(p3->vx - p1->vx))/2.0;
}

/* XZ平面多角形面積 */
static float KR_PolygonAreaXZ( FVECTOR *p, int n )
{
	FVECTOR	*p0 ;
	float s ;
	int	i ;
	
	s=0.0 ;
	p0 = p ;
    for( i=2; i<n; i++ ) {
		s += KR_TriangleAreaXZ( p0, p+1, p+2 );
	}
	return s ;
}

/*----------------------------------------------------------------*/
static void HorizonTrans( FVECTOR *p_out, SVECTOR *r_out, OBJECT *body, int num, int type )
{
	static FVECTOR	shift = {0.0, -100.0, 0.0} ;
	FVECTOR	pos2, vec ;

	KR_FMatToFvec( &BODYWORLD( body, num ), p_out ) ;
	DG_SetPos( &BODYWORLD( body, num ) ) ;
	DG_PutVector( &shift, &pos2, 1 ) ;
	_sceVu0SubVector(  &vec, &pos2, p_out ) ;
	_FVecToRotXY( &vec, r_out ) ;
	if ( type == DEAD_TYPE_UTUBUSE ) {
		r_out->vy = (r_out->vy + 2048) & 4095 ;
//		if ( 
printf("DEAD_TYPE_UTUBUSE rot x[%d] y[%d]\n", r_out->vx, r_out->vy ) ;
	} else {
		r_out->vx = (r_out->vx + 2048) & 4095 ;
printf("DEAD_TYPE_AOMUKE rot x[%d] y[%d]\n", r_out->vx, r_out->vy ) ;
	}
}

static void VerticalTrans( FVECTOR *p_out, SVECTOR *r_out, OBJECT *body, int num, int type, int dir )
{
	static FVECTOR	shift = {0.0, -100.0, 0.0} ;
	FVECTOR	pos2, vec ;

printf("obj num [%d] \n", num ) ;
	KR_FMatToFvec( &BODYWORLD( body, num ), p_out ) ;
	DG_SetPos( &BODYWORLD( body, num ) ) ;
	DG_PutVector( &shift, &pos2, 1 ) ;
	_sceVu0SubVector(  &vec, &pos2, p_out ) ;
	_FVecToRotYZ( &vec, r_out ) ;
	r_out->vy = (dir + 2048) & 4095 ;
	r_out->vz = (r_out->vz - 1024) & 4095 ;
printf("tekubi rot x[%d] y[%d] z[%d]\n", r_out->vx, r_out->vy, r_out->vz ) ;
//	r_out->vy = (r_out->vy + 2048) & 4095 ;
}

static void AddConnectPos( Work *work, FVECTOR *p )
{
	FVECTOR	*before, *center, *next ;

	before = p-1 ;
	center = p ;
	next = p+1 ;
	_sceVu0CopyVector( next, center ) ;
	
	center->vx = (next->vx + before->vx) / 2 ;
	center->vy = (next->vy < before->vy) ? next->vy : before->vy ;
	center->vz = (next->vz + before->vz) / 2 ;
printf(" connect bef [%1.3f][%1.3f][%1.3f][%1.3f]\n",center->vx, center->vy, center->vz, center->vw ) ;
	_sceVu0ApplyMatrix( center, &work->shadow_mat, center ) ;
printf(" connect aft [%1.3f][%1.3f][%1.3f][%1.3f]\n",center->vx, center->vy, center->vz, center->vw ) ;
}

static void ConnectPos( FVECTOR *center, FVECTOR *before, FVECTOR *next )
{
	_sceVu0CopyVector( next, center ) ;

	center->vx = (before->vx+next->vx) / 2 ;
	center->vy = next->vy ;
	center->vz = before->vz ;
}

static void ConnectPosRev( FVECTOR *center, FVECTOR *before, FVECTOR *next )
{
	_sceVu0CopyVector( next, center ) ;

	center->vx = (before->vx+next->vx) / 2 ;
	center->vy = before->vy ;
	center->vz = next->vz ;
}

static int ConnectPosIK( FVECTOR *now, HZX_HZD *flr1, HZX_HZD *flr2 )
{
	FVECTOR *next, *nnext, *before, cross, line[2] ;
	HZX_VEC	*p1, *p2 ;
	int i, j, n ;

	next = now+1 ;
	nnext = now+2 ;
	before = now-1 ;

	n = 0 ;
	p1 = &flr1->p1 ;
	for( i=0; i<4; i++ ) {
		p2 = &flr2->p1 ;
		for( j=0; j<4; j++ ) {
			if( (DG_FABS(p1->x - p2->x) < 1.0f) &&
				(DG_FABS(p1->z - p2->z) < 1.0f) ) {
				line[n].vx = p1->x ;
				line[n].vy = p1->y ;
				line[n].vz = p1->z ;
				if ( ++n >= 2 ) break ;
			}
			p2++ ;
		}
		if ( n >= 2 ) break ;
		p1++ ;
	}

//	ASSERT( n==2 ) ;
	if ( n!=2 ) return 0 ;

	KR_CrossLineXY( now, before, &line[0], &line[1], &cross ) ;

printf("cross [%f] [%f] [%f]\n", cross.vx, cross.vy, cross.vz ) ;
	_sceVu0CopyVector( nnext, now ) ;
//printf("nnext [%f] [%f] [%f]\n", nnext->vx, nnext->vy, nnext->vz ) ;
	_sceVu0CopyVector( now, &cross ) ;
	now->vy = before->vy ;
	_sceVu0CopyVector( next, &cross ) ;
	next->vy = nnext->vy ;

	return 1 ;
}

static	void	TransPosition( Work *work )
{
	int i, n, disp_count ;
	FVECTOR	*pera_pos, *trans_pos, pos ;
	SVECTOR rot ;
	float levels[2] ;
	HZX_HZD		flr[2] ;

	/* 初期化 */
	work->old_floor_level = -1000000.0f ;
	work->old_flr[ 0 ].attribute = 0 ;
	work->old_flr[ 1 ].attribute = 0 ;
	work->old_flr[ 0 ].type = 0 ;	/* 最初はHZX_TYPE_FLOORじゃない */
	work->old_flr[ 1 ].type = 0 ;	/* 最初はHZX_TYPE_FLOORじゃない */

	/* 内側座標 */
	pera_pos = pera ;
	trans_pos = work->trans_pos ;
	disp_count = 0 ;
	for ( i=0; i<MAX_POS; i++ ) {
		n = (int)pera_pos->vw ;
printf("num [%d] obj[%d]\n",i, n);
		if ( work->type == DEAD_TYPE_LEAN ) {
			if ( n==0 || n==1 || n==5 || n==6 || n==9 || n==10 ||
				 n==13 || n==14 || n==15 || n==16 || n==17 || n==18 || n==19 || n==20 ) {
				HorizonTrans( &pos, &rot, work->body, n, work->type ) ;
				DG_SetPos2( &pos, &rot ) ;
				DG_PutVector( pera_pos, trans_pos, 1 ) ;
				trans_pos->vy += 1000.0 ;
				KR_GetLevelsRotFromPos( levels, trans_pos, work->hzx_id, HZX_CHK_ALL, CHECK_FLOOR, flr ) ;
//				trans_pos->vy = ( (trans_pos->vy - levels[0]) < HIGHT_LEVEL ) ? levels[0] : levels[1] ;
				trans_pos->vy = ( (trans_pos->vy - levels[0]) < HIGHT_LEVEL ) ? levels[0] : trans_pos->vy-1000.0 ;
			} else {
				FVECTOR p, vec ;
				
//				VerticalTrans( &pos, &rot, work->body, n, work->type, work->lean_dir ) ;
//				DG_SetPos2( &pos, &rot ) ;

				DG_SetPos( &BODYWORLD( work->body, n ) ) ;
				DG_PutVector( pera_pos, trans_pos, 1 ) ;
				p.vx = trans_pos->vx + (1000.0 * _RsinF( work->lean_dir )) ;
				p.vy = trans_pos->vy ;
				p.vz = trans_pos->vz + (1000.0 * _RcosF( work->lean_dir )) ;
				if ( HZX_OnlineHazardCheck( work->hzx_id, trans_pos, &p,
					HZX_CHK_ALL,HZX_TYPE_ENEMY,CHECK_FLOOR ) ) {
				    HZX_GetOnlineVector( &vec ) ;
				    _sceVu0AddVector( trans_pos, trans_pos, &vec ) ;
				} else {
					HorizonTrans( &pos, &rot, work->body, n, work->type ) ;
					DG_SetPos2( &pos, &rot ) ;
					DG_PutVector( pera_pos, trans_pos, 1 ) ;
					trans_pos->vy += 1000.0 ;
					KR_GetLevelsRotFromPos( levels, trans_pos, work->hzx_id, HZX_CHK_ALL, CHECK_FLOOR, flr ) ;
//					trans_pos->vy = ( (trans_pos->vy - levels[0]) < HIGHT_LEVEL ) ? levels[0] : levels[1] ;
					trans_pos->vy = ( (trans_pos->vy - levels[0]) < HIGHT_LEVEL ) ? levels[0] : trans_pos->vy-1000.0 ;
				}
			}
			if ( i==13 || i==31 || i==80 ) {
				ConnectPos( trans_pos, trans_pos-1, trans_pos+1 ) ;
				work->max_disp_pos++ ;
				trans_pos++ ;
				disp_count++ ;
			}
			if ( i==25 || i==74 || i==92 ) {
				ConnectPosRev( trans_pos, trans_pos-1, trans_pos+1 ) ;
				work->max_disp_pos++ ;
				trans_pos++ ;
				disp_count++ ;
			}
		} else {
			if ( (work->status & DL_STATUS_FACE_LEAN) &&
				 ((n==12) || (n==11)) ) {
				DG_SetPos( &BODYWORLD( work->body, n ) ) ;
				DG_PutVector( pera_pos, trans_pos, 1 ) ;
//printf(" befire [%1.3f][%1.3f][%1.3f][%1.3f]\n",trans_pos->vx, trans_pos->vy, trans_pos->vz, trans_pos->vw ) ;
			    _sceVu0ApplyMatrix( trans_pos, &work->shadow_mat, trans_pos ) ;
//printf(" after [%1.3f][%1.3f][%1.3f][%1.3f]\n",trans_pos->vx, trans_pos->vy, trans_pos->vz, trans_pos->vw ) ;
			} else {
				HorizonTrans( &pos, &rot, work->body, n, work->type ) ;
				DG_SetPos2( &pos, &rot ) ;
				DG_PutVector( pera_pos, trans_pos, 1 ) ;
				trans_pos->vy += 1000.0 ;
				KR_GetLevelsRotFromPos( levels, trans_pos, work->hzx_id, HZX_CHK_ALL, CHECK_FLOOR, flr ) ;
	//			trans_pos->vy = ( (trans_pos->vy - levels[0]) < HIGHT_LEVEL ) ? levels[0] : levels[1] ;
				trans_pos->vy = ( (trans_pos->vy - levels[0]) < HIGHT_LEVEL ) ? levels[0] : trans_pos->vy-1000.0 ;
printf("HZX_FLOOR vy[%f] level[%f]\n",trans_pos->vy, levels[0]);

				if ( (flr[0].attribute & HZX_FLOOR_IK) &&
					 (work->old_flr[0].type == HZX_TYPE_FLOOR) &&
					 (DG_FABS(work->old_floor_level - levels[0]) > 5.0f) ) {
					if ( ConnectPosIK( trans_pos, work->old_flr, flr ) ) {
						work->max_disp_pos+=2 ;
						trans_pos+=2 ;
						disp_count+=2 ;
					}
				}
			}
			if ( (work->status & DL_STATUS_FACE_LEAN) &&
				 ((i==8) || (i==98)) ) {
					AddConnectPos( work, trans_pos ) ;
					work->max_disp_pos++ ;
					trans_pos++ ;
					disp_count++ ;
			}
		}
		if ( !((++disp_count)%64) ) {
			_sceVu0CopyVector( trans_pos+1, trans_pos ) ;
			work->max_disp_pos++ ;
			trans_pos++ ;
			disp_count++ ;
		}

		work->old_flr[0] = flr[0] ;
		work->old_flr[1] = flr[1] ;
		work->old_floor_level = levels[0] ;
		
		pera_pos++ ;
		trans_pos++ ;

printf("trans pos[%d] disp count[%d] max disp[%d]\n",i,disp_count,work->max_disp_pos) ;
	}

	trans_pos = work->effect_prim->pos[0] ;
	KR_FMatToFvec( &BODYWORLD( work->body, 0 ), trans_pos ) ;
	trans_pos->vy += 10000.0f ;
}

static void WidthPos( FVECTOR *out, FVECTOR *before_pos, FVECTOR *center_pos, FVECTOR *next_pos )
{
	int	bef_dir, next_dir, wid_dir ;

	bef_dir = _FVecTrgDir2( center_pos, before_pos ) ;
	next_dir = _FVecTrgDir2( center_pos, next_pos ) ;
	if ( bef_dir < next_dir ) bef_dir += 4096 ;
	wid_dir = (bef_dir+next_dir)/2 ;
	wid_dir &= 4095 ;

	out->vx = center_pos->vx + (LINE_WIDTH * _RsinF( wid_dir )) ;
	out->vy = center_pos->vy ;
	out->vz = center_pos->vz + (LINE_WIDTH * _RcosF( wid_dir )) ;
//printf(" bef_dir[%d] next_dir[%d] wid_dir[%d]\n",bef_dir, next_dir, wid_dir ) ;
//printf(" out->vz[%f] center_pos->vz[%f]\n",out->vz, center_pos->vz ) ;
}

static	void	SetWidthPos( FVECTOR *out, FVECTOR *trans_pos, int disp_count )
{
	FVECTOR	*before_pos, *next_pos ;
	int	i, tmp ;

	tmp = disp_count ;
	before_pos = &trans_pos[ tmp ] ;
	for( i=0; i<4; i++ ) {
		if ( --tmp < 0 ) tmp = MAX_POS -1 ;
		before_pos = &trans_pos[ tmp ] ;
		if ( !_PosInRangeXZ( before_pos, &trans_pos[disp_count], 8 ) ) break ;
	}

	tmp = disp_count ;
	next_pos = &trans_pos[ tmp ] ;
	for( i=0; i<4; i++ ) {
		if ( ++tmp >= MAX_POS ) tmp = 0 ;
		next_pos = &trans_pos[ tmp ] ;
		if ( !_PosInRangeXZ( next_pos, &trans_pos[disp_count], 8 ) ) break ;
	}

	WidthPos( out, before_pos, &trans_pos[disp_count], next_pos ) ;
}
static	void	SetPosStrip( Work *work, int disp_count )
{
	FVECTOR	*prim_pos, pos[2] ;

	prim_pos = work->prim_strip->pos[0] ;
#ifdef PRIM_LINE
	prim_pos += disp_count ;
	memcpy( prim_pos, &work->trans_pos[disp_count], sizeof(FVECTOR) * 1 ) ;
	prim_pos = work->prim_strip->pos[1] ;
	prim_pos += disp_count ;
	memcpy( prim_pos, &work->trans_pos[disp_count], sizeof(FVECTOR) * 1 ) ;
#else
	pos[0] = work->trans_pos[disp_count] ;
	SetWidthPos( &pos[1], work->trans_pos, disp_count ) ;

	prim_pos += disp_count*2 ;
	memcpy( prim_pos, &pos[0], sizeof(FVECTOR) * 2 ) ;
	prim_pos = work->prim_strip->pos[1] ;
	prim_pos += disp_count*2 ;
	memcpy( prim_pos, &pos[0], sizeof(FVECTOR) * 2 ) ;
#endif
}

static void SetDispFlag( DG_PRIM2 *prim, int count )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];

printf("disp flag count [%d] \n",count) ;

#ifdef PRIM_LINE
	uvrgb0 += count ;
	uvrgb1 += count ;
	uvrgb1->f = uvrgb0->f = 0x0fff ;
#else
	uvrgb0 += count*2 ;
	uvrgb1 += count*2 ;
	uvrgb1->f = uvrgb0->f = 0x0fff ;
	uvrgb0 ++ ;
	uvrgb1 ++ ;
	uvrgb1->f = uvrgb0->f = 0x0fff ;
#endif
}

static	void	SetPosEffect( Work *work, int disp_count )
{
	FVECTOR	*prim_pos, pos[2], center ;

printf("count [%d] \n",disp_count) ;

	prim_pos = work->effect_prim->pos[0] ;
	prim_pos++ ;
	memcpy( prim_pos, &work->trans_pos[disp_count], sizeof(FVECTOR) * 1 ) ;

#if 0
	memcpy( prim_pos, &work->trans_pos[disp_count], sizeof(FVECTOR) * 1 ) ;
	prim_pos->vy += 5000.0f ;
#endif
	memcpy( work->effect_prim->pos[1], work->effect_prim->pos[0], sizeof(FVECTOR) * 2 ) ;
}

static void SetDispLine( DG_PRIM2 *prim )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	uvrgb1->f = uvrgb0->f = 0x0fff ;
	uvrgb0 ++ ;
	uvrgb1 ++ ;
	uvrgb1->f = uvrgb0->f = 0x0fff ;
}

static void UnSetDispLine( DG_PRIM2 *prim )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	uvrgb1->f = uvrgb0->f = 0x8fff ;
	uvrgb0 ++ ;
	uvrgb1 ++ ;
	uvrgb1->f = uvrgb0->f = 0x8fff ;
}

static	void	DisplayLine( Work *work )
{
	DG_SwitchBuffPrim2( work->prim_strip );

	work->disp_count ++ ;
	if ( work->disp_count < work->max_disp_pos ) {
		SetPosStrip( work, work->disp_count ) ;
		SetDispFlag( work->prim_strip, work->disp_count ) ;

		SetPosEffect( work, work->disp_count ) ;
		SetDispLine( work->effect_prim ) ;
	} else {
		work->disp_count = work->max_disp_pos ;
		UnSetDispLine( work->effect_prim ) ;
	}
}

static	void	Act( Work *work )
{
	DisplayLine( work ) ;
}

static	void	Die( Work *work )
{
	GM_FreePrim2( work->prim_strip );
 	GM_FreePrim2( work->effect_prim );
}

/*----------------------------------------------------------------*/
static void InitPacketStrip( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	FVECTOR	*prim_pos ;
	int		j, k ;
	int		count ;

	prim->raise = RAISE;

	prim_pos = prim->pos[0] ;
	for ( k = 0 ; k < N_LINES_STRIP*N_VERTS_STRIP ; k++ ) {
		_sceVu0CopyVector( prim_pos++, &DG_ZeroVector ) ;
	}
	prim_pos = prim->pos[1] ;
	for ( k = 0 ; k < N_LINES_STRIP*N_VERTS_STRIP ; k++ ) {
		_sceVu0CopyVector( prim_pos++, &DG_ZeroVector ) ;
	}

#ifdef PRIM_LINE
	count = 0;
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	for ( k = 0 ; k < N_LINES_STRIP ; k++ ){
		for ( j = 0 ; j < N_VERTS_STRIP ; j++ ){
			uvrgb1->u = uvrgb0->u = 1 ;
			uvrgb1->v = uvrgb0->v = 1 ;
			uvrgb1->q = uvrgb0->q = 1 ;
			if ( count < MAX_POS ) {
//				uvrgb1->f = uvrgb0->f = (count<1)? 0x8fff: 0x0fff;
				uvrgb1->f = uvrgb0->f = (count<1)? 0x8fff: 0x8fff;
			} else {
				uvrgb1->f = uvrgb0->f = 0x8fff ;
			}
			uvrgb1->r = uvrgb0->r = COL_R ;
			uvrgb1->g = uvrgb0->g = COL_G ;
			uvrgb1->b = uvrgb0->b = COL_B ;
			uvrgb1->a = uvrgb0->a = COL_A;
			uvrgb0++;
			uvrgb1++;
			count++;
		}
	}

#else
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	count = 0;
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	for ( k = 0 ; k < N_PRIMS_STRIP ; k++ ){
		for ( j = 0 ; j < N_VERTS_STRIP ; j++ ){
			uvrgb1->u = uvrgb0->u = (count&1)?FTOI12( 1.0f*tex->u_scale + tex->u_offset ):FTOI12( tex->u_offset );
			uvrgb1->v = uvrgb0->v = (count&2)?
				FTOI12( 0.6f*tex->v_scale + tex->v_offset ):FTOI12( 0.5f*tex->v_scale + tex->v_offset );
			uvrgb1->q = uvrgb0->q = 4096 ;
#if 0
			uvrgb1->f = uvrgb0->f = 0x8fff ;	/* 最初は非表示*/
#else
			if ( count < N_VERTS ) {
				uvrgb1->f = uvrgb0->f = (count<2)? 0x8fff: 0x0fff;
			} else {
				uvrgb1->f = uvrgb0->f = 0x8fff ;
			}
#endif
			uvrgb1->r = uvrgb0->r = COL_R ;
			uvrgb1->g = uvrgb0->g = COL_G ;
			uvrgb1->b = uvrgb0->b = COL_B ;
			uvrgb1->a = uvrgb0->a = COL_A;
			uvrgb0++;
			uvrgb1++;
			count++;
		}
	}
#endif
}

static void InitPacketEffect( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	FVECTOR	*prim_pos ;
	int		j, k ;
	int		count ;

	prim->raise = RAISE;

	prim_pos = prim->pos[0] ;
	for ( k = 0 ; k < N_EFFECT_PRIMS*N_EFFECT_VERTS ; k++ ) {
		_sceVu0CopyVector( prim_pos++, &DG_ZeroVector ) ;
	}
	prim_pos = prim->pos[1] ;
	for ( k = 0 ; k < N_EFFECT_PRIMS*N_EFFECT_VERTS ; k++ ) {
		_sceVu0CopyVector( prim_pos++, &DG_ZeroVector ) ;
	}

	count = 0;
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	for ( k = 0 ; k < N_EFFECT_PRIMS ; k++ ){
		for ( j = 0 ; j < N_EFFECT_VERTS ; j++ ){
			uvrgb1->u = uvrgb0->u = 1 ;
			uvrgb1->v = uvrgb0->v = 1 ;
			uvrgb1->q = uvrgb0->q = 1 ;
			uvrgb1->f = uvrgb0->f = 0x8fff;	/* 最初は非表示 */
			uvrgb1->r = uvrgb0->r = COL_R ;
			uvrgb1->g = uvrgb0->g = COL_G ;
			uvrgb1->b = uvrgb0->b = COL_B ;
			uvrgb1->a = uvrgb0->a = COL_A;
			uvrgb0++;
			uvrgb1++;
			count++;
		}
	}
}

static void CheckSeg( Work *work, CONTROL *ctrl )
{
	static FVECTOR	hor_shift = {0.0, 0.0, 100.0} ;
	FVECTOR p1, p2, v0, v1, v2, pos, p_out ;
	HZX_HZD seg ;
	float	hor_check ;

	KR_FMatToFvec( &BODYWORLD( work->body, 0 ), &p1 ) ;

	p2.vx = p1.vx + (2000.0 * _RsinF( work->lean_dir )) ;
	p2.vy = p1.vy ;
	p2.vz = p1.vz + (2000.0 * _RcosF( work->lean_dir )) ;
	if ( HZX_OnlineHazardCheck( work->hzx_id, &p1, &p2,
		HZX_CHK_ALL,HZX_TYPE_ENEMY,CHECK_FLOOR ) ) {
		HZX_GetOnlineHazard2( &seg );
		if ( seg.type == HZX_TYPE_SEGMENT ) {
			v0.vx = seg.p1.x ; v0.vy = seg.p1.y ; v0.vz = seg.p1.z ;
			v1.vx = seg.p2.x ; v1.vy = seg.p2.y ; v1.vz = seg.p2.z ;
			v2.vx = seg.p1.x ; v2.vy = seg.p1.y + seg.p1.h ; v2.vz = seg.p1.z ;
			MakeShadowMatrix( &v0, &v1, &v2, &work->shadow_mat ) ;
			SET_FLAG( work->status, DL_STATUS_NEAR_SEG ) ;
		}
		KR_FMatToFvec( &BODYWORLD( work->body, 12 ), &p_out ) ;
		DG_SetPos( &BODYWORLD( work->body, 12 ) ) ;
		DG_PutVector( &hor_shift, &pos, 1 ) ;
		hor_check = p_out.vy - pos.vy ;
		if ( work->type == DEAD_TYPE_UTUBUSE ) hor_check *= -1 ;
		if ( hor_check > 0) SET_FLAG(work->status, DL_STATUS_FACE_LEAN) ;
	}
}

static	int	GetResources( Work *work, int model, OBJECT *body, CONTROL *ctrl, int type )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->body = body ;
	work->hzx_id = ctrl->hzx_id ;
	work->type = type ;

	work->lean_dir = (type == DEAD_TYPE_UTUBUSE)? (ctrl->rot.vy)&4095:(ctrl->rot.vy+2048)&4095 ;
	work->max_disp_pos = MAX_POS ;
	work->status = 0 ;

	CheckSeg( work, ctrl ) ;

printf("drow enemy dead line type [%d] \n",type) ;

	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );

#ifdef PRIM_LINE
	prim = work->prim_strip = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING, N_LINES_STRIP, N_VERTS_STRIP );
#else
	prim = work->prim_strip = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_STRIP, N_VERTS_STRIP );
#endif

	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacketStrip( work, prim, tex );
	GM_GroupPrim2( prim, work->body->map_name ) ;

	prim = work->effect_prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING, N_EFFECT_PRIMS, N_EFFECT_VERTS );
	InitPacketEffect( work, prim, tex );
	GM_GroupPrim2( prim, work->body->map_name ) ;

	TransPosition( work ) ;
	work->disp_count = 0 ;
	SetPosStrip( work, work->disp_count ) ;


{
	extern void *NewSigBreakBody(OBJECT *,int ,float ,CVECTOR *) ;
	CVECTOR col ;
	col.r = 127 ;col.g = 127; col.b = 127 ;
	NewSigBreakBody( body, model, 6.0f, &col) ;
}

	return 0 ;
}

void	*NewDeadLine( int model, OBJECT *body, CONTROL *ctrl, int type )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, model, body, ctrl, type ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
