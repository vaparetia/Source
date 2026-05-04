//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_obj.c
	オブジェクトパケット処理

	2000/07/26	K.Takabe
	$Id: demo_obj.c,v 1.1.1.3 2002/11/19 11:45:09 Yoshizawa1 Exp $

*/
/*

	void DM_DestroyObjectAll( void )

	指定ＩＤのオブジェクトを破棄


	void DM_DestroyObject( int id )
	int		id ;	破棄するオブジェクトのＩＤ

	指定ＩＤのオブジェクトを破棄


	void* DM_GetObjectHandle( int id )
	int		id ;	オブジェクトのＩＤ

	オブジェクトのハンドルを取得


	void DM_SetObjectInvisible( int id, int invisible_flag )
	int		id ;				オブジェクトのＩＤ
	int		invisible_flag ;	非表示フラグ（最下位ビットからチャンネル０～３に対応）

	オブジェクトの表示状態を変更する


	CONTROL* DM_GetObjectControl( void *handle )
	void	*handle ;	オブジェクトのハンドル

	オブジェクトのハンドルからコントロールを取得


	OBJECT* DM_GetObjectObject( void *handle )
	void	*handle ;	オブジェクトのハンドル

	オブジェクトのハンドルから（ゲームの）オブジェクトを取得


	FMATRIX* DM_GetObjectObject( void *handle, int joint )
	void	*handle ;	オブジェクトのハンドル
	int		joint ;		マトリクスを取得する関節番号

	オブジェクトのハンドルから指定関節のマトリクスを取得


	--------------------------------
	＜パケット別処理ルーチン用関数＞

	void DM_Packet_Object( DM_WORK *work, DEMO_OBJECT *packet )
	オブジェクトパケット処理
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <mgs_type.h>
#include <libgv.h>
#include <libdg.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>
#include <camera.h>

#include "libdemo.h"

/* ---------------------------------------------------------------- */
/* オブジェクトパケット処理 */
void DM_Packet_Object( DM_WORK *work, DEMO_OBJECT *packet )
{
	DM_OBJECT	*object ;

   if ( !DEMO_PACKET_IS_ENDIAN_SWAP( packet->header ) )
   {
      DEMO_PACKET_SET_ENDIAN_SWAP( packet->header );

      BP_LE_SwapSInt_Inp( &packet->cache_id );
      BP_LE_SwapSInt_Inp( &packet->flag );
      BP_LE_SwapSInt_Inp( &packet->name );
      BP_LE_SwapSInt_Inp( &packet->kms_cache_id );
   }

#ifdef DEBUG_MODE
	/* デバッグ用コード */
	/* スロー再生・逆転再生時に連続呼びしないようにチェックする */
#if 1
	if ( DM_GetObjectHandleCore(DEMO_PACKET_ID(packet->header)) != NULL )
	    return ;
#else
	if ( DM_GetObjectHandle(DEMO_PACKET_ID(packet->header)) != NULL )
	    return;
#endif

#endif

	/* オブジェクト管理構造体取得 */
	object = &work->obj_list[ work->n_object ] ;
	object->id = DEMO_PACKET_ID( packet->header );
	object->flag = 0; /* テスト */
	/* オブジェクト表示アクターの起動 */
	object->work_ptr = NewDemoObject( DEMO_PACKET_ID( packet->header ), packet->name, packet->cache_id, packet->kms_cache_id, packet->flag );
	if ( object->work_ptr == NULL ) return ;
	GV_SetActorChild( work->actor, object->work_ptr );

	work->n_object++ ;
	ASSERT( work->n_object < DM_MAX_OBJECT );
}

/* ---------------------------------------------------------------- */
	/*
		指定ＩＤのオブジェクトを破棄
	*/
void DM_DestroyObjectAll( void )
{
	DM_WORK		*work ;
	DM_OBJECT	*object ;
	int			i ;

	/* デモ管理構造体へのポインタを取得 */
	if ( ( work = DM_GetDemoWork() ) == NULL ) return ;

	object = work->obj_list ;
	for ( i = work->n_object ; i > 0 ; object++, i-- ){
		GV_CallSignalFunc( object->work_ptr, GV_SIGNAL_KILL, 0 );
	}
}
/* ---------------------------------------------------------------- */
	/*
		指定ＩＤのオブジェクトを破棄
	*/
