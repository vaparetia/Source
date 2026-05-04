/*
    fatutil.c
    ファットマンユーティリティ
    2001/06/09 Masafumi Okuta
    $Id: fatutil.c,v 1.1.1.3 2002/11/19 11:48:01 Yoshizawa1 Exp $
*/
// 指定されたベジェ曲線がゾーンを通過するか
static void FATUTL_BezierPassCheck( Work* work)
{
    int 	i, j, k;
    int time;
    float	fRate, fRateL, fRateR, fLeft, fRight, fBase;
    FVECTOR 	vecCenter, vecMin, vecMax, vecLeft, vecRight, vecSub;
    FMATRIX	mat;
    HZX_ZON*	pzone;
    NPCWORK*	npc;

#ifdef DEBUG_MODE
GV_SET_PRFC_CLOCK();
#endif

#if 0
    if( ( work->pbBezierPass = (u_char*)GV_Malloc(sizeof(u_char) * FAT_ROUTEDATA_NUM * 32)) == NULL ) {
	return;
    }
#endif

    npc = &work->npc;
    GV_ZeroMemory( work->pbBezierPass, sizeof(u_char) * FAT_ROUTEDATA_NUM * 32 );

    for ( j = 0; j < FAT_ROUTEDATA_NUM; j++){
       KP_CTASSERT( sizeof( FAT_BEZ_MOVE_POS ) / sizeof( FAT_BEZ_MOVE_POS[0] ) >= FAT_ROUTEDATA_NUM );
	// ベジェ補間用のマトリクスを計算しておく
	MAO_MakeBezierMatrix( &mat, &FAT_BEZ_MOVE_POS[j][0], &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][2]);
	for ( i = 0; i < 256; i++){
	    pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, i, i ));

	    if ( pzone == NULL ) continue;

	    vecCenter.vx = pzone->x;
	    vecCenter.vy = pzone->y;
	    vecCenter.vz = pzone->z;
	    vecCenter.vw = 1.f;
	    vecMin.vx = pzone->x - pzone->w;
	    vecMin.vy = pzone->y;
	    vecMin.vz = pzone->z - pzone->h;
	    vecMax.vx = pzone->x + pzone->w;
	    vecMax.vy = pzone->y;
	    vecMax.vz = pzone->z + pzone->h;


	    fRate = 0.5f;
	    fBase = 0.5f;

	    for ( k = 0; k < 16; k++){
		fBase *= 0.5f;
		// 左
		fRateL = fRate - fBase;
		MAO_Bezier3InterpQuick( &vecLeft, &mat, fRateL );
		_sceVu0SubVector( &vecSub, &vecLeft, &vecCenter);
		fLeft = _FVecLen2( &vecSub);
		// 右
		fRateR = fRate + fBase;
		MAO_Bezier3InterpQuick( &vecRight, &mat, fRateR );
		_sceVu0SubVector( &vecSub, &vecRight, &vecCenter);
		fRight = _FVecLen2( &vecSub);
//printf("%d %d --- %f %f\n", j, k, fRateL, fRateR);
		
		if ( fLeft < fRight ){ // 左の方が近い
		    if ( MAO_FlatCheck( &vecLeft, &vecMin, &vecMax) ){
			work->pbBezierPass[ j * 32 + (int)(i / 8) ] |= (1 << ( i % 8));
			break;
		    }
		    fRate = fRateL;
		}else{
		    if ( MAO_FlatCheck( &vecRight, &vecMin, &vecMax) ){
			work->pbBezierPass[ j * 32 + (int)(i / 8) ] |= (1 << ( i % 8));
			break;
		    }
		    fRate = fRateR;
		}
	    }
	}
    }
#ifdef DEBUG_MODE
time = GV_GET_PRFC_CLOCK();
printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
printf("BezierPassCheck-EXECTIME = %.3f\n", (float)time / 60.f / 262.f );
printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
#endif
}

// 血を出す
static void FAT_SpillBlood( Work*    work, 
                            FVECTOR* pvecHit,    // 当たった位置
                            FVECTOR* pvecForce)  // 力積
{
    GV_SetActorChild( work, NewBlood( &BODYWORLD( &work->body, HUMAN21_ATAMA), pvecHit, pvecForce, 0, 0 ) ) ;
}

