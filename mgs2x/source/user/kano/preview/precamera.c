/*

	precamera.c
	デザイナープレビュー環境用：カメラ制御
	1999/07/07 S.Okajima
	2000/02/08 K.Kano Modified
	$Id: precamera.c,v 1.1.1.3 2002/11/19 11:43:29 Yoshizawa1 Exp $

*/


#include "preview_def.h"


static void CalcCameraLen(void)
{
	float tmp,sum=0.0f;

	tmp=(PreviewCamera.ftrg_pos.vx-PreviewCamera.fcam_pos.vx);
	sum+=tmp*tmp;
	tmp=(PreviewCamera.ftrg_pos.vy-PreviewCamera.fcam_pos.vy);
	sum+=tmp*tmp;
	tmp=(PreviewCamera.ftrg_pos.vz-PreviewCamera.fcam_pos.vz);
	sum+=tmp*tmp;

	PreviewCamera.cam_len=fpu_Sqrt(sum);
}

void Preview_SetDefaultCameraData(void)
{
    PreviewCamera.ftrg_pos.vx = CAM_DEFAULT_TRG_X;
    PreviewCamera.ftrg_pos.vy = CAM_DEFAULT_TRG_Y;
    PreviewCamera.ftrg_pos.vz = CAM_DEFAULT_TRG_Z;
    PreviewCamera.fcam_pos.vx = CAM_DEFAULT_CAM_X;
    PreviewCamera.fcam_pos.vy = CAM_DEFAULT_CAM_Y;
    PreviewCamera.fcam_pos.vz = CAM_DEFAULT_CAM_Z;
    PreviewCamera.zoom        = CAM_DEFAULT_ZM;

	PreviewCamera.tcontrol_index=0;

	CalcCameraLen();
}

void Preview_GetNowCameraData(void)
{
    GM_CameraSet *now;

    /* 現行カメラから */
    now = GM_GetCurrentCameraSet( 0 ) ;

    fpu_CopyVector(&(PreviewCamera.fcam_pos),&(now->position));
    fpu_CopyVector(&(PreviewCamera.ftrg_pos),&(now->target));
	PreviewCamera.cam_len=now->track;
    PreviewCamera.zoom=now->angle/2.0f;
}

void CopyCameraToSavework(void)
{
	fpu_CopyVector(&(SaveData.camera.ftrg_pos),&(PreviewCamera.ftrg_pos));
	fpu_CopyVector(&(SaveData.camera.fcam_pos),&(PreviewCamera.fcam_pos));
    SaveData.camera.zoom=PreviewCamera.zoom;
}

void CopyCameraFromSavework(void)
{
    fpu_CopyVector(&(PreviewCamera.ftrg_pos),&(SaveData.camera.ftrg_pos));
    fpu_CopyVector(&(PreviewCamera.fcam_pos),&(SaveData.camera.fcam_pos));
    PreviewCamera.zoom=SaveData.camera.zoom;

	CalcCameraLen();
}


/* ---------------------------------------------------------------------- */

