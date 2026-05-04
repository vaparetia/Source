/*
	check_at.c
	attackerの思考補助関数

	1999/12/17 K.Sigeno
	$Id: check_at.c,v 1.1.1.3 2002/11/19 11:49:02 Yoshizawa1 Exp $
*/
/*攻撃不能状態*/
#define AT_NO_SHOOT (ACT_STATUS_DOWN|ACT_STATUS_GHOST|ACT_STATUS_DEATH)
/*プレイヤの無敵中には攻撃しない*/
//#define PL_NO_SHOOT (PLAYER_DAMAGED|PLAYER_DOWNED|PLAYER_FORCE)
//#define PL_NO_SHOOT (PLAYER_DAMAGED|PLAYER_DOWNED|PLAYER_INVINCIBLE)
#define PL_NO_SHOOT (PLAYER_INVINCIBLE)

extern int	AT_AlertCheckPad( ACTION * ) ;
extern int	AT_AlertPadShl( ACTION * ) ;
extern int Sig_GetRoute(HZX_GROUP_ID ,int ,int ) ;
extern void SIG_SetRandFvec(FVECTOR *,int ) ;
extern void SIG_SetZonePos2Fvec(HZX_ZON * ,FVECTOR * ) ;

extern	int	SIG_CheckDirSub(int ,int);
static	int	CheckSafeZone( ENETHINK * ) ;

static	int	CheckX_Fire(ENETHINK * ) ;
extern	int	Sig_InsideZone( HZX_ZON *,FVECTOR *,float) ;

extern ENETHINK *GetAtRanking(ENETHINK *,int );



#if 0
static void Layer_Off(ACTION *act,int layer){
	act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
}
#endif
static int CheckPl_NoShot(void){
	/*無敵かつ強制モーション中は攻撃しない*/
	if(
	( GM_PlayerStatus & PLAYER_INVINCIBLE) 
	&&( GM_PlayerStatus & PLAYER_FORCE) 
	)
	{
		return 1;
	}
	return 0;

}
static int	FvecBoundingCheck(FVECTOR *pos0 ,FVECTOR *pos1 ,FVECTOR *trg){
	if(
	(pos0->vx <= trg->vx )&&(trg->vx <= pos1->vx )
	&&(pos0->vy <= trg->vy )&&(trg->vy <= pos1->vy )
	&&(pos0->vz <= trg->vz )&&(trg->vz <= pos1->vz )
	){
		return 1;
	}
	return 0;
}
static int MapAddrSet(int addr,int map){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(map)<<16));
	return addr;
}
/*カバーエリアに到達している？*/
static void CheckInCover(ENETHINK *entk){
	FVECTOR *pos0,*pos1;
	AT_THK		*at_thk ;
	at_thk = (AT_THK *) entk->character ;
	
	if(entk->at_com->watch_status & AT_COM_WATCH_COVER ){
		pos0 = &entk->at_com->at_cov[entk->at_com->now_cov].cov_box[0];
		pos1 = &entk->at_com->at_cov[entk->at_com->now_cov].cov_box[1];
		if(FvecBoundingCheck(pos0,pos1,&entk->ctrl->mov)){
			at_thk->at_status |= AT_ST_IN_COVER ;
		}else {
			at_thk->at_status &= ~AT_ST_IN_COVER ;
		}
	}else {
		at_thk->at_status &= ~AT_ST_IN_COVER ;
	}
}
/*
二つのゾーン間が威嚇に適さない
体験版用 暫定処置 将来はintrptに移行
*/
static int CheckThreatCondition( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr){
//	int next;
//	HZX_GROUP_ID id  ;
	HZX_ZONE_ADD addr1 ;
//	HZX_ZON		*zone ;
	

	if(from_addr == to_addr ) {
		return 0 ;
	}
	/*別マップ*/
#if 1
	if( HZX_ZoneMapNo( from_addr ) != HZX_ZoneMapNo( to_addr ) ){
		/*問答無用で侵攻*/
		return 1 ;
	} 
#endif
	addr1 = HZX_ZoneStatusZ2Z( from_addr,to_addr, HZX_ZONE_SLIDEDOOR);
	if(addr1 == -1) return 0 ;

#if 1
	return 1 ;
#else
	id = GM_GetBit( HZX_ZoneMapNo( from_addr ) ) ;
	next = HZX_NextZone(id,HZX_Zone1(addr1),HZX_Zone1(to_addr)) ;
	if(next == HZX_Zone1(addr1) ) return 0 ;

	zone = HZX_GetZone( id, HZX_Zone1(next) ) ;

	if(zone != NULL){
		if(zone->flag & HZX_ZONE_SLIDEDOOR){
			return 1 ;
		}
	}
#endif
	return 0 ;
}
/***************
int		GV_DiffDirAbs( from, to )
int		from, to ;
{
    int		diff ;

    diff = 4095 & ( to - from ) ;
    return ( diff <= 2048 ) ? diff : 4096 - diff ;
}
***************/

#ifdef DEBUG_MODE
//#define DEBUG_NO_SHOT
#endif

static int CheckRollTime(ENETHINK *entk){

	if(SIG_CheckStealthStatus(entk)){
		return 0 ;
	}
	if(entk->act->bodyp.type & ENE_TYPE_HITECH){
/*ハイテク兵転がり禁止*/
		return 0 ;
	}
	if(entk->at_com->rollout > 0){
		return 0 ;
	}
//	entk->at_com->rollout = AT_ROLL_COUNT ;
	return 1 ;
}
/*フラグセット*/
static void SetWaitStatus(ENETHINK *entk){

	entk->at_com->watch_status |= AT_COM_WATCH_APPROACH ;
}
static int CheckWaitStatus(ENETHINK *entk){
//	return (entk->at_com->watch_status & AT_COM_WATCH_APPROACH) ;
	if(entk->at_com->watch_status & AT_COM_WATCH_APPROACH) {
		return 1;
	}else {
		return 0;
	}
}


#if 1
static void ResetWaitStatus(ENETHINK *entk){
/*atcom.c内ではこの関数を使わず直接書き換え*/
	entk->at_com->watch_status &= ~AT_COM_WATCH_APPROACH ;
}
#endif

/*守備位置が追跡者のジャマか？*/
static	int	CheckDefPosEne(ENETHINK *entk){
	AT_THK		*at_thk,*trg_atthk ;
	E_UNIT		*e_unit ;
	ENETHINK	*trgentk;
	int i ;
	float	dis ;

	at_thk = (AT_THK *) entk->character ;
	e_unit = entk->com->enemys.group[entk->g_id]->unit[entk->u_id] ;


	/*回避位置有効守備兵以外には無用*/
	if(
	(!(at_thk->at_status & AT_ST_DEFENSE))
	||(!(at_thk->at_status & AT_ST_DODGE))
	){
		return 0;
	}

	for(i=0;i<e_unit->enemy_num;i++){
		if(i==entk->id) {
			continue ;
		}
		trgentk = e_unit->entk[i];
		trg_atthk = (AT_THK *) trgentk->character ;
		if(trg_atthk->at_status & AT_ST_DEFENSE){
			/*相手も守備兵*/
			continue ;
		}
		/*距離チェック*/
		dis = GV_VecLen3F2(&entk->def_pos,&trgentk->znavi->flore_pos);
		if( dis < (float) AT_DEF_DODGE_DIS ) {
			return 1;
		}
	}
	return 0;
}

#if 1
/*拠点防衛モードへの分岐チェック*/
static int CheckDefPosMode(ENETHINK *entk){
	AT_THK *at_thk ;
	int pl2defdis ,ene2defdis ;
//	int def_addr ;

	at_thk = (AT_THK *) entk->character ;

	if ( (GM_GameStatus & STATE_VR_ANOTHER)&&(at_thk->at_status & AT_ST_DEFENSE) ) {
		return 1 ;
	}
/*守備位置があいてる？*/
	if(CheckDefPosEne(entk)){
		/*あいてない*/
		return 0;
	}
	
	if(!(at_thk->at_status & AT_ST_DEFENSE)){
		return 0;
	}
	/*プレイヤと守備位置のゾーン距離*/
	pl2defdis = 
		ENE_GetRouteDis(&GM_PlayerPosition,&entk->def_pos,
//		GM_PlayerAddress,at_thk->def_addr,AT_DEF_START_DIS_PL) ;
		GM_PlayerAddress,at_thk->def_addr,(entk->sense.eye_s - 1000)) ;
//	if(pl2defdis >= AT_DEF_START_DIS_PL) return 0 ;
	/*プレイヤが守備位置から遠ければ無し*/
	if(pl2defdis >= (entk->sense.eye_s - 1000)) return 0 ;
	/*敵兵と守備位置の距離*/
	ene2defdis = 
		ENE_GetRouteDis(&entk->znavi->flore_pos,&entk->def_pos,
		entk->ctrl->addr ,at_thk->def_addr,AT_DEF_START_DIS_ENE) ;
	/*守備値が遠ければ無し*/
	if(ene2defdis >= AT_DEF_START_DIS_ENE) return 0 ;
	/*位置関係を調べる*/
	if(pl2defdis > (ene2defdis+1000)) return 1 ;
	if( (ene2defdis+1000) < at_thk->zone_dis ) return 1;
	return 0 ;
}
#endif

/*指定点の両サイドが見えるかチェック*/
#if 0
static void SideOnline(FVECTOR *trg,FVECTOR *pos,float slide){
	FVECTOR trg0[2],trg1[2],sub;
	SVECTOR	rot;
	int dir,i ;

	_sceVu0SubVector( &sub, trg, pos ) ;
	dir = GV_VecDir2( &sub );

	rot.vx = 0;
	rot.vy = dir;
	rot.vz = 0;

	trg0[0] = trg0[1] = DG_ZeroVector ;
	trg0[0].vx =  slide ;
	trg0[1].vx = -slide ;
//	DG_SetPos2( &GM_PlayerFindPos, &rot ) ;
	DG_SetPos2( trg, &rot ) ;
	DG_PutVector( trg0, trg1, 2 );
	for(i=0;i<2;i++){
		if(!HZX_OnlineHazardCheck(GM_PlayerControl->hzx_id,
			pos,&trg1[i],HZX_CHK_ALL,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) )
		{
			/*見えた*/
			*trg = trg1[i] ;
			break ;
		}
	}
}
#endif
/*あるゾーンに立ったプレイヤが見えるか？*/
static int CheckStandOnZone( ENETHINK *entk,int addr ,FVECTOR *respos){
	HZX_ZON	*zone; 
//	FVECTOR	epos,trgpos,testpos ;
	FVECTOR	epos ,testpos ;

	if(SIG_CheckStealthStatus(entk)){
		return 1 ;
	}

	epos = entk->znavi->flore_pos ;
	epos.vy += 1500.0F;
	zone = HZX_GetZone(entk->ctrl->hzx_id,HZX_Zone1(addr) );
	SIG_SetZonePos2Fvec( zone ,respos) ;
	SIG_SetZonePos2Fvec( zone ,&testpos) ;
	testpos.vy += 1250.0F;

	if(HZX_OnlineHazardCheck(entk->ctrl->hzx_id,&epos,
	&testpos,HZX_CHK_ALL,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) ){
		/*見えない*/
		return 1 ;
	}else {
		/*たてば見える*/
//		*respos = GM_PlayerFindPos ;
		respos->vx = GM_PlayerPosition.vx ;
		respos->vz = GM_PlayerPosition.vz ;
		return 0 ;
	}

}

/*威嚇射撃の座標を決定する*/
/*目標*/
#if 0
static void CheckThreatPos( ENETHINK *entk,FVECTOR *respos){
	HZX_ZON	*zone; 
	FVECTOR	epos,trgpos ;
	epos = entk->ctrl->mov;
	zone = HZX_GetZone(GM_PlayerControl->hzx_id,
				HZX_Zone1(GM_PlayerAddress) );

	trgpos.vx = zone->x;
	trgpos.vy = zone->y;
	trgpos.vz = zone->z;

	if(trgpos.vx < epos.vx) {
		trgpos.vx += zone->w ;
	}else {
		trgpos.vx -= zone->w ;
	}
	if(trgpos.vz < epos.vz) {
		trgpos.vz += zone->h ;
	}else {
		trgpos.vz -= zone->h ;
	}
	respos->vx = trgpos.vx ;
	respos->vy = trgpos.vy ;
	respos->vz = trgpos.vz ;
}
#endif
/*ゾーンに近い点とす*/

