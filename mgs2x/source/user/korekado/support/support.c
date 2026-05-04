//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	support.c
	見張り兵

	1999/08/30 Y.Korekado
	$Id: support.c,v 1.1.1.3 2002/11/19 11:44:25 Yoshizawa1 Exp $

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

#include	"support.h"
#include	"spprepro.c"
#include	"spaction.c"
#include	"spthink.c"
#include	"../watcher/wcgetres.c"

static void SupportMain( Work *work )
{
	/* 前処理 */
	PreProcess( work ) ;
	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;
	/* 思考処理 */
	Think( work ) ;
	/* 行動処理 */
	Action( work ) ;
}

static void Act( Work *work )
{
	ENE_GM_Act( &work->enethink ) ;

	ENE_PreProcess( &work->enethink ) ;
	SupportMain( work ) ;
	ENE_AftProcess( &work->enethink ) ;
}

static void Die( Work *work )
{
	ENE_FreeResources( &(work->enethink) ) ;
}
/*--- ------------------------------------------------------------*/
/*
	シナリオオプション使用状況
	s:ステータス r:ルート n:ノード d:ディフェンス位置
*/
static int GetResources( Work *work, int name )
{
	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	ACTION		*act ;
	OBJECT		*body ;
	int			buddy ;

	entk = &work->enethink ;
	ctrl = &work->control ;
	act = &work->action ;
	body = &work->body ;

	if ( WatcherGetResources( work, name ) < 0 ) return -1 ;

	/***** サポート兵固有処理 *****/
	AT_SetType( act, ENE_TYPE_SUPPORT ) ;

	if ( (buddy = GCL_GetOptionValue( 'b', -1 )) < 0  ) {
//		buddy = 0 ;
		return -1 ;
	}

	/* サポートする兵の名前セット */
	/* サポートする敵兵をセット */
	if ( (entk->spbudy = COM_SetSupportEnemy( buddy )) == NULL ) {
		printf("support.c: Err No Budy[%d]\n",buddy ) ;
		return -1 ;
	}
	entk->bd_uniq_id  = entk->spbudy->uniq_id ;
	entk->bd_eyei.flag = EYE_INFO_FLAG_SKIP ;

	/*思考開始場所セット */
	ENE_WatcherStartModeSneak( &work->enethink ) ;

	work->action.CheckDamage = ENE_EnemyDamagePad ;

	/* ステージ情報当てはめ */
	ENE_LoadEneMemory( entk ) ;

	/* 武器位置最後に初期位置へリセット */
	ENE_ResetWeaponPosition( entk ) ;

/*待機場所から1フレームだけ見つけちゃう問題*/
/*視界off状態で起動*/
	work->action.status |= ACT_STATUS_EYE_CLOSE ;

	return (0);
}

void *NewSupport( name, where)
int	name ;
int	where ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
