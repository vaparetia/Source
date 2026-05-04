/*
    kmact.c
    カモメ動作処理
    2001/05/02 Masafumi Okuta
    $Id: kmact.c,v 1.1.1.3 2002/11/19 11:48:02 Yoshizawa1 Exp $
*/
static void KMM_ActStandStill( KAMOME* kamome, int time);
static void KMM_ActFlying( KAMOME* kamome, int time);
static void KMM_ActHovering( KAMOME* kamome, int time);
static void KMM_ActLanding( KAMOME* kamome, int time);
static void KMM_ActFlipflop( KAMOME* kamome, int time);

#define KMM_FEATHER_PPK		(32)
#define KMM_FEATHER_SHOOT	(32)	
#define KMM_FEATHER_SLEEP	(32)
#define KMM_FEATHER_FEINT	(16)
#define KMM_FEATHER_SPRAY	(4)

// 壁避け
static int Get_EvadeWallDir( KAMOME* kamome)
{
    int 	n_touches;
    int		nRotY;
    int		nDir;
    int		nWallDir[ 2 ];
    FVECTOR	vecs[ 2 ];
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;

    n_touches = HZX_NearHazardCheck( kamome->hzx_id, &kamome->mov, 800, HZX_CHK_ALL, 0, 500 ) ;

    if ( n_touches > 0 ){
	HZX_GetNearVector( vecs );

	// 壁の方向を取得
	nWallDir[0] = GV_VecDir2( &vecs[ 0 ] );
	nWallDir[1] = GV_VecDir2( &vecs[ 1 ] );

	// 壁の方向とファットマンの方向の差が小さい方を取得
	if ( GV_DiffDirAbs( nWallDir[0], kamome->rot.vy ) < GV_DiffDirAbs( nWallDir[1], kamome->rot.vy )){
	    nDir = nWallDir[0];
	}else{
	    nDir = nWallDir[1];
	}

	// 壁接触したら頭がめりこまないように壁の方向に垂直な方向に向ける
	if ( GV_DiffDirS( nDir, kamome->rot.vy ) >= 0) {
	    nRotY = (nDir + 1024) & 4095;
	}else{
	    nRotY = (nDir - 1024) & 4095;
	}
    }else{
	nRotY = kamome->rot.vy;
    }

    return (nRotY);
}

/* 麻酔が刺さる 注！モデルのcv2は必要なし */
static void KMM_SetNeedl( KAMOME* kamome, FVECTOR *pos, int model )
{
    extern int MakeAttachment4_called(int ,FVECTOR *,SVECTOR *,OBJECT *,int,FVECTOR *,int,int );
    extern void VertexSearch(FVECTOR *vans,FVECTOR *nans,DG_OBJS *objs,int objnum,FVECTOR *target) ;

    int         nUnit = 0;
    SVECTOR	rot;
    FVECTOR	vans, nans;

    VertexSearch( &vans, &nans, kamome->body.objs, nUnit, pos );
    _FVecToRotXY( &nans, &rot ) ;
    rot.vx -= 1024 ; /* "m92_bul2"モデル固有補正 */
    
    MakeAttachment4_called( model, NULL, &rot, &kamome->body, nUnit, &vans, 512, 4 );
}

// 麻酔外す
static void KMM_ClearNeedl( OBJECT *body )
{
    SearchAndFallAttachment_called( body, 0, 0, 0 ) ;
}

// ダメージコールバック
static void KMM_TargetCallback( TARGET* off, TARGET* def, void* pvKamome)
{
    int			weapon;
    KAMOME* kamome = (KAMOME*)pvKamome;
    extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );

    weapon = def->weapon_type;

    if ( kamome->action.status & KMM_STATUS_DEAD ||
	 kamome->action.status & KMM_STATUS_TRGOFF ){ // あたりなしor死亡
	return;
    }

    _sceVu0CopyVector( &kamome->vecForce, &off->power->force);	// 力積を取得

    if ( TARGET_POWER & def->damaged ) {
	if ( weapon & WP_PUNCHALL ) { // 打撃系
	    GV_SetActorChild( kamome, NewBlood( &kamome->matOrg, &def->hit, &kamome->vecForce, 0, 0 ) ) ;
	}else if ( (weapon & WP_M92) || (weapon & WP_PSG1T) ){ // 麻酔系
	    KMM_SetNeedl( kamome, &def->hit, MASUIDAN_MODEL); // 麻酔弾をつける
	}else if ( weapon & WP_COLDSPRAY ){ // C4冷却スプレー
	}else if ( (weapon & WP_STUNGRENADE) || (weapon & WP_STUNFAR)) {  // スタン
	}else if ( weapon & WP_STAMP || weapon & WP_WEAPONCORE ){
	}else if ( weapon != 0 ){
	    GV_SetActorChild( kamome, NewBlood( &kamome->matOrg, &def->hit, &kamome->vecForce, 0, 0 ) ) ;
	}
    }
}

// 体の傾き計算
void	KMM_Incline( KAMOME* kamome )
{
    int		incline ;

    incline = GV_DiffDirS( kamome->turn.vy, kamome->rot.vy ) * 4;
    if ( incline > KMM_INCLINE_ANG ) incline = KMM_INCLINE_ANG;
    else if ( incline < -KMM_INCLINE_ANG ) incline = -KMM_INCLINE_ANG;
    kamome->turn.vz = incline;    
}

