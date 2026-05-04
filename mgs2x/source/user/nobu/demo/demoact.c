/*
	demoact.c
	キャラ定義：引数解析

	2000/01/05 N.Tanaka
	$Id: demoact.c,v 1.1.1.3 2002/11/19 11:46:42 Yoshizawa1 Exp $

*/

#include <stdio.h>

#include <libdma.h>
#include <libgraph.h>
#include <libdg.h>
#include <gameheader.h>
#include <camera.h>

#include "demo.h"
#include "demo_.h"


/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX actor;
	BOOL      bAct;
	int       name;
	int       where;
	int       group;
	int       nCount;
	char*     lpstrFileName;
	int       nDemoID;		/* 読み込む dmo ファイルのＩＤ */
#ifdef DEMO_DEBUG_MODE
	int       nLoop;		/* 繰り返し再生するか */
	char*     lpstrDrawString;	/* デモ中に表示する文字列 */
#endif
} Work;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

/* シグナルを受ける関数 */
static int ReceiveSignal(void *work, int signal, int value)
{
	switch(signal)
	{
	case GV_SIGNAL_KILL:
		GV_DestroyActor(work);
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
	char* c;
	work->bAct = FALSE;

	if((c = GCL_GetOption('d')) != NULL)
		work->lpstrFileName = GCL_GetNextString();
	else
		work->lpstrFileName = NULL;

printf("Read File Name = %s.de2\n", work->lpstrFileName);

#ifdef DEMO_DEBUG_MODE
	if(GCL_GetOption('l') != NULL)
		work->nLoop = 1;

	if((c = GCL_GetOption('s')) != NULL)
		work->lpstrDrawString = GCL_GetNextString();
	else
		work->lpstrDrawString = NULL;
#endif

	/* デモスレッド起動 */
#ifdef DEMO_DEBUG_MODE
	GV_SetActorChild(work, DM_ThreadFile(0, work->lpstrFileName,
		work->nLoop, work->lpstrDrawString));
#else
	GV_SetActorChild(work, DM_ThreadFile(0, work->lpstrFileName));
#endif
	return (0);
}


/* ---------------------------------------------------------------- */
void *NewDemoActDebug(int name, int where)
{
	Work *work;

	OPERATOR();
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work));
	if(work != NULL)
	{
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);
		GV_SetActorSignalFunc(work, ReceiveSignal);
		if(GetResources(work, name, where) < 0)
		{
			GV_DestroyActor(work);
			return NULL ;
		}
	}
	return (void *)work ;
}
