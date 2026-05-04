##############################################################
# cgi.pl - decode CGI input.
# Usage:
#       require 'cgi.pl';
#       &cgi'decode;
#       ...
#       $name = $cgi'tags{'name'};
package cgi;

require "./jcode.pl" ;

sub decode_in
{
    local( $file ) = @_ ;
    local( $tag, $value ) ;

    undef( %tags ) ;
    undef( %raw  ) ;
    undef( $args ) ;

    open( CGI, $file ) ;
    $args = <CGI> ;
    close( CGI ) ;
    return 1 if ( not $args ) ;

    foreach( split('&', $args) )
    {
	($tag, $value) = split(/=/, $_, 2);
	if ($raw{$tag}){
	    $raw{$tag} .= "+$value" ;
	}else{
	    $raw{$tag}  =   $value  ;
	}
	$value =~ s/\+/ /g;
	$value =~ s/%([0-9A-Fa-f][0-9A-Fa-f])/pack("c", hex($1))/ge ;
	&jcode'convert(*value, 'euc' ) ;
	if ($tags{$tag}){
	    $tags{$tag} .= " $value" ;
	}else{
	    $tags{$tag}  =   $value  ;
	}
    }
    return 0 ;
}

sub decode_out
{
        local( $file ) = @_ ;
	local $name ;
	
	$name  = &num_file( $file ) ;

	open( CGI, "> $name" ) ;
        print CGI "$args\n" ;
	close( CGI ) ;
}

sub num_file
{
    local ( $file ) = @_ ;
    local( $num ) ;

    $date = localtime ;
    $date =~ s/[0-9]+:[0-9]+:[0-9]+ // ;
    $date =~ s/ /./g ;
    for ( $num=0 ; -e "$file.$date.$num" ; $num++ ) {}
    return "$file.$date.$num" ;
}

sub decode
{
        local( $tag, $value ) ;
	
        $ARGV[0] ? ($args = $ARGV[0]) :
	    ($ENV{'REQUEST_METHOD'} eq "POST" ?
	     sysread(STDIN, $args, $ENV{'CONTENT_LENGTH'}) : ($args = $ENV{'QUERY_STRING'})) ;
        foreach( split('&', $args) )
	{
                ($tag, $value) = split(/=/, $_, 2);
		if ($raw{$tag}){
		    $raw{$tag} .= "+$value" ;
		}else{
		    $raw{$tag}  =   $value  ;
		}
                $value =~ s/\+/ /g;
                $value =~ s/%([0-9A-Fa-f][0-9A-Fa-f])/pack("c", hex($1))/ge ;
		&jcode'convert(*value, 'euc' ) ;
		if ($tags{$tag}){
		    $tags{$tag} .= " $value" ;
		}else{
		    $tags{$tag}  =   $value  ;
		}
        }
}


sub encode
{
    local @tagkey = @_ ;
    local $str, $tmp ;

    foreach( @tagkey )
    {
	$tmp = $tags{$_} ;
	$tmp =~ s/(\W)/sprintf("%%%.2X", ord($1))/ge ;
	$tmp =~ s/%20/+/g ;
	$str .= $_."=".$tmp."&" ;
    }
    chop( $str ) ;
    return $str ;
}

sub unpack
{
    local( $value ) = @_ ;

    $value =~ s/\+/ /g ;
    $value =~ s/%([0-9A-Fa-f][0-9A-Fa-f])/pack("c", hex($1))/ge ;
    &jcode'convert( *value, 'euc' ) ;

    return $value ;
}

sub pack
{
    local( $value ) = @_ ;

    $value =~ s/(\W)/sprintf("%%%.2X", ord($1))/ge ;
    $value =~ s/%20/+/g ;

    return $value ;
}

1 ;
