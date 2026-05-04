/*
  online.c
  オンラインチェック

  メタルのはVU化してしまっているので入り口と出力をそろえてあとは適当にやる
  2001/04/16 M.Kobayashi

  $Id: online.cpp,v 1.1.1.3 2002/11/19 11:42:49 Yoshizawa1 Exp $

 */

#include <stdio.h>

#include "mgs_type.h"
#include "libgv.h"
#include "libhzx.h"

extern "C"
{
#include "bp_vector.h"
};

const int MAX_INSIDE_BLOCKS = 64;

struct BLOCK_WORK {
	VECTOR	vecFrom;
	VECTOR	vecTo;
	VECTOR	vecMin;
	VECTOR	vecMax;
	HZX_BLOCK* pBlock;
};

struct CROSS_DATA {	// 交点結果データ
	VECTOR		vecCross;	// 交点
	int 		which;		// 壁か床か
	HZX_BLOCK*	pBlock;		// 該当ブロック
	union {	// あたったセグメント
		HZX_VuSEG* 		pSegStatic;	 
		HZX_D_SEGMENT*	pSegDynamicWall;
		HZX_D_FLOOR*	pSegDynamicFloor;
	};
	int		nStr;		// ストリップ内番号
	float	fDist2Min;	// 距離二乗の最小

	int		idGrp;		// グループ
};

class ONLINE {
public:
	VECTOR	vecFromOrg;
	VECTOR	vecToOrg;

	int		nInsideBlockCurrent;

	int		N_InsideBlocks[ HZX_MAX_GROUPS ];
	int		Group[ HZX_MAX_GROUPS ];

	BLOCK_WORK	bw[ MAX_INSIDE_BLOCKS ];

	// 結果
	int 	nHit;
	CROSS_DATA	cr;

	BOOL CutLinebyBlock(VECTOR* pvecB1, VECTOR* pvecB2,
						VECTOR* pvecFrom, VECTOR* pvecTo);
	BOOL CheckOneBlockSeg( CROSS_DATA* pCr, BLOCK_WORK* pWork, int seg_flag, BOOL bRecoil );
	BOOL CheckOneBlockFlr( CROSS_DATA* pCr, BLOCK_WORK* pWork, int seg_flag, BOOL bRecoil );

	BOOL CheckDynamicSegment( CROSS_DATA* pCr, HZX_D_SEGMENT* pSeg, int seg_flag );
	BOOL CheckDynamicFloor( CROSS_DATA* pCr, HZX_D_FLOOR* pFlr, int seg_flag );
	
	void CheckBlockBound(HZX_GRP* pGrp, int nGroup);
	int OnlineHazardCheck( HZX_GROUP_ID id, FVECTOR* pvecFrom, FVECTOR* pvecTo,
						   int chk_flag, int seg_flag, int flr_flag );
	int OnlineHazardCheckOneSegment( HZX_SEG* pSeg, FVECTOR* pvecFrom, FVECTOR* pvecTo );
	
	// 結果取得
	void	GetHazard( HZX_FLR* pSeg, int* atr );
	int		GetHazardFloorType( void );
	void	GetVector( VECTOR* pvec );
	void	GetPoint( VECTOR* pvec ){ *pvec = cr.vecCross; }
	int		GetAtr( void );
	int		GetType( void ){ return cr.which; }
	int	   	GetGroup( void ){ return cr.idGrp; }
};

static ONLINE online;

/* ------------ static 関数 ---------------- */
static void SwapVector(VECTOR** ppvec0, VECTOR** ppvec1)
{	// ベクトルポインタの参照先を変える
	VECTOR* pvecTmp;
	pvecTmp = *ppvec0;
	*ppvec0 = *ppvec1;
	*ppvec1 = pvecTmp;
}

static void MakeBound(VECTOR* pvecMin, VECTOR* pvecMax,
					  VECTOR* pvec0, VECTOR* pvec1)
{	// バウンドボックス作成
	if( pvec0->x < pvec1->x ){
		pvecMin->x = pvec0->x;	pvecMax->x = pvec1->x;
	} else {
		pvecMax->x = pvec0->x;	pvecMin->x = pvec1->x;
	}
	if( pvec0->y < pvec1->y ){
		pvecMin->y = pvec0->y;	pvecMax->y = pvec1->y;
	} else {
		pvecMax->y = pvec0->y;	pvecMin->y = pvec1->y;
	}
	if( pvec0->z < pvec1->z ){
		pvecMin->z = pvec0->z;	pvecMax->z = pvec1->z;
	} else {
		pvecMax->z = pvec0->z;	pvecMin->z = pvec1->z;
	}
}


#if 1  /*Revised to 3D Check by T.Morita 2002.03.29 */
static inline float BackFaceCulling( VECTOR *n, VECTOR *v1, VECTOR *v2, VECTOR *v3 )
{
    FVECTOR a0, a1 ;

    _sceVu0SubVector( &a0, v2, v1 ) ;
    _sceVu0SubVector( &a1, v3, v1 ) ;
    _sceVu0OuterProduct( &a0, &a0, &a1 ) ;
    return _sceVu0InnerProduct( n, &a0 ) + 1 ;
}
static inline void NormalVector( VECTOR *n, VECTOR *v1, VECTOR *v2, VECTOR *v3 )
{
    FVECTOR a0, a1 ;
    _sceVu0SubVector( &a0, v2, v1 ) ;
    _sceVu0SubVector( &a1, v3, v2 ) ;
    _sceVu0OuterProduct( n, &a0, &a1 ) ;
}
static BOOL CheckInside( VECTOR* pvecCross, int nStrVert, VECTOR* pvecVert )
{
    VECTOR n ;
    union {
		float f ;
		int   i ;
    } a,b,c,d ;

    NormalVector( &n, &pvecVert[0], &pvecVert[1], &pvecVert[2] ) ;

    a.f = BackFaceCulling( &n, &pvecVert[0], &pvecVert[1], pvecCross ) ;
    b.f = BackFaceCulling( &n, &pvecVert[1], &pvecVert[2], pvecCross ) ;

    if ( nStrVert == 4 ) {
		/* 交点が4頂点の内側にあるかどうかを判定する */
		c.f = BackFaceCulling( &n, &pvecVert[2], &pvecVert[3], pvecCross ) ;
		d.f = BackFaceCulling( &n, &pvecVert[3], &pvecVert[0], pvecCross ) ;
		if ( ((a.i | b.i | c.i | d.i) & 0x80000000) == 0  ) {
			return true ;
		}
    } else {
		/* 交点が3頂点の内側にあるかどうかを判定する */
		c.f = BackFaceCulling( &n, &pvecVert[2], &pvecVert[0], pvecCross ) ;
		if ( ((a.i | b.i | c.i) & 0x80000000) == 0  ) {
			return true ;
		}
    }
    return false ;
}
#else
static BOOL CheckInside(VECTOR* pvecCross, int nStrVert, VECTOR* pvecVert )
{	// 交点が頂点列で示される凸多角形の中に含まれているかチェック
	int i;
	for ( i = 0 ; i < nStrVert; i++, pvecVert ){
		VECTOR* pv1 = pvecVert + i;
		VECTOR* pv2 = ((i == nStrVert - 1) ? pvecVert : (pvecVert + i + 1));
		if( (pv2->x - pv1->x) * (pvecCross->z - pv1->z)
			- (pv2->z - pv1->z) * (pvecCross->x - pv1->x) < 0.f ) return false;
	}

	return true;
}
#endif

