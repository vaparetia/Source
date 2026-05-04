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
	$Id: snake_status.c,v 1.1.1.3 2002/11/19 11:43:23 Yoshizawa1 Exp $
 */


#include "npc_snake.h"


Work *npcsnake_work=NULL;


#ifdef DEBUG_MODE
int _GetNPCSnakeStatus(void)
#else
int GetNPCSnakeStatus(void)
#endif
{
	if(npcsnake_work==NULL) return 0;

	if(npcsnake_work->scenario_snake_status==NPCSNAKE_SCNSTATUS_DAMAGE){
		if(npcsnake_work->snake_status2 & SNAKE_STATUS2_DAMAGED_BY_RAIDEN){
			return NPCSNAKE_SCNSTATUS_DAMAGE_BY_RAIDEN;
		}
		else{
			return NPCSNAKE_SCNSTATUS_DAMAGE;
		}
	}
	else return npcsnake_work->scenario_snake_status;
}

#ifdef DEBUG_MODE

int GetNPCSnakeStatus(void)
{
	int ans=_GetNPCSnakeStatus();
	// printf("GetNPCSnakeStatus = %d\n",ans);
	return ans;
}

#endif
