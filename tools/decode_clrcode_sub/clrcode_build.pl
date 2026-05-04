#!/usr/bin/perl

my @param;

while($line = <STDIN>) {
	$line =~ s/\n$//;
	$line =~ s/\r$//;
	$_ = $line;
	if(/^\#/) { next; }  # 行頭が '#' だったら注釈行とみなす

	# 前後の空白を除去
    $line =~ s/^[ \t]*//g;
    $line =~ s/[ \t]*$//g;
    if($line eq "") { next; }

    ($title, $radar, $playtime, $fire, $damage, $alart, $kill,
     $clearing, $extend, $cont, $ration, $save, $mech, $version, $level,
     $stage, $dogtag, $tanker, $plant, $sealouce) = split ',', $line;

    if($title eq "") { next; }

    # 必要に応じ、データに改竄を加える
    # $dogtag = 100;  # ドッグタグ回収率は強制的に100%

    # 取得したパラメータから、デコード結果となる数値を作成
    # ClrCode 1
    $param[0] =
	(($damage << 27) & 0xf8000000) |
	    (($fire << 17) & 0x07fe0000) |
		(($playtime << 2) & 0x0001fffc) |
		    ($radar & 0x00000003);
    
    # ClrCode 2
    $param[1] =
	(($extend << 31) & 0x80000000) |
	    (($sealouce << 30) & 0x40000000) |
		(($tanker << 24) & 0x3f000000) |
		    (($clearing << 16) & 0x00ff0000) |
			(($kill << 8) & 0x0000ff00) |
			    ($alart & 0x000000ff);

    # ClrCode 3
    $param[2] =
	(($plant << 26) & 0xfc000000) |
	    (($mech << 18) & 0x03fc0000) |
		(($save << 11) & 0x0003f800) |
		    (($ration << 6) & 0x000007c0) |
			($cont & 0x0000003f);
    
    # ClrCode 4
    $param[3] =
	(($dogtag << 7) & 0x00003f80) |
	    (($stage << 5) & 0x00000060) |
		(($level << 2) & 0x0000001c) |
		    ($version & 0x00000003);


    $cmd = sprintf "./encode_sub %08x %08x %08x %08x", @param;
    open ENC, "$cmd |" || die "could not execute.";
    $code = <ENC>;
    close ENC;
    $code =~ s/\n$//;
    printf "%s : %s\n", $code, $title;
}
1;
