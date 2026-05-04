/*

  POLYFUNC集

  T.Morita  2002.11.07

  $Id: geo_poly.c,v 1.12 2002/12/11 02:24:26 usr04098 Exp $

 */

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<sys/types.h> 
#include	<math.h>

#include	"scn2geo.h"
#include	"Scene2HZX.h"







/*--------------------------------------------------------------------------

  便利なサブルーチン
  static only!!!!

 --------------------------------------------------------------------------*/
static inline void cp_p3dxyz_to_fv( FVECTOR *dst, P3DXYZ *src )
{
    dst->vx = src->x ;
    dst->vy = src->y ;
    dst->vz = src->z ;
}

static int get_vertex( LPP3DPOLYGON poly, LPP3DVERTEX vert, FVECTOR *verts )
{
    int  i, n_verts ;
    LPP3DPOLYGONNODE node;
    P3DXYZ v[20] ;

    /* ２頂点以下は不可 */
    if( poly->nPolygonNodeCount < 3 || poly->nPolygonNodeCount > 10 ) {
		printf( "Error! n_vertex %d\n", poly->nPolygonNodeCount ) ;
		return -1 ;
    }

    /* 確保した領域に頂点データを格納する */
    node = poly->lpP3DPolygonNode ;
    for( i=poly->nPolygonNodeCount ; --i>=0 ; ) {
		v[i] = (vert + node->nVertex)->p3dXYZCalc ;
		verts[i].vx = (vert + node->nVertex)->p3dXYZCalc.x ;
		verts[i].vy = (vert + node->nVertex)->p3dXYZCalc.y ;
		verts[i].vz = (vert + node->nVertex)->p3dXYZCalc.z ;
		verts[i].vw = 1.0f ;
		node++ ;
    }
    verts[poly->nPolygonNodeCount] = verts[0] ;


    /* 同一直線上に３つ以上ならぶ頂点があったなら削除する */
    if ( (n_verts = OESDeleteOverVertexOnLine( v,
											   poly->nPolygonNodeCount,
											   0.01 )) < 0 ) {
		return 0 ;
    }

    /* ５頂点以上は不可 */
    if ( n_verts > 4 ) {
		printf( "Error! Found polygon>4 vertex nverts(%d)\n", n_verts );
		for( i=0 ; i<n_verts ; i++ ){
			printf("Vertex%d = (%f, %f, %f)\n", i, verts[i].vx,verts[i].vy,verts[i].vz ) ;
		}
		return -1 ;
    }
    return n_verts ;
}

static int vertex_inbound( FVECTOR *verts, int n_verts, POLYDATA *spec )
{
    int i ;
    FVECTOR min, max ;

    MUL_FVECTOR( &min, &spec->div, &spec->pos ) ;
    ADD_FVECTOR( &min, &min, &spec->bmin ) ;
    ADD_FVECTOR( &max, &min, &spec->div  ) ;
    for ( i=n_verts ; --i>=0 ; ){
#if 0
		printf( 
			"(%f %f %f) min(%f %f %f) max(%f %f %f) %d %d %d %d %d %d\n",
			verts[i].vx,verts[i].vy,verts[i].vz,
			min.vx ,min.vy ,min.vz ,max.vx ,max.vy ,max.vz  ,
			verts[i].vx > min.vx , verts[i].vx < max.vx ,
			verts[i].vy > min.vy , verts[i].vy < max.vy ,
			verts[i].vz > min.vz , verts[i].vz < max.vz  ) ;
#endif
		if ( verts[i].vx > min.vx && verts[i].vx < max.vx &&
			 verts[i].vy > min.vy && verts[i].vy < max.vy &&
			 verts[i].vz > min.vz && verts[i].vz < max.vz  ) return 1 ;
    }

    return 0 ;
}


