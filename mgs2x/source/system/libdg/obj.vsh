; OBJ描画用頂点シェーダ
;
; 2001/05/14  F.Miyauchi
; $Id: obj.vsh,v 1.1.1.3 2002/11/19 11:42:14 Yoshizawa1 Exp $
;
; 平行光源    OFF
; ウェイト    OFF
; 点光源      OFF
; 追加光源    OFF
;

#include "shader.h"

vs.1.1

; 頂点変換
dp4 R_POSITION.x, V_POSITION, c[CV_WORLD0 + 0]
dp4 R_POSITION.y, V_POSITION, c[CV_WORLD0 + 1]
dp4 R_POSITION.z, V_POSITION, c[CV_WORLD0 + 2]
dp4 R_POSITION.w, V_POSITION, c[CV_WORLD0 + 3]

; 透視変換
dp4 oPos.x, R_POSITION, c[CV_EYEPERS + 0]
dp4 oPos.y, R_POSITION, c[CV_EYEPERS + 1]
dp4 oPos.z, R_POSITION, c[CV_EYEPERS + 2]
dp4 oPos.w, R_POSITION, c[CV_EYEPERS + 3]

mov oD0, V_DIFFUSE				; カラー出力
mov oT0, V_TEX0					; テクスチャ座標0
mov oT1, V_TEX1					; テクスチャ座標1
mov oT2, V_TEX2					; テクスチャ座標2

mov oFog.x, c[CV_ZERO]			; フォグ(0.0が手前～1.0が奥)
