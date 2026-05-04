; レーダー用表示ルーチン
;
; 2002/05/19  K.Takabe
; $Id: vs_radar.vsh,v 1.3 2002/11/23 11:36:54 Yoshizawa1 Exp $
;


#define CV_ZERO				0
#define CV_ONE				1
#define CV_HALF				2
#define CV_BLOCK_OFFSET		3
#define CV_TRANS_MAT		4

#define CV_RADAR_COLOR0		16
#define CV_RADAR_COLOR1		17

vs.1.1

; 頂点生成
add		r2.xyz, v0, c[ CV_BLOCK_OFFSET ]	; 絶対座標生成

; 表示用エリアへの変換
mul		r3, r2.x, c[ CV_TRANS_MAT + 0 ]				; 
mad		r3, r2.y, c[ CV_TRANS_MAT + 1 ], r3			; 
mad		r3, r2.z, c[ CV_TRANS_MAT + 2 ], r3			; 
add		r3, c[ CV_TRANS_MAT + 3 ], r3				; r3.xy = レーダー投影座標, r3.z = 正規化投影高さ（下）
mad		r4.z, v0.w, c[ CV_TRANS_MAT + 1 ].z, r3.z	; r4.z = 正規化投影高さ（上）

; 高さのチェック
slt		r5.xyzw, r3.z, c[ CV_ONE ]						; if ( r3.z < 1.0  ) r5 = 1
slt		r6.xyzw, -c[ CV_ONE ], r4.z						; if ( -1.0 < r4.z ) r6 = 1
mul		r7.xyzw, r5, r6									; if ( r5 && r6 ) r7 = 1
sub		r8, c[ CV_ONE ], r7

mov		oPos.xy, r3.xy
mov		oPos.w, c[ CV_ONE ]
mov		oPos.z, r7

mul		r9, r7, c[ CV_RADAR_COLOR0 ]
mad		oD0.xyz, r8, c[ CV_RADAR_COLOR1 ], r9
mov		oD0.w, c[ CV_ONE ]
