//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	lockercp.c
	ロッカーから倒れる死体 *NewLockerCP
	2000/03/29 K.Sigeno
	$Id: lockercp.c,v 1.1.1.3 2002/11/19 11:49:20 Yoshizawa1 Exp $
*/

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
#include "../../korekado/conv/corps.h"

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	OBJECT		body ;
	FMATRIX		lights[2] ;
	float		y_pos;
//	CONTROL		control;
	CONTROL_NOEVT	control;
	float		w_step ;
	int			name ;
	int			model_code ;
	int			mot_flag ;
	int			light_sw;
	int			test ;
} Work ;

enum {
	MSG_DOWN_START,MSG_VISIBLE,MSG_INVISIBLE
};

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)
//#define	OBJECT_FLAG (DG_FLAG_SHADE | DG_FLAG_FINISHCALC | DG_FLAG_SHADOWMAKE)

#define		WAIST_HEIGHT	(1130)
#define		FLOOR_HEIGHT	(200)

static void MotSleep(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
}
static void MotWakeUp(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
}

static	void	CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code ;

    n_msg = work->control.n_msg ;
	msg = work->control.msg ;
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case MSG_DOWN_START :
				if(work->mot_flag == 0) {
//					printf("DEF ON RECIEVE\n");
					if(work->mot_flag == 0 ){
						work->mot_flag = 1;
						GM_ConfigControlHazard( (CONTROL *) &work->control,
						180,100,90 ) ;
					}
				}
			break;
			case MSG_VISIBLE :
				DG_VisibleObjs( work->body.objs ) ;
			break;
			case MSG_INVISIBLE :
				DG_InvisibleObjs( work->body.objs ) ;
			break;
		}
		msg++ ;
	}
}

#if 0
static void testprint(Work *work)
{
//	if (work->test > 6) return ;
	if (0) return ;
	else {
		printf("g_id %x line %d\n",
			work->body.objs->group_id , (work->test+1) );
		printf(" act map_name %x\n",work->body.map_name);
printf("CURRENT MAP %x\n",GM_CurrentMap );

work->test++;
	}
}
#endif

static void Act(Work *work)
{
	OBJECT *body ;
	body = &work->body ;


	if (work->mot_flag == 0){
		MotSleep(work);
	}
	if (work->mot_flag == 1){
		MotWakeUp(work);
	}
	if (work->mot_flag == 0){
		work->control.step.vx = 0.0F; 
		work->control.step.vz = 0.0F; 
	}
	GM_ActControl( (CONTROL *) &work->control) ;

#if 0
	work->control.mov.vy
	= work->y_pos + work->body.m_ctrl->height ; 
#else

	work->control.height = work->body.height ;
	work->control.step.vy = work->control.step.vw ;
#endif


	DG_GetLightMatrix( &work->control.mov, work->lights );
	GM_ActObject(body) ;
//    MT_SetMotionSeTable( work->body.m_ctrl, GM_CurrentMap, 1,  (work->control.flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 ) ;
    MT_SetMotionSeTable( work->body.m_ctrl, GM_CurrentMap, 1,  (0xffff) ,(0xffff) ) ;

	if( GM_CheckObject_PlayEnd( body, 0 ) ) {
		if (work->mot_flag != 2) {
			extern	void *NewCorp( int, OBJECT *, FVECTOR *, SVECTOR *, int, int,
			 void *, void *, CORPS_CALLBACK, CORPS_CALLBACK ,int ) ;
			CONTROL_NOEVT	*ctrl ;
			ctrl = &work->control ;
			NewCorp( ctrl->name, body, &ctrl->mov, &ctrl->rot, work->model_code, 0,
			  NULL, NULL, NULL, NULL ,(0x0140) ) ;
		}
		GV_DestroyActor(work) ;
	}
	CheckMessage(work);
}
static void Die(Work *work)
{
    GM_FreeObject(&(work->body));
	GM_FreeControl( (CONTROL *)&work->control);
}
#define LOCKER_FL_SHIFT (200.0F)
static int GetResources(Work *work, int name, int where)
{

    int		buf[3] , motion , mt_num ;
    extern int	GM_GetDGGroupID( int ) ;

	work->name = name ;



	GM_InitControl( (CONTROL *)&work->control, name, where );
//	GM_ConfigControlHazard( (CONTROL *)&work->control,WAIST_HEIGHT,50,40 ) ;
	GM_ConfigControlMessageCheck( (CONTROL *)&work->control ) ;
	GM_ConfigControlMapCheck( (CONTROL *)&work->control ) ;
	/*CONTROLフラグ*/
	work->control.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK|CTRL_SKIP_TRAP|CTRL_SKIP_GET_ADDRESS
	|CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK) ;
	/*シナリオリード*/
	/*方向*/
	work->control.turn.vy = GCL_GetOptionValue( 'd', 0 ) ;
	work->control.rot.vx = 0 ;
	work->control.rot.vy = work->control.turn.vy ;
	work->control.rot.vz = 0 ;

	/* 座標 */
	if ( GCL_GetOption( 'p' ) != NULL){
		GCL_GetNextIV( buf ) ;
		work->control.mov.vx = (float) buf[0] ;
		work->control.mov.vy = (float) buf[1] + LOCKER_FL_SHIFT ;
		work->control.mov.vz = (float) buf[2] ;
		work->y_pos = (float) buf[1] + LOCKER_FL_SHIFT ;
//		work->control.mov.vy += (WAIST_HEIGHT+FLOOR_HEIGHT) ;
//		work->control.mov.vy += FLOOR_HEIGHT ;
	}else {
		work->control.mov = DG_ZeroVector;
		work->y_pos = 0.0F ;
	}
	/* モデル */
	if ( GCL_GetOption( 'k' ) == NULL ) {
		work->model_code = GV_StrCode("tnc_def") ;
	} else {
		work->model_code = GCL_GetNextInt() ;
	}
	/* モーションファイル */
	if ( GCL_GetOption( 'm' ) == NULL ) {
		motion = GV_StrCode("locker_cp") ;
	} else {
		motion = GCL_GetNextInt() ;
	}
	/* モーション番号 */
	if ( GCL_GetOption( 'n' ) == NULL ) {
		mt_num = 0 ;
    } else {
		mt_num = GCL_GetNextInt() ;
    }
