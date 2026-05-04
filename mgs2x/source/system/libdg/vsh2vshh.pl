#!/usr/bin/perl
#-------------------------------------------------------------------------------#
#	Vertex Shader のテーブル作成												#
#												2002/08/14 Takaki Eiji			#
#-------------------------------------------------------------------------------#

$XSASM = "xsasm" ;

#-------------------------------------------------------------------------------#
#	引数解析																	#
#-------------------------------------------------------------------------------#

$output_file = shift( ARGV ) ;	# 第一引数が出力

@input_files = () ;
foreach $tag ( @ARGV  )
{
	push(@input_files, $tag) ;	# それ以降の引数は入力とする
}

#-------------------------------------------------------------------------------#
#	メインルーチン																#
#-------------------------------------------------------------------------------#

$tmp1         = "vsh2vshh.tmp1" ;
$tmp2         = "vsh2vshh.tmp2" ;
$tmp_listfile = "vsh2vshh.tmp3" ;

print "[ Genarate ] $output_file\n" ;
open( DST, "> $output_file" ) || die "can't open $output_file\n";

foreach $file ( @input_files  )
{
	print "[ Convert ] $file\n" ;
	if( !(-e $file) )
	{
		print "can't found $file!!\n" ;
		next ;
	}

	$tag = $file ;
	while( $tag =~ /\\/ ){ $tag =~ s/^(.*)\\// ; }	# Path除去
	while( $tag =~ /\./ ){ $tag =~ s/\.(.*)$// ; }	# 拡張子除去

	#-- xsasmのlistfile出力を参考にしたい --------------------------------------#

	execute("$XSASM /nologo /Od /D UVADJUST /D DGOBJ_REVWT $file $tmp1 $tmp2 $tmp_listfile") ;
	#---------------------------------------------------------------------------#

	#-- xsasmの前処理だけ利用 --------------------------------------------------#

	execute("$XSASM /nologo /p /D UVADJUST /D DGOBJ_REVWT $file $tmp1") ;
	#---------------------------------------------------------------------------#

	#-- Header化 ---------------------------------------------------------------#

	open(SRC, $tmp1) || die "can't open $tmp1\n";

	print DST "static const\tchar\tvshh_$tag\[\] =\n" ;

	while( $line = <SRC> )
	{
		#-- コメント等の不要物除去 ---------------------------------------------#

		$line =~ s/\n$// ;	# 終端の改行除去
		while( $line =~ /\#/ ){ $line =~ s/\#.*$// ; }	# "#"以降にあるものは排除
		while( $line =~ /^\s+/ ){ $line =~ s/^\s+// ; }	# 先頭の空白排除
		while( $line =~ /\s+$/ ){ $line =~ s/\s+$// ; }	# 後端の空白排除
		#-----------------------------------------------------------------------#

		next if ( $line =~ /^$/ ) ;		# 空なら処理しない。

		print DST "\t\"$line \"\n" ;
	}
	print DST "\t;\n" ;
	close(SRC) ;
	#---------------------------------------------------------------------------#

	#-- 参考用listfile出力 -----------------------------------------------------#

	open(SRC, $tmp_listfile) || die "can't open $tmp_listfile\n";

	print DST "/\*\n" ;

	while( $line = <SRC> )
	{
		next if( $line =~ /^\s*\n*$/ ) ;
		while( $line =~ /\/\*/ ){ $line =~ s/\/\*/\#\#/ ; }		# コメント記号置換
		while( $line =~ /\*\// ){ $line =~ s/\*\//\#\#/ ; }		# コメント記号置換
		print DST "\t$line" ;
	}
	print DST "\*/\n\n" ;

	close(SRC) ;
	#---------------------------------------------------------------------------#
}

close(DST) ;

if( -e $tmp1 ){ execute("rm $tmp1") ; }
if( -e $tmp2 ){ execute("rm $tmp2") ; }
if( -e $listfile ){ execute("rm $listfile") ; }

#-------------------------------------------------------------------------------#
#	サブルーチン																#
#-------------------------------------------------------------------------------#
sub execute( $ )
{
	my $cmd = shift @_;
	system( $cmd ) ;
}

#-- End Of File --#
