SEテーブルは２５５までが常駐、それ以降は非常駐になる



se_const.sel	/* 全ての常駐SEリスト */
se_stage.sel	/* 全ての非常駐SEリスト */

ＳＥ名 優先 固定モード チャンネル パス


st_????.sel		/* ステージ固有非常駐リスト */
ＳＥ名 （優先 固定モード チャンネル パスは書いてあれば上書きされる）


se_????.lst		/* lkseps2 用のリスト */
優先 固定モード チャンネル パス


st_00001.ztb	/* ステージのＳＥテーブル（バイナリーファイル） */
0:常駐１番目の領域番号
〜
255:常駐２５５番目の領域番号
256:非常駐１番目の領域番号
〜
MAX_SE

makesetbl -i st_???? ... -s se_stage -> se_table.ztb

makesedef	-i ??????.sel -s 全非常駐ＳＥリスト -> ??????.lst

makesehead	-c se_const.sel -s se_stage.sel -> se_defin.h
