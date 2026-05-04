$version = "rt-2.30" ;

$homedir = "/u/develop/mj001data" ;
$basedir = $homedir."/httpd/cgi-bin/".$version ;
$logdir = $basedir."/log" ;
$msgdir = $basedir."/msg" ;

$debugdir  = "debug" ;

$runtime_dir = $homedir."/runtime.xbox" ;
$fix_dir     = $homedir."/fix.xbox" ;
$work_dir    = $homedir."/tmp.xbox" ;
$tooldir     = "/u/develop/mj001data/bin" ;
$icon_dir    = $homedir."/httpd/html/runtime/images/icon_pool" ;
$backup_dir  = $homedir."/backup.xbox" ;
$test_dir    = $homedir."/test" ;
$listdir     = $homedir."/httpd/cgi-bin/html/list" ;
$lstlog_dir  = $homedir."/httpd/html/runtime.xbox/list" ;

$err_tmpfile = $msgdir."/err_tmp" ;
$errlog_dir  = $logdir."/err" ;

#ver2.0
$postdir   = $msgdir."/post_pool" ;
$bbsdir    = $msgdir."/bbs_pool"  ;
$entrydir  = $msgdir."/ent_pool"  ;
$fixrundir = $msgdir."/fix_pool"  ;
$bbstime   = $msgdir."/bbs_time"  ;
$strpool   = $msgdir."/str_pool/strcode.lst"  ;
$infrundir = $msgdir."/inf_pool"  ;
$infreport = $msgdir."/report.inf"  ;

$conffile = $basedir."/fix_run.conf" ;
$com_dir  = $basedir."/command" ;

$usrlst_dir  = "/u/home/user/usrlist" ;


# for test Mode
$work2_dir   = $homedir."/tmp2" ;



#BBS本体
$bbsfile   = $msgdir."/bbs" ;

#ランタイムエントリーファイル
$entryfile = $msgdir."/entries" ;

#テンポラリーエントリーファイル
$tentryfile = $msgdir."/tentries" ;

#Fixエントリーファイル
$fixupfile = $msgdir."/fixup" ;

#fixレポートファイル
$reportfile = $msgdir."/report" ;

#プロファイル（個人情報）
$proffile  = $msgdir."/prof/profile" ;

#削除情報
$delfile   = $msgdir."/delfile" ;

#各種テンポラリファイル
$tmpfile   = $msgdir."/.tmp" ;
$tmparea   = $tmpfile."$ENV{'REMOTE_ADDR'}.1" ;
$tmplist   = $tmpfile."$ENV{'REMOTE_ADDR'}.2" ;

#コマンド群
$mdl2kms    = $tooldir."/mdl2kms" ;
$mdl2kc     = $tooldir."/mdl2kc" ;
$km2tokms   = $tooldir."/km2tokms" ;
$kms_util   = $tooldir."/kms_util" ;
$dar        = $tooldir."/dar" ;
$mtn_conv   = $tooldir."/mtn_conv" ;
$texconv    = $tooldir."/texconv" ;
$mergescene = $tooldir."/mergescene" ;
$mailmsg    = $tooldir."/mailmsg" ;
$nkf        = $tooldir."/nkf" ;
#
# fix_up mode
#
#
$MODE_FIX   = "fix" ;
$MODE_FIXED = "fxd" ;
$MODE_ABORT = "err" ;
$MODE_RNTIM = "rtm" ;

#
# DO NOT change any sentence blow here!!!!
#
#HTML とも密接に関係している値

%where = ("Fix"    => $fix_dir                , "Temporary"    => $temp_dir,
	  "gifFix" => "/runtime/images/fix.gif", "gifTemporary" => "/runtime/images/tmp.gif" ) ;

#FIXとRuntimeファイルの
%run_to_fix = ("mdl" => "kms cv2 dar",
	       "mtn" => "mt3",
	       "scn" => "hzx",
	       "pic" => "bmp"
	       ) ;