// モーションコントロール取得
static MOTION_CONTROL* KMM_GetMotCtrl( KAMOME* kamome)
{
    MEMMOT_DATA*	pmmtData;
    pmmtData = kamome->mmt_ctrl.pmmtData;

    if ( kamome->pvManager != NULL && pmmtData != NULL){
	return pmmtData->body_mtbuff[kamome->mmt_ctrl.current_mot].m_ctrl;
    }

    return kamome->body.m_ctrl;
}
// モーション再生割合
static float KMM_GetMotPlayRate( KAMOME* kamome)
{
    MEMMOT_DATA*	pmmtData;
    MOTION_CONTROL* m_ctrl;

    pmmtData = kamome->mmt_ctrl.pmmtData;
    m_ctrl = KMM_GetMotCtrl(kamome);

    if ( kamome->pvManager != NULL){
	if ( pmmtData->m_len[kamome->mmt_ctrl.current_mot] != 0 ){
	    return ((float)kamome->mmt_ctrl.m_time / (float)pmmtData->m_len[kamome->mmt_ctrl.current_mot]);
	}else{
	    return 0.f;
	}
    }
    return ((float)m_ctrl->mt3_ctrl->play_time / (float)m_ctrl->mt3_ctrl->motion_total_time);
}

// モーション終了チェック
static int KMM_CheckObject_IsEnd( MEMMOT_CTRL* mmt_ctrl, int nLayer)
{
    return MEMMOT_CheckObject_IsEnd( mmt_ctrl, nLayer );
}

// モーション再生
#if 0 //XBOXではおかしくなるので(u_long64)、変更しました。yano2002.02.28
static void KMM_SetActMotionEx( kamome, n_layer, mot, start, mask, interp, speed )
KAMOME*	kamome ;
int	n_layer ;	/* レイヤー番号 */
int	mot ;		/* モーション番号 */
int	start ;		/* 開始時間 */
u_long64	mask ;		/* マスクビット */
int	mask ;		/* マスクビット */
int	interp ;	/* 補完時間 */
float	speed ;		/* 再生スピード */
#endif
static void KMM_SetActMotionEx( KAMOME *kamome, int n_layer, int mot, int start, u_long64 mask, int interp, float speed )
{
    int		nOldMot;
    KMMACT* act;

    act = &kamome->action;
    nOldMot = act->current_mot;
    if ( n_layer == 0 ) act->current_mot = mot;

    kamome->vecAdj = DG_ZeroSVector;
    
    MEMMOT_SetMotion( &kamome->mmt_ctrl, mot, interp);
    MEMMOT_SetMotionSpeed( &kamome->mmt_ctrl, speed);

    if ( nOldMot >= KMM_MOT_WALK_START && nOldMot <= KMM_MOT_SURPRISE_BACK ){
	kamome->fMotStartHeight = kamome->vecLand.vy - KAMOME_Y_OFFSET;
    }else{
	kamome->fMotStartHeight = kamome->vecMove.vy - KAMOME_Y_OFFSET;
    }
}
// モーションデフォルト再生
static void KMM_SetActMotion( kamome, n_layer, mot )
KAMOME*	kamome ;
int	n_layer ;	/* レイヤー番号 */
int	mot ;		/* モーション番号 */
{
    kamome->vecDistance = DG_ZeroVector;
    KMM_SetActMotionEx( kamome, n_layer, mot, 0, 0xffff, ACT_INTERP_DEF, 1.0f );
}