static void CheckThreatPosZone(ENETHINK *entk,FVECTOR *respos)
{
	int addr ;
	HZX_ZON	*zone; 
	FVECTOR	epos;
	/*プレイヤ側からこちらへ近づいたゾーン*/
	addr = HZX_Navigate( GM_PlayerAddress,entk->ctrl->addr , &GM_PlayerPosition );
	zone = HZX_GetZone(GM_PlayerControl->hzx_id,
				HZX_Zone1(addr) );
	respos->vx = zone->x;
//	respos->vy = zone->y;
	respos->vy = GM_PlayerControl->levels[ 0 ] ;
	respos->vz = zone->z;

//	epos = entk->ctrl->mov;
	epos = GM_PlayerPosition; 

	if(respos->vx < epos.vx) {
		respos->vx += zone->w ;
	}else {
		respos->vx -= zone->w ;
	}
	if(respos->vz < epos.vz) {
		respos->vz += zone->h ;
	}else {
		respos->vz -= zone->h ;
	}
}
/*目標座標に近いゾーン内座標を取得*/
/*addはmapを含む*/
static void GetNearPosInZone( HZX_GROUP_ID id ,int addr,FVECTOR *trgpos,FVECTOR *respos ){
	HZX_ZON	*zone; 

//	zone = ENE_HZX_GetZone(addr);
//extern	inline	HZX_ZON	*HZX_GetZone( HZX_GROUP_ID id, int zone )
	zone = HZX_GetZone( id, HZX_Zone1(addr) ) ;
	respos->vx = (float)zone->x;
	respos->vy = (float)zone->y;
	respos->vz = (float)zone->z;

#if 0
	if( respos->vx + (float)zone->w <= trgpos->vx ){
		respos->vx += (float)zone->w *0.8F ;
	}else if( respos->vx - (float)zone->w >= trgpos->vx ){
		respos->vx -= (float)zone->w *0.8F ;
	}

	if( respos->vz + (float)zone->h <= trgpos->vz ){
		respos->vz += (float)zone->h *0.8F ;
	}else if( respos->vz - (float)zone->h >= trgpos->vz ){
		respos->vz -= (float)zone->h *0.8F ;
	}
#else
	if( respos->vx + (float)zone->w <= trgpos->vx ){
		respos->vx += (float)zone->w ;
	}else if( respos->vx - (float)zone->w >= trgpos->vx ){
		respos->vx -= (float)zone->w ;
	}else {
		respos->vx = trgpos->vx ;
	}

	if( respos->vz + (float)zone->h <= trgpos->vz ){
		respos->vz += (float)zone->h ;
	}else if( respos->vz - (float)zone->h >= trgpos->vz ){
		respos->vz -= (float)zone->h ;
	}else {
		respos->vz = trgpos->vz ;
	}

#endif
}
#if 0
static int RouteIndex( int z1, int z2 ,int n_zone){
	int index;

	if( z2 < z1 ){
		int tmp;
		tmp = z2;
		z2 = z1;
		z1 = tmp;
	} else if( z1 == z2 ){
		return 0;
	}

	index = ( z1 * ( 2 * n_zone - z1 - 3 ) ) / 2 + z2 - 1;
	return index ;
}
#endif
#if 0
static void RoutePrint(ENETHINK *entk){
	HZX_HDL *hdl;
	HZX_GRP *grp_top, *grp;
	int mapno ,n_zones ,index;

	hdl = HZX_GetCurrentHzx();
	grp_top = hdl->grp;

	mapno = HZX_ZoneMapNo( entk->ctrl->addr ) ;

	grp = grp_top + mapno;
	n_zones = grp->n_zones ;
	printf("n_zones %d\n",n_zones);
	index = RouteIndex((entk->ctrl->addr&255),
		(GM_PlayerAddress&255),n_zones) ;
	printf("index %d\n",index);
}
#endif
/*プレイヤマップを設定*/
static int AddrSet(int addr){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(GM_PlayerAddress)<<16));
	return addr;
}

#if 0
/*体の中心から射線がずれている時の射撃方向補正*/
/* dis 体の中心から銃までの距離 */
static short GetHomingDirY(ENETHINK *entk , float dis)
{
	FVECTOR		vec ;
	short	dir ;

	vec.vx = dis;
	vec.vz = (float) entk->pl_eyei.dis;
//printf("PL DIS %f\n",vec.vz);
	dir = GV_VecDir2( &vec ) ;
//	dir += entk->pl_eyei.dir;
	return (entk->pl_eyei.dir - dir)& 4095 ;
}
#endif


static void GoNearAttack(ENETHINK *entk)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;


	entk->think2 = TH2_ATTACK ; 
	if( entk->think3 != TH3_THREAT ) entk->count3 = 0 ;
	if(
	(at_thk->at_status & AT_ST_FEEL)
	&&(at_thk->in_sight < (AT_THK_RATE*2) )
	){
		if(GM_AlertLevel != ALERT_LEVEL_MAX){
//SD_V_ATKO01 攻撃兵１対離れ「そこにいるぞ！」//gbs_a001 308
			SIG_AT_VoiceCall(entk,SD_V_ATKO01,AT_V_SOKONI) ;
		}
		entk->think2 = TH2_THREAT ; 
		entk->think3 = TH3_THREAT ;
	}else {
		entk->think3 = TH3_ATTACK_NEAR ;
	}
	at_thk->at_tmptime = 0;
}
static void GoThreat(ENETHINK *entk)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	entk->think2 = TH2_THREAT ; 
	entk->think3 = TH3_THREAT ;
	entk->count3 = 0 ;
	at_thk->at_tmptime = 0;
}

static void GoChaseWait(ENETHINK *entk)
{
	entk->think2 = TH2_CHASE ; 
	entk->think3 = TH3_WAIT_CHASE ;
	entk->count3 = 0 ;
}
static void GoMedication(ENETHINK *entk)
{
	entk->think2 = TH2_PBREAK ; 
	entk->think3 = TH3_MEDICATION ;
	entk->count3 = 0 ;
}
static void GoPbreak(ENETHINK *entk)
{
	entk->think2 = TH2_PBREAK ; 
	entk->think3 = TH3_ATTACK_NEAR ;
	entk->count3 = 0 ;
}
static void GoM4GrdShoot(ENETHINK *entk)
{
	entk->think2 = TH2_ATTACK ; 
	entk->think3 = TH3_GRD_M4_SHOOT ;
	entk->count3 = 0 ;
}
static void GoAttackPeek(ENETHINK *entk ,char mode)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	/*AK以外は覗き無し 進行する*/
	if(
	(entk->act->bodyp.type & ENE_TYPE_SHIELD)
	||(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)
	){
		at_thk->th2_buf = mode ;
		entk->bullet = 0;
		entk->think2 = TH2_CHASE ; 
		entk->think3 = TH3_SIDE_MOVE;
		entk->count3 = 0 ;
	}else {
		at_thk->th2_buf = mode ;
		entk->bullet = 0;
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_PEEK ;
		entk->count3 = 0 ;
	}
}
static void GoDirectChase(ENETHINK *entk)
{
	entk->think2 = TH2_CHASE ; 
	entk->think3 = TH3_DIRECT_CHASE ;
	entk->count3 = 0 ;
}


static void GoReload(ENETHINK *entk)
{
	AT_THK *at_thk ;
#if 0
printf("GO RELOAD \n");
printf("TH2 == %d\n",entk->think2);
printf("TH3 == %d\n",entk->think3);
printf("PAD == %d\n",entk->act->pad);
printf("KEEPPAD == %d\n",entk->act->keep_pad);
printf("bullet %d\n",entk->bullet) ;
printf("MAX bullet %d\n",entk->max_bullet) ;
#endif
	entk->think2 = TH2_ATTACK ; 
	entk->bullet = 0 ;

/*TEST E3*/
	at_thk = (AT_THK *) entk->character ;
#if 1
	if(0){
#else
	/*先頭兵は撃ち尽くすとグレネード投げ*/
	if(
	(at_thk->at_status & AT_ST_FEEL)
	&&(at_thk->dis_rank == 0 )
	&&(entk->think3 == TH3_ATTACK_NEAR)
	){
#endif
		entk->think3 = TH3_ATTACK_GRD_HIGH;
	}else {
		entk->think3 = TH3_ATTACK_RELOAD ; 
		if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
			at_thk->at_status &= ~AT_ST_SQUAT ;
			entk->act->pad = SP_RELOAD ;
		}else if(at_thk->at_status & AT_ST_SQUAT){
			/*しゃがみ状態*/
			entk->act->pad = SP_RELOAD_SQUAT ;
		}else {
			entk->act->pad = SP_RELOAD ;
		}
	}
	entk->count3 = 0 ;
	return ;
}
static void GoGrdHigh(ENETHINK *entk)
{
	entk->think2 = TH2_ATTACK ; 
	entk->think3 = TH3_ATTACK_GRD_HIGH;
	entk->act->pad = SP_GRD_HIGH;
	entk->count3 = 0 ;
}
/*死に様に驚く*/
static void GoDeathBed(ENETHINK *entk)
{
	entk->think2 = TH2_DISCOVERY ; 
	entk->think3 = TH3_DISCOVERY_DEATH ;
	entk->count3 = 0 ;
}


static inline int FvecCheck(pos1,pos2 )
FVECTOR *pos1,*pos2;
{
	if(
	  (pos1->vx != pos2->vx )
	||(pos1->vy != pos2->vy )
	||(pos1->vz != pos2->vz )
	){
		return 1;
	}
	return 0;
}

static void GoEasyAttack(ENETHINK *entk, int time)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	entk->think2 = TH2_ATTACK ;
	entk->think3 = TH3_ATTACK_EASY ;
	entk->count3 = 0 ;
	at_thk->at_tmptime = time;
}
/*伏せて隠れろ*/
static void GoEasyAttackLie(ENETHINK *entk, int time)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	entk->think2 = TH2_ATTACK ;
	entk->think3 = TH3_ATTACK_EASY_LIE ;
	entk->count3 = 0 ;
	at_thk->at_tmptime = time;
}
static void GoDogeNikita(ENETHINK *entk)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	at_thk->surprised = DIRECT_TICK(10) ;
	entk->think2 = TH2_ATTACK ;
	entk->think3 = TH3_DODGE_NIKITA ;
	entk->count3 = 0 ;
}

static void GoReturnAttack(ENETHINK *entk, int time ,int th2,int th3)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	at_thk->th2_buf = th2 ;
	at_thk->th3_buf = th3 ;

	entk->think2 = TH2_APPROACH ;
	entk->think3 = TH3_ATTACK_EASY ;
	entk->count3 = 0 ;
	at_thk->at_tmptime = time;
}
static void GoDiscoveryPlayer(ENETHINK *entk ){
	entk->think2 = TH2_DISCOVERY ;
	entk->think3 = TH3_FOUND_PL ;
	entk->count3 = 0 ;
}

static void SetReturnThink(ENETHINK *entk ){
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

//printf("RET THINK2 %d\n",at_thk->th2_buf);
//intf("RET THINK3 %d\n",at_thk->th3_buf);
	entk->think2 = at_thk->th2_buf ;
	entk->think3 = at_thk->th3_buf ;
	entk->count3 = 0 ;
}

