/*
  frame.cpp
  フレームパラメータ管理ルーチン
  
  1999/07/07 K.Takabe
  $Id: frame.cpp,v 1.1.1.3 2002/11/19 11:42:08 Yoshizawa1 Exp $
  
*/

// NOTE: 各チャンネルに格納されるカメラパラメータについて
//
//
//  FMATRIX eye_pers;           /* カメラ透視変換マトリクス（=pers*eye_inv）*/
//  FMATRIX eye_inv;            /* カメラ逆行列 */
//  FMATRIX eye;                /* カメラ行列 */
//  FMATRIX pers;               /* 透視変換マトリクス */
//  FMATRIX eye_pers2;          /* カメラ透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX pers2;              /* 透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX raise_pers;         /* 透視変換マトリクス */
//  FMATRIX raise_pers2;        /* 透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX raise_eye_pers;     /* カメラ透視変換マトリクス */
//  FMATRIX raise_eye_pers2;    /* カメラ透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  float   screen;             /* カメラ→スクリーンキョリ（default = 2.0） */
//
//  DirectX8では以下のように呼ばれているもの。
//  ・カメラ行列   → ビュー行列 (D3DXでは、D3DXMatrixLookAtLH)
//  ・透視変換行列 → 射影行列   (D3DXでは、D3DXMatrixPerspectiveLH
//                                           or D3DXMatrixPerspectiveOffCenterLH)
//  

#ifndef KP_XBOX
#include <windows.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"

//-----------------------------------------------------------------------------
// グローバル変数
//-----------------------------------------------------------------------------

// バッファ切り替えクロック
int DG_Clock = 0;

// 各種画面状態
int DG_DisplayStatus = 0;

// 引きつりフラグ
int DG_SkipFrame = 1;
int DG_DelayMode = 0;

// フィールド情報
int DG_Field;
int DG_CurrentField;

// フォグ関連パラメータ
CVECTOR DG_FogColor = {0, 0, 0, 0xff};
FVECTOR DG_FogParam = {0.0f, 255.0f, 1.0f / 255.0f, 1.0f};
float DG_FogParam1 = 0.0f;
float DG_FogParam2 = 255.0f;

// クリップ設定
float DG_ClipNear = DRAW_NEAR_CLIP;
float DG_ClipFar  = DRAW_FAR_CLIP;

#ifdef DEBUG_MODE
int DG_OffsetControlFrag = 0;
#endif

extern int DG_FrameSkip;

//-----------------------------------------------------------------------------

void DG_SetFogColor(int r, int g, int b)
{
    DG_FogColor.r = r;
    DG_FogColor.g = g;
    DG_FogColor.b = b;

	// Xbox: DirectXにフォグカラーを設定する
	DG_SetDxFogColor();
} 

void DG_SetFogParam(float fNear, float fFar)
{
	// オーダ的にはこのくらいの値な気がするんだけど…
	fNear /= 16.0f;
	fFar /= 16.0f;
	
    DG_FogParam1 = 255.0f / (fNear - fFar);
    DG_FogParam2 = 255.0f - DG_FogParam1 * fNear;

	//こちらのパラメータは頂点シェーダに渡す
	DG_FogParam.x = DG_FogParam1;
	DG_FogParam.y = DG_FogParam2;
	DG_FogParam.z = 1.0f / 255.0f;
	DG_FogParam.w = 1.0f;
}    

void DG_SetClipParam(float fNear, float fFar)
{
    DG_ClipNear = fNear;
    DG_ClipFar = fFar;
}


//-----------------------------------------------------------------------------

void DG_InitFrameSystem(void)
{
    //DG_ClearChanlSystem(1- DG_Clock);
    //DG_ClearChanlSystem(DG_Clock);
    DG_SetFogColor(0, 0, 0);
    DG_SetFogParam(20 * 65536.0f, 21 * 65536.0f);
    DG_SetClipParam(DRAW_NEAR_CLIP, DRAW_FAR_CLIP);
	DG_DisplayStatus = 0;
}

long64 DG_UnDrawFrameCount = 0;
int  DG_LastWhich = - 1;

