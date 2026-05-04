/*

	fobj.c
	デザイナープレビュー環境用：
	1999/07/07 S.Okajima
	2000/02/08 K.Kano Modified
	$Id: fobj.c,v 1.1.1.3 2002/11/19 11:43:27 Yoshizawa1 Exp $

*/


#include "preview_def.h"


#define OBJECT_FLAG \
	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)


static void Fobj_MsgToShadow(FobjControl *fobj)
{
	GV_MSG msg;
	int msg_ary[2];

	msg.address=GV_StrCode("影管理");
	msg.message=msg_ary;
	msg.message_len=sizeof(msg_ary)/sizeof(msg_ary[0]);

	if(fobj->shadow_enable){
		/* add */
		msg_ary[0]=0;
		msg_ary[1]=fobj->control.name;
	}
	else{
		/* delete */
		msg_ary[0]=1;
		msg_ary[1]=fobj->control.name;
	}

	GV_SendMessage(&msg);
}

static void Fobj_SetDefaultPosition(FobjControl *fobj,int index)
{
    fobj->pos.vx=((float)index-(float)(PreviewFobj.fobjs_size-1)/2.0f)*1000.0f;
    fobj->pos.vy=0.0f;
    fobj->pos.vz=-3000.0f;

    fobj->rot=DG_ZeroSVector;

    fpu_CopyVector(&(fobj->control.mov),&(fobj->pos));
    fobj->control.turn=fobj->rot;
    fobj->control.rot=fobj->rot;
    fpu_ClearVector(&(fobj->control.step));
}

static void Fobjs_SetDefaultPosition(void)
{
    FobjControl *fobjs=PreviewFobj.fobjs;
    int size=PreviewFobj.fobjs_size;
    int	i;

    for(i=0;i<size;i++,fobjs++) Fobj_SetDefaultPosition(fobjs,i);
}

static void Fobj_SetDefaultData(FobjControl *fobj,int index)
{
    Fobj_SetDefaultPosition(fobj,index);

    fobj->disp_enable=1;
    fobj->motion_mode=1;
    fobj->gravity_flag=0;
    fobj->pos_fix_flag=1;
    fobj->interp=40;
	fobj->boundtest_enable=0;
	fobj->skeltest_enable=0;

	/* 5/26/2001 追加  Ken Kano */
	fobj->shadow_enable=0;
	Fobj_MsgToShadow(fobj);

    fobj->change_motion_flag=0;

	fobj->root_mode=0;
	fobj->target_human_index=-1;
	fobj->human_object_index=0;
}

void Preview_SetDefaultFobjData(void)
{
    FobjControl *fobjs=PreviewFobj.fobjs;
    int size=PreviewFobj.fobjs_size;
    int	i;

    for(i=0;i<size;i++,fobjs++) Fobj_SetDefaultData(fobjs,i);
}

void CopyFobjToSavework(void)
{
	FobjControl *fobjs=PreviewFobj.fobjs;
	int i;

	memset(SaveData.fobj,0x00,sizeof(SaveData.fobj));

	for(i=0;i<PreviewFobj.fobjs_size && i<FOBJ_SAVE_SIZE;i++,fobjs++){
		SaveData.fobj[i].model_strcode=GV_StrCode(PreviewFile.fobj_filenames[i]);
		SaveData.fobj[i].motion_index=fobjs->motion_index;
		if(fobjs->rmt_anim_index!=-1){
			SaveData.fobj[i].rmtanim_strcode
				=GV_StrCode(PreviewFile.faceanim_filenames[fobjs->rmt_anim_index]);
		}

		fpu_CopyVector(&(SaveData.fobj[i].pos),&(fobjs->pos));
		SaveData.fobj[i].rot.vx=fobjs->rot.vx;
		SaveData.fobj[i].rot.vy=fobjs->rot.vy;
		SaveData.fobj[i].rot.vz=fobjs->rot.vz;
		SaveData.fobj[i].rot.pad=fobjs->rot.pad;

		SaveData.fobj[i].disp_enable=(fobjs->disp_enable & 1);

		/* 追加  5/26/2001 */
		SaveData.fobj[i].disp_enable|=((fobjs->shadow_enable & 1)<<2);

		SaveData.fobj[i].motion_mode=fobjs->motion_mode;

		SaveData.fobj[i].gravity_flag=fobjs->gravity_flag;

		SaveData.fobj[i].boundtest_enable=fobjs->boundtest_enable;

		SaveData.fobj[i].pos_fix_flag=fobjs->pos_fix_flag;
		SaveData.fobj[i].interp=fobjs->interp;

		SaveData.fobj[i].skeltest_enable=fobjs->skeltest_enable;
	}
}

