/*

	object.c
	デザイナープレビュー環境用：オブジェクトプレビュー
	1999/07/07 S.Okajima
	2000/02/09 K.Kano Modified
	$Id: object.c,v 1.1.1.3 2002/11/19 11:43:28 Yoshizawa1 Exp $

*/


#include "preview_def.h"


static FVECTOR *Object_SetPosition(ObjectControl *object)
{
    FVECTOR *pos;

    if(object->target_human_index>=0){
		int index=object->target_human_index;
		int humans_size=PreviewHuman.humans_size;

		if(index<humans_size){
			DG_OBJS *humanobj=(PreviewHuman.humans+index)->body.objs;
			int objnum=object->human_object_index;

			if(objnum>=0 && objnum<humanobj->n_models){
				object->objs->root=&(humanobj->objs[objnum].world);
				pos=(FVECTOR *)&(humanobj->objs[objnum].world.m[3][0]);

				return pos;
			}
		}

		object->target_human_index=-1;
    }
    pos=&(object->pos);
    object->objs->root=NULL;

    DG_SetPos2(&(object->pos),&(object->rot));
    DG_PutObjs(object->objs);

    return pos;
}

static void Object_SetQueue(ObjectControl *object)
{
    if(object->shade_mode){
		char *filename=*(PreviewFile.stagelight_filenames+PreviewLight.lt2_select);

		/* Pre Shade */
		object->objs=DG_MakeObjs(object->def,FLAG_PRE_SHADE,0);
		if(object->objs==NULL){
			printf("ERR MakeObjs\n");
			ASSERT(0);
		}
		DG_QueueObjs(object->objs);
		Object_SetPosition(object);
		DG_MakePreshade(object->objs,
						(LIT_DEF*)GV_GetCache(GV_CacheID(GV_StrCode(filename),'l')));
    }
    else{
		/* Normal Shade */
		object->objs=DG_MakeObjs(object->def,FLAG_NORMAL_SHADE,0);
		if(object->objs==NULL){
			printf("ERR MakeObjs\n");
			ASSERT(0);
		}
		DG_QueueObjs(object->objs);
		DG_SetLightMatrix(object->objs,object->lights);
    }
}

static void Object_SetDeafultPosition(ObjectControl *objects)
{
    objects->pos=DG_ZeroVector;
    objects->rot=DG_ZeroSVector;
}

static void Object_SetDefaultData(ObjectControl *objects)
{
    Object_SetDeafultPosition(objects);

    objects->disp_enable=0;
    objects->shade_mode=0;
    objects->target_human_index=-1;
    objects->human_object_index=-1;
}

void Objects_SetDefaultData(void)
{
    ObjectControl *object=PreviewObject.objects;
    int size=PreviewObject.objects_size;
    int i;

    for(i=0;i<size;i++,object++){
		Object_SetDefaultData(object);
    }
}


#if 0

void ResetDataObjects( Work *work )
{
    int	i;
    for(i=0; i<work->objects_num; i++){

		// printf("Reset Object : %d\n",i);

		//		DG_SetPos2( &work->save.objects_mov[i], &work->save.objects_rot[i] ) ;
		//		DG_PutObjs( work->objects[i] );
		//		DG_GetLightMatrix( (FVECTOR*)work->objects[i]->world.m[3], work->objects_lights[i] );


		if( work->save.objects_parm1[i]==1 ){
			if( work->save.objects_parm3[i]!=-1){	/* 接続先人型番号 */
				if(work->save.objects_parm4[i]>=PreviewHuman[work->save.objects_parm3[i]].body.objs->n_models){
					work->save.objects_parm4[i]=0;
				}else if(work->save.objects_parm4[i] < 0){
					work->save.objects_parm4[i]=PreviewHuman[work->save.objects_parm3[i]].body.objs->n_models-1;
				}
				DG_SetPos( &PreviewHuman[work->save.objects_parm3[i]].body.objs->objs[work->save.objects_parm4[i]].world ) ;
			}else{
				DG_SetPos2( &work->save.objects_mov[i], &work->save.objects_rot[i] ) ;
			}
			DG_PutObjs( work->objects[i] );
		}
		if(work->save.objects_parm2[i]!=1){
			DG_GetLightMatrix( (FVECTOR*)work->objects[i]->world.m[3], work->objects_lights[i] );
		}

		if(work->save.objects_parm2[i]==0){
#if 1
			DG_DequeueObjs( work->objects[i] );
			DG_FreeObjs( work->objects[i] );
			work->objects[i]
				= DG_MakeObjs( work->objects_def[i], FLAG_NORMAL_SHADE, 0 );
			DG_QueueObjs( work->objects[i] );
			DG_SetLightMatrix( work->objects[i], work->objects_lights[i] );
#endif
		}else{
#if 1
			DG_DequeueObjs( work->objects[i] );
			DG_FreeObjs( work->objects[i] );
			work->objects[i]
				= DG_MakeObjs( work->objects_def[i], FLAG_PRE_SHADE, 0 );
			DG_QueueObjs( work->objects[i] );


			{
				int	now;
				now=i;

				if( work->save.objects_parm3[now]!=-1){	/* 接続先人型番号 */
					if(work->save.objects_parm4[now]>=PreviewHuman[work->save.objects_parm3[now]].body.objs->n_models){
						work->save.objects_parm4[now]=0;
					}else if(work->save.objects_parm4[now] < 0){
						work->save.objects_parm4[now]=PreviewHuman[work->save.objects_parm3[now]].body.objs->n_models-1;
					}
					DG_SetPos( &PreviewHuman[work->save.objects_parm3[now]].body.objs->objs[work->save.objects_parm4[now]].world ) ;
				}else{
					DG_SetPos2( &work->save.objects_mov[now], &work->save.objects_rot[now] ) ;
				}
				DG_PutObjs( work->objects[now] );
			}
			{
				FMATRIX	mat;
				DG_GetPos( &mat ) ;
				printf("a:%f %f %f\n",mat.m[3][0],mat.m[3][1],mat.m[3][2]);
			}

			DG_MakePreshade( work->objects[i],
							 (LIT_DEF*)GV_GetCache(  GV_CacheID( GV_StrCode( "stage" ), 'l' ) ) );

#endif

		}
    }
}

