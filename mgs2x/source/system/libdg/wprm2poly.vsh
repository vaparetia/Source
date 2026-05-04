; wprm2poly.vsh
; DG_PRIM2用シェーダー（ポリゴン＆ライン用）
;
; 2002/03/18  K.Takabe
; $Id: wprm2poly.vsh,v 1.2 2002/11/12 04:42:32 takaki Exp $
;

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
mul r0, v0.x, c[ CV_SCREEN + 0 ]
mad r0, v0.y, c[ CV_SCREEN + 1 ], r0
mad r0, v0.z, c[ CV_SCREEN + 2 ], r0
mad r0, v0.w, c[ CV_SCREEN + 3 ], r0

mov oD0, V_DIFFUSE							; アルファは入力のまま

; 透視変換
mul r1, r0.x, c[ CV_PERS + 0 ]
mad r1, r0.y, c[ CV_PERS + 1 ], r1
mad r1, r0.z, c[ CV_PERS + 2 ], r1
mad r1, r0.w, c[ CV_PERS + 3 ], r1

mul r3, V_TEX0, c[ CV_TEX0_SCALE ]	; テクスチャ座標0[1]
mov oPos, r1						; 座標設定
mul r2.x, r1.w, c[ CV_FOG ].x		; Fog[1]
add oT0.xy, c[ CV_TEX0_OFFSET ], r3	; テクスチャ座標0[Last]
add oFog.x, c[ CV_FOG ].y, r2.x		; Fog[Last]
