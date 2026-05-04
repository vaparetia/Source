/*
  level.cpp

  天井・床チェック
  
  2001/04/23 M.Kobayashi

  $Id: level.cpp,v 1.1.1.3 2002/11/19 11:42:47 Yoshizawa1 Exp $

 */

#include <stdio.h>

#include "mgs_type.h"
#include "libgv.h"
#include "libhzx.h"

#include "bp_vector.h"

struct ANSWER {
	float fHeight;
	union {
		HZX_VuSEG* pSegStatic;		// 見つかったセグメント
		HZX_D_FLOOR* pSegDynamic;	// 見つかったセグメント
	};
	int	   		nStr;	// ストリップ内順番
	HZX_BLOCK*	pBlk;	// ブロック
};


class LEVEL {
public:
	enum {
		FIND_FLOOR	= 0x01,
		FIND_CEIL	= 0x02,
		FIND_TYPE_MASK	= 0x03,
		FIND_D_FLOOR	= 0x04,
		FIND_D_CEIL		= 0x08,
	};
	int	s32FlgHit;	// あたった結果フラグ（上記enum）
	VECTOR	vecFromOrg;		// 判定位置

	ANSWER	aFloor;
	ANSWER	aCeil;

	void CheckOneBlock( HZX_BLOCK* pBlk, HZX_VuSEG* pSeg, int nSeg, int flr_flag);
	void CheckDynamicFloor( HZX_D_FLOOR* pSeg, int flr_flag);
	int LevelHazardCheck( HZX_GROUP_ID id, FVECTOR* pvecFrom, int chk_flag, int flr_flag );

	// 結果取得
	void GetHazard( HZX_FLR* pFlr, int *pAtr );
	void GetPtr( SVECTOR **ppSv, HZX_BLOCK **ppBlk );
	void GetType( int* type );
};



#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG_MODE
int		HZX_LevelDebug = 0 ;
#endif

#ifdef __cplusplus
}
#endif


/* ---------------------------------- */

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

static float CalcHeight( VECTOR* pvecFrom, VECTOR* vecV )
{	// 一般の平面で交点の高さを決定する

	VECTOR vecNormal;
	float result ;

	vecNormal( vecV[0].w, vecV[1].w, vecV[2].w );

	result = vecV[0].y -
	  (( pvecFrom->x - vecV[0].x ) * vecNormal.vx +
	   ( pvecFrom->z - vecV[0].z ) * vecNormal.vz ) / vecNormal.y ;

#if 0 /* ＸＢＯＸでの演算結果が同じにならないが、とりあえずここは最終手段 T.Morita */
	*(int*)&result = (((*(int*)&result + 1) & 0x007fffff) |   /* 仮数部 */
					  ( *(int*)&result      & 0xff800000) ) ; /* 指数部 */
#endif

	return result ;
}
/* ---------------------------------- */
void LEVEL::CheckOneBlock( HZX_BLOCK* pBlk, HZX_VuSEG* pSeg, int nSeg, int flr_flag)
{	// １ブロック内チェック
	// 基準位置ローカライズ
	FVECTOR vecFromLocal;
	float fFloorHLocal;
	float fCeilHLocal;
	vecFromLocal.x = vecFromOrg.x - (float)pBlk->tx;
	vecFromLocal.y = vecFromOrg.y - (float)pBlk->ty;
	vecFromLocal.z = vecFromOrg.z - (float)pBlk->tz;
	fFloorHLocal = aFloor.fHeight - (float)pBlk->ty;
	fCeilHLocal = aCeil.fHeight - (float)pBlk->ty;

	for( ; nSeg > 0; nSeg--, pSeg++ ) {
		// フラグチェック
		if ( pSeg->atr & flr_flag ) continue;
		// バウンディングチェック
		if ( pSeg->b1.vx >  vecFromLocal.x || pSeg->b2.vx < vecFromLocal.x
			 || pSeg->b1.vz >  vecFromLocal.z || pSeg->b2.vz < vecFromLocal.z ) continue;
		if ( (float)pSeg->b1.vy > fCeilHLocal || (float)pSeg->b2.vy < fFloorHLocal ) continue;

		// ここからストリップチェック
		int nTotalVert = pSeg->b1.pad;
		int nStrVert = pSeg->b2.pad;
		int i, j;
		SVECTOR* psv;
		VECTOR	vecV[4];
		for( psv = pSeg->verts, j = 0 ; j < nTotalVert ; j += nStrVert ){
			for( i = 0; i < nStrVert; i++){
				vecV[i].x = (float)psv->vx;
				vecV[i].y = (float)psv->vy;
				vecV[i].z = (float)psv->vz;
				vecV[i].w = (float)psv->pad;
				psv++;
			}

			// 交点が内部にあるかチェック
			if( pSeg->atr & HZX_FLOOR_RECT ){
				if( (vecFromLocal.x - vecV[0].x) * (vecFromLocal.x - vecV[2].x) > 0.f ) continue;
				if( (vecFromLocal.z - vecV[0].z) * (vecFromLocal.z - vecV[2].z) > 0.f ) continue;
			} else if( !CheckInside( &vecFromLocal, nStrVert, vecV ) ) continue;

			// 高さ計算
			float fH;
			if ( pSeg->atr & HZX_FLOOR_FLAT ) {
				fH = vecV[0].y;
			} else fH = CalcHeight( &vecFromLocal, vecV );

			if( fH - vecFromLocal.y > 0.f ) {
				if( fCeilHLocal <= fH ) continue;
				// 天井発見
				fCeilHLocal = fH;
				aCeil.fHeight = fH + (float)pBlk->ty;	// 絶対座標に戻しておく
				aCeil.pSegStatic = pSeg;
				aCeil.nStr = j;
				aCeil.pBlk = pBlk;
				s32FlgHit |= FIND_CEIL;
			} else {
				if( fFloorHLocal >= fH ) continue;


				// 床発見
				fFloorHLocal = fH;
				aFloor.fHeight = fH + (float)pBlk->ty;// 絶対座標に戻しておく
				aFloor.pSegStatic = pSeg;
				aFloor.nStr = j;
				aFloor.pBlk = pBlk;
				s32FlgHit |= FIND_FLOOR;
			}
		}
	}
}

