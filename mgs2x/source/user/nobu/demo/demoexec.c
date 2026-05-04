/*
	demoexec.c
	    デモ実行カーネル：デモ実行

	2000/01/13 N.Tanaka
	$Id: demoexec.c,v 1.1.1.3 2002/11/19 11:46:43 Yoshizawa1 Exp $
*/

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <math.h>

#include <libdma.h>
#include <libgraph.h>
//#include <libvu0.h>
#include <libdg.h>
#include <gameheader.h>
#include <camera.h>
#include <libvu0.h>

#include "demo.h"
#include "demo_.h"
#include "common.h"
#include "DE2Struct.h"
#include "demoeffect.h"

/******************************************************************************
 * functions
 */
#define KMS_MOTION

/******************************************************************************
 * functions
 */
extern int  DG_QueueEvmObj(DG_EVMOBJ *evmobj);
extern void DG_DequeueEvmObj(DG_EVMOBJ *evmobj);


static BOOL ShowScene(LPMGSDEMOACT lpAct, LPMGSDEMOSCENEMOTION lpSMotion);
/*
static BOOL ShowEffect(LPMGSDEMOACT lpAct, LPMGSDEMOEFFECT lpEffect,
	LPMGSDEMOEFFECTACT lpEAct);
*/
static void KillEffect(LPMGSDEMOACT lpAct, int nEffectID);
static BOOL ShowEffectExecute(LPMGSDEMOACT lpAct, LPMGSDEMOFRAME lpDFrame,
	LPMGSDEMOEFFECTACT lpEAct);
static BOOL ShowEffectStop(LPMGSDEMOACT lpAct, LPMGSDEMOFRAME lpDFrame, LPMGSDEMOEFFECT lpFrameEffect);
//extern void *C4_Explosion(FVECTOR *center);

static void EvmActMotionFromOldMotion(FVECTOR* lpfvectWorldMove, FVECTOR* lpfvectWorldRotation, int nJointCount, FVECTOR* lpfvectRotation, DG_EVMOBJ* lpEVMObject);
static void DemoJointFrameObjs(DG_OBJS *objs, int n_objs);

#ifdef DEMO_DEBUG_MODE
static void DemoDebugFunction2(LPMGSDEMOACT lpAct, LPMGSDEMOFRAME lpDFrame);
#endif

/******************************************************************************
 * locals
 */


/******************************************************************************
 * publics
 */

/******************************************************************************
 * デモ生成
 */
