/********************************************************************************/
/*	har_vulc.c								*/
/*	ハリアヴァルカン							*/
/*	2001/04/09 Yuuta Kunibe   /04/17 Modified H.Satoyoshi			*/
/*	$Id: har_vulc.c,v 1.1.1.3 2002/11/19 11:48:26 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define	N_VERTS		(4)
#define	N_PRIMS		(3)

#define SIZE_X		(25.f)
#define SIZE_Y		(25.f)
#define SIZE_Z		(800.f)

#define BASE_R		(128)
#define BASE_G		(96)
#define BASE_B		(32)
#define ALPHA		(128)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

//#define SPEED		(2000.f)
//#define SPEED		(3500.f)
#define SPEED		(1000.f)
#define LIFE		(150)


/********************************************************************************/
/*	struct									*/
/********************************************************************************/
typedef	struct	{

    GV_ACT_EX	actor ;

    DG_PRIM2	*prim ;

    TARGET		vul_target;
    POWER_TARGET	vul_pow_tgt;

    float	speed;

    int life;
    char	*flag;
    char	*flag_hit;
} Vul_Work ;

extern void *NewWaterSurfaceMountain( FVECTOR *center, float radius, float intense , int );
extern void *GET_HAR_WORK;
extern void *NewSplushRotateSplush( FVECTOR *pos, float radius, float intense, int col );
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Vul_Act							*/
/*	引数:	Work	*work							*/
/*	説明:	実行関数	      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Vul_Act( Vul_Work *work )
{
    FVECTOR vectmp;
    SVECTOR   rot ;
    SVECTOR   rot_wide ;
    FVECTOR   color ;
    FVECTOR	from_vec, to_vec;
    float	bul_speed;

    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	bul_speed = 0.7f;
	break;
    case ST_LEV_EASY:
	bul_speed = 0.8f;
	break;
    case ST_LEV_NORMAL:
	bul_speed = 1.0f;
	break;

    case ST_LEV_UPNORM:
	bul_speed = 1.25f;
	break;

    case GM_LEVEL_HARD:
	bul_speed = 1.5f;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	bul_speed = 1.9f;
	break;
    default:
	bul_speed = 1.0f;
	break;
    }

    {
	FVECTOR	tmpvec = {0.0f, 0.0f, 550.0f, 1.0f};
	FMATRIX	pmat;
	pmat = work->prim->as_world;
	_sceVu0CopyVector(&to_vec, (FVECTOR*)pmat.m[3]);
	DG_SetPos(&pmat);
	DG_MovePos(&tmpvec);
	DG_GetPos(&work->prim->as_world);
    }



    /* 位置更新 */
    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&work->prim->as_world.m[2]), ( (SPEED*bul_speed)+work->speed) );
    _sceVu0CopyVector( &from_vec, (FVECTOR *)(&work->prim->as_world.m[3]));
    _sceVu0AddVector( (FVECTOR *)(&work->prim->as_world.m[3]), (FVECTOR *)(&work->prim->as_world.m[3]), &vectmp );
    _sceVu0CopyVector( &to_vec, (FVECTOR *)(&work->prim->as_world.m[3]));
    
    if ( HZX_OnlineHazardCheck(0, &from_vec, &to_vec, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE_ONLY,
			       (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE),
			       (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE)) ){

	{
	    FMATRIX	tmpmat;
	    FVECTOR	tmppos;
	    HZX_FLR	seg;
	    int		atr;

	    HZX_GetOnlinePoint(&to_vec);

	    HZX_GetOnlineHazard(&seg, &atr);
	    HZX_GetOnlinePoint(&to_vec);
	    HZX_GetNormal(&tmppos, &seg);

	    {
		FVECTOR tmpvec = {0.0f, 0.0f, 1000.0f, 1.0f};
		FVECTOR outvec;
		tmpmat = work->prim->as_world;
		_sceVu0CopyVector((FVECTOR*)tmpmat.m[3], &DG_ZeroVector);
		DG_SetPos(&tmpmat);
		DG_MovePos(&tmpvec);
		DG_GetPos(&tmpmat);
		tmpvec = *(FVECTOR*)tmpmat.m[3];

		_sceVu0Normalize(&tmppos, &tmppos);
		_sceVu0Normalize(&tmpvec, &tmpvec);

		_sceVu0ScaleVector(&outvec, &tmppos, -2.0f * _sceVu0InnerProduct(&tmpvec, &tmppos));
		_sceVu0AddVector(&outvec, &outvec, &tmpvec);

		_sceVu0CopyVector((FVECTOR*)tmpmat.m[2], &outvec);

		_sceVu0OuterProduct( (FVECTOR*)tmpmat.m[1], &outvec, &tmpvec);
		_sceVu0OuterProduct( (FVECTOR*)tmpmat.m[0], (FVECTOR*)tmpmat.m[1], (FVECTOR*)tmpmat.m[2]);
	    }
	    
	    GM_SeSetMode (SD_W_RICOCH01, &to_vec, GM_SEMODE_BOMB);
	    _sceVu0CopyVector((FVECTOR*)tmpmat.m[3], &to_vec);
	    rot.vx = 0;
	    rot.vy = 0;
	    rot.vz = 0;
	    rot_wide.vx = 256;
	    rot_wide.vy = 256;
	    rot_wide.vz = 1 ;
	    color.vx = (float)BASE_R;//255.0F ;
	    color.vy = (float)BASE_G;//128.0F ;
	    color.vz = (float)BASE_B;//128.0F ;
	    color.vw = 50.0F ;
	    NewSpark2( 24,			/* 発生火花数 */
		       &tmpmat,	/* マトリクス */
		       1.0F,		/* 最小スピード */
		       200.0F,		/* 幅スピード */
		       0.0F,		/* 重力 */
		       &rot, &rot_wide,	/* 回転 回転幅 */
		       &color,		/* 色 */
		       1.0F,		/* スピードに対する火の長さの割合 */
		       15 );		/* 生存フレーム数 */

	    _sceVu0CopyVector( (FVECTOR *)(&work->prim->as_world.m[3]), (FVECTOR *)(&tmpmat.m[3]));
	    {
		FVECTOR	tmpvec = {0.0f, 0.0f, -550.0f, 1.0f};
		FMATRIX	pmat;
		pmat = work->prim->as_world;
		_sceVu0CopyVector(&to_vec, (FVECTOR*)pmat.m[3]);
		DG_SetPos(&pmat);
		DG_MovePos(&tmpvec);
		DG_GetPos(&work->prim->as_world);
	    }
	    
	    
	    
	}
	GV_DestroyActor( work );
	return;
    }
    
    if (to_vec.vy < -40000.0f){
	Work	*pwork;
	
	pwork = (Work*)GET_HAR_WORK;
	
	
	//	if (RAND(10)==0){
	if (1){
	    // 水柱   ！！！！！！
	    pwork->waterp_pos[pwork->use_pillar_num] = to_vec;
	    pwork->waterp_pos[pwork->use_pillar_num].vy = -40000.0f;
	    
	    //		work->waterp_pos[work->use_pillar_num].vx += (RAND(2001)-1000)*1.0f;
	    //		work->waterp_pos[work->use_pillar_num].vz += (RAND(2001)-1000)*1.0f;
	    //GM_SeSetMode (SD_E_GUN_WTR1, &pwork->waterp_pos[pwork->use_pillar_num], GM_SEMODE_BOMB);	    
	    //NewTestWaterPillar( &pwork->waterp_pos[pwork->use_pillar_num], 600.0f  );
	    //	NewAutoSplushVertical_Hold( &pwork->waterp_pos[pwork->use_pillar_num], 200.0f, 800.0f, 0);
	    //	NewWaterSurfaceMountain(&pwork->waterp_pos[pwork->use_pillar_num], 300.0f, 6000.0f, 0x01);
	    //	SetSplushSequence2( &pwork->waterp_pos[pwork->use_pillar_num] );
	    {
		extern void *NewSplushRotateSplush2( FVECTOR *pos, float radius, float intense, int col );
	    	NewSplushRotateSplush2(&pwork->waterp_pos[pwork->use_pillar_num], 500.0f, 300.0f, 0x80808020 );
	    }
	    
	    pwork->use_pillar_num++;
	    if (pwork->use_pillar_num >= PILLAR_MAX){
		pwork->use_pillar_num = 0;
	    }
	}
	GV_DestroyActor( work );
    }
    
    if (*work->flag_hit == GUN_FIRE_FLG_HIT){	//通常撃ち
	if ( (to_vec.vy < 4000.0f) && (-7000.0f < to_vec.vx) && (to_vec.vx < 7000.0f) )	//ある程度以上橋に近ければ
	{
	    
	    if  ( !(GM_PlayerStatus & PLAYER_ROLLING)){
		// ***********攻撃ターゲット発生
		GM_MoveOnlineTarget (&work->vul_target, &from_vec, &to_vec);
		GM_PutTarget(&work->vul_target);
	    }
	    
	}
    }
    else if (*work->flag_hit == GUN_FIRE_FLG_NOHIT){	//通常撃ち
    }
    
    if ( ++work->life > LIFE ) {	/* 寿命による解放 */
	GV_DestroyActor( work );
    }       
    _sceVu0CopyVector( (FVECTOR *)(&work->prim->as_world.m[3]), &to_vec);
    
    {
	FVECTOR	tmpvec = {0.0f, 0.0f, -550.0f, 1.0f};
	FMATRIX	pmat;
	pmat = work->prim->as_world;
	_sceVu0CopyVector(&to_vec, (FVECTOR*)pmat.m[3]);
	DG_SetPos(&pmat);
	DG_MovePos(&tmpvec);
	DG_GetPos(&work->prim->as_world);
    }

}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Vul_Die							*/
/*	引数:	Vul_Work	*work						*/
/*	説明:	死に関数	      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Vul_Die( Vul_Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int InitPacket							*/
/*	引数:	Vul_Work	*work							*/
/*	説明:	パケットの初期化      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int InitPacket( Vul_Work *work, FMATRIX *mat, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i;
	FVECTOR		vectmp;

	DG_ConfigPrim2Tex( prim, tex );		/* テクスチャ指定 */
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	/* 初期位置設定 */
	DG_COPY_MAT( &prim->as_world, mat );
	DG_COPY_VEC( &vectmp, (FVECTOR *)(&prim->as_world.m[2]) );
	_sceVu0ScaleVector( &vectmp, &vectmp, 1000.f * frnd() );
	_sceVu0AddVector( (FVECTOR *)(&prim->as_world.m[3]), (FVECTOR *)(&prim->as_world.m[3]), &vectmp );
	
	pos   = SCR_POS;
	uvrgb = SCR_UVS;

	/* XZ平面 */
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = SIZE_X;
	pos->vz = SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-SIZE_X;
	pos->vz = SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = SIZE_X;
	pos->vz =-SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-SIZE_X;
	pos->vz =-SIZE_Z;
	pos++;

	/* YZ平面 */
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vy = SIZE_Y;
	pos->vz = SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vy =-SIZE_Y;
	pos->vz = SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vy = SIZE_Y;
	pos->vz =-SIZE_Z;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vy =-SIZE_Y;
	pos->vz =-SIZE_Z;
	pos++;

	/* XY平面 */
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = SIZE_X;
	pos->vy = SIZE_Y;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-SIZE_X;
	pos->vy = SIZE_Y;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = SIZE_X;
	pos->vy =-SIZE_Y;
	pos++;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-SIZE_X;
	pos->vy =-SIZE_Y;
	pos++;

	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );

	return 1;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Vul_GetResources						*/