#else

void ResetDataObjects(void)
{
    ObjectControl *object=PreviewObject.objects;
    int	i;
    int size;

    size=PreviewObject.objects_size;

    for(i=0;i<size;i++,object++){
		DG_DequeueObjs(object->objs);
		DG_FreeObjs(object->objs);
		Object_SetQueue(object);
    }
}

#endif


void CopyObjectToSavework(void)
{
	ObjectControl *objects=PreviewObject.objects;
	int i;

	memset(SaveData.object,0x00,sizeof(SaveData.object));

	for(i=0;i<PreviewObject.objects_size && i<OBJECT_SAVE_SIZE;i++,objects++){
		SaveData.object[i].model_strcode=GV_StrCode(PreviewFile.object_filenames[i]);

		fpu_CopyVector(&(SaveData.object[i].pos),&(objects->pos));
		SaveData.object[i].rot.vx=objects->rot.vx;
		SaveData.object[i].rot.vy=objects->rot.vy;
		SaveData.object[i].rot.vz=objects->rot.vz;
		SaveData.object[i].rot.pad=objects->rot.pad;

		SaveData.object[i].disp_enable=objects->disp_enable;
		SaveData.object[i].shade_mode=objects->shade_mode;

		if(objects->target_human_index!=-1){
			SaveData.object[i].target_human_strcode
				=GV_StrCode(PreviewFile.human_filenames[objects->target_human_index]);
		}
		SaveData.object[i].human_object_index=objects->human_object_index;
	}
}

void CopyObjectFromSavework(void)
{
	ObjectControl *objects;
	int objects_index;
	int i,j;

	for(i=0;i<OBJECT_SAVE_SIZE;i++){
		if(SaveData.object[i].model_strcode==0) break;

		objects=NULL;
		for(j=0;j<PreviewFile.object_filenames_size;j++){
			if(SaveData.object[i].model_strcode==GV_StrCode(PreviewFile.object_filenames[j])){
				objects=PreviewObject.objects+j;
				objects_index=j;
				break;
			}
		}
		if(objects==NULL) continue;

		fpu_CopyVector(&(objects->pos),&(SaveData.object[i].pos));
		objects->rot.vx=SaveData.object[i].rot.vx;
		objects->rot.vy=SaveData.object[i].rot.vy;
		objects->rot.vz=SaveData.object[i].rot.vz;
		objects->rot.pad=SaveData.object[i].rot.pad;

		objects->disp_enable=SaveData.object[i].disp_enable;
		objects->shade_mode=SaveData.object[i].shade_mode;

		objects->target_human_index=-1;
		if(SaveData.object[i].target_human_strcode!=0){
			for(j=0;j<PreviewFile.human_filenames_size;j++){
				if(SaveData.object[i].target_human_strcode
				   ==GV_StrCode(PreviewFile.human_filenames[j])){

					objects->target_human_index=j;
					break;
				}
			}
		}
		objects->human_object_index=SaveData.object[i].human_object_index;
	}
}


#if 0