#２つ対になって更新するファイルの拡張子
#２つのファイルは同じディレクトリになければならない。
%comb   = ( "mdl" => "rpt",  "far" => "fh" ) ;
%couple = ( "de2" => "lst" ) ;

#ファイルエントリー
#この中にある拡張子のみランタイムにセーブすることができる。
@cnfm_ext = ("mdl",
	     "rmt", "far", "mtn", "sev",
	     "tri", "bmp", "pic",
	     "row", "raw", "rim", "rap", "rpd", "rtx",
	     "lt2",
	     "hzx",
	     "scn", "kls", "xsi",
	     "de2", "lst",
	     "vib",
	     "ipu", "mpv", "m2v",
	     "pcm", "efx", "wvx", "mdx", "wav","mme",
	     "inf",
	     "l2d" ) ;

#IPメッセージ発信用 名前表
%msg_name = ("松花" => "matsuhana_",
	     "平野" => "hirano_",
	     "野尻" => "nojiri_",
	     "福島" => "fukushima_",
	     "向手" => "mukaide_",
	     "吉池" => "yoshiike_",
	     "山下" => "yamashita_",
	     "兼吉" => "kaneyoshi_" ,
	     "上原" => "masa_uehara_" ,

	     "植原" => "uehara_",
	     "是角" => "korekado_",
	     "園山" => "sonoyama_",
	     "高部" => "takabe_",
	     "岡嶋" => "okajima_",
	     "狩野" => "kano_",
	     "重野" => "shigeno_",
	     "吉良" => "kira_",
	     "里吉" => "satoyoshi_",
	     "奥田" => "okuta_",
	     "国部" => "kunibe_",
	     "柴田" => "shibata_",
	     "小林（聖）" => "s_kobayashi_",
	     "森田" => "morita_",
	     "矢野" => "yano_",

	     "水谷" => "mizutani_",
	     "中村" => "nakamura_",
	     "根岸" => "negishi_",
	     "佐々木" => "sasaki_",
	     "木村" => "kimura_",
	     "鹿間" => "shikama_",
	     "西城" => "nishiki_",
	     "茂木" => "mogi_",
	     "村上" => "murakami_",
	     "斉藤" => "saitoh_",
	     "田中（修）" => "tanaka_s_",
	     "榊原" => "sakakibara_",
	     "勝村" => "katsumura_",
	     "小林（政）" => "m_kobayashi_",
	     "土田" => "tsutsida_",
	     "森" => "mori_",
	     "田中（奈）" => "natsu_tanaka_",
	     "金" => "kimu_",
	     "鎌田" => "kamata_",
	     "中" => "nakama_",

	     "豊田" => "toyota_",
	     "金田" => "kaneda_",
	     "吉村" => "yoshimura_",
	     "西村" => "nishimura_",
	     "吉永" => "yoshinaga_",
	     "佐藤（久）" => "k_satou_",
	     "井上" =>  "inoue_",
	     "赤司" =>  "akashi_",
	     "清水" =>  "shimizu_",
	     "大森" =>  "ohomori_",
	     "松井" =>  "h_matsui_",
	     "小野寺" => "onodera_",

	     "田中（信）" => "tanaka_n_",

	     "村岡" => "maraoka_",
	     "日比野" => "hibino_",
	     "牧村" => "makimura_",
	     "戸島" => "tojima_",

	     "全員"             => "",
	     "シナリオ班"       => "scn",
	     "プログラマ"       => "prog",
	     "エフェクト"       => "effect",
	     "敵兵"             => "enemy",
	     "敵兵＆プレイヤー" => "ene_player",
	     "プレイヤー" 	=> "player",
	     "システム  " 	=> "system",
	     "プレビュー" 	=> "preview",
	     "デザイナー" 	=> "design",
	     "ステージ"   	=> "design_stage",
	     "キャラ"     	=> "design_chara",
	     "ツール班"   	=> "tool",
	     "デモ班"     	=> "design_demo",
	     "デモサウンド班"  	=> "sound_demo",
	     "無線機班"     	=> "radio",
	     "本人"             => ""
	     ) ;

