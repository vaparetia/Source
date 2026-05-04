; COMDL描画用頂点シェーダ
;
; 2001/05/24  F.Miyauchi
; $Id: comdl.vsh,v 1.4 2002/11/23 11:36:52 Yoshizawa1 Exp $
;

#include "shader.h"

vs.1.1

; 座標変換
mul r0, v0.x, c[ CV_WORLD0 + 0 ]
mad r0, v0.y, c[ CV_WORLD0 + 1 ], r0
mad r0, v0.z, c[ CV_WORLD0 + 2 ], r0
mad r0, v0.w, c[ CV_WORLD0 + 3 ], r0
; 透視変換
mul r1, r0.x, c[ CV_EYEPERS + 0 ]
mad r1, r0.y, c[ CV_EYEPERS + 1 ], r1
mad r1, r0.z, c[ CV_EYEPERS + 2 ], r1
mad r1, r0.w, c[ CV_EYEPERS + 3 ], r1
mov oPos, r1

mov r3, c[ CV_LIGHTCOL + 3 ]
mul oD0, r3, c[ CV_SCALE ].y ;

; フォグ
mul r2.x, r1.w, c[ CV_FOG ].x
add oFog.x, c[ CV_FOG ].y, r2.x

;mov oD0, c[ CV_LIGHTCOL + 3 ]			; カラー出力
#ifndef UVADJUST
mov oT0, V_TEX0					; テクスチャ座標0
#else
mul r9,  V_TEX0, c[ CV_TEX0_SCALE ]
add oT0,  r9, c[ CV_TEX0_OFFSET ]
#endif
