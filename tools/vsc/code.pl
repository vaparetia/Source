#!/usr/bin/perl
#
# 全ソースの漢字コードを変換
#
# ./code.pl sjis   全て Shift_JIS に変換
# ./code.pl euc    全て EUC-JP に変換
#

open LIST, "find ./ | grep '\\.[ch]\$' |";

@list = <LIST>;

if($ARGV[0] eq "sjis") { $opt = "-s"; }
elsif($ARGV[0] eq "euc") { $opt = "-e"; }
else {
    print STDERR "usage: ./code.pl sjis|euc\n";
    exit 1;
}

foreach $fname (@list) {
    $fname =~ s/\n$//g;
    $cmd = "nkf $opt < $fname > /tmp/code.tmp ; mv /tmp/code.tmp $fname";
    system $cmd;
}

1;
