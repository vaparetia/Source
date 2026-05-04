/*
	recmtn.c
		MTNの記録ACTOR

	2001/02/15 K.Kano
	$Id: recmtn.c,v 1.1.1.3 2002/11/19 11:43:33 Yoshizawa1 Exp $
 */


#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>
#include <string.h>

#include "gameheader.h"
#include "libutl.h"

#include "libfs.h"


typedef struct {
	GV_ACT actor;

	OBJECT *target;

	int rec_n_models;

	FVECTOR *rec_addr;
	int rec_frames;

	int finish_rec_flag;
	int key_rec_flag;

	FVECTOR *rec_point;
	int frame_count;

	int name;

	char output_file[0x80];
} Work;


typedef struct {
	int			flags ;			/* 各種フラグ */
	int			motion_length ;	/* モーションの有効フレーム数 */
	int			motion_joints ;	/* モーションの関節数 */
	int			motion_tick ;	/* ベースクロック値（１／３００単位） */
	int			move_size ;		/* 移動量のサイズ((motion_length+1)*16) */
	int			rots_size ;		/* 関節回転データのサイズ(((motion_length+1)*motion_joints)*16) */
	int			trans_size ;	/* 関節移動量データへのサイズ(((motion_length+1)*motion_joints)*16) */
	int			minfo_size ;	/* 移動キー情報のサイズ((motion_length+1)*4) */
	int			jinfo_size ;	/* 関節キー情報のサイズ((motion_length+1)*motion_joints)*16) */
	int			sound_size ;	/* サウンド情報のサイズ（将来の拡張用） */
	int			anime_size ;	/* アニメーションのサイズ（将来の拡張用） */
	int			pad[4] ;
	int			error_angle ;	/* 圧縮時のエラー許容角度（ＰＳ固定小数点角度） */
} MTN_FILE_HEADER ;


enum {
	REC_START=0,
	REC_STOP=1,
	REC_EXIT=2,
};


static FVECTOR *over32m_addr=(FVECTOR *)0x02000000;
static int over32m_rest_size=0x08000000-0x02000000;


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/*

  レコードフォーマット
  1フレーム分のフォーマット
  +0      FVECTOR  obj0の絶対クォータニオン
  +1      FVECTOR  obj0の絶対座標
  +2      FVECTOR  obj0の絶対クォータニオン
  +3      FVECTOR  obj0の絶対座標
              :           :
              :           :
  +n*2+0  FVECTOR  objnの絶対クォータニオン
  +n*2+1  FVECTOR  objnの絶対座標

  */


static int Record(Work *work)
{
	DG_OBJS *objs=work->target->objs;
	int i;

	if(work->frame_count>=work->rec_frames) return 0;

	for(i=0;i<work->rec_n_models;i++){
		FVECTOR q,v;

		MT_MatToQuat(&q,&(objs->objs[i].world));
		fpu_CopyVector(&v,(FVECTOR *)&(objs->objs[i].world.m[3][0]));

		fpu_CopyVector(work->rec_point+0,&q);
		fpu_CopyVector(work->rec_point+1,&v);

		work->rec_point+=2;
	}

	work->frame_count++;

	return 1;
}

static int Output_MTN_HEADER(Work *work,int fd)
{
	MTN_FILE_HEADER mh;

	mh.flags=0;
	mh.motion_length=work->frame_count;
	mh.motion_joints=work->rec_n_models;
	mh.motion_tick=300/60;
	mh.move_size=(mh.motion_length+1)*sizeof(FVECTOR);
	mh.rots_size=(mh.motion_length+1)*mh.motion_joints*sizeof(FVECTOR);
	mh.trans_size=(mh.motion_length+1)*mh.motion_joints*sizeof(FVECTOR);

	mh.minfo_size=0;	/* 移動キー情報のサイズ((motion_length+1)*4) */
	mh.jinfo_size=0;	/* 関節キー情報のサイズ((motion_length+1)*motion_joints)*16) */
	mh.sound_size=0;	/* サウンド情報のサイズ（将来の拡張用） */
	mh.anime_size=0;	/* アニメーションのサイズ（将来の拡張用） */
	mh.error_angle=0;	/* 圧縮時のエラー許容角度（ＰＳ固定小数点角度） */

	return (pcWrite(fd,&mh,sizeof(mh))>=0);
}

