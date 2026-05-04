/*
  near.c
  Nearチェック

  メタルのはVU化してしまっているので入り口と出力をそろえてあとは適当にやる
  
  2001/04/19 M.Kobayashi

  $Id: near.cpp,v 1.1.1.3 2002/11/19 11:42:48 Yoshizawa1 Exp $

  #それにしても near , NEAR がwindows で使われているので書きにくいったら

 */

#include <stdio.h>

#include "mgs_type.h"
#include "libgv.h"
#include "libhzx.h"

#include "bp_vector.h"

const int MAX_INSIDE_BLOCKS = 256;

struct NEAREST {
	FVECTOR	vecNear;	// 中心を原点とした座標系でのあたり個所
//	FVECTOR	vecEdge1;
//	FVECTOR	vecEdge2;
	BOOL	bEdge;
	union {
		float	fDist2;
		float	fDist;
	};
};

struct ANS {
	int which;
	union {
		HZX_VuSEG* pSegStatic;
		HZX_D_SEGMENT* pSegDynamic;
	};
	HZX_BLOCK*	pBlk;
	SVECTOR*	psvSeg;	
};

class NEARCHECK {
public:
	int idInsideBlock[ MAX_INSIDE_BLOCKS ];

	int nHit;
	NEAREST	neardata[2];
	ANS		ans[2];
	VECTOR	vecReact;
	
	int CheckSegment(VECTOR* vecP, VECTOR* pvecCenter);
	int	CheckBlockBound( HZX_GRP* pGrp, FVECTOR* pvecCenter, int rad, int mode );
	void CheckOneBlock( HZX_BLOCK* pBlk, HZX_VuSEG* pSeg, int nSeg, FVECTOR* pvecCenter, int seg_flag, int rad ) ;
	void CheckDynamicSegment( HZX_D_SEGMENT* pSeg, FVECTOR* pvecCenter, int seg_flag, int rad ) ;
	
	int NearHazardCheck( HZX_GROUP_ID id, FVECTOR* pvecCenter, int rad,
						 int chk_flag, int seg_flag, int react_rad );

	//// 結果格納用
	void GetAtr(int* atrs);
	void GetHazard( HZX_SEG* segs,  int* atrs );
};

#ifdef __cplusplus
extern "C" {
#endif

int             HZX_NearDebug = 0 ;

#ifdef __cplusplus
}
#endif

////////////////////

int NEARCHECK::CheckSegment(VECTOR* vecP, VECTOR* pvecCenter)
{
	NEAREST nearLocal;
	
	// オリジナルはここでバウンドチェックをやっている。
	
	// もっとも近い点を探す
   VECTOR	v2p1p2 = { 0 };
   VECTOR	v2p1cent = { 0 };
	v2p1p2.x = vecP[1].x - vecP[0].x;
	v2p1p2.y = vecP[1].z - vecP[0].z;
	v2p1cent.x = pvecCenter->x - vecP[0].x;
	v2p1cent.y = pvecCenter->z - vecP[0].z;
	
	float fRatio = BP_Vec3_InnerProduct(&v2p1p2, &v2p1cent);
	if( fRatio < 0.f ){
		/* p1が最近点 */
		BP_Vec3_SubVec(&nearLocal.vecNear, &vecP[0], pvecCenter);
		fRatio = 0.f;
		nearLocal.bEdge = true;
	} else {
		float leng = BP_Vec3_LengthSquared(&v2p1p2) ;
		if ( leng == 0.0f ) return -1 ;
		fRatio /= leng ;
		if( fRatio > 1.f ){
			/* p2が最近点 */
			BP_Vec3_SubVec(&nearLocal.vecNear, &vecP[1], pvecCenter);
			fRatio = 1.f;
			nearLocal.bEdge = true;
		} else {
			/* 垂線の足が最近点 */
			nearLocal.vecNear.x = v2p1p2.x * fRatio - v2p1cent.x;
			nearLocal.vecNear.z = v2p1p2.y * fRatio - v2p1cent.y;
			nearLocal.bEdge = false;
//			// この場合のみエッジデータをあとで使うので
//			nearLocal.vecEdge1 = vecP[0];
//			nearLocal.vecEdge2 = vecP[1];
		}
	}
	// 距離判定を先にやる
	nearLocal.fDist2 = nearLocal.vecNear.x * nearLocal.vecNear.x
			+ nearLocal.vecNear.z * nearLocal.vecNear.z;
	if( nearLocal.fDist2 >= neardata[1].fDist2 ) return -1;
	
	// 高さ判定
	
	float fY, fH;
	fY = vecP[0].y + fRatio * ( vecP[1].y - vecP[0].y );
	if ( pvecCenter->y < fY ) return -1;
	fH = vecP[0].w + fRatio * ( vecP[1].w - vecP[0].w );
	if ( pvecCenter->y > fY + fH ) return -1;
	
	// あたった。
	
	if( nearLocal.fDist2 < neardata[0].fDist2 ) {
		neardata[1] = neardata[0];
		neardata[0] = nearLocal;
		ans[1] = ans[0];
		nHit++;

		return 0;
	} else {
		if ( (int)nearLocal.vecNear.x == (int)neardata[0].vecNear.x &&
			 (int)nearLocal.vecNear.z == (int)neardata[0].vecNear.z ) return -1;
		neardata[1] = nearLocal;
		nHit++;
		return 1;
	}
}

