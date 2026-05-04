/********************************************************************************/
/*	cypher.c								*/
/*	サイファ	*NewCypher						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cyp_target.c,v 1.1.1.3 2002/11/19 11:48:15 Yoshizawa1 Exp $			*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
/********************************************************************************/
/*	extern									*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#ifdef KP_XBOX	// サイファ音は遮蔽計算なしにする
#define	GM_SeSetFromVolCurve( a, b, c ) GM_SeSetFromVolCurveAddr( a, b, c, GM_INVALID_ADDR )
#define GM_SeSetMode( a, b, c ) GM_SeSetModeAddr( a, b, c, GM_INVALID_ADDR )
#endif


extern void *NewCrushWithForce( FVECTOR * );
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	ChildTargCallBack_Body					*/	
/*	引数:	TARGET		*off					       	*/
/*		TARGET		*def						*/
/*		void		*ptr						*/
/*	説明:	ターゲットコールバック関数	****  ボディ  ****		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	ChildTargCallBack_Body( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;
    work = ( Work * )ptr ;


    
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
	    if (def->weapon_type & WP_NOPLAYER){
		work->is_player_atack = OFF;
	    }
	    else {
		work->is_player_atack = ON;
	    }
	    def->power->force = off->power->force ;
	    if( def->weapon_type & WP_BULLET){
		FMATRIX	tmpmat;
		FMATRIX	tmpmat2;
		SVECTOR	tmprot;
		
		_FVecToRotXY( &off->power->force, &tmprot ) ;
		
		work->damage_force.vx = off->power->force.vx/5.5f;
		work->damage_force.vy = off->power->force.vy/5.5f;
		work->damage_force.vz = off->power->force.vz/5.5f;
		
		if( def->weapon_type & WP_PSG1){//ＰＳＧ１は５倍のforceを持っている
		    work->damage_rot.vz = -off->power->force.vx*1.6f*(RAND(7)+7)/55;
		    work->damage_rot.vx =  off->power->force.vz*1.6f*(RAND(7)+7)/55;
		}
		else {
		    work->damage_rot.vz = -off->power->force.vx*1.6f*(RAND(7)+7)/10;
		    work->damage_rot.vx =  off->power->force.vz*1.6f*(RAND(7)+7)/10;
		}
		
		DG_SetPos2( &def->hit, &tmprot ) ;
		DG_GetPos( &tmpmat ) ;
		
		NewSpark( &tmpmat ) ;
		
		NewCrushWithForce(&def->hit);
		
		printf ("Call hit se\n");
		GM_SeSetMode( SD_E_CYPHDMG1, &def->hit, GM_SEMODE_BOMB);
		
		
		
		if ( def->power->vital <= 0 ) {		    /*破壊された*/
		    CypherBrake( work, 0 );
		    return;
		}
		
		def->power->vital = def->power->vital - off->power->damage; 
		
		if ( (def->power->vital<=0)||(GM_GameLevel==GM_LEVEL_VERYEASY) ) {
		    work->dmg_flag = 1;
		    SV_G1_STEP(work, CYP_DAMAGE);
		    work->ACTION_LAYER = EYE_LAYER_BREAK;
		    work->tuiraku_timer = 0;
		    _sceVu0InversMatrix(&tmpmat2, &work->body.objs->objs[0].world);	
		    _sceVu0MulMatrix(&work->damage_shift, &tmpmat2, &tmpmat );	
		    
		    _sceVu0MulMatrix(&work->damage_pos, &work->body.objs->objs[0].world, &work->damage_shift);	
		    
		    Dir_from_2Vec(&GM_PlayerFindPos, &work->control.mov, &work->nigeru_dir); 
		    
		    work->nigeru_dir.vx = work->nigeru_dir.vz = 0; 
		    
		    _sceVu0SubVector(&work->nigeru_vec, &work->control.mov, &GM_PlayerFindPos); 
		    _sceVu0Normalize(&work->nigeru_vec, &work->nigeru_vec);	
		    _sceVu0ScaleVector(&work->nigeru_vec, &work->nigeru_vec, 160.0f);
		    work->nigeru_vec.vy = 0.0f;
		    def->power->vital = 0;
		}
	    }
	    else if( def->weapon_type & WP_BLAST){
		if ( def->power->vital <= 0 ) {	//複数コールを阻止
		    return;
		}
		/*爆発系ヒット*/
		def->power->vital = def->power->vital - off->power->damage; 
		if ( def->power->vital <= 0 ) {
		    /*破壊された*/
		    CypherBrake( work, 0 );
		}
	    }
	    

	    else if( def->weapon_type & WP_M92 ){
		if(off->power != NULL ) {
		    CallSpark(&def->hit ,&off->power->force, work);
		}
	    }


	}
    }
    def->weapon_type = 0 ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	ChildTargCallBack_Head					*/
