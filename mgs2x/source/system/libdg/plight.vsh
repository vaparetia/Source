; 点光源計算ルーチン
; ※要最適化
;
; 2001/05/24 F.Miyauchi
; $Id: plight.vsh,v 1.1.1.3 2002/11/19 11:42:19 Yoshizawa1 Exp $
;
; 破壊: r9～r11
; 出力: R_PLIGHT_COL

/* 点光源パラメータ */
#define C_A0              c[CV_PLIGHTPARAM].x
#define C_A1              c[CV_PLIGHTPARAM].y
#define C_A2              c[CV_PLIGHTPARAM].z
#define C_RANGE           c[CV_PLIGHTCOL].w
#define C_SHADER_RANGE    c[CV_PLIGHTPARAM].w

/* ワーク */
#define R_VEC           r7
#define R_LIGHT_COEF    r8
#define FLAG            r9.x
#define ATTENUATION     r10.x

; 点光源0
; 最初のベクトルが逆なのは座標系の関係

add R_VEC, R_POSITION, -c[CV_PLIGHTPOS]						; r0 = ワールド位置
dp3 R_TMP.w, R_VEC, R_VEC									; R_TMP.w = d^2
rsq R_VEC.w, R_TMP.w										; R_VEC.w = 1 / d
dst R_TMP, R_TMP.wwww, R_VEC.wwww							; R_TMP = (1, d, d^2, 1 / d)
dp3 R_TMP.w, R_TMP, c[CV_PLIGHTPARAM]						; 3個の減衰パラメータを乗算
rcp ATTENUATION, R_TMP.w									; ATTENUATION = 減衰量

slt FLAG, R_TMP.y, C_RANGE									; FLAG = (d < C_RANGE) ? 1 : 0

dp3 R_LIGHT_COEF.x, R_NORMAL, R_VEC							; R_NORMAL = 正規化済み法線
max R_LIGHT_COEF.x, R_LIGHT_COEF.x, c[CV_ZERO].x			; 最小値0を保証
mul R_LIGHT_COEF.x, R_LIGHT_COEF.x, FLAG					; 距離限界を超えていればFLAG = 0
mul R_LIGHT_COEF.x, R_LIGHT_COEF.x, ATTENUATION				; 減衰量

mul R_PLIGHT_COL.xyz, R_LIGHT_COEF.xxx, c[CV_PLIGHTCOL].xyz

; 点光源1
; 点光源2
; 続く(かも....)



#ifdef DDDDDDDDDDDD
#define RANGE2          r9.y
#define D               r10.x
#define DxD             r11.y

; 無駄コード。
add R_VEC, R_POSITION, -c[CV_PLIGHTPOS]						; r0 = ワールド位置
dp3 DxD, R_VEC, R_VEC										; DxD = 距離2乗
mul RANGE2, C_RANGE, C_RANGE								; RANGE2 = 光到達距離2乗
slt FLAG, DxD, RANGE2										; FLAG = (DxD < RANGE2) ? 1 : 0
rsq D, DxD													; D = 1 / D
mul D, DxD, D												; 
mul ATTENUATION, DxD, C_A2									; A = d * d * A2
mad ATTENUATION, D, C_A1, ATTENUATION						; A += d * A1
add ATTENUATION, ATTENUATION, C_A0							; A += A0
rcp ATTENUATION, ATTENUATION								; A = 1 / A
#endif