void DG_StartFrame(void)
{
	int which;

	GV_PROFILE_FLIP();
	GV_PROFILE_START(GV_PROFILE_DRAW_START);
	GV_PROFILE_START(GV_PROFILE_ACT_START);

	if (DG_FrameSkip != 0) {
		// 一応切り替えておく(DG_EndFrameで元に戻す)
		DG_Clock = 1 - DG_Clock;
		printf("DG_FrameSkip\n");
		return;
	}

	DG_Clock = 1 - DG_Clock;
	which = DG_Clock;

	/* アンドローカウントの機構を入れた 2002.02.05 */
	if( DG_UnDrawFrameCount > 0 ){
		if( DG_LastWhich < 0 ){
			DG_LastWhich = which;
		}
		DG_UnDrawFrameCount--;
		DG_UnDrawChanlSystem( 1 - which );
	    goto Skip;
	} else {
		if( DG_LastWhich >= 0 ){
			if( which == DG_LastWhich ){
				DG_UnDrawChanlSystem( 1 - which );
				goto Skip;
			}
		}
	}

	//DG_DrawChanlSystem(1 - which); // 登録されているものを全部描画する
	DG_SortChanlSystem(1 - which); // 計算 & 描画
	DG_Draw2DPrim();               // 2次元プリミティブを描画
	DG_LastWhich = - 1;

  Skip:
	DG_ClearTmpLight();            // 一時光源バッファのクリア
	//DG_ClearChanlSystem(which);
	DG_CleanPrim2VertexBuffer();   // PRIM2用頂点バッファの Delayed Free
}

void DG_EndFrame(void)
{
	int which;

	which = DG_Clock;

	// 一時ＣＬＵＴバッファの設定
	//DG_SetTextureTmpClut( which );
	// ライト変更に伴う再プリシェード（チャンネル０のオブジェクトに対してのみ）
	//DG_LightReshadeChanl( DG_Chanl( 0 ), which );
	// プリシェード更新（チャンネル０のオブジェクトに対してのみ）
	//DG_TmpLightPreshadeChanl( DG_Chanl( 0 ), which );
	// 頂点アニメの復元（チャンネル０のオブジェクトに対してのみ）
	//DG_RefreshVAnimeChanl( DG_Chanl( 0 ), which );
	
	if (DG_FrameSkip == 0) {
		//DG_SortChanlSystem(which);
	} else {
		// DG_StartFrame()で変更した分を戻す
		DG_Clock = 1 - DG_Clock;
	}

	// 一時ＣＬＵＴバッファのリセット
	//DG_ResetTextureTmpClut();
}

//-----------------------------------------------------------------------------


//  FMATRIX eye_pers;           /* カメラ透視変換マトリクス（=pers*eye_inv）*/
//  FMATRIX eye_inv;            /* カメラ逆行列 */
//  FMATRIX eye;                /* カメラ行列 */
//  FMATRIX pers;               /* 透視変換マトリクス */
//  FMATRIX eye_pers2;          /* カメラ透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX pers2;              /* 透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX raise_pers;         /* 透視変換マトリクス */
//  FMATRIX raise_pers2;        /* 透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX raise_eye_pers;     /* カメラ透視変換マトリクス */
//  FMATRIX raise_eye_pers2;    /* カメラ透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  float   screen;             /* カメラ→スクリーンキョリ（default = 2.0） */
//
//  DirectX8では以下のように呼ばれているもの。
//  ・カメラ行列   → ビュー行列 (D3DXでは、D3DXMatrixLookAtLH(RH) で求める)
//  ・透視変換行列 → 射影行列   (D3DXでは、D3DXMatrixPerspectiveLH(RH)
//                                  または  D3DXMatrixPerspectiveOffCenterLH(RH)で求める)


#ifdef DEBUG_MODE
// チェック用/仮
static void PrintMatrix(char *str, FMATRIX *mat)
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

// 非0だったときのX軸ベクトルを格納しておく
static VECTOR vecPrev;

