//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	cypher.c								*/
/*	サイファ	*NewCypher						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cypher4snipe.c,v 1.1.1.3 2002/11/19 11:48:16 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include "../../korekado/enemy/enemy.h"
#include "../../korekado/enemy/enemy.x"
#include "eyecheck.h"
#include "cypher4snipe.h"
#include "satoyoshi.h"
#include "../util/sato_util.h"
#include "../../morita/emma/include/emma_com.h"

#if 0
#define	SATO_DEBUG
#endif



/********************************************************************************/
/*	extern									*/
/********************************************************************************/
#ifdef SATO_DEBUG
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif
#ifdef DEBUG_MODE
extern void *NewEyeView( FMATRIX *, int , int , int , int , COMMANDER *, int *);
#endif

extern void GM_InitRadarControl( RADAR_CTRL *, FVECTOR *, int , int );

extern FVECTOR *EMA_CommandGetPosition();

extern void *NewCypherExplosion( FVECTOR* center, float size, int flag );
extern void	AN_MazzleMeca( FMATRIX *world , FVECTOR *shift );
extern void AN_CartridgeMeca_E( FMATRIX *world , FVECTOR *shift );
extern void CYP_HMK_Check(EYEPARAM *eye , FMATRIX *head, void* p);

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define M_PI 3.14159265358979323846264338327950288419716939937510f

#ifdef KP_XBOX	// サイファ音は遮蔽計算なしにする
#define	GM_SeSetFromVolCurve( a, b, c ) GM_SeSetFromVolCurveAddr( a, b, c, GM_INVALID_ADDR )
#define GM_SeSetMode( a, b, c ) GM_SeSetModeAddr( a, b, c, GM_INVALID_ADDR )
#endif


#ifdef SATO_DEBUG
void setline(FVECTOR *point){
    FVECTOR verts[2];

    _sceVu0CopyVector(&verts[0], point);
    _sceVu0CopyVector(&verts[1], point);

    verts[0].vy += 4000.0f;
    verts[1].vy -= 4000.0f;

    NewLineView(verts, 1, 250,0,0);

}

void setline2(FVECTOR *point){
    FVECTOR verts[2];

    _sceVu0CopyVector(&verts[0], point);
    _sceVu0CopyVector(&verts[1], point);

    verts[0].vy += 4000.0f;
    verts[1].vy -= 4000.0f;

    NewLineView(verts, 1, 0,0,250);

}
#endif




