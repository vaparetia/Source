; evm_emap.vsh
; EVM描画用頂点シェーダ（環境マップ付き）
;
;
; // 定数
; c0 - c3  : world[0]
; c4 - c7  : world[1]
; c8 -c11  : world[2]
; c12-c15  : world[3]
; c16-c19  : world[4]
; c20-c23  : world[5]
; c24-c27  : world[6]
; c28-c31  : world[7]
; c32-c35  : eye_pers
; c36-c39  : light vector
; c40-c43  : light color
;
; // 入力
; v0       : position
; v2       : normal
; v3       : color
; v11      : blend
; v12      : index
;
; // レジスタ
; r2       : 頂点
; r3       : ワーク
; r4       : 法線
; r5       : ワーク

#include "shader.h"

vs.1.1

mul r0.xyz, v0.xyz, c[ CV_SCALE ].z
mov r0.w, v0.w

; 頂点/法線変換
mov a0.x, v12.x                          ; 行列インデックス
mul r2, r0.x, c[a0.x + 0]    ; インデックスに対応した行列で頂点変換
mad r2, r0.y, c[a0.x + 1], r2
mad r2, r0.z, c[a0.x + 2], r2
mad r2, r0.w, c[a0.x + 3], r2
mul r4.xyz, v2.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r4.xyz, v2.y, c[a0.x + 1].xyz, r4
mad r4.xyz, v2.z, c[a0.x + 2].xyz, r4
mul r2.xyz, r2.xyz, v11.x            ; ウェイト値乗算
mul r4.xyz, r4.xyz, v11.x            ; ウェイト値乗算

mov a0.x, v12.y
mul r3, r0.x, c[a0.x + 0]
mad r3, r0.y, c[a0.x + 1], r3
mad r3, r0.z, c[a0.x + 2], r3
mad r3, r0.w, c[a0.x + 3], r3
mul r5.xyz, v2.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5
mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5
mad r2.xyz, r3.xyz, v11.y, r2.xyz
mad r4.xyz, r5.xyz, v11.y, r4.xyz

mov a0.x, v12.z
mul r3, r0.x, c[a0.x + 0]
mad r3, r0.y, c[a0.x + 1], r3
mad r3, r0.z, c[a0.x + 2], r3
mad r3, r0.w, c[a0.x + 3], r3
mul r5.xyz, v2.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5
mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5
mad r2.xyz, r3.xyz, v11.z, r2.xyz
mad r4.xyz, r5.xyz, v11.z, r4.xyz

mov a0.x, v12.w
mul r3, r0.x, c[a0.x + 0]
mad r3, r0.y, c[a0.x + 1], r3
mad r3, r0.z, c[a0.x + 2], r3
mad r3, r0.w, c[a0.x + 3], r3
mul r5.xyz, v2.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5
mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5
mad r2.xyz, r3.xyz, v11.w, r2.xyz
mad r4.xyz, r5.xyz, v11.w, r4.xyz

mul r2.xyz, r2.xyz, c[ CV_SCALE ].w
mul r4.xyz, r4.xyz, c[ CV_SCALE ].w

; 透視変換
mul r3, r2.x, c[CV_EYEPERS + 0]
mad r3, r2.y, c[CV_EYEPERS + 1], r3
mad r3, r2.z, c[CV_EYEPERS + 2], r3
mad r3, r2.w, c[CV_EYEPERS + 3], r3

dp3 r5.x, r4, r4					; 法線正規化[1]
mov oPos, r3						; 座標設定
rsq r5.x, r5.x						; 法線正規化[2]
mul r6.x, r3.w, c[ CV_FOG ].x		; フォグ計算[1]
mul r4.xyz, r4.xyz, r5.x			; 法線正規化[Last]
add oFog.x, c[ CV_FOG ].y, r6.x		; フォグ計算[Last]

; 光源計算１
mul	r5.xyz, r4.x, c[ CV_LIGHTVEC + 0 ].xyz		; 
mad	r5.xyz, r4.y, c[ CV_LIGHTVEC + 1 ].xyz, r5.xyz		; 
mad	r5.xyz, r4.z, c[ CV_LIGHTVEC + 2 ].xyz, r5.xyz		; 
mov oD0.w, c[ CV_HALF ].x			; A成分はＭＧＳ２に合わせて１００％のみ
max r5.xyz, r5.xyz, c[CV_ZERO]		; R_TMP >= 0 を保証
; 光源計算２
mul r6.xyz, r5.x, c[CV_LIGHTCOL + 0]			; 光源強度1 x 光源色1
mad r6.xyz, r5.y, c[CV_LIGHTCOL + 1], r6.xyz	; + 光源強度2 x 光源色2
mad r6.xyz, r5.z, c[CV_LIGHTCOL + 2], r6.xyz	; + 光源強度3 x 光源色3
add r6.xyz, c[CV_LIGHTCOL + 3], r6.xyz

mul	r7.xyz, r4.x, c[ CV_EYE_INV + 0 ].xyz				; 環境マッピング計算
mad r7.xyz, r4.y, c[ CV_EYE_INV + 1 ].xyz, r7.xyz		; 環境マッピング計算
mad r7.xyz, r4.z, c[ CV_EYE_INV + 2 ].xyz, r7.xyz		; 環境マッピング計算
mul oD0.xyz, r6.xyz, c[CV_SCALE].y						; [光源計算]1/255にスケーリング
mul r7.xyz, r7.xyz, c[ CV_TEX2_SCALE ].xyz				; 
add r7.xyz, r7.xyz, c[ CV_TEX2_OFFSET ].xyz				; 
mov oT2.w, r3.w
mul oT2.xy, r7.xy, r3.w

; 出力
#ifndef UVADJUST
mov oT0, v7                           ; テクスチャ座標
mov oT1, v8                           ; テクスチャ座標
//mov oT2, v9                           ; テクスチャ座標
#else
mul r9,  v7, c[ CV_TEX0_SCALE ]
mul r10, v8, c[ CV_TEX1_SCALE ]
//mul r11, v9, c[ CV_TEX2_SCALE ]
add oT0.xy,  r9, c[ CV_TEX0_OFFSET ]
add oT1.xy, r10, c[ CV_TEX1_OFFSET ]
//add oT2, r11, c[ CV_TEX2_OFFSET ]
#endif
