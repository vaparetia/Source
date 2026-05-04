#!/usr/bin/perl

require "config.pl" ;
require "cgi.pl" ;
require "prnt.pl" ;

&cgi'decode ;
$dirs = $cgi'tags{'destinate'} ;

@now = split( ' ', localtime ) ;

open( ENTRY, "$entryfile" ) ;
foreach( <ENTRY> )
{
    s/^<OPTION>// ;
    s/<\/OPTION>\n$/:/ ;
    $entries .= $_ ;
}
close( ENTRY ) ;

$tmpfile .= $ENV{'REMOTE_ADDR'} ;
open( TMP, ">$tmpfile" ) ;
print TMP "<TR><TD><FONT COLOR=yellow>ディレクトリ</FONT></TD>" ;
print TMP "<TD><INPUT TYPE=text SIZE=12 NAME=directory>" ;
print TMP "<INPUT TYPE=submit NAME=method VALUE=作成>" ;
print TMP "<INPUT TYPE=submit NAME=method VALUE=削除></TD></TR>\n" ;


$dir_arg  = "from=$cgi'raw{'from'}&" ;
$dir_arg .= "to=$cgi'raw{'to'}&" ;
$dir_arg .= "locate=$cgi'raw{'locate'}&" ;
$dir_arg .= "comment=$cgi'raw{'comment'}&" ;
$dir_arg .= "list=$cgi'tags{'list'}&" ;
$dir_arg .= "style=$cgi'tags{'style'}&" ;
$dir_arg .= "shwdir=$tmpfile" ;

&prnt_dirs() ;

close TMP ;

@p_r_def= ("0", "html/runtime.html",
	   "list=$cgi'tags{'list'}",
	   "shwdir=$tmpfile",
	   "from=$cgi'tags{'from'}",
	   "to=$cgi'tags{'to'}",
	   "locate=$cgi'tags{'locate'}",
	   "destinate=$dirs",
	   "style=$cgi'tags{'style'}",
	   "comment=$cgi'tags{'comment'}" ) ;
push( @p_r_def, "dirlist=_tag_$cgi'tags{'list'}" ) if ( $cgi'tags{'list'} ) ;
push( @p_r_def, "ABCchecked=CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "" ) ;
push( @p_r_def, "EXTchecked=CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "X" ) ;
push( @p_r_def, "TIMchecked=CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "t" ) ;
push( @p_r_def, "SIZchecked=CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "S" ) ;
&prnt'file( @p_r_def ) ;

exit( 1 ) ;


#
# Subroutins
#
#
#
sub get_dir()
{
    local ( $dir ) = @_ ;
    local @files ;

    open( CURRENT, "ls -l --full-time $dir|" ) ;
    foreach( <CURRENT> )
    {
	next if ( /total [0-9]+/ ) ;
	push( @files, $_ ) ;
    }
    close( CURRENT ) ;
    return @files ;
}

sub time_cmp
{
    local ( $hour, @t1 ) = @_ ;
    local %month = ( 'Jan' => '01', 'Feb' => '02', 'Mar' => '03', 'Apl' => '04',
                     'May' => '05', 'Jun' => '06', 'Jul' => '07', 'Aug' => '08',
                     'Sep' => '09', 'Oct' => '10', 'Nov' => '11', 'Dec' => '12', );

    return 1 if ( int( $t1[9].$month{ $t1[6]}. $t1[7].substr( $t1[8],0,2)) + $hour >
		  int($now[4].$month{$now[1]}.$now[2].substr($now[3],0,2)) ) ;
    return 0 ;
}

sub prnt_dirs
{
    local   @data, @files, $dir, dirname ;

    $dir = $dirs ;
    $dir =~ s/[-_.0-9a-zA-Z]*$// ;
    print TMP "<TR><TD COLSPAN=3>\n" ;
    print TMP "<A HREF=/cgi-bin/dir.cgi?$dir_arg>runtime</A>/\n" ;
    foreach( split( '/', $dir ) )
    {
	$dirname .= $_."/" ;
	print TMP "<A HREF=/cgi-bin/dir.cgi?$dir_arg&destinate=$dirname>$_</A>/\n" ;
	
    }
    print TMP "</TD></TR>\n" ;
    foreach( &get_dir( "$runtime_dir/$dir" ) )
    {
	print TMP "<TR><TD></TD><TD>\n" ;
	@data = split( ' ' ) ;
	if ( $data[0] =~ m/^d/ )
	{
	    print TMP "<IMG SRC=/runtime/images/non.gif>" ;
	    print TMP "<A HREF=/cgi-bin/dir.cgi?$dir_arg&destinate=$dir$data[10]/>" ;
	    print TMP "$data[10]:</A><BR>\n" ;
	} else {
	    next if ( $data[10] eq "" ) ;

	    if ( $entries !~ "$dir$data[10]:" ) {
		print TMP "<IMG SRC=/runtime/images/fix.gif>" ;
	    } elsif ( &time_cmp( 5, @data ) )  {
		print TMP "<IMG SRC=/runtime/images/ne2.gif>" ;
	    } else {
		print TMP "<IMG SRC=/runtime/images/rtm.gif>" ;
	    }
	    print TMP "<A HREF=/cgi-bin/dir.cgi?$dir_arg&destinate=$dir$data[10]>" ;
	    print TMP "$data[10]</A><BR>\n" ;
	}
	print TMP "</TD></TR>\n" ;
    }
}

