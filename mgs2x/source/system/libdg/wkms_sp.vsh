; ＫＭＳ用(Windows版)（シングルテクスチャ＆プリシェードライティング）
;
; 2002/03/14  K.Takabe
; 2002/08/19  E.Takaki
; $Id: wkms_sp.vsh,v 1.6 2002/12/29 12:01:47 takaki Exp $
;
; 平行光源    ON
; ウェイト    ON
; 点光源      OFF
; 追加光源    OFF
;
; r0       : 頂点0
; r1       : 頂点1
; r2       : 法線0
; r3       : 法線1
; r4       : 1.0 - weight

#include "shader.h"

vs.1.1

; 頂点/法線・ワールド行列0
;dp4 r0.x, v0, c[CV_WORLD0 + 0]
;dp4 r0.y, v0, c[CV_WORLD0 + 1]
;dp4 r0.z, v0, c[CV_WORLD0 + 2]
;dp4 r0.w, v0, c[CV_WORLD0 + 3]
#ifndef DGOBJ_REVWT
mul r0, v0.x, c[ CV_WORLD0 + 0 ]
mad r0, v0.y, c[ CV_WORLD0 + 1 ], r0
mad r0, v0.z, c[ CV_WORLD0 + 2 ], r0
add r0, c[ CV_WORLD0 + 3 ], r0			; w座標の入力はありません
#else
mul r0, v0.x, c[ CV_WORLD1 + 0 ]
mad r0, v0.y, c[ CV_WORLD1 + 1 ], r0
mad r0, v0.z, c[ CV_WORLD1 + 2 ], r0
add r0, c[ CV_WORLD1 + 3 ], r0			; w座標の入力はありません
#endif

mov oD0, V_DIFFUSE							; アルファは入力のまま

; 透視変換
;dp4 oPos.x, r0, c[CV_EYEPERS + 0]
;dp4 oPos.y, r0, c[CV_EYEPERS + 1]
;dp4 oPos.z, r0, c[CV_EYEPERS + 2]
;dp4 oPos.w, r0, c[CV_EYEPERS + 3]
mul r1, r0.x, c[ CV_EYEPERS + 0 ]
mad r1, r0.y, c[ CV_EYEPERS + 1 ], r1
mad r1, r0.z, c[ CV_EYEPERS + 2 ], r1
mad r1, r0.w, c[ CV_EYEPERS + 3 ], r1

#ifndef UVADJUST
mov oT0, V_TEX0								; テクスチャ座標0
#else
mul r9, V_TEX0, c[ CV_TEX0_SCALE ]
add oT0.xy, r9, c[ CV_TEX0_OFFSET ]
#endif

mov oPos, r1								; 座標

mul r2.x, r1.w, c[ CV_FOG ].x
add oFog.x, c[ CV_FOG ].y, r2.x

