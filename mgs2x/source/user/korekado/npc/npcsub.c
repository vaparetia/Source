//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	npcsub.c
	NPCサブ

	2001/02/19 Y.Korekado
	$Id: npcsub.c,v 1.1.1.3 2002/11/19 11:44:23 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------
----------------------------------------------------------------*/

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
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"npc.h"


/*----------------------------------------------------------------
korekado/ 内で使えそうな関数

enemy.c

・プレイヤーに死体の上フラグをたてる
void ENE_PlayerOnCorp( FVECTOR *pos )
pos:死体の場所

・posが指定ゾーンのnearに入っているかチェック
int	ENE_InNearZone( FVECTOR	*pos, int zone, int grpid )
返り値 near番号

・開始位置から目標位置へ画面内手前までワープ
void ENE_WarpNearPos( ctrl, addr, dis )
CONTROL	*ctrl ;
int		addr ;	目標位置
int		dis ;	目標までの距離

・ゾーンからゾーンへの方向
int ENE_GetAddrToAddrDir( int from_addr, int to_addr )

・指定方向に一番近いニアゾーン
int ENE_GetMinDirNearZone( zoneadd, dir )

・runが自分と同マップ内でfromから逃げるのに適したゾーン
int	ENE_RunawayZone( CONTROL *run, CONTROL *from )

・posがzoneaddr内なら１外なら０
int	ENE_InZone( FVECTOR *pos, HZX_ZONE_ADD zoneaddr )

・posがtrgのrange内かチェック
int	ENE_InRange( FVECTOR *pos, FVECTOR *trg, int range )

・指定の安全地帯
int	ENE_GetSafeZone( HZX_ZONE_ADD zoneaddr, int num )
一つのゾーンには 0～HZX_MAX_SAFEZONE_NUM個のセーフゾーンがある
全ての安全地帯を取得したいなら
例：
for( i=0; i<HZX_MAX_SAFEZONE_NUM; i++ ) {
	safe[i] = ENE_GetSafeZone( ctrl->addr, i ) ;
	if ( safe[i] == 255 ) return ;
}
safe_num = i ;


----------------------------------------------------------------*/