#人体モデル 名前表
%human_name = (
       "01sna_def" => "スネーク",
       "02snh_snake_hands" => "スネーク主観腕",
       "03sna_radio" => "スネーク（通信画面）",
       "04sna_def_handanime" => "スネーク（手アニメ）",
       "05sna_skl" => "スネーク（CV2用）",
       "06sna_coat" => "スネーク（雨合羽）",
       "07sna_dive" => "スネーク（バンジー用）",
       "10gbs_def" => "ゴルルコ兵",
       "11gbs_demo" => "ゴルルコ兵（デモ用）",
       "12gbs_face" => "ゴルルコ兵（口パク）",
       "13gbs_hand" => "ゴルルコ兵（手のアニメ）",
       "14gbc_def" => "ゴルルコ通信兵",
       "15gba_def" => "重装備ゴルルコ兵",
       "16gba_nvgpuls" => "重装備ゴルルコ兵（ナイトビジョン）",
       "20org_def" => "オルガ",
       "21sco_scott" => "スコットドルフ",
       "22tnc_def" => "タンカークルー",
       "23tnr_def" => "タンカークルー（雨合羽）",
       "24crg_def" => "セルゲイ",
       "25us_parts" => "海兵隊",
       "26rev_coat" => "オセロット（コート）",
       "27rev_def" => "オセロット",
       "28rev_liq" => "オセロット（リキッド）",

       "40rai_def" => "ライデン",
       "41rah_raiden_hand" => "ライデン主観腕",
       "42rai_diver" => "ライデン（水中装備）",
       "43rai_naked" => "ライデン（裸）",
       "44rai_radio" => "ライデン（通信画面）",
       "45rai_gbs" => "ライデン（ゴルルコ兵装）",
       "50iro_def" => "イロコイプリスキン",
       "51iro_radio" => "イロコイプリスキン(通信画面)",
       "52otc_def" => "オタコン",
       "53ema_def" => "エマ",
       "54ptr_def" => "ピーター",
       "55ric_def" => "エイムズ",
       "56ric_radio" => "エイムズ(通信画面)",
       "57jam_def" => "大統領",
       "58cam_def" => "キャンベル大佐",
       "59ros_def" => "ローズ",
       "60vmp_coat" => "ヴァンプ（コート）",
       "61vmp_def" => "ヴァンプ",
       "62fat_def" => "ファットマン",
       "63for_coat" => "フォーチュン（コート）",
       "64for_def" => "フォーチュン",
       "65org_plant" => "オルガ（プラント編）",
       "66rev_plant" => "オセロット（プラント編）",
       "67sol_def" => "ソリダス",
       "70gps_def" => "プラントゴルルコ兵",
       "71gpa_def" => "プラントゴルルコ兵重装備",
       "72htc_def" => "ハイテク兵",
       "73tng_def" => "天狗",
       "74org_tng" => "忍者",
       "75vip_def" => "VIP守備兵 ",
       "80sel_bomb" => "シールズ（爆弾処理班）",
       "81sel_def"  => "シールズ",
       "83sel_dead" => "シールズ(死体)",
       "84hos_def" => "人質",
       "85nyp_def" => "NYPD",
       "84cit_def" => "マンハッタン市民",
       "90hnm_hunamushi" => "船虫",
       "91kmo_def" => "カモメ",
       "92par_def" => "オウム"
	       ) ;

