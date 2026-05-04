//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    waterdust.c
    水中ゴミ
    2001/07/16 Masafumi Okuta
    $Id: waterdust.c,v 1.1.1.3 2002/11/19 11:47:54 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"
#include	"wdust.h"


#ifdef DEBUG_MODE
// 処理時間ダンプ
static void DbgDumpExecTime( int nTime)
{
#if 0
    DEBUG_Locate( 240, 10, 0 );
    DEBUG_Printf("WATERDUST-EXECTIME = %.3f\n", (float)nTime / 60.f / 262.f );
//    printf("WATERDUST-EXECTIME = %.3f\n", (float)nTime / 60.f / 262.f );
#endif
}
#endif


// プレイヤーの関節データをVUにロード
static inline void LoadPlayerNodeData( WATERDUST* work )
{
    /*
      vf20～vf27は他の関数に壊されてはならない
     */
#ifdef BP_PSX2_ASM  // yano

    asm volatile ("
        # 入力
        lqc2		vf20,  0x00(%0)		# 左手移動量
        lqc2		vf21,  0x10(%0)		# 右手移動量
        lqc2		vf22,  0x20(%0)		# 左手移動量
        lqc2		vf23,  0x30(%0)		# 右足移動量
        lqc2		vf24,  0x00(%1)		# (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_TE).m[3][0]
        lqc2		vf25,  0x10(%1)		# (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_MIGI_TE).m[3][0]
        lqc2		vf26,  0x20(%1)		# (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_KAKATO).m[3][0]
        lqc2		vf27,  0x30(%1)		# (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_MIGI_KAKATO).m[3][0]
    ":: "r"(&work->wdmng->matPlMove), "r"(&work->wdmng->matPlNode));
#endif
}

// プレイヤーからの影響ベクトルを算出
static inline void GetPlayerConvection( WATERDUST* work, float* pfAffectDist, FVECTOR* pvecConv, FVECTOR* pvecPos)
{
    FVECTOR vecDist;

    FVECTOR vecAffect;
    FVECTOR vecResist;
    FVECTOR vecPlMove;

    FMATRIX matDiff;

    /*
      vf20～vf27は予約されている
      LoadPlayerNodeDataでプレイヤーの関節関連のデータが入っている
     */
#ifdef BP_PSX2_ASM  // yano /*未検証*/

    asm volatile ("
        # 入力
        lqc2		vf03,  0x00(%2)		# ゴミの位置をロード
        # 差分ベクトルを作成
        vsub.xyz	vf14,  vf03,  vf24 	# 左手とゴミの差分ベクトルを作成
        vsub.xyz	vf15,  vf03,  vf25 	# 右手とゴミの差分ベクトルを作成
        vsub.xyz	vf16,  vf03,  vf26 	# 左足とゴミの差分ベクトルを作成
        vsub.xyz	vf17,  vf03,  vf27 	# 右足とゴミの差分ベクトルを作成
        vmulx.wxyz	vf18,  vf00,  vf00x	# ZeroVector作成
        # 左手との距離,方向ベクトルを計算
        vmul.xyz        vf13,  vf14,  vf14	# 差分ベクトルの２乗
        vaddy.x		vf13,  vf13,  vf13y	# X成分 + Y成分
        vaddz.x		vf13,  vf13,  vf13z	# X成分 + z成分
        vsqrt		   Q,  vf13x		# 距離をQレジスタに格納
        vwaitq					# Qレジスタ待ち
        vaddq.x         vf12,  vf18x, Q		# 距離取得
        vnop					# EEのQをリードする命令とCOP2の除算命令とのバグで追加
        vnop					# （2命令待たないと例外発生した時に命令が逆転するケースがある 2001/09/11）
        vdiv               Q,  vf00w, vf12x	# 1.f / 距離
        vsub.wxyz	vf08,  vf00,  vf00	# クリア
        vmul.xyz        vf13,  vf15,  vf15	# 差分ベクトルの２乗 waitq命令のかわりに次の奴を先行計算
        vaddy.x		vf13,  vf13,  vf13y	# X成分 + Y成分
        vaddz.x		vf13,  vf13,  vf13z	# X成分 + z成分
        vwaitq					# Qレジスタ待ち(略可)
        vmulq.xyz	vf08,  vf14,  Q		# 正規化
        # 右手との距離,方向ベクトルを計算
        vsqrt		   Q,  vf13x
        vwaitq					# Qレジスタ待ち
        vaddq.y         vf12,  vf18y, Q		# 距離取得
        vnop					# EEのQをリードする命令とCOP2の除算命令とのバグで追加
        vnop					# （2命令待たないと例外発生した時に命令が逆転するケースがある 2001/09/11）
        vdiv               Q,  vf00w, vf12y	# 1.f / 距離	
        vsub.wxyz	vf09,  vf00,  vf00	# クリア
        vmul.xyz        vf13,  vf16,  vf16	# 差分ベクトルの２乗 waitq命令のかわりに次の奴を先行計算
        vaddy.x		vf13,  vf13,  vf13y	# X成分 + Y成分
        vaddz.x		vf13,  vf13,  vf13z	# X成分 + z成分
        vwaitq					# Qレジスタ待ち(略可)
        vmulq.xyz	vf09,  vf15,  Q		# 正規化
        # 左足との距離,方向ベクトルを計算
        vsqrt		   Q,  vf13x
        vwaitq					# Qレジスタ待ち
        vaddq.z         vf12,  vf18z, Q		# 距離取得
        vnop					# EEのQをリードする命令とCOP2の除算命令とのバグで追加
        vnop					# （2命令待たないと例外発生した時に命令が逆転するケースがある 2001/09/11）
        vdiv               Q,  vf00w, vf12z	# 1.f / 距離	
        vsub.wxyz	vf10,  vf00,  vf00	# クリア
        vmul.xyz        vf13,  vf17,  vf17	# 差分ベクトルの２乗 waitq命令のかわりに次の奴を先行計算
        vaddy.x		vf13,  vf13,  vf13y	# X成分 + Y成分
        vaddz.x		vf13,  vf13,  vf13z	# X成分 + z成分
        vwaitq					# Qレジスタ待ち(略可？)
        vmulq.xyz	vf10,  vf16,  Q		# 正規化
        # 右足との距離,方向ベクトルを計算
        vsqrt		   Q,  vf13x
        vwaitq					# Qレジスタ待ち
        vaddq.w         vf12,  vf18w, Q		# 距離取得
        vnop					# EEのQをリードする命令とCOP2の除算命令とのバグで追加
        vnop					# （2命令待たないと例外発生した時に命令が逆転するケースがある 2001/09/11）
        vdiv               Q,  vf00w, vf12w	# 1.f / 距離	
        vsub.wxyz	vf11,  vf00,  vf00	# クリア
        vwaitq					# Qレジスタ待ち
        vmulq.xyz	vf11,  vf17,  Q		# 正規化
        # 出力
        sqc2		vf08,  0x00(%0)		# matDiff.m[0][0]
        sqc2		vf09,  0x10(%0)		# matDiff.m[1][0]
        sqc2		vf10,  0x20(%0)		# matDiff.m[2][0]
        sqc2		vf11,  0x30(%0)		# matDiff.m[3][0]
        sqc2		vf12,  0x00(%1)		# vecDist
    ":: "r"(&matDiff), "r"(&vecDist),
	"r"(pvecPos) );
#else
	FVECTOR fvtemp;
	float ftemp;

	_sceVu0SubVector( &fvtemp, pvecPos, &work->wdmng->matPlNode.m[0] );
	ftemp = fvtemp.vx*fvtemp.vx + fvtemp.vy*fvtemp.vy + fvtemp.vz*fvtemp.vz ;
	vecDist.vx = DG_SQRT( ftemp );/*距離*/
	ftemp = 1.0f / vecDist.vx;
	_sceVu0ScaleVectorXYZ( &matDiff.m[0], &fvtemp, ftemp );/*正規化*/

	_sceVu0SubVector( &fvtemp, pvecPos, &work->wdmng->matPlNode.m[1] );
	ftemp = fvtemp.vx*fvtemp.vx + fvtemp.vy*fvtemp.vy + fvtemp.vz*fvtemp.vz ;
	vecDist.vy = DG_SQRT( ftemp );/*距離*/
	ftemp = 1.0f / vecDist.vy;
	_sceVu0ScaleVectorXYZ( &matDiff.m[1], &fvtemp, ftemp );/*正規化*/

	_sceVu0SubVector( &fvtemp, pvecPos, &work->wdmng->matPlNode.m[2] );
	ftemp = fvtemp.vx*fvtemp.vx + fvtemp.vy*fvtemp.vy + fvtemp.vz*fvtemp.vz ;
	vecDist.vz = DG_SQRT( ftemp );/*距離*/
	ftemp = 1.0f / vecDist.vz;
	_sceVu0ScaleVectorXYZ( &matDiff.m[2], &fvtemp, ftemp );/*正規化*/

	_sceVu0SubVector( &fvtemp, pvecPos, &work->wdmng->matPlNode.m[3] );
	ftemp = fvtemp.vx*fvtemp.vx + fvtemp.vy*fvtemp.vy + fvtemp.vz*fvtemp.vz ;
	vecDist.vw = DG_SQRT( ftemp );/*距離*/
	ftemp = 1.0f / vecDist.vw;
	_sceVu0ScaleVectorXYZ( &matDiff.m[3], &fvtemp, ftemp );/*正規化*/
#endif

    // 内積値と距離から影響力を出す

    vecAffect.vx = (vecDist.vx - MAX_DIST) * (*pfAffectDist);
    if ( vecAffect.vx > 1.f) vecAffect.vx = 1.f;
    else if ( vecAffect.vx < 0.f) vecAffect.vx = 0.f;

    vecAffect.vy = (vecDist.vy - MAX_DIST) * (*pfAffectDist);
    if ( vecAffect.vy > 1.f) vecAffect.vy = 1.f;
    else if ( vecAffect.vy < 0.f) vecAffect.vy = 0.f;

    vecAffect.vz = (vecDist.vz - MAX_DIST) * (*pfAffectDist);
    if ( vecAffect.vz > 1.f) vecAffect.vz = 1.f;
    else if ( vecAffect.vz < 0.f) vecAffect.vz = 0.f;

    vecAffect.vw = (vecDist.vw - MAX_DIST) * (*pfAffectDist);
    if ( vecAffect.vw > 1.f) vecAffect.vw = 1.f;
    else if ( vecAffect.vw < 0.f) vecAffect.vw = 0.f;

    // 反転
    vecAffect.vx = 1.f - vecAffect.vx;
    vecAffect.vy = 1.f - vecAffect.vy;
    vecAffect.vz = 1.f - vecAffect.vz;
    vecAffect.vw = 1.f - vecAffect.vw;

    // 押し出しベクトルの影響力
    vecResist.vx = vecAffect.vx * PUSH_LEVEL;
    vecResist.vy = vecAffect.vy * PUSH_LEVEL;
    vecResist.vz = vecAffect.vz * PUSH_LEVEL;
    vecResist.vw = vecAffect.vw * PUSH_LEVEL;

    // プレイヤーの動きの影響力
    vecPlMove.vx = vecAffect.vx * work->wdmng->vecPlMoveDist.vx * PLSWIM_LEVEL;
    vecPlMove.vy = vecAffect.vy * work->wdmng->vecPlMoveDist.vy * PLSWIM_LEVEL;
    vecPlMove.vz = vecAffect.vz * work->wdmng->vecPlMoveDist.vz * PLSWIM_LEVEL;
    vecPlMove.vw = vecAffect.vw * work->wdmng->vecPlMoveDist.vw * PLSWIM_LEVEL;

    // 反発ベクトルとプレイヤー移動ベクトル, 距離から移動ベクトルを生成
    // vf03～vf06 : プレイヤー関節とゴミの差分ベクトル(正規化済み)
    // vf07～vf10 : プレイヤー関節の移動ベクトル(正規化済み)
    // vf11       : プレイヤー関節とゴミの距離 (xyzwにそれぞれ入っている)
    // vf12       : プレイヤー関節の移動量     (xyzwにそれぞれ入っている)
    // vf13	  : 出力先
    // vf20～vf27は予約されている
    // LoadPlayerNodeDataでプレイヤーの関節関連のデータが入っている

#ifdef BP_PSX2_ASM // yano

    asm volatile ("
        # データをVUレジスタに転送
        lqc2		vf03, 0x00(%1)		# matDiff.m[0][0]
        lqc2		vf04, 0x10(%1)		# matDiff.m[1][0]
        lqc2		vf05, 0x20(%1)		# matDiff.m[2][0]
        lqc2		vf06, 0x30(%1)		# matDiff.m[3][0]
        lqc2		vf11, 0x00(%2)		# vecResist
        lqc2		vf12, 0x00(%3)		# vecPlMove
// BP        # 遠ざけベクトルを算出
        vmulax.xyz	 ACC, vf03, vf11x	# ACC  = matDiff.m[0][0] * vecResist.vx
        vmadday.xyz	 ACC, vf04, vf11y	# ACC += matDiff.m[1][0] * vecResist.vy
        vmaddaz.xyz	 ACC, vf05, vf11z	# ACC += matDiff.m[2][0] * vecResist.vz
        vmaddaw.xyz	 ACC, vf06, vf11w	# ACC += matDiff.m[3][0] * vecResist.vw
        # プレイヤー動作ベクトルを算出
        vmaddax.xyz	 ACC, vf20, vf11x	# ACC += 左手移動量 * vecPlMove.vx -->vecResist.vx だそうです。vf12は未使用
        vmadday.xyz	 ACC, vf21, vf11y	# ACC += 右手移動量 * vecPlMove.vy -->vecResist.vx だそうです。vf12は未使用
        vmaddaz.xyz	 ACC, vf22, vf11z	# ACC += 左足移動量 * vecPlMove.vz -->vecResist.vx だそうです。vf12は未使用
        vmaddw.xyz	vf13, vf23, vf11w	# 結果 = ACC + 右足移動量 * vecPlMove.vw -->vecResist.vx だそうです。vf12は未使用
        # 結果を出力
        sqc2		vf13, 0x00(%0)		
    ":: "r"(pvecConv), "r"(&matDiff), "r"(&vecResist), "r"(&vecPlMove)
    );
#else
	{
	FVECTOR fvtemp0, fvtemp1;
	_sceVu0ApplyMatrix( &fvtemp0, &matDiff, &vecResist );
	_sceVu0ApplyMatrix( &fvtemp1, &work->wdmng->matPlNode, &vecResist );
	_sceVu0AddVector( pvecConv, &fvtemp0, &fvtemp1 );
	}
#endif
}

// 動作関数
static	void Act( WATERDUST *work )
{
    int 	j,k;
    int 	nActNum = GV_Time % 4;	
#ifdef DEBUG_MODE
    int time;
    GV_SET_PRFC_CLOCK();
#endif

    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    {
	int	nDatIndex;	
	extern int KR_CameraDis( FVECTOR* pos );

	nDatIndex = work->wdmng->nIndexTbl[ work->nIndex ];
	if ( nDatIndex < 0 ){
	    work->prim->flag |= DG_PRIM2_INVISIBLE;
	    return;
	}else{
    	    work->prim->flag &= ~DG_PRIM2_INVISIBLE;
	}

	if ( work->wdmng->nPrevIndex[ work->nIndex ] != nDatIndex  ){
#if 0
printf("%d %d :: %d %p\n", work->nIndex, nDatIndex, work->wdmng->nPrevIndex[ work->nIndex ], 	
       &work->wdmng->nPrevIndex[ work->nIndex ]);	
#endif
	    {
		// ワークスワップ
		FVECTOR* pvecInitPos 	= work->vecInitPos;
		FVECTOR* pvecSpeed 	= work->vecSpeed;
		FVECTOR* pvecCenter     = &work->wdmng->vecPosTbl[nDatIndex];
		FVECTOR* pvecSize       = &work->wdmng->vecSizeTbl[nDatIndex];
		FVECTOR* pvecPos1 	= work->prim->pos[0];	
		FVECTOR* pvecPos2 	= work->prim->pos[1];	

		_sceVu0CopyVector( &work->vecSize, &work->wdmng->vecSizeTbl[nDatIndex]);
		_sceVu0CopyVector( &work->vecCenter, &work->wdmng->vecSizeTbl[nDatIndex]);
		_sceVu0CopyVector( &work->vecBound1, &work->wdmng->vecBoundMinTbl[nDatIndex]);
		_sceVu0CopyVector( &work->vecBound2, &work->wdmng->vecBoundMaxTbl[nDatIndex]);

		for ( j = 0 ; j < work->nPrimNum; j++ ){
		    for ( k = 0 ; k < N_VERTS ; k++ ){
			// 初期位置
			pvecInitPos->vx = pvecCenter->vx + frnd() * pvecSize->vx;
			pvecInitPos->vy = pvecCenter->vy +  rnd() * 30.f + 50.f;
			pvecInitPos->vz = pvecCenter->vz + frnd() * pvecSize->vz;
			pvecInitPos->vw = 1.f;
			
			// プリミティブ
			_sceVu0CopyVector( pvecPos1, pvecInitPos );
			_sceVu0CopyVector( pvecPos2, pvecInitPos );

			// 速度ベクトル
			_sceVu0CopyVector( pvecSpeed, &DG_ZeroVector); 

			pvecInitPos++;
			pvecPos1++;
			pvecPos2++;
			pvecSpeed++;
		    }
		}
	    }
	}
#if 0
	if ( KR_CameraDis( &work->vecCenter) > work->fOffDist){
	    return;
	}
#endif
    }

    {	
	FVECTOR  vecForce;

	FVECTOR* pvecSpeed; 
	FVECTOR* pvecPrim;

	FVECTOR* pvecSpdCur; 
	FVECTOR* pvecPrev;
	FVECTOR* pvecCurr;
	{
	    float*	 pfAffectDist   = work->fAffectDist;
	    FVECTOR* pvecInitPos 	= work->vecInitPos;
	    pvecSpdCur 			= work->vecSpeed;
	    pvecPrev = work->prim->pos[(work->prim->buffer_clock+1)%2];
	    pvecCurr = work->prim->pos[work->prim->buffer_clock];
	    
	    // プレイヤーノードデータをVUにロード	
	    LoadPlayerNodeData( work);

	    for ( j = 0; j < work->nPrimNum; j++){
		// メインメモリ->スクラッチパッドへ
		pvecPrim    = SCR_POS; // work->prim->pos[work->prim->buffer_clock];
		pvecSpeed   = SCR_SPD; // work->vecSpeed;
		// メインメモリ->スクラッチパッドへ
		Mao_CopyMemToScr( pvecPrev, SCR_POS, sizeof(FVECTOR), N_VERTS );
		Mao_CopyMemToScr( pvecSpdCur, SCR_SPD, sizeof(FVECTOR), N_VERTS );
		for ( k = 0; k < N_VERTS; k++){		// スプライト単位
		    // 移動ベクトル更新
		    _sceVu0ScaleVector( pvecSpeed, pvecSpeed, 0.95f);

		    if ( k % 4 == nActNum ){
			// プレイヤー押し戻しベクトル
			GetPlayerConvection( work, pfAffectDist, &vecForce, pvecPrim);
			_sceVu0AddVector( pvecSpeed, pvecSpeed, &vecForce);
		    }

		    // 座標に反映
		    pvecSpeed->vy = 0.f;
		    _sceVu0AddVector( pvecPrim, pvecPrim, pvecSpeed);

		    // 補間

#ifdef BP_PSX2_ASM // yano

		    asm volatile("
                        mfc1		$4, %3			# 補間パラメータ(fRate)の読み込み
                        mfc1		$5, %4			# 補間パラメータ(1.f - fRate)の読み込み
                        lqc2		vf10, 0x00(%1)		# 補間座標元読み込み
                		lqc2		vf11, 0x00(%2)		# 補間座標先読み込み
                        qmtc2		$4, vf01		# 補間パラメータ(fRate)の読み込み
                        qmtc2		$5, vf02		# 補間パラメータ(1.f - fRate)の読み込み
                        vmulx.xyz	vf06, vf10, vf01x	# (*pvec1) *= fRate
                        vmulx.xyz	vf07, vf11, vf02x	# (*pvec2) *= fRateRev
                        vadd.xyzw	vf08, vf06, vf07	# (*pvec1) + (*pvec2)	
                        sqc2		vf08, 0(%0)		# pvecResに出力
                    ":: "r"(pvecPrim), "r"(pvecPrim), "r"(pvecInitPos), "f"(0.99f), "f"(0.01f) : "$4", "$5");
#else
			{								
			FVECTOR fvtemp0, fvtemp1;							
			_sceVu0ScaleVectorXYZ( &fvtemp0, pvecPrim, 0.99f );
			_sceVu0ScaleVectorXYZ( &fvtemp1, pvecInitPos, 0.01f );
			_sceVu0AddVector( pvecPrim, &fvtemp0, &fvtemp1 );							
			}
#endif

		    pvecPrim->vw = 1.f;

		    // ポインタ更新
		    pfAffectDist++;
		    pvecInitPos++;
		    pvecSpeed++;
		    pvecPrim++;
		}
		// スクラッチパッド->メインメモリへ
		Mao_CopyScrToMem( pvecCurr, SCR_POS, sizeof(FVECTOR), N_VERTS );
		Mao_CopyScrToMem( pvecSpdCur, SCR_SPD, sizeof(FVECTOR), N_VERTS );
		pvecSpdCur += N_VERTS;
		pvecCurr   += N_VERTS;
		pvecPrev   += N_VERTS;
	    }
	}
    }

#ifdef DEBUG_MODE
    time = GV_GET_PRFC_CLOCK();
    DbgDumpExecTime( time);
#endif

}

// 破棄関数
static	void Die( WATERDUST *work )
{
    if ( work->fAffectDist 	!= NULL)	GV_Free( work->fAffectDist);
    if ( work->vecInitPos 	!= NULL)	GV_Free( work->vecInitPos);
    if ( work->vecSpeed   	!= NULL)	GV_Free( work->vecSpeed);

    work->prim     = MAO_FreePrim2( work->prim );
}

// パケット初期化
static int InitPacket( WATERDUST* 		work,		
		       DG_PRIM2* 	prim, 		// 
		       DG_TEX* 		tex, 		// 
		       long64		tagAlpha,	// 
		       int		nRGBA,		// 
		       int		nRaise)		// 
{
    u_char		r,g,b,a;
    int			j, k;
    FVECTOR*		pvecPos1;
    FVECTOR*		pvecPos2;
    DG_PRIM2_UVRGBWH*	uvrgbwh1;
    DG_PRIM2_UVRGBWH*	uvrgbwh2;

    // 優先設定
    prim->raise = nRaise;

    // テクスチャ設定
    DG_ConfigPrim2Tex( prim, tex );	// プリミティブにテクスチャを設定
    DG_SetPrim2Alpha( prim, tagAlpha );	// α設定

    r = ((nRGBA & 0xff000000) >> 24);
    g = ((nRGBA & 0x00ff0000) >> 16);
    b = ((nRGBA & 0x0000ff00) >> 8);
    a =  (nRGBA & 0x000000ff);

    pvecPos1 	= prim->pos[0];	
    pvecPos2 	= prim->pos[1];	
    uvrgbwh1	= prim->uvrgb[0];
    uvrgbwh2	= prim->uvrgb[1];

    {
	FVECTOR* pvecPos 	= work->vecInitPos;
	for ( j = 0 ; j < work->nPrimNum; j++ ){
	    for ( k = 0 ; k < N_VERTS ; k++ ){
		int nSize;
		_sceVu0CopyVector( pvecPos1, pvecPos );
		_sceVu0CopyVector( pvecPos2, pvecPos );

		nSize = (int)(BASE_SIZE + rnd() * ADD_SIZE);

		uvrgbwh1->w  = (int)(nSize);
		uvrgbwh1->h  = (int)(nSize);

		uvrgbwh1->u0 = FTOI12( rnd() * 0.4F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = FTOI12( rnd() * 0.4F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = FTOI12( (1.0F - rnd() * 0.4F) * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = FTOI12( (1.0F - rnd() * 0.4F) * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = 4096;
		uvrgbwh1->q1 = 4096;
		uvrgbwh1->f0 = 0x0fff;
		uvrgbwh1->f1 = 0x0fff;
		
		uvrgbwh1->r  = (int)((float)r * ( 0.8f + rnd() * 0.2f));
		uvrgbwh1->g  = (int)((float)g * ( 0.8f + rnd() * 0.2f));
		uvrgbwh1->b  = (int)((float)b * ( 0.8f + rnd() * 0.2f));
		uvrgbwh1->a  = a + MAO_Rand( COL_ADD_A);

		memcpy( uvrgbwh2, uvrgbwh1, sizeof(DG_PRIM2_UVRGBWH) );

		pvecPos1++;
		pvecPos2++;
		uvrgbwh1++;
		uvrgbwh2++;
		pvecPos++;
	    }
	}
    }

    return 1;
}


static int InitWork( WATERDUST* work, FVECTOR* pvecPos, FVECTOR* pvecSize )
{
    int		i,j;
    FVECTOR     vec;
    DG_TEX*	tex;

    // バウンド生成
    work->vecBound1.vx = pvecPos->vx - pvecSize->vx;
    work->vecBound1.vy = pvecPos->vy;
    work->vecBound1.vz = pvecPos->vz - pvecSize->vz;
    work->vecBound1.vw = 1.f;

    work->vecBound2.vx = pvecPos->vx + pvecSize->vx;
    work->vecBound2.vy = pvecPos->vy + pvecSize->vy;
    work->vecBound2.vz = pvecPos->vz + pvecSize->vz;
    work->vecBound2.vw = 1.f;

    _sceVu0SubVector( &work->vecSize, &work->vecBound2, &work->vecBound1);
    _sceVu0ScaleVector( &vec, &work->vecSize, 0.5f);
    _sceVu0AddVector( &work->vecCenter, &work->vecBound1, &vec);

    // 影響距離
    if( ( work->fAffectDist = (float*)GV_Malloc(sizeof(float) * work->nPrimNum * N_VERTS ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! :: %s %d\n", __FILE__, __LINE__ );
	return -1 ;
    }
    GV_ZeroMemory(work->fAffectDist, sizeof(float) * work->nPrimNum * N_VERTS );

    // 初期位置ワーク
    if( ( work->vecInitPos = (FVECTOR*)GV_Malloc(sizeof(FVECTOR) * work->nPrimNum * N_VERTS ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! :: %s %d\n", __FILE__, __LINE__ );
	return -1 ;
    }
    GV_ZeroMemory(work->vecInitPos, sizeof(FVECTOR) * work->nPrimNum * N_VERTS );

    // 移動ベクトルワーク
    if( ( work->vecSpeed = (FVECTOR*)GV_Malloc(sizeof(FVECTOR) * work->nPrimNum * N_VERTS ))==NULL) {
	MAO_PRINTF(" NO MEMORY !! :: %s %d\n", __FILE__, __LINE__ );
	return -1 ;
    }
    GV_ZeroMemory(work->vecSpeed, sizeof(FVECTOR) * work->nPrimNum * N_VERTS );

    {
	float*	 pfAffectDist   = work->fAffectDist;
	FVECTOR* pvecInitPos 	= work->vecInitPos;
	FVECTOR* pvecSpeed   	= work->vecSpeed;
	for ( i = 0; i < work->nPrimNum; i++){
	    for ( j = 0; j < N_VERTS; j++){
		// 影響距離
		(*pfAffectDist) = 1.f / (AFFECT_BASE + rnd() * AFFECT_ADD);
		// 初期位置
		pvecInitPos->vx = pvecPos->vx + frnd() * pvecSize->vx;
		pvecInitPos->vy = pvecPos->vy +  rnd() * 30.f + 50.f;
		pvecInitPos->vz = pvecPos->vz + frnd() * pvecSize->vz;
		pvecInitPos->vw = 1.f;
		// 速度ベクトル
		_sceVu0CopyVector( pvecSpeed, &DG_ZeroVector); 

		pfAffectDist++;
		pvecInitPos++;
		pvecSpeed++;
	    }
	}
    }

    // 計算オフ距離
    work->fOffDist = _MAO_FVecLen3( &work->vecSize) * 2.f;
work->fOffDist = 8000.f;
    // 描画用ワーク初期化
    // プリミティブ本体の作成
    work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, 
			       work->nPrimNum, N_VERTS );
    if ( work->prim == NULL){
	MAO_PRINTF("Cannot MakePrim!!\n");
	return -1;
    }
    // テクスチャの設定
//    tex = DG_GetTexture( 7733153 ); // kasu_s1
    tex = DG_GetTexture( 15638432 );

    // パケットの初期化
    InitPacket( work, work->prim, tex, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ), 
		MAO_SetRGBA( COL_R, COL_G, COL_B, COL_A ), 1000);

    return 0;
}

// シナリオ呼出時初期化
static int GetResources( WATERDUST* work, int name, int map )
{
    char* 	opt;
    FVECTOR 	vecPos;
    FVECTOR 	vecSize;
    extern int ENE_GCL_GetFV( char *ptr, FVECTOR *fvec );

    // 基本システム初期化
    work->name   = name;
    work->map    = map;
    work->hzx_id = GM_GetHzxGroupID( GM_CurrentStageMap ); 	
    work->wdmng  = NULL;

    work->nPrimNum = GCL_GetOptionValue( 'u', 10 ); // プリミティブ数
    if ( work->nPrimNum > N_PRIM ){
	work->nPrimNum = N_PRIM;
    }

    if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){	// 座標取得
	if ( ENE_GCL_GetFV( opt, &vecPos ) != 0 ){
	    vecPos.vx = 11075.f;
	    vecPos.vy = -8700.f;
	    vecPos.vz = -257690.f;
	    vecPos.vw = 1.f;
	}
    }else{
	vecPos.vx = 11075.f;
	vecPos.vy = -8700.f;
	vecPos.vz = -257690.f;
	vecPos.vw = 1.f;
    }

    if ( ( opt = GCL_GetOption( 's' ) ) != NULL ){	// サイズ取得
	if ( ENE_GCL_GetFV( opt, &vecSize ) != 0 ){
	    vecSize.vx = 1000.f;
	    vecSize.vy = 2000.f;
	    vecSize.vz = 1000.f;
	    vecSize.vw = 1.f;
	}
    }else{
	vecSize.vx = 1000.f;
	vecSize.vy = 2000.f;
	vecSize.vz = 1000.f;
	vecSize.vw = 1.f;
    }

#if 0
    if( HZX_LevelHazardCheck( work->hzx_id, &vecPos, 
			      WDUST_HZX_CHK_TYPE, WDUST_HZX_PLAYER_ONLY ) & 1){
	HZX_FLR	flr[2];
	int	flr_atrs[2];
	float	flr_height[2];
	HZX_GetLevelHazard( flr, flr_atrs );
	HZX_GetLevelHeight( flr_height );
	vecPos.vy = flr_height[0] + 50.f;
    }
#endif    

    return InitWork( work, &vecPos, &vecSize);
}

// プログラム呼出時初期化
static int GetResourcesProg( WATERDUST* work, 
			     int 	name, 
			     int 	map, 
			     int 	nPrimNum, 
			     FVECTOR* 	pvecPos, 
			     FVECTOR* 	pvecSize, 
			     WDUST_MNG* wdmng,
			     int	nIndex)
{
    // 基本システム初期化
    work->name   = name;
    work->map    = map;
    work->hzx_id = GM_GetHzxGroupID( GM_CurrentStageMap ); 	
    work->wdmng  = wdmng;
    work->nIndex = nIndex;
    work->nPrimNum = nPrimNum; // プリミティブ数
    if ( work->nPrimNum > N_PRIM ){
	work->nPrimNum = N_PRIM;
    }

    if( HZX_LevelHazardCheck( work->hzx_id, pvecPos, 
			      WDUST_HZX_CHK_TYPE, WDUST_HZX_PLAYER_ONLY ) & 1){
	HZX_FLR	flr[2];
	int	flr_atrs[2];
	float	flr_height[2];
	HZX_GetLevelHazard( flr, flr_atrs );
	HZX_GetLevelHeight( flr_height );
	pvecPos->vy = flr_height[0] + 50.f;
    }
    
    return InitWork( work, pvecPos, pvecSize);
}

// 水中ゴミ
void* NewWaterDust( name, where )
int	name;
int	where;
{
    WATERDUST*	work;

    work = (WATERDUST *)GV_NewActor(  GV_ACTOR_AFTER, sizeof( WATERDUST ) ) ;

    if( work != NULL){
	GV_SetActor( &(work->actor), Act,Die) ;
	GV_ActorEX( &work->actor );
	if( GetResources( work, name, where ) < 0){
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }				

    return (void *)work;
}

// 水中ゴミ
void* NewWaterDustProg( name, where, nPrimNum, pvecPos, pvecSize, wdmng, index )
int		name;
int		where;
int		nPrimNum;
FVECTOR*	pvecPos;
FVECTOR*	pvecSize;
WDUST_MNG*	wdmng;
int		index;
{
    WATERDUST*	work;

    work = (WATERDUST *)GV_NewActor(  GV_ACTOR_EFFECT, sizeof( WATERDUST ) ) ;

    if( work != NULL){
	GV_SetActor( &(work->actor), Act,Die) ;
	GV_ActorEX( &work->actor );
	if( GetResourcesProg( work, name, where, nPrimNum, pvecPos, pvecSize, wdmng, index ) < 0){
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }				

    return (void *)work;
}
