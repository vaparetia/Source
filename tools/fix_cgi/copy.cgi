#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

&cgi'decode ;

$cgi'tags{'dst'}     =~ s/^\[(.*)\]/$1/ ;
$cgi'tags{'dst_dir'} =~ s/^\/// ;
$cgi'tags{'src_dir'} =~ s/^\/// ;
$cgi'tags{'mode'}    = "move" if ( !$cgi'tags{'mode'} ) ;

$logfile = $now = substr( localtime, 4) ;
$logfile =~ s/[ ]+[0-9]+ [0-9]+:[0-9]+:[0-9]+ // ;
$logfile = $logdir."/".$logfile ;

$tmpsrcfile = $tmpfile.$ENV{'REMOTE_ADDR'}.".1" ;
$tmpdstfile = $tmpfile.$ENV{'REMOTE_ADDR'}.".2" ;

$allmsg = "" ;

&com_chngdir( "src_dir", $cgi'tags{'src'} ) if ( $cgi'tags{'looksrc'} eq "掘る" ) ;
&com_chngdir( "dst_dir", $cgi'tags{'dst'} ) if ( $cgi'tags{'lookdst'} eq "掘る" ) ;

BRANCH:
{
    last BRANCH if ( !$cgi'tags{'method'} || $cgi'tags{'looksrc'} || $cgi'tags{'lookdst'} ) ;

    &err_too_many()   , last BRANCH if ( $cgi'tags{'success'} == 0  ) ;
    &err_no_srcfile() , last BRANCH if ( $cgi'tags{'mode'} ne "ren" && !$cgi'tags{'src'} ) ;
    &err_no_renname() , last BRANCH if ( $cgi'tags{'mode'} eq "ren" &&
		                        (!$cgi'tags{'ren_src'} || !$cgi'tags{'ren_dst'}) ) ;

    $flag = &com_renfile( 0 ), last BRANCH if ( $cgi'tags{'method'} eq "更新" ) ;
    $flag = &com_copyfile()   if ( $cgi'tags{'mode'} eq "copy" ) ;
    $flag = &com_movefile()   if ( $cgi'tags{'mode'} eq "move" ) ;
    $flag = &com_renfile( 1 ), exit( 1 ) if ( $cgi'tags{'mode'} eq "ren"  ) ;

    &err_no_ovrwrt(), last BRANCH if ( $flag == 1 ) ;
    &err_no_dirmov(), last BRANCH if ( $flag == 2 ) ;
    &err_no_files() , last BRANCH if ( $flag == 3 ) ;
    &warn_no_bbs()  , last BRANCH if ( ! $allmsg ) ;

    exit( 1 ) if ( &com_echoback() ) ;
}

$checked = "cp_chk=CHECKED" if ( $cgi'tags{'mode'} eq "copy" ) ;
$checked = "mv_chk=CHECKED" if ( $cgi'tags{'mode'} eq "move" ) ;
$checked = "rn_chk=CHECKED" if ( $cgi'tags{'mode'} eq "ren"  ) ;
&com_showdir( "src_dir", " $cgi'tags{'src'} ", $tmpsrcfile ) ;
&com_showdir( "dst_dir", " $cgi'tags{'dst'} ", $tmpdstfile ) ;
&prnt'file( "0", "html/tree_cop.html",
	   "src_dir=$cgi'tags{'src_dir'}",
	   "dst_dir=$cgi'tags{'dst_dir'}",
	   "srcfile=$tmpsrcfile",
	   "dstfile=$tmpdstfile",
	   "comment=$comment",
	   $checked ) ;

exit( 1 ) ;
print "($debug)" ;

#    &err_too_many()   , last BRANCH if ( $cgi'tags{'success'} == 0  ) ;

# Error
#
#
sub err_too_many() 
{
    $comment = "指定されたファイル数が多すぎます。<BR>いまひとつファイルの数を少なめにして 「ちゃれんじ」 して下さい。<BR>" ;
}
sub err_no_ovrwrt
{
    $comment  = "「コピー」または「移動」では<B>上書き禁止</B>です。" ;
    $comment .= "先に移動先のファイルを消しておくか、移動先を変更して下さい。" ;
}
sub err_no_dirmov
{
    $comment  = "ディレクトリを「コピー」または「移動」はできません。" ;
    $comment .= "複数のファイルを「コピー」または「移動」することは可能です。" ;
}
sub err_no_files
{
    $comment = "ファイルがないため、「コピー」または「移動」はできませんでした。" ;
}
sub err_no_renname
{
    $comment = "リネームするための情報がありません。" ;
    $comment = "必ず両方を<B>慎重に</B>入力して下さい。" ;
}
sub err_no_srcfile
{
    $comment = "元ファイルの指定がありません。元ファイルは必ずしてして下さい。" ;
}
sub err_needdetail
{
    $comment  = "リネームの場合、リネーム元の指定は、必ず親ディレクトリと組に指定して下さい。" ;
    $comment .= "例えば(world/w01a0r→world/w01a/roof)など" ;
}
sub warn_no_bbs
{
    $comment = "このファイルに該当する BBS がないです。" ;
}

#
#Command
#
#

# エコーバック
sub com_echoback()
{
    local @prnt_tag = ( "0", "html/echoback.html") ;
    local $type ;

    $type = $cgi'tags{'dst_dir'} ;
    $type =~ s/^\/?([^\/]+)\/.*/$1/ ;
    $cgi'tags{'src'} =~ s/ [^.]+.rpt//g ;
    $allmsg =~ s/.+<[^?]+\?([^\>]+)>.*/$1/ ;
    $allmsg =~ s/&p/&/g ;
    $debug .= "($type)" ;
    return 0 if ( $allmsg =~ "type=$type" ) ;

    $allmsg =~ s/^1&html\/profile\.html&// ;
    $allmsg =~ s/type=[^&]*&/type=$type&/ ;
    $allmsg =~ s/style=[^&]*&/style=$type&/ ;
    $allmsg =~ s/files=[^&]*&/files=$cgi'tags{'src'}&/ ;
    $allmsg =~ s/&comment=/&comment=%2D%2D%0D/ ;
    $allmsg = substr( $allmsg, 0, index( $allmsg, "\n" ) ) ;
    push( @prnt_tag, &cgi'unpack( $_ ) ) ;
    foreach ( sort {$b cmp $a} split( "&", $allmsg ) )
    {
	$debug .= "($_)" ;
    	push( @prnt_tag, &cgi'unpack( $_ ) ) ;
    }
    &prnt'file( @prnt_tag,
	        "change=",
    		"date=$now",
    		"title=コメントを入力して下さい。<BR>必要であれば各パラメータも入力して下さい。" ) ;
print "($debug)" ;
    return 1 ;
}

#
#ディレクトリを「掘る」コマンド
#
sub com_chngdir()
{
    local ( $dir, $file ) = @_ ;

    $file =~ s/([^ ]+) .*/$1/g ;
    $file =~ s/[\[\]]//g ;
    $cgi'tags{$dir}  =~ s/[^\/]+\/$// if ( $file eq "親ディレクトリ" ) ;
    $cgi'tags{$dir} .= "$file/"       if ( $file ne "親ディレクトリ" && -d "$runtime_dir/$cgi'tags{$dir}$file" ) ;
    $cgi'tags{$dir}  =~ s/\/\//\// ;
    $cgi'tags{$dir}  =~ s/^\///    ;
}

#
#ディレクトリ表示コマンド
#
sub com_showdir()
{
    local ( $dir, $files, $file ) = @_ ;
    local  $tmp ;

    $files =~ s/[\[\]]//g ;
    open( TMP, ">$file" ) ;
    print TMP "<OPTION>[親ディレクトリ]</OPTION>\n" ;
    open( LST, "ls -BpX $runtime_dir/$cgi'tags{$dir} |" ) ;
    while( <LST> )
    {
	chop ;
	s/(.*)\//[$1]/ ;
	$tmp = $_ ;
	$tmp =~ s/[\[\]]//g ;
	print TMP "<OPTION SELECTED>$_</OPTION>\n" if ( $files =~ " $tmp " ) ;
	print TMP "<OPTION         >$_</OPTION>\n" if ( $files !~ " $tmp " ) ;
    }
    close LST ;
    close TMP ;
}

#
#コピーコマンド
#
sub com_copyfile()
{
    local $file, $dir ;
    local @files, $flag ;

    $flag = 0 ;
    @files = split( / /, $cgi'tags{'src'} ) ;
#    push( @files, " " ) if ( $#files == -1 ) ;
    $dir = "$cgi'tags{'dst_dir'}$cgi'tags{'dst'}/" ;
    $dir =~ s/^\/// ;
    $dir =~ s/\/\//\//g ;
    $dir =~ s/\/[^\/]+\/$/\//  if ( !-d "$runtime_dir/$dir" ) ;
    if ( -d "$runtime_dir/$dir" )
    {
	foreach( @files )
	{
	    s/^\[(.*)\]/$1/ ;
	    $file = $cgi'tags{'src_dir'}.$_ ;
	    $file =~ s/^\/// ;
	    $file =~ s/\/\//\//g ;
	    next if ( not -e "$runtime_dir/$file" ) ;
    
    # Copy the file
	    last if ( $flag = &copy( $file, $dir ) ) ;
	    &cp_bbs( $file, $cgi'tags{'src_dir'}, $dir ) ;
	    &cp_entry( $file, $cgi'tags{'src_dir'}, $dir ) ;
    
    # Append Logfile
	    &append( $logfile, "$now マシン[$ENV{'REMOTE_ADDR'}]より ランタイムの $file を $dir にコピーした。\n" ) ;
	}
}
# Release the lock
    return $flag  ;
}


#
#移動コマンド
#
sub com_movefile()
{
    local $file, $dir ;
    local @files, $flag ;

    $flag = 0 ;
    @files = split( / /, $cgi'tags{'src'} ) ;
#    push( @files, " " ) if ( $#files == -1 ) ;
#送り先のディレクトリを指定
    $dir = "$cgi'tags{'dst_dir'}$cgi'tags{'dst'}/" ;
    $dir =~ s/^\/// ;
    $dir =~ s/\/\//\//g ;
    $dir =~ s/\/[^\/]+\/$/\//  if ( !-d "$runtime_dir/$dir" ) ;
    if ( -d "$runtime_dir/$dir" )
    {
	foreach( @files )
	{
	    s/^\[(.*)\]/$1/ ;
	    $file = $cgi'tags{'src_dir'}.$_ ;
	    $file =~ s/^\/// ;
	    $file =~ s/\/\//\//g ;
	    next if ( not -e "$runtime_dir/$file" ) ;
    
    # Copy the file
	    $flag = &move( $file, $dir ) ;
	    last if ( $flag ) ;
	    &mv_bbs( $file, $cgi'tags{'src_dir'}, $dir ) ;
	    &mv_entry( $file, $cgi'tags{'src_dir'}, $dir ) ;
    
    # Append Logfile
	    &append( $logfile, "$now マシン[$ENV{'REMOTE_ADDR'}]より ランタイムの $file を $dir に移動した。\n" ) ;
	}
    }

# Release the lock
    return $flag  ;
}

#
#リネームコマンド
#
sub com_renfile()
{
    local ( $flag ) = @_ ;
    local $src, $dst, $line ;
    local $srcfile, $dstfile ;

#flagが１のときただ確認の表示をするだけ
    if ( $flag )
    {
	local @prnt_tag = ( "0", "html/show_head.html",
			   "title=リネームの確認", "cgifile=copy.cgi", "comment=これでよろしいですか？" ) ;
	local $i=0 ;
#ソートしないとタグの順番が一意にならない
	foreach ( sort {$b cmp $a} keys( %cgi'tags ) )
	{
            next if ( /method/ ) ;
	    $debug .= "($_)" ;
	    push( @prnt_tag, "arg_name$i=$_" ) ;
	    push( @prnt_tag, "arg_val$i=$cgi'tags{$_}" ) ;
            $i++ ;
        }
	&prnt'file( @prnt_tag ) ;
    }
    $src = $cgi'tags{'ren_src'} ;
    $dst = $cgi'tags{'ren_dst'} ;
    $src_c = &pack( $src ) ;
    $dst_c = &pack( $dst ) ;

#リネームするキーワードがBBSに入っているかどうか
    open( BBS,   $bbsfile ) ;
    open( BBS_N, ">$bbsfile.out" ) ;
    foreach $line ( <BBS> )
    {
        chop $line ;
	if ( $line =~ /$src/ )
	{
#BBSにあったのでリネーム処理をする
	    $srcfile =  $line ;
	    $srcfile =~ s/.*&file=([^>]*)>.*/$1/ ;
	    $line    =~ s/$src/$dst/g ;
	    $line    =~ s/$src_c/$dst_c/g  if ( $src_c != $src ) ;
	    $dstfile =  $line ;
	    $dstfile =~ s/.*&file=([^>]*)>.*/$1/ ;
            if ( $flag )
            {
                print "<TR>" ;
                print "<TD>$srcfile</TD><TD>→</TD><TD>$dstfile</TD>\n" ;
                print "<TD><FONT COLOR=yellow><BLINK>重複</BLINK></FONT></TD>" if ( -e "$runtime_dir/$dstfile" ) ;
                print "</TR>\n" ;
            }
            elsif ( ! &move_frc( $srcfile, $dstfile ) )
            {
	    	&exchange( $entryfile , "]$srcfile</OPTION>", "]$dstfile<\/OPTION>" ) ;
	    	&exchange( $tentryfile, "]$srcfile</OPTION>", "]$dstfile<\/OPTION>" ) ;
	        &append( $logfile, "$now マシン[$ENV{'REMOTE_ADDR'}]より ランタイムの $srcfile を $dstfile にリネームした。\n" ) ;
            }
#ファイルネームをリネームすることになるか？
            $srcfile =~ s/.*\/([^\/]+\..+)$/$1/ ;
            $dstfile =~ s/.*\/([^\/]+\..+)$/$1/ ;
    	    $line =~ s/$srcfile/$dstfile/g if ( $srcfile !~ $src && $dstfile ne $srcfile ) ;
	}
	print BBS_N "$line\n" ;
    }
    close BBS_N ;
    close BBS ;
    if (  $flag )
    {
        unlink( "$bbsfile.out" )  ;
    	print( "<TR><TD><INPUT TYPE=SUBMIT NAME=method VALUE=更新></TD></TR>\n" ) ;
    	print( "<TR><TD COLSPAN=4>更新ボタンを押すとリネームを開始します。</TD></TR>\n" ) ;
    	&prnt'file( "0", "html/show_tail.html" ) ;
    }
    else
    {
        &fork( "cp $bbsfile.out $bbsfile" ) ;
    }
# Release the lock

    return $flag  ;
}


sub com_renecho()
{

    return 1 ;
}


#
# Subroutines 
#
#
sub copy
{
    local ($src, $dst) = @_ ;
    local $src2, $ext=$src ;

    return 1 if ( -f "$runtime_dir/$dst" ) ;
    return 2 if ( -d "$runtime_dir/$src" ) ;
    return 3 if (!-e "$runtime_dir/$src" ) ;
    $ext =~ s/.*\.(.+)$/$1/ ;
    &fork( "mkdir -p $runtime_dir/$dst" ) if ( !-e "$runtime_dir/$dst" ) ;
    $src2 = $src ; $src2 =~ s/mdl$/$comb{'mdl'}/ ;
    &fork( "cp $runtime_dir/$src  $runtime_dir/$dst" ) ;
    &fork( "cp $runtime_dir/$src2 $runtime_dir/$dst" ) if ( -e "$runtime_dir/$src2" ) ;
    &fork( "mkdir -p $fix_dir/$dst"  ) if ( !-e "$fix_dir/$dst"  ) ;
    if ( $run_to_fix{$ext} )
    {
	foreach( split( / /, $run_to_fix{$ext} ) )
	{
	    $src2=$src ; $src2 =~ s/$ext$/$_/ ;
	    &fork( "cp $fix_dir/$src2  $fix_dir/$dst " ) ;
	}
    }
    else
    {
	&fork( "cp $fix_dir/$src  $fix_dir/$dst " ) ;
    }
    &make_icon( "cp", $src, $dst ) ;

    return 0 ;
}

sub move
{
    local ($src, $dst) = @_ ;
    local $src2, $ext=$src ;
    local $dir = $src ;

    return 1 if (!-d "$runtime_dir/$dst" ) ;
    return 2 if ( -d "$runtime_dir/$src" ) ;
    return 3 if (!-e "$runtime_dir/$src" ) ;
    $dir =~ s/\/[^\/]+$// ;
    $ext =~ s/.*\.(.+)$/$1/ ;
    &fork( "mkdir -p $runtime_dir/$dst" ) if ( !-e "$runtime_dir/$dst" ) ;
    $src2 = $src ; $src2 =~ s/mdl$/$comb{'mdl'}/ ;
    &fork( "mv $runtime_dir/$src  $runtime_dir/$dst" ) ;
    &fork( "mv $runtime_dir/$src2 $runtime_dir/$dst" ) if ( -e "$runtime_dir/$src2" ) ;
    &fork( "mkdir -p $fix_dir/$dst" ) if ( !-e "$fix_dir/$dst"  ) ;
    if ( $run_to_fix{$ext} )
    {
	foreach( split( / /, $run_to_fix{$ext} ) )
	{
	    $src2=$src ; $src2 =~ s/$ext$/$_/ ;
	    &fork( "mv $fix_dir/$src2  $fix_dir/$dst " ) ;
	}
    }
    else
    {
	&fork( "mv $fix_dir/$src  $fix_dir/$dst " ) ;
    }
    &make_icon( "mv", $src, $dst ) ;
    &fork( "rmdir $runtime_dir/$dir $fix_dir/$dir" ) ;

    return 0 ;
}

sub move_frc
{
    local ($src, $dst) = @_ ;
    local $dir = $dst ;
    local $src2, $dst2, $ext=$src ;

    return 1 if ( -f "$runtime_dir/$dst" ) ;
    $dir =~ s/\/[^\/]+$// ;
    $ext =~ s/.*\.(.+)$/$1/ ;
    &fork( "mkdir -p $runtime_dir/$dir" ) if ( !-e "$runtime_dir/$dir" ) ;
    $src2=$src ; $src2 =~ s/mdl$/$comb{'mdl'}/ ;
    $dst2=$dst ; $dst2 =~ s/mdl$/$comb{'mdl'}/ ;
    &fork( "mv $runtime_dir/$src  $runtime_dir/$dst " ) ;
    &fork( "mv $runtime_dir/$src2 $runtime_dir/$dst2" ) if ( -e "$runtime_dir/$src2" ) ;
    &fork( "mkdir -p $fix_dir/$dir" ) if ( !-e "$fix_dir/$dir" ) ;
    if ( $run_to_fix{$ext} )
    {
	foreach( split( / /, $run_to_fix{$ext} ) )
	{
	    $src2=$src ; $src2 =~ s/$ext$/$_/ ;
	    $dst2=$dst ; $dst2 =~ s/$ext$/$_/ ;
	    &fork( "mv $fix_dir/$src2  $fix_dir/$dst2"  ) ;
	}
    }
    else
    {
	&fork( "mv $fix_dir/$src  $fix_dir/$dst " ) ;
    }
    &make_icon( "mv", $src, $dst ) ;
    $src =~ s/\/[^\/]+$// ;
    &fork( "rmdir $runtime_dir/$src $fix_dir/$src" ) ;

    return 0 ;
}

sub make_icon()
{
    local ( $cmd, $src, $dst ) = @_ ;
    local $dst_d = $dst ;

    $debug .= "(icon[$src][$dst])" ;
    if ( $src =~ m/\.(pic|bmp)$/ )
    {
	$dst =~ s/\/([^\/]*)$// if ( not -d "$runtime_dir/$dst" ) ;
	&fork( "mkdir -p $icon_dir/$dst" ) if ( !-e "$icon_dir/$dst" ) ;
	$dst =~ s/\.(pic|bmp)// ;
	$src =~ s/\.(pic|bmp)// ;

	$debug .= "(kita[$src][$dst])" ;
	&fork( "$cmd $icon_dir/$src.*gif $icon_dir/$dst" ) ;
    }
}

sub cp_bbs
{
    local ( $src, $src_d, $dst_d ) = @_ ;
    local $src_c, $dst_c, $dst = $src ;

    $src_c = &pack( $src_d ) ;
    $dst_c = &pack( $dst_d ) ;

    open( IN, "grep 'file=$src>' $bbsfile |" ) ;
    $msg = <IN> ;
    chop $msg ;
    close( IN ) ;

    return if ( ! $msg ) ;

    $msg =~ s/$src_d/$dst_d/g ;
    $msg =~ s/$src_c/$dst_c/g ;
    $dst =~ s/$src_d/$dst_d/g ;
    &fork( "echo '$msg' > $bbsfile.tmp" ) ;
    &fork( "grep -v 'file=$dst>' $bbsfile > $bbsfile.out" ) ;
    &fork( "cat $bbsfile.tmp $bbsfile > $bbsfile.out" ) ;
    &fork( "cp $bbsfile.out $bbsfile" ) ;
    unlink( "$bbsfile.tmp" ) ;
    chmod( 0666, $bbsfile ) ;

    $allmsg .= $msg ;
}

sub mv_bbs
{
    local ( $src, $src_d, $dst_d ) = @_ ;
    local $src_c, $dst_c, $dst = $src ;

    $src_c = &pack( $src_d ) ;
    $dst_c = &pack( $dst_d ) ;

    open( IN, "grep 'file=$src>' $bbsfile |" ) ;
    $msg = <IN> ;
    chop $msg ;
    close( IN ) ;

    return if ( ! $msg ) ;

    $msg =~ s/$src_d/$dst_d/g ;
    $msg =~ s/$src_c/$dst_c/g ;
    $dst =~ s/$src_d/$dst_d/g ;
    &fork( "echo '$msg' > $bbsfile.tmp" ) ;
    &fork( "grep -v 'file=$src>' $bbsfile | grep -v 'file=$dst>' > $bbsfile.out" ) ; 
    &fork( "cat $bbsfile.tmp $bbsfile.out > $bbsfile" ) ;
    unlink( "$bbsfile.out" ) ;
    unlink( "$bbsfile.tmp" ) ;
    chmod( 0666, $bbsfile ) ;

    $allmsg .= $msg ;
}

sub cp_entry
{
    local ( $src, $src_d, $dst_d ) = @_ ;
    local $dst = $src ;

    $dst   =~ s/$src_d/$dst_d/ ;
    &dup( $entryfile , "]$src</OPTION>", "]$dst<\/OPTION>" ) ;
    &dup( $tentryfile, "]$src</OPTION>", "]$dst<\/OPTION>" ) ;
}

sub mv_entry
{
    local ( $src, $src_d, $dst_d ) = @_ ;
    local $dst = $src ;

    $dst   =~ s/$src_d/$dst_d/ ;
    &exchange( $entryfile , "]$src</OPTION>", "]$dst<\/OPTION>" ) ;
    &exchange( $tentryfile, "]$src</OPTION>", "]$dst<\/OPTION>" ) ;
}

sub pack
{
    local( $value ) = @_ ;

    $value =~ s/(\W)/sprintf("%%%.2X", ord($1))/ge ;
    $value =~ s/%20/+/g ;
    return $value ;
}

sub append
{
    local ($file,$literal) = @_ ;

    open( INOUT, ">>$file" ) ;
    print INOUT $literal ;
    close INOUT ;
}

sub exchange
{
    local ( $file, $except, $replace ) = @_ ;

    open( IN , $file ) ;
    open( OUT, ">$file.out" ) ;
    while( <IN> )
    {
	s/$except/$replace/ if ( $_ =~ $except ) ;
        print OUT ;
    }
    close IN  ;
    close OUT ;
    &fork( "cp $file.out $file" ) ;
    chmod( 0666, $file ) ;
}

sub dup
{
    local ( $file, $except, $replace ) = @_ ;

    open( IN , $file ) ;
    open( OUT, ">$file.out" ) ;
    while( <IN> )
    {
        print OUT ;
	s/$except/$replace/, print OUT if ( $_ =~ $except ) ;
    }
    close IN  ;
    close OUT ;
    &fork( "cp $file.out $file" ) ;
    chmod( 0666, $file ) ;
    $debug .= "([$except][$replace])" ;
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
