//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	raincoat.c

	2000/04/19 S.Yamashita
	$Id: raincoat.c,v 1.1.1.3 2002/11/19 11:46:49 Yoshizawa1 Exp $

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

#include "../demo/demo.h"
#include "../demo/demo_.h"
#include "../demo/DE2Struct.h"

/******************************************************************************
 * functions
 */
extern void *NewRaincoat_called(
	OBJECT *target,
	int *single_names,
	int single_names_size,
	int multi_name,
	float wind_min,
	float wind_max,
	float frame_min,
	float frame_max);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX  actor;
	int        nCount;
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

	nScene     = ((LPDE2EFFECTRAINCOAT)lpParam)->nScene;

printf(
"Raincoat:\n"
"  nScene           = %d\n"
"  nSingleNames[0]  = %d\n"
"  nSingleNames[1]  = %d\n"
"  nSingleNames[2]  = %d\n"
"  nSingleNames[3]  = %d\n"
"  nSingleNames[4]  = %d\n"
"  nSingleNames[5]  = %d\n"
"  nSingleNames[6]  = %d\n"
"  nSingleNames[7]  = %d\n"
"  nSingleNamesSize = %d\n"
"  nMultiName       = %d\n"
"  fWindMin         = %f\n"
"  fWindMax         = %f\n"
"  fFrameMin        = %f\n"
"  fFrameMax        = %f\n"
"  nCount           = %d\n"
"  nobjs            = %x\n\n",
	((LPDE2EFFECTRAINCOAT)(lpParam))->nScene,
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames[0],
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames[1],
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames[2],
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames[3],
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames[4],
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames[5],
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames[6],
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames[7],
	((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNamesSize,
	((LPDE2EFFECTRAINCOAT)(lpParam))->nMultiName,
	((LPDE2EFFECTRAINCOAT)(lpParam))->fWindMin,
	((LPDE2EFFECTRAINCOAT)(lpParam))->fWindMax,
	((LPDE2EFFECTRAINCOAT)(lpParam))->fFrameMin,
	((LPDE2EFFECTRAINCOAT)(lpParam))->fFrameMax,
	((LPDE2EFFECTRAINCOAT)(lpParam))->nCount,
	(lpModel + nScene)->object.objs);

	work->nCount = ((LPDE2EFFECTRAINCOAT)(lpParam))->nCount;

	if(((LPDE2EFFECTRAINCOAT)(lpParam))->nMultiName == 0)
	{
		work->lpWork = NewRaincoat_called(
			&((lpModel + nScene)->object),
			NULL,
			((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNamesSize,
			((LPDE2EFFECTRAINCOAT)(lpParam))->nMultiName,
			((LPDE2EFFECTRAINCOAT)(lpParam))->fWindMin,
			((LPDE2EFFECTRAINCOAT)(lpParam))->fWindMax,
			((LPDE2EFFECTRAINCOAT)(lpParam))->fFrameMin,
			((LPDE2EFFECTRAINCOAT)(lpParam))->fFrameMax);
	}
	else
	{
		work->lpWork = NewRaincoat_called(
			&((lpModel + nScene)->object),
			((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNames,
			((LPDE2EFFECTRAINCOAT)(lpParam))->nSingleNamesSize,
			((LPDE2EFFECTRAINCOAT)(lpParam))->nMultiName,
			((LPDE2EFFECTRAINCOAT)(lpParam))->fWindMin,
			((LPDE2EFFECTRAINCOAT)(lpParam))->fWindMax,
			((LPDE2EFFECTRAINCOAT)(lpParam))->fFrameMin,
			((LPDE2EFFECTRAINCOAT)(lpParam))->fFrameMax);
	}

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoRaincoat_called(void *lpParam, LPMGSDEMOMODEL lpModel)
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