//	printf("g_id == %x\n",body->objs->group_id );

	GM_InitObject(&(work->body),work->model_code,OBJECT_FLAG);
	GM_ConfigControlObject( (CONTROL *)&work->control, &work->body ) ;
	GM_ConfigObjectStep( &work->body,&work->control.step ) ;
	GM_ConfigObjectMotion( &(work->body), 0,
		motion,MT_FLAG_HUMAN2);
	GM_ConfigObjectAction( &work->body, 0, mt_num, 0,0xfffff,1);
	GM_ConfigObjectLight(&(work->body),work->lights) ;



	/*初期姿勢にする*/
#if 1
//	GM_ActObject(&work->body) ;
	GM_ActMotion(&work->body) ;
	GM_ActMotion(&work->body) ;

//printf("GET RES LOCKER CP [%f]\n",work->control.mov.vy);
//printf("work->body.m_ctrl->height [%f]\n",work->body.m_ctrl->height);
//printf("work->body.height [%f]\n",work->body.height);
//printf("work->control.step.vw [%f]\n",work->control.step.vw );
//printf("work->control.step.vy [%f]\n",work->control.step.vy );

	work->control.step.vx = 0.0F; 
	work->control.step.vz = 0.0F; 
	work->control.mov.vy
	= work->y_pos + work->body.m_ctrl->height ; 
	/*movからmapを判断*/
	GM_ConfigControlMapID( ( CONTROL * ) &work->control ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( work->body.objs );
#endif
	work->mot_flag = 0;
//("g_id == %x\n",work->body.objs->group_id );
work->test = 0;
#if 0
printf("where is %x\n",where);
printf("CURRENT MAP %x\n",GM_CurrentMap );
printf("WHERE GET MAP ID %x\n",GM_GetDGGroupID( where ) ) ;
printf("CURRENT GET MAP ID %x\n",GM_GetDGGroupID( GM_CurrentMap ) ) ;
printf("map_name %x\n",work->body.map_name);
printf("ctrl map %x\n",work->control.map);

#endif

	return 1;
}

/* 初期化部メイン */
void *NewLockerCP( name , where )
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