/* ---------------------------------- */

BOOL ONLINE::CutLinebyBlock(VECTOR* pvecB1, VECTOR* pvecB2,
							VECTOR* pvecFrom, VECTOR* pvecTo)
{	// 指定直方体で直線を切る
	VECTOR	vecDiffLocal ;
	float	va, vb, vc ;

	*pvecFrom = vecFromOrg;
	*pvecTo = vecToOrg;
	
	va = pvecTo->vx - pvecFrom->vx;
	if ( va != 0.0F ) {
		if ( va < 0.0F ) {
			SwapVector( &pvecFrom, &pvecTo ) ;
			va = -va ;
		}
		vb = ( pvecB1->x - pvecFrom->x ) ;
		vc = ( pvecB2->x - pvecFrom->x ) ;
		if ( vb > va || vc < 0.0F ) return false ;
		if ( vb < 0.0F ) vb = 0.0F ;
		if ( vc > va ) vc = va ;

      BP_Vec3_SubVec(&vecDiffLocal, pvecTo, pvecFrom);
      BP_Vec3_AddVec(pvecTo, pvecFrom, &(vecDiffLocal * (vc / vecDiffLocal.x) ) );
      BP_Vec3_AddVec(pvecFrom, pvecFrom, &(vecDiffLocal * (vb / vecDiffLocal.x) ) );
    }
	
	va = pvecTo->vy - pvecFrom->vy ;
	if ( va != 0.0F ) {
		if ( va < 0.0F ) {
			SwapVector( &pvecFrom, &pvecTo ) ;
			va = -va ;
		}
		vb = ( pvecB1->vy - pvecFrom->vy ) ;
		vc = ( pvecB2->vy - pvecFrom->vy ) ;
		if ( vb > va || vc < 0.0F ) return 0 ;
		if ( vb < 0.0F ) vb = 0.0F ;
		if ( vc > va ) vc = va ;

		BP_Vec3_SubVec(&vecDiffLocal, pvecTo, pvecFrom);
		BP_Vec3_AddVec(pvecTo, pvecFrom, &(vecDiffLocal * (vc / vecDiffLocal.y) ) );
		BP_Vec3_AddVec(pvecFrom, pvecFrom, &(vecDiffLocal * (vb / vecDiffLocal.y) ) );
    }
	
	va = pvecTo->z - pvecFrom->z ;
	if ( va != 0.0F ) {
		if ( va < 0.0F ) {
			SwapVector( &pvecFrom, &pvecTo ) ;
			va = -va ;
		}
		vb = ( pvecB1->z - pvecFrom->z ) ;
		vc = ( pvecB2->z - pvecFrom->z ) ;
		if ( vb > va || vc < 0.0F ) return 0 ;
		if ( vb < 0.0F ) vb = 0.0F ;
		if ( vc > va ) vc = va ;

		BP_Vec3_SubVec(&vecDiffLocal, pvecTo, pvecFrom);
		BP_Vec3_AddVec(pvecTo, pvecFrom, &(vecDiffLocal * (vc / vecDiffLocal.z) ) );
		BP_Vec3_AddVec(pvecFrom, pvecFrom, &(vecDiffLocal * (vb / vecDiffLocal.z) ) );
    }
	return true ;
}

