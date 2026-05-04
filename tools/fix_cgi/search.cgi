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

if ( $cgi'tags{'flag'} =~ "FIX" )
{
    open( ENTRY, "$entryfile" ) ;
    foreach( <ENTRY> )
    {
	s/^<OPTION>/file=/ ;
	s/<\/OPTION>\n$/ / ;
	$entries .= $_ ;
    }
    close( ENTRY ) ;
}

$flag = 1 ;
open( BBS, $bbsfile ) ;
LINE:
foreach $line ( <BBS> )
{
    if ( $cgi'tags{'flag'} =~ "FIX" )
    {
         $ent_flag = 1 ;
         foreach( split( ' ', $entries ) )
         {
             $ent_flag = 0 if ( $line =~ $_ ) ;
         }
	 next LINE if ( $ent_flag ) ;
    }
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

    &prnt'file( "0", "html/result.html" ) if ( $flag ) ;
    $flag = 0 ;
    $line =~ s/_tag_[_a-zA-Z]+[0-9]+/hidden/ ;
    $line =~ s/^<TR>.*▼<\/A><\/TD>/<TR><TD><\/TD>/ ;
    print $line ;

    chop( $srch_str ) ;
    $srch_str =~ s/.*&file=([^>]*)>.*/$1/ ;
    print "<INPUT TYPE=hidden NAME=files VALUE =$srch_str>\n" ;
}
close BBS ;

&prnt'file( "0", "html/search.html",
 "comment=残念ながら該当するファイルはありませんでした。<BR>もう一度やり直して下さい。") if ( $flag ) ;

&prnt'file( "0", "html/show_tail.html" ) ;

&prnt'tail ;

exit 0 ;


