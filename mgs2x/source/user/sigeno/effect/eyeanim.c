//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	eyeanim.c
	ゲーム中めぱち
	
	1999/11/04 K.Sigeno
	$Id: eyeanim.c,v 1.1.1.3 2002/11/19 11:49:46 Yoshizawa1 Exp $
*/
/******
現在の表情リスト
	0:NORMAL,	通常
	1:MABATAKI,	瞬き（2秒+乱数間隔で)
	2:BIKKURI,	見開いて数秒後パチパチ
	3:DAMAGE	痛い
	4:SIROME	白目
	5:TUBURI	つぶりっぱなし
	6:MIHIRAKI	開きっぱなし
	7:DEATH		死(つぶり、苦しい、ふつう、開き、白目ホールドの順でアニメ)
*******/

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
//#include	"rand.h"
#include	"libutl.h"

#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)

//#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_ONEPIECE)


enum {
	NORMAL,		/*0*/
	MABATAKI,	/*1*/
	BIKKURI,	/*2*/
	DAMAGE,		/*3*/
	SIROME,		/*4*/
	TUBURI,		/*5*/
	MIHIRAKI,	/*6*/
	DEATH,		/*7*/
	END			/*8*/
} ;
enum {
	GBS_MDL,	/*茶色迷彩*/
	URBAN_MDL	/*都市迷彩*/
} ;

#define ANIM_NUM	5	/* パターン種類数 */
#define PTN			3	/* 総パターン数 */

#define BASE_CNT	(BP_BASE_TICK())
extern int BP_FRAMES_PER_SEC();
#define BIKKURI_TIME (BP_FRAMES_PER_SEC()*3) /*ビックリ時間*/


/* アニメリスト パターン番号 表示時間の順で羅列 */
typedef	struct	{
//	GV_ACT		actor ;
	GV_ACT_EX	actor ;
	OBJECT		*body ; /*親 人体モデル*/
	OBJECT		parts[ANIM_NUM] ; 
	int			model_code[ANIM_NUM];
	int			model;
	int			*flag;
	int			count;	/*瞬き*/
	int			oldflag;
} Work ;

#define PRIO	0x40	/* 敵兵より後に処理 */
//static FVECTOR	FACE_SHIFT = { 0.0F, 0.0F, 0.20F ,0.0F} ;
static FVECTOR	FACE_SHIFT = { 0.0F, 0.0F, 0.80F ,0.0F} ;

static void SetAnimInvisible(Work *work)
{
	if(work->model >= ANIM_NUM ) printf("FACE ANIM ERROR\n");
	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ) {
		work->parts[work->model].objs->flag |= DG_FLAG_INVISIBLE ;
	} else {
		work->parts[work->model].objs->flag &= ~DG_FLAG_INVISIBLE ;
	}
/*デバッグ全部非表示*/
//	work->parts[work->model].objs->flag |= DG_FLAG_INVISIBLE ;
}
static void ModelChange(Work *work){
	int i;
	for(i=0;i<ANIM_NUM;i++){
	if(i >= ANIM_NUM ) printf("FACE ANIM ERROR\n");

		if(i==(work->model)){
			DG_VisibleObjs( work->parts[i].objs );
		}else {
			DG_InvisibleObjs( work->parts[i].objs );
		}
	}
}

#define DEATH1 4 /*閉じ*/
#define DEATH2 6 /*苦しい*/
#define DEATH3 3 /*普通*/
#define DEATH4 2 /*見開き*/

