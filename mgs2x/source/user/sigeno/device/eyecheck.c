/********************************************************************************/
/*	eyecheck.c								*/
/*	監視システム共通関数群							*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: eyecheck.c,v 1.1.1.3 2002/11/19 11:49:28 Yoshizawa1 Exp $		*/
/********************************************************************************/

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include "korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"

#include "camera.h"
#include "satoyoshi.h"

#include "eyecheck.h"

extern int CP_GetCorpNum( void ) ;
extern NEWCORP *CP_GetCorpFromEye( EYEPARAM * ,FVECTOR * ) ;
extern NEWCORP *CP_GetCorpN_start( void );

/**************************<-------local function------>*************************/
/*	名前:	int ENE_EyeCheckSub						*/
/*	返値:	int								*/
/*	引数:	EYEPARAM	*eye						*/
/*		FVECTOR		*trg						*/
/*	説明:	監視カメラ系 簡易視界チェック					*/
/********************************************************************************/
int ENE_EyeCheckSub( EYEPARAM *eye ,FVECTOR *trg)
{
	FVECTOR vec ;
	float dis ;
	int dir ,diff;

	_sceVu0SubVector( &vec, trg , eye->eyepos ) ;
	dis = GV_VecLen3F( &vec ) ;

	if ( dis > eye->length ){
	    return 0 ;
	}

	dir = GV_VecDir2( &vec ) ;
	if ( GV_DiffDirAbs( eye->rot.vy, dir ) > eye->range.vy ){
	    return 0 ;
	}

	dir = GV_VecDir2X( &vec )  ;
	diff = GV_DiffDirS( eye->rot.vx, dir ) ;
	if ( diff > eye->range.vx || diff < - eye->range.vx ){
	    return 0 ;
	}

	return 1 ;
}


/**************************<-------local function------>*************************/
/*	名前:	NEWCORP	*CP_GetCorpFromEye					*/
/*	返値:	NEWCORP								*/
/*	引数:	EYEPARAM	*eye						*/
/*		FVECTOR		*res						*/
/*	説明:	視界情報は監視カメラ型						*/
/********************************************************************************/
NEWCORP	*CP_GetCorpFromEye( EYEPARAM *eye ,FVECTOR *res )
{
	NEWCORP	*cur ;
	FVECTOR	*trg;
	cur = CP_GetCorpN_start();
	while ( cur != NULL ) {
		trg = &cur->ctrl->mov ;
		*res = *trg ;
		if(ENE_EyeCheckSub( eye ,trg)){
			if(!(ENE_EyeOnlineCheck(cur->ctrl->hzx_id,eye->eyepos,trg))){
				*res = *trg ;
				return cur ;
			}
		}
		cur = cur->next ;
	}
	return NULL ;
}





/**************************<-------local function------>*************************/
/*	名前:	int SIG_EyeCheck						*/
/*	返値:	int								*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	四角錘内 座標チェック						*/
/********************************************************************************/
int SIG_EyeCheck( EYEPARAM *eye )
{
	FVECTOR vec ;
	float dis ;
	int dir ,diff;

	_sceVu0SubVector( &vec, eye->trgpos , eye->eyepos ) ;
	dis = GV_VecLen3F( &vec ) ;

	if ( dis > eye->length ) return 0 ;
	dir = GV_VecDir2( &vec ) ;
	if ( GV_DiffDirAbs( eye->rot.vy, dir ) > eye->range.vy ) return 0 ;
	dir = GV_VecDir2X( &vec )  ;
	diff = GV_DiffDirS( eye->rot.vx, dir ) ;
	if ( diff > eye->range.vx || diff < - eye->range.vx ) return 0 ;
	return 1 ;
}