void LEVEL::CheckDynamicFloor( HZX_D_FLOOR* pSeg, int flr_flag)
{
	for( ; pSeg != NULL; pSeg = pSeg->next ){
		/* フラグチェック */
		if ( pSeg->atr & ( flr_flag | HZX_FLOOR_SKIP ) ) continue;
		/* バウンディングチェック */
		if ( pSeg->b1.vx > vecFromOrg.vx || pSeg->b2.vx < vecFromOrg.vx ||
			 pSeg->b1.vz > vecFromOrg.vz || pSeg->b2.vz < vecFromOrg.vz ) continue;
		if ( (float)pSeg->b1.vy > aCeil.fHeight || (float)pSeg->b2.vy < aFloor.fHeight ) continue;

		VECTOR	vecV[4];
		vecV[0].x = (float)pSeg->p1.vx; vecV[0].y = (float)pSeg->p1.vy;
		vecV[0].z = (float)pSeg->p1.vz; vecV[0].w = (float)pSeg->p1.vw;
		vecV[1].x = (float)pSeg->p2.vx; vecV[1].y = (float)pSeg->p2.vy;
		vecV[1].z = (float)pSeg->p2.vz; vecV[1].w = (float)pSeg->p2.vw;
		vecV[2].x = (float)pSeg->p3.vx; vecV[2].y = (float)pSeg->p3.vy;
		vecV[2].z = (float)pSeg->p3.vz; vecV[2].w = (float)pSeg->p3.vw;
		vecV[3].x = (float)pSeg->p4.vx; vecV[3].y = (float)pSeg->p4.vy;
		vecV[3].z = (float)pSeg->p4.vz; vecV[3].w = (float)pSeg->p4.vw;

		// 交点が内部にあるかチェック
		if( pSeg->atr & HZX_FLOOR_RECT ){
			if( (vecFromOrg.x - vecV[0].x) * (vecFromOrg.x - vecV[2].x) > 0.f ) continue;
			if( (vecFromOrg.z - vecV[0].z) * (vecFromOrg.z - vecV[2].z) > 0.f ) continue;
		} else if( !CheckInside( &vecFromOrg, pSeg->tag[ 0 ] , vecV ) ) continue;
		
		// 高さ計算
		float fH;
		if ( pSeg->atr & HZX_FLOOR_FLAT ) {
			fH = vecV[0].y;
		} else fH = CalcHeight( &vecFromOrg, vecV );

		if( fH - vecFromOrg.y > 0.f ) {
			if( aCeil.fHeight <= fH ) continue;
			// 天井発見
			aCeil.fHeight = fH;
			aCeil.pSegDynamic = pSeg;
			s32FlgHit |= FIND_CEIL | FIND_D_CEIL;
		} else {
			if( aFloor.fHeight >= fH ) continue;
			// 床発見
			aFloor.fHeight = fH;
			aFloor.pSegDynamic = pSeg;
			s32FlgHit |= FIND_FLOOR | FIND_D_FLOOR;
		}
	}
}


