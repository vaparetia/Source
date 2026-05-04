/*
    kmtest.c
    カモメさんテスト
    2001/05/01 Masafumi Okuta
    $Id: kmtest.c,v 1.1.1.3 2002/11/19 11:48:04 Yoshizawa1 Exp $
*/

#include "BP_Misc.h"


// LODチェック
static void KMM_SetLodMode( KAMOME* kamome, int mode)
{
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;

    switch ( mode ){
    case KMM_LOD_NONE:		// LODなし
	kamome->nLod = KMM_LOD_NONE;
	break;
    case KMM_LOD_REST:		// はねやすめ中
	if ( kmmng->nMdlName[KMM_MDLNAME_ONE1] != 0 && 
	     kamome->bodyLodStop.objs->n_models == 1 && 
	     KR_CameraDis( &kamome->mov) >= kmmng->nLodDist ){	
	    kamome->nLod = KMM_LOD_REST;
	}
	break;
    case KMM_LOD_FLYING:	// 飛行中
	if ( kmmng->nMdlName[KMM_MDLNAME_ONE2] != 0 && 
	     kamome->bodyLodFlying.objs->n_models == 1 && 
	     KR_CameraDis( &kamome->mov) >= kmmng->nLodDist ) {
	    kamome->nLod = KMM_LOD_FLYING;
	}
	break;
    }
}
#if 0
// ローポリかもめ用モーション修正
static void KMM_LowPoriMotionFix( KAMOME* kamome )
{
    _sceVu0CopyMatrix( &kamome->body.objs->objs[12].world, &kamome->body.objs->objs[13].world);
    _sceVu0CopyMatrix( &kamome->body.objs->objs[13].world, &kamome->body.objs->objs[14].world);
}
#endif
// 制御処理
static void KMM_ActControl( KAMOME* kamome )
{
    int interp;
    FVECTOR vecPre;

    // ターゲットの大きさ更新
    if ( PL_CurrentWeapon() != WP_PSG1 || PL_CurrentWeapon() != WP_PSG1T ){ // 狙撃時
	static FVECTOR vecSnipeSize = { 150.f, 150.f, 150.f, 1.f};
	GM_SetTargetSize( &kamome->trg, &vecSnipeSize);
    }else{
	static FVECTOR vecDefSize = { 200.f, 200.f, 200.f, 1.f};
	GM_SetTargetSize( &kamome->trg, &vecDefSize);
    }
    
    // 向き更新 
    if ( ( interp = kamome->interp ) == 0 ) {

      if ( BP_IsPAL()==TRUE )
         GV_NearExp8PVPAL( &( kamome->rot ), &( kamome->turn ), 3 );
      else
         GV_NearExp8PV( &( kamome->rot ), &( kamome->turn ), 3 );

    }else {
	GV_NearTimePV( &( kamome->rot ), &( kamome->turn ), interp, 3 );
	kamome->interp = interp - 1;
    }

    // 位置更新
    if ( (kamome->action.current_mot >= KMM_MOT_WALK_START && kamome->action.current_mot <= KMM_MOT_SURPRISE_BACK) || 
	 (kamome->action.current_mot == KMM_MOT_FLYAWAY_QUICK && kamome->mmt_ctrl.m_time < COUNT_VMODE(30) ) ||
	 (kamome->action.current_mot == KMM_MOT_FLYAWAY && kamome->mmt_ctrl.m_time < COUNT_VMODE(69) ) ){
	// この条件ではモーション移動量を使う
	u_short* psHeight;
	SVECTOR vec;
	FVECTOR vecShift;
	FMATRIX mat;

	vec.vx = 0;
	vec.vy = kamome->rot.vy;
	vec.vz = 0;

	DG_SetPos2( &DG_ZeroVector, &vec);
	DG_GetPos( &mat);

	kamome->vecDistance.vx  = kamome->mmt_ctrl.body->step->vx;
	kamome->vecDistance.vy  = 0.f; // kamome->mmt_ctrl.body->step->vy;
	kamome->vecDistance.vz  = kamome->mmt_ctrl.body->step->vz;
	kamome->vecDistance.vw  = 1.f; // kamome->mmt_ctrl.body->step->vw;

	_sceVu0ApplyMatrix( &vecShift, &mat, &kamome->vecDistance);
	vecShift.vy = 0.f; // (kamome->mmt_ctrl.body->step->vy - kamome->fMotStartHeight);
	_sceVu0AddVector(&kamome->vecMove, &kamome->vecMove, &vecShift);
	psHeight = kamome->mmt_ctrl.pmmtData->m_height + 
	    ( kamome->mmt_ctrl.pmmtData->m_ptr[kamome->mmt_ctrl.current_mot] + (int)kamome->mmt_ctrl.m_time);

	kamome->vecMove.vy = kamome->fMotStartHeight + (float)(*psHeight);
    }

    _sceVu0CopyVector(&vecPre, &kamome->mov);
    _sceVu0CopyVector(&kamome->mov, &kamome->vecMove);
    _sceVu0SubVector( &kamome->step, &kamome->mov, &vecPre);

#ifdef DEBUG_MODE
    // ワールドリミットチェック */
    if ( kamome->mov.vy > GM_WORLD_LIMIT_UPPER * 2.0F  ||
	 kamome->mov.vy < GM_WORLD_LIMIT_BOTTOM * 2.0F ) {
	GV_ERROR( GV_ERROR_WORLD_BOTTOM ) ;
    }
#endif
    // 定数回転マトリクスをセット 
    DG_SetPos2( &kamome->mov, &kamome->rot );
}
// システム系コントロール処理
static void KMM_ActSystem( KAMOME* kamome )
{
    KAMOME_MNG* kmmng;
    OBJECT*	body;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    body = &kamome->body;
    if ( kamome->nLod ){

    }else{
	// モーション再生
	if ( DG_BoundCheck( &kamome->matOrg, &kamome->body.objs->bound_max, &kamome->body.objs->bound_min ) != -1 ||
	    (kamome->action.current_mot >= KMM_MOT_WALK_START && kamome->action.current_mot <= KMM_MOT_SURPRISE_BACK) || 
	    (kamome->action.current_mot == KMM_MOT_FLYAWAY_QUICK && kamome->mmt_ctrl.m_time < COUNT_VMODE(30)) ||
	    (kamome->action.current_mot == KMM_MOT_FLYAWAY && kamome->mmt_ctrl.m_time < COUNT_VMODE(69) ) ){
	    MEMMOT_MakeMotion( &kamome->mmt_ctrl); 		// 画面内
	}else{
	    MEMMOT_MakeMotionSkip( &kamome->mmt_ctrl);	// 画面外
	}
    }


    // かもめ制御処理
    KMM_ActControl( kamome );

    // 描画
    if ( kamome->nLod ){
	OBJECT* 	object;	
	if ( kamome->nLod == KMM_LOD_REST){
	    DG_PutObjs( kamome->bodyLodStop.objs ); // DGのワークマトリクスの位置に表示
	    if ( kamome->bodyLodStop.flag & DG_FLAG_INVISIBLE ){
		DG_InvisibleObjs( kamome->bodyLodStop.objs);
	    }else{
		DG_VisibleObjs( kamome->bodyLodStop.objs);
	    }
	    SET_FLAG( kamome->bodyLodFlying.objs->flag, DG_FLAG_INVISIBLE);
	    SET_FLAG( kamome->body.objs->flag, DG_FLAG_INVISIBLE);
	    object = &kamome->bodyLodStop;
	}else{
	    DG_PutObjs( kamome->bodyLodFlying.objs ); // DGのワークマトリクスの位置に表示
	    SET_FLAG( kamome->body.objs->flag, DG_FLAG_INVISIBLE);
	    SET_FLAG( kamome->bodyLodStop.objs->flag, DG_FLAG_INVISIBLE);
	    if ( kamome->bodyLodFlying.flag & DG_FLAG_INVISIBLE ){
		DG_InvisibleObjs( kamome->bodyLodFlying.objs);
	    }else{
		DG_VisibleObjs( kamome->bodyLodFlying.objs);
	    }
	    object = &kamome->bodyLodFlying;
	}

	if ( object->map_name != GM_CurrentMap ) { // マップチェック : 省略可能？
	    object->map_name = GM_CurrentMap ;
	    GM_GroupObjs( object->objs, GM_CurrentMap ) ;
	}

    }else{
	OBJECT* 	object   = body; 
	MEMMOT_CTRL*	mmt_ctrl = &kamome->mmt_ctrl;
	MEMMOT_DATA*	pmmtData = mmt_ctrl->pmmtData;
	
	DG_PutObjs( object->objs ); // DGのワークマトリクスの位置に表示
	// メモリモーションによる再生	
	MEMMOT_ActMotion2( pmmtData->body_mtbuff[mmt_ctrl->current_mot].m_ctrl, mmt_ctrl->body->objs, 
			   mmt_ctrl->current_abs ) ;

	if ( kamome->body.flag & DG_FLAG_INVISIBLE ){
	    DG_InvisibleObjs( kamome->body.objs);
	}else{
	    DG_VisibleObjs( kamome->body.objs);
	}
	SET_FLAG( kamome->bodyLodStop.objs->flag, DG_FLAG_INVISIBLE);
	SET_FLAG( kamome->bodyLodFlying.objs->flag, DG_FLAG_INVISIBLE);

	if ( object->map_name != GM_CurrentMap ) { // マップチェック : 省略可能？
	    object->map_name = GM_CurrentMap ;
	    GM_GroupObjs( object->objs, GM_CurrentMap ) ;
	}
    }
}
// かもめルーチン
static void KMM_Routine( KAMOME* kamome )
{
    KAMOME_MNG* kmmng;
    FVECTOR vecPre;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    vecPre = kamome->mov;

    // 前処理
    PreProcess( kamome );

    // 思考
    Think( kamome );

    // 動作
    Action( kamome );

    // 後処理
    AfterProcess( kamome );
    
//    kamome->fSpeed = kamome->fSpeed * 0.98f + kmmng->fBaseSpeed * 0.02f;
}


