/*
	demothrd.c
	    デモスレッド：デモ進行制御

	2000/01/13 N.Tanaka
	$Id: demothrd.c,v 1.1.1.3 2002/11/19 11:46:43 Yoshizawa1 Exp $
*/

/******************************************************************************
 * included
 */

#include <stdio.h>

#include <libdma.h>
#include <sifdev.h>

#include <libgraph.h>
#include <libvu0.h>
#include <libdg.h>
#include <gameheader.h>
#include <camera.h>

#include "demo.h"
#include "demo_.h"


/******************************************************************************
 * functions
 */

static void ActFile(LPMGSDEMOACT lpAct);
static void DieFile(LPMGSDEMOACT lpAct);
static int  ReadFileData(LPMGSDEMOACT lpAct, char* lpstrFullName);
static void DestroyDemoAct(LPMGSDEMOACT lpAct);

#ifdef DEMO_DEBUG_MODE
static void DemoDebugFunction1(LPMGSDEMOACT lpAct, int* lpnTime, int nVSync);
#endif

/******************************************************************************
 * publics
 */

/******************************************************************************
 * デモスレッド(ファイル版)
 */
#ifdef DEMO_DEBUG_MODE
void* DM_ThreadFile(
	DWORD dwStyle,
	char* lpstrFileName,		/* シナリオで指定されたデモファイルの名前 */
	int   nLoop,
	char* lpstrDrawString)
#else
void* DM_ThreadFile(
	DWORD dwStyle,
	char* lpstrFileName)		/* シナリオで指定されたデモファイルの名前 */
#endif
{
	int           nCameraName;
	char          strFullName[MAX_STRING];
	LPMGSDEMOACT  lpAct;
	GM_CameraSet* cam;

#ifdef DEMO_DEBUG_MODE
	int nLength;
#endif

	OPERATOR();

	/* デモスレッド起動 */
	if((lpAct = (LPMGSDEMOACT)
		GV_NewActor(GV_ACTOR_MANAGER, sizeof(MGSDEMOACT))) == NULL)
		return NULL;

	lpAct->dwStyle = dwStyle;
	lpAct->nTime   = -1;
	lpAct->nDemoID = GV_StrCode(lpstrFileName);

	/* アクター登録 */
	GV_SetActor(&lpAct->actor, ActFile, DieFile);
	GV_ActorEX(&lpAct->actor);
	lpAct->nMap = GM_CurrentMap;

	/* 開発機専用の領域にデモファイルを読み込む */
#ifdef DEMO_DEBUG_MODE
	lpAct->lpBottomPointer = (void*)0x02000000;
	lpAct->nLoop   = nLoop;
	if(lpstrDrawString == NULL)
		lpAct->szDrawString[0] = '\0';
	else
	{
		nLength = strlen(lpstrDrawString);
		if(nLength >= 1024)
		{
			sprintf(lpAct->szDrawString, "Over 1024 Characters String.\0");
		}
		else
		{
			strcpy(lpAct->szDrawString, lpstrDrawString);
			lpAct->szDrawString[nLength] = '\0';
		}
		lpAct->nViewParam = DEMO_DEBUG_PARAM_STRING;
	}
#endif

	/* デモファイル読み込み */
	if(lpstrFileName == NULL)
	{
		printf("Error No Demo File!\n");
		return NULL;
	}

	sprintf(strFullName, "%s.de2", lpstrFileName);
	if(ReadFileData(lpAct, strFullName) == 0)
		return NULL;

	/* デモカメラ設定 */
	nCameraName = GV_StrCode( "デモカメラ" ) ;

	/* かなり強め */
	lpAct->lpGMCamera = cam = NewProgramCamera(nCameraName, 0, GM_CAMERA_PROG1, 128);
#ifdef DEMO_DEBUG_MODE
	GM_SetCameraType(cam, GM_CAM_TYPE_CAMERA_AND_TARGET, CAM_FLAG_FIX | CAM_FLAG_PAUSE_NO_STOP);
#else
	GM_SetCameraType(cam, GM_CAM_TYPE_CAMERA_AND_TARGET, CAM_FLAG_FIX);
#endif
	GM_SetCameraAngle(cam, 2.0F) ;
	GM_SetCameraInterpMode(cam, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_QUICK, 0, 0);
	cam->on = 0 ;

	return (void*)(lpAct);
}

