/*
	d_vol_shadow.c

	2000/04/06 N.Tanaka
	$Id: d_vol_shadow.c,v 1.1.1.3 2002/11/19 11:46:45 Yoshizawa1 Exp $

	ボリューミックシャドーライン

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
extern void *NewBodyShadowVolume_Demo(DG_OBJS *objs, FVECTOR *light_pos, float reach, float back_shift, int model_id, int add_sub, int life);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX  actor;
	FVECTOR*   lpfvectObjectVector;
	FVECTOR    fvectOffset;
	FVECTOR    fvectLightPos;
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
	work->fvectLightPos = *(work->lpfvectObjectVector);
	_sceVu0AddVector(&(work->fvectLightPos), &(work->fvectLightPos), &(work->fvectOffset));
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
	int      nScene1;
	int      nScene2;
	int      nNode2;
	DG_OBJS* lpDGObject1;
	DG_OBJS* lpDGObject2;

	nScene1     = ((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)lpParam)->nScene;
	lpDGObject1 = (lpModel + nScene1)->object.objs;

	nScene2     = ((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)lpParam)->nScene2;
	nNode2      = ((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)lpParam)->nNode2;
	lpDGObject2 = (lpModel + nScene2)->object.objs;

	work->lpfvectObjectVector = (FVECTOR*)&(lpDGObject2->objs[nNode2].world.m[3]);
	work->fvectOffset   = ((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)lpParam)->fvectOffset2;
	work->fvectLightPos = *(work->lpfvectObjectVector);
	_sceVu0AddVector(&(work->fvectLightPos), &(work->fvectLightPos), &(work->fvectOffset));

	/* キャラ呼び出し */
	work->lpWork = NewBodyShadowVolume_Demo(lpDGObject1, &(work->fvectLightPos),
		((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)(lpParam))->fReach,
		((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)(lpParam))->fBackShift,
		((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)(lpParam))->nModelID,
		((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)(lpParam))->nAddSub,
		((LPDE2EFFECTBODYSHADOWVOLUMEDEMO)(lpParam))->nLife);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoBodyShadowVolume_Demo(void *lpParam, LPMGSDEMOMODEL lpModel)
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
