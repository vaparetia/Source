; 影描画用シェーダ
;
; 2001/05/14  F.Miyauchi
; $Id: shadow.vsh,v 1.1.1.3 2002/11/19 11:42:24 Yoshizawa1 Exp $


#include "shader.h"

vs.1.1

; 頂点・ワールド行列
dp4 r0.x, v0, c[CV_WORLD0 + 0]
dp4 r0.y, v0, c[CV_WORLD0 + 1]
dp4 r0.z, v0, c[CV_WORLD0 + 2]
dp4 r0.w, v0, c[CV_WORLD0 + 3]

; 透視変換
dp4 oPos.x, r0, c[CV_EYEPERS + 0]
dp4 oPos.y, r0, c[CV_EYEPERS + 1]
dp4 oPos.z, r0, c[CV_EYEPERS + 2]
dp4 oPos.w, r0, c[CV_EYEPERS + 3]

dp4 r1.x, v0, c[CV_MATSHADOW + 0]
dp4 r1.y, v0, c[CV_MATSHADOW + 1]
dp4 r1.z, v0, c[CV_MATSHADOW + 2]
dp4 r1.w, v0, c[CV_MATSHADOW + 3]

rcp r2, r1.w                      ; r1.w = 1 / r1.w
mul r1.x, r1.x, r2                ; r1.x = r1.x / r1.w
mul r1.y, r1.y, -r2               ; r1.y = - r1.y / r1.w
mul r1.xy, r1.xy, c[CV_HALF].x    ; r1.xy *= 0.5f
add r1.xy, r1.xy, c[CV_HALF].x    ; r1 += 0.5f

mov oD0.xyz, v3.xyz               ; カラー出力
mov oD0.w, c[CV_SHADOWALPHA].x    ; アルファ出力
mov oT0.xy, r1.xy                 ; テクスチャ座標0