/******************************************************************************
 * statics
 */

/******************************************************************************
 * アクションプロシージャ(ファイル版)
 */
static void ActFile(
	LPMGSDEMOACT lpAct)	/* デモプロセス構造体のポインタ */
{
	int          nVSync;
	int          nTime;
	BOOL         bNext;

	OPERATOR();

/*	nVSync = GV_Time;*/
	nVSync = DG_TickCount;

	if(lpAct->nTime == -1)
	{
		/* 初期動作 */
		if(CreateDemo(lpAct, (LPMGSDEMO)lpAct->lpExtMemory) == 0)
		{
			printf("Error:Initialize demo\n");
			GV_DestroyActor(lpAct);
		}
		lpAct->nTime = 0;

		/* カメラ切り変え：初期化時に始めてデモカメラを有効にする */
		lpAct->lpGMCamera->on = 1 ;
		GM_ChangeCamera(0);

		return;
	}
	else if(lpAct->nVSync == 0)
	{
		/* こうしないと CreateDemo 終了時から FrameRunDemo 迄の間に */
		/* 他のプロセスの初期化で時間を取られることがある */
		/* DG_TickCountは60フレームでカウントしているので引く値は１になる */
		/* ゲームのシステムが30フレームなら２を引く */
		lpAct->nVSync = nVSync - 1;	/* 1フレーム目を実行 */
		printf("PlayDemoSound\n");	/* デモサウンド再生ターミナル起動 */
	}

	if((lpAct->dwStyle & DMT_WRITEFILEAVI) != 0)
		nTime = lpAct->nTime + 1;
	else
		nTime = (nVSync - lpAct->nVSync);

#ifdef DEMO_DEBUG_MODE
	/* デバッグモード用の処理 */
	/* デバッグモードパラメータ変更 */
	if((GV_PadData[1].press & PAD_SEL) != 0)
	{
		if(++(lpAct->nDebugMode) > MODE_COUNT)
		{
			lpAct->nRepeatCount    = 0;
			lpAct->nDebugMode      = 0;
		}
	}
	/* 表示の切り替え */
	if((GV_PadData[1].press & PAD_R2) != 0)
	{
		if(++(lpAct->nViewParam) > VIEW_COUNT)
			lpAct->nViewParam    = 0;
	}

	/* カメラタイプが違っている可能性があるので常に変更する */
	lpAct->lpGMCamera->type = GM_CAM_TYPE_CAMERA_AND_TARGET;
	/* ポーズレベルを常に解除：デバッグモード固有処理内で */
	/* ポーズをかけている場合があるから */
	GV_PauseOffActorSystem(GV_PAUSE_PAUSE);

	/* デバッグモード固有処理へ */
	if(lpAct->nDebugMode != 0)
	{
		DemoDebugFunction1(lpAct, &nTime, nVSync);
	}
#endif

	/* 処理落ちしたフレームを出力 */
	if((nTime - lpAct->nTime) >= 2)
		printf("frame over time = %d\n", lpAct->nTime * MGS_DEMOSECONDRATE);

	lpAct->nTime = nTime;

	if(lpAct->nTime > lpAct->lpDemo->nFrameCount)
		bNext = FALSE;
	else
	{
		while(lpAct->nTime != ((LPMGSDEMO)lpAct->lpExtMemory)->nTime)
		{
			lpAct->lpExtMemory = (char*)lpAct->lpExtMemory
				+ ((LPMGSDEMO)lpAct->lpExtMemory)->nSize;
		}

		/* シーンの描画 */
		bNext = FrameRunDemo(lpAct, (PMGSDEMOFRAME)lpAct->lpExtMemory);
	}

#ifdef DEMO_DEBUG_MODE
	/* キャンセルボタンを押したときの処理 */
	if( (lpAct->nDebugMode != DEMO_DEBUG_CAMERA) &&
		((GV_PadData[1].status & PAD_B) != 0))
		bNext = FALSE;
#else
	if((GV_PadData[1].status & PAD_B) != 0)
		bNext = FALSE;
#endif


#ifdef DEMO_DEBUG_MODE
	/* ループモードか否かで処理が変わる */
	if(lpAct->nLoop == 0)
	{
		/* ループモードにあらず。終了 */
		if(bNext == FALSE)
			DestroyDemoAct(lpAct);
	}
	else
	{
		/* ループモードなので最初に戻す */
		if(bNext == FALSE)
		{
			/* ポーズを一時的に解除する */
			GV_PauseOffActorSystem(GV_PAUSE_PAUSE);

			DestroyDemo(lpAct);

			lpAct->lpExtMemory = lpAct->lpExtMemoryBackup;
			if(CreateDemo(lpAct, (LPMGSDEMO)lpAct->lpExtMemory) == 0)
			{
				printf("Error:Initialize demo\n");
				DestroyDemoAct(lpAct);
			}
			lpAct->nTime = 0;
			lpAct->nVSync = nVSync;
		}
	}
#else
	if(bNext == FALSE)
		DestroyDemoAct(lpAct);
#endif

}

