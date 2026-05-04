//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_status.c
		NPCスネーク関係の中で、無線から呼ばれているものを
		独立させる。

	2001/08/22 K.Kano
	$Id: npc_command.c,v 1.1.1.3 2002/11/19 11:43:21 Yoshizawa1 Exp $
 */


#include "npc_snake.h"


extern Work *npcsnake_work ;

int GetNPCSnakeLife(void)
{
	if(npcsnake_work==NULL) return 0;
	return npcsnake_work->npc.action.life;
}

int ComNPCSnakeSetStage(void)
{
	if(npcsnake_work==NULL) return 0;

	if(GCL_NextStr()!=NULL){
		int arg;
		arg=GCL_GetNextInt();
		NPCSnake_SetStage(npcsnake_work,arg);
	}
	else return 0;

	return 1;
}
