; 影生成用シェーダー
;
; 2002/04/03  K.Takabe
; $Id: shdw_mk.vsh,v 1.1.1.3 2002/11/19 11:42:24 Yoshizawa1 Exp $
;

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
mul r0, v0.x, c[ CV_WORLD0 + 0 ]
mad r0, v0.y, c[ CV_WORLD0 + 1 ], r0
mad r0, v0.z, c[ CV_WORLD0 + 2 ], r0
mad r0, v0.w, c[ CV_WORLD0 + 3 ], r0
;mul r2.xyz, v2.x, c[ CV_WORLD0 + 0 ].xyz
;mad r2.xyz, v2.y, c[ CV_WORLD0 + 1 ].xyz, r2.xyz
;mad r2.xyz, v2.z, c[ CV_WORLD0 + 2 ].xyz, r2.xyz

; 頂点/法線・ワールド行列1
mul r1, v0.x, c[ CV_WORLD1 + 0 ]
mad r1, v0.y, c[ CV_WORLD1 + 1 ], r1
mad r1, v0.z, c[ CV_WORLD1 + 2 ], r1
mad r1, v0.w, c[ CV_WORLD1 + 3 ], r1
;mul r3.xyz, v2.x, c[ CV_WORLD1 + 0 ].xyz
;mad r3.xyz, v2.y, c[ CV_WORLD1 + 1 ].xyz, r3.xyz
;mad r3.xyz, v2.z, c[ CV_WORLD1 + 2 ].xyz, r3.xyz

mul r0.xyz, r0.xyz, V_WEIGHT.x					; r0 *= weight
;mul r2.xyz, r2.xyz, V_WEIGHT.x			; r2 *= weight
add R_TMP, c[CV_ONE].x, -V_WEIGHT.x		; R_TMP = 1.0f - weight
mad r0.xyz, r1.xyz, R_TMP, r0.xyz		; r0 += r1 * (1.0f - weight)
;mad r2.xyz, r3.xyz, R_TMP, r2			; r2 += r3 * (1.0f - weight)

; 透視変換
mul r1, r0.x, c[ CV_EYEPERS + 0 ]
mad r1, r0.y, c[ CV_EYEPERS + 1 ], r1
mad r1, r0.z, c[ CV_EYEPERS + 2 ], r1
mad r1, r0.w, c[ CV_EYEPERS + 3 ], r1
mov oPos, r1

;mul r2.x, r1.w, c[ CV_FOG ].x
;add oFog.x, c[ CV_FOG ].y, r2.x


; 法線正規化
;dp3 r2.w, r2, r2
;rsq r2.w, r2.w
;mul r2, r2, r2.w

; 光源計算(とりあえずMGS2に従って平行光源3つ)
;mul	R_TMP.xyz, R_NORMAL.x, c[ CV_LIGHTVEC + 0 ].xyz		; 
;mad	R_TMP.xyz, R_NORMAL.y, c[ CV_LIGHTVEC + 1 ].xyz, R_TMP.xyz		; 
;mad	R_TMP.xyz, R_NORMAL.z, c[ CV_LIGHTVEC + 2 ].xyz, R_TMP.xyz		; 
;max R_TMP.xyz, R_TMP.xyz, c[CV_ZERO]					; R_TMP >= 0 を保証

;mov R_COL.xyz, c[CV_LIGHTCOL + 3]
;mad R_COL.xyz, R_TMP.x, c[CV_LIGHTCOL + 0], R_COL.xyz	; 光源強度1 x 光源色1
;mad R_COL.xyz, R_TMP.y, c[CV_LIGHTCOL + 1], R_COL.xyz	; + 光源強度2 x 光源色2
;mad R_COL.xyz, R_TMP.z, c[CV_LIGHTCOL + 2], R_COL.xyz	; + 光源強度3 x 光源色3
;mul oD0.xyz, R_COL.xyz, c[CV_SCALE].y		; 1/255にスケーリング
mov oD0.w, c[ CV_HALF ].x
mov r5.x, c[ CV_SCALE ].y
mul oD0.xyz, c[CV_LIGHTCOL + 3], r5.x

;mov oT0, V_TEX0								; テクスチャ座標0
mov oFog.x, c[ CV_ZERO ]
