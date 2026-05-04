; 平行3光源計算ルーチン
;
; 2001/05/14 F.Miyauchi
; $Id: light.vsh,v 1.1.1.3 2002/11/19 11:42:11 Yoshizawa1 Exp $
;
; R_NORMALは 正規化されていることが前提
;
; 出力: R_COL
; 破壊: R_TMP

; 光源計算(とりあえずMGS2に従って平行光源3つ)
dp3 R_TMP.x, R_NORMAL, c[CV_LIGHTVEC + 0]				; 法線・ローカルライト0
mul R_TMP.x, R_TMP.x, c[CV_LIGHTVEC + 0].w				; x 光源強度0
dp3 R_TMP.y, R_NORMAL, c[CV_LIGHTVEC + 1]				; 法線・ローカルライト1
mul R_TMP.y, R_TMP.y, c[CV_LIGHTVEC + 1].w				; x 光源強度1
dp3 R_TMP.z, R_NORMAL, c[CV_LIGHTVEC + 2]				; 法線・ローカルライト2
mul R_TMP.z, R_TMP.z, c[CV_LIGHTVEC + 2].w				; x 光源強度2
max R_TMP.xyz, R_TMP.xyz, c[CV_ZERO]					; R_TMP >= 0 を保証

mul R_COL.xyz, R_TMP.x, c[CV_LIGHTCOL + 0]				; 光源強度1 x 光源色1
mad R_COL.xyz, R_TMP.y, c[CV_LIGHTCOL + 1], R_COL.xyz	; + 光源強度2 x 光源色2
mad R_COL.xyz, R_TMP.z, c[CV_LIGHTCOL + 2], R_COL.xyz	; + 光源強度3 x 光源色3

mad R_COL.xyz, V_DIFFUSE.xyz, c[CV_LIGHTCOL + 3], R_COL.xyz