void NEARCHECK::CheckOneBlock( HZX_BLOCK* pBlk, HZX_VuSEG* pSeg, int nSeg, FVECTOR* pvecCenter, int seg_flag, int rad )
{
	FVECTOR	b1, b2;
	FVECTOR vecCenterLocal;
	
	/* 自分のバウンディング作成 */
	vecCenterLocal.x = pvecCenter->vx - ( float )pBlk->tx ;
	vecCenterLocal.y = pvecCenter->vy - ( float )pBlk->ty ;
	vecCenterLocal.z = pvecCenter->vz - ( float )pBlk->tz ;
	b1.vx = (float)vecCenterLocal.vx - rad ;
	b1.vy = (float)vecCenterLocal.vy - rad ;
	b1.vz = (float)vecCenterLocal.vz - rad ;
	b2.vx = (float)vecCenterLocal.vx + rad ;
	b2.vy = (float)vecCenterLocal.vy + rad ;
	b2.vz = (float)vecCenterLocal.vz + rad ;
    for( ; nSeg > 0; nSeg--, pSeg++ ) {
		/* フラグチェック */
		if ( pSeg->atr & seg_flag ) continue ;
		/* バウンディングチェック */
		if ( pSeg->b1.vx > b2.vx || pSeg->b2.vx < b1.vx ||
			 pSeg->b1.vy > b2.vy || pSeg->b2.vy < b1.vy ||
			 pSeg->b1.vz > b2.vz || pSeg->b2.vz < b1.vz ) continue ;

		// ここからストリップチェック
		int nStr = pSeg->b1.pad - 1;
		SVECTOR* psv = pSeg->verts;
		VECTOR vecP[2];
		int i;
		for( i = 0 ; i < nStr; i++ ){
			vecP[0].x = (float) psv->vx;
			vecP[0].y = (float) psv->vy;
			vecP[0].z = (float) psv->vz;
			vecP[0].w = (float) psv->pad;
			psv++;
			vecP[1].x = (float) psv->vx;
			vecP[1].y = (float) psv->vy;
			vecP[1].z = (float) psv->vz;
			vecP[1].w = (float) psv->pad;

			int nInsert;

			if((nInsert = CheckSegment(vecP, &vecCenterLocal)) >= 0){
				ans[nInsert].which = HZX_KIND_WALL;
				ans[nInsert].pSegStatic = pSeg;
				ans[nInsert].pBlk = pBlk;
				ans[nInsert].psvSeg = psv - 1;
			}
		}
	}
}