%name_tag = ("Humanタイプ" => "htype 付属品"   ,
	     "モデル"      => "model シングル",
	     "LOD"         => "lod Default"   ,
	     "フォグR"	   => "rfog 0" ,
	     "フォグG"	   => "gfog 0" ,
	     "フォグB"	   => "bfog 0"  ,
	     "フォグNear"  => "nfog 0" ,
	     "フォグFar"   => "ffog 0"  ,
	     "キャラ平行X" => "cprl_dir_x 0",
	     "キャラ平行Y" => "cprl_dir_y 0",
	     "キャラ平行Z" => "cprl_dir_z 0",
	     "キャラ平行R" => "cprl_col_r 0",
	     "キャラ平行G" => "cprl_col_g 0",
	     "キャラ平行B" => "cprl_col_b 0",
	     "キャラ環境R" => "camb_col_r 0",
	     "キャラ環境G" => "camb_col_g 0",
	     "キャラ環境B" => "camb_col_b 0",
	     "位置X"       => "offs_pos_x 0" ,
	     "位置Y"       => "offs_pos_y 0" ,
	     "位置Z"       => "offs_pos_z 0"  ,
	     "回転X"       => "offs_rot_x 0" ,
	     "回転Y"       => "offs_rot_y 0" ,
	     "回転Z"       => "offs_rot_z 0"  ,
	     "Worldタイプ" => "wtype 小物"     ,
	     "ライト名"    => "light なし"    ,
	     "原点X"       => "cen_pos 0"    ,
	     "原点Y"       => "cen_pos 0"    ,
	     "原点Z"       => "cen_pos 0"     ,
	     "中心"        => "center 底中心" ,
	     "OFS詳細"     => "offs_com なし"   ) ;

%weapon_tag =  ("01m92" => "M9麻酔銃【スネーク使用武器】",
		"02usp" => "USP【スネーク使用武器】",

		"03scm" => "ソコム【ライデン使用武器】",
		"04fms" => "ファマス【ライデン使用武器】",
		"05spp" => "SPP_1M【ライデン使用武器】",
		"06psg" => "PSG-1【ライデン使用武器】",
		"07stg" => "スティンガー【ライデン使用武器】",
		"08nkt" => "ニキータ【ライデン使用武器】",
		"09clm" => "クレイモア地雷【ライデン使用武器】",
		"10cfr" => "Ｃ４爆弾【ライデン使用武器】",
		"11rgb" => "RGB_6【ライデン使用武器】",
		"12grl" => "グレネードランチャー【ライデン使用武器】",
		"13stc" => "ステルスカメラ【ライデン使用武器】",
		"14dcy" => "デコイ：風船ライデン【ライデン使用武器】",
		"15dmp" => "指向性マイク【ライデン使用武器】",
		"16hfb" => "高周波ブレード【ライデン使用武器】",
		"17cls" => "爆弾解体スプレー【ライデン使用武器】",
		"18cms" => "死体溶解スプレー【ライデン使用武器】",
		"19fex" => "消火器【ライデン使用武器】",
		"21wpb" => "週刊誌【ライデン使用武器】",
		"23grenade"       => "グレネード【ライデン使用武器】",
		"24chaff_grenade" => "チャフ・グレネード【ライデン使用武器】",
		"25stun_grenade"  => "スタン・グレネード【ライデン使用武器】",

		"29mkr" => "マカロフ【ゴルルコ兵使用武器】",
		"30aks" => "AKS_74U【ゴルルコ兵使用武器】",

		"31m4"     => "m4a1【米海軍兵使用武器】",
		"32spas12" => "スパス【米海軍兵使用武器】",

		"33saa" => "SAA【オセロット使用武器】",
		"34p90" => "P９０【天狗兵使用武器】",
		"35tkf" => "スローイングナイフ【天狗兵使用武器】",
		"36tbl" => "天狗兵刀【天狗兵使用武器】",

		"37ssk" => "special scout knife【ボス使用武器】" ) ;

