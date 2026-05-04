	//	exception_in.s
	//		例外割り込み処理
	//	2000/05/17	K.Uehara
	//	$Id: exception_in.s,v 1.4 2001/08/06 08:25:55 usr01475 Exp $
	//

	.text
	.global	DEBUG_ExceptionIn
	.global	DEBUG_ExceptionCallback

	.global DEBUG_exception_save_regs
	.global DEBUG_ExceptionStackTop

	.ent DEBUG_ExceptionIn
DEBUG_ExceptionIn:
	// すべてのレジスタをバッファに保存
	la	$kt0,DEBUG_exception_save_regs+0x20000000	// nocache

	.set noat
	sd	$1,0($kt0)
	.set at
	sd	$2,8($kt0)
	sd	$3,16($kt0)
	sd	$4,24($kt0)
	sd	$5,32($kt0)
	sd	$6,40($kt0)
	sd	$7,48($kt0)
	sd	$8,56($kt0)
	sd	$9,64($kt0)
	sd	$10,72($kt0)
	sd	$11,80($kt0)
	sd	$12,88($kt0)
	sd	$13,96($kt0)
	sd	$14,104($kt0)
	sd	$15,112($kt0)
	sd	$16,120($kt0)
	sd	$17,128($kt0)
	sd	$18,136($kt0)
	sd	$19,144($kt0)
	sd	$20,152($kt0)
	sd	$21,160($kt0)
	sd	$22,168($kt0)
	sd	$23,176($kt0)
	sd	$24,184($kt0)
	sd	$25,192($kt0)
	sd	$26,200($kt0)
	sd	$27,208($kt0)
	sd	$28,216($kt0)
	sd	$29,224($kt0)
	sd	$30,232($kt0)
	sd	$31,240($kt0)

	// スタック領域の移動

	la	$kt0,DEBUG_ExceptionStackTop
	lw	$sp,0($kt0)

	// C 関数の実行
	jal	DEBUG_ExceptionCallback
	nop

	// 戻す

	la	$kt0,DEBUG_exception_save_regs+0x20000000	// nocache

	.set noat
	ld	$1,0($kt0)
	.set at
	ld	$2,8($kt0)
	ld	$3,16($kt0)
	ld	$4,24($kt0)
	ld	$5,32($kt0)
	ld	$6,40($kt0)
	ld	$7,48($kt0)
	ld	$8,56($kt0)
	ld	$9,64($kt0)
	ld	$10,72($kt0)
	ld	$11,80($kt0)
	ld	$12,88($kt0)
	ld	$13,96($kt0)
	ld	$14,104($kt0)
	ld	$15,112($kt0)
	ld	$16,120($kt0)
	ld	$17,128($kt0)
	ld	$18,136($kt0)
	ld	$19,144($kt0)
	ld	$20,152($kt0)
	ld	$21,160($kt0)
	ld	$22,168($kt0)
	ld	$23,176($kt0)
	ld	$24,184($kt0)
	ld	$25,192($kt0)
	ld	$26,200($kt0)
	ld	$27,208($kt0)
	ld	$28,216($kt0)
	ld	$29,224($kt0)
	ld	$30,232($kt0)
	ld	$31,240($kt0)

	// デバッガへ

	j	0x80000100		// デバッグ例外ベクタ
//	j	0x80012ec8		// デバッグ例外ベクタ
//	break 0xFFFFF
	nop

	.end DEBUG_ExceptionCallback

