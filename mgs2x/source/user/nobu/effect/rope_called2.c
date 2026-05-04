/*
	rope_called2.c

	2000/04/18 S.Yamashita
	$Id: rope_called2.c,v 1.1.1.3 2002/11/19 11:46:49 Yoshizawa1 Exp $

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
extern void *NewRopeModel2_called(int model_name, int sample_num, FVECTOR *x, SVECTOR *r, int collision_flag);

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
printf(
"Rope Model 2:\n"
"  nModelName     = %d\n"
"  nSampleNum     = %d\n"
"  fvectMove.vx   = %f\n"
"  fvectMove.vy   = %f\n"
"  fvectMove.vz   = %f\n"
"  svectRot.vx    = %d\n"
"  svectRot.vy    = %d\n"
"  svectRot.vz    = %d\n"
"  nCollisionFlag = %d\n"
"  nCount         = %d\n\n",
	((LPDE2EFFECTROPEMODEL2)(lpParam))->nModelName,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->nSampleNum,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->fvectMove.vx,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->fvectMove.vy,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->fvectMove.vz,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->svectRot.vx,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->svectRot.vy,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->svectRot.vz,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->nCollisionFlag,
	((LPDE2EFFECTROPEMODEL2)(lpParam))->nCount);

	work->nCount = ((LPDE2EFFECTROPEMODEL2)(lpParam))->nCount;

	work->lpWork = NewRopeModel2_called(
		((LPDE2EFFECTROPEMODEL2)(lpParam))->nModelName,
		((LPDE2EFFECTROPEMODEL2)(lpParam))->nSampleNum,
		&((LPDE2EFFECTROPEMODEL2)(lpParam))->fvectMove,
		&((LPDE2EFFECTROPEMODEL2)(lpParam))->svectRot,
		((LPDE2EFFECTROPEMODEL2)(lpParam))->nCollisionFlag);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoRopeModel2_called(void *lpParam, LPMGSDEMOMODEL lpModel)
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
