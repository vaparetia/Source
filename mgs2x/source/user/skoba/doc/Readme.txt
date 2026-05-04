/*
	Readme.txt
	skobaディレクトリ構成

	2001/10/23	S.Kobayashi
	$Id: Readme.txt,v 1.1.1.3 2002/11/19 11:50:02 Yoshizawa1 Exp $

*/

■始めに....
ディレクトリと同じ名前がファイル名になっているテキストファイルがあります。
このファイルは、そのディレクトリ直下にあるファイルが全て含まれたドキュメントになっています。
ただし、doc無しとなっているものは含まれていません。
○参照となっている場合はその人のdocを参照してください。

■構成
bullet/ 各種弾制御関係 bullet.txt参照
blast.c						爆発処理。エフェクトではない。					doc有
blast_water.c				水中爆発処理。エフェクトではない。				doc有
blst_chf.c					チャフ爆発処理。エフェクトではない。			doc有
blst_stn.c					スタン爆発処理。エフェクトではない。			doc有
bmbctrl.c					爆発系ミサイル系武器の一括管理システム。		doc有
bul_book.c					設置された雑誌。								doc有
bul_c4.c					設置されたＣ４、解体Ｃ４、ファットマンＣ４共通プログラム。doc有
bul_c4.h					bul_c4.cのヘッダ。								doc無
bul_c4_mng.c				設置されたＣ４、解体Ｃ４、ファットマンＣ４の
							一括管理プログラム。							doc有
bul_clay.c					設置されたクレイモア。							doc有
bul_fatman_c4_mng.c			ファットマンＣ４一括管理。						doc有
bul_gre.c					投擲されたグレネード系武器。					doc有
bul_m4demo.c				デモ用のM４グレネード弾発射プログラム           doc有
bul_nkt.c                   ニキータ弾道制御								doc有
bul_rgb6.c					ＲＧＢ６弾道制御。								doc有
bul_spray.c					凍結スプレー当たり判定処理。					doc有
bul_stg.c                   スティンガー弾道制御							doc有
bullet.c					汎用弾弾道制御。								doc有
water_mine.c				水中機雷。										doc有
makefile			        メイクファイル									doc無

equip/ プレイヤー装備品 equip.txt参照
bandana.c					無限バンダナ。									doc有
box_hidden.c				ステージと同じ柄のダンボールを
							被っていれば見つからない処理。					doc有
bsensB.c					センサーＢ。									doc有
c_box.c						ダンボール。									doc有
capture.c                   JPEG化関数										doc有
capture.h					JPEG化関数各種設定								doc無
cig.c						たばこ。										doc無
demo_dgcam_mng.c			デモ用カメラマネージャ							doc有
demo_evm_skirt.c			デモ用EVMスカート								doc有
dg_cam.c					タンカー編写真判定関数							doc有
dg_cam_mng.c				カメラマネージャ								doc有
eq_magazine.c				未使用。										doc無
evm_skirt.c			        EVMスカート										doc有
goggles.c					サーマル、ナイトビジョン管理					doc有
jacket.c					未使用。										doc無
makefile				    メイクファイル									doc無
scope.c						スコープ管理									doc有
stealth.c					ステルス迷彩。									doc有
uniform.c					ゴルルコ兵装。									doc有
wig.c						カツラ共通。									doc有

irs/  田中（秀之）バックアップ保管場所。ゲーム中では一切使われていません
Senser.c				     未使用											doc無
Senser.h					 未使用											doc無
_objlist					 未使用											doc無
irs.c						 未使用											doc無
irs_break.c					 未使用											doc無
irs_lamp.c					 未使用											doc無
irs_mng.c					 未使用											doc無
makefile					 未使用											doc無
newirs.c					 未使用											doc無
newirs.h					 未使用											doc無

etc/ 便利関数群、メニュー関係 etc.txt参照 
albam.c                      アルバムモード									doc有
albam_name.c				 アルバムモードネームエントリー					doc有
album.h						 アルバムモードヘッダ							doc無
album_mng.c					 アルバムモード管理								doc有
black_rect.c				 未使用											doc無
boss_etc.c					 ボスラッシュテロップ							doc有
boss_pause.c				 ボスラッシュポーズ画面							doc有
boss_result.c				 ボスラッシュリザルト表示						doc有
clear_result.c				 クリア時リザルト表示							doc有
demo_c4.c					 デモ用C４										doc有
demo_nkt_smoke.c			 デモ用ニキータ煙エフェクト						doc有
dogtag2.c					 ドッグタグビューワ								doc有
dogtag_mng.c				 ドッグタグビューワ管理							doc有
encute.c					 きくちゃんアンケート							doc有
gameover.c					 PALNT編ゲームオーバー							doc有
makefile					 メイクファイル									doc無
mc_dogtag_mng.c				 ドッグタグビューワメモリーカード管理			doc有
mc_mng.c					 アルバムモードメモリーカード管理				doc有
name_layout.c				 ネームエントリー画面							doc有
name_layout.h				 ネームエントリー画面							doc無
photo_save.c				 写真セーブ画面									doc有
previous_story.c			 シャドーモセスの真実							doc有
print_test.c				 未使用											doc無
sk_jimaku.c					 未使用											doc無
web_site.c					 WEBサイト画面									doc有
world_map_bug_layout.c		 全体マップバグ演出								doc有
world_map_layout.c			 全体マップ枠									doc有
world_map_sub_layout.c		 全体マップ賑やかし								doc有

