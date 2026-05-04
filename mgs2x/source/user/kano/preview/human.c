/*

	human.c
	デザイナープレビュー環境用：人間プレビュー
	1999/07/07 S.Okajima
	2000/02/08 K.Kano Modified
	$Id: human.c,v 1.1.1.3 2002/11/19 11:43:27 Yoshizawa1 Exp $

*/


#include "preview_def.h"


#define OBJECT_FLAG \
	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)


static void Human_MsgToShadow(HumanControl *human)
{
	GV_MSG msg;
	int msg_ary[2];

	msg.address=GV_StrCode("影管理");
	msg.message=msg_ary;
	msg.message_len=sizeof(msg_ary)/sizeof(msg_ary[0]);

	if(human->shadow_enable){
		/* add */
		msg_ary[0]=0;
		msg_ary[1]=human->control.name;
	}
	else{
		/* delete */
		msg_ary[0]=1;
		msg_ary[1]=human->control.name;
	}

	GV_SendMessage(&msg);
}

static void Human_SetDefaultPosition(HumanControl *human,int index)
{
    human->pos.vx=((float)index-(float)(PreviewHuman.humans_size-1)/2.0f)*1000.0f;
    human->pos.vy=0.0f;
    human->pos.vz=0.0f;

    human->rot=DG_ZeroSVector;

    fpu_CopyVector(&(human->control.mov),&(human->pos));
    human->control.turn=human->rot;
    human->control.rot=human->rot;
    fpu_ClearVector(&(human->control.step));
}

static void Humans_SetDefaultPosition(void)
{
    HumanControl *humans=PreviewHuman.humans;
    int size=PreviewHuman.humans_size;
    int	i;

    for(i=0;i<size;i++,humans++) Human_SetDefaultPosition(humans,i);
}

static void Human_SetDefaultData(HumanControl *human,int index)
{
    Human_SetDefaultPosition(human,index);

    human->disp_enable=1;
    human->motion_mode=1;
    human->gravity_flag=0;
    human->pos_fix_flag=1;
    human->interp=40;
	human->boundtest_enable=0;
	human->skeltest_enable=0;

	/* 5/26/2001 追加  Ken Kano */
	human->shadow_enable=0;
	Human_MsgToShadow(human);

    human->change_motion_flag=0;
}


#if 0

void ResetDataHuman( Work *work )
{
    int	i;
    work->motion_num_old=-1;
    work->motion_interp_old=-1;

    for(i=0; i<work->human_num; i++){
		PreviewHuman[i].control.mov.vx=((float)i-(float)(work->human_num-1)/2.0f)*1000.0f;
		PreviewHuman[i].control.mov.vy=0.0f;
		PreviewHuman[i].control.mov.vz=0.0f;
		PreviewHuman[i].control.turn.vx=0;
		PreviewHuman[i].control.turn.vy=0;
		PreviewHuman[i].control.turn.vz=0;
		PreviewHuman[i].control.rot.vx=0;
		PreviewHuman[i].control.rot.vy=0;
		PreviewHuman[i].control.rot.vz=0;
    }
}

#endif


void Preview_SetDefaultHumanData(void)
{
    HumanControl *humans=PreviewHuman.humans;
    int size=PreviewHuman.humans_size;
    int	i;

    for(i=0;i<size;i++,humans++) Human_SetDefaultData(humans,i);
}

void CopyHumanToSavework(void)
{
	HumanControl *humans=PreviewHuman.humans;
	int i;

	memset(SaveData.human,0x00,sizeof(SaveData.human));

	for(i=0;i<PreviewHuman.humans_size && i<HUMAN_SAVE_SIZE;i++,humans++){
		SaveData.human[i].model_strcode=GV_StrCode(PreviewFile.human_filenames[i]);
		SaveData.human[i].motion_strcode=GV_StrCode(PreviewFile.mar_filenames[humans->mar_index]);
		SaveData.human[i].motion_index=humans->motion_index;
		if(humans->face_anim_index!=-1){
			SaveData.human[i].faceanim_strcode
				=GV_StrCode(PreviewFile.faceanim_filenames[humans->face_anim_index]);
		}

		fpu_CopyVector(&(SaveData.human[i].pos),&(humans->pos));
		SaveData.human[i].rot.vx=humans->rot.vx;
		SaveData.human[i].rot.vy=humans->rot.vy;
		SaveData.human[i].rot.vz=humans->rot.vz;
		SaveData.human[i].rot.pad=humans->rot.pad;

		SaveData.human[i].disp_enable=(humans->disp_enable & 1);

		/* 追加  5/14/2001 */
		SaveData.human[i].disp_enable|=((humans->mtn_move_enable & 1)<<1);

		/* 追加  5/26/2001 */
		SaveData.human[i].disp_enable|=((humans->shadow_enable & 1)<<2);

		SaveData.human[i].motion_mode=humans->motion_mode;

		SaveData.human[i].gravity_flag=humans->gravity_flag;

		SaveData.human[i].boundtest_enable=humans->boundtest_enable;

		SaveData.human[i].pos_fix_flag=humans->pos_fix_flag;
		SaveData.human[i].interp=humans->interp;

		SaveData.human[i].skeltest_enable=humans->skeltest_enable;
	}
}

void CopyHumanFromSavework(void)
{
	HumanControl *humans;
	int humans_index=0;
	int i,j;

	for(i=0;i<HUMAN_SAVE_SIZE;i++){
		if(SaveData.human[i].model_strcode==0) break;

		humans=NULL;
		for(j=0;j<PreviewFile.human_filenames_size;j++){
			if(SaveData.human[i].model_strcode==GV_StrCode(PreviewFile.human_filenames[j])){
				humans=PreviewHuman.humans+j;
				humans_index=j;
				break;
			}
		}
		if(humans==NULL) continue;

		for(j=0;j<PreviewFile.mar_filenames_size;j++){
			if(SaveData.human[i].motion_strcode==GV_StrCode(PreviewFile.mar_filenames[j])){
				humans->mar_index=j;
				humans->motion_index=SaveData.human[i].motion_index;
				humans->change_motion_flag=1;
				break;
			}
		}

		if(*(PreviewFile.human_format_flags+humans_index)){
			for(j=0;j<PreviewFile.faceanim_filenames_size;j++){
				if(SaveData.human[i].faceanim_strcode
				   ==GV_StrCode(PreviewFile.faceanim_filenames[j])){

					humans->face_anim_index=j;
					humans->change_face_anim_flag=1;
					break;
				}
			}
		}

		fpu_CopyVector(&(humans->pos),&(SaveData.human[i].pos));
		humans->rot.vx=SaveData.human[i].rot.vx;
		humans->rot.vy=SaveData.human[i].rot.vy;
		humans->rot.vz=SaveData.human[i].rot.vz;
		humans->rot.pad=SaveData.human[i].rot.pad;

		humans->disp_enable=(SaveData.human[i].disp_enable & 1);

		/* 追加  5/14/2001 */
		humans->mtn_move_enable=((SaveData.human[i].disp_enable>>1) & 1);

		/* 追加  5/26/2001 */
		humans->shadow_enable=((SaveData.human[i].disp_enable>>2) & 1);
		Human_MsgToShadow(humans);

		humans->motion_mode=SaveData.human[i].motion_mode;

		humans->gravity_flag=SaveData.human[i].gravity_flag;

		humans->boundtest_enable=SaveData.human[i].boundtest_enable;

		humans->pos_fix_flag=SaveData.human[i].pos_fix_flag;
		humans->interp=SaveData.human[i].interp;

		humans->skeltest_enable=SaveData.human[i].skeltest_enable;
	}
}