void ONLINE::CheckBlockBound(HZX_GRP* pGrp, int nGroup)
{	// グループ内のチェックが必要なブロックを列挙する
	VECTOR	vecSize;
	VECTOR	vecMin;

	VECTOR	vecFromLocal;
	VECTOR	vecToLocal;

	VECTOR	vecB1;
	VECTOR	vecB2;
	

	HZX_BLOCK* pBlk;

	int nBlock;

	int dx, dy, dz;
	int x1, y1, z1;

    N_InsideBlocks[ nGroup ] = 0 ;

	vecSize.x = (float)pGrp->block_size_x;
	vecSize.y = (float)pGrp->block_size_y;
	vecSize.z = (float)pGrp->block_size_z;

	vecMin.x = (float)pGrp->bound_min_x;
	vecMin.y = (float)pGrp->bound_min_y;
	vecMin.z = (float)pGrp->bound_min_z;

	// 各軸方向へのブロック数
	dx = pGrp->div_x;	
	dy = pGrp->div_y;
	dz = pGrp->div_z;

    /* ＦＲＯＭ－ＴＯベクトルを
       グループに入るように切る */
	vecB1.x = vecMin.x + 1.0F ;
	vecB1.y = vecMin.y + 1.0F ;
	vecB1.z = vecMin.z + 1.0F ;
	vecB2.x = vecMin.x + vecSize.x * ( float )dx - 1.0F ;
    vecB2.y = vecMin.y + vecSize.y * ( float )dy - 1.0F ;
    vecB2.z = vecMin.z + vecSize.z * ( float )dz - 1.0F ;
    if ( !CutLinebyBlock( &vecB1, &vecB2, &vecFromLocal, &vecToLocal ) ) return ;
	// ブロック番号の計算
    x1 = ( int )( ( vecFromLocal.x - vecMin.x ) / vecSize.x ) ;
    z1 = ( int )( ( vecFromLocal.z - vecMin.z ) / vecSize.z ) ;
    y1 = ( int )( ( vecFromLocal.y - vecMin.y ) / vecSize.y ) ;

	nBlock = dx * z1 + x1 + ( y1 * dx * dz ) ;
	if ( nBlock < 0 || nBlock >= pGrp->n_blocks ) { 
		return ;
    }

    while( 1 ) {
		int x2, y2, z2;
		nBlock = dx * z1 + x1 + ( y1 * dx * dz ) ;
		pBlk = pGrp->blocks + nBlock ;

		vecB1.x = vecMin.x + ( float )x1 * vecSize.x - 1.0F ; vecB2.x = vecB1.x + vecSize.x + 2.0F ;
		vecB1.y = vecMin.y + ( float )y1 * vecSize.y - 1.0F ; vecB2.y = vecB1.y + vecSize.y + 2.0F ;
		vecB1.z = vecMin.z + ( float )z1 * vecSize.z - 1.0F ; vecB2.z = vecB1.z + vecSize.z + 2.0F ;
		
		if ( !CutLinebyBlock( &vecB1, &vecB2, &vecFromLocal, &vecToLocal ) ) break ;
		if ( pBlk->n_segs != 0 ||
			pBlk->n_bul_segs != 0 ||
			pBlk->n_flrs != 0 ||
			pBlk->n_bul_flrs != 0 ) {
			int nBox = nInsideBlockCurrent + N_InsideBlocks[ nGroup ] ;
			if ( nBox == MAX_INSIDE_BLOCKS ) {
				printf( "inside blocks over\n" ) ;
				break ;
			}
			// オリジナルとちょっと変える
			// 座標変換・ブロック中心を原点に座標変換する（そのほうがダイレクトに情報を利用できるので）
			bw[nBox].vecFrom.x = vecFromLocal.x - (float)pBlk->tx;
			bw[nBox].vecFrom.y = vecFromLocal.y - (float)pBlk->ty;
			bw[nBox].vecFrom.z = vecFromLocal.z - (float)pBlk->tz;
			bw[nBox].vecTo.x = vecToLocal.x - (float)pBlk->tx;
			bw[nBox].vecTo.y = vecToLocal.y - (float)pBlk->ty;
			bw[nBox].vecTo.z = vecToLocal.z - (float)pBlk->tz;
			MakeBound(&bw[nBox].vecMin, &bw[nBox].vecMax, &bw[nBox].vecFrom, &bw[nBox].vecTo);
			bw[nBox].pBlock = pBlk;

			N_InsideBlocks[ nGroup ] ++ ;
		}
		x2 = ( int )( ( vecToLocal.x - vecMin.x ) / vecSize.x ) ;
		z2 = ( int )( ( vecToLocal.z - vecMin.z ) / vecSize.z ) ;
		y2 = ( int )( ( vecToLocal.y - vecMin.y ) / vecSize.y ) ;
		if ( ( x1 == x2 && y1 == y2 && z1 == z2 ) ||
			( x2 < 0 || x2 >= dx || y2 < 0 || y2 >= dy || z2 < 0 || z2 >= dz ) ) break ;
		x1 = x2 ; y1 = y2 ; z1 = z2 ;
	}
	nInsideBlockCurrent += N_InsideBlocks[ nGroup ] ;
}

////////

BOOL ONLINE::CheckOneBlockSeg( CROSS_DATA* pCr, BLOCK_WORK* pWork, int seg_flag, BOOL bRecoil )
{
	HZX_BLOCK* pBlk = pWork->pBlock;
	int nSeg;
	HZX_VuSEG* pSeg;

	// 参照データ確定
	if(bRecoil){
		nSeg = pBlk->n_bul_segs;
		pSeg = pBlk->bul_segs;
	}else{
		nSeg = pBlk->n_segs;
		pSeg = pBlk->segs;
	}
	if(nSeg <= 0) return false;
	
	VECTOR	vecDiffLocal;
	VECTOR*	pvecFrom = &pWork->vecFrom;
	VECTOR*	pvecTo = &pWork->vecTo;
	BOOL	bHit = false;

	// 必要パラメータ作成
	BP_Vec3_SubVec( &vecDiffLocal, pvecTo, pvecFrom );
	
	int i, j;
	for( i = 0 ; i < nSeg ; i++, pSeg++){
		/* フラグチェック */
		if ( pSeg->atr & seg_flag ) continue ;

		/* バウンディングチェック */
		if ( ( float )pSeg->b1.vx > pWork->vecMax.x ||
			 ( float )pSeg->b1.vy > pWork->vecMax.y ||
			 ( float )pSeg->b1.vz > pWork->vecMax.z ||
			 ( float )pSeg->b2.vx < pWork->vecMin.x ||
			 ( float )pSeg->b2.vy < pWork->vecMin.y ||
			 ( float )pSeg->b2.vz < pWork->vecMin.z ) continue;

		// ここからストリップチェック
		int nStr = pSeg->b1.pad - 1;
		SVECTOR* psv;
		VECTOR	vecP1;
		VECTOR	vecP2;
		for( psv = pSeg->verts, j = 0 ; j < nStr ; j++){
			vecP1.x = (float) psv->vx;
			vecP1.y = (float) psv->vy;
			vecP1.z = (float) psv->vz;
			vecP1.w = (float) psv->pad;
			psv++;
			vecP2.x = (float) psv->vx;
			vecP2.y = (float) psv->vy;
			vecP2.z = (float) psv->vz;
			vecP2.w = (float) psv->pad;

			// ここからはメタルと違う方法で計算する。
			// 1. from->to 直線に対する p1, p2 の符号付距離比例値を計算する。
			//    符号が同じならあたっていない。
			// 2. p1 -> p2 直線に対する from, to の符号付距離比例値を計算する。
			//    符号が同じならあたっていない。
			// 3. 2で計算した距離比で from, to を内分し、交点を算出。
			// 4. 高さチェックを行う。

			// 1. from->to 直線に対する p1, p2 の符号付距離比例値を計算する。
			float fDistP1, fDistP2;
			fDistP1 = vecDiffLocal.z * (vecP1.x - pvecFrom->x)
					- vecDiffLocal.x * (vecP1.z - pvecFrom->z);
			fDistP2 = vecDiffLocal.z * (vecP2.x - pvecFrom->x)
					- vecDiffLocal.x * (vecP2.z - pvecFrom->z);
#if 0		// あたりやすいようにしてみる	
			if( fDistP1 * fDistP2 >= 0.f ) continue;	// 各個チェックのほうが軽いか？
#else			
			if( fDistP1 * fDistP2 > 0.f ) continue;	// 各個チェックのほうが軽いか？
#endif			

         //BP_MATH - fix divide by zero
         if( fDistP1 == fDistP2 )
         {
            continue;
         }
         //BP_MATH - fix divide by zero

			// 2. p1 -> p2 直線に対する from, to の符号付距離比例値を計算する。
			float fDistFr, fDistTo, fDiffP1P2x, fDiffP1P2z;
			fDiffP1P2x = vecP2.x - vecP1.x;
			fDiffP1P2z = vecP2.z - vecP1.z;
			fDistFr = fDiffP1P2z * (pvecFrom->x - vecP1.x)
					- fDiffP1P2x * (pvecFrom->z - vecP1.z);
			fDistTo = fDiffP1P2z * (pvecTo->x - vecP1.x)
					- fDiffP1P2x * (pvecTo->z - vecP1.z);
#if 0		// あたりやすいようにしてみる	
			if( fDistFr * fDistTo >= 0.f ) continue;	// 各個チェックのほうが軽いか？
#else
			if( fDistFr * fDistTo > 0.f ) continue;	// 各個チェックのほうが軽いか？
#endif			

         //BP_MATH - fix divide by zero
         if( fDistFr == fDistTo )
         {
            continue;
         }
         //BP_MATH - fix divide by zero

			// 3. 2で計算した距離比で from, to を内分し、交点を算出。
			VECTOR vecCross;
			float	fRatio;
			fRatio = fDistFr / (fDistFr - fDistTo);
			BP_Vec3_AddVec(&vecCross, &(*pvecTo * fRatio), &(*pvecFrom * (1.f - fRatio) ) );

			// 高さチェック
			fRatio = fDistP1 / (fDistP1 - fDistP2);
			float fY = vecP2.y * fRatio + vecP1.y * (1.f - fRatio);
			if( fY > vecCross.y ) continue;
			float fH = vecP2.w * fRatio + vecP1.w * (1.f - fRatio);
			if( fH + fY < vecCross.y ) continue;

			// 距離判定
			float fDist2 = vecCross.Dist2( pvecFrom );
			if( fDist2 >= pCr->fDist2Min ) continue;
			
			// あたり判定OK 距離を比較して結果を蓄える
			nHit++;
			bHit = true;
			
			// 結果格納
			pCr->vecCross = vecCross;
			pCr->fDist2Min = fDist2;	// これは真の距離sqではない
			pCr->which = HZX_KIND_WALL;
			pCr->pBlock = pBlk;
			pCr->pSegStatic = pSeg;
			pCr->nStr = j;
		}
	}

	return bHit;
}

