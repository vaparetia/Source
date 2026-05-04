#!/bin/perl -w

use strict;

use vars qw( %chara );

# おのおののstage.cファイルからcharaのリストを取得する

sub read_stage_file( $ ) {
	my $file = shift @_;

	open( FILE, $file ) || die "not found $file\n";
	while( <FILE> ){
		if( /\s+{ ([0-9A-Fx]+), (\w+) },/ ){
			$chara{ $1 } = $2;
		}
	}
	close( FILE );
}

# charaのリストから_stage.cを作成する

sub output_list( $ ) {
	my $file = shift @_;
	my $state = 0;

	open( IN, $file );
	while( my $line = <IN> ){
		if( $state == 0 || $state == 2 ){
			print $line;
			if ( $line =~ /\/\* gcl chara \*\// ){
				$state ++;
			}
		} elsif( $state == 1 ){
			foreach my $ex ( values( %chara ) ){
				printf( "extern NEWCHARA %s;\n", $ex );
			}
			while( ( $line = <IN> ) !~ /\/\* gcl chara end \*\// ){
				;
			}
			print $line;
			$state = 2;
		} elsif( $state == 3 ){
			foreach my $key ( sort( keys( %chara ) ) ){
				printf( "\t{ %s, %s },\n", $key, $chara{ $key } );
			}
			while( ( $line = <IN> ) !~ /\/\* gcl chara end \*\// ){
				;
			}
			print $line;
			$state = 0;
		}
	}
	close( IN );
}

# ------ MAIN

foreach my $i ( 0 ... $#ARGV ){
	read_stage_file( $ARGV[ $i ] );
}

output_list $ARGV[ 0 ];