void InitObjects( Work *work )
{
    int	i,j,num,len,name_length;
    unsigned char file_name1[16];
    unsigned char file_name2[16];
    unsigned char *cp,*fn;

    sprintf( file_name1, "human" );
    len=BuffLen(file_name1);

    work->objects_num=0;
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
		if( j==len ) continue;	/* human*.kms だったのでとばす */
		//		printf("Initialize for OBJECTS TYPE:%d:%s\n",num,work->data_filename[i]);

		fn=work->data_filename[i];
		//		printf("check:::%s\n",fn);
		cp="objch";
		for( j=0; j<len; j++ ){
			if( *fn != *cp ) break;
			cp++;
			fn++;
		}
		if( j==len ) continue;	/* objch*.kms だったのでとばす */
		//		printf("Initialize for OBJECTS TYPE:%d:%s\n",num,work->data_filename[i]);

		fn=work->data_filename[i];
		//		printf("check:::%s\n",fn);
		cp="vanim";
		for( j=0; j<len; j++ ){
			if( *fn != *cp ) break;
			cp++;
			fn++;
		}
		if( j==len ) continue;	/* vanim*.kms だったのでとばす */
		//		printf("Initialize for OBJECTS TYPE:%d:%s\n",num,work->data_filename[i]);

		fn=work->data_filename[i];

		GV_ZeroMemory( file_name2, 16 );
		cp=file_name2;
		while(*fn!='.'){
			*cp=*fn;
			cp++;
			fn++;
		}

		printf("InitObject : %s\n",file_name2);

		work->objects_def[num] = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode( file_name2 ), 'k' ) ) ;
		if( work->objects_def[num]==NULL ){
			printf("ERR obj def\n");
			return;
		}
		work->objects[num] = DG_MakeObjs( work->objects_def[num], FLAG_NORMAL_SHADE, 0 );
		if( work->objects[num]==NULL ){
			printf("ERR objects\n");
			return;
		}
		DG_QueueObjs( work->objects[num] );
		DG_SetLightMatrix( work->objects[num], work->objects_lights[num] );
		work->save.objects_parm1[num]=0;
		work->save.objects_parm2[num]=0;

		name_length=BuffLen(work->data_filename[i]);
		cp=work->data_objects_name[num];
		fn=work->data_filename[i];
		for( j=0; j<name_length; j++ ){
			*cp = *fn;
			cp++;
			fn++;
		}

		num++;
		work->objects_num=num;
		if( num >= MAX_OBJECTS ){
			//			printf("MAX OBJECTS NUMBER is %d\n",MAX_OBJECTS);
			break;
		}
    }

    //printf("work->objects_num:%d\n",work->objects_num);
}

#else

int InitObjects(void)
{
    ObjectControl *objects;
    int i;
    int size;
    char *filename;

    size=PreviewFile.object_filenames_size;

    if((PreviewObject.objects
		=(ObjectControl *)GV_Malloc(sizeof(ObjectControl)*size))==NULL) return 0;
    PreviewObject.objects_size=size;

    GV_ZeroMemory(PreviewObject.objects,sizeof(ObjectControl)*size);

    objects=PreviewObject.objects;
    for(i=0;i<size;i++,objects++){
		filename=*(PreviewFile.object_filenames+i);

		printf("InitObject : %s\n",filename);

		objects->def=(DG_DEF *)GV_GetCache(GV_CacheID(GV_StrCode(filename),'k'));
		if(objects->def==NULL){
			printf("ERR obj def\n");
			return 0;
		}
		Object_SetDefaultData(objects);
		Object_SetQueue(objects);
    }

    return 1;
}

#endif


void ExitObject(void)
{
    ObjectControl *objects;
    int i;
    int size;

    size=PreviewObject.objects_size;
    objects=PreviewObject.objects;

    for(i=0;i<size;i++,objects++){
		if(objects->objs!=NULL){
			DG_DequeueObjs(objects->objs);
			DG_FreeObjs(objects->objs);
		}
    }
    GV_Free(PreviewObject.objects);
}


/* メイン処理 */

#if 0