void destroy_cypher(Work* work){

    if (work->cyp_end_destroy == ON){
	return;
    }

    if (work->is_player_atack == ON){
	GM_MecaKillCount ++;
	if ( GM_MecaKillCount > GM_MAX_RESULT_COUNT ) GM_MecaKillCount = GM_MAX_RESULT_COUNT ;
    }
    
    //破壊プロックがあったら実行
    if ( work->brake_proc_id != NULL ){
	GCL_ARGS	args;
	args.argc = 1;
	args.argv = &work->name;
	GCL_ExecProc(work->brake_proc_id, &args);
    }

    if (work->is_attack == 1){
	EMA_CommandEneDied(work->name);
    }

    NewCypherExplosion(&work->control.mov, 600.0f, 0x02);
    GM_SeSetMode( SD_W_MINEEXP1, &work->control.mov, GM_SEMODE_BOMB );
    GV_DestroyActor(work) ;
    work->r_trg.class |= TARGET_DEAD;
    work->cyp_end_destroy = ON;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void culc_ko_ten2						*/
/*	引数:	Work *work   						       	*/
/*	説明:	交点の計算				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void culc_ko_ten2(float p_x, float p_z, float p_y, float X_, float Z_, Work *work){
    FVECTOR	answer;
    FVECTOR	safe[2];
    FVECTOR	pc_pos;
    float ansx;
    float ansz;

    float PX_;
    float PZ_;

    _sceVu0CopyVector(&pc_pos, &GM_PlayerFindPos);


    //	プレイヤー狙いバグチェック
    //    pc_pos.vx = 57000.0f; 
    //    pc_pos.vz = -206000.0f;



    PX_ = (pc_pos.vz - p_z) / (pc_pos.vx - p_x);
    PZ_ = pc_pos.vz - (pc_pos.vx * PX_);


    answer.vy = p_y;
    safe[0].vy = answer.vy;
    safe[1].vy = answer.vy;

    //Z軸に平行
    if (X_ == 0.0f){
	answer.vx = Z_;

	answer.vz = Z_*PX_+PZ_;

	safe[0].vx = Z_;
	safe[1].vx = Z_;
	safe[0].vz = answer.vz+1300.0f;
	safe[1].vz = answer.vz-1300.0f;


	    if ((answer.vz > work->kaihi_pos.vz)&&
		(work->kaihi_pos.vz >	safe[1].vz) ){
		work->kaihi_flag = 1;
		_sceVu0CopyVector(&work->kaihi_pos, &safe[1]);
	    }
	    if ((answer.vz <= work->kaihi_pos.vz)&&
		(work->kaihi_pos.vz <	safe[0].vz) ){
		work->kaihi_flag = 1;
		_sceVu0CopyVector(&work->kaihi_pos, &safe[0]);
	    }

    }
    else {

	ansx = -(Z_ - PZ_) / (X_ - PX_);
	ansz = PX_*ansx + PZ_;
	answer.vx = ansx;
	answer.vz = ansz;

	//	605 : 350
	safe[0].vx = answer.vx+865.0f;
	safe[1].vx = answer.vx-865.0f;
	safe[0].vz = answer.vz-500.0f;
	safe[1].vz = answer.vz+500.0f;


	    if ((answer.vx > work->kaihi_pos.vx)&&
		(work->kaihi_pos.vx >	safe[1].vx) ){
		work->kaihi_flag = 1;
		_sceVu0CopyVector(&work->kaihi_pos, &safe[1]);
	    }
	    if ((answer.vx <= work->kaihi_pos.vx)&&
		(work->kaihi_pos.vx <	safe[0].vx) ){
		work->kaihi_flag = 1;
		_sceVu0CopyVector(&work->kaihi_pos, &safe[0]);
	    }
    }


    //ルート上になければさよなら
    if (work->trace_point[0].vx > work->trace_point[work->tp_num-1].vx){
	if ( (answer.vx < work->trace_point[work->tp_num-1].vx)||
	     (answer.vx > work->trace_point[0].vx)){
	    return;
	}
    }
    else {
	if ( (answer.vx > work->trace_point[work->tp_num-1].vx)||
	     (answer.vx < work->trace_point[0].vx)){
	    return;
	}
    }
    if (work->trace_point[0].vz > work->trace_point[work->tp_num-1].vz){
	if ( (answer.vz < work->trace_point[work->tp_num-1].vz)||
	     (answer.vz > work->trace_point[0].vz)){
	    return;
	}
    }
    else {
	if ( (answer.vz > work->trace_point[work->tp_num-1].vz)||
	     (answer.vz < work->trace_point[0].vz)){
	    return;
	}
    }
#ifdef SATO_DEBUG
    setline (&answer);
    setline2 (&safe[0]);
    setline2 (&safe[1]);
#endif
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void culc_ko_ten						*/
/*	引数:	Work *work   						       	*/
/*	説明:	交点の計算				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void culc_ko_ten(Work* work){
    FVECTOR verts[2];
    float X_;
    float Z_;
    FVECTOR	answer;
    //    short loop;

    // 柱の蔭に隠れる心配はないよ さよなら
    if ((work->trace_point[0].vx < 103000.0f) &&
	(work->trace_point[0].vz < -work->trace_point[0].vx*26.0f/45.0f-60489.0f)){
	return;
    }

    _sceVu0CopyVector(&verts[0], &work->trace_point[0]);
    _sceVu0CopyVector(&verts[1], &work->trace_point[work->tp_num-1]);

    if (verts[0].vx == verts[1].vx){
	X_ = 0.0f;
	Z_ = verts[0].vx;
    }
    else {
	X_ = (verts[0].vz - verts[1].vz) / (verts[0].vx - verts[1].vx);
	Z_ = verts[0].vz - (verts[0].vx * X_);
    }

    answer.vy = verts[0].vy;

    culc_ko_ten2(101000.0f, -165000.0f, verts[0].vy, X_, Z_, work);
    culc_ko_ten2(101000.0f, -150000.0f, verts[0].vy, X_, Z_, work);
    culc_ko_ten2(101000.0f, -139900.0f, verts[0].vy, X_, Z_, work);
    culc_ko_ten2( 89500.0f, -115000.0f, verts[0].vy, X_, Z_, work);
    culc_ko_ten2( 85000.0f, -112500.0f, verts[0].vy, X_, Z_, work);
    culc_ko_ten2( 76500.0f, -107500.0f, verts[0].vy, X_, Z_, work);
    culc_ko_ten2( 67800.0f, -102600.0f, verts[0].vy, X_, Z_, work);
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CYP_InitTracePoint						*/
/*	引数:	Work *work   						       	*/
/*	説明:	ツイビ用ルートを設定する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CYP_InitTracePoint(Work *work) {
    ROUTENAVI 	*rnavi;
    FVECTOR	*point;
    short	loop1, loop2;

    rnavi = &work->rnavi;
    point = work->trace_point;

#ifdef DEBUG_MODE
    if (rnavi->n_nodes > MAX_POINT_NUM){
	ASSERT(1);	//ここに来てはいけない
    }
#endif
    _sceVu0CopyVector(&point[0], &rnavi->nodes[0]);//コピー
    work->tp_num = 1;

    //全てのノードをツイビルートにコピー
    for (loop1=1; loop1<rnavi->n_nodes; loop1++){
	
	//全く同じ位置のツイビルートがないか検索
	//あれば上書き
	for(loop2=0; loop2<work->tp_num; loop2++){
	    if ( (point[loop2].vx == rnavi->nodes[loop1].vx) &&
		 (point[loop2].vy == rnavi->nodes[loop1].vy) &&
		 (point[loop2].vz == rnavi->nodes[loop1].vz) ){
#ifdef DEBUG_MODE
		printf("    						****NODE %d <> %d is SAME**** \n",loop1, loop2);
#endif
		work->loop_flag = 1;	//ループフラグをつけて下ルート禁止
		break;
	    }
	}
	_sceVu0CopyVector(&point[loop2], &rnavi->nodes[loop1]);//コピー
	if (loop2+1 > work->tp_num){	//ツイビルートの数＋１
	    work->tp_num ++ ;
	}
    }

    printf ("NODE NUM = 						::%d \n", work->tp_num);


}




/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CYP_SetNextnode						*/
/*	引数:	ROUTENAVI	*rnavi						*/
/*	引数:	FVECTOR *force							*/
/*	説明:	ルートの次のポイントを目標地に設定				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CYP_SetNextnode(ROUTENAVI * rnavi, Work *work)
{
    TRGPOINT	*trgp = &work->trg;

    //方向固定フラグ
    work->rot_flag = rnavi->p_action;	//アクション番号を設定する

printf ("Snipe CYP Set rot :%d r:%d n:%d*******************\n", 
	work->rot_flag, work->route, work->rnavi.next_node);


    //注視点の保存
    Dir_from_2Vec(&work->camera,&rnavi->aimnodes[(int)rnavi->next_node],&work->look_dir);

    if (work->reach_proc_id != NULL){	// 到達プロック実行許可
	if ((rnavi->c_route == work->reach_proc_route)&&(rnavi->next_node == work->reach_proc_node)){
	    GCL_ARGS	args;
	    args.argc = 1;
	    args.argv = &work->name;
	    GCL_ExecProc(work->reach_proc_id, &args);
#ifdef DEBUG_MODE
	    printf ("Call Reach Proc %d %d\n", work->reach_proc_route, work->reach_proc_node);
#endif
	}
    }

    ENE_SetNextnode( rnavi );		//次のノードの設定

    //    work->to_node = rnavi->next_node;

    trgp->pos = rnavi->nodes[ (short)rnavi->next_node ] ;
    trgp->map = rnavi->mapbit[ (short)rnavi->next_node ] ;
}

extern void	NewSpark( FMATRIX * );
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
/*	名前:	void SetPointAction						*/
/*	引数:	Work *work   						       	*/
/*	説明:	ルートポイントでのアクション等の設定				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetPointAction( Work *work ){
    ROUTENAVI *rnavi;
    rnavi = &work->rnavi;
    rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node];
    rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
    rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node];
    rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node];
    rnavi->p_actstatus = rnavi->pa_flag[(int)rnavi->next_node];

    if(rnavi->p_acttime == P_TIME_WAIT){	//メッセージ待ちの待機
	work->status = CYP_WAIT;
	work->body.objs->flag |= DG_FLAG_INVISIBLE;
	work->r_trg.class &= ~TARGET_LOCKON;
    }

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	CheckMessage						*/
/*	引数:	Work *work   						       	*/
/*	説明:	メッセージを受信する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	CheckMessage( Work *work )
{
    GV_MSG	*msg ;
    int n_msg, code  ;
    n_msg = work->control.n_msg ;
    msg = work->control.msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ] ;
	switch( code ) {
	case MSG_ROUTE_CHANGE :
	    work->route = msg->message[1] ;
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			work->route += COM_GetRootOfset( ) ;
		}
	    ENE_InitRouteNavi( &work->rnavi, work->route, msg->message[2]);
	    CYP_InitTracePoint( work );
	    SetPointAction( work );
	    work->trg.pos = work->rnavi.nodes[msg->message[2]] ;
	    work->trg.map = work->rnavi.mapbit[msg->message[2]] ;
	    work->trg.addr = HZX_GetAddress( work->trg.map,
					     &work->trg.pos, -1 ) ;
	    work->mode = MOVE_MODE ;
	    break ;
	case ENE_MSG_POINT_ACTION_START :
	    printf("Cypher Get Message Start\n");
	    /*移動許可*/
	    work->status = CYP_ACTIVE ;
	    work->body.objs->flag &= ~DG_FLAG_INVISIBLE;
	    work->r_trg.class |= TARGET_LOCKON;
	    break ;
	case MES_CYP_SNP_EYE_CONTROL:
	    work->cyp_eye_close = msg->message[1];
#ifdef DEBUG_MODE
	    if (work->cyp_eye_close==ON){
		printf("CYP_SNP Eye Close\n");
	    }
	    if (work->cyp_eye_close==OFF){
		printf("CYP_SNP Eye Open\n");
	    }
#endif
	    break;
	}
	msg++ ;
    }
}

