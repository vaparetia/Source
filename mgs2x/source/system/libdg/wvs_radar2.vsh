; レーダー用表示ルーチン（プリミティブ表示用）
;
; 2002/05/21  K.Takabe
; $Id: wvs_radar2.vsh,v 1.1 2002/09/23 07:56:01 takaki Exp $
;


#define CV_ZERO				0
#define CV_ONE				1
#define CV_HALF				2
#define CV_BLOCK_OFFSET		3
#define CV_TRANS_MAT		4

#define CV_RADAR_COLOR0		16
#define CV_RADAR_COLOR1		17

vs.1.1

; 表示用エリアへの変換
mul		r3, v0.x, c[ CV_TRANS_MAT + 0 ]				; 
mad		r3, v0.y, c[ CV_TRANS_MAT + 1 ], r3			; 
mad		r3, v0.z, c[ CV_TRANS_MAT + 2 ], r3			; 
add		r3, c[ CV_TRANS_MAT + 3 ], r3				; r3.xy = レーダー投影座標, r3.z = 正規化投影高さ（下）

mov		oPos.w, c[ CV_ONE ]
mov		oPos.xy, r3.xy
mov		oPos.z, v0.y

mov		oD0, v3
