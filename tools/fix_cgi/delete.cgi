#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;

$logfile = $now = substr( localtime, 4) ;
$logfile =~ s/[ ]+[0-9]+ [0-9]+:[0-9]+:[0-9]+ // ;
$logfile = $logdir."/".$logfile ;

$drct = $cgi'tags{'directive'} ;

$root_dir = $runtime_dir ;
$root_dir = $fix_dir  if ( $drct eq "Fix"       ) ;
$root_dir = $temp_dir if ( $drct eq "Temporary" ) ;

if ( &prnt'lock( $lockfile ) )
{
    local $src, $dst, $bbs ;

    foreach( split( / +/, $cgi'tags{'files'} ) )
    {
	s/^\[(.*)\]/$1/ ;
	$src = $cgi'tags{'cur_dir'}.$_ ;
	$dst = $cgi'tags{'cur_dir'}.".".$_ ;
	$src =~ s/^\/// ;
        next if ( not -e "$root_dir/$src" ) ;

# Rename the file
        $dst = &rename( "$root_dir/$src", "$root_dir/$dst" ) ;

# Delete from BBS
	&delete ( $entryfile , "]$src</OPTION>" ) if ( !-d $dst ) ;
	&delete ( $tentryfile, "]$src</OPTION>" ) if ( !-d $dst ) ;
        $bbs = &delete ( $bbsfile, "file=$src>" ) if ( !-d $dst && $drct eq "RunTime" ) ;
        $bbs = &deleted( $bbsfile, $src, $dst )   if (  -d $dst && $drct eq "RunTime" ) ;

# Append Delfile
        &append( $delfile, "$dst\n0\n"  ) if (!$bbs ) ;
        &append( $delfile, "$dst\n$bbs" ) if ( $bbs ) ;

# Append Logfile
        &append( $logfile, "$now マシン[$ENV{'REMOTE_ADDR'}]より $drct の $src を $dst として消去\n" ) ;
    }

# Release the lock
    &prnt'unlock( $lockfile ) ;
    &fork( "/exports/mj001data/httpd/cgi-bin/treeact.cgi 'directive=$drct&ext_mode=$cgi'tags{'ext_mode'}'" ) ;
}
else
{
    &fork( "/exports/mj001data/httpd/cgi-bin/treeact.cgi 'directive=$drct&ext_mode=$cgi'tags{'ext_mode'}&cur_dir=$cgi'tags{'cur_dir'}&showdirs=$cgi'tags{'showdirs'}&comment=どなたかが更新中です。しばらく待って「消去」して下さい。'" ) ;
}

exit( 1 ) ;
print $debug ;




#
# Subroutines 
#
#
sub rename
{
    local ($src, $dst) = @_ ;
    local $i, $ext ;

    $i = 0 ;
    do
    {
	$num = sprintf( "%06d", $i++ ) ;
    }
    while( -e "$dst.$num" ) ;
    &fork( "mv $src $dst.$num" ) ;

    return $dst.".".$num ;
}

sub delete
{
    local ( $file, $except ) = @_ ;
    local $strings ;

    open( IN , $file ) ;
    open( OUT, ">$file.out" ) ;
    while( <IN> )
    {
	$strings = $_ if ( $_ =~ $except ) ;
	print OUT     if ( $_ !~ $except ) ;
    }
    close IN  ;
    close OUT ;
    &fork( "cp $file.out $file" ) ;

    return "1\n$strings" if ( $strings ) ;
    return "" ;
}

sub deleted
{
    local ( $file, $src, $dir ) = @_ ;
    local $strings = "", $name, $cnt ;
    local $tmp1, $tmp0 ;

    open( LS, "ls $dir |" ) ;
    foreach $name ( <LS> )
    {
	chop( $name ) ;
	&delete ( $entryfile , "]$src/$name</OPTION>" )  if (!-d "$dir/$name" ) ;
	&delete ( $tentryfile, "]$src/$name</OPTION>" )  if (!-d "$dir/$name" ) ;
	$tmp0 = &delete ( $file, "file=$src/$name>" )         if (!-d "$dir/$name" ) ;
	$tmp0 = &deleted( $file, "$src/$name", "$dir/$name")  if ( -d "$dir/$name" ) ;
	$tmp0 =~ s/^[0-9]+\n// ;
	$cnt++ if (!-d "$dir/$name" && $tmp0 ) ;
	$strings .= $tmp0 ;
	$debug .= "$cnt +=$tmp1 $name<BR>" ;
    }
    close LS  ;

    return "$cnt\n$strings" ;
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

sub append
{
    local ($file,$literal) = @_ ;

    open( INOUT, ">>$file" ) ;
    print INOUT $literal ;
    close INOUT ;
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
}