/******************************************************************************
 * デモの親子関係に当たるキャラに終了を告知する
 */
static void DestroyDemoAct(
	LPMGSDEMOACT lpAct)	/* デモプロセス構造体のポインタ */
{
	/* 子に告知 */
	GV_CallChildSignalFunc(lpAct, GV_SIGNAL_KILL, 0);
	/* 親に告知 */
	GV_CallParentSignalFunc(lpAct, GV_SIGNAL_KILL, 0);
	/* キャラ終了 */
	GV_DestroyActor(lpAct);
}

/******************************************************************************
 * デアクションプロシージャ(ファイル版)
 */
static void DieFile(
	LPMGSDEMOACT lpAct)	/* デモプロセス構造体のポインタ */
{
	OPERATOR();

	DestroyDemo(lpAct);

	/* カメラの復旧 */
	lpAct->lpGMCamera->on = 0;
	GM_ChangeCamera(0);

	/* カメラ構造体の開放 */
	if(lpAct->lpGMCamera != NULL)
		GM_DeleteCamera(lpAct->lpGMCamera);
}

/******************************************************************************
 * ファイル読み込み関数。拡張用のメモリに読み込みます
 */
static int ReadFileData(	/* 0 : 異常 */
	LPMGSDEMOACT lpAct,
	char*        lpszFileName)	/* 拡張子を含めたファイル名 */
{
	int   nFile;
	int   nRead;
	long  lFileSize;
	char  szFullName[MAX_STRING];
	void* lpBuffer;
#ifdef DEMO_DEBUG_MODE
	long  lPointer;
#endif

	lpAct->lpExtMemory = (void*)0x02000000;
#ifdef DEMO_DEBUG_MODE
	lpAct->lpExtMemoryBackup = lpAct->lpExtMemory;
#endif

	/* 渡されるのはディレクトリなしのファイル名である。 */
	sprintf(szFullName, "host0:stage/%s/%s", GM_GetArea(), lpszFileName);

	/* デモファイルの読み込み */
	if((nFile = sceOpen(szFullName, SCE_RDONLY)) < 0)
	{
		printf("\"%s\" not found\n", szFullName);
		GV_DestroyActor(lpAct);
		return 0;
	}

	lFileSize = sceLseek(nFile, 0, SEEK_END);
	sceLseek(nFile, 0, SEEK_SET);
	lpBuffer = (void*)lpAct->lpExtMemory;

	ASSERT( lpBuffer != NULL );

	/* 拡張メモリの最後の１Ｍは別に使っているのでそこまで食い込むファイルは不可 */
#if 1
	if((void*)(lpAct->lpExtMemory + lFileSize) >= (void*)(0x07F00000))
#else
/* E3対応 */
	if((void*)(lpAct->lpExtMemory + lFileSize) >= (void*)MEM_ADDR)
#endif
	{
		printf("Too Large Demo File! (%s)\n", szFullName);
		printf("Size = %d\n", lFileSize);
		GV_DestroyActor(lpAct);
		return 0;
	}

	while(lFileSize > 0)
	{
		if((nRead = lFileSize) > LOAD_UNIT)
			nRead = LOAD_UNIT;

		if((nRead = sceRead(nFile, lpBuffer, nRead)) < 0)
		{
			sceClose(nFile);
			GV_DestroyActor(lpAct);
			return 0;
		}

		lFileSize -= nRead;
		lpBuffer  += nRead;
	}

#ifdef DEMO_DEBUG_MODE
	lPointer = sceLseek(nFile, 0, SEEK_CUR);
	lpAct->lpBottomPointer += lPointer;
	printf("Bottom Adress = 0x0%x\n", lpAct->lpBottomPointer);
#endif
	sceClose(nFile);

	return 1;
}

