//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	routemdl.c
	ルート移動するデモ人形  *NewSigRouteModel
	2000/04/18 K.Sigeno
	$Id: routemdl2.c,v 1.1.1.3 2002/11/19 11:49:27 Yoshizawa1 Exp $
*/
//PA_CON_NO_CHECK_DIS flag
//		if ( work->rnavi.p_actstatus & PA_CON_NO_CHECK_DIS  ) {

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"
#include	"korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"

/* ワーク */
#define MAX_PROC (8)
#define DBG_MOT_NUM		(0x01)	//再生中モーション番号表示
#define DBG_ARROW		(0x02)	//内部の方向表示
#define DBG_PRINT_ALL	(0x04)	//内部の全パラメータを毎フレーム表示	
#define DBG_ROUTE_VIEW	(0x08)

#define DEMO_LIFE	(128)
#define NPC_GAGE_LEVEL	(1)

//name
#if 0
enum {
	TRG_HEAD = 0 ,
	TRG_BODY,
	TRG_HAND1_R ,
	TRG_HAND2_R ,
	TRG_HAND1_L ,
	TRG_HAND2_L ,
	TRG_LEG1_R ,
	TRG_LEG2_R ,
	TRG_LEG1_L ,
	TRG_LEG2_L ,
	TRG_PARTS_MAX
} ;
#else

#define	TRG_PARTS_MAX	(21)

#endif

static int TargetPartsName[TRG_PARTS_MAX]={
	HUMAN21_ATAMA ,
	HUMAN21_ONAKA ,
	HUMAN21_MIGI_UDE1 ,
	HUMAN21_MIGI_UDE2 ,
	HUMAN21_HIDARI_UDE1 ,
	HUMAN21_HIDARI_UDE2 ,
	HUMAN21_MIGI_ASHI1 ,
	HUMAN21_MIGI_ASHI2 ,
	HUMAN21_HIDARI_ASHI1 ,
	HUMAN21_HIDARI_ASHI2 
};
//#define TRG_PARTS_MAX 1

enum {
	TRG_TYPE_HEAD,
	TRG_TYPE_BODY,
	TRG_TYPE_HAND_R,
	TRG_TYPE_HAND_L,
	TRG_TYPE_LEG_R,
	TRG_TYPE_LEG_L,
	TRG_TYPE_MAX,
};
static int TargetPartsObjNum[TRG_TYPE_MAX]={
	HUMAN21_ATAMA ,
	HUMAN21_ONAKA ,
	HUMAN21_MIGI_UDE1 ,
	HUMAN21_HIDARI_UDE1 ,
	HUMAN21_MIGI_ASHI1 ,
	HUMAN21_HIDARI_ASHI1 
};
static int TargetPartsType[21]={
TRG_TYPE_BODY,
TRG_TYPE_BODY,
TRG_TYPE_BODY,
TRG_TYPE_HAND_R,
TRG_TYPE_HAND_R,
TRG_TYPE_HAND_R,
TRG_TYPE_HAND_R,
TRG_TYPE_HAND_R,
TRG_TYPE_HAND_R,
TRG_TYPE_HAND_R,
TRG_TYPE_HAND_R,
TRG_TYPE_HEAD,
TRG_TYPE_HEAD,
TRG_TYPE_LEG_R,
TRG_TYPE_LEG_R,
TRG_TYPE_LEG_R,
TRG_TYPE_LEG_R,
TRG_TYPE_LEG_L,
TRG_TYPE_LEG_L,
TRG_TYPE_LEG_L,
TRG_TYPE_LEG_L,
} ;

static int DamObjNum[TRG_TYPE_MAX]={
	HUMAN21_ATAMA ,
	HUMAN21_ONAKA ,
	HUMAN21_MIGI_UDE1 ,
	HUMAN21_HIDARI_UDE1 ,
	HUMAN21_MIGI_ASHI1 ,
	HUMAN21_HIDARI_ASHI1 
} ;

#define DAM_SE_MAX	(4)
#if 1
typedef	struct _Work {
	GV_ACT_EX	actor ;
	OBJECT		body ;
	OBJECT		item ; /*持ち物*/
	FMATRIX		lights[2] ;
	CONTROL		control;
	ROUTENAVI	rnavi;	/*ルート誘導*/
	TRGPOINT	trg;	/*目標情報*/
	RADAR_CTRL	rctrl ;
	GM_GageSet	gage ;
	TARGET		parent ;
	POWER_TARGET	b_power;
	TARGET		parts[TRG_PARTS_MAX] ;
	int			proc_id[MAX_PROC] ;
	u_char		proc_pos[MAX_PROC];
	int			proc_num;
	int			route ; /*ルート番号*/
	int			node ;	/*ノード番号*/
	int			name ;
	int			move_mot; /*移動モーション*/
	int			death_mot; /*死亡モーション*/
	int			dam_mot[TRG_TYPE_MAX]; /*ダメージモーション*/
	int			mode ;
	int			status ;
	int			debug ;
	int			talk_name ;
	int			life ;
	int			mot_buf ;
	int			mode_buf ;
	int			dam_se[DAM_SE_MAX] ;
	int			dam_se_num ;
	int			death_se ;
	int			death_proc ;
} Work ;
#endif

enum {
	MOVE_MODE,ACTION_MODE,FORCE_MODE,DAM_MODE,DEATH_MODE
};
/*message*/
enum {
	MSG_NONE,MSG_ROUTE_CHANGE,MSG_LOOP_END,MSG_SIRO_HATENA,MSG_ROUTE_NODE
	,MSG_ROUTE_NODE_WARP,MSG_AKA_BIKKURI,MSG_FORCE_MOTION,MSG_MOVE_MOT_SET
};
/*シナリオ指定ステータス*/
#define RADAR_ON (0x02)
#define MT_ANIM	(0x04)
#define SAME_POS_DIS	(250)



