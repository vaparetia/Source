//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ceiling.c
	天井設置 指定の高さより下にカメラがくると描画

	1999/09/09 Y.Korekado
	$Id: ceiling.c,v 1.1.1.3 2002/11/19 11:44:01 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewCeiling( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-h	高さ

----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"camera.h"
#include	"define.h"
#include	"korekado/conv/korekado.x"

/*----------------------------------------------------------------*/
#define	BODY_FLAG	(DG_FLAG_PAINT|DG_FLAG_ONEPIECE|DG_FLAG_AUTOREPAINT)
#define MAX_CAMERA	(4)

#define MAX_CHECK_PLANE		(8)

/*----------------------------------------------------------------*/
typedef	struct
{
	GV_ACT		actor ;
	OBJECT		body ;

	FVECTOR		norm[MAX_CHECK_PLANE] ;			/* カメラ切り替え方向（平面定数算出用） */
	FVECTOR		check_pos[MAX_CHECK_PLANE] ;	/* 基準点座標（平面定数算出用） */
	FVECTOR		plane[MAX_CHECK_PLANE] ;		/* 平面定数 */
	int			name ;
	int			height ;
	int			n_plane ;		/* チェック面数 */
	int			check_mode ;	/* 複数面チェック時の条件設定（and,or） */

	int			invisible ;	/* 非表示フラグ */

	int			inv_proc ;
#ifndef MGS2_TRIAL
	int			status ;
#endif

}
SCeilingWork;

/*----------------------------------------------------------------*/
#define CEILING_STATUS_VISIBLE	0x0001	/* 常に表示 */
/*----------------------------------------------------------------*/

static int CameraCheck( SCeilingWork *work, int camera )
{
    GM_CameraSet	*cam ;
    float t ;
	int		and_flag, or_flag, tmp_flag, i ;

	and_flag = 1 ;
	or_flag = 0 ;

	cam = GM_GetCurrentCamera( camera ) ;

	for ( i = 0 ; i < work->n_plane ; i++ ){
		t = work->plane[i].vx * cam->position.vx + work->plane[i].vy * cam->position.vy + 
		  work->plane[i].vz * cam->position.vz + work->plane[i].vw ;
		tmp_flag = ( t > 0.0f ) ? 1 : 0 ;
		and_flag &= tmp_flag ;
		or_flag |= tmp_flag ;
	}

	if ( work->check_mode == 0 ) return ( and_flag );
	return ( or_flag );
}

extern int gBP_ShowCeilingsCheat;

static int Ceiling( SCeilingWork	*work )
{
    int i, flag, proc_call ;

#ifndef MGS2_TRIAL
	if ( work->status & CEILING_STATUS_VISIBLE ) {
		if ( (work->invisible & DG_FLAG_INVISIBLE) != DG_FLAG_INVISIBLE ) {
			SET_FLAG( work->invisible, DG_FLAG_INVISIBLE ) ;
			SET_FLAG( work->body.objs->flag, DG_FLAG_INVISIBLE ) ;
		}
		return 0 ;
	}
#endif

	proc_call = 0 ;
	for ( i=0; i<MAX_CAMERA; i++ )
   {
		if ( GM_CheckCameraActive( i ) )
      {
			flag = DG_FLAG_INVISIBLE0 << i ;
         if (gBP_ShowCeilingsCheat)
         {
            UNSET_FLAG( work->invisible, flag ) ;
            UNSET_FLAG( work->body.objs->flag, flag ) ;
         }
         else
         {
			   if ( CameraCheck( work, i ) )
            {	/* 天井表示 */
				   if ( work->invisible & flag )
               {
					   UNSET_FLAG( work->invisible, flag ) ;
					   UNSET_FLAG( work->body.objs->flag, flag ) ;
					   proc_call = 1 ;
				   }
			   }
            else
            {								/* 天井非表示 */
				   if ( !(work->invisible & flag) )
               {
					   SET_FLAG( work->invisible, flag ) ;
					   SET_FLAG( work->body.objs->flag, flag ) ;
					   proc_call = 1 ;
				   }
			   }
         }
		}
	} 
	
	return proc_call ;
}

/*----------------------------------------------------------------*/
#ifndef MGS2_TRIAL
enum {
	CEIL_VISIBLE = 1,
	CEIL_VISIBLE_OFF,
} ;

static void CheckMessage( SCeilingWork *work )
{
	GV_MSG *msg;
	int mes_num ;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		switch( msg->message[0] ){
		  case CEIL_VISIBLE :
			SET_FLAG( work->status, CEILING_STATUS_VISIBLE ) ;
			break;
		  case CEIL_VISIBLE_OFF :
			UNSET_FLAG( work->status, CEILING_STATUS_VISIBLE ) ;
			break;
		}
		msg--;
	}
}
#endif

static	void	Act( SCeilingWork *work )
{
#ifndef MGS2_TRIAL
	CheckMessage( work ) ;
#endif

	if ( Ceiling( work ) ) {
		if ( work->inv_proc != 0 ) {
			GCL_ARGS arg ;

			arg.argc = 1 ;
			arg.argv = &work->invisible ;
			GCL_ExecProc( work->inv_proc, &arg ) ;
		}
	}
}

