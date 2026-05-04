/*
	fhchg.c
		partchg.cを利用した、顔／手の切替え

	2000/03/28 K.Kano
	$Id: fhchg.h,v 1.1.1.3 2002/11/19 11:43:10 Yoshizawa1 Exp $
*/


#ifndef _fhchg_h_
#define _fhchg_h_


/* 顔／手の切替えを行なうアクターを起動する。
   nameは、アクターの名前を決定する。メッセージはその名前で受け取っている。
   humanは、切替える人間を指定。
   face_model,r_hand_model,l_hand_modelは、表示用の部分モデル(顔、右手、左手)のstrcode、
   cv2_models,r_cv2_models,l_cv2_modelsは、アニメーションパターン分のCV2ファイルのstrcodeの配列、
   cv2_size,r_cv2_size,l_cv2_sizeは、配列のサイズ。

   2000/5/31
   追加:
   新しくpartchg_evm.cを作ったので、そのための顔／手の切替えアクターを作成。
*/

/* メッセージでコマンドを入力。
   内容は、
   第一引数      第二引数                      第三引数

facechg.c:
   0             パターン番号                  フレーム数
   1             表示(1)/非表示(0)/反転(-1)

handchg.c:
右手:
   0             パターン番号                  フレーム数
   1             表示(1)/非表示(0)/反転(-1)
左手:
   2             パターン番号                  フレーム数
   3             表示(1)/非表示(0)/反転(-1)


2000/5/31 追加
facechg_evm.c:
   0             アニメーションファイルStrCode
   1             表示(1)/非表示(0)/反転(-1)

handchg_evm.c:
右手:
   0             アニメーションファイルStrCode
   1             表示(1)/非表示(0)/反転(-1)
左手:
   2             アニメーションファイルStrCode
   3             表示(1)/非表示(0)/反転(-1)

   */


void *NewFaceChange_called(int name,DG_OBJS *human,int face_model,int *cv2_models,int cv2_size);

void *NewHandsChange_called(int name,DG_OBJS *human,
			    int r_hand_model,int *r_cv2_models,int r_cv2_size,
			    int l_hand_model,int *l_cv2_models,int l_cv2_size);


void *NewFaceChange_called(int name,DG_OBJS *human,int face_model);

void *NewEvmHandsChange_called(int name,DG_OBJS *human,int r_hand_model,int l_hand_model);


#endif
