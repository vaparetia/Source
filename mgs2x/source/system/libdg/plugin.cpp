/*
	plugin.c
	プラグイン管理ルーチン

	2000/02/26 K.Takabe
	$Id: plugin.cpp,v 1.5 2002/11/23 11:36:54 Yoshizawa1 Exp $

*/
/*

	void DG_InitPluginSystem( void )

		プラグインシステムの初期化


	DG_PLUGIN *DG_SearchPlugin( int object_id )
	int		object_id ;	プラグイン認識ＩＤ

		現在登録中のプラグインの中から指定ＩＤの物を探し出す


	DG_PLUGIN *DG_MakePlugin( int id, int flag, int phase, int prio, void *func, int n_queue )
	int		id ;		プラグイン認識ＩＤ
	int		flag ;		初期化フラグ
	int		phase ;		実行フェース指定
	int		prio ;		同一フェーズ内でのプライオリティ
	void	*func ;		実行コールバック関数（DG_PluginCallback型）
	int		n_queue ;	確保オブジェクトキュー（必要な場合のみ）

		プラグイン管理構造体を生成


	void DG_FreePlugin( DG_PLUGIN *plugin )
	DG_PLUGIN	*plugin ;	プラグイン管理構造体

		プラグイン管理構造体の開放


	void DG_AddPlugin( DG_PLUGIN *add_plugin )
	DG_PLUGIN	*add_plugin ;	プラグイン管理構造体

		プラグインをシステムに組み込む


	void DG_DeletePlugin( DG_PLUGIN *del_plugin )
	DG_PLUGIN	*del_plugin ;	プラグイン管理構造体

		プラグインをシステムから削除


	---------------------------------------------

	void DG_PluginStartChanl( DG_CHANL *cp, int which )
	DG_CHANL	*cp ;	チャンネル構造体
	int			whitch ;処理バッファ番号

		プラグイン処理開始チャンネル


	void DG_PluginEndChanl( DG_CHANL *cp, int which )
	DG_CHANL	*cp ;	チャンネル構造体
	int			whitch ;処理バッファ番号

		プラグイン処理終了チャンネル


	void DG_PluginChanl( DG_CHANL *cp, int which )
	DG_CHANL	*cp ;	チャンネル構造体
	int			whitch ;処理バッファ番号

		プラグイン処理チャンネル




*/

#ifdef KP_XBOX //BP

#ifndef KP_XBOX
#include <windows.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"



#define PLUGIN_MAX	(32)		/* 同時登録プラグインの最大数は３２まで */


/* ---------------------------------------------------------------- */

static struct {
	DG_PLUGIN	start ;
	DG_PLUGIN	end ;
} DG_PluginList ;

static int	PluginExecPhase ;


/* ---------------------------------------------------------------- */
	/*
		プラグインシステムの初期化
	*/
void DG_InitPluginSystem( void )
{
	/* リンクリスト初期化 */
	DG_PluginList.start.prev = NULL ;
	DG_PluginList.start.next = &DG_PluginList.end ;
	DG_PluginList.end.prev = &DG_PluginList.start ;
	DG_PluginList.end.next = NULL ;
	
}

	/*
		現在登録中のプラグインの中から指定ＩＤの物を探し出す
	*/
DG_PLUGIN *DG_SearchPlugin( int object_id )
{
	DG_PLUGIN	*plugin ;
	plugin = &DG_PluginList.start ;
	while ( ( plugin = plugin->next ) != &DG_PluginList.end ){
		if ( plugin->object_id == object_id ){
			return ( plugin );
		}
	}
	return ( NULL );
}

	/*
		プラグイン管理構造体を生成
	*/
void DG_MakePlugin( DG_PLUGIN *plugin, int id, int flag, int phase, int prio, void *func, int n_queue )
{
	static DG_PLUGIN dummy_plugin ;

	plugin->flag = flag ;
	plugin->object_id = id ;
	plugin->phase = phase ;
	plugin->priority = prio ;
	plugin->exec_func = (DG_PluginCallback)func ;

	/* オブジェクトバッファ割り当て */
	if ( flag & DG_PLUGIN_FLAG_OBJBUFFER ){
		plugin->obj_buffer = DG_MakeUserObjectBuffer( n_queue, id );
	} else {
		plugin->obj_buffer = NULL ;
	}

	return ;
}

	/*
		プラグイン管理構造体の開放
	*/
void DG_FreePlugin( DG_PLUGIN *plugin )
{
	if ( plugin->obj_buffer != NULL ){
		DG_FreeUserObjectBuffer( plugin->obj_buffer );
	}
}

	/*
		プラグインをシステムに組み込む
	*/
void DG_AddPlugin( DG_PLUGIN *add_plugin )
{
	DG_PLUGIN	*plugin, *last_plugin ;

	last_plugin = &DG_PluginList.start ;	/* プラグイン挿入位置 */
	plugin = &DG_PluginList.start ;
	while ( ( plugin = plugin->next ) != &DG_PluginList.end ){
		if ( plugin->priority <= add_plugin->priority ){
			last_plugin = plugin ;
			continue ;
		}
		break ;
	}
	add_plugin->prev = last_plugin ;
	add_plugin->next = last_plugin->next ;
	add_plugin->prev->next = add_plugin ;
	add_plugin->next->prev = add_plugin ;
}

	/*
		プラグインをシステムから削除
	*/
void DG_DeletePlugin( DG_PLUGIN *del_plugin )
{
	DG_PLUGIN	*plugin, *prev, *next ;
	plugin = &DG_PluginList.start ;
	while ( ( plugin = plugin->next ) != &DG_PluginList.end ){
		if ( plugin == del_plugin ){
			prev = plugin->prev ;
			next = plugin->next ;
			prev->next = next ;
			next->prev = prev ;
			return ;
		}
	}
}


/* ---------------------------------------------------------------- */
	/*
		プラグイン処理開始チャンネル
	*/
void DG_PluginStartChanl( DG_CHANL *cp, int which )
{
	PluginExecPhase = DG_PLUGIN_PHASE_FIRST ;
}

	/*
		プラグイン処理終了チャンネル
	*/
void DG_PluginEndChanl( DG_CHANL *cp, int which )
{
	PluginExecPhase = 0 ;
}

	/*
		プラグイン処理チャンネル
	*/
void DG_PluginChanl( DG_CHANL *cp, int which )
{
	DG_PLUGIN	*plugin ;
	int			enable_check ;

#ifdef KP_WINDOWS
	DG_PerfStartD(DGPERF_CTGR_PLUGIN) ;
#endif
	MARK( "plugin.c" );
	plugin = &DG_PluginList.start ;
	enable_check = DG_PLUGIN_FLAG_ENABLE0 << cp->chanl_num ;

	while ( ( plugin = plugin->next ) != &DG_PluginList.end ){
		/* 実行フェーズを検証 */
		if ( plugin->phase & PluginExecPhase ){
			if ( !( plugin->flag & enable_check ) ) continue ;
			/* プラグインを実行 */
			(*plugin->exec_func)( cp, which, plugin->obj_buffer, PluginExecPhase );
		}
	}

	PluginExecPhase <<= 1 ;
#ifdef KP_WINDOWS
	DG_PerfEndD(DGPERF_CTGR_PLUGIN) ;
#endif
}

#endif