void DG_MakeCameraMatrix(MATRIX *mat, VECTOR *vecFrom, VECTOR *vecTo)
{
	// ※PS2版とY軸方向を逆にする
	static VECTOR vecLower = {0.0f, 1.0f, 0.0f, 0.0f}; // 下向きベクトル
	static VECTOR vecZero = {0.0f, 0.0f, 0.0f, 0.0f};   // 零ベクトル
	VECTOR vecZ, vecY, vecX;

	// 視点
	mat->m[3][0] = vecFrom->x;
	mat->m[3][1] = vecFrom->y;
	mat->m[3][2] = vecFrom->z;
	mat->m[3][3] = 1.0f;

	/*
	 * カメラ行列を計算する
	 * Ｚ軸は、「カメラ位置→注目点」ベクトルを正規化
	 * Ｘ軸は、「下向きベクトル」×Ｚ軸の結果を正規化
	 * Ｙ軸は、Ｚ軸×Ｘ軸
	 */
	//vecZ = (D3DXVECTOR3)*vecTo - (D3DXVECTOR3)*vecFrom;
	D3DXVec3Subtract(&vecZ, vecTo, vecFrom);
	D3DXVec3Cross(&vecX, &vecLower, &vecZ);

	if (vecX == vecZero) { // 0になってしまったら前回の値を使う
		vecX = vecPrev;
	} else {
		vecPrev = vecX;
	}

	D3DXVec3Normalize(&vecX, &vecX);
	D3DXVec3Normalize(&vecZ, &vecZ);	
	D3DXVec3Cross(&vecY, &vecZ, &vecX);

	mat->m[0][0] = vecX.x;
	mat->m[0][1] = vecX.y;
	mat->m[0][2] = vecX.z;
	mat->m[0][3] = 0.0f;

	mat->m[1][0] = vecY.x;
	mat->m[1][1] = vecY.y;
	mat->m[1][2] = vecY.z;

	mat->m[1][3] = 0.0f;
	mat->m[2][0] = vecZ.x;
	mat->m[2][1] = vecZ.y;
	mat->m[2][2] = vecZ.z;
	mat->m[2][3] = 0.0f;
}

// sceVu0ViewScreenMatrix 互換
void DG_ViewScreenMatrix(
		MATRIX &res,
		float scrz,              // スクリーンまでの距離
		float ax, float ay,      // アスペクト比
		float cx, float cy,      // スクリーンの中心座標
		float zmin, float zmax,  // Zバッファ最小値/最大値
		float nearz, float farz) // ニアクリップ面のZ,ファークリップ面のZ
{
	float	az, cz;
	MATRIX  m, mt;

	cz = (- zmax * nearz + zmin * farz) / (farz - nearz);
	az  = farz * nearz * (- zmin + zmax) / (farz - nearz);

	//     | scrz    0  0 0 |
	// m = |    0 scrz  0 0 | 
	//     |    0    0  0 1 |
	//     |    0    0  1 0 |
	D3DXMatrixIdentity(&m);
	m.m[0][0] = scrz; // スクリーンまでの距離
	m.m[1][1] = scrz;
	m.m[2][2] = 0.0f;
	m.m[3][3] = 0.0f;
	m.m[3][2] = 1.0f;
	m.m[2][3] = 1.0f;

	//      | ax  0  0 cx |
	// mt = |  0 ay  0 cy | 
	//      |  0  0 az cz |
	//      |  0  0  0  1 |
	D3DXMatrixIdentity(&mt);
	mt.m[0][0] = - ax; // PS2版とx軸方向を逆にする
	mt.m[1][1] = ay;
	mt.m[2][2] = az;
	mt.m[3][0] = cx;
	mt.m[3][1] = cy;
	mt.m[3][2] = cz;

	D3DXMatrixMultiply(&res, &m, &mt);
}


