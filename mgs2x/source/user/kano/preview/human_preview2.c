/*
	human_preview2.c
		モデルすり替えのプレビュー用人形

	1999/12/10 K.Kano
	$Id: human_preview2.c,v 1.1.1.3 2002/11/19 11:43:28 Yoshizawa1 Exp $
*/


#include "preview_def.h"


#define HUMAN_MODEL_OBJN	21

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


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

    DG_DEF		**mdl_list;
    int			mdl_list_index[HUMAN_MODEL_OBJN];
    int			mdl_list_size;

    /* 次のワークへのポインタ。*/
    struct _Work *next;
} Work ;


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void PutWork(Work *work)
{
    Work *list=(Work *)(PreviewHumanOC.objchange);

    PreviewHumanOC.objchange_size++;
    work->next=NULL;

    if(list==NULL){
		PreviewHumanOC.objchange=work;
    }
    else{
		while(list->next!=NULL) list=list->next;
		list->next=work;
    }
}

static Work *GetWork(Work *work,int index)
{
    while(index>0){
		if(work!=NULL) work=work->next;
		index--;
    }
    return work;
}

void HumanOC2_DebugPrint(void)
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
    Work *work;
    int x,y;
    int i;

    x=LOCATE_X;
    y=LOCATE_Y;

    work=GetWork(PreviewHumanOC.objchange,PreviewHumanOC.objchange_index);

    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("OBJECT List ");
    if(PreviewHumanOC.objchange_size>1) DEBUG_Printf("%d",PreviewHumanOC.objchange_index);

    if(work->disp_enable){
		DEBUG_Printf("  <Visible   :L1>");
    }
    else{
		DEBUG_Printf("  <Invisible :L1>");
    }

    x+=FONT_WIDTH*4;
    y+=COLUMN_HEIGHT;
    for(i=0;i<sizeof(objname)/sizeof(objname[0]);i++){
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf((char *)(objname[i]));
		if(work->mdl_list_index[i]<0) DEBUG_Printf("default");
		else DEBUG_Printf("%d",work->mdl_list_index[i]);
		y+=COLUMN_HEIGHT;
    }

    x-=FONT_WIDTH*4;
    y-=COLUMN_HEIGHT*HUMAN_MODEL_OBJN;
    DEBUG_Locate(x,y+work->objnum*0x08,MENU_MODE_NORMAL);
    DEBUG_Printf(">>>");
}

