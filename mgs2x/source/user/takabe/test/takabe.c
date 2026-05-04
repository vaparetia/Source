/*
	takabe.c
	モデル表示実験プログラム

	1999/07/07 K.Takabe
	$Id: takabe.c,v 1.1.1.3 2002/11/19 11:51:30 Yoshizawa1 Exp $

*/

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#define DUMMY_CHARA_MAX (30)
#if 1
#define DUMMY_CHARA_MODEL GV_StrCode( "gbs_def" )
#define DUMMY_CHARA_MOTION GV_StrCode("gbs")
#define DUMMY_CHARA_MOTION_NUM (0)
#else
#define DUMMY_CHARA_MODEL GV_StrCode( "vmp_test" )
#define DUMMY_CHARA_MOTION GV_StrCode("deku")
#define DUMMY_CHARA_MOTION_NUM (11)
#endif

typedef struct {
	CONTROL		control ;
	OBJECT		body ;
	FMATRIX		lights[2] ;
	int			add_rot ;
} CHARA_DATA ;


typedef	struct	{
	GV_ACT		actor ;
	int			mode ;
	CONTROL		control ;
	OBJECT		body ;
	FMATRIX		lights[2] ;
	void		*motions[ 32 ];

	/* 影実験用 */
	OBJECT		shadow ;

	/* ダミーキャラ用 */
	CHARA_DATA	chara[DUMMY_CHARA_MAX] ;
	FVECTOR		rots[DUMMY_CHARA_MAX];

	/* ステージオブジェクト用 */
	DG_OBJS		*st_objs ;

	/* ステージライト用 */
	DG_LIT		*lit ;
	int			n_lit ;

	SVECTOR		test_rot ;
	int			rot_target ;

	int			lit_rot_y ;
	int			count ;

} Work ;


void		StoFVector( FVECTOR *fvec, SVECTOR *svec )
{
	float		rot_x, rot_y, rot_z ;
	int			tmp ;

	tmp = svec->vz ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_z = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = svec->vx ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_x = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = svec->vy ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_y = (float)tmp * (float)M_PI / 2048.0F ;
	fvec->vx = rot_x ;
	fvec->vy = rot_y ;
	fvec->vz = rot_z ;
}


static void Act( Work *work )
{
	int	i, flag, pad, speed = 1 ;
	FVECTOR	step, quat, quat_base ;

#if 1
#if 0
	DG_SetFogColor( 50,80,100 );
	DG_SetFogParam( 1000.0f, 90000.0f );

	/* キャラクタ表示実験 */
	//work->body.objs->flag |= DG_FLAG_INVISIBLE ;
	GM_ActControl( &work->control ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot );
	GM_ActObject( &work->body );
	DG_GetLightMatrix( &work->control.mov, work->lights );
	work->control.height = work->body.height ;
	if ( work->control.grounded & 1 ) work->control.step.vy = 0.0F ;
	work->control.step.vy -= 64.0F ;
	work->control.rot.vy += 15 ;
	work->control.turn.vy += 15 ;

	work->control.step.vx = 0.0F ;
	work->control.step.vz = 0.0F ;

	pad = GV_PadData[0].status ;
	if ( pad & PAD_L2 ) work->test_rot.vx += 15 ;
	if ( pad & PAD_R2 ) work->test_rot.vx -= 15 ;
	StoFVector( &work->rots[work->rot_target], &work->test_rot );
	if ( GV_PadData[0].press & PAD_B ){
		work->rots[ work->rot_target ] = DG_ZeroVector ;
		if ( ++work->rot_target > 25 ) work->rot_target = 0 ;
	}
#if 0
	//work->body.objs->flag |= DG_FLAG_INVISIBLE ;
	for ( i = 0 ; i < 16 ; i++ ) work->shadow.objs->objs[i].world = work->body.objs->objs[i].world ;
	DG_PutObjs( work->shadow.objs );
	{
		FVECTOR		lit_pos = { 0.0f, 3000.0f, 6000.0f, 1.0f };
		SVECTOR		srot = {0,0,0,0};
		srot.vy = work->lit_rot_y ; work->lit_rot_y += 5 ;
		DG_SetPos2( &work->control.mov, &srot );
		DG_PutVector( &lit_pos, &lit_pos, 1 );
		DG_MakeShadowVolumeObjs( work->shadow.objs, &lit_pos, DG_Clock );
	}
#endif
	

#if 0
	/* モーション補間実験 */
	if ( GV_PadData[0].press & PAD_X ){
		MT_SetMotionInterp( work->m_ctrl, 50, 0x003fe );
		/* モーションオーバーライド実験 */
		MT_SetMotionData( work->m_ctrl, 1, GV_StrCode( "stand2_fig" ), 0, 0x003fe );
		work->m_ctrl->merge_flag = 0x0002 ;
	}
	if ( GV_PadData[0].release & PAD_X ){
		MT_SetMotionInterp( work->m_ctrl, 50, 0x003fe );
		/* モーションオーバーライド実験 */
		MT_ResetMotionData( work->m_ctrl, 1 );
		work->m_ctrl->merge_flag = 0x0000 ;
	}
#endif
#if 0
	/* モーション左右対称再生実験 */
	if ( GV_PadData[0].press & PAD_Y ){
		MT_SetMotionInterp( work->body.m_ctrl, 50, 0x003fe );
		work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1 ;
	}
	if ( GV_PadData[0].release & PAD_Y ){
		MT_SetMotionInterp( work->body.m_ctrl, 50, 0x003fe );
		work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL1 ;
	}
#endif
#endif

#if 1
	/* ダミーキャラ表示 */
	for ( i = 0 ; i < DUMMY_CHARA_MAX ; i++ ){
		CHARA_DATA	*chara ;
		chara = &work->chara[i] ;

		if ( work->count == 0 ){
			GM_ActControl( &chara->control ) ;
			DG_SetPos2( &chara->control.mov, &chara->control.rot );
			GM_ActObject( &chara->body );
			DG_GetLightMatrix( &chara->control.mov, chara->lights );
		}

		chara->control.height = chara->body.height ;
		if ( chara->control.grounded & 1 ) chara->control.step.vy = 0.0F ;
		//chara->control.step.vy -= 64.0F ;
		//chara->control.rot.vy += chara->add_rot ;
		//chara->control.turn.vy += chara->add_rot ;
	}
	work->count++ ;
#endif

#endif

}

