/*
	pshade.c
	プレシェイド（ＲＧＢ前処理計算）ルーチン

	1999/07/07 K.Takabe
	$Id: pshade.cpp,v 1.1.1.3 2002/11/19 11:42:22 Yoshizawa1 Exp $

*/
/*
	プレシェイド（ＲＧＢ前処理計算）ルーチン

	int		DG_MakePreshade(objs, lights, n_lights)
	DG_OBJS		*objs;		物体ハンドラ
	DG_LIT		*lights;	光源配列
	int		n_lights;	光源数

		プレシェイドデータを計算
		プレシェイド用バッファがなければ確保する

	void		DG_FreePreshade(objs)
	DG_OBJS		*objs;		物体ハンドラ

		プレシェイド用バッファを解放する

*/

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h" 

#define NO_CACHED(a) a

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

//-----------------------------------------------------------------------------

typedef struct {
	VECTOR          w_vert;
	VECTOR          w_vec;
	VECTOR          w_param;
	VECTOR          w_tmpvec;
	VECTOR          w_norm;
	int             n_point;
	int             n_spot;
	int             n_line;
	int             n_black;
	int             hit_flag;
	LIT_POINT       *points;
	LIT_SPOT        *spot;
	LIT_LINE        *line;
	LIT_BLACKPOINT  *blackpoints;
	int             pad[3];
	LIT_POINT       buffer[1024]; // 光源をコピーするのに十分なサイズを確保する
} ScrpadWork;

//-----------------------------------------------------------------------------

/* unsinged char*4 から int*4 へ直接変換 */
static inline int UCVtoIVim(int cv) {
}
/* short*4 から int*4 へ直接変換 */
static inline int SVtoIVim(int sv) {
}
/* int*4 から short*4 へ直接変換 */
static inline int IVtoSVim(int iv) {
}

/* ベクトルの長さを求める */
#define VEC_LEN(_x,_y,_z) (bp_sqrtf((_x) * (_x) + (_y) * (_y) + (_z) * (_z))) //BP_MATH - emulate PS2 sqrtf

/* 絶対値を取る */
#define FABS(_x) ((_x) > 0.0f ? (_x) : - (_x))

#define MAX(_x, _y) ((_x) > (_y) ? (_x) : (_y))
#define MIN(_x, _y) ((_x) > (_y) ? (_y) : (_x))

//-----------------------------------------------------------------------------

static MATRIX pshade_mat; // vf4～vf7
static VECTOR bound_max;  // vf9
static VECTOR bound_min;  // vf10

// プレシェイドに使う行列を設定する
static inline void _SetMatrix(MATRIX *m)
{
	pshade_mat = *m;
}

// デフォルトのバウンド最大/最小を設定する
static inline void _SetDefaultBound(VECTOR *max, VECTOR *min)
{
	bound_max = *max;
	bound_min = *min;
}

// 計算済みのバウンド最大値/最小値を取得する
static inline void _GetBound(VECTOR *max, VECTOR *min)
{
	*max = bound_max;
	*min = bound_min;
}

// _SetMatrixでセットされた行列を使って v を変換する
static inline void _RotTransBoundCheck(VECTOR *v)
{
	VECTOR vecTmp;
	D3DXVec4Transform(&vecTmp, v, &pshade_mat); // 変換

	bound_max.x = MAX(bound_max.x, vecTmp.x);
	bound_max.y = MAX(bound_max.y, vecTmp.y);
	bound_max.z = MAX(bound_max.z, vecTmp.z);	

	bound_min.x = MIN(bound_min.x, vecTmp.x);
	bound_min.y = MIN(bound_min.y, vecTmp.y);
	bound_min.z = MIN(bound_min.z, vecTmp.z);
}

//-----------------------------------------------------------------------------

