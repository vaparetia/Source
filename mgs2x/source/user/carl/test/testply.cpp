/*
 *   うそプレイヤー
 * 	2001/07/24 M.Kobayashi
 *  $Id: testply.cpp,v 1.1.1.3 2002/11/19 11:43:02 Yoshizawa1 Exp $
 */

#include "gameheader.h"
#include "../../../game/camera.h"


enum {
	STAND,
	WALK,
	RUN,
};


/////////////////////////////////////////////////////
typedef struct _WORK {	// ローカルなワークエリア宣言
	GV_ACT actor;

	float	fRot;
	int		mode;
	int		name;
	
	OBJECT	object;
	CONTROL	ctrl;
	VECTOR	vecRootDiff;
	
	MATRIX	matLights[2];

	int nMot;

} WORK;

extern "C" void *NewDebugCamera(int		chanl );
extern "C" void *NewDebugCameraForce(int chanl);

static int GetResources(WORK* pWork, int name, int where)
{

	pWork->name = name;

	/* コントロールの初期化 */
	FVECTOR pos;
	SVECTOR rot;
	if( GCL_GetOption( 'p' ) != NULL ) {
		int buf[3];
		GCL_GetIV( GCL_NextStr(), buf );
		pos( (float)buf[0], (float)buf[1], (float)buf[2], 1.f); 		
	}else{
		pos( 0.f, 0.f, 0.f, 1.f);
	}
	if( GCL_GetOption( 'd' ) != NULL ){
		int buf[3];
		GCL_GetIV( GCL_NextStr(), buf );
		rot.vx = buf[0]; rot.vy = buf[1]; rot.vz = buf[2];
	}else{
		rot.vx = rot.vy = rot.vz = 0;
	}

	GM_InitControl( &pWork->ctrl, name, where );
	GM_ConfigControlHzxHeight( &pWork->ctrl, 700.f, 0.f );
	GM_ConfigControlPosition( &pWork->ctrl, &pos, &rot );
	GM_ConfigControlObject( &pWork->ctrl, &pWork->object );
	GM_ConfigControlHazard( &pWork->ctrl, 1000, 474, 500);
	GM_ConfigControlTrapCheck( &pWork->ctrl );
	GM_ConfigControlAttribute( &pWork->ctrl, CTRL_ATR_ITEMCHECK | CTRL_ATR_PITFALLCHECK | 
							   CTRL_ATR_NEARONLINE ) ;
	
	HZX_SetEvent( &( pWork->ctrl.evt ), name, &pWork->ctrl.mov, &pWork->ctrl.rot ) ;
	GM_ConfigControlCameraTrapCheck( &pWork->ctrl ) ;
    GM_ConfigControlMapCheck( &pWork->ctrl ) ;

	pWork->ctrl.seg_flag |= HZX_SEG_NO_PLAYER ;
	pWork->ctrl.flr_flag |= HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ;
	pWork->ctrl.skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;

	/* オブジェクトの初期化 */
//	GM_InitObject( &pWork->object, GV_StrCode("sna_def"),
//				   DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE
//				   |DG_FLAG_PLUGINDRAW | DG_FLAG_TEST | DG_FLAG_SHADOWWRITE );
	GM_InitObject( &pWork->object, GV_StrCode("sna_def"),
				   DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE);
	DG_GetLightMatrix( &pos, pWork->matLights);
	GM_ConfigObjectLight( &pWork->object, pWork->matLights ) ;
	GM_ConfigObjectMotion( &pWork->object, 1, GV_StrCode("snake"), MT_FLAG_HUMAN2 );
	GM_ConfigObjectStep( &pWork->object, &pWork->ctrl.step );
	GM_ConfigObjectAction( &pWork->object, 0, 0, 0, 0xffffffff, 0 );

	// root データでトラップチェックするように設定
	pWork->vecRootDiff(0.f, 0.f, 0.f, 1.f);
	pWork->ctrl.root_offset = &pWork->vecRootDiff;

//	pWork->ctrl.hzx_id = GM_GetBit( 0 );	///

	// 仮イニシャライズ
	GM_ActMotion( &pWork->object );
	pWork->ctrl.height = pWork->object.height ;
	GM_ActControl( &pWork->ctrl );
	GM_ActObject2( &pWork->object );

	pWork->nMot = 0;
	pWork->fRot = 0.f;

	pWork->mode = STAND;

	GM_PlayerControl = &pWork->ctrl;
	GM_PlayerBody = &pWork->object;

#if 1
    NewDebugCamera( 0 ) ;
    NewDebugCameraForce( 0 ) ;
    NewDebugCamera( 1 ) ;
#endif	
	
	return 0;	// リソース確保に失敗した場合は負の値を返す
}

