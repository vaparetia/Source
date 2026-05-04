#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;

$cgi'tags{'strcode'} =~ s/[ \t]*//g ;


$flag = 1 ;
open( STRCODE, "grep ' $cgi'tags{'strcode'} ' $strpool |" ) ;
foreach $line ( <STRCODE> )
{
    chop $line ;
    $line =~ s/^.*=> \/u\/develop\/mj001data\/runtime// ;

    last if ( $cgi'tags{'strcode'} eq "" ) ;
    next if ( ! -e "$bbsdir/$line" ) ;

    &prnt'file( "0", "html/result2.html" ) if ( $flag ) ;
    $flag = 0 ;

    open( BBS, "$bbsdir/$line" ) ;
    $bbs = <BBS> ;
    $bbs =~ s/_tag_[_a-zA-Z]+[0-9]+/hidden/ ;
    $bbs =~ s/^<TR>.*▼<\/A><\/TD>/<TR><TD><\/TD>/ ;
    print $bbs ;

    #$bbs =~ s/.*&file=([^>]*)>.*/$1/ ;
    print "<INPUT TYPE=hidden NAME=files VALUE =$line>\n" ;
}
close STRCODE ;

&prnt'file( "0", "html/search2.html",
 "comment=残念ながら該当するファイルはありませんでした。<BR>もう一度やり直して下さい。") if ( $flag ) ;

&prnt'file( "0", "html/show_tail.html" ) ;

&prnt'tail ;

exit 0 ;


