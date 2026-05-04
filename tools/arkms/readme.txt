$Id: readme.txt,v 1.1 2001/12/16 08:53:36 usr01363 Exp $

KMSファイルから、指定のOBJECT(描画単位)を抜きだし、新たなKMSファイルを
作成します。

    arkms outputfile(*.kms) [inputfile0(*.kms) -n objnum0 -n objnum1 ...] [inputfile1(*.kms) -n objnum0 -n objnum1 ...] ... \n" ) ;

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



KMS file from the specified OBJECT (drawing units) 抜Kidashi a KMS to create a new file.

    arkms outputfile(*.kms) [inputfile0(*.kms) -n objnum0 -n objnum1 ...] [inputfile1(*.kms) -n objnum0 -n objnum1 ...] ... \n" ) ;

Options:
   -n objnum
         KMS file specified, objnum first (0 index) Remove the OBJECT.
   -a
         KMS specified output file contains all the files.
    -u objnum
         KMS file specified, objnum first (index 0) until the end of the OBJECT remove it.
    -l objnum
         KMS specified files from the first second objnum (index 0) to remove the OBJECT.
-b objnum0 objnum1
         KMS file specified, objnum0 first (index 0) from the second objnum1 (index 0) to remove the OBJECT.

inputfile in combination with the optional, inputfile to designate in the OBJECT.
Can be the number one option for the inputfile. In addition, inputfile can be multiple combinations and options.