int LEVEL::LevelHazardCheck( HZX_GROUP_ID id, FVECTOR* pvecFrom, int chk_flag, int flr_flag )
{
	HZX_HDL* pHzd ;
	int		i, nGroup;
	BOOL	bRecoil, bOnlyRecoil;
	HZX_GRP* pGrp;

	vecFromOrg = *pvecFrom;
	aFloor.fHeight = -FLOAT_MAX;
	aCeil.fHeight = FLOAT_MAX;

	pHzd = HZX_GetCurrentHzx() ;
	s32FlgHit = 0 ;

	if ( chk_flag & HZX_CHK_RECOIL_TYPE_ONLY ) {
		bOnlyRecoil = bRecoil = true;
	} else {
		bOnlyRecoil = false;
		bRecoil = ( ( flr_flag & HZX_FLOOR_RECOIL_TYPE ) || ( chk_flag & HZX_CHK_RECOIL_TYPE ) ) ? true : false ;
	}

	/* サイファー専用チェック */
	if ( chk_flag & HZX_CHK_CYPHER ) {
		flr_flag &= ~HZX_FLOOR_CYPHER ;
	} else {
		flr_flag |= HZX_FLOOR_CYPHER ;
	}

	id = HZX_AddGroupID( id ) ;

    while( id != 0 ){
		nGroup = GV_GetNo( id );
		id &= ~GV_GetBit( nGroup );
#ifdef DEBUG
		if ( nGroup < 0 || nGroup >= pHzd->def->n_groups ) {
			printf( "warning : groupNo < 0 || groupNo > Max\n : %d", nGroup ) ;
			continue ;
		}
#endif
		pGrp = pHzd->grp + nGroup ;
		if ( pGrp->n_blocks > 0 && 
			 chk_flag & HZX_CHK_F_FLOOR ) {/* 固定床チェック */
			// ブロック基準位置の確定
			int x, y, z;
			x = ( ( int )pvecFrom->vx - pGrp->bound_min_x ) / pGrp->block_size_x ;
			y = ( ( int )pvecFrom->vy - pGrp->bound_min_y ) / pGrp->block_size_y ;
			z = ( ( int )pvecFrom->vz - pGrp->bound_min_z ) / pGrp->block_size_z ;
			

			if( x >= 0 && x < pGrp->div_x && z >= 0 && z < pGrp->div_z ) {

				if( y >= pGrp->div_y ) y = pGrp->div_y - 1;
				if( y < 0 )  y = 0;

				int step = pGrp->div_x * pGrp->div_z;
				int ofst = pGrp->div_x * z + x;
				HZX_BLOCK* pBlk;
				// 基準ブロックの検査
				pBlk = pGrp->blocks + (ofst + step * y);
				if( !bOnlyRecoil && pBlk->n_flrs > 0 ) CheckOneBlock( pBlk, pBlk->flrs, pBlk->n_flrs, flr_flag );
				if( bRecoil && pBlk->n_bul_flrs > 0 ) CheckOneBlock( pBlk, pBlk->bul_flrs, pBlk->n_bul_flrs,
																	 flr_flag );
				
				// 下方向に床検査
				if( !(chk_flag & HZX_CHK_NOCHECK_FLOOR ) ) {
					for ( i = y - 1; i >= 0/* && !(s32FlgHit & FIND_FLOOR)*/ ; i-- ){
						pBlk = pGrp->blocks + (ofst + step * i);
						if( !bOnlyRecoil && pBlk->n_flrs > 0 ) CheckOneBlock( pBlk, pBlk->flrs, pBlk->n_flrs, flr_flag );
						if( bRecoil && pBlk->n_bul_flrs > 0 ) CheckOneBlock( pBlk, pBlk->bul_flrs, pBlk->n_bul_flrs,
																			 flr_flag );
					}
				}
				
				// 上方向に天井検査
				if( !(chk_flag & HZX_CHK_NOCHECK_CEIL ) ) {
					for ( i = y + 1; i < pGrp->div_y /*&& !(s32FlgHit & FIND_CEIL)*/ ; i++ ){
						pBlk = pGrp->blocks + (ofst + step * i);
						if( !bOnlyRecoil && pBlk->n_flrs > 0 ) CheckOneBlock( pBlk, pBlk->flrs, pBlk->n_flrs, flr_flag );
						if( bRecoil && pBlk->n_bul_flrs > 0 ) CheckOneBlock( pBlk, pBlk->bul_flrs, pBlk->n_bul_flrs,
																			 flr_flag );
					}
				}
			}
		}
		/* 動的床チェック */
		if ( ( chk_flag & HZX_CHK_D_FLOOR ) &&
			( pGrp->dynamics->n_flrs > 0 ) ) {
			CheckDynamicFloor( pGrp->dynamics->flrs, flr_flag ) ;
		}
	}
	return s32FlgHit & FIND_TYPE_MASK;
}

