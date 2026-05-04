	.file	1 "screen.c"
gcc2_compiled.:
__gnu_compiled_c:
	.text
	.p2align 3
	.ent	MakeBoundVerts
MakeBoundVerts:
	.frame	$sp,48,$31		# vars= 0, regs= 6/0, args= 0, extra= 0
	.mask	0x00070000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,48
	move	$10,$6
	sd	$18,32($sp)
	move	$2,$0
	sd	$17,16($sp)
	move	$8,$10
	sd	$16,0($sp)
	lw	$16,4($5)
	lw	$18,20($5)
	lw	$17,0($5)
	lw	$25,8($5)
	lw	$24,12($5)
	lw	$15,16($5)
 #APP
	
	lqc2		vf4,0x00($4)
	lqc2		vf5,0x10($4)
	lqc2		vf6,0x20($4)
	lqc2		vf7,0x30($4)
	
 #NO_APP
	li	$13,1879048192			# 0x70000000
	li	$12,16711680			# 0xff0000
	li.s	$f0,1.00000000000000000000e0
	li	$9,8			# 0x8
	ori	$13,$13,0xa40
	ori	$12,$12,0xffff
	.p2align 3
$L61:
	dsll	$14,$2,24
	li	$11,4			# 0x4
	.p2align 3
$L65:
	andi	$5,$9,0x1
	andi	$6,$9,0x2
	andi	$7,$9,0x4
	move	$2,$17
	move	$3,$16
	move	$4,$25
	movz	$2,$24,$5
	movz	$3,$15,$6
	movz	$4,$18,$7
	sw	$2,0($8)
	sw	$3,4($8)
	addu	$11,$11,-1
	sw	$4,8($8)
	addu	$9,$9,-1
	s.s	$f0,12($10)
	addu	$8,$8,16
	.set	noreorder
	.set	nomacro
	bgtz	$11,$L65
	addu	$10,$10,16
	.set	macro
	.set	reorder

	addu	$3,$10,-64
 #APP
	
	lqc2			vf8,0x00($3)
	lqc2			vf9,0x10($3)
	lqc2			vf10,0x20($3)
	lqc2			vf11,0x30($3)
	lqc2			vf1,0($13)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf8
	vmulax.xyzw		ACC, vf4,vf9
	vmadday.xyzw	ACC, vf5,vf9
	vmaddaz.xyzw	ACC, vf6,vf9
	vmaddw.xyzw		vf9, vf7,vf9
	vmulax.xyzw		ACC, vf4,vf10
	vmadday.xyzw	ACC, vf5,vf10
	vmaddaz.xyzw	ACC, vf6,vf10
	vmaddw.xyzw		vf10, vf7,vf10
	vmulax.xyzw		ACC, vf4,vf11
	vmadday.xyzw	ACC, vf5,vf11
	vmaddaz.xyzw	ACC, vf6,vf11
	vmaddw.xyzw		vf11, vf7,vf11
	vclipw.xyz		vf8xyz,vf8w
	vclipw.xyz		vf9xyz,vf9w
	vclipw.xyz		vf10xyz,vf10w
	vclipw.xyz		vf11xyz,vf11w
	vdiv			Q,vf0w,vf8w
	vwaitq
	vmulq.xy		vf8,vf8,Q
	vdiv			Q,vf0w,vf9w
	vmul.xy			vf8,vf8,vf1
	vwaitq
	vmulq.xy		vf9,vf9,Q
	vdiv			Q,vf0w,vf10w
	vmul.xy			vf9,vf9,vf1
	vwaitq
	vmulq.xy		vf10,vf10,Q
	vdiv			Q,vf0w,vf11w
	vmul.xy			vf10,vf10,vf1
	vwaitq
	vmulq.xy		vf11,vf11,Q
	cfc2			$2,$18 /*clipping*/
	sqc2			vf8,0x00($3)
	vmul.xy			vf11,vf11,vf1
	sqc2			vf9,0x10($3)
	sqc2			vf10,0x20($3)
	sqc2			vf11,0x30($3)
	
 #NO_APP
	and	$2,$12,$2
	.set	noreorder
	.set	nomacro
	bgtz	$9,$L61
	or	$2,$2,$14
	.set	macro
	.set	reorder

	ld	$18,32($sp)
	ld	$17,16($sp)
	ld	$16,0($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,48
	.set	macro
	.set	reorder

	.end	MakeBoundVerts
	.p2align 3
	.ent	BoundCheck
BoundCheck:
	.frame	$sp,48,$31		# vars= 0, regs= 6/0, args= 0, extra= 0
	.mask	0x80030000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,48
	li	$6,1879048192			# 0x70000000
	sd	$16,0($sp)
	ori	$6,$6,0x9c0
	sd	$17,16($sp)
	li	$16,1879048192			# 0x70000000
	sd	$31,32($sp)
	.set	noreorder
	.set	nomacro
	jal	MakeBoundVerts
	ori	$16,$16,0x9c0
	.set	macro
	.set	reorder

 #APP
	di
 #NO_APP
	l.s	$f0,12($16)
	l.s	$f1,28($16)
	l.s	$f2,1879050832
	add.s	$f0,$f0,$f1
	mul.s	$f12,$f0,$f2
 #APP
	ei
 #NO_APP
	move	$3,$2
	li	$4,7			# 0x7
	.p2align 3
$L80:
	dsrl	$2,$2,6
	addu	$4,$4,-1
	.set	noreorder
	.set	nomacro
	bgtz	$4,$L80
	and	$3,$3,$2
	.set	macro
	.set	reorder

	andi	$2,$3,0xf
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L99
	li	$2,2			# 0x2
	.set	macro
	.set	reorder

	dli	$2,0x30		# 48
	mtc1	$0,$f5
	or	$3,$3,$2
	li.s	$f4,-2.04800000000000000000e3
	li.s	$f3,2.04700000000000000000e3
	move	$17,$0
	li	$4,8			# 0x8
	dli	$5,0xffffffffffffffef		# -17
	dli	$2,0xffffffffffffffdf		# -33
	.p2align 3
$L86:
	l.s	$f1,8($16)
	l.s	$f0,12($16)
	#nop
	sub.s	$f0,$f0,$f1
	c.olt.s	$f0,$f5
	#nop
	.set	noreorder
	.set	nomacro
	bc1f	$L87
	mov.s	$f2,$f1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L88
	li	$17,1			# 0x1
	.set	macro
	.set	reorder

	.p2align 2
$L87:
	and	$3,$3,$5
$L88:
	c.ole.s	$f2,$f5
	#nop
	.set	noreorder
	.set	nomacro
	bc1fl	$L89
	and	$3,$3,$2
	.set	macro
	.set	reorder

	li	$17,1			# 0x1
$L89:
	l.s	$f0,0($16)
	#nop
	c.olt.s	$f0,$f4
	#nop
	.set	noreorder
	.set	nomacro
	bc1tl	$L91
	li	$17,1			# 0x1
	.set	macro
	.set	reorder

	c.olt.s	$f3,$f0
	#nop
	.set	noreorder
	.set	nomacro
	bc1fl	$L102
	l.s	$f0,4($16)
	.set	macro
	.set	reorder

	li	$17,1			# 0x1
$L91:
	l.s	$f0,4($16)
$L102:
	c.olt.s	$f0,$f4
	#nop
	.set	noreorder
	.set	nomacro
	bc1tl	$L103
	li	$17,1			# 0x1
	.set	macro
	.set	reorder

	c.olt.s	$f3,$f0
	#nop
	.set	noreorder
	.set	nomacro
	bc1fl	$L104
	addu	$4,$4,-1
	.set	macro
	.set	reorder

	li	$17,1			# 0x1
$L103:
	addu	$4,$4,-1
$L104:
	.set	noreorder
	.set	nomacro
	bgtz	$4,$L86
	addu	$16,$16,16
	.set	macro
	.set	reorder

	andi	$2,$3,0x30
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L99
	li	$2,2			# 0x2
	.set	macro
	.set	reorder

 #APP
	di
 #NO_APP
	jal	fptosi
	sw	$2,1879050836
 #APP
	ei
 #NO_APP
	move	$2,$17
$L99:
	ld	$31,32($sp)
	ld	$17,16($sp)
	ld	$16,0($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,48
	.set	macro
	.set	reorder

	.end	BoundCheck
	.p2align 3
	.ent	GetEnvelopeMatrix
GetEnvelopeMatrix:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 0, extra= 0
	.mask	0x80010000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	li	$3,1879048192			# 0x70000000
	sd	$16,0($sp)
	ori	$3,$3,0x900
	move	$16,$4
	sd	$31,16($sp)
 #APP
	lq $8,0($5);lq $9,16($5);lq $10,32($5);lq $11,48($5);									sq $8,0($3);sq $9,16($3);sq $10,32($3);sq $11,48($3);
 #NO_APP
	li	$2,1879048192			# 0x70000000
	ori	$2,$2,0x8c0
 #APP
	lq $8,0($6);lq $9,16($6);lq $10,32($6);lq $11,48($6);									sq $8,0($2);sq $9,16($2);sq $10,32($2);sq $11,48($2);
 #NO_APP
	sw	$0,56($2)
	li	$4,1879048192			# 0x70000000
	sw	$0,48($2)
	li	$5,1879048192			# 0x70000000
	sw	$0,52($2)
	ori	$4,$4,0x900
	sw	$0,56($3)
	ori	$5,$5,0x900
	sw	$0,48($3)
	.set	noreorder
	.set	nomacro
	jal	sceVu0InversMatrix
	sw	$0,52($3)
	.set	macro
	.set	reorder

	move	$4,$16
	li	$5,1879048192			# 0x70000000
	li	$6,1879048192			# 0x70000000
	ld	$31,16($sp)
	ld	$16,0($sp)
	ori	$5,$5,0x900
	ori	$6,$6,0x8c0
	.set	noreorder
	.set	nomacro
	j	sceVu0MulMatrix
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	GetEnvelopeMatrix
	.p2align 3
	.ent	OnePieceObjs
OnePieceObjs:
	.frame	$sp,96,$31		# vars= 0, regs= 12/0, args= 0, extra= 0
	.mask	0x801f0000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,96
	move	$2,$4
	sd	$16,0($sp)
	li	$4,1879048192			# 0x70000000
	sd	$17,16($sp)
	move	$16,$5
	li	$6,1879048192			# 0x70000000
	sd	$19,48($sp)
	sd	$31,80($sp)
	ori	$4,$4,0xc0
	sd	$20,64($sp)
	li	$5,1879048192			# 0x70000000
	sd	$18,32($sp)
	ori	$6,$6,0x80
	addu	$17,$2,96
	.set	noreorder
	.set	nomacro
	jal	sceVu0MulMatrix
	move	$19,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	blez	$19,$L119
	li	$18,1879048192			# 0x70000000
	.set	macro
	.set	reorder

	li	$16,1879048192			# 0x70000000
	ori	$18,$18,0x80
	ori	$16,$16,0xc0
	li	$20,1			# 0x1
	.p2align 3
$L120:
	ld	$2,0($18)
	li	$4,1879048192			# 0x70000000
	lw	$5,196($17)
	ori	$4,$4,0xc0
	sd	$2,0($17)
	addu	$5,$5,8
	ld	$2,8($18)
	#nop
	sd	$2,8($17)
	ld	$3,16($18)
	#nop
	sd	$3,16($17)
	ld	$2,24($18)
	#nop
	sd	$2,24($17)
	ld	$3,32($18)
	#nop
	sd	$3,32($17)
	ld	$2,40($18)
	#nop
	sd	$2,40($17)
	ld	$3,48($18)
	#nop
	sd	$3,48($17)
	ld	$2,56($18)
	#nop
	.set	noreorder
	.set	nomacro
	jal	BoundCheck
	sd	$2,56($17)
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	beq	$4,$20,$L113
	slt	$2,$4,2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L117
	li	$2,2			# 0x2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L112
	li	$5,1879048192			# 0x70000000
	.set	macro
	.set	reorder

	b	$L111
	.p2align 2
$L117:
	.set	noreorder
	.set	nomacro
	beql	$4,$2,$L114
	sh	$4,212($17)
	.set	macro
	.set	reorder

	b	$L111
	.p2align 2
$L112:
	li	$6,1879048192			# 0x70000000
	addu	$4,$17,64
	ori	$5,$5,0x40
	.set	noreorder
	.set	nomacro
	jal	sceVu0MulMatrix
	ori	$6,$6,0x80
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L111
	sh	$0,212($17)
	.set	macro
	.set	reorder

	.p2align 2
$L113:
	ld	$2,0($16)
	#nop
	sd	$2,64($17)
	ld	$3,8($16)
	#nop
	sd	$3,72($17)
	ld	$2,16($16)
	#nop
	sd	$2,80($17)
	ld	$3,24($16)
	#nop
	sd	$3,88($17)
	ld	$2,32($16)
	#nop
	sd	$2,96($17)
	ld	$3,40($16)
	#nop
	sd	$3,104($17)
	ld	$2,48($16)
	#nop
	sd	$2,112($17)
	ld	$3,56($16)
	sh	$4,212($17)
	sd	$3,120($17)
$L114:
$L111:
	lw	$2,1879050836
	addu	$19,$19,-1
	sw	$2,208($17)
	.set	noreorder
	.set	nomacro
	bgtz	$19,$L120
	addu	$17,$17,240
	.set	macro
	.set	reorder

$L119:
	ld	$31,80($sp)
	ld	$20,64($sp)
	ld	$19,48($sp)
	ld	$18,32($sp)
	ld	$17,16($sp)
	ld	$16,0($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,96
	.set	macro
	.set	reorder

	.end	OnePieceObjs
	.p2align 3
	.ent	ScreenObjs
ScreenObjs:
	.frame	$sp,128,$31		# vars= 0, regs= 16/0, args= 0, extra= 0
	.mask	0x807f0000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,128
	sd	$21,80($sp)
	sd	$18,32($sp)
	move	$21,$4
	sd	$19,48($sp)
	li	$18,1879048192			# 0x70000000
	sd	$16,0($sp)
	move	$19,$5
	sd	$31,112($sp)
	ori	$18,$18,0xc0
	sd	$22,96($sp)
	addu	$16,$21,96
	sd	$20,64($sp)
	.set	noreorder
	.set	nomacro
	blez	$19,$L135
	sd	$17,16($sp)
	.set	macro
	.set	reorder

	li	$17,1879048192			# 0x70000000
	li	$20,1879048192			# 0x70000000
	li	$22,1			# 0x1
	ori	$17,$17,0x8c0
	ori	$20,$20,0xc0
	.p2align 3
$L136:
	lw	$2,196($16)
	addu	$4,$16,128
	move	$5,$18
	lw	$6,44($2)
	#nop
	sll	$6,$6,6
	.set	noreorder
	.set	nomacro
	jal	GetEnvelopeMatrix
	addu	$6,$6,$20
	.set	macro
	.set	reorder

	li	$4,1879048192			# 0x70000000
	li	$5,1879048192			# 0x70000000
	ori	$4,$4,0x8c0
	.set	noreorder
	.set	nomacro
	jal	sceVu0MulMatrix
	move	$6,$18
	.set	macro
	.set	reorder

	lw	$5,196($16)
	li	$4,1879048192			# 0x70000000
	ori	$4,$4,0x8c0
	.set	noreorder
	.set	nomacro
	jal	BoundCheck
	addu	$5,$5,8
	.set	macro
	.set	reorder

	lw	$3,72($21)
	move	$4,$22
	andi	$3,$3,0x800
	movz	$4,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$4,$22,$L129
	slt	$2,$4,2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L133
	li	$2,2			# 0x2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L128
	li	$5,1879048192			# 0x70000000
	.set	macro
	.set	reorder

	b	$L127
	.p2align 2
$L133:
	.set	noreorder
	.set	nomacro
	beql	$4,$2,$L130
	sh	$4,212($16)
	.set	macro
	.set	reorder

	b	$L127
	.p2align 2
$L128:
	addu	$4,$16,64
	ori	$5,$5,0x40
	.set	noreorder
	.set	nomacro
	jal	sceVu0MulMatrix
	move	$6,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L127
	sh	$0,212($16)
	.set	macro
	.set	reorder

	.p2align 2
$L129:
	ld	$2,0($17)
	#nop
	sd	$2,64($16)
	ld	$3,8($17)
	#nop
	sd	$3,72($16)
	ld	$2,16($17)
	#nop
	sd	$2,80($16)
	ld	$3,24($17)
	#nop
	sd	$3,88($16)
	ld	$2,32($17)
	#nop
	sd	$2,96($16)
	ld	$3,40($17)
	#nop
	sd	$3,104($16)
	ld	$2,48($17)
	#nop
	sd	$2,112($16)
	ld	$3,56($17)
	sh	$4,212($16)
	sd	$3,120($16)
$L130:
$L127:
	lw	$2,1879050836
	addu	$18,$18,64
	addu	$19,$19,-1
	sw	$2,208($16)
	.set	noreorder
	.set	nomacro
	bgtz	$19,$L136
	addu	$16,$16,240
	.set	macro
	.set	reorder

$L135:
	ld	$31,112($sp)
	ld	$22,96($sp)
	ld	$21,80($sp)
	ld	$20,64($sp)
	ld	$19,48($sp)
	ld	$18,32($sp)
	ld	$17,16($sp)
	ld	$16,0($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,128
	.set	macro
	.set	reorder

	.end	ScreenObjs
	.p2align 3
	.ent	ScreenObjs2
ScreenObjs2:
	.frame	$sp,112,$31		# vars= 0, regs= 14/0, args= 0, extra= 0
	.mask	0x803f0000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,112
	sd	$20,64($sp)
	sd	$17,16($sp)
	move	$20,$4
	sd	$19,48($sp)
	li	$17,1879048192			# 0x70000000
	sd	$16,0($sp)
	move	$19,$5
	sd	$31,96($sp)
	ori	$17,$17,0xc0
	sd	$21,80($sp)
	addu	$16,$20,96
	.set	noreorder
	.set	nomacro
	blez	$19,$L151
	sd	$18,32($sp)
	.set	macro
	.set	reorder

	li	$18,1879048192			# 0x70000000
	li	$21,1			# 0x1
	ori	$18,$18,0x8c0
	.p2align 3
$L152:
	ld	$2,0($16)
	li	$7,1879048192			# 0x70000000
	ori	$7,$7,0xc0
	addu	$4,$16,128
	sd	$2,0($17)
	move	$5,$17
	ld	$2,8($16)
	#nop
	sd	$2,8($17)
	ld	$3,16($16)
	#nop
	sd	$3,16($17)
	ld	$2,24($16)
	#nop
	sd	$2,24($17)
	ld	$3,32($16)
	#nop
	sd	$3,32($17)
	ld	$2,40($16)
	#nop
	sd	$2,40($17)
	ld	$3,48($16)
	#nop
	sd	$3,48($17)
	ld	$2,56($16)
	#nop
	sd	$2,56($17)
	lw	$3,196($16)
	#nop
	lw	$6,44($3)
	#nop
	sll	$6,$6,6
	.set	noreorder
	.set	nomacro
	jal	GetEnvelopeMatrix
	addu	$6,$6,$7
	.set	macro
	.set	reorder

	li	$4,1879048192			# 0x70000000
	li	$5,1879048192			# 0x70000000
	ori	$4,$4,0x8c0
	.set	noreorder
	.set	nomacro
	jal	sceVu0MulMatrix
	move	$6,$17
	.set	macro
	.set	reorder

	lw	$5,196($16)
	li	$4,1879048192			# 0x70000000
	ori	$4,$4,0x8c0
	.set	noreorder
	.set	nomacro
	jal	BoundCheck
	addu	$5,$5,8
	.set	macro
	.set	reorder

	lw	$3,72($20)
	move	$4,$21
	andi	$3,$3,0x800
	movz	$4,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$4,$21,$L145
	slt	$2,$4,2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L149
	li	$2,2			# 0x2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L144
	li	$5,1879048192			# 0x70000000
	.set	macro
	.set	reorder

	b	$L143
	.p2align 2
$L149:
	.set	noreorder
	.set	nomacro
	beql	$4,$2,$L146
	sh	$4,212($16)
	.set	macro
	.set	reorder

	b	$L143
	.p2align 2
$L144:
	addu	$4,$16,64
	ori	$5,$5,0x40
	.set	noreorder
	.set	nomacro
	jal	sceVu0MulMatrix
	move	$6,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L143
	sh	$0,212($16)
	.set	macro
	.set	reorder

	.p2align 2
$L145:
	ld	$2,0($18)
	#nop
	sd	$2,64($16)
	ld	$3,8($18)
	#nop
	sd	$3,72($16)
	ld	$2,16($18)
	#nop
	sd	$2,80($16)
	ld	$3,24($18)
	#nop
	sd	$3,88($16)
	ld	$2,32($18)
	#nop
	sd	$2,96($16)
	ld	$3,40($18)
	#nop
	sd	$3,104($16)
	ld	$2,48($18)
	#nop
	sd	$2,112($16)
	ld	$3,56($18)
	sh	$4,212($16)
	sd	$3,120($16)
$L146:
$L143:
	lw	$2,1879050836
	addu	$17,$17,64
	addu	$19,$19,-1
	sw	$2,208($16)
	.set	noreorder
	.set	nomacro
	bgtz	$19,$L152
	addu	$16,$16,240
	.set	macro
	.set	reorder

$L151:
	ld	$31,96($sp)
	ld	$21,80($sp)
	ld	$20,64($sp)
	ld	$19,48($sp)
	ld	$18,32($sp)
	ld	$17,16($sp)
	ld	$16,0($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,112
	.set	macro
	.set	reorder

	.end	ScreenObjs2
	.p2align 3
	.ent	SlideFrameObjs
SlideFrameObjs:
	.frame	$sp,128,$31		# vars= 16, regs= 14/0, args= 0, extra= 0
	.mask	0x803f0000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,128
	sd	$19,64($sp)
	sd	$18,48($sp)
	move	$19,$5
	sd	$17,32($sp)
	addu	$18,$4,96
	sd	$31,112($sp)
	li	$17,1879048192			# 0x70000000
	sd	$21,96($sp)
	ori	$17,$17,0xc0
	sd	$16,16($sp)
	sd	$20,80($sp)
	.set	noreorder
	.set	nomacro
	blez	$19,$L159
	lw	$20,92($4)
	.set	macro
	.set	reorder

	li	$21,1879048192			# 0x70000000
	ori	$21,$21,0xc0
	.p2align 3
$L160:
	lw	$2,196($18)
	move	$6,$20
	move	$4,$sp
	addu	$20,$20,16
	lw	$16,44($2)
	addu	$19,$19,-1
	sll	$16,$16,6
	addu	$16,$16,$21
	.set	noreorder
	.set	nomacro
	jal	sceVu0ApplyMatrix
	move	$5,$16
	.set	macro
	.set	reorder

	ld	$4,0($16)
	#nop
	sd	$4,0($17)
	ld	$2,8($16)
	#nop
	sd	$2,8($17)
	ld	$3,16($16)
	#nop
	sd	$3,16($17)
	ld	$2,24($16)
	#nop
	sd	$2,24($17)
	ld	$3,32($16)
	#nop
	sd	$3,32($17)
	ld	$2,40($16)
	#nop
	sd	$2,40($17)
	ld	$3,48($16)
	#nop
	sd	$3,48($17)
	ld	$2,56($16)
	#nop
	sd	$2,56($17)
	l.s	$f0,0($sp)
	#nop
	s.s	$f0,48($17)
	l.s	$f1,4($sp)
	#nop
	s.s	$f1,52($17)
	l.s	$f0,8($sp)
	#nop
	s.s	$f0,56($17)
	sd	$4,0($18)
	ld	$2,8($17)
	#nop
	sd	$2,8($18)
	ld	$3,16($17)
	#nop
	sd	$3,16($18)
	ld	$2,24($17)
	#nop
	sd	$2,24($18)
	ld	$3,32($17)
	#nop
	sd	$3,32($18)
	ld	$2,40($17)
	#nop
	sd	$2,40($18)
	ld	$3,48($17)
	#nop
	sd	$3,48($18)
	ld	$2,56($17)
	addu	$17,$17,64
	sd	$2,56($18)
	.set	noreorder
	.set	nomacro
	bgtz	$19,$L160
	addu	$18,$18,240
	.set	macro
	.set	reorder

$L159:
	ld	$31,112($sp)
	ld	$21,96($sp)
	ld	$20,80($sp)
	ld	$19,64($sp)
	ld	$18,48($sp)
	ld	$17,32($sp)
	ld	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,128
	.set	macro
	.set	reorder

	.end	SlideFrameObjs
	.p2align 3
	.ent	JointFrameObjs
JointFrameObjs:
	.frame	$sp,160,$31		# vars= 0, regs= 18/1, args= 0, extra= 0
	.mask	0x80ff0000,-32
	.fmask	0x00100000,-16
	subu	$sp,$sp,160
	sd	$22,96($sp)
	move	$22,$4
	sd	$23,112($sp)
	li	$4,1879048192			# 0x70000000
	sd	$21,80($sp)
	sd	$20,64($sp)
	ori	$4,$4,0x8c0
	sd	$18,32($sp)
	li	$23,1879048192			# 0x70000000
	sd	$17,16($sp)
	ori	$23,$23,0xc0
	sd	$31,128($sp)
	addu	$17,$22,96
	sd	$19,48($sp)
	move	$21,$0
	sd	$16,0($sp)
	.set	noreorder
	.set	nomacro
	jal	sceVu0UnitMatrix
	s.s	$f20,144($sp)
	.set	macro
	.set	reorder

	lw	$2,68($22)
	lh	$3,78($22)
	addu	$18,$2,32
	.set	noreorder
	.set	nomacro
	blez	$3,$L167
	lw	$20,88($22)
	.set	macro
	.set	reorder

	li	$19,1879048192			# 0x70000000
	li	$16,1879048192			# 0x70000000
	li.s	$f20,1.00000000000000000000e0
	ori	$19,$19,0x8c0
	ori	$16,$16,0xc0
	.p2align 3
$L168:
	li	$4,1879048192			# 0x70000000
	addu	$21,$21,1
	.set	noreorder
	.set	nomacro
	jal	sceVu0UnitMatrix
	ori	$4,$4,0x8c0
	.set	macro
	.set	reorder

	l.s	$f12,0($20)
	li	$4,1879048192			# 0x70000000
	li	$5,1879048192			# 0x70000000
	ori	$4,$4,0x8c0
	.set	noreorder
	.set	nomacro
	jal	sceVu0RotMatrixX
	ori	$5,$5,0x8c0
	.set	macro
	.set	reorder

	l.s	$f12,4($20)
	li	$4,1879048192			# 0x70000000
	li	$5,1879048192			# 0x70000000
	ori	$4,$4,0x8c0
	.set	noreorder
	.set	nomacro
	jal	sceVu0RotMatrixY
	ori	$5,$5,0x8c0
	.set	macro
	.set	reorder

	l.s	$f12,8($20)
	li	$4,1879048192			# 0x70000000
	li	$5,1879048192			# 0x70000000
	ori	$4,$4,0x8c0
	ori	$5,$5,0x8c0
	.set	noreorder
	.set	nomacro
	jal	sceVu0RotMatrixZ
	addu	$20,$20,16
	.set	macro
	.set	reorder

	l.s	$f0,32($18)
	li	$6,1879048192			# 0x70000000
	lw	$5,44($18)
	move	$4,$16
	s.s	$f0,48($19)
	ori	$6,$6,0x8c0
	sll	$5,$5,6
	l.s	$f0,36($18)
	addu	$5,$23,$5
	s.s	$f0,52($19)
	l.s	$f1,40($18)
	s.s	$f20,60($19)
	addu	$18,$18,64
	.set	noreorder
	.set	nomacro
	jal	sceVu0MulMatrix
	s.s	$f1,56($19)
	.set	macro
	.set	reorder

	ld	$2,0($16)
	#nop
	sd	$2,0($17)
	ld	$3,8($16)
	#nop
	sd	$3,8($17)
	ld	$2,16($16)
	#nop
	sd	$2,16($17)
	ld	$3,24($16)
	#nop
	sd	$3,24($17)
	ld	$2,32($16)
	#nop
	sd	$2,32($17)
	ld	$3,40($16)
	#nop
	sd	$3,40($17)
	ld	$2,48($16)
	#nop
	sd	$2,48($17)
	ld	$3,56($16)
	addu	$16,$16,64
	sd	$3,56($17)
	lh	$2,78($22)
	#nop
	slt	$2,$21,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L168
	addu	$17,$17,240
	.set	macro
	.set	reorder

$L167:
	ld	$31,128($sp)
	ld	$23,112($sp)
	ld	$22,96($sp)
	ld	$21,80($sp)
	ld	$20,64($sp)
	ld	$19,48($sp)
	ld	$18,32($sp)
	ld	$17,16($sp)
	ld	$16,0($sp)
	l.s	$f20,144($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,160
	.set	macro
	.set	reorder

	.end	JointFrameObjs
	.p2align 3
	.globl	DG_SetObjsMatrix
	.ent	DG_SetObjsMatrix
DG_SetObjsMatrix:
	.frame	$sp,48,$31		# vars= 0, regs= 6/0, args= 0, extra= 0
	.mask	0x80030000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,48
	sd	$16,0($sp)
	sd	$31,32($sp)
	move	$16,$4
	sd	$17,16($sp)
	lw	$4,64($16)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L170
	lh	$17,78($16)
	.set	macro
	.set	reorder

	ld	$2,0($4)
	#nop
	sd	$2,0($16)
	ld	$3,8($4)
	#nop
	sd	$3,8($16)
	ld	$2,16($4)
	#nop
	sd	$2,16($16)
	ld	$3,24($4)
	#nop
	sd	$3,24($16)
	ld	$2,32($4)
	#nop
	sd	$2,32($16)
	ld	$3,40($4)
	#nop
	sd	$3,40($16)
	ld	$2,48($4)
	#nop
	sd	$2,48($16)
	ld	$3,56($4)
	#nop
	sd	$3,56($16)
$L170:
	ld	$4,0($16)
	li	$2,1879048192			# 0x70000000
	ori	$2,$2,0x80
	sd	$4,0($2)
	ld	$3,8($16)
	#nop
	sd	$3,8($2)
	ld	$4,16($16)
	#nop
	sd	$4,16($2)
	ld	$3,24($16)
	#nop
	sd	$3,24($2)
	ld	$4,32($16)
	#nop
	sd	$4,32($2)
	ld	$3,40($16)
	#nop
	sd	$3,40($2)
	ld	$4,48($16)
	#nop
	sd	$4,48($2)
	ld	$3,56($16)
	#nop
	sd	$3,56($2)
	lw	$3,72($16)
	#nop
	andi	$2,$3,0x40
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L171
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	OnePieceObjs
	move	$5,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L179
	ld	$31,32($sp)
	.set	macro
	.set	reorder

	.p2align 2
$L171:
	andi	$2,$3,0x400
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L173
	lw	$2,88($16)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	ScreenObjs2
	move	$5,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L179
	ld	$31,32($sp)
	.set	macro
	.set	reorder

	.p2align 2
$L173:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L175
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	JointFrameObjs
	move	$5,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L180
	move	$4,$16
	.set	macro
	.set	reorder

	.p2align 2
$L175:
	lw	$2,92($16)
	#nop
	beq	$2,$0,$L177
	.set	noreorder
	.set	nomacro
	jal	SlideFrameObjs
	move	$5,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L176
	move	$4,$16
	.set	macro
	.set	reorder

	.p2align 2
$L177:
	move	$5,$17
	ld	$31,32($sp)
	ld	$17,16($sp)
	ld	$16,0($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	OnePieceObjs
	addu	$sp,$sp,48
	.set	macro
	.set	reorder

	.p2align 2
$L176:
$L180:
	.set	noreorder
	.set	nomacro
	jal	ScreenObjs
	move	$5,$17
	.set	macro
	.set	reorder

	ld	$31,32($sp)
$L179:
	ld	$17,16($sp)
	ld	$16,0($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,48
	.set	macro
	.set	reorder

	.end	DG_SetObjsMatrix
	.p2align 3
	.globl	DG_ScreenChanl
	.ent	DG_ScreenChanl
DG_ScreenChanl:
	.frame	$sp,48,$31		# vars= 0, regs= 6/0, args= 0, extra= 0
	.mask	0x80030000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,48
	move	$7,$4
	sd	$31,32($sp)
	sd	$17,16($sp)
	sd	$16,0($sp)
	lw	$8,396($7)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L181
	li	$5,1879048192			# 0x70000000
	.set	macro
	.set	reorder

	ld	$2,0($7)
	li	$4,1879048192			# 0x70000000
	li	$6,1879048192			# 0x70000000
	sd	$2,0($5)
	ori	$4,$4,0x40
	ori	$6,$6,0xa40
	li.s	$f2,5.00000000000000000000e-1
	ld	$3,8($7)
	addu	$17,$8,12
	sd	$3,8($5)
	ld	$2,16($7)
	#nop
	sd	$2,16($5)
	ld	$3,24($7)
	#nop
	sd	$3,24($5)
	ld	$2,32($7)
	#nop
	sd	$2,32($5)
	ld	$3,40($7)
	#nop
	sd	$3,40($5)
	ld	$2,48($7)
	#nop
	sd	$2,48($5)
	ld	$3,56($7)
	#nop
	sd	$3,56($5)
	ld	$2,256($7)
	#nop
	sd	$2,0($4)
	ld	$3,264($7)
	#nop
	sd	$3,8($4)
	ld	$2,272($7)
	#nop
	sd	$2,16($4)
	ld	$3,280($7)
	#nop
	sd	$3,24($4)
	ld	$2,288($7)
	#nop
	sd	$2,32($4)
	ld	$3,296($7)
	#nop
	sd	$3,40($4)
	ld	$2,304($7)
	#nop
	sd	$2,48($4)
	ld	$3,312($7)
	#nop
	sd	$3,56($4)
	lw	$2,408($7)
	#nop
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,1
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	s.s	$f0,0($6)
	lw	$2,412($7)
	lw	$16,8($8)
	srl	$3,$2,31
	addu	$2,$2,$3
	sra	$2,$2,1
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	s.s	$f0,4($6)
	l.s	$f1,384($7)
	#nop
	div.s	$f2,$f2,$f1
	.set	noreorder
	.set	nomacro
	blez	$16,$L181
	s.s	$f2,2640($5)
	.set	macro
	.set	reorder

	lw	$4,0($17)
	.p2align 3
$L189:
	addu	$16,$16,-1
	.set	noreorder
	.set	nomacro
	jal	DG_SetObjsMatrix
	addu	$17,$17,4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bgtzl	$16,$L189
	lw	$4,0($17)
	.set	macro
	.set	reorder

$L181:
	ld	$31,32($sp)
	ld	$17,16($sp)
	ld	$16,0($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,48
	.set	macro
	.set	reorder

	.end	DG_ScreenChanl