// 各頂点についての照明データを計算する。
//
// 処理の流れは以下の通り。
// ・DG_LightMatrix, DG_ColorMatrixに設定されている
//   平行光源とアンビエントから頂点色(color)を求める
// ・点光源の影響を color に加える
// ・スポット光源の影響を color に加える
// ・線光源の影響を color に加える
// ・黒点光源の影響を color に加える
// ・color の値を SVECTOR rgbs に返す
//
// とりあえず素直に実装。重そうだなぁ…。
static SVECTOR* MakeLight(SVECTOR *rgbs, SVECTOR *verts, SVECTOR *norms)
{
	VECTOR  one = {1.0f,    1.0f, 1.0f, 255.0f};
	VECTOR  two = {1.5f, - 0.75f, 0.0f,   0.0f};
	VECTOR  color;
	LIT_POINT      *lit;
	LIT_SPOT       *spot;
	LIT_LINE       *line;
	LIT_BLACKPOINT *black;
	int     i, count;
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR;
	
	// 頂点をpshade_matで変換 -> work->w_vert;
	VECTOR vecTmp;
	vecTmp.vx = (float)verts->vx;
	vecTmp.vy = (float)verts->vy;
	vecTmp.vz = (float)verts->vz;
	vecTmp.vw = 1.0f;
	D3DXVec4Transform(&work->w_vert, &vecTmp, &pshade_mat);
	
	// 頂点をpshade_matの回転成分のみで変換＆正規化-> work->w_norm
	vecTmp.vx = (float)norms->vx;
	vecTmp.vy = (float)norms->vy;
	vecTmp.vz = (float)norms->vz;
	vecTmp.vw = 0.0f;
	D3DXVec4Transform(&vecTmp, &vecTmp, &pshade_mat);
	D3DXVec3Normalize(&work->w_norm, &vecTmp);

	VECTOR vecLight; // 平行光源方向
	VECTOR vecColor; // 平行光源色

	vecLight.vx = DG_LightMatrix.m[0][0]; // DG_LightMatrixを参照
	vecLight.vy = DG_LightMatrix.m[1][0];
	vecLight.vz = DG_LightMatrix.m[2][0];
	vecLight.vw = 1.0f;

	vecColor.vx = DG_ColorMatrix.m[0][0] * 256.0f; // DG_ColorMatrixを参照
	vecColor.vy = DG_ColorMatrix.m[0][1] * 256.0f;
	vecColor.vz = DG_ColorMatrix.m[0][2] * 256.0f;
	vecColor.vw = 1.0f;
	
	float eff; // 光源影響度 = 平行光源ベクトル・法線ベクトル
	eff = D3DXVec3Dot(&vecLight, &work->w_norm);
	if (eff < 0.0f) {
		eff = 0.0f;
	}

	// デフォルト頂点色 = アンビエント色 + eff * 平行光源色
	color.vx = DG_ColorMatrix.m[3][0] * 256.0f + eff * vecColor.vx;
	color.vy = DG_ColorMatrix.m[3][1] * 256.0f + eff * vecColor.vx;
	color.vz = DG_ColorMatrix.m[3][2] * 256.0f + eff * vecColor.vx;
	color.vw = 1.0f;

	if (color.vx > 255.0f) color.vx = 255.0f;
	if (color.vy > 255.0f) color.vy = 255.0f;
	if (color.vz > 255.0f) color.vz = 255.0f;

	count = 0;

	// 点光源
	lit = work->points;	
	for (i = work->n_point; i > 0; -- i, lit ++) {
		float f, r, len, scale, r_range, force;
		r = lit->e_range; // 点光源の影響範囲

		// 影響範囲に入っているか簡単にチェック
		f = work->w_vert.vx - lit->point.vx;
		if (FABS(f) > r) continue;
		work->w_vec.vx = f;		
		f = work->w_vert.vy - lit->point.vy;
		if (FABS(f) > r) continue;
		work->w_vec.vy = f;		
		f = work->w_vert.vz - lit->point.vz;
		if (FABS(f) > r) continue;
		work->w_vec.vz = f;		

		// 点光源パラメータ
		//work->w_param.vy = lit->r_range * 2;
		r_range = lit->r_range * 2;
		len = D3DXVec3Length(&work->w_vec);
		if (r_range - len <= 0.0f) {
			// 有効範囲内に入っていなければ終了
			continue;
		}
		D3DXVec3Normalize(&work->w_vec, &work->w_vec);
		force = D3DXVec3Dot(&work->w_vec, &work->w_norm);
		if (force > 0.0f) {
			scale = 1.5f * force * (r_range - len) / r_range;
			color.vx += (float)lit->color.r * scale;
			color.vy += (float)lit->color.g * scale;
			color.vz += (float)lit->color.b * scale;
			if (color.vx > 255.0f) color.vx = 255.0f;
			if (color.vy > 255.0f) color.vy = 255.0f;
			if (color.vz > 255.0f) color.vz = 255.0f;			
		}
		/* 参考 : pshadevu.dsm
		  ; ＜計算内容について＞
		  ; len = VecLength( vec ) ;
		  ; scale = ( r_range - len ) / r_range ;
		  ; vec = VecNormalize( vec ) ;
		  ; force = VecInnerProduct( normal , vec ) ;
		  ; if ( force < 0 ) force = 0 ;
		  ; total_color += color * force * 1.5 ;
		  ; 以上の計算を最適化したもの
		*/
	}
	
	// スポット光源
	spot = work->spot;
	for (i = work->n_spot; i > 0; -- i, spot ++) {
		float r_range, len, force;
#if 0
		// バウンディングボックス非対応バージョン
		r = spot->dir.vw * 2.0f;
		work->w_vec.vx = f = work->w_vert.vx - spot->point.vx;
		if (FABS(f) > r) continue;
		work->w_vec.vy = f = work->w_vert.vy - spot->point.vy;
		if (FABS(f) > r) continue;
		work->w_vec.vz = f = work->w_vert.vz - spot->point.vz;
		if (FABS(f) > r) continue;
#else
		// バウンディングボックス対応バージョン
		if (work->w_vert.vx > spot->bound_max.vx
			|| work->w_vert.vx < spot->bound_min.vx) continue;
		if (work->w_vert.vy > spot->bound_max.vy
			|| work->w_vert.vy < spot->bound_min.vy) continue;
		if (work->w_vert.vz > spot->bound_max.vz
			|| work->w_vert.vz < spot->bound_min.vz) continue;
		work->w_vec.vx = work->w_vert.vx - spot->point.vx;
		work->w_vec.vy = work->w_vert.vy - spot->point.vy;
		work->w_vec.vz = work->w_vert.vz - spot->point.vz;
#endif
		r_range = spot->dir.vw * 2.0f;
		work->w_param.vy = r_range;
		work->w_param.vz = spot->umbra;
		work->w_param.vw = spot->penumbra;

		len = D3DXVec3Length(&work->w_vec);
		if (r_range - len <= 0.0f) { // 有効範囲内に入っていなければ終了
			continue;
		}

		len = D3DXVec3Length(&work->w_vec);
		D3DXVec3Normalize(&work->w_vec, &work->w_vec);		
		force = D3DXVec3Dot(&work->w_vec, &work->w_norm);
		if (force > 0.0f) {
			float cos_angle; // スポットライトの方向との角度
			float scale, spot_force;
			cos_angle = D3DXVec3Dot(&spot->dir, &work->w_vec);
			
			scale = (r_range - len) / r_range;
			if (cos_angle < spot->penumbra) { // スポットライトの外側
				continue;
			} if (cos_angle > spot->umbra) {  // スポットライト中心の内側
				spot_force = 1.0f;
			} else {                          // スポットライト中心～外側の間
				spot_force = spot->penumbra - cos_angle;
				spot_force /= spot->penumbra - spot->umbra;
			}
			scale = (r_range - len) / r_range;
			scale *= force * spot_force * 1.5f;
			
			color.vx += (float)spot->color.r * scale;
			color.vy += (float)spot->color.g * scale;
			color.vz += (float)spot->color.b * scale;
			if (color.vx > 255.0f) color.vx = 255.0f;
			if (color.vy > 255.0f) color.vy = 255.0f;
			if (color.vz > 255.0f) color.vz = 255.0f;			
		}
		/* 参考 : pshadevu.dsm
		   //vf16 = work->w_vec;
		   //vf17 = work->w_param;
		   //vf18 = spot->color;
		   //vf19 = spot->dir;
		   ; ＜計算内容について＞
		   ; len = VecLength( vec ) ;
		   ; scale = ( r_range - len ) / r_range ;
		   ; vec = VecNormalize( vec ) ;
		   ; force = VecInnerProduct( normal , vec ) ;
		   ; if ( force < 0 ) force = 0 ;
		   ; cos_angle = VecInnerProduct( dir , vec )
		   ; if ( cos_angle > umbra ) spot_force = 1.0 ;
		   ; else if ( cos_angle < penumbra ) spot_force = 0.0 ;
		   ; else spot_force = ( penumbra - cos_angle ) / ( penumbra - umbra ) ;
		   ; total_color += color * force * spot_force * 1.5 ;
		*/
	}
	
	// 線光源
	line = work->line;
	for (i = work->n_line ; i > 0 ; -- i, line ++) {
		VECTOR end_pos;
		float n1, n2, n3;
		float r_range, len, force, scale;

		/* バウンディングボックス対応バージョン */
		if (work->w_vert.vx > line->bound_max.vx
			|| work->w_vert.vx < line->bound_min.vx) continue;
		if (work->w_vert.vy > line->bound_max.vy
			|| work->w_vert.vy < line->bound_min.vy) continue;
		if (work->w_vert.vz > line->bound_max.vz
			|| work->w_vert.vz < line->bound_min.vz) continue;
		r_range = line->r_range * 2;

		// 線光源の終端位置を求める
		end_pos.vx = line->point.vx + line->dir.vx * line->dir.vw;
		end_pos.vy = line->point.vy + line->dir.vy * line->dir.vw;
		end_pos.vz = line->point.vz + line->dir.vz * line->dir.vw;

		// 線光源のstart側,end側,startとendの間 の3通りに分岐
		n1 = D3DXVec3Dot(&line->point, &line->dir);
		n2 = D3DXVec3Dot(&end_pos, &line->dir);
		n3 = D3DXVec3Dot(&work->w_vert, &line->dir);

		if (n1 > n3) {        // start側
			work->w_vec.vx = work->w_vert.vx - line->point.vx;
			work->w_vec.vy = work->w_vert.vy - line->point.vy;
			work->w_vec.vz = work->w_vert.vz - line->point.vz;
		} else if (n2 < n3) { // end 側
			work->w_vec.vx = work->w_vert.vx - end_pos.vx;
			work->w_vec.vy = work->w_vert.vy - end_pos.vy;
			work->w_vec.vz = work->w_vert.vz - end_pos.vz;
		} else {              // 間
			VECTOR tmp_pos;   // 線光源上の点
			tmp_pos.vx = line->point.vx + (n3 - n1) * line->dir.vx;
			tmp_pos.vy = line->point.vy + (n3 - n1) * line->dir.vy;
			tmp_pos.vz = line->point.vz + (n3 - n1) * line->dir.vz;
			work->w_vec.vx = work->w_vert.vx - tmp_pos.vx;
			work->w_vec.vy = work->w_vert.vy - tmp_pos.vy;
			work->w_vec.vz = work->w_vert.vz - tmp_pos.vz;
		}
		
		// ここから後ろは点光源と全く同じ
		r_range = line->r_range * 2;
		len = D3DXVec3Length(&work->w_vec);
		if (r_range - len <= 0.0f) {
			// 有効範囲内に入っていなければ終了
			continue;
		}
		D3DXVec3Normalize(&work->w_vec, &work->w_vec);
		force = D3DXVec3Dot(&work->w_vec, &work->w_norm);
		if (force > 0.0f) {
			scale = 1.5f * force * (r_range - len) / r_range;
			color.vx += (float)line->color.r * scale;
			color.vy += (float)line->color.g * scale;
			color.vz += (float)line->color.b * scale;
			if (color.vx > 255.0f) color.vx = 255.0f;
			if (color.vy > 255.0f) color.vy = 255.0f;
			if (color.vz > 255.0f) color.vz = 255.0f;			
		}
		/* 参考 : pshadevu.dsm
		  ; start_pos = line->point;
		  ; dir = line->dir;
		  ; pos = work->w_vert;
		  ; ＜計算内容について＞
		  ; end_pos = start_pos + dir * dir.w ;
		  ; n1 = VecInnerProduct( start_pos, dir );
		  ; n2 = VecInnerProduct( end_pos, dir );
		  ; n3 = VecInnerProduct( pos, dir );
		  ; if (n1 > n3) {
		  ;     vec = pos - start_pos ;
		  ; } else if (n2 < n3) {
		  ;     vec = pos - end_pos;
		  ; } else {
		  ;     vec = pos - ( start_pos + dir * ( n3 - n1 ) ) ;
		  ; }
		  ;
		  ; ここから下は点光源と同じ
		  ; len = VecLength( vec ) ;
		  ; scale = ( r_range - len ) / r_range ;
		  ; vec = VecNormalize( vec ) ;
		  ; force = VecInnerProduct( normal , vec ) ;
		  ; if ( force < 0 ) force = 0 ;
		  ; total_color += color * force * 1.5 ;
		*/
	}
	
	// 黒点光源
	black = work->blackpoints;
	for (i = work->n_black; i > 0; -- i, black ++) {
		float len, r_range, force;
		
		if (black->bound_max.vx <= work->w_vert.vx
			|| black->bound_min.vx >= work->w_vert.vx) continue;
		if (black->bound_max.vy <= work->w_vert.vy
			|| black->bound_min.vy >= work->w_vert.vy) continue;
		if (black->bound_max.vz <= work->w_vert.vz
			|| black->bound_min.vz >= work->w_vert.vz) continue;
		work->w_vec.vx = work->w_vert.vx - black->point.vx;
		work->w_vec.vy = work->w_vert.vy - black->point.vy;
		work->w_vec.vz = work->w_vert.vz - black->point.vz;
		//work->w_param.vy = black->r_range * 2;

		r_range = black->r_range * 2;
		len = D3DXVec3Length(&work->w_vec);
		if (r_range - len <= 0.0f) {
			// 有効範囲内に入っていなければ終了
			continue;
		}
		D3DXVec3Normalize(&work->w_vec, &work->w_vec);
		force = D3DXVec3Dot(&work->w_vec, &work->w_norm);
		if (force > 0.0f) {
			force = 1.0f - force;
			color.vx *= force;
			color.vy *= force;
			color.vz *= force;
		}
		/* 参考 : pshadevu.dsm
		  ; len = VecLength( vec ) ;
		  ; scale = ( r_range - len ) / r_range ;
		  ; vec = VecNormalize( vec ) ;
		  ; force = VecInnerProduct( normal , vec ) ;
		  ; if ( force < 0 ) force = 0 ;
		  ; total_color *= ( 1.0 - force ) ;
		*/
		break; // 影響があるのは最初の1個のみ
	}

	rgbs->vx = (short)color.vx;
	rgbs->vy = (short)color.vy;
	rgbs->vz = (short)color.vz;
#if 0	
	if ((int)rgbs & 0x8) { // ??? これは何やってるんだろう…
		rgbs->pad = norms->pad;
	}
#endif
	return rgbs;
	
#if 0
	
/* 参考/プレシェイド計算準備ルーチン
  StartPreshade:
  itof0.xyzw	vf8,vf8				nop		; vf8 = 座標変換前頂点座標
  itof12.xyz	vf9,vf9				nop		; vf9 = 座標変換前法線
  mulax.xyzw	ACC,vf4,vf8			nop		; ACC  = vf4 * vf8.x
  madday.xyzw	ACC,vf5,vf8			nop		; ACC += vf5 * vf8.y
  maddaz.xyzw	ACC,vf6,vf8			nop		; ACC += vf6 * vf8.z
  maddw.xyzw	vf8,vf7,vf0			nop		; vf8 = ACC + vf7 * vf0.w; vf8 = 頂点座標

  mulax.xyz		ACC,vf4,vf9			nop		; ACC = vf4 * vf9.x
  madday.xyz	ACC,vf5,vf9			nop		; ACC += vf5 * vf9.y
  maddz.xyz		vf9,vf6,vf9			nop		; vf9 += vf6 * vf9.z

  ; 変換後の法線と平行光源方向ベクトルの内積を取る
  mulax.x		ACC,vf24,vf9		nop		; ACC =  vf24 * vf9.x
  madday.x		ACC,vf25,vf9		nop		; ACC += vf25 * vf9.y
  maddz.x		vf10,vf26,vf9		nop		; vf10 = ACC + vf26 * vf9.z;vf10x = 平行光源影響度
  addx.x		vf28,vf0,vf9		nop		; vf28x = vf9.x;            vf28x = 法線Ｘ成分
  addy.x		vf29,vf0,vf9		nop		; vf29x = vf9.y;            vf29x = 法線Ｙ成分
  addz.x		vf30,vf0,vf9		nop		; vf30x = vf9.z;            vf30x = 法線Ｚ成分
  maxx.x		vf10,vf10,vf0		nop		; vf10x = max(vf10.x, 0);   平行光源影響度＞０
  addax.xyz	ACC,vf31,vf0		nop		; ACC = アンビエント
  maddx.xyz	vf10,vf27,vf10		nop		; vf10 = ACC + vf27 * vf10.x;
  ; vf10 = 累積カラー初期化（＝平行光源＋アンビエント）
  nop[e]							nop		; 
  nop								nop		;
*/
	if ((int)rgbs & 0x8) rgbs->pad = norms->pad;
#endif
}


