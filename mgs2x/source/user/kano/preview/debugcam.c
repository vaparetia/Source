/*

	debugcam.c
	デザイナープレビュー環境用：カメラ
	1999/07/07 S.Okajima
	$Id: debugcam.c,v 1.1.1.3 2002/11/19 11:43:27 Yoshizawa1 Exp $

*/


#define _WORK_DECLARED_
#include	"preview.h"


extern void SaveScreenShot( unsigned char *filename );


#if 0

static void GetNowData(void)
{
    /* 現行の光情報から */
    Preview_GetNowLightData();

    /* 現行カメラから */
    Preview_GetNowCameraData();
}

#endif


static void SetDefaultData(void)
{
    Preview_SetTexDefaultData();

    Preview_SetDefaultLightData();

    Preview_SetDefaultCameraData();

#if 0
    work->save.motion_num=0;
    work->save.motion_interp=40;
    work->save.pos_fix_flag=1;
#endif

#if 0
    ResetDataHuman( work );
#else
    Preview_SetDefaultHumanData();
    Preview_SetDefaultFobjData();
#endif


#if 0
    ResetDataObjects( work );
#else
    ResetDataObjects();
#endif
}



enum {
	MODE_MAIN=0,
	MODE_FILE,
	MODE_CAMERA,
	MODE_MOTION,
	MODE_OBJECT,

	/* 追加  2000/4/17  Ken Kano */
	MODE_FOBJ,

	MODE_AMB,
	MODE_PAL,
	MODE_FOG,
	MODE_TEX,

	/* 追加  99/12/10  Ken Kano */
	MODE_OBJCHANGE,

	/* 追加  99/12/13  Ken Kano */
	MODE_VA,

	/* 追加  2000/5/16  Ken Kano */
	MODE_PARTICLE,

	/* 追加  2000/6/23  Tsunehiko Shibata */
	MODE_VIB,
	/* 追加  2001/2/21  Tsunehiko Shibata */
	MODE_SKY,

	/* 追加  2001/2/22  Yoshihito Kira    */
	MODE_L2D,

	MODE_MAX
};


#if 0

static void MainMenu( Work *work )
{
    int x,y;

    x=LOCATE_X+9;
    y=LOCATE_Y;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( "FILE\n"     );
    if( ( GM_Debug2PMode == GM_DEBUG_MODE_PREVIEW ) && !(GM_PlayerStatus & ( PLAYER_WATCH | PLAYER_INTRUDE) ) ){
		DEBUG_Printf( "CAMERA\n"   );
    }else{
		DEBUG_Printf( "--- PLAYER VIEW ---\n"   );
    }
    DEBUG_Printf( "MOTION\n"   );
    DEBUG_Printf( "OBJECT\n"   );
    DEBUG_Printf( "AMBIENT\n"  );
    DEBUG_Printf( "PARALLEL\n" );
    DEBUG_Printf( "FOG\n"      );
    DEBUG_Printf( "TEX\n"      );
    DEBUG_Printf( "OBJ CHANGE\n");
    DEBUG_Printf( "VERTEX ANIME\n");


    if ( work->pad  & PAD_U ){
		work->save.mode_1--;
    }
    if ( work->pad  & PAD_D ){
		work->save.mode_1++;
    }
    if( work->save.mode_1 >= MODE_MAX ) work->save.mode_1 = 1;
    if( work->save.mode_1 <  1        ) work->save.mode_1 = MODE_MAX-1;
    
    if( !( GM_Debug2PMode == GM_DEBUG_MODE_CAMERA_SET ) ){
		x=LOCATE_X;
		y=LOCATE_Y + (work->save.mode_1-1)*9;
		DEBUG_Locate( x,   y, 0 );
		DEBUG_Printf( ">\n" );
    }

    if ( (GV_PadData[1].press & PAD_A)
		 && (MainMenuParam.font_color==0 || MainMenuParam.font_color==1) ){
		work->save.mode_0 = work->save.mode_1;
		work->save.mode_1 = 0;
		PreviewCamera.cam_disp_flag=1;
    }
    if ( GV_PadData[1].press & PAD_B ){
		MainMenuParam.font_color++;
    }
}

#else

static void Main_DebugCursole(void)
{
    if( PreviewKey.auto_status & PAD_U ){
		MainMenuParam.cursole--;
    }
    if( PreviewKey.auto_status & PAD_D ){
		MainMenuParam.cursole++;
    }
    if( MainMenuParam.cursole >= MODE_MAX ) MainMenuParam.cursole = 1;
    if( MainMenuParam.cursole <  1        ) MainMenuParam.cursole = MODE_MAX-1;
    
    if ( (PreviewKey.press & PAD_A)
		 && (MainMenuParam.font_color==0 || MainMenuParam.font_color==1) ){

		MainMenuParam.mode=MainMenuParam.cursole;
		PreviewCamera.cam_disp_flag=1;
    }

#if 1
    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.font_color++;
    }