// システム版止まり
static void KMM_ActStandStill( KAMOME* kamome, int time)
{
    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE );
    }
    // LOD
    KMM_SetLodMode( kamome, KMM_LOD_REST);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;
    // パッドチェック
    if ( kamome->CheckPad( kamome ) ) return;
}
#if 0
// システム版止まり
static void KMM_ActFlyStill( KAMOME* kamome, int time)
{
    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING );
    }
    // LOD
    KMM_SetLodMode( kamome, KMM_LOD_REST);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;
    // パッドチェック
    if ( kamome->CheckPad( kamome ) ) return;
}
#endif
// 羽休め
static void KMM_ActIdle( KAMOME* kamome, int time)
{
    KMMACT* act;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    act = &kamome->action;
    
    // 初期設定
    if ( time == 0 ){
	kamome->nIdleTime = KMM_GetDemoRandom( kmmng, 120, 180);
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE );

	kamome->vecAdj.vx = KMM_GetDemoRandom( kmmng, -512, 512);
	kamome->vecAdj.vy = KMM_GetDemoRandom( kmmng, -512, 512);
	kamome->vecAdj.vz = 0;
    }

    // LOD
    KMM_SetLodMode( kamome, KMM_LOD_REST);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    if ( act->current_mot == KMM_MOT_IDLE ){ 
//	kamome->mmt_ctrl.adj_x = kamome->vecAdj.vx;
//	kamome->mmt_ctrl.adj_y = kamome->vecAdj.vy;

	// モーション再生終了
	if ( kamome->nIdleTime-- <= 0 ){
	    kamome->nIdleTime = KMM_GetDemoRandom( kmmng, 120, 180);
	    if ( BP_PS2_rand() % 100 < 20)      KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_SURPRISE ); // ばたばた
	    else if ( BP_PS2_rand() % 100 < 30) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_PICK_A );  // つつき１
	    else if ( BP_PS2_rand() % 100 < 40) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_PICK_B );  // つつき２
	    else       	                 KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE );    // 待機
	}
    }else{
	// モーション再生終了
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    kamome->nIdleTime = KMM_GetDemoRandom( kmmng, 120, 180);
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE ); // 飛行
	}
    }

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}
// 羽休め(地面)
static void KMM_ActIdleGround( KAMOME* kamome, int time)
{
    KMMACT* act;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    act = &kamome->action;
    
    // 初期設定
    if ( time == 0 ){
	kamome->nIdleTime = KMM_GetDemoRandom( kmmng, 120, 180);
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE );
	kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 4 );
    }

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    if ( act->current_mot == KMM_MOT_IDLE ){ 
	// モーション再生終了
	if ( kamome->nIdleTime-- <= 0 ){
	    int nRandSeed = BP_PS2_rand() % 100;
	    kamome->nIdleTime = KMM_GetDemoRandom( kmmng, 120, 180);
	    if ( nRandSeed < 20){
		KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_SURPRISE ); // ばたばた
	    }else if ( nRandSeed < 30){
		KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_PICK_A );  // つつき１
	    }else if ( nRandSeed < 40){
		KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_PICK_B );  // つつき２
	    }else if ( nRandSeed < 55){
		if ( _MAO_FVec2Len2( &kamome->vecLand, &kamome->mov ) < 1000.f){ // 元いた点から離れ過ぎない
		    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_WALK_START );  // 歩き
		}else{
		    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE );    // 待機
		}
	    }else{
		KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE );    // 待機
	    }
	}
    }else if ( act->current_mot == KMM_MOT_WALK_START ){ 
	// モーション再生終了
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    if ( BP_PS2_rand() % 100 < 20) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_WALK_A );  // 歩き
	    else		    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_WALK_B );  // 歩き
	    kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 4 );
	}
    }else if ( act->current_mot == KMM_MOT_WALK_A || act->current_mot == KMM_MOT_WALK_B ){ 
	// モーション再生終了
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    kamome->ucWing--;
	    if ( kamome->ucWing <= 0 ){
		KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_WALK_END );  // 歩き
	    }
	}
    }else{
	// モーション再生終了
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    kamome->nIdleTime = KMM_GetDemoRandom( kmmng, 120, 180);
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE ); // 飛行
	}
    }

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }

}
// 飛行中
static void KMM_ActFlying( KAMOME* kamome, int time)
{
    KMMACT* 	act;
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    act = &kamome->action;

    if ( time == 0 ){
	if ( act->current_mot == KMM_MOT_FLYAWAY){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING ); // 飛行
	}else if ( /* act->current_mot == KMM_MOT_ATTACK_A ||
		   act->current_mot == KMM_MOT_ATTACK_B || */
		   act->current_mot == KMM_MOT_ATTACK_C ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING_START ); // 飛行
	}else if ( act->current_mot != KMM_MOT_FLYING && 
		   act->current_mot != KMM_MOT_HOVER){ // 飛行中以外
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYAWAY ); // 飛び立ち
	}
	kamome->ucWing = KMM_DemoRand( kmmng, 3 );
    }

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // 傾き計算
    KMM_Incline( kamome);

    if ( act->current_mot == KMM_MOT_FLYAWAY){ // 飛び立ち

	// モーション再生終了
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING_START ); // 飛行開始
	    kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 3 );
	}

    }else if ( act->current_mot == KMM_MOT_FLYING_START ){ // 飛行開始
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING ); // 飛行
	}
    }else if ( act->current_mot == KMM_MOT_FLYING ){ // 飛行
	if ( !kmmng->bPigeon && 
	     KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) && kamome->mmt_ctrl.nLoop >= kamome->ucWing ){
	    if ( abs( GV_DiffDirAbs( kamome->turn.vz, kamome->rot.vz ) ) > 64 ){
		KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING_END ); // はばたき終了
	    }
	}
    }else if ( act->current_mot == KMM_MOT_FLYING_END ){ // 飛行開始
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_HOVER ); // 飛行
	    kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 4 );
	}
    }else if ( act->current_mot == KMM_MOT_FLYING02 ){ // 飛行
	if ( !kmmng->bPigeon &&  
	     KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) && kamome->mmt_ctrl.nLoop >= kamome->ucWing ){
	    if ( abs( GV_DiffDirAbs( kamome->turn.vz, kamome->rot.vz ) ) > 64 ){
		if ( KMM_DemoRand( kmmng, 2) == 0 )KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_HOVER ); // 飛行
		else				   KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_HOVER02 ); // 飛行
		kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 3 );
	    }
	}
    }else if ( act->current_mot == KMM_MOT_FLYING03_START ){ // 飛行３開始
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING03 ); // 飛行３
	}
    }else if ( act->current_mot == KMM_MOT_FLYING03 ){ // 飛行３
	if ( !kmmng->bPigeon && 
	     KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) && kamome->mmt_ctrl.nLoop >= kamome->ucWing ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING03_END ); // 飛行３終了
	}
    }else if ( act->current_mot == KMM_MOT_FLYING03_END ){ // 飛行３終了
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    if ( abs( GV_DiffDirAbs( kamome->turn.vz, kamome->rot.vz ) ) > 64 ){
		if ( KMM_DemoRand( kmmng, 2) == 0 )KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_HOVER ); // 飛行
		else				   KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_HOVER02 ); // 飛行
		kamome->ucWing = KMM_GetDemoRandom( kmmng, 1, 3 );
	    }else{
		if ( KMM_DemoRand( kmmng, 2) == 0 )KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING_START ); // 飛行
		else				   KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING03_START ); // 飛行
		kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 3 );
	    }
	}
    }else if ( act->current_mot == KMM_MOT_HOVER ){ // 滑空
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) && kamome->mmt_ctrl.nLoop >= kamome->ucWing){
	    if ( abs(kamome->turn.vz) < 10){
		if ( KMM_DemoRand( kmmng, 2) == 0 )KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING_START ); // 飛行
		else				   KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING03_START ); // 飛行
		kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 3 );
	    }else{
		if ( KMM_DemoRand( kmmng, 2) == 0 )KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING_START ); // 飛行
		else				   KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING03_START ); // 飛行
		kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 3 );
	    }
	}
        // LOD
	KMM_SetLodMode( kamome, KMM_LOD_FLYING);
    }else if ( act->current_mot == KMM_MOT_HOVER02 ){ // 滑空２
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) && kamome->mmt_ctrl.nLoop >= kamome->ucWing){
	    if ( abs(kamome->turn.vz) < 10){
		if ( KMM_DemoRand( kmmng, 100) < 80 ) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING_START ); // 飛行
		else				      KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING03_START ); // 飛行
		kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 3 );
	    }
	}
    }else{
	if ( kamome->mmt_ctrl.nLoop >= 1){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING ); // 飛行
	}
	}

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
}

}
// 滑空
static void KMM_ActHovering( KAMOME* kamome, int time)
{
    KMMACT* act;

    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_HOVER );
    }

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}
// 着地
static void KMM_ActLanding( KAMOME* kamome, int time)
{
    KMMACT* act;

    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_LANDING );
    }


    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // パッドチェック
    if ( kamome->CheckPad( kamome ) ) return;

    // モーション再生終了
    if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	act->act_end = 1;
	KMM_SetActMode( kamome, KMM_ActStandStill);
    }

    // 傾き計算
    KMM_Incline( kamome);
}