void Camera_DebugPrint(void)
{
    int	x,y;


    if( !PreviewCamera.cam_disp_flag ){
		DEBUG_Color( 1, 1, 1, 1 );
    }

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,0);
    DEBUG_Printf("CAMERA MENU");

    x=LOCATE_X+FONT_WIDTH;
    y=LOCATE_Y+COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("CAM POS: %d %d %d",
				 (int)(PreviewCamera.fcam_pos.vx),
				 (int)(PreviewCamera.fcam_pos.vy),
				 (int)(PreviewCamera.fcam_pos.vz));

	y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("TRG POS: %d %d %d",
				 (int)(PreviewCamera.ftrg_pos.vx),
				 (int)(PreviewCamera.ftrg_pos.vy),
				 (int)(PreviewCamera.ftrg_pos.vz));

	y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("LENGTH : %d",
				 (int)(PreviewCamera.cam_len));

    // DEBUG_Printf( "ZOOM   : %5.2f\n",(float)(PreviewCamera.zoom) );


	y+=COLUMN_HEIGHT*3/2;
	DEBUG_Locate(x,y,MENU_MODE_NORMAL);
	if(PreviewCamera.speed_mode){
		DEBUG_Printf("<High Speed Mode :R2>");
    }
    else{
		DEBUG_Printf("<Low Speed Mode  :R2>");
    }

	y+=COLUMN_HEIGHT;
	DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    switch(PreviewCamera.mode){
    case 0:
		DEBUG_Printf("<Rotate Mode     :L2>" );

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		if(PreviewCamera.mode2){
			DEBUG_Printf("<Center = Camera :R1>" );
		}
		else{
			DEBUG_Printf("<Center = Target :R1>" );
		}

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("<Target Select   :L1>" );
		break;
    case 1:
		DEBUG_Printf("<Camera Move Mode:L2>" );

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		if(PreviewCamera.mode2){
			DEBUG_Printf("<Rotate          :R1>" );
		}
		else{
			DEBUG_Printf("<Shift           :R1>" );
		}
		break;
    case 2:
		DEBUG_Printf("<Set Coord Mode  :L2>" );

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		if(PreviewCamera.mode2){
			DEBUG_Printf("<Rotate          :R1>" );
		}
		else{
			DEBUG_Printf("<Shift           :R1>" );

			y+=COLUMN_HEIGHT;
			DEBUG_Locate(x,y,MENU_MODE_NORMAL);
			DEBUG_Printf("<X Axis          :LR>");
			y+=COLUMN_HEIGHT;
			DEBUG_Locate(x,y,MENU_MODE_NORMAL);
			DEBUG_Printf("<Y Axis          :UD>");
			y+=COLUMN_HEIGHT;
			DEBUG_Locate(x,y,MENU_MODE_NORMAL);
			DEBUG_Printf("<Z Axis          :TRI SQR>");
		}
		break;
    }
}


/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

#if 0

static void Camera_RotationCursole(Work *work,int speed)
{
    FVECTOR	fvec1;

    if(PreviewKey.status & PAD_R1){
		if(PreviewKey.status & PAD_D){
			work->save.cam_rot.vx=( work->save.cam_rot.vx-speed ) & 4095;
		}
		if(PreviewKey.status & PAD_U){
			work->save.cam_rot.vx=(work->save.cam_rot.vx+speed ) & 4095;
		}
		if(PreviewKey.status & PAD_R){
			work->save.cam_rot.vy=(work->save.cam_rot.vy-speed) & 4095;
		}
		if(PreviewKey.status & PAD_L){
			work->save.cam_rot.vy=(work->save.cam_rot.vy+speed) & 4095;
		}
		DG_SetPos2(&work->save.fcam_pos,&work->save.cam_rot);
		fvec1.vx=0.0f;
		fvec1.vy=0.0f;
		fvec1.vz=(float)work->save.cam_length*(-1.0f);
		fvec1.vw=1.0f;
		DG_PutVector( &fvec1, &work->save.ftrg_pos, 1 );
    }
    else{
		if(PreviewKey.status & PAD_U){
			work->save.cam_rot.vx=(work->save.cam_rot.vx-speed) & 4095;
		}
		if(PreviewKey.status & PAD_D){
			work->save.cam_rot.vx=(work->save.cam_rot.vx+speed) & 4095;
		}
		if(PreviewKey.status & PAD_L){
			work->save.cam_rot.vy=(work->save.cam_rot.vy-speed) & 4095;
		}
		if(PreviewKey.status & PAD_R){
			work->save.cam_rot.vy=(work->save.cam_rot.vy+speed) & 4095;
		}
		DG_SetPos2(&work->save.ftrg_pos,&work->save.cam_rot);
		fvec1.vx=0.0f;
		fvec1.vy=0.0f;
		fvec1.vz=(float)work->save.cam_length;
		fvec1.vw=1.0f;
		DG_PutVector(&fvec1,&work->save.fcam_pos,1);
    }
}

