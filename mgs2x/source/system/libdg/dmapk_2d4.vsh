; dmapk_2d4.vsh
; DG_DMAPACK用シェーダー（４テクスチャ対応）
;
; 2002/05/16  K.Takabe
; $Id: dmapk_2d4.vsh,v 1.3 2002/11/23 11:36:53 Yoshizawa1 Exp $
;

#include "shader.h"

vs.1.1


; 一度同時座標にしているが、アセンブラ側で
; スクリーン座標に変換されるので結構無駄かも・・・
add r0.xy, V_POSITION, c[0].xy		; 表示座標補正
mul r0.xy, r0.xy, c[1].xy			; 同時座標空間に変換
add oPos.xy, -c[ CV_ONE ].xy, r0.xy	; 同時座標空間に変換
mov oPos.z, c[0].z
mov oPos.w, c[ CV_ONE ]

;#pragma screenspace
;add r0.xy, V_POSITION, c[0].xy		; 表示座標補正
;mov oPos.zw, c[ CV_ONE ]
;mul oPos.z, r12.z, c[ -38 ]			; スケール
;add oPos.z, r12.z, c[ -37 ]			; オフセット

mov oFog.x, c[ CV_ZERO ]

mov oD0, V_DIFFUSE							; カラーはそのまま

mov oT0.xy, V_TEX0
mov oT1.xy, V_TEX1
mov oT2.xy, V_TEX2
mov oT3.xy, V_TEX3