BOOL CreateDemo(	/* 0以外	正常 */
					/* 0 */
	LPMGSDEMOACT lpAct,		/* デモプロセス構造体のポインタ */
	LPMGSDEMO    lpDemo)	/* デモヘッダ構造体のポインタ */
{
	int            i;
	LPMGSDEMOSTAGE lpStage;
	LPMGSDEMOSCENE lpScene;
	LPMGSDEMOMODEL lpModel;

	OPERATOR();

	lpAct->nGameStatus = GM_GameStatus;
	lpAct->nItemSave   = GM_Item;
	lpAct->nWeaponSave = GM_Weapon;
/*
	// ステージ、シーンのポインタ設定 
	lpDemo->lpStage = (LPMGSDEMOSTAGE)
		((unsigned)lpDemo + (unsigned)lpDemo->lpStage);
	lpDemo->lpScene = (LPMGSDEMOSCENE)
		((unsigned)lpDemo + (unsigned)lpDemo->lpScene);
*/

/* 
   注意事項！このファイル版のでも再生機構はスロー・巻きも度し・ループ機構のため
   シーン等のポインタの値を上書きしません。よって、修正等を加えるときは注意して
   ください
*/

	InitChain(&lpAct->lpEffectPrevious);

	/* デモヘッダを複写 */
	if((lpAct->lpDemo = (LPMGSDEMO)GV_Malloc(sizeof(MGSDEMO))) == NULL)
		return 0;

	memcpy(lpAct->lpDemo, lpDemo, sizeof(MGSDEMO));
	lpAct->lpDemo->lpStage = NULL;
	lpAct->lpDemo->lpScene = NULL;

	/* +1 : 0 確保するとハングアップするため */
	if(((lpAct->lpDemo->lpStage = (LPMGSDEMOSTAGE)GV_Malloc(
			sizeof(MGSDEMOSTAGE) * lpDemo->nStageCount + 1)) == NULL)
		|| ((lpAct->lpDemo->lpScene = (LPMGSDEMOSCENE)GV_Malloc(
			sizeof(MGSDEMOSCENE) * lpDemo->nSceneCount + 1)) == NULL))
		return 0;

	lpStage = (LPMGSDEMOSTAGE)((unsigned)lpDemo + (unsigned)lpDemo->lpStage);
	memcpy(lpAct->lpDemo->lpStage,
		lpStage, sizeof(MGSDEMOSTAGE) * lpAct->lpDemo->nStageCount);

	lpScene = (LPMGSDEMOSCENE)((unsigned)lpDemo + (unsigned)lpDemo->lpScene);
	memcpy(lpAct->lpDemo->lpScene,
		lpScene, sizeof(MGSDEMOSCENE) * lpAct->lpDemo->nSceneCount);

	for(i = 0, lpStage = lpAct->lpDemo->lpStage;
		i < lpAct->lpDemo->nStageCount; i++, lpStage++)
	{
		if(GV_GetCache(lpStage->nCacheID) == NULL)
		{
#ifdef DEMO_DEBUG_MODE
			printf("Noload model ( Stage )\n");
#endif
			return 0;
		}
	}

	/* +1 : 0 確保するとハングアップするため */
	if((lpAct->lpModel = (LPMGSDEMOMODEL)GV_Malloc(
			sizeof(MGSDEMOMODEL) * lpAct->lpDemo->nSceneCount + 1)) == NULL)
		return 0;
	memset(lpAct->lpModel, 0x00,
		sizeof(MGSDEMOMODEL) * lpAct->lpDemo->nSceneCount);

	for(i = 0, lpScene = lpAct->lpDemo->lpScene, lpModel = lpAct->lpModel;
		i < lpAct->lpDemo->nSceneCount; i++, lpScene++, lpModel++)
	{
		if(GV_GetCache(lpScene->nCacheID) == NULL)
		{
#ifdef DEMO_DEBUG_MODE
			printf("Noload model ( Scene = No.%d)\n", i + 1);
#endif
			return 0;
		}

		/* KMS モデルか EVM モデルか */
		if((((lpScene->nCacheID & ~GV_CACHEID_RESIDENT) >> 24) + 'a') == 'k')
		{
#ifdef DEMO_DEBUG_MODE
			printf("Scene %d Init KMS Model\n", i);
#endif
			/* KMS モデルの初期化 */
			if(GM_InitControl(&lpModel->control, lpScene->nStrCode, lpAct->nMap) < 0)
			{
				printf("Error init control ( Scene = No.%d )\n", i + 1);
				return 0;
			}

			lpModel->control.skip_flag |= CTRL_SKIP_HZX;
			lpModel->control.r_sphere = 0;
			lpModel->control.interp   = 0;
			lpModel->lpevm_object = NULL;

			if((lpScene->dwStyle & MGS_SCENETOSTAGE) != 0)
			{
				GM_InitObject(&lpModel->object, lpScene->nStrCode,
					DG_FLAG_TEXT | DG_FLAG_TRANS | DG_FLAG_PAINT | DG_FLAG_ONEPIECE);
				continue;
			}

			/* オブジェクトを生成 */
#ifdef KMS_MOTION
			GM_InitObject(&lpModel->object, lpScene->nStrCode,
				DG_FLAG_FINISHCALC | DG_FLAG_TEXT | DG_FLAG_TRANS | DG_FLAG_SHADE | DG_FLAG_SHADOWMAKE);
#else
			GM_InitObject(&lpModel->object, lpScene->nStrCode,
				DG_FLAG_TEXT | DG_FLAG_TRANS | DG_FLAG_SHADE | DG_FLAG_SHADOWMAKE);
#endif
			/* オブジェクトにライトの影響を加えるための処理 */
			GM_ConfigObjectLight(&lpModel->object, lpModel->light);

			/* 非表示 */
			DG_InvisibleObjs(lpModel->object.objs);
		}
		else
		{
#ifdef DEMO_DEBUG_MODE
			printf("Scene %d Init EVM Model\n", i);
#endif
			/* EVM モデルの初期化 */
			lpModel->lpevm_object = DG_MakeEvmObj(GV_GetCache(lpScene->nCacheID),
				0, 0);
			DG_QueueEvmObj(lpModel->lpevm_object);

			/* 非表示 */
			lpModel->lpevm_object->flag |= DG_EVMOBJ_INVISIBLE;
		}
	}

#if 0
	/* 効果用のモデル */
	if(GV_GetCache(IDMODEL_CACHEEFFECT) == NULL)
	{
#ifdef _DEMODEBUG
		printf("Noload model ( null.kmd )\n");
#endif
		return 0;
	}

	if(GM_InitControl(&lpAct->modelEffect.control, 0, lpAct->nMap) < 0)
	{
#ifdef _DEMODEBUG
		printf("Error init control ( null.kmd )\n");
#endif
		return 0;
	}

	lpAct->modelEffect.control.r_sphere = 0;
	lpAct->modelEffect.control.interp   = 0;

	GM_InitObject(&lpAct->modelEffect.object, IDMODEL_EFFECT,
		DG_FLAG_TEXT | DG_FLAG_TRANS | DG_FLAG_SHADE);
	/* ライト設定 */
	GM_ConfigObjectLight(&lpAct->modelEffect.object, lpAct->modelEffect.light);
	/* 非表示 */
	DG_InvisibleObjs(lpAct->modelEffect.object.objs);
#endif

	GM_GameStatus |= STATE_DEMO;
	return 1;
}