static	int	SetAnim(work)
Work *work;
{
	int model;
	if(work->oldflag != *work->flag) work->count=0;
	work->oldflag = *work->flag;

	/*通常*/
	model = *work->flag;
	/*例外処理*/
	switch(*work->flag){
		/*目パチ*/
		case MABATAKI :
			model = NORMAL;
			if(work->count<5){
				model = MABATAKI;
			}else if(work->count==9){
				if(irnd()%4==0)	work->count = 0;
			}else if(work->count>BASE_CNT*20){
				if(irnd()%90==0) work->count = 0;
			}
			break;
		/*見開き後パチパチ*/
		case BIKKURI :
			model = BIKKURI;
			if(work->count >= BIKKURI_TIME){
				if(work->count < (BIKKURI_TIME+4)){
					model = MABATAKI;
				} else if(work->count == (BIKKURI_TIME+8)){
					if(irnd()%4==0)	work->count = BIKKURI_TIME;
				}else if(work->count>BIKKURI_TIME+BASE_CNT*20){
					/*乱数周期で*/
					if((irnd()%(BASE_CNT*10))==0) work->count = BIKKURI_TIME ;
				}
			}
			break;
		case TUBURI :
			model = MABATAKI;
			break;
		case MIHIRAKI :
			model = BIKKURI;
			break;
		case DEATH :
			/*苦しみから白目へ*/
			model = SIROME;
			if(work->count < DEATH1){
				/*閉じ*/
				model = MABATAKI;
			} else if(work->count < (DEATH1+DEATH2)){
				/*苦しい*/
				model = DAMAGE;
			} else if(work->count< (DEATH1+DEATH2+DEATH3)){
				/*普通*/
				model = NORMAL;
			} else if(work->count< (DEATH1+DEATH2+DEATH3+DEATH4)){
				/*見開き*/
				model = BIKKURI;
			}
			break;
	}

	if((model<0)||(model>=ANIM_NUM)){
		printf("FACE ANIME FLAG ERROR!! %d\n",work->model);
		model = NORMAL;
	}
	return model;
}
static	void	Act( work )
Work		*work ;
{
	int model, i ;

	/*flag －１で終了*/
	if(*work->flag == -1){
		GV_DestroyActor( work ) ;
		return ;
	}

	model = SetAnim(work);
	if(work->model != model){
		work->model = model;
		ModelChange(work);
	}
	SetAnimInvisible( work ) ;
	DG_SetPos( &work->body->objs->objs[HUMAN21_ATAMA].world ) ;
	DG_MovePos( &FACE_SHIFT ) ;
#if 1
	/*マップ切り替え対応 2000.01.24*/
	for(i=0;i<ANIM_NUM;i++){
		/*親のマップをセット*/
	if( i >= ANIM_NUM ) printf("FACE ANIM ERROR\n");

		work->parts[i].map_name = work->body->map_name ; 
		GM_GroupObjs( work->parts[i].objs,work->body->map_name ) ;
	}
#endif
	if(work->model >= ANIM_NUM ) printf("FACE ANIM ERROR\n");

	DG_PutObjs( work->parts[work->model].objs ) ;
	work->count++;
	work->count &= 0xffff;
}

static	void	Die( work )
Work		*work ;
{
	int i;
	for(i=0;i<ANIM_NUM;i++){
	if(i >= ANIM_NUM ) printf("FACE ANIM ERROR\n");
		GM_FreeObject( &work->parts[i] );
	}
}

/*----------------------------------------------------------------*/

//def = (DG_DEF*) GV_GetCache( GV_CacheID( model, 'k' ) );

static	int	GetResources( work, body ,flag,type)
Work	*work ;
OBJECT	*body ;	/* ボディーオブジェクト */
int		*flag;
int		type ;
{
	int i;
	/*胴体オブジェクト*/
	work->body = body ;
	/*制御フラグ*/
	work->flag = flag ;

	/*モデルID*/
	switch (type){
		case GBS_MDL :
			/*通常*/
			work->model_code[0] = GV_StrCode( "gbs_eye0" ) ;
			/*閉じ*/
			work->model_code[1] = GV_StrCode( "gbs_eye2" ) ;
			/*開き*/
				work->model_code[2] = GV_StrCode( "gbs_eye1" ) ;
			/*ダメージ*/
			work->model_code[3] = GV_StrCode( "gbs_eye3" ) ;
			/*白目*/
			work->model_code[4] = GV_StrCode( "gbs_eye4" ) ;
			break ;
		case URBAN_MDL :
		default :
			/*通常*/
			work->model_code[0] = GV_StrCode( "gps_eye0" ) ;
			/*閉じ*/
			work->model_code[1] = GV_StrCode( "gps_eye2" ) ;
			/*開き*/
				work->model_code[2] = GV_StrCode( "gps_eye1" ) ;
			/*ダメージ*/
			work->model_code[3] = GV_StrCode( "gps_eye3" ) ;
			/*白目*/
			work->model_code[4] = GV_StrCode( "gps_eye4" ) ;
			break ;
	}
	work->model = 0;
	work->count =0;
	work->oldflag = -1;
	for(i=0;i<ANIM_NUM;i++){
	if(i >= ANIM_NUM ) printf("FACE ANIM ERROR\n");

		GM_InitObject( &work->parts[i], work->model_code[i], DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_IRREACTION );
		if(work->parts[i].objs == NULL){
			printf("AT FACE INIT ERROR\n");
			return -1;
		}

		DG_SetPos( &work->body->objs->objs[HUMAN21_ATAMA].world ) ;
	if(i >= ANIM_NUM ) printf("FACE ANIM ERROR\n");

		DG_PutObjs( work->parts[i].objs ) ;
		DG_SetLightMatrix( work->parts[i].objs, work->body->objs->light );
		DG_InvisibleObjs( work->parts[i].objs );
	}
	DG_VisibleObjs( work->parts[0].objs );
	return 0 ;
}
void	*NewEyeAnim( body ,flag)
OBJECT	*body ;	/* ボディーオブジェクト */
int		*flag;	/* 制御フラグ */
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body ,flag,GBS_MDL) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
/*都市迷彩兵用*/
void	*NewEyeAnimUrban( body ,flag)
OBJECT	*body ;	/* ボディーオブジェクト */
int		*flag;	/* 制御フラグ */
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body ,flag,URBAN_MDL) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