/* ---------------------------------------------------------------- */
/* ＭＴＮデータのセットアップ */
void SetMtnMotionData( MTN_DATA *mtn_data, int name_id )
{
    MTN_FILE_HEADER	*mtn_file_header ;
    int				size ;
    mtn_file_header = GV_GetCache( GV_CacheID( name_id, 'r' ) );
    if ( mtn_file_header != NULL ){
		mtn_data->length = mtn_file_header->motion_length ;
		mtn_data->n_joints = mtn_file_header->motion_joints ;
		size = 0 ;
		mtn_data->move = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size = mtn_file_header->move_size ;
		mtn_data->rots = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size += mtn_file_header->rots_size ;
		mtn_data->trans = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size += mtn_file_header->trans_size ;
    }
}

/* EVMオブジェクトにMTNファイルの内容を展開 */
void EvmActMotion(DG_EVMOBJ *evmobj,int first,int n_joints,
				  FVECTOR *skel_trans,FVECTOR *skel_rot,DG_OBJS *objs)
{
    int	i ;
    FMATRIX *skel_mats ;
    FVECTOR vec ;
    FMATRIX mat ;
    EVM_SKEL *skel ;

    skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

    /* オブジェクトにマトリクスを設定する */
    skel = &evmobj->def->skeleton[ first ] ;
    vec.vw = 1.0F ;

    /* モデル情報から親子関係を取得して求める */
    for ( i = first ; i < ( first + n_joints ) ; i++ ){
		FMATRIX	*parent ;

		MT_QuatToMat( &mat, skel_rot );

		mat.m[3][0] = skel->rt_tx + skel_trans->vx ;
		mat.m[3][1] = skel->rt_ty + skel_trans->vy ;
		mat.m[3][2] = skel->rt_tz + skel_trans->vz ;

		if(skel->parent==-1) parent=&(evmobj->world);
		else parent = &skel_mats[ skel->parent ] ;

		_sceVu0MulMatrix( &mat, parent, &mat ) ;
		if(i<objs->n_models){
			fpu_CopyMatrix(&(objs->objs[i].world),&mat);
		}

		vec.vx = -skel->rt_tx ;
		vec.vy = -skel->rt_ty ;
		vec.vz = -skel->rt_tz ;
		_sceVu0ApplyMatrix( &(mat.m[3][0]), &mat, &vec );
		skel_mats[i] = mat ;

		skel_rot++ ;
		skel_trans++ ;
		skel++ ;
    }
}

void EvmActMotionDummy(DG_EVMOBJ *evmobj,int first,FMATRIX *mat)
{
    EVM_SKEL *skel ;
    FMATRIX *skel_mats ;
    int i;

    skel=evmobj->def->skeleton+first;
    skel_mats=evmobj->matrix[evmobj->use_buffer]+first;

    for(i=first;i<evmobj->def->n_x_models;i++,skel++,skel_mats++){
		FVECTOR vec;
		int parent=skel->parent;

		if(parent==-1){
			fpu_CopyUnitMatrix(mat+i);
			continue;
		}

		fpu_CopyMatrix(mat+i,mat+parent);
		fpu_CopyMatrix(skel_mats,mat+i);

		vec.vx=skel->tx;
		vec.vy=skel->ty;
		vec.vz=skel->tz;
		vec.vw=1.0f;
		_sceVu0ApplyMatrix(&vec,mat+i,&vec);
		(mat+i)->m[3][0]=vec.vx;
		(mat+i)->m[3][1]=vec.vy;
		(mat+i)->m[3][2]=vec.vz;

		vec.vx=-skel->rt_tx;
		vec.vy=-skel->rt_ty;
		vec.vz=-skel->rt_tz;
		vec.vw=1.0f;
		_sceVu0ApplyMatrix(&vec,mat+i,&vec);
		skel_mats->m[3][0]=vec.vx;
		skel_mats->m[3][1]=vec.vy;
		skel_mats->m[3][2]=vec.vz;
    }
}

/* ---------------------------------------------------------------- */


/* 初期化 */
#if 0

static	void	InitControl( ctrl,name )
CONTROL	*ctrl ;
int name;
{
#if 1
    GM_InitControl(ctrl,name,0) ;
#else
    GM_InitControl(ctrl,222,0) ;
#endif
    ctrl->hzx_height = 750 ;
    ctrl->height = 1049.0F ;
    GM_ConfigControlHazard( ctrl, 1200, 450, 500 ) ;

    ctrl->seg_flag = 0 ;

    /* 床チェックを省く */
    ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
}