static void Camera_ShiftCursole(Work *work,int speed)
{
    FVECTOR	fvec1;

    if(PreviewKey.status & PAD_R1 ){
		if(PreviewKey.auto_status & PAD_U){
			work->save.ftrg_pos.vy=work->save.ftrg_pos.vy+(float)speed;
		}
		if(PreviewKey.auto_status & PAD_D){
			work->save.ftrg_pos.vy=work->save.ftrg_pos.vy-(float)speed;
		}
    }else{
		if(PreviewKey.status & PAD_U){
			work->save.ftrg_pos.vz=work->save.ftrg_pos.vz-(float)speed;
		}
		if(PreviewKey.status & PAD_D){
			work->save.ftrg_pos.vz=work->save.ftrg_pos.vz+(float)speed;
		}
		if(PreviewKey.status & PAD_L){
			work->save.ftrg_pos.vx=work->save.ftrg_pos.vx-(float)speed;
		}
		if(PreviewKey.status & PAD_R){
			work->save.ftrg_pos.vx=work->save.ftrg_pos.vx+(float)speed;
		}
    }
    DG_SetPos2(&work->save.ftrg_pos, &work->save.cam_rot);
    fvec1.vx=0.0f;
    fvec1.vy=0.0f;
    fvec1.vz=(float)work->save.cam_length ;
    fvec1.vw=1.0f;
    DG_PutVector(&fvec1,&work->save.fcam_pos,1);
}

#endif


/* ---------------------------------------------------------------------- */