static int set_prim( int type, FVECTOR *verts, int n_verts, int attribute,
					 POLYDATA *spec, char *name )
{
    if ( !type ){
		type = n_verts + GEO_PRIM_TYPE_MAX ;
    }

    switch( type ) {
    case GEO_F_PRIM_DOT :
    case GEO_PRIM_TYPE_MAX+1:
		core_prim_set_dot( GEO_F_NONE, attribute, name, NULL, &verts[0] ) ;
		break ;
    case GEO_F_PRIM_LINE:
    case GEO_PRIM_TYPE_MAX+2:
		core_prim_set_line( GEO_F_NONE, attribute, name, NULL, &verts[0], &verts[1] ) ;
		break ;
    case GEO_F_PRIM_POLY:
    case GEO_PRIM_TYPE_MAX+3:
	{
		FVECTOR base ;

		MUL_FVECTOR( &base, &spec->div, &spec->pos ) ;
		ADD_FVECTOR( &base, &base, &spec->bmin ) ;

		core_prim_set_poly( GEO_F_NONE, attribute, name,
							&base,
							&verts[0], &verts[1], &verts[2], &verts[3] ) ;
		break ;
	}
    case GEO_F_PRIM_BOX :
    {
		FVECTOR size, cen ;

		ADD_FVECTOR( &cen , &verts[0], &verts[1] ) ;
		SCALE_FVECTOR( &cen , &cen , 0.5f ) ;
		SUB_FVECTOR( &size, &verts[0], &verts[1] ) ;
		SCALE_FVECTOR( &size, &size, 0.5f ) ;
       	core_prim_set_box( GEO_F_NONE, attribute, name, &size, &cen ) ;
		break ;
    }
    case GEO_F_PRIM_FIELD:
    {
		FVECTOR size, cen ;

		ADD_FVECTOR( &cen , &verts[1], &verts[0] ) ;
		SCALE_FVECTOR( &cen , &cen , 0.5f ) ;
		SUB_FVECTOR( &size, &verts[1], &verts[0] ) ;
		SCALE_FVECTOR( &size, &size, 0.5f ) ;
       	core_prim_set_field( GEO_F_NONE, attribute, name, &size, &cen ) ;
		break ;
    }

    default:
		break ;
    }
}



/*--------------------------------------------------------------------------

  BOUNDチェックルーチン

 --------------------------------------------------------------------------*/
static inline float GEO_BackFaceCulling( FVECTOR *n,
										 FVECTOR *v1, FVECTOR *v2,FVECTOR *v3 )
{
	FVECTOR a0, a1 ;

	SUB_FVECTOR( &a0, v2, v1 ) ;
	SUB_FVECTOR( &a1, v3, v1 ) ;
	OUTER_PRODUCT( &a0, &a0, &a1 ) ;
	return INNER_PRODUCT( &a0, n ) ;
}

static inline int GEO_CheckPointInPoly( FVECTOR *lpos, FVECTOR *poly, FVECTOR *norm )
{
	union {
		float f ;
		int	  i ;
	} a,b,c ;

	/* 交点が3頂点の内側にあるかどうかを判定する */
	a.f = GEO_BackFaceCulling( norm, poly+0, poly+1, lpos ) ;
	b.f = GEO_BackFaceCulling( norm, poly+1, poly+2, lpos ) ;
	c.f = GEO_BackFaceCulling( norm, poly+2, poly+0, lpos ) ;
	if ( !((a.i | b.i | c.i) & 0x80000000) || 
		  ((a.i & b.i & c.i) & 0x80000000) ) return 1 ;
	return	0 ;
}

static int GEO_CheckLineInPoly( FVECTOR *lpos, FVECTOR *ldir,
								FVECTOR *poly, FVECTOR *norm )
{
	float	  l, m ;
	FVECTOR	  v ;
	
	SUB_FVECTOR( &v, lpos, poly ) ;
	l = INNER_PRODUCT( norm, &v	) ;
	m = INNER_PRODUCT( norm, ldir ) ;
	if ( m == 0.0f ) return 0 ;
	m = -l/m ;
	if ( m>0.0f && m<1.0f ){
		SCALE_FVECTOR( &v, ldir, m ) ;
		ADD_FVECTOR( &v, &v, lpos ) ;
		return GEO_CheckPointInPoly( &v, poly, norm ) ;
	}
	return 0 ;
}