static	void	SetFloor( HZX_FLR* pFlr, ANSWER* pAns)
{
	HZX_BLOCK* pBlk = pAns->pBlk;
	SVECTOR* psv = pAns->pSegStatic->verts + pAns->nStr;
	pFlr->p1.x = (float)(psv->vx + pBlk->tx);
	pFlr->p1.y = (float)(psv->vy + pBlk->ty);
	pFlr->p1.z = (float)(psv->vz + pBlk->tz);
	pFlr->p1.h = (float)psv->pad; psv++;
	pFlr->p2.x = (float)(psv->vx + pBlk->tx);
	pFlr->p2.y = (float)(psv->vy + pBlk->ty);
	pFlr->p2.z = (float)(psv->vz + pBlk->tz);
	pFlr->p2.h = (float)psv->pad; psv++;
	pFlr->p3.x = (float)(psv->vx + pBlk->tx);
	pFlr->p3.y = (float)(psv->vy + pBlk->ty);
	pFlr->p3.z = (float)(psv->vz + pBlk->tz);
	pFlr->p3.h = (float)psv->pad; psv++;

	float fTmp = pFlr->p2.h;
	pFlr->p2.h = pFlr->p3.h; pFlr->p3.h = fTmp;
	pFlr->p4.h = pAns->pSegStatic->b2.pad;

	if( pAns->pSegStatic->b2.pad == 3 ) return;
	pFlr->p4.x = (float)(psv->vx + pBlk->tx);
	pFlr->p4.y = (float)(psv->vy + pBlk->ty);
	pFlr->p4.z = (float)(psv->vz + pBlk->tz);
}

extern "C" void	GV_IVtoFV(void *src, void *dst, int n ) ;
static	void	SetFloorD( HZX_FLR *flr, HZX_D_FLOOR *dflr )
{
    float	tmp ;
	
    GV_IVtoFV( &dflr->p1, &flr->p1, 4 ) ;
    GV_IVtoFV( &dflr->p2, &flr->p2, 4 ) ;
    GV_IVtoFV( &dflr->p3, &flr->p3, 4 ) ;
    GV_IVtoFV( &dflr->p4, &flr->p4, 3 ) ;
    tmp = flr->p1.y ; flr->p1.y = flr->p1.z ; flr->p1.z = tmp ;
    tmp = flr->p2.y ; flr->p2.y = flr->p2.z ; flr->p2.z = tmp ;
    tmp = flr->p3.y ; flr->p3.y = flr->p3.z ; flr->p3.z = tmp ;
    tmp = flr->p4.y ; flr->p4.y = flr->p4.z ; flr->p4.z = tmp ;
    tmp = flr->p2.h ; flr->p2.h = flr->p3.h ; flr->p3.h = tmp ;
	flr->p4.h = (float)dflr->tag[ 0 ] ;
}