static void CameraMove(void)
{
	switch(PreviewCamera.mode2){
	case 0:
		{
			float speed;

			if(PreviewCamera.speed_mode){
				speed=50.0f*5.0f;
			}
			else{
				speed=10.0f*5.0f;
			}

			/* shift */
			switch(PreviewKey.status & (PAD_U|PAD_D)){
			case PAD_U:
				PreviewCamera.fcam_pos.vx-=DG_Chanls->eye.m[1][0]*speed;
				PreviewCamera.fcam_pos.vy-=DG_Chanls->eye.m[1][1]*speed;
				PreviewCamera.fcam_pos.vz-=DG_Chanls->eye.m[1][2]*speed;

				PreviewCamera.ftrg_pos.vx-=DG_Chanls->eye.m[1][0]*speed;
				PreviewCamera.ftrg_pos.vy-=DG_Chanls->eye.m[1][1]*speed;
				PreviewCamera.ftrg_pos.vz-=DG_Chanls->eye.m[1][2]*speed;
				break;
			case PAD_D:
				PreviewCamera.fcam_pos.vx+=DG_Chanls->eye.m[1][0]*speed;
				PreviewCamera.fcam_pos.vy+=DG_Chanls->eye.m[1][1]*speed;
				PreviewCamera.fcam_pos.vz+=DG_Chanls->eye.m[1][2]*speed;

				PreviewCamera.ftrg_pos.vx+=DG_Chanls->eye.m[1][0]*speed;
				PreviewCamera.ftrg_pos.vy+=DG_Chanls->eye.m[1][1]*speed;
				PreviewCamera.ftrg_pos.vz+=DG_Chanls->eye.m[1][2]*speed;
				break;
			}
			switch(PreviewKey.status & (PAD_L|PAD_R)){
			case PAD_L:
				PreviewCamera.fcam_pos.vx-=DG_Chanls->eye.m[0][0]*speed;
				PreviewCamera.fcam_pos.vy-=DG_Chanls->eye.m[0][1]*speed;
				PreviewCamera.fcam_pos.vz-=DG_Chanls->eye.m[0][2]*speed;

				PreviewCamera.ftrg_pos.vx-=DG_Chanls->eye.m[0][0]*speed;
				PreviewCamera.ftrg_pos.vy-=DG_Chanls->eye.m[0][1]*speed;
				PreviewCamera.ftrg_pos.vz-=DG_Chanls->eye.m[0][2]*speed;
				break;
			case PAD_R:
				PreviewCamera.fcam_pos.vx+=DG_Chanls->eye.m[0][0]*speed;
				PreviewCamera.fcam_pos.vy+=DG_Chanls->eye.m[0][1]*speed;
				PreviewCamera.fcam_pos.vz+=DG_Chanls->eye.m[0][2]*speed;

				PreviewCamera.ftrg_pos.vx+=DG_Chanls->eye.m[0][0]*speed;
				PreviewCamera.ftrg_pos.vy+=DG_Chanls->eye.m[0][1]*speed;
				PreviewCamera.ftrg_pos.vz+=DG_Chanls->eye.m[0][2]*speed;
				break;
			}
			switch(PreviewKey.status & (PAD_X|PAD_Y)){
			case PAD_X:
				PreviewCamera.fcam_pos.vx+=DG_Chanls->eye.m[2][0]*speed;
				PreviewCamera.fcam_pos.vy+=DG_Chanls->eye.m[2][1]*speed;
				PreviewCamera.fcam_pos.vz+=DG_Chanls->eye.m[2][2]*speed;

				PreviewCamera.ftrg_pos.vx+=DG_Chanls->eye.m[2][0]*speed;
				PreviewCamera.ftrg_pos.vy+=DG_Chanls->eye.m[2][1]*speed;
				PreviewCamera.ftrg_pos.vz+=DG_Chanls->eye.m[2][2]*speed;
				break;
			case PAD_Y:
				PreviewCamera.fcam_pos.vx-=DG_Chanls->eye.m[2][0]*speed;
				PreviewCamera.fcam_pos.vy-=DG_Chanls->eye.m[2][1]*speed;
				PreviewCamera.fcam_pos.vz-=DG_Chanls->eye.m[2][2]*speed;

				PreviewCamera.ftrg_pos.vx-=DG_Chanls->eye.m[2][0]*speed;
				PreviewCamera.ftrg_pos.vy-=DG_Chanls->eye.m[2][1]*speed;
				PreviewCamera.ftrg_pos.vz-=DG_Chanls->eye.m[2][2]*speed;
				break;
			}
		}
		break;

	case 1:
		/* rotate */
		{
			FMATRIX mat;
			FVECTOR vec;
			SVECTOR rot;
			int speed_rot;

			rot=DG_ZeroSVector;

			if(PreviewCamera.speed_mode){
				speed_rot=48;
			}
			else{
				speed_rot=24;
			}

			switch(PreviewKey.status & (PAD_U|PAD_D)){
			case PAD_U:
				rot.vx+=speed_rot;
				break;
			case PAD_D:
				rot.vx-=speed_rot;
				break;
			}
			switch(PreviewKey.status & (PAD_L|PAD_R)){
			case PAD_L:
				rot.vy-=speed_rot;
				break;
			case PAD_R:
				rot.vy+=speed_rot;
				break;
			}

			/* Center = Camera */
			DG_SetPos2(&DG_ZeroVector,&rot);
			DG_GetPos(&mat);

			vec.vx=vec.vy=0.0f;
			vec.vz=PreviewCamera.cam_len;
			vec.vw=1.0f;

			vu0_Ldm1(&mat);
			vu0_Ldm0(&(DG_Chanls->eye));
			vu0_Ldv0(&vec);
			vu0_Mulm2m0m1();
			vu0_Mulv0m2v0();

			vu0_Stv0(&(PreviewCamera.ftrg_pos));
		}
		break;
	}
}