// モデルのバウンディングに触れる全光源をスクラッチパッドへ転送する
// XBOX版では、静的に確保したワークにコピー
static void CreateLightBuffer(DG_MDL *mdl, LIT_DEF *lit_def)
{
	static VECTOR _bound_max = {-99999999.0f,-99999999.0f,-99999999.0f, 0.0f};
	static VECTOR _bound_min = { 99999999.0f, 99999999.0f, 99999999.0f, 0.0f};
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR;
	int     i, j;
	LIT_GRP *grp;
	void    *ptr;
	VECTOR  tmp, mdl_max, mdl_min;

	{// モデルバウンディング生成
		_SetDefaultBound(&_bound_max, &_bound_min);
		for (i = 8; i > 0;) {
			for (j = 4; j > 0; j--, i--) {
				tmp.vx = (i & 1) ? mdl->lx : mdl->ux;
				tmp.vy = (i & 2) ? mdl->ly : mdl->uy;
				tmp.vz = (i & 4) ? mdl->lz : mdl->uz;
				tmp.vw = 1.0F;
				_RotTransBoundCheck(&tmp);
			}
		}
		_GetBound(&mdl_max, &mdl_min);
	}
	//printf("mdl_max = %f, %f, %f, %f\n", mdl_max.x, mdl_max.y, mdl_max.z, mdl_max.w);
	//printf("mdl_min = %f, %f, %f, %f\n", mdl_min.x, mdl_min.y, mdl_min.z, mdl_min.w);

	work->hit_flag = 0;
	ptr = work->buffer;

	/* 点光源検索 */
	work->n_point = 0;
	work->points = (LIT_POINT *)ptr;
	for (grp = (LIT_GRP *)&lit_def[1], i = lit_def->n_lit_group; i > 0; grp++, i--){
		LIT_POINT *src, *dst;
		/* 属性チェック */
		if (grp->type & LIT_TYPE_DISABLE) continue;
		if (!(grp->type & LIT_TYPE_POINT)) continue;
		/* バウンディングチェック */
		if (!(grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz)) continue;
		if (grp->type & LIT_TYPE_CHANGE) {
			work->hit_flag = 1;
		}
		/* スクラッチパッドへコピー */
		src = (LIT_POINT *)grp->lit;
		dst = (LIT_POINT *)ptr;
		for (j = grp->n_lights; j > 0; src++, j--){
			if (src->flag & LIT_FLAG_DISABLE) continue;
			if (!(src->flag & LIT_FLAG_BGONLY)) continue;
			/* さらに個々のあたり判定をチェックする */
			if ((src->point.vx + src->e_range) < mdl_min.vx
				|| (src->point.vx - src->e_range) > mdl_max.vx
				|| (src->point.vy + src->e_range) < mdl_min.vy
				|| (src->point.vy - src->e_range) > mdl_max.vy
				|| (src->point.vz + src->e_range) < mdl_min.vz
				|| (src->point.vz - src->e_range) > mdl_max.vz) {
				continue;
			}
			*dst ++ = *src;
			work->n_point ++;
		}
		ptr = dst;
	}
	
	/* スポット光源検索 */
	work->n_spot = 0;
	work->spot = (LIT_SPOT *)ptr;
	for (grp = (LIT_GRP *)&lit_def[1], i = lit_def->n_lit_group; i > 0; grp++, i--){
		LIT_SPOT *src, *dst;
		/* 属性チェック */
		if (grp->type & LIT_TYPE_DISABLE) continue;
		if (!(grp->type & LIT_TYPE_SPOT)) continue;
		/* バウンディングチェック */
		if (!(grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			  grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			  grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz)) continue;
		if (grp->type & LIT_TYPE_CHANGE) work->hit_flag = 1;
		src = (LIT_SPOT *)grp->lit;
		dst = (LIT_SPOT *)ptr;
		for (j = grp->n_lights; j > 0; src++, j--){
			if (src->flag & LIT_FLAG_DISABLE) continue;
			if (!(src->flag & LIT_FLAG_BGONLY)) continue;
			/* さらに個々のあたり判定をチェックする */
			if ((src->bound_max.vx) < mdl_min.vx || (src->bound_min.vx) > mdl_max.vx ||
				(src->bound_max.vy) < mdl_min.vy || (src->bound_min.vy) > mdl_max.vy ||
				(src->bound_max.vz) < mdl_min.vz || (src->bound_min.vz) > mdl_max.vz){
				continue;
			}
			*dst ++ = *src;
			work->n_spot ++;
		}
		ptr = dst;
	}
	
	/* 線光源検索 */
	work->n_line = 0;
	work->line = (LIT_LINE *)ptr;
	for (grp = (LIT_GRP *)&lit_def[1], i = lit_def->n_lit_group; i > 0; grp++, i--){
		LIT_LINE	*src, *dst;
		/* 属性チェック */
		if (grp->type & LIT_TYPE_DISABLE) continue;
		if (!(grp->type & LIT_TYPE_LINE)) continue;
		/* バウンディングチェック */
		if (!(grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz)) continue;
		if (grp->type & LIT_TYPE_CHANGE) work->hit_flag = 1;
		src = (LIT_LINE *)grp->lit;
		dst = (LIT_LINE *)ptr;
		for (j = grp->n_lights; j > 0; src++, j--){
			if (src->flag & LIT_FLAG_DISABLE) continue;
			if (!(src->flag & LIT_FLAG_BGONLY)) continue;
			/* さらに個々のあたり判定をチェックする */
			if ((src->bound_max.vx) < mdl_min.vx || (src->bound_min.vx) > mdl_max.vx ||
				(src->bound_max.vy) < mdl_min.vy || (src->bound_min.vy) > mdl_max.vy ||
				(src->bound_max.vz) < mdl_min.vz || (src->bound_min.vz) > mdl_max.vz){
				continue;
			}
			*dst++ = *src;
			work->n_line ++;
		}
		ptr = dst;
	}
	
	/* 黒点光源検索 */
	work->n_black = 0;
	work->blackpoints = (LIT_BLACKPOINT *)ptr;
	for (grp = (LIT_GRP *)&lit_def[1], i = lit_def->n_lit_group; i > 0; grp++, i--){
		LIT_BLACKPOINT *src, *dst;
		/* 属性チェック */
		if (grp->type & LIT_TYPE_DISABLE) continue;
		if (!(grp->type & LIT_TYPE_BLACKPOINT)) continue;
		/* バウンディングチェック */
		if (!(grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz)) continue;
		if (grp->type & LIT_TYPE_CHANGE) work->hit_flag = 1;
		src = (LIT_BLACKPOINT *)grp->lit;
		dst = (LIT_BLACKPOINT *)ptr;
		for (j = grp->n_lights; j > 0; src++, j--){
			if (src->flag & LIT_FLAG_DISABLE) continue;
			if (!(src->flag & LIT_FLAG_BGONLY)) continue;
			/* さらに個々のあたり判定をチェックする */
			if ((src->bound_max.vx) < mdl_min.vx || (src->bound_min.vx) > mdl_max.vx ||
				(src->bound_max.vy) < mdl_min.vy || (src->bound_min.vy) > mdl_max.vy ||
				(src->bound_max.vz) < mdl_min.vz || (src->bound_min.vz) > mdl_max.vz){
				continue;
			}
			*dst ++ = *src;
			work->n_black ++;
		}
		ptr = dst;
	}
}


