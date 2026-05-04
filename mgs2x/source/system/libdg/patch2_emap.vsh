; 曲面パッチ描画用頂点シェーダ（環境マップ版）強制平面描画版 [The vertex shader for drawing surface patches (environment map version) version of the plane drawing force]
;
; 2002/05/10  K.Takabe
; $Id: patch2_emap.vsh,v 1.1.1.3 2002/11/19 11:42:18 Yoshizawa1 Exp $
;

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0 [Vertex / normal world matrix 0]
// r2 = world pos
mul r2, v0.x, c[ CV_WORLD0 + 0 ]
mad r2, v0.y, c[ CV_WORLD0 + 1 ], r2
mad r2, v0.z, c[ CV_WORLD0 + 2 ], r2
mad r2, v0.w, c[ CV_WORLD0 + 3 ], r2

// r5 = world pos (ignoring y part of transform)
mul r5, v0.x, c[ CV_WORLD0 + 0 ]						; 実際に描画する座標（Ｙ＝０）[Actual drawing coordinate (Y = 0)]
mad r5, v0.z, c[ CV_WORLD0 + 2 ], r5					; 
mad r5, v0.w, c[ CV_WORLD0 + 3 ], r5					; 

// r4 = normal
mul r4.xyz, v2.x, c[ CV_WORLD0 + 0 ].xyz
mad r4.xyz, v2.y, c[ CV_WORLD0 + 1 ].xyz, r4.xyz
mad r4.xyz, v2.z, c[ CV_WORLD0 + 2 ].xyz, r4.xyz

; 透視変換 [Perspective transformation]
// r3 = eyePers * worldPos(ignoring y part)
mul r3, r5.x, c[ CV_EYEPERS + 0 ]
mad r3, r5.y, c[ CV_EYEPERS + 1 ], r3
mad r3, r5.z, c[ CV_EYEPERS + 2 ], r3
mad r3, r5.w, c[ CV_EYEPERS + 3 ], r3
mov oPos, r3

// Calculate fog based on flat position
mul r5.x, r3.w, c[ CV_FOG ].x
add oFog.x, c[ CV_FOG ].y, r5.x

; 法線正規化 [Normalized normal]
// r4 = normalize(worldNormal)
dp3 r0.x, r4, r4
rsq r1.x, r0.x
mul r4.xyz, r4.xyz, r1.x

; 環境マップ計算（視線ベクトル参照）[Computed environment map (see eye vector)]
// r7 = eye space of real world pos
mov r7.xyzw, c[ CV_EYE_INV + 3 ]
mad r7.xyzw, r2.x, c[ CV_EYE_INV + 0 ], r7
mad r7.xyzw, r2.y, c[ CV_EYE_INV + 1 ], r7
mad r7.xyzw, r2.z, c[ CV_EYE_INV + 2 ], r7

// r8 = eye space of normalized normal
mul r8.xyz, r4.x, c[ CV_EYE_INV + 0 ].xyz
mad r8.xyz, r4.y, c[ CV_EYE_INV + 1 ].xyz, r8.xyz
mad r8.xyz, r4.z, c[ CV_EYE_INV + 2 ].xyz, r8.xyz

// r7 = normalize(eyeSpaceWorldPos)
dp3 r0.x, r7, r7										; 視線ベクトル正規化
rsq r1.x, r0.x											; 視線ベクトル正規化
mul r7.xyz, r7.xyz, r1.x								; 視線ベクトル正規化

dp3 r8.w, r8.xyz, r7.xyz								; 反射ベクトル計算
mad r7.xyz, -r8.xyz, r8.w, r7.xyz						; 反射ベクトル計算
mad r7.xyz, -r8.xyz, r8.w, r7.xyz						; 反射ベクトル計算
mul r7.xyz, r7.xyz, c[ CV_TEX0_SCALE ].xyz				; 
add r7.xyz, r7.xyz, c[ CV_TEX0_OFFSET ].xyz				; 
mov oT0.xy, r7
mov oT0.w, c[ CV_ONE ]


; 環境マップの場合には光源計算はせずアンビエントのみ反映 [If the environment map is calculated only reflected ambient light source is not]
mov r6.xyzw, c[CV_LIGHTCOL + 3]
mul oD0.xyz, r6.xyzw, c[CV_SCALE].y		; 1/255にスケーリング
mov oD0.w, c[CV_HALF]

