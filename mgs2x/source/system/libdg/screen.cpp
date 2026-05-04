/*
	screen.c
	チャンネル処理ユニット／モデルスクリーン座標計算ルーチン

	1999/07/07 K.Takabe
	$Id: screen.cpp,v 1.1.1.3 2002/11/19 11:42:23 Yoshizawa1 Exp $
*/

// XBOX version /  by F.Miyauchi

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <windows.h>
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"

//-----------------------------------------------------------------------------

#ifdef DEBUG_MODE
static void PrintMatrix(char *str, MATRIX *mat)
{
	printf(str);
	printf("%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n",
		   mat->m[0][0], mat->m[0][1], mat->m[0][2], mat->m[0][3],
		   mat->m[1][0], mat->m[1][1], mat->m[1][2], mat->m[1][3],
		   mat->m[2][0], mat->m[2][1], mat->m[2][2], mat->m[2][3],
		   mat->m[3][0], mat->m[3][1], mat->m[3][2], mat->m[3][3]);
}
#endif

//-----------------------------------------------------------------------------

// バウンディングチェック用構造体
typedef struct tagDG_BOUNDTRANS {
	VECTOR min, max, trans;
} DG_BOUNDTRANS;

// スクラッチパッド
// TODO: スクラッチパッドまわりの処理
typedef	struct {
	MATRIX eye_pers;
	MATRIX eye_pers2;
	MATRIX eye_inv;

	VECTOR bound[8];
	VECTOR scale;
	VECTOR tmp_vec;
    float  screen; /* Ｚ値算出用補正定数 */
    int    sort_z; /* Ｗより求めたＺ値 */
    int    group_bound;
    int    invisible_flag;
    float  fog_param1, fog_param2;
    int    pad[2];

    MATRIX j_root; // objs->world
    MATRIX joints[DG_MAX_JOINTS];
    MATRIX j_root2;
    MATRIX tmp[4];

    //u_long128    local_work[0];
	u_long64 local_work[1024]; // 適当
} ScrPad;

#define	SCRPAD		((ScrPad *)SCRPAD_ADDR)
#define	EYE_PERS	(&(SCRPAD->eye_pers))
#define	EYE_PERS2	(&(SCRPAD->eye_pers2))
#define	J_ROOT		(&(SCRPAD->j_root))
#define	J_ROOT2		(&(SCRPAD->j_root2))
#define	JOINTS		(SCRPAD->joints)
#define BOUNDS		(SCRPAD->bound)
#define SCALE		(&SCRPAD->scale)
#define SCREEN		(SCRPAD->screen)
#define SORT_Z		(SCRPAD->sort_z)
#define GBOUND		(SCRPAD->group_bound)
#define	WORK_MAT	(&SCRPAD->tmp[0])
#define	WORK_MAT2	(&SCRPAD->tmp[1])


//-----------------------------------------------------------------------------
// バウンドチェックルーチン
//-----------------------------------------------------------------------------

