/*
	hzx_conv : hzm -> hzx コンバートコマンド
		   hzx ブロック分割もできる。

	programed by M.Sonoyama 1999
	$Id: README.txt,v 1.1 1999/11/15 07:08:59 usr02011 Exp $
*/

＜使用方法＞

	hzx_conv -i <inputfile(*.hzm or *.hzx)> [-o <outputfile(*.hzx)] \
			[-d <divide param( x_size y_size z_size )>]

	inputfile : 入力ファイル ( hzx or hzm )

	outputfile : 出力ファイル（省略時は、入力ファイルと同じ名前.hzx）

	divide param : 分割パラメータ ( xサイズ、ｙサイズ、ｚサイズ ）
			分割数ではないので注意。