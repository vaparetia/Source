/********************************************************************************/
/*	Sv_camera.c								*/
/*	監視カメラ  *NewSvCamera						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cam_target.c,v 1.1.1.3 2002/11/19 11:48:13 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	include files								*/
/********************************************************************************/

#include "BP_TrophyLogicMGS2.h"

/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern void *NewSpark( FMATRIX   *world ) ;
extern void *NewCrushWithForce( FVECTOR *pos, FVECTOR *force );
extern void *NewFlyingSmoke2( FVECTOR * , FVECTOR * , int );
extern int EMA_GunCameraBroken() ;

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define	SE_START	(TIME_BASE*150)	/*破壊後エフェクト用カウンタ*/


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CameraCrash						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*pos							*/
/*		FVECTOR *force							*/
/*	説明:	カメラ壊れた	      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Break_camera_block_ex(Work *work, int type){
    GCL_ARGS		args;
    int			buf[2];
    
    args.argc = 2 ;
    args.argv = buf;
    buf[0] = work->name;
    buf[1] = type;
    
    if(work->exec != 0){
	GM_ExecBlock( ( char * )work->exec, &args ) ;
	printf ("Exec camera broken block\n");
    }
    
    COM_SetAccident(work->com_uniq_id, &work->ctrl.mov,
		    work->ctrl.hzx_id, ACCIDENT_DELAY_TIME,
		    ENE_ACCIDENT_MECA_BREAK);
    
    if (work->is_player_atack == ON){
	GM_MecaKillCount ++;
	if ( GM_MecaKillCount > GM_MAX_RESULT_COUNT ) GM_MecaKillCount = GM_MAX_RESULT_COUNT ;

	if (work->camera_type == GUN){
	    EMA_GunCameraBroken();
	}

    }    
    
    printf ("Camera broken %d \n", type);
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CallSpark							*/
/*	引数:	FVECTOR *pos							*/
/*	引数:	FVECTOR *force							*/
/*	説明:	火花を呼ぶ	       						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CallSpark(FVECTOR *pos ,FVECTOR *force) {
    SVECTOR	rot ;
    FMATRIX	w ;
    
    _FVecToRotXY( force, &rot ) ;
    DG_SetPos2( pos, &rot ) ;
    DG_GetPos( &w ) ;
    NewSpark( &w ) ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CameraCrash						*/
/*	引数:	Work	*work							*/
/*	説明:	カメラ壊れた	      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CameraCrush(Work *work){
    
    /* 破壊エフェクト */
    GM_SeSetMode( SD_A_CAMGLS01,
		  (FVECTOR*)work->body.objs->objs[1].world.m[3], GM_SEMODE_BOMB );	/* レンズ割れ */
    NewCrushWithForce( (FVECTOR*)work->body.objs->objs[1].world.m[3], &DG_ZeroVector );
    
    SV_G1_STEP(work, MOD_BROKEN);	//壊れモードに移行
    
    
    work->eye.alert_time = 0 ;
    work->se_cnt = SE_START;
    work->rctrl.flag &= (~RADAR_VISIBLE);
    
    DG_ChangeModelObj( work->body.objs, &work->body.objs->objs[1], work->broken, &work->broken->models[1] ) ;
    
    Break_camera_block_ex(work, SV_BREAK_CRUSH);

    bp_trophy_destroyed_camera();
}


