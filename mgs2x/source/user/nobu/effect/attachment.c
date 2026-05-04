//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachment.c

	2000/04/19 S.Yamashita
	$Id: attachment.c,v 1.1.1.3 2002/11/19 11:46:44 Yoshizawa1 Exp $

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
extern void *NewAttachment3_called(
	int model_num,
	SVECTOR *r,
	OBJECT *target,
	int objnum,
	FVECTOR *x,
	int frames);

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

	nScene     = ((LPDE2EFFECTATTACHMENT3)lpParam)->nScene;

	work->nCount = ((LPDE2EFFECTATTACHMENT3)(lpParam))->nCount;

	work->lpWork = NewAttachment3_called(
		((LPDE2EFFECTATTACHMENT3)(lpParam))->nModelNum,
		&((LPDE2EFFECTATTACHMENT3)(lpParam))->svectRot,
		&(lpModel + nScene)->object,
		((LPDE2EFFECTATTACHMENT3)(lpParam))->nNode,
		&((LPDE2EFFECTATTACHMENT3)(lpParam))->fvectMove,
		((LPDE2EFFECTATTACHMENT3)(lpParam))->nFrames);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoAttachment3_called(void *lpParam, LPMGSDEMOMODEL lpModel)
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