#if 0
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void HitPos2Rot							*/
/*	引数:	FVECTOR		*pos						*/
/*		FVECTOR		*res						*/
/*		FMATRIX		*world						*/
/*	説明:	絶対座標をwolrd上の相対座標に変換				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void HitPos2Rot( FVECTOR *pos,FVECTOR *res,FMATRIX *world )
{
    FVECTOR	sub;
    DG_SetPos( world );
    /*hit場所の相対位置*/
    sub.vx = pos->vx - world->m[ 3 ][ 0 ];
    sub.vy = pos->vy - world->m[ 3 ][ 1 ];
    sub.vz = pos->vz - world->m[ 3 ][ 2 ];
    /*world座標に配置*/
    DG_RotVector( &sub, res, 1 );
}
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
	    def->power->force = off->power->force ;


	    if( def->weapon_type & (WP_PSG1T|WP_M92)){
		CallSpark(&def->hit ,&off->power->force);
	    }

	    if( def->weapon_type & WP_BULLET){

		CallSpark(&def->hit ,&off->power->force);


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

		def->power->vital = def->power->vital - off->power->damage; 

#ifdef DEBUG_MODE
printf(" CYPHER Damaged Body: %d - %d \n",def->power->vital, off->power->damage ); 
#endif
		if ( (def->power->vital <= 0)||(GM_GameLevel==GM_LEVEL_VERYEASY) ) {
		    /*破壊された*/
		    if (def->weapon_type & WP_NOPLAYER){
			work->is_player_atack = OFF;
		    }
		    else {
			work->is_player_atack = ON;
		    }
		    destroy_cypher(work);
		    def->power->vital = 0;
		}
		else {
		    NewCrushWithForce(&def->hit);
		}
	    }else if( def->weapon_type & WP_BLAST){
		/*爆発系ヒット*/
		def->power->vital = def->power->vital - off->power->damage; 
		if ( def->power->vital <= 0 ) {
		    /*破壊された*/
    if (def->weapon_type & WP_NOPLAYER){
	work->is_player_atack = OFF;
    }
    else {
	work->is_player_atack = ON;
    }
		    destroy_cypher(work);
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
	    def->power->force = off->power->force ;
	    if(( def->weapon_type & WP_BULLET)){

		//仮
		CallSpark(&def->hit ,&off->power->force);
		NewCrushWithForce(&def->hit);

#ifdef DEBUG_MODE
printf(" CYPHER Damaged Head: %d - %d \n",def->power->vital, off->power->damage ); 
#endif
		def->power->vital = def->power->vital - off->power->damage; 




		if ( def->power->vital <= 0 ) {
		    /*破壊された*/
    if (def->weapon_type & WP_NOPLAYER){
	work->is_player_atack = OFF;
    }
    else {
	work->is_player_atack = ON;
    }
		    destroy_cypher(work);
		}
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
	    if( def->weapon_type & WP_BULLET){
		
		CallSpark(&def->hit ,&off->power->force);
		NewCrushWithForce(&def->hit);
    if (def->weapon_type & WP_NOPLAYER){
	work->is_player_atack = OFF;
    }
    else {
	work->is_player_atack = ON;
    }
		destroy_cypher(work);
	    }
	}
    }
    def->weapon_type = 0 ;
}


/*胴体アタリサイズ*/
#define	BODY_SIZE_X	(390.0F)
#define	BODY_SIZE_Z	(210.0F)
#define	BODY_SIZE_GUN_Y	(220.0F)

/*GUNアタリサイズ*/
#define	GUN_SIZE_X		(130.0F)
#define	GUN_SIZE_Y		(150.0F)
#define	GUN_SIZE_Z		(600.0F)

/*ローターあたりサイズ*/
#define	ROAT_SIZE_XZ		(70.0F)
#define	ROAT_SIZE_Y		(200.0F)
#define	ROAT_SIZE_GUN_Y		(100.0F)

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
    FVECTOR	b_size = { BODY_SIZE_X, BODY_SIZE_GUN_Y, BODY_SIZE_Z } ;
    FVECTOR	r_size = { ROAT_SIZE_XZ, ROAT_SIZE_GUN_Y, ROAT_SIZE_XZ } ;
    FVECTOR	h_size = { GUN_SIZE_X, GUN_SIZE_Y, GUN_SIZE_Z};


    /*ターゲット設定*/
    flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);
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
	//	NewTargetView ( &work->b_trg[loop], 250, 0, 0);
    }

    //カメラ部分設定
    GM_SetTarget( &work->h_trg, flag,map, ENEMY_SIDE, 
		  &h_size, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->h_trg, ChildTargCallBack_Head, work ) ;
    GM_SetTargetWeaponType(&work->h_trg,0);
    GM_SetPowerTarget( &work->h_trg, &work->h_power,
		       POWER_DECREASE, CYP_HEAD_VITAL, 0, 0, &DG_ZeroVector );
    GM_PutTarget( &work->h_trg );
    //    NewTargetView ( &work->h_trg, 0, 250, 0);

    flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_LOCKON|TARGET_ROTATE|TARGET_POWER);
    //ローター部分設定
    GM_SetTarget( &work->r_trg, flag,map, ENEMY_SIDE, 
		  &r_size, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->r_trg, ChildTargCallBack_Roat, work ) ;
    GM_SetTargetWeaponType(&work->r_trg,0);
    GM_PutTarget( &work->r_trg );

}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void DecayStep							*/
/*	引数:	CONTROL	*ctrl							*/
/*		float	rate							*/
/*	説明:	ベクトルの減衰/増加を行なう					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void DecayStep(CONTROL *ctrl ,float rate){
    FVECTOR *step;
    step = &ctrl->step ;

    step->vx *= rate ;
    step->vy *= rate ;
    step->vz *= rate ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int DirectTrace3D						*/
/*	引数:	Work	*work	  					       	*/
/*		float	range	誤差						*/
/*		int	*brake							*/
/*	説明:	３Ｄ目標地点に移動？						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int DirectTrace3D(Work *work, float range,int *brake){

    FVECTOR		shift ,*pos;
    float		dis, accele;
    TRGPOINT *trgp ;
    CONTROL *ctrl ;

    trgp = &work->trg ;
    ctrl = &work->control ;

    pos = &ctrl->mov;
    SAT_Minus_FVECTOR(&shift, &trgp->pos, pos);

    //目標地点の方向と距離を算出
    trgp->dir = _FVecDir2( &shift ) ;
    //    trgp->h_dis = (int)_FVecLen2( &shift ) ;
    
    accele = dis = GV_VecLen3F( &shift ) ;

    //加速度の制限
    if (accele > CYP_ACCELE){
    	GV_LenVec3F( &shift, &shift, 0.0F, CYP_ACCELE ) ;
    }

    if( dis > range ) {
	DecayStep(ctrl,DECAY_RATE) ;
	/*現在の速度	サイファの加速度が決まっている  */
	ctrl->step.vx += shift.vx;
	ctrl->step.vy += shift.vy;
	ctrl->step.vz += shift.vz;
    }
    //    else if( dis > range/1.50f ) {
    //	DecayStep(ctrl,DECAY_RATE) ;
    //    }
    else {
	DecayStep(ctrl,BRAKE_RATE2) ;
	return -1;
    }


    dis = GV_VecLen3F( &ctrl->step );
    if( dis > work->max_speed) {
	GV_LenVec3F( &ctrl->step, &ctrl->step, 0.0F, work->max_speed ) ;
    }

    return 0 ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void InLineCheck						*/