void InitHuman( Work *work )
{
    int	i,j,num,len;
    unsigned char file_name1[16];
    unsigned char file_name2[16];
    unsigned char *cp,*fn;

    sprintf( file_name1, "human" );
    len=BuffLen(file_name1);

    work->human_num=0;
    num=0;
    for(i=work->kms_top; i<work->kms_top+work->kms_num; i++) {
		fn=work->data_filename[i];
		//		printf("check:::%s\n",fn);
		cp=file_name1;
		for( j=0; j<len; j++ ){
			if( *fn != *cp ) break;
			cp++;
			fn++;
		}
		if( j!=len ) continue;
		//		printf("Initialize for HUMAN TYPE:%d:%s\n",num,work->data_filename[i]);


		fn=work->data_filename[i];

		GV_ZeroMemory( file_name2, 16 );
		cp=file_name2;
		while(*fn!='.'){
			*cp=*fn;
			cp++;
			fn++;
		}

		printf("InitHuman : %s\n",file_name2);

		/* コントロールの初期化 */
		InitControl( &PreviewHuman[num].control,GV_StrCode( (char *)file_name2 ) ) ;
		/* 関節型モデルの表示 */
		GM_InitObject( &PreviewHuman[num].body, GV_StrCode( (char *)file_name2 ),OBJECT_FLAG );

		GM_ConfigObjectMotion( &PreviewHuman[num].body, 1, GV_StrCode("motion"), MT_FLAG_HUMAN1 );
		if(PreviewHuman[num].body.m_ctrl->motion_arc==NULL){
			return ;
		}
		GM_ConfigObjectStep( &PreviewHuman[num].body, &PreviewHuman[num].control.step );
		GM_ConfigObjectAction( &PreviewHuman[num].body, 0,       0     , 0, 0xfffff, 0 );

		num++;
		work->human_num=num;
		if( num >= MAX_HUMAN ){
			//			printf("MAX HUMAN BODY NUMBER is %d\n",MAX_HUMAN);
			break;
		}
    }
}

#else


#if 0
#include "../waving_cloth/waving_cloth_called.h"
#endif

#if 0
#include "../raincoat/raincoat_called.h"
#endif


int InitHuman(void)
{
    HumanControl *humans;
    int i;
    int size;
    int motion;
    char *filename;

    size=PreviewFile.human_filenames_size;

    if(PreviewFile.mar_filenames_size<=0) return 0;

    filename=*(PreviewFile.mar_filenames+0);
    motion=GV_StrCode(filename);

    if((PreviewHuman.humans=(HumanControl *)GV_Malloc(sizeof(HumanControl)*size))==NULL) return 0;
    PreviewHuman.humans_size=size;

    GV_ZeroMemory(PreviewHuman.humans,sizeof(HumanControl)*size);

    humans=PreviewHuman.humans;
    for(i=0;i<size;i++,humans++){
		int model;

		filename=*(PreviewFile.human_filenames+i);

		printf("InitHuman : %s\n",filename);

		model=GV_StrCode(filename);

		GM_InitControl(&(humans->control),model,0);
		humans->control.height = 1049.0F ;

		// humans->evm=NULL;
		humans->body.evmobj=NULL;

		if(*(PreviewFile.human_format_flags+i)){
			int dummy_model=GV_StrCode(DUMMY_KMS_FILE);

			GM_InitObject(&(humans->body),dummy_model,OBJECT_FLAG);
			GM_ConfigObjectLight(&(humans->body),humans->lights);

#if 0
			if((def=GV_GetCache(GV_CacheID(model,'e')))!=NULL){
				humans->evm=DG_MakeEvmObj(def,0,0);
				DG_QueueEvmObj(humans->evm);
				DG_InvisibleObjs(humans->body.objs);
				humans->evm->light=humans->lights;
			}
#else
			GM_ConfigObjectEvm(&(humans->body),model,0);
			DG_InvisibleObjs(humans->body.objs);
			humans->body.evmobj->light=humans->lights;
#endif

		}
		else{
			GM_InitObject(&(humans->body),model,OBJECT_FLAG);
			GM_ConfigObjectLight(&(humans->body),humans->lights);
		}

		GM_ConfigControlObject(&(humans->control),&(humans->body));

		GM_ConfigObjectMotion(&(humans->body),1,motion,MT_FLAG_HUMAN1);
		GM_ConfigObjectAction(&(humans->body),0,0,0,0xfffff,0) ;
		humans->mar_index=0;
		humans->motion_index=0;
		humans->motion_max=humans->body.m_ctrl->motion_arc->n_motion;
		humans->face_anim_index=-1;
		humans->face_anim_frame=0;
		humans->mtn_move_enable=0;

		GM_ConfigObjectStep(&(humans->body),&(humans->control.step));
		GM_ConfigControlPosition(&(humans->control),&(humans->pos),&(humans->rot));

		GM_ConfigControlAddressCheck(&(humans->control));
		GM_ConfigControlMapCheck(&(humans->control));

		//humans->control.skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
		humans->control.skip_flag |= CTRL_SKIP_HZX ;

		/* 初期値の設定 */
		Human_SetDefaultData(humans,i);
    }


#if 0 /* TEST */
    {
		int cv2_models[]={
			GV_StrCode("gbs_hand_def"),
			GV_StrCode("gbs_hand_def"),
			GV_StrCode("gbs_hand_def"),
			GV_StrCode("gbs_hand_def"),
			GV_StrCode("gbs_hand_def"),
		};

		humans=PreviewHuman.humans;
		NewHandsChange_called(GV_StrCode("手のテスト"),humans->body.objs,
							  GV_StrCode("gbs_hand_def"),cv2_models,
							  sizeof(cv2_models)/sizeof(cv2_models[0]),
							  GV_StrCode("gbs_hand_def"),cv2_models,
							  sizeof(cv2_models)/sizeof(cv2_models[0]));
    }
#endif


#if 0
    {
		int models[]={
			GV_StrCode("rev_parts_coat"),
		};
		CONTROL *ctrl;
		OBJECT *object;

		ctrl=GM_SearchWhere(GV_StrCode("human5"));
		object=(OBJECT *)(ctrl+1);

		NewWavingClothModelW_called(0,
									models,1,1,
									object,2,NULL,NULL,
									GV_StrCode("rev_coat_bounding"),1.40f,1);
    }
#endif

#if 0
    {
		CONTROL *ctrl;
		OBJECT *object;

		ctrl=GM_SearchWhere(GV_StrCode("human5"));
		object=(OBJECT *)(ctrl+1);

		NewRaincoat_called(object,NULL,0,0,
						   0.0f,0.0f,0.0f,0.0f);
    }
#endif

    return 1;
}

#endif

void ExitHuman(void)
{
    HumanControl *humans;
    int i;
    int size;

    size=PreviewHuman.humans_size;

    humans=PreviewHuman.humans;
    for(i=0;i<size;i++,humans++){
		GM_FreeControl(&(humans->control));
		GM_FreeObject(&(humans->body));

#if 0
		if(humans->evm!=NULL){
			DG_DequeueEvmObj(humans->evm);
			DG_FreeEvmObj(humans->evm);
		}
#endif

    }
    GV_Free(PreviewHuman.humans);
}


/* メイン処理 */

#if 0

