//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	Equip.c									*/
/*	装備品表示								*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: equip.c,v 1.1.1.3 2002/11/19 11:48:18 Yoshizawa1 Exp $			*/
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
#include "libutl.h"
#include "eq_list.h"
#include "equip.h"

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
static void Die(Work *work)
{
    GM_FreeObject(&(work->body));
}

/**************************<-------local function------>*************************/
/*	名前:	void NullAct							*/
/*	引数:	Work	*work							*/
/*	説明:	空 Ａct								*/
/********************************************************************************/
static void NullAct(Work *work)
{
}

/**************************<-------local function------>*************************/
/*	名前:	void Act							*/
/*	引数:	Work	*work							*/
/*	説明:	指定のオブジェに追従						*/
/********************************************************************************/
static void Act(Work *work)
{
    OBJECT	*tmp;
    FMATRIX	tmpmat;
    short	Data_Num = work->number;
    tmp = work->oya_obj;

    tmpmat = tmp->objs->objs[work->eq_data[Data_Num].joint_id].world;

    //マトリクスの計算 libDGを使用
    DG_SetPos(&tmpmat);					//マトリクスを計算ワークに設定
    DG_MovePos( &work->eq_data[Data_Num].shift );		//計算ワークを相対移動
    DG_RotatePos( &work->eq_data[Data_Num].rot );		//計算ワークを相対回転
    DG_GetPos(&tmpmat);					//計算ワークからマトリクスを取得
    _sceVu0CopyMatrix( &(work->body.objs->world), &tmpmat);


    //	非表示フラグの設定
    work->body.objs->flag &= ~(DG_FLAG_INVISIBLE);
    if( tmp->evmobj ){		//マルチウェイトモデルかどうかの判定
	work->body.objs->flag |= ((tmp->evmobj->flag & DG_EVMOBJ_INVISIBLE)<<4) & (tmp->objs->flag & DG_FLAG_INVISIBLE) ;
    }else{
	work->body.objs->flag |= tmp->objs->flag & DG_FLAG_INVISIBLE;
    }
		       
}

/**************************<-------local function------>*************************/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/********************************************************************************/
static int GetResources(Work *work, OBJECT *object, int id, int Data_Num)
{
    work->oya_obj = object;	//被装備オブジェクト登録
    work->number = Data_Num;	//装備セット番号保存
    work->eq_data = equip_list[id] ;
    GM_InitObject(&work->body,
		  GV_StrCode( work->eq_data[Data_Num].m_name ),
		  DG_FLAG_SHADE|DG_FLAG_ONEPIECE);

    printf( "NewEquipments(%d) %d\n", id, Data_Num ) ;

    Act(work);
    return 1;
}

/**************************<-------local function------>*************************/
/*	名前:	void *NewEquipments						*/
/*	引数:	OBJECT *object							*/
/*		short num							*/
/*	説明:	装備品を表示するアクター					*/
/********************************************************************************/
static void *NewEquipments(OBJECT *object, int id, int num)
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 255 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources(work, object, id, num)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}


/**************************<-------local function------>*************************/
/*	名前:	int GetNullResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	フラグを分解して装備品をコール					*/
/********************************************************************************/
static int GetNullResources(Work *work, OBJECT *object, short ID, int Data_Num)
{
    int	loop=0;

    ASSERT( equip_n_list[ID] < 32 ) ;

    while (loop < equip_n_list[ID]){ 
	if (Data_Num&1 ){
	    GV_SetActorChild( work,NewEquipments(object, ID, loop) );//子アクターを呼ぶ
	}
	loop++;
	Data_Num = Data_Num >> 1;
    }
    return 1;
}


/*******************************<Global function>********************************/
/*	名前:	void NewCreateEquipment						*/
/*	引数:	OBJECT	*object		装備品つける対象オブジェクト		*/
/*		short	ID		装備品セットID				*/
/*		short	Data_Num	装備品フラグ				*/
/*	説明:	装備品管理アクター作成						*/
/********************************************************************************/
void *NewCreateEquipment(OBJECT *object, short ID, int Data_Num)
{

    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),NullAct,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetNullResources(work, object, ID,  Data_Num)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}

#ifdef DEBUG_MODE
/*******************************<Global function>********************************/
/*	名前:	void *NewHarrier						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewTestEquip( int name, int where )
{
    OBJECT *object;

    object = GM_PlayerBody ;    

    NewCreateEquipment(object, 0, 63) ;

    return (void *)object ;
}
#endif


void *NewCreateEquipmentScn( int name, int where )
{
    int i   ;
    int id  ;
    int num ;
    void *parent, *child ;

    extern void* DM_GetDemoWork( void ) ;

    name = GCL_GetOptionValue( 'n', 7698271/*rai_def_mh_demo  */ ) ;
    id   = GCL_GetOptionValue( 'i', 3      /*ライデン銃 装備品*/ ) ;
    num  = GCL_GetOptionValue( 'e', 1      /*装備フラグ       */ ) ;
    
    if ( GCL_GetOption( 'd' ) )
    {
	if ( (parent = DM_GetDemoWork()) )
	    parent = (int)parent - sizeof(GV_ACT_EX) ; 
    }
    else
	parent = NULL ;
    
    for ( i=GM_N_WhereList ; --i>=0 ; )
    {
	if ( GM_WhereList[i]->name == name )
	{
	    child = NewCreateEquipment( GM_WhereList[i]->object, id, num )  ;
	    if ( parent && child )
		GV_SetActorChild( parent, child ) ;
	    return child ;
	}
    }

    return NULL ;
}
