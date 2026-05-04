; OBJ描画用頂点シェーダ
;
; 2001/05/11  F.Miyauchi
; $Id: objw.vsh,v 1.1.1.3 2002/11/19 11:42:15 Yoshizawa1 Exp $
;
; 平行光源    OFF
; ウェイト    ON
; 点光源      OFF
; 追加光源    OFF
;

#include "shader.h"

vs.1.1

; 頂点・ワールド行列0
dp4 r0.x, v0, c[CV_WORLD0 + 0]
dp4 r0.y, v0, c[CV_WORLD0 + 1]
dp4 r0.z, v0, c[CV_WORLD0 + 2]
dp4 r0.w, v0, c[CV_WORLD0 + 3]

; 頂点・ワールド行列1
dp4 r1.x, v0, c[CV_WORLD1 + 0]
dp4 r1.y, v0, c[CV_WORLD1 + 1]
dp4 r1.z, v0, c[CV_WORLD1 + 2]
dp4 r1.w, v0, c[CV_WORLD1 + 3]

mul r0, r0, V_WEIGHT.x				; ウェイト値乗算
add r3, c[CV_ONE], -V_WEIGHT.x		; r3 = 1.0f - weight
mad r0, r1, r3, r0

; 透視変換
dp4 oPos.x, r0, c[CV_EYEPERS + 0]
dp4 oPos.y, r0, c[CV_EYEPERS + 1]
dp4 oPos.z, r0, c[CV_EYEPERS + 2]
dp4 oPos.w, r0, c[CV_EYEPERS + 3]

mov oD0, V_DIFFUSE					; カラー出力
mov oT0, V_TEX0						; テクスチャ座標0
mov oT1, V_TEX1						; テクスチャ座標1
mov oT2, V_TEX2						; テクスチャ座標2