void Human_DebugPrint(Work *work)
{
    int	x,y;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( "HUMAN MENU\n" );

    x=LOCATE_X;
    y=LOCATE_Y+9 + work->save.mode_2*9;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( ">\n" );

    x=LOCATE_X+9;
    y=LOCATE_Y+9;
    DEBUG_Locate( x , y, 0 );
    DEBUG_Printf( "MOTION NUM : %3d[%3d]\n",work->save.motion_num,
				  PreviewHuman[0].body.m_ctrl->motion_arc->n_motion-1 );
    if( work->save.pos_fix_flag == 1 ){
		DEBUG_Printf( "POS        : FIX\n" );
    }else{
		DEBUG_Printf( "POS        : FREE\n" );
    }
    DEBUG_Printf( "INTERPORATE:%d\n",work->save.motion_interp );
    if( work->motion_stop_flag == 1 ){
		DEBUG_Printf( "STOP       :\n" );
    }else{
		DEBUG_Printf( "MOVE       :\n" );
    }
    DEBUG_Printf( "TURN       :\n" );
}

void Human_DebugCursole(Work *work)
{
    int i;

    if ( work->pad  & PAD_U ) work->save.mode_2--;
    if ( work->pad  & PAD_D ) work->save.mode_2++;
    if ( work->save.mode_2 >= 5 ) work->save.mode_2 = 0;
    if ( work->save.mode_2 <  0 ) work->save.mode_2 = 4;

    work->motion_stop_flag=0;
    switch(work->save.mode_2){
    case 0:
		if( work->pad & PAD_L ){
			work->save.motion_num--;
		}else if( work->pad & PAD_R ){
			work->save.motion_num++;
		}
		break;
    case 1:
		if( work->pad & PAD_L || work->pad & PAD_R){
			work->save.pos_fix_flag = 1-work->save.pos_fix_flag;
		}
		break;
    case 2:
		if( work->pad & PAD_L ){
			work->save.motion_interp-=10;
			if(work->save.motion_interp<0) work->save.motion_interp=0;
		}else if( work->pad & PAD_R ){
			work->save.motion_interp+=10;
		}
		break;
    case 3:
		if( (work->pad & PAD_L || work->pad & PAD_R) ){
			if(work->key_count>=SPEEDUP_2 && GV_Time%2==0){
				work->motion_stop_flag=1;
			}else{
				work->motion_stop_flag=0;
			}
		}else{
			work->motion_stop_flag=1;
		}
		break;
    case 4:
		if( work->pad & PAD_L ){
			for(i=0; i<work->human_num; i++){
				PreviewHuman[i].control.turn.vy-=256;
			}
		}else if( work->pad & PAD_R ){
			for(i=0; i<work->human_num; i++){
				PreviewHuman[i].control.turn.vy+=256;
			}
		}
		break;
    }

    if( work->pad & PAD_SEL ){
		work->save.motion_num=0;
		work->save.motion_interp=40;
		work->save.pos_fix_flag=1;
		ResetDataHuman( work );
    }

    if ( work->save.motion_num >= PreviewHuman[0].body.m_ctrl->motion_arc->n_motion ){
		work->save.motion_num   = 0;
    }else if ( work->save.motion_num < 0 ){
		work->save.motion_num   = PreviewHuman[0].body.m_ctrl->motion_arc->n_motion - 1;
    }
}

#else


#define HUMAN_PREVIEW_LOCATE_X	(LOCATE_X+0x08*4)
#define HUMAN_PREVIEW_LOCATE_Y	(LOCATE_Y+COLUMN_HEIGHT*2)


static void Human_NormalDebugPrint(HumanControl *human)
{
    char *human_filename=*(PreviewFile.human_filenames+PreviewHuman.humans_index);
    char *mar_filename=*(PreviewFile.mar_filenames+human->mar_index);
    int evm_flag=*(PreviewFile.human_format_flags+PreviewHuman.humans_index);
    int x,y;
    
    x=HUMAN_PREVIEW_LOCATE_X;
    y=HUMAN_PREVIEW_LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Model Name : %s",human_filename);
    if(evm_flag){
		DEBUG_Printf(".evm");
    }
    else{
		DEBUG_Printf(".kms");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("MAR Name   : %s",mar_filename);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Motion No  : %3d [%3d]",human->motion_index,human->motion_max-1);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Face Anim  : ");
    if(human->face_anim_index==-1){
		DEBUG_Printf("--------");
    }
    else{
		char *face_anim_filename=*(PreviewFile.faceanim_filenames+human->face_anim_index);
		DEBUG_Printf("%s",face_anim_filename);
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Mtn Move   : ");
    if(human->face_anim_index==-1){
		DEBUG_Printf("--------");
    }
    else if(human->mtn_move_enable){
		DEBUG_Printf("ON");
	}
	else{
		DEBUG_Printf("OFF");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    switch(human->motion_mode){
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
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(human->pos_fix_flag){
		DEBUG_Printf("POS Mode   : FIX");
    }
    else{
		DEBUG_Printf("POS Mode   : FREE");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    switch(human->gravity_flag){
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
    if(human->boundtest_enable){
		DEBUG_Printf("Bound Test : ON");
    }
    else{
		DEBUG_Printf("Bound Test : OFF");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(human->skeltest_enable){
		DEBUG_Printf("Skel Test  : ON");
    }
    else{
		DEBUG_Printf("Skel Test  : OFF");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(human->shadow_enable){
		DEBUG_Printf("ShadowTest : ON");
    }
    else{
		DEBUG_Printf("ShadowTest : OFF");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Interp     : %d",human->interp);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Restart Motion>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Restart Motion AllMdls>");

	//add shibata
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Restart Motion With Vib>");
    /* カーソル */
    x=LOCATE_X;
    y=HUMAN_PREVIEW_LOCATE_Y+COLUMN_HEIGHT*PreviewHuman.cursole_pos;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf(">>>");
}

static void Human_MoveDebugPrint(HumanControl *human)
{
    char *filename=*(PreviewFile.human_filenames+PreviewHuman.humans_index);
    int evm_flag=*(PreviewFile.human_format_flags+PreviewHuman.humans_index);
    int x,y;

    x=LOCATE_X;
    y=HUMAN_PREVIEW_LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(PreviewHuman.speed_mode){
		DEBUG_Printf("<High Speed Mode  :R2>");
    }
    else{
		DEBUG_Printf("<Low Speed Mode   :R2>");
    }

    x=HUMAN_PREVIEW_LOCATE_X;
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Human Name : %s",filename);
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
    DEBUG_Printf("POS X : %d",(int)(human->pos.vx));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("POS Y : %d",(int)(human->pos.vy));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("POS Z : %d",(int)(human->pos.vz));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT X : %d",human->rot.vx);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT Y : %d",human->rot.vy);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT Z : %d",human->rot.vz);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Reset Pos/Rot>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Reset All Pos/Rot>");


    /* カーソル */
    x=LOCATE_X;
    y=HUMAN_PREVIEW_LOCATE_Y+COLUMN_HEIGHT+PreviewHuman.cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("R1>");
}

void Human_DebugPrint(void)
{
    HumanControl *human=PreviewHuman.humans+PreviewHuman.humans_index;
    int x,y;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("HUMAN MENU");

    DEBUG_Locate(x+0x08*0x0e,y,MENU_MODE_NORMAL);
    if(human->disp_enable) DEBUG_Printf("<Visible   :L1>");
    else DEBUG_Printf("<Invisible :L1>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    switch(PreviewHuman.cursole_mode){
    case 0:
		DEBUG_Printf("<Motion Mode      :L2>");
		Human_NormalDebugPrint(human);
		break;
    case 1:
		DEBUG_Printf("<Move/Rotate Mode :L2>");
		Human_MoveDebugPrint(human);
		break;
    }
}

static void SelectModelfile(void)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		PreviewHuman.humans_index--;
		if(PreviewHuman.humans_index<0) PreviewHuman.humans_index=PreviewHuman.humans_size-1;
		break;
    case PAD_R:
		PreviewHuman.humans_index++;
		if(PreviewHuman.humans_index>=PreviewHuman.humans_size) PreviewHuman.humans_index=0;
		break;
    }
}

static void SelectMarfile(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		human->mar_index--;
		if(human->mar_index<0) human->mar_index=PreviewFile.mar_filenames_size-1;
		human->change_motion_flag=1;
		break;
    case PAD_R:
		human->mar_index++;
		if(human->mar_index>=PreviewFile.mar_filenames_size) human->mar_index=0;
		human->change_motion_flag=1;
		break;
    }
}

static void SelectMotionNumber(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		human->motion_index--;
		if(human->motion_index<0) human->motion_index=human->motion_max-1;
		human->change_motion_flag=1;
		break;
    case PAD_R:
		human->motion_index++;
		if(human->motion_index>=human->motion_max) human->motion_index=0;
		human->change_motion_flag=1;
		break;
    }
}

static void SelectFaceAnimeMotion(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		human->face_anim_index--;
		if(human->face_anim_index<-1) human->face_anim_index=PreviewFile.faceanim_filenames_size-1;
		else if(human->face_anim_index==-1){
			human->change_motion_flag=1;
		}
		human->change_face_anim_flag=1;
		break;
    case PAD_R:
		human->face_anim_index++;
		if(human->face_anim_index>=PreviewFile.faceanim_filenames_size){
			human->face_anim_index=-1;
			human->change_motion_flag=1;
		}
		human->change_face_anim_flag=1;
		break;
    }
}

