//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_utl_aim.c
   壊れ用 狙い系汎用ルーチン

   2000/09/14 T. Morita
   $Id: brk_utl_aim.c,v 1.1.1.3 2002/11/19 11:45:50 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_utl.h"
#include "../brk_hzd/brk_hazard.h"


/*
  点と直線の距離

  lposからldirの方向に伸びている直線に対して p から垂直に伸ばした線の長さ
*/
float BRK_UTL_DistPointToLine( FVECTOR *p, FVECTOR *lp, FVECTOR *ld )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
}


/*
  直線が 3角ポリゴンを貫くか

  lposからldirの方向に伸びている線分に対して,vtxで作られる3角ポリゴンに含まれるどうかを調べる
  つき抜ける  １を返す
  つき抜けない０を返す

  更に lpos には,3角ポリゴンとの交点を結果として返す。
*/
int BRK_UTL_CheckLineInFloatPolygon( FVECTOR *lpos, FVECTOR *ldir,
									FVECTOR *ppos
									)
{
    FVECTOR a0, a1, pnrm ;
    float a,b,c ;

    /* 法線 pnrmを求める    */
    _sceVu0SubVector( &a0, &ppos[0], &ppos[1] ) ;
    _sceVu0SubVector( &a1, &ppos[1], &ppos[2] ) ;

    _sceVu0OuterProduct( &pnrm, &a0, &a1 ) ;
    /* 直線が平面に対して平行なら貫通しない */
    if ( (b = _sceVu0InnerProduct( ldir, &pnrm )) == 0.0f )
	return 0 ;

    /* 直線と平面の交点を求める */
    _sceVu0SubVector( &a0, &ppos[0], lpos ) ;
    a = _sceVu0InnerProduct( &pnrm, &a0 ) ;
    _sceVu0ScaleVector( &a0, ldir, a/b ) ;
    _sceVu0AddVector( lpos, &a0, lpos ) ;

    /* 交点が3頂点の内側にあるかどうかを判定する */
    a = BackFaceCulling( &ppos[0], &ppos[1], lpos ) ;
    b = BackFaceCulling( &ppos[1], &ppos[2], lpos ) ;
    c = BackFaceCulling( &ppos[2], &ppos[0], lpos ) ;
    if ( (a>0.0f && b>0.0f && c>0.0f) || (a<0.0f && b<0.0f && c<0.0f) )
	return 1 ;
    return 0 ;
}


/* モデルのSVECTORチェック */
int BRK_UTL_CheckLineInPolygon( FVECTOR *lpos, FVECTOR *ldir,
#ifdef PSX2
							    SVECTOR *vtx
#else
							    DG_VERTEX_KMSS *p0,
							    DG_VERTEX_KMSS *p1,
							    DG_VERTEX_KMSS *p2
#endif
							   )
{
    FVECTOR ppos[3] ;

    /* 浮動少数の頂点に変換 */
#ifdef PSX2
    SVector2FVector( &ppos[0], vtx++ ) ;
    SVector2FVector( &ppos[1], vtx++ ) ;
    SVector2FVector( &ppos[2], vtx   ) ;
#else
	ppos[0].vx = p0->vx ;
	ppos[0].vy = p0->vy ;
	ppos[0].vz = p0->vz ;
	ppos[1].vx = p1->vx ;
	ppos[1].vy = p1->vy ;
	ppos[1].vz = p1->vz ;
	ppos[2].vx = p2->vx ;
	ppos[2].vy = p2->vy ;
	ppos[2].vz = p2->vz ;
	ppos[0].vw = ppos[1].vw = ppos[2].vw = 1.0f ;
#endif

    return  BRK_UTL_CheckLineInFloatPolygon( lpos, ldir, ppos ) ;
}


/* ワールドのローカル座標を得る */
void BRK_UTL_LocalForceAndPoint( FVECTOR *lpos, FVECTOR *ldir,
				 FVECTOR *pos , FVECTOR *dir,
				 FMATRIX *world )
{
    FMATRIX     mtx ;

    /*ローカル座標系での着弾位置と着弾方向を求める*/
    _sceVu0CopyVector( ldir, dir ) ;
    _sceVu0CopyVector( lpos, pos ) ;
    lpos->vw = 1.0f ;
    ldir->vw = 0.0f ;
    _sceVu0InversMatrix( &mtx, world ) ;
    _sceVu0ApplyMatrix( ldir , &mtx, ldir  ) ;
    _sceVu0ApplyMatrix( lpos, &mtx, lpos ) ;
}


/* モデルのポリゴンチェック */
int BRK_UTL_CheckModelLine( FVECTOR *offs,
			    FVECTOR *hit, FVECTOR *force,
			    DG_MDL *model, FMATRIX *world )
{
    int         i, j ;
    DG_MDLPACK *m ;
    FVECTOR     frc ;
#ifdef KP_XBOX
	DG_VERTEX_KMSS *kmss = model->vbuff ;
#endif

    /* モデルのローカルを得る */
    BRK_UTL_LocalForceAndPoint( offs, hit, &frc, force, world ) ;

    /*弾のポリゴン当たり判定*/
    for ( m=model->packs, i=model->n_packs ; --i>=0 ; m++ )
    {
#ifdef PSX2
		SVECTOR    *s, *n ;

		s = (SVECTOR *)m->verts ;
		n = (SVECTOR *)m->norms ;
		for ( j=m->n_verts ; --j>=0 ; s++, n++ )
		  if ( !(n->pad & 0x8000) )
			if ( BRK_UTL_CheckLineInPolygon( offs, &frc, s-2 ) )
			  return 1 ;
#else
		int v0, v1, v2 ;

		v0 = v1= v2= *(short*)m->index ;
		for ( j=0 ; j<m->n_indices ; j++, kmss++ ) {
			if ( v0!=v1 && v0!=v2 && v1!=v2 ) {
				if ( BRK_UTL_CheckLineInPolygon( offs, &frc,
												&kmss[v0],
												&kmss[v1],
												&kmss[v2] ) ) {
					return 1 ;
				}
			}
			v2 = v1 ;
			v1 = v0 ;
			v0 =*((short*)m->index + j) ;
		}
#endif
    }
    return 0 ;
}
