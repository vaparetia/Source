/********************************************************************************/
/*	cam_act.c								*/
/*	監視カメラ  色々な実行関数						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cam_act.c,v 1.1.1.3 2002/11/19 11:48:13 Yoshizawa1 Exp $			*/
/********************************************************************************/
/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#include "BP_Misc.h"
/********************************************************************************/
/*	extern									*/
/********************************************************************************/
/********************************************************************************/
/*	define      								*/
/********************************************************************************/

#define CHF_CYC	(work->chaff_time)	/*チャフ時揺れ周期*/

#define	SV_DISCV_DELAY		(90*5)	//	捉えてから通報までの時間差
#define	SV_HMARK_DELAY		(60*5)	//	捉えてからビックリまで*/
#define	SV_ALERT_TIME		(120*5)	//	捉えてからビックリまで*/

#define CALL_HMARK_DELAY	(30*5)	//
#define	SV_CALL_DELAY		(120*5)	//
#define	SV_CALL_END_TIME	(300*5)	//


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	CheckDirSub							*/
/*	引数:	int dir1							*/
/*		int dir2							*/
/*	説明:	角度の差を得る	       						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int CheckDirSub(int dir1, int dir2){
    int sub;
    sub = dir2 - dir1;
    sub	&= 4095;
    if(sub >= 2048) sub= -(4096-sub);
    return sub;
}	

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SV_ActChaff(Work *work){

    work->ctrl.turn.vx &=4095 ;
    work->ctrl.turn.vy &=4095 ;

    if(work->chf_cnt >= CHF_CYC){	//チャフサイクル(CHF_CYC)毎に振舞い変化
	int	tmp;
	int	save_tmp;

	work->chf_cnt = work->chf_cnt-CHF_CYC;
	CHF_CYC = (RAND(51)+10)*5;

	work->chf_cnt = 0;
	save_tmp = work->rnd_speed.vx;
	work->rnd_speed.vx = (irnd()&21)-10;

   if ( BP_IsPAL()==TRUE )
   	work->rnd_speed.vx = work->rnd_speed.vx*1.2f;

	tmp = (int)abs((save_tmp - work->rnd_speed.vx));

	save_tmp = work->rnd_speed.vy;
	work->rnd_speed.vy = (irnd()&21)-10;

   if ( BP_IsPAL()==TRUE )
   	work->rnd_speed.vy = work->rnd_speed.vy*1.2f;

	tmp += (int)abs((save_tmp - work->rnd_speed.vy));

	if((irnd()&1)) work->rnd_speed.vx *= -1 ;
	if((irnd()&1)) work->rnd_speed.vy *= -1 ;

	if (tmp > 13){
	    GM_SeSetFromVolCurve( work->chaf_se_num ,&work->ctrl.mov, &svc_curves);
	}
    }

    /*LIMIT*/
    work->ctrl.turn.vx += work->rnd_speed.vx ;
    work->ctrl.turn.vy += work->rnd_speed.vy ;

    work->ctrl.turn.vz = 0 ;
    work->chf_cnt+=TIME_BASE;		 

    //とりあえずランダム
    SET_COLOR_CAM(irnd()&127 ,irnd()&127 ,irnd()&127 ,127, &work->rgba);

    {
	static int chaff_timer;
	if (GM_CheckGameStatus(STATE_CHAFF)){
	    chaff_timer = 120*5;
	}
	else {
	    chaff_timer -= TIME_BASE;
	    if (chaff_timer <= 0){
		SV_G1_STEP(work, MOD_NORMAL);
		work->ACTION_LAYER = EYE_LAYER_Normal;
	    }
	}
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void red_bikkuri_set						*/
/*	引数:	Work *work   						       	*/
/*	説明:	赤びっくりマークをセットする		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void red_bikkuri_set(Work *work){
	if ( !GM_CheckGameStatus( STATE_GAMEOVER ) && !( GM_VRStatus & GM_VR_CLEAR ) ) {
	    GM_SeSetMode( SD_E_BIKKRI01 , work->eye.eyepos,
			  GM_SEMODE_BOMB );
	    CallActHeadMarks( work->hmk_work_p, HMK2_TYPE_RED_AT_MIN);
	    COM_DetectVibration( );
	}
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActSearch						*/
/*	引数:	Work *work   						       	*/
/*	説明:	プレイヤ探し挙動の制御を行なう		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SV_ActSearch(Work *work){

    int dirsub ;

    dirsub = CheckDirSub(work->now_dir , work->alert_max_dir) ;
    switch(work->mode){

    case RIGHT_TURN :	// *****右回転
	dirsub = work->alert_max_dir - work->now_dir ;
	if(work->wait_cnt > 0){
	    work->wait_cnt -= TIME_BASE ;
	}else if ( (dirsub > TURN_SPEED*2) ){
	    work->now_dir += TURN_SPEED*2 ;
        } else {
	    work->wait_cnt = SV_WAIT_TIME/2 ;
	    work->mode = LEFT_TURN ;
	}
	break;

    case LEFT_TURN :	// *****左回転
        dirsub = (-work->alert_max_dir) - work->now_dir ;
	if(work->wait_cnt > 0){
	    work->wait_cnt -= TIME_BASE ;
        }else if( (abs(dirsub) > TURN_SPEED*2 ) ){
	    work->now_dir -= TURN_SPEED*2 ;
	}else {
	    work->wait_cnt = SV_WAIT_TIME/2 ;
	    work->mode = RIGHT_TURN ;
	}
        break;
    }
    work->ctrl.turn.vx = work->rot.vx ;
    work->ctrl.turn.vy = work->center+work->now_dir ;
    work->ctrl.turn.vz = 0 ;

}

extern void *NewLineSmoke( FMATRIX *,FVECTOR *,int ,float ,float  ) ;
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Call_spark							*/
/*	引数:	Work	worl							*/
/*	説明:	やられ時のスパークなどを呼ぶ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Call_spark(Work *work){
    static FVECTOR tempvec0 = {90.0f,50.0f,150.0f,1.0f};
    static FVECTOR tempvec1 = {-80.0f,40.0f, -30.0f,1.0f};

    FVECTOR pos, force;
    int dir ;
    if(work->se_cnt <= 0){
	SET_COLOR_CAM(0 ,0 ,0 ,0, &work->rgba);	//ライトを消す
	return ;
    }
    if(work->se_cnt == (SE_START-3*TIME_BASE)){		/* 本体破壊音 */
	GM_SeSetMode( SD_E_ECODBR01, &work->camera_pos, GM_SEMODE_BOMB );


	if (work->camera_type == NORMAL){
	    tempvec0.vx = 63;
	    tempvec1.vx = -58;
	}

	GV_SetActorChild( work, NewLineSmoke( &work->body.objs->objs[CAMERA_PARTS].world , &tempvec0 , 6000, 25.0f, 10.0f) );
	GV_SetActorChild( work, NewLineSmoke( &work->body.objs->objs[CAMERA_PARTS].world , &tempvec0 , 6000, 25.0f, 10.0f) );
	GV_SetActorChild( work, NewLineSmoke( &work->body.objs->objs[CAMERA_PARTS].world , &tempvec0 , 6000, 25.0f, 10.0f) );
	GV_SetActorChild( work, NewLineSmoke( &work->body.objs->objs[CAMERA_PARTS].world , &tempvec1 , 6000, 25.0f, 10.0f) );
	GV_SetActorChild( work, NewLineSmoke( &work->body.objs->objs[CAMERA_PARTS].world , &tempvec1 , 6000, 25.0f, 10.0f) );
	GV_SetActorChild( work, NewLineSmoke( &work->body.objs->objs[CAMERA_PARTS].world , &tempvec1 , 6000, 25.0f, 10.0f) );

    }
    if(work->se_cnt < (SE_START - TIME_BASE*5) ){	/* 火花破壊音 */

	if (work->se_cnt<128*5){
	    SET_COLOR_CAM(128 ,0 ,0 ,work->se_cnt/5, &work->rgba);	//ライトを消す
	}

	if(!((irnd()>>8)&15)){		//ＰＡＬ版もこの確率で良いのでしょうか- - - 
	    GM_SeSetMode( SD_E_HIBANA02, &work->camera_pos, GM_SEMODE_BOMB );
	    dir = ((irnd()>>8)&4095) ;
	    force.vx = 100.0F * _RcosF( dir ) ;
	    force.vz = 100.0F * _RsinF( dir ) ;
	    force.vy = +1.0F * (float)((irnd()>>8)&127) ;
	    pos = work->eye_pos ;
	    pos.vx -= (force.vx*3/4) ;
	    pos.vz -= (force.vz*3/4) ;
	    CallSpark( &pos , &force) ;
	}
    }
    work->se_cnt-=TIME_BASE;
}
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActBreak						*/
/*	引数:	Work *work   						       	*/
/*	説明:	カメラが壊れてうなだれる		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SV_ActBreak(Work *work){

    Call_spark(work);
    work->ctrl.turn.vx += 2;
    if (work->ctrl.turn.vx > 4096){
	work->ctrl.turn.vx %= 4096;
	work->ctrl.rot.vx %= 4096;
    }
}
