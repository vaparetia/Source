//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_eye.c
	デモシーン用 汎用目パチ
	2001/04/09 K.Sigeno
	$Id: demo_eye.c,v 1.1.1.3 2002/11/19 11:49:11 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"libutl.h"

#include "BP_Misc.h"

#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_ONEPIECE)
enum {
	NORMAL,
	CLOSE,
	PATTERN_MAX
} ;
#define ANIM_NUM	PATTERN_MAX	/* パターン種類数 */

typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		*body ; /*親 人体モデル*/
	DG_OBJS		*objs;
    DG_DEF		*parts[ANIM_NUM] ;	/*切り替え用モデルリスト*/
	int			name ;
	int			model;	/*現在の表示番号*/
	int			count;	/*時間*/
} Work ;

static	FVECTOR	eye_shift = { 0.0F, 0.0F, 1.0F ,0.0F} ;

#define PRIO	0x40	

static void MSG_Check(Work *work){
	GV_MSG *msg;
	int n_msg,code;
	float	time;
	n_msg = GV_ReceiveMessage( work->name, &msg );
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case 1 :
				if(msg->message[ 1 ] >= 0){
					time = (float) msg->message[ 1 ] ;
					/*ミリ秒換算*/

               if ( BP_IsPAL()!=TRUE )
					   time *= (6.0F/100.0F);
               else
					   time *= (5.0F/100.0F);

               work->count = (int) time ;
				}else {
					work->count = -1 ;
				}
			break ;
		}
		msg++ ;
	}
}

#if 0
static void ModelChange(Work *work){
	if((work->count>0)&&(work->model == 0)){
		/**/
//	    DG_ChangeModelObj( &work->body.objs->objs[1], &work->broken->models[1] ) ;
	    DG_ChangeModelObj( &work->objs->objs[0],&work->parts[1]->models[0] ) ;
		work->model = 1 ;
	}else if((work->count==0)&&(work->model == 1)){
	    DG_ChangeModelObj( &work->objs->objs[0],&work->parts[0]->models[0] ) ;
		work->model = 0 ;
	}
}
#endif
static	void	Act( work )
Work		*work ;
{
	MSG_Check(work);
	/*－１で終了*/
	if(work->count == -1){
		GV_DestroyActor( work ) ;
		return ;
	}

//	ModelChange(work);

#if 1
	DG_SetPos( &work->body->objs->objs[HUMAN21_ATAMA].world ) ;
//	DG_SetPos( &work->objs->world );
	DG_MovePos( &eye_shift ) ;
	DG_GetPos( &work->objs->world );
#endif

#if 1
	/*親のマップをセット*/
	GM_GroupObjs( work->objs, work->body->map_name ) ;
#endif
	if(work->count>0) work->count--;
}

static	void	Die( work )
Work		*work ;
{
	if ( work->objs != NULL ){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( work, body ,name,code0,code1) 
Work	*work ;
OBJECT	*body ;	/* ボディーオブジェクト */
int		name;
int		code0 ;
int		code1 ;
{
	int i,model_code[PATTERN_MAX];

	/*胴体オブジェクト*/
	work->body = body ;
	work->name = name ;
	/*モデルID*/
	model_code[0] = code0 ;
	model_code[1] = code1 ;

	work->model = 0;
	work->count =0;

	for(i=0;i<PATTERN_MAX;i++){
		work->parts[i] = (DG_DEF*) GV_GetCache( GV_CacheID( model_code[i], 'k' ) ) ;
		ASSERT(work->parts[i] != NULL) ;
	}
	work->objs = DG_MakeObjs( work->parts[0], BODY_FLAG, DG_CHANL_MAIN ) ;
	DG_QueueObjs( work->objs ) ;
	DG_SetLightMatrix( work->objs, work->body->objs->light );

	DG_SetPos( &work->body->objs->objs[HUMAN21_ATAMA].world ) ;
	DG_MovePos( &eye_shift ) ;
	DG_GetPos( &work->objs->world );
	return 0 ;
}
void	*NewEyeAnimDemo( body ,name , code0 ,code1)
OBJECT	*body ;	/* ボディーオブジェクト */
int		name ;
int code0	;
int	code1	;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body ,name,code0,code1) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
/*****
data
	human/sel_def/
0(default)	sel_eyelittle0.kms
		sel_eyelittle1.kms
1		sel_eye_a0.kms
		sel_eye_a1.kms
2		sel_eye_b0.kms
		sel_eye_b1.kms
3		sel_eye_c0.kms
		sel_eye_c1.kms
4		sel_eye_d0.kms
		sel_eye_d1.kms
5		sel_eye_e0.kms
		sel_eye_e1.kms
******/
void *NewEyeAnimSEALS_Demo(OBJECT *body,int name,int type){
	int code0=0,code1=0;
	switch(type){
		case 0 :
			code0 = GV_StrCode( "sel_eyelittle0" ) ;
			code1 = GV_StrCode( "sel_eyelittle1" ) ;
			break ;
		case 1 :
			code0 = GV_StrCode( "sel_eye_a0" ) ;
			code1 = GV_StrCode( "sel_eye_a1" ) ;
			break ;
		case 2 :
			code0 = GV_StrCode( "sel_eye_b0" ) ;
			code1 = GV_StrCode( "sel_eye_b1" ) ;
			break ;
		case 3 :
			code0 = GV_StrCode( "sel_eye_c0" ) ;
			code1 = GV_StrCode( "sel_eye_c1" ) ;
			break ;
		case 4 :
			code0 = GV_StrCode( "sel_eye_d0" ) ;
			code1 = GV_StrCode( "sel_eye_d1" ) ;
			break ;
		case 5 :
			code0 = GV_StrCode( "sel_eye_e0" ) ;
			code1 = GV_StrCode( "sel_eye_e1" ) ;
			break ;
		default :
			printf("EYE ANIM TYPE CODE ERROR !! [%d]\n",type);
			ASSERT(0);
	}
	return (NewEyeAnimDemo( body ,name , code0 ,code1)) ;
}