/**************************<-------local function------>*************************/
/*	名前:	void SIG_VecDir							*/
/*	返値:	なし								*/
/*	引数:	FVECTOR	*pos1							*/
/*	    :	FVECTOR	*pos2							*/
/*	    :	SVECTOR	*dir							*/
/*	説明:									*/
/********************************************************************************/
void SIG_VecDir(FVECTOR *pos1,FVECTOR *pos2,SVECTOR *dir){
	FVECTOR	vec ;
	float	dis_y,dis_x ;

	/*Ｙ角度*/	
	_sceVu0SubVector(  &vec, pos2, pos1 ) ;
	dir->vy = GV_VecDir2( &vec ) ;

	/*Ｘ角度*/
	dis_y = -vec.vy ;
	vec.vy = 0.0F;
	dis_x = GV_VecLen3F( &vec ) ;
	vec.vx = dis_y ;
	vec.vz = dis_x ;
	dir->vx = GV_VecDir2( &vec ) ;
	dir->vz = 0 ;
}

/**************************<-------local function------>*************************/
/*	名前:	void DEV_DirLimitX						*/
/*	返値:	なし								*/
/*	引数:	SVECTOR	*turn		向きたい方向				*/
/*	    :	int	d_limit		下方向限界				*/
/*	    :	int	u_limit		上方向限界				*/
/*	説明:	X軸の向きたい方向と向くことのできる限界角度から方向を算出する	*/
/*	    :	算出結果は直接 *turn に書き込んでいる				*/
/********************************************************************************/
void DEV_DirLimitX(SVECTOR *turn ,int d_limit,int u_limit){
		/*限界設定*/
    if((turn->vx > d_limit )&&(turn->vx <  2048 ))
    {
	turn->vx = d_limit ;
    }else {
	if((turn->vx < (4096-u_limit) )&&(turn->vx >= 2048 )){
	    turn->vx = (4096-u_limit) ;
	}
    }
}


/**************************<-------local function------>*************************/
/*	名前:	void DEV_DirLimitY						*/
/*	返値:	なし								*/
/*	引数:	SVECTOR	*turn		向きたい方向				*/
/*	    :	int	r_limit		右方向限界				*/
/*	    :	int	l_limit		左方向限界				*/
/*	    :	int	now_vy		現在のＹ軸回転量			*/
/*	説明:	Y軸の向きたい方向と向くことのできる限界角度から方向を算出する	*/
/*	    :	算出結果は直接 *turn に書き込んでいる				*/
/********************************************************************************/
void DEV_DirLimitY2(SVECTOR *turn, int center, int haba, short now_vy){
    int r_limit = -1 * haba;
    int l_limit = haba;

	//*******一度全て０を中心とした角度系に置き換える
    turn->vy -= center;
    now_vy -= center;

	//*******値域は-2048～2048で考える
    if (turn->vy >= 2048){
	turn->vy -= 4096;
    } 
    else if (turn->vy <= -2048){
	turn->vy += 4096;
    }
    if (now_vy >= 2048){
	now_vy -= 4096;
    } 
    else if (now_vy <= -2048){
	now_vy += 4096;
    }
    
	//*******限界以上は向かない
    if(r_limit > turn->vy) {
	    turn->vy = r_limit ;
    }else if(l_limit < turn->vy) {
	    turn->vy = l_limit ;
    }

    if (abs(now_vy-turn->vy) > 2000){
	turn->vy = (turn->vy + now_vy)/2;
    }

    turn->vy += center;		//角度系を元に戻す



#ifdef DEBUG_MODE
			MENU_Locate( 150, 180, 0 ) ;
			MENU_SetColor( 200, 200, 200 ) ;
			MENU_Printf( "TT: %d  NT: %d   ", 
				     turn->vy, now_vy );
#endif
}



void DEV_DirLimitY(SVECTOR *turn ,int r_limit,int l_limit){
	int center ;

	/*限界設定*/
	r_limit &= 4095; l_limit &= 4095; turn->vy &= 4095;
	if(r_limit > l_limit) l_limit += 4096 ;
	if(r_limit > turn->vy) turn->vy += 4096 ;
	if(turn->vy <= l_limit) {
		turn->vy &= 4095 ;
		return ;
	}

	r_limit &= 4095;l_limit &= 4095;turn->vy &= 4095;
	center = ((r_limit + l_limit)/2) ;
	if(l_limit > center) center += 2048 ;
	if(l_limit > turn->vy) turn->vy += 4096 ;

	if(center > turn->vy) {
		turn->vy = l_limit ;
	}else {
		turn->vy = r_limit ;
	}
	turn->vy &= 4095 ; 
}


