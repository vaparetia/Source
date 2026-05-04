#!/usr/bin/perl
# 2002/10/24 : メール通知機構作成
# paellaの環境では、sh 上では ">>&" ">&" が使用できないので、
# exec 実行時に "tcsh" を指定して動かしている。

require "/u/develop/mj001b/tools/bin/linux/jcode.pl" ;

$PROJECT  = "mgs2x";
$BACKUP   = "mgs2x_back";
$UPDATE   = "new_mgs2x";
$LOG      = "log.txt";
$LOG_BACK = "log_back.txt"; 

$ENV_DIR       = "/usr/local/develop/preview";
$PROJ_DIR      = "$ENV_DIR/$PROJECT";
$PROJ_BACK_DIR = "$ENV_DIR/$BACKUP";
$LOG_FILE      = "$ENV_DIR/$LOG";
$LOG_BACK_FILE = "$ENV_DIR/$LOG_BACK";

$UPDATE_DIR  = "$ENV_DIR/$UPDATE";
$UPDATE_PROJ = "$UPDATE_DIR/$PROJECT";
$UPDATE_LOG  = "$UPDATE_DIR/$LOG";

# 環境変数設定
$ENV{'CVSROOT'} = "/u/develop/mj001b/cvsroot";
# $ENV{'MGS2_XBOX'} = 1;
$ENV{'MGS2_LANG'} = "ENGLISH";
$ENV{'MGS2_SUBSTANCE'} = 1;

# 実行マシンの確認
if($ENV{'HOST'} ne "paella.konami"){
    print "Error! Please login to \"paella\".\n";
    exit 1;
}

# 実行ディレクトリ確認
if( !-d "$UPDATE_DIR") {
    print "Error! Not Found \"$UPDATE_DIR\".\n";
    exit 1;
}

# ゴミの削除
if( -d $UPDATE_PROJ ){
    print "rm -rf $UPDATE_PROJ\n";
    &fork("rm -rf $UPDATE_PROJ");
}
if( -e $UPDATE_LOG ){
    print "rm -f $UPDATE_LOG\n";
    &fork("rm -f $UPDATE_LOG");
}

# 更新開始
chdir $UPDATE_DIR;
print "cd $UPDATE_DIR\n";
$command = "cvs checkout mgs2x >& $UPDATE_LOG";
print "$command\n";
if(&fork($command)){
    print "Error! \"cvs checkout mgs2x\"\n";
	goto ERROR_EXIT;
}

exit 0;

# variable.symをコピーしてくる
$command = "cp $PROJ_DIR/scn/variable.sym $UPDATE_PROJ/scn/. >>& $UPDATE_LOG";
print "$command\n";
if(&fork($command)){
    print "Error! \"cp variable.sym\"\n";
	goto ERROR_EXIT;
}

chdir $UPDATE_PROJ;
print "cd $UPDATE_PROJ\n";
$command = "make allnew >>&  $UPDATE_LOG";
print "$command\n";
if(&fork("$command")){
    print "Error! \"make allnew\"\n";
	goto ERROR_EXIT;
}

# モーションデバッグ環境の構築
$ENV{'MGS2_SCN'} = 1;

chdir "$UPDATE_PROJ/list";
print "cd $UPDATE_PROJ/list";
$command = "make vox movie >>& $UPDATE_LOG";
print "$command\n";
if(&fork("$command")){
	print "Error! \"make vox movie\"\n";
	goto ERROR_EXIT;
}

chdir "$UPDATE_PROJ/scn";
print "cd $UPDATE_PROJ/scn";
$command = "make clean all >>& $UPDATE_LOG";
print "$command\n";
if(&fork("$command")){
	print "Error! \"make clean all\"\n";
	goto ERROR_EXIT;
}

chdir "$UPDATE_PROJ/source";
print "cd $UPDATE_PROJ/source";
$command = "make stage link >>& $UPDATE_LOG";
print "$command\n";
if(&fork("$command")){
	print "Error! \"make stage link\"\n";
	goto ERROR_EXIT;
}

chdir $UPDATE_PROJ;