static void SelectPlayMode(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R|PAD_A)){
    case PAD_L:
		/* -> STOP */
		human->motion_mode=0;
		break;
    case PAD_R:
		/* -> PLAY */
		human->motion_mode=1;
		break;
    case PAD_A:
		if(human->motion_mode==2){
			/* PAUSE -> PLAY ONCE */
			human->motion_mode=3;
		}
		else{
			/* STOP, MOVE -> PAUSE */
			human->motion_mode=2;
		}
		break;
    }
}

static void SelectPosMode(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		human->pos_fix_flag^=1;
		break;
    }
}

static void SelectMtnMove(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		human->mtn_move_enable^=1;
		human->change_face_anim_flag=1;
		break;
    }
}

static void SelectGravityMode(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		human->gravity_flag--;
		if(human->gravity_flag<0) human->gravity_flag=2;
		break;
    case PAD_R:
		human->gravity_flag++;
		if(human->gravity_flag>2) human->gravity_flag=0;
		break;
    }
}

static void SelectBoundTest(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		human->boundtest_enable^=1;
		break;
    }
}

static void SelectSkelTest(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		human->skeltest_enable^=1;
		break;
    }
}

static void SelectShadow(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
    case PAD_R:
		human->shadow_enable^=1;
		Human_MsgToShadow(human);
		break;
    }
}

static void SelectInterporate(HumanControl *human)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		human->interp--;
		if(human->interp<1) human->interp=1;
		human->change_motion_flag=1;
		break;
    case PAD_R:
		human->interp++;
		if(human->interp>999) human->interp=999;
		human->change_motion_flag=1;
		break;
    }
}

static void SwRestartMotion(HumanControl *human)
{
    if(PreviewKey.press & PAD_A){
		human->change_motion_flag=1;
		human->change_face_anim_flag=1;

		/* 追加 2001/5/25  Ken Kano */
		fpu_CopyVector(&(human->control.mov),&(human->pos));
		fpu_ClearVector(&(human->control.step));
    }
}

void HumanRestartAllMotion(void)
{
	HumanControl *humans=PreviewHuman.humans;
	int size=PreviewHuman.humans_size;
	int i;

	for(i=0;i<size;i++,humans++){
		humans->change_motion_flag=1;
		humans->change_face_anim_flag=1;

		/* 追加 2001/5/25  Ken Kano */
		fpu_CopyVector(&(humans->control.mov),&(humans->pos));
		fpu_ClearVector(&(humans->control.step));
	}
}

static void SwRestartAllMotion(void)
{
    if(PreviewKey.press & PAD_A){
		HumanRestartAllMotion();
		FobjRestartAllMotion();
    }
}

// add shibata
static void SwRestartMotionWithVib()
{
	if(PreviewKey.press & PAD_A){
		typedef struct {
			int		n_h_param;
			int		n_l_param;
			short	h_param[255+1];
			short	l_param[255+1];
		} VIB_PACK_WORK;
		extern void *NewPadVibration( char *script, int type );
		extern VIB_PACK_WORK G_With_Motion_Vib;

		NewPadVibration( (unsigned char*)G_With_Motion_Vib.h_param , 1 );
		NewPadVibration( (unsigned char*)G_With_Motion_Vib.l_param , 2 );
			
		printf("restart motion with vib\n");
	}
}

static void SwDefaultPosRot(HumanControl *human,int index)
{
    if(PreviewKey.press & PAD_A){
		Human_SetDefaultPosition(human,index);
    }
}

static void SwDefaultAllPosRot(void)
{
    if(PreviewKey.press & PAD_A){
		Humans_SetDefaultPosition();
    }
}

