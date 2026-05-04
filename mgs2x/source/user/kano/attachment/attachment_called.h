/*
	attachment_called.h
		装備品揺らし

	1999/11/08 K.Kano
	$Id: attachment_called.h,v 1.1.1.3 2002/11/19 11:43:04 Yoshizawa1 Exp $
*/

#ifndef _attachment_called_h_
#define _attachment_called_h_

/*
//chara 装備品Ａ[NewAttachment] $s:名前 \
//	-n $s:モデル名 \
//	-y $s:つける相手の名前 \
//	-z $i:つける場所のOBJ番号 \
//	-x $v:ローカル座標上でのつける位置 \
//	-v $v:当たりを見る位置 \
//	-a $f:X軸可動の指定 $f:Y軸可動の指定 $f:Z軸可動の指定 \
//	-r $v:モデルの回転 \
//	-l $i:モデルをどの程度可動できるかを回転角で指定 \
//	-o $i:楕円球当たり判定用パラメータ
*/
void *NewAttachment_called(int model_name,FVECTOR *v,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,
						   int *a,int angle_limit,float oval_param);

/*
//chara 装備品Ｂ[NewAttachment2] $s:名前 \
//	-n $s:モデル名 \
//	-y $s:つける相手の名前 \
//	-z $i:つける場所のOBJ番号 \
//	-x $v:ローカル座標上でのつける位置 \
//	-q $i:回転決定に用いる位置のOBJ番号 \
//	-p $v:ローカル座標上での、回転決定に用いる位置 \
//	-v $v:回転決定のために見る位置 \
//	-r $v:モデルの回転
*/
void *NewAttachment2_called(int model_name,FVECTOR *v,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p);

/*
//chara 装備品Ｃ[NewAttachment3] $s:名前 \
//	-n $s:モデル名 \
//	-y $s:つける相手の名前 \
//	-z $i:つける場所のOBJ番号 \
//	-x $v:ローカル座標上でのつける位置 \
//	-r $v:モデルの回転 \
//	-f $i:遅れフレーム数
*/
void *NewAttachment3_called(int model_num,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int frames);


void *NewAttachment4_called(int model_name,FVECTOR *v,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int angle_limit,int frames);


/* 装備品サーバー用関数 */

/* 重力に引かれて落ちたり、動きに応じて跳ねたりする
   腰にぶら下げた携帯のような動き */
int MakeAttachment_called(int model_name,FVECTOR *v,SVECTOR *r,
						  OBJECT *target,int objnum,FVECTOR *x,
						  int *a,int angle_limit,float oval_param);
int MakeAttachmentV_called(int model_name,FVECTOR *v,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,
						   int *a,int angle_limit,float oval_param);

/* 相手の二箇所の点を参照して、それを結ぶように動く
   腰に付けたホルスターのような動き */
int MakeAttachment2_called(int model_name,FVECTOR *v,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p);
int MakeAttachment2V_called(int model_name,FVECTOR *v,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p);

/* 動きを遅らせる
   背中に付けたバックパックのような動き */
int MakeAttachment3_called(int model_name,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,int frames);
int MakeAttachment3V_called(int model_name,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int frames);

/* 動きを遅らせる
   回転のみに影響するので、動きが少ない */
int MakeAttachment3A_called(int model_name,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int frames);
int MakeAttachment3AV_called(int model_name,SVECTOR *r,
							 OBJECT *target,int objnum,FVECTOR *x,int frames);

/* 動きを遅らせる点と、動きに付いてくる点(原点)を結ぶように、モデルを動かす
   人に刺さったナイフのような動き */
int MakeAttachment4_called(int model_name,FVECTOR *v,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,int angle_limit,int frames);
int MakeAttachment4V_called(int model_name,FVECTOR *v,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int angle_limit,int frames);

/* 上記関数は全てIDを返します。
   装備品を消したい時は、下の関数にIDを指定して、呼び出して下さい */
void KillAttachment_called(int id);

/* IDのDG_OBJSを返します */
DG_OBJS	*GetAttachmentDGobjs(int id) ;

/* そのOBJにくっついている装備品を全て消します */
void SearchAndKillAttachment_called(OBJECT *obj);

/* 装備品のタイプも指定できるSearchAndKillAttachment_calledです。*/
void SearchAndKillAttachment_called2(int type,OBJECT *obj);

/* 装備品の消し方に、もう一つ落として消えるを加えました。
   この関数を呼び出すと、落ちて消えます。*/
void FallAttachment_called(int id,int func_type,int delay_count,int rnd_delay_count);

/* そのOBJにくっついている装備品を、落して消します。*/
void SearchAndFallAttachment_called(OBJECT *obj,int func_type,int delay_count,int rnd_delay_count);

/* 装備品のタイプも指定できるSearchAndFallAttachment_calledです。*/
void SearchAndFallAttachment_called2(int type,OBJECT *obj,int func_type,
									 int delay_count,int rnd_delay_count);

/* AttachmentのオブジェクトのIRフラグのON/OFF操作を行います */
void TurnOnOff_IR_called(int id,int onoff);
void SearchAndTurnOnOff_IR_called(OBJECT *obj,int onoff);


#endif