// ばたつき
static void KMM_ActFlipflop( KAMOME* kamome, int time)
{
    KAMOME_MNG* kmmng;
    KMMACT* act;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
	if ( KMM_DemoRand( kmmng, 2 ) == 0) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING ); 
	else				    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING03 ); 
    }

    if ( act->current_mot == KMM_MOT_FLYING ){ // 飛行
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) && kamome->mmt_ctrl.nLoop >= kamome->ucWing ){
	    kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 4);
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_HOVER ); 
	}
    }else if ( act->current_mot == KMM_MOT_FLYING03 ){ // 飛行３
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) && kamome->mmt_ctrl.nLoop >= kamome->ucWing ){
	    kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 4);
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_HOVER ); 
	}
    }else if ( act->current_mot == KMM_MOT_HOVER ){ // 滑空
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) && kamome->mmt_ctrl.nLoop >= kamome->ucWing ){
	    kamome->ucWing = KMM_GetDemoRandom( kmmng, 2, 4);
	    if ( KMM_DemoRand( kmmng, 2 ) == 0) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING ); 
	    else				KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING03 ); 
	}
    }
    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}
// 騒ぐ
static void KMM_ActCaution( KAMOME* kamome, int time)
{
    KAMOME_MNG* kmmng;
    KMMACT* act;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE,  KMM_MOT_SURPRISE_BACK ); 
    }

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }

    // モーション再生終了
    if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	act->act_end = 1;
	KMM_SetActMode( kamome, KMM_ActFlying);
    }
}
// 飛行中
static void KMM_ActTakeoff( KAMOME* kamome, int time)
{
    KMMACT* act;

    act = &kamome->action;

    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYAWAY_QUICK ); // 飛び立ち
    }

    // 状態設定
    KMM_ActStatus( act, KMM_STATUS_TRGOFF);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;
    // パッドチェック
    if ( kamome->CheckPad( kamome ) ) return;

    // モーション再生終了
    if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	act->act_end = 1;
	act->pad = PAD_FLYING;
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FLYING ); // 飛行
	KMM_SetActMode( kamome, KMM_ActFlying);
    }
    // 傾き計算
    if ( (kamome->action.current_mot == KMM_MOT_FLYAWAY_QUICK && kamome->mmt_ctrl.m_time < COUNT_VMODE(30) ) ||
	 (kamome->action.current_mot == KMM_MOT_FLYAWAY && kamome->mmt_ctrl.m_time < COUNT_VMODE(69) ) ){

    }else{
	KMM_Incline( kamome);
    }
}
// つつき
static void KMM_ActPokeAttack( KAMOME* kamome, int time)
{
    KMMACT* act;

    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
#if 0
	KMM_SetActMotionEx( kamome, LAYER_BASE, KMM_MOT_FLYING, 0, 0xffff, 
			    ACT_INTERP_DEF, (float)TIME_BASE * 1.f );
#endif
	//	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_ATTACK_A );
	//	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_ATTACK_B );
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_ATTACK_C );
    }

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // キャンセルチェック
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}
// ダメージ
static void KMM_ActDamage( KAMOME* kamome, int time)
{
    KMMACT* act;

    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, act->set_mot );
    }

    // 状態設定
    KMM_ActStatus( act, KMM_STATUS_DAMAGE | KMM_STATUS_TRGOFF);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;
    // パッドチェック
    if ( kamome->CheckPad( kamome ) ) return;

    // 終了	
    if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	act->act_end = 1;
	KMM_SetActMode( kamome, KMM_ActStandStill);
    }
}
// 落下処理
static void KMM_ActFall( KAMOME* kamome, int time)
{
    KMMACT* act;

    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
	if ( kamome->vecForce.vy >= 0){ // 下からくらった
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DAM1_START );
	}else{
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DAM2_START );
	}
    }

    if ( act->current_mot == KMM_MOT_DAM1_START ){
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FALL );
	}
    }
    if ( act->current_mot == KMM_MOT_DAM2_START ){
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_FALL2 );
	}
    }

    // 状態設定
    KMM_ActStatus( act, KMM_STATUS_TRGOFF);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}