static void Act(WORK* pWork)
{
#if 0
	if((GV_PadData[0].press & PAD_D) && pWork->nMot < pWork->object.m_ctrl->motion_arc->n_motion - 1){
		pWork->nMot++;
		GM_ConfigObjectAction( &pWork->object, 0, pWork->nMot, 0, 0xffffffff, 0 );
		printf("motion %d / %d\n", pWork->nMot, pWork->object.m_ctrl->motion_arc->n_motion);
	}
	if((GV_PadData[0].press & PAD_U) && pWork->nMot > 0){
		pWork->nMot--;
		GM_ConfigObjectAction( &pWork->object, 0, pWork->nMot, 0, 0xffffffff, 0 );
		printf("motion %d / %d\n", pWork->nMot, pWork->object.m_ctrl->motion_arc->n_motion);
	}
#endif
	
	float fThDiff = (0x80 - GV_PadData[0].right_dx) / 3200.f;
	pWork->fRot += fThDiff;
	if( pWork->fRot > M_PI ) pWork->fRot -= 2.f * M_PI;
	if( pWork->fRot < -M_PI ) pWork->fRot += 2.f * M_PI;

	GM_ActMotion( &pWork->object );
	pWork->ctrl.height = pWork->object.height ;
	GM_ActControl( &pWork->ctrl );
	GM_ActObject2( &pWork->object );
	pWork->ctrl.step.vy -= 96.f;

	// 方向決定
	if( GV_PadData[0].dir != -1 ){
		int dir;
		dir = GV_PadData[0].dir;
//		dir += (int)(pWork->fRot * 2048.f / M_PI);
		dir &= 0xfff;
		pWork->ctrl.turn.vy = dir;
	}
	GM_PlayerDir = pWork->ctrl.turn.vy & 0xfff;

	// モーション決定
	int intensity = max(abs(GV_PadData[0].left_dx - 0x80), abs(GV_PadData[0].left_dy - 0x80));
	const int TH_WALK = 0x40;
	const int TH_RUN = 0x70;

	switch ( pWork->mode ){
	case STAND:
		if( intensity > TH_RUN ) {
			pWork->mode = RUN;
			GM_ConfigObjectAction( &pWork->object, 0,
								   18,
								   0, 0xffffffff, 30 );
			pWork->vecRootDiff(0.f, 0.f, 0.f);
		} else if ( intensity > TH_WALK ) {
			pWork->mode = WALK;
			GM_ConfigObjectAction( &pWork->object, 0,
								   17,
								   0, 0xffffffff, 30 );
			pWork->vecRootDiff(0.f, 0.f, 0.f);
		}
		break;
	case WALK:
		if( intensity > TH_RUN ) {
			pWork->mode = RUN;
			GM_ConfigObjectAction( &pWork->object, 0,
								   18,
								   0, 0xffffffff, 30 );
			pWork->vecRootDiff(0.f, 0.f, 0.f);
		} else if ( intensity < TH_WALK ) {
			pWork->mode = STAND;
			GM_ConfigObjectAction( &pWork->object, 0,
								   0,
								   0, 0xffffffff, 30 );
			pWork->vecRootDiff(0.f, 0.f, 0.f);
		}
		break;
	case RUN:
		if( intensity < TH_WALK ) {
			pWork->mode = STAND;
			GM_ConfigObjectAction( &pWork->object, 0,
								   0,
								   0, 0xffffffff, 30 );
			pWork->vecRootDiff(0.f, 0.f, 0.f);
		} else if ( intensity < TH_RUN ) {
			pWork->mode = WALK;
			GM_ConfigObjectAction( &pWork->object, 0,
								   17,
								   0, 0xffffffff, 30 );
			pWork->vecRootDiff(0.f, 0.f, 0.f);
		}
		break;
	}

	// root データ処理
	if ( !( pWork->object.m_ctrl->mt3_ctrl->flag & MT3_SLEEP ) ) {
		pWork->vecRootDiff.x += pWork->object.m_ctrl->root_step.vx - pWork->object.m_ctrl->step.vx;
		pWork->vecRootDiff.z += pWork->object.m_ctrl->root_step.vz - pWork->object.m_ctrl->step.vz;
		pWork->vecRootDiff.y = pWork->object.m_ctrl->root_height - pWork->object.m_ctrl->height ;
    } 

	// カメラ設定
#if 0
	VECTOR cam;
	VECTOR trg;
	trg(pWork->ctrl.mov.x, pWork->ctrl.mov.y, pWork->ctrl.mov.z);
	
	MATRIX mat;
	D3DXMatrixRotationY(&mat, pWork->fRot);
	cam( 0.f, 100.f, 3500.f, 0.f );
	D3DXVec3Transform(&cam, &cam, &mat);

	cam += trg;
	cam.w = trg.w = 0.f;
   BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
	DG_SetCamera2(DG_Chanls, &cam, &trg, 2.0f);
#endif	

    GM_CameraTarget = pWork->ctrl.mov + pWork->vecRootDiff;
	GM_PlayerDir = pWork->ctrl.turn.vy & 0xfff;
	GM_CameraDir = pWork->ctrl.rot;
	
	// ライト取得
	DG_GetLightMatrix( &pWork->ctrl.mov, pWork->matLights);
}

static void Die(WORK* pWork)
{
	GM_FreeControl(&pWork->ctrl);
	GM_FreeObject(&pWork->object);
	GM_PlayerControl = NULL;
	GM_PlayerBody = NULL;
}


extern "C" void* NewControlTest(int name, int mapid)
{
	WORK* pWork;

	// アクター用メモリを確保
    if ((pWork = (WORK *)GV_NewActor(GV_ACTOR_USER, sizeof(WORK))) != NULL){
		// メモリが確保できれば、関数を設定する
		GV_SetActor( &(pWork->actor ), Act, Die ) ;
		if ( GetResources( pWork, name, mapid ) < 0 ) {	// リソースを確保する
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
    }
    return (void *)pWork ;
}
