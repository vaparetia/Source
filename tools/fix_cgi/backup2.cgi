#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;

$logfile = $now = substr( localtime, 4) ;
$logfile =~ s/[ ]+[0-9]+ [0-9]+:[0-9]+:[0-9]+ // ;
$logfile = $logdir."/Neo".$logfile ;

$now =~ s/[ ]+[0-9]+:[0-9]+:[0-9]+// ;
$now .= $now ;
$now =~ s/^[ a-zA-Z]+ [0-9]+// ;
$now =~ s/ [0-9]+$// ;
$now =~ s/ //g ;

foreach( split( / /, $cgi'tags{'files'} ) )
{
	s/^\[(.*)\]/$1/ ;
	$file = $cgi'tags{'cur_dir'}.$_ ;
    next if ( not -e "$runtime_dir/$file" ) ;

    # Copy the file
    $dir = "$backup_dir/$cgi'tags{'cur_dir'}" ;
    &fork( "mkdir -p $dir" ) if ( !-d $dir ) ;
	$dir = &copy( "$runtime_dir/$file", "$backup_dir/$file" ) ;

    # Append Logfile
    &append( $logfile, "$now マシン[$ENV{'REMOTE_ADDR'}]より ランタイムの $file をバックアップした。\n" ) ;
}

&fork( "$basedir/treeact.cgi 'directive=RunTime&ext_mode=$cgi'tags{'ext_mode'}'" ) ;

exit( 1 ) ;




#
# Subroutines 
#
#
sub copy
{
    local ($src, $dst) = @_ ;
    local $i, $ext ;

    $i = 0 ;
    do
    {
	$num = sprintf( "%02d", $i++ ) ;
    }
    while( -e "$dst.$now.$num" ) ;
    &fork( "cp    $src $dst.$now.$num" ) if ( !-d $src ) ;
    &fork( "cp -r $src $dst.$now.$num" ) if (  -d $src ) ;

    return "$dst.$now.$num" ;
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