/**************************<-------local function------>*************************/
/*	名前:	void DEV_No_Sight						*/
/*	返値:	なし								*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	標的が見えない時に警戒度を下げる				*/
/********************************************************************************/
void DEV_No_Sight(EYEPARAM *eye){
	if(eye->alert_time>0) eye->alert_time -- ;
}


/**************************<-------local function------>*************************/
/*	名前:	void DEV_In_Sight						*/
/*	返値:	なし								*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	敵が視界の中にいた場合の処理					*/
/********************************************************************************/
void DEV_In_Sight(EYEPARAM *eye){

    if( (GM_AlertMode != ALERT_MODE_ALERT) &&(eye->alert_time == 0) ){
	//*通報*/
	//*ベリーハード即ゲームオーバー*/
#if 1
	if ( GM_GameLevel == GM_LEVEL_EXTREME ) {
	    GM_SeSetMode( SD_E_BIKKRI01, eye->eyepos, GM_SEMODE_BOMB ) ;
	    COM_SetFlameFlag( CMFLAG_DETECT ) ;
	    COM_DetectVibration( ) ;
	}
#endif
	eye->discv_time = DISCV_DELAY ;	//通報までの時間をセット
	GM_SeSetMode(SD_E_CAMFND01,eye->eyepos, GM_SEMODE_BOMB ); //SE:「カシャッ」
    }

    eye->alert_time = SIGHT_ALERT ;	//警戒タイムのセット

    //*****白びっくりキャンセル*/
    eye->alert2 = 0 ;
    eye->discv2 = 0;
}


/**************************<-------local function------>*************************/
/*	名前:	void DEV_In_SightSub						*/
/*	返値:	なし								*/
/*	引数:	EYEPAEAM	*eye						*/
/*	    :	FVECTOR		*trg						*/
/*	    :	int		uniq_id						*/
/*	説明:	白びっくりレベルのものを発見					*/
/********************************************************************************/
void DEV_In_SightSub(EYEPARAM *eye, FVECTOR *trg, int uniq_id){
	if(
	( GM_AlertMode != ALERT_MODE_ALERT )
	&&(eye->alert_time == 0 )
	&&(eye->alert2 == 0 )
	){
		eye->discv2 = DISCV_DELAY ;
		eye->accident_pos = *trg ;
		eye->accident_uniq_id = uniq_id;
		GM_SeSetMode(SD_E_CAMFND01,eye->eyepos,
			GM_SEMODE_BOMB ) ;
	}
	eye->alert2 = SIGHT_ALERT2 ;
	
}


/**************************<-------local function------>*************************/
/*	名前:	void DEV_Security						*/
/*	返値:	なし								*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	発見から通報までのプロセス					*/
/********************************************************************************/
void DEV_Security(EYEPARAM *eye){
    extern void COM_SetSecurityAlert( FVECTOR * , int );
    
    //    FMATRIX headmat;

    //    GV_VecToMat( eye->eyepos, &headmat ) ;
    if( ((eye->discv_time ==0 )&&(eye->alert_time == SIGHT_ALERT)) ||
	(( GM_AlertMode == ALERT_MODE_ALERT )&&(eye->alert_time == SIGHT_ALERT)) ||
	(eye->discv_time == 1 ) ){
	COM_SetSecurityAlert(eye->trgpos,GM_PlayerMap);	//通報
    }
    if(eye->discv_time > 0 ) eye->discv_time --;

    if(	((eye->discv2 ==0 )&&(eye->alert2 == SIGHT_ALERT2))||
	(eye->discv2 == 1 ) ){
	/*通報*/
	COM_SetAccident( eye->accident_uniq_id, &eye->accident_pos ,
			eye->accident_hzx_id, DEV_ACCIDENT_DELAY_TIME ) ;
    }
    if(eye->discv2 > 0 ){
	eye->discv2 --;
    }
}