enum {
    MODELNAME=0,
    MARNAME,
    MOTIONNUM,
    FACEANIM_MTN,
	MTN_MOVE,
    PLAYMODE,
    POSMODE,
    GMODE,
    BOUNDTEST,
    SKELTEST,
	DISPSHADOW,
    INTERP,
    RESTART_MOTION,
    RESTART_ALLMOTION,
    RESTART_MOTION_WITH_VIB,

    NORMALDEBUG_MAX,

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

static void Human_NormalDebugCursole(void)
{
    HumanControl *human=PreviewHuman.humans+PreviewHuman.humans_index;

    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
		PreviewHuman.cursole_pos--;

		if(PreviewHuman.cursole_pos==MTN_MOVE &&
		   !(*(PreviewFile.human_format_flags+PreviewHuman.humans_index))) PreviewHuman.cursole_pos--;

		if(PreviewHuman.cursole_pos==FACEANIM_MTN &&
		   !(*(PreviewFile.human_format_flags+PreviewHuman.humans_index))) PreviewHuman.cursole_pos--;

		if(PreviewHuman.cursole_pos<0) PreviewHuman.cursole_pos=NORMALDEBUG_MAX-1;
		break;
    case PAD_D:
		PreviewHuman.cursole_pos++;

		if(PreviewHuman.cursole_pos==FACEANIM_MTN &&
		   !(*(PreviewFile.human_format_flags+PreviewHuman.humans_index))) PreviewHuman.cursole_pos++;

		if(PreviewHuman.cursole_pos==MTN_MOVE &&
		   !(*(PreviewFile.human_format_flags+PreviewHuman.humans_index))) PreviewHuman.cursole_pos++;

		if(PreviewHuman.cursole_pos>=NORMALDEBUG_MAX) PreviewHuman.cursole_pos=0;
		break;
    }

    switch(PreviewHuman.cursole_pos){
    case MODELNAME: /* model filename */
		SelectModelfile();
		break;
    case MARNAME: /* mar filename */
		SelectMarfile(human);
		break;
    case MOTIONNUM: /* motion number */
		SelectMotionNumber(human);
		break;
    case FACEANIM_MTN: /* face anime */
		SelectFaceAnimeMotion(human);
		break;
	case MTN_MOVE: /* mtn move */
		SelectMtnMove(human);
		break;
    case PLAYMODE: /* play mode */
		SelectPlayMode(human);
		break;
    case POSMODE: /* pos mode */
		SelectPosMode(human);
		break;
    case GMODE: /* gravity mode */
		SelectGravityMode(human);
		break;
    case BOUNDTEST: /* Draw Bounding Box */
		SelectBoundTest(human);
		break;
    case SKELTEST: /* Draw Skeleton */
		SelectSkelTest(human);
		break;
	case DISPSHADOW:
		SelectShadow(human);
		break;
    case INTERP: /* interporate */
		SelectInterporate(human);
		break;
	//add shibata	start
	case RESTART_MOTION_WITH_VIB: /* Restart Motion With Vib */
	    SwRestartMotionWithVib();
	// 				end
    case RESTART_MOTION: /* Restart Motion */
		SwRestartMotion(human);
		break;
    case RESTART_ALLMOTION: /* Restart Motion allmdl */
		SwRestartAllMotion();
		break;
    }
}

static void Human_MoveDebugCursole(void)
{
    HumanControl *human=PreviewHuman.humans+PreviewHuman.humans_index;
    float mv;
    int rv;

    if(PreviewKey.status & PAD_R2){
		PreviewHuman.speed_mode=1;
		mv=100.0f;
		rv=10;
    }
    else{
		PreviewHuman.speed_mode=0;
		mv=10.0f;
		rv=1;
    }

    if(PreviewKey.status & PAD_R1){
		switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
		case PAD_U:
			PreviewHuman.cursole_pos--;
			if(PreviewHuman.cursole_pos<0) PreviewHuman.cursole_pos=MOVEDEBUG_MAX-1;
			break;
		case PAD_D:
			PreviewHuman.cursole_pos++;
			if(PreviewHuman.cursole_pos>=MOVEDEBUG_MAX) PreviewHuman.cursole_pos=0;
			break;
		}
    }

    switch(PreviewHuman.cursole_pos){
    case MODELNAME: /* model filename */
		SelectModelfile();
		break;
    case MOVE_VIEWCORRD: /* Move On ViewCoord */
		if(!(PreviewKey.status & PAD_R1)){
			if(PreviewVMoveXYZ(&(human->pos),mv)){
				human->gravity_flag=0;
				human->pos_fix_flag=1;
			}
		}
		break;

#if 0
    case ROT_VIEWCORRD: /* Rotate On ViewCoord */
		if(!(PreviewKey.status & PAD_R1)){
			if(PreviewVRotXYZ(&(human->body.objs->world),&(human->rot),rv)){
				human->gravity_flag=0;
				human->pos_fix_flag=1;
			}
		}
		break;
#endif

    case POSX: /* POS X */
		if(PreviewMoveX(&(human->pos),mv)){
			human->gravity_flag=0;
			human->pos_fix_flag=1;
		}
		break;
    case POSY: /* POS Y */
		if(PreviewMoveY(&(human->pos),mv)){
			human->gravity_flag=0;
			human->pos_fix_flag=1;
		}
		break;
    case POSZ: /* POS Z */
		if(PreviewMoveZ(&(human->pos),mv)){
			human->gravity_flag=0;
			human->pos_fix_flag=1;
		}
		break;
    case ROTX: /* ROT X */
		if(PreviewRotX(&(human->rot),rv)){
			human->gravity_flag=0;
			human->pos_fix_flag=1;
		}
		break;
    case ROTY: /* ROT Y */
		if(PreviewRotY(&(human->rot),rv)){
			human->gravity_flag=0;
			human->pos_fix_flag=1;
		}
		break;
    case ROTZ: /* ROT Z */
		if(PreviewRotZ(&(human->rot),rv)){
			human->gravity_flag=0;
			human->pos_fix_flag=1;
		}
		break;
    case RESET_POSROT: /* Reset Pos/Rot */
		SwDefaultPosRot(human,PreviewHuman.humans_index);
		break;
    case RESET_ALLPOSROT: /* Reset All Pos/Rot */
		SwDefaultAllPosRot();
		break;
    }
}

