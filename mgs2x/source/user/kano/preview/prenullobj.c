/*

	prenullobj.c
		オブジェを、モーションを付けてただ表示するだけ

	2000/2/1 K.Kano
	$Id: prenullobj.c,v 1.1.1.3 2002/11/19 11:43:30 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"gameheader.h"
#include	"libutl.h"


typedef struct {

    GV_ACT	actor;		/* リンク */

    CONTROL	ctrl;
    OBJECT	body;

    FMATRIX	lights[2];

    FVECTOR	pos;
    SVECTOR	rot;

} Work;


/* ------------------------------------------------------------------------ */

static void Act(Work *work)
{
    float	height ;

    height = work->body.height ;
    GM_ActMotion(&(work->body));
    work->ctrl.step.vy = work->body.height - height ;
    work->ctrl.height=work->body.height;
    //work->ctrl.hzx_height=work->ctrl.height;
    //work->ctrl.hzx_base=work->ctrl.mov.vy-work->ctrl.height;

//    GM_ConfigControlMapID(&(work->ctrl));
    GM_ActControl(&(work->ctrl));
    GM_ActObject2(&(work->body));
    DG_GetLightMatrix((FVECTOR *)&(work->ctrl.mov),work->lights);

#if 1
    if ( work->ctrl.grounded & 1 ) work->ctrl.step.vy = 0.0F ;
    work->ctrl.step.vy+=P_GRAVITY;
#endif
}

static void Die(Work *work)
{
    GM_FreeObject(&(work->body));
    GM_FreeControl(&(work->ctrl));
}

/* ------------------------------------------------------------------------ */
/* 初期化部 */

/* 資源を獲得 */
static int GetResources(Work *work,int name)
{
    int model=GV_StrCode("snake");
    int motion=GV_StrCode("motion");
    int action=0;
    int evm_model=0;

    work->pos=DG_ZeroVector;
    work->rot=DG_ZeroSVector;

    if(GCL_GetOption( 'x' )!=NULL){
	int buf[ 3 ];

	GCL_GetIV(GCL_NextStr(),buf);
	vu0_IV0toFV((IVECTOR *)buf,&(work->pos));
    } 
    if( GCL_GetOption( 'r' ) != NULL ){
	int buf[ 3 ];

	GCL_GetIV(GCL_NextStr(),buf);
	work->rot.vx=buf[0];
	work->rot.vy=buf[1];
	work->rot.vz=buf[2];
    }
    if(GCL_GetOption('m')!=NULL){
	model=GCL_GetNextInt();
    }
    if(GCL_GetOption('e')!=NULL){
	evm_model=GCL_GetNextInt();
    }
    if(GCL_GetOption('a')!=NULL){
	motion=GCL_GetNextInt();
    }
    if(GCL_GetOption('n')!=NULL){
	action=GCL_GetNextInt();
    }

    GM_InitControl(&(work->ctrl),name,0);
    work->ctrl.height = 1049.0F ;


    GM_InitObject(&(work->body),model,
		  DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC);
    if(evm_model){
	DG_InvisibleObjs(work->body.objs);
	GM_ConfigObjectEvm(&(work->body),evm_model,0);
    }

    GM_ConfigObjectLight(&(work->body),work->lights);

    GM_ConfigObjectMotion(&(work->body),1,motion,MT_FLAG_HUMAN1);
    GM_ConfigObjectAction(&(work->body),0,action,0,0xfffff,0) ;

    GM_ConfigObjectStep(&(work->body),&(work->ctrl.step));
    GM_ConfigControlPosition(&(work->ctrl),&(work->pos),&(work->rot));

    GM_ConfigControlAddressCheck( &(work->ctrl) ) ;
    GM_ConfigControlMapCheck( &(work->ctrl) ) ;

    work->ctrl.step=DG_ZeroVector;

    /* 当たりは見ません */
    work->ctrl.skip_flag |= CTRL_SKIP_HZX ;
    return 1;
}

static void NullAct(Work *work)
{
    printf("Abort Null Object\n");
    GV_DestroyActor(work);
}

static void NullDie(Work *work)
{
}

/* 初期化部メイン */
void *NewNullObj(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
	if(!GetResources(work,name)) GV_SetActor(&(work->actor),NullAct,NullDie);
	else GV_SetActor(&(work->actor),Act,Die);
    }
    return (void *)work ;
}
