; 影投影用シェーダー(Windows FVF頂点 1Texture版)
;
; 2002/04/03  K.Takabe
; 2002/08/21  E.Takaki
; $Id: wshdw_wt_fvf_1tex.vsh,v 1.1 2002/10/22 05:32:22 takaki Exp $
;

#include "shader.h"

vs.1.1

add r1.x,    c[CV_ONE].x,     -V_WEIGHT_WIN.w	; r1 = 1.0f - weight

; 頂点/法線・ワールド行列0
mul r2, v0.x, c[ CV_WORLD0 + 0 ]
mad r2, v0.y, c[ CV_WORLD0 + 1 ], r2
mad r2, v0.z, c[ CV_WORLD0 + 2 ], r2
add r2, c[ CV_WORLD0 + 3 ], r2			; w値入力無し
mul r3, v0.x, c[ CV_WORLD1 + 0 ]
mad r3, v0.y, c[ CV_WORLD1 + 1 ], r3
mad r3, v0.z, c[ CV_WORLD1 + 2 ], r3
add r3, c[ CV_WORLD1 + 3 ], r3			; w値入力無し
mul r4.xyz, v2.x, c[ CV_WORLD0 + 0 ].xyz
mad r4.xyz, v2.y, c[ CV_WORLD0 + 1 ].xyz, r4.xyz
mad r4.xyz, v2.z, c[ CV_WORLD0 + 2 ].xyz, r4.xyz
mul r5.xyz, v2.x, c[ CV_WORLD1 + 0 ].xyz
mad r5.xyz, v2.y, c[ CV_WORLD1 + 1 ].xyz, r5.xyz
mad r5.xyz, v2.z, c[ CV_WORLD1 + 2 ].xyz, r5.xyz

mul r2.xyz, r2.xyz, V_WEIGHT_WIN.w	; r2 *= weight
mul r4.xyz, r4.xyz, V_WEIGHT_WIN.w	; r4 *= weight
mad r2.xyz, r3.xyz, r1.x, r2.xyz	; r2 += r3 * (1.0f - weight)
mad r4.xyz, r5.xyz, r1.x, r4.xyz	; r4 += r5 * (1.0f - weight)

; テクスチャ座標の算出
mul r3, r2.x, c[ CV_LIGHTVEC + 0 ]		; テクスチャの投影変換
mad r3, r2.y, c[ CV_LIGHTVEC + 1 ], r3	; テクスチャの投影変換
mad r3, r2.z, c[ CV_LIGHTVEC + 2 ], r3	; テクスチャの投影変換
mad r3, r2.w, c[ CV_LIGHTVEC + 3 ], r3	; テクスチャの投影変換

mul r3.x, r3.x, c[ CV_SCALE ].w				; テクスチャ座標補正（スケール）
mul r3.y, r3.y, -c[ CV_SCALE ].w			; テクスチャ座標補正（スケール）XBOXではＹが逆になるため

dp3 r4.w, r4, r4							; 法線正規化[1]
mad oT0.xy, c[ CV_CONST0 ].xy, r3.w, r3.xy	; テクスチャ座標補正（オフセット）
;mad oT1.xy, c[ CV_CONST1 ].xy, r3.w, r3.xy	; テクスチャ座標補正（オフセット）
;mad oT2.xy, c[ CV_CONST2 ].xy, r3.w, r3.xy	; テクスチャ座標補正（オフセット）
rsq r1.x, r4.w								; 法線正規化[2]
mov oT0.w, r3.w								; テクスチャ座標設定（Ｗ）
mul r4, r4, r1.x							; 法線正規化[Last]
;mov oT1.w, r3.w								; テクスチャ座標設定（Ｗ）
;mov oT2.w, r3.w								; テクスチャ座標設定（Ｗ）
;mov oT3.w, r3.w								; テクスチャ座標設定（クリップ用）

; 光源ベクトルを求める
;add r6.xyz, r2, -c[ CV_LIGHTCOL + 3 ]	; 
;dp3 r6.w, r6, r6
;rsq r1.x, r6.w
;mul r6, r6, r1.x

; 減衰パラメータ計算
rcp r1.x, c[ CV_LIGHTCOL + 1 ].w		; 減衰距離の逆数計算
dp3 r5.x, r4, c[ CV_LIGHTCOL + 2 ]		; r5.x = 光源ベクトルと法線の内積
;dp3 r5.x, r4, r6						; r5.x = 光源ベクトルと法線の内積（厳密計算Ｖｅｒ．）
add r5.w, c[ CV_LIGHTCOL + 1 ].w, -r3.w	; r5.w = 最大減衰距離-光源からの距離
max r5.x, r5.x, c[ CV_ZERO ]			; r5.x = 法線による光源影響度
mul r5.w, r5.w, r1.x					; r5.w = 距離による減衰量計算

; 透視変換
mul r3, r2.x, c[ CV_EYEPERS + 0 ]
mad r3, r2.y, c[ CV_EYEPERS + 1 ], r3
mad r3, r2.z, c[ CV_EYEPERS + 2 ], r3
mad r3, r2.w, c[ CV_EYEPERS + 3 ], r3
mov oPos, r3

mul r0.x, r3.w, c[ CV_FOG ].x
add oFog.x, c[ CV_FOG ].y, r0.x


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
;mov oD0.w, c[ CV_HALF ].x
mul r5.w, r5.x, r5.w
;mov r5.xyz, c[ CV_SCALE ].y							; こちらが正しい
mul r5.xyz, c[ CV_SCALE ].y, r5.w						; しかしＰＳ２がこうなってしまっていた
mul r5.w, r5.w, c[ CV_SCALE ].y
mul oD0.xyz, c[CV_LIGHTCOL + 0].xyz, r5.xyz
mul oD0.w, c[CV_LIGHTCOL + 0].w, r5.w

