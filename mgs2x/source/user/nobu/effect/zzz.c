/*
	zzz.c

	2000/02/11 N.Tanaka
	$Id: zzz.c,v 1.1.1.3 2002/11/19 11:46:50 Yoshizawa1 Exp $

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
extern void *New_Zzz_Demo(FMATRIX *world);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX actor;
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
static void Act(Work *work)
{
}

/******************************************************************************
 * 終了関数
 */
static void Die(Work *work)
{
}

/******************************************************************************
 * シグナルを受ける関数
 */
static int ReceiveSignal(void *workp, int signal, int value)
{
	Work* work;

	work = (Work*)workp;

	switch(signal)
	{
	case GV_SIGNAL_KILL:
		/* 子に告知 */
//		GV_CallChildSignalFunc(work, GV_SIGNAL_KILL, 0);

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

	nScene       = ((LPDE2EFFECTZZZ)lpParam)->nScene;
	nNode        = ((LPDE2EFFECTZZZ)lpParam)->nNode;
	lpDGObject   = (lpModel + nScene)->object.objs;

	/* キャラ呼び出し */
	work->lpWork = New_Zzz_Demo(&(lpDGObject->objs[nNode].world));
	GV_SetActorChild(work, work->lpWork);

	return (1);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemo_Zzz_Demo(void *lpParam, LPMGSDEMOMODEL lpModel)
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
