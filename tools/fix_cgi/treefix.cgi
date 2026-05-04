#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;

$ENV{'REQUEST_METHOD'} eq "POST" ?
    ($n_read = sysread(STDIN, $showdirs, $ENV{'CONTENT_LENGTH'})):
    ($showdirs = $ENV{'QUERY_STRING'}) ;
#    ($showdirs = ($ENV{'QUERY_STRING'} eq ""? $ARGV[0]: $ENV{'QUERY_STRING'})) ;

@now = split( ' ', localtime ) ;

$i = 0 ;
open( CURRENT, "ls -lR --full-time $fix_dir|" ) ;
foreach( <CURRENT> )
{
    $files[$i][$j++] = $_ ;
    if ( $_ =~ "total " )
    {
	$files[++$i][$j=0] = $dirname ;
	foreach( split( '/', $dirname ) )
	{
	    /:/ && chop, $dirnm{$_} = $i ;
	}
    }
    $dirname = $_ ;
}
close( CURRENT ) ;
$files[$i][$j] = "total " ;

open( ENTRY, "$entryfile" ) ;
foreach( <ENTRY> )
{
    s/^<OPTION>// ;
    s/<\/OPTION>\n$/ / ;
    $entries .= $_ ;
}
close( ENTRY ) ;

&prnt'head ;
&prnt_dirs( 1, "" ) ;
&prnt'tail ;

exit( 1 ) ;



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
    local ( $s, $dir )= @_ ;
    local $j, @data ;

    printf( "<DL>\n" ) ;
    $j = 0 ;
    while ( !($files[$s][$j] =~ "total ") )
    {
	@data = split( ' ', $files[$s][$j++] ) ;
	if ( $data[0] =~ m/^d/ )
	{
	    printf "<IMG SRC=/runtime/images/non.gif><A HREF=/cgi-bin/treefix.cgi?" ;
	    if ( $showdirs =~ "$data[10]:" )
            {
                $dirs = $showdirs ;
                $dirs =~ s/$data[10]:// ;
	        print "$dirs>$data[10]:</A><BR>\n" ;
	        &prnt_dirs( $dirnm{"$data[10]:"}, "$dir$data[10]/" ) ;
            } else {
	        print "$showdirs$data[10]:>$data[10]:</A><BR>\n" ;
            }
	} else {
	    next if ( $data[10] eq "" ) ;

	    if ( &time_cmp( 1, @data ) )  {
		print "<IMG SRC=/runtime/images/non.gif>" ;
	    } else {
		print "<IMG SRC=/runtime/images/ne2.gif>" ;
	    }
	    print "$data[6] $data[7] $data[8] <FONT COLOR=yellow>$data[10]</FONT><BR>" ;
	}
    }
    printf( "</DL>\n" ) ;
}