static void GoPosChase(ENETHINK *entk ,FVECTOR *pos)
{
	CONTROL	ctrl;
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	/*目標地点セット*/
#ifdef BP_PS2
	(FVECTOR) entk->trgpoint.pos = *pos ;
	(FVECTOR) ctrl.mov =  *pos ;
#else
	entk->trgpoint.pos = *pos ;
	ctrl.mov =  *pos ;
#endif
//printf("GoPosChase [%x][%x][%x]\n",ctrl.mov.vx,ctrl.mov.vy,ctrl.mov.vz);

	GM_ConfigControlMapID( &ctrl ) ;
//printf("ctrl map %X HZX_ID %X addr %X\n",ctrl.map,ctrl.hzx_id,ctrl.addr);

	entk->trgpoint.addr = ctrl.addr ;
	entk->trgpoint.map = ctrl.map ;
	
	entk->think2 = TH2_CHASE ; 
	entk->think3 = TH3_ZONE_POS ;
	entk->count3 = 0 ;
	/*
	poschase終了後に分岐チェックを行うので
	標準では無指定に初期化
	使用者が個別に対応する事
	*/
	at_thk->th2_buf = -1 ; /*シンクモードの行き先予約*/
	at_thk->th3_buf = -1 ;

}

static inline void GoIntZoneChase(ENETHINK *entk )
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	GetNearPosInZone( entk->com->plmap_in_zone[0],
		HZX_Zone1(entk->com->plzone_in_zone[0]),
		&GM_PlayerPosition,&at_thk->subtrg ) ;
	GoPosChase( entk , &at_thk->subtrg) ;

	entk->think2 = TH2_INTRUDE ;
	entk->think3 = TH3_INTRUDE_ZONE_POS ;

	entk->count3 = 0 ;
}
static inline void GoIntWait(ENETHINK *entk )
{
	entk->think2 = TH2_INTRUDE ;
	entk->think3 = TH3_WAIT ;
	entk->count3 = 0 ;
}
static inline void GoLowShot(ENETHINK *entk )
{
	entk->think2 = TH2_INTRUDE ;
	entk->think3 = TH3_ATTACK_NEAR ;
	entk->count3 = 0 ;
}

/*
全員の視界から外れたら
プレイヤが移動再開したら
逃げや隠れをキャンセル
*/
static int EscCancel(ENETHINK *entk){
	return 0;
	if((entk->at_com->Pl_StayTime == 0)
	||(entk->at_com->com_sight==0))
	{
		GoEasyAttack( entk, (AT_THK_RATE*5));
		return 1;
	}
	return 0;
}
static void GoDanger(ENETHINK *entk){
//printf("GO DANGER TH3 %d\n",entk->think3);
	entk->think2 = TH2_ATTACK;
	entk->think3 = TH3_DANGER;
	entk->count3 = 0;
}
/*ビックリに移行*/
static void GoHangSurprise(ENETHINK *entk)
{
	entk->think2 = TH2_DISCOVERY ; 
	entk->think3 = TH3_DISCOVERY_HANG ;
	entk->count3 = 0 ;
}
static void GoStopSign(ENETHINK *entk)
{
//	entk->act->pad = SP_STOPSIGN ;
	entk->think2 = TH2_DISCOVERY ; 
	entk->think3 = TH3_STOPSIGN ;
	entk->count3 = 0 ;
}

static void GoAbsEscape(ENETHINK *entk)
{
	entk->think2 = TH2_DISCOVERY ; 
	entk->think3 = TH3_ABS_ESCAPE ;
	entk->count3 = 0 ;
}

static void GoDiscoveryStinger(ENETHINK *entk)
{
#if 0
	GoEasyAttackLie( entk, DIRECT_TICK(180) );
#else
	entk->think2 = TH2_DISCOVERY ; 
	entk->think3 = TH3_DISCOVERY_STINGER ;
	entk->count3 = 0 ;
#endif
}
static void GoDiscoveryDanger(ENETHINK *entk,int mode)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
//	GoEasyAttackLie( entk, DIRECT_TICK(180) );
	at_thk->surprised = DIRECT_TICK(90) ;

	entk->think2 = TH2_DISCOVERY ; 
	entk->think3 = TH3_DISCOVERY_STINGER ;
	at_thk->th3_buf =mode ;
	entk->count3 = 0 ;
}

static void GoDefPosChase(ENETHINK *entk ){
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;


	GoPosChase(entk ,&entk->def_pos) ;
	at_thk->th2_buf = TH2_ATTACK ; 
	at_thk->th3_buf = TH3_DEFENCE ;
}

static void GoEscape(ENETHINK *entk )
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

//printf("GO ESC TH3 %d\n",entk->think3);

	entk->think2 = TH2_CHASE;
	at_thk->escaddr = HZX_NO_ZONE ;
	entk->think3 = TH3_ESC_ZONECHASE;
	entk->count3 = 0 ;
}

static void GoSafeDirect(ENETHINK *entk ){
//printf("GO SAFE DIRECT %d\n",entk->think3);
	entk->think2 = TH2_CHASE ;
	entk->think3 = TH3_SAFEDIRECT_CHASE ;
	entk->count3 = 0;
}
static void GoDeploy(ENETHINK *entk )
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	if(
	(CheckSafeZone( entk ) )
//	&&(at_thk->safeaddr != entk->ctrl->addr)
	){
	/*安地発見*/
//printf("CHECK AT LINE %d\n",__LINE__);
		entk->think2 = TH2_CHASE;
		entk->think3 = TH3_SAFEZONE_CHASE ;
		entk->count3 = 0 ;
	}else {
		at_thk->escaddr = HZX_NO_ZONE ;
		entk->think2 = TH2_CHASE;
		entk->think3 = TH3_DEPLOY;
		entk->count3 = 0 ;
	}
}

/*安全地帯あるときのみ動く*/
static int GoDeploy2(ENETHINK *entk )
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	if(
	(CheckSafeZone( entk ) )
//	&&(at_thk->safeaddr != entk->ctrl->addr)
	){
	/*安地発見*/
//printf("CHECK AT LINE %d\n",__LINE__);
		entk->think2 = TH2_CHASE;
		entk->think3 = TH3_SAFEZONE_CHASE ;
		entk->count3 = 0 ;
		return 1 ;
	}
	return 0 ;
}

/**/
static void SetThBuf(ENETHINK *entk){
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	at_thk->th2_buf = entk->think2 ;
	at_thk->th3_buf = entk->think3 ;
}

static void GoApproach(ENETHINK *entk)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	/*現在位置を帰還目標として待避*/
	at_thk->subtrg = entk->znavi->flore_pos ;
	
	entk->think2 = TH2_APPROACH;
	entk->think3 = TH3_APPROACH;
	SetThBuf(entk) ;
	SetWaitStatus(entk);
	entk->count3 = 0 ;
}
static void GoApproachWait(ENETHINK *entk)
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	/*現在位置を帰還目標として待避*/
	at_thk->subtrg = entk->znavi->flore_pos ;
	
	entk->think2 = TH2_APPROACH ;
	entk->think3 = TH3_WAIT_CHASE ;
	SetThBuf(entk) ;
	entk->count3 = 0 ;
}

/*テスト ロッカー情報取得*/
static R_INTRPT *GetLockerInfo( FVECTOR *pos ,int *dir){
	HZX_ZON *pl_zone ;
	int		near_addr ;
	R_INTRPT	*r_intrpt ;

//	static FVECTOR locker_open={ 1020.0f, 0.0f, 720.0f} ;
//void	ENE_StandPosRintrpt( R_INTRPT *r_intrpt, int mot, 
//FVECTOR *pos, int *dir )
//	case MOT_RINTRPT_LOCKER_OPEN :


	/*ロッカーの隣接ゾーン*/
	pl_zone = ENE_HZX_GetZone(GM_PlayerAddress) ;
	near_addr = pl_zone->nears[ 0 ] ;
	near_addr = HZX_Zone1(near_addr);
	near_addr = (near_addr|(near_addr<<8)
		|(HZX_ZoneMapNo(GM_PlayerAddress)<<16));
	r_intrpt = GM_GetRIntrpt( GM_PlayerAddress, near_addr ) ;
	if(r_intrpt != NULL ){
//		PosBox(&r_intrpt->pos ,50.0F ,(SVECTOR *) NULL );
		ENE_StandPosRintrpt( r_intrpt,MOT_RINTRPT_LOCKER_OPEN , 
			pos, dir ) ;
	}else {
		printf("r_intrpt NULL !!!!!!!!!!!!!\n");
//		ASSERT(0);
	}
	return r_intrpt ;
}

static int GoAttackLocker(ENETHINK *entk)
{
	FVECTOR pos ;
	int		dir ;
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	if(at_thk->dis_rank != 0 ) return 0 ;
	
	if(GetLockerInfo(&pos,&dir) != NULL ){
		GoPosChase(entk ,&pos) ;
		at_thk->th2_buf = TH2_ATTACK ; 
		at_thk->th3_buf = TH3_ATTACK_LOCKER ;
		entk->count3 = 0 ;
		return 1 ;
	}
	return 0 ;
}


static int SetAimPosPlayer(ENETHINK *entk ,int mode)
{
	int subdir;
	FVECTOR		shotpos;
/*TEST*/
#if 0
	{
		entk->status2 &= ~ENE_STATUS2_AIM_GUNSHOOT ;
		return -1;
	}
#endif

	subdir = SIG_CheckDirSub(entk->ctrl->turn.vy,entk->pl_eyei.dir);
//	if(abs(subdir)<700){
	if(abs(subdir)<400){
//	if(1){
//		entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
//		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	entk->status2 |= (ENE_STATUS2_AIM_FACE|ENE_STATUS2_AIM_GUNSHOOT) ;
//		shotpos = GM_PlayerPosition;
		shotpos = GM_PlayerFindPos ;
		entk->act->aim_pos = shotpos;
		return 0;
	}
	return -1;
}
static void SetAbsAimPosPlayer(ENETHINK *entk )
{
//	int subdir;
//	FVECTOR		shotpos;
//	entk->status2 |= ENE_STATUS2_AIM_GUN ;
	entk->status2 |= (ENE_STATUS2_AIM_FACE|ENE_STATUS2_AIM_GUNSHOOT) ;
	entk->act->aim_pos = GM_PlayerFindPos ;
#if 0
	PosBox(&entk->act->aim_pos ,250.0F ,(SVECTOR *) NULL );
#endif
}
static void SetAimPosEyei(ENETHINK *entk)
{
//	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->status2 |= ENE_STATUS2_AIM_GUN ;
	entk->act->aim_pos = *entk->pl_eyei.pos ;
}



static void SetAimPosNPC_Eyei(ENETHINK *entk)
{
	entk->status2 |= ENE_STATUS2_AIM_GUN ;
	entk->act->aim_pos = *entk->npc_eyei.pos ;
}
static void SetAimPosTrgp(ENETHINK *entk)
{
	entk->status2 |= ENE_STATUS2_AIM_GUN ;
	entk->act->aim_pos = entk->trgpoint.pos ;
}

/*撹乱開始チェック*/
static int CheckPrudence(ENETHINK *entk){
	AT_THK *at_thk;
	long64 pl_status ;

	at_thk = (AT_THK *) entk->character ;

	if(SIG_CheckStealthStatus(entk)) return 0 ;
	pl_status = GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ;
	if(pl_status) return 0;
	if(!(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)) return 0;

	if(
	(ENE_AlertGameLevel >= AT_PRUDENCE_LEVEL)
	&&(entk->count3 > (AT_THK_RATE*2))&&(at_thk->dis_rank>0)
	&&(CheckX_Fire(entk))
//	&&(!ENE_ReadOnlinInfo(entk->ctrl->addr,GM_PlayerAddress))
	){
#if 0
		GoDeploy(entk);
		return 1;
#else
		if(GoDeploy2(entk)){
			return 1;
		}
#endif
	}
	return 0;
}



/*----- サブルーチン --------------------------------------------------*/

#define WALLHIT	512
#define	LEFT_WALL		(0x01)
#define	RIGHT_WALL		(0x02)
#define	LEFT_NOZONE		(0x04)
#define	RIGHT_NOZONE	(0x08)
/****  チェック関数群  ****/