# "オルガ" で "orgaF"となっているのは orga.inf で女性なので F
# "スネーク" で "snakeM"となっているのは snake.inf で男性なので M
#
#
%inf_name = ("スネーク" => "snakeM" ,
	     "wavsna"   => "snakeM" ,
	     "2677990"  => "sna",
	     "sna" => "2677990",

	     "オタコン" => "otaconM" ,
	     "wavotc"   => "otaconM" ,
	     "wavota"   => "otaconM" ,
	     "4896105"  => "ota",
	     "ota" => "4896105",

	     "オルガ"   => "orpF" ,
	     "plant_wavorg"   => "orpF" ,
	     "16138912" => "orp",
	     "orp" => "16138912",

	     "タンカーオルガ"   => "orgF" ,
	     "wavorg"   => "orgF" ,
	     "10272771" => "org",
	     "org" => "10272771",

	     "メイリン" => "meiF",
	     "wavmei"   => "meiF" ,
	     "10198823" => "mei",
	     "mei" => "10198823",

	     "オセロット" => "revM",
	     "wavrev"   => "revM",
	     "7666070"   => "rev",
	     "rev" => "7666070",

	     "セルゲイ" => "crgM",
	     "wavcrg"   => "crgM",
	     "701789"   => "crg",
	     "crg" => "701789",

	     "スコット" => "scoM",
	     "wavsco"   => "scoM",
	     "9107709"   => "sco",
	     "sco" => "9107709",



	     "敵兵" => "gbsM",
	     "wavgbs"   => "gbsM",
	     "6633980"   => "gbs",
	     "gbs" => "6633980",

	     "海兵" => "usaM",
	     "wavusa"   => "usaM",
	     "6040060"   => "usa",
	     "usa" => "6040060",

	     "指揮官"   => "comM",
	     "wavgbs"   => "comM",
	     "14547124" => "com",
	     "com"      => "14547124",



	     "ヴァンプ" => "vmpM",
	     "wavvmp"   => "vmpM",
	     "7063499"   => "vmp",
	     "vmp" => "7063499",

	     "フォーチュン" => "forF",
	     "wavfor"   => "forF",
	     "11843538"   => "for",
	     "for" => "11843538",

	     "ファットマン" => "fatM",
	     "wavfat"   => "fatM",
	     "1883539"   => "fat",
	     "fat" => "1883539",

	     "エマ" => "emaF",
	     "wavema"   => "emaF",
	     "5584254"   => "ema",
	     "ema" => "5584254",

	     "ローズ" => "rosF",
	     "wavros"   => "rosF",
	     "2328243"   => "ros",
	     "ros" => "2328243",


	     "大佐顔ローズ" => "bugF",
	     "wavros"   => "bugF",
	     "1951143"   => "bug",
	     "bug" => "1951143",


	     "ライデン" => "raiM",
	     "wavrai"   => "raiM",
	     "10163495"   => "rai",
	     "rai" => "10163495",


	     "プリスキン" => "iroM",
	     "waviro"   => "iroM",
	     "6401406"   => "iro",
	     "iro" => "6401406",

	     "シールズ" => "selM",
	     "wavsel"   => "selM",
	     "1808488"   => "sel",
	     "sel" => "1808488",

	     "シールズ隊長" => "serM",
	     "wavsel"   => "serM",
	     "15096511"   => "ser",
	     "ser" => "15096511",

	     "ピーター" => "ptrM",
	     "wavptr"   => "ptrM",
	     "1765162"   => "ptr",
	     "ptr" => "1765162",

	     "大統領" => "jamM",
	     "wavjam"   => "jamM",
	     "14061600"   => "jam",
	     "jam" => "14061600",

	     "人質" => "hosM",
	     "wavhos"   => "hosM",
	     "6474817"   => "hos",
	     "hos" => "6474817",

	     "大佐" => "camM",
	     "wavcam"   => "camM",
	     "6599668"   => "cam",
	     "cam" => "6599668",

	     "エイムズ" => "ricM",
	     "wavric"   => "ricM",
	     "10184878"   => "ric",
	     "ric" => "10184878",

	     "オウム" => "parF",
	     "wavpar"   => "parF",
	     "16068308"   => "par",
	     "par" => "16068308",


	     "ジョニー" => "jonM",
	     "wavjon"   => "jonM",
	     "13310004"   => "jon",
	     "jon" => "13310004",


	     "ミスターＸ"   => "mrxM" ,
	     "wavmrx"   => "mrxM" ,
	     "13098211" => "mrx",
	     "mrx" => "13098211",


	     "ソリダス" => "solM",
	     "wavsol"   => "solM",
	     "16444913"   => "sol",
	     "sol" => "16444913",


	     "兼吉" => "kaneyoshi" ) ;

1 ;