// 地面睡眠処理
static void KMM_ActGroundSleep( KAMOME* kamome, int time)
{
    KMMACT* act;
    KAMOME_MNG* kmmng;

    act = &kamome->action;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 初期設定
    if ( time == 0 ){
	if ( act->current_mot == KMM_MOT_FALL ) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DAM1_END );
	else					KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DAM2_END );
    }

    KMM_ActStatus( act, KMM_STATUS_GROUND);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ){
	return;
    }

    if ( act->current_mot == KMM_MOT_DAM1_END ){
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_SLEEP1 );
	}
    }
    if ( act->current_mot == KMM_MOT_DAM2_END ){
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_SLEEP2 );
	}
    }

    // 状態設定
//    KMM_ActStatus( act, KMM_STATUS_SLEEP);

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}
// 立ち睡眠処理
static void KMM_ActStandSleep( KAMOME* kamome, int time)
{
    KMMACT* act;

    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_IDLE );
    }

    // 状態設定
 //    KMM_ActStatus( act, KMM_STATUS_SLEEP);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}
// 気絶処理
static void KMM_ActFeint( KAMOME* kamome, int time)
{
    KMMACT* act;
    KAMOME_MNG* kmmng;

    act = &kamome->action;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DAM1_START );
    }

    KMM_ActStatus( act, KMM_STATUS_GROUND);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    if ( act->current_mot == KMM_MOT_DAM1_START ){
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_SLEEP1 );
	}
    }

    // 状態設定
//    KMM_ActStatus( act, KMM_STATUS_SLEEP);

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}
// 起きる処理
static void KMM_ActWake( KAMOME* kamome, int time)
{
    KMMACT* act;
    KAMOME_MNG* kmmng;

    act = &kamome->action;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 初期設定
    if ( time == 0 ){
	if ( act->current_mot == KMM_MOT_SLEEP1 ) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_WAKE_HIGH );
	else					  KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_WAKE_LOW );
	
	// 起きる時の高さを設定
	kamome->vecFallSpd.vy = kamome->vecMove.vy + (KAMOME_Y_OFFSET - KMM_FLOOR_OFFSET);
    }

    // 状態設定
    KMM_ActStatus( act, KMM_STATUS_TRGOFF);

    // 高さ修正
    kamome->vecMove.vy = kamome->vecMove.vy * 0.95f + kamome->vecFallSpd.vy * 0.05f;

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // パッドチェック
    if ( kamome->CheckPad( kamome ) ) return;

    if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	act->act_end = 1;
	KMM_SetActMode( kamome, KMM_ActStandStill);
//kamome->fMotStartHeight = kamome->vecFallSpd.vy;
	KMM_ClearNeedl( &kamome->body ); // 麻酔弾を抜く
    }
}
// 立ち状態で起きる処理
static void KMM_ActWakeStand( KAMOME* kamome, int time)
{
    KMMACT* act;
    KAMOME_MNG* kmmng;

    act = &kamome->action;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 初期設定
    if ( time == 0 ){
	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_WAKE_HIGH );
    }

    // 状態設定
    KMM_ActStatus( act, KMM_STATUS_TRGOFF);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // パッドチェック
    if ( kamome->CheckPad( kamome ) ) return;

    if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	act->act_end = 1;
	KMM_SetActMode( kamome, KMM_ActStandStill);
	KMM_ClearNeedl( &kamome->body ); // 麻酔弾を抜く
    }
}
// 死亡
static void KMM_ActDead( KAMOME* kamome, int time)
{
    KMMACT* act;

    act = &kamome->action;

    // 初期設定
    if ( time == 0 ){
	if ( act->current_mot == KMM_MOT_FALL ) KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DAM1_END );
	else					KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DAM2_END );
//	KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DAM1_END );
	GM_FreeTarget( &kamome->trg ); // ターゲットリストからけしてあげる
    }

    if ( act->current_mot == KMM_MOT_DAM1_END || act->current_mot == KMM_MOT_DAM2_END ){
	if ( KMM_CheckObject_IsEnd( &kamome->mmt_ctrl, 0 ) ){
	    KMM_SetActMotion( kamome, LAYER_BASE, KMM_MOT_DEAD );
	}
    }

    // 状態設定
    KMM_ActStatus( act, KMM_STATUS_DEAD | KMM_STATUS_TRGOFF);

    // ダメージチェック
    if ( kamome->CheckDamage( kamome ) ) return;

    // キャンセル
    if ( act->pad != act->set_pad ) {
	if ( !kamome->CheckPad( kamome ) ) {
	    KMM_SetActMode( kamome, KMM_ActStandStill);
	}
	return;
    }
}