void Human_DebugCursole(void)
{
    HumanControl *human=PreviewHuman.humans+PreviewHuman.humans_index;

    switch(PreviewHuman.cursole_mode){
    case 0:
		Human_NormalDebugCursole();
		break;
    case 1:
		Human_MoveDebugCursole();
		break;
    }

    if(PreviewKey.press & PAD_L1){
		human->disp_enable^=1;
    }
    if(PreviewKey.press & PAD_L2){
		PreviewHuman.cursole_mode^=1;
		PreviewHuman.cursole_pos=0;
    }
    if(PreviewKey.press & PAD_SEL){
		Preview_SetDefaultHumanData();
    }
}

#endif


#if 0

void Preview_HumanAct(Work *work)
{
    int i;

    for(i=0; i<work->human_num; i++){
		PreviewHuman[i].body.objs->flag &= ~DG_FLAG_INVISIBLE ;
    }

    if( work->motion_stop_flag!=1 ){
		for(i=0; i<work->human_num; i++){
			DG_COPY_VEC( &work->save.pos_fix_before[i], &PreviewHuman[i].control.mov );
			GM_ActControl( &(PreviewHuman[i].control ) ) ;
			GM_ActObject( &PreviewHuman[i].body );
			DG_GetLightMatrix( &PreviewHuman[i].control.mov, PreviewHuman[i].lights );

			PreviewHuman[i].control.height = PreviewHuman[i].body.height ;
			PreviewHuman[i].control.step.vy -= 96.0f ;

			if ( PreviewHuman[i].control.grounded & 1 ){
				PreviewHuman[i].control.step.vy = 0.0F ;
			}
			if( work->save.pos_fix_flag == 1 ){
				DG_COPY_VEC( &PreviewHuman[i].control.mov, &work->save.pos_fix_before[i] );
			}
		}
    }
    if(work->motion_num_old!=work->save.motion_num
       || work->motion_interp_old!=work->save.motion_interp){

		work->motion_num_old=work->save.motion_num;
		work->motion_interp_old=work->save.motion_interp;
		for(i=0; i<work->human_num; i++){
			GM_ConfigObjectAction( &PreviewHuman[i].body, 0, work->save.motion_num,
								   0, 0xfffff, work->save.motion_interp );
		}
    }
}

#else

static void EvmCalcSkeleton(HumanControl *human)
{
	DG_OBJS *objs;
	DG_EVMOBJ *evm;
    int i;

	objs=human->body.objs;
	evm=human->body.evmobj;

    if(human->face_anim_index>=0){
		FVECTOR *trans,*rots;
		int n_face_skeletons;

		n_face_skeletons
			=evm->def->n_models-objs->def->n_models;

		if((trans=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*human->mtn.n_joints*2))==NULL) goto next;
		rots=trans+human->mtn.n_joints;

		for(i=0;i<human->mtn.n_joints;i++){
			int index=(human->mtn.length+1)*i+human->face_anim_frame+1;
			fpu_CopyVector(rots+i,human->mtn.rots+index);
			fpu_CopyVector(trans+i,human->mtn.trans+index);
		}

		if(n_face_skeletons>=human->mtn.n_joints){
			/* 顔のみ */
			EvmActMotion(evm,objs->def->n_models,human->mtn.n_joints,
						 trans,rots,objs);
		}
		else{
			/* 全身 */

			// printf("n_joint n_x_models = %d %d\n",human->mtn.n_joints,evm->def->n_x_models);

			if(human->mtn.n_joints>evm->def->n_x_models){
				EvmActMotion(evm,0,evm->def->n_x_models,
							 trans,rots,objs);
			}
			else{
				EvmActMotion(evm,0,human->mtn.n_joints,
							 trans,rots,objs);
			}
		}
		GV_Free(trans);

	next:
		human->face_anim_frame++;
		if(human->face_anim_frame>=human->mtn.length) human->face_anim_frame=0;
    }
}

static void CalcEvmMat2ObjMat(HumanControl *human)
{
	DG_OBJS *objs;
	DG_EVMOBJ *evm;
    EVM_SKEL *skel ;
    FMATRIX *skel_mats ;
    int i;
    int n_models;

	objs=human->body.objs;
	evm=human->body.evmobj;

	objs=human->body.objs;
    skel=evm->def->skeleton;

    skel_mats=evm->matrix[evm->use_buffer];
    n_models=human->body.objs->def->n_models;
	if(n_models>evm->def->n_x_models){
		n_models=evm->def->n_x_models;
	}

    fpu_CopyMatrix(&(objs->world),&(evm->world));

    for(i=0;i<n_models;i++,skel++,skel_mats++){
		FVECTOR vec;

		vec.vx=skel->rt_tx;
		vec.vy=skel->rt_ty;
		vec.vz=skel->rt_tz;
		vec.vw=1.0f;

		_sceVu0ApplyMatrix(&vec,skel_mats,&vec);

		objs->objs[i].world.m[3][0]=vec.vx;
		objs->objs[i].world.m[3][1]=vec.vy;
		objs->objs[i].world.m[3][2]=vec.vz;
    }
    for( ;i<human->body.objs->n_models;i++){
		FVECTOR vec;
		int parent=objs->objs[i].parent;

		fpu_CopyMatrix(&(objs->objs[i].world),&(objs->objs[parent].world));

		_sceVu0ApplyMatrix(&vec,&(objs->objs[parent].world),&(objs->objs[i].trans));

		objs->objs[i].world.m[3][0]=vec.vx;
		objs->objs[i].world.m[3][1]=vec.vy;
		objs->objs[i].world.m[3][2]=vec.vz;
	}
}

