/*
	head.s
		ステージバイナリヘッダ
	2000/10/19	K.Uehara
	$Id: binhead.s,v 1.3 2002/02/01 04:43:17 usr01475 Exp $
*/
	.set nomips16

	.global StageBinHeader

	.xref _mgs2_keep_end
	.xref _StageCharacterEntries
	.xref _stage_bss_top
	.xref _stage_bin_end

	.data
StageBinHeader:
	.word _mgs2_keep_end
	.word _stage_bss_top
	.word _stage_bin_end
	.word _StageCharacterEntries