/* カメラマトリクスを用いてカメラを設定 */
void DG_SetCamera(DG_CHANL *chanl, MATRIX *mat, float screen)
{
	chanl->screen = screen;
	chanl->eye = *mat;
	D3DXMatrixInverse(&chanl->eye_inv, NULL, &chanl->eye);

	// 透視変換行列を生成する
	DG_ViewScreenMatrix(chanl->pers,
						chanl->screen,
						ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, // アスペクト比
						0.0f, 0.0f,                             // ＸＹオフセット値
						//(float)-1.0f, (float)1.0f,            // 変換後Ｚ範囲(PS2版)
						(float)0.0f, (float)1.0f,               // 変換後Ｚ範囲
						(float)DG_ClipNear, (float)DG_ClipFar); // Ｚクリップ
	DG_ViewScreenMatrix(chanl->pers2,
						chanl->screen * (chanl->width / 2),
						ASPECT_X(), ASPECT_Y(),
						2048.0f, 2048.0f,
						(float)DRAW_Z_MIN, (float)DRAW_Z_MAX,
						(float)DG_ClipNear, (float)DG_ClipFar);
	DG_ViewScreenMatrix(chanl->raise_pers,
						chanl->screen,
						ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height,
						0.0f, 0.0f,
						//(float)-1.0f, (float)1.0f,
						(float)0.0f, (float)1.0f,
						(float)DG_ClipNear+0.05f, (float)DG_ClipFar+0.0f);
	DG_ViewScreenMatrix(chanl->raise_pers2,
						chanl->screen * (chanl->width / 2),
						ASPECT_X(), ASPECT_Y(),
						2048.0f, 2048.0f,
						(float)DRAW_Z_MIN, (float)DRAW_Z_MAX,
						(float)DG_ClipNear+0.05f, (float)DG_ClipFar+0.0f);

	D3DXMatrixMultiply(&chanl->eye_pers, &chanl->eye_inv, &chanl->pers);
	D3DXMatrixMultiply(&chanl->eye_pers2, &chanl->eye_inv, &chanl->pers2);
	D3DXMatrixMultiply(&chanl->raise_eye_pers, &chanl->eye_inv, &chanl->raise_pers);
	D3DXMatrixMultiply(&chanl->raise_eye_pers2, &chanl->eye_inv, &chanl->raise_pers2);
	//_sceVu0MulMatrix(&chanl->eye_pers, &chanl->pers, &chanl->eye_inv);
	//_sceVu0MulMatrix(&chanl->eye_pers2, &chanl->pers2, &chanl->eye_inv);
	//_sceVu0MulMatrix(&chanl->raise_eye_pers, &chanl->raise_pers, &chanl->eye_inv);
	//_sceVu0MulMatrix(&chanl->raise_eye_pers2, &chanl->raise_pers2, &chanl->eye_inv);
}


void DG_SetCamera2(DG_CHANL *chanl, VECTOR *vecFrom, VECTOR *vecTo, float screen)
{
	MATRIX mat, mat2, mat3;

#if 0
	// テスト
	vecTo->x = 0.0f;
	vecTo->y = 0.0f;
	vecTo->z = 0.0f;
	vecFrom->x = 0.0f;
	vecFrom->y = 4000.0f;
	vecFrom->z = 2000.0f;
#endif
	
	DG_MakeCameraMatrix(&mat, vecFrom, vecTo);

	// カメラ逆行列を計算する
	D3DXMatrixInverse(&mat2, NULL, &mat);
	// Z 回転
	D3DXMatrixRotationZ(&mat3, vecFrom->w);
	//mat2 = (D3DXMATRIX)mat3 * (D3DXMATRIX)mat2;
	D3DXMatrixMultiply(&mat2, &mat3, &mat2);
	D3DXMatrixInverse(&mat, NULL, &mat2);

	// カメラ設定
	DG_SetCamera(chanl, &mat, screen);

#ifdef ORIGINAL
	FMATRIX		mat, mat2 ;
	DG_MakeCameraMatrix( &mat, from, to );
	/*
		カメラ逆行列を計算する
	*/
	_sceVu0InversMatrix( &mat2, &mat );

	/* Ｚ回転 */
	_sceVu0RotMatrixZ( &mat2, &mat2, from->vw ) ;
	_sceVu0InversMatrix( &mat, &mat2 ) ;
	/* カメラ設定 */
	DG_SetCamera( chanl, &mat, screen );
#endif
}


//-----------------------------------------------------------------------------

// フレーム初期化パケット接続チャンネル
void DG_FrameChanl(DG_CHANL *cp, int which)
{
	//printf("DG_FrameChanl(%d)\n", which);
}

//-----------------------------------------------------------------------------
// 表示グループユーティリティ
//-----------------------------------------------------------------------------

/* 表示グループに追加 */
void DG_AddCurrentGroup(int id)
{
    DG_Chanl(0)->group_id |= id;
    DG_Chanl(1)->group_id |= id;
    DG_Chanl(2)->group_id |= id;
    DG_Chanl(3)->group_id |= id;
}

/* 表示グループから削除 */
void DG_DeleteCurrentGroup(int id)
{
    DG_Chanl(0)->group_id &= ~id;
    DG_Chanl(1)->group_id &= ~id;
    DG_Chanl(2)->group_id &= ~id;
    DG_Chanl(3)->group_id &= ~id;
}

/* 表示グループに追加 */
void DG_AddCurrentGroup2(int chanl, int id)
{
    DG_Chanl(chanl)->group_id |= id;
}

/* 表示グループから削除 */
void DG_DeleteCurrentGroup2(int chanl, int id)
{
    DG_Chanl(chanl)->group_id &= ~id;
}
