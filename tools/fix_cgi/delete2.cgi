#!/usr/bin/perl

require "config.pl" ;
require "cgi.pl" ;

&cgi'decode ;

$logfile = $now = substr( localtime, 4) ;
$logfile =~ s/[ ]+[0-9]+ [0-9]+:[0-9]+:[0-9]+ // ;
$logfile = $logdir."/Neo".$logfile ;

$drct = $cgi'tags{'directive'} ;

$root_dir = $runtime_dir ;
$root_dir = $fix_dir  if ( $drct eq "Fix"       ) ;

foreach( split( / +/, $cgi'tags{'files'} ) )
{
    s/^\[(.*)\]/$1/ ;
    $src = $cgi'tags{'cur_dir'} . $_ ;
    $src = $cgi'tags{'cur_dir'} . $_ ;
    $dst = $cgi'tags{'cur_dir'} . "." . $_ ;
    $src =~ s/^\/// ;
    $dst =~ s/^\/// ;
    next if ( not -e "$root_dir/$src" ) ;
    $dst = &get_name( $root_dir, $dst ) ;

    # Delete from BBS
    if ( $drct eq "RunTime" )
    {
        &delete_entry( $src ) ;
	rename( "$bbsdir/$src", "$bbsdir/$dst" ) ;
    }

    # Rename the file
    rename( "$root_dir/$src", "$root_dir/$dst" ) ;
    foreach ( keys %comb )
    {
        next if ( $src !~ m/$_$/ ) ;

        $dst =~ s/\.$_\./\.$comb{$_}\./ ;
        $src =~ s/\.$_$/\.$comb{$_}/ ;
	rename( "$root_dir/$src", "$root_dir/$dst" ) ;
        last ;
    }

    # Append Logfile
    &append( $logfile, "$now マシン[$ENV{'REMOTE_ADDR'}]より $drct の $src を $dst として消去\n" ) ;
}

&fork( "$basedir/treeact.cgi 'directive=$drct&ext_mode=$cgi'tags{'ext_mode'}'" ) ;

exit( 1 ) ;

print $debug ;




#
# Subroutines 
#
#
sub get_name
{
    local ( $dir, $dst ) = @_ ;
    local $i ;

    $i = 0 ;
    do
    {
	$num = sprintf( "%06d", $i++ ) ;
    }
    while( -e "$dir/$dst.$num" ) ;

    return "$dst.$num" ;
}


sub delete_entry()
{
    local ( $file ) = @_ ;

    open( DUPLICATE, "grep -H '$file<' $entrydir/*  | sed -e 's/:.*\$//' |" ) ;
    foreach( <DUPLICATE> ) { chop ; unlink }
    close( DUPLICATE ) ;
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