// 動作関数
static	void	Act( kamome )
KAMOME*	kamome;
{
    float fPrevHeight;

    GM_CurrentMap = kamome->map;

    fPrevHeight = kamome->mov.vy;

    // システム系コントロール処理
    KMM_ActSystem( kamome );

    // システム前処理

    // かもめルーチン
    KMM_Routine( kamome );

    // システム後処理
    {
	KAMOME_MNG* kmmng;

	kmmng = (KAMOME_MNG*)kamome->pvManager;

	// 水しぶきチェック
	if ( !kamome->ucSeaSplush ){
	    if ( kamome->mov.vy <= GM_WaterLevel &&  fPrevHeight > GM_WaterLevel ){
		extern void* SetSplushSequence3( FVECTOR* );
		SetSplushSequence3( &kamome->mov ); // 水しぶき
		kamome->ucSeaSplush = 1;
	    }
	}

	// 一定の高さより下になったら消す
	if ( kamome->mov.vy < kmmng->nDeadHeight){
	    KMM_RemoveKamome( kmmng, kamome); // リストからはずす
	    SearchAndFallAttachment_called( &kamome->body, 0, 0, 0 ); // 麻酔を外す
	    GV_DestroyActor( kamome );
	    return;
	}
    }
}

// 破棄関数
static	void	Die( kamome )
KAMOME*	kamome;
{
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;

    GM_FreeObject( &kamome->body);
    if ( kmmng->nMdlName[KMM_MDLNAME_ONE1] != 0 ) GM_FreeObject( &kamome->bodyLodStop);
    if ( kmmng->nMdlName[KMM_MDLNAME_ONE2] != 0 ) GM_FreeObject( &kamome->bodyLodFlying);
    GM_FreeTarget( &kamome->trg );

    KMM_EraseHeadMark( kamome );

    GV_DestroyChild( kamome );

}

