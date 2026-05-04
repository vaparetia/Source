#!/usr/bin/perl

require "config.pl" ;
require "cgi.pl" ;
require "prnt.pl" ;

$ARGV[0] ? ($args = $ARGV[0]) :
    ($ENV{'REQUEST_METHOD'} eq "POST" ?
     sysread(STDIN, $args, $ENV{'CONTENT_LENGTH'}) :
     ($args = $ENV{'QUERY_STRING'})) ;

$args = &cgi'unpack( $args ) ;

&prnt'file( split( '&', $args ) ) ;
