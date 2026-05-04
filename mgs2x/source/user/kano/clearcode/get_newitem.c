//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	get_newitem.c
		アイテムの取得結果の表示

	2001/08/17  K.Kano
	$Id: get_newitem.c,v 1.1.1.3 2002/11/19 11:43:07 Yoshizawa1 Exp $
 */


#include "clearcode.h"

#include "font.h"

#include "BP_TrophySystem.h"
#include "BP_BuildDefines.h"
#include "BP_Font.h"

#include "libfs.h"

#define MAX_ITEMS			10
#define ITEM_NAME_LEN		30


typedef struct {
	GV_ACT_EX actor;
	int name;

	int end_proc;

	int busy_flag;
	int l2d_handle;
	int action_strcode;

	int step;
	int sub_step;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int start_flag;

	unsigned char item;

	int base_u,base_v;
	SPR_OBJ *item_name[ITEM_NAME_LEN];

	void *strman;
	int width;

#if 0
	struct {
		int part_code;
		int key0_code;
		int key1_code;
		float p;
		int count;
	} key_anim[ANIM_WORK_SIZE];
#endif

} Work;


#define REPEAT_FIRST		20
#define REPEAT_NEXT			1


#define GETITEM_RESOURCE		0x00219bb2		/* アイテム取得時メッセージ */

#define CODE_MASK_TOP			0x00a4123a		/* blk_mask_top */


#if 0
#define FONT_WIDTH				24
#define FONT_HEIGHT				24
#define LINE_SPACE				12
#else
#define FONT_WIDTH				FONT_SIZE_W
#define FONT_HEIGHT				FONT_SIZE_H
#define LINE_SPACE				RUBI_SIZE_H
#endif
#define LINE_HEIGHT				(FONT_HEIGHT+LINE_SPACE)

#define FONT_DISP_WIDTH			16
#define FONT_DISP_HEIGHT		16

#define RES_GETITEM_U			0
#define RES_GETITEM_V			0
#define RES_GETITEM_WIDTH		(FONT_WIDTH*30)
#if 0
#define RES_GETITEM_HEIGHT		(FONT_HEIGHT*3)
#else
#define RES_GETITEM_HEIGHT		(LINE_HEIGHT*3+DOWN_MARGINE)
#endif
#define RES_GETITEM_X			56

#if 0
#  ifdef JAPANESE_BP_IGNORE()
#    define RES_GETITEM_Y			330
#  endif

#  ifdef ENGLISH
#    define RES_GETITEM_Y			310
#  endif
#endif

#define RES_GETITEM_Y         ( BP_Area_JP() ? 330 : 310 )

#if 0
#define RES_GETITEM_W			(FONT_DISP_WIDTH*30)
#define RES_GETITEM_H			(FONT_DISP_HEIGHT*3)
#else
#define RES_GETITEM_W			(RES_GETITEM_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define RES_GETITEM_H			(RES_GETITEM_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define TEXT_R				(160/2)
#define TEXT_G				(180/2)
#define TEXT_B				(170/2)
#define TEXT_A				128

#define TEXTCOLOR			(TEXT_R | (TEXT_G<<8) | (TEXT_B<<16) | (0x80<<24))


#define NODE_FONT_A_U		18
#define NODE_FONT_A_V		14

#define NODE_FONT_WIDTH		18
#define NODE_FONT_HEIGHT	14

#define NODE_FONT_DW		16
#define NODE_FONT_DH		12

#define ITEM_NAME_STR_X		130
#define ITEM_NAME_STR_Y		224


static const int item_code[][2]={
	{ CODE_OPEN_ITEM_BANDANA, CODE_CLOSE_ITEM_BANDANA, },
	{ CODE_OPEN_ITEM_CAMERA,  CODE_CLOSE_ITEM_CAMERA,  },
	{ CODE_OPEN_ITEM_CAMERA,  CODE_CLOSE_ITEM_CAMERA,  },
	{ CODE_OPEN_ITEM_CAMERA,  CODE_CLOSE_ITEM_CAMERA,  },
	{ CODE_OPEN_ITEM_STELTH,  CODE_CLOSE_ITEM_STELTH,  },
	{ CODE_OPEN_ITEM_STELTH,  CODE_CLOSE_ITEM_STELTH,  },
	{ CODE_OPEN_ITEM_WIG3,    CODE_CLOSE_ITEM_WIG3,    },
	{ CODE_OPEN_ITEM_WIG1,    CODE_CLOSE_ITEM_WIG1,    },
	{ CODE_OPEN_ITEM_WIG2,    CODE_CLOSE_ITEM_WIG2,    },
};

static const char *item_names[]={
	"BANDANA",
	"DIGITAL CAMERA",
	"DIGITAL CAMERA",
	"DIGITAL CAMERA",
	"STEALTH",
	"STEALTH",
	"WIG (BROWN)",
	"WIG (ORANGE)",
	"WIG (BLUE)",
};


/* ------------------------------------------------------------------------ */