#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE|DG_FLAG_IRREACTION)
#define	ITEM_FLAG (DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
/*モーション補完*/
extern int BP_FRAMES_PER_SEC();
#define INTERP   10*(300/BP_FRAMES_PER_SEC())



//#define		DEBUG_TARGET_NAME GV_StrCode( "オルガ" )
#define		DEBUG_TARGET_NAME 0




#define	EQUIP_HLST		0x00000001
#define	EQUIP_KNIF		0x00000002
#define	EQUIP_KNIFCASE	0x00000004
#define	EQUIP_MAG		0x00000008
#define	EQUIP_BACKPACK	0x00000010
#define	EQUIP_RADIO		0x00000020
#define	EQUIP_NSIGHT	0x00000040
#define	EQUIP_WEAPON	0x00000080
#define	EQUIP_WPFREE	0x00000100
#define	EQUIP_NSIGHT_OPEN	0x00000200
#define	EQUIP_LIGHT_R	0x00000400	/* ライト右手 モデルは'aks,aks_sp,tnr_frashlight に対応 */
#define	EQUIP_LIGHT_L	0x00000800	/* ライト左手 モデルは'aks,aks_sp,tnr_frashlight に対応  */
#define	EQUIP_LEFT		0x00001000	/* 左手にモデル持つ（モデル名を指定する必要有り） */

#define	EQUIP_GPS		0x00002000	/* プラントゴル兵 */
#define	EQUIP_GPA		0x00004000	/* プラント攻撃ゴル兵 */

#define		BODY_SIZE_X		(500.0f)
#define		BODY_SIZE_Y		(1000.0f)
#define		BODY_SIZE_Z		(500.0f)



#define	EUIP_NORMAL_GPS	(EQUIP_HLST|EQUIP_KNIF|EQUIP_KNIFCASE|EQUIP_MAG|EQUIP_RADIO|EQUIP_WEAPON|EQUIP_WPFREE|EQUIP_LIGHT_R|EQUIP_GPS)
#define	EUIP_NORMAL_GBS	(EQUIP_HLST|EQUIP_KNIF|EQUIP_KNIFCASE|EQUIP_MAG|EQUIP_RADIO|EQUIP_WEAPON|EQUIP_WPFREE|EQUIP_LIGHT_R)
#define	EUIP_NORMAL_W24	(EQUIP_HLST|EQUIP_KNIF|EQUIP_KNIFCASE|EQUIP_MAG|EQUIP_RADIO|EQUIP_WEAPON|EQUIP_GPS)

extern void RouteView(ROUTENAVI *) ;
extern void GM_MouthAnimation( int , DG_EVMOBJ * ) ;

static void CheckPointMode(Work *) ;
static void ForceChangeMot(Work * ,int) ;

static inline void setfvec(FVECTOR *fv,float x,float y,float z){
	fv->vx = x ;
	fv->vy = y ;
	fv->vz = z ;
}

static void SetNeedle( OBJECT *body, int n_obj, FVECTOR *pos, FVECTOR *dir )
{
    extern void VertexSearch( FVECTOR *vans, FVECTOR *nans,
			      DG_OBJS *objs, int objnum, FVECTOR *target) ;
    extern void *NewAttachment4_called( int model_name, FVECTOR *v, SVECTOR *r,
					OBJECT *target, int objnum, FVECTOR *x,int angle_limit,
					int frames ) ;
    FVECTOR	vans, nans ;
    int model_name ;

	/* 麻酔弾 */
	model_name = GV_StrCode( "m92_bul2" ) ;
    VertexSearch( &vans, &nans, body->objs, n_obj, pos );
    NewAttachment4_called( model_name, dir, NULL, body, n_obj, &vans, 512, 4 );
}
static void SetBulletDamMode(Work	*work ,int dam_mot){
	if(work->mode != DAM_MODE){
		work->mode_buf = work->mode;
	}
	work->mode = DAM_MODE ;
	ForceChangeMot(work,dam_mot) ;
	work->life-- ;
}
static void SetPunchDamMode(Work	*work ,int dam_mot){
	if(work->mode != DAM_MODE){
		work->mode_buf = work->mode;
	}
	work->mode = DAM_MODE ;
	ForceChangeMot(work,dam_mot) ;
}
static void SetDeathMode(Work	*work ){
	work->mode = DEATH_MODE ;
	ForceChangeMot(work,work->death_mot) ;
	work->parent.class |= TARGET_SKIP ;
	if(work->death_proc != 0){
		GM_ExecProc( work->death_proc, NULL );
	}
}
static void Blood( Work *work,FVECTOR *cent ,FVECTOR *force ,int num)
{
	extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
	int mode = 0 ;
	FMATRIX	*world ;
	world = &(BODYWORLD( &work->body, num )) ;
	GV_SetActorChild( work, NewBlood( world, cent, force, mode, 0 ) ) ;
}