void Object_DebugPrint(Work *work)
{
    int	x,y;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,0);
    DEBUG_Printf("OBJECTS MENU\n");

    x=LOCATE_X;
    y=LOCATE_Y+9+work->save.mode_2*9;
    DEBUG_Locate(x,y,0);
    DEBUG_Printf(">\n");

    x=LOCATE_X+9;
    y=LOCATE_Y+9;
    DEBUG_Locate(x,y,0);
    DEBUG_Printf("ALL    SELECT: %3d[%3d]:%s\n",
				 work->save.objects_now_num,
				 work->objects_num-1,
				 work->data_objects_name[work->save.objects_now_num]);

    if(work->save.objects_parm1[work->save.objects_now_num]==1){
		DEBUG_Printf( "ON      :\n" );
    }else{
		DEBUG_Printf( "OFF     :\n" );
    }

    DEBUG_Printf( "ACTIVE SELECT:\n" );
    if(work->save.objects_parm2[work->save.objects_now_num]==1){
		DEBUG_Printf( "SHADE  : PRE SHADE\n" );
    }else{
		DEBUG_Printf( "SHADE  : NORMAL SHADE\n" );
    }

    DEBUG_Printf( "POS X   : %d\n",(int)work->save.objects_mov[work->save.objects_now_num].vx );
    DEBUG_Printf( "POS Y   : %d\n",(int)work->save.objects_mov[work->save.objects_now_num].vy );
    DEBUG_Printf( "POS Z   : %d\n",(int)work->save.objects_mov[work->save.objects_now_num].vz );
    DEBUG_Printf( "ROT X   : %d\n",work->save.objects_rot[work->save.objects_now_num].vx );
    DEBUG_Printf( "ROT Y   : %d\n",work->save.objects_rot[work->save.objects_now_num].vy );
    DEBUG_Printf( "HUMAN NO: %d\n",work->save.objects_parm3[work->save.objects_now_num] );
    DEBUG_Printf( "JOINT NO: %d\n",work->save.objects_parm4[work->save.objects_now_num] );
    DEBUG_Printf( "ALL ON  :\n" );
    DEBUG_Printf( "ALL OFF :\n" );
}