void LEVEL::GetHazard( HZX_FLR* pFlr, int *pAtr )
{
	if( s32FlgHit & FIND_FLOOR ){
		if( s32FlgHit & FIND_D_FLOOR ){
			SetFloorD( pFlr, aFloor.pSegDynamic );
			pFlr->ptr = aFloor.pSegDynamic;
			if(pAtr != NULL) *pAtr = aFloor.pSegDynamic->atr;
			pFlr->attribute = aFloor.pSegDynamic->atr;
		} else {
			SetFloor( pFlr, &aFloor ) ;
			pFlr->ptr = NULL;
			if(pAtr != NULL) *pAtr = aFloor.pSegStatic->atr;
			pFlr->attribute = aFloor.pSegStatic->atr;
		}
		pFlr->type = HZX_TYPE_FLOOR ;
	}
	pFlr++; if(pAtr != NULL) pAtr++;
	if( s32FlgHit & FIND_CEIL ){
		if( s32FlgHit & FIND_D_CEIL ){
			SetFloorD( pFlr, aCeil.pSegDynamic );
			if(pAtr != NULL) *pAtr = aCeil.pSegDynamic->atr;
			pFlr->attribute = aCeil.pSegDynamic->atr;
			pFlr->ptr = aCeil.pSegDynamic;
		} else {
			SetFloor( pFlr, &aCeil ) ;
			pFlr->ptr = NULL;
			if(pAtr != NULL) *pAtr = aCeil.pSegStatic->atr;
			pFlr->attribute = aCeil.pSegStatic->atr;
		}
		pFlr->type = HZX_TYPE_FLOOR ;
	}
}

void LEVEL::GetPtr( SVECTOR **ppSv, HZX_BLOCK **ppBlk )
{
	if( s32FlgHit & FIND_FLOOR ){
		if( s32FlgHit & FIND_D_FLOOR ){
			*ppSv = NULL;
			*ppBlk = NULL;
		} else {
			*ppSv = aFloor.pSegStatic->verts + aFloor.nStr;
			*ppBlk = aFloor.pBlk;
		}
	}
	if( s32FlgHit & FIND_CEIL ){
		if( s32FlgHit & FIND_D_CEIL ){
			*ppSv = NULL;
			*ppBlk = NULL;
		} else {
			*ppSv = aCeil.pSegStatic->verts + aCeil.nStr;
			*ppBlk = aCeil.pBlk;
		}
	}
}

void LEVEL::GetType( int* type )
{
	if( s32FlgHit & FIND_FLOOR ){
		if ( s32FlgHit & FIND_D_FLOOR ) {
			type[0] = aFloor.pSegDynamic->tag[0];
		} else {
			type[0] = aFloor.pSegStatic->b2.pad;
		}
	}
	if( s32FlgHit & FIND_CEIL ){
		if ( s32FlgHit & FIND_D_CEIL ) {
			type[1] = aCeil.pSegDynamic->tag[0];
		} else {
			type[1] = aCeil.pSegStatic->b2.pad;
		}
	}
}


////////////////////////////////////////////
static LEVEL level;

int		HZX_LevelHazardCheck( HZX_GROUP_ID id, FVECTOR *from, int chk_flag, int flr_flag )
{
	return level.LevelHazardCheck( id, from, chk_flag, flr_flag );
}

/* 結果を取得 */
void	HZX_GetLevelHazard( HZX_FLR *flrs, int *atrs )
{
	level.GetHazard( flrs, atrs );
}

/* 結果のポインタを取得 */
void	HZX_GetLevelHazardPtr( SVECTOR **flr, HZX_BLOCK **blk )
{
	level.GetPtr(flr, blk);
}

/* 結果の形状を取得（３ ＯＲ ４） */
void	HZX_GetLevelHazardType( int *type )
{
	level.GetType( type );
}

/* 結果の高さのみ取得 */
void	HZX_GetLevelHeight( float *lvl_ptr )
{
	lvl_ptr[ 0 ] = level.aFloor.fHeight;
	lvl_ptr[ 1 ] = level.aCeil.fHeight;
}

/* 結果の床高さを取得 */
float	HZX_GetFloorLevel( void )
{
	return level.aFloor.fHeight;
}
/*----------------------------------------------------------------*/

/*
   ３Ｄベクトルを障害ベクトルに変換
   */
static	void	FV_to_HV(
		FVECTOR		*sv ,
		HZX_FVEC	*hv )
{
    hv->x = (float)sv->vx ;
    hv->y = (float)sv->vy ;
    hv->z = (float)sv->vz ;
}

/*
   傾斜型ユカの高さを計算
   */
