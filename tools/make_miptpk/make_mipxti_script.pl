#!/usr/bin/perl

if ( @ARGV == 0 ){
	print "Make mipmap xti \n" ;
	print "Usage: make_mipxti_script.pl xti_name <input bmp file> \n" ;
	exit 0 ;
}

{
	my $tpk_name ;
	my $xti_name ;
	my $option_flag ;
	my @tpk_list ;
	my $work_dir ;

	$xti_name = "mipxti.xti" ;
	$option_flag = 0 ;
	$work_dir = "./" ;
	foreach $target ( @ARGV ){
	    if ( $option_flag == 0 ){
		# オプションチェック
		if ( $target eq "-o" ){
		    $option_flag = 1 ;
		} elsif ( $target eq "-l" ){
		    $option_flag = 2 ;
		} elsif ( $target eq "-i" ){
		    $option_flag = 0 ;
		} else {
		    $tpk_name = $target ;
		    $tpk_name =~ s/.*\/([^\/]*)$/$1/ ;
		    $tpk_name =~ s/.bmp$// ;
		    print "makexti6 -o $work_dir$tpk_name -i $target -f DXT3\n" ;
		    system( "makexti6 -o $work_dir$tpk_name -i $target -f DXT3" );
		    @tpk_list = ( @tpk_list, $work_dir.$tpk_name."_0.tpk" );
		}
	    } else {
		$xti_name = $target if ( $option_flag == 1 ) ;
		$work_dir = $target if ( $option_flag == 2 ) ;
		$xti_name =~ s/\.tri$/.xti/g ;
		$work_dir .= "/" ;
		$work_dir =~ s/\/\//\//g ;
		$option_flag = 0 ;
	    }
	}
	print "make_mipxti -o $work_dir$xti_name -i @tpk_list\n" ;
	system( "make_mipxti -o $work_dir$xti_name -i @tpk_list" );
	print "rm -f @tpk_list\n" ;
	system( "rm -f @tpk_list" );
}