static int	CheckEnePlOnline(ENETHINK *entk){
	FVECTOR	pl_pos;


#if 0
	pl_pos = GM_PlayerPosition;
	pl_pos.vy +=750.0F;
#else
	pl_pos = GM_PlayerFindPos ;
#endif

	if(HZX_OnlineHazardCheck(entk->ctrl->hzx_id,&entk->ctrl->mov,
	&pl_pos,HZX_CHK_ALL,
	HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) )
//	HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY) )
	{
		return 1;
	}
	return 0;
}


#if 0
static int CheckFrontWall(ENETHINK *entk)
{
	int i,walldir,dirsub ;
	/*壁方向角度チェック*/
	if(entk->ctrl->n_touches){
		for(i=0;i<entk->ctrl->n_touches;i++){
			walldir = GV_VecDir2( &entk->ctrl->vecs[i] )+4096;
			walldir = (walldir+2048)&4095;
			/*dir1からみた角度差*/
			dirsub = SIG_CheckDirSub(entk->ctrl->turn.vy,walldir);
			if((dirsub > -512)&&( dirsub <512))
				return 1;
		}
	}
	return 0;
}
#endif

static int CheckToucheWallDir(ENETHINK *entk,int testdir ,int range )
{
	int i,walldir,dirsub ;
	/*壁方向角度チェック*/
	if(entk->ctrl->n_touches){
		for(i=0;i<entk->ctrl->n_touches;i++){
			walldir = GV_VecDir2( &entk->ctrl->vecs[i] ) ;
			dirsub = GV_DiffDirAbs( testdir, walldir ) ;
			if(dirsub < range) return 1;
		}
	}
	return 0;
}


#define W_SPHERE (600)
#define AT_CHK_SEG (HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT )
/* 壁衝突判定 */
static int CheckWallDir(ENETHINK *entk)
{
	int i,walldir,subdir,side = 0 ;

//	int		ch_hzx,group;
	int		ch_hzx ;
	FVECTOR		vect_ptr[2]; /*壁判定*/
	CONTROL		*ctrl;

	ctrl = entk->ctrl;

//ctrl->s_sphere
	/*壁方向角度チェック*/
	ch_hzx = HZX_NearHazardCheck( ctrl->hzx_id,&( ctrl->mov ),
//	W_SPHERE,ctrl->hzx_check_type, ctrl->seg_flag,W_SPHERE+100 ) ;
	ctrl->s_sphere,ctrl->hzx_check_type, ctrl->seg_flag,ctrl->s_sphere ) ;

	if(ch_hzx){
#if 0
		HZX_SEG segs[2] ;
		int atrs[2] ; 
		HZX_GetNearHazard( segs, atrs ) ;
#endif

		HZX_GetNearVector( vect_ptr );

		for(i=0;i<ch_hzx;i++){
#if 0
			HZX_ViewSegment( &segs[i] ) ;
#endif
			walldir = GV_VecDir2( &vect_ptr[i] );
			subdir = SIG_CheckDirSub(entk->ctrl->turn.vy,walldir);
/*壁チェックは前方にも広げろ*/
			if( (subdir >= (-1024-512) ) &&(subdir <= -64 ) ) {
				/*右壁*/
				side |= RIGHT_WALL;
			}else if ( (subdir >= 64) &&(subdir <= 1024+512 )){
				/*左壁*/
				side |= LEFT_WALL;
			}
		}
	}
	return side;
}



#if 0
static int CheckBackWall( ENETHINK *entk){
	int walldir,i;
	if(entk->ctrl->n_touches){
		for(i=0;i<entk->ctrl->n_touches;i++){
			walldir = GV_VecDir2( &entk->ctrl->vecs[i] );
			walldir = SIG_CheckDirSub(entk->ctrl->turn.vy,walldir);
			if( abs(walldir)>(1024+512+256)) return 1;
		}
	}
	return 0;
}
#endif

/*サイドのゾーン番号を返す*/
static int CheckSideNum(ENETHINK *entk,float checklen,FVECTOR *trgpos)
{
	int testdir,testaddr1,testaddr2,returnaddr;
//	FVECTOR testpos[2];

/*TEST チェック幅を固定*/
	testdir = (int)(entk->pl_eyei.dir+1024);
	testdir &= 4095;
	/*右*/

	trgpos[0] = entk->znavi->flore_pos;
	trgpos[0].vx -= checklen * _RsinF( testdir ) ;
	trgpos[0].vz -= checklen * _RcosF( testdir ) ;
	/*左*/
	trgpos[1] = entk->znavi->flore_pos;
//	trgpos[1] = entk->ctrl->mov;
	trgpos[1].vx += checklen * _RsinF( testdir ) ;
	trgpos[1].vz += checklen * _RcosF( testdir ) ;

	testaddr1 = 
	HZX_GetAddress( entk->ctrl->hzx_id, &trgpos[0], entk->ctrl->addr );
	testaddr2 =
	HZX_GetAddress( entk->ctrl->hzx_id, &trgpos[1], entk->ctrl->addr );
/*********************************************
ここでは通常のゾーン形式ではなく、二つのゾーン番号を
ひとつの変数内にシフトで収めている。 
MAPNOを追加しなければならない
近日対応。
*******************/

	returnaddr = ((HZX_Zone2(testaddr1))|(HZX_Zone2(testaddr2)<<8));
	return returnaddr;
}
#if 0
static int CheckBackNum(ENETHINK *entk,float checklen,FVECTOR *trgpos)
{
	int testdir,testaddr,returnaddr;
//	FVECTOR testpos;
	testdir = (int)(entk->pl_eyei.dir+2048);
	testdir &= 4095;
	*trgpos = entk->znavi->flore_pos;
	trgpos->vx -= checklen * _RsinF( testdir ) ;
	trgpos->vz -= checklen * _RcosF( testdir ) ;

	testaddr = 
	HZX_GetAddress( entk->ctrl->hzx_id, trgpos, entk->ctrl->addr );

//	returnaddr = HZX_Zone2(testaddr);
	returnaddr = testaddr;
	return returnaddr;
}
#endif

/*横移動で行く先が安地になってるか*/
/* あるいはNO_ZONEか */
static int CheckSideSafe(ENETHINK *entk,float checklen)
{
	int i,j,side = 0,safeaddr,testdir,testaddr,reach;
	FVECTOR testpos[2];
	HZX_ZON *pl_zone ;
	pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);

//if(pl_zone == NULL) printf("PL ZONE NULL!!\n");


	testdir = (int)(entk->pl_eyei.dir+1024);
	testdir &= 4095;

	/*右*/

	testpos[0] = entk->ctrl->mov;
	testpos[0].vx -= checklen * _RsinF( testdir ) ;
	testpos[0].vz -= checklen * _RcosF( testdir ) ;

	/*左*/
	testpos[1] = entk->ctrl->mov;
	testpos[1].vx += checklen * _RsinF( testdir ) ;
	testpos[1].vz += checklen * _RcosF( testdir ) ;


	for(j=0;j<2;j++){

		testaddr = HZX_GetAddress( entk->ctrl->hzx_id, &testpos[j], 
		entk->ctrl->addr );

		reach = Sig_GetRoute( entk->ctrl->hzx_id,
			testaddr,entk->ctrl->addr);


		testaddr = HZX_Zone1(testaddr);
		/* no zone check*/
		/* 隣接ゾーンでなければ不正 */
		if((testaddr == HZX_NO_ZONE)||(reach > HZX_DIRECT_REACH)
		){

			if(j==0) side |= RIGHT_NOZONE;
			else side |= LEFT_NOZONE;
			continue;
		}


		for(i=0;i<HZX_MAX_SAFEZONE_NUM;i++){
			safeaddr = pl_zone->safes[i];
			if(safeaddr == HZX_NO_ZONE) break;
			if(
			(testaddr == safeaddr)
			){
				if(j==0) side |= RIGHT_WALL;
				else side |= LEFT_WALL;
			}
		}
/*2000.06.20追加 見えないゾーンも安地扱い*/
		if(ENE_ReadOnlinInfo(MapAddrSet(testaddr,entk->ctrl->addr),
		GM_PlayerAddress)){
		}
			if(j==0) side |= RIGHT_WALL;
			else side |= LEFT_WALL;
	}
	return side;
}



#define STOPLEN 1000 /*許容誤差*/
#define RUNLEN (STOPLEN+4000) /* 横移動と走りの分岐基準 */
/*包囲するための移動方向を決定*/
/*flag １の時のみ安地突入チェックを行う*/
#if 0
static int CheckForm(ENETHINK *entk,int flag)
{
	int formdir1,formdir2,nowdir1,nowdir2;
	int side ;

	int dir,len,pad;
	float lenf;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	if(entk->at_com->siege[entk->u_id]==0){
//printf("SIEG STOP\n");
		return SP_READYGUN;
	}

	/*円周距離で判定*/
	dir =(entk->pl_eyei.dir+4096 - at_thk->form_dir)&4095;

	if(dir>=2048) dir = (4096-dir);

	len = (entk->pl_eyei.dis*dir)>>12;
	lenf = (float)len *(6.28F);

	if(lenf<STOPLEN){
		/*範囲内なので停止*/
		return SP_READYGUN;
	}

	/*壁方向角度チェック*/
	side = CheckWallDir(entk);
	/*安地突入チェック*/

	if(flag) {
//printf("CHECK FORM\n");
//		side |= CheckSideSafe(entk,1000.0F);
		side |= CheckSideSafe(entk,2000.0F);
	}
	pad = SP_READYGUN;

	nowdir1 = entk->pl_eyei.dir;
	nowdir2 = entk->pl_eyei.dir-2048;
	formdir1 = at_thk->form_dir;
	formdir2 = at_thk->form_dir-4096;

	if( ( (nowdir2 < formdir1) && (formdir1 < nowdir1) )
	||  ( (nowdir2 < formdir2) && (formdir2 < nowdir1) ) )
	{
		/*左*/
		if((side & LEFT_WALL)||(side & LEFT_NOZONE)) {
		/*壁ぶつかってる*/
			return SP_READYGUN;
		} else {
//			if((lenf>RUNLEN)&&(entk->act->keep_pad == SP_CLE_PEEP_L )) {
			if(( entk->pl_eyei.dis > entk->at_com->minlen )&&(lenf>RUNLEN)) {
				pad = SP_MOVE_RUN_L;
			}
			else {
				pad = SP_CLE_PEEP_L;
			}
		}
	} else {
		/*右*/
		if((side & RIGHT_WALL)||(side & RIGHT_NOZONE)) {
			/*壁ぶつかってる*/
			return SP_READYGUN;
		} else {
//			if(lenf>RUNLEN) {
//			if((lenf>RUNLEN)&&(entk->act->keep_pad == SP_CLE_PEEP_R )) {
			if(( entk->pl_eyei.dis > entk->at_com->minlen )&&(lenf>RUNLEN)) {
				pad = SP_MOVE_RUN_R;
			}else {
				pad = SP_CLE_PEEP_R;
			}
		}
	}
	/* アクション変更タイミング調整 */
	return pad;
}
#endif

/*コーナー飛び出し時の方向判定*/
static int CheckTurnSide(ENETHINK *entk)
{
	int formdir1,formdir2,nowdir1,nowdir2;


	nowdir1 = entk->trgpoint.dir;
	nowdir2 = entk->trgpoint.dir-2048;

	formdir1 = entk->pl_eyei.dir;
	formdir2 = entk->pl_eyei.dir-4096;

	if( ( (nowdir2 < formdir1) && (formdir1 < nowdir1) )
	||  ( (nowdir2 < formdir2) && (formdir2 < nowdir1) ) )
	{
		/*左*/
		return 0;
	} else {
		/*右*/
		return 1;
	}
		return 0;
}
#if 0
/** 指定された行くためのゾーンの方向を調べる **/
static int CheckZoneSide(ENETHINK *entk,int addr)
{
	HZX_ZON		*zone; 
	FVECTOR		trgpos;
	int zonedir,formdir1,formdir2,nowdir1,nowdir2;

	zone = ENE_HZX_GetZone( addr ) ;

	trgpos.vx = zone->x;
	trgpos.vy = zone->y;
	trgpos.vz = zone->z;
	zonedir = GV_VecDir2(&trgpos)+4096;


	nowdir1 = zonedir;
	nowdir2 = zonedir-2048;
	formdir1 = entk->pl_eyei.dir;
	formdir2 = entk->pl_eyei.dir-4096;

	if( ( (nowdir2 < formdir1) && (formdir1 < nowdir1) )
	||  ( (nowdir2 < formdir2) && (formdir2 < nowdir1) ) )
	{
		/*左*/
		return 0;
	} else {
		/*右*/
		return 1;
	}
}

