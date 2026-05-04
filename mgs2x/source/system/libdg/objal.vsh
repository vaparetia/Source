; OBJ描画用頂点シェーダ
;
; 2001/05/11  F.Miyauchi
; $Id: objal.vsh,v 1.1.1.3 2002/11/19 11:42:14 Yoshizawa1 Exp $
;
; 平行光源    ON
; ウェイト    OFF
; 点光源      ON
; 追加光源    ON
;

#include "shader.h"

vs.1.1

; 頂点/法線変換
dp4 R_POSITION.x, V_POSITION, c[CV_WORLD0 + 0]
dp4 R_POSITION.y, V_POSITION, c[CV_WORLD0 + 1]
dp4 R_POSITION.z, V_POSITION, c[CV_WORLD0 + 2]
dp4 R_POSITION.w, V_POSITION, c[CV_WORLD0 + 3]
dp3 R_NORMAL.x, V_NORMAL, c[CV_WORLD0 + 0]
dp3 R_NORMAL.y, V_NORMAL, c[CV_WORLD0 + 1]
dp3 R_NORMAL.z, V_NORMAL, c[CV_WORLD0 + 2]

; 透視変換
dp4 oPos.x, R_POSITION, c[CV_EYEPERS + 0]
dp4 oPos.y, R_POSITION, c[CV_EYEPERS + 1]
dp4 oPos.z, R_POSITION, c[CV_EYEPERS + 2]
dp4 oPos.w, R_POSITION, c[CV_EYEPERS + 3]

; 法線正規化
dp3 R_NORMAL.w, R_NORMAL, R_NORMAL
rsq R_NORMAL.w, R_NORMAL.w
mul R_NORMAL, R_NORMAL, R_NORMAL.w

#include "light.vsh"						; 平行光源
#include "plight.vsh"               		; 点光源
#include "addlight.vsh"						; 追加光源

add R_TMP.xyz, R_COL.xyz, R_PLIGHT_COL.xyz	; light.vsh出力 + plight.vsh出力
add oD0.xyz, R_TMP.xyz, R_ADDCOL.xyz		; + addlight.vsh 出力

mov oD0.w, V_DIFFUSE.w						; アルファは入力のまま
mov oT0, V_TEX0								; テクスチャ座標0
mov oT1, V_TEX1								; テクスチャ座標1
mov oT2, V_TEX2								; テクスチャ座標2
