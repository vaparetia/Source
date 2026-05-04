//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	har_command.c								*/
/*	ハリア  コマンド/プロック						*/
/*	2001/07/09 H.Satoyoshi							*/
/*	$Id: har_command.c,v 1.1.1.3 2002/11/19 11:48:20 Yoshizawa1 Exp $		*/
/********************************************************************************/


/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#include "harrier.h"


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandGetPosition()					*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカアイテムセット						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewHarItemSet()
{
    Kas_Work *kwork;

    kwork = GET_KAK_WORK;

    if (kwork->is_bossrush == ON){
	printf("U can't use command [ItemSet] if U using Snake");
	ASSERT(0);
	return -1;
    }

    if (kwork->item_reserve_flag == 0){    
	KAS_ITEM_DROP_POS.vx = 1.0f * GCL_GetNextInt();
	KAS_ITEM_DROP_POS.vy = 1.0f * GCL_GetNextInt();
	KAS_ITEM_DROP_POS.vz = 1.0f * GCL_GetNextInt();
	KAS_ITEM_NUM = GCL_GetNextInt();

	printf ("Receive Item Putting order pos:%f, %f, %f, num:%d \n",KAS_ITEM_DROP_POS.vx,KAS_ITEM_DROP_POS.vy,KAS_ITEM_DROP_POS.vz,
		KAS_ITEM_NUM);
	kwork->item_reserve_flag = 1;
    }
    return 1;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandGetPosition()					*/
/*	引数:	Work	*work							*/
/*	説明:	ハリアの座標を返す						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewHar_CommandGetPosition()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    Work	*work;

    if ( GET_HAR_WORK == NULL )
        return 0;

    work = (Work*)GET_HAR_WORK;

    // *************  中心
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->control.mov.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->control.mov.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->control.mov.vz ) ;

    // *************  鼻先
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->pos_hanna.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->pos_hanna.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->pos_hanna.vz ) ;

    // *************  尻尾
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->pos_sippo.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->pos_sippo.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->pos_sippo.vz ) ;

    // *************  翼端右
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->wing_edge1.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->wing_edge1.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->wing_edge1.vz ) ;

    // *************  翼端左
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->wing_edge2.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->wing_edge2.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->wing_edge2.vz ) ;

    return 1 ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandGetClaster()					*/
/*	引数:	Work	*work							*/
/*	説明:	クラスター位置取得						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewHar_CommandGetClaster()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    Work	*work;

    if ( GET_HAR_WORK == NULL )
        return 0;
    work = (Work*)GET_HAR_WORK;

    // *************  右クラスター爆弾位置
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->r_cls_pos.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->r_cls_pos.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->r_cls_pos.vz ) ;

    // *************  左クラスター爆弾位置
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->l_cls_pos.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->l_cls_pos.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->l_cls_pos.vz ) ;




#ifdef DEBUG_MODE    
    if (HAR_RoutView == ON){
	FVECTOR	verts[2];
	verts[0] = work->l_cls_pos;
	verts[1] = work->r_cls_pos;
	NewLineView(verts, 1, 255 , 0, 0);
    }
#endif

    return 1;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandGetClaster()					*/
/*	引数:	Work	*work							*/
/*	説明:	ミサイル位置取得						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewHar_CommandGetAmram()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    Work	*work;

    if ( GET_HAR_WORK == NULL )
        return 0;
    work = (Work*)GET_HAR_WORK;

    // *************  右クラスター爆弾位置
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->r_amr_pos.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->r_amr_pos.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->r_amr_pos.vz ) ;

    // *************  左クラスター爆弾位置
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->l_amr_pos.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->l_amr_pos.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->l_amr_pos.vz ) ;

    return 1;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandSetInvisible()					*/
/*	引数:	Work	*work							*/
/*	説明:	非表示設定							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewHar_CommandSetInvisible(){
    Work	*work;

    if ( GET_HAR_WORK == NULL )
	return 0;
    work = (Work*)GET_HAR_WORK;

    work->body.objs->flag |= DG_FLAG_INVISIBLE;

    printf ("ハリアー透明化セット\n");
    return 1;
}