// モデル初期化
#define OBJECT_FLAG (DG_FLAG_IRREACTION|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
static void KMM_InitObject( KAMOME* kamome, int modelname )
{
    // 関節型モデルの初期化 
    GM_InitObject( &kamome->body, modelname, OBJECT_FLAG );
    
}

// モーション初期化
static void KMM_InitMotion( KAMOME* kamome, int motname)
{
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;

    MEMMOT_InitMotion( &kamome->mmt_ctrl, &kmmng->mmtData, &kamome->body, 1);
    // 移動量設定
    kamome->vecDistance = DG_ZeroVector;
    GM_ConfigObjectStep( kamome->mmt_ctrl.body, &kamome->vecDistance);
}
// ポーズ初期化
static void KMM_InitPose( KAMOME*    kamome, 	// かもめワーク
			  FVECTOR* pvecPos,	// 初期位置
			  int      nDirY,	// 初期方向
			  int	   nInitMar,	// 初期使用モーション
			  int	   nInitMot)	// 初期再生モーション番号
{
    OBJECT*	body;

    body = &kamome->body;

    kamome->mov    = *pvecPos;	
    kamome->nHeight = pvecPos->vy;
    kamome->mov.vy += KAMOME_Y_OFFSET;
    kamome->rot    = kamome->turn = DG_ZeroSVector;
    kamome->rot.vy = kamome->turn.vy = kamome->nLandYRot = nDirY;
    kamome->interp = 8;
    kamome->hzx_id = GM_GetHzxGroupID( kamome->map );

    kamome->vecAdj = DG_ZeroSVector;

    kamome->fSpeed = 1.f;
}
// ターゲット設定
static void KMM_InitTarget( KAMOME* kamome)
{
    static FVECTOR vecSize = { 200.f, 200.f, 200.f, 1.f};
    static FVECTOR vecShift = { 0.f, 0.f, 0.f, 1.f};

    GM_SetTarget( &kamome->trg, TARGET_DEFENSE|TARGET_SEEK, 1, ENEMY_SIDE, &vecSize, &vecShift ) ;
    GM_SetTargetWeaponType( &kamome->trg, 0 ) ;
    GM_PutTarget( &kamome->trg ) ;
    kamome->trg.class = (TARGET_LOCKON|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER);

    // コールバック設定
    {
	extern void	GM_SetTargetCallBack( TARGET*, TARGET_CALLBACK, void* );
	GM_SetTargetCallBack( &kamome->trg, KMM_TargetCallback, kamome);
    }

    kamome->nLife = 50;
    KMM_SetCheckDamage( kamome, KMM_CheckDamage );
}
// ヘッドマーク初期化
static void KMM_InitHeadMark( KAMOME* kamome )
{
    kamome->pvHeadMark = NULL;
}
// ヘッドマーク呼出
static void KMM_CallHeadMark( KAMOME* kamome, int mode )
{
    // 既に存在していたら破棄する
    if ( kamome->pvHeadMark != NULL){
	GV_DestroyOtherActor( kamome->pvHeadMark );
	kamome->pvHeadMark = NULL;
    }

    switch (mode){
    case KMM_HEADMARK_ZZZ:	// ZZZ
	GV_SetActorChild( kamome, kamome->pvHeadMark = New_Zzz( &BODYWORLD( &kamome->body, KAMOME16_ATAMA)) );
	break;
    case KMM_HEADMARK_PIYO:	// ★★★
	GV_SetActorChild( kamome, kamome->pvHeadMark = NewPiyori( &BODYWORLD( &kamome->body, KAMOME16_ATAMA), &kamome->map ) );
	break;
    default:
	GV_SetActorChild( kamome, kamome->pvHeadMark = New_Zzz( &BODYWORLD( &kamome->body, KAMOME16_ATAMA)) );
	break;
    }
}
// ヘッドマーク消滅
static void KMM_EraseHeadMark( KAMOME* kamome )
{
    if ( kamome->pvHeadMark == NULL){
	return ;
    }
    GV_DestroyOtherActor( kamome->pvHeadMark );
    kamome->pvHeadMark = NULL;
}
// 拡張ワーク初期化
static void KMM_InitAppendWork( KAMOME* kamome )
{
    kamome->nAimName = 0;
}