#endif

/*安地使用権 破棄*/
static inline void ResetUseZone(ENETHINK *entk)
{
	ASSERT(entk->u_id < ATUNIT_MAX) ;
	ASSERT(entk->id < ATENEMY_MAX) ;
	entk->at_com->usezones[entk->u_id][entk->id] = HZX_NO_ZONE;
}
/*ゾーンに誰かいるかチェック*/
static int CheckUseZone( ENETHINK *entk ,int tmpaddr )
{
	int i;
	int ene_num;
	ene_num = entk->com->enemys.group[entk->g_id]->unit[entk->u_id]->enemy_num ;
	/*同じチーム内でのみぶつかり判定*/
	for(i=0;i < ene_num; i++){
		if(i==entk->id) continue; /*自分用はスキップ*/
		if(entk->at_com->usezones[entk->u_id][i] == tmpaddr) return (-1);
		/* 誰かいる?*/
ASSERT(entk->at_com->teamaddr[entk->u_id][i] != NULL) ;
		if( *entk->at_com->teamaddr[entk->u_id][i] == tmpaddr) 
			return (-1);
	}
	return (0);
}
/*両サイドに他の攻撃兵がいるか？*/
/* 角度で比較 */
#define CHECK_LENGTH 450
static int CheckSideAT(ENETHINK *entk,float checklen)
{
	int i,dirval,sub,ene_num,c_dir1,c_dir2,
	t_dir,m_dir,res=0;
	FVECTOR vec;
	ENETHINK	**teamentk;

	ene_num = entk->com->enemys.group[entk->g_id]->
		unit[entk->u_id]->enemy_num;
	teamentk = entk->com->enemys.group[entk->g_id]->
		unit[entk->u_id]->entk;

	vec.vy = 0;
	vec.vz = checklen;

	for(i=0;i < ene_num;i++){
		if(i==entk->id) continue;
		sub = (teamentk[i]->pl_eyei.dis -
			teamentk[entk->id]->pl_eyei.dis);
		if( abs(sub) > CHECK_LENGTH) continue;
		vec.vx = (float)teamentk[i]->pl_eyei.dis;  
		/*推定転がり角度 */
		dirval = GV_VecDir2( &vec );

		c_dir1 =(entk->pl_eyei.dir-dirval);
		c_dir2 =(entk->pl_eyei.dir+dirval);
		m_dir = (entk->pl_eyei.dir);
		t_dir =teamentk[i]->pl_eyei.dir;

		/* +side */
		if(t_dir < m_dir) t_dir += 4096;
//		if((m_dir <= t_dir)&&(t_dir < c_dir2)) res |= RIGHT_WALL;
		if((m_dir <= t_dir)&&(t_dir < c_dir2)) res |= LEFT_WALL;

		/* -side */
		if(t_dir > m_dir) t_dir -= 4096;
//		if((c_dir1 <t_dir)&&(t_dir <= m_dir)) res |= LEFT_WALL;
		if((c_dir1 <t_dir)&&(t_dir <= m_dir)) res |= RIGHT_WALL;
		if((res&RIGHT_WALL)&&(res&LEFT_WALL)) break;
	}
	return res;
}



#if 0
/*背後に他の攻撃兵がいるか？*/
static int CheckBackAT(ENETHINK *entk,int checklen)
{
	int i,subdir,ene_num,res=0,sub;
	ENETHINK	**teamentk;

	ene_num = entk->com->enemys.group[entk->g_id]->
		unit[entk->u_id]->enemy_num;
	teamentk = entk->com->enemys.group[entk->g_id]->
		unit[entk->u_id]->entk;
	for(i=0;i < ene_num;i++){
		if(i==entk->id) continue;
		/*極座標比較*/
		/*ＸＺ角度*/
		subdir =SIG_CheckDirSub(entk->pl_eyei.dir,teamentk[i]->pl_eyei.dir);
		if( abs(subdir) > 64) continue;
		/*距離*/
		sub = (teamentk[i]->pl_eyei.dis -
			teamentk[entk->id]->pl_eyei.dis);
		if( (sub > (checklen-700) )&&(sub < (checklen+700) ) ){
			res =1;
			break;
		}
	}
	return res;
}

#endif


//R_INTRPT	*GM_GetRIntrptOne( HZX_ZONE_ADD from )
/*
1byte
わたしたゾーンの隣接に遮蔽物がないか？
*/

/*プレイヤの位置に対する安全地帯を取得*/
/* 見つかればentkに番号セットして１なければ０*/
static int CheckSafeZone( entk )
ENETHINK	*entk ;
{

//	int trg_addr=HZX_NO_ZONE,safe_addr=HZX_NO_ZONE,tmpsafe;
	int safe_addr=HZX_NO_ZONE,tmpsafe;
	int i,reach=(255*500),enereach,type = 0;

	HZX_ZON *pl_zone ;
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;

	pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);

#if 0
	for(i=0;i <HZX_MAX_SAFEZONE_NUM;i++){
		view_addr = AddrSet(pl_zone->safes[i]); ;
		NewZoneViewer( view_addr,1,0);
	}
#endif

	/*追跡目標のゾーンに対する安全ゾーンを調べる*/
	/*安全地帯候補4個から適切なものを検索*/
	for(i=0;i<HZX_MAX_SAFEZONE_NUM;i++){
		/*候補地を取得*/
		tmpsafe = pl_zone->safes[i] ;
		/*異常値*/
		if(tmpsafe == HZX_NO_ZONE) continue;
		/*他が使ってないか*/
		tmpsafe = AddrSet(tmpsafe);
		if( CheckUseZone( entk , tmpsafe ) ) {
			continue;
		}
		/*ドアの向こうには行かない*/
//R_INTRPT
//	*GM_GetRIntrptZ2Z( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr )
		if(GM_GetRIntrptZ2Z(GM_PlayerAddress,tmpsafe) != NULL) continue ;
		if( ENE_ZoneIntrptCheck( tmpsafe ) ) continue ;
		/*敵兵の現在地と安地候補とのゾーン数を調べる*/
		/*ゾーン間距離に変更*/
		/*ゾーンアドレス形式変更*/
		enereach = 
			ENE_GetRouteDis(&entk->znavi->flore_pos,NULL,
			entk->ctrl->addr,tmpsafe,10000);
//printf("ENEREACH %d\n",enereach);
		/*遠すぎるゾーンは無効*/
		if(enereach > 10000) continue;
		/*近さ新記録でプレイヤより遠ければ有効*/
		/*リーチ数をbuffから取得*/
//printf("BUF REACH %d\n",((pl_zone->safe_dists[i]-1)*500));
		if((enereach<reach)
		&&( enereach < (pl_zone->safe_dists[i]-1)*500) ){
			reach = enereach;
			safe_addr = tmpsafe;
			type = pl_zone->safe_types[i];
		}
	}
	at_thk->safeaddr = entk->at_com->usezones[entk->u_id][entk->id] 
		= safe_addr;
	at_thk->safetype = type;
	if(at_thk->safeaddr != HZX_NO_ZONE) return 1;
	else return 0;
}
/*指定ゾーンの隣接ゾーンのうち、
指定座標に最も近いゾーンを返す*/
static int CheckPosNearZone( FVECTOR *nowpos, int nowaddr ){
	int i,trgaddr,minaddr;
	HZX_ZON *nowzone ,*trgzone ;
	FVECTOR	trgpos ;
	float	minlen = 30000.0F ,trglen ;

	minaddr = HZX_NO_ZONE ;
	nowzone = ENE_HZX_GetZone(nowaddr);
	for(i=0;i<6;i++){
		trgaddr = nowzone->nears[ i ] ;
		if(trgaddr == HZX_NO_ZONE) break ;
		trgaddr = MapAddrSet(trgaddr,nowaddr) ;
		if(GM_GetRIntrptZ2Z(trgaddr,nowaddr) != NULL) continue ;
		trgzone = ENE_HZX_GetZone(trgaddr);
		SIG_SetZonePos2Fvec(trgzone ,&trgpos);
		trglen = GV_VecLen3F2( nowpos, &trgpos ) ;
		if(minlen >= trglen) {
			minlen = trglen ;
			minaddr = trgaddr ;
		}
	}
	return minaddr ;
}

#if 0
/* 周りの空間を調べる*/
static int CheckEscArea( entk )
ENETHINK	*entk ;
{
	int wall,sideaddr,side_at; 
	FVECTOR	trgpos[2];
//	CheckSafeZone( entk );
	if( entk->pl_eyei.dis > 2000 ){
		/* 接触壁方向チェック*/
		wall = CheckWallDir(entk);

		sideaddr = CheckSideNum(entk,1500.0F,trgpos);
		if(HZX_Zone1(sideaddr)==HZX_NO_ZONE){
			wall |= RIGHT_WALL;
		}
		if(HZX_Zone2(sideaddr)==HZX_NO_ZONE){
			wall |= LEFT_WALL;
		}
		side_at = CheckSideAT(entk,2100.0F);
		wall |= side_at;
		if((wall&RIGHT_WALL)&&(wall&LEFT_WALL)){
			return 0;
		}
		return 1;
	}
	return 0;
}
#endif

/*****
目標となる兵士の近くの空き地を調べる
mode	０ 攻撃 両サイドを調べる
		１ 守備 後ろを調べる
返り血	０前
		１右
		２後ろ
		３左
		－１ 空き地無し
*****/
#if 0
static int CheckFreeSpace(entk,id,mode,respos)
ENETHINK *entk; /*自分のentk*/
int id;		/*調査対象ｉｄ*/
int mode;	/*攻撃or守備*/
FVECTOR		*respos;
{
	int checkaddr,checkat,res = -1;
	int tmpres=0;
	ENETHINK	*trgentk;
	FVECTOR	trgpos[2];

	/*目標のentkアドレス*/
	trgentk = entk->com->enemys.group[entk->g_id]->
		unit[entk->u_id]->entk[id];

	switch(mode){
		case 0:
			/*両サイドチェック*/
			checkaddr = CheckSideNum(trgentk,1500.0F,trgpos);
			if( ( HZX_Zone1(checkaddr) == HZX_NO_ZONE)
			&&(HZX_Zone2(checkaddr) == HZX_NO_ZONE) ) break;
			checkat = CheckSideAT(trgentk,1500.0F);
			/*右*/
			if ((HZX_Zone1(checkaddr) == HZX_NO_ZONE)
			||(checkat&RIGHT_WALL)) {
				tmpres = 1;
			}
			/*左*/
			if ((HZX_Zone2(checkaddr) == HZX_NO_ZONE)
			||(checkat&LEFT_WALL)) {
				tmpres |= (1<<1);
			}
			switch(tmpres){
				case 0:
					/*両方空き*/
					if((irnd()>>8)%2){
						res = 1;
						*respos = trgpos[0];
					}else {
						res = 3;
						*respos = trgpos[1];
					}
					break;
				case (1<<1):
					/*右空き*/
					res = 1;
					*respos = trgpos[0];
					break;
				case (1):
				/*左空き*/
					res = 3;
					*respos = trgpos[1];
					break;
				case (1|(1<<1)):
				/*両ふさがり*/
					res = -1;
					break;
			}
			break;
		case 1:
			/*背後チェック*/
			checkaddr = CheckBackNum(trgentk,1500.0F,trgpos);
			if(checkaddr != HZX_NO_ZONE){
				/*他の兵士の衝突チェック*/
				if(CheckBackAT(trgentk,1500)==0) res = 2;
			}
			break;
	}
	return res;
}
#endif


