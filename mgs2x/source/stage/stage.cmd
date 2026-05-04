GROUP( -lm -lc -lkernl -lgcc )
SECTIONS {
	. = _mgs2_keep_end;
	.data		ALIGN(128): { *(.data) }
	.text		ALIGN(128): { *(.text); QUAD(0); }
	.rodata		ALIGN(128): { *(.rodata) }
	.rdata		ALIGN(128): { *(.rdata) }
	_stage_bss_top = .;
	.bss		ALIGN(128): { *(.bss) }
	_stage_bin_end = .;
	stage_bin_end = .;
}
