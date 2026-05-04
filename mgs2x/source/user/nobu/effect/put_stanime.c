//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	put_stanime.c

	2000/04/06 N.Tanaka
	$Id: put_stanime.c,v 1.1.1.3 2002/11/19 11:46:48 Yoshizawa1 Exp $

	頂点ストリームアニメーション
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
extern void *NewPutSTanimeObjectCall(int name, int model, DG_OBJ *obj, FVECTOR *pos, SVECTOR *rot, FVECTOR *scale, FVECTOR *bound_u, FVECTOR *bound_l, int mode);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX  actor;
	DG_OBJ*    lpobj;
	int        nName;
	int        nCount;
	int        nState;
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
static void Act(Work *work)
{
	int    nParam;
	GV_MSG gvMessage;

	if((work->nCount-- <= 0) && (work->nState != 0))
	{
		nParam = 2;
		memset(&gvMessage, 0x00, sizeof(GV_MSG));
		gvMessage.address     = work->nName;
		gvMessage.message_len = 1;
		gvMessage.message     = &nParam;
		GV_SendMessage(&gvMessage);
		work->nState = 0;
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
	int    nParam;
	Work*  work;
	GV_MSG gvMessage;

	work = workp;

	switch(signal)
	{
	case GV_SIGNAL_KILL:
		nParam = 2;
		memset(&gvMessage, 0x00, sizeof(GV_MSG));
		gvMessage.address     = work->nName;
		gvMessage.message_len = 1;
		gvMessage.message     = &nParam;
		GV_SendMessage(&gvMessage);

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

	nScene           = ((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->nScene;
	nNode            = ((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->nNode;
	if(nScene == -1)
	{
		work->lpobj = NULL;
	}
	else
	{
		lpDGObject  = (lpModel + nScene)->object.objs;
		work->lpobj = &(lpDGObject->objs[nNode]);
	}

	work->nName  = ((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->nName;
	work->nCount = ((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->nCount;
	work->nState = 1;

	work->lpWork = NewPutSTanimeObjectCall(
		((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->nName,
		((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->nModel,
		work->lpobj,
		&(((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->fvectPos),
		&(((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->svectRot),
		&(((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->fvectScale),
		&(((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->fvectBound_U),
		&(((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->fvectBound_L),
		((LPDE2EFFECTPUTSTANIMEOBJECTCALL)lpParam)->nMode);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoPutSTanimeObjectCall(void *lpParam, LPMGSDEMOMODEL lpModel)
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