void CopyFobjFromSavework(void)
{
	FobjControl *fobjs;
	int fobjs_index=0;
	int i,j;

	for(i=0;i<FOBJ_SAVE_SIZE;i++){
		if(SaveData.fobj[i].model_strcode==0) break;

		fobjs=NULL;
		for(j=0;j<PreviewFile.fobj_filenames_size;j++){
			if(SaveData.fobj[i].model_strcode==GV_StrCode(PreviewFile.fobj_filenames[j])){
				fobjs=PreviewFobj.fobjs+j;
				fobjs_index=j;
				break;
			}
		}
		if(fobjs==NULL) continue;

		fobjs->motion_index=SaveData.fobj[i].motion_index;
		fobjs->change_motion_flag=1;

		if(*(PreviewFile.fobj_format_flags+fobjs_index)){
			for(j=0;j<PreviewFile.faceanim_filenames_size;j++){
				if(SaveData.fobj[i].rmtanim_strcode
				   ==GV_StrCode(PreviewFile.faceanim_filenames[j])){

					fobjs->rmt_anim_index=j;
					fobjs->change_rmt_anim_flag=1;
					break;
				}
			}
		}

		fpu_CopyVector(&(fobjs->pos),&(SaveData.fobj[i].pos));
		fobjs->rot.vx=SaveData.fobj[i].rot.vx;
		fobjs->rot.vy=SaveData.fobj[i].rot.vy;
		fobjs->rot.vz=SaveData.fobj[i].rot.vz;
		fobjs->rot.pad=SaveData.fobj[i].rot.pad;

		fobjs->disp_enable=(SaveData.fobj[i].disp_enable & 1);

		/* 追加  5/26/2001 */
		fobjs->shadow_enable=((SaveData.fobj[i].disp_enable>>2) & 1);
		Fobj_MsgToShadow(fobjs);

		fobjs->motion_mode=SaveData.fobj[i].motion_mode;

		fobjs->gravity_flag=SaveData.fobj[i].gravity_flag;

		fobjs->boundtest_enable=SaveData.fobj[i].boundtest_enable;

		fobjs->pos_fix_flag=SaveData.fobj[i].pos_fix_flag;
		fobjs->interp=SaveData.fobj[i].interp;

		fobjs->skeltest_enable=SaveData.fobj[i].skeltest_enable;
	}
}


/* 初期化 */

int InitFobj(void)
{
    FobjControl *fobjs;
    int i;
    int size;
    int motion;
    char *filename;

    size=PreviewFile.fobj_filenames_size;

    if((PreviewFobj.fobjs=(FobjControl *)GV_Malloc(sizeof(FobjControl)*size))==NULL) return 0;
    PreviewFobj.fobjs_size=size;

    GV_ZeroMemory(PreviewFobj.fobjs,sizeof(FobjControl)*size);

    fobjs=PreviewFobj.fobjs;
    for(i=0;i<size;i++,fobjs++){
		int model;

		filename=*(PreviewFile.fobj_filenames+i);

		printf("InitFobj : %s\n",filename);

		model=GV_StrCode(filename);

		if((filename=*(PreviewFile.fobj_mar_filenames+i))==NULL){
			motion=0;
		}
		else{
			motion=GV_StrCode(filename);
		}

		GM_InitControl(&(fobjs->control),model,0);
		fobjs->control.height = 1049.0F ;

		// fobjs->evm=NULL;
		fobjs->body.evmobj=NULL;

		if(*(PreviewFile.fobj_format_flags+i)){
			// EVM_DEF *def ;
			int dummy_model=GV_StrCode(DUMMY_KMS_FILE); /* ダミーデータ発注 */

			GM_InitObject(&(fobjs->body),dummy_model,OBJECT_FLAG);
			GM_ConfigObjectLight(&(fobjs->body),fobjs->lights);

#if 0
			if((def=GV_GetCache(GV_CacheID(model,'e')))!=NULL){
				fobjs->evm=DG_MakeEvmObj(def,0,0);
				DG_QueueEvmObj(fobjs->evm);
				DG_InvisibleObjs(fobjs->body.objs);
				fobjs->evm->light=fobjs->lights;
			}
#else
			GM_ConfigObjectEvm(&(fobjs->body),model,0);
			DG_InvisibleObjs(fobjs->body.objs);
			fobjs->body.evmobj->light=fobjs->lights;
#endif

		}
		else{
			GM_InitObject(&(fobjs->body),model,OBJECT_FLAG);
			GM_ConfigObjectLight(&(fobjs->body),fobjs->lights);
		}

		if(motion!=0){
			GM_ConfigObjectMotion(&(fobjs->body),1,motion,MT_FLAG_HUMAN1);
			GM_ConfigObjectAction(&(fobjs->body),0,0,0,0xfffff,0) ;
			fobjs->motion_max=fobjs->body.m_ctrl->motion_arc->n_motion;
		}
		else{
			fobjs->motion_max=0;
		}
		fobjs->motion_index=0;

#if 1
		fobjs->rmt_anim_index=-1;
		fobjs->rmt_anim_frame=0;
#endif

		GM_ConfigObjectStep(&(fobjs->body),&(fobjs->control.step));
		GM_ConfigControlPosition(&(fobjs->control),&(fobjs->pos),&(fobjs->rot));

		GM_ConfigControlAddressCheck(&(fobjs->control));
		GM_ConfigControlMapCheck(&(fobjs->control));

		//fobjs->control.skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
		fobjs->control.skip_flag |= CTRL_SKIP_HZX ;

		/* 初期値の設定 */
		Fobj_SetDefaultData(fobjs,i);
    }

    return 1;
}


void ExitFobj(void)
{
    FobjControl *fobjs;
    int i;
    int size;

    size=PreviewFobj.fobjs_size;
    fobjs=PreviewFobj.fobjs;

    for(i=0;i<size;i++,fobjs++){
		GM_FreeControl(&(fobjs->control));
		GM_FreeObject(&(fobjs->body));

#if 0
		if(fobjs->evm!=NULL){
			DG_DequeueEvmObj(fobjs->evm);
			DG_FreeEvmObj(fobjs->evm);
		}
#endif

    }
    GV_Free(PreviewFobj.fobjs);
}


/* メイン処理 */

#define FOBJ_PREVIEW_LOCATE_X	(LOCATE_X+0x08*4)
#define FOBJ_PREVIEW_LOCATE_Y	(LOCATE_Y+COLUMN_HEIGHT*2)


