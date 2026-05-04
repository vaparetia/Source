/*
    prezroute.c
    大統領ニキータイベント用ルート
    2001/04/25 Masafumi Okuta
    $Id: prezroute.c,v 1.1.1.3 2002/11/19 11:48:10 Yoshizawa1 Exp $
*/

// 次の位置をルートから取得
static void PREZ_SetTrgpNextZone(Work* work)
{
    NPCWORK* npc;
    ROUTENAVI* rnavi;

    npc = &work->npc;
    rnavi = &work->rnavi;

    work->vecNextPos = rnavi->nodes[(short)rnavi->next_node];
    work->nNextZone  = HZX_GetAddress( npc->ctrl->hzx_id, &work->vecNextPos, -1);
    ENE_SetNextnode( rnavi );	// 次の地点を設定

    PREZ_SetThink2( work, TH2_ATARAXIA, TH3_ROUTE_MOVE);
}
// ルート待機設定
static void PREZ_SetRouteWait( Work* work )
{
    NPCWORK* npc;
    ROUTENAVI* rnavi;

    npc = &work->npc;
    rnavi = &work->rnavi;

    rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
    work->nRouteWaitTime = rnavi->p_acttime;

    if ( work->nRouteWaitTime == 0){
	PREZ_SetTrgpNextZone( work );
    }else{
	PREZ_SetThink3( work, TH3_ROUTE_STAND);
    }
}

