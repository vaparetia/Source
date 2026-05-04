/*
	demoeffect.c
	    デモエフェクト分岐部分

	2000/04/07 N.Tanaka
	$Id: demoeffect.c,v 1.1.1.3 2002/11/19 11:46:42 Yoshizawa1 Exp $
*/

/******************************************************************************
 * included
 */

#include <stdio.h>

#include <libdma.h>
#include <libgraph.h>
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

/******************************************************************************
 * locals
 */


/******************************************************************************
 * publics
 */


/******************************************************************************
 * 効果の表示
 */
BOOL ShowEffect(	/* 0以外	正常 */
						/* 0 */
	LPMGSDEMOACT       lpAct,		/* デモプロセス構造体のポインタ */
	LPMGSDEMOEFFECT    lpEffect,	/* 効果 */
	LPMGSDEMOEFFECTACT lpEAct)		/* 効果実行監視情報 */
{
	int      nScene1;
	int      nScene2;
	int      nNode1;
	int      nNode2;
	void*    lpfnEffect;
	GV_MSG   gvMessage;
	DG_OBJS* lpDGObject1;
	DG_OBJS* lpDGObject2;
	FVECTOR  fvectBound1;
	FVECTOR  fvectBound2;

	OPERATOR();

	/* メッセージ処理 */
	if((*(int*)(lpEffect->byBuffer) & ~0x01000000) == DE2_EFFECTMESSAGE)
	{
		memset(&gvMessage, 0x00, sizeof(GV_MSG));
		gvMessage.address     = ((LPDE2EFFECTMESSAGE)(lpEffect->byBuffer))->nName;
		gvMessage.message_len = ((LPDE2EFFECTMESSAGE)(lpEffect->byBuffer))->nLength;
		gvMessage.message     = &(((LPDE2EFFECTMESSAGE)(lpEffect->byBuffer))->nParam[0]);
		GV_SendMessage(&gvMessage);
		return 1;
	}

	/* エフェクトの関数ポインタを獲得する */
	if((lpfnEffect = GM_GetCharaID(*(int*)(lpEffect->byBuffer))) == NULL)
	{
		printf("lpEffect->nEffectID : %x\n", *(int*)(lpEffect->byBuffer));
		return 1;
	}

	switch(*(int*)(lpEffect->byBuffer) & ~0x01000000)
	{
	/* ダミーキャラ起動 */
	case DE2_EFFECTPIYORI:					/* ぴよぴよ（ノーマル） */
	case DE2_EFFECTPIYORIANES:				/* ぴよぴよ（麻酔） */
	case DE2_EFFECTZZZ:						/* 居眠り */
	case DE2_EFFECTCIRCLELIGHT:				/* 敵兵の銃のライト */
	case DE2_EFFECTFADEINOUT:				/* フェードイン・アウト */
	case DE2_EFFECTBLOODDEMO:				/* 任意血 */
	case DE2_EFFECTPLASMALINEDEMO:			/* プラズマライン */
	case DE2_EFFECTENEEQUIP:				/* 敵兵装備品 */
	case DE2_EFFECTBODYSHADOW:				/* 遮り影 */
	case DE2_EFFECTCARTRIDGECONTROL:		/* 銃のカートリッジ */
	case DE2_EFFECTNEARFOCUS:				/* 近景ぼかし */
	case DE2_EFFECTFARFOCUS:				/* 遠景ぼかし */
	case DE2_EFFECTBODYSHADOWVOLUMEDEMO:	/* ボリューミックシャドーライン */
	case DE2_EFFECTGBSHANDDEMO:				/* ゴルルコ フィンガーサイン */
	case DE2_EFFECTPUTSTANIMEOBJECTCALL:	/* 頂点ストリームアニメーション */
	case DE2_EFFECTE3FACEANIMATION:			/* Ｅ３限定フェイスアニメ制御 */
	case DE2_EFFECTPLASMAPOLYDEMO:			/* プラズマポリゴン */
	case DE2_EFFECTROPEMODEL3:				/* ロープモデル３ */
	case DE2_EFFECTOOZEBLOODDEMO:			/* 敵兵滲み血 */
	case DE2_EFFECTWAVINGCLOTHMODELW:		/* オブジェに付く布モデル */
	case DE2_EFFECTROPEMODEL2:				/* ロープモデル２ */
	case DE2_EFFECTEVMMMORGA:				/* オルガの揺れ物 */
	case DE2_EFFECTSMOKEBLUR:				/* 煙型ブラー */
	case DE2_EFFECTBODYSPLASH:				/* 体からの水はね */
	case DE2_EFFECTRAINCOAT:				/* レインコートの揺れ */
	case DE2_EFFECTSHADOW:					/* 足影 */
	case DE2_EFFECTDROPSHADOW:				/* キャラ影 */
	case DE2_EFFECTATTACHMENT3:				/* 装備品Ｃ */
	case DE2_EFFECTINTERPOLYDEMO:			/* ポリゴン頂点補完残像 */
	case DE2_EFFECTPUTATTACHMENTS:			/* ゆれる付属品 */
	case DE2_EFFECTFOOTSPLASH:				/* 足元水飛沫 */
	case DE2_EFFECTSAASMOKE:				/* ＳＡＡ銃口煙 */
	case DE2_EFFECTVANIMEBULLER:			/* 頂点アニメブラー */
	case DE2_EFFECTDEMOBULLET:				/* 弾丸 */
	case DE2_EFFECTGBSFACEDEMO:				/* ゴルルゴ兵目パチ */
		GV_SetActorChild(lpAct,
			((CHARAMODEL)lpfnEffect)(lpEffect->byBuffer, lpAct->lpModel));
		break;

	case DE2_EFFECTC4EXPLOSION:				/* 爆発 */
		((CHARAFVECT)lpfnEffect)(&(((LPDE2EFFECTC4EXPLOSION)(lpEffect->byBuffer))->fvectCenter));
		break;

	case DE2_EFFECTBLOOD2D:					/* 血煙 */
		((CHARAFVECT)lpfnEffect)(&((LPDE2EFFECTBLOOD2D)(lpEffect->byBuffer))->fvectCenter);
		break;

	case DE2_EFFECTBREATH:					/* 息 */
		((CHARAFMATFVECT)lpfnEffect)(&((LPDE2EFFECTBREATH)(lpEffect->byBuffer))->fmatWorld,
			&((LPDE2EFFECTBREATH)(lpEffect->byBuffer))->fvectMove);
		break;

	case DE2_EFFECTSHIELDSMOKE:				/* 盾煙 */
		((CHARAFMAT)lpfnEffect)(&((LPDE2EFFECTSHIELDSMOKE)(lpEffect->byBuffer))->fmatWorld);
		break;

	case DE2_EFFECTSPARK1:					/* 火花 */
		((CHARASPARK)lpfnEffect)(
			((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->nPrims,
			&((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->fvectCenter,
			((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->fMinSpeed,
			((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->fSpeedWide,
			((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->fGravity,
			&((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->svectRot,
			&((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->svectRotWide,
			&((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->fvectColor,
			((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->fLength,
			((LPDE2EFFECTSPARK1)(lpEffect->byBuffer))->nCount);
		break;

	case DE2_EFFECTCAMERABLOOD:				/* カメラ血飛沫 */

printf(
"\nCameraBlood:\n"
"   Pos X = %f\n"
"   Pos Y = %f\n\n",
	((LPDE2EFFECTCAMERABLOOD)(lpEffect->byBuffer))->fvectPosPers.vx,
	((LPDE2EFFECTCAMERABLOOD)(lpEffect->byBuffer))->fvectPosPers.vy
);

		((CHARACAMERABLOOD)lpfnEffect)(
			&((LPDE2EFFECTCAMERABLOOD)(lpEffect->byBuffer))->fvectPosPers,
			((LPDE2EFFECTCAMERABLOOD)(lpEffect->byBuffer))->nWhite);
		break;

	case DE2_EFFECTBLOODSPREAD:				/* 床に広がる血 */

printf(
"\nBloodSpread:\n"
"   Pos X  = %f\n"
"   Pos Y  = %f\n"
"   Pos Z  = %f\n"
"   Rot X  = %d\n"
"   Rot Y  = %d\n"
"   Rot Z  = %d\n"
"   nDecay = %d\n"
"   fSize  = %f\n\n",
	((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->fvectPos.vx,
	((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->fvectPos.vy,
	((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->fvectPos.vz,
	((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->svectRot.vx,
	((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->svectRot.vy,
	((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->svectRot.vz,
	((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->nDecay,
	((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->fSize
);

		((CHARABLOODSPREAD)lpfnEffect)(
			&((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->fvectPos,
			&((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->svectRot,
			((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->nDecay,
			((LPDE2EFFECTBLOODSPREAD)(lpEffect->byBuffer))->fSize);
		break;

	case DE2_EFFECTWALLBLOOD:				/* 壁に付着する血 */
		((CHARAWALLBLOOD)lpfnEffect)(
			&((LPDE2EFFECTWALLBLOOD)(lpEffect->byBuffer))->fvectPos,
			&((LPDE2EFFECTWALLBLOOD)(lpEffect->byBuffer))->svectRot,
			((LPDE2EFFECTWALLBLOOD)(lpEffect->byBuffer))->nWhite);
		break;

	case DE2_EFFECTFADEINOUTDEMO:			/* フェードイン・アウトデモ */
		((CHARAFADEINOUTDEMO)lpfnEffect)(
			((LPDE2EFFECTFADEINOUTDEMO)(lpEffect->byBuffer))->nColorR,
			((LPDE2EFFECTFADEINOUTDEMO)(lpEffect->byBuffer))->nColorG,
			((LPDE2EFFECTFADEINOUTDEMO)(lpEffect->byBuffer))->nColorB,
			((LPDE2EFFECTFADEINOUTDEMO)(lpEffect->byBuffer))->nColorA,
			((LPDE2EFFECTFADEINOUTDEMO)(lpEffect->byBuffer))->nCount);
		break;

	case DE2_EFFECTBLOODDROPS:				/* 落ちる血 */
		((CHARADLOODDROPS)lpfnEffect)(
			&((LPDE2EFFECTBLOODDROPS)(lpEffect->byBuffer))->fvectPos,
			((LPDE2EFFECTBLOODDROPS)(lpEffect->byBuffer))->nDecay,
			((LPDE2EFFECTBLOODDROPS)(lpEffect->byBuffer))->nWhite,
			((LPDE2EFFECTBLOODDROPS)(lpEffect->byBuffer))->nSpreadSize);
		break;

	case DE2_EFFECTBODYPLASMADOTDEMO:		/* 光学迷彩壊れスパーク */
		nScene1      = ((LPDE2EFFECTBODYPLASMADOTDEMO)(lpEffect->byBuffer))->nScene;
		lpDGObject1 = (lpAct->lpModel + nScene1)->object.objs;

		((CHARABODYPLASMADOTDEMO)lpfnEffect)(
			lpDGObject1,
			((LPDE2EFFECTBODYPLASMADOTDEMO)(lpEffect->byBuffer))->nModel,
			((LPDE2EFFECTBODYPLASMADOTDEMO)(lpEffect->byBuffer))->nCount);
		break;

	case DE2_EFFECTFLUSH:					/* ＣＬＵＴフラッシュ */
		((CHARACTFLUSH)lpfnEffect)(
			((LPDE2EFFECTFLUSH)(lpEffect->byBuffer))->nDecay,
			((LPDE2EFFECTFLUSH)(lpEffect->byBuffer))->nMode);
		break;

	case DE2_EFFECTRAINCAMERADEMO:			/* カメラ前水滴 */
		((CHARARAINCAMERADEMO)lpfnEffect)(
			((LPDE2EFFECTRAINCAMERADEMO)(lpEffect->byBuffer))->nLife);
		break;

	case DE2_EFFECTSPLASHDEMO:				/* 風紋水飛沫 */
		fvectBound1 = ((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->fvectOrigin;
		fvectBound2 = ((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->fvectCenter;

		if((nScene1 = ((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->nScene1) >= 0)
		{
			nNode1      = ((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->nNode1;
			lpDGObject1 = (lpAct->lpModel + nScene1)->object.objs;
			fvectBound1.vx += lpDGObject1->objs[nNode1].world.m[3][0];
			fvectBound1.vy += lpDGObject1->objs[nNode1].world.m[3][1];
			fvectBound1.vz += lpDGObject1->objs[nNode1].world.m[3][2];
		}

		if((nScene2 = ((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->nScene2) >= 0)
		{
			nNode2      = ((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->nNode2;
			lpDGObject2 = (lpAct->lpModel + nScene2)->object.objs;
			fvectBound2.vx += lpDGObject2->objs[nNode2].world.m[3][0];
			fvectBound2.vy += lpDGObject2->objs[nNode2].world.m[3][1];
			fvectBound2.vz += lpDGObject2->objs[nNode2].world.m[3][2];
		}

		((CHARASPLASHDEMO)lpfnEffect)(
			&fvectBound1, &fvectBound2,
			((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->fRadius,
			((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->fSize,
			((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->nMultiple,
			((LPDE2EFFECTSPLASHDEMO)(lpEffect->byBuffer))->nLife);
		break;

	case DE2_EFFECTSPLASHPARTSDEMO:				/* 単発水飛沫 */
		((CHARASPLASHPARTSDEMO)lpfnEffect)(
			&(((LPDE2EFFECTSPLASHPARTSDEMO)(lpEffect->byBuffer))->fvectCenter),
			&(((LPDE2EFFECTSPLASHPARTSDEMO)(lpEffect->byBuffer))->svectRot),
			((LPDE2EFFECTSPLASHPARTSDEMO)(lpEffect->byBuffer))->fIntense);
		break;

	case DE2_EFFECTSPLASHRIPPLEDEMO:			/* 水飛沫複数呼び出し */
		((CHARASPLASHRIPPLEDEMO)lpfnEffect)(
			&(((LPDE2EFFECTSPLASHRIPPLEDEMO)(lpEffect->byBuffer))->fvectCenter),
			((LPDE2EFFECTSPLASHRIPPLEDEMO)(lpEffect->byBuffer))->fRadius,
			((LPDE2EFFECTSPLASHRIPPLEDEMO)(lpEffect->byBuffer))->fDirection,
			((LPDE2EFFECTSPLASHRIPPLEDEMO)(lpEffect->byBuffer))->fAngle,
			((LPDE2EFFECTSPLASHRIPPLEDEMO)(lpEffect->byBuffer))->fSize,
			((LPDE2EFFECTSPLASHRIPPLEDEMO)(lpEffect->byBuffer))->nMultiple,
			((LPDE2EFFECTSPLASHRIPPLEDEMO)(lpEffect->byBuffer))->nLife);
		break;

	case DE2_EFFECTSPRITEFOGWORLDDEMO:			/* テクセル流しワールド */
		fvectBound1 = ((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->fvectBound1;
		fvectBound2 = ((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->fvectBound2;

		if((nScene1 = ((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->nScene1) >= 0)
		{
			nNode1      = ((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->nNode1;
			lpDGObject1 = (lpAct->lpModel + nScene1)->object.objs;
			fvectBound1.vx += lpDGObject1->objs[nNode1].world.m[3][0];
			fvectBound1.vy += lpDGObject1->objs[nNode1].world.m[3][1];
			fvectBound1.vz += lpDGObject1->objs[nNode1].world.m[3][2];
		}

		if((nScene2 = ((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->nScene2) >= 0)
		{
			nNode2      = ((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->nNode2;
			lpDGObject2 = (lpAct->lpModel + nScene2)->object.objs;
			fvectBound2.vx += lpDGObject2->objs[nNode2].world.m[3][0];
			fvectBound2.vy += lpDGObject2->objs[nNode2].world.m[3][1];
			fvectBound2.vz += lpDGObject2->objs[nNode2].world.m[3][2];
		}

		((CHARASPRITEFOGWORLDDEMO)lpfnEffect)(
			&fvectBound1, &fvectBound2,
			&(((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->fvectDir),
			&(((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->fvectColor),
			((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->nTexName,
			((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->nSize,
			((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->nLife,
			((LPDE2EFFECTSPRITEFOGWORLDDEMO)(lpEffect->byBuffer))->nMode);
		break;

	case DE2_EFFECTSPRITEFOGPERSDEMO:			/* テクセル流しワールド透視変換 */
		fvectBound1 = ((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->fvectBound1;
		fvectBound2 = ((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->fvectBound2;

		if((nScene1 = ((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->nScene1) >= 0)
		{
			nNode1      = ((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->nNode1;
			lpDGObject1 = (lpAct->lpModel + nScene1)->object.objs;
			fvectBound1.vx += lpDGObject1->objs[nNode1].world.m[3][0];
			fvectBound1.vy += lpDGObject1->objs[nNode1].world.m[3][1];
			fvectBound1.vz += lpDGObject1->objs[nNode1].world.m[3][2];
		}

		if((nScene2 = ((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->nScene2) >= 0)
		{
			nNode2      = ((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->nNode2;
			lpDGObject2 = (lpAct->lpModel + nScene2)->object.objs;
			fvectBound2.vx += lpDGObject2->objs[nNode2].world.m[3][0];
			fvectBound2.vy += lpDGObject2->objs[nNode2].world.m[3][1];
			fvectBound2.vz += lpDGObject2->objs[nNode2].world.m[3][2];
		}

		((CHARASPRITEFOGPERSDEMO)lpfnEffect)(
			&fvectBound1, &fvectBound2,
			&(((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->fvectDir),
			&(((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->fvectColor),
			((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->nTexName,
			((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->nSize,
			((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->nLife,
			((LPDE2EFFECTSPRITEFOGPERSDEMO)(lpEffect->byBuffer))->nMode);
		break;

	case DE2_EFFECTSPRITEFOGONCAMERADEMO:			/* テクセル流しカメラ */
		((CHARASPRITEFOGONCAMERADEMO)lpfnEffect)(
			&(((LPDE2EFFECTSPRITEFOGONCAMERADEMO)(lpEffect->byBuffer))->fvectDir),
			&(((LPDE2EFFECTSPRITEFOGONCAMERADEMO)(lpEffect->byBuffer))->fvectColor),
			((LPDE2EFFECTSPRITEFOGONCAMERADEMO)(lpEffect->byBuffer))->nTexName,
			((LPDE2EFFECTSPRITEFOGONCAMERADEMO)(lpEffect->byBuffer))->nSize,
			((LPDE2EFFECTSPRITEFOGONCAMERADEMO)(lpEffect->byBuffer))->nLife,
			((LPDE2EFFECTSPRITEFOGONCAMERADEMO)(lpEffect->byBuffer))->nMode);
		break;

	case DE2_EFFECTOPTICALCAMOBREAKDEMO:		/* 光学迷彩壊れ */
		nScene1      = ((LPDE2EFFECTOPTICALCAMOBREAKDEMO)(lpEffect->byBuffer))->nScene;
		lpDGObject1 = (lpAct->lpModel + nScene1)->object.objs;

		((CHARAOPTICALCAMOUFLAGEBREAKDEMO)lpfnEffect)(lpDGObject1,
			((LPDE2EFFECTOPTICALCAMOBREAKDEMO)(lpEffect->byBuffer))->nType,
			((LPDE2EFFECTOPTICALCAMOBREAKDEMO)(lpEffect->byBuffer))->nStartCount,
			((LPDE2EFFECTOPTICALCAMOBREAKDEMO)(lpEffect->byBuffer))->nEndCount,
			((LPDE2EFFECTOPTICALCAMOBREAKDEMO)(lpEffect->byBuffer))->nColor);
		break;

	case DE2_EFFECTCONTRASTDEMO:			/* コントラスト＆ネガポジ */
		((CHARACONTRASTDEMO)lpfnEffect)(
			((LPDE2EFFECTCONTRASTDEMO)(lpEffect->byBuffer))->nColU_R,
			((LPDE2EFFECTCONTRASTDEMO)(lpEffect->byBuffer))->nColU_G,
			((LPDE2EFFECTCONTRASTDEMO)(lpEffect->byBuffer))->nColU_B,
			((LPDE2EFFECTCONTRASTDEMO)(lpEffect->byBuffer))->nColD_R,
			((LPDE2EFFECTCONTRASTDEMO)(lpEffect->byBuffer))->nColD_G,
			((LPDE2EFFECTCONTRASTDEMO)(lpEffect->byBuffer))->nColD_B,
			((LPDE2EFFECTCONTRASTDEMO)(lpEffect->byBuffer))->nNegaPosiFlag,
			((LPDE2EFFECTCONTRASTDEMO)(lpEffect->byBuffer))->nTime);
		break;

	case DE2_EFFECTTHUNDERDEMO:					/* 稲光 */
		((CHARATHUNDERDEMO)lpfnEffect)(
			((LPDE2EFFECTTHUNDERDEMO)(lpEffect->byBuffer))->fX,
			((LPDE2EFFECTTHUNDERDEMO)(lpEffect->byBuffer))->fY);
		break;

	case DE2_EFFECTFOGSETDEMO:				/* フォグセット */
		((CHARAFOGSETDEMO)lpfnEffect)(
			((LPDE2EFFECTFOGSETDEMO)(lpEffect->byBuffer))->nCol_R,
			((LPDE2EFFECTFOGSETDEMO)(lpEffect->byBuffer))->nCol_G,
			((LPDE2EFFECTFOGSETDEMO)(lpEffect->byBuffer))->nCol_B,
			((LPDE2EFFECTFOGSETDEMO)(lpEffect->byBuffer))->fNear,
			((LPDE2EFFECTFOGSETDEMO)(lpEffect->byBuffer))->fFar,
			((LPDE2EFFECTFOGSETDEMO)(lpEffect->byBuffer))->nTime);
		break;

	case DE2_EFFECTSPLASHMOTIONDEMO:		/* モーション連動水飛沫 */
		nScene1      = ((LPDE2EFFECTSPLASHMOTIONDEMO)(lpEffect->byBuffer))->nScene;
		lpDGObject1 = (lpAct->lpModel + nScene1)->object.objs;

		((CHARASPLASHMOTIONDEMO)lpfnEffect)(lpDGObject1,
			((LPDE2EFFECTSPLASHMOTIONDEMO)(lpEffect->byBuffer))->fStepLimit,
			((LPDE2EFFECTSPLASHMOTIONDEMO)(lpEffect->byBuffer))->nLife);
		break;

	case DE2_EFFECTSPARKDEMO:				/* 跳弾 */
		((CHARASPARKDEMO)lpfnEffect)(
			&((LPDE2EFFECTSPARKDEMO)(lpEffect->byBuffer))->fmatDWorld,
			&((LPDE2EFFECTSPARKDEMO)(lpEffect->byBuffer))->fvectDColor);
		break;
	}

	return 1;
}
