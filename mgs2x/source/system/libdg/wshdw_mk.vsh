; 影生成用シェーダー(Windows版)
;
; 2002/04/03  K.Takabe
; 2002/08/21  E.Takaki
; $Id: wshdw_mk.vsh,v 1.2 2002/09/23 07:56:38 takaki Exp $
;

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
mul r0, v0.x, c[ CV_WORLD0 + 0 ]
mad r0, v0.y, c[ CV_WORLD0 + 1 ], r0
mad r0, v0.z, c[ CV_WORLD0 + 2 ], r0
add r0, c[ CV_WORLD0 + 3 ], r0			; w値入力無し

mul R_TMP.w, c[ CV_SCALE ].z, V_WEIGHT_WIN.w	; 正規化(1/(1 << 15))

; 頂点/法線・ワールド行列1
mul r1, v0.x, c[ CV_WORLD1 + 0 ]
mad r1, v0.y, c[ CV_WORLD1 + 1 ], r1
mad r1, v0.z, c[ CV_WORLD1 + 2 ], r1
add r1, c[ CV_WORLD1 + 3 ], r1			; w値入力無し

add r4.w, c[CV_ONE].x,       -R_TMP.w			; r4 = 1.0f - weight
mul r0.xyz,  r0.xyz,          R_TMP.w			; r0 *= weight
mad r0.xyz,  r1.xyz,          r4.w,      r0.xyz	; r0 += r1 * (1.0f - weight)

mov oFog.x, c[ CV_ZERO ]				; Fog

; 透視変換
mul r1, r0.x, c[ CV_EYEPERS + 0 ]
mad r1, r0.y, c[ CV_EYEPERS + 1 ], r1
mad r1, r0.z, c[ CV_EYEPERS + 2 ], r1
mad r1, r0.w, c[ CV_EYEPERS + 3 ], r1

mov oD0.w, c[ CV_HALF ].x				; 光源計算[1]
mov oPos, r1							; 座標設定
mov r5.x, c[ CV_SCALE ].y				; 光源計算[2]
mul oD0.xyz, c[CV_LIGHTCOL + 3], r5.x	; 光源計算[Last]
