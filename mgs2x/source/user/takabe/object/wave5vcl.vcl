;
; wave5vcl.vcl
; 水面辺り判定補助ルーチン
;
; 2001/03/09 K.Takabe
; $Id: wave5vcl.vcl,v 1.1.1.3 2002/11/19 11:51:18 Yoshizawa1 Exp $
;
;
	.init_vf_all
	.init_vi_all


; ----------------------------------------------------------------
;WORK_MEM		.equ	128
S0_INTERP_MAT	.equ	WORK_MEM + 0
S1_INTERP_MAT	.equ	WORK_MEM + 4
T0_INTERP_MAT	.equ	WORK_MEM + 8
T1_INTERP_MAT	.equ	WORK_MEM + 12
;MAT_HERMITE		.equ	0
;FV_TTBL			.equ	4


; ----------------------------------------------------------------
; ＶＣＬ汎用マクロ

; ベクトルクリア
.macro	ResetVec	vec
	sub.xyzw		\vec, vf00, vf00
.endm

; マトリクスクリア
.macro	ResetMAT	mat
	sub.xyzw		\mat[0], vf00, vf00
	sub.xyzw		\mat[1], vf00, vf00
	sub.xyzw		\mat[2], vf00, vf00
	sub.xyzw		\mat[3], vf00, vf00
.endm

; マトリクスコピー
.macro MoveMatrix	dst, src
	move.xyzw		\dst[0], \src[0]
	move.xyzw		\dst[1], \src[1]
	move.xyzw		\dst[2], \src[2]
	move.xyzw		\dst[3], \src[3]
.endm

; マトリクスロード
.macro LoadMatrix	mat, base, addr
	lq.xyzw			\mat[0], \addr+0(\base)
	lq.xyzw			\mat[1], \addr+1(\base)
	lq.xyzw			\mat[2], \addr+2(\base)
	lq.xyzw			\mat[3], \addr+3(\base)
.endm

; マトリクスストア
.macro StoreMatrix	mat, base, addr
	sq.xyzw			\mat[0], \addr+0(\base)
	sq.xyzw			\mat[1], \addr+1(\base)
	sq.xyzw			\mat[2], \addr+2(\base)
	sq.xyzw			\mat[3], \addr+3(\base)
.endm

; マトリクス乗算
.macro MulMatrix res, mat0, mat1
	mulax.xyzw		ACC, \mat0[0], \mat1[0]
	madday.xyzw		ACC, \mat0[1], \mat1[0]
	maddaz.xyzw		ACC, \mat0[2], \mat1[0]
	maddw.xyzw		\res[0], \mat0[3], \mat1[0]
	mulax.xyzw		ACC, \mat0[0], \mat1[1]
	madday.xyzw		ACC, \mat0[1], \mat1[1]
	maddaz.xyzw		ACC, \mat0[2], \mat1[1]
	maddw.xyzw		\res[1], \mat0[3], \mat1[1]
	mulax.xyzw		ACC, \mat0[0], \mat1[2]
	madday.xyzw		ACC, \mat0[1], \mat1[2]
	maddaz.xyzw		ACC, \mat0[2], \mat1[2]
	maddw.xyzw		\res[2], \mat0[3], \mat1[2]
	mulax.xyzw		ACC, \mat0[0], \mat1[3]
	madday.xyzw		ACC, \mat0[1], \mat1[3]
	maddaz.xyzw		ACC, \mat0[2], \mat1[3]
	maddw.xyzw		\res[3], \mat0[3], \mat1[3]
.endm

; ベクトル乗算
.macro ApplyMatrix res, mat0, vec
	mulax.xyzw		ACC, \mat0[0], \vec
	madday.xyzw		ACC, \mat0[1], \vec
	maddaz.xyzw		ACC, \mat0[2], \vec
	maddw.xyzw		\res, \mat0[3], \vec
.endm

; 総和計算
.macro SumVecXYZ	res, vec
	mulax.w			ACC, vf00, \vec
	madday.w		ACC, vf00, \vec
	maddz.w			\res, vf00, \vec
.endm

; ベクトル外積
.macro OuterProduct ans,vec0,vec1
	opmula.xyz		ACC, \vec0, \vec1
	opmsub.xyz		\ans, \vec1, \vec0
.endm

; ベクトル内積
.macro InnerProduct	ans, vec0, vec1
	mul.xyz			tmp_vec0, \vec0, \vec1
	SumVecXYZ		\ans, tmp_vec0
.endm

; ベクトル正規化
.macro Normalize ans, vec
	InnerProduct	tmp_vec0, \vec, \vec
	rsqrt			Q, vf00w, tmp_vec0w
	mulq.xyz		\ans, \vec, Q
.endm

; ----------------------------------------------------------------
; ローカルマクロ

; エルミート補完マトリクス生成
.macro MakeHermiteMatrix	res, q0, q1, d0, d1
	move.xyzw		tmp_mat0[0], \q0
	move.xyzw		tmp_mat0[1], \q1
	move.xyzw		tmp_mat0[2], \d0
	move.xyzw		tmp_mat0[3], \d1
	LoadMatrix		tmp_mat1, vi00, MAT_HERMITE
	MulMatrix		\res, tmp_mat0, tmp_mat1
.endm

; ベクトルの線形補完（t_vec.vzをtとみなす）
.macro LerpVecZ	res, vec0, vec1, t_vec
	subz.w			tmp_vec0, vf00, \t_vec
	mulaz.xyzw		ACC, \vec0, \t_vec
	maddw.xyzw		\res, \vec1, tmp_vec0
.endm