#if 0
static inline u_int _RotTransPers( FVECTOR *v )
{
	u_int	ret ;
	/*
	  lqc2			vf8,0x00(%1)  ; v[0]
	  lqc2			vf9,0x10(%1)  ; v[1]
	  lqc2			vf10,0x20(%1) ; v[2]
	  lqc2			vf11,0x30(%1) ; v[3]
	  lqc2			vf1,0(%2)     ; scale;
	  vmulax.xyzw	ACC, vf4,vf8 ; 
	  vmadday.xyzw	ACC, vf5,vf8 ;
	  vmaddaz.xyzw	ACC, vf6,vf8 ;
	  vmaddw.xyzw	vf8, vf7,vf8 ; v[0] の変換結果→vf8
	  
	  vmulax.xyzw	ACC, vf4,vf9
	  vmadday.xyzw	ACC, vf5,vf9
	  vmaddaz.xyzw	ACC, vf6,vf9
	  vmaddw.xyzw	vf9, vf7,vf9 ; v[1] の変換結果→vf9
	  
	  vmulax.xyzw	ACC, vf4,vf10
	  vmadday.xyzw	ACC, vf5,vf10
	  vmaddaz.xyzw	ACC, vf6,vf10
	  vmaddw.xyzw	vf10, vf7,vf10 ; v[2] の変換結果→vf10
	  
	  vmulax.xyzw	ACC, vf4,vf11
	  vmadday.xyzw	ACC, vf5,vf11
	  vmaddaz.xyzw  ACC, vf6,vf11
	  vmaddw.xyzw	vf11, vf7,vf11 ; v[3] の変換結果→vf10

	  ; clipw の動作
	  ; if (x >   w) +x flag on
	  ; if (x < - w) -x flag on
	  ; if (y >   w) +y flag on
	  ; if (y < - w) -x flag on
	  ; if (z >   w) +z flag on
	  ; if (z < - w) -z flag on
	  
	  vclipw.xyz		vf8xyz,vf8w  ; xyz,wでクリッピング判定→CFに結果
	  vclipw.xyz		vf9xyz,vf9w  ; 4回分の結果が6bitづつCFに入る。
	  vclipw.xyz		vf10xyz,vf10w ;
	  vclipw.xyz		vf11xyz,vf11w ;

	  ; この辺は、わかりやすいように順番を入れ替えたので注意。そのままじゃ動きません。
	  vdiv			Q,vf0w,vf8w   ; Q = 1.0f / vf8w;
	  vmulq.xy		vf8,vf8,Q     ; vf8 = vf8 * Q
	  vmul.xy		vf8,vf8,vf1   ; vf8 * scale(x = w, y = h)

	  vdiv			Q,vf0w,vf9w   ; Q = 1.0f / vf9w;
	  vmulq.xy		vf9,vf9,Q     ; vf9 = vf9 * Q
	  vmul.xy		vf9,vf9,vf1   ; vf9 * scale

	  vdiv			Q,vf0w,vf10w
	  vmulq.xy		vf10,vf10,Q
	  vmul.xy		vf10,vf10,vf1 ; vf10 * scale;
	  
	  vdiv			Q,vf0w,vf11w  ; Q = vf0w / vf8w;
	  vmulq.xy		vf11,vf11,Q
	  vmul.xy		vf11,vf11,vf1 ; vf11 = vf11 * vf1
	  
	  cfc2			%0,$18              ; クリッピングフラグを ret に転送
	  sqc2			vf8,0x00(%1)        ;
	  sqc2			vf9,0x10(%1)
	  sqc2			vf10,0x20(%1)
	  sqc2			vf11,0x30(%1)
	  ": "&=r"(ret) : "r" (v),"r"(SCALE):"memory" );
	  return ( ret );
	*/
}
#endif

// 透視変換後の同時座標とクリッピングフラグを計算する
// 
// PS2版では VU1 プログラム。clipw 命令があるので話が早い…。
// とりあえず、アルゴリズムをそのまま再現してみる。

static u_long64 MakeBoundVerts(MATRIX *eye_pers, float *org, FVECTOR *verts)
{
	float   lx, ly, lz, ux, uy, uz;
	int     i, j;
	u_long64  flag = 0;
	float   w;
	
	// bound 座標が入ってくる
	lx = org[0];
	ly = org[1];
	lz = org[2];

	// XBOXでは要素を VECTOR にしてしまったので、org[3] は空き
	ux = org[4];
	uy = org[5];
	uz = org[6];

	for (i = 8; i > 0;) {
		for (j = 4; j > 0; j --, i --) {
			verts->vx = (i & 0x01) ? lx : ux;
			verts->vy = (i & 0x02) ? ly : uy;
			verts->vz = (i & 0x04) ? lz : uz;
			verts->vw = 1.0f;

			//if (GV_PadData[0].status & PAD_A) {
			//printf(" verts[%d][%d]  = %8f, %8f, %8f, %8f\n",
			//i, j, verts->vx, verts->vy, verts->vz, verts->vw);
			//}
			// 透視変換
			D3DXVec4Transform(verts, verts, eye_pers);

			flag <<= 6; // 最初1回のシフトは無駄

			// clipw 命令のエミュレート …
			w = DG_FABS(verts->vw);
			if (verts->vx >   w) flag |= CLIP_X0_FLAG;
			if (verts->vx < - w) flag |= CLIP_X1_FLAG;
			if (verts->vy >   w) flag |= CLIP_Y0_FLAG;
			if (verts->vy < - w) flag |= CLIP_Y1_FLAG;
			if (verts->vz >   w) flag |= CLIP_Z0_FLAG;
			if (verts->vz < - w) flag |= CLIP_Z1_FLAG;

			verts->vx *= SCALE->vx / verts->w;
			verts->vy *= SCALE->vy / verts->w;
			
			//if (GV_PadData[0].status & PAD_A) {
			//printf("pverts1[%d][%d] = %8f, %8f, %8f, %8f\n",
			//i, j, verts->vx, verts->vy, verts->vz, verts->vw);
			//}
			verts ++;
		}
	}
	return flag;
}