static void Fobj_HumanDebugPrint(FobjControl *fobj,int y)
{
    int x;
    
    x=FOBJ_PREVIEW_LOCATE_X;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Root Mode  : HUMAN");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(fobj->pos_fix_flag){
		DEBUG_Printf("POS Mode   : FIX");
    }
    else{
		DEBUG_Printf("POS Mode   : FREE");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    switch(fobj->gravity_flag){
    case 0:
		DEBUG_Printf("G Mode     : FLY");
		break;
    case 1:
		DEBUG_Printf("G Mode     : FALL");
		break;
    case 2:
		DEBUG_Printf("G Mode     : FALL, NO FLOOR");
		break;
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(fobj->boundtest_enable){
		DEBUG_Printf("Bound Test : ON");
    }
    else{
		DEBUG_Printf("Bound Test : OFF");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(fobj->skeltest_enable){
		DEBUG_Printf("Skel Test  : ON");
    }
    else{
		DEBUG_Printf("Skel Test  : OFF");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(fobj->shadow_enable){
		DEBUG_Printf("ShadowTest : ON");
    }
    else{
		DEBUG_Printf("ShadowTest : OFF");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Interp     : %d",fobj->interp);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Restart Motion>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Restart Motion AllMdls>");
}

static void Fobj_ObjectDebugPrint(FobjControl *fobj,int y)
{
    static const char *objname[]={
		"Koshi           ",
		"Onaka           ",
		"Mune            ",
		"Migi   Kata     ",
		"Migi   Ude1     ",
		"Migi   Ude2     ",
		"Migi   Te       ",
		"Hidari Kata     ",
		"Hidari Ude1     ",
		"Hidari Ude2     ",
		"Hidari Te       ",
		"Kubi            ",
		"Atama           ",
		"Migi   Ashi1    ",
		"Migi   Ashi2    ",
		"Migi   Kakato   ",
		"Migi   Tsumasaki",
		"Hidari Ashi1    ",
		"Hidari Ashi2    ",
		"Hidari Kakato   ",
		"Hidari Tsumasaki",
    };
    int x;

    x=FOBJ_PREVIEW_LOCATE_X;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
	DEBUG_Printf("Root Mode  : OBJECT");

    if(fobj->target_human_index>=0){
		char *filename=*(PreviewFile.human_filenames+fobj->target_human_index);
		int index=fobj->human_object_index;

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Human Name : %s",filename);

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		if(index<sizeof(objname)/sizeof(objname[0])){
			DEBUG_Printf("Part Name  : %s (ID=%d)",objname[index],index);
		}
		else{
			DEBUG_Printf("Part Name  : Extend           (ID=%d)",index);
		}
    }
    else{
		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Human Name : No Selected");

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Part Name  : -----------");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Interp     : %d",fobj->interp);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Restart Motion>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Restart Motion AllMdls>");
}


static void Fobj_NormalDebugPrint(FobjControl *fobj)
{
    char *fobj_filename=*(PreviewFile.fobj_filenames+PreviewFobj.fobjs_index);
    char *mar_filename=*(PreviewFile.fobj_mar_filenames+PreviewFobj.fobjs_index);
    int evm_flag=*(PreviewFile.fobj_format_flags+PreviewFobj.fobjs_index);
    int x,y;
    
    x=FOBJ_PREVIEW_LOCATE_X;
    y=FOBJ_PREVIEW_LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Model Name : %s",fobj_filename);
    if(evm_flag){
		DEBUG_Printf(".evm");
    }
    else{
		DEBUG_Printf(".kms");
    }

	if(mar_filename==NULL){
		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("MAR Name   : --------");

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Motion No  :   0 [  0]");
	}
	else{
		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("MAR Name   : %s",mar_filename);

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Motion No  : %3d [%3d]",fobj->motion_index,fobj->motion_max-1);
	}


#if 1
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Mtn Anim   : ");
    if(fobj->rmt_anim_index==-1){
		DEBUG_Printf("--------");
    }
    else{
		char *rmt_anim_filename=*(PreviewFile.faceanim_filenames+fobj->rmt_anim_index);
		DEBUG_Printf("%s",rmt_anim_filename);
    }
#endif


    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    switch(fobj->motion_mode){
    case 0: /* STOP */
		DEBUG_Printf("Play Mode  : STOP");
		break;
    case 1: /* MOVE */
		DEBUG_Printf("Play Mode  : PLAY");
		break;
    case 2: /* PAUSE */
    case 3:
		DEBUG_Printf("Play Mode  : PAUSE");
		break;
    }

    y+=COLUMN_HEIGHT;
	if(fobj->root_mode){
		Fobj_ObjectDebugPrint(fobj,y);
	}
	else{
		Fobj_HumanDebugPrint(fobj,y);
	}

    /* カーソル */
    x=LOCATE_X;
    y=FOBJ_PREVIEW_LOCATE_Y+COLUMN_HEIGHT*PreviewFobj.cursole_pos;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf(">>>");
}

static void Fobj_MoveDebugPrint(FobjControl *fobj)
{
    char *filename=*(PreviewFile.fobj_filenames+PreviewFobj.fobjs_index);
    int evm_flag=*(PreviewFile.fobj_format_flags+PreviewFobj.fobjs_index);
    int x,y;

    x=LOCATE_X;
    y=FOBJ_PREVIEW_LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(PreviewFobj.speed_mode){
		DEBUG_Printf("<High Speed Mode  :R2>");
    }
    else{
		DEBUG_Printf("<Low Speed Mode   :R2>");
    }

    x=FOBJ_PREVIEW_LOCATE_X;
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Fobj Name : %s",filename);
    if(evm_flag){
		DEBUG_Printf(".evm");
    }
    else{
		DEBUG_Printf(".kms");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<MODE: Move On ViewCoord>");

#if 0
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<MODE: Rotate On ViewCoord>");
#endif

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("POS X : %d",(int)(fobj->pos.vx));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("POS Y : %d",(int)(fobj->pos.vy));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("POS Z : %d",(int)(fobj->pos.vz));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT X : %d",fobj->rot.vx);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT Y : %d",fobj->rot.vy);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT Z : %d",fobj->rot.vz);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Reset Pos/Rot>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Reset All Pos/Rot>");


    /* カーソル */
    x=LOCATE_X;
    y=FOBJ_PREVIEW_LOCATE_Y+COLUMN_HEIGHT+PreviewFobj.cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("R1>");
}

void Fobj_DebugPrint(void)
{
    FobjControl *fobj=PreviewFobj.fobjs+PreviewFobj.fobjs_index;
    int x,y;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("FOBJ MENU");

    DEBUG_Locate(x+0x08*0x0e,y,MENU_MODE_NORMAL);
    if(fobj->disp_enable) DEBUG_Printf("<Visible   :L1>");
    else DEBUG_Printf("<Invisible :L1>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    switch(PreviewFobj.cursole_mode){
    case 0:
		DEBUG_Printf("<Motion Mode      :L2>");
		Fobj_NormalDebugPrint(fobj);
		break;
    case 1:
		DEBUG_Printf("<Move/Rotate Mode :L2>");
		Fobj_MoveDebugPrint(fobj);
		break;
    }
}

static void SelectModelfile(void)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		PreviewFobj.fobjs_index--;
		if(PreviewFobj.fobjs_index<0) PreviewFobj.fobjs_index=PreviewFobj.fobjs_size-1;
		break;
    case PAD_R:
		PreviewFobj.fobjs_index++;
		if(PreviewFobj.fobjs_index>=PreviewFobj.fobjs_size) PreviewFobj.fobjs_index=0;
		break;
    }
}

