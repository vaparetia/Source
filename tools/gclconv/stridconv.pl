#!/usr/bin/perl -w
# strid 変換を行う perl

use strict;

use vars qw( $bit );

$bit = 24;

sub strid( $ ) {
	my $str = shift @_;
	my $code = 0;
	my $mask = ( 1 << $bit ) - 1;
	
	for my $i ( 0...length($str) ){
		my $c = ord( substr( $str, $i, 1 ) );
		last if( $c == 0 );
		$code = ( $code << 5 ) | ( $code >> ( $bit - 5 ) );
		$code += $c;
		$code &= $mask;
	}
	if( $code == 0 ){
		$code = 1;
	}
	$code;
}

sub fileconv( $ ){
	my $file = shift @_;

	open( IN, $file ) or die "not found $file\n";

	print "//\n";
	print "//\n";
	print "// このファイルは $file からstridconv.plによって生成されたものです。\n";
	print "// 修正するときは $file を修正してください。\n";
	print "//\n";
	print "//\n";
	print "//\n";

	while( <IN> ){
		s/\$\$\s*STRID\s*\(\s*\"([^\"]*)\"\s*\)\s*\$\$/&strid($1)/eg;
		print $_;
	}
	close( IN );
}

# --------------------------------------------- main

if( $#ARGV >= 1 ){
	$bit = $ARGV[ 1 ];
}

if( $#ARGV >= 0 ){
	fileconv $ARGV[ 0 ];
} else {
	print $0, " <file> [bit]\n";
	print "format is \$\$STRID( \"...\" )\$\$\n";
}
