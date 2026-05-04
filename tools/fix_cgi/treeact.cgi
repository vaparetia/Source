#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;

if ( $cgi'tags{'act'} eq "掘る" || !$cgi'tags{'act'} || !$cgi'tags{'files'} )
{
    $root_dir = $fix_dir ;
    $root_dir = $runtime_dir if ( $cgi'tags{'directive'} eq "RunTime"   ) ;

    @files = split( ' ', $cgi'tags{'files'} ) ;
    $files[0] =~ s/[\[\]]//g ;
    $cgi'tags{'cur_dir'} .= "$files[0]/"  if ( $files[0] ne "親ディレクトリ" ) ;
    $cgi'tags{'cur_dir'} =~ s/[^\/]+\/$// if ( $files[0] eq "親ディレクトリ" ) ;
    $cgi'tags{'cur_dir'} =~ s/\/\//\// ;

    $tmpfile .= $ENV{'REMOTE_ADDR'}.".2" ;
    open( TMP, ">$tmpfile" ) ;
    print TMP "<OPTION>[親ディレクトリ]</OPTION>\n" ;
    open( LST, "ls -BpX $root_dir/$cgi'tags{'cur_dir'} |" ) ;
    while( <LST> )
    {
	chop ;
	s/(.*)\//[$1]/ ;
	print TMP "<OPTION>$_</OPTION>\n" ;
    }
    close LST ;
    close TMP ;

    $file = "html/tree2_".substr( $cgi'tags{'ext_mode'},0,3 ). ".html" ;
    &prnt'file( "0", $file,
            "comment=$cgi'tags{'comment'}",
            "ext_mode=$cgi'tags{'ext_mode'}",
            "directive=$cgi'tags{'directive'}",
            "cur_dir=$cgi'tags{'cur_dir'}",
            "tmpfile=$tmpfile",
            "showdirs=$showdirs" ) ;
}
else
{
    local $src, $dst, $line ;
    local $srcfile, $dstfile ;
    local @prnt_tag = ( "0", "html/show_head.html",
		       "title=これでよろしいですか？",
		       "cgifile=$cgi'tags{'ext_mode'}2.cgi",
		       "comment=" ) ;
    local $i=0 ;
    foreach ( sort {$b cmp $a} keys( %cgi'tags ) )
    {
	next if ( /method/ ) ;
	$debug .= "($_)" ;
	push( @prnt_tag, "arg_name$i=$_" ) ;
	push( @prnt_tag, "arg_val$i=$cgi'tags{$_}" ) ;
	$i++ ;
    }
    &prnt'file( @prnt_tag ) ;

    foreach( split( / +/, $cgi'tags{'files'} ) )
    {
        print "<TR><TD>$_</TD></TR>\n" ;
    }
    print( "<TR><TD><INPUT TYPE=SUBMIT NAME=method VALUE=実行></TD></TR>\n" ) ;
    print( "<TR><TD COLSPAN=4>実行ボタンを押すと実行します。</TD></TR>\n" ) ;
    &prnt'file( "0", "html/show_tail.html" ) ;
}
exit( 1 ) ;


sub fork
{
    local $pid ;

    unless ( $pid = fork )
    {
	exec( $_[0] ) ;
	exit 0 ;
    }
    waitpid( $pid, 0 ) ;
}