static int Output_MTN_BODY(Work *work,int fd)
{
	DG_DEF *def=work->target->objs->def;
	FVECTOR *p,*pp;
	FVECTOR base;
	int i,j;
	int count;

	count=0;

	/* MOVE */
	p=work->rec_addr+1;

	fpu_CopyVector(&base,p);

	for(i=-1;i<work->frame_count;i++){
		FVECTOR v;

		fpu_SubVectors(&v,p,&base);
#if 0
		if(pcWrite(fd,p,sizeof(FVECTOR))<0) return 0;
#else
		if(pcWrite(fd,&v,sizeof(FVECTOR))<0) return 0;
#endif
		if(i>=0) p+=work->rec_n_models*2;

#if 0
		/* 進行チェック用 */
		if((count & 0xf)==0){
			printf(".");
			fflush(stdout);
		}
#endif

		count++;
	}


	count=0;

	/* ROT */
	for(j=0;j<work->rec_n_models;j++){
		for(i=-1;i<work->frame_count;i++){
			p=work->rec_addr+(work->rec_n_models*i+j)*2;

			if(i<0){
				FVECTOR v={ 0.0f,0.0f,0.0f,1.0f, };
				if(pcWrite(fd,&v,sizeof(v))<0) return 0;
			}
			else{
				FVECTOR q,inv,parent;
				int i_parent=def->models[j].parent;

				if(i_parent<0){
					fpu_ClearVector(&parent);
					parent.vw=1.0f;
				}
				else{
					pp=work->rec_addr+(work->rec_n_models*i+i_parent)*2;
					fpu_CopyVector(&parent,pp);
				}

				MT_QuatInverse(&inv,&parent);
				MT_QuatMul(&q,&inv,p);

				if(pcWrite(fd,&q,sizeof(q))<0) return 0;

#if 0
				if((count & 0xf)==0){
					printf("%f %f %f\n",q.vx,q.vy,q.vz);
				}
#endif

			}

#if 0
			/* 進行チェック用 */
			if((count & 0xf)==0){
				printf("#");
				fflush(stdout);
			}
#endif
			count++;
		}
	}


	count=0;

	/* TRANS */
	for(j=0;j<work->rec_n_models;j++){
		for(i=-1;i<work->frame_count;i++){
			p=work->rec_addr+(work->rec_n_models*i+j)*2;

			if(i<0){
				FVECTOR v={ 0.0f,0.0f,0.0f,1.0f, };
				if(pcWrite(fd,&v,sizeof(v))<0) return 0;
			}
			else{
				int i_parent=def->models[j].parent;

				if(i_parent<0){
					FVECTOR v={ 0.0f,0.0f,0.0f,1.0f, };
					if(pcWrite(fd,&v,sizeof(v))<0) return 0;
				}
				else{
					FMATRIX inv,m;
					FVECTOR r,b,x;

					pp=work->rec_addr+(work->rec_n_models*i+i_parent)*2;

					MT_QuatToMat(&m,pp);
					fpu_CopyVector((FVECTOR *)&(m.m[3][0]),pp+1);
					m.m[3][3]=1.0f;

					FastInverseMatrix(&inv,&m);
					fpu_CopyVector(&x,p+1);
					x.vw=1.0f;

					fpu_MulMatrixVector(&r,&inv,&x);

					b.vx=def->models[j].tx;
					b.vy=def->models[j].ty;
					b.vz=def->models[j].tz;

					fpu_SubVectors(&r,&r,&b);

					r.vw=1.0f;

					if(pcWrite(fd,&r,sizeof(r))<0) return 0;

#if 0
					if((count & 0xf)==0){
						printf("m = %f %f %f %f\n",m.m[0][0],m.m[1][0],m.m[2][0],m.m[3][0]);
						printf("    %f %f %f %f\n",m.m[0][1],m.m[1][1],m.m[2][1],m.m[3][1]);
						printf("    %f %f %f %f\n",m.m[0][2],m.m[1][2],m.m[2][2],m.m[3][2]);
						printf("    %f %f %f %f\n",m.m[0][3],m.m[1][3],m.m[2][3],m.m[3][3]);
						printf("iv= %f %f %f %f\n",inv.m[0][0],inv.m[1][0],inv.m[2][0],inv.m[3][0]);
						printf("    %f %f %f %f\n",inv.m[0][1],inv.m[1][1],inv.m[2][1],inv.m[3][1]);
						printf("    %f %f %f %f\n",inv.m[0][2],inv.m[1][2],inv.m[2][2],inv.m[3][2]);
						printf("    %f %f %f %f\n",inv.m[0][3],inv.m[1][3],inv.m[2][3],inv.m[3][3]);
						printf("r = %f %f %f\n",r.vx,r.vy,r.vz);
						printf("b = %f %f %f\n",b.vx,b.vy,b.vz);
						printf("x = %f %f %f\n",x.vx,x.vy,x.vz);
					}
#endif
				}
			}

#if 0
			/* 進行チェック用 */
			if((count & 0xf)==0){
				printf("$");
				fflush(stdout);
			}
#endif
			count++;
		}
	}

	return 1;
}

