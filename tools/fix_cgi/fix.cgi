#!/usr/bin/perl

require "/exports/mj001data/httpd/cgi-bin/config.pl" ;
require "/exports/mj001data/httpd/cgi-bin/prnt.pl" ;
require "/exports/mj001data/httpd/cgi-bin/cgi.pl" ;

&cgi'decode ;
$cgi'tags{'from'} =~ s/^[ ]*// ;
$cgi'tags{'from'} =~ s/[ ]*$// ;
$cgi'tags{'files'} .= " " ;
$cgi'tags{'files'} =~ s/\[[^\[]*\]//g ;

$logfile = $date = substr( localtime, 4) ;
$logfile =~ s/[ ]+[0-9]+ [0-9]+:[0-9]+:[0-9]+ // ;
$logfile = $logdir."/".$logfile ;

$gifimg   = $where{"gif$cgi'tags{'where'}"} ;
$dest_dir = $where{$cgi'tags{'where'}} ;
$title    = substr(  $cgi'tags{'where'}, 0, 4 ) ;

$msg  = "<TR><TD></TD>" ;
$msg .= "<TD><IMG SRC=$gifimg></TD>" ;
$msg .= "<TD></TD>" ;
$msg .= "<TD>&lt; 更新/$title &gt;</TD>" ;
$msg .= "<TD>[Name: $cgi'tags{'from'}]</TD>" ;
$msg .= "<TD>$date</TD></TR>" ;
$date =~ s/ /+/g ;

$entry = "entry=msg/entries" ;
$entry = "entry=msg/tentries" if ( $cgi'tags{'where'} eq "Temporary" ) ;

BRANCH:
{
    &com_too_many()   , last BRANCH if ( $cgi'tags{'success'} == 0  ) ;
    &com_no_name()    , last BRANCH if ( $cgi'tags{'from'} eq ""    ) ;
    &com_someone_fix(), last BRANCH if ( &prnt'lock( $lockfile )==0 ) ;
    &com_fix_up() ;
}

exit( 1 ) ;

print "($cgi'tags{'from'})($cgi'args)" ;
print "$debug" ;


#
#
#
# Command
#

sub com_no_name()
{
    &prnt'file( "0", "html/fix.html",
                "incomment=名前がありません。入れて下さい。",
                "title=$cgi'tags{'where'}", $entry ) ;
}

sub com_someone_fix()
{
    &prnt'file( "0", "html/fix.html",
                "incomment=只今、誰かが更新中です。<BR>しばらく待ってもう一度、更新ボタンを押して下さい。",
                "title=$cgi'tags{'where'}", $entry ) ;
}

sub com_too_many() 
{
    &prnt'file( "0", "html/fix.html",
                "incomment=指定されたファイル数が多すぎます。<BR>いまひとつファイルの数を少なめにして 「ちゃれんじ」 して下さい。<BR>",
                "title=$cgi'tags{'where'}", $entry ) ;
}

sub com_fix_up()
{
    if ( $cgi'tags{'method'} eq "破棄"  )
    {
        $cgi'tags{'where'} = "Fix" ;
	&update_entries() ;
	&prnt'unlock( $lockfile ) ;
    }
    else
    {
	&update_entries() ;
	&update_bbs() ;
	$date =~ s/\+//g ;
	&update_log_fixup() ;

# awake the fixup process
	open( PID, $pidfile ) ;
	chop( $_=<PID> ) ;
        kill 'HUP', $_ ;
	close PID ;
    }
    &prnt'bbs( "html/bbs.html", $cgi'tags{'bbs'}, $cgi'tags{'bbs'}+20 ) ;
}


#
# Subroutines 
#
#
sub update_entries()
{
    $entryfile = "$basedir/$cgi'tags{'entry'}" if ( $cgi'tags{'entry'} ) ;

    if ( $cgi'tags{'flag'} =~ "ALL" )
    {
	$cgi'tags{'files'} = "" ;
	open( ENT,   $entryfile  ) ;
	while( <ENT> )
	{
	    $det = $_ ;
	    $det =~ s/^<OPTION>// ;
	    $det =~ s/^\[.*\]//g ;
	    $det =~ s/<\/OPTION>\n$/ / ;
	    $cgi'tags{'files'} .= $det ;

	    $det =~ s/ //g ;
            &renew( $tentryfile, $_, "]$det</OPTION>" ) if ( $cgi'tags{'where'} eq "Temporary" ) ;
	}
	close ENT ;
	unlink( $entryfile ) ;
	open( ENT, ">$entryfile" ) ;
	close ENT ;
    }
    else
    {
	open( ENT_IN ,   $entryfile      ) ;
	open( ENT_OUT, ">$entryfile.out" ) ;
	while( <ENT_IN> )
	{
	    $det = $_ ;
	    $det =~ s/^<OPTION>// ;
	    $det =~ s/^\[.*\]//g ;
	    $det =~ s/<\/OPTION>\n$/ / ;

	    if ( $cgi'tags{'files'} !~ $det )
            {
	        print ENT_OUT $_ ;
            }
            else
            {
	        $det =~ s/ //g ;
                &renew( $tentryfile, $_, "]$det</OPTION>" ) if ( $cgi'tags{'where'} eq "Temporary" ) ;
	    }
	}
	close ENT_IN  ;
	close ENT_OUT ;
	&fork( "cp $entryfile.out $entryfile" ) ;
    }
}

sub get_host()
{
    local $host  ;

    return "allup" if ( !($host = $ENV{'REMOTE_ADDR'}) ) ;

    open( NSLOOKUP, "nslookup $ENV{'REMOTE_ADDR'} | grep Name |" ) ;
    chop( $host=<NSLOOKUP> ) ;
    close( NSLOOKUP ) ;
    $host =~ s/^Name:[ ]+// ;

    return $host ;
}

sub update_bbs()
{
    local $fname, $host ;

    $host  = &get_host() ;

    open( BBS_IN ,   $bbsfile      ) ;
    open( BBS_OUT, ">$bbsfile.out" ) ;
    while( <BBS_IN> )
    {
	foreach $fname ( split( ' ', $cgi'tags{'files'} ) )
	{
	     if ( /file=$fname>/ )
	     {
		s/&fname=[^&]*&/&fname=$cgi'raw{'from'}&/ ;
		s/&fcnmt=[^&]*&/&fcnmt=$cgi'tags{'fcnmt'}&/ ;
		s/&fdate=[^&]*&/&fdate=$date&/ ;
		s/&fhost=[^&]*&/&fhost=$host&/ ;
		s/&tmp_fix=[^&]*&/&tmp_fix=$cgi'tags{'where'}&/ ;
		s/&fix=submit/&fix=hidden/ ;
		s/&icmnt=text/&icmnt=hidden/ ;
		s/<TR><TD><\/TD><TD><IMG SRC=.*gif>.+[0-9: ]+<\/TD><\/TR>//      if ( !/fix.gif/ || /fxd.gif/ || /err.gif/ || /tmp.gif/ ) ;
		s/<TR><TD><\/TD><TD><\/TD><TD>/$msg<TR><TD><\/TD><TD><\/TD><TD>/ if ( !/fix.gif/ || /fxd.gif/ || /err.gif/ || /tmp.gif/ ) ;
		s/fxd\.gif/fix.gif/ if ( /fxd.gif/ ) ;
		s/err\.gif/fix.gif/ if ( /err.gif/ ) ;
		print BBS_OUT ;
		last ;
	     }
	}
    }
    seek( BBS_IN, 0, 0 ) ;
  BBS_FILE:
    while( <BBS_IN> )
    {
	foreach $fname ( split( ' ', $cgi'tags{'files'} ) )
	{
	     next BBS_FILE if ( /file=$fname>/ ) ;
	}
	print BBS_OUT ;
    }
    close BBS_IN ;
    close BBS_OUT ;
    &fork( "cp $bbsfile.out $bbsfile" ) ;    
}

sub update_log_fixup()
{
    local @files ;

    @files = split( ' ', $cgi'tags{'files'} ) ;
    open( FIXUP, ">$fixupfile" ) ;
    open( LOG, ">>$logfile" ) ;
    printf( FIXUP "$where{$cgi'tags{'where'}}\n%d\n", $#files+1 ) ;
    print LOG "$date $cgi'tags{'from'}は、マシン[$ENV{'REMOTE_ADDR'}]から以下のファイルを$cgi'tags{'where'}に更新依頼\n"  ;
    foreach( @files )
    {
	print FIXUP "$_\n" ;
	print LOG   "\t$_\n" ;
    }
    print LOG substr( localtime, 4)." FixUpに更新依頼\n" ;
    close( FIXUP ) ;
}


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
    &fork( "cp $in.out $in" ) ;
}

sub fork
{
    local $pid ;

    unless ( $pid = fork )
    {
        exec( $_[0] ) ;
        exit 0 ;
    }
    waitpid( $pid, 0 ) ;
    return $? ;
}