void HumanOC2_DebugCursole(void)
{
    Work *work;
    OBJCHANGE_WORK *oc;

    work=GetWork(PreviewHumanOC.objchange,PreviewHumanOC.objchange_index);
    oc=work->oc;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R|PAD_U|PAD_D)){
    case PAD_L:
		work->parts_index--;
		if(work->parts_index<-1){
			work->parts_index=work->mdl_list_size-2;
		}
		ChangeObjByMDL(oc,work->objnum,
					   (*(work->mdl_list+work->parts_index+1))->models+work->objnum);
		work->mdl_list_index[work->objnum]=work->parts_index;
		break;
    case PAD_R:
		work->parts_index++;
		if(work->parts_index>=work->mdl_list_size-1){
			work->parts_index=-1;
		}
		ChangeObjByMDL(oc,work->objnum,
					   (*(work->mdl_list+work->parts_index+1))->models+work->objnum);
		work->mdl_list_index[work->objnum]=work->parts_index;
		break;
    case PAD_U:
		work->objnum--;
		if(work->objnum<0){
			work->objnum=HUMAN_MODEL_OBJN-1;
		}
		work->parts_index=work->mdl_list_index[work->objnum];
		break;
    case PAD_D:
		work->objnum++;
		if(work->objnum>=HUMAN_MODEL_OBJN){
			work->objnum=0;
		}
		work->parts_index=work->mdl_list_index[work->objnum];
		break;
    }
    switch(PreviewKey.press & (PAD_L2|PAD_R2)){
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

/* 資源を獲得 */
static int GetResources_called(Work *work,FVECTOR *x,SVECTOR *r,int mtname)
{
    OBJECT *body;
    DG_DEF *def;
    int i;
    DG_DEF **mdl_list=PreviewHumanOC.objchange_def;
    int mdl_list_size=PreviewHumanOC.objchange_def_size;

    PutWork(work);
    
    work->pos=*x;
    work->rot=*r;

    body=&(work->body);
    def=*mdl_list;

    if((work->oc=GM_InitObject2ForObjChange(body,def,OBJECT_FLAG))==NULL) return 0;

    work->mdl_list=mdl_list;
    work->mdl_list_size=mdl_list_size;

    for(i=0;i<HUMAN_MODEL_OBJN;i++){
		work->mdl_list_index[i]=-1;
    }

    GM_ConfigObjectLight(body,work->lights) ;
    GM_ConfigObjectMotion(body,1,mtname,MT_FLAG_HUMAN1);
    GM_ConfigObjectAction(body,0,0 /* action */,0,0xfffff,(int)(0.5f*60.0f)) ;

    work->objnum=0;
    work->parts_index=-1;
    work->disp_enable=0;

    return 1;
}

/* 初期化部メイン */
void *NewOCHumanPreview2_called(FVECTOR *x,SVECTOR *r,int mtname)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
	
		if(!GetResources_called(work,x,r,mtname)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


/* 資源を獲得 */
static int GetResources(Work *work)
{
    OBJECT *body;
    DG_DEF *def;
    int i;
    DG_DEF **mdl_list;
    int mdl_list_size=PreviewFile.objchange_filenames_size;
    int mtname;

    if(PreviewFile.mar_filenames_size<=0) return 0;

    mtname=GV_StrCode(*(PreviewFile.mar_filenames));

    if(mdl_list_size<=0) return 0;
    if((mdl_list=(DG_DEF **)GV_Malloc(sizeof(DG_DEF *)*mdl_list_size))==NULL) return 0;

    for(i=0;i<mdl_list_size;i++){
		*(mdl_list+i)
			=(DG_DEF *)GV_GetCache(GV_CacheID(GV_StrCode(*(PreviewFile.objchange_filenames+i)),'k'));
    }
    PreviewHumanOC.objchange_def=mdl_list;
    PreviewHumanOC.objchange_def_size=mdl_list_size;

    PutWork(work);

    work->pos=DG_ZeroVector;
    work->rot=DG_ZeroSVector;
    work->disp_enable=0;

    if(GCL_GetOption( 'x' )!=NULL){
		int buf[ 3 ];

		GCL_GetIV(GCL_NextStr(),buf);
		vu0_IV0toFV((IVECTOR *)buf,&(work->pos));
    } 
    if( GCL_GetOption( 'r' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV(GCL_NextStr(),buf);
		work->rot.vx=buf[0];
		work->rot.vy=buf[1];
		work->rot.vz=buf[2];
    }
    if( GCL_GetOption( 'm' ) != NULL ){
		mtname=GCL_GetNextInt();
    }
    if( GCL_GetOption( 'd' ) != NULL ){
		work->disp_enable=GCL_GetNextInt();
    }


    body=&(work->body);
    def=*mdl_list;

    if((work->oc=GM_InitObject2ForObjChange(body,def,OBJECT_FLAG))==NULL) return 0;

    work->mdl_list=mdl_list;
    work->mdl_list_size=mdl_list_size;

    for(i=0;i<HUMAN_MODEL_OBJN;i++){
		work->mdl_list_index[i]=-1;
    }

    GM_ConfigObjectLight(body,work->lights) ;
    GM_ConfigObjectMotion(body,1,mtname,MT_FLAG_HUMAN1);
    GM_ConfigObjectAction(body,0,0 /* action */,0,0xfffff,(int)(0.5f*60.0f)) ;

    work->objnum=0;
    work->parts_index=-1;

    return 1;
}

static void NullAct(Work *work)
{
    printf("Abort Human Objchange\n");
    GV_DestroyActor(work);
}

static void NullDie(Work *work)
{
}

/* 初期化部メイン */
void *NewOCHumanPreview2(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
		if(!GetResources(work)) GV_SetActor(&(work->actor),NullAct,NullDie);
		else GV_SetActor(&(work->actor),Act,Die);
		GV_ActorEX(&(work->actor));
    }
    return (void *)work ;
}
