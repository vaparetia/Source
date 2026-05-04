#!/usr/bin/perl

require "jcode.pl" ;

if($#ARGV != 0){
	print "replace.pl <ext>\n";
	exit 1;
}

# 基本となるファイルとリストファイル
#$filename = $ARGV[0];
$listname = "conv_list_res.txt";

# 置き換え用文字列
$key = "##!##";

@list1 = ();
@list2 = ();
@searchlist = ();
%replacelist = {};

#print "BaseFile = $filename\n";
#print "ListFile = $listname\n";
#print "TmpFile  = $tmpname\n";
#print "BackFile = $backname\n";

# まずリストファイルから日本語文字列と変換用文字列を取り出す
$listcount = 0;
open( LIST, $listname);
foreach $line ( <LIST> ){
	chop $line;

	# euc へ
	&jcode'sjis2euc(*line);

	# 先頭の空白文字を削除
	$line =~ s/^[\s]*//;

	# 空行は飛ばす
	if(!length($line)){
		next;
	}

	# 先頭に#があったら飛ばす
	if($line =~ m/^\#/){
		next;
	}

	# \" を置き換え
	$line =~ s/\\\"/$key/g;

	# "" を置き換え
	$line =~ s/\"\"/$key/g;

	# 必要な文字列のみを取りだす
	$line =~ s/^.*?(\".*?\").*?(\".*?\")/$1 $2/;

	$word1 = $1;
	$word2 = $2;

	$word1 =~ s/^.*?\"//;
	$word1 =~ s/\".*?$//;
#	$word1 =~ s/$key/\\\"/g;

	# もしも空きの文字列がきたら飛ばす
	nect, if(length($word1) == 0);

	# パターンマッチのため、特殊文字にはすべて\をつける
	$word1 =~ s/(\W)/\\\1/g;

	$word2 =~ s/^.*?\"//;
	$word2 =~ s/\".*?$//;
#	$word2 =~ s/$key/\\\"/g;

	# list1 は検索順用
	# replacelist は変換用
	$list1[$listcount] = $word1;
	$replacelist{$word1} = $word2;

	$listcount++;
}
close(LIST);
# 検索用の配列を文字列の長さ順にソートする
@searchlist = sort descend @list1;

# ファイル検索・置換
open( PATH, "find . -name \"*.$ARGV[0]\" |");
foreach $filename ( <PATH> ){
	chop $filename;

	# 一時ファイル
	# ファイル名_tmp番号.拡張子
	$tmpname = $filename;
	$tmpname =~ s/\.$ARGV[0]$//;
	$tmpname .= "_tmp";
	$count = 0;
	while(1){
		last, if( ! -e "$tmpname$count.txt");
		$count++;
	}
	$tmpname .= "$count.txt";

	# バックアップファイル
	# ファイル名.拡張子_back番号
	$backname = $filename;
	$backname .= "_back";
	$count = 0;
	while(1){
		last, if( ! -e "$backname$count");
		$count++;
	}
	$backname .= "$count";

	$convflag = 0;

print "file = $filename\n";
print "back = $backname\n";
print "tmp  = $tmpname\n";

	open(OUT, "> $tmpname");
	open(BASE, $filename);
	foreach $line ( <BASE> ){
		# 重複が無いようにeucへ
		&jcode'sjis2euc(*line);

		if($line =~ m/\"/){
			if($line =~ m/[\x81-\xff]/){

				# \" を置き換え
				$line =~ s/\\\"/$key/g;

				# "" を置き換え
				$line =~ s/\"\"/$key/g;

				for($i = 0; $i < $listcount; $i++){

					if($line =~ m/$searchlist[$i]/){
						$line =~ s/$searchlist[$i]/$replacelist{$searchlist[$i]}/;
						$convflag = 1;
						last;
					}
				}
			}
		}

		$line =~ s/$key/\"\"/g;
#		$line =~ s/$key2/\"\"/g;

		# sjisに戻す
		&jcode'euc2sjis(*line);
		print OUT $line;
	}
	close(BASE);
	close(OUT);

	if($convflag){
		# 置き換えが発生した
		rename $filename, $backname;
		rename $tmpname, $filename;
	}else{
		# 置き換えは無かったので変更なし
		unlink $tmpname;
	}
}
print "Complete!\n";

exit 0;


sub descend {
	length($b) <=> length($a);
}