// バウンディングボックスと透視変換マトリクスからバウンディング結果を求める
static int BoundCheck(MATRIX *eye_pers, float *bound, int arg_flag)
{
	VECTOR    *verts;
	u_long64    flag, and_flag;
	int       i, bound_flag;
	float     z;

	bound_flag = 0;
	verts = BOUNDS; // SCRPAD上の VECTOR[8] 先頭
	flag = MakeBoundVerts(eye_pers, bound, verts);

	z = (verts[0].vw + verts[1].vw) * SCREEN;

	// フラグの累積チェック
	and_flag = flag;
	for (i = 7; i > 0; i --) {
		flag >>= 6;
		and_flag &= flag;
	}

	// XYクリップで完全に画面外
	if (and_flag & CLIP_XY_FLAG) return 2;

	// Ｚ距離のチェック
	and_flag |= CLIP_Z_FLAG;
	bound_flag = 0;

	for (i = 8; i > 0; i--) {
		// ニアクリップチェック
		if (verts->vw - verts->vz < 0) {
			//printf("vw = %f, vz = %f\nnear clip\n", verts->vw, verts->vz);
			bound_flag = 1;
		} else {
			and_flag &= ~0x10;
		}
		// ファークリップチェック
		if (verts->vz <= 0) {
			bound_flag = 1;
			//printf("vz = %f\far clip\n", verts->vz);
		} else {
			and_flag &= ~0x20;
		}

		// プリミティブ座標オーバーフラグ
		if (verts->vx < -2048.0f || verts->vx > 2047.0f
			|| verts->vy < -2048.0f || verts->vy > 2047.0f) {
			//printf("prim coords over\n");
			bound_flag = 1;
		}
		verts ++;
	}

	// Zクリップで完全に画面外
	if (and_flag & CLIP_Z_FLAG) return 2;

	SORT_Z = DG_FTOI(z);

	// XYZどれかが引っかかっているのでクリップ必要
	if (bound_flag) return 1;

	return 0; // 完全画面内
}

static int CheckSortZ(MATRIX *mat, VECTOR *bound)
{
	VECTOR vec;

	// バウンディングの中心点を変換する
	vec.vx = (bound[0].vx + bound[1].vx) * 0.5f;
	vec.vy = (bound[0].vy + bound[1].vy) * 0.5f;
	vec.vz = (bound[0].vz + bound[1].vz) * 0.5f;
	vec.vw = 1.0f;

	D3DXVec4Transform(BOUNDS, &vec, mat);

	// バウンディング中央値のz座標取得
	SORT_Z = DG_FTOI(BOUNDS[0].vw);
	return 0;
}


//-----------------------------------------------------------------------------