BOOL ONLINE::CheckOneBlockFlr( CROSS_DATA* pCr, BLOCK_WORK* pWork, int seg_flag, BOOL bRecoil )
{
	HZX_BLOCK* pBlk = pWork->pBlock;
	int nSeg;
	HZX_VuSEG* pSeg;

	// 参照データ確定
	if(bRecoil){
		nSeg = pBlk->n_bul_flrs;
		pSeg = pBlk->bul_flrs;
	}else{
		nSeg = pBlk->n_flrs;
		pSeg = pBlk->flrs;
	}
	if(nSeg <= 0) return false;
	
	VECTOR*	pvecFrom = &pWork->vecFrom;
	VECTOR*	pvecTo = &pWork->vecTo;
	BOOL	bHit = false;

	int i, j, k;
	for( i = 0 ; i < nSeg ; i++, pSeg++){
		/* フラグチェック */
		if ( pSeg->atr & seg_flag ) continue ;
		/* バウンディングチェック */
		if ( ( float )pSeg->b1.vx > pWork->vecMax.x ||
			 ( float )pSeg->b1.vy > pWork->vecMax.y ||
			 ( float )pSeg->b1.vz > pWork->vecMax.z ||
			 ( float )pSeg->b2.vx < pWork->vecMin.x ||
			 ( float )pSeg->b2.vy < pWork->vecMin.y ||
			 ( float )pSeg->b2.vz < pWork->vecMin.z ) continue;

		// ここからストリップチェック
		int nTotalVert = pSeg->b1.pad;
		int nStrVert = pSeg->b2.pad;
		SVECTOR* psv;
		VECTOR	vecV[4];
		for( psv = pSeg->verts, j = 0 ; j < nTotalVert ; j += nStrVert ){
			for( k = 0; k < nStrVert; k++){
				vecV[k].x = (float)psv->vx;
				vecV[k].y = (float)psv->vy;
				vecV[k].z = (float)psv->vz;
				vecV[k].w = (float)psv->pad;
				psv++;
			}
			// 交点の算出
			float fDistFr, fDistTo;
			if(pSeg->atr & HZX_FLOOR_FLAT){
				fDistFr = pvecFrom->y - vecV[0].y;
				fDistTo = pvecTo->y - vecV[0].y;
			}else{
				VECTOR vecNorm;
				VECTOR vecTmp;
				vecNorm.x = vecV[0].w;
				vecNorm.y = vecV[1].w;
				vecNorm.z = vecV[2].w;
				BP_Vec3_SubVec(&vecTmp, pvecFrom, &vecV[0]);
				fDistFr = BP_Vec3_InnerProduct(&vecTmp, &vecNorm);
				BP_Vec3_SubVec(&vecTmp, pvecTo, &vecV[0]);
				fDistTo = BP_Vec3_InnerProduct(&vecTmp, &vecNorm);
			}
#if 0		// あたりやすいようにしてみる	
			if( fDistFr * fDistTo >= 0.f ) continue;	// 平面の同じがわにある→あたってない
#else
			if( fDistFr * fDistTo > 0.f ) continue;	// 平面の同じがわにある→あたってない
			if( fDistFr == fDistTo ) continue;	    // 平面の同じがわにある→あたってない
#endif			
			
			VECTOR vecCross;
			float	fRatio;
			fRatio = fDistFr / (fDistFr - fDistTo);
			BP_Vec3_AddVec(&vecCross, &(*pvecTo * fRatio), &(*pvecFrom * (1.f - fRatio) ) );

			float fDist2 = vecCross.Dist2( pvecFrom );
			if( fDist2 >= pCr->fDist2Min ) continue;

			// 交点が内部にあるかチェック
			if( pSeg->atr & HZX_FLOOR_RECT ){
				if( (vecCross.x - vecV[0].x) * (vecCross.x - vecV[2].x) > 0.f ) continue;
				if( (vecCross.z - vecV[0].z) * (vecCross.z - vecV[2].z) > 0.f ) continue;
			} else if( !CheckInside( &vecCross, nStrVert, vecV ) ) continue;

			// あたってる
			nHit++;
			bHit = true;
			
			// 結果格納			
			pCr->vecCross = vecCross;
			pCr->fDist2Min = fDist2;	// これは真の距離sqではない
			pCr->which = HZX_KIND_FLOOR;
			pCr->pBlock = pBlk;
			pCr->pSegStatic = pSeg;
			pCr->nStr = j;
		}
	}

	return bHit;
}

