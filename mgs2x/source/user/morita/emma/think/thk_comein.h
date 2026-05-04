/*
  thk_afraid.h
  舟虫用 思考処理

  2001/06/14 T.Morita Revised
  $Id: thk_comein.h,v 1.1.1.3 2002/11/19 11:46:05 Yoshizawa1 Exp $
*/



/*

  そっちいかれたら困る場合の時に使う

 */
enum { MAX, MIN, AREABOUND, AREAMAX=3 } ;
static FVECTOR EMA_OutArea[][AREABOUND] =
{
    /*EMA_STG_W28A*/
    { { 55000, 3000,-223500 },/*max*/  { 47530, -200,-230745 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },

    /*EMA_STG_W25D*/
    { { 57500, 4000,-212285 },/*max*/  { 44600, -200,-223500 }/*min*/ },
    { { 19100, 4000,-232000 },/*max*/  { 11600,-2000,-240000 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },

    /*EMA_STG_W31B*/
    { { -7250,-4000,-232400 },/*max*/  { -9750,-7000,-237000 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },

    /*EMA_STG_W31D*/
    { { 24000, 4000,-233400 },/*max*/  { 19000, -200,-242252 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },

    /*EMA_STG_W31F*/
    { { -7250,-4000,-230000 },/*max*/  { -9750,-7000,-232400 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },


    /*EMA_STG_A16A*/
    { {-40000, 8000, -70000 },/*max*/  { -54900,-7000, -96900 }/*min*/ },
    { {-40000, 8000, -94000 },/*max*/  { -45000,-7000,-103000 }/*min*/ },
    { {-54500, 8000, -70000 },/*max*/  { -67000,-7000, -85000 }/*min*/ },

    /*EMA_STG_A20E*/
    { { 44710, 4000, -98820 },/*max*/  { 40000,-7000,-102000 }/*min*/ },
    { { 67000, 8000, -87000 },/*max*/  { 62500, 3800, -92000 }/*min*/ },
    { { 62000, 4000, -70000 },/*max*/  { 50000,-7000, -76600 }/*min*/ },

    /*EMA_STG_A21A*/
    { { 61000, 4000, -30000 },/*max*/  { 47000,-7000, -43750 }/*min*/ },
    { { 62000, 4000, -76300 },/*max*/  { 47000,-7000, -90000 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },

    /*EMA_STG_A22B*/
    { { 60000, 4000, -43500 },/*max*/  { 47000,-7000, -50000 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },
    { {     0,    0,      0 },/*max*/  {     0,    0,      0 }/*min*/ },

} ;

int EMA_ThinkCheckOutSide( Work *work, FVECTOR *pos )
{
    int i ;
    FVECTOR *area ;

    if ( work->stage >= 0 )
	for ( i=AREAMAX ; --i>=0 ; )
	{
	    area = EMA_OutArea[work->stage*AREAMAX + i] ;

	    ASSERT( area[MIN].vx <= area[MAX].vx ) ;
	    ASSERT( area[MIN].vy <= area[MAX].vy ) ;
	    ASSERT( area[MIN].vz <= area[MAX].vz ) ;

	    if ( pos->vx > area[MIN].vx && pos->vx < area[MAX].vx &&
		 pos->vy > area[MIN].vy && pos->vy < area[MAX].vy &&
		 pos->vz > area[MIN].vz && pos->vz < area[MAX].vz  )
	    {
		return 1 ;
	    }
	}
    return 0 ;
}

static inline int EMA_ThinkCheckOutSideEmma( Work *work )
{
    if ( work->pl_dis < 800.0f )
	return  0 ;
    return EMA_ThinkCheckOutSide( work, &work->control.mov ) ;
}

static int EMA_ThinkComeInside( Work *work )
{
    if ( EMA_ThinkCheckOutSideEmma( work ) )
	if ( EMA_SetNaviTarget( work,
				GM_PlayerControl->addr,
				&GM_PlayerControl->mov ) )
	{
	    EMA_SetFlag( EMA_F_IS_COMMING ) ;
	    return 1 ;
	}
    return 0 ;
}


static void EMA_ThinkComeInMove( Work *work, int reach_pad, int move_pad )
{
    if ( GM_Navi( &work->navigate, &work->navitrg, 500 ) )
    {
	 /* nvtrgに向ってゾーン移動 */
	work->npc.action.pad = reach_pad ;
	EMA_SetAdjustPosition( work, &work->navitrg.pos, 20.0f ) ;
    }
    else
    {
	/* 移動方向指定 */
	work->npc.action.dir = work->navigate.next_dir ;
	work->npc.action.pad = move_pad ;
    }
}