void NEARCHECK::CheckDynamicSegment( HZX_D_SEGMENT* pSeg, FVECTOR* pvecCenter, int seg_flag, int rad )
{
	FVECTOR	b1, b2;
    /* 自分のバウンディング作成 */	
	b1.vx = (float)(pvecCenter->vx - rad);
	b1.vy = (float)(pvecCenter->vy - rad);
	b1.vz = (float)(pvecCenter->vz - rad);
	b2.vx = (float)(pvecCenter->vx + rad);
	b2.vy = (float)(pvecCenter->vy + rad);
	b2.vz = (float)(pvecCenter->vz + rad);

	for( ; pSeg != NULL; pSeg = pSeg->next ){
		/* フラグチェック */
		if ( pSeg->atr & ( seg_flag | HZX_SEG_SKIP ) ) continue;
		/* バウンディングチェック */
		if ( pSeg->b1.vx > b2.vx || pSeg->b2.vx < b1.vx ||
			 pSeg->b1.vy > b2.vy || pSeg->b2.vy < b1.vy ||
			 pSeg->b1.vz > b2.vz || pSeg->b2.vz < b1.vz ) continue ;

		VECTOR vecP[2];
		
		vecP[0].x = (float) pSeg->p1.vx;
		vecP[0].y = (float) pSeg->p1.vy;
		vecP[0].z = (float) pSeg->p1.vz;
		vecP[0].w = (float) pSeg->p1.vw;
		vecP[1].x = (float) pSeg->p2.vx;
		vecP[1].y = (float) pSeg->p2.vy;
		vecP[1].z = (float) pSeg->p2.vz;
		vecP[1].w = (float) pSeg->p2.vw;

		int nInsert;
		if((nInsert = CheckSegment(vecP, pvecCenter)) >= 0){
			ans[nInsert].which = HZX_KIND_WALL | HZX_KIND_DYNAMIC;
			ans[nInsert].pSegDynamic = pSeg;
			ans[nInsert].pBlk = NULL;
			ans[nInsert].psvSeg = NULL;
		}
	}
}

int	NEARCHECK::CheckBlockBound( HZX_GRP* pGrp, FVECTOR* pvecCenter, int rad, int mode )
{	// ブロックチェック
	int		i, j, k, x1, x2, y1, y2, z1, z2;
	int		min, size, dx, dy, dz ;
	HZX_BLOCK*	pBlk ;
	int		nInsideBlocks;

	dx = pGrp->div_x ; dz = pGrp->div_z ; dy = pGrp->div_y ;
	
    min = pGrp->bound_min_x ; size = pGrp->block_size_x ;
	x1 = ( ( int )pvecCenter->vx - rad - min ) / size ;
	x2 = ( ( int )pvecCenter->vx + rad - min ) / size ;
	if( x1 < 0 ) x1 = 0;
	if( x2 >= dx ) x2 = dx - 1;
	min = pGrp->bound_min_z ; size = pGrp->block_size_z ;
	z1 = ( ( int )pvecCenter->vz - rad - min ) / size ;
	z2 = ( ( int )pvecCenter->vz + rad - min ) / size ;
	if( z1 < 0 ) z1 = 0;
	if( z2 >= dz ) z2 = dz - 1;
	min = pGrp->bound_min_y ; size = pGrp->block_size_y ;
	if ( mode == 0 ) {
		y1 = ( ( int )pvecCenter->vy - min ) / size ;
		y2 = y1 ;
    } else {
		y1 = ( ( int )pvecCenter->vy - rad - min ) / size ;
		y2 = ( ( int )pvecCenter->vy + rad - min ) / size ;
    }
	if( y1 < 0 ) y1 = 0;
	if( y2 >= dy ) y2 = dy - 1;
	nInsideBlocks = 0 ;
    for ( k = y1; k <= y2; k ++ ) {
		for ( j = z1; j <= z2; j ++ ) {
			for ( i = x1; i <= x2; i ++ ) {
				int block;
				block = dx * j + i + ( k * dx * dz ) ;
				if ( block >= 0 && block < pGrp->n_blocks ) {
#ifdef DEBUG
					if ( nInsideBlocks == MAX_INSIDE_BLOCKS ) {
						printf( "too many inside blocks\n" ) ;
						break ;
					}
#endif
					pBlk = pGrp->blocks + block ;
#if 0	// 常に真ではないか？				
					if ( nBlocks && 
						 ( block <= InsideBlock[ N_InsideBlocks - 1 ] ) ) continue ;
#endif					
					if ( pBlk->n_segs == 0 ) continue ;
					idInsideBlock[ nInsideBlocks ] = block ;
					nInsideBlocks ++ ;
				}
			}
		}
		if ( mode == 0 ) break ;
    }

	return nInsideBlocks;
}