static	void	ChildTargCallBack_Parent( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			if (def->weapon_type & (WP_M92|WP_PSG1T)){ 
//				SetNeedle( &work->body, TargetPartsObjNum[def->name], &def->hit, &off->power->force ) ;
				SetNeedle( &work->body, def->name, &def->hit, &off->power->force ) ;
			}
			if( def->weapon_type & WP_BULLET){
//TargetPartsType[def->name] 
//				if(def->name == TRG_TYPE_HEAD){
				if(TargetPartsType[def->name] == TRG_TYPE_HEAD){
					work->life = 0;
					SetDeathMode( work ) ;
				}else {
//					SetBulletDamMode(work ,work->dam_mot[def->name]) ;
					SetBulletDamMode(work ,work->dam_mot[TargetPartsType[def->name]]) ;
//					Blood( work,&def->center,&off->power->force,DamObjNum[def->name]) ;
					Blood( work,&def->center,&off->power->force,def->name) ;
				}
			}
			if(def->weapon_type & (WP_PUNCHR|WP_PUNCHL|WP_BLADEFAINT|WP_KICK|WP_KICK1)){
				SetPunchDamMode(work ,work->dam_mot[TRG_TYPE_BODY]) ;
				if(def->weapon_type & (WP_KICK|WP_KICK1)){
					GM_SeSetMode( SD_P_KICK02, &work->control.mov, GM_SEMODE_NORMAL ) ;
				}else {
					GM_SeSetMode( SD_P_PUNCH02, &work->control.mov, GM_SEMODE_NORMAL ) ;
				}
			}

			if(! (def->weapon_type & (WP_M92|WP_PSG1T))){ 
				if(work->life <=0){
					SetDeathMode( work ) ;
					GM_SeSetMode( work->death_se, &work->control.mov, GM_SEMODE_BOMB ) ;
				}else {
					if(work->dam_se_num>0){
						GM_SeSetMode( work->dam_se[((irnd()>>8)% work->dam_se_num)],
							&work->control.mov, GM_SEMODE_BOMB ) ;
					}
				}
			}
		}
	}

	def->weapon_type = 0 ;
}
static void SetTargetSize(Work *work){
	int i,parts_num ,flag,map;
	FMATRIX mat ;
	FVECTOR	core_size ;
	FVECTOR	off_set ;
	DG_OBJ *obj ;
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON|TARGET_ROTATE|TARGET_POWER);
	map = work->control.map;
//bound
	for(i=0;i<TRG_PARTS_MAX;i++){
//		parts_num = TargetPartsName[i] ;
		parts_num = i ;

		obj = &work->body.objs->objs[parts_num] ;
//		setfvec(&off_set, 0.0f,	0.0f,	0.0f) ;
//		off_set = obj->trans ;
//		off_set.vx /= 2.0f; off_set.vy /= 2.0f; off_set.vz /= 2.0f;
		core_size.vx = (obj->bound_max.vx - obj->bound_min.vx)/2.0f ;
		core_size.vy = (obj->bound_max.vy - obj->bound_min.vy)/2.0f ;
		core_size.vz = (obj->bound_max.vz - obj->bound_min.vz)/2.0f ;

		off_set.vx = core_size.vx + obj->bound_min.vx ;
		off_set.vy = core_size.vy + obj->bound_min.vy ;
		off_set.vz = core_size.vz + obj->bound_min.vz ;

		GM_SetTarget( &work->parts[i], flag,map, BOTH_SIDE, &core_size , &off_set ) ;
		GM_SetTargetCallBack( &work->parts[i], ChildTargCallBack_Parent, work ) ;
		GM_SetTargetWeaponType(&work->parts[i],0);
//		work->parts[i].name = TargetPartsType[i] ;
		work->parts[i].name = i ;

	}
}

static void SetTarget(Work *work)
{
	int flag,map;
	int i;
	FVECTOR	b_size = { BODY_SIZE_X, BODY_SIZE_Y,BODY_SIZE_Z ,1.0f} ;
	FVECTOR	b_off_set = { 0.0f , 0.0f,150.0f,1.0f} ;
	FVECTOR	core_size01[TRG_PARTS_MAX] ;
	FVECTOR	off_set01[TRG_PARTS_MAX] ;


	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON|TARGET_ROTATE|TARGET_POWER);
	GM_CurrentMap =	map = work->control.map;

	GM_SetTarget( &work->parent, (flag|TARGET_CHILD_ALWAYS),map, BOTH_SIDE, &b_size, &b_off_set ) ;
	GM_SetTargetCallBack( &work->parent, ChildTargCallBack_Parent, work ) ;
	GM_SetTargetWeaponType( &work->parent, 0 ) ;
	GM_SetPowerTarget( &work->parent, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->parent );

	work->parent.name = TRG_TYPE_BODY ;

#if 1
	SetTargetSize(work) ;
	GM_SetTargetParts( &work->parent, &work->parts[0],TRG_PARTS_MAX, 0 ) ;
#else
	for(i=0;i<TRG_PARTS_MAX;i++){
		GM_SetTarget( &work->parts[i], flag|TARGET_CHILD_ALWAYS,map, BOTH_SIDE, &b_size, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( &work->parts[i], ChildTargCallBack_Parent, work ) ;
		GM_SetTargetWeaponType(&work->parts[i],0);
//		GM_SetPowerTarget(&work->parts[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
//		GM_PutTarget( &work->parts[i] );
//		GM_SetTargetParts( &work->parent, &work->parts[i],1, 0 ) ;
	}
#endif
#if 0
	{
		NewTargetView( &work->parent, 255, 0, 0 ) ;
		for(i=0;i<TRG_PARTS_MAX;i++){
			NewTargetView( &work->parts[i], 100, 0, 250 ) ;
		}
	}
#endif


}

static void MoveTarget(Work *work){
	int i,parts_num ;
	FMATRIX mat ;

	DG_COPY_MAT( &mat, &(work->body.objs->objs[HUMAN21_ONAKA].world) );
	GM_MoveTarget2( &work->parent, &mat ) ;
	for (i=0;i<TRG_PARTS_MAX; i++){
//		parts_num = TargetPartsName[i] ;
		parts_num = i ;
		DG_COPY_MAT( &mat, &(work->body.objs->objs[parts_num].world) );
		GM_MoveTarget2( &work->parts[i], &mat ) ;
	}
}


