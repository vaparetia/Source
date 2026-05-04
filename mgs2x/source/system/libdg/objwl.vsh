; OBJ描画用頂点シェーダ
;
; 2001/05/11  F.Miyauchi
; $Id: objwl.vsh,v 1.1.1.3 2002/11/19 11:42:16 Yoshizawa1 Exp $
;
; 平行光源    ON
; ウェイト    ON
; 点光源      OFF
; 追加光源    OFF
;
; r0       : 頂点0
; r1       : 頂点1
; r2       : 法線0
; r3       : 法線1
; r4       : 1.0 - weight

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
dp4 r0.x, v0, c[CV_WORLD0 + 0]
dp4 r0.y, v0, c[CV_WORLD0 + 1]
dp4 r0.z, v0, c[CV_WORLD0 + 2]
dp4 r0.w, v0, c[CV_WORLD0 + 3]
dp3 r2.x, v2, c[CV_WORLD0 + 0]
dp3 r2.y, v2, c[CV_WORLD0 + 1]
dp3 r2.z, v2, c[CV_WORLD0 + 2]

; 頂点/法線・ワールド行列1
dp4 r1.x, v0, c[CV_WORLD1 + 0]
dp4 r1.y, v0, c[CV_WORLD1 + 1]
dp4 r1.z, v0, c[CV_WORLD1 + 2]
dp4 r1.w, v0, c[CV_WORLD1 + 3]
dp3 r3.x, v2, c[CV_WORLD1 + 0]
dp3 r3.y, v2, c[CV_WORLD1 + 1]
dp3 r3.z, v2, c[CV_WORLD1 + 2]

mul r0, r0, V_WEIGHT.x					; r0 *= weight
mul r2.xyz, r2.xyz, V_WEIGHT.x			; r2 *= weight
add R_TMP, c[CV_ONE].x, -V_WEIGHT.x		; R_TMP = 1.0f - weight
mad r0, r1, R_TMP, r0					; r0 += r1 * (1.0f - weight)
mad r2.xyz, r3.xyz, R_TMP, r2			; r2 += r3 * (1.0f - weight)

; 透視変換
dp4 oPos.x, r0, c[CV_EYEPERS + 0]
dp4 oPos.y, r0, c[CV_EYEPERS + 1]
dp4 oPos.z, r0, c[CV_EYEPERS + 2]
dp4 oPos.w, r0, c[CV_EYEPERS + 3]

; 法線正規化
dp3 r2.w, r2, r2
rsq r2.w, r2.w
mul r2, r2, r2.w

#include "light.vsh"						; 平行光源 + アンビエント
#include "plight.vsh"						; 点光源(重ければ別シェーダを用意する)

add oD0.xyz, R_COL.xyz, R_PLIGHT_COL.xyz	; light.vsh出力 + plight.vsh出力
mov oD0.w, V_DIFFUSE.w              		; アルファは入力のまま

mov oT0, V_TEX0								; テクスチャ座標0
mov oT1, V_TEX1								; テクスチャ座標1
mov oT2, V_TEX2                  			; テクスチャ座標2
