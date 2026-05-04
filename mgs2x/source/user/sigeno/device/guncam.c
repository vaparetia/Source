//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	guncam.c
	ガンカメラ  *NewGunCamera
	2000/07/05 K.Sigeno
	$Id: guncam.c,v 1.1.1.3 2002/11/19 11:49:29 Yoshizawa1 Exp $
*/

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


#include "tmp_inc.h"


#include "korekado/enemy/enemy.x"
#include "eyecheck.h"


typedef	struct _Work {
	GV_ACT_EX	actor ;
	OBJECT		body ;
	CONTROL		ctrl ;
	FMATRIX		lights[2] ;
	FVECTOR		camera_pos;	/*カメラ座標*/
	SVECTOR		rot;	/*土台方向*/

        int		gun_count;

	int			map ;
	int			name ;
	short 		status ; /*シナリオ指定*/
	short		max_dir ;	/*可動角度*/
	short		now_dir ;	/*土台からの相対角度*/
	short		mode ;
	int			headmark ;
	EYEPARAM	eye ;
} Work ;

#if 1
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#else
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#endif

#define	TURN_SPEED (4)
#define	SVC_EYE_S_DEF	(4000)
#define	DIR_X_MAX		(512)
#define	DIR_Y_MAX		(1024+512)

#define		CAMERA_PARTS (1)


/*駆動制御*/
enum{
	RIGHT_TURN ,
	LEFT_TURN ,
	STOP_TURN
};


static void SetFaceDir(Work *work){
	work->eye.rot = work->ctrl.rot ;
}

#if 0
SVECTOR	rgb ;
FVECTOR	line[2] ;
extern void PosBox(FVECTOR *,float ,SVECTOR *) ;
extern void *NewLineView(FVECTOR * ,int,u_char,u_char,u_char) ;

		line[0] = work->camera_pos ;
		line[1] = *work->eye.trgpos ;
		
		rgb.vx = 255; rgb.vy = 0; rgb.vz = 0;
		PosBox(&work->camera_pos,200.0F ,&rgb) ;
		NewLineView( &line[0] ,1,0,0,255) ;
#endif


