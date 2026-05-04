package prnt ;

require "./config.pl" ;


sub head
{
    print "Content-type: text/html\n\n<BODY TEXT=white LINK=pink VLINK=pink ALINK=purple BACKGROUND=/runtime/images/bg.gif>\n" ;
}

sub tail
{
    print "</BODY>\n" ;
}

sub lock
{
    local $cnt = 0 ;

    while( -e $_[0] )
    {
	return 0 if ( ++$cnt >= 10 ) ;
	select( undef, undef, undef, 0.05 ) ;
    }
    open( LOCKFD, "> $_[0]" ) ;
    close( LOCKFD );
    return 1 ;
}

sub invoke
{
    select( undef, undef, undef, 0.05 ) ;
    return 1 if ( -e $_[0] && -e $_[1] ) ;
    return 0 ;
}

sub unlock
{
    unlink( $_[0] ) ;
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




sub set_cookies
{
    print "Set-Cookie: chgbbs=$cgi'tags{'from'};expires=Wednesday, 02-Sep-2006, 00:00:00 GMT\n" ;
}

sub file
{
    local ( $mode, $file, @data ) = @_ ;
    local @tag, $tmp, @cookie ;
    local $flag ;
    local $prof = "$msgdir/prof/profile" ;

    # cgi のバージョン情報
    push( @data, "version=$version" ) ;

    # COOKIE の取得 および 解析
    foreach( split( /; /, $ENV{'HTTP_COOKIE'} ) )
    {
	@cookie = split( '=' ) ;
	next if ( $cookie[0] ne "chgbbs" ) ;

	$flag = 0 ;
	open( PROF, "grep -h '$cookie[1] \;' $prof$ENV{'REMOTE_ADDR'} |" ) ;
	while( <PROF> )
	{
	    $flag = 1 ;
	    push( @data, split( ';' ) ) ;
	}
	close PROF ;

	if ( !$flag )
	{
	    open( PROF, "grep -h '$cookie[1] \;' $prof|" ) ;
	    while( <PROF> )
	    {
		push( @data, split( ';' ) ) ;
	    }
	    close PROF ;
	}
    }

    sub show
    {
	local $line, $tmp ;

	open( PRNT_FILE, $_[1] ) ;
	foreach $line ( <PRNT_FILE> )
	{
	    foreach ( @data )
	    {
		@tag = split( '=' ) ;
		$tmp = "_tag_".$tag[0] ;
		$line =~ s/$tmp/$tag[1]/g             if ( $mode==0 || $mode==2 ) ;
		$line =~ s/$tmp/$cgi'tags{$tag[0]}/g  if ( $mode==1 ) ;
	    }
	    $line =~ s/_tag_[_a-zA-Z0-9]+//g       if ( $mode!=2 && not $line =~ /_tag_include::/ ) ;
	    $line =~ s/_tag_[_a-zA-Z0-9]+/hidden/g if ( $mode==2 && not $line =~ /_tag_include::/ ) ;
	    &show( split( /::/, $line ) ) if ( $line =~ "_tag_include" ) ;
	    print $line                   if ( $line !~ "_tag_include" ) ;
        }
        close( PRNT_FILE ) ;
    }

    &show( 0, $file ) ;
}

sub bbs
{
    local ( $file, $start_line, $end_line, @data ) = @_ ;
    local @tag, $tmp, $lines ;
    local $line ;

    push( @data, sprintf( "pstart=%d", $start_line>20?$start_line-20:0 ) ) ;
    push( @data, sprintf( "pend=%d"  , $start_line ) ) ;
    push( @data, sprintf( "nstart=%d", $end_line   ) ) ;
    push( @data, sprintf( "nend=%d"  , $end_line+20) ) ;

    &file( 0, $file, @data ) ;

    $lines = 0 ;
    open( PRNT_FILE, "msg/bbs" ) ;
    foreach $line ( <PRNT_FILE> )
    {
	foreach ( @data )
	{
	    @tag = split( '=' ) ;
	    $tmp = "_tag_".$tag[0] ;
	    $line =~ s/$tmp/$tag[1]/g ;
	}
	$line =~ s/_tag_[_a-zA-Z0-9]+/hidden/g if ( not $line =~ /_tag_include::/ ) ;
	&show_bbs( split( /::/, $line ) ) if ( $line =~ "_tag_include" ) ;
	next if ( ++$lines < $start_line ) ;
	last if (   $lines >= $end_line   ) ;
	print $line if ( $line !~ "_tag_include" ) ;
    }
    close( PRNT_FILE ) ;

    print "</TABLE></FORM></BODY>\n" ;
}


sub dirbbs
{
    local ( $dir, $start, $end, @data ) = @_ ;
    local $line, $num ;
    local @tag, $tmp ;

    push( @data, sprintf( "pstart=%d", $start>20?$start-20:0 ) ) ;
    push( @data, sprintf( "pend=%d"  , $start ) ) ;
    push( @data, sprintf( "nstart=%d", $end   ) ) ;
    push( @data, sprintf( "nend=%d"  , $end+20) ) ;

    &prnt'file( "0", "html/bbs_head.html", @data ) ;

    open( LS, "ls -t $dir |" ) ;
    for( $num=0 ; $line=<LS> ; $num++ )
    {
	if ( ($num >= $start || $start <= 0) &&
	     ($num <  $end   || $end   <= 0) )
	{
	    open( ENT, "$dir/$line" ) ;
	    $line = <ENT> ;
	    foreach( @data )
	    {
		@tag = split( '=' ) ;
		$line =~ s/_tag_$tag[0]/$tag[1]/g ;
	    }
	    $line =~ s/_tag_[_a-zA-Z0-9]+/hidden/g if ( not $line =~ /_tag_include::/ ) ;
	    print $line ;
	    close( ENT ) ;
	}
    }
    close( LS ) ;

    &prnt'file( "0", "html/bbs_tail.html", @data ) ;
}

sub dir
{
    local ( $dir, $start, $end, @data ) = @_ ;
    local $line, $num ;
    local @tag, $tmp ;

    open( LS, "ls -t $dir |" ) ;
    for( $num=0 ; $line=<LS> ; $num++ )
    {
	if ( ($num >= $start || $start <= 0) &&
	     ($num <  $end   || $end   <= 0) )
	{
	    open( ENT, "$dir/$line" ) ;
	    $line = <ENT> ;
	    foreach( @data )
	    {
		@tag = split( '=' ) ;
		$line =~ s/_tag_$tag[0]/$tag[1]/g ;
	    }
	    print $line ;
	    close( ENT ) ;
	}
    }
    close( LS ) ;
}

1 ;