static void Output(Work *work)
{
	char filename[0x100];
	int fd;

	printf("RECMTN : Output to %s\n",work->output_file);

	strcpy(filename,"host0:./");
	strcat(filename,work->output_file);

	if((fd=pcOpen(filename,SCE_WRONLY|SCE_TRUNC|SCE_CREAT))<0){
		printf("RECMTN : File could not be opened.\n");
		return;
	}

	if(!Output_MTN_HEADER(work,fd)){
		printf("RECMTN : failed to write.\n");
		pcClose(fd);
		return;
	}

	if(!Output_MTN_BODY(work,fd)){
		printf("RECMTN : failed to write.\n");
		pcClose(fd);
		return;
	}

	pcClose(fd);

	printf("RECMTN : finished to write.\n");
}

static void PrintInitLocate(Work *work)
{
	FVECTOR *p=work->rec_addr+1;
	printf("RECMTN : Init locate = ( %f %f %f )\n",p->vx,p->vy,p->vz);
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void WaitAct(Work *work);
static void Act(Work *work);
static void EndAct(Work *work);


static int keycheck(void)
{
	if((GV_PadData[1].status & (PAD_L1|PAD_L2)) &&
	   (GV_PadData[1].press & PAD_A)) return 1;
	return 0;
}

static void RecMain(Work *work)
{
	if(!Record(work)){
		Output(work);
		PrintInitLocate(work);

		if(work->finish_rec_flag){
			printf("RECMTN : Stop recording by Timeout\n");
			GV_ChangeActFunc(&(work->actor),WaitAct);
		}
		else{
			printf("RECMTN : Finished recording by Timeout\n");
			GV_ChangeActFunc(&(work->actor),EndAct);
		}
	}
}


static void WaitAct(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);

		switch(command){
		case REC_START:
			printf("RECMTN : Start recording\n");

			work->rec_point=work->rec_addr;
			work->frame_count=0;

			GV_ChangeActFunc(&(work->actor),Act);
			RecMain(work);
			return;

		case REC_EXIT:
			printf("RECMTN : Finished recording\n");
			GV_ChangeActFunc(&(work->actor),EndAct);
			return;
		}

		msg++;
		n_msg--;
    }

	if(work->key_rec_flag){
		if(keycheck()){
			printf("RECMTN : Start recording by Key\n");

			work->rec_point=work->rec_addr;
			work->frame_count=0;

			GV_ChangeActFunc(&(work->actor),Act);
			RecMain(work);
			return;
		}
	}
}

