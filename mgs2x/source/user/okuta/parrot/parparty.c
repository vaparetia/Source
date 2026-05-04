/*
    parparty.c
    オウム構造体用関数群（C++でいうところのSet,Get関数,メンバ関数など）
    2001/05/12 Masafumi Okuta
    $Id: parparty.c,v 1.1.1.3 2002/11/19 11:48:06 Yoshizawa1 Exp $
*/

#include "BP_Misc.h"

// 思考初期化チェック
static inline int CheckThinkInit( PARROT* work )
{
    if ( work->bInit ) {
	work->bInit = 0;
	return 1;
    }
    return 0;
}

// 思考レベル１変更
static void PAR_SetThink1( PARROT* work,	// オウムワーク
			   int	 nNewThink1,	// 新しいルーチン:優先レベル
			   int	 nNewThink2,	// 新しいルーチン:フェーズレベル
			   int	 nNewThink3)	// 新しいルーチン:動作レベル
{
    work->think1 = nNewThink1;
    work->think2 = nNewThink2;
    work->think3 = nNewThink3;
    work->count3 = 0;
    work->bInit  = 1;

    PAR_ClearAdjust( work);
}
// 思考レベル２変更
static void PAR_SetThink2( PARROT* work,	// オウムワーク
			   int	 nNewThink2,	// 新しいルーチン:フェーズレベル
			   int	 nNewThink3)	// 新しいルーチン:動作レベル
{
    work->think2 = nNewThink2;
    work->think3 = nNewThink3;
    work->count3 = 0;
    work->bInit  = 1;

    PAR_ClearAdjust( work);
}
// 思考レベル３変更
static void PAR_SetThink3( PARROT* work,	// オウムワーク
			   int	 nNewThink3)	// 新しいルーチン
{
    work->think3 = nNewThink3;
    work->count3 = 0;
    work->bInit  = 1;

    PAR_ClearAdjust( work);
}

// 鳥篭を揺らす
static void PAR_ShakeCage( PARROT* work, int nLevel )
{
    if ( abs( work->vecCageRotAim.vx < 512 ) ) work->vecCageRotAim.vx += MAO_GetRandom( -nLevel, nLevel); 
    if ( abs( work->vecCageRotAim.vz < 512 ) ) work->vecCageRotAim.vz += MAO_GetRandom( -nLevel, nLevel); 
}
// 鳥篭揺れ更新
static void PAR_UpdateCageRot( PARROT* work )
{
    if ( work->nInterpCage-- == 0){
	work->nInterpCage = 4;
	work->vecCageRotAim.vx = -work->vecCageRotAim.vx;
//	work->vecCageRotAim.vy = -work->vecCageRotAim.vy;
	work->vecCageRotAim.vz = -work->vecCageRotAim.vz;

   if ( BP_IsPAL()==TRUE )
	   GV_NearExp8PVPAL( &work->vecCageRotAim, &DG_ZeroSVector, 3 ) ;
   else
	   GV_NearExp8PV( &work->vecCageRotAim, &DG_ZeroSVector, 3 ) ;
    }

   if ( BP_IsPAL()==TRUE )
      GV_NearExp8PVPAL( &work->vecCageRot, &work->vecCageRotAim, 3 ) ;
   else
      GV_NearExp8PV( &work->vecCageRot, &work->vecCageRotAim, 3 ) ;

}
