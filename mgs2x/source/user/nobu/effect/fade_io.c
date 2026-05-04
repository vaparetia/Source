/*
	fade_io.c

	2000/01/05 N.Tanaka
	$Id: fade_io.c,v 1.1.1.3 2002/11/19 11:46:47 Yoshizawa1 Exp $

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
extern void *NewFadeInOut(int add_sub, int color_r, int color_g, int color_b, int fade_in_out, int variation, int count, int stable, FVECTOR *pos);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX actor;
	int       nVariation;
	int       nStable;
	int       nCount;
	FVECTOR*  lpfvectObjectVector;
	FVECTOR   fvectObjectPos;
	FVECTOR   fvectOffset;
	void*     lpWork;
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
	if(work->nVariation == 3)
	{
		work->fvectObjectPos = *(work->lpfvectObjectVector);
		_sceVu0AddVector(&(work->fvectObjectPos), &(work->fvectObjectPos), &(work->fvectOffset));
	}

	/* -1 のときは死ぬのを待つ */
	if(work->nStable == -1 )
		return;

	/* -1 以外はカウンタで判断。０になったら終了 */
	if(--(work->nCount) == 0)
	{
		GV_DestroyActor(work->lpWork);
		work->lpWork = NULL;
		/* 子に告知 */
/*		GV_CallChildSignalFunc(work, GV_SIGNAL_KILL, 0);*/
		return;
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

	work = (Work*)(workp);

	switch(signal)
	{
	case GV_SIGNAL_KILL:
		/* 子に告知 */
/*		GV_CallChildSignalFunc(work, GV_SIGNAL_KILL, 0);*/
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
static int GetResources( Work *work, void* lpParam, LPMGSDEMOMODEL lpModel)
{
	int      nScene;
	int      nNode;
	DG_OBJS* lpDGObject;

	int nAddSub;
	int nColorR;
	int nColorG;
	int nColorB;
	int nFadeInOut;
	int nCount;

	nAddSub           = ((LPDE2EFFECTFADEINOUT)lpParam)->nAddSub;
	nColorR           = ((LPDE2EFFECTFADEINOUT)lpParam)->nColorR;
	nColorG           = ((LPDE2EFFECTFADEINOUT)lpParam)->nColorG;
	nColorB           = ((LPDE2EFFECTFADEINOUT)lpParam)->nColorB;
	nFadeInOut        = ((LPDE2EFFECTFADEINOUT)lpParam)->nFadeInOut;
	nCount            = ((LPDE2EFFECTFADEINOUT)lpParam)->nCount;
	work->nVariation  = ((LPDE2EFFECTFADEINOUT)lpParam)->nVariation;
	work->nStable     = ((LPDE2EFFECTFADEINOUT)lpParam)->nStable;
	work->nCount      = ((LPDE2EFFECTFADEINOUT)lpParam)->nKillCount;

	if(work->nVariation != 3)
	{
		/* キャラ起動 */
		work->lpWork = NewFadeInOut(
			nAddSub, nColorR, nColorG, nColorB, nFadeInOut, work->nVariation, nCount,
			work->nStable, NULL);
	}
	else
	{
		nScene     = ((LPDE2EFFECTFADEINOUT)lpParam)->nScene;
		nNode      = ((LPDE2EFFECTFADEINOUT)lpParam)->nNode;
		lpDGObject = (lpModel + nScene)->object.objs;

		work->lpfvectObjectVector = (FVECTOR*)&(lpDGObject->objs[nNode].world.m[3]);
		work->fvectObjectPos      = *(work->lpfvectObjectVector);
		work->fvectOffset         = ((LPDE2EFFECTFADEINOUT)lpParam)->fvectOffset;

		_sceVu0AddVector(&(work->fvectObjectPos), &(work->fvectObjectPos), &(work->fvectOffset));

		/* キャラ起動 */
		work->lpWork = NewFadeInOut(
			nAddSub, nColorR, nColorG, nColorB, nFadeInOut, work->nVariation, nCount,
			work->nStable, &(work->fvectObjectPos));
	}

//	GV_SetActorChild(work, work->lpWork);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoFadeInOut(void *lpParam, LPMGSDEMOMODEL lpModel)
{
	Work* work;

	OPERATOR() ;

	work = (Work *)GV_NewActor(GV_ACTOR_USER, sizeof(Work));
	if(work != NULL)
	{
		GV_SetActor(&(work->actor), Act, Die);
		GV_ActorEX(&(work->actor))
		GV_SetActorSignalFunc(work, ReceiveSignal);
		if(GetResources(work, lpParam, lpModel) < 0 )
		{
			GV_DestroyActor(work);
			return NULL;
		}
	}
	return (void *)work;
}
