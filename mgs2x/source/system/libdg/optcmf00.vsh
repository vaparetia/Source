; ＫＭＳ用（シングルテクスチャ＆通常ライティング）
;
; 2002/03/13  K.Takabe
; $Id: optcmf00.vsh,v 1.1.1.3 2002/11/19 11:42:16 Yoshizawa1 Exp $
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
mul r2, v0.x, c[ CV_WORLD0 + 0 ]
mad r2, v0.y, c[ CV_WORLD0 + 1 ], r2
mad r2, v0.z, c[ CV_WORLD0 + 2 ], r2
mad r2, v0.w, c[ CV_WORLD0 + 3 ], r2
mul r3, v0.x, c[ CV_WORLD1 + 0 ]
mad r3, v0.y, c[ CV_WORLD1 + 1 ], r3
mad r3, v0.z, c[ CV_WORLD1 + 2 ], r3
mad r3, v0.w, c[ CV_WORLD1 + 3 ], r3

mul r4.xyz, v2.x, c[ CV_WORLD0 + 0 ].xyz
mad r4.xyz, v2.y, c[ CV_WORLD0 + 1 ].xyz, r4.xyz
mad r4.xyz, v2.z, c[ CV_WORLD0 + 2 ].xyz, r4.xyz
mul r5.xyz, v2.x, c[ CV_WORLD1 + 0 ].xyz
mad r5.xyz, v2.y, c[ CV_WORLD1 + 1 ].xyz, r5.xyz
mad r5.xyz, v2.z, c[ CV_WORLD1 + 2 ].xyz, r5.xyz

add r6.x, c[CV_ONE].x, -V_WEIGHT.x		; r6.x = 1.0f - weight
mul r2.xyz, r2.xyz, V_WEIGHT.x			; r2 *= weight
mul r4.xyz, r4.xyz, V_WEIGHT.x			; r4 *= weight
mad r2.xyz, r3.xyz, r6.x, r2.xyz		; r2 += r3 * (1.0f - weight)
mad r4.xyz, r5.xyz, r6.x, r4			; r4 += r5 * (1.0f - weight)

; 透視変換
mul r3, r2.x, c[ CV_EYEPERS + 0 ]
mad r3, r2.y, c[ CV_EYEPERS + 1 ], r3
mad r3, r2.z, c[ CV_EYEPERS + 2 ], r3
mad r3, r2.w, c[ CV_EYEPERS + 3 ], r3
mov oPos, r3

mul r6.x, r3.w, c[ CV_FOG ].x
add oFog.x, c[ CV_FOG ].y, r6.x


; 法線正規化
dp3 r5.x, r4.xyz, r4.xyz
rsq r1.x, r5.x
mul r4.xyz, r4.xyz, r1.x

; 環境マッピング計算（法線を元にゆがませる）
rcp r1.x, r3.w
;mul r7, r2.x, c[ CV_OPTCMF_EYEPERS + 0 ]
;mad r7, r2.y, c[ CV_OPTCMF_EYEPERS + 1 ], r7
;mad r7, r2.z, c[ CV_OPTCMF_EYEPERS + 2 ], r7
;mad r7, r2.w, c[ CV_OPTCMF_EYEPERS + 3 ], r7
mul r5.xy, r4.x, c[ CV_EYE_INV + 0 ]
mad r5.xy, r4.y, c[ CV_EYE_INV + 1 ], r5
mad r5.xy, r4.z, c[ CV_EYE_INV + 2 ], r5
mul r7.xy, r3.xy, r1.x						; 座標の透視変換
mul r7.xy, r7.xy, c[ CV_LIGHTCOL + 0 ]		; テクスチャ座標生成（テクスチャ座標系へスケーリング）
add r7.xy, c[ CV_LIGHTCOL + 1 ], r7			; テクスチャ座標生成（テクスチャ座標系へオフセット加算）
mad r7.xy, r5.xy, c[ CV_LIGHTCOL + 2 ], r7	; テクスチャ座標生成（法線の影響度を加算）

mov oT0.w, r3.w
mul oT0.xy, r7, r3.w

; カラー出力
mov r8.x, c[ CV_SCALE ].y
mul oD0.xyzw, c[CV_LIGHTCOL + 3].xyzw, r8.x