// KMSデータから頂点数を計算する。
static int KMS_GetVertexNum(DG_DEF *def)
{
	DG_MDL     *mdl;
	DG_MDLPACK *pack;
	int i, j;
	int n_vertex = 0;

	mdl = def->models;
	for (i = def->n_x_models; i > 0; i--, mdl++) {
		pack = mdl->packs;
		for (j = mdl->n_packs; j > 0; j--, pack++) {
			n_vertex += pack->n_verts;               // 頂点数
		}
	}
}


// プレシェイドバッファにＲＧＢ値を書き込む(シェイドなし)
// いったん生成された頂点バッファを再ロックして書き込みに行くので無駄。
// 
// でも、アプリケーション側では
//
// obj = DG_MakeObjs(....);
// DG_MakePreshade(obj, lit); 
//
// という使い方をしているので、とりあえずは仕方ない…かな？
//
// 頂点バッファにRGB値を書き込む
static unsigned char *NoShadeRGB(DG_VBUF_INFO *info, DG_OBJ *obj, DG_MDL *mdl,
								 unsigned char *rgbs)
{
	DG_MDLPACK	  *pack;
	DG_OBJ_PACKET *pkt;
	u_long64		  *norms;
	u_long64		  code =     0x0000008000800080ULL;
	u_long64		  and_mask = 0xffff000000000000ULL;
	u_long64        tmp_rgbs;
	SVECTOR       *v;
	int		      i, j;

	pack = mdl->packs;
	pkt = &obj->packets[0];
	v = (SVECTOR *)&tmp_rgbs;
	for (i = mdl->n_packs; i > 0; i--, pack++, pkt++) {
		norms = (u_long64 *)pack->norms;
		for (j = pack->n_verts; j > 0; j--) {
			tmp_rgbs = code | (*norms & and_mask);
			*(u_int *)rgbs = D3DCOLOR_RGBA(v->vx, v->vy, v->vz, 0x80);
			norms ++;
			rgbs += info->stride;
		}
	}
	return rgbs; // 次の書き込み位置を返す
}

