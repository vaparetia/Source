/*
	crtrg.c

	2000/04/03 N.Tanaka
	$Id: crtrg.c,v 1.1.1.3 2002/11/19 11:46:45 Yoshizawa1 Exp $

	メッセージ駆動用のダミーキャラ
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
extern void *NewCartridgeControl(int* control_num, OBJECT* body, OBJECT* weapon, int pat); 

/******************************************************************************
 * definitions and typedefs and structures
 */
typedef struct {
	GV_ACT_EX  actor;
	int        nName;
	int        nControlNumber;
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
static void Act(Work *work)
{
	int     nMessageCount;
	GV_MSG* lpGVMessage;

	if((nMessageCount = GV_ReceiveMessage(work->nName, &lpGVMessage)) != 0)
	{
		/* メッセージチェック */
		for (; nMessageCount > 0; nMessageCount--, lpGVMessage++)
			work->nControlNumber = lpGVMessage->message[0];
	}

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
		/* 子に告知 */
//		GV_CallChildSignalFunc(work, GV_SIGNAL_KILL, 0);
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

printf(
"Cartridge Control :\n"
"  nName       = %d\n"
"  nScene1     = %d\n"
"  nScene2     = %d\n"
"  nPat        = %d\n"
"  nControlNum = %d\n"
"  nCount      = %d\n\n",
	((LPDE2EFFECTCARTRIDGECONTROL)(lpParam))->nName,
	((LPDE2EFFECTCARTRIDGECONTROL)(lpParam))->nScene1,
	((LPDE2EFFECTCARTRIDGECONTROL)(lpParam))->nScene2,
	((LPDE2EFFECTCARTRIDGECONTROL)(lpParam))->nPat,
	((LPDE2EFFECTCARTRIDGECONTROL)(lpParam))->nControlNum,
	((LPDE2EFFECTCARTRIDGECONTROL)(lpParam))->nCount);

	/* 人体オブジェクト */
	nScene1     = ((LPDE2EFFECTCARTRIDGECONTROL)lpParam)->nScene1;
	/* 武器 */
	nScene2     = ((LPDE2EFFECTCARTRIDGECONTROL)lpParam)->nScene2;

	work->nName          = ((LPDE2EFFECTCARTRIDGECONTROL)lpParam)->nName;
	work->nCount         = ((LPDE2EFFECTCARTRIDGECONTROL)lpParam)->nCount;
	work->nControlNumber = ((LPDE2EFFECTCARTRIDGECONTROL)lpParam)->nControlNum;

	work->lpWork = NewCartridgeControl(
		&(work->nControlNumber),
		&((lpModel + nScene1)->object),
		&((lpModel + nScene2)->object),
		((LPDE2EFFECTCARTRIDGECONTROL)lpParam)->nPat);

	return (0);
}

/******************************************************************************
 * 登録部分
 */
void *NewDemoCartridgeControl(void *lpParam, LPMGSDEMOMODEL lpModel)
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