/* 点a 点b ともにsize以内離れているかどうか */
static inline int GEO_CheckPointInBall( FVECTOR *a, FVECTOR *b, float size )
{
	FVECTOR	 v ;
	
	SUB_FVECTOR( &v, a, b ) ;
	if ( INNER_PRODUCT( &v, &v ) < size*size ) {
		return 1 ;
	}
	return 0 ;
}

/* 点v が 点c1と点c2 を直径とする円に入っているかどうか */
static inline int GEO_CheckCornerInBall( FVECTOR *v, FVECTOR *c1, FVECTOR *c2 )
{
	FVECTOR s, t ;

	ADD_FVECTOR( &t, c2, c1 ) ;
	SCALE_FVECTOR( &t, &t, 0.5f ) ;
	SUB_FVECTOR( &s, c1, &t ) ;
	SUB_FVECTOR( &t,  v, &t ) ;
	if ( INNER_PRODUCT( &t, &t ) < INNER_PRODUCT( &s, &s ) ) {
		return 1 ;
	}
	return 0 ;
}

/* ポリゴンを太らす作業 */
static inline void GEO_GetPosPolyScaling( FVECTOR *out, FVECTOR *in,
										  FVECTOR *center, float size )
{
	int		  i ;

	size = size/center->vw + 1.0f ;
	for( i=3 ; --i>=0 ;	 ){
		SUB_FVECTOR( out, in, center ) ;
		SCALE_FVECTOR( out, out, size ) ;
		ADD_FVECTOR( out, out, center ) ;
		in++ ;
		out++ ;
	}
}

static inline int GEO_CheckPointPolyCorner( FVECTOR *pos, FVECTOR *fat, FVECTOR *org, float size )
{
	/* とりあえず三角形に入った 各頂点の端チェック */
	if ( GEO_CheckCornerInBall( pos, org+0, fat+0 ) ){
		return GEO_CheckPointInBall( pos, org+0, size ) ;
	}
	if ( GEO_CheckCornerInBall( pos, org+1, fat+1 ) ){
		return GEO_CheckPointInBall( pos, org+1, size ) ;
	}
	if ( GEO_CheckCornerInBall( pos, org+2, fat+2 ) ){
		return GEO_CheckPointInBall( pos, org+2, size ) ;
	}
	/* 中の方 */
	return 1 ;
}

static inline int GEO_CheckRegion( FVECTOR *pos, FVECTOR *size )
{
	int ret = 0 ;

	if ( pos->vx <-size->vx ) ret |= 0x0001 ;/*CLIP命令と同じ*/
	if ( pos->vx > size->vx ) ret |= 0x0002 ;
	if ( pos->vy <-size->vy ) ret |= 0x0010 ;
	if ( pos->vy > size->vy ) ret |= 0x0020 ;
	if ( pos->vz <-size->vz ) ret |= 0x0100 ;
	if ( pos->vz > size->vz ) ret |= 0x0200 ;
	return ret ;
}

static int GEO_CheckPointInRegion( FVECTOR *size, FVECTOR *pos,
								   FVECTOR *lsize )
{
	if ( lsize ) {
		FVECTOR s = { size->vx + lsize->vx,
					  size->vy + lsize->vx,
					  size->vz + lsize->vx } ;
		return GEO_CheckRegion( pos, &s ) ;
	}
	return GEO_CheckRegion( pos, size ) ;
}

