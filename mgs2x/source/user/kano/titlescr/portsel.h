/*
	portsel.h
		ポートセレクト

	2001/06/12 K.Kano
	$Id: portsel.h,v 1.1.1.3 2002/11/19 11:43:40 Yoshizawa1 Exp $
*/


#ifndef _portsel_h_
#define _portsel_h_


/* メモリーカードポートのチェック、ポートの選択、ファイルのチェックを
   処理する子アクターです。
   使用法:
     MCManの起動
	   NewMCMan(&mcman);
	 ポートセレクトの起動
	   NewPortsel(name,parent_name,l2d_handle,title_mode,start_mode,void *strman,int resname);
       // name        ... ポートセレクトアクターの名前
	   // parent_name ... 親のアクターの名前
	   // l2d_handle  ... 初期化済みのハンドル
	   // title_mode  ... 0 : DATALOAD , 1 : DATASAVE , 2 : PHOTOLOAD , 3 : PHOTOSAVE
	   // start_mode  ... 0 : 一回目の起動 , 1 : 二回目以降の起動
	   // strman      ... NewTextScreenControl()が返すワーク
	   // resname     ... 使用するリソースの名前

	 ポートセレクターの終了メッセージが送られてくるのを待つ
	   
	    GV_MSG *msg;
	    int n_msg ;

	    n_msg=GV_ReceiveMessage(work->name,&msg);

	    while(n_msg>0){
			int sender=*(msg->message+0);
			int arg=*(msg->message+1);
			int port=*(msg->message+2);

			if(sender==PORTSEL_NAME){
				switch(arg){
				case 1:
					// ポート番号の受取
					if(work->step==MCARD_FILE_CHECK){
						work->step=MCARD_FILE_SEL;
						work->sub_step=0;
						work->port_sel=port;
						work->start_mode=1;
					}
					break;
				case 2:
					// キャンセルされた
					work->ans=TITLE_MSG_MODECANCEL;
					GV_DestroyActor(work);
					break;
				}
			}

			msg++;
			n_msg--;
	    }
 */


#define SIGNAL_PORTSEL_OK		0x2000
#define SIGNAL_PORTSEL_CANCEL	0x2001
#define SIGNAL_PORTSEL_AGAIN	0x2002


/* portsel.c */
void *NewPortsel(int name,int parent_name,int l2d_handle,
				 int title_mode,int start_mode,void *strman,int resname,void* mcman);

void *NewPortsel2(int name,int parent_name,int l2d_handle,
				  int title_mode,int start_mode,void *strman,int resname,void* mcman);

void *NewPortselForCodec(int name,int parent_name,int l2d_handle,
						 int title_mode,int start_mode,void *strman,int resname,void* mcman);

/* 初期化部メイン */
void *NewPortselForPhotosave(int l2d_handle,SPR_OBJ **sprwork,int sprwork_size,void *strman, int currentMode);


#endif