#ifdef DEMO_DEBUG_MODE
/******************************************************************************
 * デバッグモード固有緒利関数その１
 */
static void DemoDebugFunction1(	/* 0 : 異常 */
	LPMGSDEMOACT lpAct,
	int*         lpnTime,
	int          nVSync)
{
	int     nRot;
	int     nZoom;
	int     track;
	float   fMove;
	SVECTOR svectRotate;
	FVECTOR fvectShift;

	/* 実行を停止 */
	lpAct->nVSync++;
	*lpnTime = lpAct->nTime + 1;
	GV_PauseOnActorSystem(GV_PAUSE_PAUSE);

	switch(lpAct->nDebugMode)
	{
	case DEMO_DEBUG_VIEW:
		_MENU_Locate(12, 12, 0);
		_MENU_Printf("Demo Debug Mode : View\n");
		break;

	case DEMO_DEBUG_CAMERA:
		_MENU_Locate(12, 12, 0);
		_MENU_Printf("Demo Debug Mode : Camera\n");
		break;

	default:
		break;
	}

	if((GV_PadData[1].status & PAD_L1) == 0)
	{
		/* スロー再生 */
		if((GV_PadData[1].press & PAD_X) != 0)
			lpAct->nRepeatCount = REPEAT_COUNT;

		if((GV_PadData[1].status & PAD_X) == 0)
		{
			/*  */
			if(*lpnTime != 1) *lpnTime = lpAct->nTime;
		}
		else
		{
			if(lpAct->nRepeatCount < REPEAT_COUNT)
			{
				if(*lpnTime != 1) *lpnTime = lpAct->nTime;
				lpAct->nRepeatCount++;
			}
			else
			{
				/* 再生を行う */
				if((lpAct->dwStyle & DMT_WRITEFILEAVI) == 0) lpAct->nVSync--;
				lpAct->nRepeatCount = 1;
				GV_PauseOffActorSystem(GV_PAUSE_PAUSE);
			}
		}

		/* 逆転再生（ただし、PAD_X が押されていないことが条件） */
		if((GV_PadData[1].status & PAD_X) == 0)
		{
			/* 逆転再生 */
			if((GV_PadData[1].press & PAD_Y) != 0)
				lpAct->nRepeatCount = REPEAT_COUNT;

			if((GV_PadData[1].status & PAD_Y) == 0)
			{
				if(*lpnTime != 1) *lpnTime = lpAct->nTime;
			}
			else
			{
				if(lpAct->nRepeatCount < REPEAT_COUNT)
				{
					if(*lpnTime != 1) *lpnTime = lpAct->nTime;
					lpAct->nRepeatCount++;
				}
				else
				{
					/* 逆転再生を行う */
					lpAct->lpExtMemory = lpAct->lpExtMemoryBackup;
					if(lpAct->nTime != 1) *lpnTime = lpAct->nTime - 1;
					if(((lpAct->dwStyle & DMT_WRITEFILEAVI) == 0) && (lpAct->nTime != 1))
						lpAct->nVSync++;

					lpAct->nRepeatCount = 1;
					GV_PauseOffActorSystem(GV_PAUSE_PAUSE);
				}
			}
		}
	}
	else
	{
		lpAct->nVSync--;
		*lpnTime = (nVSync - lpAct->nVSync);
		GV_PauseOffActorSystem(GV_PAUSE_PAUSE);
	}

	/* カメラ移動：視点のみ */
	if(lpAct->nDebugMode == DEMO_DEBUG_CAMERA)
	{
		if((GV_PadData[1].status & PAD_R1) != 0)
		{
			if(lpAct->nMoveSpeedLevel == 0)
				lpAct->nMoveSpeedLevel = 1;

			if((GV_PadData[1].press & PAD_L2) != 0)
			{
				lpAct->nMoveSpeedLevel += 2;
				if(lpAct->nMoveSpeedLevel > 6)
					lpAct->nMoveSpeedLevel = 1;
			}

			fMove = MOVE_COUNT * lpAct->nMoveSpeedLevel;
			nRot  = ROTATE_COUNT * lpAct->nRotateSpeedLevel;
			nZoom = ZOOM_COUNT * lpAct->nRotateSpeedLevel;

			_MENU_Locate(236, 12, 0);
			_MENU_Printf("Move");
			_MENU_Locate(280, 12, 0);
			_MENU_Printf("x%d\n", lpAct->nMoveSpeedLevel);

			/* 現在のRotateとtrackを求める */
			GM_CameraMakeRotate(&(lpAct->lpGMCamera->position),
				&(lpAct->lpGMCamera->target), &svectRotate, &track);

			/* 平行移動 */
			fvectShift = DG_ZeroVector;
			if((GV_PadData[1].status & PAD_U) != 0)
				fvectShift.vy = fMove;
			if((GV_PadData[1].status & PAD_D) != 0)
				fvectShift.vy = -fMove;
			if((GV_PadData[1].status & PAD_R) != 0)
				fvectShift.vx = -fMove;
			if((GV_PadData[1].status & PAD_L) != 0)
				fvectShift.vx = fMove;

			/* 平行移動量の設定 */
			DG_SetPos2(&DG_ZeroVector, &svectRotate);
			DG_RotVector(&fvectShift, &fvectShift, 1);
			_sceVu0AddVector(&(lpAct->lpGMCamera->position),
				&(lpAct->lpGMCamera->position), &fvectShift);
			_sceVu0AddVector(&(lpAct->lpGMCamera->target),
				&(lpAct->lpGMCamera->target), &fvectShift);
		}
		else
		{
			if(lpAct->nRotateSpeedLevel == 0)
				lpAct->nRotateSpeedLevel = 1;

			if((GV_PadData[1].press & PAD_L2) != 0)
			{
				lpAct->nRotateSpeedLevel++;
				if(lpAct->nRotateSpeedLevel > 3)
					lpAct->nRotateSpeedLevel = 1;
			}

			fMove = MOVE_COUNT * lpAct->nMoveSpeedLevel;
			nRot  = ROTATE_COUNT * lpAct->nRotateSpeedLevel;
			nZoom = ZOOM_COUNT * lpAct->nRotateSpeedLevel;

			_MENU_Locate(236, 12, 0);
			_MENU_Printf("Rotate");
			_MENU_Locate(296, 12, 0);
			_MENU_Printf("x%d\n", lpAct->nRotateSpeedLevel);

			/* 回転 */
			if((GV_PadData[1].status & PAD_U) != 0)
				lpAct->lpGMCamera->rotate.vx = (lpAct->lpGMCamera->rotate.vx + nRot) & 4095;

			if((GV_PadData[1].status & PAD_D) != 0)
				lpAct->lpGMCamera->rotate.vx = (lpAct->lpGMCamera->rotate.vx - nRot) & 4095;

			if((GV_PadData[1].status & PAD_R) != 0)
				lpAct->lpGMCamera->rotate.vy = (lpAct->lpGMCamera->rotate.vy + nRot) & 4095;

			if((GV_PadData[1].status & PAD_L) != 0)
				lpAct->lpGMCamera->rotate.vy = (lpAct->lpGMCamera->rotate.vy - nRot) & 4095;

			if((GV_PadData[1].status & PAD_A) != 0)
				lpAct->lpGMCamera->track += nZoom;

			if((GV_PadData[1].status & PAD_B) != 0)
				lpAct->lpGMCamera->track -= nZoom;

			if(lpAct->lpGMCamera->track < 100)
				lpAct->lpGMCamera->track = 100;

			/* 回転を考慮した新しいpositionを求める */
			GM_CameraMakeCamera(&(lpAct->lpGMCamera->position),
				&(lpAct->lpGMCamera->target), &(lpAct->lpGMCamera->rotate),
				&(lpAct->lpGMCamera->track));
			lpAct->lpGMCamera->type = GM_CAM_TYPE_TARGET_AND_ROTATE ;
		}
	}
}

#endif