void Object_DebugCursole(Work *work)
{
    int	i;
    int	speed;
    int	itemp;
    int	activ_num;

    activ_num=0;
    for( i=0; i<work->objects_num; i++ ){
		if(work->save.objects_parm1[i]==1){
			activ_num++;
		}
    }

    if ( work->pad  & PAD_U ) work->save.mode_2--;
    if ( work->pad  & PAD_D ) work->save.mode_2++;
    if ( work->save.mode_2 >=13  ) work->save.mode_2 = 0;
    if ( work->save.mode_2 <  0 ) work->save.mode_2 = 12;

    work->motion_stop_flag=0;
    switch(work->save.mode_2){
    case 0:
		if( work->pad & PAD_L ){
			work->save.objects_now_num--;
		}else if( work->pad & PAD_R ){
			work->save.objects_now_num++;
		}
		break;
    case 1:
		if( work->pad & PAD_L  ||  work->pad & PAD_R ){
			work->save.objects_parm1[work->save.objects_now_num] = 1-work->save.objects_parm1[work->save.objects_now_num];
		}
		break;
    case 2:
		if( work->pad & PAD_L ){
			itemp=work->save.objects_now_num-1;
			for( i=itemp; i>-1; i-- ){
				if(work->save.objects_parm1[i]==1){
					work->save.objects_now_num=i;
					break;
				}
			}
			if(i!=-1)break;
			for( i=work->objects_num-1; i>itemp; i-- ){
				if(work->save.objects_parm1[i]==1){
					work->save.objects_now_num=i;
					break;
				}
			}
		}else if( work->pad & PAD_R ){
			itemp=work->save.objects_now_num+1;
			for( i=itemp; i<work->objects_num; i++ ){
				if(work->save.objects_parm1[i]==1){
					work->save.objects_now_num=i;
					break;
				}
			}
			if(i!=work->objects_num)break;
			for( i=0; i<itemp; i++ ){
				if(work->save.objects_parm1[i]==1){
					work->save.objects_now_num=i;
					break;
				}
			}
		}
		break;
    case 3:
		if( work->pad & PAD_L  ||  work->pad & PAD_R ){
			if(work->save.objects_parm2[work->save.objects_now_num]==1){
				work->save.objects_parm2[work->save.objects_now_num]=0;
				DG_DequeueObjs( work->objects[work->save.objects_now_num] );
				DG_FreeObjs( work->objects[work->save.objects_now_num] );
				work->objects[work->save.objects_now_num]
					= DG_MakeObjs( work->objects_def[work->save.objects_now_num], FLAG_NORMAL_SHADE, 0 );
				DG_QueueObjs( work->objects[work->save.objects_now_num] );
				DG_SetLightMatrix( work->objects[work->save.objects_now_num], work->objects_lights[work->save.objects_now_num] );
			}else{
				work->save.objects_parm2[work->save.objects_now_num]=1;
				DG_DequeueObjs( work->objects[work->save.objects_now_num] );
				DG_FreeObjs( work->objects[work->save.objects_now_num] );
				work->objects[work->save.objects_now_num]
					= DG_MakeObjs( work->objects_def[work->save.objects_now_num], FLAG_PRE_SHADE, 0 );
				DG_QueueObjs( work->objects[work->save.objects_now_num] );

				{
					int	now;
					now=work->save.objects_now_num;

					if( work->save.objects_parm3[now]!=-1){	/* 接続先人型番号 */
						if(work->save.objects_parm4[now]>=PreviewHuman[work->save.objects_parm3[now]].body.objs->n_models){
							work->save.objects_parm4[now]=0;
						}else if(work->save.objects_parm4[now] < 0){
							work->save.objects_parm4[now]=PreviewHuman[work->save.objects_parm3[now]].body.objs->n_models-1;
						}
						DG_SetPos( &PreviewHuman[work->save.objects_parm3[now]].body.objs->objs[work->save.objects_parm4[now]].world ) ;
					}else{
						DG_SetPos2( &work->save.objects_mov[now], &work->save.objects_rot[now] ) ;
					}
					DG_PutObjs( work->objects[now] );
				}

				{
					FMATRIX	mat;
					DG_GetPos( &mat ) ;
					printf("b:%f %f %f\n",mat.m[3][0],mat.m[3][1],mat.m[3][2]);
				}

				DG_MakePreshade( work->objects[work->save.objects_now_num],
								 (LIT_DEF*)GV_GetCache(  GV_CacheID( GV_StrCode( "stage" ), 'l' ) ) );
			}
		}
		break;
    case 4:
		if(work->key_count>=SPEEDUP_2){
			speed=100;
		}else if(work->key_count>=SPEEDUP_1){
			speed=10;
		}else{
			speed=1;
		}
		if( work->pad & PAD_L ){
			work->save.objects_mov[work->save.objects_now_num].vx -= (float)speed;
		}else if( work->pad & PAD_R ){
			work->save.objects_mov[work->save.objects_now_num].vx += (float)speed;
		}
		break;
    case 5:
		if(work->key_count>=SPEEDUP_2){
			speed=100;
		}else if(work->key_count>=SPEEDUP_1){
			speed=10;
		}else{
			speed=1;
		}
		if( work->pad & PAD_L ){
			work->save.objects_mov[work->save.objects_now_num].vy -= (float)speed;
		}else if( work->pad & PAD_R ){
			work->save.objects_mov[work->save.objects_now_num].vy += (float)speed;
		}
		break;
    case 6:
		if(work->key_count>=SPEEDUP_2){
			speed=100;
		}else if(work->key_count>=SPEEDUP_1){
			speed=10;
		}else{
			speed=1;
		}
		if( work->pad & PAD_L ){
			work->save.objects_mov[work->save.objects_now_num].vz -= (float)speed;
		}else if( work->pad & PAD_R ){
			work->save.objects_mov[work->save.objects_now_num].vz += (float)speed;
		}
		break;
    case 7:
		if(work->key_count>=SPEEDUP_2){
			speed=10;
		}else if(work->key_count>=SPEEDUP_1){
			speed=10;
		}else{
			speed=1;
		}
		if( work->pad & PAD_L ){
			work->save.objects_rot[work->save.objects_now_num].vx -= speed;
		}else if( work->pad & PAD_R ){
			work->save.objects_rot[work->save.objects_now_num].vx += speed;
		}
		break;
    case 8:
		if(work->key_count>=SPEEDUP_2){
			speed=10;
		}else if(work->key_count>=SPEEDUP_1){
			speed=10;
		}else{
			speed=1;
		}
		if( work->pad & PAD_L ){
			work->save.objects_rot[work->save.objects_now_num].vy -= speed;
		}else if( work->pad & PAD_R ){
			work->save.objects_rot[work->save.objects_now_num].vy += speed;
		}
		break;
    case 9:
		if( work->pad & PAD_L ){
			work->save.objects_parm3[work->save.objects_now_num]--;
			if(work->save.objects_parm3[work->save.objects_now_num] < -1){
				work->save.objects_parm3[work->save.objects_now_num]=work->human_num-1;
			}
		}else if( work->pad & PAD_R ){
			work->save.objects_parm3[work->save.objects_now_num]++;
			if(work->save.objects_parm3[work->save.objects_now_num] >= work->human_num){
				work->save.objects_parm3[work->save.objects_now_num]=-1;
			}
		}
		break;
    case 10:
		if( work->pad & PAD_L ){
			work->save.objects_parm4[work->save.objects_now_num]--;
		}else if( work->pad & PAD_R ){
			work->save.objects_parm4[work->save.objects_now_num]++;
		}
		break;
    case 11:
		if( work->pad & PAD_A  ||  work->pad & PAD_L  ||  work->pad & PAD_R ){
			for(i=0; i<work->objects_num; i++){
				work->save.objects_parm1[i] = 1;	/* モード：アクティブ */
			}
		}
		break;
    case 12:
		if( work->pad & PAD_A  ||  work->pad & PAD_L  ||  work->pad & PAD_R ){
			for(i=0; i<work->objects_num; i++){
				work->save.objects_parm1[i] = 0;	/* モード：非アクティブ */
			}
		}
		break;
    }

    if( work->pad & PAD_SEL ){
		for(i=0; i<work->objects_num; i++){
			work->save.objects_parm3[i]=-1;	/* 接続先人型番号 */
			work->save.objects_mov[i].vx=0.0f;
			work->save.objects_mov[i].vy=0.0f;
			work->save.objects_mov[i].vz=0.0f;
			work->save.objects_rot[i].vx=0;
			work->save.objects_rot[i].vy=0;
			work->save.objects_rot[i].vz=0;
		}
		ResetDataObjects( work );
    }

    if ( work->save.objects_now_num >= work->objects_num ){
		work->save.objects_now_num   = 0;
    }else if ( work->save.objects_now_num < 0 ){
		work->save.objects_now_num   = work->objects_num - 1;
    }
}

