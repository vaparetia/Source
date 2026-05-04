/*
	smk_blur.c

	2000/04/18 S.Yamashita
	$Id: smk_blur.c,v 1.1.1.3 2002/11/19 11:46:49 Yoshizawa1 Exp $

*/

/*
#蘭(EUC判定用文字)
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

#include "../demo/demo.h"
#include "../demo/demo_.h"
#include "../demo/DE2Struct.h"

/******************************************************************************
 * functions
 */
extern void *NewSmokeBlurEffect(FMATRIX *world, int start_speed, int end_speed, int start_size, int end_size, int spot_size, int spot_angle, int n_prims, int interval, int color, int flag);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX  actor;
	int        nCount;
	FMATRIX*   lpfmatObjectWorld;
	FMATRIX    fmatWorld;
	FVECTOR    fvectMove;
	FVECTOR    fvectRot;
	void*      lpWork;
} Work;

/******************************************************************************
 * locals
 */


/******************************************************************************
 * publics
 */

/******************************************************************************
 * 実行部分
 */
static void Act( Work *work )
{
	_sceVu0RotMatrix(&(work->fmatWorld), work->lpfmatObjectWorld, &(work->fvectRot));
	_sceVu0TransMatrix(&(work->fmatWorld), &(work->fmatWorld), &(work->fvectMove));

	if(work->nCount-- <= 0)
	{
		if(work->lpWork != NULL)
		{
			GV_DestroyActor(work->lpWork);
			work->lpWork = NULL;
		}
	}
}

/******************************************************************************
 * 終了関数
 */
static void Die( Work *work )
{
}

/******************************************************************************
 * シグナルを受ける関数
 */
static int ReceiveSignal(void *workp, int signal, int value)
{
	Work* work;
	work = workp;

	switch(signal)
	{
	case GV_SIGNAL_KILL:
		if(work->lpWork != NULL)
			GV_DestroyActor(work->lpWork);

		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor(work);
		return 1;
	}
	return 0;
}

/******************************************************************************
 * リソースの確保
 */
static int GetResources(Work *work, void* lpParam, LPMGSDEMOMODEL lpModel)
{
	int      nScene;
	int      nNode;
	DG_OBJS* lpDGObject;

printf(
"Smoke Blur:\n"
"nScene       = %d\n"
"nNode        = %d\n"
"fvectMove.vx = %f\n"
"fvectMove.vy = %f\n"
"fvectMove.vz = %f\n"
"fvectRot.vx  = %f\n"
"fvectRot.vy  = %f\n"
"fvectRot.vz  = %f\n"
"nStartSpeed  = %d\n"
"nEndSpeed    = %d\n"
"nStartSize   = %d\n"
"nEndSize     = %d\n"
"nSpotSize    = %d\n"
"nSpotAngle   = %d\n"
"nPrims       = %d\n"
"nInterval    = %d\n"
"nColor       = %x\n"
"nFlag        = %x\n"
"nCount       = %x\n\n",
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nScene,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nNode,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->fvectMove.vx,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->fvectMove.vy,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->fvectMove.vz,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->fvectRot.vx,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->fvectRot.vy,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->fvectRot.vz,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nStartSpeed,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nEndSpeed,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nStartSize,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nEndSize,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nSpotSize,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nSpotAngle,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nPrims,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nInterval,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nColor,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nFlag,
	((LPDE2EFFECTSMOKEBLUR)(lpParam))->nCount);

	nScene     = ((LPDE2EFFECTSMOKEBLUR)lpParam)->nScene;
	nNode      = ((LPDE2EFFECTSMOKEBLUR)lpParam)->nNode;
	lpDGObject = (lpModel + nScene)->object.objs;

	work->nCount = ((LPDE2EFFECTSMOKEBLUR)(lpParam))->nCount;

	work->fvectMove         = ((LPDE2EFFECTSMOKEBLUR)lpParam)->fvectMove;
	work->fvectRot          = ((LPDE2EFFECTSMOKEBLUR)lpParam)->fvectRot;
	work->lpfmatObjectWorld = &(lpDGObject->objs[nNode].world);

	_sceVu0RotMatrix(&(work->fmatWorld), work->lpfmatObjectWorld, &(work->fvectRot));
	_sceVu0TransMatrix(&(work->fmatWorld), &(work->fmatWorld), &(work->fvectMove));

	work->lpWork = NewSmokeBlurEffect(
		&(work->fmatWorld),
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nStartSpeed,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nEndSpeed,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nStartSize,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nEndSize,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nSpotSize,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nSpotAngle,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nPrims,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nInterval,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nColor,
		((LPDE2EFFECTSMOKEBLUR)(lpParam))->nFlag);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoSmokeBlurEffect(void *lpParam, LPMGSDEMOMODEL lpModel)
{
	Work *work;

	OPERATOR();

	work = (Work *)GV_NewActor(GV_ACTOR_USER, sizeof(Work));
	if(work != NULL)
	{
		GV_SetActor(&(work->actor), Act, Die);
		GV_ActorEX(&(work->actor))
		GV_SetActorSignalFunc(work, ReceiveSignal);
		if(GetResources(work, lpParam, lpModel) < 0)
		{
			GV_DestroyActor(work);
			return NULL;
		}
	}
	return (void *)work;
}