// ブラストスーツ部分の防弾エフェクト
static void FAT_BlastSuitEvadeShootEffect( Work* work, FVECTOR* pvecPos, FVECTOR* pvecDir)
{
    SVECTOR   rot ;
    SVECTOR   rot_wide ;
    FVECTOR   color ;
    FVECTOR   vec, vecDir;
    FMATRIX   mat;

    _sceVu0Normalize( &vecDir, pvecDir);

    NewTs_Min_Fog( pvecPos);
    NewTs_Spark( pvecPos, &vecDir, 5.f);

    vec.vx = -vecDir.vx;
    vec.vy = -vecDir.vy;
    vec.vz = -vecDir.vz;

    mat = DG_UnitMatrix;
    mat.m[3][0] = pvecPos->vx;
    mat.m[3][1] = pvecPos->vy;
    mat.m[3][2] = pvecPos->vz;
    MAO_SetAxisZ( &mat, &mat, &vec);
    
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0;
    rot_wide.vx = 256;
    rot_wide.vy = 256;
    rot_wide.vz = 1;
    color.vx = 255.f;
    color.vy = 128.f;
    color.vz = 128.f;
    color.vw = 80.f;
    NewSpark2( 40, &mat, 36.f, 16.f, 0.5f, &rot, &rot_wide, &color, 1.f, 6 ) ;
    // ブラストスーツ跳弾音
//    FAT_CallSe( work, SE_SUITREV4, pvecPos, GM_SEMODE_BOMB);
}

// ファットマン地震	
static void FAT_Quake( Work* work, int nVibTime, int nIntence, int nShakeTime)
{
    work->nQuakeCntr = nVibTime;

    //カメラ揺らし
    if( !GM_CheckGameStatus( STATE_PLAY_DEMO ) ){
	NewShakeCamera2( 0, nIntence, nShakeTime, &work->npc.ctrl->mov );

	if ( work->npc.action.current_mot != FAT_MOT_BLAST_DAM_3 ){
	    if ( work->pl_dis < 3000 ){
		NewPadVibration( FAT_FallNearVibH, 1 );
		NewPadVibration( FAT_FallNearVibL, 2 );
	    }else{
		NewPadVibration( FAT_FallMiddleVibH, 1 );
		NewPadVibration( FAT_FallMiddleVibL, 2 );
	    }
	}else {
	    NewPadVibration( FAT_FallFarVibH, 1 );
	    NewPadVibration( FAT_FallFarVibL, 2 );
	}
    }

    // 煙
    {
	FVECTOR vec;
	_sceVu0CopyVector( &vec, &work->npc.ctrl->mov);
	if ( work->npc.ctrl->level_found & 1){
	    vec.vy -= work->npc.ctrl->height;
	    vec.vy += ( (work->npc.ctrl->height < 50.f) ? work->npc.ctrl->height : 50.f);
	}
	NewLandingSmoke( &vec, 1000.f);
    }
}
// ファットマン地震更新	
static void FAT_QuakeUpdate( Work* work)
{
#if 0
    //振動
    if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
	if ( work->nQuakeCntr > 0 ){
	    work->nQuakeCntr--;
// NewPadVibration
	    GM_SetVibration1( 0, irnd()%2 );
	    GM_SetVibration1( 0, 192 + irnd()%64 );
	}
    }
#endif
}

// 仮思考分岐用
static int FAT_ChkThkTemporary( Work* work){ return 0; }

// 麻酔弾を抜く
static void FAT_ClearNeedl( OBJECT* body ){
    SearchAndFallAttachment_called( body, 0, COUNT_VMODE(30), COUNT_VMODE(10) ) ;
}

// 顔アニメキャラにモーション再生メッセージ
static void FAT_SendFaceMess( Work* work, int nType)
{
    NPCWORK*	npc;

    npc = &work->npc;
    { // メッセージ設定
	GV_MSG msg ;
	u_int buffer[  ] = { 0, nType };
	
	msg.address = work->nFaceChara;
	msg.message = buffer;
	msg.message_len = sizeof(buffer)/sizeof(u_int);
	GV_SendMessage( &msg ) ;
    }

}
#if 0
// 顔アニメキャラに視線制御メッセージ
static void FAT_SendEyeMess( Work* work, int nInterpTime, int nType, FVECTOR* pvec)
{
    NPCWORK*	npc;

    npc = &work->npc;
    { // メッセージ設定
	GV_MSG msg ;
	u_int buffer[  ] = { 1, nType, nInterpTime, (int)pvec->vx, (int)pvec->vy, (int)pvec->vz };

	msg.address = work->nFaceChara;
	msg.message = buffer;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
    }

}
#endif