static void ProgMessage(Work *work)
{

#if 0
    GV_MSG *msg;
    int n_msg;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);

		msg++;
		n_msg--;
    }
#endif

}

static void Key(Work *work)
{
	work->key_press=GV_PadDataDirect[0].press;

	if(GV_PadDataDirect[0].status && GV_PadDataDirect[0].status==work->key_status){
		if(work->key_count>REPEAT_FIRST){
			work->key_count-=REPEAT_NEXT;
			work->key_autostatus=work->key_status;
		}
		else work->key_autostatus=0;

		work->key_count++;
	}
	else{
		work->key_autostatus=work->key_status=GV_PadDataDirect[0].status;
		work->key_count=0;
	}
}


static void SetFont(Work *work)
{
	char *str;
	int i;
	int len;
	float d;

	str=(char *)(item_names[work->item]);

	SK_PrintfNormal(NULL,work->item_name,work->base_u,work->base_v,
					NODE_FONT_WIDTH,NODE_FONT_HEIGHT,ITEM_NAME_LEN);

	for(i=0;i<ITEM_NAME_LEN;i++){
		SPR_OBJ *spr=work->item_name[i];

		spr->sprite.head.tex.w=SPR_FIXED(NODE_FONT_WIDTH);
		spr->sprite.head.tex.h=SPR_FIXED(NODE_FONT_HEIGHT);
		spr->sprite.head.tex.pw=SPR_FIXED(NODE_FONT_WIDTH);
		spr->sprite.head.tex.ph=SPR_FIXED(NODE_FONT_HEIGHT);

		spr->sprite.pos.x=0;
		spr->sprite.pos.y=ITEM_NAME_STR_Y;
		spr->sprite.dw=NODE_FONT_DW;
		spr->sprite.dh=NODE_FONT_DH;
	}

	SK_PrintfNormal(str,work->item_name,work->base_u,work->base_v,
					NODE_FONT_WIDTH,NODE_FONT_HEIGHT,ITEM_NAME_LEN);

	/* センタリング */
	len=strlen(str);
	d=SPR_SCRN_WIDTH/2.0f
		-((work->item_name[len-1]->sprite.pos.x+NODE_FONT_DW)+
		  work->item_name[0]->sprite.pos.x)/2.0f;

	for(i=0;i<ITEM_NAME_LEN;i++){
		SPR_OBJ *spr=work->item_name[i];
		spr->sprite.pos.x+=d;
	}

	SK_PrintfChengColor2(work->item_name,TEXT_R,TEXT_G,TEXT_B,TEXT_A,ITEM_NAME_LEN);

	SK_AllShow(work->item_name,ITEM_NAME_LEN);
}

static void HideFont(Work *work)
{
	SK_AllHide(work->item_name,ITEM_NAME_LEN);
}

static void CreateString(Work *work)
{
	char *str;

	str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(GETITEM_RESOURCE,work->item));

	MENU_ClearTextTexture(work->strman);

	work->width=MENU_CreateTextTexture(work->strman,
									   RES_GETITEM_U,RES_GETITEM_V,
									   RES_GETITEM_WIDTH,RES_GETITEM_HEIGHT,0,LINE_SPACE,0,str);
}

static void DispString(Work *work)
{
	int x;

	/* センターリング */
	x = SPR_SCRN_WIDTH/2-(work->width*FONT_DISP_WIDTH/FONT_WIDTH)/2;/* menupriは仮想座標 */

	MENU_PutTextScreen(work->strman,x,RES_GETITEM_Y,
					   x+RES_GETITEM_W,RES_GETITEM_Y+RES_GETITEM_H,
					   RES_GETITEM_U,RES_GETITEM_V,
					   RES_GETITEM_U+RES_GETITEM_WIDTH,
					   RES_GETITEM_V+RES_GETITEM_HEIGHT,
					   TEXTCOLOR);
}

static int InitSubStep(Work *work)
{
	SPR_OBJ *font;
	SPR_OBJ *mask;
	int i;

	if(work->busy_flag) return 0;

	font=L2D_GetObject(work->l2d_handle,CODE_NODE_FONT);

#if 0 //BP_PS2 def PSX2
	work->base_u=font->sprite.head.tex.u;
	work->base_v=font->sprite.head.tex.v;
#else
	work->base_u= (int)( font->sprite.head.tex.u * 16.0f );
	work->base_v= (int)( font->sprite.head.tex.v * 16.0f );
#endif

	for(i=0;i<ITEM_NAME_LEN;i++){
		work->item_name[i]=SPR_DuplicateTree(font);
	}

	mask=L2D_GetObject(work->l2d_handle,CODE_MASK_TOP);
	if(mask!=NULL) mask->sprite.col.a=0;

	return 1;
}

static int NewItemSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		work->action_strcode=CODE_GET_NEW_ITEM;
		work->sub_step++;

		SE_GET_NEWITEM();
		break;
	case 1:
		if(work->busy_flag) break;
		return 1;
	}
	return 0;
}