static int GEO_CheckLineInRegion( FVECTOR *size,
								  FVECTOR *ldir, FVECTOR *lpos,
								  FVECTOR *lsize )
{
	int     ret ;
	float   t, dist = 1.0f ;
	FVECTOR v, s ;

	ret = 0 ;

	s.vx = size->vx + lsize->vx ;
	s.vy = size->vy + lsize->vx ;
	s.vz = size->vz + lsize->vx ;
	v.vx = (lpos->vx>0.0f ? s.vx : -s.vx) - lpos->vx ;
	v.vy = (lpos->vy>0.0f ? s.vy : -s.vy) - lpos->vy ;
	v.vz = (lpos->vz>0.0f ? s.vz : -s.vz) - lpos->vz ;
	t = v.vx / ldir->vx ;
	if ( t>0.0 && dist > t ) {
		if ( fabs(lpos->vy + ldir->vy*t) < s.vy &&
			 fabs(lpos->vz + ldir->vz*t) < s.vz ) {
			dist = t ;
			ret = ldir->vx>0.0f ? 0x0001 : 0x0002 ;
		}
	}
	t = v.vy / ldir->vy ;
	if ( t>0.0 && dist > t ) {
		if ( fabs(lpos->vx + ldir->vx*t) < s.vx &&
			 fabs(lpos->vz + ldir->vz*t) < s.vz ) {
			dist = t ;
			ret = ldir->vx>0.0f ? 0x0010 : 0x0020 ;
		}
	}
	t = v.vz / ldir->vz ;
	if ( t>0.0 && dist > t ) {
		if ( fabs(lpos->vx + ldir->vx*t) < s.vx &&
			 fabs(lpos->vy + ldir->vy*t) < s.vy ) {
			dist = t ;
			ret = ldir->vx>0.0f ? 0x0100 : 0x0200 ;
		}
	}

	return ret ;
}




static void mk_pos_size( FVECTOR *pos, FVECTOR *size,
						 FVECTOR *min, FVECTOR *max )
{
	ADD_FVECTOR( pos , min, max ) ;
	SUB_FVECTOR( size, min, max ) ;
	SCALE_FVECTOR( pos , pos , 0.5f ) ;
	SCALE_FVECTOR( size, size, 0.5f ) ;
	size->vx = fabs( size->vx ) ;
	size->vy = fabs( size->vy ) ;
	size->vz = fabs( size->vz ) ;
}

static void mk_bound_pos_size( FVECTOR *pos, FVECTOR *size, POLYDATA *spec )
{
	FVECTOR min ;
	FVECTOR max ;

    MUL_FVECTOR( &min, &spec->div, &spec->pos ) ;
    ADD_FVECTOR( &min, &min, &spec->bmin ) ;
    ADD_FVECTOR( &max, &min, &spec->div  ) ;
	mk_pos_size( pos, size, &max, &min ) ;
}

static int in_bound_dot( POLYDATA *spec, FVECTOR *pos, float size )
{
	FVECTOR	 apos, asize ;
	FVECTOR	 fpos, fsize ;

	mk_bound_pos_size( &fpos, &fsize, spec ) ;

	apos.vx = fabs( pos->vx - fpos.vx ) ;
	apos.vy = fabs( pos->vy - fpos.vy ) ;
	apos.vz = fabs( pos->vz - fpos.vz ) ;
	asize.vx = fsize.vx + size ;
	asize.vy = fsize.vy + size ;
	asize.vz = fsize.vz + size ;
	if ( apos.vx < asize.vx && apos.vy < asize.vy && apos.vz < asize.vz ) {
		if ( GEO_CheckPointInBall( &apos, pos, size ) ){
			if ( GEO_CheckPointInBall( &apos, &fsize, size ) ){
				return 0 ;
			}
			/* ここに来るということは,角に当たっていることになる */
		}
		return 1 ;
	}
	return 0 ;
}

static int in_bound_line( POLYDATA *spec, FVECTOR *from, FVECTOR *to )
{
	FVECTOR apos, adir, asize ;
	FVECTOR	 fpos, fsize ;

	mk_bound_pos_size( &fpos, &fsize, spec ) ;

	SUB_FVECTOR( &apos, from, &fpos ) ;
	SUB_FVECTOR( &adir, to  , from ) ;
	asize.vx = 1.0 ;

	return GEO_CheckLineInRegion( &fsize, &adir, &apos, &asize ) ;
}