static void Die( Work *work )
{
}

static int GetResources( Work *work, int where )
{
	int	i ;
	char	buff[256] ;
	DG_DEF	*def ;
	FVECTOR	vec ;

#if 0
	{/* カメラの設定 */
		FVECTOR	cam = {1000.0F/4,2000.0F/4,-1000.0F/4,0.0F}, trg = {0.0F,0.0F,0.0F,0.0F} ;
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
		DG_SetCamera2( DG_Chanls, &cam, &trg, 2.0F );
	}
	{/* カメラの設定 */
		FVECTOR	cam = {-1000.0F,2000.0F,-1500.0F,0.0F}, trg = {0.0F,1000.0F,0.0F,0.0F} ;
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
		DG_SetCamera2( DG_Chanls + 1, &cam, &trg, 2.0F );
	}

	/* コントロールの初期化 */
	GM_InitControl( &work->control, 222, 0 ) ;
	work->control.mov = DG_ZeroVector ; 
	work->control.mov.vy = 1049.0F*3 ;
	work->control.mov.vz = -3049.0F ;
	work->control.step = DG_ZeroVector ;
	work->control.rot = DG_ZeroSVector ;
	work->control.turn = DG_ZeroSVector ;
	work->control.hzd = HZD_GetHandler( GV_StrCode( "01a" ) ) ;
	work->control.hzd_height = 750 ;
	work->control.height = 1049.0F ;
	GM_ConfigControlHazard( &work->control, 1200, 450, 500 ) ;
	work->control.seg_flag = 0 ;
	work->control.skip_flag |= CTRL_HZD_SEG_HORIZON_CHECK ;

	/* 関節型モデルの表示 */
	GM_InitObject( &work->body, GV_StrCode( "vmp_test" ), DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE );
	work->body.objs->rots = work->rots ;
	GM_ConfigObjectLight( &work->body, work->lights );
	GM_ConfigObjectMotion( &work->body, 2, GV_StrCode("dummy"), MT_FLAG_HUMAN1 );
	GM_ConfigObjectStep( &work->body, &work->control.step );
	GM_ConfigObjectAction( &work->body, 0, GV_StrCode( "run2_fig" ), 0, 0xfffff, 0 );
	/* モーションオーバーライド実験 */
	GM_ConfigObjectAction( &work->body, 1, GV_StrCode( "stand2_fig" ), 0, 0x003fe, 0 );
	work->body.m_ctrl->merge_flag = 0x0002 ;
	/* モーションアジャスト実験 */
	vec = DG_ZeroVector ;
	vec.vx = 0.5F ;
	MT_EulerToQuatXYZ( &work->body.m_ctrl->adjust[1], &vec );
	work->body.m_ctrl->adjust_flag = 0x0002 ;

#endif

	/* ダミーキャラ初期化 */
	for ( i = 0 ; i < DUMMY_CHARA_MAX ; i++ ){
		CHARA_DATA	*chara ;
		chara = &work->chara[i] ;
		/* オブジェクト初期化 */
		GM_InitObject( &chara->body, DUMMY_CHARA_MODEL, DG_FLAG_SHADE|DG_FLAG_FINISHCALC );
		//GM_InitObject( &chara->body, GV_StrCode( "vmp_test" ), DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE );
//chara->body.objs->flag |= DG_FLAG_INVISIBLE ;
		GM_ConfigObjectMotion( &chara->body, 1, DUMMY_CHARA_MOTION, MT_FLAG_HUMAN2 );
		GM_ConfigObjectStep( &chara->body, &chara->control.step );
		GM_ConfigObjectAction( &chara->body, 0, DUMMY_CHARA_MOTION_NUM, 0, 0xfffff, 0 );
		//GM_ConfigObjectAction( &chara->body, 0, GV_StrCode( "run2_fig" ), 0, 0xfffff, 0 );
		//GM_ConfigObjectAction( &chara->body, 0, 1, 0, 0xfffff, 0 );
		GM_ConfigObjectLight( &chara->body, chara->lights );
		/* コントロール設定 */
		GM_InitControl( &chara->control, 10000+i, 0 ) ;
		GM_ChangeControlMapFromMapId(  &chara->control, where )
		chara->control.mov.vx = 0.0F ;
		chara->control.mov.vy = 1000.0F ;
		chara->control.mov.vz = -3000.0F ;
	chara->control.mov.vx = 0.0F + rand()%5000-2500;
	chara->control.mov.vy = 1000.0F ;
	chara->control.mov.vz = -3000.0F - rand()%5000+2500 + 20000 ;
		chara->control.rot.vx = 0 ;
		chara->control.rot.vy = rand() % 4096 ;
		chara->control.rot.vz = 0 ;
		chara->control.turn = chara->control.rot ;
		//chara->control.hzd_id = HZD_GetHandler( GV_StrCode( "tnk3" ) ) ;
		chara->control.hzd_height = 750 ;
		chara->control.height = 1049.0F ;
		GM_ConfigControlHazard( &chara->control, 1200, 450, 500 ) ;
		chara->control.seg_flag = 0 ;
		chara->control.skip_flag |= CTRL_HZD_SEG_HORIZON_CHECK ;
		chara->add_rot = rand() % 60 - 30 ;
	}


#if 0
	GM_InitObject( &work->shadow, GV_StrCode( "sdw_doll" ), DG_FLAG_PAINT|DG_FLAG_SHADOWVOL|DG_FLAG_FINISHCALC );
	DG_MakePreshade( work->shadow.objs, NULL, 0 );
	work->shadow.objs->root = &work->body.objs->world ;
	work->shadow.objs->rots = work->rots ;
#endif


#if 0
	{/* ライトの初期化 */
		DG_LITS_OLD	*lights ;
		lights = (DG_LITS_OLD*)GV_GetCache( GV_CacheID( GV_StrCode( "01a" ), 'l' ) ) ;
		work->lit = GV_Malloc( sizeof(DG_LIT) * lights->n_lights );
		for ( i = 0 ; i < lights->n_lights ; i++ ){
			work->lit[i].point.vx = lights->lights[i].point.vx ;
			work->lit[i].point.vy = lights->lights[i].point.vy ;
			work->lit[i].point.vz = lights->lights[i].point.vz ;
			work->lit[i].color = lights->lights[i].color ;
			work->lit[i].r_range = lights->lights[i].r_range ;
			work->lit[i].e_range = lights->lights[i].e_range ;
#if 0
			work->lit[i].force = sqrtf(
									   (float)( 
											   work->lit[i].color.r * work->lit[i].color.r
											   + work->lit[i].color.g * work->lit[i].color.g
											   + work->lit[i].color.b * work->lit[i].color.b
											   )
									   ) / 255.0F ;
#else
			work->lit[i].force = 
			  ( (float)work->lit[i].color.r * 0.30F
			   + (float)work->lit[i].color.g * 0.59F
			   + (float)work->lit[i].color.b * 0.11F ) / 255.0F ;
#endif
		}
		work->n_lit = lights->n_lights ;
		/* ライトをステージライトとして登録 */
		//DG_ResetFixedLight();
		//DG_SetFixedLight( work->lit, work->n_lit );
	}

	/* ステージモデルをプリシェードして表示 */
	def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode( "01a" ), 'k' ) ) ;
	work->st_objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE|DG_FLAG_SHADOWWRITE, 0 );
	//DG_MakePreshade( work->st_objs, work->lit, work->n_lit );
	DG_QueueObjs( work->st_objs );
#endif
	//def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode( "01a" ), 'k' ) ) ;
	//work->st_objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE|DG_FLAG_SHADOWWRITE, 0 );
	//DG_QueueObjs( work->st_objs );

	return (0);
}


void *NewTestProgram( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