// 頂点バッファにRGB値を書き込む
static unsigned char *ShadeRGB(DG_VBUF_INFO *info, DG_OBJ *obj, DG_MDL *mdl,
							   unsigned char *rgbs, LIT_DEF *lit_def)
{
	int i, j;
	DG_MDLPACK    *pack;
	SVECTOR *verts, *norms;
	SVECTOR tmp_rgbs;
	
	// 対象となる光源データをスクラッチパッドへ転送する
	CreateLightBuffer(mdl, lit_def);

	pack = mdl->packs;
	for (i = mdl->n_packs; i > 0; i--, pack++) {
		// 頂点数分繰り返す
		verts = (SVECTOR *)pack->verts;
		norms = (SVECTOR *)pack->norms;	
		for (j = pack->n_verts; j > 0; j--) {
			MakeLight(&tmp_rgbs, verts, norms);
			*(u_int *)rgbs = D3DCOLOR_RGBA(tmp_rgbs.vx, tmp_rgbs.vy, tmp_rgbs.vz, 0x80);
			verts ++;
			norms ++;
			rgbs += info->stride;
		}
	}
	return rgbs;
}

// プレシェイドバッファの再計算
// TODO:ReshadeRGB
static unsigned char *ReshadeRGB(DG_VBUF_INFO *info, DG_OBJ *obj, DG_MDL *mdl,
								 unsigned char *rgbs, LIT_DEF *lit_def)
{
	int i, j;
	DG_MDLPACK    *pack;
	SVECTOR *verts, *norms;
	SVECTOR tmp_rgbs;
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR;
	
	// ShadeRGB とここだけが違う
	if (work->hit_flag == 0) {
		pack = mdl->packs;
		for (i = mdl->n_packs; i > 0; i--, pack++) {
			for (j = pack->n_verts; j > 0; j--) {
				rgbs += info->stride;
			}
		}
		// 計算せずに次のポインタを返す
		return rgbs;
	}

	// 対象となる光源データをスクラッチパッドへ転送する
	CreateLightBuffer(mdl, lit_def);

	pack = mdl->packs;
	for (i = mdl->n_packs; i > 0; i--, pack++) {
		// 頂点数分繰り返す
		verts = (SVECTOR *)pack->verts;
		norms = (SVECTOR *)pack->norms;	
		for (j = pack->n_verts; j > 0; j--) {
			MakeLight(&tmp_rgbs, verts, norms);
			*(u_int *)rgbs = D3DCOLOR_RGBA(tmp_rgbs.vx, tmp_rgbs.vy, tmp_rgbs.vz, 0x80);
			verts ++;
			norms ++;
			rgbs += info->stride;
		}
	}
	return rgbs;
}

//-----------------------------------------------------------------------------

// プレシェイドバッファのサイズを計算する
static int SizeofRGB(DG_OBJS *objs)
{
	DG_MDL        *mdl;
	DG_OBJ        *obj;
	DG_OBJ_PACKET *pack;
	int i, j, size;

	size = 0;
	obj = objs->objs;
	for (i = objs->n_models; i > 0; -- i) {
		mdl = obj->model;
		pack = obj->packets;
		for (j = obj->n_packs; j > 0; j--){
			size += (pack->n_verts + 1) & 0xfffe;
			pack ++;
		}
		obj ++;
	}
	return sizeof(SVECTOR) * size;
}