/******************************************************************************
 * デモ破棄
 */
BOOL DestroyDemo(	/* 0以外	正常 */
					/* 0 */
	LPMGSDEMOACT lpAct)		/* デモプロセス構造体のポインタ */
{
	int                i;
	LPMGSDEMOEFFECTACT lpEAct;
	LPMGSDEMOSCENE     lpScene;
	LPMGSDEMOMODEL     lpModel;

	OPERATOR();

/*	まだ STATE＿VIEW_LOAD が設定されていません
	if((lpAct->dwStyle & DMT_STATEVIEWLOAD) != 0)
		GM_GameStatus |= STATE_VIEW_LOAD;
*/
	/* 子に告知 */
	if(lpAct->actor.actor.class & GV_CLASS_PARENT)
		GV_CallChildSignalFunc(lpAct, GV_SIGNAL_KILL, 0);

	/* 効果実行監視情報を破棄 */
	while((lpEAct = (LPMGSDEMOEFFECTACT)lpAct->lpEffectNext)
		!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious)
	{
		/* 自己破棄できない効果プロセスを強制停止 */
		if(lpEAct->lpActor != NULL)
			GV_DestroyOtherActor(lpEAct->lpActor);
		if(lpEAct->lpActor2 != NULL)
			GV_DestroyOtherActor(lpEAct->lpActor2);

		DeleteChain(&lpAct->lpEffectPrevious, &lpEAct->lpPrevious);
		GV_Free(lpEAct);
	}

	/* オブジェクトを破棄 */
	if(lpAct->lpModel != NULL)
	{
		for(i = 0, lpScene = lpAct->lpDemo->lpScene, lpModel = lpAct->lpModel;
			i < lpAct->lpDemo->nSceneCount; i++, lpScene++, lpModel++)
		{
			/* モデルの破棄 */
			if(lpModel->lpevm_object == NULL)
			{
				/* KMS モデルの破棄 */
				GM_FreeObject(&lpModel->object);
				GM_FreeControl(&lpModel->control);

				if(lpModel->lpData == NULL)
					continue;

				GV_Free(lpModel->lpData);
			}
			else
			{
				/* EVM オブジェクトの破棄 */
				DG_DequeueEvmObj(lpModel->lpevm_object);
				DG_FreeEvmObj(lpModel->lpevm_object);
			}
		}
		/* デモモデル構造体の破棄 */
		GV_Free(lpAct->lpModel);
		lpAct->lpModel = NULL;
	}

	/* メモリ開放 */
	if(lpAct->lpDemo != NULL)
	{
		if(lpAct->lpDemo->lpStage != NULL) GV_Free(lpAct->lpDemo->lpStage);
		if(lpAct->lpDemo->lpScene != NULL) GV_Free(lpAct->lpDemo->lpScene);
		GV_Free(lpAct->lpDemo);
		lpAct->lpDemo = NULL;
	}

	/* この後、CreateDemo 関数で STATE_DEMO が付けられる前の保存値を復帰するが明示的にしておく */
	GM_GameStatus &= ~STATE_DEMO;

	GM_GameStatus = lpAct->nGameStatus;
	GM_Item       = lpAct->nItemSave;
	GM_Weapon     = lpAct->nWeaponSave;

	return 1;
}

/******************************************************************************
 * 1フレーム実行
 */