static void SelectMotionNumber(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		fobj->motion_index--;
		if(fobj->motion_index<0) fobj->motion_index=fobj->motion_max-1;
		fobj->change_motion_flag=1;
		break;
    case PAD_R:
		fobj->motion_index++;
		if(fobj->motion_index>=fobj->motion_max) fobj->motion_index=0;
		fobj->change_motion_flag=1;
		break;
    }
}


#if 1

static void SelectRmtAnimeMotion(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		fobj->rmt_anim_index--;
		if(fobj->rmt_anim_index<-1) fobj->rmt_anim_index=PreviewFile.faceanim_filenames_size-1;
		fobj->change_rmt_anim_flag=1;
		break;
    case PAD_R:
		fobj->rmt_anim_index++;
		if(fobj->rmt_anim_index>=PreviewFile.faceanim_filenames_size) fobj->rmt_anim_index=-1;
		fobj->change_rmt_anim_flag=1;
		break;
    }
}

#endif


static void SelectPlayMode(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R|PAD_A)){
    case PAD_L:
		/* -> STOP */
		fobj->motion_mode=0;
		break;
    case PAD_R:
		/* -> PLAY */
		fobj->motion_mode=1;
		break;
    case PAD_A:
		if(fobj->motion_mode==2){
			/* PAUSE -> PLAY ONCE */
			fobj->motion_mode=3;
		}
		else{
			/* STOP, MOVE -> PAUSE */
			fobj->motion_mode=2;
		}
		break;
    }
}

static void SelectPosMode(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		fobj->pos_fix_flag^=1;
		break;
    }
}

static void SelectGravityMode(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		fobj->gravity_flag--;
		if(fobj->gravity_flag<0) fobj->gravity_flag=2;
		break;
    case PAD_R:
		fobj->gravity_flag++;
		if(fobj->gravity_flag>2) fobj->gravity_flag=0;
		break;
    }
}

static void SelectBoundTest(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		if(fobj->body.evmobj==NULL){
			fobj->boundtest_enable^=1;
		}
		break;
    }
}

static void SelectSkelTest(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		fobj->skeltest_enable^=1;
		break;
    }
}

static void SelectShadow(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		fobj->shadow_enable^=1;
		Fobj_MsgToShadow(fobj);
		break;
    }
}

static void SelectRootMode(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		fobj->root_mode^=1;
		break;
    }
}

static void SelectInterporate(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		fobj->interp--;
		if(fobj->interp<1) fobj->interp=1;
		fobj->change_motion_flag=1;
		break;
    case PAD_R:
		fobj->interp++;
		if(fobj->interp>999) fobj->interp=999;
		fobj->change_motion_flag=1;
		break;
    }
}

static void SelectHumanFile(FobjControl *fobj)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		fobj->target_human_index--;
		if(fobj->target_human_index<-1) fobj->target_human_index=PreviewHuman.humans_size-1;
		break;
    case PAD_R:
		fobj->target_human_index++;
		if(fobj->target_human_index>=PreviewHuman.humans_size) fobj->target_human_index=-1;
		break;
    }
    if(fobj->target_human_index>=0){
		HumanControl *human=PreviewHuman.humans+fobj->target_human_index;
		int size=human->body.objs->n_models;

		if(fobj->human_object_index<0) fobj->human_object_index=0;
		else if(fobj->human_object_index>=size) fobj->human_object_index=size-1;
    }
}

static void SelectHumanObject(FobjControl *fobj)
{
    HumanControl *human=PreviewHuman.humans+fobj->target_human_index;
    int size=human->body.objs->n_models;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		fobj->human_object_index--;
		if(fobj->human_object_index<0) fobj->human_object_index=size-1;
		break;
    case PAD_R:
		fobj->human_object_index++;
		if(fobj->human_object_index>=size) fobj->human_object_index=0;
		break;
    }
}

static void SwRestartMotion(FobjControl *fobj)
{
    if(PreviewKey.press & PAD_A){
		fobj->change_motion_flag=1;
		fobj->change_rmt_anim_flag=1;

		/* 追加 2001/5/25  Ken Kano */
		fpu_CopyVector(&(fobj->control.mov),&(fobj->pos));
		fpu_ClearVector(&(fobj->control.step));
    }
}