static void CheckPointProc(Work *work){
	int i ;
	if(work->proc_num <=0 )return ;
	for(i=0;i<work->proc_num;i++){
		if(work->proc_pos[i]== work->rnavi.next_node){
			GM_ExecProc( work->proc_id[i], NULL );
		}
	}
}
static void SetPointAction( ROUTENAVI *rnavi ){
	rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node];

	rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;

	if(rnavi->pa_time[(int)rnavi->next_node] > 0 ){
		rnavi->p_acttime = DIRECT_TICK(rnavi->pa_time[(int)rnavi->next_node]) ;
	}else {
		rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
	}

	rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node];
	rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node];
	rnavi->p_actstatus = rnavi->pa_flag[(int)rnavi->next_node];
}

static int TargetDisCheck(Work *work,float len){
	FVECTOR	vec ;
	float dis ;
	if( work->rnavi.p_actstatus & PA_CON_NO_CHECK_DIS  ) {
		return 1 ;
	}

	if(work->debug & DBG_PRINT_ALL){
		printf("TargetDisCheck\n");
		printf("ctrl mov x[%f] z[%f]\n",work->control.mov.vx,work->control.mov.vz);
		printf("trg  pos x[%f] z[%f]\n",work->trg.pos.vx,work->trg.pos.vz);
	}
	_sceVu0SubVector(  &vec, &work->trg.pos, &work->control.mov ) ;
	vec.vy = 0.0f ;
	dis = _FVecLen2( &vec ) ;
	if(dis < len){
		if(work->debug & DBG_PRINT_ALL) printf("TargetNear\n");
		return 1 ;
	}
		if(work->debug & DBG_PRINT_ALL) printf("TargetFar\n");
	return 0 ;
}

static	void	CheckMessage( work )
Work	*work ;
{
	FVECTOR		pos ;
    GV_MSG	*msg ;
	int n_msg, code ;
    n_msg = work->control.n_msg ;
	msg = work->control.msg ;

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
printf("ROUTE DEMO NINGYO id[%d] MSG[%d]RECIEVED!!\n",work->control.name,code);
		switch( code ) {
			case MSG_ROUTE_CHANGE :
				work->route = msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->route += GM_RouteOffset ; 
				}
				ENE_InitRouteNavi( &work->rnavi, work->route, 0 );
printf("INIT N_NODES %d\n",work->rnavi.n_nodes);

				SetPointAction( &work->rnavi );
printf("ROUTE CHANGE TO %d\n",work->route);
				work->trg.pos = work->rnavi.nodes[ 0 ] ;
				work->trg.map = work->rnavi.mapbit[ 0 ] ;
				work->trg.addr = HZX_GetAddress( work->trg.map,
					 &work->trg.pos, -1 ) ;
				if(TargetDisCheck(work,(float)SAME_POS_DIS)){
					/*到達*/
					CheckPointMode(work) ;
				}else {
					work->mode = MOVE_MODE ;
				}
				break ;
			case MSG_ROUTE_NODE :
printf("ROUTE NODE CHANGE TO %d\n",work->route);
				work->route = msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->route += GM_RouteOffset ; 
				}
				work->node = msg->message[ 2 ] ;
				ENE_InitRouteNavi( &work->rnavi, work->route, work->node );
				SetPointAction( &work->rnavi );
				work->trg.pos = work->rnavi.nodes[ work->node ] ;
				work->trg.map = work->rnavi.mapbit[ work->node ] ;
				work->trg.addr = HZX_GetAddress( work->trg.map,
					 &work->trg.pos, -1 ) ;
				if(TargetDisCheck(work,(float)SAME_POS_DIS)){
					/*到達*/
					CheckPointMode(work) ;
				}else {
					work->mode = MOVE_MODE ;
				}
				break ;
			case MSG_LOOP_END :
printf("MSG_LOOP_END RECIEVED!!!!\n");
				if(work->rnavi.p_acttime == PTIME_MESWAIT){
					/*ループ終了*/
					work->rnavi.p_acttime = 0;
				}
				break ;
			case MSG_SIRO_HATENA :
				HeadMarkRun(
					&work->body.objs->objs[HUMAN21_ATAMA].world ,5 ) ;
				break ;
			case MSG_FORCE_MOTION :
printf("MSG_FORCE_MOTION MOT [%d] DIR [%d]\n",msg->message[ 1 ],msg->message[ 2 ]);
				work->mode = FORCE_MODE ;
				ForceChangeMot(work,msg->message[ 1 ] ) ;
				work->control.turn.vy = msg->message[ 2 ] ;

				break ;
			case MSG_AKA_BIKKURI :
				HeadMarkRun(
					&work->body.objs->objs[HUMAN21_ATAMA].world ,0 ) ;
				break ;
			case MSG_ROUTE_NODE_WARP :
				work->route = msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->route += GM_RouteOffset ; 
				}
				work->node = msg->message[ 2 ] ;
				ENE_InitRouteNavi( &work->rnavi, work->route, work->node );
				SetPointAction( &work->rnavi );
				work->trg.pos = work->rnavi.nodes[ work->node ] ;
				work->trg.map = work->rnavi.mapbit[ work->node ] ;
				work->trg.addr = HZX_GetAddress( work->trg.map,
					 &work->trg.pos, -1 ) ;
				work->mode = MOVE_MODE ;

				pos = work->trg.pos ;
				pos.vy += 1000.0f ;
				GM_ResetControlPositionAndGroup( &work->control,
					&pos, work->trg.map ) ;
				break ;
			case MSG_MOVE_MOT_SET :
				work->move_mot = msg->message[ 1 ] ;
		}
		msg++ ;
	}
}



/*再生中のモーション番号*/
//work->body.m_ctrl->mt3_ctrl->motion_num ;


