//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    maoutil.c
    便利関数
    2001/04/04 Masafumi Okuta
    $Id: maoutil.c,v 1.1.1.3 2002/11/19 11:47:48 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "maoutil.h"

#include "bp_matrix.h"

// ２点間の方向ベクトルと距離,Y方向を返すを返す
void MAO_GetDiffVec3( FVECTOR* pvecDiff,	// 方向ベクトル:出力
		      int*     pnDist,		// 差分距離:出力
		      int*     pnDir,		// Y方向:出力
		      FVECTOR* pvecFrom,	// 始点
		      FVECTOR* pvecTo)		// 終点
{
    FVECTOR vec;

    _sceVu0SubVector( &vec, pvecTo, pvecFrom );		// 差分ベクトルを求める
    if ( pvecDiff != NULL ) _sceVu0Normalize( pvecDiff, &vec);	// 方向ベクトル取得
    if ( pnDist != NULL ) (*pnDist) = _FVecLen3( &vec );	// 距離取得
    if ( pnDir != NULL )  (*pnDir)  = _FVecDir2( &vec );	// Y方向取得
}

// 平面チェック
int MAO_FlatCheck(FVECTOR* pvecPos, FVECTOR* pvec1, FVECTOR* pvec2)
{
    if ( pvecPos->vx >= pvec1->vx && 
	 pvecPos->vz >= pvec1->vz && 
	 pvecPos->vx <= pvec2->vx && 
	 pvecPos->vz <= pvec2->vz)
	return 1;
    return 0;
}
// 指定範囲内チェック
int MAO_BoundCheck(FVECTOR* pvecPos, FVECTOR* pvec1, FVECTOR* pvec2)
{
    if ( pvecPos->vx >= pvec1->vx && 
	 pvecPos->vy >= pvec1->vy && 
	 pvecPos->vz >= pvec1->vz && 
	 pvecPos->vx <= pvec2->vx && 
	 pvecPos->vy <= pvec2->vy && 
	 pvecPos->vz <= pvec2->vz)
	return 1;
    return 0;
}

// 指定球内チェック
int MAO_SphereCheck(FVECTOR* pvecPos, FVECTOR* pvecCenter, int nRad)
{
    FVECTOR vec, vecBound1, vecBound2;

    // バウンディングボックス
    vecBound1.vx = pvecCenter->vx - nRad;
    vecBound1.vy = pvecCenter->vy - nRad;
    vecBound1.vz = pvecCenter->vz - nRad;
    vecBound2.vx = pvecCenter->vx + nRad;
    vecBound2.vy = pvecCenter->vy + nRad;
    vecBound2.vz = pvecCenter->vz + nRad;

    // 距離判定前にバウンドチェックである程度弾いておく
    if ( !MAO_BoundCheck( pvecPos, &vecBound1, &vecBound2 ) ) return 0; 

    // 距離判定
    _sceVu0SubVector( &vec, pvecPos, pvecCenter);
    if ( _FVecLen3( &vec ) >= nRad ){
	return 0;
    }

    return 1;
}
void MAO_DbgPlayerPosDump(void)
{
#ifdef DEBUG_MODE
    if ( (GV_PadData[ 0 ].press & PAD_L1) ){
	printf("{ %8.1ff, %8.1ff, %8.1ff }, // \n", GM_PlayerPosition.vx, GM_PlayerPosition.vy, GM_PlayerPosition.vz);
    }
#endif
}
// デバック用マトリクスダンプ
void MAO_DbgDumpMatrix(FMATRIX*	pmat)
{
#ifdef DEBUG_MODE
    printf("-------- dump matrix\n");
    printf("X : %.2f, %.2f, %.2f, %.2f\n", pmat->m[0][0], pmat->m[0][1], pmat->m[0][2], pmat->m[0][3]);
    printf("Y : %.2f, %.2f, %.2f, %.2f\n", pmat->m[1][0], pmat->m[1][1], pmat->m[1][2], pmat->m[1][3]);
    printf("Z : %.2f, %.2f, %.2f, %.2f\n", pmat->m[2][0], pmat->m[2][1], pmat->m[2][2], pmat->m[2][3]);
    printf("T : %.2f, %.2f, %.2f, %.2f\n", pmat->m[3][0], pmat->m[3][1], pmat->m[3][2], pmat->m[3][3]);
#endif
}
void MAO_DbgDumpVector(FVECTOR*	pvec)
{
#ifdef DEBUG_MODE
    printf("%.4ff, %.4ff, %.4ff, %.4ff\n", pvec->vx, pvec->vy, pvec->vz, pvec->vw);
#endif
}
void MAO_DbgDumpSVector(SVECTOR* pvec)
{
#ifdef DEBUG_MODE
    printf("%d, %d, %d\n", pvec->vx, pvec->vy, pvec->vz);
#endif
}