// かもめパッドチェック関数
static int KamomeCheckPad( KAMOME* kamome )
{
    KMMACT* act;

    act = &kamome->action;

    if ( act->pad == PAD_NONE ) return 0 ;

    switch (act->pad){
    case PAD_IDLE:		// 羽休め
	KMM_SetModeFromPad( kamome, KMM_ActIdle, KMM_MOT_IDLE, act->pad );
	break;
    case PAD_IDLEGROUND:	// 羽休め(地面)
	KMM_SetModeFromPad( kamome, KMM_ActIdleGround, KMM_MOT_IDLE, act->pad );
	break;
    case PAD_FLYING:		// 飛行中
	KMM_SetModeFromPad( kamome, KMM_ActFlying, KMM_MOT_FLYAWAY, act->pad );
	break;
    case PAD_HOVERING:		// 滑空中
	KMM_SetModeFromPad( kamome, KMM_ActHovering, KMM_MOT_HOVER, act->pad );
	break;
    case PAD_LANDING:		// 着地
	KMM_SetModeFromPad( kamome, KMM_ActLanding, KMM_MOT_LANDING, act->pad );
	break;
    case PAD_FLIPFLOP:		// ばたつき
	KMM_SetModeFromPad( kamome, KMM_ActFlipflop, KMM_MOT_FLYING, act->pad );
	break;
    case PAD_CAUTION:		// 危険ばたつき
	KMM_SetModeFromPad( kamome, KMM_ActCaution, KMM_MOT_FLYING, act->pad );
	break;
    case PAD_TAKEOFF:		// 離陸
	KMM_SetModeFromPad( kamome, KMM_ActTakeoff, KMM_MOT_FLYAWAY_QUICK, act->pad );
	break;
    case PAD_POKE:		// つつき
	KMM_SetModeFromPad( kamome, KMM_ActPokeAttack, KMM_MOT_FLYING, act->pad );
	break;
    case PAD_DAMAGE:		// ダメージ
	KMM_SetModeFromPad( kamome, KMM_ActDamage, KMM_MOT_FLYING, act->pad );
	break;
    case PAD_FALL:		// 落下
	KMM_SetModeFromPad( kamome, KMM_ActFall, KMM_MOT_HOVER, act->pad );
	break;
    case PAD_DEAD:		// 死亡
	KMM_SetModeFromPad( kamome, KMM_ActDead, KMM_MOT_HOVER, act->pad );
	break;
    case PAD_GROUNDSLEEP: 	// 地面で睡眠
	KMM_SetModeFromPad( kamome, KMM_ActGroundSleep, KMM_MOT_IDLE, act->pad );
	break;
    case PAD_STANDSLEEP: 	// 立ったまま睡眠
	KMM_SetModeFromPad( kamome, KMM_ActStandSleep, KMM_MOT_IDLE, act->pad );
	break;
    case PAD_FEINT: 		// 気絶
	KMM_SetModeFromPad( kamome, KMM_ActFeint, KMM_MOT_IDLE, act->pad );
	break;
    case PAD_WAKEUP: 		// 起きる
	KMM_SetModeFromPad( kamome, KMM_ActWake, KMM_MOT_IDLE, act->pad );
	break;
    case PAD_STANDWAKEUP: 	// 立ち状態の起きる
	KMM_SetModeFromPad( kamome, KMM_ActWakeStand, KMM_MOT_IDLE, act->pad );
	break;
    }

    return 1;

}