#define	MAX_VEC		(150.0f)
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CameraVanish						*/
/*	引数:	Work	*work							*/
/*	説明:	カメラ爆散	      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CameraVanish(Work *work){
    
    FVECTOR		fvtemp;
    SVECTOR		svtemp;
    short	i;
    
    for(i=0; i<2; i++){
	fvtemp.vx = 0.0f;
	fvtemp.vy = rnd()*MAX_VEC*0.5f + MAX_VEC*0.5f;
	fvtemp.vz = 0.0f;
	svtemp.vx = irnd()%2048;
	svtemp.vy = irnd()%4096;
	svtemp.vz = 0;
	DG_SetPos2( &DG_ZeroVector, &svtemp );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
	NewFlyingSmoke2( (FVECTOR*)work->body.objs->objs[1].world.m[3], &fvtemp, 20 );
    }
    
    work->body.objs->objs[1].flag |= DG_FLAG_INVISIBLE;
    work->h_trg.class |= TARGET_SKIP; 
    work->lens_trg.class |= TARGET_SKIP; 
    
    work->eye.alert_time = 0 ;
    work->se_cnt = 0;
    work->rctrl.flag &= (~RADAR_VISIBLE);
    
    work->h_trg.class &= ~TARGET_LOCKON; 
    
    DG_ChangeModelObj( work->body.objs, &work->body.objs->objs[0], work->broken, &work->broken->models[0] ) ;
    
    Break_camera_block_ex(work, SV_BREAK_VANISH);
    
    SV_G1_STEP(work, MOD_BROKEN);	//壊れモードに移行
    
    bp_trophy_destroyed_camera();
}









/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	ChildTargCallBack_muteki					*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void*	*ptr							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	ChildTargCallBack_muteki( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
	    if( def->weapon_type & (WP_BULLET|WP_M92|WP_PSG1T) )//これらの攻撃タイプで破壊
	    {
		if(off->power != NULL ) {
		    CallSpark(&def->hit ,&off->power->force);
		    GM_SeSetMode( SD_W_RICOCH01, &work->camera_pos, GM_SEMODE_BOMB ) ;
		}
	    }
	}
    }
    def->weapon_type = 0 ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	ChildTargCallBack_break					*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void*	*ptr							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	ChildTargCallBack_break( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
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
	    if( def->weapon_type & (WP_BULLET|WP_M92|WP_PSG1T) )//これらの攻撃タイプで破壊
	    {
		// *****銃弾ヒット カメラ壊れ
		if( ( def->weapon_type & (WP_BULLET))&&(G1_STEP != MOD_BROKEN) ){
		    work->SV_Break_Flag = SV_BREAK_CRUSH;
		}
		// ****跳弾
		else if(off->power != NULL ) {
		    CallSpark(&def->hit ,&off->power->force);
		    GM_SeSetMode( SD_W_RICOCH01, &work->camera_pos, GM_SEMODE_BOMB ) ;
		}
	    }
	    // *****爆発物ヒット カメラ爆散
	    else if( ( def->weapon_type & (WP_BLAST))&&(G1_STEP != MOD_BROKEN) ){
		work->SV_Break_Flag = SV_BREAK_VANISH;
	    }
	}
    }
    def->weapon_type = 0 ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	ChildTargCallBack_break					*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void*	*ptr							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	ChildTargCallBack_break2( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
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
	    
	    
	    if( def->weapon_type & (WP_BULLET|WP_M92|WP_PSG1T) )//これらの攻撃タイプで破壊
	    {
		CallSpark(&def->hit ,&off->power->force);
		GM_SeSetMode( SD_W_RICOCH01, &work->camera_pos, GM_SEMODE_BOMB ) ;
		
	    }
	    // *****爆発物ヒット カメラ爆散
	    else if( ( def->weapon_type & (WP_BLAST))&&(G1_STEP != MOD_BROKEN) ){
		work->SV_Break_Flag = SV_BREAK_VANISH;
	    }
	}
    }
    def->weapon_type = 0 ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	ChildTargCallBack_Lens					*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void*	*ptr							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	ChildTargCallBack_week( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
	    if( def->weapon_type & (WP_BULLET|WP_M92|WP_BLAST) )//これらの攻撃タイプで破壊
	    {
		if (def->weapon_type & WP_NOPLAYER){
		    work->is_player_atack = OFF;
		}
		else {
		    work->is_player_atack = ON;
		}
		// *****爆発物ヒット カメラ爆散
		if( ( def->weapon_type & (WP_BLAST))&&(G1_STEP != MOD_BROKEN) ){
		    work->SV_Break_Flag = SV_BREAK_VANISH;
		}
		// *****銃弾ヒット カメラ壊れ
		else if(G1_STEP != MOD_BROKEN){
		    work->SV_Break_Flag = SV_BREAK_CRUSH;
		}
		// ****跳弾
		else if(off->power != NULL ) {
		    CallSpark(&def->hit ,&off->power->force);
		    GM_SeSetMode( SD_W_RICOCH01, &work->camera_pos, GM_SEMODE_BOMB ) ;
		}
	    }
	}
    }
    def->weapon_type = 0 ;
}

/*ターゲット設定*/
#define HEAD_SIZE_XY (50.0F)
#define HEAD_SIZE_Z (120.0F)
#define LENS_SIZE_Z (40.0F)
#define HEAD_SHIFT_Y (70.0F)
#define HEAD_SHIFT_Z (40.0F)

#define PHEAD_SIZE_X (80.0F)
#define PHEAD_SIZE_Y (60.0F)
#define PHEAD_SIZE_Z (160.0F)
#define PLENS_SIZE_XY (30.0F)
#define PLENS_SIZE_Z (130.0F)


