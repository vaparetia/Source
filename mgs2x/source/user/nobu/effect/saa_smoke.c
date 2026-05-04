/*
	saa_smoke.c

	2000/04/21 S.Yamashita
	$Id: saa_smoke.c,v 1.1.1.3 2002/11/19 11:46:49 Yoshizawa1 Exp $

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
extern void *NewSAA_Smoke(
	FMATRIX *world,
	int life);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX actor;
	FMATRIX*  lpfmatWorld;
	FMATRIX   fmatWorld;
	FVECTOR   fvectMove;
	FVECTOR   fvectRot;
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
	_sceVu0RotMatrix  (&(work->fmatWorld), work->lpfmatWorld , &(work->fvectRot ));
	_sceVu0TransMatrix(&(work->fmatWorld), &(work->fmatWorld), &(work->fvectMove));
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

printf(
"SAA Smoke :\n"
"  nScene       = %d\n"
"  nNode        = %d\n"
"  fvectMove.vx = %f\n"
"  fvectMove.vy = %f\n"
"  fvectMove.vz = %f\n"
"  fvectRot.vx  = %f\n"
"  fvectRot.vy  = %f\n"
"  fvectRot.vz  = %f\n"
"  nLife       = %x\n\n",
	((LPDE2EFFECTSAASMOKE)(lpParam))->nScene,
	((LPDE2EFFECTSAASMOKE)(lpParam))->nNode,
	((LPDE2EFFECTSAASMOKE)(lpParam))->fvectMove.vx,
	((LPDE2EFFECTSAASMOKE)(lpParam))->fvectMove.vy,
	((LPDE2EFFECTSAASMOKE)(lpParam))->fvectMove.vz,
	((LPDE2EFFECTSAASMOKE)(lpParam))->fvectRot.vx,
	((LPDE2EFFECTSAASMOKE)(lpParam))->fvectRot.vy,
	((LPDE2EFFECTSAASMOKE)(lpParam))->fvectRot.vz,
	((LPDE2EFFECTSAASMOKE)(lpParam))->nLife);

	nScene     = ((LPDE2EFFECTSAASMOKE)lpParam)->nScene;
	nNode      = ((LPDE2EFFECTSAASMOKE)lpParam)->nNode;
	lpDGObject = (lpModel + nScene)->object.objs;

	work->lpfmatWorld = &(lpDGObject->objs[nNode].world);
	work->fvectMove   = ((LPDE2EFFECTSAASMOKE)lpParam)->fvectMove;
	work->fvectRot    = ((LPDE2EFFECTSAASMOKE)lpParam)->fvectRot;

	_sceVu0RotMatrix  (&(work->fmatWorld), work->lpfmatWorld , &(work->fvectRot ));
	_sceVu0TransMatrix(&(work->fmatWorld), &(work->fmatWorld), &(work->fvectMove));

	work->lpWork = NewSAA_Smoke(
		&(work->fmatWorld),
		((LPDE2EFFECTSAASMOKE)(lpParam))->nLife);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoSAA_Smoke(void *lpParam, LPMGSDEMOMODEL lpModel)
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