// かもめ動作制御構造体初期化
static void KMM_InitAction( KAMOME* kamome, int nInitThink )
{
    KMMACT* act;

    act = &kamome->action;

    switch ( nInitThink ){
    case KMM_INITTHK_REST:	// 羽休め
	kamome->actmode_call = KMM_ActStandStill;
	break;
    case KMM_INITTHK_FLYING:	// 飛行
	kamome->actmode_call = KMM_ActFlying;
	break;
    case KMM_INITTHK_FLIPFLOP:	// ばたばた
	kamome->actmode_call = KMM_ActFlipflop;
	break;
    default:			// 羽休め
	kamome->actmode_call = KMM_ActStandStill;
	break;
    }
    KMM_SetCheckPad( kamome, KamomeCheckPad);
    act->time 	       = 0;
    act->status        = 0;
    act->dir           = 0;
    act->act_end       = 0;
    act->pad           = PAD_NONE;
    act->headmark      = NULL ;
}
// ダメージチェック
static int KMM_CheckDamage( KAMOME* kamome )
{
    TARGET*		def;
    KMMACT* 		act;
    KAMOME_MNG* 	kmmng;
    long64		weapon;
    extern void* NewKamomeFeather( FVECTOR*, FVECTOR*, int, int);

    def = &kamome->trg;
    act = &kamome->action;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    weapon = 0;

    if ( (act->status & KMM_STATUS_TRGOFF) || (act->status & KMM_STATUS_DEAD)){ // あたりなしor死亡
	// ダメージフラグクリア
	def->weapon_type = 0 ;
	def->damaged = 0 ;
	return 0;
    }

    if ( TARGET_POWER & def->damaged ) {
	weapon = def->weapon_type;

	// 状態設定
	KMM_ActStatus( act, KMM_STATUS_DAMAGE | KMM_STATUS_TRGOFF); // 連続当たり回避

	// ダメージフラグクリア
	def->weapon_type = 0;
	def->damaged = 0;

	// 武器による分岐 
	if ( (weapon & WP_PUNCHR) || (weapon & WP_PUNCHL) || (weapon & WP_KICK) ) { // 打撃系
	    GM_SeSetMode( SD_P_PUNCH02, &def->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( (weapon & WP_PUNCHR) || (weapon & WP_PUNCHL) ){
		if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ){    // 振動コール
		    NewPadVibration2( GV_StrCode("rai_punch_02"), 0); // 強
		}else{
		    NewPadVibration2( GV_StrCode("rai_punch_01"), 0); // 弱
		}
	    }else{
		if ( weapon & WP_KICK1 ) 	NewPadVibration2( GV_StrCode("rai_kick_01"), 0); // キック１回目
		else				NewPadVibration2( GV_StrCode("rai_kick_02"), 0); // キック２回目
	    }

	    KMM_SetModeFromPad( kamome, KMM_ActDamage, KMM_MOT_FLYING, act->pad );
	    // 音設定
	    GM_SetNoise( NOISE_M , &kamome->mov, kamome->map );	  // 音発生
	    kamome->nLife -= 50;
	    if (kamome->nLife <= 0) KMM_CallDmgProc( kmmng, KMM_DMGPROC_DEAD); // プロシージャコール
	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_PPK, -1 );

	    kmmng->nAngryLimit++;	// 怒りカウンタ
	}else if ( (weapon & WP_M92) || (weapon & WP_PSG1T) ){ // 麻酔系
	    KMM_ActStatus( act, KMM_STATUS_SLEEP);
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    // 音設定
	    GM_SetNoise( NOISE_S , &kamome->mov, kamome->map );	  // 音発生
	    KMM_CallDmgProc( kmmng, KMM_DMGPROC_SLEEP); // プロシージャコール
	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_SLEEP, -1 );
	    kmmng->nAngryLimit++;	// 怒りカウンタ
	}else if ( (weapon & WP_STUNGRENADE) ){ // スタン系
	    KMM_ActStatus( act, KMM_STATUS_FEINT);
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    // 音設定
	    GM_SetNoise( NOISE_S , &kamome->mov, kamome->map );	  // 音発生
	    KMM_CallDmgProc( kmmng, KMM_DMGPROC_FEINT); // プロシージャコール
	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_FEINT, -1 );
	}else if ( weapon & WP_COLDSPRAY ){ // C4冷却スプレー
#if 1
	    if ( (kamome->action.current_mot == KMM_MOT_IDLE) ||
		 (kamome->action.current_mot >= KMM_MOT_WALK_START && kamome->action.current_mot <= KMM_MOT_SURPRISE_BACK) || 
		 (kamome->action.current_mot == KMM_MOT_FLYAWAY_QUICK && kamome->mmt_ctrl.m_time < COUNT_VMODE(30)) ||
		 (kamome->action.current_mot == KMM_MOT_FLYAWAY && kamome->mmt_ctrl.m_time < COUNT_VMODE(69) ) ){
		KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_TAKEOFF);       // 飛び立つ
	    }else{
		KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_EVADE);       // 逃げ
	    }
#else
	    KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_EVADE);       // 逃げ