void FobjRestartAllMotion(void)
{
	FobjControl *fobjs=PreviewFobj.fobjs;
	int size=PreviewFobj.fobjs_size;
	int i;

	for(i=0;i<size;i++,fobjs++){
		fobjs->change_motion_flag=1;
		fobjs->change_rmt_anim_flag=1;

		/* 追加 2001/5/25  Ken Kano */
		fpu_CopyVector(&(fobjs->control.mov),&(fobjs->pos));
		fpu_ClearVector(&(fobjs->control.step));
	}
}

static void SwRestartAllMotion(void)
{
    if(PreviewKey.press & PAD_A){
		HumanRestartAllMotion();
		FobjRestartAllMotion();
    }
}

static void SwDefaultPosRot(FobjControl *fobj,int index)
{
    if(PreviewKey.press & PAD_A){
		Fobj_SetDefaultPosition(fobj,index);
    }
}

static void SwDefaultAllPosRot(void)
{
    if(PreviewKey.press & PAD_A){
		Fobjs_SetDefaultPosition();
    }
}

enum {
    MODELNAME=0,
    MARNAME,
    MOTIONNUM,
    RMTANIM_MTN,
    PLAYMODE,

	ROOTMODE,
    POSMODE,
    GMODE,
    BOUNDTEST,
    SKELTEST,
	DISPSHADOW,
    INTERP,
    RESTART_MOTION,
    RESTART_ALLMOTION,

    HUMANDEBUG_MAX,

	HUMAN_INDEX=ROOTMODE+1,
	PARTS_INDEX,
    INTERP2,
    RESTART_MOTION2,
    RESTART_ALLMOTION2,

	OBJECTDEBUG_MAX,

    MOVE_VIEWCORRD=MODELNAME+1,
    // ROT_VIEWCORRD,
    POSX,
    POSY,
    POSZ,
    ROTX,
    ROTY,
    ROTZ,
    RESET_POSROT,
    RESET_ALLPOSROT,

    MOVEDEBUG_MAX,
};

static void Fobj_NormalDebugCursole(void)
{
    FobjControl *fobj=PreviewFobj.fobjs+PreviewFobj.fobjs_index;
    char *mar_filename=*(PreviewFile.fobj_mar_filenames+PreviewFobj.fobjs_index);

	if(fobj->root_mode){
		/* Object */

		switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
		case PAD_U:
			PreviewFobj.cursole_pos--;

			if(PreviewFobj.cursole_pos==PARTS_INDEX &&
			   fobj->target_human_index<0) PreviewFobj.cursole_pos--;

			if(PreviewFobj.cursole_pos==RMTANIM_MTN &&
			   !(*(PreviewFile.fobj_format_flags+PreviewFobj.fobjs_index))) PreviewFobj.cursole_pos--;

			if(PreviewFobj.cursole_pos==MOTIONNUM &&
			   mar_filename==NULL) PreviewFobj.cursole_pos--;
			if(PreviewFobj.cursole_pos==MARNAME) PreviewFobj.cursole_pos--;

			if(PreviewFobj.cursole_pos<0) PreviewFobj.cursole_pos=OBJECTDEBUG_MAX-1;
			break;
		case PAD_D:
			PreviewFobj.cursole_pos++;

			if(PreviewFobj.cursole_pos==MARNAME) PreviewFobj.cursole_pos++;
			if(PreviewFobj.cursole_pos==MOTIONNUM &&
			   mar_filename==NULL) PreviewFobj.cursole_pos++;

			if(PreviewFobj.cursole_pos==RMTANIM_MTN &&
			   !(*(PreviewFile.fobj_format_flags+PreviewFobj.fobjs_index))) PreviewFobj.cursole_pos++;

			if(PreviewFobj.cursole_pos==PARTS_INDEX &&
			   fobj->target_human_index<0) PreviewFobj.cursole_pos++;

			if(PreviewFobj.cursole_pos>=OBJECTDEBUG_MAX) PreviewFobj.cursole_pos=0;
			break;
		}

		switch(PreviewFobj.cursole_pos){
		case MODELNAME: /* model filename */
			SelectModelfile();
			break;
		case MARNAME: /* mar filename */
			break;
		case MOTIONNUM: /* motion number */
			SelectMotionNumber(fobj);
			break;
#if 1
		case RMTANIM_MTN: /* MTN anime */
			SelectRmtAnimeMotion(fobj);
			break;
#endif

		case PLAYMODE: /* play mode */
			SelectPlayMode(fobj);
			break;
		case ROOTMODE: /* root mode */
			SelectRootMode(fobj);
			break;
		case HUMAN_INDEX: /* target human */
			SelectHumanFile(fobj);
			break;
		case PARTS_INDEX: /* target parts */
			SelectHumanObject(fobj);
			break;
		case INTERP2: /* interporate */
			SelectInterporate(fobj);
			break;
		case RESTART_MOTION2: /* Restart Motion */
			SwRestartMotion(fobj);
			break;
		case RESTART_ALLMOTION2: /* Restart Motion allmdl */
			SwRestartAllMotion();
			break;
		}
	}
	else{
		/* Human */

		switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
		case PAD_U:
			PreviewFobj.cursole_pos--;

			if(PreviewFobj.cursole_pos==RMTANIM_MTN &&
			   !(*(PreviewFile.fobj_format_flags+PreviewFobj.fobjs_index))) PreviewFobj.cursole_pos--;

			if(PreviewFobj.cursole_pos==MOTIONNUM &&
			   mar_filename==NULL) PreviewFobj.cursole_pos--;
			if(PreviewFobj.cursole_pos==MARNAME) PreviewFobj.cursole_pos--;

			if(PreviewFobj.cursole_pos<0) PreviewFobj.cursole_pos=HUMANDEBUG_MAX-1;
			break;
		case PAD_D:
			PreviewFobj.cursole_pos++;

			if(PreviewFobj.cursole_pos==MARNAME) PreviewFobj.cursole_pos++;
			if(PreviewFobj.cursole_pos==MOTIONNUM &&
			   mar_filename==NULL) PreviewFobj.cursole_pos++;

			if(PreviewFobj.cursole_pos==RMTANIM_MTN &&
			   !(*(PreviewFile.fobj_format_flags+PreviewFobj.fobjs_index))) PreviewFobj.cursole_pos++;

			if(PreviewFobj.cursole_pos>=HUMANDEBUG_MAX) PreviewFobj.cursole_pos=0;
			break;
		}

		switch(PreviewFobj.cursole_pos){
		case MODELNAME: /* model filename */
			SelectModelfile();
			break;
		case MARNAME: /* mar filename */
			break;
		case MOTIONNUM: /* motion number */
			SelectMotionNumber(fobj);
			break;
#if 1
		case RMTANIM_MTN: /* MTN anime */
			SelectRmtAnimeMotion(fobj);
			break;
#endif

		case PLAYMODE: /* play mode */
			SelectPlayMode(fobj);
			break;
		case ROOTMODE: /* root mode */
			SelectRootMode(fobj);
			break;
		case POSMODE: /* pos mode */
			SelectPosMode(fobj);
			break;
		case GMODE: /* gravity mode */
			SelectGravityMode(fobj);
			break;
		case BOUNDTEST: /* Draw Bounding Box */
			SelectBoundTest(fobj);
			break;
		case SKELTEST: /* Draw Skeleton */
			SelectSkelTest(fobj);
			break;
		case DISPSHADOW:
			SelectShadow(fobj);
			break;
		case INTERP: /* interporate */
			SelectInterporate(fobj);
			break;
		case RESTART_MOTION: /* Restart Motion */
			SwRestartMotion(fobj);
			break;
		case RESTART_ALLMOTION: /* Restart Motion allmdl */
			SwRestartAllMotion();
			break;
		}
	}
}