// エンベロープ用補正マトリクスの生成
//
// ※PS2版とは全く異なるマトリクスを返すので注意。
//   XBOX では、ジオメトリブレンディングを使って描画。
//   描画の際には、
//     WORLDMATRIX(0) = base
//     WORLDMATRIX(1) = offset_mat
//   として辻褄が合うようなoffset_matを返す。
static void GetEnvelopeMatrix(MATRIX *offset_mat,
							  MATRIX *base,
							  MATRIX *target,
							  float *trans)
{
	*WORK_MAT2 = *base;  // 自分
	*WORK_MAT = *target; // 親

	// 回転成分は親で、位置成分が自分であるような行列を返す
	*offset_mat = *target;
	offset_mat->m[3][0] = base->m[3][0];
	offset_mat->m[3][1] = base->m[3][1];
	offset_mat->m[3][2] = base->m[3][2];
	offset_mat->m[3][3] = base->m[3][3];	

#if 0 // PS2
	_CopyMatrix(WORK_MAT2, base);
	_CopyMatrix(WORK_MAT, target);
	// *WORK_MAT2 = *base;
	// *WORK_MAT = *target;
	WORK_MAT->m[3][0] = 0.0F; WORK_MAT->m[3][1] = 0.0F; WORK_MAT->m[3][2] = 0.0F; 
	WORK_MAT2->m[3][0] = 0.0F; WORK_MAT2->m[3][1] = 0.0F; WORK_MAT2->m[3][2] = 0.0F; 
	sceVu0InversMatrix(WORK_MAT2, WORK_MAT2);
	sceVu0MulMatrix(offset_mat, WORK_MAT2, WORK_MAT);
	sceVu0ApplyMatrix(&(SCRPAD)->tmp_vec, target, &(SCRPAD)->tmp_vec);
	sceVu0SubVector(offset_mat->m[3], &(SCRPAD)->tmp_vec, base->m[3]);
	sceVu0ApplyMatrix(offset_mat->m[3], WORK_MAT2, offset_mat->m[3]);
	/* (base)*(offset_mat)=(target) */
#endif
}


// 各ユニットのワールドマトリクスから
// スクリーンマトリクスを計算する
static void ScreenObjs(DG_OBJS *objs, int n_objs)
{
    MATRIX *joints;
    DG_OBJ *obj;
    int    i, flag, type;
	DG_BOUNDTRANS *boundtrans_top;
    ScrPad *work = (ScrPad *)SCRPAD_ADDR;

	boundtrans_top = (DG_BOUNDTRANS *)work->local_work;

    joints = JOINTS;
    obj = objs->objs;
    for (i = n_objs; i > 0; -- i) {
		memcpy(boundtrans_top, &obj->bound_min, sizeof(VECTOR) * 3);
        type = obj->mdl_type;

        // 拡張モデル(親にくっつく)処理
        if (type & DG_TYPE_EXTEND) {
            DG_OBJ *parent = &objs->objs[obj->parent];
            obj->inv_mat = parent->inv_mat;
            obj->world = parent->world;
            obj->screen = parent->screen;
            obj->bound_mode = parent->bound_mode;
            obj->sort_z = parent->sort_z;
            obj ++;
            continue;
        }

		// シングルウェイトエンベロープ用補正マトリクスを計算
		if (obj->model->parent != - 1) {
			GetEnvelopeMatrix(&obj->inv_mat,
							  joints,
							  JOINTS + obj->model->parent,
							  &obj->model->tx);
			//printf("inv_mat[%d]\n", i);
			//PrintMatrix("", &obj->inv_mat);
		}
		// バウンドチェック
		if (GBOUND == 1) {
			flag = BoundCheck(joints, (float *)&boundtrans_top->min, 0);
		} else {
			flag = GBOUND;
			if (type & DG_TYPE_TRANS || !(obj->flag & DG_FLAG_PAINT)) {
				CheckSortZ(joints, &boundtrans_top->min);
			}
		}

        // バウンディングフラグ設定
		// ０：クリップ必要なし、１：クリップ必要、２：完全画面外
		obj->bound_mode = flag;
		if (flag != 2) {
			D3DXMatrixMultiply(&obj->screen, joints, EYE_PERS2);
		}
        obj->sort_z = SORT_Z; // ソート値設定
    	obj ++;
		joints ++;
	}
}