BOOL FrameRunDemo(	/* 0以外	正常 */
					/* 0 */
	LPMGSDEMOACT   lpAct,		/* デモプロセス構造体のポインタ */
	LPMGSDEMOFRAME lpDFrame)	/* デモフレーム構造体のポインタ */
{
	int                  i;
	LPMGSDEMOEFFECT      lpEffect;
	LPMGSDEMOEFFECT      lpFrameEffect;
	LPMGSDEMOEFFECTACT   lpEAct;
	LPMGSDEMOEFFECTACT   lpEActN;
	LPMGSDEMOSCENEMOTION lpSMotion;

	/* エフェクト */
	lpFrameEffect = (LPMGSDEMOEFFECT)
		((unsigned)lpDFrame + (unsigned)lpDFrame->lpEffect);

	/* 効果実行監視情報確認識別をクリア・残存時間デクリメント */
	if((lpEAct = (LPMGSDEMOEFFECTACT)lpAct->lpEffectNext)
		!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious)
	{
		/* 効果プロセスの bCheck すべてに FALSE を入れる */
		do
			lpEAct->bCheck = FALSE;
		while((lpEAct = (LPMGSDEMOEFFECTACT)lpEAct->lpNext)
			!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious);
	}

	/* エフェクト実行 */
	for(i = 0, lpEffect = lpFrameEffect;
		i < lpDFrame->nEffectCount; i++, lpEffect++)
	{
		/* 効果実行監視情報からフレームオーバー等による未動作の効果を動作させる */
		if((lpEAct = (LPMGSDEMOEFFECTACT)lpAct->lpEffectNext)
			!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious)
		{
			do
			{
/*				if(lpEAct->effect.nID == lpEffect->nID)*/
				if( *(int*)(lpEAct->effect.byBuffer + sizeof(int)) ==
					*(int*)(lpEffect->byBuffer + sizeof(int)))
				{
					/* 既に動作している効果 */
					lpEAct->bCheck = TRUE;
					break;
				}
			}
			while((lpEAct = (LPMGSDEMOEFFECTACT)lpEAct->lpNext)
				!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious);
/*			if(lpEAct->effect.nID == lpEffect->nID) */
			if( *(int*)(lpEAct->effect.byBuffer + sizeof(int)) ==
				*(int*)(lpEffect->byBuffer + sizeof(int)))
				continue;
		}

		/* 新しいエフェクト(メモリ確保に失敗しても止まらない) */
		if((lpEAct = (LPMGSDEMOEFFECTACT)
			GV_Malloc(sizeof(MGSDEMOEFFECTACT))) == NULL)
			return 1;
		memset(lpEAct, 0x00, sizeof(MGSDEMOEFFECTACT));

		NextChain(&lpAct->lpEffectPrevious, &lpEAct->lpPrevious);
		lpEAct->bCheck = TRUE;
		lpEAct->effect = *lpEffect;

		if(ShowEffect(lpAct, lpEffect, lpEAct) == 0)
			return 0;
	}

	/* 効果実行監視情報確認識別がされてないものは終了したとみなす */
	if((lpEActN = (LPMGSDEMOEFFECTACT)lpAct->lpEffectNext)
		!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious)
	{
		do
		{
			lpEAct  = lpEActN;
			lpEActN = (LPMGSDEMOEFFECTACT)lpEAct->lpNext;

			if((lpEAct->bCheck == TRUE) || (lpEAct->lpActor != NULL))
				continue;

			DeleteChain(&lpAct->lpEffectPrevious, &lpEAct->lpPrevious);
			GV_Free(lpEAct);
		}
		while(lpEActN != (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious);
	}

	/* 効果の終了検査 */
	ShowEffectStop(lpAct, lpDFrame, lpFrameEffect);

	/* キャラ・モーションの表示 */
	/* シーン */
	lpSMotion = (LPMGSDEMOSCENEMOTION)
		((unsigned)lpDFrame + (unsigned)lpDFrame->lpSMotion);

	for(i = 0; i < lpDFrame->nSMotionCount; i++, lpSMotion++)
	{
		if(ShowScene(lpAct, lpSMotion) == 0)
			return 0;
	}

#ifdef DEMO_DEBUG_MODE
	if(lpAct->nDebugMode != DEMO_DEBUG_CAMERA)
	{
		/* カメラ操作モードでないのならカメラをデモのフレームにあわせて稼動 */
		GM_SetCameraQuick(0);
		lpAct->lpGMCamera->position.vx = lpDFrame->fvectCamera.vx;
		lpAct->lpGMCamera->position.vy = lpDFrame->fvectCamera.vy;
		lpAct->lpGMCamera->position.vz = lpDFrame->fvectCamera.vz;

		lpAct->lpGMCamera->target.vx = lpDFrame->fvectTarget.vx;
		lpAct->lpGMCamera->target.vy = lpDFrame->fvectTarget.vy;
		lpAct->lpGMCamera->target.vz = lpDFrame->fvectTarget.vz;

		lpAct->lpGMCamera->angle     = ((float)lpDFrame->nScreen / 320.0) * 2.0;
		lpAct->lpGMCamera->rotate.vz = lpDFrame->nZRotate;
	}
	DemoDebugFunction2(lpAct, lpDFrame);
#else	
	/* カメラの移動 */
	GM_SetCameraQuick(0);
	lpAct->lpGMCamera->position.vx = lpDFrame->fvectCamera.vx;
	lpAct->lpGMCamera->position.vy = lpDFrame->fvectCamera.vy;
	lpAct->lpGMCamera->position.vz = lpDFrame->fvectCamera.vz;

	lpAct->lpGMCamera->target.vx = lpDFrame->fvectTarget.vx;
	lpAct->lpGMCamera->target.vy = lpDFrame->fvectTarget.vy;
	lpAct->lpGMCamera->target.vz = lpDFrame->fvectTarget.vz;

	lpAct->lpGMCamera->angle     = ((float)lpDFrame->nScreen / 320.0) * 2.0;
	lpAct->lpGMCamera->rotate.vz = lpDFrame->nZRotate;
#endif

	return 1;
}

/******************************************************************************
 * 効果の停止（実行する前に削除する）
 */
static void KillEffect(
	LPMGSDEMOACT lpAct,			/* デモプロセス構造体のポインタ */
	int          nEffectID)		/* 停止する効果 */
{
	LPMGSDEMOEFFECTACT lpEActL;

	OPERATOR();

	if((lpEActL = (LPMGSDEMOEFFECTACT)lpAct->lpEffectNext)
		!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious)
	{
		do
		{
			if(((*(int*)(lpEActL->effect.byBuffer) & ~0x01000000) == nEffectID) &&
				(lpEActL->lpActor != NULL))
			{
				GV_DestroyOtherActor(lpEActL->lpActor);
				if(lpEActL->lpActor2 != NULL)
					GV_DestroyOtherActor(lpEActL->lpActor2);
				lpEActL->lpActor  = NULL;
				lpEActL->lpActor2 = NULL;
			}
		}
		while((lpEActL = (LPMGSDEMOEFFECTACT)lpEActL->lpNext)
			!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious);
	}
}

/******************************************************************************
 * 効果の表示継続更新
 */
static BOOL ShowEffectExecute(	/* 0以外	正常 */
								/* 0 */
	LPMGSDEMOACT       lpAct,		/* デモプロセス構造体のポインタ */
	LPMGSDEMOFRAME     lpDFrame,	/* デモフレーム構造体のポインタ */
	LPMGSDEMOEFFECTACT lpEAct)		/* 効果実行監視情報 */
{
#if 0
	int                  i;
	long                 lFloor[2];
	SVECTOR              svect1;
	LPMGSDEMOSCENEMOTION lpSMotion;

	OPERATOR();

	switch(*(int*)(lpEAct->effect.byBuffer) & ~0x01000000)
	{
	case EFFECT_SHADOW:		/* 影 */
		/* シーンに対応したオブジェクトを探す */
		for(i = 0, lpSMotion = lpDFrame->lpSMotion;
			(i < lpDFrame->nSMotionCount) && (lpSMotion->nSceneID
			!= ((LPEFFECTSHADOW)lpEAct->effect.byBuffer)->nSceneID);
			i++, lpSMotion++);
		if(i >= lpDFrame->nSMotionCount)
			break;

		/* 床を求める */
		svect1.vx = lpSMotion->svectMove.vx;
		svect1.vy = lpSMotion->svectMove.vy;
		svect1.vz = lpSMotion->svectMove.vz;
		i = HZD_LevelHazardCheck(
			lpAct->modelEffect.control.map->hzd, &svect1, HZD_CHK_F_FLOOR);
		HZD_GetLevelHeight(lFloor);

		lpEAct->nParam1 = lpSMotion->svectRotate.vy;
			 if((i & 0x01) != 0) lpEAct->nParam2 = (short)lFloor[0] + 50;
		else if((i & 0x02) != 0) lpEAct->nParam2 = (short)lFloor[1] + 50;
		else lpEAct->nParam2 = lpSMotion->svectMove.vy + 50;
		break;
	}
#endif

	return 1;
}

/******************************************************************************
 * 効果の停止
 */
static BOOL ShowEffectStop(	/* 0以外	正常 */
							/* 0 */
	LPMGSDEMOACT    lpAct,		/* デモプロセス構造体のポインタ */
	LPMGSDEMOFRAME  lpDFrame,	/* デモフレーム構造体のポインタ */
	LPMGSDEMOEFFECT lpFrameEffect)
{
	int                i;
	LPMGSDEMOEFFECT    lpEffect;
	LPMGSDEMOEFFECTACT lpEAct;

	for(i = 0, lpEffect = lpFrameEffect;
		i < lpDFrame->nEffectCount; i++, lpEffect++)
	{
		if((int)(lpEffect->byBuffer) == EFFECT_METALGEARLASER)
			return 1;
	}

	if((lpEAct = (LPMGSDEMOEFFECTACT)lpAct->lpEffectNext)
		!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious)
	{
		do
		{
			if((int)(lpEAct->effect.byBuffer) == EFFECT_METALGEARLASER)
				break;
		}
		while((lpEAct = (LPMGSDEMOEFFECTACT)lpEAct->lpNext)
			!= (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious);
	}

	if(lpEAct != (LPMGSDEMOEFFECTACT)&lpAct->lpEffectPrevious)
	{
		GV_DestroyOtherActor(lpEAct->lpActor);
		lpEAct->lpActor = NULL;

		DeleteChain(&lpAct->lpEffectPrevious, &lpEAct->lpPrevious);
		GV_Free(lpEAct);
	}

	return 1;
}

/******************************************************************************
 * シーンの表示
 */
static BOOL ShowScene(	/* 0以外	正常 */
						/* 0 */
	LPMGSDEMOACT         lpAct,		/* デモプロセス構造体のポインタ */
	LPMGSDEMOSCENEMOTION lpSMotion)	/* シーンモーション */
{
	int            i;
	int            nFlag;
	float          fLevels[2];
	FVECTOR        fvectMove;
	FVECTOR        fvectRotation;
	LPMGSDEMOMODEL lpModel;
	LPMGSDEMOSCENE lpScene;

	OPERATOR();

	/* シーンモーションに対応したオブジェクトを探す */
	for(i = 0, lpScene = lpAct->lpDemo->lpScene,
		lpModel = lpAct->lpModel; (i < lpAct->lpDemo->nSceneCount)
		&& (lpScene->nID != lpSMotion->nSceneID); i++, lpScene++, lpModel++);

	if(i >= lpAct->lpDemo->nSceneCount)
		return 0;

	if(lpModel->lpevm_object == NULL)
	{
		/* KMS モデルを設定 */
		if(lpSMotion->bPreview == FALSE)
		{
			/* 非表示 */
			DG_InvisibleObjs(lpModel->object.objs);
		}
		else
		{
			/* 表示 */
			DG_VisibleObjs(lpModel->object.objs);
		}

		/* 位置を設定 */
		lpModel->control.mov.vx = lpSMotion->fvectMove.vx;
		lpModel->control.mov.vy = lpSMotion->fvectMove.vy;
		lpModel->control.mov.vz = lpSMotion->fvectMove.vz;
		lpModel->control.rot.vx = lpSMotion->fvectRotate.vx;
		lpModel->control.rot.vy = lpSMotion->fvectRotate.vy;
		lpModel->control.rot.vz = lpSMotion->fvectRotate.vz;
		lpModel->control.turn.vx = lpSMotion->fvectRotate.vx;
		lpModel->control.turn.vy = lpSMotion->fvectRotate.vy;
		lpModel->control.turn.vz = lpSMotion->fvectRotate.vz;

		if((lpScene->dwStyle & MGS_SCENETOSTAGE) != 0)
		{
			GM_ActControl(&lpModel->control);
			GM_ActObject(&lpModel->object);
			return 1;
		}

		/* モーションを設定 */
		if(lpSMotion->nRotateCount != 0)
		{
			lpModel->object.objs->rots = (FVECTOR*)((unsigned)lpSMotion +
				(unsigned)lpSMotion->lpfvectRotate);
		}

		GM_ActMotion(&lpModel->object);
		GM_ActControl(&lpModel->control);
		GM_ActObject2(&lpModel->object);

#ifdef KMS_MOTION
		/* モーションを独自に計算 */
		DemoJointFrameObjs(lpModel->object.objs, lpSMotion->nRotateCount);
#endif

		DG_GetLightMatrix(&lpModel->control.mov, lpModel->light);

		/* ハザードのチェックがスキップされているのでその分の処理をここで行う */
		nFlag = HZX_LevelHazardCheck(lpModel->control.hzx_id, &(lpModel->control.mov),
			 lpModel->control.hzx_check_type, lpModel->control.flr_flag);
//			 (lpModel->control.hzx_check_type & ~CTRL_SKIP_HZX), lpModel->control.flr_flag);
		if(nFlag != 0)
		{
			HZX_GetLevelHeight(fLevels);
			lpModel->control.levels[0] = fLevels[0];
			if(lpModel->control.level[0] != NULL)
				HZX_GetLevelHazard(lpModel->control.level[0], lpModel->control.flr_atrs);
		}
		/* 水飛沫用フラグ立て */
		lpModel->control.grounded = 1;
	}
	else
	{
		/* EVM モデルを設定 */
		if(lpSMotion->bPreview == FALSE)
		{
			/* 非表示 */
			lpModel->lpevm_object->flag |= DG_EVMOBJ_INVISIBLE;
		}
		else
		{
			/* 表示 */
			lpModel->lpevm_object->flag &= ~DG_EVMOBJ_INVISIBLE;
		}

/*
		if((lpScene->dwStyle & MGS_SCENETOSTAGE) != 0)
		{
			GM_ActControl(&lpModel->control);
			GM_ActObject(&lpModel->object);
			return 1;
		}
*/
		fvectMove.vx = lpSMotion->fvectMove.vx;
		fvectMove.vy = lpSMotion->fvectMove.vy;
		fvectMove.vz = lpSMotion->fvectMove.vz;
		fvectMove.vw = 1;

		fvectRotation.vx = lpSMotion->fvectRotate.vx;
		fvectRotation.vy = lpSMotion->fvectRotate.vy;
		fvectRotation.vz = lpSMotion->fvectRotate.vz;
		fvectRotation.vw = 1;

		/* モーションを設定 */
		if(lpSMotion->nRotateCount != 0)
		{
			if(lpSMotion->nRotateCount > 80)
			{
				EvmActMotionFromOldMotion(&fvectMove, &fvectRotation, lpSMotion->nRotateCount,
					(FVECTOR*)((unsigned)lpSMotion + (unsigned)lpSMotion->lpfvectRotate),
					lpModel->lpevm_object);
			}
			else
			{
				EvmActMotionFromOldMotion(&fvectMove, &fvectRotation, 21,
					(FVECTOR*)((unsigned)lpSMotion + (unsigned)lpSMotion->lpfvectRotate),
					lpModel->lpevm_object);
			}
		}
		/* ライトを設定 */
		DG_GetLightMatrix(&fvectMove, lpModel->light);
		lpModel->lpevm_object->light = lpModel->light;

	}
	return 1;
}

/******************************************************************************
 * 旧形式モーションデータ（オイラー角配列データ）をＥＶＭモデルに割り当てる
 */
static void EvmActMotionFromOldMotion(
	FVECTOR*   lpfvectWorldMove,		/* モデルの移動 */
	FVECTOR*   lpfvectWorldRotation,	/* モデルの回転 */
	int        nJointCount,		/* 関節数 */
	FVECTOR*   lpfvectRotation,	/* モーション回転配列 */
	DG_EVMOBJ* lpEVMObject)		/* EVM オブジェクト */
{
	int         i;
	ScrPadWork* scrpad = (ScrPadWork*)SCRPAD_ADDR;
	FMATRIX     fmatWorld;
	FMATRIX*    lpfmatSkeleton;
	FVECTOR     fvectVector;
	FMATRIX*    lpfmatMatrix = scrpad->mats;
	EVM_SKEL*   lpEVMSkeleton;

	/* 中心を設定：回転 */
	_sceVu0UnitMatrix(&fmatWorld);
	if(lpfvectWorldRotation->vx != 0.0f)
		_sceVu0RotMatrixX(&fmatWorld, &fmatWorld, lpfvectWorldRotation->vx);
	if(lpfvectWorldRotation->vy != 0.0f)
		_sceVu0RotMatrixY(&fmatWorld, &fmatWorld, lpfvectWorldRotation->vy);
	if(lpfvectWorldRotation->vz != 0.0f)
		_sceVu0RotMatrixZ(&fmatWorld, &fmatWorld, lpfvectWorldRotation->vz);
	/* 中心を設定：移動 */
	fmatWorld.m[3][0] = lpfvectWorldMove->vx;
	fmatWorld.m[3][1] = lpfvectWorldMove->vy;
	fmatWorld.m[3][2] = lpfvectWorldMove->vz;
	/* オブジェクトに設定 */
	lpEVMObject->world = fmatWorld;

	/* ダブルバッファを切り替え */
	lpEVMObject->use_buffer = 1 - lpEVMObject->use_buffer;
	lpfmatSkeleton = lpEVMObject->matrix[lpEVMObject->use_buffer];

	/* オブジェクトにマトリクスを設定する */
	lpEVMSkeleton  = lpEVMObject->def->skeleton;
	fvectVector.vw = 1.0F;

	/* モデル情報から親子関係を取得して求める */
	for(i = 0 ; i < lpEVMObject->n_skeleton; i++)
	{
		FMATRIX* lpfmatParent;

		if(i < nJointCount)
		{
			/* 回転をマトリクスに設定 */
			_sceVu0UnitMatrix(lpfmatMatrix);
			if(lpfvectRotation->vx != 0.0f)
				_sceVu0RotMatrixX(lpfmatMatrix, lpfmatMatrix, lpfvectRotation->vx);
			if(lpfvectRotation->vy != 0.0f)
				_sceVu0RotMatrixY(lpfmatMatrix, lpfmatMatrix, lpfvectRotation->vy);
			if(lpfvectRotation->vz != 0.0f)
				_sceVu0RotMatrixZ(lpfmatMatrix, lpfmatMatrix, lpfvectRotation->vz);

			/* オフセットをマトリクスに設定 */
			lpfmatMatrix->m[3][0] = lpEVMSkeleton->rt_tx;
			lpfmatMatrix->m[3][1] = lpEVMSkeleton->rt_ty;
			lpfmatMatrix->m[3][2] = lpEVMSkeleton->rt_tz;

			/* 親子関係を反映させる */
			if(lpEVMSkeleton->parent != -1)
			{
				/* 親がある */
				lpfmatParent = &scrpad->mats[lpEVMSkeleton->parent];
				_sceVu0MulMatrix(lpfmatMatrix, lpfmatParent, lpfmatMatrix);
			}
			else
			{
				/* 親なし */
				_sceVu0MulMatrix(lpfmatMatrix, &lpEVMObject->world, lpfmatMatrix);
			}
			fvectVector.vx = -lpEVMSkeleton->rt_tx;
			fvectVector.vy = -lpEVMSkeleton->rt_ty;
			fvectVector.vz = -lpEVMSkeleton->rt_tz;
			_sceVu0ApplyMatrix(&lpfmatMatrix->m[3][0], lpfmatMatrix, &fvectVector);
			*lpfmatSkeleton = *lpfmatMatrix;
			lpfvectRotation++;
		}
		else
		{
			/* ２１関節以降はこちらにくるようにしましょう。 */
			*lpfmatMatrix = *lpfmatSkeleton = scrpad->mats[lpEVMSkeleton->parent];
		}

		lpEVMSkeleton++;
		lpfmatMatrix++;
		lpfmatSkeleton++;
	}
//	lpEVMObject->world = *(lpEVMObject->matrix[lpEVMObject->use_buffer]);
	
}



#ifdef DEMO_DEBUG_MODE
/******************************************************************************
 * デモデバッグ関数その２
 */
static void DemoDebugFunction2(	/* 0以外	正常 */
						/* 0 */
	LPMGSDEMOACT   lpAct,		/* デモプロセス構造体のポインタ */
	LPMGSDEMOFRAME lpDFrame)	/* デモフレーム構造体のポインタ */
{
	/* デバッグパラメータ表示 */
	if(lpAct->nViewParam >= DEMO_DEBUG_PARAM_FRAME)
	{
		_MENU_Locate(12, 24, 0);
		_MENU_Printf("Frame    :%d    Second :%d\n", lpAct->nTime, lpAct->nTime * MGS_DEMOSECONDRATE);
	}

	if(lpAct->nViewParam >= DEMO_DEBUG_PARAM_STRING)
	{
		if(lpAct->szDrawString[0] != '\0')
		{
			_MENU_Locate(12, 200, 0);
			_MENU_Printf("%s\n", lpAct->szDrawString);
		}
		else
			lpAct->nViewParam++;
	}

	if(lpAct->nViewParam >= DEMO_DEBUG_PARAM_POSITION)
	{
		_MENU_Locate(12, 36, 0);
		_MENU_Printf("Position :");
		_MENU_Locate(96, 36, 0);
		_MENU_Printf("%7.0f", lpAct->lpGMCamera->position.vx);
		_MENU_Locate(162, 36, 0);
		_MENU_Printf("%7.0f", lpAct->lpGMCamera->position.vy);
		_MENU_Locate(228, 36, 0);
		_MENU_Printf("%7.0f\n", lpAct->lpGMCamera->position.vz);
		_MENU_Locate(12, 48, 0);
		_MENU_Printf("Target   :");
		_MENU_Locate(96, 48, 0);
		_MENU_Printf("%7.0f", lpAct->lpGMCamera->target.vx);
		_MENU_Locate(162, 48, 0);
		_MENU_Printf("%7.0f", lpAct->lpGMCamera->target.vy);
		_MENU_Locate(228, 48, 0);
		_MENU_Printf("%7.0f\n", lpAct->lpGMCamera->target.vz);
	}

}
#endif

/******************************************************************************
 * モーションのワールドを計算
 */
static void DemoJointFrameObjs( DG_OBJS *objs, int n_objs )
{
	int			i;
	DG_OBJ		*obj;
	FMATRIX		*joints;
	FVECTOR		*rots;

	if(objs->flag & DG_FLAG_ONEPIECE)
		return;

	if(objs->root != NULL) objs->world = *(objs->root);
	*J_ROOT = objs->world;

	_sceVu0UnitMatrix(WORK_MAT);

	obj = objs->objs;
	rots = objs->rots;
	joints = JOINTS;
	for(i = 0 ; i < objs->def->n_models; i++, obj++, rots++)
	{
		_sceVu0UnitMatrix(WORK_MAT);
		if(rots->vx != 0.0f) _sceVu0RotMatrixX(WORK_MAT, WORK_MAT, rots->vx);
		if(rots->vy != 0.0f) _sceVu0RotMatrixY(WORK_MAT, WORK_MAT, rots->vy);
		if(rots->vz != 0.0f) _sceVu0RotMatrixZ(WORK_MAT, WORK_MAT, rots->vz);
		*(FVECTOR*)WORK_MAT->m[3] = obj->trans;
		_sceVu0MulMatrix((void*)&joints[i], (void*)&joints[obj->parent], (void*)WORK_MAT);
		obj->world = joints[i];
	}
}