static void CameraSetCoord(void)
{
	switch(PreviewCamera.mode2){
	case 0:
		{
			float speed;

			if(PreviewCamera.speed_mode){
				speed=50.0f*5.0f;
			}
			else{
				speed=10.0f*5.0f;
			}

			/* shift */
			switch(PreviewKey.status & (PAD_U|PAD_D)){
			case PAD_U:
				PreviewCamera.fcam_pos.vy-=speed;
				PreviewCamera.ftrg_pos.vy-=speed;
				break;
			case PAD_D:
				PreviewCamera.fcam_pos.vy+=speed;
				PreviewCamera.ftrg_pos.vy+=speed;
				break;
			}
			switch(PreviewKey.status & (PAD_L|PAD_R)){
			case PAD_L:
				PreviewCamera.fcam_pos.vx-=speed;
				PreviewCamera.ftrg_pos.vx-=speed;
				break;
			case PAD_R:
				PreviewCamera.fcam_pos.vx+=speed;
				PreviewCamera.ftrg_pos.vx+=speed;
				break;
			}
			switch(PreviewKey.status & (PAD_X|PAD_Y)){
			case PAD_X:
				PreviewCamera.fcam_pos.vz+=speed;
				PreviewCamera.ftrg_pos.vz+=speed;
				break;
			case PAD_Y:
				PreviewCamera.fcam_pos.vz-=speed;
				PreviewCamera.ftrg_pos.vz-=speed;
				break;
			}
		}
		break;

	case 1:
		/* rotate */
		{
			FMATRIX mat;
			FVECTOR vec;
			SVECTOR rot;
			int speed_rot;

			rot=DG_ZeroSVector;

			if(PreviewCamera.speed_mode){
				speed_rot=48;
			}
			else{
				speed_rot=24;
			}

			switch(PreviewKey.status & (PAD_U|PAD_D)){
			case PAD_U:
				rot.vx+=speed_rot;
				break;
			case PAD_D:
				rot.vx-=speed_rot;
				break;
			}
			switch(PreviewKey.status & (PAD_L|PAD_R)){
			case PAD_L:
				rot.vy-=speed_rot;
				break;
			case PAD_R:
				rot.vy+=speed_rot;
				break;
			}

			/* Center = Camera */
			DG_SetPos2(&DG_ZeroVector,&rot);
			DG_GetPos(&mat);

			vec.vx=vec.vy=0.0f;
			vec.vz=PreviewCamera.cam_len;
			vec.vw=1.0f;

			vu0_Ldm1(&mat);
			vu0_Ldm0(&(DG_Chanls->eye));
			vu0_Ldv0(&vec);
			vu0_Mulm2m0m1();
			vu0_Mulv0m2v0();

			vu0_Stv0(&(PreviewCamera.ftrg_pos));
		}
		break;
	}
}

static int CameraCalcTargetForHuman(void)
{
	CONTROL *ctrl;
	OBJECT *object;

	ctrl=GM_WhereList[PreviewCamera.tcontrol_index];
	object=ctrl->object;

	if(object==NULL) return 0;

	vu0_Ldv0((FVECTOR *)&(object->objs->world.m[3][0]));
	vu0_Ldv1(&(PreviewCamera.ftrg_pos));
	vu0_Ldv2(&(PreviewCamera.fcam_pos));

	vu0_Subv1v0v1();

	vu0_Stv0(&(PreviewCamera.ftrg_pos));
	vu0_Stv0(&(PreviewCamera.pre_ctrl_mov));

	vu0_Addv1v2();

	vu0_Stv1(&(PreviewCamera.fcam_pos));

	return 1;
}

static int CameraRecalcTargetForHuman(void)
{
	CONTROL *ctrl;
	OBJECT *object;

	ctrl=GM_WhereList[PreviewCamera.tcontrol_index];
	object=ctrl->object;

	if(object==NULL) return 0;

	vu0_Ldv0((FVECTOR *)&(object->objs->world.m[3][0]));
	vu0_Ldv1(&(PreviewCamera.pre_ctrl_mov));
	vu0_Ldv2(&(PreviewCamera.ftrg_pos));

	vu0_Subv1v0v1();

	vu0_Stv0(&(PreviewCamera.pre_ctrl_mov));

	vu0_Addv2v1();

	vu0_Ldv0(&(PreviewCamera.fcam_pos));

	vu0_Stv2(&(PreviewCamera.ftrg_pos));

	vu0_Addv0v1();

	vu0_Stv0(&(PreviewCamera.fcam_pos));

	return 1;
}