static inline int GEO_CheckLineThroughPlane( FVECTOR *norm, FVECTOR *poly, FVECTOR *diagonal  )
{
	float   a , b  ;
	FVECTOR va, vb ;

#if 0
	SUB_FVECTOR( &va, poly, diagonal ) ;
	ADD_FVECTOR( &vb, poly, diagonal ) ;
#else
	SUB_FVECTOR( &va, diagonal, poly ) ;
	SCALE_FVECTOR( &vb, diagonal, -1.0f ) ;
	SUB_FVECTOR( &vb, &vb, poly ) ;
#endif
	a = INNER_PRODUCT( norm, &va ) ;
	b = INNER_PRODUCT( norm, &vb ) ;

	if ( a*b < 0.0f ) {
		return 1 ;
	}
	return 0 ;
}

int GEO_CheckPlaneInRegion( FVECTOR *norm, FVECTOR *poly, FVECTOR *fsize )
{
	FVECTOR v = { fsize->vx, fsize->vy, fsize->vz } ;

	if ( GEO_CheckLineThroughPlane( norm, poly, &v ) ) return 1 ;
	v.vz = -fsize->vz ;
	if ( GEO_CheckLineThroughPlane( norm, poly, &v ) ) return 1 ;
	v.vx = -fsize->vx, v.vz =  fsize->vz ;
	if ( GEO_CheckLineThroughPlane( norm, poly, &v ) ) return 1 ;
	v.vz = -fsize->vz ;
	if ( GEO_CheckLineThroughPlane( norm, poly, &v ) ) return 1 ;

	return 0 ;
}

static int in_bound_poly( POLYDATA *spec, FVECTOR *pos )
{
	FVECTOR  dir ;
	FVECTOR  poly[3] ;
	FVECTOR	 fpos, fsize ;
	FVECTOR  zero = { 0.0f, 0.0f, 0.0f, 1.0f } ;
	int p0, p1, p2 ;

	void core_prim_calc_normal( FVECTOR *n,
							FVECTOR *p0, FVECTOR *p1, FVECTOR *p2 ) ;

	mk_bound_pos_size( &fpos, &fsize, spec ) ;

	SUB_FVECTOR( &poly[0], pos+0, &fpos ) ;
	SUB_FVECTOR( &poly[1], pos+1, &fpos ) ;
	SUB_FVECTOR( &poly[2], pos+2, &fpos ) ;

	/* 頂点がなかに入っているか */
	p0 = GEO_CheckRegion( &poly[0], &fsize ) ;
	if ( p0 == 0 ) return 1 ;
	p1 = GEO_CheckRegion( &poly[1], &fsize ) ;
	if ( p1 == 0 ) return 1 ;
	p2 = GEO_CheckRegion( &poly[2], &fsize ) ;
	if ( p2 == 0 ) return 1 ;

	/* １辺が突き抜けるかどうかをチェックする */
	SUB_FVECTOR( &dir, &poly[1], &poly[0] ) ;
	if ( GEO_CheckLineInRegion( &fsize, &dir, &poly[0], &zero ) ) return 1 ;
	SUB_FVECTOR( &dir, &poly[2], &poly[1] ) ;
	if ( GEO_CheckLineInRegion( &fsize, &dir, &poly[1], &zero ) ) return 1 ;
	SUB_FVECTOR( &dir, &poly[0], &poly[2] ) ;
	if ( GEO_CheckLineInRegion( &fsize, &dir, &poly[2], &zero ) ) return 1 ;

	/* 平面が,領域を分割するか？ */
	if ( (p0 & p1 & p2)==0 )
	{
		FVECTOR  norm ;

		core_prim_calc_normal( &norm, &poly[0], &poly[1], &poly[2] ) ;
		return GEO_CheckPlaneInRegion( &norm, poly, &fsize ) ;
	}
	return  0 ; 
}