#define P_SIGHT 64
//#define P_SIGHT 128
/************
mode ０視界外でも向きさえあっていれば有効
mode １視界外なら無効
************/

static int CheckPlayerSight( ENETHINK *entk ,int mode ,int p_sight)
{
	short p_dir,eye_dir,sub;
	if(GM_PlayerStatus &PLAYER_DEAD) return 0;

	/*プレイヤが武器構えじゃなければ無効*/
	if(entk->at_com->pl_weapon==0) return 0;
	/*盾装備は逃げない*/
#if 1
	if(entk->act->bodyp.type & ENE_TYPE_SHIELD) return 0;
#endif
	/*視界外なら無効*/
	if((mode)
	&&
//	( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN )
	(
	(ENE_ReadOnlinInfo(entk->znavi->next_addr,GM_PlayerAddress))
	||( entk->pl_eyei.dis > entk->sense.eye_s )
	)
	){
		return 0;
	}
	eye_dir = entk->pl_eyei.dir&4095;
	p_dir = GM_PlayerControl->turn.vy&4095;
	sub = SIG_CheckDirSub( p_dir,eye_dir);
	if ( (sub > (2048-p_sight) ) || (sub < (-2048+p_sight) ) ){
	/*範囲内*/
		if(sub>0) {
			return LEFT_WALL; 
		}else {
			return RIGHT_WALL; /*左へ行け*/
		}
	}
	return 0;
}


/*CheckPlayerSight類似　特別武器チェック*/
static int CheckPlayerWeapon( ENETHINK *entk ,int p_sight)
{
	short p_dir,eye_dir,sub;
	if(GM_PlayerStatus &PLAYER_DEAD) return 0;
//extern	u_char		GM_WeaponFire ;		/* 武器発射トリガー */

	/*プレイヤが武器構えじゃなければ無効*/
	/*スティンガーチェック*/
	if(
	(GM_WeaponFire == WP_Stinger) 
	&&((ENE_ReadOnlinInfo(entk->ctrl->addr,GM_PlayerAddress))==0)
	&&( entk->pl_eyei.dis < (entk->sense.eye_s*2) )
	){
		eye_dir = entk->pl_eyei.dir&4095;
		p_dir = GM_PlayerControl->turn.vy&4095;
		sub = SIG_CheckDirSub( p_dir,eye_dir);
		if ( (sub > (2048-p_sight) ) || (sub < (-2048+p_sight) ) ){
	/*範囲内*/
			return AT_FOUND_STINGER ;
#if 0
			if(sub>0) {
				return LEFT_WALL; 
			}else {
				return RIGHT_WALL; /*左へ行け*/
			}
#endif
		}
	}
	if(SIG_NktCheck(entk)){
		return AT_FOUND_NIKITA ;
	}
	return 0;
}
static int CheckPlayerWeapon_OLD( ENETHINK *entk ,int p_sight)
{
	short p_dir,eye_dir,sub;
	if(GM_PlayerStatus &PLAYER_DEAD) return 0;
//extern	u_char		GM_WeaponFire ;		/* 武器発射トリガー */

	/*プレイヤが武器構えじゃなければ無効*/
//	if(entk->at_com->pl_weapon==0) return 0;
//printf("GM_WeaponFire[%d]\n",GM_WeaponFire);
	/*スティンガーチェック*/
	if(
	(GM_WeaponFire == WP_Stinger) 
//	&&(GM_WeaponFire != WP_Nikita)
	) {
		return 0 ;
	}
	/*視界外なら無効*/
#if 1
	if(
	(
//	(ENE_ReadOnlinInfo(entk->znavi->next_addr,GM_PlayerAddress))
	(ENE_ReadOnlinInfo(entk->ctrl->addr,GM_PlayerAddress))
	||( entk->pl_eyei.dis > (entk->sense.eye_s*2) )
	)
	){
printf("OUT OF EYE RANGE!!!!!!!!!!!!!!!!!!!!!!\n");
		return 0;
	}
#endif

	eye_dir = entk->pl_eyei.dir&4095;
	p_dir = GM_PlayerControl->turn.vy&4095;
	sub = SIG_CheckDirSub( p_dir,eye_dir);
	if ( (sub > (2048-p_sight) ) || (sub < (-2048+p_sight) ) ){
	/*範囲内*/
		if(sub>0) {
			return LEFT_WALL; 
		}else {
			return RIGHT_WALL; /*左へ行け*/
		}
	}
	return 0;
}



static int CheckPlayerSight_Shl( ENETHINK *entk )
{
	short p_dir,eye_dir,sub;
	/*プレイヤが素手なら無効*/
	if(entk->at_com->pl_weapon==0) return 0;
	/*視界外なら無効*/
	if( CheckPl_NoShot()) {
//		printf("SHL PL NO_ACT\n");
		return 0;
	}

	if( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ){
		return 0;
	}
	eye_dir = (entk->pl_eyei.dir)&4095;
	p_dir = (GM_PlayerControl->turn.vy)&4095;
	sub = SIG_CheckDirSub( p_dir,eye_dir);
	if (( sub > 1950 )||(sub < -2000 )){
		return 1;
	}
	return 0;
}
static int CheckShlBehind(entk)
ENETHINK	*entk;
{
	int subdir,ene_num,i;
	ENETHINK	**teamentk;

/*2000.07.17*/
	return 0;


	ene_num = entk->com->enemys.group[entk->g_id]->
		unit[entk->u_id]->enemy_num;
	teamentk = entk->com->enemys.group[entk->g_id]->
		unit[entk->u_id]->entk;

	for(i=0;i < ene_num;i++){
		if(i==entk->id) continue;
		if((teamentk[i]->pl_eyei.dis > (entk->pl_eyei.dis-750) )
		&&(teamentk[i]->pl_eyei.dis < (entk->pl_eyei.dis+750) ))
		{
			/*DISの近い兵士がいる*/
			/*角度チェック*/
			subdir = SIG_CheckDirSub( teamentk[i]->pl_eyei.dir,entk->pl_eyei.dir);
			if( ( subdir < 50 )&&(subdir > -50 )){
				/*角度も近いので接触警戒*/
//				printf("NEAR AT SEARCH\n");
				return 0;
			}
		}
	}
	for(i=0;i < ene_num;i++){
		if(i==entk->id) continue;
		if(!(teamentk[i]->act->bodyp.type & ENE_TYPE_SHIELD) ) continue ;
		if(teamentk[i]->pl_eyei.dis > (entk->pl_eyei.dis-500) ) continue;
		subdir = SIG_CheckDirSub( teamentk[i]->pl_eyei.dir,entk->pl_eyei.dir);
		if( ( subdir < 50 )&&(subdir > -50 )){
//printf("SHL BACK !!\n");
			return 1;
		}
	}
	return 0;
}

/*プレイヤを正面に捕らえるための横移動*/
#if 0
static int CheckPlayerPos( ENETHINK *entk )
{
	short sub;
	sub = (entk->pl_eyei.dir-entk->act->ctrl->turn.vy);
	if(sub>10) return SP_CLE_PEEP_L;
	else if(sub<(-10)) return SP_CLE_PEEP_R; 
	else return  SP_READYGUN ;
}
#endif


static void CheckSightTime(ENETHINK *entk){
	int i,safeaddr;
	HZX_ZON *pl_zone ;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	
	pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);

#if 0
	/*プレイヤ行動不能なら警戒状態キャンセル*/
	if( CheckPl_NoShot())
	{
		at_thk->sight_time = 0;
		return ;
	}
#endif
	/*武器構えてなければ*/
	if(!(GM_PlayerStatus & PLAYER_HOLD)){
		if(at_thk->sight_time >0 )
			at_thk->sight_time--;
		return ;
	}
	/*安地内ではチェック不要*/

	for(i=0;i<HZX_MAX_SAFEZONE_NUM;i++){
		safeaddr = pl_zone->safes[i];
		if(safeaddr == HZX_NO_ZONE) break;
		if((HZX_Zone1(entk->ctrl->addr) == safeaddr)&&
		(!(pl_zone->safe_types[i]&SAFE_LOW))){
			at_thk->sight_time = 0;
			return;
		}
	}
	/* 連続見られ時間*/
	
//	if(entk->count3 < 30){
//	if(entk->count3 < 10){
	if(0){
		at_thk->sight_time = 0;
	}else if( CheckPlayerSight(entk,1,64) ){
		/*順位後ろほど逃げやすい*/
		at_thk->sight_time += (at_thk->dis_rank+1); 
	}else {
		if(at_thk->sight_time >0 )
			at_thk->sight_time--;
	}
}



