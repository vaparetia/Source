; 曲面パッチ描画用頂点シェーダ
;
; 2001/06/04  F.Miyauchi
; 2002/09/08  E.Takaki(Windows)
; $Id: wpatch.vsh,v 1.2 2002/09/23 07:56:38 takaki Exp $
;
; 光源計算無し/透視変換のみ
;

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
mul r0, v0.x, c[ CV_WORLD0 + 0 ]
mad r0, v0.y, c[ CV_WORLD0 + 1 ], r0
mad r0, v0.z, c[ CV_WORLD0 + 2 ], r0
mad r0, v0.w, c[ CV_WORLD0 + 3 ], r0
mul r2.xyz, v2.x, c[ CV_WORLD0 + 0 ].xyz
mad r2.xyz, v2.y, c[ CV_WORLD0 + 1 ].xyz, r2.xyz
mad r2.xyz, v2.z, c[ CV_WORLD0 + 2 ].xyz, r2.xyz

; 透視変換
mul r1, r0.x, c[ CV_EYEPERS + 0 ]
mad r1, r0.y, c[ CV_EYEPERS + 1 ], r1
mad r1, r0.z, c[ CV_EYEPERS + 2 ], r1
mad r1, r0.w, c[ CV_EYEPERS + 3 ], r1

dp3 r3.x, r2, r2				; 法線正規化[1]
mov oPos, r1					; 座標設定
mul r2.x, r1.w, c[ CV_FOG ].x	; Fog[1]
rsq r3.x, r3.x					; 法線正規化[2]
add oFog.x, c[ CV_FOG ].y, r2.x	; Fog[Last]
mul r2.xyz, r2.xyz, r3.x		; 法線正規化[Last]

; 光源計算(とりあえずMGS2に従って平行光源3つ)
mul	R_TMP.xyz, R_NORMAL.x, c[ CV_LIGHTVEC + 0 ].xyz		; 
mad	R_TMP.xyz, R_NORMAL.y, c[ CV_LIGHTVEC + 1 ].xyz, R_TMP.xyz		; 
mad	R_TMP.xyz, R_NORMAL.z, c[ CV_LIGHTVEC + 2 ].xyz, R_TMP.xyz		; 
mov oD0.w, V_DIFFUSE.w              		; アルファは入力のまま
max R_TMP.xyz, R_TMP.xyz, c[CV_ZERO]					; R_TMP >= 0 を保証

mul R_COL.xyz, R_TMP.x, c[CV_LIGHTCOL + 0]	; 光源強度1 x 光源色1
mad R_COL.xyz, R_TMP.y, c[CV_LIGHTCOL + 1], R_COL.xyz	; + 光源強度2 x 光源色2
mad R_COL.xyz, R_TMP.z, c[CV_LIGHTCOL + 2], R_COL.xyz	; + 光源強度3 x 光源色3
add R_COL.xyz, c[CV_LIGHTCOL + 3], R_COL.xyz

#ifndef UVADJUST
mov oT0.xy, V_TEX0							; テクスチャ座標0
mul oD0.xyz, R_COL.xyz, c[CV_SCALE].y		; [光源計算]1/255にスケーリング
#else
mul r9, V_TEX0, c[ CV_TEX0_SCALE ]
mul oD0.xyz, R_COL.xyz, c[CV_SCALE].y		; [光源計算]1/255にスケーリング
add oT0.xy, r9, c[ CV_TEX0_OFFSET ]
#endif