#if 0
static int in_bound_box( POLYDATA *spec, FVECTOR *amin, FVECTOR *amax )
{
	FVECTOR	 fpos, fsize ;
	FVECTOR	 apos, asize ;

	mk_bound_pos_size( &fpos, &fsize, spec ) ;
	mk_pos_size( &apos, &asize, amin, amax ) ;

	/* aからみてbのサイズを回転させた箱に点が入る
	   bからみてaのサイズを回転させた箱に点が入る
	   この２つの条件を満たせば,箱はコンフリクトしていることになる
	   */
	MA_ApplyMatrixPos( &apos, &inv, pos ) ;
	MA_ApplyMatrixRot( &size, &inv, &BOX(REG_B,size) ) ;
	if ( !GEO_CheckPointInRegion( &size, &BOX(REG_B,pos), NULL ) ){
		if ( !GEO_CheckPointInRegion( &asize, &BOX(REG_A,pos), NULL ) ){
			return 1 ;
		}
	}
	return 0 ;
}
#endif

static int in_bound_field( POLYDATA *spec, FVECTOR *amin, FVECTOR *amax )
{
	FVECTOR	 fpos, fsize ;
	FVECTOR	 apos, asize ;

	mk_bound_pos_size( &fpos, &fsize, spec ) ;
	mk_pos_size( &apos , &asize, amin, amax ) ;
	SUB_FVECTOR( &apos , &apos , &fpos  ) ;
	ADD_FVECTOR( &asize, &asize, &fsize ) ;
	if ( !GEO_CheckPointInRegion( &asize, &apos, NULL ) ){
		return 1 ;
	}
	return 0 ;
}





/*--------------------------------------------------------------------------

  ここからPOLYFUNC集

 --------------------------------------------------------------------------*/

int poly_bound( HP3DOBJECT obj, void *data )
{
    POLYDATA *spec = data ;
    int     i, j ;

    LPP3DPOLYGON poly = P3DObject(obj)->lpP3DPolygon  ;
    LPP3DVERTEX  vert = P3DObject(obj)->lpP3DVertex   ;

    for( i=P3DObject(obj)->nPolygonCount ; --i>=0 ; poly++ )
    {
		P3DXYZ *v ;
		LPP3DPOLYGONNODE node;
#if 0
		int    size ;
		HZXHAZARDINFO *h_info ;
		HZXFLOORINFO  *f_info ;

		/* 挑弾当たりのバウンドは見ない */
		f_info = P3DUserDataPolygon( poly, HZX_FLOORKEYNAME, &size ) ;
		if ( f_info && f_info->bRecoilOnly == TRUE ) continue ;
		h_info = P3DUserDataPolygon( poly, HZX_HAZARDKEYNAME, &size ) ;
		if ( h_info && h_info->bRecoilOnly == TRUE ) continue ;
#endif

		node = poly->lpP3DPolygonNode ;
		for( j=poly->nPolygonNodeCount ; --j>=0 ; ) {
			v = &(vert + node->nVertex)->p3dXYZCalc ;
			spec->cen.vx += v->x ;
			spec->cen.vy += v->y ;
			spec->cen.vz += v->z ;
			spec->cen.vw++ ;
			if ( v->x < spec->bmin.vx ) spec->bmin.vx = v->x ;
			if ( v->y < spec->bmin.vy ) spec->bmin.vy = v->y ;
			if ( v->z < spec->bmin.vz ) spec->bmin.vz = v->z ;
			if ( v->x > spec->bmax.vx ) spec->bmax.vx = v->x ;
			if ( v->y > spec->bmax.vy ) spec->bmax.vy = v->y ;
			if ( v->z > spec->bmax.vz ) spec->bmax.vz = v->z ;
			node++ ;
		}
    }
    return 1 ;
}