static	float	SlopeFloorLevel(
		HZX_FLR		*flr ,
		HZX_FVEC	*from )
{
    float		pf_n, h ;
    FDVECTOR		flr_p1, p1_from, normal, *vfrom ;

    flr_p1.vx = flr->p1.x ;
    flr_p1.vy = flr->p1.z ;

    vfrom = ( FDVECTOR * )from ;
    p1_from.vx = vfrom->vx - flr_p1.vx ;
    p1_from.vy = vfrom->vy - flr_p1.vy ;

    normal.vx = flr->p1.h ;
    normal.vy = flr->p2.h ;
    {
		FVECTOR		f1, f2, f0 ;

		f1.vx = p1_from.vx ;
		f1.vy = p1_from.vy ;
		f1.vz = f1.vw = 0 ;
		f2.vx = - normal.vy ;
		f2.vy = normal.vx ;
		f2.vz = f2.vw = 0 ;
		_sceVu0OuterProduct( &f0, &f1, &f2 ) ;
		pf_n = f0.vz ;
    }
	h = flr->p3.h ? (float)flr->p3.h : 0.00001f ;
    h = ( float )flr->p1.y - pf_n / h ;
    return h ;
}

/* 傾斜床の高さを調べる２ */
void	HZX_SlopeFloorLevel( float *h, FVECTOR *mov, HZX_FLR *flr )
{
    HZX_FVEC	from ;

    FV_to_HV( mov, &from ) ;
    *h = SlopeFloorLevel( flr, &from ) ;
}

/*
   現在のブロック
   */
HZX_BLOCK	*HZX_GetInsideBlock( HZX_GROUP_ID id, FVECTOR *mov )
{
    HZX_GRP	*grp ;
    HZX_BLOCK	*blk ;
	HZX_HDL *hzd;
    int		i, flag ;
    int		x, y, z, block ;

	hzd = HZX_GetCurrentHzx();
    grp = hzd->grp ;
    for ( i = 0 ;i < hzd->def->n_groups; i ++, grp ++ ) {
		flag = id & GV_GetBit( i ) ;
		if ( !flag || grp->n_blocks == 0 ) continue ;
		blk = grp->blocks ;
		x = ( int )( ( mov->vx - grp->bound_min_x ) / grp->block_size_x ) ;
		z = ( int )( ( mov->vz - grp->bound_min_z ) / grp->block_size_z ) ;
		y = ( int )( ( mov->vy - grp->bound_min_y ) / grp->block_size_y ) ;
		if ( x < 0 || y < 0 || z < 0 || x >= grp->div_x ||
			z >= grp->div_z || z >= grp->div_z ) return NULL ;
		block = grp->div_x * z + x + y * grp->div_x * grp->div_z ;
		if ( block >= 0 && block < grp->n_blocks ) return ( blk + block ) ;
    }
    return NULL ;
}

/* バウンディング計算 */
void	HZX_GetBounding( HZX_HZD *hzd, FVECTOR *max, FVECTOR *min )
{
	VECTOR	vec1, vec2;

	if ( hzd->type == HZX_TYPE_SEGMENT ) {
		vec1( hzd->p1.x, hzd->p1.y + hzd->p1.h, hzd->p1.z );
		vec2( hzd->p2.x, hzd->p2.y + hzd->p2.h, hzd->p2.z );
		BP_Vec3_MaxVec(max, &vec1, &vec2);
		vec1.y = hzd->p1.y;
		vec2.y = hzd->p2.y;
		BP_Vec3_MinVec(min, &vec1, &vec2);
	} else {
		vec1( hzd->p1.x, hzd->p1.y, hzd->p1.z ); 
		vec2( hzd->p2.x, hzd->p2.y, hzd->p2.z );
		BP_Vec3_MaxVec(max, &vec1, &vec2);
		BP_Vec3_MinVec(min, &vec1, &vec2);
		vec1( hzd->p3.x, hzd->p3.y, hzd->p3.z ); 
		BP_Vec3_MaxVec(max, max, &vec1);
		BP_Vec3_MinVec(min, min, &vec1);

		if ( hzd->p4.h == 3 ) return ;

		vec1( hzd->p4.x, hzd->p4.y, hzd->p4.z ); 
		BP_Vec3_MaxVec(max, max, &vec1);
		BP_Vec3_MinVec(min, min, &vec1);
	}
}