static void ChangeMot(Work *work,int mot){
	if( work->body.m_ctrl->mt3_ctrl->motion_num != mot){
//printf("NEW MOTION SET!!![%d]\n",mot);
		if (work->body.m_ctrl->mt3_ctrl[ 0 ].file_header->flag & MT3_FLAG_TURN_FLAG){
			if(work->debug & DBG_PRINT_ALL) printf("MT3_FLAG_TURN_FLAG\n");
			work->control.turn.vy = work->control.rot.vy = MatToYRot( &BODYWORLD( &work->body, HUMAN21_KOSHI) ) ;
		}
		GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,INTERP);
	}else {
//printf("SAME MOTION REQEST!!!!!!!!!!\n");
	}
}

static void ForceChangeMot(Work *work,int mot){
printf("Force Set Mot [%d]\n",mot);
	if (work->body.m_ctrl->mt3_ctrl[ 0 ].file_header->flag & MT3_FLAG_TURN_FLAG){
		if(work->debug & DBG_PRINT_ALL) printf("MT3_FLAG_TURN_FLAG\n");
		work->control.turn.vy = work->control.rot.vy = MatToYRot( &BODYWORLD( &work->body, HUMAN21_KOSHI) ) ;
	}
	GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,INTERP);
}

static void CheckPointMode(Work *work){
	CheckPointProc(work);
	if(work->rnavi.p_acttime == 0 ){
		/**引き続きMOVE_MODE**/
		if(work->debug & DBG_PRINT_ALL) printf("MOVE 2 MOVE MODE!!!\n");
		ENE_SetTrgpNextnode( &work->rnavi, &work->trg ) ;
		SetPointAction( &work->rnavi );
		work->mode = MOVE_MODE ;
	}else {
		if(work->debug & DBG_PRINT_ALL) printf("MOVE 2 ACT MODE!!!\n");
		work->mode = ACTION_MODE ;
	}
}
static void MoveRoute(Work *work){

	int dir;
	FVECTOR	vec ;
	float	speed , dis ;



	DG_COPY_VEC( &vec, &work->control.step ) ;
	vec.vy = 0.0F;
	vec.vw = 0.0F;
	speed = GV_VecLen3F( &vec ) ;

if(work->debug & DBG_PRINT_ALL) printf("MOVE SPEED [%f]!!\n",speed);

	if (( ENE_DirectTrace( &work->trg,&work->control.mov,SAME_POS_DIS) < 0 )
		||( work->rnavi.p_actstatus & PA_CON_NO_CHECK_DIS  ) ){
//printf("POINT NEAR!!\n");
if(work->debug & DBG_PRINT_ALL) printf("POINT NEAR!!\n");
		/*ポイントモード*/
		if ( work->rnavi.p_actstatus & PA_CON_PINPOINT  ) {
//printf("PIN POINT CHECK!!!\n");
if(work->debug & DBG_PRINT_ALL) printf("PIN POINT CHECK!!!\n");

			/*ピンポイントチェック*/
			_sceVu0SubVector(  &vec, &work->trg.pos, &work->control.mov ) ;
			vec.vy = 0.0f ;
			dir = _FVecDir2( &vec ) ;
			dis = _FVecLen2( &vec ) ;
if(work->debug & DBG_PRINT_ALL) printf("PIN POINT DIS [%f]!!\n",dis);
			work->trg.dir = dir ;
			work->trg.h_dis = (int)dis ;
			if(( dis < (speed*2.0f) )
			||( work->rnavi.p_actstatus & PA_CON_NO_CHECK_DIS  ) ){
//			if(( dis < (speed*2) )||(dis<100.0f)) {	
//printf("PIN POINT HIT!!!\n");
				/* 次のフレームで到着予定 */
				work->body.flag |= OBJECT_MOTIONSTEP_THROUGH ;
				work->control.step.vx = 0.0f ;
				work->control.step.vz = 0.0f ;
				work->control.mov.vx = work->trg.pos.vx ;
				work->control.mov.vz = work->trg.pos.vz ;
				ChangeMot(work,work->move_mot);
if(work->debug & DBG_PRINT_ALL) printf("PINPOINT HIT!!!!\n");
				CheckPointMode(work) ;
			}else {
				/* まだ到着しない */
if(work->debug & DBG_PRINT_ALL) printf("PINPOINT OUT!!! DIR[%d]\n",work->trg.dir);
				ChangeMot(work,work->move_mot);
				work->control.turn.vy = work->trg.dir ;
				work->control.rot.vy = work->trg.dir ;
				work->body.flag |= OBJECT_MOTIONSTEP_THROUGH ;
				work->control.step.vx = speed * _RsinF( dir ) ;
				work->control.step.vz = speed * _RcosF( dir ) ;
			}
		}else {
//printf("NO PIN POINT MODE REACH!!!\n");
			CheckPointMode(work) ;
		}
	}else {
		ChangeMot(work,work->move_mot);
		work->control.turn.vy = work->trg.dir ;
if(work->debug & DBG_PRINT_ALL) printf("NORMAL MOVE MODE DIR[%d]\n",work->trg.dir);

	}
}

static void DamSeq(Work *work)
{
	if( GM_GetObjectMotionEnd( &work->body, 0 ) ) {
		work->mode = work->mode_buf ;
	}
	work->control.step.vx = 0.0f;
	work->control.step.vz = 0.0f;

}
static void DeathSeq(Work *work)
{
	if( GM_GetObjectMotionEnd( &work->body, 0 ) ) {
		work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	}
}