/**************************<-------local function------>*************************/
/*	名前:	void DEV_HeadMarkCheck						*/
/*	返値:	なし								*/
/*	引数:	EYEPARAM	*eye						*/
/*	    :	FMATRIX		*head						*/
/*	    :	int		*flag						*/
/*	説明:									*/
/********************************************************************************/
void DEV_HeadMarkCheck(EYEPARAM *eye , FMATRIX *head ,int *flag){
    
    if(eye->discv_time == HEADMARK_SET){
	GM_SeSetMode( SD_E_BIKKRI01 , eye->eyepos,
		      GM_SEMODE_BOMB ) ;
	
	*flag = HMK2_TYPE_RED_AT_MIN ;
	COM_DetectVibration( ) ;
    }else if(eye->discv2 == HEADMARK_SET){
	*flag = HMK2_TYPE_WHT_AT_MIN ;
    }

}


/**************************<-------local function------>*************************/
/*	名前:	void DEV_CameraSe						*/
/*	返値:	なし								*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	首振り効果音を鳴らす						*/
/********************************************************************************/
void DEV_CameraSe(EYEPARAM *eye ){
	/*首振り効果音*/
	if((DG_TickCount - eye->se_tick) > CAM_SE_INT){	//音を鳴らした瞬間を保存
		GM_SeSetMode( SD_E_CAMMOV01 ,eye->eyepos,GM_SEMODE_NORMAL ) ;
		eye->se_tick = DG_TickCount ;
	}
}

#define CB_BOX_FAIL	(PLAYER_MOVE|PLAYER_CB_BOX_STAND)
/**************************<-------local function------>*************************/
/*	名前:	int CB_BoxCheck							*/
/*	返値:	ダンボール有効 : １	無効 : ０				*/
/*	引数:	なし								*/
/*	説明:	ダンボールの有効チェックを行なう				*/
/********************************************************************************/
static int CB_BoxCheck(void){
    if(GM_PlayerStatus & PLAYER_CB_BOX){
	if(!(GM_PlayerStatus & CB_BOX_FAIL)) {//動いているor気絶
	    return 1;
	}
    }
    return 0;
}


/**************************<-------local function------>*************************/
/*	名前:	int DEV_EyeInfoCheck						*/
/*	返値:									*/
/*	引数:	EYEPARAM	*eye						*/
/*		HZX_GROUP_ID	hzx_id						*/
/*	説明:	視界チェック全般を行なう					*/
/********************************************************************************/
int DEV_EyeInfoCheck(EYEPARAM *eye , HZX_GROUP_ID hzx_id ){
#ifdef DEBUG_MODE
    if(eye->debug_st & DEV_SIGHT_OFF ){
	/*視界off*/
	//	DEV_No_Sight( eye ) ;
	return 0 ;
    }
#endif

    //*****ダンボールは見えない*****
    if(CB_BoxCheck()) {
	DEV_No_Sight( eye ) ;
	return 0 ;
    }

    //*****通常チェック*****
    if(SIG_EyeCheck( eye )){
	if(ENE_EyeOnlineCheck( hzx_id, eye->eyepos, eye->trgpos ) ){
	    //*****壁
	    DEV_No_Sight( eye ) ;
	    return 0 ;
	}else {
	    //*****見えた
	    DEV_In_Sight( eye ) ;
	    return 1 ;
	}
    }else {
	DEV_No_Sight( eye ) ;
	return 0 ;
    }
}


/**************************<-------local function------>*************************/
/*	名前:	int DEV_EnemyStatus_Id						*/
/*	返値:	int	-1:失敗							*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	敵兵のステータスをチェックする					*/
/********************************************************************************/
int DEV_EnemyStatus_Id( EYEPARAM *eye){
    ENETHINK	*entk;
    NEWCORP	*corpp;
    int uniq_id = eye->accident_uniq_id;

    if(eye->discv2>0){
	eye->discv2 -- ;
    }

    entk = COM_GetEnemyFromUniqID(uniq_id);	//uniq_idから敵兵の構造体を取得

    //    if ( (entk != NULL)&&(~((entk->act->status)&ACT_STATUS_GHOST)) ){
    if ( (entk != NULL)){

	if ( (entk->act->status)&(ACT_STATUS_FAINT|ACT_STATUS_DEATH) ){	//倒れているかチェック
	    return 1;
	}
    }

    corpp = CP_GetCorp(uniq_id);		//死んでいるのかも
    if (corpp != NULL){
	return 1;
    }
    else {
	return 0;
    }



}