/*	引数:	FVECTOR	*answer		ある点。直接求めた座標を入れる		*/
/*		FVECTOR	*pos1	  	線分を成す２点			       	*/
/*		FVECTOR	*pos2	  		〃			       	*/
/*		short	mode		比較する値	0:	vx		*/
/*							1:	vy		*/
/*							2:	vz		*/
/*	説明:	ある点が線分の中に入るようにする				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void InLineCheck(FVECTOR *answer, FVECTOR *pos1, FVECTOR *pos2, short mode){

    float	num1	= 0.0f;
    float	num2	= 0.0f;
    float	ans	= 0.0f;

    switch	(mode){
    case 0:	// vxの比較
	num1 = pos1->vx;
	num2 = pos2->vx;
	ans =  answer->vx;
	break;

    case 1:	// vyの比較
	num1 = pos1->vy;
	num2 = pos2->vy;
	ans =  answer->vy;
	break;

    case 2:	// vzの比較
	num1 = pos1->vz;
	num2 = pos2->vz;
	ans =  answer->vz;
	break;

#ifdef DEBUG_MODE	
    default:
	ASSERT(1);	//ここに来てはいけない
#endif
    }

    //		*****比較開始
    if (num1 < num2){
	if (ans < num1){	//pos1が最近接点
	    _sceVu0CopyVector(answer, pos1);
	}
	else if (num2 < ans){	//pos2が最近接点
	    _sceVu0CopyVector(answer, pos2);
	}
#ifdef DEBUG_MODE
	else {
	    ASSERT (1);	//ここに来てはいけない
	}
#endif
    }
    else if (num1 > num2){
	if (ans > num1){	//pos1が最近接点
	    _sceVu0CopyVector(answer, pos1);
	}
	else if (num2 > ans){	//pos2が最近接点
	    _sceVu0CopyVector(answer, pos2);
	}
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void GetShortestP						*/
/*	引数:	FVECTOR	*answer		求めた座標を入れる			*/
/*		FVECTOR	*point	  					       	*/
/*		FVECTOR	*pos1	  					       	*/
/*		FVECTOR	*pos2	  					       	*/
/*										*/
/*	説明:	pos1,pos2の2点間を結ぶ直線にpointから下ろす垂線の足の座標	*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void GetShortestP(FVECTOR *answer, FVECTOR *point, FVECTOR *pos1, FVECTOR *pos2){

    FVECTOR	vecV0_p1_pot;		//pos1 > point の方向ベクトル [V0]
    FVECTOR	vecV1_p1_p2;		//pos1 > pos2  の方向ベクトル [V1]
    FVECTOR	OutProOV0_V0V1;		//方向ベクトル V0 と V1 の外積 [OV0]
    FVECTOR	OutProOV1_OV0V0;	//外積 OV0 と 方向ベクトル V1 の外積 [OV1]
    float	block1, block2, block3;

    //		****直線がZ軸だった場合
    if ((pos1->vx == pos2->vx)&&(pos1->vy == pos2->vy)){
	answer->vx = pos1->vx;
	answer->vy = pos1->vy;
	answer->vz = point->vz;
	InLineCheck(answer, pos1, pos2, 2);	//vzの比較
    }
    //		****直線がY軸だった場合
    else if ((pos1->vx == pos2->vx)&&(pos1->vz == pos2->vz)){
	answer->vx = pos1->vx;
	answer->vz = pos1->vz;
	answer->vy = point->vy;
	InLineCheck(answer, pos1, pos2, 1);	//vyの比較
    }
    //		****直線がX軸だった場合
    else if ((pos1->vz == pos2->vz)&&(pos1->vy == pos2->vy)){
	answer->vz = pos1->vz;
	answer->vy = pos1->vy;
	answer->vx = point->vx;
	InLineCheck(answer, pos1, pos2, 0);	//vxの比較
    }
    //		****マトモな計算
    else {

//		***pos1 > point の方向ベクトル [V0]***
    SAT_Minus_FVECTOR(&vecV0_p1_pot, point, pos1);
//		***pos1 > pos2  の方向ベクトル [V1]***
    SAT_Minus_FVECTOR(&vecV1_p1_p2, pos2, pos1);
//		***方向ベクトル V0 と V1 の外積 [OV0]***
    _sceVu0OuterProduct(&OutProOV0_V0V1, &vecV0_p1_pot, &vecV1_p1_p2);
//		***外積 OV0 と 方向ベクトル V1 の外積 [OV1]***
    _sceVu0OuterProduct(&OutProOV1_OV0V0, &OutProOV0_V0V1, &vecV1_p1_p2);


//	プレイヤのいる位置 点 point から pos1 を含み法線 OutProOv1_OV0V0 を持つ
//      平面に垂線を下ろした場合の垂線の足と平面の交点を求める
//      以下は Ｘ の値
//    (a^2 + b^2 + c^2) X = a (ap + bq + cr - bt - cu) + s (b^2 + c^2)
//     |----block3----|        |-------block2--------|     |-block2--| :STEP1
//     |----block3----|     |---------block2---------|   |---block2--| :STEP2
//     |----block3----|     |------------------block2----------------| :STEP3

//		****STEP1****
    block1 = (OutProOV1_OV0V0.vx * pos1->vx) +
	     (OutProOV1_OV0V0.vy * pos1->vy) +
	     (OutProOV1_OV0V0.vz * pos1->vz) -
	     (OutProOV1_OV0V0.vy * point->vy) -
	     (OutProOV1_OV0V0.vz * point->vz);

    block2 = (OutProOV1_OV0V0.vy * OutProOV1_OV0V0.vy) +
	     (OutProOV1_OV0V0.vz * OutProOV1_OV0V0.vz);
        
    block3 = (OutProOV1_OV0V0.vx * OutProOV1_OV0V0.vx) +
	     (OutProOV1_OV0V0.vy * OutProOV1_OV0V0.vy) +
	     (OutProOV1_OV0V0.vz * OutProOV1_OV0V0.vz);

//		****STEP2****
//	block1に a を掛ける
    block1 = block1 * OutProOV1_OV0V0.vx;
//	block2に s を掛ける
    block2 = block2 * point->vx;

//		****STEP3****
    block2 = block2 + block1;


//	Ｘの解
    block3 = block2 / block3;

//	Ｙの解	Ｘを代入して求める(ひょっとしら誤差が大きくなる？)
//	Y = b (x - s) / a + t
    block2 = (OutProOV1_OV0V0.vy*(block3 - point->vx))/OutProOV1_OV0V0.vx + point->vy;

//	Zの解	Ｘを代入して求める(ひょっとしら誤差が大きくなる？)
//	Z = c (x - s) / a + u
    block1 = (OutProOV1_OV0V0.vz*(block3 - point->vx))/OutProOV1_OV0V0.vx + point->vz;

    answer->vx = block3;
    answer->vy = block2; 
    answer->vz = block1;
    }


    //		****線分の外ではないかのチェック
    InLineCheck(answer, pos1, pos2, 0);	//vxの比較


}

#define	NONE_MOVE	0
#define	PLUS_MOVE	1
#define	MINUS_MOVE	2
#define CHANGE_NODE_DIST	1800
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int TracePlayer3D						*/
/*	引数:	Work	*work	  					       	*/
/*	説明:	ルート上でプレイヤを追跡移動					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int TracePlayer3D(Work *work){

    FVECTOR	*target_pos, *cyp_pos;
    FVECTOR	tmp;
    FVECTOR	shift;
    short	loop;
    int		min_dist = 0;	//最近接ノードまでの距離距離
    char	min_num = 0;	//最短距離ノードナンバー
    char	now_num = 0;	//現在ノード間ナンバー
    short	node0 = 0;
    short	node1 = 0;    
    int		distance = 0;

    _sceVu0CopyVector(&tmp, EMA_CommandGetPosition());

    tmp.vy = tmp.vy += 500.0f;	//基本的にプレイヤより上に陣どる
    target_pos	= &tmp;
    cyp_pos	= &work->control.mov;	//サイファをセット

    {		/****サイファがどの点から一番近いか算出****/
	FVECTOR 	ans;
	FVECTOR		sht;
	FVECTOR		answer_point;
	short		nd0, nd1;    

	for (loop=0; loop<work->tp_num; loop++){
	    if (loop!=0){
		GetShortestP(&ans, cyp_pos,
			     &work->trace_point[loop],
			     &work->trace_point[loop-1]);
		nd0 = loop;
		nd1 = loop-1;
	    }
	    else {
		GetShortestP(&ans, cyp_pos,
			     &work->trace_point[0],
			     &work->trace_point[work->tp_num-1]);
		nd0 = 0;
		nd1 = work->tp_num-1;

		if (work->loop_flag == 1){	//ループしていたら０番無効
		    ans.vy -= 9000000.0f;
		}
	    }

	    SAT_Minus_FVECTOR(&sht, &ans, cyp_pos);	//距離を算出

#ifdef SATO_DEBUG
    MENU_Locate( 30, 50+loop*20, 0 ) ;
    MENU_SetColor( 150, 0, 0 ) ;
    MENU_Printf( "%d :: %d",
		 loop,
		 (int)_FVecLen3( &sht)
		 );
#endif
	    if ( (loop==0)||(min_dist > (int)_FVecLen3( &sht) )){//最小値の比較・保存
		now_num = loop;
		min_dist = (int)_FVecLen3( &sht);
		answer_point.vx = ans.vx;
		answer_point.vy = ans.vy;
		answer_point.vz = ans.vz;
		node0 = nd0;
		node1 = nd1;
	    }
	}
    }

    {
	short dist1, dist2;
	SAT_Minus_FVECTOR(&shift, &work->trace_point[node0], cyp_pos);	//距離を算出
	dist1 = (int)_FVecLen3( &shift);

#ifdef SATO_DEBUG
	MENU_Locate( 130, 50, 0 ) ;
	MENU_SetColor( 150, 0, 0 ) ;
	MENU_Printf( "%d:-:%d",
		     node0,
		     dist1);
#endif

	SAT_Minus_FVECTOR(&shift, &work->trace_point[node1], cyp_pos);	//距離を算出
	dist2 = (int)_FVecLen3( &shift);

#ifdef SATO_DEBUG
	MENU_Locate( 130, 70, 0 ) ;
	MENU_SetColor( 150, 0, 0 ) ;
	MENU_Printf( "%d:-:%d",
		     node1,
		     dist2);
#endif

	if (dist1 < dist2){
	    work->rnavi.next_node = node0 ;
	}
	else {
	    work->rnavi.next_node = node1 ;
	}

//printf ("!!!!!!!!!!!!CYP Action cancel\n");

	work->rnavi.p_action = 0;	//アクション番号なし
    }



    {	/****プレイヤからどの点が一番近いか算出****/
	FVECTOR 	ans;
	FVECTOR		sht;
	FVECTOR		answer_point;

	for (loop=0; loop<work->tp_num; loop++){
	    if (loop!=0){
		GetShortestP(&ans, target_pos,
			     &work->trace_point[loop],
			     &work->trace_point[loop-1]);
	    }
	    else {
		GetShortestP(&ans, target_pos,
			     &work->trace_point[0],
			     &work->trace_point[work->tp_num-1]);
		if (work->loop_flag == 1){	//ループしていたら０番無効
		    ans.vy = ans.vy - 10000.0f;
		}
	    }

	    if (ans.vy < target_pos->vy ){	//絶対下にはいかせない

	    }

	    SAT_Minus_FVECTOR(&sht, &ans, target_pos);	//距離を算出

	    if ( (loop==0)||(min_dist > (int)_FVecLen3( &sht) )){//最小値の比較・保存
		min_num = loop;
		min_dist = (int)_FVecLen3( &sht);
		answer_point.vx = ans.vx;
		answer_point.vy = ans.vy;
		answer_point.vz = ans.vz;
	    }
	}

	//  ****************柱の蔭回避
	work->kaihi_flag = 0;
	_sceVu0CopyVector(&work->kaihi_pos, &answer_point);





	culc_ko_ten(work);







	SAT_Minus_FVECTOR(&shift, &work->kaihi_pos, cyp_pos);	//距離を算出


	distance = (int)_FVecLen3( &shift);
    }


