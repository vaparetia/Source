//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bgscr.c
		タイトル等のバックグランド画面のアニメーション

	2001/06/12 K.Kano
	$Id: bgscr.c,v 1.1.1.3 2002/11/19 11:43:36 Yoshizawa1 Exp $
*/


#include "bgscr.h"

#include "../../mode/node/node_bg.h"

#include "BP_TrophySystem.h"
#include "libfs.h"

#if defined(BP_VITA)
#include "gesture_vta.h"
#endif

typedef struct {
	GV_ACT_EX actor;
	int name;

	int alpha;
	int talpha;
	int count;

	SPR_OBJ *fade;
} Work;


#define DISP_CHANL		NODE_BG_CHANL
#define DISP_PRI		7


#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) \
	SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);

		// printf("Message = %d\n",command);

		switch(command){
		case BG_MSG_UP:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_UPPER,0);

#ifdef DEBUG_MODE
			printf("Frame Signal Up\n");
#endif

			break;
		case BG_MSG_DOWN:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_LOWER,0);

#ifdef DEBUG_MODE
			printf("Frame Signal Down\n");
#endif

			break;
		case BG_MSG_LEFT:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_LEFT,0);

#ifdef DEBUG_MODE
			printf("Frame Signal Left\n");
#endif

			break;
		case BG_MSG_RIGHT:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_RIGHT,0);

#ifdef DEBUG_MODE
			printf("Frame Signal Right\n");
#endif

			break;
		case BG_MSG_RIGHTUP:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_RIGHTUP,0);

#ifdef DEBUG_MODE
			printf("Frame Signal RightUp\n");
#endif

			break;
		case BG_MSG_LEFTDOWN:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_LEFTDOWN,0);

#ifdef DEBUG_MODE
			printf("Frame Signal LeftDown\n");
#endif

			break;
		case BG_MSG_STOP:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_STOP,0);

#ifdef DEBUG_MODE
			printf("Frame Signal Stop\n");
#endif

			break;
		case BG_MSG_DISAPPEAR:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_KILL,0);

#ifdef DEBUG_MODE
			printf("Frame Signal Disappear\n");
#endif

			break;
		case BG_MSG_FADE:
			{
				int talpha=*(msg->message+1);
				int count=*(msg->message+2);

				work->talpha=talpha;
				work->count=count;
			}
#ifdef DEBUG_MODE
			printf("BG Signal Fade\n");
#endif
			break;
		case BG_MSG_OPENGRD:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_OPENGRD,0);
#ifdef DEBUG_MODE
			printf("BG Signal Open Grd\n");
#endif
			break;
		case BG_MSG_CLOSEGRD:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_CLOSEGRD,0);
#ifdef DEBUG_MODE
			printf("BG Signal Close Grd\n");
#endif
			break;
		case BG_MSG_OPENPLMAP:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_OPENPLMAP,0);
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_OPENRAY,0);
#ifdef DEBUG_MODE
			printf("BG Signal Open PlantMap\n");
#endif
			break;
		case BG_MSG_CLOSEPLMAP:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_CLOSEPLMAP,0);
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_CLOSERAY,0);
#ifdef DEBUG_MODE
			printf("BG Signal Close PlantMap\n");
#endif
			break;
		case BG_MSG_CHGFACE:
			GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_CHGFACE,*(msg->message+1));
#ifdef DEBUG_MODE
			printf("BG Signal Change Raiden Face\n");
#endif
			break;



//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)

		case BG_MSG_GENZI:
			if ( BP_Area_EU() || BP_Area_JP() )
         {
				static const int genzi_signal[]={
					NODE_BG_SIGNAL_GZ_SHOW_MAIN,
					NODE_BG_SIGNAL_GZ_HIDE_MAIN,

					NODE_BG_SIGNAL_GZ_SHOW_NEWGAME,
					NODE_BG_SIGNAL_GZ_HIDE_NEWGAME,

					NODE_BG_SIGNAL_GZ_SHOW_QUEST,
					NODE_BG_SIGNAL_GZ_HIDE_QUEST,

					NODE_BG_SIGNAL_GZ_SHOW_DIF,
					NODE_BG_SIGNAL_GZ_HIDE_DIF,

					NODE_BG_SIGNAL_GZ_SHOW_OPTION,
					NODE_BG_SIGNAL_GZ_HIDE_OPTION,
				};
				int index=*(msg->message+1);

				GV_CallChildSignalFunc(work,genzi_signal[index],0);
			}
#ifdef DEBUG_MODE
			printf("BG Signal Genzi\n");
#endif
			break;

//#endif

		}

		msg++;
		n_msg--;
    }

    {
       int call_thunder = GV_PadDataDirect[0].press & PAD_L2;

#if defined(BP_VITA)
       call_thunder = GestureGetFrontTouchTapped(kGPR_Direct);
       GestureResetFrontTouchTappedStatus();
#endif

       if(call_thunder){
          if(work->count==0 && work->alpha==0){
             // Removed in MGS_2_and_3_Trophy_Info_110615rev.xls
             //BP_TrophySystem_UnlockTrophy( kTRP_TitleScreenGunshot );
             GV_CallChildSignalFunc(work,NODE_BG_SIGNAL_THUNDER,0);
          }
       }
    }

	if(work->count>0){
		work->alpha+=(work->talpha-work->alpha)/work->count;
		work->count--;

		SET_COLOR_2DPRIM2(work->fade,0,0,0,work->alpha);
	}
}

static void Die(Work *work)
{
	if(work->fade){
		SPR_Destroy_2D_Object(work->fade);
		work->fade=NULL;
	}
}


/* ------------------------------------------------------------------------ */


#if 0

/* オプションの評価 */
static void GetOptionValue(Work *work)
{
}

#endif


/* 資源を獲得 */
static int GetResources(Work *work)
{
	// DG_SetDrawEnv(DG_Chanl(DISP_CHANL),0,0,DRAW_WIDTH,DRAW_HEIGHT);

	/* フェードと同じプライオリティのOBJがBGの中にあるので、
	   それを回避するため、SPRライブラリ特有の癖を利用する。
	   すなわち、先に登録されたものが後に描かれることを利用し、
	   フェードが最も後で描画されるようにする。*/

	work->fade=SPR_Create_2D_Object(SP_SPRITE,DISP_CHANL,NULL);
	if(work->fade==NULL){
		printf("ERR!! Make2DObj[work->title_konami]\n");
		return 0;
	}
#if 0 //BP_PS2
//#ifdef PSX2
	SPR_SetPosSprite(work->fade, &(SPR_POS){ 0.0f, 0.0f});
#else
	SPR_SetPosSprite(work->fade, (SPR_POS*)&DG_ZeroVector );
#endif	
	SPR_SetSizeSprite(work->fade, 512.0f, 384.0f );
	work->fade->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->fade->head.flags |= SPR_FLAG_ALPHA;
	SPR_SetPriority(work->fade,DISP_PRI);
	SET_COLOR_2DPRIM2(work->fade,0,0,0,128);
	SPR_SHOW(work->fade);

	work->alpha=work->talpha=128;
	work->count=0;

    return 1;
}

/* 初期化部メイン */
void *NewTitleBackground(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}

		/* フェードを後描きにするために、フェードの登録より後に呼び出す */
		GV_SetActorChild(work,NewNodeBG());
    }
    return (void *)work ;
}
