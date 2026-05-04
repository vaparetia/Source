//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	Dogtag.c								*/
/*	ドッグタグ								*/
/*	2001/06/21 H.Satoyoshi							*/
/*	$Id: dogtag.c,v 1.1.1.3 2002/11/19 11:48:18 Yoshizawa1 Exp $			*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
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
#include "../util/sato_util.h"

typedef struct {
    GV_ACT_EX		actor ;
    OBJECT		body;
    FVECTOR		*shift;
    OBJECT		*oya_obj;
    int			*flag;
    int			*model_name;
}DT_Work;

/********************************************************************************/
/*	extern									*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

/**************************<-------local function------>*************************/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:	終了処理:ワーク解放    						*/
/********************************************************************************/
static void Die(DT_Work *work){
    GM_FreeObject(&work->body);
}

/**************************<-------local function------>*************************/
/*	名前:	void Act							*/
/*	引数:	Work	*work							*/
/*	説明:	指定のオブジェに追従						*/
/********************************************************************************/
static void Do_Dog_Act(DT_Work *work){

    // ***プログラム終了
    if (*work->flag == -1){
	GV_DestroyActor(work);
    }

    //マトリクスの計算 libDGを使用
    DG_SetPos( &work->oya_obj->objs->objs[HUMAN21_MUNE].world );	//マトリクスを計算ワークに設定
    DG_MovePos( work->shift );					//計算ワークを相対移動
    DG_GetPos( &work->body.objs->world );			//計算ワークからマトリクスを取得
    DG_GetPos( &work->body.objs->objs[0].world );		//計算ワークからマトリクスを取得

    //	非表示フラグの設定
    work->body.objs->flag &= ~(DG_FLAG_INVISIBLE);
    if( work->oya_obj->evmobj ){		//マルチウェイトモデルかどうかの判定
	work->body.objs->flag |= (work->oya_obj->evmobj->flag & DG_EVMOBJ_INVISIBLE)<<4;
    }else{
	work->body.objs->flag |= work->oya_obj->objs->flag & DG_FLAG_INVISIBLE;
    }

    if ( !(PL_GetPlayerItem() == IT_Thermal) ){
	work->body.objs->flag |= DG_FLAG_INVISIBLE;
    }

}

/**************************<-------local function------>*************************/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/********************************************************************************/
static int GetResources(DT_Work *work,OBJECT *oya_obj, int model_name, FVECTOR *shift, int *flag){
    GM_InitObject(&work->body,
		  model_name,
		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_ONEPIECE);
    work->oya_obj = oya_obj;
    work->shift	= shift;
    work->flag = flag;
    return 1;
}

/*******************************<Global function>********************************/
/*	名前:	void NewCreateDogTag						*/
/*	引数:	OBJECT	*oya_obj	ドッグタグをつけている人のオブジェ	*/
/*		int	*model_name	ドッグタグモデル名			*/
/*		FVECTOR	*shift		シフト値				*/
/*		int	*flag		終了フラグ -1:終了			*/
/*	説明:	ドッグタグ作成							*/
/********************************************************************************/
void *NewCreateDogtag(OBJECT *oya_obj, int model_name, FVECTOR *shift, int *flag)
{
    DT_Work *work ;
    work = (DT_Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( DT_Work ), 0x40 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Do_Dog_Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources(work, oya_obj, model_name, shift, flag)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}








