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
mul r8.xyz, v9.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r8.xyz, v9.y, c[a0.x + 1].xyz, r8
mad r8.xyz, v9.z, c[a0.x + 2].xyz, r8
mul r2.xyz, r2.xyz, v11.x            ; ウェイト値乗算
mul r4.xyz, r4.xyz, v11.x            ; ウェイト値乗算
mul r8.xyz, r8.xyz, v11.x            ; ウェイト値乗算

mov a0.x, v12.y
mul r3, r0.x, c[a0.x + 0]
mad r3, r0.y, c[a0.x + 1], r3
mad r3, r0.z, c[a0.x + 2], r3
mad r3, r0.w, c[a0.x + 3], r3
mul r5.xyz, v2.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5
mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5
mul r9.xyz, v9.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9
mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9
mad r2.xyz, r3.xyz, v11.y, r2.xyz
mad r4.xyz, r5.xyz, v11.y, r4.xyz
mad r8.xyz, r9.xyz, v11.y, r8.xyz

mov a0.x, v12.z
mul r3, r0.x, c[a0.x + 0]
mad r3, r0.y, c[a0.x + 1], r3
mad r3, r0.z, c[a0.x + 2], r3
mad r3, r0.w, c[a0.x + 3], r3
mul r5.xyz, v2.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5
mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5
mul r9.xyz, v9.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9
mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9
mad r2.xyz, r3.xyz, v11.z, r2.xyz
mad r4.xyz, r5.xyz, v11.z, r4.xyz
mad r8.xyz, r9.xyz, v11.z, r8.xyz

mov a0.x, v12.w
mul r3, r0.x, c[a0.x + 0]
mad r3, r0.y, c[a0.x + 1], r3
mad r3, r0.z, c[a0.x + 2], r3
mad r3, r0.w, c[a0.x + 3], r3
mul r5.xyz, v2.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5
mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5
mul r9.xyz, v9.x, c[a0.x + 0].xyz    ; インデックスに対応した行列で頂点変換
mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9
mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9
mad r2.xyz, r3.xyz, v11.w, r2.xyz
mad r4.xyz, r5.xyz, v11.w, r4.xyz
mad r8.xyz, r9.xyz, v11.w, r8.xyz

// r2 = world pos
// r4 = world normal
// r8 = world bump vec
mul r2.xyz, r2.xyz, c[ CV_SCALE ].w
mul r4.xyz, r4.xyz, c[ CV_SCALE ].w
mul r8.xyz, r8.xyz, c[ CV_SCALE ].w

; 透視変換
mul r3, r2.x, c[CV_EYEPERS + 0]
mad r3, r2.y, c[CV_EYEPERS + 1], r3
mad r3, r2.z, c[CV_EYEPERS + 2], r3
mad r3, r2.w, c[CV_EYEPERS + 3], r3

mov oPos, r3

; フォグ計算
mul r6.x, r3.w, c[ CV_FOG ].x
add oFog.x, c[ CV_FOG ].y, r6.x
;mov oFog.x, c[ CV_ZERO ]

// r4 = normalize(worldNormal)
; 法線正規化
dp3 r4.w, r4, r4
rsq r1.x, r4.w
mul r4.xyz, r4.xyz, r1.x

// r5 = light amounts
; 光源計算１
mul	r5.xyz, r4.x, c[ CV_LIGHTVEC + 0 ].xyz		; 
mad	r5.xyz, r4.y, c[ CV_LIGHTVEC + 1 ].xyz, r5.xyz		; 
mad	r5.xyz, r4.z, c[ CV_LIGHTVEC + 2 ].xyz, r5.xyz		; 
max r5.xyz, r5.xyz, c[CV_ZERO]					; R_TMP >= 0 を保証

// r6 = light color
; 光源計算２
mul r6.xyz, r5.x, c[CV_LIGHTCOL + 0]			; 光源強度1 x 光源色1
mad r6.xyz, r5.y, c[CV_LIGHTCOL + 1], r6.xyz	; + 光源強度2 x 光源色2
mad r6.xyz, r5.z, c[CV_LIGHTCOL + 2], r6.xyz	; + 光源強度3 x 光源色3
add r6.xyz, c[CV_LIGHTCOL + 3], r6.xyz
mul oD0.xyz, r6.xyz, c[CV_SCALE].y		; 1/255にスケーリング
mov oD0.w, c[ CV_HALF ].x								; A成分はＭＧＳ２に合わせて１００％のみ

; Ｓベクトル正規化
// r8 = normalize(bumpVec)
dp3 r8.w, r8, r8
rsq r1.x, r8.w
mul r8.xyz, r8.xyz, r1.x

// r4 = normalize(world normal)
// r8 = normalize(world bump vec S)
// r7 = world bump vec T
mul r7.xyz, r4.yzx, r8.zxy								; 法線×Ｓベクトル(r7=r4*r8)
mad r7.xyz, -r4.zxy, r8.yzx, r7							; 法線×Ｓベクトル(r7=r4*r8)

; バンプマップ処理
; r8 = Ｓベクトル, r7 = Ｔベクトル, r4 = 法線


dp3 r8.x, r8.xyz, c[ CV_CONST0 ].xyz					; 光源内積Ｕ
dp3 r8.y, r7.xyz, c[ CV_CONST0 ].xyz					; 光源内積Ｖ


mad r8.xy, r8.xy, c[ CV_TEX1_SCALE ].zw, v8				; エンボステクスチャＵＶ生成
min r8.xy, r8.xy, c[ CV_ONE ]
max r8.xy, r8.xy, c[ CV_ZERO ]

; 出力
#ifndef UVADJUST
mov oT0, v7                           ; テクスチャ座標
mov oT1, v8                           ; テクスチャ座標
#else
mul r9,  v7, c[ CV_TEX0_SCALE ]

mul r10.xy, v8, c[ CV_TEX1_SCALE ]
mul r11.xy, r8.xy, c[ CV_TEX1_SCALE ]

add oT0,  r9, c[ CV_TEX0_OFFSET ]

add oT1.xy, r10.xy, c[ CV_TEX1_OFFSET ]
add oT2.xy, r11.xy, c[ CV_TEX1_OFFSET ]

mov oT1.w, c[ CV_ONE ]
mov oT2.w, c[ CV_ONE ]
#endif
