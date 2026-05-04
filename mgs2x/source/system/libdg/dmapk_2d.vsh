; dmapk_2d.vsh
; DG_DMAPACK用シェーダー
;
; 2002/03/20  K.Takabe
; $Id: dmapk_2d.vsh,v 1.6 2002/11/23 11:36:53 Yoshizawa1 Exp $
;

#include "shader.h"

vs.1.1


; 一度同時座標にしているが、アセンブラ側で
; スクリーン座標に変換されるので結構無駄かも・・・

mul r0, V_POSITION, c[1]		; 同時座標空間に変換
mov oD0, V_DIFFUSE				; カラーはそのまま
add oPos, r0, c[0]				; 表示座標補正

mov oFog.x, c[ CV_ZERO ]

mul r3.xy, V_TEX0, c[ CV_TEX0_SCALE ]		; テクスチャ座標0
add oT0.xy, c[ CV_TEX0_OFFSET ], r3			; テクスチャ座標0
