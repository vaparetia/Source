; 曲面パッチ描画用頂点シェーダ（環境マップ版）強制平面描画版
;
; 2002/05/10  K.Takabe
; $Id: wpatch2_emap.vsh,v 1.1 2002/09/23 07:56:38 takaki Exp $
;

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
mul r2, v0.x, c[ CV_WORLD0 + 0 ]
mad r2, v0.y, c[ CV_WORLD0 + 1 ], r2
mad r2, v0.z, c[ CV_WORLD0 + 2 ], r2
mad r2, v0.w, c[ CV_WORLD0 + 3 ], r2
mul r5, v0.x, c[ CV_WORLD0 + 0 ]						; 実際に描画する座標（Ｙ＝０）
mad r5, v0.z, c[ CV_WORLD0 + 2 ], r5					; 
mad r5, v0.w, c[ CV_WORLD0 + 3 ], r5					; 
mul r4.xyz, v2.x, c[ CV_WORLD0 + 0 ].xyz
mad r4.xyz, v2.y, c[ CV_WORLD0 + 1 ].xyz, r4.xyz
mad r4.xyz, v2.z, c[ CV_WORLD0 + 2 ].xyz, r4.xyz

; 透視変換
mul r3, r5.x, c[ CV_EYEPERS + 0 ]
mad r3, r5.y, c[ CV_EYEPERS + 1 ], r3
mad r3, r5.z, c[ CV_EYEPERS + 2 ], r3
mad r3, r5.w, c[ CV_EYEPERS + 3 ], r3

dp3 r0.x, r4, r4					; 法線正規化[1]
mov oPos, r3						; 座標設定
mul r5.x, r3.w, c[ CV_FOG ].x		; Fog[1]
rsq r1.x, r0.x						; 法線正規化[2]
add oFog.x, c[ CV_FOG ].y, r5.x		; Fog[Last]
mul r4.xyz, r4.xyz, r1.x			; 法線正規化[Last]

; 環境マップ計算
;mul r7.xyz, r4.x, c[ CV_EYE_INV + 0 ].xyz				; 環境マッピング計算
;mad r7.xyz, r4.y, c[ CV_EYE_INV + 1 ].xyz, r7.xyz		; 環境マッピング計算
;mad r7.xyz, r4.z, c[ CV_EYE_INV + 2 ].xyz, r7.xyz		; 環境マッピング計算
;mul r7.xyz, r7.xyz, c[ CV_TEX0_SCALE ].xyz				; 
;add r7.xyz, r7.xyz, c[ CV_TEX0_OFFSET ].xyz				; 
;mov oT0.w, r3.w
;mul oT0.xy, r7.xy, r3.w
;mov oFog.x, c[ CV_ZERO ]
; 環境マップ計算（視線ベクトル参照）
mov r7.xyzw, c[ CV_EYE_INV + 3 ]
mad r7.xyzw, r2.x, c[ CV_EYE_INV + 0 ], r7
mad r7.xyzw, r2.y, c[ CV_EYE_INV + 1 ], r7
mad r7.xyzw, r2.z, c[ CV_EYE_INV + 2 ], r7
mul r8.xyz, r4.x, c[ CV_EYE_INV + 0 ].xyz
mad r8.xyz, r4.y, c[ CV_EYE_INV + 1 ].xyz, r8.xyz
mad r8.xyz, r4.z, c[ CV_EYE_INV + 2 ].xyz, r8.xyz
dp3 r0.x, r7, r7										; 視線ベクトル正規化
mov oD0.w, c[CV_HALF]									; [光源計算]
rsq r1.x, r0.x											; 視線ベクトル正規化
mul r7.xyz, r7.xyz, r1.x								; 視線ベクトル正規化
mov r6.xyzw, c[CV_LIGHTCOL + 3]							; [光源計算]
dp3 r8.w, r8.xyz, r7.xyz								; 反射ベクトル計算
mul oD0.xyz, r6.xyzw, c[CV_SCALE].y						; [光源計算]11/255にスケーリング
mad r7.xyz, -r8.xyz, r8.w, r7.xyz						; 反射ベクトル計算
mad r7.xyz, -r8.xyz, r8.w, r7.xyz						; 反射ベクトル計算
;mov r8.w, c[ CV_HALF ].w								; アルファ算出
;mad r8.w, r7.z, c[ CV_HALF ], r8.w						; アルファ算出
mul r7.xyz, r7.xyz, c[ CV_TEX0_SCALE ].xyz				; 
add r7.xyz, r7.xyz, c[ CV_TEX0_OFFSET ].xyz				; 
mov oT0.xy, r7
mov oT0.w, c[ CV_ONE ]
;mul oD0.w, V_DIFFUSE.w, r8.w								; アルファ出力


;; 光源計算(とりあえずMGS2に従って平行光源3つ)
;mul	r5.xyz, r4.x, c[ CV_LIGHTVEC + 0 ].xyz			; 
;mad	r5.xyz, r4.y, c[ CV_LIGHTVEC + 1 ].xyz, r5.xyz	; 
;mad	r5.xyz, r4.z, c[ CV_LIGHTVEC + 2 ].xyz, r5.xyz	; 
;max r5.xyz, r5.xyz, c[CV_ZERO]								; r5 >= 0 を保証

;mul r6.xyz, r5.x, c[CV_LIGHTCOL + 0]			; 光源強度1 x 光源色1
;mad r6.xyz, r5.y, c[CV_LIGHTCOL + 1], r6.xyz	; + 光源強度2 x 光源色2
;mad r6.xyz, r5.z, c[CV_LIGHTCOL + 2], r6.xyz	; + 光源強度3 x 光源色3
;add r6.xyz, c[CV_LIGHTCOL + 3], r6.xyz
;mul oD0.xyz, r6.xyz, c[CV_SCALE].y		; 1/255にスケーリング
;;mov oD0.w, V_DIFFUSE.w              		; アルファは入力のまま

; 環境マップの場合には光源計算はせずアンビエントのみ反映
;mov r6.xyzw, c[CV_LIGHTCOL + 3]
;mul oD0.xyz, r6.xyzw, c[CV_SCALE].y		; 1/255にスケーリング
;mov oD0.w, c[CV_HALF]

;mov oT0, V_TEX0								; テクスチャ座標0
;mov oT0.xy, r8.xy
;mov oT0.w, c[ CV_ONE ]