BOOL ONLINE::CheckDynamicSegment( CROSS_DATA* pCr, HZX_D_SEGMENT* pSeg, int seg_flag )
{	// CheckOneBlockSeg と似ている
	VECTOR*	pvecFrom = &vecFromOrg;
	VECTOR*	pvecTo = &vecToOrg;
	VECTOR	vecDiffLocal;
	VECTOR	vecMin;
	VECTOR	vecMax;
	BOOL	bHit = false;

	// 必要パラメータ作成
	BP_Vec3_SubVec( &vecDiffLocal, pvecTo, pvecFrom );
	MakeBound(&vecMin, &vecMax, pvecFrom, pvecTo);

	for( ; pSeg != NULL; pSeg = pSeg->next ){
		/* フラグチェック */
		if ( pSeg->atr & ( seg_flag | HZX_SEG_SKIP ) ) continue ;
		/* バウンディングチェック */
		if ( ( float )pSeg->b1.vx > vecMax.x ||
			 ( float )pSeg->b1.vy > vecMax.y ||
			 ( float )pSeg->b1.vz > vecMax.z ||
			 ( float )pSeg->b2.vx < vecMin.x ||
			 ( float )pSeg->b2.vy < vecMin.y ||
			 ( float )pSeg->b2.vz < vecMin.z ) continue;

		// ここからストリップチェック
		VECTOR	vecP1;
		VECTOR	vecP2;
		
		vecP1.x = (float) pSeg->p1.vx;
		vecP1.y = (float) pSeg->p1.vy;
		vecP1.z = (float) pSeg->p1.vz;
		vecP1.w = (float) pSeg->p1.vw;
		vecP2.x = (float) pSeg->p2.vx;
		vecP2.y = (float) pSeg->p2.vy;
		vecP2.z = (float) pSeg->p2.vz;
		vecP2.w = (float) pSeg->p2.vw;

		// 1. from->to 直線に対する p1, p2 の符号付距離比例値を計算する。
		float fDistP1, fDistP2;
		fDistP1 = vecDiffLocal.z * (vecP1.x - pvecFrom->x)
				- vecDiffLocal.x * (vecP1.z - pvecFrom->z);
		fDistP2 = vecDiffLocal.z * (vecP2.x - pvecFrom->x)
				- vecDiffLocal.x * (vecP2.z - pvecFrom->z);
		if( fDistP1 * fDistP2 >= 0.f ) continue;	// 各個チェックのほうが軽いか？
		ASSERT( fDistP1 != fDistP2 ) ;


		// 2. p1 -> p2 直線に対する from, to の符号付距離比例値を計算する。
		float fDistFr, fDistTo, fDiffP1P2x, fDiffP1P2z;
		fDiffP1P2x = vecP2.x - vecP1.x;
		fDiffP1P2z = vecP2.z - vecP1.z;
		fDistFr = fDiffP1P2z * (pvecFrom->x - vecP1.x)
				- fDiffP1P2x * (pvecFrom->z - vecP1.z);
		fDistTo = fDiffP1P2z * (pvecTo->x - vecP1.x)
				- fDiffP1P2x * (pvecTo->z - vecP1.z);
		if( fDistFr * fDistTo >= 0.f ) continue;	// 各個チェックのほうが軽いか？
		ASSERT( fDistFr != fDistTo ) ;

      //BP_MATH - fix divide by zero
      if( fDistFr == fDistTo )
      {
         continue;
      }
      //BP_MATH - fix divide by zero

		// 3. 2で計算した距離比で from, to を内分し、交点を算出。
		VECTOR vecCross;
		float	fRatio;
		fRatio = fDistFr / (fDistFr - fDistTo);
		BP_Vec3_AddVec(&vecCross, &(*pvecTo * fRatio), &(*pvecFrom * (1.f - fRatio) ) );

      //BP_MATH - fix divide by zero
      if( fDistP1 == fDistP2 )
      {
         continue;
      }
      //BP_MATH - fix divide by zero

		// 高さチェック
		fRatio = fDistP1 / (fDistP1 - fDistP2);
		float fY = vecP2.y * fRatio + vecP1.y * (1.f - fRatio);
		if( fY > vecCross.y ) continue;
		float fH = vecP2.w * fRatio + vecP1.w * (1.f - fRatio);
		if( fH + fY < vecCross.y ) continue;
		
		float fDist2 = vecCross.Dist2( pvecFrom );
		if( fDist2 >= pCr->fDist2Min ) continue;
		// あたり判定OK 距離を比較して結果を蓄える
		nHit++;
		bHit = true;
		// 結果格納			
		pCr->vecCross = vecCross;
		pCr->fDist2Min = fDist2;
		pCr->which = HZX_KIND_WALL | HZX_KIND_DYNAMIC;
		pCr->pBlock = NULL;
		pCr->pSegDynamicWall = pSeg;
		pCr->nStr = 0;
	}
	return bHit;
}

