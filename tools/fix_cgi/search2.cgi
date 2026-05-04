#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;

$cgi'tags{'keyword'} =~ s/\|[ ]*/ |/ ;
$cgi'tags{'keyword'} =~ s/\-[ ]*/ -/ ;
$cgi'tags{'keyword'} =~ s/\+[ ]*/ +/ ;

foreach( split( ' ', $cgi'tags{'keyword'} ) )
{
    $word = &cgi'pack( $_ ) ;
    $word =~ s/%2E/./g ;
    $word =~ s/%2F/\//g ;
    $word =~ s/^%2B// ; # take off '+'
    $word =~ s/^%2D// ; # take off '-'
    $word =~ s/^%7C// ; # take off '|'
    push( @and, $word ), next if ( /^\+/ ) ;
    push( @not, $word ), next if ( /^\-/ ) ;
    push( @or , $word ) ;
#    push( @or , $word ), next if ( /^\|/ ) ;
#    push( @and, $word ) ;
}

$cgi'tags{'extend'} = "*" if ( ! $cgi'tags{'extend'} ) ;

$flag = 1 ;
open( BBS, "find $bbsdir/$cgi'tags{'dir'} -name '*.$cgi'tags{'extend'}' -exec cat {} \\; |" ) ;
LINE:
foreach $line ( <BBS> )
{
    $srch_str = $line ;
    $srch_str =~ s/^.*&preason=// ;
    $srch_str =~ s/pcomment=// ;
    $srch_str =~ s/<\/A>.*$// ;

    $found = 0 ;
    foreach( @not )
    {
        next LINE  if ( $srch_str =~ $_ ) ;
    }
    foreach( @or )
    {
        $found = 1 if ( $srch_str =~ $_ ) ;
    }
    foreach( @and )
    {
        next LINE  if ( $srch_str !~ $_ ) ;
    }
    next LINE if ( !$found ) ;

    &prnt'file( "0", "html/result2.html" ) if ( $flag ) ;
    $flag = 0 ;
    $line =~ s/_tag_[_a-zA-Z]+[0-9]+/hidden/ ;
    $line =~ s/^<TR>.*▼<\/A><\/TD>/<TR><TD><\/TD>/ ;
    $line =~ s/<TD COLSPAN=3>[^V]*VALUE=[^>]*>// ;
    print $line ;

    chop( $srch_str ) ;

    $srch_str =~ s/.*&file=([^>]*)>.*/$1/ ;
    print "<INPUT TYPE=hidden NAME=files VALUE =$srch_str>\n" ;
}
close BBS ;


&prnt'file( "0", "html/search2.html",
 "comment=残念ながら該当するファイルはありませんでした。<BR>もう一度やり直して下さい。") if ( $flag ) ;

&prnt'file( "0", "html/show_tail.html" ) ;

&prnt'tail ;

exit 0 ;