// デバック用マトリクス表示
void MAO_DbgDrawMatrix(FMATRIX*	pmat,
		       float	fLength)
{
    FVECTOR vec[6];
#if 0
    HZX_ViewMatrix( pmat, fLength);
#else
    _sceVu0CopyVector( &vec[0], (FVECTOR*)&pmat->m[3][0]);
    _sceVu0ScaleVector( &vec[1], (FVECTOR*)&pmat->m[0][0], fLength);
    _sceVu0AddVector( &vec[1], &vec[1], &vec[0]);
    NewLineView( &vec[0], 1, 0xff, 0x00, 0x00);
    _sceVu0CopyVector( &vec[2], (FVECTOR*)&pmat->m[3][0]);
    _sceVu0ScaleVector( &vec[3], (FVECTOR*)&pmat->m[1][0], fLength);
    _sceVu0AddVector( &vec[3], &vec[3], (FVECTOR*)&pmat->m[3][0]);
    NewLineView( &vec[2], 1, 0x00, 0xff, 0x00);
    _sceVu0CopyVector( &vec[4], (FVECTOR*)&pmat->m[3][0]);
    _sceVu0ScaleVector( &vec[5], (FVECTOR*)&pmat->m[2][0], fLength);
    _sceVu0AddVector( &vec[5], &vec[5], (FVECTOR*)&pmat->m[3][0]);
    NewLineView( &vec[4], 1, 0x00, 0x00, 0xff);
#endif
}

// デバック用箱表示
void MAO_DbgDrawBox(FVECTOR*	pvecPos,
		    float fX, float fY, float fZ,
		    u_char r, u_char g, u_char b)
{
#ifdef DEBUG_MODE
    FVECTOR vec1, vec2;

    vec1.vx = pvecPos->vx - fX;
    vec1.vy = pvecPos->vy - fY;
    vec1.vz = pvecPos->vz - fZ;
    vec2.vx = pvecPos->vx + fX;
    vec2.vy = pvecPos->vy + fY;
    vec2.vz = pvecPos->vz + fZ;

    NewBoundingBoxView( &vec1, &vec2, r, g, b );
#endif
}

// 指定されたトラップを表示
int MAO_DbgDrawTrap( int nTrap, u_char r, u_char g, u_char b)
{
    FVECTOR     vecFrom, vecTo;
    HZX_BLOCK* 	blk;
    HZX_TRP* 	trp;
	
    HZX_FindTrap(GM_GetHzxGroupID( GM_CurrentMap ), nTrap, &blk, &trp);

    if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	MAO_PRINTF("trap == %d not found!!", nTrap);
#endif
	return 1;
    }

    vecFrom.vx  = blk->tx + trp->b1.vx;
    vecFrom.vy  = blk->ty + trp->b1.vy;
    vecFrom.vz  = blk->tz + trp->b1.vz;
    vecTo.vx    = blk->tx + trp->b2.vx;
    vecTo.vy    = blk->ty + trp->b2.vy;
    vecTo.vz    = blk->tz + trp->b2.vz;
    
    NewBoundingBoxView( &vecFrom, &vecTo, r, g, b );
    return 0;
}

int 	MAO_DbgDrawZone( int hzx_id, int nZone, u_char r, u_char g, u_char b)
{
    FVECTOR     vecFrom, vecTo;
    HZX_ZON* 	pzone;

    extern HZX_ZON *ENE_HZX_GetZone(int addr);

    pzone = ENE_HZX_GetZone( HZX_Address( hzx_id, nZone, nZone ));	

    if ( pzone == NULL ) return 1;

    vecFrom.vx  = pzone->x - pzone->w;
    vecFrom.vy  = pzone->y - 500.f;
    vecFrom.vz  = pzone->z - pzone->h;
    vecTo.vx    = pzone->x + pzone->w;
    vecTo.vy    = pzone->y + 800.f;
    vecTo.vz    = pzone->z + pzone->h;

    NewBoundingBoxView( &vecFrom, &vecTo, r, g, b );
    return 0;
}