static void Fobj_MoveDebugCursole(void)
{
    FobjControl *fobj=PreviewFobj.fobjs+PreviewFobj.fobjs_index;
    float mv;
    int rv;

    if(PreviewKey.status & PAD_R2){
		PreviewFobj.speed_mode=1;
		mv=100.0f;
		rv=10;
    }
    else{
		PreviewFobj.speed_mode=0;
		mv=10.0f;
		rv=1;
    }

    if(PreviewKey.status & PAD_R1){
		switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
		case PAD_U:
			PreviewFobj.cursole_pos--;
			if(PreviewFobj.cursole_pos<0) PreviewFobj.cursole_pos=MOVEDEBUG_MAX-1;
			break;
		case PAD_D:
			PreviewFobj.cursole_pos++;
			if(PreviewFobj.cursole_pos>=MOVEDEBUG_MAX) PreviewFobj.cursole_pos=0;
			break;
		}
    }

    switch(PreviewFobj.cursole_pos){
    case MODELNAME: /* model filename */
		SelectModelfile();
		break;
    case MOVE_VIEWCORRD: /* Move On ViewCoord */
		if(!(PreviewKey.status & PAD_R1)){
			if(PreviewVMoveXYZ(&(fobj->pos),mv)){
				fobj->gravity_flag=0;
				fobj->pos_fix_flag=1;
			}
		}
		break;

#if 0
    case ROT_VIEWCORRD: /* Rotate On ViewCoord */
		if(!(PreviewKey.status & PAD_R1)){
			if(PreviewVRotXYZ(&(fobj->body.objs->world),&(fobj->rot),rv)){
				fobj->gravity_flag=0;
				fobj->pos_fix_flag=1;
			}
		}
		break;
#endif

    case POSX: /* POS X */
		if(PreviewMoveX(&(fobj->pos),mv)){
			fobj->gravity_flag=0;
			fobj->pos_fix_flag=1;
		}
		break;
    case POSY: /* POS Y */
		if(PreviewMoveY(&(fobj->pos),mv)){
			fobj->gravity_flag=0;
			fobj->pos_fix_flag=1;
		}
		break;
    case POSZ: /* POS Z */
		if(PreviewMoveZ(&(fobj->pos),mv)){
			fobj->gravity_flag=0;
			fobj->pos_fix_flag=1;
		}
		break;
    case ROTX: /* ROT X */
		if(PreviewRotX(&(fobj->rot),rv)){
			fobj->gravity_flag=0;
			fobj->pos_fix_flag=1;
		}
		break;
    case ROTY: /* ROT Y */
		if(PreviewRotY(&(fobj->rot),rv)){
			fobj->gravity_flag=0;
			fobj->pos_fix_flag=1;
		}
		break;
    case ROTZ: /* ROT Z */
		if(PreviewRotZ(&(fobj->rot),rv)){
			fobj->gravity_flag=0;
			fobj->pos_fix_flag=1;
		}
		break;
    case RESET_POSROT: /* Reset Pos/Rot */
		SwDefaultPosRot(fobj,PreviewFobj.fobjs_index);
		break;
    case RESET_ALLPOSROT: /* Reset All Pos/Rot */
		SwDefaultAllPosRot();
		break;
    }
}

void Fobj_DebugCursole(void)
{
    FobjControl *fobj=PreviewFobj.fobjs+PreviewFobj.fobjs_index;

    switch(PreviewFobj.cursole_mode){
    case 0:
		Fobj_NormalDebugCursole();
		break;
    case 1:
		Fobj_MoveDebugCursole();
		break;
    }

    if(PreviewKey.press & PAD_L1){
		fobj->disp_enable^=1;
    }
    if(PreviewKey.press & PAD_L2){
		PreviewFobj.cursole_mode^=1;
		PreviewFobj.cursole_pos=0;
    }
    if(PreviewKey.press & PAD_SEL){
		Preview_SetDefaultFobjData();
    }
}


