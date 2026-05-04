/*
	d_inter_poly.c

	2000/04/21 S.Yamashita
	$Id: d_inter_poly.c,v 1.1.1.3 2002/11/19 11:46:45 Yoshizawa1 Exp $

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
extern void *NewInterPoly_Demo(
	FVECTOR *pos0,
	FVECTOR *pos1,
	int disp_f,
	SVECTOR *col);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX  actor;
	int        nCount;
	FVECTOR*  lpfvectObjectVector1;
	FVECTOR*  lpfvectObjectVector2;
	FVECTOR   fvectPos1;
	FVECTOR   fvectPos2;
	FVECTOR   fvectOffset1;
	FVECTOR   fvectOffset2;
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
	work->fvectPos1    = *(work->lpfvectObjectVector1);
	work->fvectPos2    = *(work->lpfvectObjectVector2);
	_sceVu0AddVector(&(work->fvectPos1), &(work->fvectPos1), &(work->fvectOffset1));
	_sceVu0AddVector(&(work->fvectPos2), &(work->fvectPos2), &(work->fvectOffset2));

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
	int      nScene1;
	int      nScene2;
	int      nNode1;
	int      nNode2;
	DG_OBJS*   lpDGObject1;
	DG_OBJS*   lpDGObject2;

printf(
"Interpoly Demo :\n"
"  svectCol.vx     = %d\n"
"  svectCol.vy     = %d\n"
"  svectCol.vz     = %d\n"
"  fvectOffset1.vx = %f\n"
"  fvectOffset1.vy = %f\n"
"  fvectOffset1.vz = %f\n"
"  fvectOffset2.vx = %f\n"
"  fvectOffset2.vy = %f\n"
"  fvectOffset2.vz = %f\n"
"  nScene1         = %d\n"
"  nNode1          = %d\n"
"  nScene2         = %d\n"
"  nNode2          = %d\n"
"  nDispF          = %d\n"
"  nCount          = %d\n\n",
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->svectCol.vx,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->svectCol.vy,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->svectCol.vz,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->fvectOffset1.vx,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->fvectOffset1.vy,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->fvectOffset1.vz,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->fvectOffset2.vx,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->fvectOffset2.vy,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->fvectOffset2.vz,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->nScene1,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->nNode1,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->nScene2,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->nNode2,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->nDispF,
	((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->nCount);

	nScene1     = ((LPDE2EFFECTINTERPOLYDEMO)lpParam)->nScene1;
	nNode1      = ((LPDE2EFFECTINTERPOLYDEMO)lpParam)->nNode1;
	lpDGObject1 = (lpModel + nScene1)->object.objs;

	nScene2     = ((LPDE2EFFECTINTERPOLYDEMO)lpParam)->nScene2;
	nNode2      = ((LPDE2EFFECTINTERPOLYDEMO)lpParam)->nNode2;
	lpDGObject2 = (lpModel + nScene2)->object.objs;

	work->lpfvectObjectVector1 = (FVECTOR*)&(lpDGObject1->objs[nNode1].world.m[3]);
	work->lpfvectObjectVector2 = (FVECTOR*)&(lpDGObject2->objs[nNode2].world.m[3]);
	work->fvectOffset1 = ((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->fvectOffset1;
	work->fvectOffset2 = ((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->fvectOffset2;
	work->fvectPos1    = *(work->lpfvectObjectVector1);
	work->fvectPos2    = *(work->lpfvectObjectVector2);
	_sceVu0AddVector(&(work->fvectPos1), &(work->fvectPos1), &(work->fvectOffset1));
	_sceVu0AddVector(&(work->fvectPos2), &(work->fvectPos2), &(work->fvectOffset2));

	work->nCount = ((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->nCount;

	work->lpWork = NewInterPoly_Demo(
		&(work->fvectPos1),
		&(work->fvectPos2),
		((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->nDispF,
		&((LPDE2EFFECTINTERPOLYDEMO)(lpParam))->svectCol);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoInterPoly_Demo(void *lpParam, LPMGSDEMOMODEL lpModel)
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