static int OpenItemSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		work->action_strcode=item_code[work->item][0];
		work->sub_step++;

		SE_WINOPEN();
		break;
	case 1:
		if(work->busy_flag) break;
		return 1;
	}
	return 0;
}

static int CloseItemSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		work->action_strcode=item_code[work->item][1];
		work->sub_step++;

		SE_WINCLOSE();
		break;
	case 1:
		if(work->busy_flag) break;
		return 1;
	}
	return 0;
}

static void Kill(Work *work)
{
	GV_DestroyActor(work);

	if(work->strman!=NULL){
		GV_DestroyOtherActor(work->strman);
		work->strman=NULL;
	}

	if(work->end_proc!=0 && work->end_proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		GCL_ExecProc(work->end_proc,&arg);
	}
}

static void Step(Work *work)
{
	switch(work->step){
	case 0:
		if(InitSubStep(work)){
			if(work->start_flag) work->step++;
			else work->step+=2;
			work->sub_step=0;
		}
		break;
	case 1:
		if(NewItemSubStep(work)){
			work->step++;
			work->sub_step=0;
		}
		break;
	case 2:
		if(OpenItemSubStep(work)){
			work->step++;
			work->sub_step=0;

			CreateString(work);
		}
		break;
	case 3:
		DispString(work);

		if(work->key_press & PAD_OK){
			work->step++;
			work->sub_step=0;

			HideFont(work);
		}
		break;
	case 4:
		if(CloseItemSubStep(work)){
			work->step++;
			work->sub_step=0;
		}
		break;
	case 5:
		Kill(work);
		break;
	}
}


/* ------------------------------------------------------------------------ */


static void Act(Work *work)
{
	if(work->l2d_handle>=0){
		if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
			work->busy_flag=0;
			if(work->action_strcode!=0){
				int stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);

#ifdef DEBUG_MODE
				printf("L2D Stat = %d\n",stat);
#endif

				work->action_strcode=0;
				work->busy_flag=1;
			}
		}
		else{
			work->busy_flag=1;
		}

		// KeyAnim(work);
	}
	else{
		work->busy_flag=0;
	}


	ProgMessage(work);

	Key(work);
	Step(work);
}

static void Die(Work *work)
{
	if(work->strman!=NULL){
		GV_DestroyOtherActor(work->strman);
		work->strman=NULL;
	}
	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}
}


/* ------------------------------------------------------------------------ */


static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_SHOW_ITEM_NAME:
		SetFont(work);
		break;
	case CODE_HIDE_ITEM_NAME:
		HideFont(work);
		break;
	}
}

/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->l2d_handle=-1;
	work->end_proc=0;
	work->start_flag=0;

	work->item=0;

    if(GCL_GetOption('p')!=NULL){
		work->end_proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('i')!=NULL){
		work->item=GCL_GetNextInt();
    }
    if(GCL_GetOption('s')!=NULL){
		work->start_flag=GCL_GetNextInt();
    }
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);


	work->l2d_handle=L2D_LoadLayout(CODE_L2D_CLEAR_CODE,DISP_CHANL,DISP_PRIORITY,0);

	if(work->l2d_handle<0){
#ifdef DEBUG_MODE
		printf("GetNewItem : L2D Init Error = %d\n",work->l2d_handle);
#endif

		ASSERT(0);
	}
	else{
		/* シグナルハンドラの設定 */
		L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);

		/* layoutの初期化 */
		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}

    return 1;
}

static void InitMenu(Work *work)
{
	// int i;

	work->step=0;
	work->sub_step=0;

#if 0
	for(i=0;i<ANIM_WORK_SIZE;i++){
		work->key_anim[i].count=0;
	}
#endif

   //see ending.gcl, following line 172 for these cases
   switch( work->item )
   {
   case 0: //infinite ammo bandana
      BP_TrophySystem_UnlockTrophy( kTRP_GotBandana );
      break;
   case 1: //tanker digital camera
   case 2: //plant digital camera
      BP_TrophySystem_UnlockTrophy( kTRP_GotDigitalCamera );
      break;
   case 4: //tanker stealth
      BP_TrophySystem_UnlockTrophy( kTRP_GotStealthCamo1 );
      break;
   case 5: //plant stealth
      BP_TrophySystem_UnlockTrophy( kTRP_GotStealthCamo2 );
      break;
   case 6: //brown wig
      BP_TrophySystem_UnlockTrophy( kTRP_GotBrownWig );
      break;
   case 7: //orange wig
      BP_TrophySystem_UnlockTrophy( kTRP_GotOrangeWig );
      break;
   case 8: //blue wig
      BP_TrophySystem_UnlockTrophy( kTRP_GotBlueWig );
      break;
   }
}

/* 初期化部メイン */
void *NewGetNewItemScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->busy_flag=0;
		work->action_strcode=0;

		work->strman=NULL;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL ;
		}
		if((work->strman=NewTextScreenControl())==NULL){
			GV_DestroyActor(work);
			return NULL;
		}

		InitMenu(work);

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}
