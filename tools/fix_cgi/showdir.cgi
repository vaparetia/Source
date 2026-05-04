#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;

&prnt'file( "0",
	   "html/show_head.html",
	   "title=$cgi'tags{'title'}",
	   "comment=$cgi'tags{'comment'}" ) ;
&prnt_dirs() ;
&prnt'file( "0", "html/show_tail.html" ) ;

exit( 1 ) ;

sub prnt_dirs
{
    local $i = 0 ;
    local $name, $file ;

    open( CURRENT, "ls -rt $cgi'tags{'destination'} |" ) ;
    foreach( <CURRENT> )
    {
	chop ;
	next if ( /\/$/ ) ;
	$name = &cgi'pack( $_ ) ;
	$file = &cgi'pack( "$cgi'tags{'destination'}/$_" ) ;
	print "<TR><TD>\n" ;
	print "<IMG SRC=/runtime/images/ball.gif>" ;
	print "<A HREF=/cgi-bin/show.cgi?0&" ;
	print "html/fixup.html&file=$file" if ( !$cgi'tags{'mode'} ) ;
	print "$file"                      if (  $cgi'tags{'mode'} ) ;
	print "&title=$name> $_ </A>\n" ;
	print "</TD></TR>\n" ;
	$i++ ;
    }
    close( CURRENT ) ;
}