// 関節なし一体型モデルについて処理する。
//
// 各ユニットのワールドマトリクス／スクリーンマトリクスは
// 全て物体そのものと同じになる
static void OnePieceObjs(DG_OBJS *objs, int n_objs)
{
	DG_OBJ *obj;
	int    i, n, last_objs, flag, type;
	ScrPad *work = (ScrPad *)SCRPAD_ADDR;
	DG_BOUNDTRANS *boundtrans_top;
	
	n = 0;

	/* トランスレーションとバウンディング情報をスクラッチパッドへコピーする */
	boundtrans_top = (DG_BOUNDTRANS *)work->local_work;

	last_objs = n_objs;

	D3DXMatrixMultiply(JOINTS, J_ROOT, EYE_PERS);
	D3DXMatrixMultiply(J_ROOT2, J_ROOT, EYE_PERS2);
	obj = objs->objs;
	for (i = n_objs; i > 0; -- i, obj ++, n --) {
		memcpy(boundtrans_top, &obj->bound_min, sizeof(VECTOR) * 3);

		obj->world = *J_ROOT;
		type = obj->mdl_type;
        // 拡張モデル(親にくっつく)処理
		if (type & DG_TYPE_EXTEND){
			DG_OBJ *parent = &objs->objs[obj->parent];
			obj->screen = parent->screen;
			obj->bound_mode = parent->bound_mode;
			obj->sort_z = parent->sort_z;
			continue;
		}

		if (GBOUND == 1) {
			flag = BoundCheck(JOINTS, (float *)&boundtrans_top->min, 0);
		} else {
			flag = GBOUND;
			if (type & DG_TYPE_TRANS || !(obj->flag & DG_FLAG_PAINT)) {
				CheckSortZ(JOINTS, &boundtrans_top->min);
			}
		}

#if 0
		{   // バウンディング表示用デバッグキャラ(毎フレーム死ぬようになっている)
			extern void *NewBoundingBoxView_1(
					FMATRIX *world, float *max, float *min, int color );
			switch (flag) {
			case 0:
				NewBoundingBoxView_1(J_ROOT, &obj->model->lx, &obj->model->ux, 0x0000ffff);
				break ;
			case 1:
				NewBoundingBoxView_1(J_ROOT, &obj->model->lx, &obj->model->ux, 0x008080ff);
				break;
			}
		}
#endif
		// バウンディングフラグ設定
		// ０：クリップ必要なし、１：クリップ必要、２：完全画面外
		obj->bound_mode = flag;
		if (flag != 2) obj->screen = *JOINTS; // 投影マトリクス設定
		obj->sort_z = SORT_Z;                 // ソート値設定
	}
}

// 横すべり関節型モデルについて処理
//   ※objs->movs は親ユニットからの相対位置
static void SlideFrameObjs(DG_OBJS *objs, int n_objs)
{
	MATRIX *joints, *parent;
	VECTOR *movs, tmp;
	DG_OBJ *obj;
	DG_MDL *model;
	int	   i;

	movs = objs->movs;
	joints = JOINTS;
	obj = objs->objs;
	for (i = n_objs; i > 0; -- i) {
		model = obj->model;
		parent = JOINTS + model->parent;
		//sceVu0ApplyMatrix(&tmp, parent, movs);
		D3DXVec4Transform(&tmp, movs, parent);
		*joints = *parent;
		joints->m[3][0] = tmp.vx;
		joints->m[3][1] = tmp.vy;
		joints->m[3][2] = tmp.vz;
		obj->world = *joints;
		movs ++;
		obj ++;
		joints ++;
	}
}

