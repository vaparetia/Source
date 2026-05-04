/*
	body_sph.c

	2000/04/19 S.Yamashita
	$Id: body_sph.c,v 1.1.1.3 2002/11/19 11:46:44 Yoshizawa1 Exp $

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
extern void *NewBodySplash(DG_OBJS *objs, CONTROL *control, int model_id);

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
	DG_OBJS* lpDGObject;

	nScene     = ((LPDE2EFFECTBODYSPLASH)lpParam)->nScene;
	lpDGObject = (lpModel + nScene)->object.objs;

	work->nCount = ((LPDE2EFFECTBODYSPLASH)(lpParam))->nCount;

	work->lpWork = NewBodySplash(
		lpDGObject,
		&((lpModel + nScene)->control),
		((LPDE2EFFECTBODYSPLASH)(lpParam))->nModelID);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoBodySplash(void *lpParam, LPMGSDEMOMODEL lpModel)
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