void DM_DestroyObject( int id )
{
	DM_WORK		*work ;
	DM_OBJECT	*object ;
	int			i ;

	/* デモ管理構造体へのポインタを取得 */
	if ( ( work = DM_GetDemoWork() ) == NULL ) return ;

	/* 検索 */
	object = work->obj_list ;
	for ( i = work->n_object ; i > 0 ; object++, i-- ){
		if ( object->id == id ) break ;
	}
	if ( i == 0 ) return ;

	/* オブジェクト表示アクターに終了シグナルを送る */
	GV_CallSignalFunc( object->work_ptr, GV_SIGNAL_KILL, 0 );

	/* 削除するオブジェクト以降を前に詰める */
	for (  ; i > 0 ; object++, i-- ){
		object[0] = object[1] ;
	}
}
	/*
		オブジェクトの表示状態を変更する
	*/
void DM_SetObjectInvisible( int id, int invisible_flag )
{
	void		*handle ;

	if ( ( handle = DM_GetObjectHandle( id ) ) == NULL ) return ;
	GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_VISIBLE, invisible_flag );
}
/* ---------------------------------------------------------------- */
	/*
		オブジェクトのハンドルを取得
	*/
void* DM_GetObjectHandleCore( int id )
{
	DM_WORK		*work ;
	DM_OBJECT	*object ;
	int			i ;

	/* デモ管理構造体へのポインタを取得 */
	if ( ( work = DM_GetDemoWork() ) == NULL ) return ( NULL );

	/* 検索 */
	object = work->obj_list ;
	for ( i = work->n_object ; i > 0 ; object++, i-- ){
		if ( object->id == id ) return ( object->work_ptr );
	}
	return NULL ;
}

void *DM_GetObjectHandle( int id )
{
    void *ret ;

    if ( !(ret = DM_GetObjectHandleCore( id )) )
    {
      printf( "Not found object !!!!!!!!!! Please Link to Object!!!!\n" ) ;
      //BP - likely that there's some bad demo object struct alignment causing this case.
      //Look up the callstack a bit and see if there's some demo data being misinterpreted.
      //Add padding to fix, wash rinse repeat.
      if( id != -1)
      {
         printf("BP WARNING: MISSING DEMO (CUTSCENE) OBJECT. If this is not just after Ames, could be a problem.\n");
      }
    }
    return ret ;
}

/* ---------------------------------------------------------------- */
	/*
		オブジェクトのハンドルからコントロールを取得
	*/
CONTROL* DM_GetObjectControl( void *handle )
{
	if ( handle == NULL ) return ( NULL );
	return ( (CONTROL*)GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_GET_CONTROL, 0 ) );
}
	/*
		オブジェクトのハンドルから（ゲームの）オブジェクトを取得
	*/
OBJECT* DM_GetObjectObject( void *handle )
{
	if ( handle == NULL ) return ( NULL );
	return ( (OBJECT*)GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_GET_OBJECT, 0 ) );
}
	/*
		オブジェクトのハンドルから（ゲームの）EVMオブジェクトを取得
	*/
DG_EVMOBJ* DM_GetEvmObjObject( void *handle )
{
	if ( handle == NULL ) return ( NULL );
	return ( (DG_EVMOBJ*)GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_GET_EVM, 0 ) );
}
	/*
		オブジェクトのハンドルから DG_OBJS を取得
	*/
DG_OBJS* DM_GetObjectDgObjs( void *handle )
{
	if ( handle == NULL ) return ( NULL );
	return ( (DG_OBJS *)GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_GET_DGOBJS, 0 ) );
}

	/*
		オブジェクトのハンドルから指定関節のマトリクスを取得
	*/
FMATRIX* DM_GetObjectMatrix( void *handle, int joint )
{
	if ( handle == NULL ) return ( NULL );
	return ( (FMATRIX *)GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_GET_MATRIX, joint ) );
}

	/*
		オブジェクトのハンドルから指定関節のマトリクスを取得
	*/
FVECTOR* DM_GetObjectPos( void *handle, int joint )
{
	FMATRIX *mat ;
	if ( handle == NULL ) return ( NULL );

	mat = (FMATRIX*)GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_GET_MATRIX, joint ) ;
	return (FVECTOR *)mat->m[3] ;
}
	/*
		オブジェクトのハンドルから DG_OBJS を取得
	*/
DG_OBJ* DM_GetObjectDgObj( void *handle, int joint )
{
	if ( handle == NULL ) return ( NULL );
	return ( (DG_OBJ* )GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_GET_DGOBJ, joint ) );
}

	/*
		オブジェクトのハンドルから ライトマトリックス を取得
	*/
FMATRIX* DM_GetObjectLightMtx( void *handle )
{
	if ( handle == NULL ) return ( NULL );
	return ( (FMATRIX *)GV_CallSignalFunc( handle, DM_SIGNAL_OBJECT_GET_LIGHTMTX, 0 ) );
}