/*通常首ふり角度*/
static void SetDir(Work *work){
	work->now_dir &= 4095 ;
//printf("NOW_DIR %d\n",work->now_dir);
	switch(work->mode){
		case RIGHT_TURN :
			if(
			(work->now_dir > work->max_dir)
			&&(work->now_dir < 2048)
			){
				work->mode = LEFT_TURN ;
			} else {
				work->now_dir += TURN_SPEED ;
			}
			break;
		case LEFT_TURN :
			if(
			(work->now_dir < (4096-work->max_dir) )
			&&(work->now_dir >= 2048 ) 
			){
				work->mode = RIGHT_TURN ;
			}else {
				work->now_dir -= TURN_SPEED ;
			}
			break;
	}
	work->ctrl.turn.vx = work->rot.vx ;
	work->ctrl.turn.vy = work->rot.vy+work->now_dir ;
	work->ctrl.turn.vz = 0 ;
}
static void SetObjs(Work *work){
	SVECTOR base_rot;

	base_rot = work->rot;
	base_rot.vx = 0 ;

	GM_GroupObjs( work->body.objs, work->map );
	/*柱*/
	DG_SetPos2( &work->ctrl.mov, &base_rot ) ;
	DG_PutObjs( work->body.objs );
	DG_GetPos( &work->body.objs->objs[0].world  ) ;
	/*頭*/
	DG_SetPos2( &work->camera_pos, &work->ctrl.rot ) ;
	DG_GetPos( &work->body.objs->objs[1].world  ) ;
}
static void Act(Work *work)
{
	OBJECT *body ;

	body = &work->body ;
#ifdef DEBUG_MODE
	DEV_DebugMode(&work->eye);
#endif

	SetFaceDir(work);
	/*プレイヤをチェック*/
	work->eye.trgpos = &GM_PlayerFindPos ;
	DEV_CameraSe(&work->eye) ;
	if(DEV_EyeInfoCheck( &work->eye ,work->ctrl.hzx_id ) ==0){
		/*プレイヤ未見かつニキータ中ならニキータを調べる*/
		if(GM_NikitaAlive[0] == NKT_NORMAL){
			work->eye.trgpos = &GM_NikitaPosition[ 0 ] ;
			DEV_EyeInfoCheck( &work->eye ,work->ctrl.hzx_id ) ;
		}
	}
	/*発見プロセス*/
	DEV_HeadMarkCheck(&work->eye,
		&work->body.objs->objs[CAMERA_PARTS].world,&work->headmark) ;
/*ガンカメは通報の代わりに発砲*/
	if((work->eye.discv_time <= HEADMARK_SET )
	&&(work->eye.alert_time == SIGHT_ALERT)){
	    if( work->gun_count >= 100 ){ 
		
		if ((work->gun_count/100)%4==0){	
		    DEV_Bullet(&work->eye,
			       &work->body.objs->objs[CAMERA_PARTS].world);
		}
		work->gun_count -= 100;
	    }

            if (work->gun_count < 100){
		if (work->gun_count == 0){
		    work->gun_count = (6+(irnd()&10))*400;
		    work->gun_count += 10+(irnd()&25);
		}
		work->gun_count--;
	    }
	}
	if(work->eye.discv_time > 0 ) work->eye.discv_time --;
	if(work->eye.alert_time > 0 ){
		/*目標物へ向く*/
		SIG_VecDir(work->eye.eyepos,work->eye.trgpos,
		&work->ctrl.turn);
	}else {
		/*巡回に復帰*/
		if(work->mode == STOP_TURN){
		/*探査モード用変数の初期化*/
			work->now_dir = (work->ctrl.turn.vy - work->rot.vy)&4095  ;
			if(work->now_dir < 2048 ) {
				work->mode = LEFT_TURN ;
			}else {
				work->mode = RIGHT_TURN ;
			}
		}
		/*探査中*/
		SetDir(work);
	}
	/*限界設定*/
	DEV_DirLimitX(&work->ctrl.turn,DIR_X_MAX,128 ) ;
	GM_ActControl(&work->ctrl) ;

	DG_GetLightMatrix( &work->ctrl.mov, work->lights );
	SetObjs(work);
}
static void Die(Work *work)
{
	GM_FreeObject(&(work->body));
	GM_FreeControl( &work->ctrl);
}
static int GetResources(Work *work, int name, int where)
{
    int buf[3],model ;
	FVECTOR		*trans,tmppos ;
	FMATRIX		tmpmat;
	SVECTOR		base_rot,range;
	float		length ;

	work->name = name ;
	/*ステータス*/
	work->status = GCL_GetOptionValue( 's', 0 ) ;
	/*シナリオリード*/
	//GCL_GetOptionValue( 'd', 0 ) ;
	/* 座標 */
	if ( GCL_GetOption( 'p' ) ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &tmppos ) ;
	}else {
		tmppos = DG_ZeroVector;
	}
	/* 方向 */
	if ( GCL_GetOption( 'r' ) ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		work->rot.vx =  buf[0];
		work->rot.vy =  buf[1];
		work->rot.vz = 0;
	}else {
		work->rot = DG_ZeroSVector;
	}
	/*可動幅*/
	work->max_dir = GCL_GetOptionValue( 'd', 512 ) ;
	work->now_dir = 0 ;

	/*探査目標*/
	work->eye.trgpos = &GM_PlayerFindPos ;
	/*顔の位置*/
	work->eye.eyepos = &work->camera_pos ;
	/*顔の向き*/
	work->ctrl.rot = work->ctrl.turn = 
		work->eye.rot  = work->rot ;
	/*視野角*/
	range.vx = GCL_GetOptionValue( 'x', 512 ) ; 
	range.vy = GCL_GetOptionValue( 'y', 512 ) ;
	range.vz = 0;
	/*視力*/
	length = GCL_GetOptionValue( 'i', SVC_EYE_S_DEF  );

	DEV_InitEyeParam(&work->eye,&range,length);

	work->mode = RIGHT_TURN ;
	model = GV_StrCode("gcm") ;

	GM_InitObject(&(work->body),model,OBJECT_FLAG );
	GM_ConfigObjectLight(&(work->body),work->lights) ;

	GM_InitControl( &work->ctrl, name, 0 );
	GM_ConfigControlMessageCheck( &work->ctrl ) ;
	GM_ConfigControlMapCheck( &work->ctrl ) ;
	work->ctrl.interp = 16 ;
	/*CONTROLフラグ*/
	work->ctrl.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK
	|CTRL_SKIP_GET_ADDRESS
	|CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK
	|CTRL_SKIP_TRAP
	); 
	work->ctrl.mov = tmppos ;
	/*カメラ部の原点座標を保持*/
	base_rot = DG_ZeroSVector ;
	base_rot.vy = work->rot.vy ;

	DG_SetPos2( &tmppos, &base_rot ) ;
	trans = &work->body.objs->objs[CAMERA_PARTS].trans ;
	DG_MovePos(trans );
	DG_GetPos( &tmpmat ) ;
	GV_MatToVec(&tmpmat,&work->camera_pos);

	work->ctrl.map = where ;

	work->map = work->ctrl.map ;
	GM_ActControl(&work->ctrl) ;
	GM_GroupObjs( work->body.objs, work->ctrl.map ) ;
	DG_SetPos2( &work->ctrl.mov, &work->ctrl.turn ) ;
	DG_PutObjs( work->body.objs );
	return 1;
}

/* 初期化部メイン */
void *NewGunCamera( name , where )
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