/*	引数:	Vul_Work	*work						*/
/*	説明:	ワークの初期化など     						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int Vul_GetResources( Vul_Work *work, FMATRIX *mat ,float har_speed, char *flg, char *flg_hit)
{
    DG_PRIM2	*prim ;
    DG_TEX		*tex ;
    
    work->life = 0;
    work->speed = har_speed;
    work->flag = flg;
    work->flag_hit = flg_hit;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
	printf("null prim\n");
	return -1;
    }

    work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;

    tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );

    {
	
	FMATRIX	tmpmat;
	SVECTOR	tmprot={0,0,0,0};

	if (*flg == GUN_FIRE_FLG_DELAY){	//遅れ狙い

	    //difficult
	    if (GM_GameLevel == GM_LEVEL_HARD){
		tmprot.vx += RAND(51)-25-10;
		tmprot.vy += RAND(51)-25;
	    }
	    else if (GM_GameLevel >= GM_LEVEL_EXTREME){
		tmprot.vx += RAND(31)-15-10;
		tmprot.vy += RAND(31)-15;
	    }
	    else {
		tmprot.vx += RAND(31)-15-10;
		tmprot.vy += RAND(31)-15;
	    }

	    DG_SetPos( mat );
	    DG_RotatePos( &tmprot );
	    DG_GetPos( &tmpmat );
	}
	else if (*flg == GUN_FIRE_FLG_NORM){	//通常撃ち
	    Dir_from_2Vec((FVECTOR*)mat->m[3], &GM_PlayerFindPos, &tmprot);
	    //	    tmprot.vx += RAND(25)-12-18;
	    //	    tmprot.vy += RAND(25)-12-9;
	    tmprot.vx += -18;
	    tmprot.vy +=  -9;
	    DG_SetPos( mat );
	    DG_RotatePos( &tmprot );
	    DG_GetPos( &tmpmat );
	}
	else if (*flg == GUN_FIRE_FLG_AIM){	//狙い撃ち
	    int	far_y;
	    FVECTOR	shift;
	    _sceVu0SubVector(&shift, (FVECTOR*)mat->m[3], &GM_PlayerFindPos);



	    far_y = (int)_FVecLen2(&shift);


	    Dir_from_2Vec((FVECTOR*)mat->m[3], &GM_PlayerFindPos, &tmprot);	    

	    if (far_y > 50000){
		tmprot.vx += (far_y - 55000)*4/1000;
		if (tmprot.vx > 1000){
		    tmprot.vx = 1000;
		}
	    }

	    tmprot.vz += RAND(4096);
    	    DG_SetPos2( (FVECTOR*)mat->m[3],&tmprot );
	    tmprot.vx = 3;
	    tmprot.vy = 3;
	    tmprot.vz = 0; 


	    


	    DG_RotatePos( &tmprot );	    
	    DG_GetPos( &tmpmat );
	}






	InitPacket( work, &tmpmat, prim, tex );
	{
	    int	map = 0;
	    FVECTOR	size = {1.0f, 1.0f, 1.0f, 1.0f};
	    FVECTOR	force = {0.0f, 0.0f, 300.0f, 1.0f};

	    *((FVECTOR*)tmpmat.m[3]) = DG_ZeroVector;
	    DG_SetPos(&tmpmat);
	    DG_MovePos(&force);
	    DG_GetPos(&tmpmat);

	    force = *((FVECTOR*)tmpmat.m[3]);



	    /*攻撃ターゲット設定*/
	    GM_SetTarget( &work->vul_target,
			  TARGET_OFFENSE|TARGET_ONLINE,
			  map, PLAYER_SIDE,
			  &size, &DG_ZeroVector ) ;
	    GM_SetTargetWeaponType( &work->vul_target,
				    WP_GRENADE	/* グレネード */);
	    {
		int damage = 6;
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	damage = 3;
	break;
    case ST_LEV_EASY:
	damage = 5;
	break;
    case ST_LEV_NORMAL:
	damage = 6;
	break;

    case ST_LEV_UPNORM:
	damage = 10;
	break;

    case GM_LEVEL_HARD:
	damage = 14;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	damage = 18;
	break;
    }

	    GM_SetPowerTarget( &work->vul_target,
			       &work->vul_pow_tgt,
			       POWER_ONCE,
			       50,
			       6,
			       damage,
			       &force );

	    }
	    GM_SetTargetCallBack( &work->vul_target,
				  (void*)dumfunc,
				  NULL);
	    
	}
    }
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void *NewHarGunBulet						*/
/*	引数:	Vul_Work	*work						*/
/*	説明:	エントリー関数	      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void *NewHarGunBulet( FMATRIX *mat, float speed, char *flg, char *flg_hit)
{
	Vul_Work		*work ;

	work = (Vul_Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Vul_Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Vul_Act, Vul_Die ) ;
		GV_ActorEX( &work->actor )
		if ( Vul_GetResources( work, mat, speed, flg, flg_hit) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