#ifdef SATO_DEBUG
	MENU_Locate( 50, 150, 0 ) ;
	MENU_SetColor( 150, 0, 0 ) ;
	MENU_Printf( "M:%d  N:%d",
		     min_num,
		     now_num);
#endif


	// ********ルートを真面目に巡ってツイビ
    if (min_num != now_num){	
	if (min_num > now_num){	//正順廻り
	    SAT_Minus_FVECTOR(&shift, &work->trace_point[node0], cyp_pos);
	    distance = (int)_FVecLen3( &shift);

#ifdef SATO_DEBUG
	    MENU_Locate( 50, 170, 0 ) ;
	    MENU_SetColor( 150, 0, 0 ) ;
	    MENU_Printf( "DIS:%d",
			 distance);
#endif

	    if (distance < CHANGE_NODE_DIST){//ノードに到着
		node0 = node0+1;
		if (node0 >= work->tp_num){
		    node0 = 0;
		}
		SAT_Minus_FVECTOR(&shift, &work->trace_point[node0], cyp_pos);	//距離を算出
		distance = (int)_FVecLen3( &shift);
	    }

	}
	else {
	    SAT_Minus_FVECTOR(&shift, &work->trace_point[node1], cyp_pos);	//距離を算出
	    distance = (int)_FVecLen3( &shift);

#ifdef SATO_DEBUG
	MENU_Locate( 50, 170, 0 ) ;
	MENU_SetColor( 150, 0, 0 ) ;
	MENU_Printf( "DIS:%d",
		     distance);
#endif

	if (distance < CHANGE_NODE_DIST){//ノードに到着
		node1 = node1-1;
		if (node1 < 0){
		    node1 = work->tp_num-1;
		}
		SAT_Minus_FVECTOR(&shift, &work->trace_point[node1], cyp_pos);	//距離を算出
		distance = (int)_FVecLen3( &shift);
	    }

	}

    }
//		***************近接追尾***************
    else {
    }


    //		***************実際の移動***************
    //加速度の制限
    if (distance > CYP_ACCELE){
	GV_LenVec3F( &shift, &shift, 0.0F, CYP_ACCELE ) ;
    }

    DecayStep(&work->control,DECAY_RATE) ;
    if 	( ((work->kaihi_flag == 0)&&(distance > 1500)) || 
	  (distance > 200) ){
	work->control.step.vx += shift.vx;
	work->control.step.vz += shift.vz;
	work->control.step.vy += shift.vy;
    }
    else {
	DecayStep(&work->control,BRAKE_RATE2) ;
    }

    if ((int)_FVecLen3( &work->control.step) > 60.0f){//work->max_speed*1.3f){	//速度制限
	GV_LenVec3F( &work->control.step, &work->control.step, 0.0F, 60.0f); //work->max_speed*1.3f) ;
    }

    return 1;
}

#ifdef DEBUG_MODE
    extern void *NewLineView(FVECTOR * ,int,u_char,u_char,u_char) ;
