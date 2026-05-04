; prm2poly.vsh
; DG_PRIM2用シェーダー（ポリゴン＆ライン用）
;
; 2002/03/18  K.Takabe
; $Id: prm2poly.vsh,v 1.1.1.3 2002/11/19 11:42:21 Yoshizawa1 Exp $
;

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
mul r0, v0.x, c[ CV_SCREEN + 0 ]
mad r0, v0.y, c[ CV_SCREEN + 1 ], r0
mad r0, v0.z, c[ CV_SCREEN + 2 ], r0
mad r0, v0.w, c[ CV_SCREEN + 3 ], r0

; 透視変換
mul r1, r0.x, c[ CV_PERS + 0 ]
mad r1, r0.y, c[ CV_PERS + 1 ], r1
mad r1, r0.z, c[ CV_PERS + 2 ], r1
mad r1, r0.w, c[ CV_PERS + 3 ], r1
mov oPos, r1

mul r2.x, r1.w, c[ CV_FOG ].x
add oFog.x, c[ CV_FOG ].y, r2.x

mov oD0, V_DIFFUSE							; アルファは入力のまま

mul r3, V_TEX0, c[ CV_TEX0_SCALE ]				; テクスチャ座標0
add oT0, c[ CV_TEX0_OFFSET ], r3				; テクスチャ座標0
;mul oT0.xy, V_TEX0.xy, c[ CV_SCALE ].x			; テクスチャ座標0
;mov oT0.zw, V_TEX0.zw							; テクスチャ座標0
