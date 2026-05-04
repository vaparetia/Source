#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

#flush the buffer
$| = 1 ;

&cgi'decode ;

$cgi'tags{'from'}   =~ s/^[ ]*([^ ]+).*/$1/ ;
$cgi'tags{'to'}     =~ s/^[ ]*([^ ]+).*/$1/ ;
$cgi'tags{'date'}   =  localtime ;
$cgi'tags{'title'}  = "何の問題もなさそうですがよろしいですか？" ;
$cgi'tags{'reason'} = "新規" ;
#$cgi'tags{'destinate'} =~ s/[ ]+$// ;
$cgi'tags{'destinate'} =~ s/^\/// ;
$cgi'tags{'type'}   = $cgi'tags{'destinate'} ;
$cgi'tags{'type'}   =~ s/^([^\/]+)\/.*/$1/ ;
$all_seletcted = " $cgi'tags{'files'} " ;

&prnt'set_cookies ;

@all_files = split( ' ', $cgi'tags{'files'} ) ;
$cgi'tags{'files'} = "" ;
foreach( @all_files )
{
    next if ( !length ) ;
    s/\[(.*)\]/$1/ ;
    foreach $ext ( @cnfm_ext )
    {
	$cgi'tags{'files'} .= "$_ ", last if ( /$ext$/ ) ;
    }
}

$src_d  = $cgi'tags{'locate'} ;
if ( $#all_files == 0 && $all_files[0] eq "親ディレクトリ" )
{
    $src_d =~ s/[^\/]+\/$// ;
}
elsif ( $#all_files == 0 && -d "$src_d/$all_files[0]" )
{
    $src_d =~ s/$/\// if ( $src_d !~ m/\/$/ ) ;
    $src_d .= $all_files[0] ;
}
@src   = split( '/', $cgi'tags{'locate'} ) ;
$cgi'tags{'files'} = " $src[$#src]" if ( -e $src_d && not -d $src_d ) ;
$src_d =~ s/$/\// if ( $src_d !~ m/\/$/ && -d $src_d ) ;
$src_d =~ s/[-_.0-9a-zA-Z]*$// ;
@src   = split( ' ', $cgi'tags{'files'} ) ;

$dst = $dst_d = $cgi'tags{'destinate'} ;
$dst   =~ s/.*\/([^\/]*)$/$1/ if ( $dst   =~ m/\// )  ;
$dst   = $src[0]              if ( $#src == 0 && $dst_d =~ m/\/$/ || !$dst_d ) ;
$dst_d =~ s/[^\/]*$// ;
$dst_d =~ s/^\/$// ;

@p_r_def = ( "0", "html/runtime.html",
	    "from=$cgi'tags{'from'}",
	    "to=$cgi'tags{'to'}",
	    "reason=$cgi'tags{'reason'}",
	    "type=$cgi'tags{'type'}",
	    "files=$cgi'tags{'files'}",
	    "destinate=$cgi'tags{'destinate'}",
	    "comment=$cgi'tags{'comment'}",
	    "send_ip=$cgi'tags{'send_ip'}",
	    "locate=$src_d" ) ;
#push( @p_r_def, "ABCchecked=CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "" ) ;
#push( @p_r_def, "EXTchecked=CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "X" ) ;
#push( @p_r_def, "TIMchecked=CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "t" ) ;
#push( @p_r_def, "SIZchecked=CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "S" ) ;
push( @p_r_def, "SNDchecked=CHECKED" ) if ( $cgi'tags{'send_fix'} ) ;
push( @p_r_def, "shwdir=$cgi'tags{'shwdir'}" ) ;
push( @p_r_def, "style=$cgi'tags{'style'}" ) ;
push( @p_r_def, "list=$cgi'tags{'list'}" ) ;
$cgi'args =~ s/locate=[^&]*/locate=$src_d/ ;
$cgi'args =~ s/\//%2F/g ;

#Renew the profile file
if ( $cgi'tags{'from'} )
{
    $profile  = "from=$cgi'tags{'from'} ;" ;
    $profile .= "locate=$src_d;" ;
    $profile .= "destinate=$cgi'tags{'destinate'};" ;
    $profile .= "reason=$cgi'tags{'reason'};" ;
    $profile .= "type=$cgi'tags{'type'};" ;
    $profile .= "send_ip=$cgi'tags{'send_ip'};\n" ;
    &renew( "$proffile$ENV{'REMOTE_ADDR'}", $profile, "from=$cgi'tags{'from'} " ) ;
}


BRANCH:
{
    &renew_list() if ( $cgi'tags{'list'} eq $tmplist ) ;
    &renew_area() if ( $cgi'tags{'list'} eq $tmparea ) ;

    &com_too_many()  , last BRANCH if ( $cgi'tags{'success'} == 0 ) ;

#command
    &com_start_dir() , last BRANCH if ( $cgi'tags{'method'} eq "詳細"   ) ;
    &com_start_list(), last BRANCH if ( $cgi'tags{'method'} eq "リスト" ) ;
    &com_start_area(), last BRANCH if ( $cgi'tags{'method'} eq "複数"   ) ;
    &com_mkdir()     , last BRANCH if ( $cgi'tags{'method'} eq "作成"   ) ;
    &com_rmdir()     , last BRANCH if ( $cgi'tags{'method'} eq "削除"   ) ;

    &com_cntl_memo( "add"  ), last BRANCH if ( $cgi'tags{'method'} eq "追加"   ) ;
    &com_cntl_memo( "del"  ), last BRANCH if ( $cgi'tags{'method'} eq "前削除" ) ;
    &com_cntl_memo( "kill" ), last BRANCH if ( $cgi'tags{'method'} eq "破棄"   ) ;

#error
    &com_no_name()   , last BRANCH if ( not $cgi'tags{'from'} ) ;
    &com_no_srcfile(), last BRANCH if ( &check_files( $src_d, @src ) != $#src+1 || $#src == -1 ) ;
    &com_no_srcdir() , last BRANCH if ( &check_dirs ( $src_d, @src ) ) ;
    &com_no_srccomb(), last BRANCH if ( &check_combs( $src_d, @src ) ) ;
#    &com_no_srccomb(), last BRANCH if ( &check_couple( $src_d, @src ) ) ;
    &com_no_dsttyp() , last BRANCH if ( $dst_d !~ '/' ) ;
    &com_be_dsttyp() , last BRANCH if ( $dst_d !~ m/[^\/]+\/[^\/]+\/[^\/]*/ && $dst_d =~ m/^anm|debug|effect|goods|human|item|meca|weapon|world/ ) ;
    &com_no_dstdir() , last BRANCH if ( not -e "$runtime_dir/$dst_d" ) ;
    &com_il_dstnam() , last BRANCH if ( $dst && $dst =~ m/[!\@#\$%^&\*():;\'\"<>,?|\[\]\\]/ ) ;
    &com_be_dstdir() , last BRANCH if ( $dst && $#src != 0 ) ;
    &com_no_dstext() , last BRANCH if ( $dst && $dst !~ m/\./ ) ;

#complete command 
    &com_complete( $cgi'tags{'type'} ), last BRANCH if ( $cgi'tags{'method'} eq "コメント" ) ;
    &com_complete( "human_man"       ), last BRANCH if ( $cgi'tags{'method'} eq "人体"     ) ;
    &com_complete( "human_obj"       ), last BRANCH if ( $cgi'tags{'method'} eq "付属品"   ) ;
    &com_complete( "human_etc"       ), last BRANCH if ( $cgi'tags{'method'} eq "その他"   ) ;
    &com_complete( "world_stg"       ), last BRANCH if ( $cgi'tags{'method'} eq "ステージ" ) ;
    &com_complete( "world_obj"       ), last BRANCH if ( $cgi'tags{'method'} eq "小物"     ) ;
    &com_complete( "world_dor"       ), last BRANCH if ( $cgi'tags{'method'} eq "ドア"     ) ;
    &com_complete( "world_cpt"       ), last BRANCH if ( $cgi'tags{'method'} eq "コンセプト" ) ;

#Waring
    &com_overwrite( $dst ), last BRANCH if (  $dst && &check_files( "$runtime_dir/$dst_d", $dst ) ) ;
    &com_overwrite( @src ), last BRANCH if ( !$dst && &check_files( "$runtime_dir/$dst_d", @src ) ) ;
    &com_no_problem() ;
}

exit( 1 ) ;

print "($debug)" ;
print "($cgi'args)" ;

print "($cgi'tags{'files'})<BR>" ;
#debug info
print "(src=$#src)" ;
print "($src_d)/" ;
print "($runtime_dir/$dst_d)($dst)<BR>" ;
print "<BR>" ;
print "(mdl->$comb{'mdl'})<BR>" ;
printf( "chek_files = %d<BR>", &check_files( "$runtime_dir/$dst_d", $dst ) ) ;










#
# Command Subroutines
#
#
#
sub com_complete()
{
    local ( $type ) = @_ ;
    local $tmp, $style ;

    $style = "style=default" ;
    $style = "style=lt2"    if ( $type eq "lt2"   ) ;
    $style = "style=item"   if ( $type eq "item"   ) ;
    $style = "style=weapon" if ( $type eq "weapon" ) ;
    $style = "style=human"  if ( $type eq "human" ) ;
    $style = "style=human_man" if ( $type eq "human_man" ) ;
    $style = "style=human_obj" if ( $type eq "human_obj" ) ;
    $style = "style=human_etc" if ( $type eq "human_etc" ) ;
    $style = "style=debug"  if ( $type eq "debug"  ) ;
    $style = "style=goods"  if ( $type eq "goods"  ) ;
    $style = "style=world"  if ( $type eq "world"  ) ;
    $style = "style=world_stg" if ( $type eq "world_stg"  ) ;
    $style = "style=world_obj" if ( $type eq "world_obj"  ) ;
    $style = "style=world_dor" if ( $type eq "world_dor"  ) ;
    $style = "style=world_cpt" if ( $type eq "world_cpt"  ) ;
    $cgi'tags{'style'} = $style ;

    $tmp = pop( @p_r_def ) ;
    pop( @p_r_def ) ;
    pop( @p_r_def ) ;
    push( @p_r_def, "shwdir=msg/blank" ) ;
    push( @p_r_def, $style ) ;
    push( @p_r_def, $tmp ) ;

    &prnt'file( @p_r_def ) ;
}

sub com_cntl_memo()
{
    local ( $mode ) = @_ ;
    local @tags, $show_memo, $i ;
    local @cmma ;

    @tags = ("spec_memo9", "spec_memo2", "spec_memo3",
             "spec_memo4", "spec_memo5", "spec_memo6", "spec_memo7" )
	if ( $cgi'tags{'style'} eq "world_obj" || $cgi'tags{'style'} eq "world_dor" ) ;
    @tags = ("spec_memo8", "spec_memo1", "spec_memo2",
             "spec_memo3", "spec_memo4", "spec_memo5", "spec_memo6" )
	if ( $cgi'tags{'style'} eq "weapon"    || $cgi'tags{'style'} eq "goods" ||
	     $cgi'tags{'style'} eq "item"      || $cgi'tags{'style'} eq "meca" ) ;
    @tags = ("spec_memo8", "spec_memo2", "spec_memo3",
             "spec_memo4", "spec_memo5", "spec_memo6", "spec_memo7" )
	if ( $cgi'tags{'style'} eq "human_obj" ) ;

    foreach( @tags )
    {
	$cgi'tags{$_} = ",".$cgi'tags{$_} if ( $mode eq "add"  ) ;
	$cgi'tags{$_} =~ s/^,[^,]*//      if ( $mode eq "del"  ) ;
	$cgi'tags{$_} = ""                if ( $mode eq "kill" ) ;
    }
    @cmma = split( ",", $cgi'tags{$tags[0]} ) ;
    $cgi'tags{'show_memo'} = "" ;
    for( $i=1 ; $i<=$#cmma ; $i++ )
    {
        $cgi'tags{'show_memo'} .= "<TR><TD></TD><TD>[" ;
        $cgi'tags{'show_memo'} .= (split( ",", $cgi'tags{$tags[0]} ) )[$i]. ":位置(" ;
        $cgi'tags{'show_memo'} .= (split( ",", $cgi'tags{$tags[1]} ) )[$i]. "," ;
        $cgi'tags{'show_memo'} .= (split( ",", $cgi'tags{$tags[2]} ) )[$i]. "," ;
        $cgi'tags{'show_memo'} .= (split( ",", $cgi'tags{$tags[3]} ) )[$i]. ")回転角度(" ;
        $cgi'tags{'show_memo'} .= (split( ",", $cgi'tags{$tags[4]} ) )[$i]. "," ;
        $cgi'tags{'show_memo'} .= (split( ",", $cgi'tags{$tags[5]} ) )[$i]. "," ;
        $cgi'tags{'show_memo'} .= (split( ",", $cgi'tags{$tags[6]} ) )[$i]. ")" ;
        $cgi'tags{'show_memo'} .= "]</TD></TR>" ;
    }
    foreach ( sort {$b cmp $a} keys( %cgi'tags ) )
    {
	push( @p_r_def, "$_=$cgi'tags{$_}" ) ;
    }
    &prnt'file( @p_r_def ) ;
}



sub com_start_dir()
{
    local $tmp ;

    if ( $cgi'tags{'shwdir'} eq "msg/blank" )
    {
        $cgi'args =~ s/style=[^&]*/style=ready/ ;
        &fork( "$basedir/dir.cgi '$cgi'args'" ) ;
    }
    else
    {
        $tmp = pop( @p_r_def ) ;
        pop( @p_r_def ) ;
        pop( @p_r_def ) ;
	push( @p_r_def, "shwdir=msg/blank" ) ;
	push( @p_r_def, "style=ready" ) ;
	push( @p_r_def, $tmp ) ;
        &prnt'file( @p_r_def  ) ;
    }
}

sub com_start_list()
{
    local $dir ;

    &renew_list() ;
    if ( $cgi'tags{'shwdir'} eq "msg/blank" )
    {
        pop( @p_r_def ) ;
        pop( @p_r_def ) ;
        &prnt'file( @p_r_def, "list=$tmplist", "style=ready" ) ;
    }
    else
    {
        $cgi'args =~ s/list=[^&]*/list=$tmplist/ ;
        $cgi'args =~ s/style=[^&]*/style=ready/ ;
        $cgi'args =~ s/\//%2F/g ;
        &fork( "$basedir/dir.cgi '$cgi'args'" ) ;
    }
}

sub com_start_area()
{
    &renew_area() ;
    if ( $cgi'tags{'shwdir'} eq "msg/blank" )
    {
	pop( @p_r_def ) ;
        pop( @p_r_def ) ;
	&prnt'file( @p_r_def, "list=$tmparea", "style=ready" ) ;
    }
    else
    {
        $cgi'args =~ s/list=[^&]*/list=$tmparea/ ;
        $cgi'args =~ s/style=[^&]*/style=ready/ ;
        $cgi'args =~ s/\//%2F/g ;
        &fork( "$basedir/dir.cgi '$cgi'args'" ) ;
    }
}

sub com_mkdir()
{
    local $dir ;

    $dir = "$runtime_dir/$dst_d/$cgi'tags{'directory'}" ;
    $dir =~ s/[!\@#\$%^&\*():;\'\"<>,?|\[\]\\]//g ;
    &fork( "mkdir -p  $dir" ) if ( defined($cgi'tags{'directory'}) ) ;
    &fork( "chmod a+w $dir" ) if ( defined($cgi'tags{'directory'}) ) ;
    &fork( "$basedir/dir.cgi '$cgi'args'" ) ;
}

sub com_rmdir()
{
    local $dir ;

    $dir = "$runtime_dir/$dst_d/$cgi'tags{'directory'}" ;
    &fork( "rmdir $dir" ) if ( defined($cgi'tags{'directory'}) ) ;
    &fork( "$basedir/dir.cgi '$cgi'args'" ) ;
}

sub com_too_many() 
{
    &prnt'file( @p_r_def, "incomment=指定されたファイル数が多すぎます。<BR>いまひとつファイルの数を少なめにして 「ちゃれんじ」 して下さい。<BR>" ) ;
}

sub com_no_name()
{
    &prnt'file( @p_r_def, "incomment=お前は名なしなのか？さっさと入れろ！！<BR>" ) ;
}

sub com_no_srcfile()
{
    &prnt'file( @p_r_def, "incomment=格納元のファイルが見つかりませんでした。<BR>" ) ;
}

sub com_no_srcdir()
{
    &prnt'file( @p_r_def, "incomment=格納元のファイルは、ディレクトリです。<BR>"   ) ;
}

sub com_no_srccomb()
{
    local $file, $flag = 0 ;
    local $warn = "incomment=" ;

    foreach( @src )
    {
	foreach $ext ( keys( %comb ) )
	{
	    next if ( not m/$ext$/ ) ;
	    s/.$ext$/.$comb{$ext}/ ;
	    next if ( -e "$src_d/$_" ) ;
	    $warn .= " および ", if ( $flag ) ;
	    $warn .= $_ ;
	    $flag=1 ;
	}
    }
    $warn .= " のファイルが必要です。<BR>同じディレクトリに置いて下さい。<BR>" ;
    &prnt'file( @p_r_def, $warn ) ;
}

sub com_no_dsttyp()
{
    &prnt'file( @p_r_def, "incomment=格納先のディレクトリが然るべき深さまでありません。<BR>どれかのカテゴリにあるディレクトリまで指定して下さい。<BR>" ) ;
}

sub com_be_dsttyp()
{
    &prnt'file( @p_r_def, "incomment=このカテゴリの直に置くことが出来ません。<BR>さらにディレクトリを追加し、指定して下さい。<BR>" ) ;
}

sub com_no_dstdir()
{
    &prnt'file( @p_r_def, "incomment=格納先のディレクトリがありませんでした。<BR>" ) ;
}

sub com_be_dstdir()
{
    &prnt'file( @p_r_def, "incomment=複数のファイルを選ぶ時は、格納先はディレクトリでなければなりません。<BR>ディレクトリ指定の場合は最後に／を付けて下さい。<BR>" ) ;
}

sub com_no_dstext()
{
    &prnt'file( @p_r_def, "incomment=格納先の拡張子がありません。<BR>必ず拡張子のついた格納先をご指定下さい。<BR>" ) ;
}

sub com_il_dstnam()
{
    &prnt'file( @p_r_def, "incomment=格納先に変な文字(%#?など)が含まれています。<BR>取り除いて格納先をご指定下さい。<BR>" ) ;
}

sub com_overwrite()
{
    local ( @search ) = @_ ;
    local $comment ;

    $flag = 1 ;

    open( TMP, ">$tmpfile$ENV{'REMOTE_ADDR'}" ) ;
    print TMP "前の更新履歴です。<BR>" ;
    foreach $file ( @search )
    {
	open( BBS, "$bbsdir/$dst_d$file" ) ;
	$_ = <BBS> ;
	close BBS ;

	$comment = $_ ;
	$comment =~ s/.+&pcomment=([^&]*)&.+/$1/ ;
	s/<INPUT[^>]+>// ;
	s/^<TR>.*▼<\/A><\/TD>/<TR><TD><\/TD>/ ;
	print TMP ;

	$flag = 0 ;
    }
    print TMP "前の情報がありません。<BR>" if ( $flag ) ;
    close TMP ;

#    $cgi'tags{'comment'}   = $cgi'tags{'comment'}."\n--\n".&unpack( $comment ) ;
    $cgi'tags{'incomment'} = "_tag_include::$tmpfile$ENV{'REMOTE_ADDR'}" ;
    $cgi'tags{'title'}     = "ファイルを上書きしますがよろしいですか？" ;
    $cgi'tags{'reason'}    = "変更" ;

    &com_no_problem() ;
}

sub com_no_problem()
{
    local @prnt_tag = ( "0", "html/echoback.html" ) ;

    $cgi'tags{'locate'   } =  $src_d      ;
    $cgi'tags{'destinate'} = "$dst_d$dst" ;
    $cgi'tags{'change'   } = "echo/" ;
    foreach ( sort {$b cmp $a} keys( %cgi'tags )  )
    {
	push( @prnt_tag, "$_=$cgi'tags{$_}" ) ;
    }
    push( @prnt_tag ) ;
    &prnt'file( @prnt_tag ) ;
}



#
# Subroutines 
#
#
sub renew_area()
{
    open( TMP, ">$tmparea" ) ;
    print TMP "<TR><TD></TD><TD>\n" ;
    print TMP "<TEXTAREA NAME=files COLS=17 ROWS=5>$cgi'tags{'files'}</TEXTAREA>\n" ;
    print TMP "</TD><TD VALIGN=TOP><FONT size=2>複数のファイルを入力可能。</FONT>\n" ;
    print TMP "</TD></TR>\n" ;
    close TMP ;
}

sub renew_list()
{
    local $tmp ;

    open( TMP, ">$tmplist" ) ;
    print TMP "<TR><TD></TD><TD>\n" ;
    print TMP "<SELECT SIZE=20 NAME=files MULTIPLE>\n" ;
    print TMP "<OPTION>[親ディレクトリ]</OPTION>\n" ;
    open( LST, "ls -Bp$cgi'tags{'ls_opt'} $src_d |" ) ;
    while( <LST> )
    {
        chop ;
        s/(.*)\//[$1]/ ;
        s/(.*)@/[$1]/ ;
	$tmp = $_ ;
	$tmp =~ s/[\(\)]//g ;
	$tmp =~ s/[\[\]]//g ;
        print TMP "<OPTION SELECTED>$_</OPTION>\n" if ( $all_seletcted =~ " $tmp " ) ;
        print TMP "<OPTION         >$_</OPTION>\n" if ( $all_seletcted !~ " $tmp " ) ;
    }
    close LST ;
    print TMP "</SELECT>" ;

    print TMP "</TD><TD VALIGN=TOP>\n" ;

    print TMP "<INPUT TYPE=radio NAME=ls_opt VALUE=\"\" "; 
    print( TMP "CHECKED" ) if ( !$cgi'tags{'ls_opt'} ) ;
    print TMP "><FONT size=2>ABC順</FONT>" ;
    print TMP "<INPUT TYPE=radio NAME=ls_opt VALUE=X ";
    print( TMP "CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "X" ) ;
    print TMP "><FONT size=2>拡張子順</FONT>" ;
    print TMP "<INPUT TYPE=radio NAME=ls_opt VALUE=t ";
    print( TMP "CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "t" ) ;
    print TMP "><FONT size=2>日付順</FONT>" ;
    print TMP "<INPUT TYPE=radio NAME=ls_opt VALUE=S ";
    print( TMP "CHECKED" ) if ( $cgi'tags{'ls_opt'} eq "S" ) ;
    print TMP "><FONT size=2>サイズ順</FONT><BR>" ;
    print TMP "<FONT size=2>複数のファイルを選ぶことができます。</FONT>" ;

    print TMP "</TD></TR>\n" ;
    close TMP ;
}

sub check_files()
{
    local ( $dir, @files ) = @_ ;
    local $cnt = 0 ;

    foreach( @files )
    {
	$cnt++ if ( -f "$dir/$_" ) ;
    }
    return $cnt ;
}

sub check_dirs()
{
    local ( $dir, @files ) = @_ ;
    local $cnt = 0 ;

    foreach( @files )
    {
	$cnt++ if ( -d "$dir/$_" ) ;
    }
    return $cnt ;
}

sub check_combs()
{
    local ( $dir, @files ) = @_ ;
    local $cnt = 0 ;

    foreach( @files )
    {
	foreach $ext ( keys( %comb ) )
	{
	    next if ( not m/$ext$/ ) ;
	    s/$ext$/$comb{$ext}/ ;
	    $cnt++ if ( not -e "$dir/$_" ) ;
	}
    }
    return $cnt ;
}

#sub check_couple()
#{
#    local ( $dir, @files ) = @_ ;
#    local $cnt = 0 ;
#    local $fil_a, $fil_b ;
#    local %check_cpl ;
#
#    foreach $fil_a ( @files )
#    {
#	foreach $ext ( keys( %couple ) )
#	{
#	    next if ( not $fil_a =~ m/$ext$/ ) ;
#	    $cnt = 1 ;
#	    foreach $fil_b ( @files )
#	    {
#		next if ( $fil_a eq $fil_a ) ;
#		next if ( not $fil_a =~ m/$couple{$ext}$/ ) ;
#		$cnt = 0 ;
#	    }
#	}
#    }
#    foreach( keys %check_cpl )
#    {
#	if ( $check_cpl{$_} < ) ;
#    }
#    return $cnt ;
#}

sub renew
{
    local ( $in, $literal, $except ) = @_ ;

    open( IN , $in ) ;
    open( OUT, ">$in.out" ) ;
    print OUT $literal ;
    while( <IN> )
    {
	print OUT $_ if ( $_ !~ $except )
    }
    close IN  ;
    close OUT ;
    &fork( "cp $in.out $in" ) ;
}

sub append
{
    local ($in,$literal) = @_ ;

    open( INOUT, ">>$in" ) ;
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

sub unpack
{
    local( $value ) = @_ ;

    $value =~ s/\+/ /g ;
    $value =~ s/%([0-9A-Fa-f][0-9A-Fa-f])/pack("c", hex($1))/ge ;
    &jcode'convert( *value, 'euc' ) ;

    return $value ;
}