#endif

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int MoveRoute							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ルートを廻る				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int MoveRoute(Work *work){
    ROUTENAVI *rnavi ;
    FVECTOR	aim;
    int brake = 0;

    rnavi = &work->rnavi ;

    /*移動制御と到達チェック*/

    if(DirectTrace3D(work,1000.0F,&brake) < 0 ){
	/*ポイント到達*/
	if(rnavi->p_acttime == 0 ){

	    /*待機時間無しなのですぐに次ポイントへ*/
	    work->nowpos = work->trg.pos ;
	    CYP_SetNextnode( rnavi, work ) ;
	    SetPointAction( work );
	}else {
	    /*待機モードへ*/
	    work->mode = ACTION_MODE ;
	}
    }else {
	/*移動中の顔方向チェック*/
	/*発見していなければ*/
	/*注視点見ながら移動*/
	if(rnavi->p_actstatus & PA_CON_AIM_FACE_MOVE){
	    aim = rnavi->aimnodes[(int)rnavi->next_node];
	    Dir_from_2Vec(&work->camera,&aim,&work->control.turn);
//	    line[0] = work->camera ;
//	    line[1] = aim ;
//	    NewLineView( &line[0] ,1,0,255,0) ;
	}
	else if(work->rot_flag == CYP_FLAG_FIXROT){ 

	    work->control.turn.vx = work->look_dir.vx;
	    work->control.turn.vy = work->look_dir.vy;
	    work->control.turn.vz = work->look_dir.vz;
	}
	else {
	    /*進行方向*/
	    work->control.turn.vy = work->trg.dir ;
	    work->control.turn.vx = 0 ;
	}
    }
    return	brake ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void ActionSeq							*/
/*	引数:	Work *work   						       	*/
/*	説明:	時間が来るまで待機						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void ActionSeq(Work *work)
{
    if(work->rnavi.p_dir >=0 ) {
	if(work->rot_flag != CYP_FLAG_FIXROT){ 	// 待機場所での注視禁止
	    work->control.turn.vy = work->rnavi.p_dir ;
	    work->control.turn.vx = 0 ;
	}
    }
    if((work->rnavi.p_acttime>0)
       &&(work->rnavi.p_acttime != P_TIME_WAIT)){

	/*待機時間中*/
	work->rnavi.p_acttime--;
    }
    else if((work->rnavi.p_acttime <= 0)
	     &&(work->rnavi.p_acttime != P_TIME_WAIT)){

	/*待機終了 次のポイントセット*/
	work->nowpos = work->trg.pos ;
	CYP_SetNextnode( &work->rnavi, work ) ;
	
	SetPointAction( work );
	work->mode = MOVE_MODE ;
    }
    /*メッセージ受信するまで待機*/	
    else if (work->rnavi.p_acttime == P_TIME_WAIT){

	if(work->status == CYP_ACTIVE ){
	    /*待機終了 次のポイントセット*/
	    work->nowpos = work->trg.pos ;
	    CYP_SetNextnode( &work->rnavi, work ) ;
	    SetPointAction( work );
	    work->mode = MOVE_MODE ;
	}
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetFaceDir							*/
/*	引数:	Work *work   						       	*/
/*	説明:	カメラの向きを設定する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetFaceDir(Work *work){
    FMATRIX	tmpmat;
    FVECTOR	trans;

    trans.vx = 0.0f;
    trans.vy = -270.0f;
    trans.vz = 500.0f;

    work->eye.rot = work->control.rot ;
    DG_SetPos( &BODYWORLD(&work->body, CAMERA_PARTS));

	DG_MovePos( &trans );

    DG_GetPos( &tmpmat ) ;
    GV_MatToVec(&tmpmat,&work->camera);

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetObjs							*/
/*	引数:	Work *work   						       	*/
/*	説明:	オブジェの表示				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetObjs(Work *work){
    //	FVECTOR	pos= {0.0f,0.0f,0.0f,0.0f};
	FMATRIX	world;

	/*ボディをゆっくり,羽を早く回転*/
	work->rot_fin  += ROT_SPEED;
	work->rot_fin  &= 4095 ;

	//胴体の傾き
	//DG_SetPos2( &work->control.mov, &work->b_ctrl.rot );
	//	DG_MovePos( &pos ) ;
	//	DG_GetPos( &world ) ;


	/*胴体*/
	{	
	    FVECTOR	tmpvec= {0.0f,0.0f,0.0f,1.0f};
	    SVECTOR	tmp2 = {work->rot_body.vx/5,
				work->rot_body.vy/5,
				work->rot_body.vz/5,
			    0};
	    SVECTOR	dummy_rot = {0,0,0,0};
			    

	    work->kyodo_timer += (TIME_BASE+RAND(20));

	    // **上下のゆらゆら
	    tmpvec.vy += sinf(M_PI*work->kyodo_timer/1200.0f)*100.0f;

	    if (work->kyodo_timer > 2400.0f){
		work->kyodo_timer -= 2400.0f;
	    }


	    DG_SetPos2( &work->control.mov, &dummy_rot);

	    // *****銃発射の発射振動
	    if (work->fire_flag){	
		SVECTOR	tmprot = {0,0,0,0};
		FVECTOR	tmpsft= {0.0f,0.0f,-50.0f,1.0f};
		FMATRIX tmpmat;

		_sceVu0CopyMatrix(&tmpmat, &work->body.objs->objs[1].world);
		_sceVu0CopyVector(&tmpmat.m[3], &DG_ZeroVector);

		DG_SetPos( &tmpmat );
		tmprot.vy = RAND(81)-40;
		DG_RotatePos(&tmprot);
		DG_MovePos(&tmpsft);
		DG_GetPos( &tmpmat);
		_sceVu0CopyVector(&tmpsft, tmpmat.m[3]);

		DG_SetPos2( &work->control.mov, &dummy_rot);
		DG_MovePos(&tmpsft);
		DG_GetPos( &world ) ;
		_sceVu0CopyVector(&work->control.mov, world.m[3]);
	    }

	    DG_MovePos(&tmpvec);
	    DG_GetPos( &world ) ;
	    DG_RotatePos( &tmp2 );

	}

	/*胴体設置*/
	DG_PutObjs( work->body.objs );
	GM_GroupObjs( work->body.objs, work->control.map ) ;
	DG_GetPos( &work->body.objs->objs[0].world  ) ;

	/*羽*/
	DG_SetPos( &world ) ;
	DG_MovePos( &work->body.objs->objs[2].trans ) ;
	{	
	    SVECTOR tmp = {0,work->rot_fin,0,0};
	    DG_RotatePos( &tmp );
	}
	DG_GetPos( &work->body.objs->objs[2].world  ) ;


	/*頭*/
	{
	    FMATRIX	tmpmat;
	    DG_SetPos( &work->body.objs->objs[0].world  ) ;
	    DG_MovePos( &work->body.objs->objs[1].trans ) ;
	    DG_GetPos( &tmpmat ) ;

	    *((FVECTOR*)world.m[3]) = *((FVECTOR*)tmpmat.m[3]);
	}
	DG_SetPos( &world ) ;
	//	DG_MovePos( &work->body.objs->objs[1].trans ) ;
	DG_RotatePos( &work->eye.rot ) ;

	if (work->fire_flag){	// *****発射振動
	    SVECTOR tmprot = {40,0,0,0};
	    DG_RotatePos( &tmprot ) ;
	}
	work->fire_flag = OFF;


	DG_GetPos( &work->body.objs->objs[1].world  ) ;

	work->rctrl.dir = work->eye.rot.vy ;	//レーダ
#ifdef DEBUG_MODE
    DG_SetPos2( work->eye.eyepos, &work->eye.rot ) ;
    DG_GetPos(&work->eyeview) ;
#endif

}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void MoveTarget							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ターゲットを移動する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void MoveTargets(Work *work)
{
    FMATRIX  mat;
    FVECTOR	tmpvec = {0.0f,-40.0f,0.0f,0.0f};
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
	tmpvec.vy = -10.0f;
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

	tmpvec.vy = -210.0f;

    tmpvec.vz = -30.0f;
    DG_SetPos( &work->body.objs->objs[1].world);
    DG_MovePos( &tmpvec ) ;
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->h_trg,
		       &mat,work->control.map);

    //ロータのターゲット 
    tmpvec.vx = 0.0f;
	tmpvec.vy = 30.0f;
    tmpvec.vz = 0.0f;
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &tmpvec ) ;
    DG_GetPos( &mat  ) ;
    GM_MoveTarget2Map( &work->r_trg,
		       &mat,work->control.map);
}


#ifdef SATO_DEBUG
extern void RouteView(ROUTENAVI *);


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Debug_Print						*/
/*	引数:	Work *work   						       	*/
/*	説明:	デバッグ用のプリント			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void DebugPrint(Work *work)
{
#if 1
    ROUTENAVI	*rnavi;
    rnavi = &work->rnavi ;

    DEV_DebugMode(&work->eye);		//視界表示

    MENU_Locate( 30, 420, 0 ) ;
    MENU_SetColor( 150, 0, 0 ) ;
    MENU_Printf( "STEP :%d   discv: %d  alert: %d   al_mod %d",
		 G1_STEP,
		 work->eye.discv_time,
		 work->eye.alert_time,
		 GM_AlertMode==ALERT_MODE_ALERT
		 );

#endif
}

#endif


/**************************<-------local function------>*************************/
/*	名前:	int DEV_EyeInfoCheck						*/
/*	返値:									*/
/*	引数:	EYEPARAM	*eye						*/
/*		HZX_GROUP_ID	hzx_id						*/
/*	説明:	視界チェック全般を行なう					*/
/********************************************************************************/
int CYP_EyeInfoCheck(EYEPARAM *eye , HZX_GROUP_ID hzx_id ){
    COMMANDER	*commander;

    commander = COM_GetCommander();

    if(commander->status & CMST_ENEMY_SIGHT_OFF ){
	/*視界off*/
	DEV_No_Sight( eye ) ;
	return 0 ;
    }


    // *****通常チェック*****
    if(SIG_EyeCheck( eye )){
	if(ENE_EyeOnlineCheck( hzx_id, eye->eyepos, eye->trgpos ) ){
	    // *****壁
	    DEV_No_Sight( eye ) ;
	    return 0 ;
	}else {
	    // *****見えた
	    DEV_In_Sight( eye ) ;
	    return 1 ;
	}
    }else {
	DEV_No_Sight( eye ) ;
	return 0 ;
    }
}






