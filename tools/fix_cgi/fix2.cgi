#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;
$cgi'tags{'from'} =~ s/^[ ]*// ;
$cgi'tags{'from'} =~ s/[ ]*$// ;
$cgi'tags{'files'} .= " " ;
$cgi'tags{'files'} =~ s/\[[^\[]*\]//g ;

$logfile = $date = substr( localtime, 4) ;
$logfile =~ s/[ ]+[0-9]+ [0-9]+:[0-9]+:[0-9]+ // ;
$logfile = $logdir."/Neo".$logfile ;

$cgi'tags{'where'} = "Fix" ;
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

BRANCH:
{
    &com_debug()   , last BRANCH if ( $cgi'tags{'method'} eq "デバッグ" ) ;
    &com_showfix() , last BRANCH if ( $cgi'tags{'files'} eq " " ) ;
    &com_no_name() , last BRANCH if ( $cgi'tags{'from'}  eq "" ) ;
    &com_too_many(), last BRANCH if ( $cgi'tags{'success'} ==  0 ) ;
    &com_fix_up() ;
}

exit( 1 ) ;

print "$debug" ;

print "($cgi'tags{'from'})($cgi'args)" ;


#
#
#
# Command
#
sub com_debug()
{
    local $entry ;

    &prnt'file( "0",
	   "html/show_head.html",
	   "title=FIXデバッグ(管理者用)",
	   "comment=実行されるコマンドを確かめることができます。なにも実行されません。<BR><BR>以下結果です。<HR>" ) ;

    foreach $entry ( split( ' ', $cgi'tags{'files'} ) )
    {
        if ( $entry )
        {
	    print "[ <FONT COLOR=yellow> $entry </FONT> ]<BR>\n" ;
	    open( DUP, "$basedir/fix_cmd $entry |" ) ;
	    foreach( <DUP> ) { print $_ ; }
    	    close( DUP ) ;
	    print "<HR>\n" ;
        }
    }

    &prnt'file( "0", "html/show_tail.html" ) ;
}

sub com_no_name()
{
    &com_showfix( "名前がありません。入れて下さい。" ) ;
}

sub com_too_many() 
{
    &com_showfix( "指定されたファイル数が多すぎます。<BR>いまひとつファイルの数を少なめにして 「ちゃれんじ」 して下さい。<BR>" ) ;
}

sub com_fix_up()
{
    if ( $cgi'tags{'method'} ne "破棄"  )
    {
	&update_bbs() ;
	$date =~ s/\+/ /g ;
	&update_fixup() ;
    }
    &update_entry() ;
    &prnt'dirbbs( $bbstime, $cgi'tags{'bbs'}, $cgi'tags{'bbs'}+20 ) ;
}

sub com_showfix()
{
    local ( $comment ) = @_ ;

    &prnt'file( "0", "html/fix_head.html",
                "incomment=$comment",
                "title=$cgi'tags{'where'}" ) ;
    &prnt'dir( $entrydir ) ;
    &prnt'file( "0", "html/fix_tail.html" ) ;
}


#
#
# Subroutines 
#
#
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

sub update_entry()
{
    local $entry ;
    local $file ;

    foreach $entry ( split( ' ', $cgi'tags{'files'} ) )
    {
        if ( $entry )
        {
	    $entry =~ s/[ \t]*$//g ;
	    open( DUP, "grep -H '$entry<' msg/ent_pool/* | sed -e 's/:.*\$//' |");
	    foreach( <DUP> ) { s/[\n\r]*$// ; unlink ; }
    	    close( DUP ) ;
        }
    }
}

# エントリーファイルの内容から
sub update_entry2()
{
    local $file, $ent ;
    local @entries ;

# エントリーファイルから指定の拡張子ファイルを除外する
    @entries = split( ' ', $cgi'tags{'files'} ) ;

    opendir( ENTDIR, $entrydir ) ;
    foreach $file ( readdir( ENTDIR ) )
    {
	$file = $entrydir . "/" . $file  ;

        foreach( @entries )
        {
	    $debug .= "(grep '$_<' $file |)" ;
	    #open( DUP, "grep '$_<' $file |" ) ;
	    #$debug .= "($file)" if ( <DUP> ) ;
	    #unlink $file if ( <DUP> ) ;
            #close( DUP ) ;
        }
    }
    closedir( ENTDIR ) ;
}

sub make_bbstime()
{
    local ( $fname, $content ) = @_ ;
    local $file, $num ;

    if ( $fname )
    {
	open( LS, "ls -t msg/bbs_time/* | sed -e 's/:.*\$//' |" ) ;
	for( $num=0 ; $file=<LS> ; $num++ )
	{
	    chop $file ;
	    open( DUP, "grep -H 'file=$fname>' $file |" ) ;
	    unlink( $file ) if ( <DUP> || $num>=100 ) ;
	    close( DUP ) ;
	}
	close( LS ) ;

#新しいものを作る
	$file = "$bbstime/bbs" ;
	for ( $num=0 ; -e $file.sprintf( ".%04d", $num ) ; $num++ ){}
	$file .= sprintf( ".%04d", $num ) ;
	
	open( BBT, "> $file" ) ;
	print BBT $content ;
	close( BBT ) ;
    }
}

sub update_bbs()
{
    local $fname, $host ;

    $host = &get_host() ;
    foreach $fname ( split( ' ', $cgi'tags{'files'} ) )
    {
	if ( -e "$bbsdir/$fname" )
        {
            open( BBS_IN, "$bbsdir/$fname" ) ;
            if ( $_ = <BBS_IN> )
            {
                open( BBS_OUT, ">$bbsdir/$fname.fix" ) ;
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
		close( BBS_OUT ) ;

		&make_bbstime( $fname, $_ ) ;
	    }
	    close( BBS_IN ) ;
        }
    }
}


sub update_fixup()
{
    local @files ;
    local $file, $file2 ;

    open( LOG, ">>$logfile" ) ;
    print LOG "$date $cgi'tags{'from'}は、マシン[$ENV{'REMOTE_ADDR'}]から以下のファイルを$cgi'tags{'where'}に更新依頼\n"  ;

    $file = &cgi'num_file( "$fixrundir/$cgi'tags{'from'}" ) ;
    @files = split( ' ', $cgi'tags{'files'} ) ;
    open( FIXUP, ">$file" ) ;
    printf( FIXUP "$where{$cgi'tags{'where'}}\n%d\n", $#files+1 ) ;
    foreach( @files )
    {
        s/^\/// ;
        s/[ \t]*$// ;
	print FIXUP "$_\n" ;
	print LOG   "\t$_\n" ;
    }
    print FIXUP "EOF\n" ;
    close( FIXUP ) ;

    $file2 = $file ;
    $file2 =~ s/fix_pool/test_pool/ ;
    &fork( "cp $file $file2" ) ;

    print LOG substr( localtime, 4)." FixUpに更新依頼\n" ;
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