// 回転関節型モデルについて処理
static void JointFrameObjs(DG_OBJS *objs, int n_objs)
{
	int         i;
	DG_OBJ      *obj;
	DG_MDL      *mdl;
	MATRIX      *joints;
	VECTOR      *rots;

	obj = objs->objs;
	rots = objs->rots;
	if (rots == NULL) {
		printf("############ rots == NULL\n");
		return;
	}

	mdl = objs->def->models;
	joints = JOINTS;
	for (i = 0; i < objs->n_models; i ++, obj ++, rots ++, mdl ++){
		MATRIX matTmp;
		D3DXMatrixIdentity(WORK_MAT);
		// 各軸の回転
		D3DXMatrixRotationZ(&matTmp, rots->vz);
		D3DXMatrixMultiply(WORK_MAT, &matTmp, WORK_MAT);
		D3DXMatrixRotationY(&matTmp, rots->vy);
		D3DXMatrixMultiply(WORK_MAT, &matTmp, WORK_MAT);
		D3DXMatrixRotationX(&matTmp, rots->vx);
		D3DXMatrixMultiply(WORK_MAT, &matTmp, WORK_MAT);
		//sceVu0RotMatrixX(WORK_MAT, WORK_MAT, rots->vx);
		//sceVu0RotMatrixY(WORK_MAT, WORK_MAT, rots->vy);
		//sceVu0RotMatrixZ(WORK_MAT, WORK_MAT, rots->vz);
		// 移動
		WORK_MAT->m[3][0] = mdl->tx;
		WORK_MAT->m[3][1] = mdl->ty;
		WORK_MAT->m[3][2] = mdl->tz;
		WORK_MAT->m[3][3] = 1.0f;
		//sceVu0MulMatrix((void*)&joints[i], (void*)&joints[mdl->parent], (void*)WORK_MAT);
		//joints[i] = (D3DXMATRIX)*WORK_MAT * (D3DXMATRIX)joints[mdl->parent];
		// DirectXでは逆順
		D3DXMatrixMultiply(&joints[i], WORK_MAT, &joints[mdl->parent]);
		obj->world = joints[i];
	}
}


// オブジェクトのマトリクスをセットアップする
static void DG_SetObjsMatrix(DG_OBJS *objs)
{
	int     n_objs, n_joints;
	ScrPad  *scrpad = (ScrPad *)SCRPAD_ADDR;

	n_objs = objs->n_models;
	n_joints = objs->def->n_models;

	// 物体自体に親物体が指定されていた場合、
	// 親物体のワールド座標を使う
	if (objs->root != NULL) {
		objs->world = *(objs->root);
	}

	*J_ROOT = objs->world; // obj->parent が - 1 の時参照される
	if (objs->flag & DG_FLAG_ONEPIECE) {
		/* 原点オフセット付加 */
		J_ROOT->m[3][0] += objs->trans.vx;
		J_ROOT->m[3][1] += objs->trans.vy;
		J_ROOT->m[3][2] += objs->trans.vz;
	}

	// 全体バウンディングを行う
	D3DXMatrixMultiply(WORK_MAT, J_ROOT, EYE_PERS);
	GBOUND = BoundCheck(WORK_MAT, (float *)&objs->bound_min,
						(objs->flag & DG_FLAG_ONEPIECE) == 0);

	// モデル全体のバウンディングボックスを表示する
#if 0
	{
		extern void *NewBoundingBoxView_1(FMATRIX *world, float *max, float *min, int color);
		int color;

		switch (GBOUND) {
		case 0: color = (int)0x0000ffff; break;
		case 1: color = (int)0x000000ff; break;			
		default: color = (int)0x00ff0000; break;
		}
		NewBoundingBoxView_1(&objs->world,
							 (float *)&objs->bound_max,
							 (float *)&objs->bound_min,
							 color);
		if (0) {
			// GBOUND によってBGの色を変えてみる
			if (GBOUND == 2) {
				DG_SetBgColor(0x0000ff00);
			} else if (GBOUND == 1) {
				DG_SetBgColor(0x00008000);
			} else {
				DG_SetBgColor(0x00000000);
			}
		}
	}
#endif

	objs->bound_mode = GBOUND;
	objs->bound_mode = 0;

	// 処理フラグにしたがって、マトリクスのセットアップ
	if (DG_FLAG_ONEPIECE & objs->flag) {
		//printf("OnePieceObjs\n");
		OnePieceObjs(objs, n_objs);
	} else if (DG_FLAG_FINISHCALC & objs->flag) { // 計算済み
		if ((objs->flag & scrpad->invisible_flag)
			&& !(objs->flag & DG_FLAG_SHADOWMAKE)) return;
		// オブジェクトのマトリクスをまとめてスクラッチパッドへコピーする
		int i;
		MATRIX *mat = JOINTS;
		DG_OBJ *obj = objs->objs;

		for(i = n_objs; i > 0; i --, mat ++, obj ++) {
			*mat = obj->world;
			//printf("world[%d]\n", i);
			//PrintMatrix("", &obj->world);
		}
		ScreenObjs(objs, n_objs);
	} else {
		if (objs->rots != NULL) {
			JointFrameObjs(objs, n_joints);
		} else if (objs->movs != NULL) {
			printf("SlideFrameObjs\n");
			SlideFrameObjs(objs, n_joints);
		} else {
			OnePieceObjs(objs, n_objs);
			return;
		}
		ScreenObjs(objs, n_objs);
	}
}