void Ema_SendMess(int buffer)	// 4-5
{
    GV_MSG msg ;

    printf( "Send a message Go %d\n",buffer ) ;

    msg.address = GV_StrCode( "エマ" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}






/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_warn							*/
/*	引数:	Work *work   						       	*/
/*	説明:	実行関数				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_gun_fire(Work *work){
    
    if( work->gun_count >= 0 ){ 
	// *************ガンは通報の代わりに発砲
	if ((work->gun_count/5)%4==0){	
	    extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
	    int type ;
	    FMATRIX tmpmat;
	    FVECTOR trans = {0.0f, -280.0f, 0.0f};
	    FVECTOR shift = {20.0f, -275.0f, 85.0f};
	    SVECTOR tmprot = {-1024, 0, 0};
	    
	    //	弾と軌跡発生
	    type = (BUL_TYPE_VISIBLE|BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_NO_PLAYER);
	    DG_SetPos( &work->body.objs->objs[CAMERA_PARTS].world) ;
	    DG_MovePos(&trans );

	    tmprot.vx += RAND(121)-60;
	    tmprot.vy += RAND(121)-60;

	    DG_RotatePos(&tmprot );
	    DG_GetPos( &tmpmat ) ;
	    NewBullet( &tmpmat,type, PLAYER_SIDE, 25, 5, 20000, 1000, WP_m4 ) ;
	    GM_SeSetMode( SD_E_FAMAS03 ,(FVECTOR*)tmpmat.m[3],GM_SEMODE_BOMB ) ;
	    
	    trans.vy = -650.0f;
	    DG_MovePos(&trans );
	    DG_GetPos( &tmpmat ) ;
	    AN_MazzleMeca (&tmpmat, &DG_ZeroVector);	// まずるふらっしゅ
	    
	    //メカカートリッジ
	    DG_SetPos( &work->body.objs->objs[1].world) ;
	    DG_MovePos(&shift );
	    DG_RotatePos(&tmprot);
	    DG_GetPos( &tmpmat ) ;
	    AN_CartridgeMeca_E( &tmpmat , &DG_ZeroVector);	
	    work->fire_flag = 1;
	}
	work->gun_count -= TIME_BASE;
    }
    else if( work->rest_count >= 0 ){ 
	work->rest_count -= TIME_BASE;
    }
    else {
	work->gun_count = (work->guntime_fix+RAND(work->guntime_rand))*5;
	work->rest_count = (work->resttime_fix+RAND(work->resttime_rand))*5;
    }
}








/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Do_Act							*/
/*	引数:	Work *work   						       	*/
/*	説明:	実行関数				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Do_Act(Work *work)
{
    OBJECT 	*body ;
//    int		pbrake = 0;	//SE用の動いているかどうかフラグ
//    EYEPARAM	*eye = &work->eye;
//    short	tmp_uniq_id, loop;
    body = 	&work->body;

    GM_ActControl(&work->control) ;
    CheckMessage(work);	//メッセージのチェック

    if (work->status == CYP_WAIT){
	return;
    }


    switch(G1_STEP){ // ****************ステップ分岐****************

    case CYP_NORMAL:	// *****ノーマルモード*****

	//視界内の敵チェック
	if (work->cyp_eye_close == OFF){
	    if (CYP_EyeInfoCheck( &work->eye ,work->control.hzx_id )){
		SET_G1_STEP(work, CYP_WARN);
		return;
	    }
	}
	switch(work->mode){

	    case MOVE_MODE :	//移動モード
		MoveRoute(work);
		break;

	    case ACTION_MODE :	//待機場所へ補正
		if(DirectTrace3D(work,1000.0F,NULL)<0){
		}
		ActionSeq(work);
		break;
	}
	break;

    case CYP_WARN:	// *****警戒モード*****
    {
	EYEPARAM *eye = &work->eye ;
	
	//赤びっくり通報
	if( ((eye->discv_time ==0 )&&(eye->alert_time == SIGHT_ALERT)) ||
	    (( GM_AlertMode == ALERT_MODE_ALERT )&&(eye->alert_time == SIGHT_ALERT)) ||
	    (eye->discv_time == 1 ) ){

	    GM_SeSetMode( SD_E_BIKKRI01 , eye->eyepos,
			  GM_SEMODE_BOMB ) ;


	    CallActHeadMarks( work->hmk_work_p, HMK2_TYPE_RED_AT_MIN);


	    COM_DetectVibration( ) ;
	    SET_G1_STEP(work, CYP_ALERT);
	}
	if(eye->discv_time > 0 ) eye->discv_time --;
		
	/*びっくりセット*/
	CYP_HMK_Check(&work->eye,
		      &work->body.objs->objs[CAMERA_PARTS].world,work->hmk_work_p);


	/*目標物へ向く*/
	DecayStep(&work->control,BRAKE_RATE2);
	TracePlayer3D(work);	//プレイヤツイビ
	Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,
		      &work->control.turn);//敵の方を向く
	break;
    }

    case CYP_ALERT:		// *****警戒モード*****
	
	if ( (DEV_EyeInfoCheck( &work->eye ,work->control.hzx_id)) || 
	     (work->is_attack == 1) ){
	    work->eye.alert_time = SIGHT_ALERT;

	    cyp_gun_fire(work);

	    if (work->is_attack == 0){
		EMA_CommandEneAttack(work->name);
		work->hom.status |= HOMING_EMA_IN_SIGHT;
		work->is_attack = 1;
	    }

	}
	else if ( (GM_AlertMode != ALERT_MODE_ALERT)&&
		  (GM_AlertMode != ALERT_MODE_AVOID)&&(work->eye.alert_time==0) ){
	    SET_G1_STEP(work, CYP_NORMAL);
	    CYP_SetNextnode( &work->rnavi, work ) ;//ルートに復帰
	}
	// *目標物へ向く
	TracePlayer3D(work);	//プレイヤツイビ
	Dir_from_2Vec(work->eye.eyepos,work->eye.trgpos,
		      &work->control.turn);//敵の方を向く
	break;	

    }





    {
	    work->control.step.vx += work->damage_force.vx*0.1f;
	    work->control.step.vy += work->damage_force.vy*0.1f;
	    work->control.step.vz += work->damage_force.vz*0.1f;
	    work->damage_force.vx = work->damage_force.vx * 0.6f;
	    work->damage_force.vy = work->damage_force.vy * 0.6f;
	    work->damage_force.vz = work->damage_force.vz * 0.6f;


	    work->rot_body.vx = work->rot_body.vx * 0.92;
	    work->rot_body.vz = work->rot_body.vz * 0.92;

	    work->rot_body.vx += work->damage_rot.vx;
	    work->rot_body.vz += work->damage_rot.vz;
	    work->damage_rot.vx = 0;
	    work->damage_rot.vz = 0;

    }



    work->control.interp = 8 ;

    //ライトマトリックスの取得
    DG_GetLightMatrix( &work->control.mov, work->lights );
	
    SetObjs(work);	//本体位置設定
    SetFaceDir(work);	//カメラ方向設定
    MoveTargets(work);	//ターゲットの移動


#ifdef SATO_DEBUG
    DebugPrint(work);	//いろいろな情報表示
