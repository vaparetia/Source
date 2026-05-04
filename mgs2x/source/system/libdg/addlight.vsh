; 追加平行3光源の計算ルーチン
; 
; 2001/05/14 F.Miyauchi
; $Id: addlight.vsh,v 1.1.1.3 2002/11/19 11:42:01 Yoshizawa1 Exp $
;
; R_NORMAL は正規化されていることが前提
;
; 出力: R_ADDCOL
; 破壊: R_TMP

; 追加光源計算(3方向まで認める)
dp3 R_TMP.x, R_NORMAL, c[CV_ADDLIGHTVEC + 0]		; 法線・ローカルライト0
mul R_TMP.x, R_TMP.x, c[CV_ADDLIGHTVEC + 0].w		; x 光源強度0
dp3 R_TMP.y, R_NORMAL, c[CV_ADDLIGHTVEC + 1]		; 法線・ローカルライト1
mul R_TMP.y, R_TMP.y, c[CV_ADDLIGHTVEC + 1].w		; x 光源強度1
dp3 R_TMP.z, R_NORMAL, c[CV_ADDLIGHTVEC + 2]		; 法線・ローカルライト2
mul R_TMP.z, R_TMP.z, c[CV_ADDLIGHTVEC + 2].w		; x 光源強度2
max R_TMP.xyz, R_TMP.xyz, c[CV_ZERO]				; R_TMP >= 0 を保証

mul R_ADDCOL.xyz, R_TMP.x, c[CV_ADDLIGHTCOL + 0]				; 光源強度1 x 光源色1
mad R_ADDCOL.xyz, R_TMP.y, c[CV_ADDLIGHTCOL + 1], R_ADDCOL.xyz	; + 光源強度2 x 光源色2
mad R_ADDCOL.xyz, R_TMP.z, c[CV_ADDLIGHTCOL + 2], R_ADDCOL.xyz	; + 光源強度3 x 光源色3

mad R_ADDCOL.xyz, V_DIFFUSE.xyz, c[CV_ADDLIGHTCOL + 3].xyz, R_ADDCOL.xyz ; 追加アンビエント