void Preview_HumanAct(void)
{
    HumanControl *human=PreviewHuman.humans;
    int size=PreviewHuman.humans_size;
    int i;

	GM_CurrentMap=PreviewLight.where;

    for(i=0;i<size;i++,human++){
		// DG_EVMOBJ *evm=human->evm;
		DG_EVMOBJ *evm=human->body.evmobj;

		if(human->disp_enable){
			if(evm!=NULL){
				evm->flag&=~DG_EVMOBJ_INVISIBLE;
			}
			else{
				DG_VisibleObjs(human->body.objs);
			}
		}
		else{
			if(evm!=NULL){
				evm->flag|=DG_EVMOBJ_INVISIBLE;
			}
			else{
				DG_InvisibleObjs(human->body.objs);
			}
		}

		human->control.turn=human->rot;
		human->control.rot=human->rot;

		if(human->change_motion_flag){
			char *mar_filename=*(PreviewFile.mar_filenames+human->mar_index);
			int motion=GV_StrCode(mar_filename);

			human->change_motion_flag=0;

			GM_ConfigObjectMotion(&(human->body),1,motion,MT_FLAG_HUMAN1);

			human->motion_max=human->body.m_ctrl->motion_arc->n_motion;
			if(human->motion_index>=human->motion_max) human->motion_index=human->motion_max-1;

			GM_ConfigObjectAction(&(human->body),0,human->motion_index,
								  0,0xfffff,human->interp);
		}

		if(human->change_face_anim_flag){
			if(human->face_anim_index>=0){
				char *face_anim_filename=*(PreviewFile.faceanim_filenames+human->face_anim_index);
				int motion=GV_StrCode(face_anim_filename);

				SetMtnMotionData(&(human->mtn),motion);
			}
			human->face_anim_frame=0;
			human->change_face_anim_flag=0;
		}

		if(human->gravity_flag==1){
			human->control.skip_flag&=~CTRL_SKIP_FLR_CHECK;
		}
		else{
			human->control.skip_flag|=CTRL_SKIP_FLR_CHECK;
		}

		switch(human->motion_mode){
		case 3:
			human->motion_mode=2;

		case 1:
			{
				OBJECT *object=&(human->body);
				float p_height=object->height;

#if 0
				GM_ActMotion(object);
#else
				/*
				  モーション処理
				  */
				if ( object->m_ctrl != NULL ){
					DG_SetScratchStack( SCRPAD_ADDR + 16*1024-16 );
					MT_ActMotion( object->m_ctrl, object->objs, object->evmobj );
					DG_ResetScratchStack();

#if 0
					object->height = object->m_ctrl->height ;
					if ( object->step != NULL && !(object->flag & OBJECT_MOTIONSTEP_THROUGH) ){
						object->step->vx += object->m_ctrl->step.vx ;
						object->step->vz += object->m_ctrl->step.vz ;
						object->step->vw += object->m_ctrl->step.vw ;
					}
#endif

				}
#endif

				if(evm!=NULL && human->face_anim_index>=0 &&
				   human->mtn_move_enable){
					FVECTOR *move,*move0,*pmove;
					FVECTOR vec;
					MOTION_CONTROL *m_ctrl=object->m_ctrl;

					move0=human->mtn.move+1;
					pmove=human->mtn.move+human->face_anim_frame;
					move=human->mtn.move+human->face_anim_frame+1;

					vec.vx=move->vx-pmove->vx;
					vec.vy=0.0f;
					vec.vz=move->vz-pmove->vz;
					vec.vw=0.0f;

					vu0_Ldv0(&vec);
					if(object->objs->root!=NULL) vu0_Ldm0(object->objs->root);
					else vu0_Ldm0(&(object->objs->world));

					vu0_Mulv0m0v0();

					vu0_Ldv0(&vec);

					m_ctrl->step.vx=vec.vx;
					m_ctrl->step.vy=vec.vy;
					m_ctrl->step.vz=vec.vz;
					/* m_ctrl->step.vw=move->vy-move0->vy; */
					m_ctrl->step.vw=move->vy;

					m_ctrl->root_step.vx=vec.vx;
					m_ctrl->root_step.vy=vec.vy;
					m_ctrl->root_step.vz=vec.vz;
					/* m_ctrl->root_step.vw=move->vy-move0->vy; */
					m_ctrl->root_step.vw=move->vy;

					m_ctrl->height=m_ctrl->step.vw;
					m_ctrl->root_height=m_ctrl->root_step.vw;

					// human->body.height=move->vy;
				}

				if ( object->m_ctrl != NULL ){
					MOTION_CONTROL *m_ctrl=object->m_ctrl;

					object->height=m_ctrl->height;
					if(object->step!=NULL && !(object->flag & OBJECT_MOTIONSTEP_THROUGH)){
						object->step->vx=m_ctrl->step.vx;
						object->step->vz=m_ctrl->step.vz;
						object->step->vw=m_ctrl->step.vw;
					}
				}

				human->control.height=human->body.height;
				if(human->gravity_flag==2) human->control.step.vy=human->body.height-p_height;

				GM_ActControl(&(human->control));
			}

			GM_ActObject2(&(human->body));
			DG_GetLightMatrix(&(human->control.mov),human->lights);

			switch(human->gravity_flag){
			case 0:
				human->control.step.vy=0.0f;
				break;
			case 1:
				human->control.step.vy+=P_GRAVITY;
				if(human->control.grounded & 1){
					human->control.step.vy=0.0F;
				}
				break;
			}

			if(evm!=NULL){
				EvmCalcSkeleton(human);
				CalcEvmMat2ObjMat(human);
			}

			if(human->pos_fix_flag){
				fpu_CopyVector(&(human->control.mov),&(human->pos));
				fpu_ClearVector(&(human->control.step));
			}

			break;
		}

		if(human->boundtest_enable){
			/* バウンディングのデバッグ表示 */
			FVECTOR max, min ;
			DG_MDL *mdl;
			int j;

			mdl=human->body.objs->def->models;
			for(j=0;j<human->body.objs->def->n_models;j++,mdl++){
				void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );

				max.vx=mdl->ux;
				max.vy=mdl->uy;
				max.vz=mdl->uz;
				max.vw=1.0f;

				min.vx=mdl->lx;
				min.vy=mdl->ly;
				min.vz=mdl->lz;
				min.vw=1.0f;

				NewBoundingBoxView_1(&(human->body.objs->objs[j].world),
									 (float *)&max,(float *)&min,0x00ffff00);
			}
		}

		if(human->skeltest_enable){
			if(evm!=NULL) NewEvmSkeletonTest(evm,0x00ffff00);
			else NewKmsSkeletonTest(human->body.objs,0x00ffff00);
		}
    }

#if 0 /* TEST */
    {
		static int count=0;
		int com[]={ 1,-1,60, };

		GV_MSG send={
			GV_StrCode("手のテスト"),
			0,0,
			com,
			3,
		};

		if(count>120){
			GV_SendMessage(&send);
			count=0;
		}
		count++;
    }
#endif

}

typedef struct {
    GV_ACT_EX	actor;
} Work;

static int GetResources(Work *work)
{
    PreviewHuman.cursole_mode=0;
    PreviewHuman.cursole_pos=0;

    return InitHuman();
}

static void NullAct(Work *work)
{
    printf("Abort Preview Human\n");
    GV_DestroyActor(work);
}

static void NullDie(void)
{
}

/* 初期化部メイン */
void *NewPreviewHuman(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
		if(!GetResources(work)) GV_SetActor(&(work->actor),NullAct,NullDie);
		else GV_SetActor(&(work->actor),Preview_HumanAct,ExitHuman);
		GV_ActorEX(&(work->actor));
    }
    return (void *)work ;
}

#endif