BOOL ONLINE::CheckDynamicFloor( CROSS_DATA* pCr, HZX_D_FLOOR* pSeg, int seg_flag )
{	// CheckOneBlockFlr と似ている
	VECTOR*	pvecFrom = &vecFromOrg;
	VECTOR*	pvecTo = &vecToOrg;

	VECTOR	vecMin;
	VECTOR	vecMax;
	BOOL	bHit = false;

	MakeBound(&vecMin, &vecMax, pvecFrom, pvecTo);
	
	for( ; pSeg != NULL; pSeg = pSeg->next ){
		/* フラグチェック */
		if ( pSeg->atr & ( seg_flag /*| HZX_SEG_SKIP*/ ) ) continue ;
		/* バウンディングチェック */
		if ( ( float )pSeg->b1.vx > vecMax.x ||
			 ( float )pSeg->b1.vy > vecMax.y ||
			 ( float )pSeg->b1.vz > vecMax.z ||
			 ( float )pSeg->b2.vx < vecMin.x ||
			 ( float )pSeg->b2.vy < vecMin.y ||
			 ( float )pSeg->b2.vz < vecMin.z ) continue;

		// ここからストリップチェック
		int nStrVert = pSeg->tag[ 0 ];
		IVECTOR* piv = &pSeg->p1;	// 少し危険
		VECTOR	vecV[4];
		int i;
		for( i = 0; i < nStrVert; i++){
			vecV[i].x = (float)piv->vx;
			vecV[i].y = (float)piv->vy;
			vecV[i].z = (float)piv->vz;
			vecV[i].w = (float)piv->vw;
			piv++;
		}
		// 交点の算出
		float fDistFr, fDistTo;
		if(pSeg->atr & HZX_FLOOR_FLAT){
			fDistFr = pvecFrom->y - vecV[0].y;
			fDistTo = pvecTo->y - vecV[0].y;
		}else{
			VECTOR vecNorm;
			VECTOR vecTmp;
			vecNorm.x = vecV[0].w;
			vecNorm.y = vecV[1].w;
			vecNorm.z = vecV[2].w;
			BP_Vec3_SubVec(&vecTmp, pvecFrom, &vecV[0]);
			fDistFr = BP_Vec3_InnerProduct(&vecTmp, &vecNorm);
			BP_Vec3_SubVec(&vecTmp, pvecTo, &vecV[0]);
			fDistTo = BP_Vec3_InnerProduct(&vecTmp, &vecNorm);
		}
		if( fDistFr * fDistTo >= 0.f ) continue;	// 平面の同じがわにある→あたってない
			
      //BP_MATH - fix divide by zero
      if( fDistFr == fDistTo )
      {
         continue;
      }
      //BP_MATH - fix divide by zero

		VECTOR vecCross;
		float	fRatio;
		fRatio = fDistFr / (fDistFr - fDistTo);
		BP_Vec3_AddVec(&vecCross, &(*pvecTo * fRatio), &(*pvecFrom * (1.f - fRatio) ) );

		// 距離チェック
		float fDist2 = vecCross.Dist2( pvecFrom );
		if( fDist2 >= pCr->fDist2Min ) continue;

		// 交点が内部にあるかチェック
		if( pSeg->atr & HZX_FLOOR_RECT ){
			if( (vecCross.x - vecV[0].x) * (vecCross.x - vecV[2].x) > 0.f ) continue;
			if( (vecCross.z - vecV[0].z) * (vecCross.z - vecV[2].z) > 0.f ) continue;
		} else if( !CheckInside( &vecCross, nStrVert, vecV ) ) continue;

		// あたってる
		nHit++;
		bHit = true;
			
		pCr->vecCross = vecCross;
		pCr->fDist2Min = fDist2;	// これは真の距離sqではない
		pCr->which = HZX_KIND_FLOOR | HZX_KIND_DYNAMIC;
		pCr->pBlock = NULL;
		pCr->pSegDynamicFloor = pSeg;
		pCr->nStr = 0;
	}
	return bHit;
}

int ONLINE::OnlineHazardCheck( HZX_GROUP_ID id, FVECTOR* pvecFrom, FVECTOR* pvecTo,
							   int chk_flag, int seg_flag, int flr_flag )
{
	
    HZX_HDL*	pHzd;
	BOOL		bRecoilOnly, bSegRecoil, bFlrRecoil;
	int			N_Groups;
	
	HZX_GRP*	pGrp;

	int bit, bit2, nGroup, i;
	

	vecFromOrg = *pvecFrom;
	vecToOrg = *pvecTo;
	
    if ( fabsf( pvecFrom->vx - pvecTo->vx ) < 0.01F &&
		fabsf( pvecFrom->vy - pvecTo->vy ) < 0.01F &&
		fabsf( pvecFrom->vz - pvecTo->vz ) < 0.01F ) return 0 ;

    pHzd = HZX_GetCurrentHzx() ;
	
	cr.which = HZX_KIND_NOTHING;
	cr.fDist2Min = FLOAT_MAX;
	
	nHit = 0;
	nInsideBlockCurrent = 0;

	/* サイファー専用チェック */
	if ( chk_flag & HZX_CHK_CYPHER ) {
		seg_flag &= ~HZX_SEG_CYPHER ;
		flr_flag &= ~HZX_FLOOR_CYPHER ;
	} else {
		seg_flag |= HZX_SEG_CYPHER ;
		flr_flag |= HZX_FLOOR_CYPHER ;
	}
	/* 崖あたりチェック */
	if ( chk_flag & HZX_CHK_CLIFF ) {
		seg_flag &= ~HZX_SEG_CLIFF ;
	} else {
		seg_flag |= HZX_SEG_CLIFF ;
	}

	int n = 0;	// グループの数
	id = HZX_AddGroupID( id ) ;
    bit = HZX_AddGroupID( HZX_CurrentGroupID ) | id;
    while( bit != 0 ) {
		nGroup = GV_GetNo( bit ) ; 
		bit2 = GV_GetBit( nGroup ) ;
		if ( nGroup >= 0 && nGroup < pHzd->def->n_groups ) {
			pGrp = pHzd->grp + nGroup ;
			if ( pGrp->n_blocks > 0 ) {
				Group[ n ] = nGroup ; n ++ ;
				CheckBlockBound( pGrp, nGroup ) ;
			}
		}
		bit &= ~bit2 ;
    }
    N_Groups = n ;

	if ( chk_flag & HZX_CHK_RECOIL_TYPE_ONLY ) {
		bRecoilOnly = bSegRecoil = bFlrRecoil = true;
	} else {
		bRecoilOnly = false;
		bSegRecoil = (( seg_flag & HZX_SEG_RECOIL_TYPE  ) || (chk_flag & HZX_CHK_RECOIL_TYPE )) ? true : false ;
		bFlrRecoil = (( flr_flag & HZX_FLOOR_RECOIL_TYPE) || (chk_flag & HZX_CHK_RECOIL_TYPE )) ? true : false ;
	}

	nInsideBlockCurrent = 0;
	
	for ( n = 0; n < N_Groups; n ++ ) {
		int idGroup;
		
		idGroup = Group[ n ] ;
		pGrp = pHzd->grp + idGroup ;
		for ( i = 0; i < N_InsideBlocks[ idGroup ]; i ++ ) {
			int nBox = nInsideBlockCurrent + i ;
         CROSS_DATA crLocal = {0};//BP_COLL - init struct
			BOOL	bHit = false;
			crLocal.fDist2Min = FLOAT_MAX;
			if ( chk_flag & HZX_CHK_F_SEGMENT ) {
				/* 固定壁チェック */
				if ( !bRecoilOnly ) {
					bHit = CheckOneBlockSeg( &crLocal, &bw[ nBox ], seg_flag, false);
				}
				if ( bSegRecoil ) {
					bHit = CheckOneBlockSeg( &crLocal, &bw[ nBox ], seg_flag, true) || bHit;
				}
			}
			if ( chk_flag & HZX_CHK_F_FLOOR ) {
				/* 固定床チェック */
				if ( !bRecoilOnly ) {
					bHit = CheckOneBlockFlr( &crLocal, &bw[ nBox ], flr_flag, false ) || bHit;
				}
				if ( bFlrRecoil ) {
					bHit = CheckOneBlockFlr( &crLocal, &bw[ nBox ], flr_flag, true ) || bHit;
				}
			}
			if(bHit){
				// ローカル結果データの座標変換
				HZX_BLOCK* pBlk = bw[ nBox ].pBlock;
				crLocal.vecCross.x += pBlk->tx;
				crLocal.vecCross.y += pBlk->ty;
				crLocal.vecCross.z += pBlk->tz;

				float fDist2 = pvecFrom->Dist2(&crLocal.vecCross);
				if( fDist2 < cr.fDist2Min ) {
					cr = crLocal;
					cr.fDist2Min = fDist2;
					cr.idGrp = idGroup;
				}
				// このグループではもうチェックが必要ない(from に近いブロックから順に調べられているため)
				break;
			}
		}
		nInsideBlockCurrent += N_InsideBlocks[ idGroup ] ;
		/* 動的壁チェック */
		if ( ( chk_flag & HZX_CHK_D_SEGMENT ) && 
			( pGrp->dynamics->n_segs > 0 ) ) {
			CheckDynamicSegment( &cr, pGrp->dynamics->segs, seg_flag ) ;
		}
		/* 動的床チェック */
		if ( ( chk_flag & HZX_CHK_D_FLOOR ) && 
			( pGrp->dynamics->n_flrs > 0 ) ) {
			CheckDynamicFloor( &cr, pGrp->dynamics->flrs, flr_flag ) ;
		}
    }

	return cr.which & HZX_KIND_WHICHMASK;
}

