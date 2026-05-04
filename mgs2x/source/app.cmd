/* SCEI CONFIDENTIAL
 "PlayStation2" Programmer Tool Runtime Library  Release 1.1
 */
/*
 *                      Emotion Engine Library
 *                          Version 0.10
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libkernel - app.cmd
 *                        kernel libraly
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.11            May.10.1999     horikawa    
 *      1.10            Oct.12.1999     horikawa   add .stack section
 */

/*
	for MetalGearSolid2
		1999/12/02 K.Uehara		HEAPSIZE set in makefile.def
		2000/02/07 K.Uehara		STACK_SIZE, STACK_TOP set in makefile.def
		$Id: app.cmd,v 1.1.1.3 2002/11/19 11:41:40 Yoshizawa1 Exp $
*/

_stack		= STACK_TOP;
_stack_size = STACK_SIZE;
_heap_size = HEAPSIZE;

GROUP( -lm -lc -lkernl -lgcc )
ENTRY(ENTRYPOINT)
SECTIONS {
/*	.indata		0x00100000: { *(.indata) }*/

	. = 0x00100000;

	_program_top = .;
	.text : {
		_compiled_time = .;
		LONG( _COMPILE_DATE )
		crt0.o(.text)
		_text_top = .;
		*(.text)
		_text_end = .;
		QUAD(0)
	}

	.data		ALIGN(128): { *(.data) }
	.rodata		ALIGN(128): { *(.rodata) }
	.rdata		ALIGN(128): { *(.rdata) }
	.gcc_except_table ALIGN(128): { *(.gcc_except_table) }

	_gp =		ALIGN(128) + 0x7ff0;

	.lit8       ALIGN(128): { *(.lit8) }
	.lit4       ALIGN(128): { *(.lit4) }
	.sdata		ALIGN(128): { *(.sdata) }

	_fbss =		ALIGN(128);	/* clear start address ( in crt0.o ) */

	.sbss		ALIGN(128): { *(.sbss) *(.scommon) }
	.bss		ALIGN(128): { *(.bss) }

	.reginfo			  : { KEEP(*(.reginfo)) }

	end = .;
	_end = .;		/* HEAP START ADDRESS ( in crt0.o ) */

	. += _heap_size;
	. = ALIGN(128);
	_mgs2_keep_end = .;

	.spad		0x70000000: {
		 crt0.o(.spad)
		 *(.spad)
	}
	_32M_end	= 0x02000000;
	_128M_end	= 0x08000000;
}