int poly_floor( HP3DOBJECT obj, void *data )
{
    POLYDATA *spec = data ;
    FVECTOR verts[10] ;
    int     i, n_verts   ;
    
    LPP3DPOLYGON poly = P3DObject(obj)->lpP3DPolygon  ;
    LPP3DVERTEX  vert = P3DObject(obj)->lpP3DVertex   ;

    for( i=P3DObject(obj)->nPolygonCount ; --i>=0 ; poly++ )
    {
		HZXFLOORINFO *info;
		int    size ;
		int    attribute ;

		/* ポリゴンの頂点データを取り出す */
		n_verts = get_vertex( poly, vert, verts ) ;
		if ( n_verts < 0 )
			return 0 ;
		if ( n_verts == 0 ) continue ;
		if ( in_bound_poly( spec, verts ) == 0 ) continue ;

		/* ユーザーデータからアトリビュートを取り出す */
		info = P3DUserDataPolygon( poly, HZX_FLOORKEYNAME, &size ) ;
		if ( info == NULL ) return 0 ;

		/* 属性 */
		attribute = (info->nSound & 0x0000000f) << 28;
		if(info->bNoPlayer == TRUE)    attribute |= HZX_FLOOR_NO_PLAYER;
		if(info->bNoEnemy == TRUE)     attribute |= HZX_FLOOR_NO_ENEMY;
		if(info->bNoBullet == TRUE)    attribute |= HZX_FLOOR_NO_BULLET;
		if(info->bNoMissile == TRUE)   attribute |= HZX_FLOOR_NO_MISSILE;
		if(info->bNoC4 == TRUE)	       attribute |= HZX_FLOOR_NO_C4;
		if(info->bNoRecoil == TRUE)    attribute |= HZX_FLOOR_NO_RECOIL;
		if(info->bNoClaymore == TRUE)  attribute |= HZX_FLOOR_NO_CLAYMORE;
		if(info->bNoBlood == TRUE)     attribute |= HZX_FLOOR_NO_BLOOD;
		if(info->bNoBulletHole == TRUE)attribute |= HZX_FLOOR_NO_BULLETHOLE;
		if(info->bNoSpray == TRUE)     attribute |= HZX_FLOOR_NO_SPRAY;
		if(info->bInverseKinematics == TRUE)attribute |= HZX_FLOOR_INVERSE_KINEMATICS;
		if(info->bStep == TRUE)        attribute |= HZX_FLOOR_STEP;
		if(info->bRecoilOnly == TRUE)  attribute |= HZX_FLOOR_RECOILE_TYPE;
		if(info->bNoEnemyEyes == TRUE) attribute |= HZX_FLOOR_NO_ENEMY_EYES;
		if(info->bNoObject == TRUE)    attribute |= HZX_FLOOR_NO_OBJECT;
		if(info->bCypher == TRUE)      attribute |= HZX_FLOOR_CYPHER;

		/*if(info->bRecoilOnly == TRUE)  continue ; debug bug bug*/
														
		/* プリミティブをセットする */
		set_prim( GEO_F_PRIM_POLY, verts, n_verts, attribute, spec,
				  P3DObject(obj)->szName ) ;
    }

    return 1 ;
}

int poly_hazard( HP3DOBJECT obj, void *data )
{
    POLYDATA *spec = data ;
    FVECTOR verts[10] ;
    int     i, n_verts   ;
    
    LPP3DPOLYGON poly = P3DObject(obj)->lpP3DPolygon  ;
    LPP3DVERTEX  vert = P3DObject(obj)->lpP3DVertex   ;

    for( i=P3DObject(obj)->nPolygonCount ; --i>=0 ; poly++ )
    {
		HZXHAZARDINFO *info;
		int    size ;
		int    attribute ;

		/* ポリゴンの頂点データを取り出す */
		n_verts = get_vertex( poly, vert, verts ) ;
		if ( n_verts < 0 )
			return 0 ;
		if ( n_verts == 0 ) continue ;
		if ( in_bound_poly( spec, verts ) == 0 ) continue ;

		/* ユーザーデータからアトリビュートを取り出す */
		info = P3DUserDataPolygon( poly, HZX_HAZARDKEYNAME, &size ) ;
		if ( info == NULL ) return 0 ;

		/* 属性 */
		attribute = (info->nSound & 0x0000000f) << 28;
        if(info->bNoPlayer == TRUE)    attribute |= HZX_SEG_NO_PLAYER;
        if(info->bNoEnemy == TRUE)     attribute |= HZX_SEG_NO_ENEMY;
        if(info->bNoBullet == TRUE)    attribute |= HZX_SEG_NO_BULLET;
        if(info->bNoMissile == TRUE)   attribute |= HZX_SEG_NO_MISSILE;
        if(info->bNoC4 == TRUE)        attribute |= HZX_SEG_NO_C4;
        if(info->bNoRecoil == TRUE)    attribute |= HZX_SEG_NO_RECOIL;
        if(info->bNoHarituki == TRUE)  attribute |= HZX_SEG_NO_HARITSUKI;
        if(info->bNoDispRadar == TRUE) attribute |= HZX_SEG_NO_DISP_RADAR;
        if(info->bNoBulletHole == TRUE)attribute |= HZX_SEG_NO_BULLETHOLE;
        if(info->bNoSpray == TRUE)     attribute |= HZX_SEG_NO_SPRAY;
        if(info->bRecoilOnly == TRUE)  attribute |= HZX_SEG_RECOIL_TYPE;
        if(info->bNoEnemyEyes == TRUE) attribute |= HZX_SEG_NO_ENEMY_EYES;
        if(info->bNoKnockSE == TRUE)   attribute |= HZX_SEG_NO_KNOCK_SE;
        if(info->bCypher == TRUE)      attribute |= HZX_SEG_CYPHER;
        if(info->bNoEnemyIK == TRUE)   attribute |= HZX_SEG_NO_ENEMY_IK;
        if(info->bCliff == TRUE)       attribute |= HZX_SEG_CLIFF;

		/*if(info->bRecoilOnly == TRUE)  continue ; //debug bug bug */

		/* プリミティブをセットする */
		set_prim( GEO_F_PRIM_POLY, verts, n_verts, attribute, spec,
				  P3DObject(obj)->szName ) ;
    }

    return 1 ;
}