static void CameraRotate(void)
{
    FMATRIX mat;
	FVECTOR vec;
    SVECTOR rot;
    int speed_rot;
    float speed;

    rot=DG_ZeroSVector;

    if(PreviewCamera.speed_mode){
		speed_rot=48;
		speed=50.0f*2.0f;
    }
    else{
		speed_rot=24;
		speed=10.0f*2.0f;
    }

	if(PreviewKey.press & PAD_L1){
		while(1){
			PreviewCamera.tcontrol_index++;
			if(PreviewCamera.tcontrol_index>=GM_N_WhereList){
				PreviewCamera.tcontrol_index=0;
			}

			if(CameraCalcTargetForHuman()) break;
		}
	}

	switch(PreviewKey.status & (PAD_U|PAD_D)){
	case PAD_U:
		rot.vx+=speed_rot;
		break;
	case PAD_D:
		rot.vx-=speed_rot;
		break;
	}
	switch(PreviewKey.status & (PAD_L|PAD_R)){
	case PAD_L:
		rot.vy-=speed_rot;
		break;
	case PAD_R:
		rot.vy+=speed_rot;
		break;
	}

	switch(PreviewCamera.mode2){
	case 0:
		/* Move Len */
		switch(PreviewKey.status & (PAD_X|PAD_Y)){
		case PAD_X:
			PreviewCamera.cam_len-=speed;
			if(PreviewCamera.cam_len<100.0f) PreviewCamera.cam_len=100.0f;
			break;
		case PAD_Y:
			PreviewCamera.cam_len+=speed;
			if(PreviewCamera.cam_len>100000.0f) PreviewCamera.cam_len=100000.0f;
			break;
		}

		/* Center = Target */
		rot.vx=-rot.vx;
		rot.vy=-rot.vy;

		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&mat);

		vec.vx=vec.vy=0.0f;
		vec.vz=-PreviewCamera.cam_len;
		vec.vw=0.0f;

		vu0_Ldm1(&mat);
		vu0_Ldm0(&(DG_Chanls->eye));
		vu0_Ldv0(&vec);
		vu0_Ldv1(&(PreviewCamera.ftrg_pos));
		vu0_Mulm2m0m1();
		vu0_Mulv0m2v0();

		vu0_Addv0v1();

		vu0_Stv0(&(PreviewCamera.fcam_pos));
		break;

	case 1:
		/* Center = Camera */
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&mat);

		vec.vx=vec.vy=0.0f;
		vec.vz=PreviewCamera.cam_len;
		vec.vw=0.0f;

		vu0_Ldm1(&mat);
		vu0_Ldm0(&(DG_Chanls->eye));
		vu0_Ldv0(&vec);
		vu0_Ldv1(&(PreviewCamera.fcam_pos));
		vu0_Mulm2m0m1();
		vu0_Mulv0m2v0();

		vu0_Addv0v1();

		vu0_Stv0(&(PreviewCamera.ftrg_pos));
		break;
	}
}

void Camera_DebugCursole(void)
{
	if(PreviewKey.status & PAD_R2){
		/* high speed mode */
		PreviewCamera.speed_mode=1;
	}
	else{
		/* low speed mode */
		PreviewCamera.speed_mode=0;
	}

	/* switch mode */
	if(PreviewKey.press & PAD_L2){
		PreviewCamera.mode++;
		if(PreviewCamera.mode>=3) PreviewCamera.mode=0;
	}

	/* switch mode2 */
	if(PreviewKey.status & PAD_R1){
		PreviewCamera.mode2=1;
	}
	else{
		PreviewCamera.mode2=0;
	}

	switch(PreviewCamera.mode){
	case 0:
		CameraRotate();
		break;
	case 1:
		CameraMove();
		break;
	case 2:
		CameraSetCoord();
		break;
	}

    if(PreviewKey.press & PAD_SEL){
		Preview_SetDefaultCameraData();
    }
}