static inline float BP_Vec2_CCW( const FVECTOR* v0, const FVECTOR* v1 )
{
   return v0->x * v1->y - v0->y * v1->x;
}

int NEARCHECK::NearHazardCheck( HZX_GROUP_ID id, FVECTOR* pvecCenter, int rad,
								int chk_flag, int seg_flag, int react_rad )
{
    HZX_HDL*	pHzd;
	BOOL		bRecoilOnly, bRecoil;
	HZX_GRP*	pGrp;

	pHzd = HZX_GetCurrentHzx() ;
	nHit = 0;

	neardata[0].fDist2 = (float)rad * rad;
	neardata[1].fDist2 = (float)rad * rad;

	if ( chk_flag & HZX_CHK_RECOIL_TYPE_ONLY ) {
		bRecoilOnly = bRecoil = true ;
	} else {
		bRecoilOnly = false ;
		bRecoil = ( ( seg_flag & HZX_SEG_RECOIL_TYPE ) || ( chk_flag & HZX_CHK_RECOIL_TYPE ) );
	}

	/* サイファー専用チェック */
	if ( chk_flag & HZX_CHK_CYPHER ) {
		seg_flag &= ~HZX_SEG_CYPHER ;
	} else {
		seg_flag |= HZX_SEG_CYPHER ;
	}
	/* 崖あたりチェック */
	if ( chk_flag & HZX_CHK_CLIFF ) {
		seg_flag &= ~HZX_SEG_CLIFF ;
	} else {
		seg_flag |= HZX_SEG_CLIFF ;
	}

	id = HZX_AddGroupID( id ) ;

    while( id != 0 ){
		int nGroup = GV_GetNo( id );
		id &= ~GV_GetBit( nGroup );

#if 0 //PS2版に準拠させました。 yano 2002.03.08
		ASSERT( nGroup >= 0 && nGroup < pHzd->def->n_groups ) ;
#else
		//ASSERT( nGroup >= 0 && nGroup < pHzd->def->n_groups ) ;
		if ( nGroup < 0 || nGroup >= pHzd->def->n_groups ) {
			continue ;
		}
#endif
		pGrp = pHzd->def->groups + nGroup ;
		if ( pGrp->n_blocks == 0 ) continue ;
	
		/* 固定壁チェック */
		if ( chk_flag & HZX_CHK_F_SEGMENT ) {
			/* ブロックチェック */
			int nBlock = CheckBlockBound( pGrp, pvecCenter, rad, 0 ) ;
			int i;
			for ( i = 0; i < nBlock; i ++ ) {
				HZX_BLOCK* pBlk = pGrp->blocks + idInsideBlock[ i ];

				if ( !bRecoilOnly && pBlk->n_segs > 0 ) {
					CheckOneBlock( pBlk, pBlk->segs, pBlk->n_segs, pvecCenter, seg_flag, rad ) ;
				}
				if ( bRecoil && pBlk->n_bul_segs > 0 ) {
					CheckOneBlock( pBlk, pBlk->bul_segs, pBlk->n_bul_segs, pvecCenter, seg_flag, rad ) ;
				}
			}
		}
		/* 動的壁チェック */
		if ( ( chk_flag & HZX_CHK_D_SEGMENT ) &&
			( pGrp->dynamics->n_segs > 0 ) ) {
			CheckDynamicSegment( pGrp->dynamics->segs, pvecCenter, 
								 seg_flag, rad );
		}
    }

	// 不正データの修正
	neardata[0].vecNear.y = 0.f;
	neardata[1].vecNear.y = 0.f;
	if( nHit > 2 ) nHit = 2;

	// 反発計算
	if( nHit != 0 ) {
      VECTOR v2R1 = { 0 };
      VECTOR v2R2 = { 0 };
		neardata[0].fDist = bp_sqrtf(neardata[0].fDist2);  //BP_MATH - emulate PS2 sqrtf
#if 0
		float fRatio = 1.f - react_rad / neardata[0].fDist;
#else
		if( neardata[0].fDist == 0.0f ) neardata[0].fDist = 0.000001f;
		float fRatio = 1.f - react_rad / neardata[0].fDist;
#endif
		
		if( fRatio < 0.f ) {
			v2R1.x = neardata[0].vecNear.x * fRatio;
			v2R1.y = neardata[0].vecNear.z * fRatio;
			if( nHit == 2 ) {
				neardata[1].fDist = bp_sqrtf(neardata[1].fDist2);  //BP_MATH - emulate PS2 sqrtf
				fRatio = 1.f - react_rad / neardata[1].fDist;
				if( fRatio < 0.f ){
					v2R2.x = neardata[1].vecNear.x * fRatio;
					v2R2.y = neardata[1].vecNear.z * fRatio;

					float fR1R2, fR1R1, fR2R2;
					fR1R2 = BP_Vec3_InnerProduct( &v2R1, &v2R2 );
					fR2R2 = BP_Vec3_LengthSquared( &v2R2 );

					if( fR2R2 > fR1R2 ){
						// ２つの反発要求を満たす必要がある
						fR1R1 = BP_Vec3_LengthSquared( &v2R1 );
#if 0 /* 0DIV回避 yano 2002.03.11 */
						float f1_R1xR2 = 1.f / D3DXVec2CCW( &v2R1, &v2R2 );
#else
						float f1_R1xR2, dmdm;
						dmdm = BP_Vec2_CCW( &v2R1, &v2R2 );
						if( dmdm == 0.0f ) dmdm = 0.000001f;
						f1_R1xR2 = 1.f / dmdm;
#endif

						vecReact( (fR1R1 * v2R2.y - fR2R2 * v2R1.y) * f1_R1xR2 ,
								  0.f, 
								  (fR2R2 * v2R1.x - fR1R1 * v2R2.x) * f1_R1xR2 ,
								  0.f );
						return nHit;
					} // else neardata[0] のリアクションを満たすと
					//十分 neardata[1] のリアクションも満たせる
				} // else neardata[1] はリアクション半径外
			} // else あたったデータが１つ
			// １つあたったものとして考えてよい
			vecReact( v2R1.x, 0.f, v2R1.y, 0.f );
			return nHit;
		} // else neardata[0] はリアクション半径外
	} // else あたってない
	vecReact(0.f, 0.f, 0.f, 0.f);
	return nHit;
}