#define OUTLEN	(1000.0F)
/*安全地帯から飛び出す時は注意深く行動させる*/
static int Checkout(entk)
ENETHINK *entk;
{
	int safelevel = 0,zone1,zone2,
	check,blur_s,dir1,dir2,subdir;
	HZX_ZON		*nzone; 
	FVECTOR		npos,epos;
	float		w,h,outlen;
	FVECTOR		shift ;
	HZX_ZON *pl_zone ;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	if(!(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)) return 0;

	pl_zone = ENE_HZX_GetZone(entk->at_com->at_trg_addr);

	/*見える位置まで来ている*/
	if(entk->pl_eyei.sight>=EYE_INFO_SIGHT_BLURR) return 0;
	/*毎フレームチェックは不要*/
	if(entk->count3&1) return 0;
	/*目標が遠いと飛び出さない*/
//	if(entk->pl_eyei.dis > (entk->sense.eye_s*2) ) return 0;
	if(at_thk->zone_dis > (entk->sense.eye_s) ) return 0;

	blur_s = entk->sense.eye_s+(entk->sense.eye_s/2);

	/*手前が近くにいると飛び出さない*/
	if((entk->at_com->rollout)&&(at_thk->dis_rank==1))
	{
		/*先頭が転がり中で自分が二番手なら接近*/
		check = 2000;
	} else check = 2500;

	if(at_thk->dis_dif < check ) return 0;

	/*  */
	outlen = 1000.0F ;

	zone1 = HZX_Zone1(entk->ctrl->addr);
	zone2 = HZX_Zone2(entk->ctrl->addr);
	/*ゾーンまたぎあるいはゾーン外*/
	if((zone1 != zone2)||(zone1==HZX_NO_ZONE)||(zone2==HZX_NO_ZONE))
		return 0;


	/*次のゾーンが近い？*/
	nzone = ENE_HZX_GetZone( entk->znavi->next_addr ) ;
	epos = entk->ctrl->mov;
	npos.vx = (float)(nzone->x);
	npos.vy = (float)(nzone->y);
	npos.vz = (float)(nzone->z);
	w =(float)(nzone->w)+outlen;
	h =(float)(nzone->h)+outlen;

#if 0

	if(
	((npos.vx-w)< epos.vx)&&(epos.vx <(npos.vx+w) )
	&&
	((npos.vz-h)< epos.vz)&&(epos.vz <(npos.vz+h) )
	){
#else
//	if(Sig_InsideZone( nzone, epos ,outlen )){
	if(Sig_InsideZone( nzone, &epos ,outlen )){
#endif

		/*次のゾーンが近い*/
		/*横方向で近づくのに適正な範囲にいる*/
		shift.vx = npos.vx - (float)entk->ctrl->mov.vx ;
		shift.vz = npos.vz - (float)entk->ctrl->mov.vz ;
		dir1 = _FVecDir2( &shift ) ;
		dir2 = entk->pl_eyei.dir;
		subdir = SIG_CheckDirSub( dir1,dir2);
		if(abs(subdir)< 512  ) return 0;

/*今見えないかチェック*/
		if(ENE_ReadOnlinInfo(entk->ctrl->addr,GM_PlayerAddress)){
			safelevel = 1;
		}else {
/*今安地かチェック*/
#if 0
			for(i=0;i<HZX_MAX_SAFEZONE_NUM;i++){	
				if(pl_zone->safe_types[i] & SAFE_LOW)
					continue;
				safeaddr = pl_zone->safes[i];
				if( zone1 == safeaddr )
				{
					/*今安地にいる*/
					safelevel = 1;
					break;
				}
			}
#endif
		}
		if(safelevel){
			/*次のゾーンなら見えるか？*/
			if(ENE_ReadOnlinInfo(entk->znavi->next_addr,
				GM_PlayerAddress)){
				/*次も見えないので警戒不要*/
				safelevel = 2;
			}else {
#if 0
				for(i=0;i<HZX_MAX_SAFEZONE_NUM;i++){	
					if(pl_zone->safe_types[i] & SAFE_LOW)
						continue;
					safeaddr = pl_zone->safes[i];
					if( HZX_Zone1(entk->znavi->next_addr) == safeaddr )
					{
						/*次も安地だ*/
						safelevel = 2;
						break;
					}
				}
#endif
			}
		}
		if(safelevel==1) {
			/*視点の高さで判断*/
//			npos.vy += 1500;
			npos.vy += 250;

			{
#if 0
	/*射程内で狙われていたら待機*/
				if(
				(entk->pl_eyei.dis < blur_s)
				&&(CheckPlayerSight(entk,0,64)))
				{
					/*角で待機*/
					entk->think2 = TH2_ATTACK ; 
					entk->think3 = TH3_WAIT_SAFE ;
					entk->count3 = 0;
					return 1;
				}
#endif
				if(entk->pl_eyei.dis > entk->sense.eye_s ) return 0;
				/*慎重に移動*/
				entk->think2 = TH2_CHASE ; 
				entk->count3 = 0 ;
				/*基本は横歩き*/
				entk->think3 = TH3_SIDE_MOVE;
				/*飛び出しかたの使い分け*/
				/*盾兵は特種なことしない*/
				if(
				(entk->act->bodyp.type & ENE_TYPE_SHIELD)
				||(entk->act->bodyp.type & ENE_TYPE_HITECH)
				||(at_thk->dis_rank>1)
				)
				{
					return 1;
				}
				/*同じゾーンで待ち構えているようなら*/
				/*特殊なアクション*/
				if(entk->at_com->Pl_StayTime > (AT_THK_RATE*20)){
					/*ROLL_OUT内でステップとの分岐等も行う*/
					entk->think3 = TH3_ROLL_OUT ;
					return 1;
				}
			}
		}
	}
	return 0;
}


#define AT_NOFLAT_ATR (HZX_FLOOR_IK|HZX_FLOOR_STEP)
//HZX_FLOOR_FLAT 水平床
/*通常の床なら０ */
static int CheckFlrAtr(ENETHINK *entk){
	if((entk->ctrl->flr_atrs[ 0 ] & AT_NOFLAT_ATR)
	||(!(entk->ctrl->flr_atrs[ 0 ] & HZX_FLOOR_FLAT)))
	{
		return 1;
	}
	return 0;
}


/*逃げゾーン調べ*/
/*route>0であること*/
/*指定されたゾーンから逃げるための目標ゾーンを得る*/
/*routeで指定されたゾーン数後退*/

static int GetFarZone(int trgaddr ,int route,ENETHINK *entk){
	int rout=0,nowaddr,tmpaddr,i;

	trgaddr = HZX_Zone1(trgaddr);
	if((trgaddr == HZX_Zone1(entk->ctrl->addr) )
	||( trgaddr ==HZX_NO_ZONE)
	||(HZX_Zone1(entk->ctrl->addr)==HZX_NO_ZONE)
	||(GM_GetRIntrptOne( trgaddr ) != NULL)

	){
		return entk->ctrl->addr ;
	}
	nowaddr = tmpaddr = HZX_Zone1(entk->ctrl->addr);
	for(i=0;i<8;i++){
		tmpaddr = HZX_FarZoneNavigate( entk->ctrl->hzx_id,
		HZX_Zone1(nowaddr),HZX_Zone1(trgaddr),&rout);
		if( ENE_ZoneIntrptCheck( tmpaddr ) ) break;
		if(GM_GetRIntrptOne( tmpaddr ) != NULL) break;
		if(tmpaddr == trgaddr) break;
		if(tmpaddr == nowaddr) break;
		nowaddr = tmpaddr;
		if(route <= ENE_GetRouteDis(&entk->znavi->flore_pos,
					NULL,entk->ctrl->addr,
					AddrSet(tmpaddr),route) ){
			break;
		}
	}
	ASSERT(tmpaddr != HZX_NO_ZONE);
	return AddrSet(tmpaddr);
}

/** 射撃制御系関数 **/

/*大きいほど間隔長い*/
#if 0
/*ＡＫ*/
#define AK_INTER 3
/*マカロフ*/
#define MKR_INTER 8
#else 
/*ランダム間隔射撃*/
#define AK_INTER 2
/*マカロフ*/
#define MKR_INTER 4
#endif


/*ＳＰＳ*/
#define SPS_INTER 1


/*敵を盾にしてる判定*/
#define HOSTAGE_DIR (512) /*攻撃可能角度*/
static int CheckHang(ENETHINK *entk){
	int subdir ;
	if( GM_PlayerStatus & PLAYER_ENEMY_HANG) {
#if 0
		/* NPC */
		if(GM_PlayerTarget->capture!=NULL){
			GM_PlayerTarget->capture->flag
		}
#endif
		/*******/
		subdir = SIG_CheckDirSub( entk->pl_eyei.dir,
		GM_PlayerControl->turn.vy);
		if(abs(subdir) > HOSTAGE_DIR  ) {
			SIG_AT_VoiceCall(entk,SD_V_ATKO05,AT_V_UTUNA) ;
			return 1;
		}
	}
	return 0;
}
/*クロスファイアチェック*/
static int CheckX_Fire(ENETHINK *entk){
	AT_THK *at_thk ;
	E_UNIT *e_unit ;
	ENETHINK	*trgentk;
	int i,subdir;

	at_thk = (AT_THK *)entk->character ;

	/*盾兵は動かない*/
#if 0
	if ( entk->act->bodyp.type & ENE_TYPE_SHIELD){
		return 0;
	}
#endif
	if(
	(at_thk->zone_dis >= (entk->at_com->chasedis + 1000) )
	) {
		return 0 ;
	}
	if(at_thk->dis_rank>0){
		e_unit = entk->com->enemys.
		group[entk->g_id]->unit[entk->u_id] ;
		for(i=0;i<e_unit->enemy_num;i++){
			if(i==entk->id) continue ;
			trgentk = e_unit->entk[i];
			if(entk->pl_eyei.dis < trgentk->pl_eyei.dis){
				continue ;
			}
			/*行動不能兵は対象外*/
			if((trgentk->act->status & AT_NO_ACTIVE)
			||(trgentk->act->bodyp.anesthesia == -1)){
				continue ;
			}
			/*盾兵の後ろから出ない*/
//			if ( trgentk->act->bodyp.type & ENE_TYPE_SHIELD){
//				continue ;
//			}
			subdir = SIG_CheckDirSub(entk->pl_eyei.dir,trgentk->pl_eyei.dir) ;
			if( X_FIRE_DIR > abs(subdir) ){
#if 0
				if ( trgentk->act->bodyp.type & ENE_TYPE_SHIELD){
					return 0;
				}
#endif
//printf("X_FIRE_DIR %d\n",abs(subdir) );
				return 1;
			}

		}
	}
	return 0 ;
}

/*捕まえられた人を見た*/
static int AT_CheckHostage(ENETHINK *entk){
	AT_THK *at_thk ;
	at_thk = (AT_THK *)entk->character ;

//	if(entk->at_com->surprised > 0) return 0;

	if(
	(entk->at_com->watch_status & AT_COM_WATCH_HOSTAGE )
	&&(entk->pl_eyei.sight == EYE_INFO_SIGHT_IN)
	){
//printf("GO SURPRISE\n");
		GoHangSurprise(entk);
		return 1;
	}
	return 0 ;
}
static int AT_CheckDanger(ENETHINK *entk){
	int check;
	AT_THK *at_thk ;
	at_thk = (AT_THK *)entk->character ;

	if(at_thk->surprised > 0) {
//printf("at_thk->surprised OVER!!\n");
		return 0;
	}
//	if(CheckPlayerWeapon(entk,256)){
	check = CheckPlayerWeapon(entk,256) ;
	if(check) {
//printf("DANGER!!sed OVER!!\n");
/*かわしてから行動分岐*/
		GoDiscoveryDanger(entk,check) ;
/*一目散に逃げる*/
//		GoAbsEscape(entk);
		return 1;
	}
	return 0 ;
}

static int AT_CheckPlayer(ENETHINK *entk){
	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		return 0 ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		GoDiscoveryPlayer(entk);
		return 1 ;
	}
	return 0 ;
}

#if 0
static int CheckShield2(ENETHINK *entk){

	if(entk->sw.shield == SHL_ST_BREAK) {
		entk->act->bodyp.type &= (~ENE_TYPE_SHIELD) ;
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.weapon) ;
		entk->sw.shield = SHL_ST_NOP;
		return 1 ;
	}
	return 0 ;
}
#endif

/*ビックリすることが起きた*/
static int CheckDeathBed(ENETHINK *entk)
{
	FVECTOR	death,watch ;
	float	sub ;
	if(!(entk->at_com->watch_status & AT_COM_WATCH_DEATHBED )){
		/*死んでない*/
		return 0;
	}

	/*死人の位置*/
	if( entk->at_com->tmptrg.vw > (float) (entk->pl_eyei.dis+2000) ){
		return 0;
	}

	/*距離判定*/	
#ifdef BP_PS2
	death = (FVECTOR) entk->at_com->tmptrg ;
	watch = (FVECTOR) entk->ctrl->mov ;;
#else
	death = entk->at_com->tmptrg ;
	watch = entk->ctrl->mov ;;
#endif
	//_sceVu0SubVector( &sub, &death, &watch ) ;
   sub = death.x - watch.x ; // BP_Math - Fix for stack corruption
	if(sub > entk->sense.eye_s){
		return 0;
	}
	/*ビックリへ移行*/
	GoDeathBed(entk);
	return 1;
}

/*見えないゾーンから飛びだした直後は撃たない*/
static int CheckProtrude(ENETHINK *entk){
	AT_THK	*at_thk ;

	at_thk = (AT_THK *)entk->character ;

/*

*/
	if(
	((at_thk->dis_rank > 0)&&(ENE_AlertGameLevel < AT_SURPORT_LEVEL))
	&&(( entk->at_com->Pl_StayTime <= (AT_THK_RATE*20) )
	&&(at_thk->in_sight <= entk->at_com->shoot_delay ))
	){
		/*撃たない*/
		return 1;
	}
	return 0 ;
}

/*近接攻撃チェック*/
#define BEAT_DIS 1500	/*殴り距離*/
static int CheckBeat(ENETHINK *entk){ 
	if(GM_PlayerStatus &PLAYER_DEAD) return 0;
#if 0
	if(SIG_CheckStealthStatus(entk)){
		return 0 ;
	}
#endif
	if(
	(entk->count3>(AT_THK_RATE))&&
	( entk->pl_eyei.dis <= BEAT_DIS )
	&&(!(GM_PlayerStatus & (PLAYER_BEYOND|PLAYER_INTRUDE|PLAYER_INVINCIBLE)))
	&&(!(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE))
#if 0
	&&(Sig_GetRoute( entk->ctrl->hzx_id,GM_PlayerAddress,
	entk->ctrl->addr)<=HZX_INDIRECT_REACH)
#else
	&&(Sig_GetRoute( entk->ctrl->hzx_id,
	*entk->pl_eyei.addr ,
	entk->ctrl->addr)<=HZX_INDIRECT_REACH)
#endif
	) {
		entk->think2 = TH2_ATTACK;
		entk->think3 = TH3_ATTACK_BEAT ; 
		entk->count3 = 0 ;
		return 1;
	}
	return 0 ;
}