static void ActionSeq(Work *work)
{
	ChangeMot(work,work->rnavi.p_action);
//printf("rnavi.p_acttime [%d]\n",work->rnavi.p_acttime) ;
	if(work->rnavi.p_dir >=0 ) {
if(work->debug & DBG_PRINT_ALL) printf("POINT ACT  DIR[%d]\n",work->rnavi.p_dir);
		work->control.turn.vy = work->rnavi.p_dir ;
	}
	if(work->rnavi.p_acttime>0){
		work->rnavi.p_acttime--;
	}else if(work->rnavi.p_acttime==0){
		ENE_SetTrgpNextnode( &work->rnavi, &work->trg ) ;
		SetPointAction( &work->rnavi );
//printf("COUNT MODE END\n");
//printf("ctrl mov x[%f] z[%f]\n",work->control.mov.vx,work->control.mov.vz);
//printf("trg  pos x[%f] z[%f]\n",work->trg.pos.vx,work->trg.pos.vz);
		if (( ENE_DirectTrace( &work->trg,&work->control.mov, SAME_POS_DIS) < 0 )
		||( work->rnavi.p_actstatus & PA_CON_NO_CHECK_DIS  ) ){
			CheckPointProc(work);
			work->mode = ACTION_MODE ;
//printf("CONT MODE END 2 ACTION MODE\n");
//			ForceChangeMot(work,work->rnavi.p_action);
			ChangeMot(work,work->rnavi.p_action);
		}else {
//printf("CONT MODE END 2 MOVE MODE\n");
			work->mode = MOVE_MODE ;
			ChangeMot(work,work->move_mot);
		}
	}else if(work->rnavi.p_acttime == PTIME_MESWAIT){
//printf("NOW WAIT\n");
		/*メッセージ受信までループモーションで待機*/
		if( GM_GetObjectMotionEnd( &work->body, 0 ) ) {
//		if( GM_CheckObject_PlayEnd( &work->body, 0 ) ) {
			/*モーション終了判定*/
//printf("IN WAIT MODE MOT END\n");
			ForceChangeMot(work,work->rnavi.p_action);
		}
	}else {
//printf("PLAY 2 END MODE\n");
		if( GM_GetObjectMotionEnd( &work->body, 0 ) ) {
//		if( GM_CheckObject_PlayEnd( &work->body, 0 ) ) {
			/*モーション終了判定*/
//printf("IN PLAY 2 END MODE\n");
			ENE_SetTrgpNextnode( &work->rnavi, &work->trg ) ;
			SetPointAction( &work->rnavi );
#if 0
			work->mode = MOVE_MODE ;
#else
	/*2001.06.09*/
//printf("ctrl mov x[%f] z[%f]\n",work->control.mov.vx,work->control.mov.vz);
//printf("trg  pos x[%f] z[%f]\n",work->trg.pos.vx,work->trg.pos.vz);
			if(( ENE_DirectTrace( &work->trg,&work->control.mov, SAME_POS_DIS) < 0 )
			||( work->rnavi.p_actstatus & PA_CON_NO_CHECK_DIS  ) ){
				CheckPointProc(work);
				work->mode = ACTION_MODE ;
//				printf("SAME POS 2 ACTION MODE\n");
				ForceChangeMot(work,work->rnavi.p_action);
			}else {
//			printf("GO MOVE MODE\n");
				work->mode = MOVE_MODE ;
				ChangeMot(work,work->move_mot);
			}
#endif
		}
	}
}
static void Act(Work *work)
{
	OBJECT *body ;
	CONTROL	*ctrl;

	MoveTarget(work) ;
	body = &work->body ;
	ctrl = &work->control ;

	if ( work->gage.value != work->life ) {
		work->gage.value = work->life ;
	}

	work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH ;

	switch(work->mode){
		case ACTION_MODE :
//printf("ACT MODE\n");
			ActionSeq(work);
			break;
		case MOVE_MODE :
//printf("MOVE MODE\n");
			MoveRoute(work);
			break;
		case FORCE_MODE :
			break;
		case DAM_MODE :
			DamSeq(work) ;
			break;
		case DEATH_MODE :
			DeathSeq(work) ;
			break;
	}
	GM_ActControl(ctrl) ;

//RouteView(&work->rnavi) ;
//printf("N_NODES %d\n",work->rnavi.n_nodes);

	DG_GetLightMatrix(&ctrl->mov, work->lights );
	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	GM_ActObject(body) ;

	/* 口パク*/

	if(work->status & MT_ANIM ){
		GM_MouthAnimation( work->talk_name, work->body.evmobj  ) ;
	}
	if(work->status & RADAR_ON){
		work->rctrl.dir = ctrl->rot.vy ;	//レーダ
	}
	/*重力処理*/
	if(ctrl->level_found){
		ctrl->step.vy = 0;
		ctrl->mov.vy 
		= ctrl->levels[0]  + work->body.m_ctrl->height ; 
		GM_ConfigControlHzxHeight( ctrl,
		500.0F, ctrl->levels[0] ) ;
	}else {
		ctrl->step.vy -= 16.0F;
	}
	/*あたり高さ設定*/
	ctrl->height = work->body.m_ctrl->height ;
	CheckMessage(work) ;
	/*debug*/
	{
// DBG_MOT_NUM		(0x01)	//再生中モーション番号表示
// DBG_ARROW		(0x02)	//内部の方向表示

		FMATRIX world ;
//void SIG_NumPrint(FVECTOR *pos ,int disp){
		
		if(work->debug &DBG_ARROW){
			DG_SetPos2( &ctrl->mov, &ctrl->turn ) ;
			DG_GetPos( &world ) ;
			HZX_ViewMatrix( &world, 1000.0f);
		}
#ifdef DEBUG_MODE
		if(work->debug &DBG_MOT_NUM){
extern void SIG_NumPrint(FVECTOR * ,int ) ;
			SIG_NumPrint(&ctrl->mov ,work->body.m_ctrl->mt3_ctrl->motion_num) ;
		}
		if(work->debug &DBG_ROUTE_VIEW){
extern void ALL_PatRouteView(HZX_PAT * ,int) ;
			HZX_HDL *hdl ;
			hdl = HZX_GetCurrentHzx();
			if(hdl->def->n_patrols >0) {
				GM_CurrentMap = GM_CurrentStageMap ;
				ALL_PatRouteView(&hdl->def->patrols[work->route],0) ;
			}
		}
#endif
	}
//	MoveTarget(work) ;
}
static void Die(Work *work)
{
	GM_RemoveGageSet( &work->gage ) ;
	GM_FreeTarget( &work->parent ) ;
	if(work->status & RADAR_ON){
		GM_FreeRadarControl(&work->rctrl);
	}
	HZX_FlashTrap( (work->control.hzx_id), &work->control.evt ) ;

	if(work->item.objs != NULL ){
		extern void DG_DisconnectObjs( DG_OBJS *, DG_OBJS * ) ;
		DG_DisconnectObjs( work->body.objs, work->item.objs ) ;
		GM_FreeObject(&(work->item));
	}
	GM_FreeObject(&(work->body));
	GM_FreeControl( &work->control);
}
static int GetResources(Work *work, int name, int where)
{

	int i,weapon,model,motion ,evmmodel,evm_sw,item,set_num = -1;
	int equip_name ,gage_name;

	work->name = name ;
	work->status = GCL_GetOptionValue( 's', 0 ) ;

	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->status |= MT_ANIM ;
		work->talk_name = GCL_GetNextInt() ;
	}

	GM_InitControl( &work->control, name, where );