#else


#define OBJECT_PREVIEW_LOCATE_X		(LOCATE_X+0x08*4)
#define OBJECT_PREVIEW_LOCATE_Y		(LOCATE_Y+COLUMN_HEIGHT*2)


static void Object_AttrDebugPrint(ObjectControl *object)
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
    char *filename=*(PreviewFile.object_filenames+PreviewObject.objects_index);
    int x,y;

    x=OBJECT_PREVIEW_LOCATE_X;
    y=OBJECT_PREVIEW_LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Object Name : %s",filename);

    if(object->target_human_index>=0){
		char *filename=*(PreviewFile.human_filenames+object->target_human_index);
		int index=object->human_object_index;

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Human Name  : %s",filename);

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		if(index<sizeof(objname)/sizeof(objname[0])){
			DEBUG_Printf("Part Name   : %s (ID=%d)",objname[index],index);
		}
		else{
			DEBUG_Printf("Part Name  : Extend           (ID=%d)",index);
		}
    }
    else{
		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Human Name  : No Selected");

		y+=COLUMN_HEIGHT;
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Part Name   : -----------");
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(object->shade_mode){
		DEBUG_Printf("Shade Mode  : PRE SHADE\n" );
    }else{
		DEBUG_Printf("Shade Mode  : NORMAL SHADE\n" );
    }

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: ALL ON >");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: ALL OFF >");


    /* カーソル */
    x=LOCATE_X;
    y=OBJECT_PREVIEW_LOCATE_Y+PreviewObject.cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf(">>>");
}

static void Object_MoveDebugPrint(ObjectControl *object)
{
    char *filename=*(PreviewFile.object_filenames+PreviewObject.objects_index);
    int x,y;

    x=LOCATE_X;
    y=OBJECT_PREVIEW_LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(PreviewObject.speed_mode){
		DEBUG_Printf("<High Speed Mode  :R2>");
    }
    else{
		DEBUG_Printf("<Low Speed Mode   :R2>");
    }

    x=OBJECT_PREVIEW_LOCATE_X;
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Object Name : %s",filename);

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
    DEBUG_Printf("POS X : %d",(int)(object->pos.vx));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("POS Y : %d",(int)(object->pos.vy));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("POS Z : %d",(int)(object->pos.vz));

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT X : %d",object->rot.vx);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT Y : %d",object->rot.vy);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("ROT Z : %d",object->rot.vz);

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<SW: Reset Pos/Rot>");


    /* カーソル */
    x=LOCATE_X;
    y=OBJECT_PREVIEW_LOCATE_Y+COLUMN_HEIGHT+PreviewObject.cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("R1>");
}

void Object_DebugPrint(void)
{
    ObjectControl *object;
    int	x,y;

    object=PreviewObject.objects+PreviewObject.objects_index;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("OBJECTS MENU");

    DEBUG_Locate(x+0x08*0x0e,y,MENU_MODE_NORMAL);
    if(object->disp_enable) DEBUG_Printf("<Visible   :L1>");
    else DEBUG_Printf("<Invisible :L1>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    switch(PreviewObject.cursole_mode){
    case 0:
		DEBUG_Printf("<Attribute Mode   :L2>");
		Object_AttrDebugPrint(object);
		break;
    case 1:
		DEBUG_Printf("<Move/Rotate Mode :L2>");
		Object_MoveDebugPrint(object);
		break;
    }
}

static void SelectObjectFile(void)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		PreviewObject.objects_index--;
		if(PreviewObject.objects_index<0) PreviewObject.objects_index=PreviewObject.objects_size-1;
		break;
    case PAD_R:
		PreviewObject.objects_index++;
		if(PreviewObject.objects_index>=PreviewObject.objects_size) PreviewObject.objects_index=0;
		break;
    }
}

