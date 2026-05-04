//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	raincoat.c
		レインコート

	2000/2/15 K.Kano
	$Id: raincoat.c,v 1.1.1.3 2002/11/19 11:43:32 Yoshizawa1 Exp $
*/


#include "raincoat.h"


typedef struct {
    GV_ACT_EX	actor;

#if 0
    void *mm_coat;
#else
    DISP_COAT *dispcoat;
    void *va_coat;
#endif

} Work;


/* メイン処理 */
static void Act(Work *work)
{

#if 0
    MoveMMCoat(work->mm_coat);
#else
    MoveDispCoat(work->dispcoat);
    MoveVACoat(work->va_coat);
#endif

}

static void Die(Work *work)
{
#if 0
    ExitMMCoat(work->mm_coat);
#else
    if(work->va_coat!=NULL) ExitVACoat(work->va_coat);
    if(work->dispcoat!=NULL) ExitDispCoat(work->dispcoat);
#endif
}

/* 初期化処理 */
static int GetResources(Work *work)
{
#if 0

    CONTROL *ctrl;
    OBJECT *object;
    DG_OBJS *objs;
    CV2_DEF *cv2;

    ctrl=GM_SearchWhere(GV_StrCode("スネーク"));
    if(ctrl==NULL) return 0;

    object=(OBJECT *)(ctrl+1);

    objs=object->objs;
    cv2=(CV2_DEF *)GV_GetCache(GV_CacheID(GV_StrCode("tnr_def"),'c'));

    work->mm_coat=InitMMCoat(objs,cv2);

#else

    CONTROL *ctrl;
    OBJECT *object;
    DG_OBJS *target;
    CV2_DEF *cv2[64];

    int name_data[]={
#if 0
		GV_StrCode("tnr_coat_sgl_anim00"),
		GV_StrCode("tnr_coat_sgl_anim01"),
		GV_StrCode("tnr_coat_sgl_anim02"),
#else
		GV_StrCode("tnr_coat_sgl"),
		GV_StrCode("tnr_coat_sgl_anim00"),
		GV_StrCode("tnr_coat_sgl_anim01"),
#endif
    };
    int multi_name=GV_StrCode("tnr_coat_mlch");
    int single_names_size=sizeof(name_data)/sizeof(name_data[0]);
    int single_disp_name=name_data[0];
    float wind_min=0.0f,wind_max=0.0f;
    float frame_min=0.0f,frame_max=0.0f;


    ctrl=GM_SearchWhere(GV_StrCode("スネーク"));
    if(GCL_GetOption('y')!=NULL){
		int name=GCL_GetNextInt();
		ctrl=GM_SearchWhere(name);
    }
    if(ctrl==NULL) return 0;
    object=(OBJECT *)(ctrl+1);

    target=object->objs;

    if(GCL_GetOption('e')!=NULL){
		multi_name=GCL_GetNextInt();
    }
    if(GCL_GetOption('m')!=NULL){
		int single_names_size=GCL_GetNextInt();
		int i;

		for(i=0;i<single_names_size;i++){
			int name=GCL_GetNextInt();
			if(i==0) single_disp_name=name;
			*(cv2+i)=(CV2_DEF *)GV_GetCache(GV_CacheID(name,'c'));
		}
    }
    else{
		int i;

		for(i=0;i<single_names_size;i++){
			*(cv2+i)=(CV2_DEF *)GV_GetCache(GV_CacheID(name_data[i],'c'));
		}
    }

    if(GCL_GetOption('w')!=NULL){
		wind_min=(float)GCL_GetNextInt();
		wind_max=(float)GCL_GetNextInt();
    }
    if(GCL_GetOption('f')!=NULL){
		frame_min=(float)GCL_GetNextInt();
		frame_max=(float)GCL_GetNextInt();
    }

    if((work->dispcoat=InitDispCoat(single_disp_name,multi_name,object))==NULL) return 0;

#if 1
    if((work->va_coat=InitVACoat(work->dispcoat->objs,cv2,single_names_size,
								 wind_min,wind_max,frame_max,frame_min))==NULL) return 0;
#endif

#endif

    return 1;
}

void *NewRaincoat(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


/* 初期化処理 */
static int GetResources_called(Work *work,OBJECT *object,
							   int *single_names,int single_names_size,int multi_name,
							   float wind_min,float wind_max,float frame_min,float frame_max)
{
#if 0

    CONTROL *ctrl;
    OBJECT *object;
    DG_OBJS *objs;
    CV2_DEF *cv2;

    ctrl=GM_SearchWhere(GV_StrCode("スネーク"));
    if(ctrl==NULL) return 0;

    object=(OBJECT *)(ctrl+1);

    objs=object->objs;
    cv2=(CV2_DEF *)GV_GetCache(GV_CacheID(GV_StrCode("tnr_def"),'c'));

    work->mm_coat=InitMMCoat(objs,cv2);

#else

    CV2_DEF *cv2[64];

    int name_data[]={
		GV_StrCode("tnr_coat_sgl"),
		GV_StrCode("tnr_coat_sgl_anim00"),
		GV_StrCode("tnr_coat_sgl_anim01"),
    };
    int single_disp_name;
    int i;


    if(single_names==NULL){
		single_names=name_data;
		single_names_size=sizeof(name_data)/sizeof(name_data[0]);
    }
    if(multi_name==0) multi_name=GV_StrCode("tnr_coat_mlch");
    single_disp_name=*(single_names+0);


    for(i=0;i<single_names_size;i++){
		int name=*(single_names+i);
		*(cv2+i)=(CV2_DEF *)GV_GetCache(GV_CacheID(name,'c'));
    }


    if ( (work->dispcoat=InitDispCoat(single_disp_name,multi_name,object)) )
	work->va_coat=InitVACoat(work->dispcoat->objs,cv2,single_names_size,
							 wind_min,wind_max,frame_min,frame_max);
#endif

    return 1;
}

void *NewRaincoat_called(OBJECT *target,int *single_names,int single_names_size,int multi_name,
						 float wind_min,float wind_max,float frame_min,float frame_max)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		if(!GetResources_called(work,target,single_names,single_names_size,multi_name,
								wind_min,wind_max,frame_min,frame_max)){

			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