//	GM_InitControl( &work->control, name, 0 );
	GM_ConfigControlMessageCheck( &work->control ) ;
	/*プレイヤと同じあたりサイズ*/
	GM_ConfigControlHazard( &work->control, 500, 496, 500 ) ;
	GM_ConfigControlMapCheck( &work->control ) ;
	GM_ConfigControlTrapCheck( &work->control ) ;

	work->control.interp = 4 ;


//	work->control.seg_flag |= HZX_SEG_NO_PLAYER ;
//	work->control.flr_flag |= HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ;
	work->control.flr_flag |= HZX_FLOOR_NO_PLAYER  ;
	/*CONTROLフラグ*/
//	work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
//	(CTRL_SKIP_TRAP|CTRL_SKIP_GET_ADDRESS);
//	(CTRL_SKIP_SEG_CHECK|CTRL_SKIP_TRAP|CTRL_SKIP_GET_ADDRESS
//	|CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK
//	|CTRL_HZX_SEG_HORIZON_CHECK); 

	/*シナリオリード*/

	/* モデル */
	if ( GCL_GetOption( 'k' ) == NULL ) {
		return 0 ;
	} else {
		model = GCL_GetNextInt() ;
	}

	/*EVMモデル*/
	if ( GCL_GetOption( 'e' ) == NULL ) {
		evmmodel = -1 ;
		evm_sw = 0;
	} else {
		evmmodel = GCL_GetNextInt() ;
		evm_sw = 1;
	}

	/* モーションファイル */
	if ( GCL_GetOption( 'm' ) == NULL ) {
		return 0 ;
	} else {
		motion = GCL_GetNextInt() ;
	}
	/* モーション番号 */
	work->move_mot = 1 ;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		work->move_mot = GCL_GetNextInt() ;
		work->death_mot= GCL_GetNextInt() ;
		/*ヘッドショットは死亡*/
		work->dam_mot[TRG_TYPE_HEAD]= work->death_mot ;
		if( GCL_NextStr() != NULL ) work->dam_mot[TRG_TYPE_BODY]= GCL_GetNextInt() ;
		if( GCL_NextStr() != NULL ) work->dam_mot[TRG_TYPE_HAND_R]= GCL_GetNextInt() ;
		if( GCL_NextStr() != NULL ) work->dam_mot[TRG_TYPE_HAND_L]= GCL_GetNextInt() ;
		if( GCL_NextStr() != NULL ) work->dam_mot[TRG_TYPE_LEG_R]= GCL_GetNextInt() ;
		if( GCL_NextStr() != NULL ) work->dam_mot[TRG_TYPE_LEG_L]= GCL_GetNextInt() ;
	}else {
		return 0 ;
	}

	/*ルート設定*/
	work->node = 0;
	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->route = GCL_GetNextInt() ;
		if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
			work->route += GM_RouteOffset ; 
		}
		if( GCL_NextStr() != NULL ){
			work->node = GCL_GetNextInt() ;
		}
		ENE_InitRouteNavi( &work->rnavi, work->route, work->node );
		SetPointAction( &work->rnavi );
	}else {
		return 0 ;
	}
	/*ルートポイントに配置*/
	work->trg.pos = work->control.mov = work->rnavi.nodes[ work->node ] ;
	/*初期方向をポイント指定*/
	work->control.turn.vy = work->control.rot.vy = work->rnavi.pa_dir[ work->node ] ;
	/* 装備品 */
	item = -1;
	if ( GCL_GetOption( 'i' ) != NULL ) {
		item = GCL_GetNextInt() ;
		set_num = GCL_GetNextInt() ;
    }
	/*debug*/
	if ( GCL_GetOption( 'd' ) != NULL ) {
		work->debug = GCL_GetNextInt() ;
    }else {
		work->debug = 0 ;
	}

	if(evm_sw){
		/*envモデル*/
		GM_InitObject(&(work->body),model,
			(OBJECT_FLAG|DG_FLAG_INVISIBLE));
		/*写りこみのためobjのフラグだけ立てる*/
		KR_UnsetAllObjsFlag( work->body.objs, DG_FLAG_INVISIBLE ) ;
		work->body.objs->flag |= DG_FLAG_INVISIBLE ;

		GM_ConfigObjectEvm( &(work->body),evmmodel, DG_EVMOBJ_IRREACTION );
	}else {
		GM_InitObject(&(work->body),model,OBJECT_FLAG );
	}
	if(item != -1 ){
		extern void DG_ConnectObjs( DG_OBJS *, DG_OBJS * ) ;

		GM_InitObject(&(work->item),item,ITEM_FLAG);
		GM_ConfigObjectRoot(&work->item,&work->body,set_num );
		GM_ConfigObjectLight(&(work->item),work->lights) ;
		DG_ConnectObjs( work->body.objs, work->item.objs ) ;
	}else {
		work->item.objs = NULL ;
	}

	/*ctrlからobjectを参照*/
	GM_ConfigControlObject( &work->control, &work->body ) ;
	GM_ConfigObjectStep( &work->body,&work->control.step ) ;
	GM_ConfigObjectMotion( &(work->body), 0,
		motion,MT_FLAG_HUMAN2);