# バックアップディレクトリの削除
if( -d "$PROJ_BACK_DIR" ){
    &fork("rm -rf $PROJ_BACK_DIR");
}
# ログバックアップの削除
if( -e "$LOG_BACK_FILE" ){
    &fork("rm -f $LOG_BACK_FILE\n");
}

# バックアップ
if( -d $PROJ_DIR){
    if(&fork("mv $PROJ_DIR $PROJ_BACK_DIR")){
		print "Error! \"mv $PROJ_DIR $PROJ_BACK_DIR\"\n";
		goto ERROR_EXIT;
    }
}
# ログバックアップ
if( -e $LOG_FILE){
    if(&fork("mv $LOG_FILE $LOG_BACK_FILE")){
		print "Error! \"mv $LOG_FILE $LOG_BACK_FILE\"\n";
		goto ERROR_EXIT;
    }
}

# 更新
if(&fork("mv $UPDATE_PROJ $PROJ_DIR")){
    print "Error! \"mv $UPDATE_PROJ $PROJ_DIR\n";
	goto ERROR_EXIT;
}
# ログ更新
if(&fork("mv $UPDATE_LOG $LOG_FILE")){
    print "Error! \"mv $UPDATE_LOG $LOG_FILE\n";
	goto ERROR_EXIT;
}

# 終了
print "\nComplete Update Job!\n\n";
exit 0;


ERROR_EXIT:
# エラー処理

$USER_LIST = "/u/home/user/usrlist/mgs2_mail/mgs2x_game_log_user.lst";
$ADDR_LIST = "/u/home/user/usrlist/kcejw_user_address.lst";

$log_text      = $UPDATE_LOG;
$mail_text     = "$ENV_DIR/mail.txt";
$mail_header   = "/u/develop/mj001b/tools/bin/linux/mail_text/mgs2x_game_update_header.txt";
$subject_text  = "Update failed. (paella : mgs2x_game_update_cron)";

@ADDRESS   = ();
%ADDRESS_LIST = ();

# 警告メールを配信する

# アドレス先を取得
open(ADDRFILE, "$USER_LIST") or die "Not Found.($USER_LIST)\n" ;
foreach $line ( <ADDRFILE> ){
	chop $line;
	$line =~ s/^[\s]*//;

	next, if($line =~ m/^\#/);

	foreach $user (split(/[\s]+/, $line)){
		$addr = &GetAddress($ADDR_LIST, $user);

		die "No User.($user)", if(length($addr) == 0);

		print "send user = $addr\n";
		push( ADDRESS, "$addr\@sol.konami" );
		last;
	}
}
close(ADDRFILE);

# メール本文作成
unlink $mail_text, if(-e $mail_text);
&fork("cp $mail_header $mail_text");
open(MAIL, ">> $mail_text");
open(LOG, "tail -n 50 $log_text |");
foreach $line (<LOG>){
	# ログファイルはEUCなのでSJISに変換
	&jcode'euc2sjis(*line);
	print MAIL "$line";
}
close(LOG);
close(MAIL);

#&fork("tail -n 50 $log_text >>& $mail_text");

# メール配信
print "mail -s \"$subject_text\" @ADDRESS\n";
&fork("mail -s \"$subject_text\" @ADDRESS < $mail_text");

# メール配信用ファイルの削除
unlink $mail_file;

exit 1;


# シェルでコマンドを実行
sub fork
{
    local $pid ;

    unless ( $pid = fork )
    {
        exec( "/bin/tcsh", "-c", "$_[0]" ) ;
        exit 0 ;
    }
    waitpid( $pid, 0 ) ;
    return $? ;
}

# アドレス情報を格納
# 引数として
# $path = アドレスファイルへのパス
# $user = アドレスの欲しいユーザー名
sub GetAddress()
{
	local ($path, $user) = @_;
	local $list, $flag, $addr;

	$flag = 0;
	open(LIST, $path);
	foreach $list ( <LIST> ){
		chop $list;

		$list =~ s/^[\s]*//;

		next, if(length($list) == 0);
		next, if($list =~ m/^\#/);

		$list =~ s/^([^\[]*)\[([^\[\]]*)\]/$1 $2/;

		if($1 eq $user){
			$flag = 1;
			$addr = $2;
			last;
		}
	}
	close(LIST);

	if($flag == 1){
		return $addr;
	}else{
		return "";
	}
}
