; vertex shader for evm model
;
; 2001/05/11  F.Miyauchi
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
; r0       : 頂点
; r1       : ワーク
; r2       : 法線
; r3       : ワーク

#include "shader.h"

vs.1.1

; 頂点/法線変換
mov a0.x, v12.x                          ; 行列インデックス
dp4 r0.x, v0, c[a0.x + CV_WORLD0 + 0]    ; インデックスに対応した行列で頂点変換
dp4 r0.y, v0, c[a0.x + CV_WORLD0 + 1]
dp4 r0.z, v0, c[a0.x + CV_WORLD0 + 2]
dp4 r0.w, v0, c[a0.x + CV_WORLD0 + 3]
dp3 r2.x, v2, c[a0.x + CV_WORLD0 + 0]    ; インデックスに対応した行列で法線変換
dp3 r2.y, v2, c[a0.x + CV_WORLD0 + 1]
dp3 r2.z, v2, c[a0.x + CV_WORLD0 + 2]
mul r0, r0, v11.x            ; ウェイト値乗算
mul r2.xyz, r2.xyz, v11.x

mov a0.x, v12.y
dp4 r1.x, v0, c[a0.x + CV_WORLD0 + 0]
dp4 r1.y, v0, c[a0.x + CV_WORLD0 + 1]
dp4 r1.z, v0, c[a0.x + CV_WORLD0 + 2]
dp4 r1.w, v0, c[a0.x + CV_WORLD0 + 3]
dp3 r3.x, v2, c[a0.x + CV_WORLD0 + 0]
dp3 r3.y, v2, c[a0.x + CV_WORLD0 + 1]
dp3 r3.z, v2, c[a0.x + CV_WORLD0 + 2]
mad r0, r1, v11.y, r0
mad r2.xyz, r3.xyz, v11.y, r2.xyz

mov a0.x, v12.z
dp4 r1.x, v0, c[a0.x + CV_WORLD0 + 0]
dp4 r1.y, v0, c[a0.x + CV_WORLD0 + 1]
dp4 r1.z, v0, c[a0.x + CV_WORLD0 + 2]
dp4 r1.w, v0, c[a0.x + CV_WORLD0 + 3]
dp3 r3.x, v2, c[a0.x + CV_WORLD0 + 0]
dp3 r3.y, v2, c[a0.x + CV_WORLD0 + 1]
dp3 r3.z, v2, c[a0.x + CV_WORLD0 + 2]
mad r0, r1, v11.z, r0
mad r2.xyz, r3.xyz, v11.z, r2.xyz

mov a0.x, v12.w
dp4 r1.x, v0, c[a0.x + CV_WORLD0 + 0]
dp4 r1.y, v0, c[a0.x + CV_WORLD0 + 1]
dp4 r1.z, v0, c[a0.x + CV_WORLD0 + 2]
dp4 r1.w, v0, c[a0.x + CV_WORLD0 + 3]
dp3 r3.x, v2, c[a0.x + CV_WORLD0 + 0]
dp3 r3.y, v2, c[a0.x + CV_WORLD0 + 1]
dp3 r3.z, v2, c[a0.x + CV_WORLD0 + 2]
mad r0, r1, v11.w, r0
mad r2.xyz, r3.xyz, v11.w, r2.xyz

; 透視変換
dp4 oPos.x, r0, c[CV_EYEPERS + 0]
dp4 oPos.y, r0, c[CV_EYEPERS + 1]
dp4 oPos.z, r0, c[CV_EYEPERS + 2]
dp4 oPos.w, r0, c[CV_EYEPERS + 3]

; 法線正規化
dp3 r2.w, r2, r2
rsq r2.w, r2.w
mul r2, r2, r2.w

; 光源計算ルーチン
#include "light.vsh"

mov oD0.xyz, r3.xyz                   ; カラー出力
mov oD0.w, v3.w                       ; アルファは入力カラーのまま
mov oT0, v7                           ; テクスチャ座標
mov oT1, v8                           ; テクスチャ座標
mov oT2, v9                           ; テクスチャ座標
