//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	facedemo.c
	デモシーン用 めぱち
	
	2000/04/20 K.Sigeno
	$Id: facedemo.c,v 1.1.1.3 2002/11/19 11:49:12 Yoshizawa1 Exp $
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

//#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_ONEPIECE)

enum {
	NORMAL,
	BIKKURI,
	CLOSE,
	DAMAGE,
	SIROME,
} ;
#define ANIM_NUM	5	/* パターン種類数 */

/* アニメリスト パターン番号 表示時間の順で羅列 */
typedef	struct	{
//	GV_ACT		actor ;
	GV_ACT_EX	actor ;

	OBJECT		*body ; /*親 人体モデル*/
	OBJECT		parts[ANIM_NUM] ; 
	int			name ;
	int			model_code[ANIM_NUM];
	int			model;	/*現在の表示番号*/
	int			count;	/*時間*/
} Work ;

#define PRIO	0x40	/* 敵兵より後に処理 */
//static FVECTOR	FACE_SHIFT = { 0.2F, 0.0F, 1.5F } ;
static FVECTOR	FACE_SHIFT = { 0.0F, 0.0F, 0.1F } ;


static void MSG_Check(Work *work){
	GV_MSG *msg;
	int n_msg,code;
	float	time;
	n_msg = GV_ReceiveMessage( work->name, &msg );
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case 1 :
				work->model = msg->message[ 1 ] ;
				if(msg->message[ 2 ] >= 0){
					time = (float) msg->message[ 2 ] ;
					/*ミリ秒換算*/

               if ( BP_IsPAL()==FALSE )
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

static void ModelChange(Work *work){
	int i;
	for(i=0;i<ANIM_NUM;i++){
		if(i==(work->model)){
			DG_VisibleObjs( work->parts[i].objs );
		}else {
			DG_InvisibleObjs( work->parts[i].objs );
		}
	}
}
static	void	Act( work )
Work		*work ;
{
	int	i ;

	MSG_Check(work);
	/*－１で終了*/
	if(work->model == -1){
		GV_DestroyActor( work ) ;
		return ;
	}

	if(work->count == 0) {
		work->model = 0;
//printf("DEFAULT EYE SET\n");
	}else {
	}
	ModelChange(work);

	DG_SetPos( &work->body->objs->objs[HUMAN21_ATAMA].world ) ;
	DG_MovePos( &FACE_SHIFT ) ;
#if 1
	for(i=0;i<ANIM_NUM;i++){
		/*親のマップをセット*/
		work->parts[i].map_name = work->body->map_name ; 
		GM_GroupObjs( work->parts[i].objs,work->body->map_name ) ;
	}
#endif
	DG_PutObjs( work->parts[work->model].objs ) ;
	if(work->count>0) work->count--;
}

static	void	Die( work )
Work		*work ;
{
	int i;
	for(i=0;i<ANIM_NUM;i++){
		GM_FreeObject( &work->parts[i] );
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( work, body ,name)
Work	*work ;
OBJECT	*body ;	/* ボディーオブジェクト */
int		name;
{
	int i;
	/*胴体オブジェクト*/
	work->body = body ;
	work->name = name ;
	/*モデルID*/

	/*通常*/
	work->model_code[NORMAL] = GV_StrCode( "gbs_eye0" ) ;
	/*大開き*/
	work->model_code[BIKKURI] = GV_StrCode( "gbs_eye1" ) ;
	/*閉じ*/
	work->model_code[CLOSE] = GV_StrCode( "gbs_eye2" ) ;
	/*ダメージ*/
	work->model_code[DAMAGE] = GV_StrCode( "gbs_eye3" ) ;
	/*白目*/
	work->model_code[SIROME] = GV_StrCode( "gbs_eye4" ) ;
	

	work->model = 0;
	work->count =0;

	for(i=0;i<ANIM_NUM;i++){
		GM_InitObject( &work->parts[i], work->model_code[i], BODY_FLAG );
		DG_SetPos( &work->body->objs->objs[HUMAN21_ATAMA].world ) ;
		DG_PutObjs( work->parts[i].objs ) ;
		DG_SetLightMatrix( work->parts[i].objs, work->body->objs->light );
		DG_InvisibleObjs( work->parts[i].objs );
	}
	DG_VisibleObjs( work->parts[0].objs );
	return 0 ;
}
void	*NewGbsFaceDemo( body ,name)
OBJECT	*body ;	/* ボディーオブジェクト */
int		name ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body ,name) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