int ONLINE::OnlineHazardCheckOneSegment( HZX_SEG* pSeg, FVECTOR* pvecFrom, FVECTOR* pvecTo )
{	// ある壁とのオンラインチェック
    static HZX_D_SEGMENT	dseg ;
    IVECTOR			p[ 2 ] ;

    if ( fabsf( pvecFrom->vx - pvecTo->vx ) < 0.10F &&
		 fabsf( pvecFrom->vy - pvecTo->vy ) < 0.10F &&
		 fabsf( pvecFrom->vz - pvecTo->vz ) < 0.10F ) return 0 ;
	
	vecFromOrg = *pvecFrom;
	vecToOrg = *pvecTo;

	cr.which = HZX_KIND_NOTHING;
	cr.fDist2Min = FLOAT_MAX;
	
	nHit = 0;

    p[ 0 ].vx = ( int )pSeg->p1.x ;
    p[ 0 ].vy = ( int )pSeg->p1.y ;
    p[ 0 ].vz = ( int )pSeg->p1.z ;
    p[ 0 ].vw = ( int )pSeg->p1.h ;
	p[ 1 ].vx = ( int )pSeg->p2.x ;
    p[ 1 ].vy = ( int )pSeg->p2.y ;
    p[ 1 ].vz = ( int )pSeg->p2.z ;
    p[ 1 ].vw = ( int )pSeg->p2.h ;

	dseg.callback = NULL ;
	HZX_MoveDynamicSegment( &dseg, &p[ 0 ], &p[ 1 ] ) ;
	dseg.tag[ 0 ] = 2 ;
	dseg.tag[ 1 ] = -1 ;
	dseg.tag[ 2 ] = 0 ;
	dseg.tag[ 3 ] = 0 ;
	dseg.next = NULL ; 
	dseg.atr = 0 ;

	CheckDynamicSegment( &cr, &dseg, 0 ) ;

	return nHit ;
}


//////////////////////