#if 1

static void EvmCalcSkeleton(FobjControl *fobj)
{
	DG_OBJS *objs;
	DG_EVMOBJ *evm;
    // EVM_SKEL *skel ;
    // FMATRIX *skel_mats ;
    // FMATRIX *mat;
    int i;
    // int n_models;

	objs=fobj->body.objs;
	evm=fobj->body.evmobj;

#if 0
    if((mat=(FMATRIX *)GV_Malloc(sizeof(FMATRIX)*fobj->evm->def->n_x_models))==NULL) return;

	objs=fobj->body.objs;
    skel=fobj->evm->def->skeleton;

    fobj->evm->use_buffer=1-fobj->evm->use_buffer;
    skel_mats=fobj->evm->matrix[fobj->evm->use_buffer];
    n_models=objs->def->n_models;
	if(n_models>fobj->evm->def->n_x_models){
		n_models=fobj->evm->def->n_x_models;
	}

    fpu_CopyMatrix(&(fobj->evm->world),&(objs->world));

    for(i=0;i<n_models;i++,skel++,skel_mats++){
		FVECTOR vec;
		int parent=skel->parent;

		fpu_CopyMatrix(mat+i,&(objs->objs[i].world));
		fpu_CopyMatrix(skel_mats,mat+i);

		if(parent!=-1){
			vec.vx=skel->tx;
			vec.vy=skel->ty;
			vec.vz=skel->tz;
			vec.vw=1.0f;
			_sceVu0ApplyMatrix(&vec,mat+parent,&vec);
			(mat+i)->m[3][0]=vec.vx;
			(mat+i)->m[3][1]=vec.vy;
			(mat+i)->m[3][2]=vec.vz;
			fpu_CopyMatrix(&(objs->objs[i].world),mat+i);
		}
		vec.vx=-skel->rt_tx;
		vec.vy=-skel->rt_ty;
		vec.vz=-skel->rt_tz;
		vec.vw=1.0f;
		_sceVu0ApplyMatrix(&vec,mat+i,&vec);
		skel_mats->m[3][0]=vec.vx;
		skel_mats->m[3][1]=vec.vy;
		skel_mats->m[3][2]=vec.vz;
    }

    if(fobj->rmt_anim_index==-1) EvmActMotionDummy(fobj->evm,i,mat);
    else
#endif

	{
		FVECTOR *trans,*rots;
		// FVECTOR *move;
		int n_evm_models=evm->def->n_x_models;

		if((trans=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*fobj->mtn.n_joints*2))==NULL) goto next;
		rots=trans+fobj->mtn.n_joints;

#if 0
		move=fobj->mtn.move+fobj->rmt_anim_frame+1;
#endif

		for(i=0;i<fobj->mtn.n_joints;i++){
			int index=(fobj->mtn.length+1)*i+fobj->rmt_anim_frame+1;
			fpu_CopyVector(rots+i,fobj->mtn.rots+index);
			fpu_CopyVector(trans+i,fobj->mtn.trans+index);
		}

		if(fobj->mtn.n_joints>n_evm_models){
			EvmActMotion(evm,0,n_evm_models,trans,rots,objs);
		}
		else{
			EvmActMotion(evm,0,fobj->mtn.n_joints,trans,rots,objs);
		}

#if 0
		vu0_Ldv0(move+0);
		vu0_Ldv1(move-1);
		vu0_Subv0v1();
		vu0_Stv0(&(fobj->control.step));
#endif

		GV_Free(trans);

	next:
		fobj->rmt_anim_frame++;
		if(fobj->rmt_anim_frame>=fobj->mtn.length) fobj->rmt_anim_frame=0;
    }
    

#if 0
    GV_Free(mat);
#endif

}

#endif