/*	引数:	TARGET	*off   						       	*/
/*		TARGET	*def							*/
/*		void	*ptr							*/
/*	説明:	ターゲットコールバック関数	****  頭  ****			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void	ChildTargCallBack_Head( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;
    work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
	    if (def->weapon_type & WP_NOPLAYER){
		work->is_player_atack = OFF;
	    }
	    else {
		work->is_player_atack = ON;
	    }
	    def->power->force = off->power->force ;
	    if(( def->weapon_type & WP_BULLET)){
		
		//仮
		CallSpark(&def->hit ,&off->power->force, work);
		NewCrushWithForce(&def->hit);
		
		def->power->vital = def->power->vital - off->power->damage; 
		
		if ( def->power->vital <= 0 ) {
		    /*破壊された*/
		    CypherBrake( work, 0 );
		}
	    }

	    // ************** Satoyoshi Syu-sei 2001.09.05
	    else if( def->weapon_type & WP_BLAST){
		if ( def->power->vital <= 0 ) {	//複数コールを阻止
		    return;
		}
		/*爆発系ヒット*/
		def->power->vital = def->power->vital - off->power->damage; 
		if ( def->power->vital <= 0 ) {
		    /*破壊された*/
		    CypherBrake( work, 0 );
		}
	    }
	    // ****************

	}
	else if( def->weapon_type & WP_M92 ){
	    if(off->power != NULL ) {
		CallSpark(&def->hit ,&off->power->force, work);
	    }
	}	
    }
    def->weapon_type = 0 ;
}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	ChildTargCallBack_Roat					*/
/*	引数:	TARGET	*off   						       	*/
/*		TARGET	*def							*/
/*		void	*ptr							*/
/*	説明:	ターゲットコールバック関数	****  ローター  ****		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void	ChildTargCallBack_Roat( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;
    work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
    if (def->weapon_type & WP_NOPLAYER){
	work->is_player_atack = OFF;
    }
    else {
	work->is_player_atack = ON;
    }
	    if( def->weapon_type & WP_BULLET){
		
		CallSpark(&def->hit ,&off->power->force, work);
		NewCrushWithForce(&def->hit);
		/*破壊された*/
		CypherBrake( work, 0 );
	    }
	}
    }
    def->weapon_type = 0 ;
}


/*胴体アタリサイズ*/
#define	BODY_SIZE_X	(400.0F)
#define	BODY_SIZE_Z	(250.0F)
#define	BODY_SIZE_Y	(200.0F)
#define	BODY_SIZE_GUN_Y	(150.0F)

/*カメラアタリサイズ*/
#define	HEAD_SIZE_X		(190.0F)
#define	HEAD_SIZE_Y		(100.0F)
#define	HEAD_SIZE_Z		(200.0F)

/*GUNアタリサイズ*/
#define	GUN_SIZE_X		(100.0F)
#define	GUN_SIZE_Y		(130.0F)
#define	GUN_SIZE_Z		(550.0F)