#endif

}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void InitRadarParam						*/
/*	引数:	Work	*work							*/
/*	説明:	レーダーシステムのイニシャライズ				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void InitRadarParam(Work *work){
    RADAR_CTRL *rctrl ;
    rctrl = &work->rctrl;
    rctrl->angle = (work->eye.range.vy)*2 ;/* 視野 */
    rctrl->col = RADAR_COLOR_YELOW ;	/* 視野描画色 */
    rctrl->range = work->eye.length;	/* 視力 */
    GM_RadarSetVRange( rctrl, 5000 , -3000 );
}

	    extern void *NewHarrierFlare( FVECTOR *pos, float size );


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int GetResources						*/
/*	引数:	Work *work   						       	*/
/*		int  name							*/
/*		int  where							*/	
/*	説明:	ワークの確保をする			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int GetResources(Work *work, int name, int where)
{
    int model;
    SVECTOR	range ;
    float	length ;
    FVECTOR pos = {0.0f, 0.0f, 0.0f, 0.0f};
    SVECTOR rot = {0, 0, 0, 0}; 
    short	loop;
    work->name = name;

    // ********コントロールのイニシャライズ********
    GM_InitControl( &work->control, name, where );	//コントロールシステムに登録
    GM_ConfigControlMessageCheck( &work->control );	//メッセージ受信処理を行なう 	
    GM_ConfigControlMapCheck( &work->control );		//マップ変更チェックを行なう

    GM_ConfigControlPosition( &work->control, &pos, &rot);


    work->control.interp = 16 ;
    
	/*CONTROLフラグ*/
    work->control.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK|	//床あたり・壁あたりのチェックをしない
	CTRL_SKIP_GET_ADDRESS				//アドレス変更処理をしない
	|CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK	//近接壁・移動直線あたりチェックをしない
	|CTRL_SKIP_TRAP					//トラップイベント処理をしない
	);
    /*胴体の方向とゆれを乱数セット*/

    /*最高速度*/
    work->max_speed = (float) GCL_GetOptionValue( 's', DEF_MAX_SPEED );

    /*探査目標*/
    work->eye.trgpos = EMA_CommandGetPosition();	//プレイヤをセット
    /*顔の位置*/
    work->eye.eyepos = &work->camera;		//視線の原点をセット
    /*顔の向き*/
    work->eye.rot  = DG_ZeroSVector;
    work->face_y_buf = (int) work->eye.rot.vy;	

    /*視野角*/
    range.vx = GCL_GetOptionValue( 'x', 512 ) ; 
    range.vy = GCL_GetOptionValue( 'y', 512 ) ;
    range.vz = 0;

    /*視力*/
    length = GCL_GetOptionValue( 'i', CYP_EYE_S_DEF  );

    /*破壊プロック*/
    work->brake_proc_id = GCL_GetOptionValue( 'b', 0 );


    if( GCL_GetOption( 'g' ) != NULL ){
	work->guntime_fix = GCL_GetInt( GCL_NextStr() );
	work->guntime_rand = GCL_GetInt( GCL_NextStr() );
	work->resttime_fix = GCL_GetInt( GCL_NextStr() );
	work->resttime_rand = GCL_GetInt( GCL_NextStr() );
    }
    else {
	work->guntime_fix = 30;
	work->guntime_rand = 30;
	work->resttime_fix = 30;
	work->resttime_rand = 30;
    }

    /*到達プロック*/
    if( GCL_GetOption( 'C' ) != NULL ){
	work->reach_proc_id = GCL_GetInt( GCL_NextStr() );
	work->reach_proc_route = GCL_GetInt( GCL_NextStr() );
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		work->reach_proc_route += COM_GetRootOfset( ) ;
	}
	work->reach_proc_node = GCL_GetInt( GCL_NextStr() );
    }


    DEV_InitEyeParam(&work->eye,&range,length);	//視界パラメータ設定
    work->alert = 0;
    work->mode = MOVE_MODE;

    /* モデル */
    model = GV_StrCode("gcyp") ;

    // ****視認敵死体＆眠り兵記憶領域初期化
    for (loop=0; loop<UNIQ_LIST_MAX; loop++){
	work->uniq_id_list[loop] = -1;	//発見した死体や眠り兵を覚えておく
    }


    work->se_sw = (work->route%5)*2 ;
    switch (work->route%4) {
    case 0 :
	work->se_num = SD_E_PHYFAR01 ;
	work->still_se_num = SD_E_PHYFAR05 ;
	break;
    case 1 :
	work->se_num = SD_E_PHYFAR02 ;
	work->still_se_num = SD_E_PHYFAR06 ;
	break;
    case 2 :
	work->se_num = SD_E_PHYFAR03 ;
	work->still_se_num = SD_E_PHYFAR07 ;
	break;
    case 3 :
	work->se_num = SD_E_PHYFAR04 ;
	work->still_se_num = SD_E_PHYFAR08 ;
	break;
    default :
	work->se_num = SD_E_PHYFAR01 ;
	work->still_se_num = SD_E_PHYFAR05 ;
    }

    GM_InitObject(&(work->body),model,(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION) );
    /*ctrlからobjectを参照*/
    GM_ConfigControlObject( &work->control, &work->body ) ;
    GM_ConfigObjectLight(&(work->body),work->lights) ;
    GM_ConfigControlMapID( &work->control ) ;
    DG_SetPos2( &work->control.mov, &work->control.rot ) ;
    DG_PutObjs( work->body.objs );

    /*ターゲット設定*/
    SetTarget(work);

    //ルート設定*/
    //    work->now_node = 0;
    if ( GCL_GetOption( 'r' ) != NULL ){
	short	now_node;

	work->route = GCL_GetNextInt() ;
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		work->route += COM_GetRootOfset( ) ;
	}
	now_node = GCL_GetNextInt();

	ENE_InitRouteNavi( &work->rnavi, work->route, now_node );	//ルートナビ初期化
	CYP_InitTracePoint( work );
	SetPointAction( work );

    /*ルートポイントに配置*/
    work->trg.pos.vx = work->control.mov.vx = work->rnavi.nodes[ now_node ].vx ;
    work->trg.pos.vy = work->control.mov.vy = work->rnavi.nodes[ now_node ].vy ;
    work->trg.pos.vz = work->control.mov.vz = work->rnavi.nodes[ now_node ].vz ;
    work->nowpos.vx = work->control.mov.vx ;
    work->nowpos.vy = work->control.mov.vy ;
    work->nowpos.vz = work->control.mov.vz ;

    }else {
	return 0 ;
    }




    //ヘッドマーク表示用の子アクターを呼ぶ

    work->hmk_work_p = NewControl_Headmark3( &work->body.objs->objs[CAMERA_PARTS].world,
					     NULL, &work->control);
    GV_SetActorChild( work, work->hmk_work_p);

    //レーダーのイニシャライズ
    GM_InitRadarControl(&work->rctrl,&work->camera,
			RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT, work->control.map ) ;
    InitRadarParam(work);


#ifdef DEBUG_MODE	//デバッグ用視界表示
    GV_SetActorChild( work,NewEyeView( &work->eyeview,
	(int)length,(int)(range.vy)/2,(int)(-range.vx)/2,(int)range.vx,COM_GetCommander(),NULL));
#endif
#ifdef SATO_DEBUG
	//当たり判定を表示
	NewTargetView( &work->h_trg, 0, 0, 255 ) ;
	{
	    short loop;
	    for (loop=0; loop<6; loop++){
		NewTargetView( &work->b_trg[loop], 255, 0, 0 ) ;
	    }
	}
	NewTargetView( &work->r_trg, 0, 255, 0 ) ;
#endif
    //  ********ホーミング設定
    GM_SetHomingTrg( &work->hom, &work->body.objs->world, &work->body, &work->control.hzx_id 
	, &work->control, HOMING_ENEMY ) ;

    GM_PutHomingTrg( &work->hom );

    //コントロールシステムで移動
    GM_ActControl(&work->control) ;

    //ライトマトリックスの取得
    DG_GetLightMatrix( &work->control.mov, work->lights );
	
    SetObjs(work);	//本体位置設定
    SetFaceDir(work);	//カメラ方向設定
    MoveTargets(work);	//ターゲットの移動
	return 1;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ワークの解放をする			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Die(Work *work)
{
    short loop;
    GM_FreeObject(&(work->body));
    GM_FreeControl( &work->control);
    GM_FreeTarget( &work->h_trg ) ;
    GM_FreeTarget( &work->r_trg ) ;
    GM_FreeRadarControl( &work->rctrl);
    for(loop=0; loop<6; loop++){
	GM_FreeTarget( &work->b_trg[loop] );
    }
    GM_FreeHomingTrg( &work->hom );
}

/*******************************<Global function>********************************/
/*	名前:	void *NewCypher							*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewCypher4snipe( name , where )
int	name ;
int	where ;
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Do_Act,Die) ;
	GV_ActorEX( &work->actor ) ;
	if(!GetResources( work,name,where )){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}