/*銃ゆらし付きＰＡＤ射撃*/
/*静止しての攻撃なので狙いが正確*/
/*ＡＴ専用ＰＡＤ押しタイプ*/
static void StillShoot( ENETHINK	*entk ,int mode)
{
	int rnd_rate ;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

#ifdef DEBUG_NO_SHOT
	return ;
#endif

	if(entk->at_com->watch_status & AT_COM_WATCH_NO_SHOT){
		return ;
	}
	if(GM_PlayerStatus &PLAYER_DEAD){
		return ;
	}
	if(
	( mode & AT_SHT_BLIND)
	||(GM_PlayerStatus & PLAYER_NARROW)
//	||(GM_CheckPlayerStatusEX(0,PLAYER2_NARROW_HIDDEN))

	){
		/*見えなくても撃つ*/
		/*せまい所に追い込んだら見えてなくてもＯＫ*/
		if(AT_SURPORT_LEVEL > ENE_AlertGameLevel){
			/*低レベル時は威嚇射撃は先頭のみ*/
			if(at_thk->dis_rank > 0) {
				return ;
			}
		}
		if(
		( entk->act->status & AT_NO_SHOOT )
		||( at_thk->dis_rank > entk->at_com->attack_num)
		){
			return;
		}
	}else {
		if(
		((entk->pl_eyei.sight <= EYE_INFO_SIGHT_OUT_HZD ) 
		&&(!(entk->at_com->watch_status & AT_COM_WATCH_BEYOND))
		)
		||(at_thk->in_sight <= entk->at_com->shoot_delay)
		||( entk->act->status & AT_NO_SHOOT )
		||( at_thk->dis_rank > entk->at_com->attack_num)
		){
			return;
		}
	}
	if(CheckPl_NoShot()) {
		return;
	}
	/*敵を盾にしてる*/
	if(CheckHang(entk)) {
		return ;
	}
	/*クロスファイア*/
	if(CheckX_Fire(entk)) {
		return ;
	}
	/*見えないゾーンから飛びだした直後は撃たない*/
	if(CheckProtrude(entk)){
		return ;
	}
	/*残弾チェックしてリロードへ*/
	if(!(mode & AT_SHT_NO_RELOAD)) {
		if ( entk->bullet >= entk->max_bullet ) {
		/*リロード禁止なら撃たずにリターン*/
			/* その場リロード*/
#if 1
			GoReload(entk);
			entk->count3 = 0 ;
#endif
			return ;
		}
	}
	/*乱数で発砲*/
	if(entk->act->bodyp.type & ENE_TYPE_SHIELD){
		/*盾兵 マカロフ*/
		rnd_rate = MKR_INTER;
	}else if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		/*ショットガン ＳＰＳ*/
//		entk->bullet = 0;
		rnd_rate = SPS_INTER;
	}else {
		/*ＡＫ*/
		rnd_rate = AK_INTER;
	}
	if ( !(entk->at_com->alert_time % rnd_rate) ) {
		if(!(mode & AT_SHT_NO_RAND)){
			if ( (irnd()>>8)%(4+at_thk->dis_rank)  ) {
				return;
			}
		}
		if(at_thk->at_status & AT_ST_SQUAT){
			entk->act->pad = SP_SQUATSHOOTGUN ;
		}else {
			if(entk->act->bodyp.type & ENE_TYPE_SHIELD){
				if(at_thk->dis_rank ==0 ){
					entk->act->pad = SP_SHOOTGUN ;

				}else {
					entk->act->pad = SP_SHL_SIDE_SHOOTGUN ;
				}
			}else {
				if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
					entk->act->pad = SP_SHOOTGUN ;
				}else {
					entk->act->pad = SP_SHOOTGUN ;
				}
			}
		}
//		entk->bullet++;
	}
}


/****銃揺らさず直接bulletＣＡＬＬ
EYE_INFO_SIGHT_IN		2
EYE_INFO_SIGHT_BLURR	1
EYE_INFO_SIGHT_OUT		0
*************/
/*移動しながらなので狙いがぶれる*/
static void RandShoot( ENETHINK	*entk )
{
	if(entk->at_com->watch_status & AT_COM_WATCH_NO_SHOT){
		return ;
	}
	if(entk->ctrl->map != GM_PlayerMap ){
		return ;
	}
	/*ショットガンは静止以外で発砲しない*/
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		return ;
	}
	/*見えない時は発砲しない*/
	/*移動しながらの攻撃なのではっきりみえて無くても撃つ*/
	/*見えてなくても撃つ*/
	if(
	( entk->act->status & AT_NO_SHOOT )
	){
		return;
	}
	/*プレイヤが*/
	if(CheckPl_NoShot()){
		return;
	}
	/*敵を盾にしてる*/
	if(CheckHang(entk)){
		return ;
	}
	/*見えないゾーンから飛びだした直後は撃たない*/
//	if(CheckProtrude(entk)) return ;
	/*クロスファイア*/
	if(CheckX_Fire(entk)){
		return ;
	}
	SIG_RandShoot( entk );
}
static void RandShoot_TNG( ENETHINK	*entk )
{
	if(entk->at_com->watch_status & AT_COM_WATCH_NO_SHOT){
		return ;
	}
	if(entk->ctrl->map != GM_PlayerMap ){
		return ;
	}
	/*ショットガンは静止以外で発砲しない*/
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		return ;
	}
	/*見えない時は発砲しない*/
	/*移動しながらの攻撃なのではっきりみえて無くても撃つ*/
	/*見えてなくても撃つ*/
	if(
	( entk->act->status & AT_NO_SHOOT )
	){
		return;
	}
	/*プレイヤが*/
	if(CheckPl_NoShot()){
		return;
	}
	/*敵を盾にしてる*/
	if(CheckHang(entk)){
		return ;
	}
	/*見えないゾーンから飛びだした直後は撃たない*/
//	if(CheckProtrude(entk)) return ;
	/*クロスファイア*/
	if(CheckX_Fire(entk)){
		return ;
	}
	SIG_RandShoot_TNG( entk );
}
/*棚を挟んで移動しながら狙いがぶれる*/
static void SymmRandShoot( ENETHINK	*entk )
{
	int subdir;
	FVECTOR	trgpos ;
	float	len;
	AT_THK	*at_thk ;

	at_thk = (AT_THK *)entk->character ;
#ifdef DEBUG_NO_SHOT
	return ;
#endif

	if(entk->at_com->watch_status & AT_COM_WATCH_NO_SHOT)return ;

	/*ショットガンは静止以外で発砲しない*/
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
//		entk->bullet = 0;
		return ;
	}
	/*見えない時は発砲しない*/
	/*移動しながらの攻撃なのではっきりみえて無くても撃つ*/
	/*見えてなくても撃つ*/
	if(
	( entk->act->status & AT_NO_SHOOT )
	){
		return;
	}
	/*プレイヤが*/
	if(CheckPl_NoShot()) return;
	/*敵を盾にしてる*/
	if(CheckHang(entk)) return ;
	/*見えないゾーンから飛びだした直後は撃たない*/
//	if(CheckProtrude(entk)) return ;
	/*クロスファイア*/
//	if(CheckX_Fire(entk)) return ;

	subdir = SIG_CheckDirSub(entk->sense.facedir,entk->pl_eyei.dir);
	if((subdir < -90 )||(90 < subdir )) {
		return;
	}
	/*狙いを揺らす*/

#if 0
	trgpos = entk->act->aim_pos ;
#else
//	GV_MatToVec(&BODYWORLD( GM_Pentk->act->body, HUMAN21_KUBI ),&trgpos);
	trgpos = GM_PlayerFindPos ;
#endif

	len = GV_VecLen3F2( &trgpos, &entk->ctrl->mov );
	/*リロード無しなので毎回初期化*/
	entk->bullet = 0;
	if(len > 2000.0F){
		/*乱数で発砲*/
		/*距離の10分の１*/
		SIG_SetRandFvec( &trgpos,(len/10.0F ));
		SIG_BasicShoot( entk ,&trgpos ,0);
	}else {
		/*近すぎる時は銃身マトリクス*/
		//ENE_BULLET_NOATTACK
		//ENE_BULLET_NOLINE
		SIG_BasicShoot( entk ,NULL ,0);
	}
}


static void ThreatShot( ENETHINK *entk ,FVECTOR *trgpos)
{
	AT_THK	*at_thk ;
	float	len;

	at_thk = (AT_THK *)entk->character ;

#ifdef DEBUG_NO_SHOT
	return ;
#endif

	if(entk->at_com->watch_status & AT_COM_WATCH_NO_SHOT)return ;

	/*味方が盾*/
	if(CheckHang(entk)) return ;
	
	if(CheckPl_NoShot()) return;

	/*クロスファイア*/
	if(CheckX_Fire(entk)) return ;

	if(!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_CB_BOX) )){
		if(AT_SURPORT_LEVEL > ENE_AlertGameLevel){
			/*低レベル時は威嚇射撃は先頭のみ*/
			if(at_thk->dis_rank > 0) return ;
		}
	}
	len = GV_VecLen3F2( trgpos, &entk->ctrl->mov );
	if(len > 2000.0F){
		SIG_BasicShoot( entk ,trgpos ,0);
	}else {
		/*近すぎる時は銃身マトリクス*/
		SIG_BasicShoot( entk ,NULL ,0);
	}
}

/*重なり防止*/
static int	CheckNearAT(entk,trgentk)
ENETHINK *entk;
ENETHINK *trgentk;
{
	FVECTOR	nearsub ;
	short	neardir ;

	_sceVu0SubVector( &nearsub, &trgentk->ctrl->mov,&entk->ctrl->mov ) ;
	if( 2000.0F > GV_VecLen3F( &nearsub )){
		neardir = GV_VecDir2( &nearsub ) ;
		if( 512 > abs(SIG_CheckDirSub(entk->trgpoint.dir,neardir))){
			return -1 ;
		}
	}
	return 0 ;
}
/*重なりチェックから待機へ*/
static int CheckNearAT2Wait(entk)
ENETHINK	*entk;
{
	AT_THK		*at_thk ;
	ENETHINK	*nearentk;

	at_thk = (AT_THK *)entk->character ;

	if(at_thk->dis_rank > 0) {
		nearentk = GetAtRanking(entk,(at_thk->dis_rank-1)) ;
		if(nearentk != NULL ){
			if(CheckNearAT(entk,nearentk)){
				return -1 ;
			}
		}
	}
	if(at_thk->dis_rank < 
		(entk->com->enemys.group[entk->g_id]->
		unit[entk->u_id]->enemy_num - 1)
	) {
		nearentk = GetAtRanking(entk,(at_thk->dis_rank+1)) ;
		if(nearentk != NULL ){
			if(CheckNearAT(entk,nearentk)){
				return -1 ;
			}
		}
	}
	return 0 ;
}

/*entk使わない版*/
#if 0
static int GetFarZone2(int trgaddr,int nowaddr ,int reach){
	int rout=0,tmpaddr,i;
	int hzx_id ;
	
	hzx_id = GV_GetBit( HZX_ZoneMapNo(trgaddr) );

	trgaddr = HZX_Zone1(trgaddr);
	if((trgaddr == HZX_Zone1(nowaddr) )
	||( trgaddr ==HZX_NO_ZONE)
	||(HZX_Zone1(nowaddr)==HZX_NO_ZONE)
	||(GM_GetRIntrptOne( trgaddr ) != NULL)

	){
		return nowaddr ;
	}
	nowaddr = tmpaddr = HZX_Zone1(nowaddr);
	for(i=0;i<reach;i++){
		tmpaddr = HZX_FarZoneNavigate( hzx_id,
		HZX_Zone1(nowaddr),HZX_Zone1(trgaddr),&rout);
		if( ENE_ZoneIntrptCheck( tmpaddr ) ) break;
		if(GM_GetRIntrptOne( tmpaddr ) != NULL) break;
		if(tmpaddr == trgaddr) break;
		if(tmpaddr == nowaddr) break;
		nowaddr = tmpaddr;
	}
	ASSERT(tmpaddr != HZX_NO_ZONE);
	return AddrSet(tmpaddr);
}
#endif
static void SetBerserkTime(ENETHINK *entk,int time){
	if(entk->at_com->berserk < time ){
		entk->at_com->berserk = time ;
	}
}