/*ローターあたりサイズ*/
#define	ROAT_SIZE_XZ		(150.0F)
#define	ROAT_SIZE_Y		(200.0F)
#define	ROAT_SIZE_GUN_Y		(150.0F)

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetTarget							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ターゲット(当たり判定)の設定					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetTarget(Work *work)
{
    int		flag, map;
    short	loop;

    /*ちょっと厚めにしとく : あたりサイズ 厚み 高さ 横幅 の順*/
    FVECTOR	b_size = { BODY_SIZE_X, BODY_SIZE_Y, BODY_SIZE_Z } ;
    FVECTOR	r_size = { ROAT_SIZE_XZ, ROAT_SIZE_Y, ROAT_SIZE_XZ } ;
    FVECTOR	h_size;

    switch (work->type){	// **サイファのタイプで分岐**
        case NORMAL_TYPE :	//ノーマルサイファ
	    h_size.vx = HEAD_SIZE_X;
	    h_size.vy = HEAD_SIZE_Y;
	    h_size.vz = HEAD_SIZE_Z;
	    break ;

        case GUN_TYPE :	//GUNサイファ
	    h_size.vx = GUN_SIZE_X;
	    h_size.vy = GUN_SIZE_Y;
	    h_size.vz = GUN_SIZE_Z;
	    b_size.vy = BODY_SIZE_GUN_Y;
	    r_size.vy = ROAT_SIZE_GUN_Y;
	    break ;
    }

    /*ターゲット設定*/
    flag =(TARGET_DEFENSE|TARGET_SEEK
	   |TARGET_ROTATE|TARGET_NO_CLAYMORE
	   |TARGET_POWER);
    map = work->control.map;

    //ドーナツ部分設定
    for (loop=0; loop<6; loop++){
	GM_SetTarget( &work->b_trg[loop], flag,map, ENEMY_SIDE, 
		      &b_size, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( &work->b_trg[loop], ChildTargCallBack_Body, work ) ;
	GM_SetTargetWeaponType(&work->b_trg[loop],0);
	GM_SetPowerTarget( &work->b_trg[loop], &work->b_power,
			   POWER_DECREASE, CYP_BODY_VITAL, 0, 0, &DG_ZeroVector );
	GM_PutTarget( &work->b_trg[loop] );
    }

    //カメラ部分設定
    GM_SetTarget( &work->h_trg, flag,map, ENEMY_SIDE, 
		  &h_size, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->h_trg, ChildTargCallBack_Head, work ) ;
    GM_SetTargetWeaponType(&work->h_trg,0);
    GM_SetPowerTarget( &work->h_trg, &work->h_power,
		       POWER_DECREASE, CYP_HEAD_VITAL, 0, 0, &DG_ZeroVector );
    GM_PutTarget( &work->h_trg );

    flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_LOCKON
	   |TARGET_ROTATE|TARGET_NO_CLAYMORE
	   |TARGET_POWER);

    //ローター部分設定
    GM_SetTarget( &work->r_trg, flag,map, ENEMY_SIDE, 
		  &r_size, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->r_trg, ChildTargCallBack_Roat, work ) ;
    GM_SetTargetWeaponType(&work->r_trg,0);
    GM_PutTarget( &work->r_trg );
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void MoveTarget							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ターゲットを移動する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void MoveTargets(Work *work)
{
    FMATRIX  mat;
    FVECTOR	tmpvec = {0.0f,-55.0f,0.0f,0.0f};
    short	long_x;
    short	short_x;
    short	short_z;

    SVECTOR	rot1 = {0, 1024,  0,1};
    SVECTOR	rot2 = {0, 1028/3,0,1};
    SVECTOR	rot3 = {0,-1028/3,0,1};

    long_x = 710;
    short_x = long_x/2;
    short_z = short_x*1732/1000;	

    /*		****ボディのターゲット****	*/
    //ボディのターゲット １
    if (work->type == GUN_TYPE){
	tmpvec.vy = 10.0f;
    }
    tmpvec.vx = long_x;
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &tmpvec ) ;
    DG_RotatePos( &rot1 );
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->b_trg[0], &mat,
		       work->control.map);
    //ボディのターゲット 
    tmpvec.vx = -long_x;
    _sceVu0CopyMatrix(&mat, &work->body.objs->objs[0].world);	
    DG_SetPos( &mat ) ;
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &tmpvec ) ;
    DG_RotatePos( &rot1 );
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->b_trg[1], &mat,
		       work->control.map);
    //ボディのターゲット 
    tmpvec.vx = short_x;
    tmpvec.vz = short_z;
    _sceVu0CopyMatrix(&mat, &work->body.objs->objs[0].world);
    DG_SetPos( &mat ) ;
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &tmpvec ) ;
    DG_RotatePos( &rot2 );
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->b_trg[2], &mat,
		       work->control.map);
    //ボディのターゲット 
    tmpvec.vz = -short_z;
    _sceVu0CopyMatrix(&mat, &work->body.objs->objs[0].world);
    DG_SetPos( &mat ) ;
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &tmpvec ) ;
    DG_RotatePos( &rot3 );
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->b_trg[4], &mat,
		       work->control.map);
    //ボディのターゲット 
    tmpvec.vx = -short_x;
    _sceVu0CopyMatrix(&mat, &work->body.objs->objs[0].world);
    DG_SetPos( &mat ) ;
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &tmpvec ) ;
    DG_RotatePos( &rot2 );
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->b_trg[3], &mat,
		       work->control.map);
    //ボディのターゲット 
    tmpvec.vz = short_z;
    _sceVu0CopyMatrix(&mat, &work->body.objs->objs[0].world);
    DG_SetPos( &mat ) ;
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &tmpvec ) ;
    DG_RotatePos( &rot3 );
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->b_trg[5], &mat,
		       work->control.map);

    //ヘッドのターゲット 
    tmpvec.vx = 0.0f;

    if (work->type == GUN_TYPE){
	tmpvec.vy = -210.0f;
    }
    else {
	tmpvec.vy = 40.0f;
    }
    tmpvec.vz = -30.0f;
    DG_SetPos( &work->body.objs->objs[1].world);
    DG_MovePos( &tmpvec ) ;
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->h_trg,
		       &mat,work->control.map);

    //ロータのターゲット 
    tmpvec.vx = 0.0f;
    if (work->type == GUN_TYPE){
	tmpvec.vy = 30.0f;
    }
    else {
	tmpvec.vy = -10.0f;
    }
    tmpvec.vz = 0.0f;
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &tmpvec ) ;
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->r_trg,
		       &mat,work->control.map);
}