#endif

}

static void Main_DebugPrint(void)
{
    int x,y;

    x=LOCATE_X+9;
    y=LOCATE_Y;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( "FILE\n"     );

    if( ( GM_Debug2PMode == GM_DEBUG_MODE_PREVIEW ) &&
		!(GM_PlayerStatus & ( PLAYER_WATCH | PLAYER_INTRUDE) ) ){

		DEBUG_Printf( "CAMERA\n"   );
    }else{
		DEBUG_Printf( "--- PLAYER VIEW ---\n"   );
    }
    DEBUG_Printf( "HUMAN\n" );
    DEBUG_Printf( "OBJECT\n" );
    DEBUG_Printf( "FOBJ\n" );
    DEBUG_Printf( "AMBIENT\n" );
    DEBUG_Printf( "PARALLEL\n" );
    DEBUG_Printf( "FOG\n" );
    DEBUG_Printf( "TEX\n" );
    DEBUG_Printf( "OBJ CHANGE\n" );
    DEBUG_Printf( "VERTEX ANIME\n" );
    DEBUG_Printf( "PARTICLE\n" );
    DEBUG_Printf( "VIBRATION\n" );
    DEBUG_Printf( "SKY\n" );
    DEBUG_Printf( "2D LAYOUT\n" );

    /* カーソル */
    x=LOCATE_X;
    y=LOCATE_Y + (MainMenuParam.cursole-1)*FONT_HEIGHT;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( ">\n" );
}

static void MainMenu(void)
{
    Main_DebugPrint();
    Main_DebugCursole();
}

#endif


static void SelectFontColor(void)
{
    if(PreviewKey.press & PAD_AR){
		MainMenuParam.font_color++;
    }
}