static void SelectHumanFile(ObjectControl *object)
{
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		object->target_human_index--;
		if(object->target_human_index<-1) object->target_human_index=PreviewHuman.humans_size-1;
		break;
    case PAD_R:
		object->target_human_index++;
		if(object->target_human_index>=PreviewHuman.humans_size) object->target_human_index=-1;
		break;
    }
    if(object->target_human_index>=0){
		HumanControl *human=PreviewHuman.humans+object->target_human_index;
		int size=human->body.objs->n_models;

		if(object->human_object_index<0) object->human_object_index=0;
		else if(object->human_object_index>=size) object->human_object_index=size-1;
    }
}

static void SelectHumanObject(ObjectControl *object)
{
    HumanControl *human=PreviewHuman.humans+object->target_human_index;
    int size=human->body.objs->n_models;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
		object->human_object_index--;
		if(object->human_object_index<0) object->human_object_index=size-1;
		break;
    case PAD_R:
		object->human_object_index++;
		if(object->human_object_index>=size) object->human_object_index=0;
		break;
    }
}

static void SelectShadeMode(ObjectControl *object)
{
    if(PreviewKey.auto_status & (PAD_L|PAD_R)){
		object->shade_mode^=1;

		DG_DequeueObjs(object->objs);
		DG_FreeObjs(object->objs);
		Object_SetQueue(object);
    }
    if(PreviewKey.press & PAD_A){
		if(object->shade_mode){
			DG_DequeueObjs(object->objs);
			DG_FreeObjs(object->objs);
			Object_SetQueue(object);
		}
    }
}

static void SwAllOnOff(int disp)
{
    if(PreviewKey.press & PAD_A){
		ObjectControl *object=PreviewObject.objects;
		int i;
		int size;

		size=PreviewObject.objects_size;
		for(i=0;i<size;i++,object++){
			object->disp_enable=disp;
		}
    }
}

static void SwDefaultPosRot(ObjectControl *object)
{
    if(PreviewKey.press & PAD_A){
		Object_SetDeafultPosition(object);
    }
}

enum {
    MODELNAME=0,
    HUMANNAME,
    OBJECTNUM,
    SHADEMODE,
    ALLON,
    ALLOFF,

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

    MOVEDEBUG_MAX,
};

static void NormalSelect(ObjectControl *object)
{
    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
		PreviewObject.cursole_pos--;
		if(PreviewObject.cursole_pos<0) PreviewObject.cursole_pos=NORMALDEBUG_MAX-1;
		else if(PreviewObject.cursole_pos==OBJECTNUM && object->target_human_index==-1){
			PreviewObject.cursole_pos--;
		}
		break;
    case PAD_D:
		PreviewObject.cursole_pos++;
		if(PreviewObject.cursole_pos>=NORMALDEBUG_MAX) PreviewObject.cursole_pos=0;
		else if(PreviewObject.cursole_pos==OBJECTNUM && object->target_human_index==-1){
			PreviewObject.cursole_pos++;
		}
		break;
    }

    switch(PreviewObject.cursole_pos){
    case MODELNAME:
		SelectObjectFile();
		break;
    case HUMANNAME:
		SelectHumanFile(object);
		break;
    case OBJECTNUM:
		SelectHumanObject(object);
		break;
    case SHADEMODE:
		SelectShadeMode(object);
		break;
    case ALLON:
		SwAllOnOff(1);
		break;
    case ALLOFF:
		SwAllOnOff(0);
		break;
    }
}

static void MoveRotSelect(ObjectControl *object)
{
    float mv;
    int rv;

    if(PreviewKey.status & PAD_R2){
		PreviewObject.speed_mode=1;
		mv=100.0f;
		rv=10;
    }
    else{
		PreviewObject.speed_mode=0;
		mv=10.0f;
		rv=1;
    }

    if(PreviewKey.status & PAD_R1){
		switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
		case PAD_U:
			PreviewObject.cursole_pos--;
			if(PreviewObject.cursole_pos<0) PreviewObject.cursole_pos=MOVEDEBUG_MAX-1;
			break;
		case PAD_D:
			PreviewObject.cursole_pos++;
			if(PreviewObject.cursole_pos>=MOVEDEBUG_MAX) PreviewObject.cursole_pos=0;
			break;
		}
    }

    switch(PreviewObject.cursole_pos){
    case MODELNAME: /* Object Name */
		SelectObjectFile();
		break;
    case MOVE_VIEWCORRD: /* Move On ViewCoord */
		if(!(PreviewKey.status & PAD_R1)){
			PreviewVMoveXYZ(&(object->pos),mv);
		}
		break;

#if 0
    case ROT_VIEWCORRD: /* Rotate On ViewCoord */
		if(!(PreviewKey.status & PAD_R1)){
			PreviewVRotXYZ(&(object->objs->world),&(object->rot),rv);
		}
		break;
#endif

    case POSX: /* POS X */
		PreviewMoveX(&(object->pos),mv);
		break;
    case POSY: /* POS Y */
		PreviewMoveY(&(object->pos),mv);
		break;
    case POSZ: /* POS Z */
		PreviewMoveZ(&(object->pos),mv);
		break;
    case ROTX: /* ROT X */
		PreviewRotX(&(object->rot),rv);
		break;
    case ROTY: /* ROT Y */
		PreviewRotY(&(object->rot),rv);
		break;
    case ROTZ: /* ROT Z */
		PreviewRotZ(&(object->rot),rv);
		break;
    case RESET_POSROT: /* Reset Pos/Rot */
		SwDefaultPosRot(object);
		break;
    }
}

