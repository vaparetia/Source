//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	se_exchg.c
	シーケンスデータ再生処理関連

	1999/12/21 K.Takabe
	$Id: se_exchg.c,v 1.1.1.3 2002/11/19 11:42:53 Yoshizawa1 Exp $
*/
#ifdef PSX2
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
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"
#include	"libgcl.h"

#include	"g_define.h"
#include	"g_sound.h"

//#define LOCAL_DEBUG
#ifdef LOCAL_DEBUG
#define DEBUG_MES(str,param)	printf( str,param )
#define DEBUG_MES2(str,param1,param2)	printf( str,param1,param2 )
#else
#define DEBUG_MES(str,param)	/**/
#define DEBUG_MES2(str,param1,param2)	/**/
#endif


MT_SE_EXCHANGE		MT_SeExchange ;



/* ---------------------------------------------------------------- */
	/*
		変換リスト全体の初期化
	*/
void MT_InitExchangeSeList( void )
{
	int		i ;

	/* 変換リストの初期化 */
	for ( i = 0 ; i < MAX_EXCHG_LIST ; i++ ){
		MT_SeExchange.se_list[ i ].target_se_code = 0xffff ;
		MT_SeExchange.se_list[ i ].id = -1 ;
		MT_SeExchange.se_list[ i ].type = 0xff ;
		MT_SeExchange.se_list[ i ].data_ptr = &MT_SeExchange.exchg_data[ i ][ 0 ] ;
	}
}

	/*
		指定ＩＤのリストを削除
	*/
void MT_DeleteExchangeSeList( int table_id )
{
	int		i ;

	/* 変換リストの初期化 */
	for ( i = 0 ; i < MAX_EXCHG_LIST ; i++ ){
		if ( MT_SeExchange.se_list[ i ].id == table_id ){
			MT_SeExchange.se_list[ i ].id = -1 ;
			MT_SeExchange.se_list[ i ].target_se_code = 0xffff ;
			MT_SeExchange.se_list[ i ].type = 0xff ;
		}
	}
}

	/*
		指定ＩＤ＆指定コードのリストを削除
	*/
void MT_DeleteExchangeSeListCode( int table_id, int code )
{
	int		i ;

	/* 変換リストの初期化 */
	for ( i = 0 ; i < MAX_EXCHG_LIST ; i++ ){
		if ( MT_SeExchange.se_list[ i ].id == table_id ){
			if ( MT_SeExchange.se_list[ i ].target_se_code == code ){
				MT_SeExchange.se_list[ i ].id = -1 ;
				MT_SeExchange.se_list[ i ].target_se_code = 0xffff ;
				MT_SeExchange.se_list[ i ].type = 0xff ;
			}
		}
	}
}

	/*
		空きの変換リストテーブルのポインタを取得する
	*/
MT_EXCHG_SE_LIST* MT_GetExchangeSeList( void )
{
	int		i ;

	for ( i = 0 ; i < MAX_EXCHG_LIST ; i++ ){
		if ( MT_SeExchange.se_list[ i ].target_se_code == 0xffff ){
			return ( &MT_SeExchange.se_list[ i ] );
		}
	}
	return ( NULL );
}

	/*
		変換リストテーブルに登録されているものがあるか
		 登録されている   : 1
		 登録されていない : 0
	*/
int MT_IsSeListRegist( void )
{
	int		i ;

	for ( i = 0 ; i < MAX_EXCHG_LIST ; i++ ){
		if ( MT_SeExchange.se_list[ i ].target_se_code != 0xffff ){
		    return 1 ;/* 登録されている */
		}
	}
	return 0 ;/* 登録されていない */
}

/* ---------------------------------------------------------------- */

	/*
		対象となる効果音変換が設定されている場合には変換結果を返す
	*/
void MT_GetExchangeSe( MT_EXCHG_SE *se_data, int se_code, int table_id, int hazard_type, int segment_type )
{
	int		i ;

	DEBUG_MES("se_exchg.c: se code :0x%x\n", se_code );
	DEBUG_MES("se_exchg.c: hazard :%d\n", hazard_type );
	DEBUG_MES("se_exchg.c: segment :%d\n", segment_type );
	for ( i = 0 ; i < MAX_EXCHG_LIST ; i++ ){
		if ( MT_SeExchange.se_list[ i ].id != table_id ) continue ;
		if ( MT_SeExchange.se_list[ i ].target_se_code != se_code ) continue ;
		if ( MT_SeExchange.se_list[ i ].type == 0 ){
			/* ハザード用属性参照 */
			if ( hazard_type < 0 ){
				se_data->se_code = -1 ;
				return ;
			}
			*se_data = MT_SeExchange.se_list[ i ].data_ptr[ hazard_type ] ;
			DEBUG_MES("se_exchg.c: hit hazard :0x%x\n", se_data->se_code );
		} else {
			/* セグメント用属性参照 */
			if ( segment_type < 0 ){
				se_data->se_code = -1 ;
				return ;
			}
			*se_data = MT_SeExchange.se_list[ i ].data_ptr[ segment_type ] ;
			DEBUG_MES("se_exchg.c: hit segment :0x%x\n", se_data->se_code );
		}
		return ;
	}
	/* 該当するものがなかった場合 */
	se_data->se_code = se_code & 0x0fff ;
	se_data->noise_vol = 0 ;
	se_data->flag = 0 ;
}

	/*
		リストの設定コマンド（ＧＣＬ用）
	*/
int NewSetExchangeSeCode( void )
{
	int		id, type, code ;
	MT_EXCHG_SE_LIST	*list ;
	int		i ;

	/* リストのクリア設定 */
    if ( GCL_GetOption( 'c' ) != NULL ) {
		MT_InitExchangeSeList();
	}

	id = 0 ;
	type = 0 ;

	/* 登録ＩＤの取得 */
    if ( GCL_GetOption( 'i' ) != NULL ) {
		id = GCL_GetNextInt() ;
	} else {
		return ( 0 );
	}

	/* 設定タイプの取得 */
    if ( GCL_GetOption( 't' ) != NULL ) {
		type = GCL_GetNextInt() ;
	}

	/* 変換対象コードの設定 */
    if ( GCL_GetOption( 's' ) != NULL ) {
		code = GCL_GetNextInt() ;
	} else {
		return ( 0 );
	}

	/* 既に設定済みであればテーブルを削除する */
	MT_DeleteExchangeSeListCode( id, code );
	/* 空きリストの作成 */
	list = MT_GetExchangeSeList();
	if ( list == NULL ){
		printf("se_exchg.c: can't create se exchange list!!!\n");
		return (-1);
	}

	DEBUG_MES2("se_exchg.c: regist se code id:%d code:0x%x \n", id, code );
	list->id = id ;
	list->type = type ;
	list->target_se_code = code ;

	/* リストデータの生成 */
    if ( GCL_GetOption( 'd' ) != NULL ) {
		for ( i = 0 ; i < 8 ; i++ ){
			list->data_ptr[i].se_code = GCL_GetNextInt() ;
			list->data_ptr[i].noise_vol = GCL_GetNextInt() ;
		}
	}
	
	return ( 0 );
}
