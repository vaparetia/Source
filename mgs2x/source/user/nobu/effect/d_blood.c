/*
	d_blood.c

	2000/03/29 N.Tanaka
	$Id: d_blood.c,v 1.1.1.3 2002/11/19 11:46:45 Yoshizawa1 Exp $

	任意血

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
extern void *NewBlood_Demo(FMATRIX *world, FVECTOR *force, int size, int time, int pat);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX  actor;
	FMATRIX    fmatWorld;
	FMATRIX*   lpfmatObjectWorld;
	FVECTOR    fvectOffset;
	FVECTOR    fvectForce;
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
	_sceVu0TransMatrix(&(work->fmatWorld), work->lpfmatObjectWorld, &(work->fvectOffset));
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

	nScene     = ((LPDE2EFFECTBLOODDEMO)lpParam)->nScene;
	nNode      = ((LPDE2EFFECTBLOODDEMO)lpParam)->nNode;
	lpDGObject = (lpModel + nScene)->object.objs;

	work->fvectForce        = ((LPDE2EFFECTBLOODDEMO)lpParam)->fvectForce;
	work->fvectOffset       = ((LPDE2EFFECTBLOODDEMO)lpParam)->fvectOffset;
	work->lpfmatObjectWorld = &(lpDGObject->objs[nNode].world);
	_sceVu0TransMatrix(&(work->fmatWorld), work->lpfmatObjectWorld, &(work->fvectOffset));

	/* キャラ呼び出し */
	work->lpWork = NewBlood_Demo(
		&(work->fmatWorld),
		&(work->fvectForce),
		((LPDE2EFFECTBLOODDEMO)(lpParam))->nSize,
		((LPDE2EFFECTBLOODDEMO)(lpParam))->nCount,
		((LPDE2EFFECTBLOODDEMO)(lpParam))->nPat);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoBlood_Demo(void *lpParam, LPMGSDEMOMODEL lpModel)
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