/**************************<-------local function------>*************************/
/*	名前:	int DEV_EnemyCheck						*/
/*	返値:	int	いない: -1   発見: uniq_id				*/
/*	引数:	EYEPARAM	*eye						*/
/*		HZX_GROUP_ID	hzx_id						*/
/*	説明:	ダメージ兵と敵死兵を探す					*/
/********************************************************************************/
int DEV_EnemyCheck(EYEPARAM *eye , HZX_GROUP_ID hzx_id, Work *work ){
    
    COMMANDER	*com ;
    int i,corp_num ,uniq_id ;
    short	loop;
    short	tmp_uniq_id;
    ENETHINK *dam_entk ;
    FVECTOR		*trg ,res;
    SVECTOR		rgb;
    NEWCORP	*corp ;
    rgb.vx = 255; rgb.vy = 0; rgb.vz = 0;
	
#ifdef DEBUG_MODE
    if(eye->debug_st & DEV_SIGHT_OFF ){
	return 0 ;
    }
#endif

    if(eye->discv2>0){
	eye->discv2 -- ;
    }

    com = COM_GetCommander() ;
    for ( i=0; i<com->dam_en_num; i++ ) {
	dam_entk = com->dam_entk[i] ;
	trg = &dam_entk->ctrl->mov ;
	if(ENE_EyeCheckSub( eye ,trg)){
	    if(!(ENE_EyeOnlineCheck(hzx_id,eye->eyepos,trg))){

		tmp_uniq_id = dam_entk->uniq_id+1;
		//******既に通報済みチェック
		for (loop=0; loop<UNIQ_LIST_MAX; loop++){
		    if (work->uniq_id_list[loop] == tmp_uniq_id){
			tmp_uniq_id = 0	;
			break;
		    }
		}
	    
		if(tmp_uniq_id){
		    eye->accident_hzx_id = dam_entk->ctrl->hzx_id ;
		    DEV_In_SightSub(eye,trg ,dam_entk->uniq_id);
		    return dam_entk->uniq_id+1 ;
		    		}
	    }
	}
    }
    corp_num = CP_GetCorpNum();

    if(corp_num != 0){

	corp = CP_GetCorpFromEye( eye,&res) ;
	if(corp != NULL){
	    tmp_uniq_id = corp->uniq_id+1;
	    //******既に通報済みチェック
	    for (loop=0; loop<UNIQ_LIST_MAX; loop++){
		if (work->uniq_id_list[loop] == tmp_uniq_id){
		    tmp_uniq_id = 0	;
		    break;
		}
	    }
	    if(tmp_uniq_id){
		eye->accident_hzx_id = corp->ctrl->hzx_id ;
		DEV_In_SightSub(eye,&res,corp->uniq_id);
		return corp->uniq_id+1 ;
	    }
	}
    }

    return 0 ;
}


/**************************<-------local function------>*************************/
/*	名前:	int DEV_EyeInfoCheck						*/
/*	返値:									*/
/*	引数:	EYEPARAM	*eye						*/
/*		HZX_GROUP_ID	hzx_id						*/
/*	説明:	視界チェック全般を行なう					*/
/********************************************************************************/
void DEV_Bullet(EYEPARAM *eye ,FMATRIX *head){
    extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
    SVECTOR		rot = { -1024 ,0,0} ;
    FMATRIX		mat ;
    int type ;

    DG_SetPos(head );
    DG_RotatePos( &rot ) ;
    DG_GetPos(&mat) ;
    type = (BUL_TYPE_VISIBLE|BUL_TYPE_SPARK|BUL_TYPE_SCAR);
    NewBullet( &mat,type, PLAYER_SIDE, 25, 5, 20000, 1000, WP_Famas ) ;
    GM_SeSetMode( SD_E_FAMAS03 ,eye->eyepos,GM_SEMODE_BOMB ) ;
}