//////////////////////////

static	int	SetSegment( HZX_SEG* pSeg, HZX_BLOCK* pBlk, SVECTOR* psv)
{
	if ( pBlk == NULL || psv == NULL ) return -1 ;
	pSeg->p1.x = (float)(psv->vx + pBlk->tx);
	pSeg->p1.y = (float)(psv->vy + pBlk->ty);
	pSeg->p1.z = (float)(psv->vz + pBlk->tz);
	pSeg->p1.h = (float)psv->pad ;	psv ++ ;
	pSeg->p2.x = (float)(psv->vx + pBlk->tx);
	pSeg->p2.y = (float)(psv->vy + pBlk->ty);
	pSeg->p2.z = (float)(psv->vz + pBlk->tz);
	pSeg->p2.h = (float)psv->pad ;
	return 0 ;
}

static	void	SetSegmentD( HZX_SEG* pSeg, HZX_D_SEGMENT* pDseg )
{
	pSeg->p1.x = (float)pDseg->p1.vx;
	pSeg->p1.y = (float)pDseg->p1.vy;
	pSeg->p1.z = (float)pDseg->p1.vz;
	pSeg->p1.h = (float)pDseg->p1.vw;
	pSeg->p2.x = (float)pDseg->p2.vx;
	pSeg->p2.y = (float)pDseg->p2.vy;
	pSeg->p2.z = (float)pDseg->p2.vz;
	pSeg->p2.h = (float)pDseg->p2.vw;
}

