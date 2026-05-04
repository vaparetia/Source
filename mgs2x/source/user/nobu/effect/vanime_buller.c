/*
	vanime_buller.c

	2000/04/21 S.Yamashita
	$Id: vanime_buller.c,v 1.1.1.3 2002/11/19 11:46:49 Yoshizawa1 Exp $

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
extern void *NewVanimeBullerCall(
	int id,
	float rot_v,
	float speed,
	FVECTOR *from,
	FVECTOR *to,
	int base,
	int rand,
	int flag);

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
printf(
"VAnime Buller :\n"
"  nModelID     = %d\n"
"  fRotVy       = %f\n"
"  fvectFrom.vx = %f\n"
"  fvectFrom.vy = %f\n"
"  fvectFrom.vz = %f\n"
"  fvectTo.vx   = %f\n"
"  fvectTo.vy   = %f\n"
"  fvectTo.vz   = %f\n"
"  fSpeed       = %f\n"
"  nBase        = %d\n"
"  nRand        = %d\n"
"  nCount       = %d\n"
"  nFlag        = %d\n\n",
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->nModelID,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->fRotVy,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->fvectFrom.vx,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->fvectFrom.vy,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->fvectFrom.vz,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->fvectTo.vx,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->fvectTo.vy,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->fvectTo.vz,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->fSpeed,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->nBase,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->nRand,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->nCount,
	((LPDE2EFFECTVANIMEBULLER)(lpParam))->nFlag);

	work->lpWork = NewVanimeBullerCall(
		((LPDE2EFFECTVANIMEBULLER)(lpParam))->nModelID,
		((LPDE2EFFECTVANIMEBULLER)(lpParam))->fRotVy,
		((LPDE2EFFECTVANIMEBULLER)(lpParam))->fSpeed,
		&((LPDE2EFFECTVANIMEBULLER)(lpParam))->fvectFrom,
		&((LPDE2EFFECTVANIMEBULLER)(lpParam))->fvectTo,
		((LPDE2EFFECTVANIMEBULLER)(lpParam))->nBase,
		((LPDE2EFFECTVANIMEBULLER)(lpParam))->nRand,
		((LPDE2EFFECTVANIMEBULLER)(lpParam))->nFlag);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoVanimeBullerCall(void *lpParam, LPMGSDEMOMODEL lpModel)
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