void Object_DebugCursole(void)
{
    ObjectControl *object=PreviewObject.objects+PreviewObject.objects_index;

    switch(PreviewObject.cursole_mode){
    case 0:
		NormalSelect(object);
		break;
    case 1:
		MoveRotSelect(object);
		break;
    }
    if(PreviewKey.press & PAD_L1){
		object->disp_enable^=1;
    }
    if(PreviewKey.press & PAD_L2){
		PreviewObject.cursole_mode^=1;
		PreviewObject.cursole_pos=0;
    }
    if(PreviewKey.press & PAD_SEL){
		Objects_SetDefaultData();
    }
}

#endif



#if 0

void Preview_ObjectAct(Work *work)
{
    int i;

    /* オブジェクツ（物） */
    for(i=0; i<work->objects_num; i++){
		if( work->save.objects_parm1[i]==1 ){
			work->objects[i]->flag &= ~DG_FLAG_INVISIBLE ;
		}else{
			work->objects[i]->flag |= DG_FLAG_INVISIBLE ;
		}
    }

    for(i=0; i<work->objects_num; i++){
		if( work->save.objects_parm1[i]==1 ){
			if( work->save.objects_parm3[i]!=-1){	/* 接続先人型番号 */
				if(work->save.objects_parm4[i]>=PreviewHuman[work->save.objects_parm3[i]].body.objs->n_models){
					work->save.objects_parm4[i]=0;
				}else if(work->save.objects_parm4[i] < 0){
					work->save.objects_parm4[i]=PreviewHuman[work->save.objects_parm3[i]].body.objs->n_models-1;
				}
				DG_SetPos( &PreviewHuman[work->save.objects_parm3[i]].body.objs->objs[work->save.objects_parm4[i]].world ) ;
			}else{
				DG_SetPos2( &work->save.objects_mov[i], &work->save.objects_rot[i] ) ;
			}
			DG_PutObjs( work->objects[i] );
		}
		if(work->save.objects_parm2[i]!=1){
			DG_GetLightMatrix( (FVECTOR*)work->objects[i]->world.m[3], work->objects_lights[i] );
		}
    }
}

#else

void Preview_ObjectAct(void)
{
    ObjectControl *object;
    int i;
    int size;

	GM_CurrentMap=PreviewLight.where;

    object=PreviewObject.objects;
    size=PreviewObject.objects_size;

    for(i=0;i<size;i++,object++){
		FVECTOR *pos;

		if(object->disp_enable){
			DG_VisibleObjs(object->objs);
		}
		else{
			DG_InvisibleObjs(object->objs);
		}

		pos=Object_SetPosition(object);

		if(object->shade_mode==0){
			DG_GetLightMatrix(pos,object->lights);
		}
    }
}

typedef struct {
    GV_ACT_EX	actor;
} WorkO;

static int GetResources(WorkO *work)
{
    PreviewObject.cursole_mode=0;
    PreviewObject.cursole_pos=0;
    PreviewObject.speed_mode=0;

    return InitObjects();
}

static void NullAct(WorkO *work)
{
    printf("Abort Preview Human\n");
    GV_DestroyActor(work);
}

static void NullDie(void)
{
}

/* 初期化部メイン */
void *NewPreviewObject(void)
{
    WorkO *work ;

    work=(WorkO *)GV_NewActor(GV_ACTOR_USER,sizeof(WorkO));
    if(work!=NULL) {
		if(!GetResources(work)) GV_SetActor(&(work->actor),NullAct,NullDie);
		else GV_SetActor(&(work->actor),Preview_ObjectAct,ExitObject);
		GV_ActorEX(&(work->actor));
    }
    return (void *)work ;
}

#endif
