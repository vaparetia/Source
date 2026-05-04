$Id: readme.txt,v 1.1 2001/12/16 08:54:13 usr01363 Exp $

CV2ファイルから、指定のOBJECT(描画単位)を抜きだし、新たなCV2ファイルを
作成します。

    arcvd outputfile(*.kms) [inputfile0(*.kms) -n objnum0 -n objnum1 ...] [inputfile1(*.kms) -n objnum0 -n objnum1 ...] ... \n" ) ;

	オプション:
		-n objnum		指定のKMSファイルの、objnum番目(0からのインデックス)のOBJECTを取り出します。
		-a				指定のKMSファイルに含まれる全てのファイルを出力します。
    	-u objnum		指定のKMSファイルの、objnum番目(0からのインデックス)から最後までの
						OBJECTを取り出します。
    	-l objnum		指定のKMSファイルの、最初からobjnum番目(0からのインデックス)までの
						OBJECTを取り出します。
		-b objnum0 objnum1
						指定のKMSファイルの、objnum0番目(0からのインデックス)から
						objnum1番目(0からのインデックス)までのOBJECTを取り出します。

	inputfileとオプションの組み合わせで、inputfile内のOBJECTの指定を行います。オプションは
	一つのinputfileに対しいくつでも構いません。また、inputfileとオプションの組み合わせも
	複数個指定できます。