#if 0
// オブジェクトのメモリをパケットメモリに書き出す
//  ※ XBOXでは不要
static void StoreMatrixObjs(DG_OBJS *objs)
{
	ScrPad	*scrpad = (ScrPad *)SCRPAD_ADDR;
	int i, n;
	DG_OBJ *obj;

	obj = objs->objs;
	n = objs->n_models;

	for (i = 0; i < n; i++, obj++) {
		if (!(objs->flag & DG_FLAG_NOFOG)) {
			float f;
			int   ifog;
			f = (float)obj->sort_z;
			DG_ADDA(scrpad->fog_param2, 0.0f);
			f = DG_MADD(scrpad->fog_param1, f);
			f = DG_MAX(f, 0.0f);
			f = DG_MIN(f, 255.0f);
			ifog = DG_FTOI(f);
			obj->fog = (ifog << 4) | 0x8000;
		} else {
			obj->fog = (255 << 4) | 0x8000;
		}
	}
}
#endif


// カメラに応じたマトリクスのセットアップを行う
void DG_ScreenChanl(DG_CHANL *cp, int which)
{
	DG_OBJS         **oque;
	DG_OBJ_QUEUE    *que;
	DG_OBJ_BUFFER   *obj_buff;
	ScrPad	*scrpad = (ScrPad *)SCRPAD_ADDR;
	int		i;

	que = cp->obj_queue;
	if (que == NULL) return;
	obj_buff = &que->objs_buffer;
	if (obj_buff->n_queue == 0) return;

	/*
	 * カメラ逆行列を、スクラッチパッドに複製しておく
	 * その際、ピクセルアスペクト比率を調整しておく
	 */
	*EYE_PERS = cp->eye_pers;
	*EYE_PERS2 = cp->eye_pers2;
	scrpad->eye_inv = cp->eye_inv;
	SCALE->vx = (float)cp->width / 2;
	SCALE->vy = (float)cp->height / 2;
	
	SCREEN = 0.5f;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num;
	scrpad->fog_param1 = DG_FogParam1;
	scrpad->fog_param2 = DG_FogParam2;

	/* マトリクスのセットアップ */
	oque = (DG_OBJS **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; -- i) {
		DG_SetObjsMatrix(*oque);
		oque ++;
	}

#if 0
	/* マトリクス情報のメモリへの書き出し
	 * ※ Xboxでは不要
	 */
	oque = (DG_OBJS **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; i--, oque ++) {
		DG_OBJS *objs = *oque;
		if (cp->chanl_num < 2) {
			if (objs->flag & scrpad->invisible_flag
				&& !(objs->flag & DG_FLAG_SHADOWMAKE)) continue;
		} else {
			if (objs->flag & scrpad->invisible_flag) continue;
		}
		if (objs->bound_mode == 2) continue;
		StoreMatrixObjs(*oque);
	}
#endif
}