// オブジェクトに対してプリシェード計算を行なう
int DG_MakePreshade(DG_OBJS *objs, LIT_DEF *lit_def)
{
	MATRIX  tmp_mat;
	//SVECTOR *rgbs;
	unsigned char *rgbs;
	DG_DEF  *def;
	DG_OBJ  *obj;
	int     i, flag;

	flag = 0;

	def = objs->def;
	ASSERT(def != NULL);

return (-1);
#if 0
	// プレシェイドバッファの存在を確認。無ければ新規確保。
	// TODO:プレシェイドバッファ不要か？？
	rgbs = objs->rgb_buff;
	if (rgbs == NULL) {
		printf("Preshade Buffer NULL\n");
		rgbs = (SVECTOR *)GV_Malloc(SizeofRGB(objs));
		objs->rgb_buff = rgbs;
		if (rgbs == NULL) {
			return - 1;
		}
		flag = 1;
	}
#endif

	//GV_GET_PRFC_CLOCK();
	// プレシェイドデータを計算する
	tmp_mat = objs->world;
	tmp_mat.m[3][0] += objs->def->tx;
	tmp_mat.m[3][1] += objs->def->ty;
	tmp_mat.m[3][2] += objs->def->tz;
	obj = objs->objs;

	// プレシェイド計算に使うマトリクスを設定(ShadeRGB内部で使う)
	_SetMatrix(&tmp_mat);

#if 1
	// DG_OBJS1個について VertexBuffer1個に変更
	DG_VBUF_INFO *info;
	unsigned char *verts_buf;
	
	info = objs->vbuf_info;
	info->vbuf->Lock(0, info->size, &verts_buf, 0);
	rgbs = (unsigned char *)(verts_buf + info->rgba_ofs);
	for (i = objs->n_models; i > 0; -- i, obj ++) {
		DG_MDL      *mdl;
		//obj->rgbs = rgbs;
		mdl = obj->model;
		if (DG_TYPE_NOSHADE & mdl->type
			/*|| DG_FLAG_SHADOWVOL & objs->flag*/
			|| lit_def == NULL) {
			rgbs = NoShadeRGB(objs->vbuf_info, obj, mdl, rgbs);
		} else {
			//rgbs = NoShadeRGB(obj, mdl, rgbs);
			rgbs = ShadeRGB(objs->vbuf_info, obj, mdl, rgbs, lit_def);
		}
	}
	info->vbuf->Unlock();
#else
	for (i = objs->n_models; i > 0; -- i, obj ++) {
		DG_MDL      *mdl;
		obj->rgbs = rgbs;
		mdl = obj->model;
		if (DG_TYPE_NOSHADE & mdl->type
			/*|| DG_FLAG_SHADOWVOL & objs->flag*/
			|| lit_def == NULL) {
			rgbs = NoShadeRGB(obj, mdl, rgbs);
		} else {
			//rgbs = NoShadeRGB(obj, mdl, rgbs);
			rgbs = ShadeRGB(obj, mdl, rgbs, lit_def);
		}
	}
#endif

#if 0
	// 法線情報を輝度情報のポインタにする ?????
	// とりあえず削除。
	obj = objs->objs;
	for (i = objs->n_models; i > 0; -- i) {
		obj->norms = obj->rgbs;
		obj ++;
	}
#endif

	/* 自動再計算の為にポインタを記憶しておく */
	objs->fix_light = lit_def;

	return 0;
}


// ライトの変更部分のみに対してプリシェード計算を行なう
int DG_MakePreshadeParts(DG_OBJS *objs, LIT_DEF *lit_def)
{
	MATRIX tmp_mat;
	DG_OBJ *obj;
	DG_VBUF_INFO *info;
	unsigned char *verts_buf;
	unsigned char *rgbs;
	int    i;

	// プレシェイドデータを計算する
	tmp_mat = objs->world;
	tmp_mat.m[3][0] += objs->def->tx;
	tmp_mat.m[3][1] += objs->def->ty;
	tmp_mat.m[3][2] += objs->def->tz;
	obj = objs->objs;
	info = objs->vbuf_info;

	info->vbuf->Lock(0, info->size, &verts_buf, 0);
	rgbs = verts_buf + info->rgba_ofs;
	for (i = objs->n_models; i > 0; -- i, obj ++) {
		DG_MDL *mdl;
		mdl = obj->model;
		_SetMatrix(&tmp_mat);

		if (DG_TYPE_NOSHADE & mdl->type) {
			int j, k;
			DG_MDLPACK *pack = mdl->packs;
			for (j = mdl->n_packs; j > 0; j--, pack++) {
				for (k = pack->n_verts; k > 0; k--) {
					rgbs += info->stride;
				}
			}
			continue;
		}
		//ReshadeRGB(objs->vbuf_info, obj, mdl, obj->rgbs, lit_def);
		rgbs = ReshadeRGB(objs->vbuf_info, obj, mdl, rgbs, lit_def);
	}
	info->vbuf->Unlock();
	return 0;
}

// プリシェイドバッファを開放する
void DG_FreePreshade(DG_OBJS *objs)
{
	SVECTOR *rgbs;

	if (!(objs->flag & DG_FLAG_PAINT)) return;

	rgbs = objs->rgb_buff;
	if (rgbs != NULL) {
		GV_DelayedFree(rgbs);
		objs->rgb_buff = NULL;
	}
}

//-----------------------------------------------------------------------------
// 一時光源関連
//-----------------------------------------------------------------------------