void Preview_FobjAct(void)
{
    FobjControl *fobj=PreviewFobj.fobjs;
    int size=PreviewFobj.fobjs_size;
    int i;

	GM_CurrentMap=PreviewLight.where;

    for(i=0;i<size;i++,fobj++){
		// DG_EVMOBJ *evm=fobj->evm;
		DG_EVMOBJ *evm=fobj->body.evmobj;

		if(fobj->disp_enable){
			if(evm!=NULL){
				evm->flag&=~DG_EVMOBJ_INVISIBLE;
			}
			else{
				DG_VisibleObjs(fobj->body.objs);
			}
		}
		else{
			if(evm!=NULL){
				evm->flag|=DG_EVMOBJ_INVISIBLE;
			}
			else{
				DG_InvisibleObjs(fobj->body.objs);
			}
		}

		fobj->control.turn=fobj->rot;
		fobj->control.rot=fobj->rot;

		if(fobj->change_motion_flag){
			char *mar_filename=*(PreviewFile.fobj_mar_filenames+PreviewFobj.fobjs_index);
			int motion;

			if(mar_filename!=NULL){
				motion=GV_StrCode(mar_filename);

				fobj->change_motion_flag=0;

				GM_ConfigObjectMotion(&(fobj->body),1,motion,MT_FLAG_HUMAN1);

				fobj->motion_max=fobj->body.m_ctrl->motion_arc->n_motion;
				if(fobj->motion_index>=fobj->motion_max) fobj->motion_index=fobj->motion_max-1;

				GM_ConfigObjectAction(&(fobj->body),0,fobj->motion_index,
									  0,0xfffff,fobj->interp);
			}
		}

		if(fobj->change_rmt_anim_flag){
			if(fobj->rmt_anim_index>=0){
				char *rmt_anim_filename=*(PreviewFile.faceanim_filenames+fobj->rmt_anim_index);
				int motion=GV_StrCode(rmt_anim_filename);

				SetMtnMotionData(&(fobj->mtn),motion);
			}
			fobj->rmt_anim_frame=0;
			fobj->change_rmt_anim_flag=0;
		}

		if(fobj->gravity_flag==1){
			fobj->control.skip_flag&=~CTRL_SKIP_FLR_CHECK;
		}
		else{
			fobj->control.skip_flag|=CTRL_SKIP_FLR_CHECK;
		}

		switch(fobj->motion_mode){
		case 3:
			fobj->motion_mode=2;

		case 1:
			{
				float p_height=fobj->body.height;

				GM_ActMotion(&(fobj->body));

				fobj->control.height=fobj->body.height;
				if(fobj->gravity_flag==2) fobj->control.step.vy=fobj->body.height-p_height;

				GM_ActControl(&(fobj->control));
			}

			GM_ActObject2(&(fobj->body));

			switch(fobj->gravity_flag){
			case 0:
				fobj->control.step.vy=0.0f;
				break;
			case 1:
				fobj->control.step.vy+=P_GRAVITY;
				if(fobj->control.grounded & 1){
					fobj->control.step.vy=0.0F;
				}
				break;
			}

#if 1
			if(evm!=NULL) EvmCalcSkeleton(fobj);
#endif

			if(fobj->pos_fix_flag){
				fpu_CopyVector(&(fobj->control.mov),&(fobj->pos));
				fpu_ClearVector(&(fobj->control.step));
			}


			if(fobj->root_mode){
				int index=fobj->target_human_index;
				int humans_size=PreviewHuman.humans_size;

				if(index>=0 && index<humans_size){
					DG_OBJS *humanobj=(PreviewHuman.humans+index)->body.objs;
					int objnum=fobj->human_object_index;

					if(objnum>=0 && objnum<humanobj->n_models){
						FMATRIX inv,*root,*target;
						int i,size;

						root=&(humanobj->objs[objnum].world);
						if(evm!=NULL){
							EVM_SKEL *skel=evm->def->skeleton;

							target=&(evm->world);
							FastInverseMatrix(&inv,target);

							vu0_Ldm1(root);
							vu0_Ldm2(&inv);
							vu0_Mulm0m1m2();

							fpu_CopyMatrix(target,root);
							target=evm->matrix[evm->use_buffer];

							size=evm->n_skeleton;
							for(i=0;i<size;i++,target++,skel++){
								FVECTOR vec;

								vec.vx=skel->rt_tx;
								vec.vy=skel->rt_ty;
								vec.vz=skel->rt_tz;
								vec.vw=1.0f;

								vu0_Ldv0(&vec);
								vu0_Ldm1(target);
								vu0_Mulv1m1v0();

								asm volatile ("vsub.xyz vf1,vf0,vf1");
								asm volatile ("vmove.xyzw vf11,vf2");

								vu0_Mulm2m0m1();
								vu0_Mulv1m2v0();

								vu0_Stm2(target);
								vu0_Stv1((FVECTOR *)&(target->m[3][0]));
							}
						}
						else{
							target=&(fobj->body.objs->world);
							FastInverseMatrix(&inv,target);

							vu0_Ldm1(root);
							vu0_Ldm2(&inv);
							vu0_Mulm0m1m2();

							fpu_CopyMatrix(target,root);

							size=fobj->body.objs->n_models;
							for(i=0;i<size;i++){
								vu0_Ldm1(&(fobj->body.objs->objs[i].world));
								vu0_Mulm2m0m1();
								vu0_Stm2(&(fobj->body.objs->objs[i].world));
							}
						}
					}
				}
			}
			break;
		}

		if(evm!=NULL){
			DG_GetLightMatrix((FVECTOR *)&(evm->world.m[3][0]),fobj->lights);
		}
		else{
			DG_GetLightMatrix((FVECTOR *)&(fobj->body.objs->world.m[3][0]),fobj->lights);
		}

		if(fobj->boundtest_enable){
			/* バウンディングのデバッグ表示 */
			FVECTOR max, min ;
			DG_MDL *mdl;
			int j;

			mdl=fobj->body.objs->def->models;
			for(j=0;j<fobj->body.objs->def->n_models;j++,mdl++){
				void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );

				max.vx=mdl->ux;
				max.vy=mdl->uy;
				max.vz=mdl->uz;
				max.vw=1.0f;

				min.vx=mdl->lx;
				min.vy=mdl->ly;
				min.vz=mdl->lz;
				min.vw=1.0f;

				NewBoundingBoxView_1(&(fobj->body.objs->objs[j].world),
									 (float *)&max,(float *)&min,0x00ffff00);
			}
		}

		if(fobj->skeltest_enable){
			if(evm!=NULL) NewEvmSkeletonTest(evm,0x00ffff00);
			else NewKmsSkeletonTest(fobj->body.objs,0x00ffff00);
		}
    }
}


typedef struct {
    GV_ACT_EX	actor;
} Work;

static int GetResources(Work *work)
{
    PreviewFobj.cursole_mode=0;
    PreviewFobj.cursole_pos=0;

    return InitFobj();
}

static void NullAct(Work *work)
{
    printf("Abort Preview Fobj\n");
    GV_DestroyActor(work);
}

static void NullDie(void)
{
}

/* 初期化部メイン */
void *NewPreviewFobj(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
		if(!GetResources(work)) GV_SetActor(&(work->actor),NullAct,NullDie);
		else GV_SetActor(&(work->actor),Preview_FobjAct,ExitFobj);
		GV_ActorEX(&(work->actor));
    }
    return (void *)work ;
}
