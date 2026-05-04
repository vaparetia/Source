; 曲面パッチ描画用頂点シェーダ
; バンプマップテスト フォグなし
; 2001/06/12  M.Kobayashi
; $Id: patchbl.vsh,v 1.1.1.3 2002/11/19 11:42:18 Yoshizawa1 Exp $
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
mov oT1, V_TEX0                 						; テクスチャ座標1(bump)

; 頂点/法線変換
dp3 R_NORMAL.x, V_NORMAL, c[CV_WORLD0 + 0]
dp3 R_NORMAL.y, V_NORMAL, c[CV_WORLD0 + 1]
dp3 R_NORMAL.z, V_NORMAL, c[CV_WORLD0 + 2]

; 法線正規化
dp3 R_NORMAL.w, R_NORMAL, R_NORMAL
rsq R_NORMAL.w, R_NORMAL.w
mul R_NORMAL, R_NORMAL, R_NORMAL.w

; S(CV_CONST5), T(CV_CONST6) よりバンプ用光源ベクトルをタンジェントスペースに変換
; Norm(n X ( S X n )) -> S とする
mul R_SVEC, c[CV_CONST5].yzx, R_NORMAL.zxy
mul R_TVEC, c[CV_CONST6].yzx, R_NORMAL.zxy
mad R_SVEC, -c[CV_CONST5].zxy, R_NORMAL.yzx, R_SVEC
mad R_TVEC, -c[CV_CONST6].zxy, R_NORMAL.yzx, R_TVEC
mul R_TMP, R_NORMAL.yzx, R_SVEC.zxy
mad R_SVEC, -R_NORMAL.zxy, R_SVEC.yzx, R_TMP
mul R_TMP, R_NORMAL.yzx, R_TVEC.zxy
mad R_TVEC, -R_NORMAL.zxy, R_TVEC.yzx, R_TMP
dp3 R_SVEC.w, R_SVEC, R_SVEC
rsq R_SVEC.w, R_SVEC.w
mul R_SVEC, R_SVEC, R_SVEC.w
dp3 R_TVEC.w, R_TVEC, R_TVEC
rsq R_TVEC.w, R_TVEC.w
mul R_TVEC, R_TVEC, R_TVEC.w
;mov R_SVEC, c[CV_CONST5]
;mov R_TVEC, c[CV_CONST6]

; Hベクトル計算
dp4 R_POSITION.x, V_POSITION, c[CV_WORLD + 0]
dp4 R_POSITION.y, V_POSITION, c[CV_WORLD + 1]
dp4 R_POSITION.z, V_POSITION, c[CV_WORLD + 2]
sub R_TMP, R_POSITION, c[CV_ADDLIGHTVEC + 1]
dp3 R_TMP.w, R_TMP, R_TMP
rsq R_TMP.w, R_TMP.w
mul R_TMP, R_TMP, R_TMP.w
add R_TMP, R_TMP, c[CV_ADDLIGHTVEC]
dp3 R_TMP.w, R_TMP, R_TMP
rsq R_TMP.w, R_TMP.w
mul R_TMP, R_TMP, R_TMP.w


dp3 oT2.x, R_SVEC, c[CV_ADDLIGHTVEC]
dp3 oT2.y, R_TVEC, c[CV_ADDLIGHTVEC]
dp3 oT2.z, R_NORMAL, c[CV_ADDLIGHTVEC]
;dp3 oT3.x, R_SVEC, c[CV_ADDLIGHTVEC + 1]
;dp3 oT3.y, R_TVEC, c[CV_ADDLIGHTVEC + 1]
;dp3 oT3.z, R_NORMAL, c[CV_ADDLIGHTVEC + 1]
;dp3 oT3.x, R_SVEC, R_TMP
;dp3 oT3.y, R_TVEC, R_TMP
;dp3 oT3.z, R_NORMAL, R_TMP
dp3 R_HVEC.x, R_SVEC, R_TMP
dp3 R_HVEC.y, R_TVEC, R_TMP
dp3 R_HVEC.z, R_NORMAL, R_TMP
mul R_HVEC.xyz, R_HVEC.xyz, c[CV_CONST4].yyyy
add oT3.xyz, R_HVEC.xyz, c[CV_CONST4].yyyy

mov oD1, c[CV_ADDLIGHTCOL]

; 光源計算
dp3 R_TMP.x, R_NORMAL, c[CV_LIGHTVEC]					; 法線・ローカルライト
max R_TMP.x, R_TMP.x, c[CV_ZERO]						; R_TMP >= 0 を保証
mul R_COL.xyz, R_TMP.xxx, c[CV_LIGHTCOL]				; 光源強度 x 光源色

mad oD0.xyz, V_DIFFUSE.xyz, c[CV_AMBIENT], R_COL.xyz 	; アンビエント加算
mov oD0.w, V_DIFFUSE.w									; アルファは入力のまま

;mov oD0, V_DIFFUSE

