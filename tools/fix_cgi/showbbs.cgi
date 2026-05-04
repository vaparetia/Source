#!/usr/bin/perl

require "config.pl" ;
require "cgi.pl" ;
require "prnt.pl" ;

&cgi'decode ;

&com_showbbs( split( '&', $cgi'args ) ) ;

exit( 0 ) ;

sub com_showbbs()
{
    local ( $dummy, $start_line, $end_line, @data ) = @_ ;

    &prnt'dirbbs( $bbstime, $start_line, $end_line, @data ) ;
}

sub com_showfix()
{
    local ( $comment ) = @_ ;
    
    &prnt'file( "0", "html/fix_head.html",
                "incomment=$comment",
                "title=$cgi'tags{'where'}" ) ;
    &prnt'dir( $entrydir, $start_line, $end_line ) ;
    &prnt'file( "0", "html/fix_tail.html" ) ;
}