static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);

		switch(command){
		case REC_STOP:
			Record(work);
			Output(work);
			PrintInitLocate(work);

			if(work->finish_rec_flag){
				printf("RECMTN : Stop recording\n");
				GV_ChangeActFunc(&(work->actor),WaitAct);
			}
			else{
				GV_ChangeActFunc(&(work->actor),EndAct);
			}
			return;
		case REC_EXIT:
			printf("RECMTN : Finished recording\n");
			GV_ChangeActFunc(&(work->actor),EndAct);
			return;
		}

		msg++;
		n_msg--;
    }

	if(work->key_rec_flag){
		if(keycheck()){
			Record(work);
			Output(work);
			PrintInitLocate(work);

			if(work->finish_rec_flag){
				printf("RECMTN : Stop recording by Key\n");
				GV_ChangeActFunc(&(work->actor),WaitAct);
			}
			else{
				GV_ChangeActFunc(&(work->actor),EndAct);
			}
			return;
		}
	}

	RecMain(work);
}

static void EndAct(Work *work)
{
	GV_DestroyActor(&(work->actor));
}

static void Die(Work *work)
{
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
	work->target=NULL;
	work->rec_n_models=21;

	work->rec_addr=NULL;
	work->rec_frames=60*60;

	work->finish_rec_flag=0;
	work->key_rec_flag=0;

	strcpy(work->output_file,"recmtn.mtn");


	/* ターゲット名 */
	if( GCL_GetOption( 't' ) != NULL ){
		CONTROL *ctrl;
		int name;

		name=GCL_GetNextInt();
		ctrl=GM_SearchWhere(name);
		if(ctrl!=NULL){
			if(ctrl->object==NULL){
				work->target=(OBJECT *)(ctrl+1);
			}
			else{
				work->target=ctrl->object;
			}
			work->rec_n_models=work->target->objs->n_models;
		}
	}

	/* 取り出すOBJ数 */
	if( GCL_GetOption( 'm' ) != NULL ){
		work->rec_n_models=GCL_GetNextInt();
	}

	/* 出力ファイル名 */
	if( GCL_GetOption( 'o' ) != NULL ){
		char *file=GCL_GetNextString();
		strcpy(work->output_file,file);
	}

	/* 用意するメモリのサイズ */
	if( GCL_GetOption( 'f' ) != NULL ){
		work->rec_frames=GCL_GetNextInt();
	}

	/* 記録終了時の処理の指定 */
	if( GCL_GetOption( 'e' ) != NULL ){
		work->finish_rec_flag=GCL_GetNextInt();
	}

	/* キーによる記録スタート／ストップの指示 */
	if( GCL_GetOption( 'k' ) != NULL ){
		work->key_rec_flag=GCL_GetNextInt();
	}
}

/* 資源を獲得 */
static int GetResources(Work *work,int name)
{
    GetOptionValue(work);

	if(work->target==NULL) return 0;

	if(work->rec_n_models>work->target->objs->n_models){
		work->rec_n_models=work->target->objs->n_models;
	}

	{
		int size=work->rec_n_models*work->rec_frames*sizeof(FVECTOR)*2;

		if(size>over32m_rest_size) return 0;

		work->rec_addr=over32m_addr;

		over32m_addr=(FVECTOR *)((int)over32m_addr+size);
		over32m_rest_size-=size;
	}

    return 1;
}

/* 初期化部メイン */
void *NewRecmtn(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_EFFECT,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		GV_SetActor(&(work->actor),WaitAct,Die);

		if(!GetResources(work,name)){
			printf("RECMTN : Actor is not be started.\n");
			GV_SetActor(&(work->actor),EndAct,Die);
		}
    }
    return (void *)work;
}