static	void	Die( SCeilingWork *work )
{
	DG_FreePreshade( work->body.objs ) ;
	GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( SCeilingWork *work, int name, int where )
{
	FVECTOR		pos ;
	char		*opt ;
	int			model, lit ;
	int			i ;

	work->name = name ;

	work->n_plane = 1 ;

	if ( (model = GCL_GetOptionValue( 'm', 0 )) != 0 ) {
		GM_InitObject( &work->body, model, BODY_FLAG );
	} else {
		return -1 ;
	}

	/* 表示グループセット */
	KR_GroupObject( &work->body, where ) ;

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &pos ) ;
	} else {
		return -1 ;
	}

	/* チェック面数の決定 */
	if ( GCL_GetOption( 'e' ) != NULL ){
		work->n_plane = GCL_GetNextInt();
		work->check_mode = GCL_GetNextInt();
	}

	//height = GCL_GetOptionValue( 'h', 0 ) ;
	//work->height = height ;
	/* 高さの取得（デフォルトのＹ軸座標として設定する） */
	if ( GCL_GetOption( 'h' ) != NULL ){
		work->check_pos[0].vx = 0.0f ;
		work->check_pos[0].vy = GCL_GetNextInt() ;
		work->check_pos[0].vz = 0.0f ;
	}
	/* デフォルトのＹ軸負方向チェックとして設定 */
	work->norm[0].vx = 0.0f ;
	work->norm[0].vy = -1.0f ;
	work->norm[0].vz = 0.0f ;

	/* 拡張設定（判定座標の取得） */
	if ( GCL_GetOption( 'c' ) != NULL ){
		for ( i = 0 ; i < work->n_plane ; i++ ){
			work->check_pos[i].vx = GCL_GetNextInt() ;
			work->check_pos[i].vy = GCL_GetNextInt() ;
			work->check_pos[i].vz = GCL_GetNextInt() ;
		}
	}
	/* 拡張設定（判定方向の取得） */
	if ( GCL_GetOption( 'n' ) != NULL ){
		for ( i = 0 ; i < work->n_plane ; i++ ){
			work->norm[i].vx = GCL_GetNextInt() ;
			work->norm[i].vy = GCL_GetNextInt() ;
			work->norm[i].vz = GCL_GetNextInt() ;
			_sceVu0Normalize( &work->norm[i], &work->norm[i] );
		}
	}
	/* 拡張設定（簡易チェック方向の取得） */
	if ( GCL_GetOption( 'a' ) != NULL ){
		for ( i = 0 ; i < work->n_plane ; i++ ){
			switch ( GCL_GetNextInt() ){
			  case 0:	/* X+ */
				work->norm[i].vx = 1.0f ; work->norm[i].vy = 0.0f ; work->norm[i].vz = 0.0f ;
				break ;
			  case 1:	/* X- */
				work->norm[i].vx = -1.0f ; work->norm[i].vy = 0.0f ; work->norm[i].vz = 0.0f ;
				break ;
			  case 2:	/* Y+ */
				work->norm[i].vx = 0.0f ; work->norm[i].vy = 1.0f ; work->norm[i].vz = 0.0f ;
				break ;
			  default:
			  case 3:	/* Y- */
				work->norm[i].vx = 0.0f ; work->norm[i].vy = -1.0f ; work->norm[i].vz = 0.0f ;
				break ;
			  case 4:	/* Z+ */
				work->norm[i].vx = 0.0f ; work->norm[i].vy = 0.0f ; work->norm[i].vz = 1.0f ;
				break ;
			  case 5:	/* Z- */
				work->norm[i].vx = 0.0f ; work->norm[i].vy = 0.0f ; work->norm[i].vz = -1.0f ;
				break ;
			}
		}
	}

	/* 平面の定数を求める */
	for ( i = 0 ; i < work->n_plane ; i++ ){
		work->plane[i] = work->norm[i] ;
		work->plane[i].vw = - ( ( work->plane[i].vx * work->check_pos[i].vx ) +
							   ( work->plane[i].vy * work->check_pos[i].vy ) +
							   ( work->plane[i].vz * work->check_pos[i].vz ) ) ;
		printf("ceiling.c: no.%d  a=%f b=%f c=%f d=%f\n", i,
			   work->plane[i].vx, work->plane[i].vy, work->plane[i].vz, work->plane[i].vw );
	}


	DG_SetPos2( &pos, &DG_ZeroSVector ) ;
	DG_PutObjs( work->body.objs ) ;
	if ( (lit = GCL_GetOptionValue( 'l', 0 )) != 0 ) {
		ENE_PreShade( work->body.objs, lit ) ;
	}

	/* 表示切替えプロック */
	if ( ( opt = GCL_GetOption( 'v' ) ) != NULL ){
		work->inv_proc = GCL_GetNextInt( ) ;
	} else {
		work->inv_proc = 0 ;
	}

	work->invisible = 0 ;	/* 初めは全カメラに対してONにする */
#ifndef MGS2_TRIAL
	work->status = 0 ;
#endif

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewCeiling( int name, int where )
{
	SCeilingWork		*work ;

	OPERATOR() ;
	work = (SCeilingWork *)GV_NewActor( GV_ACTOR_AFTER, sizeof( SCeilingWork ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
