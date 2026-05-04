//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_bullet.c

	2000/04/21 S.Yamashita
	$Id: demo_bullet.c,v 1.1.1.3 2002/11/19 11:46:46 Yoshizawa1 Exp $

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
extern void *NewDemoBulletCall(
	int id,
	float rot_vy,
	float speed,
	FVECTOR *from,
	FVECTOR *t,
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
"Demo Bullet :\n"
"  nModelID     = %d\n"
"  fRotVy       = %f\n"
"  fvectFrom.vx = %f\n"
"  fvectFrom.vy = %f\n"
"  fvectFrom.vz = %f\n"
"  fvectTo.vx   = %f\n"
"  fvectTo.vy   = %f\n"
"  fvectTo.vz   = %f\n"
"  fSpeed       = %f\n"
"  nCount       = %d\n"
"  nFlag        = %d\n\n",
	((LPDE2EFFECTDEMOBULLET)(lpParam))->nModelID,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->fRotVy,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->fvectFrom.vx,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->fvectFrom.vy,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->fvectFrom.vz,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->fvectTo.vx,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->fvectTo.vy,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->fvectTo.vz,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->fSpeed,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->nCount,
	((LPDE2EFFECTDEMOBULLET)(lpParam))->nFlag);

	work->nCount = ((LPDE2EFFECTDEMOBULLET)(lpParam))->nCount;

	work->lpWork = NewDemoBulletCall(
		((LPDE2EFFECTDEMOBULLET)(lpParam))->nModelID,
		((LPDE2EFFECTDEMOBULLET)(lpParam))->fRotVy,
		((LPDE2EFFECTDEMOBULLET)(lpParam))->fSpeed,
		&((LPDE2EFFECTDEMOBULLET)(lpParam))->fvectFrom,
		&((LPDE2EFFECTDEMOBULLET)(lpParam))->fvectTo,
		((LPDE2EFFECTDEMOBULLET)(lpParam))->nFlag);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoDemoBulletCall(void *lpParam, LPMGSDEMOMODEL lpModel)
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