#define	TARGET_SETUP( _target, _x, _y, _z, _sx, _sy, _sz )\
{\
    targ_size.vx = (_x);						\
    targ_size.vy = (_y);						\
    targ_size.vz = (_z);						\
    targ_shift.vx = (_sx);						\
    targ_shift.vy = (_sy);						\
    targ_shift.vz = (_sz);						\
    GM_SetTarget( (_target), flag,map, ENEMY_SIDE, 			\
		  &targ_size, &targ_shift );				\
    GM_SetTargetCallBack( (_target), CB_func, work );	\
    GM_SetTargetWeaponType( (_target) ,0);			       	\
    GM_PutTarget( (_target) );						\
}

//   NewTargetView ( (_target), 250, 250, 250);			\

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SVC_TargetSet						*/
/*	引数:	Work	*work							*/
/*	説明:	やられ判定をターゲットに登録					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SVC_TargetSet(Work *work)
{
    int flag,map;
    /*あたりサイズ 厚み 高さ 横幅 の順*/
    FVECTOR	targ_size;
    FVECTOR	targ_shift;
    TARGET_CALLBACK	CB_func;

	/*ターゲット設定*/
    flag =(TARGET_DEFENSE|TARGET_SEEK|
	   TARGET_POWER|TARGET_ROTATE|TARGET_LOCKON);
    map = work->ctrl.map;

    // ================================通常カメラ
    if (work->camera_type == NORMAL){

	CB_func = ChildTargCallBack_break;
	TARGET_SETUP(&work->h_trg, 
		     HEAD_SIZE_XY,HEAD_SIZE_XY,HEAD_SIZE_Z,
		     0.0F,HEAD_SHIFT_Y,HEAD_SHIFT_Z ) ;

	flag =(TARGET_DEFENSE|TARGET_SEEK
	       |TARGET_POWER|TARGET_ROTATE);

	CB_func = ChildTargCallBack_break;
	TARGET_SETUP(&work->lens_trg, 
		     HEAD_SIZE_XY,HEAD_SIZE_XY,LENS_SIZE_Z,
		     0.0F,HEAD_SHIFT_Y,175.0F ) ;

	CB_func = ChildTargCallBack_muteki;
	TARGET_SETUP(&work->pilor_trg, 
		     17.0f, 40.0f, 100.0f,
		     0.0F,  15.0f, 80.0f ) ;

    }
    // ================================ガンカメラ
    else if (work->camera_type == GUN){
	CB_func = ChildTargCallBack_break2;
	TARGET_SETUP(&work->h_trg, 
		     20.0f, 30.0f, 550.0f,
		     0.0F, 230.0f, 80.0f ) ;

	flag =(TARGET_DEFENSE|TARGET_SEEK
	       |TARGET_POWER|TARGET_ROTATE);

	TARGET_SETUP(&work->bar_trg, 
		     20.0f, 40.0f, 250.0f,
		     0.0F, -20.0f, 260.0f ) ;

	CB_func = ChildTargCallBack_break;
	TARGET_SETUP(&work->lens_trg, 
		     32.0f, 32.0f, 150.0f,
		     -56.0F, 185.0f, 400.0F ) ;

	TARGET_SETUP(&work->mag_trg, 
		     144.0f, 74.0f, 50.0f,
		     0.0F, 140.0f, 100.0f ) ;

	CB_func = ChildTargCallBack_muteki;
	TARGET_SETUP(&work->pilor_trg, 
		     50.0f, 70.0f, 300.0f,
		     0.0F, -30.0f, 320.0f ) ;
    }
    else if (work->camera_type == PLANT){

	CB_func = ChildTargCallBack_break;
	TARGET_SETUP(&work->h_trg, 
		     75.0f, 55.0f, 140.0f,
		     0.0F, 60.0f, 30.0f ) ;

	flag =(TARGET_DEFENSE|TARGET_SEEK
	       |TARGET_POWER|TARGET_ROTATE);

	CB_func = ChildTargCallBack_break;
	TARGET_SETUP(&work->lens_trg, 
		     30.0f, 60.0f, 70.0f,
		     0.0F, 25.0f, 240.0F ) ;

	CB_func = ChildTargCallBack_muteki;
	TARGET_SETUP(&work->pilor_trg, 
		     30.0f, 45.0f, 140.0f,
		     0.0F,  5.0f, 130.0f ) ;

    }

#if 0
    NewTargetView( &work->h_trg, 0, 0, 255 ) ;
    NewTargetView( &work->pilor_trg, 0, 0, 255 ) ;
    NewTargetView( &work->lens_trg, 255, 0, 0 ) ;
    NewTargetView( &work->bar_trg, 0, 0, 255 ) ;
    NewTargetView( &work->mag_trg, 255, 0, 0 ) ;
#endif
}