// MakeLight に base 色を追加したもの
static SVECTOR* MakeLight2(SVECTOR *rgbs, SVECTOR *verts, SVECTOR *norms, SVECTOR *base)
{
	VECTOR  one = {1.0f,    1.0f, 1.0f, 255.0f};
	VECTOR  two = {1.5f, - 0.75f, 0.0f,   0.0f};
	VECTOR  color;
	LIT_POINT      *lit;
	LIT_SPOT       *spot;
	LIT_LINE       *line;
	LIT_BLACKPOINT *black;
	int     i, count;
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR;
	
	// 頂点をpshade_matで変換 -> work->w_vert;
	VECTOR vecTmp;
	vecTmp.vx = (float)verts->vx;
	vecTmp.vy = (float)verts->vy;
	vecTmp.vz = (float)verts->vz;
	vecTmp.vw = 1.0f;
	D3DXVec4Transform(&work->w_vert, &vecTmp, &pshade_mat);
	
	// 頂点をpshade_matの回転成分のみで変換＆正規化-> work->w_norm
	vecTmp.vx = (float)norms->vx;
	vecTmp.vy = (float)norms->vy;
	vecTmp.vz = (float)norms->vz;
	vecTmp.vw = 0.0f;
	D3DXVec4Transform(&vecTmp, &vecTmp, &pshade_mat);
	D3DXVec3Normalize(&work->w_norm, &vecTmp);

	VECTOR vecLight; // 平行光源方向
	VECTOR vecColor; // 平行光源色

	vecLight.vx = DG_LightMatrix.m[0][0]; // DG_LightMatrixを参照
	vecLight.vy = DG_LightMatrix.m[1][0];
	vecLight.vz = DG_LightMatrix.m[2][0];
	vecLight.vw = 1.0f;

	vecColor.vx = DG_ColorMatrix.m[0][0] * 256.0f; // DG_ColorMatrixを参照
	vecColor.vy = DG_ColorMatrix.m[0][1] * 256.0f;
	vecColor.vz = DG_ColorMatrix.m[0][2] * 256.0f;
	vecColor.vw = 1.0f;
	
	// デフォルト頂点色 = base
	color.vx = (float)base->vx;
	color.vy = (float)base->vy;
	color.vz = (float)base->vz;
	color.vw = 1.0f;

	count = 0;

	// 点光源
	lit = work->points;	
	for (i = work->n_point; i > 0; -- i, lit ++) {
		float f, r, len, scale, r_range, force;
		r = lit->e_range; // 点光源の影響範囲

		// 影響範囲に入っているか簡単にチェック
		f = work->w_vert.vx - lit->point.vx;
		if (FABS(f) > r) continue;
		work->w_vec.vx = f;		
		f = work->w_vert.vy - lit->point.vy;
		if (FABS(f) > r) continue;
		work->w_vec.vy = f;		
		f = work->w_vert.vz - lit->point.vz;
		if (FABS(f) > r) continue;
		work->w_vec.vz = f;		

		// 点光源パラメータ
		//work->w_param.vy = lit->r_range * 2;
		r_range = lit->r_range * 2;
		len = D3DXVec3Length(&work->w_vec);
		if (r_range - len <= 0.0f) {
			// 有効範囲内に入っていなければ終了
			continue;
		}
		D3DXVec3Normalize(&work->w_vec, &work->w_vec);
		force = D3DXVec3Dot(&work->w_vec, &work->w_norm);
		if (force > 0.0f) {
			scale = 1.5f * force * (r_range - len) / r_range;
			color.vx += (float)lit->color.r * scale;
			color.vy += (float)lit->color.g * scale;
			color.vz += (float)lit->color.b * scale;
			if (color.vx > 255.0f) color.vx = 255.0f;
			if (color.vy > 255.0f) color.vy = 255.0f;
			if (color.vz > 255.0f) color.vz = 255.0f;			
		}
	}
	
	// スポット光源
	spot = work->spot;
	for (i = work->n_spot; i > 0; -- i, spot ++) {
		float r_range, len, force;
#if 0
		// バウンディングボックス非対応バージョン
		r = spot->dir.vw * 2.0f;
		work->w_vec.vx = f = work->w_vert.vx - spot->point.vx;
		if (FABS(f) > r) continue;
		work->w_vec.vy = f = work->w_vert.vy - spot->point.vy;
		if (FABS(f) > r) continue;
		work->w_vec.vz = f = work->w_vert.vz - spot->point.vz;
		if (FABS(f) > r) continue;
#else
		// バウンディングボックス対応バージョン
		if (work->w_vert.vx > spot->bound_max.vx
			|| work->w_vert.vx < spot->bound_min.vx) continue;
		if (work->w_vert.vy > spot->bound_max.vy
			|| work->w_vert.vy < spot->bound_min.vy) continue;
		if (work->w_vert.vz > spot->bound_max.vz
			|| work->w_vert.vz < spot->bound_min.vz) continue;
		work->w_vec.vx = work->w_vert.vx - spot->point.vx;
		work->w_vec.vy = work->w_vert.vy - spot->point.vy;
		work->w_vec.vz = work->w_vert.vz - spot->point.vz;
#endif
		r_range = spot->dir.vw * 2.0f;
		work->w_param.vy = r_range;
		work->w_param.vz = spot->umbra;
		work->w_param.vw = spot->penumbra;

		if (r_range - len <= 0.0f) { // 有効範囲内に入っていなければ終了
			continue;
		}

		len = D3DXVec3Length(&work->w_vec);
		D3DXVec3Normalize(&work->w_vec, &work->w_vec);		
		force = D3DXVec3Dot(&work->w_vec, &work->w_norm);
		if (force > 0.0f) {
			float cos_angle; // スポットライトの方向との角度
			float scale, spot_force;
			cos_angle = D3DXVec3Dot(&spot->dir, &work->w_vec);
			
			scale = (r_range - len) / r_range;
			if (cos_angle < spot->penumbra) { // スポットライトの外側
				continue;
			} if (cos_angle > spot->umbra) {  // スポットライト中心の内側
				spot_force = 1.0f;
			} else {                          // スポットライト中心～外側の間
				spot_force = spot->penumbra - cos_angle;
				spot_force /= spot->penumbra - spot->umbra;
			}
			scale = (r_range - len) / r_range;
			scale *= force * spot_force * 1.5f;
			
			color.vx += (float)spot->color.r * scale;
			color.vy += (float)spot->color.g * scale;
			color.vz += (float)spot->color.b * scale;
			if (color.vx > 255.0f) color.vx = 255.0f;
			if (color.vy > 255.0f) color.vy = 255.0f;
			if (color.vz > 255.0f) color.vz = 255.0f;			
		}
	}
	
	// 線光源
	line = work->line;
	for (i = work->n_line ; i > 0 ; -- i, line ++) {
		VECTOR end_pos;
		float n1, n2, n3;
		float r_range, len, force, scale;

		/* バウンディングボックス対応バージョン */
		if (work->w_vert.vx > line->bound_max.vx
			|| work->w_vert.vx < line->bound_min.vx) continue;
		if (work->w_vert.vy > line->bound_max.vy
			|| work->w_vert.vy < line->bound_min.vy) continue;
		if (work->w_vert.vz > line->bound_max.vz
			|| work->w_vert.vz < line->bound_min.vz) continue;
		r_range = line->r_range * 2;

		// 線光源の終端位置を求める
		end_pos.vx = line->point.vx + line->dir.vx * line->dir.vw;
		end_pos.vy = line->point.vy + line->dir.vy * line->dir.vw;
		end_pos.vz = line->point.vz + line->dir.vz * line->dir.vw;

		// 線光源のstart側,end側,startとendの間 の3通りに分岐
		n1 = D3DXVec3Dot(&line->point, &line->dir);
		n2 = D3DXVec3Dot(&end_pos, &line->dir);
		n3 = D3DXVec3Dot(&work->w_vert, &line->dir);

		if (n1 > n3) {        // start側
			work->w_vec.vx = work->w_vert.vx - line->point.vx;
			work->w_vec.vy = work->w_vert.vy - line->point.vy;
			work->w_vec.vz = work->w_vert.vz - line->point.vz;
		} else if (n2 < n3) { // end 側
			work->w_vec.vx = work->w_vert.vx - end_pos.vx;
			work->w_vec.vy = work->w_vert.vy - end_pos.vy;
			work->w_vec.vz = work->w_vert.vz - end_pos.vz;
		} else {              // 間
			VECTOR tmp_pos;   // 線光源上の点
			tmp_pos.vx = line->point.vx + (n3 - n1) * line->dir.vx;
			tmp_pos.vy = line->point.vy + (n3 - n1) * line->dir.vy;
			tmp_pos.vz = line->point.vz + (n3 - n1) * line->dir.vz;
			work->w_vec.vx = work->w_vert.vx - tmp_pos.vx;
			work->w_vec.vy = work->w_vert.vy - tmp_pos.vy;
			work->w_vec.vz = work->w_vert.vz - tmp_pos.vz;
		}
		
		// ここから後ろは点光源と全く同じ
		r_range = line->r_range * 2;
		len = D3DXVec3Length(&work->w_vec);
		if (r_range - len <= 0.0f) {
			// 有効範囲内に入っていなければ終了
			continue;
		}
		D3DXVec3Normalize(&work->w_vec, &work->w_vec);
		force = D3DXVec3Dot(&work->w_vec, &work->w_norm);
		if (force > 0.0f) {
			scale = 1.5f * force * (r_range - len) / r_range;
			color.vx += (float)line->color.r * scale;
			color.vy += (float)line->color.g * scale;
			color.vz += (float)line->color.b * scale;
			if (color.vx > 255.0f) color.vx = 255.0f;
			if (color.vy > 255.0f) color.vy = 255.0f;
			if (color.vz > 255.0f) color.vz = 255.0f;			
		}
	}
	
	// 黒点光源
	black = work->blackpoints;
	for (i = work->n_black; i > 0; -- i, black ++) {
		float len, r_range, force;
		
		if (black->bound_max.vx <= work->w_vert.vx
			|| black->bound_min.vx >= work->w_vert.vx) continue;
		if (black->bound_max.vy <= work->w_vert.vy
			|| black->bound_min.vy >= work->w_vert.vy) continue;
		if (black->bound_max.vz <= work->w_vert.vz
			|| black->bound_min.vz >= work->w_vert.vz) continue;
		work->w_vec.vx = work->w_vert.vx - black->point.vx;
		work->w_vec.vy = work->w_vert.vy - black->point.vy;
		work->w_vec.vz = work->w_vert.vz - black->point.vz;
		//work->w_param.vy = black->r_range * 2;

		r_range = black->r_range * 2;
		len = D3DXVec3Length(&work->w_vec);
		if (r_range - len <= 0.0f) {
			// 有効範囲内に入っていなければ終了
			continue;
		}
		D3DXVec3Normalize(&work->w_vec, &work->w_vec);
		force = D3DXVec3Dot(&work->w_vec, &work->w_norm);
		if (force > 0.0f) {
			force = 1.0f - force;
			color.vx *= force;
			color.vy *= force;
			color.vz *= force;
		}
		break; // 影響があるのは最初の1個のみ
	}

	rgbs->vx = (short)color.vx;
	rgbs->vy = (short)color.vy;
	rgbs->vz = (short)color.vz;
#if 0	
	if ((int)rgbs & 0x8) { // ??? これは何やってるんだろう…
		rgbs->pad = norms->pad;
	}
#endif
	return rgbs;
}