int poly_trap( HP3DOBJECT obj, void *data )
{
    POLYDATA *spec = data ;
    FVECTOR verts[2] ;
    P3DXYZ  xyz1, xyz2;

    /* エリア情報を取り出す */
    P3DAreaObject( &xyz1, &xyz2, obj ) ;
    cp_p3dxyz_to_fv( &verts[0], &xyz1 ) ;
    cp_p3dxyz_to_fv( &verts[1], &xyz2 ) ;
    {
		LPHZXTRAPINFO *info;
		int    size ;
		int    attribute ;

		if ( in_bound_field( spec, &verts[0], &verts[1] ) == 0 ) {
			return 1 ;
		}

#if 0
		if ( vertex_inbound( verts, 2, spec ) == 0 ){
printf( "nande?\n" ) ;
			return 1 ;
		}
#endif

#if 0
		/* ユーザーデータからアトリビュートを取り出す */
		info = P3DUserDataObject( obj, HZX_TRAPKEYNAME, &size ) ;
		if ( info == NULL) return 0 ;

		/* 属性 */
		attribute = (0x00F0 & (info->nDarkArea << 4)) ;
        if ( info->bNoCheck == TRUE ) attribute |= HZX_TRAP_NO_CHECK ;
#else
		attribute = 0 ;
#endif
		/* プリミティブを加える */
		set_prim( GEO_F_PRIM_FIELD, verts, 2, attribute, spec,
				  P3DObject(obj)->szName ) ;
    }

    return 1 ;
}


int poly_dump( HP3DOBJECT obj, void *data )
{
    int          i, j ;
    LPP3DPOLYGON poly = P3DObject(obj)->lpP3DPolygon ;
    LPP3DVERTEX  vert = P3DObject(obj)->lpP3DVertex  ;

    for( i=P3DObject(obj)->nPolygonCount ; --i>=0 ; poly++ ) {
		LPP3DPOLYGONNODE node;

		/* 確保した領域に頂点データを格納する */
		node = poly->lpP3DPolygonNode ;
		printf( "Poly %d\n", poly->nPolygonNodeCount ) ;
		for( i=0 ; i<poly->nPolygonNodeCount ; i++ ) {
			printf( " Vertex%d = (%f, %f, %f)\n", i,
					(vert + node->nVertex)->p3dXYZCalc.x,
					(vert + node->nVertex)->p3dXYZCalc.y,
					(vert + node->nVertex)->p3dXYZCalc.z ) ;
			node++ ;
		}
    }
    return 1 ;
}