static void FileMenu(void)
{
    File_DebugPrint();
    File_DebugCursole();

    if(PreviewKey.press & PAD_B){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void CameraMenu(void)
{
    Camera_DebugPrint();
    Camera_DebugCursole();

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void HumanMenu(void)
{
    int index=PreviewHuman.humans_index;
    int size=PreviewHuman.humans_size;

    if(index<size){

#if 0
		Human_DebugPrint(work);
		Human_DebugCursole(work);
#else
		Human_DebugPrint();
		Human_DebugCursole();
#endif

    }
    else{
		PreviewHuman.humans_index=0;
    }

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void ObjMenu(void)
{
    int index=PreviewObject.objects_index;
    int size=PreviewObject.objects_size;

    if(index<size){
		Object_DebugPrint();
		Object_DebugCursole();
    }
    else{
		PreviewObject.objects_index=0;
    }

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void FobjMenu(void)
{
    int index=PreviewFobj.fobjs_index;
    int size=PreviewFobj.fobjs_size;

    if(index<size){
		Fobj_DebugPrint();
		Fobj_DebugCursole();
    }
    else{
		PreviewFobj.fobjs_index=0;
    }

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void AmbMenu(void)
{
    Ambient_DebugPrint();
    Ambient_DebugCursole();

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void PllMenu(void)
{
    Parallel_DebugPrint();
    Parallel_DebugCursole();

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void FogMenu(void)
{
    Fog_DebugPrint();
    Fog_DebugCursole();

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void TexMenu(void)
{
    Tex_DebugPrint();
    Tex_DebugCursole();

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void ObjchangeMenu(void)
{
    int index=PreviewHumanOC.objchange_index;
    int size=PreviewHumanOC.objchange_size;

    if(index<size){
		HumanOC2_DebugPrint();
		HumanOC2_DebugCursole();
    }
    else index=0;

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
    else if ( PreviewKey.press & PAD_SEL ){
		index++;
		if(index>=size) index=0;
    }
    PreviewHumanOC.objchange_index=index;
}

static void VAnimeMenu(void)
{
    int index=PreviewVA.va_index;
    int size=PreviewVA.va_size;

    if(index<size){
		VAPreview_DebugPrint();
		VAPreview_DebugCursole();
    }
    else index=0;

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
    else if ( PreviewKey.press & PAD_SEL ){
		index++;
		if(index>=size) index=0;
    }
    PreviewVA.va_index=index;
}

static void ParticleMenu(void)
{
	Particle_DebugPrint();
	Particle_DebugCursole();

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}


static void VibrationMenu(void)
{
	if( Vibration_Control() ) MainMenuParam.mode=MODE_MAIN;
	//Vibration_DebugPrint();
	//Vibration_DebugCursole();

//    if ( PreviewKey.press & PAD_B ){
//		MainMenuParam.mode=MODE_MAIN;
 //   }
}

static void PrevSkyMenu(void)
{
	PrevSkyPrint();
	PrevSkyControl();

    if ( PreviewKey.press & PAD_B ){
		MainMenuParam.mode=MODE_MAIN;
    }
}

static void LayoutMenu(void)
{
  if(Layout_Control()) MainMenuParam.mode = MODE_MAIN;
}

static void Menus(void)
{
    DEBUG_Locate(LOCATE_X,LOCATE_Y-0x10,MENU_MODE_NORMAL);
    DEBUG_Printf("Design Previewer for PS2");

	SelectFontColor();

    switch(MainMenuParam.mode){
    case MODE_MAIN:
		MainMenu();
		ForVRSlight();
		break;
    case MODE_FILE:
		FileMenu();
		break;
    case MODE_CAMERA:
		if((GM_Debug2PMode == GM_DEBUG_MODE_PREVIEW )
		   && !(GM_PlayerStatus & ( PLAYER_WATCH | PLAYER_INTRUDE) ) ){

			CameraMenu();
		}
		else{
			MainMenuParam.mode=MODE_MAIN;
		}
		break;
    case MODE_MOTION:
		HumanMenu();
		break;
    case MODE_OBJECT:
		ObjMenu();
		break;
    case MODE_FOBJ:
		FobjMenu();
		break;
    case MODE_AMB:
		AmbMenu();
		break;
    case MODE_PAL:
		PllMenu();
		break;
    case MODE_FOG:
		FogMenu();
		break;
    case MODE_TEX:
		if(PreviewTex.prim){
			DG_VisiblePrim( (DG_PRIM *)(PreviewTex.prim) ) ;
			TexMenu();
		}
		break;
    case MODE_OBJCHANGE:
		ObjchangeMenu();
		break;
    case MODE_VA:
		VAnimeMenu();
		break;
	case MODE_PARTICLE:
		ParticleMenu();
		break;
	case MODE_VIB:
		VibrationMenu();
		break;
	case MODE_SKY:
	    PrevSkyMenu();
		break;
    case MODE_L2D:
      LayoutMenu();
    default:
		break;
    }
}

static void Act( Work *work )
{

#if 0
    work->pad=0;
    if( !( GM_Debug2PMode == GM_DEBUG_MODE_CAMERA_SET ) ){
		if ( GV_PadData[1].status ){
			if(work->key_count==0){
				work->pad=GV_PadData[1].status;
			}else if(work->key_count>=SPEEDUP_2){
				work->pad=GV_PadData[1].status;
			}else if(work->key_count>=SPEEDUP_1){
				if(work->key_count%4==0){
					work->pad=GV_PadData[1].status;
				}
			}
			work->key_count++;
		}else{
			work->key_count=0;
		}
    }
#endif

    if(!(GM_Debug2PMode==GM_DEBUG_MODE_CAMERA_SET)) PreKeyAct();

    //	DEBUG_Locate( 10, 10+9*15, 0 );
    //	DEBUG_Printf( "%d %d %d\n",work->save.mode_0,work->save.mode_1,work->save.mode_2 );

    if ( PreviewKey.status & PAD_L2
		 && PreviewKey.press & PAD_L1 ){

		void DG_SaveScreen( char *filename );

//		SaveScreenShot( "host0:./screen_shot.raw" );
		DG_SaveScreen( "host0:./screen_shot.raw" );
    }


    // Preview_CameraAct();

    switch(MainMenuParam.font_color){
    case 0:
		DEBUG_Color( 255, 255, 255, 255 );
		break;
    case 1:
		DEBUG_Color( 0, 0, 0, 255 );
		break;
    case 2:
		if(PreviewLight.yajirushi_objs!=NULL){
			PreviewLight.yajirushi_objs->flag |= DG_FLAG_INVISIBLE ;
		}
		DEBUG_Color( 1,1,1,1 );
		break;
    default:
		MainMenuParam.font_color=0;
		break;
    }
    //printf("%d\n",MainMenuParam.font_color);


    if(PreviewTex.prim) DG_InvisiblePrim( (DG_PRIM *)(PreviewTex.prim) ) ;

    if( !( GM_Debug2PMode == GM_DEBUG_MODE_CAMERA_SET ) ){
		Menus();
    }

    //Preview_HumanAct();

    //Preview_ObjectAct();

    //Preview_LightAct();
}

static void Die(Work *work)
{
	if(work->camera_work!=NULL) GV_DestroyActor(work->camera_work);
	if(work->light_work!=NULL) GV_DestroyActor(work->light_work);
	if(work->particle_work!=NULL) GV_DestroyActor(work->light_work);

	if(work->human_work!=NULL) GV_DestroyActor(work->human_work);
    if(work->object_work!=NULL) GV_DestroyActor(work->object_work);
    if(work->fobj_work!=NULL) GV_DestroyActor(work->fobj_work);
    if(work->vib_work!=NULL) GV_DestroyActor(work->vib_work);
    if(work->sky_work!=NULL) GV_DestroyActor(work->sky_work);
    if(work->layout_work != NULL) GV_DestroyActor(work->layout_work);

	Preview_ExitTex();
    FreePreviewFile();

    //ExitHuman();
    //ExitObject();
}


static int GetResources( Work *work )
{
	// Preview_InitCamera();

#if 0

    /* data.cnf から読み込み */
    work->max_num_files=LoadDataConf( work );
    //	printf("max_num_files:%d\n",work->max_num_files);
    if( work->max_num_files <= 0 ){
		printf("ERR::::");
		return -1;
    }

    /* ＴＲＩファイルネーム読み込み */
    NumSuffix( work, "tri", &work->tri_top, &work->tri_num );
    //	printf("%d %d \n",work->tri_top,work->tri_num);

	/* ＫＭＳファイルネーム読み込み */
    NumSuffix( work, "kms", &work->kms_top, &work->kms_num );
    //	printf("%d %d \n",work->kms_top,work->kms_num);

	/* ＣＶ２ファイルネーム読み込み */
    NumSuffix( work, "cv2", &work->cv2_top, &work->cv2_num );

    /* ファイルネーム"human*.kms"は人型で初期化 */
    work->human_num=0;
    InitHuman( work );

	/* ファイルネーム"human*.kms","objch*.kms"以外の"*.kms"をオブジェクツ（物）として初期化 */
    InitObjects( work );

    /* ファイルネーム"objch*.kms"は、人型でオブジェすり替え用として初期化 */
    GetObjchangeKMS( work );

    /* ファイルネーム"vanim*.cv2"は、頂点アニメ用として初期化 */
    GetVertexAnimationCV2( work );

#else

    if(!LoadDataConf()){
		printf("ERR::::");
		return -1;
    }

    //InitHuman();
    //InitObjects();

#endif

#if 0
	/* サウンドの仮ロードルーチン */
	KariLoad();
#endif

	MainMenuParam.cursole=MODE_FILE;

	if((work->camera_work=NewPreviewCamera())==NULL) return -1;
	if((work->light_work=NewPreviewLight())==NULL) return -1;
	if((work->particle_work=NewPreviewParticle())==NULL) return -1;

	if((work->human_work=NewPreviewHuman())==NULL) return -1;
	if((work->object_work=NewPreviewObject())==NULL) return -1;
	if((work->fobj_work=NewPreviewFobj())==NULL) return -1;

	if((work->vib_work=NewVibrationEditorSet())==NULL) return -1;
	if((work->sky_work=NewPrevSky())==NULL) return -1;
	if((work->layout_work = NewLayoutPreview()) == NULL) return -1;
	
	if(!Preview_InitTex()) return -1;

#if 0
	SetStageLight();
#endif

	SetDefaultData();

#if 0
	{
		void *NewMemTest(void);

		// NewMemTest();
	}

	{
		CONTROL *ctrl=GM_SearchWhere(GV_StrCode("スネーク"));
		OBJECT *object=(OBJECT *)(ctrl+1);

		NewToolParticle_called(GV_ACTOR_AFTER,GV_StrCode("jet"),NULL,NULL,1,&(object->objs->world));
	}
#endif

    return 0;
}


#include "../particle/partsub2.h"


void *NewDebugCam(int name,int where)
{
    Work		*work ;

    printf("NewDebugCam::::::::::::::::\n");

    OPERATOR() ;

	printf("where = 0x%08x\n",where);

	memset(&MainMenuParam,0x00,sizeof(MainMenuParam));
	memset(&PreviewFile,0x00,sizeof(PreviewFile));
	memset(&PreviewLight,0x00,sizeof(PreviewLight));
	memset(&PreviewCamera,0x00,sizeof(PreviewCamera));
	memset(&PreviewHuman,0x00,sizeof(PreviewHuman));
	memset(&PreviewObject,0x00,sizeof(PreviewObject));
	memset(&PreviewFobj,0x00,sizeof(PreviewFobj));
	memset(&PreviewTex,0x00,sizeof(PreviewTex));
	memset(&PreviewHumanOC,0x00,sizeof(PreviewHumanOC));
	memset(&PreviewVA,0x00,sizeof(PreviewVA));
	memset(&PreviewParticle,0x00,sizeof(PreviewParticle));

	PreviewLight.where=where;

	PreKeyInit();

    work = (Work *)GV_NewActor( GV_ACTOR_PREV, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }

    return (void *)work ;
}
