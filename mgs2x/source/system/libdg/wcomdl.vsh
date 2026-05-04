; COMDL描画用頂点シェーダ
;
; 2001/05/24  F.Miyauchi
; $Id: wcomdl.vsh,v 1.2 2002/09/23 07:56:36 takaki Exp $
;

#include "shader.h"

vs.1.1

; 座標変換
mul r0, v0.x, c[ CV_WORLD0 + 0 ]
mad r0, v0.y, c[ CV_WORLD0 + 1 ], r0
mad r0, v0.z, c[ CV_WORLD0 + 2 ], r0
mad r0, v0.w, c[ CV_WORLD0 + 3 ], r0

mov r3, c[ CV_LIGHTCOL + 3 ]	; 光源計算[1]

; 透視変換
mul r1, r0.x, c[ CV_EYEPERS + 0 ]
mad r1, r0.y, c[ CV_EYEPERS + 1 ], r1
mad r1, r0.z, c[ CV_EYEPERS + 2 ], r1
mad r1, r0.w, c[ CV_EYEPERS + 3 ], r1

mul oD0, r3, c[ CV_SCALE ].y 	; 光源計算[Last]

mov oPos, r1	; 座標設定

#ifndef UVADJUST

mul r2.x, r1.w, c[ CV_FOG ].x	; Fog[1]
mov oT0, V_TEX0					; テクスチャ座標0
add oFog.x, c[ CV_FOG ].y, r2.x	; Fog[Last]

#else

mul r2.x, r1.w, c[ CV_FOG ].x	; Fog[1]
mul r9,  	V_TEX0, c[ CV_TEX0_SCALE ]	; TexCoord0[0] 
add oFog.x, c[ CV_FOG ].y, r2.x	; Fog[Last]
add oT0.xy,  r9, c[ CV_TEX0_OFFSET ]	; TexCoord0[Last] 
#endif
