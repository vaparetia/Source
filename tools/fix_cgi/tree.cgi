#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;

@now = split( ' ', localtime ) ;

$root_dir = $fix_dir ;
$root_dir = $runtime_dir if ( $cgi'tags{'directive'} eq "RunTime"   ) ;

&prnt'file( "0", "html/tree_head.html",
            "ext_mode=$cgi'tags{'ext_mode'}",
            "directive=$cgi'tags{'directive'}",
            "comment=$cgi'tags{'comment'}" ) ;
&prnt_dirs( $root_dir, $cgi'tags{'showdirs'} ) ;
&prnt'file( "0", "html/tree_$cgi'tags{'ext_mode'}",
            "ext_mode=$cgi'tags{'ext_mode'}",
            "directive=$cgi'tags{'directive'}",
            "cur_dir=$cgi'tags{'cur_dir'}",
            "tmpfile=$tmpfile",
            "showdirs=$showdirs" ) ;

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

    return 0 if ( int( $t1[9].$month{ $t1[6]}. $t1[7].substr( $t1[8],0,2)) + $hour >
		  int($now[4].$month{$now[1]}.$now[2].substr($now[3],0,2)) ) ;
    return 1 ;
}

sub prnt_cgi()
{
    local( $directive, $ext_mode, $showdirs, $last_str ) = @_ ;

    print "<A HREF=/cgi-bin/tree2.cgi?" ;
    print "directive=$directive&" ;
    print "ext_mode=$ext_mode&" ;
    print "showdirs=$showdirs>" ;
    print $last_str
}

sub prnt_dirs
{
    local  ( $root, $dir ) = @_ ;
    local   @data, $dirname, $flag, @files, @bbs ;

    &prnt_cgi( $cgi'tags{'directive'},
	       $cgi'tags{'ext_mode'},
	       "",
	       "$cgi'tags{'directive'}/</A>\n" ) ;
    foreach( split( '/', $dir ) )
    {
	$dirname .= $_."/" ;
	&prnt_cgi( $cgi'tags{'directive'},
		   $cgi'tags{'ext_mode'},
		   $dirname,
		   "$_</A>/\n" ) ;
    }
    print "<BR>\n" ;

    @files = &get_dir( "$root/$dir" ) ;
    if ( $cgi'tags{'directive'} eq "RunTime" )
    {
	@bbs = get_bbs( $dir, @files ) ;
    }

    print "<TABLE>\n" ;
    foreach( @files )
    {
	print "<TR><TD>" ;
	@data = split( ' ' ) ;
	if ( $data[0] =~ m/^d/ )
	{
	    print "<IMG SRC=/runtime/images/non.gif>" ;
	    &prnt_cgi( $cgi'tags{'directive'},
		       $cgi'tags{'ext_mode'},
		       "$dir$data[10]/",
		       "$data[10]:</A>" ) ;
	} else {
	    next if ( $data[10] eq "" ) ;

            $flag = $cgi'tags{'directive'} eq "RunTime" ? &prnt_bbs( "$dir$data[10]", @bbs ) : 0 ;
	    if ( !$flag ) {
		if ( &time_cmp( 5, @data ) ) {
		    print "<IMG SRC=/runtime/images/non.gif>" ;
		} else {
		    print "<IMG SRC=/runtime/images/ne2.gif>" ;
		}
		print "<FONT COLOR=YELLOW>$data[10]</FONT></TD><TD></TD><TD></TD><TD>$data[6] $data[7] $data[8] $data[9]" ;
	    }
	}
	print "</TD></TR>\n" ;
    }
    print "</TABLE>\n" ;
}


sub get_bbs()
{
    local ( $dir, @files ) = @_ ;
    local @data, @bbs ;
    local $head, $image ;

    return if ( $dir eq "" ) ;

    foreach $file ( @files )
    {
	@data = split( ' ', $file ) ;
	open( BBS, "$bbsdir/$dir$data[10]" ) ;
	if ( $_ = <BBS> )
	{
	    chop ;
	    s/^.*<\/TD><TD><A HREF/<A HREF/ ;
	    s/<\/TD><\/TR><TR><TD><\/TD><TD><\/TD><TD><\/TD><TD COLSPAN=3>.*<\/TR>$// ;
	    s/<\/TR><TR><TD><\/TD><TD>/<\/TR><TR><TD>/ ;
	    s/gif><\/TD><TD><\/TD><TD>/gif><\/TD><TD>/ ;
	    s/<BR>$/<BR CLEAR>/ ;
	    s/<BR>/<BR><IMG SRC=\/runtime\/images\/non.gif>/g ;
	    $head = "<IMG SRC=/runtime/images/" ;
	    if ( $data[10] =~ m/(bmp|pic)$/ )
	    {
		$image = $_ ;
		$image =~ s/.*_pend&image=([^&]*)&.*/$1/ ;
		$image = "no_img" if ( !-e "$icon_dir/$image.32.gif" ) ;
		$head .= "icon_pool/$image.32" ;
	    }
	    else
	    {
		$head .= &time_cmp( 1, @data )? "rtm" : "ne2" ;
	    }
	    $head .= ".gif>" ;
	    push( @bbs, "$head$_") ;
	}
	close BBS ;
    }

    return @bbs ;
}


sub prnt_bbs()
{
    local ( $file, @bbs ) = @_ ;

    foreach( @bbs )
    {
	if ( $_ =~ "file=$dir$data[10]>" )
	{
	    print ;
	    return 1 ;
	}
    }

    return 0 ;
}