// 共通初期化部分
#define ONEOBJ_FLAG (DG_FLAG_IRREACTION|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
static void InitWork( kamome, name, where, pvec, nYRot, nInitMot, nInitThink, nNumber )
KAMOME*		kamome;
int		name;
int		where;
FVECTOR* 	pvec;
int	 	nYRot;
int		nInitMot;
int		nInitThink;
int		nNumber;
{
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->name = name;
    kamome->map  = where;
    kamome->nNumber = nNumber;

    kamome->nMessage 	= 0;	// メッセージ種類
    kamome->nMsgIndex	= -1;	// メッセージインデックス

    kamome->usDannaWait = KMM_GetDemoRandom( kmmng, 0, 32); // フンカウンタ

    kamome->ucSeaSplush = 0;

    // モデル初期化
    if ( kmmng->nMdlName[KMM_MDLNAME_NORM] != 0 )    KMM_InitObject( kamome, kmmng->nMdlName[KMM_MDLNAME_NORM]);
    else					     KMM_InitObject( kamome, MODEL_NAME);

    // LOD
    if ( kmmng->nMdlName[KMM_MDLNAME_ONE1] != 0 ){
	GM_InitObject( &kamome->bodyLodStop, kmmng->nMdlName[KMM_MDLNAME_ONE1], ONEOBJ_FLAG );
    }
    if ( kmmng->nMdlName[KMM_MDLNAME_ONE2] != 0 ){
	GM_InitObject( &kamome->bodyLodFlying, kmmng->nMdlName[KMM_MDLNAME_ONE2], ONEOBJ_FLAG );
    }
    

    // モーション初期化
    KMM_InitMotion( kamome, BASE_MOTION);
    // ポーズ初期化
    KMM_InitPose( kamome, pvec, nYRot, BASE_MOTION, nInitMot);
    // ターゲット設定
    KMM_InitTarget( kamome );
    // ヘッドマーク設定
    KMM_InitHeadMark( kamome );
    // 拡張ワーク初期化
    KMM_InitAppendWork( kamome );
    // 移動系初期化
    KMM_InitMoveWork( kamome );
    // 思考制御初期化
    KMM_InitThink( kamome, nInitThink );
    // 動作制御初期化
    KMM_InitAction( kamome, nInitThink  );
    // 音関連初期化
    KMM_InitSoundWork( kamome);
    // 関数設定
    kamome->ActFunc = (void*)Act;
    kamome->DieFunc = (void*)Die;

    // モーション設定
    KMM_SetActMotion( kamome, 0, nInitMot);

    // 初回メッセージ処理
    KMM_RecieveMessage( kamome);
}
// プログラム側呼出初期化
static	int	GetResources2( kamome, name, where, pvec, nYRot, nInitMot, nInitThink, pvMng, nNumber, vecArea1, vecArea2 )
KAMOME*		kamome;
int		name;
int		where;
FVECTOR* 	pvec;
int	 	nYRot;
int		nInitMot;	
int		nInitThink;
void*		pvMng;
int		nNumber;
FVECTOR		*vecArea1;	// 飛行エリア（最小値）
FVECTOR		*vecArea2;	// 飛行エリア（最大値）
{
    kamome->pvManager = pvMng;

    kamome->vecMoveArea1 = *vecArea1;
    kamome->vecMoveArea2 = *vecArea2;

    InitWork( kamome, name, where, pvec, nYRot, nInitMot, nInitThink, nNumber );

    return 0;
}
// GCL側呼出初期化
static	int	GetResources( kamome, name, where )
KAMOME*	kamome;
int	name;
int	where;
{
    FVECTOR start_pos;
    int	start_yrot;

    // GCLからデータ取得
    {
	char*	opt;
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){ // 初期位置
	    ENE_GCL_GetFV( opt, &start_pos );
	} else {
	    start_pos = GM_PlayerPosition;
	    start_pos.vz -= 1500;
	}
	if ( ( opt = GCL_GetOption( 'y' ) ) != NULL ){ // 初期Y回転角度
	    start_yrot = GCL_GetNextInt();	
	}else{
	    start_yrot = 0;
	}
	if ( ( opt = GCL_GetOption( 't' ) ) != NULL ){ // 飛行エリア用トラップ
	    int 	nTrpName;
	    HZX_BLOCK* 	blk;
	    HZX_TRP* 	trp;
	    nTrpName = GCL_GetNextInt();
	    HZX_FindTrap(GM_GetHzxGroupID( where ), nTrpName, &blk, &trp);
	    ASSERT( blk != NULL);
	    ASSERT( trp != NULL);
	    kamome->vecMoveArea1.vx = blk->tx + trp->b1.vx;
	    kamome->vecMoveArea1.vy = blk->ty + trp->b1.vy;
	    kamome->vecMoveArea1.vz = blk->tz + trp->b1.vz;
	    kamome->vecMoveArea2.vx = blk->tx + trp->b2.vx;
	    kamome->vecMoveArea2.vy = blk->ty + trp->b2.vy;
	    kamome->vecMoveArea2.vz = blk->tz + trp->b2.vz;
	}else{
	    if ( ( opt = GCL_GetOption( 'a' ) ) != NULL ){ // エリア座標指定
		ENE_GCL_GetFV( opt, &kamome->vecMoveArea1 ) ;
		ENE_GCL_GetFV( opt, &kamome->vecMoveArea2 ) ;
	    }else{
		kamome->vecMoveArea1.vx = start_pos.vx - 500.f;
		kamome->vecMoveArea1.vy = start_pos.vy - 500.f;
		kamome->vecMoveArea1.vz = start_pos.vz - 500.f;
		kamome->vecMoveArea2.vx = start_pos.vx + 500.f;
		kamome->vecMoveArea2.vy = start_pos.vy + 500.f;
		kamome->vecMoveArea2.vz = start_pos.vz + 500.f;
	    }
	}
    }

    // マネージャ登録はしない
    kamome->pvManager = NULL;

    InitWork( kamome, name, where, &start_pos, start_yrot, 0, KMM_INITTHK_REST, 0 );

    return 0;
}

