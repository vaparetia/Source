/*
	waving_cloth_called.h
		布シミュレート
		モデルが全て布モデルであるようなケース

	1999/11/18 K.Kano
	$Id: waving_cloth_called.h,v 1.1.1.3 2002/11/19 11:43:45 Yoshizawa1 Exp $
*/

#ifndef _waving_cloth_called_h_
#define _waving_cloth_called_h_


//chara 布モデル[NewWavingClothModel] $s:名前 \
//	-n $s:モデル名 \
//	-x $v:出現位置 \
//	-r $v:モデルの回転 \
//	-d $i:サンプルデータ \

void *NewWavingClothModel_called(int model_name,int sample_num,FVECTOR *x,SVECTOR *r);

void *NewWavingClothModel_withoutDATAFILE_called(int model_name,int sample_num,FVECTOR *x,SVECTOR *r);


//chara オブジェに付く布モデル[NewWavingClothModelW] $s:名前 \
//	-n $i:モデル数 $s:モデル名 ... \
//	-x $v:出現位置 \
//	-r $v:モデルの回転 \
//	-y $s:付ける相手の名前 \
//	-z $i:付ける場所のOBJ番号 \
//	-b $s:バウンディングボックスを使った当たり判定に使うモデル \
//	-d $i:サンプルデータ \
//	-p $i:風の影響の度合 \
//	-k $i:速度対する抵抗値 \
//	-u $i:バネ定数 \
//	-s $i:元位置に戻るためのバネ定数 \
//	-t $i:可動範囲係数 \
//	-o $i:楕円球当たり判定用パラメータ \
//	-v // 強制表示フラグ

void *NewWavingClothModelW_called(int address,int *model_name,int model_name_size,int sample_num,
				  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
				  int boundmodel_name,float oval_param,int force_disp_flag);

void *NewWavingClothModelW_withoutDATAFILE_called(int address,int *model_name,int model_name_size,
						  int sample_num,
						  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
						  int boundmodel_name,float oval_param,
						  int force_disp_flag);


#endif
