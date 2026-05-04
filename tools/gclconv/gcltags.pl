#!/usr/bin/perl -w
# gclをパースして定義部を抜き出す。

use strict;
use English;

use vars qw( @files );
use vars qw( $out_format );

sub output_line( $ $ $ $ $ )
{
	my $pat = shift;
	my $tag = shift;
	my $file = shift;
	my $line_no = shift;
	my $offset = shift;

	my $result;

	if( $out_format == 0 ){
		$result = $pat . "\c?" . $tag. "\cA" . $line_no . "," . $offset . "\n";
	} elsif( $out_format == 1 ){
		$result = "$file($line_no) : $pat\n"
	}

	return $result;
}

sub parse_files( $ $ $ ) {
	my $out = shift;			# output handle
	my $file = shift;			# filename
	my $flag = shift;			# 0/1 でcommand.defかどうか。

	my $line_no = 0;
	my $offset = 0;
	my $status = 0;

	my $outstr = "";

	open( IN, $file ) or return;

	# 共通のパース

	while( my $line = <IN> ){
		$line_no ++;
		if( $status == 0 ){
			# 通常パーズ
			if( $line =~ /\s*(\#define|\#enum|enum|resource)\s+(\S+)/ ){
				# define 
				if( ( $1 eq "\#enum" || $1 eq "enum" ) && ( $2 eq "{" ) ){
					$status = 1;
				} else {
					$outstr .= output_line( $MATCH, $2, $file, $line_no, $offset );
				}
			} elsif( $line =~ /\s*\#include\s+\"([^\"]+)\"/ ){
				# include
				push @files, $1;
			} elsif( $flag == 0 ){
				# 通常gcl
				if( ( $line =~ /\s*proc\s+([^\{ ]+?)\s*[\{|\s+]/ )
				   || ( $line =~ /\s*block\s+\S+\s+(\S+)/ ) ){
					# proc
					$outstr .= output_line( $MATCH, $1, $file, $line_no, $offset );
				}
			} elsif( $flag == 1 ){
				if( $line =~ /\s*(chara|mesg|command)\s+([^\[]+?)\s*\[/ ){
					$outstr .= output_line( $MATCH, $2, $file, $line_no, $offset );
				}
			}
		} elsif( $status == 1 ){
			# enumのなか
			if( $line =~ /\s*([^=\s]+)\s*=/ ){
				$outstr .= output_line( $MATCH, $1, $file, $line_no, $offset );
			}
			if( $line =~ /\}/ ){
				$status =  0;
			}
		}
		$offset = tell( IN );
	}

	close( IN );

	$file =~ s/\/cygdrive\/([a-z])/$1:/;

	if( $out_format == 0 ){
		# emacs TAG
		print $out "\cL","\n";
		print $out $file, ",", length( $outstr ), "\n";
	} elsif( $out_format == 1 ){
		# HIDEMARU TAG
	}
	print $out $outstr;
}

# ------------------ MAIN

my $outfile = "-";

$out_format = 0;

while( @ARGV && ( $_ = $ARGV[ 0 ] ), /^-/ ){
	shift( @ARGV );
	if( /^-o/ ){
		$outfile = shift( @ARGV ) or die "Set Output File Name.";
	}
	if( /^-H/ ){
		# hidemaru
		$out_format = 1;
	}
	if( /^-e/ ){
		# etag
		$out_format = 0;
	}
}

if( $#ARGV >= 0 ){
	push @files, @ARGV;
	my %done;

	open( OUT, ">".$outfile );

	parse_files( \*OUT, "command.def", 1 );
	while( my $f = shift @files ){
		if( ! exists( $done{ $f } ) ){
			parse_files( \*OUT, $f, 0 );
			$done{ $f } = 1;
		}
	}

	close( OUT );
} else {
	print $0," <gclfile> ...\n";
}