; ----------------------------------------------------------------
; パッチ曲面との垂直方向当たり判定計算ルーチン
	.name CalcPatchRayPick
--enter
	in_vf			pos(vf03)
	in_vf			pos0(vf04)
	in_vf			pos1(vf05)
	in_vf			pos2(vf06)
	in_vf			pos3(vf07)
	in_vf			pos0_ds(vf08)
	in_vf			pos1_ds(vf09)
	in_vf			pos2_ds(vf10)
	in_vf			pos3_ds(vf11)
	in_vf			pos0_dt(vf12)
	in_vf			pos1_dt(vf13)
	in_vf			pos2_dt(vf14)
	in_vf			pos3_dt(vf15)
--endenter

	ResetVec			normal
	iaddiu				result, vi00, 0

; --------
	iaddiu				tt, vi00, 0
LOOP_T:
	LoadMatrix			s0_interp_mat, vi00, S1_INTERP_MAT
	StoreMatrix			s0_interp_mat, vi00, S0_INTERP_MAT

	lq.xyzw				t0, FV_TTBL(tt)

	MakeHermiteMatrix	t0_interp_mat, pos0, pos2, pos0_dt, pos2_dt
	ApplyMatrix			p0, t0_interp_mat, t0
	MakeHermiteMatrix	t1_interp_mat, pos1, pos3, pos1_dt, pos3_dt
	ApplyMatrix			p1, t1_interp_mat, t0

	LerpVecZ			dp0, pos0_ds, pos2_ds, t0
	LerpVecZ			dp1, pos1_ds, pos3_ds, t0

	MakeHermiteMatrix	s1_interp_mat, p0, p1, dp0, dp1
	StoreMatrix			s1_interp_mat, vi00, S1_INTERP_MAT

	ibeq				tt, vi00, LOOP_T_CNT

; --------
	ResetVec			pv1
	ResetVec			pv3
	iaddiu				ts, vi00, 0
LOOP_S:
	lq.xyzw				t1, FV_TTBL(ts)
	move.xyzw			pv0, pv1
	move.xyzw			pv2, pv3

	LoadMatrix			s0_interp_mat, vi00, S0_INTERP_MAT
	ApplyMatrix			pv1, s0_interp_mat, t1
	LoadMatrix			s1_interp_mat, vi00, S1_INTERP_MAT
	ApplyMatrix			pv3, s1_interp_mat, t1

	ibeq				ts, vi00, LOOP_S_CNT

	iaddiu				sign_y, vi00, 0x40			; Y < 0 flag

	sub.xyz				vec_v1_v0, pv1, pv0
	sub.xyz				vec_pos_v0, pos, pv0
	sub.xyz				vec_v3_v1, pv3, pv1
	sub.xyz				vec_pos_v1, pos, pv1
	sub.xyz				vec_v2_v3, pv2, pv3
	sub.xyz				vec_pos_v3, pos, pv3
	sub.xyz				vec_v0_v2, pv0, pv2
	sub.xyz				vec_pos_v2, pos, pv2

	OuterProduct		op1, vec_pos_v0, vec_v1_v0
	fmand				flag_check, sign_y
	OuterProduct		op2, vec_pos_v1, vec_v3_v1
	fmand				tmp_int, sign_y
	ior					flag_check, flag_check, tmp_int
	OuterProduct		op3, vec_pos_v3, vec_v2_v3
	fmand				tmp_int, sign_y
	ior					flag_check, flag_check, tmp_int
	OuterProduct		op4, vec_pos_v2, vec_v0_v2
	fmand				tmp_int, sign_y
	ior					flag_check, flag_check, tmp_int

	OuterProduct		normal, vec_v1_v0, vec_v0_v2
	mul.xyz				tmp_vec1, normal, normal

	ibne				flag_check, vi00, LOOP_S_CNT

	SumVecXYZ			tmp_vec1, tmp_vec1					; 法線の二乗和算出
	InnerProduct		normal, vec_pos_v2, normal
	rsqrt				Q, vf00w, tmp_vec1w
	mulq.xyzw			normal, normal, Q
	subw.y				pos, pos, normal


;	OuterProduct		op1, vec_v1_v0, vec_pos_v0
;	fmand				flag_check, sign_y
;	ibeq				flag_check, vi00, LOOP_S_CNT
;	OuterProduct		op2, vec_v3_v1, vec_pos_v1
;	fmand				flag_check, sign_y
;	ibeq				flag_check, vi00, LOOP_S_CNT
;	OuterProduct		op3, vec_v2_v3, vec_pos_v3
;	fmand				flag_check, sign_y
;	ibeq				flag_check, vi00, LOOP_S_CNT
;	OuterProduct		op4, vec_v0_v2, vec_pos_v2
;	fmand				flag_check, sign_y
;	ibeq				flag_check, vi00, LOOP_S_CNT

;	OuterProduct		normal, vec_v1_v0, vec_v0_v2
;	Normalize			normal, normal
;	InnerProduct		tmp_vec0, vec_pos_v2, normal

;	subw.y				pos, pos, tmp_vec0

	iaddi				result, vi00, -1
	b					END_PROG

LOOP_S_CNT:
	iaddi				ts, ts, 4
	iaddiu				max_count, vi00, 16
	isub				max_count, max_count, ts
	ibgez				max_count, LOOP_S
LOOP_S_END:


LOOP_T_CNT:
	iaddi				tt, tt, 4
	iaddiu				max_count, vi00, 16
	isub				max_count, max_count, tt
	ibgez				max_count, LOOP_T
LOOP_T_END:

END_PROG:

--exit
	out_vf				pos(vf03)
	out_vf				normal(vf02)
	out_vi				result(vi15)
--endexit

.END