///////////////////////

void NEARCHECK::GetHazard( HZX_SEG* segs,  int* atrs )
{
	int i;
	for( i = 0; i < nHit; i++ ){
		if( ans[i].which & HZX_KIND_DYNAMIC ){
			SetSegmentD( segs, ans[i].pSegDynamic );
			segs->ptr = ans[i].pSegDynamic;
		} else {
			SetSegment( segs, ans[i].pBlk, ans[i].psvSeg );
		}
		segs->type = HZX_TYPE_SEGMENT;
		segs++ ;
	}
	if(atrs != NULL) GetAtr(atrs);
}

void NEARCHECK::GetAtr( int* atrs )
{
	int i;
	for( i = 0; i < nHit; i++ ){
		if(ans[i].which & HZX_KIND_DYNAMIC){
			atrs[i] = ans[i].pSegDynamic->atr;
		}else{
			atrs[i] = ans[i].pSegStatic->atr;
		}
	}
}

//////////////////////////
static NEARCHECK nearcheck;

int	HZX_NearHazardCheck( HZX_GROUP_ID id, FVECTOR *from, int sphere
						, int chk_flag, int seg_flag, int r_sphere )
{
	return nearcheck.NearHazardCheck(id, from, sphere, chk_flag, seg_flag, r_sphere);
}

void	HZX_GetNearHazard( HZX_SEG *segs, int *atrs ) 
{
	nearcheck.GetHazard(segs, atrs);
}

void		HZX_GetNearHazardPtr( SVECTOR **segs, HZX_BLOCK **blk )
{	// dynamic が入ってくるとバグるのでは？
	// いちおうオリジナルと同じにしておく
	segs[ 0 ] = nearcheck.ans[ 0 ].psvSeg;
	segs[ 1 ] = nearcheck.ans[ 1 ].psvSeg;
	
	blk[ 0 ] = nearcheck.ans[ 0 ].pBlk;
	blk[ 1 ] = nearcheck.ans[ 1 ].pBlk;
}

/* 結果のポインタからＨＺＤ＿ＳＥＧを生成 */
void		HZX_MakeNearHazard( HZX_SEG *segs, SVECTOR **v, HZX_BLOCK **blk )
{	// dynamic が入ってくるとバグるのでは？
	// いちおうオリジナルと同じにしておく
	SetSegment( segs, blk[ 0 ], v[ 0 ] ) ;
	SetSegment( segs + 1, blk[ 1 ], v[ 1 ] ) ;
}

void HZX_GetNearVector( VECTOR* pvecs )
{	// あたり箇所（中心相対位置）取得
	pvecs[0] = nearcheck.neardata[0].vecNear;
	pvecs[1] = nearcheck.neardata[1].vecNear;
}

void	HZX_GetIsEdge( signed char *ie )
{	/* 近接壁への最近点がエッジであるかどうか */

	ie[0] = (signed char) nearcheck.neardata[0].bEdge;
	ie[1] = (signed char) nearcheck.neardata[1].bEdge;
}

void HZX_GetReactVector( VECTOR* pvec )
{	// 反発ベクトル取得
	*pvec = nearcheck.vecReact;
}

void	HZX_GetNearHazardAtr( int *atrs )
{
	nearcheck.GetAtr(atrs);
}

/*----------------------------------------------------------------------*/

/*
  レーダー用
*/
int	*HZX_GetNearBlockID( HZX_GRP* pGrp, FVECTOR* pvecCenter, int rad, int *n_blocks )
{/* ブロック単位でのバウンディングチェック */
	*n_blocks = 0;
	if ( pGrp->n_blocks > 0 ) *n_blocks = nearcheck.CheckBlockBound( pGrp, pvecCenter, rad, 1 ) ;
	return nearcheck.idInsideBlock;
}

/*----------------------------------------------------------------------*/
