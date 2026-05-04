#!/usr/bin/perl

require "config.pl" ;
require "cgi.pl" ;
require "prnt.pl" ;

&cgi'decode ;

$ENV{'REQUEST_METHOD'} eq "POST" ?
    ($n_read = sysread(STDIN, $args, $ENV{'CONTENT_LENGTH'})):
    ($args = $ENV{'QUERY_STRING'});
#$args = $ARGV[0] ;

&prnt'file( split( '&', $args ) ) ;