/*2001.06.09*/
	/*初期モーションを最初のポイントアクションにする*/
	GM_ConfigObjectAction( &work->body, 0, work->rnavi.pa_action[ work->node ], 0,0xfffff,0);
	GM_ConfigObjectLight(&(work->body),work->lights) ;


	/*初期姿勢にする*/
	GM_ActMotion( &work->body ) ;

	GM_ConfigControlMapID( ( CONTROL * ) &work->control ) ;
	

	work->control.mov.vy += work->body.m_ctrl->height ; 
	
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( work->body.objs );
/*2001.06.09*/
	work->mode = ACTION_MODE ;
	GV_SetActorMessageKill( work, name ) ;
	/*レーダーに表示する*/
	/*とりあえず腰の向きとする*/
	/*将来 頭部方向参照にするかも*/
	if(work->status & RADAR_ON){
		GM_InitRadarControl(&work->rctrl,&work->control.mov,
			RADAR_VISIBLE|RADAR_SIGHT, work->control.map ) ;
		work->rctrl.angle = DEF_EYE_RANGE*2 ;/* 視野 */
		work->rctrl.col = RADAR_COLOR_BLUE ;	/* 視野描画色 */
		work->rctrl.range = 3000 ;		/* 視力 */
	    GM_RadarSetVRange( &work->rctrl, 3000 , -3000 );
	}
	
	work->proc_num = 0;
	if ( GCL_GetOption( 'p' ) != NULL ){
		for(work->proc_num = 0;work->proc_num <MAX_PROC;work->proc_num++){
			if( GCL_NextStr() != NULL ){
				work->proc_id[work->proc_num] = GCL_GetNextInt();
				work->proc_pos[work->proc_num] = GCL_GetNextInt();
			}else {
				break;
			}
		}
	}

	if ( GCL_GetOption( 'x' ) != NULL ){
		work->death_proc = GCL_GetNextInt();
	}else {
		work->death_proc = 0;
	}

	SetTarget(work) ;
	if ( GCL_GetOption( 'l' ) != NULL ){
		work->life = GCL_GetNextInt() ;
	}

	work->dam_se_num = 0 ;
	if ( GCL_GetOption( 'v' ) != NULL ){
		work->death_se = GCL_GetNextInt() ;	/*SD_V_GBSOUT01 ;*/
		for (i= 0 ;i<DAM_SE_MAX;i++){
			if( GCL_NextStr() != NULL ){
				work->dam_se[i] = GCL_GetNextInt() ;	/*SD_V_GBSDMG01;*/
				work->dam_se_num = i + 1 ;
			}
		}
	}


	/* ライフゲージ */
	if ( GCL_GetOption( 'g' ) != NULL ){
		gage_name = GCL_GetNextInt() ;
	}else {
		gage_name = 0 ;
	}
	switch (gage_name){
		case 0 :
			GM_InitGageSet( &work->gage, "MERYL", 16, 196, 3, work->life,work->life, 0, 30, NPC_GAGE_LEVEL ) ;
			break ;
		case 1 :
			GM_InitGageSet( &work->gage, "EMMA", 16, 196, 3, work->life,work->life, 0, 30, NPC_GAGE_LEVEL ) ;
			break ;
	}
//	work->gage.text_len = 23 ;	/* 見栄えで、えー感じに設定 */
	GM_SetGageColor( &work->gage, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 ) ;
	GM_AppendGageSet( &work->gage ) ;
	GM_VisibleGage( &work->gage ) ;


	return 1;
}

/* 初期化部メイン */
void *NewSigRouteModel2( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
#if 0
static void SetNeedle( OBJECT *body, int n_obj, FVECTOR *pos, FVECTOR *dir )
{
    extern void VertexSearch( FVECTOR *vans, FVECTOR *nans,
			      DG_OBJS *objs, int objnum, FVECTOR *target) ;
    extern void *NewAttachment4_called( int model_name, FVECTOR *v, SVECTOR *r,
					OBJECT *target, int objnum, FVECTOR *x,int angle_limit,
					int frames ) ;
    FVECTOR	vans, nans ;
    int model_name ;

	/* 麻酔弾 */
	model_name = GV_StrCode( "m92_bul2" ) ;
    VertexSearch( &vans, &nans, body->objs, n_obj, pos );
    NewAttachment4_called( model_name, dir, NULL, body, n_obj, &vans, 512, 4 );
}
{
	/* M９２だったら針をつける */
	if (off->weapon_type & (WP_M92|WP_PSG1T)){ 
		SetNeedle( &work->body, p->part_id, &def->hit, &off->power->force ) ;
	}
}
#endif
