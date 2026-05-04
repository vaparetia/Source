#!/usr/bin/perl

require "config.pl" ;
require "prnt.pl" ;
require "cgi.pl" ;

$| = 1 ;

&cgi'decode ;

$now = substr( localtime, 4) ;
$logfile = $now ;
$logfile =~ s/[ ]+[0-9]+ [0-9]+:[0-9]+:[0-9]+ // ;
$logfile = $logdir."/".$logfile ;


BRANCH:
{
    &com_too_many()             , last BRANCH if ( $cgi'tags{'success'} == 0 ) ;

    &com_complete( "human_man" ), last BRANCH if ( $cgi'tags{'method'} eq "人体"     ) ;
    &com_complete( "human_obj" ), last BRANCH if ( $cgi'tags{'method'} eq "付属品"   ) ;
    &com_complete( "human_etc" ), last BRANCH if ( $cgi'tags{'method'} eq "その他"   ) ;
    &com_complete( "world_stg" ), last BRANCH if ( $cgi'tags{'method'} eq "ステージ" ) ;
    &com_complete( "world_obj" ), last BRANCH if ( $cgi'tags{'method'} eq "小物"     ) ;
    &com_complete( "world_dor" ), last BRANCH if ( $cgi'tags{'method'} eq "ドア"     ) ;
    &com_complete( "world_cpt" ), last BRANCH if ( $cgi'tags{'method'} eq "コンセプト" ) ;
    &com_chngecho()             , last BRANCH if ( $cgi'tags{'method'} eq "変更"     ) ;

    &com_cntl_memo( "add"  ), last BRANCH if ( $cgi'tags{'method'} eq "追加"   ) ;
    &com_cntl_memo( "del"  ), last BRANCH if ( $cgi'tags{'method'} eq "前削除" ) ;
    &com_cntl_memo( "kill" ), last BRANCH if ( $cgi'tags{'method'} eq "破棄"   ) ;

    &com_renew_bbs() ;
}

exit(0) ;
#print "($debug)" ;
#print "[$cgi'debug]" ;




#
#Command
#
#

#「変更」コマンド
sub com_chngecho()
{
    $cgi'tags{'change'} = "" ;
    &com_complete( $cgi'tags{'type'} ) ;
}

#コメントタイプを合わせて決めるコマンド
sub com_complete()
{
    local ( $type ) = @_ ;

    $cgi'tags{'style'} = "default" ;
    $cgi'tags{'style'} = "lt2"    if ( $type eq "lt2"   ) ;
    $cgi'tags{'style'} = "item"   if ( $type eq "item"   ) ;
    $cgi'tags{'style'} = "weapon" if ( $type eq "weapon" ) ;
    $cgi'tags{'style'} = "human"  if ( $type eq "human" ) ;
    $cgi'tags{'style'} = "human_man" if ( $type eq "human_man" ) ;
    $cgi'tags{'style'} = "human_obj" if ( $type eq "human_obj" ) ;
    $cgi'tags{'style'} = "debug"  if ( $type eq "debug"  ) ;
    $cgi'tags{'style'} = "goods"  if ( $type eq "goods"  ) ;
    $cgi'tags{'style'} = "world"  if ( $type eq "world"  ) ;
    $cgi'tags{'style'} = "world_stg" if ( $type eq "world_stg"  ) ;
    $cgi'tags{'style'} = "world_obj" if ( $type eq "world_obj"  ) ;
    $cgi'tags{'style'} = "world_dor" if ( $type eq "world_dor"  ) ;
    &com_echoback(  "コメントを入力して下さい。<BR>必要であれば各パラメータも入力して下さい。" ) ;
}

#スペックメモを変更する時のコマンド（現在未使用）
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
    &com_echoback( "コメントを入力して下さい。<BR>必要であれば各パラメータも入力して下さい。" ) ;
}


# BBSの内容を変更する
sub com_renew_bbs()
{
    &cgi'decode_out( "$postdir/$cgi'tags{'from'}" ) ;
    &prnt'dirbbs( $bbstime, $cgi'tags{'bbs'}, $cgi'tags{'bbs'}+20 ) ;
}

#エコバック（ポスト画面に戻すため かなり汎用に使われている）
sub com_echoback()
{
    local ( $title ) = @_ ;
    local @prnt_tag ;

    @prnt_tag = ( "0", "html/echoback.html" ) ;
    foreach ( sort {$b cmp $a} keys( %cgi'tags )  )
    {
	push( @prnt_tag, "$_=$cgi'tags{$_}" ) ;
    }
    &prnt'file( @prnt_tag,
	        "date=$now",
	        "title=$title" ) ;
}

#長過ぎる場合にランタイム画面に戻すためのもの
sub com_too_many() 
{
    local ( $title ) = @_ ;
    local @prnt_tag ;

    @prnt_tag = ( "0", "html/runtime.html" ) ;
    foreach ( sort {$b cmp $a} keys( %cgi'tags )  )
    {
	push( @prnt_tag, "$_=$cgi'tags{$_}" ) ;
    }
    &prnt'file( @prnt_tag, "incomment=指定されたファイル数が多すぎます。<BR>いまひとつファイルの数を少なめにして 「ちゃれんじ」 して下さい。<BR>" ) ;
}


#
# Ordinary Subroutine Sets
#
#

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

    return $? ;
}
