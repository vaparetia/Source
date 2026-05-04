; ＫＭＳ用(Windows(1 Texture)版)（マルチテクスチャ＆通常ライティング）
;
; 2002/04/19  K.Takabe
; 2002/08/20  E.Takaki
; $Id: wkms_ml_1tex.vsh,v 1.4 2002/11/12 04:42:31 takaki Exp $
;

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
mul r2, v0.x, c[ CV_WORLD0 + 0 ]
mad r2, v0.y, c[ CV_WORLD0 + 1 ], r2
mad r2, v0.z, c[ CV_WORLD0 + 2 ], r2
add r2, c[ CV_WORLD0 + 3 ], r2					; w座標の入力はありません
mul r4.xyz, v2.x, c[ CV_WORLD0 + 0 ].xyz
mad r4.xyz, v2.y, c[ CV_WORLD0 + 1 ].xyz, r4.xyz
mad r4.xyz, v2.z, c[ CV_WORLD0 + 2 ].xyz, r4.xyz

; 頂点/法線・ワールド行列1
mul r3, v0.x, c[ CV_WORLD1 + 0 ]
mad r3, v0.y, c[ CV_WORLD1 + 1 ], r3
mad r3, v0.z, c[ CV_WORLD1 + 2 ], r3
add r3, c[ CV_WORLD1 + 3 ], r3					; w座標の入力はありません
mul r5.xyz, v2.x, c[ CV_WORLD1 + 0 ].xyz
mad r5.xyz, v2.y, c[ CV_WORLD1 + 1 ].xyz, r5.xyz
mad r5.xyz, v2.z, c[ CV_WORLD1 + 2 ].xyz, r5.xyz

mul	R_TMP, c[ CV_SCALE ].z, V_WEIGHT_WIN.w	; 正規化(1/(1 << 5))
add r6, c[CV_ONE].x, -R_TMP				; r6 = 1.0f - weight
mul r2.xyz, r2.xyz, R_TMP				; r0 *= weight
mul r4.xyz, r4.xyz, R_TMP				; r2 *= weight
mad r2.xyz, r3.xyz, r6, r2.xyz			; r0 += r1 * (1.0f - weight)
mad r4.xyz, r5.xyz, r6, r4				; r2 += r3 * (1.0f - weight)

; 透視変換
mul r3, r2.x, c[ CV_EYEPERS + 0 ]
mad r3, r2.y, c[ CV_EYEPERS + 1 ], r3
mad r3, r2.z, c[ CV_EYEPERS + 2 ], r3
mad r3, r2.w, c[ CV_EYEPERS + 3 ], r3

dp3 r0.x, r4, r4				; 法線正規化(1)
mov oPos, r3					; 座標設定
rsq r1.x, r0.x					; 法線正規化(1)
mul r2.x, r3.w, c[ CV_FOG ].x	; Fog設定(1)
mul r4.xyz, r4.xyz, r1.x		; 法線正規化(Last)
add oFog.x, c[ CV_FOG ].y, r2.x	; Fog設定(Last)


; 光源計算(とりあえずMGS2に従って平行光源3つ)
mul	r5.xyz, r4.x, c[ CV_LIGHTVEC + 0 ].xyz				; 
mad	r5.xyz, r4.y, c[ CV_LIGHTVEC + 1 ].xyz, r5.xyz		; 
mad	r5.xyz, r4.z, c[ CV_LIGHTVEC + 2 ].xyz, r5.xyz		; 

mov r6.xyz, c[CV_LIGHTCOL + 3]
max r5.xyz, r5.xyz, c[CV_ZERO]							; R_TMP >= 0 を保証
mad r6.xyz, r5.x, c[CV_LIGHTCOL + 0], r6.xyz			; 光源強度1 x 光源色1
mad r6.xyz, r5.y, c[CV_LIGHTCOL + 1], r6.xyz			; + 光源強度2 x 光源色2
mad r6.xyz, r5.z, c[CV_LIGHTCOL + 2], r6.xyz			; + 光源強度3 x 光源色3
mov r6.w, c[ CV_HALF ].x								; A成分はＭＧＳ２に合わせて１００％のみ
;mul r6.xyz, r6.xyz, c[CV_SCALE].y					; 1/255にスケーリング

;mul r11.xyzw, v3, c[ CV_CONST0 ]
;mul oD0, r6, r11										; 頂点カラー参照

mul oD0, r6, c[ CV_CONST0 ]								; 頂点カラー参照(By Takaki)

#ifndef UVADJUST
mov oT0.xy, V_TEX0								; テクスチャ座標0
//mov oT1.xy, V_TEX1							; テクスチャ座標1
//mov oT2.xy, V_TEX2                  			; テクスチャ座標2

#else

mul r9,  V_TEX0, c[ CV_TEX0_SCALE ]
//mul r10, V_TEX1, c[ CV_TEX1_SCALE ]
//mul r11, V_TEX2, c[ CV_TEX2_SCALE ]
add oT0,  r9, c[ CV_TEX0_OFFSET ]
//add oT1.xy, r10, c[ CV_TEX1_OFFSET ]
//add oT2.xy, r11, c[ CV_TEX2_OFFSET ]

#endif