// シナリオ呼出:カモメさんテスト
void*	NewKamomeTest( name, where )
int	name;
int	where;
{
    KAMOME*	kamome;

    OPERATOR();
    kamome = (KAMOME *)GV_NewActor(  GV_ACTOR_USER, sizeof( KAMOME ) );
    if ( kamome != NULL ) {
	GV_SetActor( &( kamome->actor ), Act, Die );
	GV_ActorEX( &kamome->actor );
	if ( GetResources( kamome, name, where ) < 0 ) {
	    GV_DestroyActor( kamome );
	    return NULL;
	}
    }
    return kamome;
}

static void Dummy( KAMOME* kamome){} // Actのだみー

// プログラム呼出:カモメさんテスト
void*	NewKamomeTestProg( name, where, pvec, nYRot, nInitMot, nInitThink, pvMng, nNumber, vecArea1, vecArea2 )
int		name;		// 名前
int		where;		// マップ
FVECTOR*	pvec;		// 初期位置
int		nYRot;		// 初期Y回転角度
int		nInitMot;	// 初期モーション
int		nInitThink;	// 初期思考
void*		pvMng;		// かもめマネージャ
int		nNumber;	// 通し番号
FVECTOR		*vecArea1;	// 飛行エリア（最小値）
FVECTOR		*vecArea2;	// 飛行エリア（最大値）
{
    KAMOME*	kamome;

    OPERATOR();
    kamome = (KAMOME *)GV_NewActor(  GV_ACTOR_USER, sizeof( KAMOME ) );
    if ( kamome != NULL ) {
	GV_SetActor( &( kamome->actor ), Dummy, Die );
	GV_ActorEX( &kamome->actor );
	if ( GetResources2( kamome, name, where, pvec, nYRot, nInitMot, nInitThink, pvMng, nNumber, vecArea1, vecArea2 ) < 0 ) {
	    GV_DestroyActor( kamome );
	    return NULL;
	}
    }

    return kamome;
}