/**************************<-------local function------>*************************/
/*	名前:	int DEV_EyeInfoCheck						*/
/*	返値:									*/
/*	引数:	EYEPARAM	*eye						*/
/*		HZX_GROUP_ID	hzx_id						*/
/*	説明:	視界チェック全般を行なう					*/
/********************************************************************************/
/*CONTROLの方向処理*/
void DEV_InterpRot(DEV_CTRL *ctrl)
{
    int interp ;
    // 向き更新 */
    if ( ( interp = ctrl->interp ) == 0 ) {
	GV_NearExp8PV( &( ctrl->rot ), &( ctrl->turn ), 3 ) ;
    } else {
	GV_NearTimePV( &( ctrl->rot ), &( ctrl->turn ), interp, 3 ) ;
	ctrl->interp = interp - 1 ;
    }
}


/**************************<-------local function------>*************************/
/*	名前:	int DEV_EyeInfoCheck						*/
/*	返値:									*/
/*	引数:	EYEPARAM	*eye						*/
/*		HZX_GROUP_ID	hzx_id						*/
/*	説明:	視界チェック全般を行なう					*/
/********************************************************************************/
void DEV_InitDevCtrl(DEV_CTRL *ctrl,int interp){
	ctrl->rot = DG_ZeroSVector ;
	ctrl->turn = DG_ZeroSVector ;
	ctrl->interp = interp ;
}


/**************************<-------local function------>*************************/
/*	名前:	void DEV_CopySvec2Fvec						*/
/*	返値:									*/
/*	引数:	SVECTOR		*svec						*/
/*		FVECTOR		*fvec						*/
/*	説明:									*/
/********************************************************************************/
void DEV_CopySvec2Fvec(SVECTOR *svec ,FVECTOR *fvec){
	fvec->vx = (float) svec->vx ;
	fvec->vy = (float) svec->vy ;
	fvec->vz = (float) svec->vz ;
}


#ifdef DEBUG_MODE
/**************************<-------local function------>*************************/
/*	名前:	void DEV_DebugMode						*/
/*	返値:									*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:									*/
/********************************************************************************/
void DEV_DebugMode(EYEPARAM *eye){

	if( GM_DebugModeEnable ){
		if ( GV_PadData[1].press & PAD_R2 && GV_PadData[1].status & PAD_X ){
			if ( eye->debug_st & DEV_SIGHT_OFF ) {
				eye->debug_st &= ~DEV_SIGHT_OFF ;
				printf("CAMERA EYE OPEN\n");
			} else {
				eye->debug_st |= DEV_SIGHT_OFF ;
				printf("CAMERA EYE CLOSE\n");
			}
		}
	}
}
#endif


/**************************<-------local function------>*************************/
/*	名前:	void DEV_InitEyeParam						*/
/*	返値:									*/
/*	引数:	EYEPARAM	*eye						*/
/*	    :	SVECTOR		*range						*/
/*	    :	float		length						*/
/*	説明:									*/
/********************************************************************************/
void DEV_InitEyeParam( EYEPARAM *eye,
                       SVECTOR *range,  /*視界範囲*/
                       float length )       /*視界距離*/
{
	eye->alert_time = 0 ;
	eye->discv_time = 0;
	eye->alert2 = 0 ;
	eye->discv2 = 0;
	eye->range = *range ;
	eye->length = length ;
	eye->accident_uniq_id = -1 ;

	eye->se_tick = -10 ;

#ifdef DEBUG_MODE
	eye->debug_st = 0;	/*デバッグ用*/
#endif
}


/**************************<-------local function------>*************************/
/*	名前:	int DEV_CheckDirSub						*/
/*	返値:									*/
/*	引数:	int	dir1							*/
/*		int	dir2							*/
/*	説明:									*/
/********************************************************************************/
int DEV_CheckDirSub(int dir1,int dir2){
	int sub;
	
	sub = dir2 - dir1;
	sub	&= 4095;
	if(sub >= 2048) sub= -(4096-sub);
	return sub;
}
