static const	char	vshh_wkms_sl[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11.x, c[ 83 ].z, v0.w "
	"mul r2.xyz, r2.xyz, r11.x "
	"add r6.x, c[81].x, -r11.x "
	"mul r4.xyz, r4.xyz, r11.x "
	"mad r2.xyz, r3.xyz, r6.x, r2.xyz "
	"mad r4.xyz, r5.xyz, r6.x, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r4.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r4.xyz, r5.x, c[44 + 0], r4.xyz "
	"mad r4.xyz, r5.y, c[44 + 1], r4.xyz "
	"mad r4.xyz, r5.z, c[44 + 2], r4.xyz "
	"mov oD0.w, c[ 82 ].x "
	"mul oD0.xyz, r4.xyz, c[83].y "
	"mul r9, v7, c[ 60 ] "
	"add oT0.xy, r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 44
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11.x, c83.z, v0.w
	##  15 B 0.00 ##  mul r2.xyz, r2.xyz, r11.x
	##  16        ##  add r6.x, c81.x, -r11.x
	##  17        ##  mul r4.xyz, r4.xyz, r11.x
	##  18        ##  mad r2.xyz, r3.xyz, r6.x, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6.x, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r4.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r4.xyz, r5.x, c44, r4.xyz
	##  36 B 0.00 ##  mad r4.xyz, r5.y, c45, r4.xyz
	##  37 B 0.00 ##  mad r4.xyz, r5.z, c46, r4.xyz
	##  38        ##  mov oD0.w, c82.x
	##  39        ##  mul oD0.xyz, r4.xyz, c83.y
	##  40        ##  mul r9, v7, c60
	##  41 B 0.00 ##  add oT0.xy, r9, c61
	##  42        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  43 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 44 slots, 24 cycles, cpi = 0.545455
*/

static const	char	vshh_wkms_sp[] =
	"vs.1.1 "
	"mul r0, v0.x, c[ 0 + 0 ] "
	"mad r0, v0.y, c[ 0 + 1 ], r0 "
	"mad r0, v0.z, c[ 0 + 2 ], r0 "
	"add r0, c[ 0 + 3 ], r0 "
	"mov oD0, v3 "
	"mul r1, r0.x, c[ 32 + 0 ] "
	"mad r1, r0.y, c[ 32 + 1 ], r1 "
	"mad r1, r0.z, c[ 32 + 2 ], r1 "
	"mad r1, r0.w, c[ 32 + 3 ], r1 "
	"mul r9, v7, c[ 60 ] "
	"add oT0.xy, r9, c[ 61 ] "
	"mov oPos, r1 "
	"mul r2.x, r1.w, c[ 69 ].x "
	"add oFog.x, c[ 69 ].y, r2.x "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 16
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0, v0.x, c0
	##   1 A 1.50 ##  mad r0, v0.y, c1, r0
	##   2 B 0.00 ##  mad r0, v0.z, c2, r0
	##   3 B 0.00 ##  add r0, c3, r0
	##   4        ##  mov oD0, v3
	##   5        ##  mul r1, r0.x, c32
	##   6 B 0.00 ##  mad r1, r0.y, c33, r1
	##   7 B 0.00 ##  mad r1, r0.z, c34, r1
	##   8 B 0.00 ##  mad r1, r0.w, c35, r1
	##   9        ##  mul r9, v7, c60
	##  10 B 0.00 ##  add oT0.xy, r9, c61
	##  11        ##  mov oPos, r1
	##  12        ##  mul r2.x, r1.w, c69.x
	##  13 B 0.00 ##  add oFog.x, c69.y, r2.x
	##  14        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  15 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 16 slots, 10 cycles, cpi = 0.625
*/

static const	char	vshh_wkms_ml[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"max r5.xyz, r5.xyz, c[80] "
	"mov r6.xyz, c[44 + 3] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul oD0, r6, c[ 84 ] "
	"mul r9,  v7, c[ 60 ] "
	"mul r10, v8, c[ 62 ] "
	"mul r11, v9, c[ 64 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"add oT1.xy, r10, c[ 63 ] "
	"add oT2.xy, r11, c[ 65 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 48
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33 G 0.83 ##  max r5.xyz, r5.xyz, c80
	##  34        ##  mov r6.xyz, c47
	##  35 D 0.40 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul oD0, r6, c84
	##  40        ##  mul r9, v7, c60
	##  41        ##  mul r10, v8, c62
	##  42        ##  mul r11, v9, c64
	##  43        ##  add oT0.xy, r9, c61
	##  44        ##  add oT1.xy, r10, c63
	##  45        ##  add oT2.xy, r11, c65
	##  46        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  47 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 48 slots, 27.23 cycles, cpi = 0.567292
*/

static const	char	vshh_wkms_emap[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul	r7.xyz, r4.x, c[ 48 + 0 ].xyz "
	"mad r7.xyz, r4.y, c[ 48 + 1 ].xyz, r7.xyz "
	"mad r7.xyz, r4.z, c[ 48 + 2 ].xyz, r7.xyz "
	"mul r7.xyz, r7.xyz, c[ 64 ].xyz "
	"mul oD0, r6, c[ 84 ] "
	"add r7.xyz, r7.xyz, c[ 65 ].xyz "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"mul r10, v8, c[ 62 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"add oT1.xy, r10, c[ 63 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 53
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r7.xyz, r4.x, c48.xyz
	##  40 B 0.00 ##  mad r7.xyz, r4.y, c49.xyz, r7.xyz
	##  41 B 0.00 ##  mad r7.xyz, r4.z, c50.xyz, r7.xyz
	##  42 G 0.83 ##  mul r7.xyz, r7.xyz, c64.xyz
	##  43        ##  mul oD0, r6, c84
	##  44 D 0.40 ##  add r7.xyz, r7.xyz, c65.xyz
	##  45        ##  mov oT2.w, r3.w
	##  46        ##  mul oT2.xy, r7.xy, r3.w
	##  47        ##  mul r9, v7, c60
	##  48        ##  mul r10, v8, c62
	##  49        ##  add oT0.xy, r9, c61
	##  50        ##  add oT1.xy, r10, c63
	##  51        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  52 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 53 slots, 29.73 cycles, cpi = 0.560943
*/

static const	char	vshh_wkms_smap[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mov r7.y, c[ 80 ].y "
	"mul	r7.x, r4.x, c[ 40 + 0 ].x "
	"mad r7.x, r4.y, c[ 40 + 1 ].x, r7.x "
	"mad r7.x, r4.z, c[ 40 + 2 ].x, r7.x "
	"mul r7.xy, r7.xy, c[ 64 ].xy "
	"mul oD0, r6, c[ 84 ] "
	"add r7.xy, r7.xy, c[ 65 ].xy "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"mul r10, v8, c[ 62 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"add oT1.xy, r10, c[ 63 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 54
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mov r7.y, c80.y
	##  40        ##  mul r7.x, r4.x, c40.x
	##  41 B 0.00 ##  mad r7.x, r4.y, c41.x, r7.x
	##  42 B 0.00 ##  mad r7.x, r4.z, c42.x, r7.x
	##  43 G 0.83 ##  mul r7.xy, r7.xy, c64.xy
	##  44        ##  mul oD0, r6, c84
	##  45 D 0.40 ##  add r7.xy, r7.xy, c65.xy
	##  46        ##  mov oT2.w, r3.w
	##  47        ##  mul oT2.xy, r7.xy, r3.w
	##  48        ##  mul r9, v7, c60
	##  49        ##  mul r10, v8, c62
	##  50        ##  add oT0.xy, r9, c61
	##  51        ##  add oT1.xy, r10, c63
	##  52        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  53 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 54 slots, 30.23 cycles, cpi = 0.559815
*/

static const	char	vshh_wkms_mlvc[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul r11.xyzw, v3, c[ 84 ] "
	"mul oD0, r6, r11 "
	"mul r9,  v7, c[ 60 ] "
	"mul r10, v8, c[ 62 ] "
	"mul r11, v9, c[ 64 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"add oT1.xy, r10, c[ 63 ] "
	"add oT2.xy, r11, c[ 65 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 49
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r11, v3, c84
	##  40 B 0.00 ##  mul oD0, r6, r11
	##  41        ##  mul r9, v7, c60
	##  42        ##  mul r10, v8, c62
	##  43        ##  mul r11, v9, c64
	##  44        ##  add oT0.xy, r9, c61
	##  45        ##  add oT1.xy, r10, c63
	##  46        ##  add oT2.xy, r11, c65
	##  47        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  48 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 49 slots, 26.5 cycles, cpi = 0.540816
*/

static const	char	vshh_wkms_emapvc[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul	r7.xyz, r4.x, c[ 48 + 0 ].xyz "
	"mad r7.xyz, r4.y, c[ 48 + 1 ].xyz, r7.xyz "
	"mad r7.xyz, r4.z, c[ 48 + 2 ].xyz, r7.xyz "
	"mul r7.xyz, r7.xyz, c[ 64 ].xyz "
	"mul r11.xyzw, v3, c[ 84 ] "
	"add r7.xyz, r7.xyz, c[ 65 ].xyz "
	"mul oD0, r6, r11 "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"mul r10, v8, c[ 62 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"add oT1.xy, r10, c[ 63 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 54
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r7.xyz, r4.x, c48.xyz
	##  40 B 0.00 ##  mad r7.xyz, r4.y, c49.xyz, r7.xyz
	##  41 B 0.00 ##  mad r7.xyz, r4.z, c50.xyz, r7.xyz
	##  42 G 0.83 ##  mul r7.xyz, r7.xyz, c64.xyz
	##  43        ##  mul r11, v3, c84
	##  44 D 0.40 ##  add r7.xyz, r7.xyz, c65.xyz
	##  45        ##  mul oD0, r6, r11
	##  46        ##  mov oT2.w, r3.w
	##  47        ##  mul oT2.xy, r7.xy, r3.w
	##  48        ##  mul r9, v7, c60
	##  49        ##  mul r10, v8, c62
	##  50        ##  add oT0.xy, r9, c61
	##  51        ##  add oT1.xy, r10, c63
	##  52        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  53 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 54 slots, 30.23 cycles, cpi = 0.559815
*/

static const	char	vshh_wkms_smapvc[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mov r7.y, c[ 80 ].y "
	"mul	r7.x, r4.x, c[ 40 + 0 ].x "
	"mad r7.x, r4.y, c[ 40 + 1 ].x, r7.x "
	"mad r7.x, r4.z, c[ 40 + 2 ].x, r7.x "
	"mul r11.xyzw, v3, c[ 84 ] "
	"mul r7.xy, r7.xy, c[ 64 ].xy "
	"mul oD0, r6, r11 "
	"add r7.xy, r7.xy, c[ 65 ].xy "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"mul r10, v8, c[ 62 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"add oT1.xy, r10, c[ 63 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 55
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mov r7.y, c80.y
	##  40        ##  mul r7.x, r4.x, c40.x
	##  41 B 0.00 ##  mad r7.x, r4.y, c41.x, r7.x
	##  42 B 0.00 ##  mad r7.x, r4.z, c42.x, r7.x
	##  43        ##  mul r11, v3, c84
	##  44        ##  mul r7.xy, r7.xy, c64.xy
	##  45        ##  mul oD0, r6, r11
	##  46        ##  add r7.xy, r7.xy, c65.xy
	##  47        ##  mov oT2.w, r3.w
	##  48        ##  mul oT2.xy, r7.xy, r3.w
	##  49        ##  mul r9, v7, c60
	##  50        ##  mul r10, v8, c62
	##  51        ##  add oT0.xy, r9, c61
	##  52        ##  add oT1.xy, r10, c63
	##  53        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  54 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 55 slots, 29.5 cycles, cpi = 0.536364
*/

static const	char	vshh_wkms_ml_1tex[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul oD0, r6, c[ 84 ] "
	"mul r9,  v7, c[ 60 ] "
	"add oT0,  r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 44
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul oD0, r6, c84
	##  40        ##  mul r9, v7, c60
	##  41 B 0.00 ##  add oT0, r9, c61
	##  42        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  43 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 44 slots, 24 cycles, cpi = 0.545455
*/

static const	char	vshh_wkms_emap_1tex[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul oD0, r6, c[ 84 ] "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 44
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul oD0, r6, c84
	##  40        ##  mul r9, v7, c60
	##  41 B 0.00 ##  add oT0.xy, r9, c61
	##  42        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  43 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 44 slots, 24 cycles, cpi = 0.545455
*/

static const	char	vshh_wkms_smap_1tex[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul oD0, r6, c[ 84 ] "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 44
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul oD0, r6, c84
	##  40        ##  mul r9, v7, c60
	##  41 B 0.00 ##  add oT0.xy, r9, c61
	##  42        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  43 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 44 slots, 24 cycles, cpi = 0.545455
*/

static const	char	vshh_wkms_mlvc_1tex[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul r11.xyzw, v3, c[ 84 ] "
	"mul oD0, r6, r11 "
	"mul r9,  v7, c[ 60 ] "
	"add oT0,  r9.xy, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 45
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r11, v3, c84
	##  40 B 0.00 ##  mul oD0, r6, r11
	##  41        ##  mul r9, v7, c60
	##  42 B 0.00 ##  add oT0, r9.xy, c61
	##  43        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  44 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 45 slots, 24.5 cycles, cpi = 0.544444
*/

static const	char	vshh_wkms_emapvc_1tex[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul r11.xyzw, v3, c[ 84 ] "
	"mul oD0, r6, r11 "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 45
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r11, v3, c84
	##  40 B 0.00 ##  mul oD0, r6, r11
	##  41        ##  mul r9, v7, c60
	##  42 B 0.00 ##  add oT0.xy, r9, c61
	##  43        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  44 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 45 slots, 24.5 cycles, cpi = 0.544444
*/

static const	char	vshh_wkms_smapvc_1tex[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul r11.xyzw, v3, c[ 84 ] "
	"mul oD0, r6, r11 "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 45
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r11, v3, c84
	##  40 B 0.00 ##  mul oD0, r6, r11
	##  41        ##  mul r9, v7, c60
	##  42 B 0.00 ##  add oT0.xy, r9, c61
	##  43        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  44 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 45 slots, 24.5 cycles, cpi = 0.544444
*/

static const	char	vshh_wkms_ml_kmss[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"max r5.xyz, r5.xyz, c[80] "
	"mov r6.xyz, c[44 + 3] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul oD0, r6, c[ 84 ] "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"mov oT1.xy, c[ 80 ] "
	"mov oT2.xy, c[ 80 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 46
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33 G 0.83 ##  max r5.xyz, r5.xyz, c80
	##  34        ##  mov r6.xyz, c47
	##  35 D 0.40 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul oD0, r6, c84
	##  40        ##  mul r9, v7, c60
	##  41 B 0.00 ##  add oT0.xy, r9, c61
	##  42        ##  mov oT1.xy, c80
	##  43        ##  mov oT2.xy, c80
	##  44        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  45 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 46 slots, 26.23 cycles, cpi = 0.570217
*/

static const	char	vshh_wkms_emap_kmss[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul	r7.xyz, r4.x, c[ 48 + 0 ].xyz "
	"mad r7.xyz, r4.y, c[ 48 + 1 ].xyz, r7.xyz "
	"mad r7.xyz, r4.z, c[ 48 + 2 ].xyz, r7.xyz "
	"mul r7.xyz, r7.xyz, c[ 64 ].xyz "
	"mul oD0, r6, c[ 84 ] "
	"add r7.xyz, r7.xyz, c[ 65 ].xyz "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"mov oT1.xy, c[ 80 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 52
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r7.xyz, r4.x, c48.xyz
	##  40 B 0.00 ##  mad r7.xyz, r4.y, c49.xyz, r7.xyz
	##  41 B 0.00 ##  mad r7.xyz, r4.z, c50.xyz, r7.xyz
	##  42 G 0.83 ##  mul r7.xyz, r7.xyz, c64.xyz
	##  43        ##  mul oD0, r6, c84
	##  44 D 0.40 ##  add r7.xyz, r7.xyz, c65.xyz
	##  45        ##  mov oT2.w, r3.w
	##  46        ##  mul oT2.xy, r7.xy, r3.w
	##  47        ##  mul r9, v7, c60
	##  48 B 0.00 ##  add oT0.xy, r9, c61
	##  49        ##  mov oT1.xy, c80
	##  50        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  51 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 52 slots, 29.23 cycles, cpi = 0.562115
*/

static const	char	vshh_wkms_smap_kmss[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mov r7.y, c[ 80 ].y "
	"mul	r7.x, r4.x, c[ 40 + 0 ].x "
	"mad r7.x, r4.y, c[ 40 + 1 ].x, r7.x "
	"mad r7.x, r4.z, c[ 40 + 2 ].x, r7.x "
	"mul r7.xy, r7.xy, c[ 64 ].xy "
	"mul oD0, r6, c[ 84 ] "
	"add r7.xy, r7.xy, c[ 65 ].xy "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"mov oT1.xy, c[ 80 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 53
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mov r7.y, c80.y
	##  40        ##  mul r7.x, r4.x, c40.x
	##  41 B 0.00 ##  mad r7.x, r4.y, c41.x, r7.x
	##  42 B 0.00 ##  mad r7.x, r4.z, c42.x, r7.x
	##  43 G 0.83 ##  mul r7.xy, r7.xy, c64.xy
	##  44        ##  mul oD0, r6, c84
	##  45 D 0.40 ##  add r7.xy, r7.xy, c65.xy
	##  46        ##  mov oT2.w, r3.w
	##  47        ##  mul oT2.xy, r7.xy, r3.w
	##  48        ##  mul r9, v7, c60
	##  49 B 0.00 ##  add oT0.xy, r9, c61
	##  50        ##  mov oT1.xy, c80
	##  51        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  52 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 53 slots, 29.73 cycles, cpi = 0.560943
*/

static const	char	vshh_wkms_mlvc_kmss[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul r11.xyzw, v3, c[ 84 ] "
	"mul oD0, r6, r11 "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"mov oT1.xy, c[ 80 ] "
	"mov oT2.xy, c[ 80 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 47
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r11, v3, c84
	##  40 B 0.00 ##  mul oD0, r6, r11
	##  41        ##  mul r9, v7, c60
	##  42 B 0.00 ##  add oT0.xy, r9, c61
	##  43        ##  mov oT1.xy, c80
	##  44        ##  mov oT2.xy, c80
	##  45        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  46 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 47 slots, 25.5 cycles, cpi = 0.542553
*/

static const	char	vshh_wkms_emapvc_kmss[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mul	r7.xyz, r4.x, c[ 48 + 0 ].xyz "
	"mad r7.xyz, r4.y, c[ 48 + 1 ].xyz, r7.xyz "
	"mad r7.xyz, r4.z, c[ 48 + 2 ].xyz, r7.xyz "
	"mul r7.xyz, r7.xyz, c[ 64 ].xyz "
	"mul r11.xyzw, v3, c[ 84 ] "
	"add r7.xyz, r7.xyz, c[ 65 ].xyz "
	"mul oD0, r6, r11 "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"mov oT1.xy, c[ 80 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 53
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mul r7.xyz, r4.x, c48.xyz
	##  40 B 0.00 ##  mad r7.xyz, r4.y, c49.xyz, r7.xyz
	##  41 B 0.00 ##  mad r7.xyz, r4.z, c50.xyz, r7.xyz
	##  42 G 0.83 ##  mul r7.xyz, r7.xyz, c64.xyz
	##  43        ##  mul r11, v3, c84
	##  44 D 0.40 ##  add r7.xyz, r7.xyz, c65.xyz
	##  45        ##  mul oD0, r6, r11
	##  46        ##  mov oT2.w, r3.w
	##  47        ##  mul oT2.xy, r7.xy, r3.w
	##  48        ##  mul r9, v7, c60
	##  49 B 0.00 ##  add oT0.xy, r9, c61
	##  50        ##  mov oT1.xy, c80
	##  51        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  52 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 53 slots, 29.73 cycles, cpi = 0.560943
*/

static const	char	vshh_wkms_smapvc_kmss[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11, c[ 83 ].z, v0.w "
	"add r6, c[81].x, -r11 "
	"mul r2.xyz, r2.xyz, r11 "
	"mul r4.xyz, r4.xyz, r11 "
	"mad r2.xyz, r3.xyz, r6, r2.xyz "
	"mad r4.xyz, r5.xyz, r6, r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r0.x "
	"mul r2.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov r6.xyz, c[44 + 3] "
	"max r5.xyz, r5.xyz, c[80] "
	"mad r6.xyz, r5.x, c[44 + 0], r6.xyz "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"mov r6.w, c[ 82 ].x "
	"mov r7.y, c[ 80 ].y "
	"mul	r7.x, r4.x, c[ 40 + 0 ].x "
	"mad r7.x, r4.y, c[ 40 + 1 ].x, r7.x "
	"mad r7.x, r4.z, c[ 40 + 2 ].x, r7.x "
	"mul r11.xyzw, v3, c[ 84 ] "
	"mul r7.xy, r7.xy, c[ 64 ].xy "
	"mul oD0, r6, r11 "
	"add r7.xy, r7.xy, c[ 65 ].xy "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"mov oT1.xy, c[ 80 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 54
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, v0.x, c4
	##   8 B 0.00 ##  mad r3, v0.y, c5, r3
	##   9 B 0.00 ##  mad r3, v0.z, c6, r3
	##  10 B 0.00 ##  add r3, c7, r3
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11, c83.z, v0.w
	##  15 B 0.00 ##  add r6, c81.x, -r11
	##  16        ##  mul r2.xyz, r2.xyz, r11
	##  17        ##  mul r4.xyz, r4.xyz, r11
	##  18        ##  mad r2.xyz, r3.xyz, r6, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r0.x, r4, r4
	##  25        ##  mov oPos, r3
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mul r2.x, r3.w, c69.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  add oFog.x, c69.y, r2.x
	##  30        ##  mul r5.xyz, r4.x, c40.xyz
	##  31 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  32 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  33        ##  mov r6.xyz, c47
	##  34        ##  max r5.xyz, r5.xyz, c80
	##  35 B 0.00 ##  mad r6.xyz, r5.x, c44, r6.xyz
	##  36 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  37 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  38        ##  mov r6.w, c82.x
	##  39        ##  mov r7.y, c80.y
	##  40        ##  mul r7.x, r4.x, c40.x
	##  41 B 0.00 ##  mad r7.x, r4.y, c41.x, r7.x
	##  42 B 0.00 ##  mad r7.x, r4.z, c42.x, r7.x
	##  43        ##  mul r11, v3, c84
	##  44        ##  mul r7.xy, r7.xy, c64.xy
	##  45        ##  mul oD0, r6, r11
	##  46        ##  add r7.xy, r7.xy, c65.xy
	##  47        ##  mov oT2.w, r3.w
	##  48        ##  mul oT2.xy, r7.xy, r3.w
	##  49        ##  mul r9, v7, c60
	##  50 B 0.00 ##  add oT0.xy, r9, c61
	##  51        ##  mov oT1.xy, c80
	##  52        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  53 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 54 slots, 29 cycles, cpi = 0.537037
*/

static const	char	vshh_wevm[] =
	"vs.1.1 "
	"mul r0.xyz, v0.xyz, c[ 83 ].z "
	"mov r0.w, v0.w "
	"mov a0.x, v12.x "
	"mul r2, r0.x, c[a0.x + 0] "
	"mad r2, r0.y, c[a0.x + 1], r2 "
	"mad r2, r0.z, c[a0.x + 2], r2 "
	"mad r2, r0.w, c[a0.x + 3], r2 "
	"mul r4.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r4.xyz, v2.y, c[a0.x + 1].xyz, r4 "
	"mad r4.xyz, v2.z, c[a0.x + 2].xyz, r4 "
	"mul r2.xyz, r2.xyz, v11.x "
	"mul r4.xyz, r4.xyz, v11.x "
	"mov a0.x, v12.y "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.y, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.y, r4.xyz "
	"mov a0.x, v12.z "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.z, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.z, r4.xyz "
	"mov a0.x, v12.w "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.w, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.w, r4.xyz "
	"mul r2.xyz, r2.xyz, c[ 83 ].w "
	"mul r4.xyz, r4.xyz, c[ 83 ].w "
	"mul r3, r2.x, c[32 + 0] "
	"mad r3, r2.y, c[32 + 1], r3 "
	"mad r3, r2.z, c[32 + 2], r3 "
	"mad r3, r2.w, c[32 + 3], r3 "
	"dp3 r5.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r5.x, r5.x "
	"mul r6.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r5.x "
	"add oFog.x, c[ 69 ].y, r6.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov oD0.w, c[ 82 ].x "
	"max r5.xyz, r5.xyz, c[80] "
	"mul r4.xyz, r5.x, c[44 + 0] "
	"mad r4.xyz, r5.y, c[44 + 1], r4.xyz "
	"mad r4.xyz, r5.z, c[44 + 2], r4.xyz "
	"add r4.xyz, c[44 + 3], r4.xyz "
	"mul r9,  v7, c[ 60 ] "
	"mul r10, v8, c[ 62 ] "
	"mul r11, v9, c[ 64 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"add oT1.xy, r10, c[ 63 ] "
	"add oT2.xy, r11, c[ 65 ] "
	"mul oD0.xyz, r4.xyz, c[83].y "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 72
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0.xyz, v0.xyz, c83.z
	##   1        ##  mov r0.w, v0.w
	##   2        ##  mov a0.x, v12.x
	##   3 H 0.17 ##  mul r2, r0.x, c[a0.x]
	##   4 B 0.00 ##  mad r2, r0.y, c[a0.x+1], r2
	##   5 B 0.00 ##  mad r2, r0.z, c[a0.x+2], r2
	##   6 B 0.00 ##  mad r2, r0.w, c[a0.x+3], r2
	##   7        ##  mul r4.xyz, v2.x, c[a0.x].xyz
	##   8 B 0.00 ##  mad r4.xyz, v2.y, c[a0.x+1].xyz, r4
	##   9 B 0.00 ##  mad r4.xyz, v2.z, c[a0.x+2].xyz, r4
	##  10        ##  mul r2.xyz, r2.xyz, v11.x
	##  11        ##  mul r4.xyz, r4.xyz, v11.x
	##  12        ##  mov a0.x, v12.y
	##  13 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  14 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  15 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  16 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  17        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  18 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  19 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  20        ##  mad r2.xyz, r3.xyz, v11.y, r2.xyz
	##  21        ##  mad r4.xyz, r5.xyz, v11.y, r4.xyz
	##  22        ##  mov a0.x, v12.z
	##  23 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  24 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  25 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  26 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  27        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  28 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  29 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  30        ##  mad r2.xyz, r3.xyz, v11.z, r2.xyz
	##  31        ##  mad r4.xyz, r5.xyz, v11.z, r4.xyz
	##  32        ##  mov a0.x, v12.w
	##  33 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  34 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  35 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  36 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  37        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  38 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  39 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  40        ##  mad r2.xyz, r3.xyz, v11.w, r2.xyz
	##  41        ##  mad r4.xyz, r5.xyz, v11.w, r4.xyz
	##  42        ##  mul r2.xyz, r2.xyz, c83.w
	##  43        ##  mul r4.xyz, r4.xyz, c83.w
	##  44        ##  mul r3, r2.x, c32
	##  45 B 0.00 ##  mad r3, r2.y, c33, r3
	##  46 B 0.00 ##  mad r3, r2.z, c34, r3
	##  47 B 0.00 ##  mad r3, r2.w, c35, r3
	##  48        ##  dp3 r5.x, r4, r4
	##  49        ##  mov oPos, r3
	##  50        ##  rsq r5.x, r5.x
	##  51        ##  mul r6.x, r3.w, c69.x
	##  52        ##  mul r4.xyz, r4.xyz, r5.x
	##  53        ##  add oFog.x, c69.y, r6.x
	##  54        ##  mul r5.xyz, r4.x, c40.xyz
	##  55 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  56 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  57        ##  mov oD0.w, c82.x
	##  58        ##  max r5.xyz, r5.xyz, c80
	##  59 B 0.00 ##  mul r4.xyz, r5.x, c44
	##  60 B 0.00 ##  mad r4.xyz, r5.y, c45, r4.xyz
	##  61 B 0.00 ##  mad r4.xyz, r5.z, c46, r4.xyz
	##  62 B 0.00 ##  add r4.xyz, c47, r4.xyz
	##  63        ##  mul r9, v7, c60
	##  64        ##  mul r10, v8, c62
	##  65        ##  mul r11, v9, c64
	##  66        ##  add oT0.xy, r9, c61
	##  67        ##  add oT1.xy, r10, c63
	##  68        ##  add oT2.xy, r11, c65
	##  69        ##  mul oD0.xyz, r4.xyz, c83.y
	##  70        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  71 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	H: stall from using a0.x the instruction after writing to it
	//    microcode: 72 slots, 37.18 cycles, cpi = 0.516389
*/

static const	char	vshh_wevm_emap[] =
	"vs.1.1 "
	"mul r0.xyz, v0.xyz, c[ 83 ].z "
	"mov r0.w, v0.w "
	"mov a0.x, v12.x "
	"mul r2, r0.x, c[a0.x + 0] "
	"mad r2, r0.y, c[a0.x + 1], r2 "
	"mad r2, r0.z, c[a0.x + 2], r2 "
	"mad r2, r0.w, c[a0.x + 3], r2 "
	"mul r4.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r4.xyz, v2.y, c[a0.x + 1].xyz, r4 "
	"mad r4.xyz, v2.z, c[a0.x + 2].xyz, r4 "
	"mul r2.xyz, r2.xyz, v11.x "
	"mul r4.xyz, r4.xyz, v11.x "
	"mov a0.x, v12.y "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.y, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.y, r4.xyz "
	"mov a0.x, v12.z "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.z, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.z, r4.xyz "
	"mov a0.x, v12.w "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.w, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.w, r4.xyz "
	"mul r2.xyz, r2.xyz, c[ 83 ].w "
	"mul r4.xyz, r4.xyz, c[ 83 ].w "
	"mul r3, r2.x, c[32 + 0] "
	"mad r3, r2.y, c[32 + 1], r3 "
	"mad r3, r2.z, c[32 + 2], r3 "
	"mad r3, r2.w, c[32 + 3], r3 "
	"dp3 r5.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r5.x, r5.x "
	"mul r6.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r5.x "
	"add oFog.x, c[ 69 ].y, r6.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov oD0.w, c[ 82 ].x "
	"max r5.xyz, r5.xyz, c[80] "
	"mul r6.xyz, r5.x, c[44 + 0] "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"add r6.xyz, c[44 + 3], r6.xyz "
	"mul	r7.xyz, r4.x, c[ 48 + 0 ].xyz "
	"mad r7.xyz, r4.y, c[ 48 + 1 ].xyz, r7.xyz "
	"mad r7.xyz, r4.z, c[ 48 + 2 ].xyz, r7.xyz "
	"mul oD0.xyz, r6.xyz, c[83].y "
	"mul r7.xyz, r7.xyz, c[ 64 ].xyz "
	"add r7.xyz, r7.xyz, c[ 65 ].xyz "
	"mov oT2.w, r3.w "
	"mul oT2.xy, r7.xy, r3.w "
	"mul r9,  v7, c[ 60 ] "
	"mul r10, v8, c[ 62 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"add oT1.xy, r10, c[ 63 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 77
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0.xyz, v0.xyz, c83.z
	##   1        ##  mov r0.w, v0.w
	##   2        ##  mov a0.x, v12.x
	##   3 H 0.17 ##  mul r2, r0.x, c[a0.x]
	##   4 B 0.00 ##  mad r2, r0.y, c[a0.x+1], r2
	##   5 B 0.00 ##  mad r2, r0.z, c[a0.x+2], r2
	##   6 B 0.00 ##  mad r2, r0.w, c[a0.x+3], r2
	##   7        ##  mul r4.xyz, v2.x, c[a0.x].xyz
	##   8 B 0.00 ##  mad r4.xyz, v2.y, c[a0.x+1].xyz, r4
	##   9 B 0.00 ##  mad r4.xyz, v2.z, c[a0.x+2].xyz, r4
	##  10        ##  mul r2.xyz, r2.xyz, v11.x
	##  11        ##  mul r4.xyz, r4.xyz, v11.x
	##  12        ##  mov a0.x, v12.y
	##  13 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  14 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  15 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  16 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  17        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  18 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  19 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  20        ##  mad r2.xyz, r3.xyz, v11.y, r2.xyz
	##  21        ##  mad r4.xyz, r5.xyz, v11.y, r4.xyz
	##  22        ##  mov a0.x, v12.z
	##  23 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  24 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  25 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  26 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  27        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  28 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  29 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  30        ##  mad r2.xyz, r3.xyz, v11.z, r2.xyz
	##  31        ##  mad r4.xyz, r5.xyz, v11.z, r4.xyz
	##  32        ##  mov a0.x, v12.w
	##  33 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  34 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  35 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  36 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  37        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  38 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  39 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  40        ##  mad r2.xyz, r3.xyz, v11.w, r2.xyz
	##  41        ##  mad r4.xyz, r5.xyz, v11.w, r4.xyz
	##  42        ##  mul r2.xyz, r2.xyz, c83.w
	##  43        ##  mul r4.xyz, r4.xyz, c83.w
	##  44        ##  mul r3, r2.x, c32
	##  45 B 0.00 ##  mad r3, r2.y, c33, r3
	##  46 B 0.00 ##  mad r3, r2.z, c34, r3
	##  47 B 0.00 ##  mad r3, r2.w, c35, r3
	##  48        ##  dp3 r5.x, r4, r4
	##  49        ##  mov oPos, r3
	##  50        ##  rsq r5.x, r5.x
	##  51        ##  mul r6.x, r3.w, c69.x
	##  52        ##  mul r4.xyz, r4.xyz, r5.x
	##  53        ##  add oFog.x, c69.y, r6.x
	##  54        ##  mul r5.xyz, r4.x, c40.xyz
	##  55 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  56 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  57        ##  mov oD0.w, c82.x
	##  58        ##  max r5.xyz, r5.xyz, c80
	##  59 B 0.00 ##  mul r6.xyz, r5.x, c44
	##  60 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  61 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  62 B 0.00 ##  add r6.xyz, c47, r6.xyz
	##  63        ##  mul r7.xyz, r4.x, c48.xyz
	##  64 B 0.00 ##  mad r7.xyz, r4.y, c49.xyz, r7.xyz
	##  65 B 0.00 ##  mad r7.xyz, r4.z, c50.xyz, r7.xyz
	##  66        ##  mul oD0.xyz, r6.xyz, c83.y
	##  67        ##  mul r7.xyz, r7.xyz, c64.xyz
	##  68 B 0.00 ##  add r7.xyz, r7.xyz, c65.xyz
	##  69        ##  mov oT2.w, r3.w
	##  70        ##  mul oT2.xy, r7.xy, r3.w
	##  71        ##  mul r9, v7, c60
	##  72        ##  mul r10, v8, c62
	##  73        ##  add oT0.xy, r9, c61
	##  74        ##  add oT1.xy, r10, c63
	##  75        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  76 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	H: stall from using a0.x the instruction after writing to it
	//    microcode: 77 slots, 39.68 cycles, cpi = 0.515325
*/

static const	char	vshh_wevm_bmap[] =
	"vs.1.1 "
	"mul r0.xyz, v0.xyz, c[ 83 ].z "
	"mov r0.w, v0.w "
	"mov a0.x, v12.x "
	"mul r2, r0.x, c[a0.x + 0] "
	"mad r2, r0.y, c[a0.x + 1], r2 "
	"mad r2, r0.z, c[a0.x + 2], r2 "
	"mad r2, r0.w, c[a0.x + 3], r2 "
	"mul r4.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r4.xyz, v2.y, c[a0.x + 1].xyz, r4 "
	"mad r4.xyz, v2.z, c[a0.x + 2].xyz, r4 "
	"mul r8.xyz, v9.x, c[a0.x + 0].xyz "
	"mad r8.xyz, v9.y, c[a0.x + 1].xyz, r8 "
	"mad r8.xyz, v9.z, c[a0.x + 2].xyz, r8 "
	"mul r2.xyz, r2.xyz, v11.x "
	"mul r4.xyz, r4.xyz, v11.x "
	"mul r8.xyz, r8.xyz, v11.x "
	"mov a0.x, v12.y "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mul r9.xyz, v9.x, c[a0.x + 0].xyz "
	"mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9 "
	"mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9 "
	"mad r2.xyz, r3.xyz, v11.y, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.y, r4.xyz "
	"mad r8.xyz, r9.xyz, v11.y, r8.xyz "
	"mov a0.x, v12.z "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mul r9.xyz, v9.x, c[a0.x + 0].xyz "
	"mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9 "
	"mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9 "
	"mad r2.xyz, r3.xyz, v11.z, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.z, r4.xyz "
	"mad r8.xyz, r9.xyz, v11.z, r8.xyz "
	"mov a0.x, v12.w "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mul r9.xyz, v9.x, c[a0.x + 0].xyz "
	"mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9 "
	"mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9 "
	"mad r2.xyz, r3.xyz, v11.w, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.w, r4.xyz "
	"mad r8.xyz, r9.xyz, v11.w, r8.xyz "
	"mul r2.xyz, r2.xyz, c[ 83 ].w "
	"mul r4.xyz, r4.xyz, c[ 83 ].w "
	"mul r8.xyz, r8.xyz, c[ 83 ].w "
	"mul r3, r2.x, c[32 + 0] "
	"mad r3, r2.y, c[32 + 1], r3 "
	"mad r3, r2.z, c[32 + 2], r3 "
	"mad r3, r2.w, c[32 + 3], r3 "
	"dp3 r4.w, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r4.w "
	"mul r6.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r6.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov oD0.w, c[ 82 ].x "
	"max r5.xyz, r5.xyz, c[80] "
	"mul r6.xyz, r5.x, c[44 + 0] "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"add r6.xyz, c[44 + 3], r6.xyz "
	"dp3 r8.w, r8, r8 "
	"mul oD0.xyz, r6.xyz, c[83].y "
	"rsq r1.x, r8.w "
	"mul r8.xyz, r8.xyz, r1.x "
	"mul r7.xyz, r4.yzx, r8.zxy "
	"mad r7.xyz, -r4.zxy, r8.yzx, r7 "
	"dp3 r8.x, r8.xyz, c[ 84 ].xyz "
	"dp3 r8.y, r7.xyz, c[ 84 ].xyz "
	"mul r8.xy, r8.xy, c[ 64 ].xy "
	"mad r8.xy, r8.xy, c[ 62 ].zw, v8 "
	"min r8.xy, r8.xy, c[ 64 ].xy "
	"max r8.xy, r8.xy, c[ 80 ] "
	"mul r9.xy,  v7.xy, c[ 60 ] "
	"mul r10.xy, v8.xy, c[ 62 ] "
	"mul r11.xy, r8.xy, c[ 62 ] "
	"add oT0.xy,  r9.xy, c[ 61 ] "
	"add oT1.xy, r10.xy, c[ 63 ] "
	"add oT2.xy, r11.xy, c[ 63 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 100
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0.xyz, v0.xyz, c83.z
	##   1        ##  mov r0.w, v0.w
	##   2        ##  mov a0.x, v12.x
	##   3 H 0.17 ##  mul r2, r0.x, c[a0.x]
	##   4 B 0.00 ##  mad r2, r0.y, c[a0.x+1], r2
	##   5 B 0.00 ##  mad r2, r0.z, c[a0.x+2], r2
	##   6 B 0.00 ##  mad r2, r0.w, c[a0.x+3], r2
	##   7        ##  mul r4.xyz, v2.x, c[a0.x].xyz
	##   8 B 0.00 ##  mad r4.xyz, v2.y, c[a0.x+1].xyz, r4
	##   9 B 0.00 ##  mad r4.xyz, v2.z, c[a0.x+2].xyz, r4
	##  10        ##  mul r8.xyz, v9.x, c[a0.x].xyz
	##  11 B 0.00 ##  mad r8.xyz, v9.y, c[a0.x+1].xyz, r8
	##  12 B 0.00 ##  mad r8.xyz, v9.z, c[a0.x+2].xyz, r8
	##  13        ##  mul r2.xyz, r2.xyz, v11.x
	##  14        ##  mul r4.xyz, r4.xyz, v11.x
	##  15        ##  mul r8.xyz, r8.xyz, v11.x
	##  16        ##  mov a0.x, v12.y
	##  17 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  18 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  19 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  20 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  21        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  22 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  23 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  24        ##  mul r9.xyz, v9.x, c[a0.x].xyz
	##  25 B 0.00 ##  mad r9.xyz, v9.y, c[a0.x+1].xyz, r9
	##  26 B 0.00 ##  mad r9.xyz, v9.z, c[a0.x+2].xyz, r9
	##  27        ##  mad r2.xyz, r3.xyz, v11.y, r2.xyz
	##  28        ##  mad r4.xyz, r5.xyz, v11.y, r4.xyz
	##  29        ##  mad r8.xyz, r9.xyz, v11.y, r8.xyz
	##  30        ##  mov a0.x, v12.z
	##  31 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  32 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  33 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  34 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  35        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  36 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  37 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  38        ##  mul r9.xyz, v9.x, c[a0.x].xyz
	##  39 B 0.00 ##  mad r9.xyz, v9.y, c[a0.x+1].xyz, r9
	##  40 B 0.00 ##  mad r9.xyz, v9.z, c[a0.x+2].xyz, r9
	##  41        ##  mad r2.xyz, r3.xyz, v11.z, r2.xyz
	##  42        ##  mad r4.xyz, r5.xyz, v11.z, r4.xyz
	##  43        ##  mad r8.xyz, r9.xyz, v11.z, r8.xyz
	##  44        ##  mov a0.x, v12.w
	##  45 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  46 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  47 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  48 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  49        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  50 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  51 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  52        ##  mul r9.xyz, v9.x, c[a0.x].xyz
	##  53 B 0.00 ##  mad r9.xyz, v9.y, c[a0.x+1].xyz, r9
	##  54 B 0.00 ##  mad r9.xyz, v9.z, c[a0.x+2].xyz, r9
	##  55        ##  mad r2.xyz, r3.xyz, v11.w, r2.xyz
	##  56        ##  mad r4.xyz, r5.xyz, v11.w, r4.xyz
	##  57        ##  mad r8.xyz, r9.xyz, v11.w, r8.xyz
	##  58        ##  mul r2.xyz, r2.xyz, c83.w
	##  59        ##  mul r4.xyz, r4.xyz, c83.w
	##  60        ##  mul r8.xyz, r8.xyz, c83.w
	##  61        ##  mul r3, r2.x, c32
	##  62 B 0.00 ##  mad r3, r2.y, c33, r3
	##  63 B 0.00 ##  mad r3, r2.z, c34, r3
	##  64 B 0.00 ##  mad r3, r2.w, c35, r3
	##  65        ##  dp3 r4.w, r4, r4
	##  66        ##  mov oPos, r3
	##  67        ##  rsq r1.x, r4.w
	##  68        ##  mul r6.x, r3.w, c69.x
	##  69        ##  mul r4.xyz, r4.xyz, r1.x
	##  70        ##  add oFog.x, c69.y, r6.x
	##  71        ##  mul r5.xyz, r4.x, c40.xyz
	##  72 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  73 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  74        ##  mov oD0.w, c82.x
	##  75        ##  max r5.xyz, r5.xyz, c80
	##  76 B 0.00 ##  mul r6.xyz, r5.x, c44
	##  77 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  78 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  79 B 0.00 ##  add r6.xyz, c47, r6.xyz
	##  80        ##  dp3 r8.w, r8, r8
	##  81        ##  mul oD0.xyz, r6.xyz, c83.y
	##  82        ##  rsq r1.x, r8.w
	##  83 C 0.50 ##  mul r8.xyz, r8.xyz, r1.x
	##  84 F 0.50 ##  mul r7.xyz, r4.yzx, r8.zxy
	##  85 B 0.00 ##  mad r7.xyz, -r4.zxy, r8.yzx, r7
	##  86        ##  dp3 r8.x, r8.xyz, c84.xyz
	##  87        ##  dp3 r8.y, r7.xyz, c84.xyz
	##  88 C 0.50 ##  mul r8.xy, r8.xy, c64.xy
	##  89 F 0.50 ##  mad r8.xy, r8.xy, c62.zw, v8
	##  90 G 0.83 ##  min r8.xy, r8.xy, c64.xy
	##  91 F 0.50 ##  max r8.xy, r8.xy, c80
	##  92        ##  mul r9.xy, v7.xy, c60
	##  93        ##  mul r10.xy, v8.xy, c62
	##  94        ##  mul r11.xy, r8.xy, c62
	##  95        ##  add oT0.xy, r9.xy, c61
	##  96        ##  add oT1.xy, r10.xy, c63
	##  97        ##  add oT2.xy, r11.xy, c63
	##  98        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  99 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	F: bypass plus shadow-stall from using a register the instruction after it stalled
	G: stall from stalling immediately after using a bypass
	H: stall from using a0.x the instruction after writing to it
	//    microcode: 100 slots, 54.51 cycles, cpi = 0.5451
*/

static const	char	vshh_wevm_1tex[] =
	"vs.1.1 "
	"mul r0.xyz, v0.xyz, c[ 83 ].z "
	"mov r0.w, v0.w "
	"mov a0.x, v12.x "
	"mul r2, r0.x, c[a0.x + 0] "
	"mad r2, r0.y, c[a0.x + 1], r2 "
	"mad r2, r0.z, c[a0.x + 2], r2 "
	"mad r2, r0.w, c[a0.x + 3], r2 "
	"mul r4.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r4.xyz, v2.y, c[a0.x + 1].xyz, r4 "
	"mad r4.xyz, v2.z, c[a0.x + 2].xyz, r4 "
	"mul r2.xyz, r2.xyz, v11.x "
	"mul r4.xyz, r4.xyz, v11.x "
	"mov a0.x, v12.y "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.y, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.y, r4.xyz "
	"mov a0.x, v12.z "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.z, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.z, r4.xyz "
	"mov a0.x, v12.w "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.w, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.w, r4.xyz "
	"mul r2.xyz, r2.xyz, c[ 83 ].w "
	"mul r4.xyz, r4.xyz, c[ 83 ].w "
	"mul r3, r2.x, c[32 + 0] "
	"mad r3, r2.y, c[32 + 1], r3 "
	"mad r3, r2.z, c[32 + 2], r3 "
	"mad r3, r2.w, c[32 + 3], r3 "
	"dp3 r5.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r5.x, r5.x "
	"mul r6.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r5.x "
	"add oFog.x, c[ 69 ].y, r6.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov oD0.w, c[ 82 ].x "
	"max r5.xyz, r5.xyz, c[80] "
	"mul r4.xyz, r5.x, c[44 + 0] "
	"mad r4.xyz, r5.y, c[44 + 1], r4.xyz "
	"mad r4.xyz, r5.z, c[44 + 2], r4.xyz "
	"add r4.xyz, c[44 + 3], r4.xyz "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"mul oD0.xyz, r4.xyz, c[83].y "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 68
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0.xyz, v0.xyz, c83.z
	##   1        ##  mov r0.w, v0.w
	##   2        ##  mov a0.x, v12.x
	##   3 H 0.17 ##  mul r2, r0.x, c[a0.x]
	##   4 B 0.00 ##  mad r2, r0.y, c[a0.x+1], r2
	##   5 B 0.00 ##  mad r2, r0.z, c[a0.x+2], r2
	##   6 B 0.00 ##  mad r2, r0.w, c[a0.x+3], r2
	##   7        ##  mul r4.xyz, v2.x, c[a0.x].xyz
	##   8 B 0.00 ##  mad r4.xyz, v2.y, c[a0.x+1].xyz, r4
	##   9 B 0.00 ##  mad r4.xyz, v2.z, c[a0.x+2].xyz, r4
	##  10        ##  mul r2.xyz, r2.xyz, v11.x
	##  11        ##  mul r4.xyz, r4.xyz, v11.x
	##  12        ##  mov a0.x, v12.y
	##  13 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  14 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  15 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  16 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  17        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  18 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  19 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  20        ##  mad r2.xyz, r3.xyz, v11.y, r2.xyz
	##  21        ##  mad r4.xyz, r5.xyz, v11.y, r4.xyz
	##  22        ##  mov a0.x, v12.z
	##  23 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  24 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  25 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  26 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  27        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  28 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  29 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  30        ##  mad r2.xyz, r3.xyz, v11.z, r2.xyz
	##  31        ##  mad r4.xyz, r5.xyz, v11.z, r4.xyz
	##  32        ##  mov a0.x, v12.w
	##  33 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  34 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  35 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  36 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  37        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  38 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  39 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  40        ##  mad r2.xyz, r3.xyz, v11.w, r2.xyz
	##  41        ##  mad r4.xyz, r5.xyz, v11.w, r4.xyz
	##  42        ##  mul r2.xyz, r2.xyz, c83.w
	##  43        ##  mul r4.xyz, r4.xyz, c83.w
	##  44        ##  mul r3, r2.x, c32
	##  45 B 0.00 ##  mad r3, r2.y, c33, r3
	##  46 B 0.00 ##  mad r3, r2.z, c34, r3
	##  47 B 0.00 ##  mad r3, r2.w, c35, r3
	##  48        ##  dp3 r5.x, r4, r4
	##  49        ##  mov oPos, r3
	##  50        ##  rsq r5.x, r5.x
	##  51        ##  mul r6.x, r3.w, c69.x
	##  52        ##  mul r4.xyz, r4.xyz, r5.x
	##  53        ##  add oFog.x, c69.y, r6.x
	##  54        ##  mul r5.xyz, r4.x, c40.xyz
	##  55 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  56 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  57        ##  mov oD0.w, c82.x
	##  58        ##  max r5.xyz, r5.xyz, c80
	##  59 B 0.00 ##  mul r4.xyz, r5.x, c44
	##  60 B 0.00 ##  mad r4.xyz, r5.y, c45, r4.xyz
	##  61 B 0.00 ##  mad r4.xyz, r5.z, c46, r4.xyz
	##  62 B 0.00 ##  add r4.xyz, c47, r4.xyz
	##  63        ##  mul r9, v7, c60
	##  64 B 0.00 ##  add oT0.xy, r9, c61
	##  65        ##  mul oD0.xyz, r4.xyz, c83.y
	##  66        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  67 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	H: stall from using a0.x the instruction after writing to it
	//    microcode: 68 slots, 35.18 cycles, cpi = 0.517353
*/

static const	char	vshh_wevm_emap_1tex[] =
	"vs.1.1 "
	"mul r0.xyz, v0.xyz, c[ 83 ].z "
	"mov r0.w, v0.w "
	"mov a0.x, v12.x "
	"mul r2, r0.x, c[a0.x + 0] "
	"mad r2, r0.y, c[a0.x + 1], r2 "
	"mad r2, r0.z, c[a0.x + 2], r2 "
	"mad r2, r0.w, c[a0.x + 3], r2 "
	"mul r4.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r4.xyz, v2.y, c[a0.x + 1].xyz, r4 "
	"mad r4.xyz, v2.z, c[a0.x + 2].xyz, r4 "
	"mul r2.xyz, r2.xyz, v11.x "
	"mul r4.xyz, r4.xyz, v11.x "
	"mov a0.x, v12.y "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.y, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.y, r4.xyz "
	"mov a0.x, v12.z "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.z, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.z, r4.xyz "
	"mov a0.x, v12.w "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mad r2.xyz, r3.xyz, v11.w, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.w, r4.xyz "
	"mul r2.xyz, r2.xyz, c[ 83 ].w "
	"mul r4.xyz, r4.xyz, c[ 83 ].w "
	"mul r3, r2.x, c[32 + 0] "
	"mad r3, r2.y, c[32 + 1], r3 "
	"mad r3, r2.z, c[32 + 2], r3 "
	"mad r3, r2.w, c[32 + 3], r3 "
	"dp3 r5.x, r4, r4 "
	"mov oPos, r3 "
	"rsq r5.x, r5.x "
	"mul r6.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r5.x "
	"add oFog.x, c[ 69 ].y, r6.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov oD0.w, c[ 82 ].x "
	"max r5.xyz, r5.xyz, c[80] "
	"mul r6.xyz, r5.x, c[44 + 0] "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"add r6.xyz, c[44 + 3], r6.xyz "
	"mul r9,  v7.xy, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	"mul oD0.xyz, r6.xyz, c[83].y "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 68
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0.xyz, v0.xyz, c83.z
	##   1        ##  mov r0.w, v0.w
	##   2        ##  mov a0.x, v12.x
	##   3 H 0.17 ##  mul r2, r0.x, c[a0.x]
	##   4 B 0.00 ##  mad r2, r0.y, c[a0.x+1], r2
	##   5 B 0.00 ##  mad r2, r0.z, c[a0.x+2], r2
	##   6 B 0.00 ##  mad r2, r0.w, c[a0.x+3], r2
	##   7        ##  mul r4.xyz, v2.x, c[a0.x].xyz
	##   8 B 0.00 ##  mad r4.xyz, v2.y, c[a0.x+1].xyz, r4
	##   9 B 0.00 ##  mad r4.xyz, v2.z, c[a0.x+2].xyz, r4
	##  10        ##  mul r2.xyz, r2.xyz, v11.x
	##  11        ##  mul r4.xyz, r4.xyz, v11.x
	##  12        ##  mov a0.x, v12.y
	##  13 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  14 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  15 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  16 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  17        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  18 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  19 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  20        ##  mad r2.xyz, r3.xyz, v11.y, r2.xyz
	##  21        ##  mad r4.xyz, r5.xyz, v11.y, r4.xyz
	##  22        ##  mov a0.x, v12.z
	##  23 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  24 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  25 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  26 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  27        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  28 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  29 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  30        ##  mad r2.xyz, r3.xyz, v11.z, r2.xyz
	##  31        ##  mad r4.xyz, r5.xyz, v11.z, r4.xyz
	##  32        ##  mov a0.x, v12.w
	##  33 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  34 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  35 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  36 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  37        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  38 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  39 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  40        ##  mad r2.xyz, r3.xyz, v11.w, r2.xyz
	##  41        ##  mad r4.xyz, r5.xyz, v11.w, r4.xyz
	##  42        ##  mul r2.xyz, r2.xyz, c83.w
	##  43        ##  mul r4.xyz, r4.xyz, c83.w
	##  44        ##  mul r3, r2.x, c32
	##  45 B 0.00 ##  mad r3, r2.y, c33, r3
	##  46 B 0.00 ##  mad r3, r2.z, c34, r3
	##  47 B 0.00 ##  mad r3, r2.w, c35, r3
	##  48        ##  dp3 r5.x, r4, r4
	##  49        ##  mov oPos, r3
	##  50        ##  rsq r5.x, r5.x
	##  51        ##  mul r6.x, r3.w, c69.x
	##  52        ##  mul r4.xyz, r4.xyz, r5.x
	##  53        ##  add oFog.x, c69.y, r6.x
	##  54        ##  mul r5.xyz, r4.x, c40.xyz
	##  55 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  56 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  57        ##  mov oD0.w, c82.x
	##  58        ##  max r5.xyz, r5.xyz, c80
	##  59 B 0.00 ##  mul r6.xyz, r5.x, c44
	##  60 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  61 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  62 B 0.00 ##  add r6.xyz, c47, r6.xyz
	##  63        ##  mul r9, v7.xy, c60
	##  64 B 0.00 ##  add oT0.xy, r9, c61
	##  65        ##  mul oD0.xyz, r6.xyz, c83.y
	##  66        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  67 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	H: stall from using a0.x the instruction after writing to it
	//    microcode: 68 slots, 35.18 cycles, cpi = 0.517353
*/

static const	char	vshh_wevm_bmap_1tex[] =
	"vs.1.1 "
	"mul r0.xyz, v0.xyz, c[ 83 ].z "
	"mov r0.w, v0.w "
	"mov a0.x, v12.x "
	"mul r2, r0.x, c[a0.x + 0] "
	"mad r2, r0.y, c[a0.x + 1], r2 "
	"mad r2, r0.z, c[a0.x + 2], r2 "
	"mad r2, r0.w, c[a0.x + 3], r2 "
	"mul r4.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r4.xyz, v2.y, c[a0.x + 1].xyz, r4 "
	"mad r4.xyz, v2.z, c[a0.x + 2].xyz, r4 "
	"mul r8.xyz, v9.x, c[a0.x + 0].xyz "
	"mad r8.xyz, v9.y, c[a0.x + 1].xyz, r8 "
	"mad r8.xyz, v9.z, c[a0.x + 2].xyz, r8 "
	"mul r2.xyz, r2.xyz, v11.x "
	"mul r4.xyz, r4.xyz, v11.x "
	"mul r8.xyz, r8.xyz, v11.x "
	"mov a0.x, v12.y "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mul r9.xyz, v9.x, c[a0.x + 0].xyz "
	"mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9 "
	"mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9 "
	"mad r2.xyz, r3.xyz, v11.y, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.y, r4.xyz "
	"mad r8.xyz, r9.xyz, v11.y, r8.xyz "
	"mov a0.x, v12.z "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mul r9.xyz, v9.x, c[a0.x + 0].xyz "
	"mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9 "
	"mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9 "
	"mad r2.xyz, r3.xyz, v11.z, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.z, r4.xyz "
	"mad r8.xyz, r9.xyz, v11.z, r8.xyz "
	"mov a0.x, v12.w "
	"mul r3, r0.x, c[a0.x + 0] "
	"mad r3, r0.y, c[a0.x + 1], r3 "
	"mad r3, r0.z, c[a0.x + 2], r3 "
	"mad r3, r0.w, c[a0.x + 3], r3 "
	"mul r5.xyz, v2.x, c[a0.x + 0].xyz "
	"mad r5.xyz, v2.y, c[a0.x + 1].xyz, r5 "
	"mad r5.xyz, v2.z, c[a0.x + 2].xyz, r5 "
	"mul r9.xyz, v9.x, c[a0.x + 0].xyz "
	"mad r9.xyz, v9.y, c[a0.x + 1].xyz, r9 "
	"mad r9.xyz, v9.z, c[a0.x + 2].xyz, r9 "
	"mad r2.xyz, r3.xyz, v11.w, r2.xyz "
	"mad r4.xyz, r5.xyz, v11.w, r4.xyz "
	"mad r8.xyz, r9.xyz, v11.w, r8.xyz "
	"mul r2.xyz, r2.xyz, c[ 83 ].w "
	"mul r4.xyz, r4.xyz, c[ 83 ].w "
	"mul r8.xyz, r8.xyz, c[ 83 ].w "
	"mul r3, r2.x, c[32 + 0] "
	"mad r3, r2.y, c[32 + 1], r3 "
	"mad r3, r2.z, c[32 + 2], r3 "
	"mad r3, r2.w, c[32 + 3], r3 "
	"dp3 r4.w, r4, r4 "
	"mov oPos, r3 "
	"rsq r1.x, r4.w "
	"mul r6.x, r3.w, c[ 69 ].x "
	"mul r4.xyz, r4.xyz, r1.x "
	"add oFog.x, c[ 69 ].y, r6.x "
	"mul	r5.xyz, r4.x, c[ 40 + 0 ].xyz "
	"mad	r5.xyz, r4.y, c[ 40 + 1 ].xyz, r5.xyz "
	"mad	r5.xyz, r4.z, c[ 40 + 2 ].xyz, r5.xyz "
	"mov oD0.w, c[ 82 ].x "
	"max r5.xyz, r5.xyz, c[80] "
	"mul r6.xyz, r5.x, c[44 + 0] "
	"mad r6.xyz, r5.y, c[44 + 1], r6.xyz "
	"mad r6.xyz, r5.z, c[44 + 2], r6.xyz "
	"add r6.xyz, c[44 + 3], r6.xyz "
	"dp3 r8.w, r8, r8 "
	"mul oD0.xyz, r6.xyz, c[83].y "
	"rsq r1.x, r8.w "
	"mul r8.xyz, r8.xyz, r1.x "
	"mul r7.xyz, r4.yzx, r8.zxy "
	"mad r7.xyz, -r4.zxy, r8.yzx, r7 "
	"dp3 r8.x, r8.xyz, c[ 84 ].xyz "
	"dp3 r8.y, r7.xyz, c[ 84 ].xyz "
	"mad r8.xy, r8.xy, c[ 62 ].zw, v8 "
	"min r8.xy, r8.xy, c[ 81 ] "
	"max r8.xy, r8.xy, c[ 80 ] "
	"mul r9,  v7, c[ 60 ] "
	"add oT0.xy,  r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 95
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0.xyz, v0.xyz, c83.z
	##   1        ##  mov r0.w, v0.w
	##   2        ##  mov a0.x, v12.x
	##   3 H 0.17 ##  mul r2, r0.x, c[a0.x]
	##   4 B 0.00 ##  mad r2, r0.y, c[a0.x+1], r2
	##   5 B 0.00 ##  mad r2, r0.z, c[a0.x+2], r2
	##   6 B 0.00 ##  mad r2, r0.w, c[a0.x+3], r2
	##   7        ##  mul r4.xyz, v2.x, c[a0.x].xyz
	##   8 B 0.00 ##  mad r4.xyz, v2.y, c[a0.x+1].xyz, r4
	##   9 B 0.00 ##  mad r4.xyz, v2.z, c[a0.x+2].xyz, r4
	##  10        ##  mul r8.xyz, v9.x, c[a0.x].xyz
	##  11 B 0.00 ##  mad r8.xyz, v9.y, c[a0.x+1].xyz, r8
	##  12 B 0.00 ##  mad r8.xyz, v9.z, c[a0.x+2].xyz, r8
	##  13        ##  mul r2.xyz, r2.xyz, v11.x
	##  14        ##  mul r4.xyz, r4.xyz, v11.x
	##  15        ##  mul r8.xyz, r8.xyz, v11.x
	##  16        ##  mov a0.x, v12.y
	##  17 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  18 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  19 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  20 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  21        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  22 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  23 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  24        ##  mul r9.xyz, v9.x, c[a0.x].xyz
	##  25 B 0.00 ##  mad r9.xyz, v9.y, c[a0.x+1].xyz, r9
	##  26 B 0.00 ##  mad r9.xyz, v9.z, c[a0.x+2].xyz, r9
	##  27        ##  mad r2.xyz, r3.xyz, v11.y, r2.xyz
	##  28        ##  mad r4.xyz, r5.xyz, v11.y, r4.xyz
	##  29        ##  mad r8.xyz, r9.xyz, v11.y, r8.xyz
	##  30        ##  mov a0.x, v12.z
	##  31 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  32 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  33 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  34 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  35        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  36 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  37 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  38        ##  mul r9.xyz, v9.x, c[a0.x].xyz
	##  39 B 0.00 ##  mad r9.xyz, v9.y, c[a0.x+1].xyz, r9
	##  40 B 0.00 ##  mad r9.xyz, v9.z, c[a0.x+2].xyz, r9
	##  41        ##  mad r2.xyz, r3.xyz, v11.z, r2.xyz
	##  42        ##  mad r4.xyz, r5.xyz, v11.z, r4.xyz
	##  43        ##  mad r8.xyz, r9.xyz, v11.z, r8.xyz
	##  44        ##  mov a0.x, v12.w
	##  45 H 0.17 ##  mul r3, r0.x, c[a0.x]
	##  46 B 0.00 ##  mad r3, r0.y, c[a0.x+1], r3
	##  47 B 0.00 ##  mad r3, r0.z, c[a0.x+2], r3
	##  48 B 0.00 ##  mad r3, r0.w, c[a0.x+3], r3
	##  49        ##  mul r5.xyz, v2.x, c[a0.x].xyz
	##  50 B 0.00 ##  mad r5.xyz, v2.y, c[a0.x+1].xyz, r5
	##  51 B 0.00 ##  mad r5.xyz, v2.z, c[a0.x+2].xyz, r5
	##  52        ##  mul r9.xyz, v9.x, c[a0.x].xyz
	##  53 B 0.00 ##  mad r9.xyz, v9.y, c[a0.x+1].xyz, r9
	##  54 B 0.00 ##  mad r9.xyz, v9.z, c[a0.x+2].xyz, r9
	##  55        ##  mad r2.xyz, r3.xyz, v11.w, r2.xyz
	##  56        ##  mad r4.xyz, r5.xyz, v11.w, r4.xyz
	##  57        ##  mad r8.xyz, r9.xyz, v11.w, r8.xyz
	##  58        ##  mul r2.xyz, r2.xyz, c83.w
	##  59        ##  mul r4.xyz, r4.xyz, c83.w
	##  60        ##  mul r8.xyz, r8.xyz, c83.w
	##  61        ##  mul r3, r2.x, c32
	##  62 B 0.00 ##  mad r3, r2.y, c33, r3
	##  63 B 0.00 ##  mad r3, r2.z, c34, r3
	##  64 B 0.00 ##  mad r3, r2.w, c35, r3
	##  65        ##  dp3 r4.w, r4, r4
	##  66        ##  mov oPos, r3
	##  67        ##  rsq r1.x, r4.w
	##  68        ##  mul r6.x, r3.w, c69.x
	##  69        ##  mul r4.xyz, r4.xyz, r1.x
	##  70        ##  add oFog.x, c69.y, r6.x
	##  71        ##  mul r5.xyz, r4.x, c40.xyz
	##  72 B 0.00 ##  mad r5.xyz, r4.y, c41.xyz, r5.xyz
	##  73 B 0.00 ##  mad r5.xyz, r4.z, c42.xyz, r5.xyz
	##  74        ##  mov oD0.w, c82.x
	##  75        ##  max r5.xyz, r5.xyz, c80
	##  76 B 0.00 ##  mul r6.xyz, r5.x, c44
	##  77 B 0.00 ##  mad r6.xyz, r5.y, c45, r6.xyz
	##  78 B 0.00 ##  mad r6.xyz, r5.z, c46, r6.xyz
	##  79 B 0.00 ##  add r6.xyz, c47, r6.xyz
	##  80        ##  dp3 r8.w, r8, r8
	##  81        ##  mul oD0.xyz, r6.xyz, c83.y
	##  82        ##  rsq r1.x, r8.w
	##  83 C 0.50 ##  mul r8.xyz, r8.xyz, r1.x
	##  84 F 0.50 ##  mul r7.xyz, r4.yzx, r8.zxy
	##  85 B 0.00 ##  mad r7.xyz, -r4.zxy, r8.yzx, r7
	##  86        ##  dp3 r8.x, r8.xyz, c84.xyz
	##  87        ##  dp3 r8.y, r7.xyz, c84.xyz
	##  88 C 0.50 ##  mad r8.xy, r8.xy, c62.zw, v8
	##  89 E 1.05 ##  min r8.xy, r8.xy, c81
	##  90 F 0.50 ##  max r8.xy, r8.xy, c80
	##  91        ##  mul r9, v7, c60
	##  92 B 0.00 ##  add oT0.xy, r9, c61
	##  93        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  94 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	E: standard stall plus shadow-stall from using a register the instruction after it stalled
	F: bypass plus shadow-stall from using a register the instruction after it stalled
	H: stall from using a0.x the instruction after writing to it
	//    microcode: 95 slots, 51.73 cycles, cpi = 0.544526
*/

static const	char	vshh_wprm2poly[] =
	"vs.1.1 "
	"mul r0, v0.x, c[ 0		  + 0 ] "
	"mad r0, v0.y, c[ 0		  + 1 ], r0 "
	"mad r0, v0.z, c[ 0		  + 2 ], r0 "
	"mad r0, v0.w, c[ 0		  + 3 ], r0 "
	"mov oD0, v3 "
	"mul r1, r0.x, c[ 36	  + 0 ] "
	"mad r1, r0.y, c[ 36	  + 1 ], r1 "
	"mad r1, r0.z, c[ 36	  + 2 ], r1 "
	"mad r1, r0.w, c[ 36	  + 3 ], r1 "
	"mul r3, v7, c[ 60 ] "
	"mov oPos, r1 "
	"mul r2.x, r1.w, c[ 69 ].x "
	"add oT0.xy, c[ 61 ], r3 "
	"add oFog.x, c[ 69 ].y, r2.x "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 16
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0, v0.x, c0
	##   1 A 1.50 ##  mad r0, v0.y, c1, r0
	##   2 B 0.00 ##  mad r0, v0.z, c2, r0
	##   3 B 0.00 ##  mad r0, v0.w, c3, r0
	##   4        ##  mov oD0, v3
	##   5        ##  mul r1, r0.x, c36
	##   6 B 0.00 ##  mad r1, r0.y, c37, r1
	##   7 B 0.00 ##  mad r1, r0.z, c38, r1
	##   8 B 0.00 ##  mad r1, r0.w, c39, r1
	##   9        ##  mul r3, v7, c60
	##  10        ##  mov oPos, r1
	##  11        ##  mul r2.x, r1.w, c69.x
	##  12        ##  add oT0.xy, c61, r3
	##  13        ##  add oFog.x, c69.y, r2.x
	##  14        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  15 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 16 slots, 10 cycles, cpi = 0.625
*/

static const	char	vshh_wprm2sprt[] =
	"vs.1.1 "
	"mul r0, v0.x, c[ 0		  + 0 ] "
	"mad r0, v0.y, c[ 0		  + 1 ], r0 "
	"mad r0, v0.z, c[ 0		  + 2 ], r0 "
	"mad r0, v0.w, c[ 0		  + 3 ], r0 "
	"add r0.xy, v2.xy, r0.xy "
	"mov oD0, v3 "
	"mul r1, r0.x, c[ 36	  + 0 ] "
	"mad r1, r0.y, c[ 36	  + 1 ], r1 "
	"mad r1, r0.z, c[ 36	  + 2 ], r1 "
	"mad r1, r0.w, c[ 36	  + 3 ], r1 "
	"mul r3, v7, c[ 60 ] "
	"mov oPos, r1 "
	"mul r2.x, r1.w, c[ 69 ].x "
	"add oT0.xy, c[ 61 ], r3 "
	"add oFog.x, c[ 69 ].y, r2.x "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 17
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0, v0.x, c0
	##   1 A 1.50 ##  mad r0, v0.y, c1, r0
	##   2 B 0.00 ##  mad r0, v0.z, c2, r0
	##   3 B 0.00 ##  mad r0, v0.w, c3, r0
	##   4 B 0.00 ##  add r0.xy, v2.xy, r0.xy
	##   5        ##  mov oD0, v3
	##   6        ##  mul r1, r0.x, c36
	##   7 B 0.00 ##  mad r1, r0.y, c37, r1
	##   8 B 0.00 ##  mad r1, r0.z, c38, r1
	##   9 B 0.00 ##  mad r1, r0.w, c39, r1
	##  10        ##  mul r3, v7, c60
	##  11        ##  mov oPos, r1
	##  12        ##  mul r2.x, r1.w, c69.x
	##  13        ##  add oT0.xy, c61, r3
	##  14        ##  add oFog.x, c69.y, r2.x
	##  15        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  16 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 17 slots, 10.5 cycles, cpi = 0.617647
*/

static const	char	vshh_wcomdl[] =
	"vs.1.1 "
	"mul r0, v0.x, c[ 0 + 0 ] "
	"mad r0, v0.y, c[ 0 + 1 ], r0 "
	"mad r0, v0.z, c[ 0 + 2 ], r0 "
	"mad r0, v0.w, c[ 0 + 3 ], r0 "
	"mov r3, c[ 44 + 3 ] "
	"mul r1, r0.x, c[ 32 + 0 ] "
	"mad r1, r0.y, c[ 32 + 1 ], r1 "
	"mad r1, r0.z, c[ 32 + 2 ], r1 "
	"mad r1, r0.w, c[ 32 + 3 ], r1 "
	"mul oD0, r3, c[ 83 ].y "
	"mov oPos, r1 "
	"mul r2.x, r1.w, c[ 69 ].x "
	"mul r9,  	v7, c[ 60 ] "
	"add oFog.x, c[ 69 ].y, r2.x "
	"add oT0.xy,  r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 17
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0, v0.x, c0
	##   1 A 1.50 ##  mad r0, v0.y, c1, r0
	##   2 B 0.00 ##  mad r0, v0.z, c2, r0
	##   3 B 0.00 ##  mad r0, v0.w, c3, r0
	##   4        ##  mov r3, c47
	##   5        ##  mul r1, r0.x, c32
	##   6 B 0.00 ##  mad r1, r0.y, c33, r1
	##   7 B 0.00 ##  mad r1, r0.z, c34, r1
	##   8 B 0.00 ##  mad r1, r0.w, c35, r1
	##   9        ##  mul oD0, r3, c83.y
	##  10        ##  mov oPos, r1
	##  11        ##  mul r2.x, r1.w, c69.x
	##  12        ##  mul r9, v7, c60
	##  13        ##  add oFog.x, c69.y, r2.x
	##  14        ##  add oT0.xy, r9, c61
	##  15        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  16 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 17 slots, 10.5 cycles, cpi = 0.617647
*/

static const	char	vshh_woptcmf00[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul	r11.x, c[ 83 ].z, v0.w "
	"add r6.x,    c[81].x,     -r11.x "
	"mul r2.xyz,  r2.xyz,          r11.x "
	"mul r4.xyz,  r4.xyz,          r11.x "
	"mad r2.xyz,  r3.xyz, r6.x,    r2.xyz "
	"mad r4.xyz,  r5.xyz, r6.x,    r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r5.x, r4.xyz, r4.xyz "
	"mov oPos, r3 "
	"mul r6.x, r3.w, c[ 69 ].x "
	"rsq r1.x, r5.x "
	"add oFog.x, c[ 69 ].y, r6.x "
	"mul r4.xyz, r4.xyz, r1.x "
	"rcp r1.x, r3.w "
	"mul r5.xy, r4.x, c[ 48 + 0 ] "
	"mad r5.xy, r4.y, c[ 48 + 1 ], r5 "
	"mad r5.xy, r4.z, c[ 48 + 2 ], r5 "
	"mul r7.xy, r3.xy, r1.x "
	"mul r7.xy, r7.xy, c[ 44 + 0 ] "
	"add r7.xy, c[ 44 + 1 ], r7 "
	"mad r7.xy, r5.xy, c[ 44 + 2 ], r7 "
	"mov oT0.w, r3.w "
	"mul oT0.xy, r7, r3.w "
	"mov r8.x, c[ 83 ].y "
	"mul oD0.xyzw, c[44 + 3].xyzw, r8.x "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 44
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r3, v0.x, c4
	##   5 B 0.00 ##  mad r3, v0.y, c5, r3
	##   6 B 0.00 ##  mad r3, v0.z, c6, r3
	##   7 B 0.00 ##  add r3, c7, r3
	##   8        ##  mul r4.xyz, v2.x, c0.xyz
	##   9 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##  10 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  mul r11.x, c83.z, v0.w
	##  15 B 0.00 ##  add r6.x, c81.x, -r11.x
	##  16        ##  mul r2.xyz, r2.xyz, r11.x
	##  17        ##  mul r4.xyz, r4.xyz, r11.x
	##  18        ##  mad r2.xyz, r3.xyz, r6.x, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r6.x, r4
	##  20        ##  mul r3, r2.x, c32
	##  21 B 0.00 ##  mad r3, r2.y, c33, r3
	##  22 B 0.00 ##  mad r3, r2.z, c34, r3
	##  23 B 0.00 ##  mad r3, r2.w, c35, r3
	##  24        ##  dp3 r5.x, r4.xyz, r4.xyz
	##  25        ##  mov oPos, r3
	##  26        ##  mul r6.x, r3.w, c69.x
	##  27        ##  rsq r1.x, r5.x
	##  28        ##  add oFog.x, c69.y, r6.x
	##  29        ##  mul r4.xyz, r4.xyz, r1.x
	##  30        ##  rcp r1.x, r3.w
	##  31        ##  mul r5.xy, r4.x, c48
	##  32 B 0.00 ##  mad r5.xy, r4.y, c49, r5
	##  33 B 0.00 ##  mad r5.xy, r4.z, c50, r5
	##  34        ##  mul r7.xy, r3.xy, r1.x
	##  35 B 0.00 ##  mul r7.xy, r7.xy, c44
	##  36 B 0.00 ##  add r7.xy, c45, r7
	##  37 B 0.00 ##  mad r7.xy, r5.xy, c46, r7
	##  38        ##  mov oT0.w, r3.w
	##  39        ##  mul oT0.xy, r7, r3.w
	##  40        ##  mov r8.x, c83.y
	##  41 B 0.00 ##  mul oD0, c47, r8.x
	##  42        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  43 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 44 slots, 24 cycles, cpi = 0.545455
*/

static const	char	vshh_woptcmf00_fvf[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"add r6.x,    c[81].x,     -v0.w "
	"mul r2.xyz,  r2.xyz,          v0.w "
	"mul r4.xyz,  r4.xyz,          v0.w "
	"mad r2.xyz,  r3.xyz, r6.x,    r2.xyz "
	"mad r4.xyz,  r5.xyz, r6.x,    r4 "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r5.x, r4.xyz, r4.xyz "
	"mov oPos, r3 "
	"mul r6.x, r3.w, c[ 69 ].x "
	"rsq r1.x, r5.x "
	"add oFog.x, c[ 69 ].y, r6.x "
	"mul r4.xyz, r4.xyz, r1.x "
	"rcp r1.x, r3.w "
	"mul r5.xy, r4.x, c[ 48 + 0 ] "
	"mad r5.xy, r4.y, c[ 48 + 1 ], r5 "
	"mad r5.xy, r4.z, c[ 48 + 2 ], r5 "
	"mul r7.xy, r3.xy, r1.x "
	"mul r7.xy, r7.xy, c[ 44 + 0 ] "
	"add r7.xy, c[ 44 + 1 ], r7 "
	"mad r7.xy, r5.xy, c[ 44 + 2 ], r7 "
	"mov oT0.w, r3.w "
	"mul oT0.xy, r7, r3.w "
	"mov r8.x, c[ 83 ].y "
	"mul oD0.xyzw, c[44 + 3].xyzw, r8.x "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 43
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  add r2, c3, r2
	##   4        ##  mul r3, v0.x, c4
	##   5 B 0.00 ##  mad r3, v0.y, c5, r3
	##   6 B 0.00 ##  mad r3, v0.z, c6, r3
	##   7 B 0.00 ##  add r3, c7, r3
	##   8        ##  mul r4.xyz, v2.x, c0.xyz
	##   9 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##  10 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##  11        ##  mul r5.xyz, v2.x, c4.xyz
	##  12 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  14        ##  add r6.x, c81.x, -v0.w
	##  15        ##  mul r2.xyz, r2.xyz, v0.w
	##  16        ##  mul r4.xyz, r4.xyz, v0.w
	##  17        ##  mad r2.xyz, r3.xyz, r6.x, r2.xyz
	##  18        ##  mad r4.xyz, r5.xyz, r6.x, r4
	##  19        ##  mul r3, r2.x, c32
	##  20 B 0.00 ##  mad r3, r2.y, c33, r3
	##  21 B 0.00 ##  mad r3, r2.z, c34, r3
	##  22 B 0.00 ##  mad r3, r2.w, c35, r3
	##  23        ##  dp3 r5.x, r4.xyz, r4.xyz
	##  24        ##  mov oPos, r3
	##  25        ##  mul r6.x, r3.w, c69.x
	##  26        ##  rsq r1.x, r5.x
	##  27        ##  add oFog.x, c69.y, r6.x
	##  28        ##  mul r4.xyz, r4.xyz, r1.x
	##  29        ##  rcp r1.x, r3.w
	##  30        ##  mul r5.xy, r4.x, c48
	##  31 B 0.00 ##  mad r5.xy, r4.y, c49, r5
	##  32 B 0.00 ##  mad r5.xy, r4.z, c50, r5
	##  33        ##  mul r7.xy, r3.xy, r1.x
	##  34 B 0.00 ##  mul r7.xy, r7.xy, c44
	##  35 B 0.00 ##  add r7.xy, c45, r7
	##  36 B 0.00 ##  mad r7.xy, r5.xy, c46, r7
	##  37        ##  mov oT0.w, r3.w
	##  38        ##  mul oT0.xy, r7, r3.w
	##  39        ##  mov r8.x, c83.y
	##  40 B 0.00 ##  mul oD0, c47, r8.x
	##  41        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  42 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 43 slots, 23.5 cycles, cpi = 0.546512
*/

static const	char	vshh_wpatch[] =
	"vs.1.1 "
	"mul r0, v0.x, c[ 0 + 0 ] "
	"mad r0, v0.y, c[ 0 + 1 ], r0 "
	"mad r0, v0.z, c[ 0 + 2 ], r0 "
	"mad r0, v0.w, c[ 0 + 3 ], r0 "
	"mul r2.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r2.xyz, v2.y, c[ 0 + 1 ].xyz, r2.xyz "
	"mad r2.xyz, v2.z, c[ 0 + 2 ].xyz, r2.xyz "
	"mul r1, r0.x, c[ 32 + 0 ] "
	"mad r1, r0.y, c[ 32 + 1 ], r1 "
	"mad r1, r0.z, c[ 32 + 2 ], r1 "
	"mad r1, r0.w, c[ 32 + 3 ], r1 "
	"dp3 r3.x, r2, r2 "
	"mov oPos, r1 "
	"mul r2.x, r1.w, c[ 69 ].x "
	"rsq r3.x, r3.x "
	"add oFog.x, c[ 69 ].y, r2.x "
	"mul r2.xyz, r2.xyz, r3.x "
	"mul	r11.xyz, r2.x, c[ 40 + 0 ].xyz "
	"mad	r11.xyz, r2.y, c[ 40 + 1 ].xyz, r11.xyz "
	"mad	r11.xyz, r2.z, c[ 40 + 2 ].xyz, r11.xyz "
	"mov oD0.w, v3.w "
	"max r11.xyz, r11.xyz, c[80] "
	"mul r3.xyz, r11.x, c[44 + 0] "
	"mad r3.xyz, r11.y, c[44 + 1], r3.xyz "
	"mad r3.xyz, r11.z, c[44 + 2], r3.xyz "
	"add r3.xyz, c[44 + 3], r3.xyz "
	"mul r9, v7, c[ 60 ] "
	"mul oD0.xyz, r3.xyz, c[83].y "
	"add oT0.xy, r9, c[ 61 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 31
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0, v0.x, c0
	##   1 A 1.50 ##  mad r0, v0.y, c1, r0
	##   2 B 0.00 ##  mad r0, v0.z, c2, r0
	##   3 B 0.00 ##  mad r0, v0.w, c3, r0
	##   4        ##  mul r2.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r2.xyz, v2.y, c1.xyz, r2.xyz
	##   6 B 0.00 ##  mad r2.xyz, v2.z, c2.xyz, r2.xyz
	##   7        ##  mul r1, r0.x, c32
	##   8 B 0.00 ##  mad r1, r0.y, c33, r1
	##   9 B 0.00 ##  mad r1, r0.z, c34, r1
	##  10 B 0.00 ##  mad r1, r0.w, c35, r1
	##  11        ##  dp3 r3.x, r2, r2
	##  12        ##  mov oPos, r1
	##  13        ##  mul r2.x, r1.w, c69.x
	##  14        ##  rsq r3.x, r3.x
	##  15        ##  add oFog.x, c69.y, r2.x
	##  16        ##  mul r2.xyz, r2.xyz, r3.x
	##  17 B 0.00 ##  mul r11.xyz, r2.x, c40.xyz
	##  18 B 0.00 ##  mad r11.xyz, r2.y, c41.xyz, r11.xyz
	##  19 B 0.00 ##  mad r11.xyz, r2.z, c42.xyz, r11.xyz
	##  20        ##  mov oD0.w, v3.w
	##  21        ##  max r11.xyz, r11.xyz, c80
	##  22 B 0.00 ##  mul r3.xyz, r11.x, c44
	##  23 B 0.00 ##  mad r3.xyz, r11.y, c45, r3.xyz
	##  24 B 0.00 ##  mad r3.xyz, r11.z, c46, r3.xyz
	##  25 B 0.00 ##  add r3.xyz, c47, r3.xyz
	##  26        ##  mul r9, v7, c60
	##  27        ##  mul oD0.xyz, r3.xyz, c83.y
	##  28        ##  add oT0.xy, r9, c61
	##  29        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  30 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 31 slots, 17.5 cycles, cpi = 0.564516
*/

static const	char	vshh_wpatch_emap[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"mad r2, v0.w, c[ 0 + 3 ], r2 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"mul r5.x, r3.w, c[ 69 ].x "
	"rsq r1.x, r0.x "
	"add oFog.x, c[ 69 ].y, r5.x "
	"mul r4.xyz, r4.xyz, r1.x "
	"mov r7.xyzw, c[ 48 + 3 ] "
	"mad r7.xyzw, r2.x, c[ 48 + 0 ], r7 "
	"mad r7.xyzw, r2.y, c[ 48 + 1 ], r7 "
	"mad r7.xyzw, r2.z, c[ 48 + 2 ], r7 "
	"mul r8.xyz, r4.x, c[ 48 + 0 ].xyz "
	"mad r8.xyz, r4.y, c[ 48 + 1 ].xyz, r8.xyz "
	"mad r8.xyz, r4.z, c[ 48 + 2 ].xyz, r8.xyz "
	"dp3 r0.x, r7, r7 "
	"mov oD0.w, c[82] "
	"rsq r1.x, r0.x "
	"mov r6.xyzw, c[44 + 3] "
	"mul r7.xyz, r7.xyz, r1.x "
	"dp3 r8.w, r8.xyz, r7.xyz "
	"mul oD0.xyz, r6.xyzw, c[83].y "
	"mad r7.xyz, -r8.xyz, r8.w, r7.xyz "
	"mad r7.xyz, -r8.xyz, r8.w, r7.xyz "
	"mul r7.xyz, r7.xyz, c[ 60 ].xyz "
	"add r7.xyz, r7.xyz, c[ 61 ].xyz "
	"mov oT0.xy, r7 "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 38
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  mad r2, v0.w, c3, r2
	##   4        ##  mul r4.xyz, v2.x, c0.xyz
	##   5 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   6 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##   7        ##  mul r3, r2.x, c32
	##   8 B 0.00 ##  mad r3, r2.y, c33, r3
	##   9 B 0.00 ##  mad r3, r2.z, c34, r3
	##  10 B 0.00 ##  mad r3, r2.w, c35, r3
	##  11        ##  dp3 r0.x, r4, r4
	##  12        ##  mov oPos, r3
	##  13        ##  mul r5.x, r3.w, c69.x
	##  14        ##  rsq r1.x, r0.x
	##  15        ##  add oFog.x, c69.y, r5.x
	##  16        ##  mul r4.xyz, r4.xyz, r1.x
	##  17        ##  mov r7, c51
	##  18 B 0.00 ##  mad r7, r2.x, c48, r7
	##  19 B 0.00 ##  mad r7, r2.y, c49, r7
	##  20 B 0.00 ##  mad r7, r2.z, c50, r7
	##  21        ##  mul r8.xyz, r4.x, c48.xyz
	##  22 B 0.00 ##  mad r8.xyz, r4.y, c49.xyz, r8.xyz
	##  23 B 0.00 ##  mad r8.xyz, r4.z, c50.xyz, r8.xyz
	##  24        ##  dp3 r0.x, r7, r7
	##  25        ##  mov oD0.w, c82
	##  26        ##  rsq r1.x, r0.x
	##  27        ##  mov r6, c47
	##  28        ##  mul r7.xyz, r7.xyz, r1.x
	##  29 B 0.00 ##  dp3 r8.w, r8.xyz, r7.xyz
	##  30        ##  mul oD0.xyz, r6, c83.y
	##  31        ##  mad r7.xyz, -r8.xyz, r8.w, r7.xyz
	##  32 B 0.00 ##  mad r7.xyz, -r8.xyz, r8.w, r7.xyz
	##  33 G 0.83 ##  mul r7.xyz, r7.xyz, c60.xyz
	##  34 F 0.50 ##  add r7.xyz, r7.xyz, c61.xyz
	##  35 G 0.83 ##  mov oT0.xy, r7
	##  36        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  37 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	F: bypass plus shadow-stall from using a register the instruction after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 38 slots, 23.16 cycles, cpi = 0.609474
*/

static const	char	vshh_wpatch2_emap[] =
	"vs.1.1 "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"mad r2, v0.w, c[ 0 + 3 ], r2 "
	"mul r5, v0.x, c[ 0 + 0 ] "
	"mad r5, v0.z, c[ 0 + 2 ], r5 "
	"mad r5, v0.w, c[ 0 + 3 ], r5 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r3, r5.x, c[ 32 + 0 ] "
	"mad r3, r5.y, c[ 32 + 1 ], r3 "
	"mad r3, r5.z, c[ 32 + 2 ], r3 "
	"mad r3, r5.w, c[ 32 + 3 ], r3 "
	"dp3 r0.x, r4, r4 "
	"mov oPos, r3 "
	"mul r5.x, r3.w, c[ 69 ].x "
	"rsq r1.x, r0.x "
	"add oFog.x, c[ 69 ].y, r5.x "
	"mul r4.xyz, r4.xyz, r1.x "
	"mov r7.xyzw, c[ 48 + 3 ] "
	"mad r7.xyzw, r2.x, c[ 48 + 0 ], r7 "
	"mad r7.xyzw, r2.y, c[ 48 + 1 ], r7 "
	"mad r7.xyzw, r2.z, c[ 48 + 2 ], r7 "
	"mul r8.xyz, r4.x, c[ 48 + 0 ].xyz "
	"mad r8.xyz, r4.y, c[ 48 + 1 ].xyz, r8.xyz "
	"mad r8.xyz, r4.z, c[ 48 + 2 ].xyz, r8.xyz "
	"dp3 r0.x, r7, r7 "
	"mov oD0.w, c[82] "
	"rsq r1.x, r0.x "
	"mul r7.xyz, r7.xyz, r1.x "
	"mov r6.xyzw, c[44 + 3] "
	"dp3 r8.w, r8.xyz, r7.xyz "
	"mul oD0.xyz, r6.xyzw, c[83].y "
	"mad r7.xyz, -r8.xyz, r8.w, r7.xyz "
	"mad r7.xyz, -r8.xyz, r8.w, r7.xyz "
	"mul r7.xyz, r7.xyz, c[ 60 ].xyz "
	"add r7.xyz, r7.xyz, c[ 61 ].xyz "
	"mov oT0.xy, r7 "
	"mov oT0.w, c[ 81 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 42
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r2, v0.x, c0
	##   1 A 1.50 ##  mad r2, v0.y, c1, r2
	##   2 B 0.00 ##  mad r2, v0.z, c2, r2
	##   3 B 0.00 ##  mad r2, v0.w, c3, r2
	##   4        ##  mul r5, v0.x, c0
	##   5 B 0.00 ##  mad r5, v0.z, c2, r5
	##   6 B 0.00 ##  mad r5, v0.w, c3, r5
	##   7        ##  mul r4.xyz, v2.x, c0.xyz
	##   8 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##   9 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##  10        ##  mul r3, r5.x, c32
	##  11 B 0.00 ##  mad r3, r5.y, c33, r3
	##  12 B 0.00 ##  mad r3, r5.z, c34, r3
	##  13 B 0.00 ##  mad r3, r5.w, c35, r3
	##  14        ##  dp3 r0.x, r4, r4
	##  15        ##  mov oPos, r3
	##  16        ##  mul r5.x, r3.w, c69.x
	##  17        ##  rsq r1.x, r0.x
	##  18        ##  add oFog.x, c69.y, r5.x
	##  19        ##  mul r4.xyz, r4.xyz, r1.x
	##  20        ##  mov r7, c51
	##  21 B 0.00 ##  mad r7, r2.x, c48, r7
	##  22 B 0.00 ##  mad r7, r2.y, c49, r7
	##  23 B 0.00 ##  mad r7, r2.z, c50, r7
	##  24        ##  mul r8.xyz, r4.x, c48.xyz
	##  25 B 0.00 ##  mad r8.xyz, r4.y, c49.xyz, r8.xyz
	##  26 B 0.00 ##  mad r8.xyz, r4.z, c50.xyz, r8.xyz
	##  27        ##  dp3 r0.x, r7, r7
	##  28        ##  mov oD0.w, c82
	##  29        ##  rsq r1.x, r0.x
	##  30 C 0.50 ##  mul r7.xyz, r7.xyz, r1.x
	##  31        ##  mov r6, c47
	##  32 D 0.40 ##  dp3 r8.w, r8.xyz, r7.xyz
	##  33        ##  mul oD0.xyz, r6, c83.y
	##  34        ##  mad r7.xyz, -r8.xyz, r8.w, r7.xyz
	##  35 B 0.00 ##  mad r7.xyz, -r8.xyz, r8.w, r7.xyz
	##  36 G 0.83 ##  mul r7.xyz, r7.xyz, c60.xyz
	##  37 F 0.50 ##  add r7.xyz, r7.xyz, c61.xyz
	##  38 G 0.83 ##  mov oT0.xy, r7
	##  39        ##  mov oT0.w, c81
	##  40        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  41 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	F: bypass plus shadow-stall from using a register the instruction after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 42 slots, 26.06 cycles, cpi = 0.620476
*/

static const	char	vshh_dmapk_2d[] =
	"vs.1.1 "
	"mul r0, v0, c[1] "
	"mov oD0, v3 "
	"add oPos, r0, c[0] "
	"mov oFog.x, c[ 80 ] "
	"mul r3.xy, v7, c[ 60 ] "
	"add oT0.xy, c[ 61 ], r3 "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 8
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0, v0, c1
	##   1        ##  mov oD0, v3
	##   2        ##  add oPos, r0, c0
	##   3        ##  mov oFog.x, c80
	##   4        ##  mul r3.xy, v7, c60
	##   5 B 0.00 ##  add oT0.xy, c61, r3
	##   6        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##   7 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	//    microcode: 8 slots, 4.5 cycles, cpi = 0.5625
*/

static const	char	vshh_dmapk_2d4[] =
	"vs.1.1 "
	"add r0.xy, v0, c[0].xy "
	"mul r0.xy, r0.xy, c[1].xy "
	"add oPos.xy, -c[ 81 ].xy, r0.xy "
	"mov oPos.z, c[0].z "
	"mov oPos.w, c[ 81 ] "
	"mov oFog.x, c[ 80 ] "
	"mov oD0, v3 "
	"mov oT0.xy, v7 "
	"mov oT1.xy, v8 "
	"mov oT2.xy, v9 "
	"mov oT3.xy, v10 "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 13
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  add r0.xy, v0, c0.xy
	##   1 A 1.50 ##  mul r0.xy, r0.xy, c1.xy
	##   2 B 0.00 ##  add oPos.xy, -c81.xy, r0.xy
	##   3        ##  mov oPos.z, c0.z
	##   4        ##  mov oPos.w, c81
	##   5        ##  mov oFog.x, c80
	##   6        ##  mov oD0, v3
	##   7        ##  mov oT0.xy, v7
	##   8        ##  mov oT1.xy, v8
	##   9        ##  mov oT2.xy, v9
	##  10        ##  mov oT3.xy, v10
	##  11        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  12 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 13 slots, 8.5 cycles, cpi = 0.653846
*/

static const	char	vshh_wshdw_mk[] =
	"vs.1.1 "
	"mul r0, v0.x, c[ 0 + 0 ] "
	"mad r0, v0.y, c[ 0 + 1 ], r0 "
	"mad r0, v0.z, c[ 0 + 2 ], r0 "
	"add r0, c[ 0 + 3 ], r0 "
	"mul r11.w, c[ 83 ].z, v0.w "
	"mul r1, v0.x, c[ 4 + 0 ] "
	"mad r1, v0.y, c[ 4 + 1 ], r1 "
	"mad r1, v0.z, c[ 4 + 2 ], r1 "
	"add r1, c[ 4 + 3 ], r1 "
	"add r4.w, c[81].x,       -r11.w "
	"mul r0.xyz,  r0.xyz,          r11.w "
	"mad r0.xyz,  r1.xyz,          r4.w,      r0.xyz "
	"mov oFog.x, c[ 80 ] "
	"mul r1, r0.x, c[ 32 + 0 ] "
	"mad r1, r0.y, c[ 32 + 1 ], r1 "
	"mad r1, r0.z, c[ 32 + 2 ], r1 "
	"mad r1, r0.w, c[ 32 + 3 ], r1 "
	"mov oD0.w, c[ 82 ].x "
	"mov oPos, r1 "
	"mov r5.x, c[ 83 ].y "
	"mul oD0.xyz, c[44 + 3], r5.x "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 23
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r0, v0.x, c0
	##   1 A 1.50 ##  mad r0, v0.y, c1, r0
	##   2 B 0.00 ##  mad r0, v0.z, c2, r0
	##   3 B 0.00 ##  add r0, c3, r0
	##   4        ##  mul r11.w, c83.z, v0.w
	##   5        ##  mul r1, v0.x, c4
	##   6 B 0.00 ##  mad r1, v0.y, c5, r1
	##   7 B 0.00 ##  mad r1, v0.z, c6, r1
	##   8 B 0.00 ##  add r1, c7, r1
	##   9        ##  add r4.w, c81.x, -r11.w
	##  10        ##  mul r0.xyz, r0.xyz, r11.w
	##  11 B 0.00 ##  mad r0.xyz, r1.xyz, r4.w, r0.xyz
	##  12        ##  mov oFog.x, c80
	##  13        ##  mul r1, r0.x, c32
	##  14 B 0.00 ##  mad r1, r0.y, c33, r1
	##  15 B 0.00 ##  mad r1, r0.z, c34, r1
	##  16 B 0.00 ##  mad r1, r0.w, c35, r1
	##  17        ##  mov oD0.w, c82.x
	##  18        ##  mov oPos, r1
	##  19        ##  mov r5.x, c83.y
	##  20 B 0.00 ##  mul oD0.xyz, c47, r5.x
	##  21        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  22 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 23 slots, 13.5 cycles, cpi = 0.586957
*/

static const	char	vshh_wshdw_wt[] =
	"vs.1.1 "
	"mul r11.w, c[ 83 ].z, v0.w "
	"add r1.x,    c[81].x,     -r11.w "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul r2.xyz, r2.xyz, r11.w "
	"mul r4.xyz, r4.xyz, r11.w "
	"mad r2.xyz, r3.xyz, r1.x, r2.xyz "
	"mad r4.xyz, r5.xyz, r1.x, r4.xyz "
	"mul r3, r2.x, c[ 40 + 0 ] "
	"mad r3, r2.y, c[ 40 + 1 ], r3 "
	"mad r3, r2.z, c[ 40 + 2 ], r3 "
	"mad r3, r2.w, c[ 40 + 3 ], r3 "
	"mul r3.x, r3.x, c[ 83 ].w "
	"mul r3.y, r3.y, -c[ 83 ].w "
	"mad oT0.xy, c[ 84 ].xy, r3.w, r3.xy "
	"mad oT1.xy, c[ 85 ].xy, r3.w, r3.xy "
	"mad oT2.xy, c[ 86 ].xy, r3.w, r3.xy "
	"mov oT0.w, r3.w "
	"dp3 r4.w, r4, r4 "
	"mov oT1.w, r3.w "
	"rsq r1.x, r4.w "
	"mov oT2.w, r3.w "
	"mul r4, r4, r1.x "
	"mov oT3.w, r3.w "
	"rcp r1.x, c[ 44 + 1 ].w "
	"dp3 r5.x, r4, c[ 44 + 2 ] "
	"add r5.w, c[ 44 + 1 ].w, -r3.w "
	"max r5.x, r5.x, c[ 80 ] "
	"mul r5.w, r5.w, r1.x "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"mov oPos, r3 "
	"mul r0.x, r3.w, c[ 69 ].x "
	"add oFog.x, c[ 69 ].y, r0.x "
	"mul r5.w, r5.x, r5.w "
	"mul r5.xyz, c[ 83 ].y, r5.w "
	"mul r5.w, r5.w, c[ 83 ].y "
	"mul oD0.xyz, c[44 + 0].xyz, r5.xyz "
	"mul oD0.w, c[44 + 0].w, r5.w "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 55
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r11.w, c83.z, v0.w
	##   1 A 1.50 ##  add r1.x, c81.x, -r11.w
	##   2        ##  mul r2, v0.x, c0
	##   3 B 0.00 ##  mad r2, v0.y, c1, r2
	##   4 B 0.00 ##  mad r2, v0.z, c2, r2
	##   5 B 0.00 ##  add r2, c3, r2
	##   6        ##  mul r3, v0.x, c4
	##   7 B 0.00 ##  mad r3, v0.y, c5, r3
	##   8 B 0.00 ##  mad r3, v0.z, c6, r3
	##   9 B 0.00 ##  add r3, c7, r3
	##  10        ##  mul r4.xyz, v2.x, c0.xyz
	##  11 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##  12 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##  13        ##  mul r5.xyz, v2.x, c4.xyz
	##  14 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  15 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  16        ##  mul r2.xyz, r2.xyz, r11.w
	##  17        ##  mul r4.xyz, r4.xyz, r11.w
	##  18        ##  mad r2.xyz, r3.xyz, r1.x, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r1.x, r4.xyz
	##  20        ##  mul r3, r2.x, c40
	##  21 B 0.00 ##  mad r3, r2.y, c41, r3
	##  22 B 0.00 ##  mad r3, r2.z, c42, r3
	##  23 B 0.00 ##  mad r3, r2.w, c43, r3
	##  24 G 0.83 ##  mul r3.x, r3.x, c83.w
	##  25        ##  mul r3.y, r3.y, -c83.w
	##  26 D 0.40 ##  mad oT0.xy, c84.xy, r3.w, r3.xy
	##  27        ##  mad oT1.xy, c85.xy, r3.w, r3.xy
	##  28        ##  mad oT2.xy, c86.xy, r3.w, r3.xy
	##  29        ##  mov oT0.w, r3.w
	##  30        ##  dp3 r4.w, r4, r4
	##  31        ##  mov oT1.w, r3.w
	##  32        ##  rsq r1.x, r4.w
	##  33        ##  mov oT2.w, r3.w
	##  34        ##  mul r4, r4, r1.x
	##  35        ##  mov oT3.w, r3.w
	##  36        ##  rcp r1.x, c45.w
	##  37        ##  dp3 r5.x, r4, c46
	##  38        ##  add r5.w, c45.w, -r3.w
	##  39        ##  max r5.x, r5.x, c80
	##  40        ##  mul r5.w, r5.w, r1.x
	##  41        ##  mul r3, r2.x, c32
	##  42 B 0.00 ##  mad r3, r2.y, c33, r3
	##  43 B 0.00 ##  mad r3, r2.z, c34, r3
	##  44 B 0.00 ##  mad r3, r2.w, c35, r3
	##  45 G 0.83 ##  mov oPos, r3
	##  46        ##  mul r0.x, r3.w, c69.x
	##  47 B 0.00 ##  add oFog.x, c69.y, r0.x
	##  48        ##  mul r5.w, r5.x, r5.w
	##  49 B 0.00 ##  mul r5.xyz, c83.y, r5.w
	##  50        ##  mul r5.w, r5.w, c83.y
	##  51        ##  mul oD0.xyz, c44.xyz, r5.xyz
	##  52        ##  mul oD0.w, c44.w, r5.w
	##  53        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  54 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 55 slots, 31.56 cycles, cpi = 0.573818
*/

static const	char	vshh_wshdw_wt_fvf[] =
	"vs.1.1 "
	"add r1.x,    c[81].x,     -v0.w "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul r2.xyz, r2.xyz, v0.w "
	"mul r4.xyz, r4.xyz, v0.w "
	"mad r2.xyz, r3.xyz, r1.x, r2.xyz "
	"mad r4.xyz, r5.xyz, r1.x, r4.xyz "
	"mul r3, r2.x, c[ 40 + 0 ] "
	"mad r3, r2.y, c[ 40 + 1 ], r3 "
	"mad r3, r2.z, c[ 40 + 2 ], r3 "
	"mad r3, r2.w, c[ 40 + 3 ], r3 "
	"mul r3.x, r3.x, c[ 83 ].w "
	"mul r3.y, r3.y, -c[ 83 ].w "
	"mad oT0.xy, c[ 84 ].xy, r3.w, r3.xy "
	"mad oT1.xy, c[ 85 ].xy, r3.w, r3.xy "
	"mad oT2.xy, c[ 86 ].xy, r3.w, r3.xy "
	"mov oT0.w, r3.w "
	"dp3 r4.w, r4, r4 "
	"mov oT1.w, r3.w "
	"rsq r1.x, r4.w "
	"mov oT2.w, r3.w "
	"mul r4, r4, r1.x "
	"mov oT3.w, r3.w "
	"rcp r1.x, c[ 44 + 1 ].w "
	"dp3 r5.x, r4, c[ 44 + 2 ] "
	"add r5.w, c[ 44 + 1 ].w, -r3.w "
	"max r5.x, r5.x, c[ 80 ] "
	"mul r5.w, r5.w, r1.x "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"mov oPos, r3 "
	"mul r0.x, r3.w, c[ 69 ].x "
	"add oFog.x, c[ 69 ].y, r0.x "
	"mul r5.w, r5.x, r5.w "
	"mul r5.xyz, c[ 83 ].y, r5.w "
	"mul r5.w, r5.w, c[ 83 ].y "
	"mul oD0.xyz, c[44 + 0].xyz, r5.xyz "
	"mul oD0.w, c[44 + 0].w, r5.w "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 54
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  add r1.x, c81.x, -v0.w
	##   1        ##  mul r2, v0.x, c0
	##   2 B 0.00 ##  mad r2, v0.y, c1, r2
	##   3 B 0.00 ##  mad r2, v0.z, c2, r2
	##   4 B 0.00 ##  add r2, c3, r2
	##   5        ##  mul r3, v0.x, c4
	##   6 B 0.00 ##  mad r3, v0.y, c5, r3
	##   7 B 0.00 ##  mad r3, v0.z, c6, r3
	##   8 B 0.00 ##  add r3, c7, r3
	##   9        ##  mul r4.xyz, v2.x, c0.xyz
	##  10 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##  11 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##  12        ##  mul r5.xyz, v2.x, c4.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  14 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  15        ##  mul r2.xyz, r2.xyz, v0.w
	##  16        ##  mul r4.xyz, r4.xyz, v0.w
	##  17        ##  mad r2.xyz, r3.xyz, r1.x, r2.xyz
	##  18        ##  mad r4.xyz, r5.xyz, r1.x, r4.xyz
	##  19        ##  mul r3, r2.x, c40
	##  20 B 0.00 ##  mad r3, r2.y, c41, r3
	##  21 B 0.00 ##  mad r3, r2.z, c42, r3
	##  22 B 0.00 ##  mad r3, r2.w, c43, r3
	##  23 G 0.83 ##  mul r3.x, r3.x, c83.w
	##  24        ##  mul r3.y, r3.y, -c83.w
	##  25 D 0.40 ##  mad oT0.xy, c84.xy, r3.w, r3.xy
	##  26        ##  mad oT1.xy, c85.xy, r3.w, r3.xy
	##  27        ##  mad oT2.xy, c86.xy, r3.w, r3.xy
	##  28        ##  mov oT0.w, r3.w
	##  29        ##  dp3 r4.w, r4, r4
	##  30        ##  mov oT1.w, r3.w
	##  31        ##  rsq r1.x, r4.w
	##  32        ##  mov oT2.w, r3.w
	##  33        ##  mul r4, r4, r1.x
	##  34        ##  mov oT3.w, r3.w
	##  35        ##  rcp r1.x, c45.w
	##  36        ##  dp3 r5.x, r4, c46
	##  37        ##  add r5.w, c45.w, -r3.w
	##  38        ##  max r5.x, r5.x, c80
	##  39        ##  mul r5.w, r5.w, r1.x
	##  40        ##  mul r3, r2.x, c32
	##  41 B 0.00 ##  mad r3, r2.y, c33, r3
	##  42 B 0.00 ##  mad r3, r2.z, c34, r3
	##  43 B 0.00 ##  mad r3, r2.w, c35, r3
	##  44 G 0.83 ##  mov oPos, r3
	##  45        ##  mul r0.x, r3.w, c69.x
	##  46 B 0.00 ##  add oFog.x, c69.y, r0.x
	##  47        ##  mul r5.w, r5.x, r5.w
	##  48 B 0.00 ##  mul r5.xyz, c83.y, r5.w
	##  49        ##  mul r5.w, r5.w, c83.y
	##  50        ##  mul oD0.xyz, c44.xyz, r5.xyz
	##  51        ##  mul oD0.w, c44.w, r5.w
	##  52        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  53 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	D: shadow-stall from using a register two instructions after it stalled
	G: stall from stalling immediately after using a bypass
	//    microcode: 54 slots, 29.56 cycles, cpi = 0.547407
*/

static const	char	vshh_wshdw_wt_1tex[] =
	"vs.1.1 "
	"mul r11.w, c[ 83 ].z, v0.w "
	"add r1.x,    c[81].x,     -r11.w "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul r2.xyz, r2.xyz, r11.w "
	"mul r4.xyz, r4.xyz, r11.w "
	"mad r2.xyz, r3.xyz, r1.x, r2.xyz "
	"mad r4.xyz, r5.xyz, r1.x, r4.xyz "
	"mul r3, r2.x, c[ 40 + 0 ] "
	"mad r3, r2.y, c[ 40 + 1 ], r3 "
	"mad r3, r2.z, c[ 40 + 2 ], r3 "
	"mad r3, r2.w, c[ 40 + 3 ], r3 "
	"mul r3.x, r3.x, c[ 83 ].w "
	"mul r3.y, r3.y, -c[ 83 ].w "
	"dp3 r4.w, r4, r4 "
	"mad oT0.xy, c[ 84 ].xy, r3.w, r3.xy "
	"rsq r1.x, r4.w "
	"mov oT0.w, r3.w "
	"mul r4, r4, r1.x "
	"rcp r1.x, c[ 44 + 1 ].w "
	"dp3 r5.x, r4, c[ 44 + 2 ] "
	"add r5.w, c[ 44 + 1 ].w, -r3.w "
	"max r5.x, r5.x, c[ 80 ] "
	"mul r5.w, r5.w, r1.x "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"mov oPos, r3 "
	"mul r0.x, r3.w, c[ 69 ].x "
	"add oFog.x, c[ 69 ].y, r0.x "
	"mul r5.w, r5.x, r5.w "
	"mul r5.xyz, c[ 83 ].y, r5.w "
	"mul r5.w, r5.w, c[ 83 ].y "
	"mul oD0.xyz, c[44 + 0].xyz, r5.xyz "
	"mul oD0.w, c[44 + 0].w, r5.w "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 50
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r11.w, c83.z, v0.w
	##   1 A 1.50 ##  add r1.x, c81.x, -r11.w
	##   2        ##  mul r2, v0.x, c0
	##   3 B 0.00 ##  mad r2, v0.y, c1, r2
	##   4 B 0.00 ##  mad r2, v0.z, c2, r2
	##   5 B 0.00 ##  add r2, c3, r2
	##   6        ##  mul r3, v0.x, c4
	##   7 B 0.00 ##  mad r3, v0.y, c5, r3
	##   8 B 0.00 ##  mad r3, v0.z, c6, r3
	##   9 B 0.00 ##  add r3, c7, r3
	##  10        ##  mul r4.xyz, v2.x, c0.xyz
	##  11 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##  12 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##  13        ##  mul r5.xyz, v2.x, c4.xyz
	##  14 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  15 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  16        ##  mul r2.xyz, r2.xyz, r11.w
	##  17        ##  mul r4.xyz, r4.xyz, r11.w
	##  18        ##  mad r2.xyz, r3.xyz, r1.x, r2.xyz
	##  19        ##  mad r4.xyz, r5.xyz, r1.x, r4.xyz
	##  20        ##  mul r3, r2.x, c40
	##  21 B 0.00 ##  mad r3, r2.y, c41, r3
	##  22 B 0.00 ##  mad r3, r2.z, c42, r3
	##  23 B 0.00 ##  mad r3, r2.w, c43, r3
	##  24 G 0.83 ##  mul r3.x, r3.x, c83.w
	##  25        ##  mul r3.y, r3.y, -c83.w
	##  26        ##  dp3 r4.w, r4, r4
	##  27        ##  mad oT0.xy, c84.xy, r3.w, r3.xy
	##  28        ##  rsq r1.x, r4.w
	##  29        ##  mov oT0.w, r3.w
	##  30        ##  mul r4, r4, r1.x
	##  31        ##  rcp r1.x, c45.w
	##  32        ##  dp3 r5.x, r4, c46
	##  33        ##  add r5.w, c45.w, -r3.w
	##  34        ##  max r5.x, r5.x, c80
	##  35        ##  mul r5.w, r5.w, r1.x
	##  36        ##  mul r3, r2.x, c32
	##  37 B 0.00 ##  mad r3, r2.y, c33, r3
	##  38 B 0.00 ##  mad r3, r2.z, c34, r3
	##  39 B 0.00 ##  mad r3, r2.w, c35, r3
	##  40 G 0.83 ##  mov oPos, r3
	##  41        ##  mul r0.x, r3.w, c69.x
	##  42 B 0.00 ##  add oFog.x, c69.y, r0.x
	##  43        ##  mul r5.w, r5.x, r5.w
	##  44 B 0.00 ##  mul r5.xyz, c83.y, r5.w
	##  45        ##  mul r5.w, r5.w, c83.y
	##  46        ##  mul oD0.xyz, c44.xyz, r5.xyz
	##  47        ##  mul oD0.w, c44.w, r5.w
	##  48        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  49 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	G: stall from stalling immediately after using a bypass
	//    microcode: 50 slots, 28.66 cycles, cpi = 0.5732
*/

static const	char	vshh_wshdw_wt_fvf_1tex[] =
	"vs.1.1 "
	"add r1.x,    c[81].x,     -v0.w "
	"mul r2, v0.x, c[ 0 + 0 ] "
	"mad r2, v0.y, c[ 0 + 1 ], r2 "
	"mad r2, v0.z, c[ 0 + 2 ], r2 "
	"add r2, c[ 0 + 3 ], r2 "
	"mul r3, v0.x, c[ 4 + 0 ] "
	"mad r3, v0.y, c[ 4 + 1 ], r3 "
	"mad r3, v0.z, c[ 4 + 2 ], r3 "
	"add r3, c[ 4 + 3 ], r3 "
	"mul r4.xyz, v2.x, c[ 0 + 0 ].xyz "
	"mad r4.xyz, v2.y, c[ 0 + 1 ].xyz, r4.xyz "
	"mad r4.xyz, v2.z, c[ 0 + 2 ].xyz, r4.xyz "
	"mul r5.xyz, v2.x, c[ 4 + 0 ].xyz "
	"mad r5.xyz, v2.y, c[ 4 + 1 ].xyz, r5.xyz "
	"mad r5.xyz, v2.z, c[ 4 + 2 ].xyz, r5.xyz "
	"mul r2.xyz, r2.xyz, v0.w "
	"mul r4.xyz, r4.xyz, v0.w "
	"mad r2.xyz, r3.xyz, r1.x, r2.xyz "
	"mad r4.xyz, r5.xyz, r1.x, r4.xyz "
	"mul r3, r2.x, c[ 40 + 0 ] "
	"mad r3, r2.y, c[ 40 + 1 ], r3 "
	"mad r3, r2.z, c[ 40 + 2 ], r3 "
	"mad r3, r2.w, c[ 40 + 3 ], r3 "
	"mul r3.x, r3.x, c[ 83 ].w "
	"mul r3.y, r3.y, -c[ 83 ].w "
	"dp3 r4.w, r4, r4 "
	"mad oT0.xy, c[ 84 ].xy, r3.w, r3.xy "
	"rsq r1.x, r4.w "
	"mov oT0.w, r3.w "
	"mul r4, r4, r1.x "
	"rcp r1.x, c[ 44 + 1 ].w "
	"dp3 r5.x, r4, c[ 44 + 2 ] "
	"add r5.w, c[ 44 + 1 ].w, -r3.w "
	"max r5.x, r5.x, c[ 80 ] "
	"mul r5.w, r5.w, r1.x "
	"mul r3, r2.x, c[ 32 + 0 ] "
	"mad r3, r2.y, c[ 32 + 1 ], r3 "
	"mad r3, r2.z, c[ 32 + 2 ], r3 "
	"mad r3, r2.w, c[ 32 + 3 ], r3 "
	"mov oPos, r3 "
	"mul r0.x, r3.w, c[ 69 ].x "
	"add oFog.x, c[ 69 ].y, r0.x "
	"mul r5.w, r5.x, r5.w "
	"mul r5.xyz, c[ 83 ].y, r5.w "
	"mul r5.w, r5.w, c[ 83 ].y "
	"mul oD0.xyz, c[44 + 0].xyz, r5.xyz "
	"mul oD0.w, c[44 + 0].w, r5.w "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 49
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  add r1.x, c81.x, -v0.w
	##   1        ##  mul r2, v0.x, c0
	##   2 B 0.00 ##  mad r2, v0.y, c1, r2
	##   3 B 0.00 ##  mad r2, v0.z, c2, r2
	##   4 B 0.00 ##  add r2, c3, r2
	##   5        ##  mul r3, v0.x, c4
	##   6 B 0.00 ##  mad r3, v0.y, c5, r3
	##   7 B 0.00 ##  mad r3, v0.z, c6, r3
	##   8 B 0.00 ##  add r3, c7, r3
	##   9        ##  mul r4.xyz, v2.x, c0.xyz
	##  10 B 0.00 ##  mad r4.xyz, v2.y, c1.xyz, r4.xyz
	##  11 B 0.00 ##  mad r4.xyz, v2.z, c2.xyz, r4.xyz
	##  12        ##  mul r5.xyz, v2.x, c4.xyz
	##  13 B 0.00 ##  mad r5.xyz, v2.y, c5.xyz, r5.xyz
	##  14 B 0.00 ##  mad r5.xyz, v2.z, c6.xyz, r5.xyz
	##  15        ##  mul r2.xyz, r2.xyz, v0.w
	##  16        ##  mul r4.xyz, r4.xyz, v0.w
	##  17        ##  mad r2.xyz, r3.xyz, r1.x, r2.xyz
	##  18        ##  mad r4.xyz, r5.xyz, r1.x, r4.xyz
	##  19        ##  mul r3, r2.x, c40
	##  20 B 0.00 ##  mad r3, r2.y, c41, r3
	##  21 B 0.00 ##  mad r3, r2.z, c42, r3
	##  22 B 0.00 ##  mad r3, r2.w, c43, r3
	##  23 G 0.83 ##  mul r3.x, r3.x, c83.w
	##  24        ##  mul r3.y, r3.y, -c83.w
	##  25        ##  dp3 r4.w, r4, r4
	##  26        ##  mad oT0.xy, c84.xy, r3.w, r3.xy
	##  27        ##  rsq r1.x, r4.w
	##  28        ##  mov oT0.w, r3.w
	##  29        ##  mul r4, r4, r1.x
	##  30        ##  rcp r1.x, c45.w
	##  31        ##  dp3 r5.x, r4, c46
	##  32        ##  add r5.w, c45.w, -r3.w
	##  33        ##  max r5.x, r5.x, c80
	##  34        ##  mul r5.w, r5.w, r1.x
	##  35        ##  mul r3, r2.x, c32
	##  36 B 0.00 ##  mad r3, r2.y, c33, r3
	##  37 B 0.00 ##  mad r3, r2.z, c34, r3
	##  38 B 0.00 ##  mad r3, r2.w, c35, r3
	##  39 G 0.83 ##  mov oPos, r3
	##  40        ##  mul r0.x, r3.w, c69.x
	##  41 B 0.00 ##  add oFog.x, c69.y, r0.x
	##  42        ##  mul r5.w, r5.x, r5.w
	##  43 B 0.00 ##  mul r5.xyz, c83.y, r5.w
	##  44        ##  mul r5.w, r5.w, c83.y
	##  45        ##  mul oD0.xyz, c44.xyz, r5.xyz
	##  46        ##  mul oD0.w, c44.w, r5.w
	##  47        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  48 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	B: bypass (no stall)
	C: standard stall
	G: stall from stalling immediately after using a bypass
	//    microcode: 49 slots, 26.66 cycles, cpi = 0.544082
*/

static const	char	vshh_vs_radar[] =
	"vs.1.1 "
	"add		r2.xyz, v0, c[ 3 ] "
	"mul		r3, r2.x, c[ 4 + 0 ] "
	"mad		r3, r2.y, c[ 4 + 1 ], r3 "
	"mad		r3, r2.z, c[ 4 + 2 ], r3 "
	"add		r3, c[ 4 + 3 ], r3 "
	"mad		r4.z, v0.w, c[ 4 + 1 ].z, r3.z "
	"slt		r5.xyzw, r3.z, c[ 1 ] "
	"slt		r6.xyzw, -c[ 1 ], r4.z "
	"mul		r7.xyzw, r5, r6 "
	"sub		r8, c[ 1 ], r7 "
	"mov		oPos.xy, r3.xy "
	"mov		oPos.w, c[ 1 ] "
	"mov		oPos.z, r7 "
	"mul		r9, r7, c[ 16 ] "
	"mad		oD0.xyz, r8, c[ 17 ], r9 "
	"mov		oD0.w, c[ 1 ] "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 18
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  add r2.xyz, v0, c3
	##   1 A 1.50 ##  mul r3, r2.x, c4
	##   2 B 0.00 ##  mad r3, r2.y, c5, r3
	##   3 B 0.00 ##  mad r3, r2.z, c6, r3
	##   4 B 0.00 ##  add r3, c7, r3
	##   5 B 0.00 ##  mad r4.z, v0.w, c5.z, r3.z
	##   6        ##  slt r5, r3.z, c1
	##   7        ##  slt r6, -c1, r4.z
	##   8 B 0.00 ##  mul r7, r5, r6
	##   9 B 0.00 ##  add r8, c1, -r7
	##  10        ##  mov oPos.xy, r3.xy
	##  11        ##  mov oPos.w, c1
	##  12        ##  mov oPos.z, r7
	##  13        ##  mul r9, r7, c16
	##  14 B 0.00 ##  mad oD0.xyz, r8, c17, r9
	##  15        ##  mov oD0.w, c1
	##  16        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##  17 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 18 slots, 11 cycles, cpi = 0.611111
*/

static const	char	vshh_wvs_radar2[] =
	"vs.1.1 "
	"mul		r3, v0.x, c[ 4 + 0 ] "
	"mad		r3, v0.y, c[ 4 + 1 ], r3 "
	"mad		r3, v0.z, c[ 4 + 2 ], r3 "
	"add		r3, c[ 4 + 3 ], r3 "
	"mov		oPos.w, c[ 1 ] "
	"mov		oPos.xy, r3.xy "
	"mov		oPos.z, v0.y "
	"mov		oD0, v3 "
	;
/*
	// Xbox Shader Assembler 1.00.4531.1
	// instructions: 10
	xvs.1.1
	#pragma screenspace
	##slot stall ##
	##   0        ##  mul r3, v0.x, c4
	##   1 A 1.50 ##  mad r3, v0.y, c5, r3
	##   2 B 0.00 ##  mad r3, v0.z, c6, r3
	##   3 B 0.00 ##  add r3, c7, r3
	##   4        ##  mov oPos.w, c1
	##   5        ##  mov oPos.xy, r3.xy
	##   6        ##  mov oPos.z, v0.y
	##   7        ##  mov oD0, v3
	##   8        ##  mul oPos.xyz, r12, c-38
	                 +   rcc r1.x, r12.w
	##   9 C 0.50 ##  mad oPos.xyz, r12, r1.x, c-37
	Stall Types:
	A: register initialization (first two instructions only)
	B: bypass (no stall)
	C: standard stall
	//    microcode: 10 slots, 7 cycles, cpi = 0.7
*/