void DG_TempPreshadeRGB(DG_OBJ *obj, LIT_DEF *lit_def, int which, int start_flag)
{
	printf("未サポート\n");
	ASSERT(0);
#if 0
	DG_MDL     *mdl;
	DG_MDLPACK *mdl_pack;
	int        i, packet_size;
	SVECTOR    *tmp_color, *org_rgbs;
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR;

	mdl = obj->model;

	// 対象となる光源データをスクラッチパッドへ転送する
	_SetMatrix(&obj->world);
	CreateLightBuffer(mdl, lit_def);

	// 光源が一つもなかったら終了
	if ((work->n_point + work->n_spot + work->n_black) == 0) {
		return;
	}

	// パケットサイズ計算
	packet_size = 0 ;
	mdl_pack = mdl->packs;
	for ( i = mdl->n_packs; i > 0; mdl_pack ++, i --) {
		packet_size += (mdl_pack->n_verts + 1) & 0xfffe;
	}

	// TODO: 一時光源によるプレシェイドルーチン
	/* メモリ確保（ＤＭＡパケット領域から確保） */	
	tmp_color = (SVECTOR*)DG_CurrentDmaAddr;
	DG_CurrentDmaAddr = (void *)((int)DG_CurrentDmaAddr + sizeof(SVECTOR) * packet_size);

	obj->vanime_flag &= ~DG_VANIME_DELNORMS; // 法線アドレス復帰フラグを消しておく
	obj->vanime_flag |= DG_VANIME_NORMS;     //一時法線(＝頂点カラー)アニメのフラグを立てる
	obj->norms = tmp_color;
	mdl_pack = mdl->packs;
	org_rgbs = obj->rgbs;
	for (i = obj->n_packs ; i > 0; i --) {
		MakeLight2(tmp_color, (SVECTOR*)mdl_pack->verts,
				   (SVECTOR*)mdl_pack->norms, mdl_pack->n_verts, org_rgbs);
		tmp_color += (mdl_pack->n_verts + 1) & 0xfffe;
		org_rgbs += (mdl_pack->n_verts + 1) & 0xfffe;
		mdl_pack ++;
	}
#endif
}

extern "C" DG_TLIGHT    DG_TLights[2];
extern "C" int          DG_LightClock;
// テンポラリライトプリシェーディングを行なう
void DG_TmpLightPreshadeChanl(DG_CHANL *cp, int which)
{
	DG_TLIGHT     *flp;
	DG_OBJ_QUEUE  *queue;
	DG_OBJS	      **oque, *objs;
	DG_OBJ_BUFFER *obj_buff;
	int           i, j, start_flag = 1;

	flp = &DG_TLights[1 - DG_LightClock];

	if ((queue = cp->obj_queue) == NULL) return;
	obj_buff = &queue->objs_buffer;

	if ((flp->TmpLightGrp[0].n_lights
		 + flp->TmpLightGrp[1].n_lights
		 + flp->TmpLightGrp[2].n_lights) == 0) return;

	oque = (DG_OBJS **)obj_buff->queue;
	for (i = obj_buff->n_queue; i > 0; i --) {
		objs = *oque ++;
		if (!(objs->flag & DG_FLAG_PAINT)) continue;
		for (j = 0; j < objs->n_models; j ++) {
			if (objs->objs[j].rgbs == NULL) continue;
			DG_TempPreshadeRGB(&objs->objs[j], (LIT_DEF*)&flp->TmpLightDef, which, start_flag);
			start_flag = 0;
		}
	}
}

void DG_RefreshVAnimeChanl(DG_CHANL *cp, int which)
{
	printf("未サポートです\n");
	ASSERT(0);
}
