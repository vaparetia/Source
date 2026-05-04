; 曲面パッチ描画用頂点シェーダ
;
; 2001/06/04  F.Miyauchi
; $Id: patchl.vsh,v 1.1.1.3 2002/11/19 11:42:18 Yoshizawa1 Exp $
;
; 光源計算(1平行光源 + アンビエント) + フォグ付き
;

#include "shader.h"

vs.1.1

; 頂点変換
dp4 oPos.x, V_POSITION, c[CV_WORLDEYEPERS + 0]
dp4 oPos.y, V_POSITION, c[CV_WORLDEYEPERS + 1]
dp4 oPos.z, V_POSITION, c[CV_WORLDEYEPERS + 2]
dp4 R_POSITION.w, V_POSITION, c[CV_WORLDEYEPERS + 3]
mov oPos.w, R_POSITION.w								; oPosは書き込み専用

mov oT0, V_TEX0                 						; テクスチャ座標0

; 頂点/法線変換
dp3 R_NORMAL.x, V_NORMAL, c[CV_WORLD0 + 0]
dp3 R_NORMAL.y, V_NORMAL, c[CV_WORLD0 + 1]
dp3 R_NORMAL.z, V_NORMAL, c[CV_WORLD0 + 2]

; 法線正規化
dp3 R_NORMAL.w, R_NORMAL, R_NORMAL
rsq R_NORMAL.w, R_NORMAL.w
mul R_NORMAL, R_NORMAL, R_NORMAL.w

; 光源計算
dp3 R_TMP.x, R_NORMAL, c[CV_LIGHTVEC]					; 法線・ローカルライト
max R_TMP.x, R_TMP.x, c[CV_ZERO]						; R_TMP >= 0 を保証
mul R_COL.xyz, R_TMP.xxx, c[CV_LIGHTCOL]				; 光源強度 x 光源色

mad oD0.xyz, V_DIFFUSE.xyz, c[CV_AMBIENT], R_COL.xyz 	; アンビエント加算
mov oD0.w, V_DIFFUSE.w									; アルファは入力のまま

;mov oD0, V_DIFFUSE

; フォグ計算
; oPos.wに相当する R_POSITION.w を使う
; oFog.x : フォグ(0.0が手前～1.0が奥)

mov R_TMP.w, c[CV_FOG].y							; fog_param2
mad R_TMP.w, R_POSITION.w, c[CV_FOG].x, R_TMP.w		;  + w * fog_param1
mul R_TMP.w, R_TMP.w, c[CV_FOG].z					; 1 / 255
max R_TMP.w, R_TMP.w, c[CV_ZERO].x					; >= 0
min R_TMP.w, R_TMP.w, c[CV_FOG].w					; <= 1
add oFog.x, c[CV_FOG].w, -R_TMP.w;					; DirectXに合わせて0と1を逆に

#ifdef FOG_TEST
mul R_TMP.w, R_POSITION.w, c[CV_CONST2].z
max R_TMP.w, R_TMP.w, c[CV_ZERO].x
min R_TMP.w, R_TMP.w, c[CV_CONST0].w
mov oFog.x, R_TMP.w
#endif
