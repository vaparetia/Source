/*
	human_preview.c
		モデルすり替えのプレビュー用人形

	1999/12/08 K.Kano
	$Id: human_preview.c,v 1.1.1.3 2002/11/19 11:43:28 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif


#include "preview_def.h"


/* ワーク */
typedef	struct _Work {
    GV_ACT_EX	actor;		/* リンク */
    OBJECT		body;

    FMATRIX		lights[2];

    FVECTOR		pos;
    SVECTOR		rot;

    OBJCHANGE_WORK	*oc;

    int			objnum;
    int			parts_index;
    int			disp_enable;
} Work ;


#define HUMAN_MODEL_OBJN	21

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


/* ------------------------------------------------------------------------ */
/* メイン処理 */

void HumanOC_DebugPrint(Work *work,int index)
{
    static const char *objname[]={
		"Koshi           :",
		"Onaka           :",
		"Mune            :",
		"Migi   Kata     :",
		"Migi   Ude1     :",
		"Migi   Ude2     :",
		"Migi   Te       :",
		"Hidari Kata     :",
		"Hidari Ude1     :",
		"Hidari Ude2     :",
		"Hidari Te       :",
		"Kubi            :",
		"Atama           :",
		"Migi   Ashi1    :",
		"Migi   Ashi2    :",
		"Migi   Kakato   :",
		"Migi   Tsumasaki:",
		"Hidari Ashi1    :",
		"Hidari Ashi2    :",
		"Hidari Kakato   :",
		"Hidari Tsumasaki:",
    };
    int x=LOCATE_X,y=LOCATE_Y;
    int i;
    OBJCHANGE_WORK *oc=work->oc;


    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("OBJECT List ");
    if(index>=0) DEBUG_Printf("%d",index);
    if(work->disp_enable){
		DEBUG_Printf("  <Visible>");
    }
    else{
		DEBUG_Printf("  <Invisible>");
    }

    x+=FONT_WIDTH*2;
    y+=COLUMN_HEIGHT;
    for(i=0;i<sizeof(objname)/sizeof(objname[0]);i++){
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf(objname[i]);
		if(oc->mdl_list_index[i]<0) DEBUG_Printf("default");
		else DEBUG_Printf("%d",oc->mdl_list_index[i]);
		y+=COLUMN_HEIGHT;
    }

    y-=COLUMN_HEIGHT*HUMAN_MODEL_OBJN;
    x-=FONT_WIDTH;

    DEBUG_Locate(x,y+work->objnum*0x08,MENU_MODE_NORMAL);
    DEBUG_Printf(">");
}

void HumanOC_DebugCursole(Work *work)
{
    OBJCHANGE_WORK *oc=work->oc;

    switch(GV_PadData[1].press & (PAD_L|PAD_R|PAD_U|PAD_D)){
    case PAD_L:
		work->parts_index--;
		if(work->parts_index<-1){
			work->parts_index=oc->parts->n_models-1;
		}
		ChangeObj(oc,work->objnum,work->parts_index);
		break;
    case PAD_R:
		work->parts_index++;
		if(work->parts_index>=oc->parts->n_models){
			work->parts_index=-1;
		}
		ChangeObj(oc,work->objnum,work->parts_index);
		break;
    case PAD_U:
		work->objnum--;
		if(work->objnum<0){
			work->objnum=HUMAN_MODEL_OBJN-1;
		}
		work->parts_index=oc->mdl_list_index[work->objnum];
		break;
    case PAD_D:
		work->objnum++;
		if(work->objnum>=HUMAN_MODEL_OBJN){
			work->objnum=0;
		}
		work->parts_index=oc->mdl_list_index[work->objnum];
		break;
    }
    switch(GV_PadData[1].press & (PAD_L2|PAD_R2)){
    case PAD_L2:
    case PAD_R2:
		work->disp_enable^=1;
		break;
    }
}

static void Act(Work *work)
{
	GM_CurrentMap=PreviewLight.where;

#if 0
    if(GM_Debug2PMode==GM_DEBUG_MODE_PREVIEW){
		HumanOC_DebugCursole(work);
		HumanOC_DebugPrint(work,-1);
    }
#endif

    if(work->disp_enable){
		DG_VisibleObjs(work->body.objs);
    }
    else{
		DG_InvisibleObjs(work->body.objs);
    }

    DG_GetLightMatrix(&(work->pos),work->lights);
    DG_SetPos2(&(work->pos),&(work->rot));
    GM_ActObject(&(work->body));
}

static void Die(Work *work)
{
    if(work->oc!=NULL){
		GM_FreeObjectForObjChange(&(work->body),work->oc);
    }
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


/* 初期設定値を取得 */
static void GetOptionValue(Work *work,int *mname,int *mname2,int *mtname)
{
    work->pos=DG_ZeroVector;
    work->rot=DG_ZeroSVector;

    *mname=GV_StrCode("usa_debug1");
    *mname2=GV_StrCode("test");
    *mtname=GV_StrCode("snake");

    if(GCL_GetOption('m')!=NULL){
		*mname=GCL_GetNextInt();
    }
    if(GCL_GetOption('l')!=NULL){
		*mname2=GCL_GetNextInt();
    }
    if(GCL_GetOption('a')!=NULL){
		*mtname=GCL_GetNextInt();
    }

    if(GCL_GetOption('x')!=NULL){
		int buf[3];
		GCL_GetIV(GCL_NextStr(),buf);
		vu0_IV0toFV((IVECTOR *)buf,&(work->pos));
    }
    if(GCL_GetOption('r')!=NULL){
		int buf[3];
		GCL_GetIV(GCL_NextStr(),buf);
		work->rot.vx=buf[0];
		work->rot.vy=buf[1];
		work->rot.vz=buf[2];
    }
}


/* 資源を獲得 */
static int GetResources(Work *work)
{
    OBJECT *body;
    int mname,mname2,mtname;

    GetOptionValue(work,&mname,&mname2,&mtname);

    body=&(work->body);

    if((work->oc=GM_InitObjectForObjChange(body,mname,mname2,
										   NULL,OBJECT_FLAG))==NULL) return 0;

    GM_ConfigObjectLight(body,work->lights) ;
    GM_ConfigObjectMotion(body,2,mtname,MT_FLAG_HUMAN2);
    GM_ConfigObjectAction(body,0,0 /* action */,0,0xfffff,(int)(0.5f*60.0f)) ;

    work->objnum=0;
    work->parts_index=-1;

    return 1;
}

/* 初期化部メイン */
void *NewOCHumanPreview(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
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



/* 資源を獲得 */
static int GetResources_called(Work *work,FVECTOR *x,SVECTOR *r,int mname,int mname2,int mtname)
{
    OBJECT *body;

    work->pos=*x;
    work->rot=*r;

    body=&(work->body);

    if((work->oc=GM_InitObjectForObjChange(body,mname,mname2,
										   NULL,OBJECT_FLAG))==NULL) return 0;

    GM_ConfigObjectLight(body,work->lights) ;
    GM_ConfigObjectMotion(body,2,mtname,MT_FLAG_HUMAN2);
    GM_ConfigObjectAction(body,0,0 /* action */,0,0xfffff,(int)(0.5f*60.0f)) ;

    work->objnum=0;
    work->parts_index=-1;
    work->disp_enable=0;

    return 1;
}

/* 初期化部メイン */
void *NewOCHumanPreview_called(FVECTOR *x,SVECTOR *r,int mname,int mname2,int mtname)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
	
		if(!GetResources_called(work,x,r,mname,mname2,mtname)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