#endif
	    KMM_ActStatus( act, KMM_STATUS_TRGOFF);    // 無敵にする
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    // 音設定
//	    GM_SetNoise( NOISE_MM, &kamome->mov, kamome->map );	  // 音発生
//	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_SPRAY, -1 );
	    return 0;
	}else if ( (weapon & WP_STUNFAR) ) {  	// 遠距離スタン
	    KMM_ActStatus( act, KMM_STATUS_TRGOFF);    // 無敵にする
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    return 0;
	}else if ( (weapon & WP_STAMP)  || (weapon & WP_WEAPONCORE) || (weapon & WP_NONE) ){ // 足踏み:落下カモメ
	    KMM_ActStatus( act, KMM_STATUS_TRGOFF);    // 無敵にする
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    return 0;
	}else{
	    KMM_SetModeFromPad( kamome, KMM_ActDamage, KMM_MOT_FLYING, act->pad );
	    // 音設定
	    GM_SetNoise( NOISE_M , &kamome->mov, kamome->map );	  // 音発生
	    kamome->nLife -= 50;
	    KMM_CallDmgProc( kmmng, KMM_DMGPROC_DEAD); // プロシージャコール
	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_SHOOT, -1 );
	    kmmng->nAngryLimit++;	// 怒りカウンタ
	}

	if (kamome->nLife <= 0){
	    // 状態設定
	    KMM_ActStatus( act, KMM_STATUS_DEAD);
	    kamome->nLife = 0;
	    // アウト鳴き声
	    KAMOME_Say( kamome, KMM_SE_OUT01 + BP_PS2_rand() % 2, GM_SEMODE_BOMB);
	}else{
	    // ダメージ鳴き声
	    KAMOME_Say( kamome, KMM_SE_DMG01 + BP_PS2_rand() % 2, GM_SEMODE_BOMB);
	} 

	return 1;
    }

    return 0;
}
// ダウン中のダメージ判定
static int KMM_CheckDownDamage( KAMOME* kamome )
{
    TARGET*		def;
    KMMACT* 		act;
    KAMOME_MNG* 	kmmng;
    long64		weapon;
    extern void* NewKamomeFeather( FVECTOR*, FVECTOR*, int, int);

    def = &kamome->trg;
    act = &kamome->action;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    weapon = 0;

    if ( (act->status & KMM_STATUS_TRGOFF) || (act->status & KMM_STATUS_DEAD)){ // あたりなしor死亡
	// ダメージフラグクリア
	def->weapon_type = 0 ;
	def->damaged = 0 ;
	return 0;
    }


    if ( TARGET_POWER & def->damaged ) {
	weapon = def->weapon_type;

	// 状態設定
	KMM_ActStatus( act, KMM_STATUS_DAMAGE | KMM_STATUS_TRGOFF); // 連続当たり回避

	// ダメージフラグクリア
	def->weapon_type = 0 ;
	def->damaged = 0 ;

	// 武器による分岐 
	if ( (weapon & WP_PUNCHR) || (weapon & WP_PUNCHL) || (weapon & WP_KICK) ) { // 打撃系
	    GM_SeSetMode( SD_P_PUNCH02, &def->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( (weapon & WP_PUNCHR) || (weapon & WP_PUNCHL )){
		if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ){    // 振動コール
		    NewPadVibration2( GV_StrCode("rai_punch_02"), 0); // 強
		}else{
		    NewPadVibration2( GV_StrCode("rai_punch_01"), 0); // 弱
		}
	    }else{
		if ( weapon & WP_KICK1 ) 	NewPadVibration2( GV_StrCode("rai_kick_01"), 0); // キック１回目
		else				NewPadVibration2( GV_StrCode("rai_kick_02"), 0); // キック２回目
	    }

	    KMM_SetModeFromPad( kamome, KMM_ActDamage, KMM_MOT_FLYING, act->pad );
	    // 音設定
	    GM_SetNoise( NOISE_M , &kamome->mov, kamome->map );	  // 音発生
	    kamome->nLife -= 50;
	    if (kamome->nLife <= 0) KMM_CallDmgProc( kmmng, KMM_DMGPROC_DEAD); // プロシージャコール
	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_PPK, -1 );

	    kmmng->nAngryLimit++;	// 怒りカウンタ
	}else if ( (weapon & WP_M92) || (weapon & WP_PSG1T) ){ // 麻酔系
	    KMM_ActStatus( act, KMM_STATUS_SLEEP);
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    // 音設定
	    GM_SetNoise( NOISE_S , &kamome->mov, kamome->map );	  // 音発生
	    KMM_CallDmgProc( kmmng, KMM_DMGPROC_SLEEP); // プロシージャコール
	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_SLEEP, -1 );
	    kmmng->nAngryLimit++;	// 怒りカウンタ
	}else if ( (weapon & WP_STUNGRENADE) ){ // スタン系
	    KMM_ActStatus( act, KMM_STATUS_FEINT);
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    // 音設定
	    GM_SetNoise( NOISE_S , &kamome->mov, kamome->map );	  // 音発生
	    KMM_CallDmgProc( kmmng, KMM_DMGPROC_FEINT); // プロシージャコール
	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_FEINT, -1 );
	}else if ( weapon & WP_COLDSPRAY ){ // C4冷却スプレー
	    if ( act->status & KMM_STATUS_GROUND ) { // 着地していたら
		KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_WAKEUP);       // 起きる
	    }else {
		KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_TAKEOFF);      // 起きる
	    }
	    KMM_ActStatus( act, KMM_STATUS_TRGOFF);    // 無敵にする
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    // 音設定
//	    GM_SetNoise( NOISE_MM, &kamome->mov, kamome->map );	  // 音発生
//	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_SPRAY, -1 );
	    return 0;
	}else if ( (weapon & WP_STUNFAR) ) {  	// 遠距離スタン
	    KMM_ActStatus( act, KMM_STATUS_TRGOFF);    // 無敵にする
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    return 0;
	}else if ( (weapon & WP_STAMP) || (weapon & WP_WEAPONCORE) || (weapon & WP_NONE) ){ // 足踏み:落下カモメ
	    KMM_ActStatus( act, KMM_STATUS_TRGOFF);    // 無敵にする
	    KMM_ActStatusOff( act, KMM_STATUS_DAMAGE); // ダメージフラグを消す
	    return 0;
	}else{
	    KMM_SetModeFromPad( kamome, KMM_ActDamage, KMM_MOT_FLYING, act->pad );
	    // 音設定
	    GM_SetNoise( NOISE_M , &kamome->mov, kamome->map );	  // 音発生
	    kamome->nLife -= 50;
	    KMM_CallDmgProc( kmmng, KMM_DMGPROC_DEAD); // プロシージャコール
	    NewKamomeFeather( &kamome->mov, &kamome->vecForce, KMM_FEATHER_SHOOT, -1 );
	    kmmng->nAngryLimit++;	// 怒りカウンタ
	}

	if (kamome->nLife <= 0){
	    // 状態設定
	    KMM_ActStatus( act, KMM_STATUS_DEAD);
	    kamome->nLife = 0;
	    kmmng->nAngryLimit++;	// 怒りカウンタ
	    // アウト鳴き声
	    KAMOME_Say( kamome, KMM_SE_OUT01 + BP_PS2_rand() % 2, GM_SEMODE_BOMB);
	}else{
	    // ダメージ鳴き声
	    KAMOME_Say( kamome, KMM_SE_DMG01 + BP_PS2_rand() % 2, GM_SEMODE_BOMB);
	} 

	return 1;
    }

    return 0;
}

// かもめ動作関数
static void Action( KAMOME* kamome )
{
    int time;
    KMMACT* act;
    act = &kamome->action;

    // 毎フレーム初期化
    act->act_end = 0;
    act->status  = 0;

    // 死亡チェック
    if ( kamome->nLife <= 0 ){
	KMM_ActStatus( act, KMM_STATUS_DEAD | KMM_STATUS_TRGOFF);
    }


    time = act->time++;
    // 動作関数コール
    if ( kamome->actmode_call == NULL ) return ;
    ( *kamome->actmode_call )( kamome, time);
}