void	ONLINE::GetHazard( HZX_FLR* pSeg, int* pAtr )
{
	if( cr.which == HZX_KIND_NOTHING ) return;

	if ( pAtr != NULL ) *pAtr = GetAtr();
	pSeg->attribute = GetAtr() ;

	if( cr.which & HZX_KIND_WALL ){
		pSeg->type = HZX_TYPE_SEGMENT ;
	} else {
		pSeg->type = HZX_TYPE_FLOOR ;
	}

	if( cr.which & HZX_KIND_DYNAMIC ){
		if( cr.which & HZX_KIND_WALL ){
			// 動的壁
			HZX_D_SEGMENT* pDSeg;
			pSeg->ptr = pDSeg = cr.pSegDynamicWall;
			pSeg->p1.x = (float)pDSeg->p1.vx; pSeg->p1.y = (float)pDSeg->p1.vy;
			pSeg->p1.z = (float)pDSeg->p1.vz; pSeg->p1.h = (float)pDSeg->p1.vw;
			pSeg->p2.x = (float)pDSeg->p2.vx; pSeg->p2.y = (float)pDSeg->p2.vy;
			pSeg->p2.z = (float)pDSeg->p2.vz; pSeg->p2.h = (float)pDSeg->p2.vw;
		}else{
			// 動的床
			HZX_D_FLOOR* pDSeg;
			pSeg->ptr = pDSeg = cr.pSegDynamicFloor;
			pSeg->p1.x = (float)pDSeg->p1.vx; pSeg->p1.y = (float)pDSeg->p1.vy;
			pSeg->p1.z = (float)pDSeg->p1.vz; pSeg->p1.h = (float)pDSeg->p1.vw;
			pSeg->p2.x = (float)pDSeg->p2.vx; pSeg->p2.y = (float)pDSeg->p2.vy;
			pSeg->p2.z = (float)pDSeg->p2.vz; pSeg->p2.h = (float)pDSeg->p2.vw;
			pSeg->p3.x = (float)pDSeg->p1.vx; pSeg->p3.y = (float)pDSeg->p1.vy;
			pSeg->p3.z = (float)pDSeg->p1.vz; pSeg->p3.h = (float)pDSeg->p1.vw;
			pSeg->p4.x = (float)pDSeg->p2.vx; pSeg->p4.y = (float)pDSeg->p2.vy;
			pSeg->p4.z = (float)pDSeg->p2.vz; pSeg->p4.h = (float)pDSeg->tag[ 0 ] ;
		}
	}else{
		if( cr.which & HZX_KIND_WALL ){
			// 静的壁
			SVECTOR* psv;
			psv = cr.pSegStatic->verts + cr.nStr;
			
			pSeg->ptr = NULL;
			pSeg->p1.x = (float)(psv->vx + cr.pBlock->tx);
			pSeg->p1.y = (float)(psv->vy + cr.pBlock->ty);
			pSeg->p1.z = (float)(psv->vz + cr.pBlock->tz);
			pSeg->p1.h = (float)psv->pad; psv++;
			pSeg->p2.x = (float)(psv->vx + cr.pBlock->tx);
			pSeg->p2.y = (float)(psv->vy + cr.pBlock->ty);
			pSeg->p2.z = (float)(psv->vz + cr.pBlock->tz);
			pSeg->p2.h = (float)psv->pad;
		}else{
			// 静的床
			SVECTOR* psv;
			psv = cr.pSegStatic->verts + cr.nStr;
			
			pSeg->ptr = NULL;
			pSeg->p1.x = (float)(psv->vx + cr.pBlock->tx);
			pSeg->p1.y = (float)(psv->vy + cr.pBlock->ty);
			pSeg->p1.z = (float)(psv->vz + cr.pBlock->tz);
			pSeg->p1.h = (float)psv->pad; psv++;
			pSeg->p2.x = (float)(psv->vx + cr.pBlock->tx);
			pSeg->p2.y = (float)(psv->vy + cr.pBlock->ty);
			pSeg->p2.z = (float)(psv->vz + cr.pBlock->tz);
			pSeg->p2.h = (float)psv->pad; psv++;
			pSeg->p3.x = (float)(psv->vx + cr.pBlock->tx);
			pSeg->p3.y = (float)(psv->vy + cr.pBlock->ty);
			pSeg->p3.z = (float)(psv->vz + cr.pBlock->tz);
			pSeg->p3.h = (float)psv->pad; psv++;

			float fTmp = pSeg->p2.h;
			pSeg->p2.h = pSeg->p3.h; pSeg->p3.h = fTmp;

			if((pSeg->p4.h = cr.pSegStatic->b2.pad) != 3) {
				pSeg->p4.x = (float)(psv->vx + cr.pBlock->tx);
				pSeg->p4.y = (float)(psv->vy + cr.pBlock->ty);
				pSeg->p4.z = (float)(psv->vz + cr.pBlock->tz);
			}
		}
	}
}
int		ONLINE::GetHazardFloorType( void )
{
	if( cr.which & HZX_KIND_DYNAMIC ) {
		return cr.pSegDynamicFloor->tag[ 4 ] + 3;
	} else {
		return cr.pSegStatic->b2.pad;
	}
}

void	ONLINE::GetVector( VECTOR* pvec )
{
	GetPoint( pvec );
	*pvec -= vecFromOrg;
}

int		ONLINE::GetAtr( void )
{
	if( cr.which & HZX_KIND_DYNAMIC ){
		if(cr.which & HZX_KIND_FLOOR ){
			return cr.pSegDynamicFloor->atr;									   
		}else {
			return cr.pSegDynamicWall->atr;									   
		}
	} else {
		return cr.pSegStatic->atr;
	}
}

int 	HZX_OnlineHazardCheck( HZX_GROUP_ID id, FVECTOR* pvecFrom, FVECTOR* pvecTo,
							  int chk_flag, int seg_flag, int flr_flag )
{
   memset( &online, 0, sizeof(online) );//BP_COLL - init struct
	return online.OnlineHazardCheck(id, pvecFrom, pvecTo, chk_flag, seg_flag, flr_flag);
}

int		HZX_OnlineHazardCheckOneSegment( HZX_SEG* pSeg, FVECTOR* pvecFrom, FVECTOR* pvecTo )
{
   memset( &online, 0, sizeof(online) );//BP_COLL - init struct
	return online.OnlineHazardCheckOneSegment( pSeg, pvecFrom, pvecTo );
}

/* 結果を取得 */
void	HZX_GetOnlineHazard( HZX_FLR *seg, int *atr )
{
	online.GetHazard(seg, atr);
}
/* 結果を取得 */
void	HZX_GetOnlineHazard2( HZX_FLR *seg )
{
	online.GetHazard(seg, NULL );
}

int	HZX_GetOnlineHazardFloorType( void )
{
	return online.GetHazardFloorType();
}

/* 結果のポインタを取得 */
#if 0	// もともとのソースには意味がない→使ってないのでは
void	HZX_GetOnlineHazardPtr( SVECTOR *v, HZX_BLOCK *blk )
{
    v = HZX_SEG_PTR ;
    blk = HZX_BLOCK_PTR ;
}
#endif

/* ＦＲＯＭ－交差点ベクトルを取得 */
void	HZX_GetOnlineVector( FVECTOR *vect_ptr )
{
	online.GetVector(vect_ptr);
}

/* 結果の交差点座標を取得 */
void	HZX_GetOnlinePoint( FVECTOR *ptp_ptr )
{
	online.GetPoint(ptp_ptr);
}

/* 結果のアトリビュート取得 */
int	HZX_GetOnlineHazardAtr( void )
{
	return online.GetAtr();
}

/* 当たったのが壁か床か */
int	HZX_GetOnlineHazardType( void )
{
	return ( online.GetType() & HZX_KIND_WHICHMASK );
}

/* 当たった壁のグループ */
int	HZX_GetOnlineHazardGroup( void )
{
	return online.GetGroup();
}