test/ 複数赤外線 、 C４凍結エフェクト等 test.txt参照
Senser.c                     赤外線センサー									doc有
Senser.h					 赤外線センサーヘッダ							doc無
TestBox.c					 未使用											doc無
c4_comdl.c					 C４凍りエフェクト								doc有
c4_eff.c					 C４ランプエフェクト							doc有
c4_ice.c					 冷却エフェクト									doc有
c4_ice2.c					 未使用											doc無
c4_ice_mng.c				 冷却エフェクト管理関数							doc有
etc.c						 便利関数郡										doc無
etc.h						 便利関数郡ヘッダ								doc無
irs_lamp.c					 IRSランプ										doc有
irs_mng.c					 IRSマネージャ									doc有
makefile					 メイクファイル									doc無
newirs.c					 IRS本体										doc有
newirs.h					 IRSヘッダ										doc無
sensor_lamp.c				 センサーランプエフェクト						doc有
sk_printf.c				     コバ４PRINTF関数								doc無
test_sprite.c                未使用											doc無

weapon/ サイト系エフェクト郡 、デモコントロール関数 weapon.txt参照
ai_ray_layout.c				AIRAYサイト										doc有
arms_con.c                  デモ武器エフェクト管理関数						doc有
arms_data.h                 デモ武器エフェクト管理関数ヘッダ				doc無
blade.c						高周波ブレード。								doc有
book.c						雑誌。											doc有
cfr.c						Ｃ４、クレイモア共通。							doc有
clay_eff.c                  クレイモア火線エフェクト						doc有
clay_mng.c                  クレイモア管理関数								doc有
demo_dg_cam_layout.c        デモ用デジタルカメラレイアウト					doc有
demo_dg_cam_shuter_layout.c デモ用デジタルカメラシャッターエフェクト		doc有
dg_cam_layout.c             デジタルカメラレイアウト						doc有
dg_cam_shuter_layout.c      デジタルカメラシャッターエフェクト				doc有
equip_layout.c				装備説明画面レイアウト							doc有
grenade.c					グレネード系共通。								doc有
makefile                    メイクファイル									doc無
night_layout.c				ナイトビジョンレイアウト						doc有
nikita_layout.c				ニキータレイアウト								doc有
nkt.c						ニキータ管理関数								doc有
psg.c						PSG１管理関数									doc有
psg_layout.c                PSG１レイアウト									doc有
ray_layout.c                RAY主観レイアウト								doc有
ray_uv_layout.c				未使用											doc無
scope_layout.c              スコープレイアウト								doc有
spray.c						スプレー、集音マイク共通。						doc有
stg.c                       スティンガー管理関数							doc有
stg_layout.c				スティンガーレイアウト							doc有
stg_uv_layout.c				未使用											doc無
tdg_cam_layout.c			タンカーカメラレイアウト						doc有
thermal_layout.c			サーマルゴーグルレイアウト						doc有
usp.c						ハンドガン、マシンガン共通。					doc有
vtr_layout.c                VTRレイアウト									doc有
weapon_mng.c				未使用											doc無
wp_mng.c					武器表示制御。									doc有

weapon_old/ 武器エフェクト郡 weapon_old.txt参照
LineSmokeMng.c               未使用											doc無
big_weapon.h				 武器エフェクト関数externヘッダ					doc無
blowback.c                   ブローバック関数								doc有
cartridge.c                  武器のカートリッジ関数							doc有
clay_bomb_smoke.c            クレイモア爆発煙関数							doc有
connect_smoke.c              付随煙関数										岡嶋さんdoc参照
connect_smoke_mng.c          未使用											doc無
connect_smoke_skoba.c        ポインタ参照先に接続する煙						岡嶋さんdoc参照
connect_smoke_skoba2.c		 ポインタ参照先に接続する煙						doc有
crtrg.c                      カートリッジ関連								岡嶋さんdoc参照
crtrg_ft.c                   カートリッジ落下関連							岡嶋さんdoc参照
emb_call.c																	柴田さんdoc参照
emb_control.c																柴田さんdoc参照
emb_control.h																柴田さんdoc参照
gunsmoke_single.c            武器のマズルフラッシュ							doc有
gunsmoke_single2.c           武器のマズルフラッシュ							doc有
line_box.c                   未使用									        doc無
makefile                     メイクファイル									doc無
matsu.h                      ブローバック構造体ヘッダ						doc無
mazzle.c                     武器のマズルフラッシュ管理関数					doc有
mazzleflash.c				 単発銃のマズルフラッシュ						doc無
mazzleflash2.c				 単発銃のマズルフラッシュ改良型					doc無
mazzleflash_sub.c			 マズルフラッシュ星部分(未使用)					doc無
mazzleflash_sub2.c			 マズルフラッシュ星部分改良型					doc有
mdl_cartridge.c              薬莢の３Dモデル(旧バージョン)					doc有
p90_mazzle.c				 未使用											doc無
smoke.c						 煙エフェクト									doc有
smoke2.c					 ポインタ参照先に接続する煙エフェクト			doc有
smokeflash.c				 未使用											doc無
sps_amo.c                    未使用											doc無
sps_mazzle.c                 未使用											doc無
sps_smoke.c                  未使用											doc無
stg_blur.c																	岡嶋さんdoc参照
stg_fall_parts.c															岡嶋さんdoc参照
stg_smoke_back.c															岡嶋さんdoc参照
stg_smoke_front.c															岡嶋さんdoc参照
stg_spark.c																	岡嶋さんdoc参照
sub_mazzle.c	             主観用マズルフラッシュ							doc有
sub_mazzle_mng.c			 主観用マズルフラッシュ管理						doc有
wpeffect.c					 ブローバック関数								doc有
wpeffect.h					 ブローバックヘッダ								doc無