/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void Preview_InitCamera(void)
{
	extern GM_CameraSet	*NewProgramCamera( int name, int chanl, int level, int prio );
    GM_CameraSet	*cam ;
    int		name ;


	PreviewCamera.mode=0;
	PreviewCamera.mode2=0;
	PreviewCamera.first_in_flag=1;
	PreviewCamera.speed_mode=0;
	PreviewCamera.tcontrol_index=0;

    name = GV_StrCode( "プレビュー環境カメラ" ) ;

    /* 主観よりは弱い */
    PreviewCamera.camera = cam = NewProgramCamera( name, 0, GM_CAMERA_PROG2, 0 ) ;
    GM_SetCameraType( cam, GM_CAM_TYPE_TARGET_AND_ROTATE, CAM_FLAG_FIX ) ;
    GM_SetCameraAngle( cam, 2.0F ) ;
    GM_SetCameraInterpMode( cam, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_NO_SET, 0, 0 ) ;
}

void Preview_CameraAct(void)
{
    if ( GM_Debug2PMode != GM_DEBUG_MODE_PREVIEW ) {
		PreviewCamera.first_in_flag=1;
		PreviewCamera.camera->on = 0 ;
    }
	else{
		/* カメラの起動 */
		PreviewCamera.camera->on=1;

		if(PreviewCamera.first_in_flag==1){		/* 本モードに切り替わった直後 */
			MainMenuParam.font_color=0;

			if(PreviewLight.yajirushi_objs!=NULL){
#if 0
				PreviewLight.yajirushi_objs->flag &= ~DG_FLAG_INVISIBLE ;
#else
				PreviewLight.yajirushi_objs->flag |= DG_FLAG_INVISIBLE ;
#endif
			}
			PreviewCamera.first_in_flag=0;

			/* 切り替わる直前のデータを変換 */
			//GetNowData( work );
			Preview_GetNowCameraData();

			GM_ChangeCamera(0);
		}

		/* ROTATEモードであり、かつプレイヤー以外のキャラを追いかけている場合 */
		if(PreviewCamera.mode==0 && PreviewCamera.tcontrol_index){
			CameraRecalcTargetForHuman();
		}

		/* カメラ */
		/* if(!(GM_PlayerStatus & (PLAYER_WATCH | PLAYER_INTRUDE))) */ {

#if 0
			PreviewCamera.fcam_pos.vw=0.0f;
			PreviewCamera.ftrg_pos.vw=0.0f;

         BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
			DG_SetCamera2(DG_Chanls,&(PreviewCamera.fcam_pos),&(PreviewCamera.ftrg_pos),
						  2.0f*PreviewCamera.zoom);
#endif

			PreviewCamera.camera->position = PreviewCamera.fcam_pos ;
			PreviewCamera.camera->target   = PreviewCamera.ftrg_pos ;
			PreviewCamera.camera->angle    = 2.0f*PreviewCamera.zoom ;
			PreviewCamera.camera->type     = GM_CAM_TYPE_CAMERA_AND_TARGET ;
		}
    }
}

typedef struct {
    GV_ACT_EX	actor;
} Work;

static void Preview_CameraDie(void)
{
    GM_DeleteCamera(PreviewCamera.camera);
}

/* 初期化部メイン */
void *NewPreviewCamera(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
		Preview_InitCamera();
		GV_SetActor(&(work->actor),Preview_CameraAct,Preview_CameraDie);
		GV_ActorEX(&(work->actor));
    }
    return (void *)work ;
}
