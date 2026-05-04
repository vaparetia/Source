#!/usr/bin/perl

require "cgi.pl" ;


&cgi'decode ;

print "Content-type: text/plain\n\n" ;

printf "$cgi'tags{'file1'}\n" ;

exit(1) ;

sub renew
{
    local ( $in, $literal, $except ) = @_ ;
    open( IN , $in ) ;
    open( OUT, ">$in.out" ) ;
    print OUT $literal ;
    while( <IN> )
    {
	print OUT if ( $_ !~ $except )
    }
    close IN  ;
    close OUT ;

    rename( "$in.out", $in ) ;
    chmod( 0666, $in ) ;
}