// ２つのゾーン間の方向ベクトルを取得
int MAO_GetDirZone2(FVECTOR* 	pvecDir,
		    int 	nFromZone,
		    int 	nToZone)
{
    FVECTOR  vecFrom, vecTo;
    HZX_ZON* pzoneFrom;
    HZX_ZON* pzoneTo;
    
    extern HZX_ZON *ENE_HZX_GetZone(int addr);

    if ( nFromZone == -1 || nToZone == -1 ){
	_sceVu0CopyVector(pvecDir, &DG_ZeroVector);
	return -1;
    }

    pzoneFrom 	= ENE_HZX_GetZone( nFromZone );
    pzoneTo 	= ENE_HZX_GetZone( nToZone );

    vecFrom.vx  = pzoneFrom->x;
    vecFrom.vy  = pzoneFrom->y;
    vecFrom.vz 	= pzoneFrom->z;
    vecFrom.vw 	= 1.f;
    vecTo.vx 	= pzoneTo->x;
    vecTo.vy 	= pzoneTo->y;
    vecTo.vz 	= pzoneTo->z;
    vecTo.vw 	= 1.f;

    _sceVu0SubVector( pvecDir, &vecTo, &vecFrom );		// 差分ベクトルを求める
    _sceVu0Normalize( pvecDir, pvecDir);			// 方向ベクトル取得

    return (0);
}
// ２つのゾーン間の方向を取得
int MAO_GetDirZoneToZone(int nFromZone,
			 int nToZone)
{
    int nDir;
    FVECTOR  vec, vecFrom, vecTo;
    HZX_ZON* pzoneFrom;
    HZX_ZON* pzoneTo;
    
    extern HZX_ZON *ENE_HZX_GetZone(int addr);

    if ( nFromZone == -1 || nToZone == -1 ){
	return 0;
    }

    pzoneFrom 	= ENE_HZX_GetZone( nFromZone );
    pzoneTo 	= ENE_HZX_GetZone( nToZone );

    vecFrom.vx  = pzoneFrom->x;
    vecFrom.vy  = pzoneFrom->y;
    vecFrom.vz 	= pzoneFrom->z;
    vecFrom.vw 	= 1.f;
    vecTo.vx 	= pzoneTo->x;
    vecTo.vy 	= pzoneTo->y;
    vecTo.vz 	= pzoneTo->z;
    vecTo.vw 	= 1.f;

    _sceVu0SubVector( &vec, &vecTo, &vecFrom );		// 差分ベクトルを求める
    _sceVu0Normalize( &vec, &vec);
    nDir = _FVecDir2( &vec );					// 方向取得

    return (nDir);
}

// メインメモリ->スクラッチパッド
void Mao_CopyMemToScr( void *pvMainMem, void *pvScrPad, int nSize, int nNum )
{
    UTL_StartMemToSpr( pvScrPad, pvMainMem, (nSize * nNum) / sizeof(u_long128) );
    UTL_EndSprToMem();
}

// スクラッチパッド->メインメモリ
void Mao_CopyScrToMem( void *pvMainMem, void *pvScrPad, int nSize, int nNum )
{
    extern void UTL_EndMemToSpr( void );
    UTL_StartSprToMem( pvMainMem, pvScrPad, (nSize * nNum) / sizeof(u_long128) );
    UTL_EndMemToSpr();
}

// ２つのゾーンの最短経路上に指定ゾーンがあるかチェック
int MAO_ChkZoneInZ2Z( HZX_GROUP_ID hzx_id, int nZone1, int nZone2, int nAimzone)
{
    int nTmpRes;

    if ( nZone1 == nZone2)   return 0;
    if ( nZone1 == nAimzone) return 1;

    nTmpRes = nZone1;
    while( nTmpRes != NULL){
	nTmpRes = HZX_NextZone( hzx_id, nTmpRes, nZone2);
	if ( nTmpRes == nAimzone ) return 1;
	if ( nTmpRes == nZone2 )   break;
    }

    return 0;
}

// プリミティブワーク解放
DG_PRIM2* MAO_FreePrim2( DG_PRIM2* prim )
{
    if ( prim != NULL ) {
	DG_DequeuePrim2( prim ) ;
	DG_FreePrim2( prim ) ;
	prim = NULL;
    }
    return prim;
}

// RGBA設定
int MAO_SetRGBA( u_char R, u_char G, u_char B, u_char A)
{
    return ( (R<<24) | (G<<16) | (B<<8) | (A) );
}

#ifdef DEBUG_MODE
// メモリいっぱい
static void* _FullCheckMem = NULL;
void* MAO_MemoryFullCreate( void )
{
    int nByte = 5000000;
    void* pvMem = NULL;

    if ( _FullCheckMem != NULL ){
	GV_Free( _FullCheckMem ) ;
	_FullCheckMem = NULL;
    }

    while( nByte > 0 ){
	if ( ( pvMem = (void*)GV_Malloc( nByte )) != NULL) {
	    printf("Memory FULL Check == %d\n", nByte);
	    break;
	} 
	nByte -= 128;
    }
    _FullCheckMem = pvMem;

    return pvMem;
}
void MAO_MemoryFullDestroy( void )
{
    if ( _FullCheckMem != NULL ){
	GV_Free( _FullCheckMem ) ;
	_FullCheckMem = NULL;
    }
}
#else
void* MAO_MemoryFullCreate( void ){ return NULL; }
void MAO_MemoryFullDestroy( void ){}
#endif
