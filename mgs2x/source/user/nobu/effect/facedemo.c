/*
	facedemo.c

	2000/04/25 S.Yamashita
	$Id: facedemo.c,v 1.1.1.3 2002/11/19 11:46:46 Yoshizawa1 Exp $

	ゴルルコ兵目パチ

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
extern void *NewGbsFaceDemo(OBJECT *body, int name);

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX actor;
	int       nName;
	int       nCount;
	int       nState;
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
	int    nParam[3];
	GV_MSG gvMessage;

	if((work->nCount-- <= 0) && (work->nState != 0))
	{
		nParam[0] = 1;
		nParam[1] = -1;
		nParam[2] = 1;
		memset(&gvMessage, 0x00, sizeof(GV_MSG));
		gvMessage.address     = work->nName;
		gvMessage.message_len = 3;
		gvMessage.message     = nParam;
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
	int    nParam[3];
	Work*  work;
	GV_MSG gvMessage;

	work = workp;

	switch(signal)
	{
	case GV_SIGNAL_KILL:
		if(work->nState != 0)
		{
			nParam[0] = 1;
			nParam[1] = -1;
			nParam[2] = 1;
			memset(&gvMessage, 0x00, sizeof(GV_MSG));
			gvMessage.address     = work->nName;
			gvMessage.message_len = 3;
			gvMessage.message     = nParam;
			GV_SendMessage(&gvMessage);
		}

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
	work->nName  = ((LPDE2EFFECTGBSFACEDEMO)lpParam)->nName;
	work->nCount = ((LPDE2EFFECTGBSFACEDEMO)lpParam)->nCount;
	work->nState = 1;

	work->lpWork = NewGbsFaceDemo(
		&(lpModel + ((LPDE2EFFECTGBSFACEDEMO)lpParam)->nScene)->object,
		((LPDE2EFFECTGBSFACEDEMO)lpParam)->nName);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoGbsFaceDemo(void *lpParam, LPMGSDEMOMODEL lpModel)
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
