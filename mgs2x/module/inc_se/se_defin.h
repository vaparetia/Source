enum se_code{

/* 非常駐ＳＥ 256～1567 */
SD_V_GBSDMG01=256,	//敵１・ダメージ//gbsdmg01 256
SD_V_GBSDMG02,	//敵２・ダメージ//gbsdmg02 257
SD_V_GBSDMG03,	//敵３・ダメージ//gbsdmg03 258
SD_V_GBSDMG04,	//敵４・ダメージ//gbsdmg04 259
SD_V_GBSDWN01,	//敵１・キック倒れ//gbsdwn01 260
SD_V_GBSDWN02,	//敵２・キック倒れ//gbsdwn02 261
SD_V_GBSDWN03,	//敵３・キック倒れ//gbsdwn03 262
SD_V_GBSDWN04,	//敵４・キック倒れ//gbsdwn04 263
SD_V_GBSFAL01,	//敵１・落下アウト//gbsfal01 264
SD_V_GBSFAL02,	//敵２・落下アウト//gbsfal02 265
SD_V_GBSFAL03,	//敵３・落下アウト//gbsfal03 266
SD_V_GBSFAL04,	//敵４・落下アウト//gbsfal04 267
SD_V_GBSOUT01,	//敵１・アウト//gbsout01 268
SD_V_GBSOUT02,	//敵２・アウト//gbsout02 269
SD_V_GBSOUT03,	//敵３・アウト//gbsout03 270
SD_V_GBSOUT04,	//敵４・アウト//gbsout04 271
SD_V_GBSFUN01,	//敵１・白兵攻撃気合//gbsfun01 272
SD_V_GBSFUN02,	//敵２・白兵攻撃気合//gbsfun02 273
SD_V_GBSFUN03,	//敵３・白兵攻撃気合//gbsfun03 274
SD_V_GBSFUN04,	//敵４・白兵攻撃気合//gbsfun04 275
SD_V_GBSNEC01,	//敵１・首絞まる//gbsnec01 276
SD_V_GBSNEC02,	//敵２・首絞まる//gbsnec02 277
SD_V_GBSNEC03,	//敵３・首絞まる//gbsnec03 278
SD_V_GBSNEC04,	//敵４・首絞まる//gbsnec04 279
SD_V_GBSNEB01,	//敵１・頚椎折る//gbsneb01 280
SD_V_GBSNEB02,	//敵２・頚椎折る//gbsneb02 281
SD_V_GBSNEB03,	//敵３・頚椎折る//gbsneb03 282
SD_V_GBSNEB04,	//敵４・頚椎折る//gbsneb04 283
SD_V_GBSIBK01,	//敵兵１・いびき//gbsibk01 284
SD_V_GBSIBK02,	//敵兵２・いびき//gbsibk02 285
SD_V_GBSIBK03,	//敵兵３・いびき//gbsibk03 286
SD_V_GBSIBK04,	//敵兵４・いびき//gbsibk04 287
SD_V_GBSSLP01,	//敵兵１・麻酔倒れ//gbsslp01 288
SD_V_GBSSLP02,	//敵兵２・麻酔倒れ//gbsslp02 289
SD_V_GBSSLP03,	//敵兵３・麻酔倒れ//gbsslp03 290
SD_V_GBSSLP04,	//敵兵４・麻酔倒れ//gbsslp04 291
SD_V_C01MAKI,	//敵兵１「グレネード！」//gbs_a004 292
SD_V_C01MORI,	//敵兵２「グレネード！」//gbs_b004 293
SD_V_C01OKAJ,	//敵兵３「グレネード！」//gbs_c004 294
SD_V_C01SHIG,	//敵兵４「グレネード！」//gbs_d004 295
SD_V_C04MAKI,	//敵兵１「くらえ」//gbs_a026 296
SD_V_C04MORI,	//敵兵２「くらえ」//gbs_b026 297
SD_V_C04OKAJ,	//敵兵３「くらえ」//gbs_c026 298
SD_V_C04SHIG,	//敵兵４「くらえ」//gbs_d026 299
SD_V_C07MAKI,	//敵兵１「誰だ！」//gbsdrd01 300
SD_V_C07MORI,	//敵兵２「誰だ！」//gbsdrd02 301
SD_V_C07OKAJ,	//敵兵３「誰だ！」//gbsdrd03 302
SD_V_C07SHIG,	//敵兵４「誰だ！」//gbsdrd04 303
SD_V_C08MAKI,	//敵兵１「クリア」//gbs_a02f 304
SD_V_C08MORI,	//敵兵２「クリア」//gbs_b02f 305
SD_V_C08OKAJ,	//敵兵３「クリア」//gbs_c02f 306
SD_V_C08SHIG,	//敵兵４「クリア」//gbs_d02f 307
SD_V_ATKO01,	//攻撃兵１対離れ「そこにいるぞ！」//gbs_a001 308
SD_V_ATMO01,	//攻撃兵２対離れ「そこだ！」//gbs_b001 309
SD_V_ATKO02,	//攻撃兵３対離れ「そこだな！」//gbs_c001 310
SD_V_ATMO02,	//攻撃兵４対離れ「そこにいる！」//gbs_d001 311
SD_V_ATKO03,	//攻撃兵１対逃げ「逃げたぞ！」//gbs_a002 312
SD_V_ATMO03,	//攻撃兵２対逃げ「追跡しろ！」//gbs_b002 313
SD_V_ATKO04,	//攻撃兵３対逃げ「追え！」//gbs_c002 314
SD_V_ATMO04,	//攻撃兵４対逃げ「待て！」//gbs_d002 315
SD_V_ATKO05,	//攻撃兵１非発砲「撃つな！」//gbs_a003 316
SD_V_ATMO05,	//攻撃兵２非発砲「味方に当たる！」//gbs_b003 317
SD_V_ATKO06,	//攻撃兵３非発砲「くそ！」//gbs_c003 318
SD_V_ATMO06,	//攻撃兵４非発砲「卑怯な！」//gbs_d003 319
SD_V_MAKI20,	//敵兵１足跡モード「ん？」//gbs_a006 320
SD_V_MORI20,	//敵兵２足跡モード「ん？」//gbs_b006 321
SD_V_OKAJ20,	//敵兵３足跡モード「ん？」//gbs_c006 322
SD_V_SHIG20,	//敵兵４足跡モード「ん？」//gbs_d006 323
SD_V_MAKI21,	//敵兵１足跡モード「うん？」//gbs_a007 324
SD_V_MORI21,	//敵兵２足跡モード「うん？」//gbs_b007 325
SD_V_OKAJ21,	//敵兵３足跡モード「うん？」//gbs_c007 326
SD_V_SHIG21,	//敵兵４足跡モード「うん？」//gbs_d007 327
SD_V_MAKI24,	//敵兵１プレイヤー発見モード「あっ」//gbs_a00a 328
SD_V_MORI24,	//敵兵２プレイヤー発見モード「あっ」//gbs_b00a 329
SD_V_OKAJ24,	//敵兵３プレイヤー発見モード「あっ」//gbs_c00a 330
SD_V_SHIG24,	//敵兵４プレイヤー発見モード「あっ」//gbs_d00a 331
SD_V_MAKI27,	//敵兵１味方ダメージモード「んっ！」//gbs_a00d 332
SD_V_MORI27,	//敵兵２味方ダメージモード「んっ！」//gbs_b00d 333
SD_V_OKAJ27,	//敵兵３味方ダメージモード「んっ！」//gbs_c00d 334
SD_V_SHIG27,	//敵兵４味方ダメージモード「んっ！」//gbs_d00d 335
SD_V_MAKI35,	//敵兵１ホールドアップモード「ひぃ」//gbs_a015 336
SD_V_MORI35,	//敵兵２ホールドアップモード「ひぃ」//gbs_b015 337
SD_V_OKAJ35,	//敵兵３ホールドアップモード「ひぃ」//gbs_c015 338
SD_V_SHIG35,	//敵兵４ホールドアップモード「ひぃ」//gbs_d015 339
SD_V_GBS_A016,	//敵兵１ホールドアップ２「お助け」//gbs_a016 340
SD_V_GBS_B016,	//敵兵２ホールドアップ２「勘弁」//gbs_b016 341
SD_V_GBS_C016,	//敵兵３ホールドアップ２「助けて」//gbs_c016 342
SD_V_GBS_D016,	//敵兵４ホールドアップ２「撃たな」//gbs_d016 343
SD_V_MAKI36,	//敵兵１エロ本発見「ん！」//gbs_a018 344
SD_V_MORI36,	//敵兵２エロ本発見「ん！」//gbs_b018 345
SD_V_OKAJ36,	//敵兵３エロ本発見「ん！」//gbs_c018 346
SD_V_SHIG36,	//敵兵４エロ本発見「ん！」//gbs_d018 347
SD_V_MAKI37,	//敵兵１「ゴッ（Ｇｏ)」//gbs_a019 348
SD_V_MORI37,	//敵兵２「ゴッ（Ｇｏ)」//gbs_b019 349
SD_V_OKAJ37,	//敵兵３「ゴッ（Ｇｏ)」//gbs_c019 350
SD_V_SHIG37,	//敵兵４「ゴッ（Ｇｏ)」//gbs_d019 351
SD_V_MAKI39,	//敵兵１「待てっ」//gbs_a021 352
SD_V_MORI39,	//敵兵２「待てっ」//gbs_b021 353
SD_V_OKAJ39,	//敵兵３「待てっ」//gbs_c021 354
SD_V_SHIG39,	//敵兵４「待てっ」//gbs_d021 355
SD_V_GBSNOBI1,	//敵兵伸び始め「んっっ」//gbsnobi1 356
SD_V_GBSNOBI2,	//敵兵伸び終わり「はぁぁ」//gbsnobi2 357
SD_E_EFOOT_1L1,	//敵兵１通常足音（左）//efoot_1l 358
SD_E_EFOOT_1R1,	//敵兵１通常足音（右）//efoot_1r 359
SD_E_EFOOT_1L2,	//敵兵２通常足音（左）//efoot_1l 360
SD_E_EFOOT_1R2,	//敵兵２通常足音（右）//efoot_1r 361
SD_E_EFOOT_1L3,	//敵兵３通常足音（左）//efoot_1l 362
SD_E_EFOOT_1R3,	//敵兵３通常足音（右）//efoot_1r 363
SD_E_EFOOT_1L4,	//敵兵４通常足音（左）//efoot_1l 364
SD_E_EFOOT_1R4,	//敵兵４通常足音（右）//efoot_1r 365
SD_E_EFOOT_2L1,	//敵兵１鉄板床足音（左）//efootm1l 366
SD_E_EFOOT_2R1,	//敵兵１鉄板床足音（右）//efootm1r 367
SD_E_EFOOT_2L2,	//敵兵２鉄板床足音（左）//efootm1l 368
SD_E_EFOOT_2R2,	//敵兵２鉄板床足音（右）//efootm1r 369
SD_E_EFOOT_2L3,	//敵兵３鉄板床足音（左）//efootm1l 370
SD_E_EFOOT_2R3,	//敵兵３鉄板床足音（右）//efootm1r 371
SD_E_EFOOT_2L4,	//敵兵４鉄板床足音（左）//efootm1l 372
SD_E_EFOOT_2R4,	//敵兵４鉄板床足音（右）//efootm1r 373
SD_E_EFOOT_3L1,	//敵兵１濡れ床足音（左）//efootw1l 374
SD_E_EFOOT_3R1,	//敵兵１濡れ床足音（右）//efootw1r 375
SD_E_EFOOT_3L2,	//敵兵２濡れ床足音（左）//efootw1l 376
SD_E_EFOOT_3R2,	//敵兵２濡れ床足音（右）//efootw1r 377
SD_E_EFOOT_3L3,	//敵兵３濡れ床足音（左）//efootw1l 378
SD_E_EFOOT_3R3,	//敵兵３濡れ床足音（右）//efootw1r 379
SD_E_EFOOT_3L4,	//敵兵４濡れ床足音（左）//efootw1l 380
SD_E_EFOOT_3R4,	//敵兵４濡れ床足音（右）//efootw1r 381
SD_E_EFOOT_4L1,	//敵兵１濡れ鉄板床足音（左）//efootw2l 382
SD_E_EFOOT_4R1,	//敵兵１濡れ鉄板床足音（右）//efootw2r 383
SD_E_EFOOT_4L2,	//敵兵２濡れ鉄板床足音（左）//efootw2l 384
SD_E_EFOOT_4R2,	//敵兵２濡れ鉄板床足音（右）//efootw2r 385
SD_E_EFOOT_4L3,	//敵兵３濡れ鉄板床足音（左）//efootw2l 386
SD_E_EFOOT_4R3,	//敵兵３濡れ鉄板床足音（右）//efootw2r 387
SD_E_EFOOT_4L4,	//敵兵４濡れ鉄板床足音（左）//efootw2l 388
SD_E_EFOOT_4R4,	//敵兵４濡れ鉄板床足音（右）//efootw2r 389
SD_E_EFOOT_5L1,	//敵兵１足音５（左）//efoot_1l 390
SD_E_EFOOT_5R1,	//敵兵１足音５（右）//efoot_1r 391
SD_E_EFOOT_5L2,	//敵兵２足音５（左）//efoot_1l 392
SD_E_EFOOT_5R2,	//敵兵２足音５（右）//efoot_1r 393
SD_E_EFOOT_5L3,	//敵兵３足音５（左）//efoot_1l 394
SD_E_EFOOT_5R3,	//敵兵３足音５（右）//efoot_1r 395
SD_E_EFOOT_5L4,	//敵兵４足音５（左）//efoot_1l 396
SD_E_EFOOT_5R4,	//敵兵４足音５（右）//efoot_1r 397
SD_E_EFOOT_6L1,	//敵兵１足音６（左）//efoot_1l 398
SD_E_EFOOT_6R1,	//敵兵１足音６（右）//efoot_1r 399
SD_E_EFOOT_6L2,	//敵兵２足音６（左）//efoot_1l 400
SD_E_EFOOT_6R2,	//敵兵２足音６（右）//efoot_1r 401
SD_E_EFOOT_6L3,	//敵兵３足音６（左）//efoot_1l 402
SD_E_EFOOT_6R3,	//敵兵３足音６（右）//efoot_1r 403
SD_E_EFOOT_6L4,	//敵兵４足音６（左）//efoot_1l 404
SD_E_EFOOT_6R4,	//敵兵４足音６（右）//efoot_1r 405
SD_E_EFOOT_7L1,	//敵兵１足音７（左）//efoot_1l 406
SD_E_EFOOT_7R1,	//敵兵１足音７（右）//efoot_1r 407
SD_E_EFOOT_7L2,	//敵兵２足音７（左）//efoot_1l 408
SD_E_EFOOT_7R2,	//敵兵２足音７（右）//efoot_1r 409
SD_E_EFOOT_7L3,	//敵兵３足音７（左）//efoot_1l 410
SD_E_EFOOT_7R3,	//敵兵３足音７（右）//efoot_1r 411
SD_E_EFOOT_7L4,	//敵兵４足音７（左）//efoot_1l 412
SD_E_EFOOT_7R4,	//敵兵４足音７（右）//efoot_1r 413
SD_E_EFOOT_8L1,	//敵兵１足音８（左）//efoot_1l 414
SD_E_EFOOT_8R1,	//敵兵１足音８（右）//efoot_1r 415
SD_E_EFOOT_8L2,	//敵兵２足音８（左）//efoot_1l 416
SD_E_EFOOT_8R2,	//敵兵２足音８（右）//efoot_1r 417
SD_E_EFOOT_8L3,	//敵兵３足音８（左）//efoot_1l 418
SD_E_EFOOT_8R3,	//敵兵３足音８（右）//efoot_1r 419
SD_E_EFOOT_8L4,	//敵兵４足音８（左）//efoot_1l 420
SD_E_EFOOT_8R4,	//敵兵４足音８（右）//efoot_1r 421
SD_P_FOOT_01L,	//プレイヤー通常足音（左）//foot_01l 422
SD_P_FOOT_01R,	//プレイヤー通常足音（右）//foot_01r 423
SD_P_FOOT_02L,	//プレイヤー鉄板床足音（左）//foot_m1l 424
SD_P_FOOT_02R,	//プレイヤー鉄板床足音（右）//foot_m1r 425
SD_P_FOOT_03L,	//プレイヤー濡れ床足音（左）//foot_w1l 426
SD_P_FOOT_03R,	//プレイヤー濡れ床足音（右）//foot_w1r 427
SD_P_FOOT_04L,	//プレイヤー濡れ鉄板床足音（左）//foot_w2l 428
SD_P_FOOT_04R,	//プレイヤー濡れ鉄板床足音（右）//foot_w2r 429
SD_P_FOOT_05L,	//プレイヤー足音５（左）//foot_m1l 430
SD_P_FOOT_05R,	//プレイヤー足音５（右）//foot_m1r 431
SD_P_FOOT_06L,	//プレイヤー足音６（左）//foot_m1l 432
SD_P_FOOT_06R,	//プレイヤー足音６（右）//foot_m1r 433
SD_P_FOOT_07L,	//プレイヤー足音７（左）//foot_m1l 434
SD_P_FOOT_07R,	//プレイヤー足音７（右）//foot_m1r 435
SD_P_FOOT_08L,	//プレイヤー足音８（左）//foot_m1l 436
SD_P_FOOT_08R,	//プレイヤー足音８（右）//foot_m1r 437
SD_A_WALLM01,	//壁叩く１（通常）//wallm01 438
SD_A_WALLM02,	//壁叩く２（鉄筋）//wallm02 439
SD_A_WALLM03,	//壁叩く３（鉄板）//wallm03 440
SD_A_WALLM04,	//壁叩く４（木）//wallm04 441
SD_A_WALLM05,	//壁叩く５（段ボール）//wallm05 442
SD_A_WALLM06,	//壁叩く６//wallm01 443
SD_A_WALLM07,	//壁叩く７//wallm01 444
SD_A_WALLM08,	//壁叩く８//wallm01 445
SD_A_RICOCH01,	//跳弾１（通常）//ricoch01 446
SD_A_RICOCH02,	//跳弾２（金属１密）//ricmet01 447
SD_A_RICOCH03,	//跳弾３（金属２空洞）//ricmet02 448
SD_A_RICOCH04,	//跳弾４（木箱）//ricbox01 449
SD_A_RICOCH05,	//跳弾５（段ボール）//ricdan02 450
SD_A_RICOCH06,	//跳弾６//ricoch01 451
SD_A_RICOCH07,	//跳弾７//ricoch01 452
SD_A_RICOCH08,	//跳弾８//ricoch01 453
SD_E_BLOOD_H1,	//血しぶき（強）//blood_h1 454
SD_E_BLOOD_L1,	//血しぶき（弱）//blood_l1 455
SD_E_BLOOD_S1,	//血しぶき（ポタッ）//blood_s1 456
SD_E_SHIELD01,	//盾兵、盾破壊//shield01 457
SD_E_SHIRIC01,	//盾兵、盾に弾めり込む//shiric01 458
SD_E_SLIGHT01,	//盾兵、盾ライト破壊//slight01 459
SD_E_PHYFAR01,	//サイファー飛行音１//phyfar01 460
SD_E_PHYFAR02,	//サイファー飛行音２//phyfar01 461
SD_E_PHYFAR03,	//サイファー飛行音３//phyfar01 462
SD_E_PHYFAR04,	//サイファー飛行音４//phyfar01 463
SD_E_PHYFAR05,	//サイファー静止飛行音１//phyfar02 464
SD_E_PHYFAR06,	//サイファー静止飛行音２//phyfar02 465
SD_E_PHYFAR07,	//サイファー静止飛行音３//phyfar02 466
SD_E_PHYFAR08,	//サイファー静止飛行音４//phyfar02 467
SD_A_RAIN_SE1,	//ＳＥ雨風音//rain_se1 468
SD_A_CLOUD01,	//雷（遠１）//cloud01 469
SD_A_CLOUD02,	//雷（遠２）//cloud02 470
SD_P_DOBOON01,	//海に落ちる//doboon01 471
SD_A_THUNDR01,	//雷（近１）//thundr01 472
SD_A_THUNDR02,	//雷（近２）//thundr02 473
SD_A_WAVE01,	//波音（大）//wave01 474
SD_A_WAVE02,	//波音（小）//wave02 475
SD_A_LAMP02,	//電灯破壊//lamp01 476
SD_A_TOBCLO01,	//甲板フェンス閉める//tobclo01 477
SD_A_TOBOPN01,	//甲板フェンス開く//tobopn01 478
SD_A_CORPTH01,	//死体投げ捨て//corpth01 479
SD_V_POBORE01,	//スネーク着水アウト寸前声//pobore01 480
SD_V_ORGABR01,	//オルガ、寝息（吐）//orgabr01 481
SD_V_ORGABR02,	//オルガ、寝息（吸）//orgabr02 482
SD_V_SORGA01,	//オルガ、「隠れてるだけ！？」//sorga01 483
SD_V_SORGA02,	//オルガ、「たいした事無いわね！」//sorga02 484
SD_V_SORGA03,	//オルガ、「そこね！」//sorga03 485
SD_V_SORGA04,	//オルガ、「出て来なさい！」//sorga04 486
SD_V_SORGA05,	//オルガ、「その程度？」//sorga05 487
SD_V_SORGA06,	//オルガ、「丸見えよ！」//sorga06 488
SD_V_SORGA07,	//オルガ、「どこを狙ってるの？」//sorga07 489
SD_V_SORGA08,	//オルガ、「ハハハ…（嘲笑）」//sorga08 490
SD_V_SORGA09,	//オルガ、「やるわねぇ」//sorga09 491
SD_V_SORGA10,	//オルガ、「まだよ！」//sorga10 492
SD_V_SORGA19,	//オルガ、マスト切り気合「ティ！」//sorga19 493
SD_V_SORGA22,	//オルガ、「逃げてるつもり？」//sorga22 494
SD_V_SORGA23,	//オルガ、「バカにしてるの？」//sorga23 495
SD_V_ORGA03,	//オルガ、気合「ィヤッ！」//sorga25 496
SD_V_SORGA28,	//オルガ、息切れ「ッッハァハァ」//sorga28 497
SD_V_ORGA01,	//オルガ、舌打ち「チィッ！」//sorga30 498
SD_V_ORGA02,	//オルガ、見つかった！？「ハッ！」//sorga34 499
SD_V_ORGA04,	//オルガ、手榴弾投げる「食らえ！」//sorga36 500
SD_V_ORGA05,	//オルガ、ダメージ１「ウッ！」//sorga37 501
SD_V_ORGA06,	//オルガ、ダメージ２「アァッ！」//sorga38 502
SD_V_ORGA07,	//オルガ、気を失うやられ「アッハゥ」//sorga44 503
SD_A_CLOTH01,	//布めくれあがる//cloth01 504
SD_A_CLOTH02,	//布バタバタ１（1と2をランダムに）//cloth02 505
SD_A_CLOTH03,	//布バタバタ２（不定周期で連続呼び//cloth03 506
SD_A_LLIGHT01,	//投光器破壊//llight01 507
SD_A_LHIBI01,	//投光機ヒビ入る//lhibi01 508
SD_W_SOCOM02,	//オルガ拳銃発砲//socom02 509
SD_A_RICO_OM1,	//オルガ戦用金属跳弾（鉄板鉄柵）//rico_om1 510
SD_A_RICO_OM2,	//オルガ戦用鉄板跳弾（コンテナ）//rico_om2 511
SD_A_RICO_ON1,	//オルガ戦用通常跳弾（床、壁）//rico_on1 512
SD_A_RICO_OW1,	//オルガ戦用木製跳弾（木箱）//rico_ow1 513
SD_E_ORGABR01,	//オルガ、失神ダウン（鉄柵）//downsaku 514
SD_E_ORGABR02,	//オルガ、失神ダウン（雨床）//downyuka 515
SD_A_BOTTLE01,	//酒ビン破壊（銃器で）//bottle01 516
SD_A_BOTTLE02,	//酒ビン割れる（落下で）//bottle02 517
SD_A_GLASS01,	//ガラス割れる１//glass01 518
SD_A_GLASS02,	//ガラス割れる２//glass02 519
SD_A_GLASS03,	//ガラス割れる３//glass03 520
SD_A_GLSFALL1,	//ガラス破片落ち１//glsfall1 521
SD_A_GLSFALL2,	//ガラス破片落ち２（交互に）//glsfall2 522
SD_A_HIBI01,	//ガラスヒビ入り１（着弾時）//hibi01 523
SD_A_HIBI02,	//ガラスヒビ入り２（割れ前、8fps連）//hibi02 524
SD_A_LEAF01,	//観葉植物の跳弾//riclef01 525
SD_A_KOBORE01,	//酒こぼれる//kobore01 526
SD_A_WSDOOR01,	//カウンタードア軋み（低）//wsdoor01 527
SD_A_WSDOOR02,	//カウンタードア軋み（高）//wsdoor02 528
SD_A_OTAMA_CS,	//おたま跳弾４（ラ＃）//otama_cs 529
SD_A_OTAMA_DS,	//おたま跳弾５（ド）//otama_ds 530
SD_A_OTAMA_E,	//おたま跳弾６（レ）小さい方//otama_e 531
SD_A_FLYPAN_F,	//フライパン跳弾（大）//flypan_f 532
SD_A_FLYPAN_G,	//フライパン跳弾（中大）//flypan_g 533
SD_A_FLYPAN_A,	//フライパン跳弾（中）//flypan_a 534
SD_A_FLYPAN_B,	//フライパン跳弾（中小）//flypan_b 535
SD_A_FLYPAN_C,	//フライパン跳弾（小）//flypan_c 536
SD_A_RICSOF_C,	//ソファー、跳弾//ricsof01 537
SD_A_RICKOM01,	//小麦粉袋、跳弾//rickom01 538
SD_A_RICDAN01,	//ダンボール跳弾//ricdan01 539
SD_A_RICNAP01,	//紙ナプキン跳弾//ricnap01 540
SD_A_KOMFAL01,	//小麦粉こぼれる//komfal01 541
SD_A_RICVGT01,	//ジャガイモ破壊（跳弾？）//ricvgt01 542
SD_A_VGTFAL01,	//ジャガイモ落下（単発コールで）//vgtfal01 543
SD_A_DANBOR01,	//ダンボール壊れ（倒れ）//danbor01 544
SD_A_RICVGT02,	//カボチャ破壊（跳弾）//ricvgt02 545
SD_A_VGTFAL02,	//カボチャ落下１//vgtfal02 546
SD_A_VGTFAL03,	//カボチャ落下２//vgtfal03 547
SD_A_VGTFAL04,	//カボチャ落下３（初の落下のみ）//vgtfal04 548
SD_A_VGTFAL05,	//カボチャ落下４（４種ランダム）//vgtfal05 549
SD_A_W_RAIN01,	//窓の外の雨（持続SE）//w_rain01 550
SD_A_SUPON01,	//水密扉取っ手取れる//supon01 551
SD_V_SNADOR01,	//スネークうろたえ声//snador01 552
SD_A_SYOKAK01,	//消火器噴射１（着弾時）//syokak04 553
SD_A_SYOKAK02,	//消火器噴射２（ホースくねくね）//syokak03 554
SD_A_SYOKAK03,	//消火器噴射３（噴出中連続）//syokak05 555
SD_V_KISS_GO,	//主観ポスターへのキス開始//kiss_go 556
SD_V_KISS_FIN,	//主観ポスターへのキス終了//kiss_fin 557
SD_A_DRORAIN1,	//w00aへの水密扉開け雨風//drorain1 558
SD_A_DRCRAIN1,	//w00aからの水密扉閉め雨風//drcrain1 559
SD_A_CAMGLS01,	//監視カメラレンズ割れ//camgls01 560
SD_A_SARABRE1,	//皿着弾破壊//sarabre1 561
SD_A_SARAOTI1,	//皿破片落下１//saraoti1 562
SD_A_SARAOTI2,	//皿破片落下２//saraoti2 563
SD_A_BOOKFA01,	//雑誌落下１（開いている時）//bookfa01 564
SD_A_BOOKFA02,	//雑誌落下２（閉じている時）//bookfa02 565
SD_A_BOOKBR01,	//雑誌への着弾//bookbr01 566
SD_A_ICECAN01,	//氷缶への着弾１（氷無し）//icecan01 567
SD_A_ICECAN02,	//氷缶はずむ//icecan02 568
SD_A_ICECAN03,	//氷缶への着弾２（氷有り）//icecan03 569
SD_A_ICEDRP01,	//氷の落下１（ワンバウンド目）//icedrp01 570
SD_A_ICEDRP02,	//氷の落下２（４種ランダム）//icedrp02 571
SD_A_ICEDRP03,	//氷の落下３//icedrp03 572
SD_A_ICEDRP04,	//氷の落下４//icedrp04 573
SD_A_MONITA01,	//大型モニター破損（着弾時）//monita01 574
SD_A_CONSOL01,	//コンソールキータイプ音１//consol01 575
SD_A_CONSOL02,	//コンソールキータイプ音２//consol02 576
SD_A_CONSOL03,	//コンソールキータイプ音３//consol03 577
SD_A_CONSOL04,	//コンソールキータイプ音４//consol04 578
SD_A_ENGINESE,	//SEエンジン音(持続)//enginese 579
SD_A_BBHIT01,	//ＢＢ弾、跳弾//bbhit01 580
SD_A_BBSHOT01,	//フィギュア、鉄砲発射中//bbshot01 581
SD_A_BBSHOT00,	//フィギュア、鉄砲発射開始//bbshot00 582
SD_A_DOLLV01,	//フィギュア、叫び声//dollv01 583
SD_A_HIBANA01,	//懐中電灯、火花//hibana01 584
SD_A_PLIGHT01,	//懐中電灯、壊れる//plight01 585
SD_E_SHAKOUT1,	//ウォークマン壊れる//shakout1 586
SD_A_STEAM01,	//配管スチーム大１//steam21 587
SD_A_STEAM02,	//配管スチーム大２//steam21 588
SD_A_STEAM03,	//配管スチーム中１//steam26 589
SD_A_STEAM04,	//配管スチーム中２//steam26 590
SD_A_STEAM05,	//配管スチーム小１//steam24 591
SD_A_STEAM06,	//配管スチーム小２//steam24 592
SD_A_PIPEDW01,	//パイプ落ち１（１バウンド目）//pipedw01 593
SD_A_PIPEDW02,	//パイプ落ち２（２バウンド以降）//pipedw02 594
SD_A_PIPEDW03,	//パイプ落ち３（コロコロ）//pipedw03 595
SD_A_BULLET01,	//敵兵からの弾丸飛び風切り１//bullet01 596
SD_A_BULLET02,	//敵兵からの弾丸飛び風切り２//bullet02 597
SD_A_BULLET03,	//敵兵からの弾丸飛び風切り３//bullet03 598
SD_A_BULLET04,	//敵兵からの弾丸飛び風切り４//bullet04 599
SD_A_RICO_NOM,	//敵兵からの弾丸跳弾（通常）//rico_nom 600
SD_A_RICO_IRO,	//敵兵からの弾丸跳弾（金属）//rico_iro 601
SD_A_RICO_PIP,	//敵兵からの弾丸跳弾（パイプ）//rico_pip 602
SD_A_RICO_BOX,	//敵兵からの弾丸跳弾（木箱）//rico_box 603
SD_E_USAEQU01,	//海兵隊員銃構える１//usaequ01 604
SD_E_USAEQU02,	//海兵隊員銃構える２//usaequ02 605
SD_E_USAEQU03,	//海兵隊員銃構える３//usaequ03 606
SD_E_USAEQU04,	//海兵隊員銃構える４//usaequ04 607
SD_E_USATRN01,	//海兵隊員振り向く１//usatrn01 608
SD_E_USATRN02,	//海兵隊員振り向く２//usatrn02 609
SD_E_USATRN03,	//海兵隊員振り向く３//usatrn03 610
SD_E_USATRN04,	//海兵隊員振り向く４//usatrn04 611
SD_E_BIKKRIS21,	//海兵隊員びっくり単音（！）１//bikkris2 612
SD_E_BIKKRIS22,	//海兵隊員びっくり単音（！）２//bikkris2 613
SD_E_BIKKRIS23,	//海兵隊員びっくり単音（！）３//bikkris2 614
SD_E_BIKKRIS24,	//海兵隊員びっくり単音（！）４//bikkris2 615
SD_E_USKERE01,	//海兵隊員敬礼１//uskere01 616
SD_E_USKERE02,	//海兵隊員敬礼２//uskere02 617
SD_E_USKERE03,	//海兵隊員敬礼３//uskere03 618
SD_E_USKERE04,	//海兵隊員敬礼４//uskere04 619
SD_E_USNAOR01,	//海兵隊員直れ１//usnaor01 620
SD_E_USNAOR02,	//海兵隊員直れ２//usnaor02 621
SD_E_USNAOR03,	//海兵隊員直れ３//usnaor03 622
SD_E_USNAOR04,	//海兵隊員直れ４//usnaor04 623
SD_E_USKIWO01,	//海兵隊員気をつけ１//uskiwo01 624
SD_E_USKIWO02,	//海兵隊員気をつけ２//uskiwo02 625
SD_E_USKIWO03,	//海兵隊員気をつけ３//uskiwo03 626
SD_E_USKIWO04,	//海兵隊員気をつけ４//uskiwo04 627
SD_A_HATOPN01,	//床ハッチ開き始める//hatopn01 628
SD_A_HATOPN02,	//床ハッチ開いている//hatopn02 629
SD_A_HATOPN03,	//床ハッチ開ききる//hatopn03 630
SD_S_JINGLE01,	//緊迫ジングル音１//jingle01 631
SD_E_FACEDOWN,	//海兵隊員、顔から倒れる//facedown 632
SD_P_INTLUDE1,	//スネークダクト内ホフク左//intlude1 633
SD_P_INTLUDE2,	//スネークダクト内ホフク右//intlude2 634
SD_A_LIGHTER1,	//ライター落とし１バウンド目//lighter1 635
SD_A_LIGHTER2,	//ライター落とし２バウンド目以降//lighter2 636
SD_P_POLE_GO1,	//ポール降り、スタート//pole_go1 637
SD_P_POLE_MID,	//ポール降り、滑っている//pole_mid 638
SD_P_POLE_END,	//ポール降り、着地//pole_end 639
SD_A_PROJMOTO,	//プロジェクタ駆動音(18fps連)//projmoto 640
SD_A_PROJSWIT,	//プロジェクタリモコンスイッチ//projswit 641
SD_A_PROJCHUG,	//プロジェクタ点灯//projchng 642
SD_A_PROJOFF1,	//プロジェクタ消灯//projoff1 643
SD_A_CMCDMG01,	//司令官ダメージ１「うっ！」//cmcdmg01 644
SD_A_CMCDMG02,	//司令官ダメージ２「ぐむぅ！」//cmcdmg02 645
SD_A_CMCCAP03,	//司令官「つかまえろっ！！」//cmccap03 646
SD_V_SNAYES01,	//スネーク撮影ＯＫ１「うん」//snayes01 647
SD_V_SNAYES02,	//スネーク撮影ＯＫ２「よし」//snayes02 648
SD_V_SNABAD01,	//スネーク撮影ＮＧ１「うぅむ」//snabad01 649
SD_V_SNABAD02,	//スネーク撮影ＮＧ２「チッ」//snabad02 650
SD_V_SNABIM01,	//スネーク撮影微妙１「ふぅん？」//snabim01 651
SD_V_SNABIM02,	//スネーク撮影微妙２「うぅーん」//snabim02 652
SD_A_HELIMIXM,	//タイトルヘリ羽音//helimixm 653
SD_A_TENDER_M,	//タイトル雷//tender_m 654
SD_A_PAPER01,	//ページめくり音//paper01 655
SD_V_FORDMG01,	//フォーチュンダメージ「うぉっ」//fordmg01 656
SD_V_FORDMG02,	//フォーチュンダメージ「ぉあっ」//fordmg02 657
SD_V_FORDMG03,	//フォーチュンダメージ「えぇは」//fordmg03 658
SD_V_FORDMG04,	//フォーチュンダメージ「ぅえひ」//fordmg04 659
SD_V_FORTUN01,	//フォーチュン嘲笑「あはははっ」//fortun01 660
SD_V_FORTUN02,	//フォーチュン挑発「出てらっ～」//fortun02 661
SD_V_FORTUN03,	//フォーチュン弾逸れ「だめね」//fortun03 662
SD_V_FORTUN04,	//フォーチュン弾逸れ「はずれ」//fortun04 663
SD_V_FORTUN05,	//フォーチュン弾逸れ「あたら～」//fortun05 664
SD_V_FORTUN06,	//フォーチュン弾逸れ「お前に～」//fortun06 665
SD_V_FORTUN07,	//フォーチュン気合「ふんっ」//fortun07 666
SD_V_FORTUN08,	//フォーチュン気合「はぁっ」//fortun08 667
SD_V_FORTUN09,	//フォーチュン気合「でぇえぃ」//fortun09 668
SD_V_FORTUN10,	//フォーチュン気合「はっ」//fortun10 669
SD_V_FORTUN11,	//フォーチュン「早く私を殺して」//fortun11 670
SD_V_FORTUN12,	//フォーチュン「何してるの？～」//fortun12 671
SD_V_FORTUN13,	//フォーチュン「殺してみなさ～」//fortun13 672
SD_V_FORTUN14,	//フォーチュン「貴方に私の悲～」//fortun14 673
SD_V_FORTUN15,	//フォーチュン「当たってないわ」//fortun15 674
SD_V_FORTUN16,	//フォーチュン「当たらないわ」//fortun16 675
SD_E_LGUNFIRE,	//フォーチュン、ガン発射//lgunfire 676
SD_E_LGUNTAME,	//フォーチュン、ガンエネルギ充填//lguntame 677
SD_E_MAGAZI01,	//フォーチュン、マガジンはめる//magazi01 678
SD_E_KAMAE001,	//フォーチュン、ガン構える//kamae01 679
SD_E_TURN0001,	//フォーチュン、方向転換//turn01 680
SD_E_RGSPARK1,	//レールガンスパーク「パァン！」//rgspark1 681
SD_E_RGSPARK2,	//レールガン帯電ヴヴ..(8fps連)//rgspark2 682
SD_A_RICFOR01,	//フォーチュン弾丸無効跳弾//ricfor01 683
SD_A_EVCALL01,	//エレベータ到着チャイム//evcall01 684
SD_A_EVDCLO01,	//エレベータドア閉まり切る//evdclo01 685
SD_A_EVDMOV01,	//エレベータドア動く//evdmov01 686
SD_A_EVDOPN01,	//エレベータドア開き切る//evdopn01 687
SD_A_EVMOTOR1,	//エレベータ動く（14/60fps連続）//evmotor1 688
SD_A_EVSTART1,	//エレベータ動き始める//evstart1 689
SD_A_EVSTOP01,	//エレベータ止まる//evstop01 690
SD_A_EVCALL02,	//エレベータ赤ランプ点滅//evcall02 691
SD_A_EVCALL03,	//エレベータ緑ランプ点灯//evcall03 692
SD_A_FLAME001,	//フォークリフトフレーム落下//flame01 693
SD_A_FLAME002,	//フォークリフト倒れる//flame02 694
SD_A_LANPOTI1,	//電灯落ちる//lanpoti1 695
SD_A_LANPSWI1,	//電灯揺れる１//lanpswi1 696
SD_A_LANPSWI2,	//電灯揺れる２（左右で交互に）//lanpswi2 697
SD_A_LANPKIE1,	//蛍光灯消える//LANPKIE1 698
SD_A_DRUMCAN1,	//ドラム缶倒れる//drumcan1 699
SD_A_METLANA1,	//鉄コンテナに穴が空く//metlana1 700
SD_A_METLBRE1,	//鉄コンテナ破壊//metlbre1 701
SD_A_METLOTI1,	//鉄破片落ち１//metloti1 702
SD_A_METLOTI2,	//鉄破片落ち２//metloti2 703
SD_A_WOODBRE1,	//木箱破壊//woodbre1 704
SD_A_WOODOTI1,	//木破片落ち１//woodoti1 705
SD_A_WOODOTI2,	//木破片落ち２//woodoti2 706
SD_A_OILFIRE1,	//燃料燃える(15fps連)//oilfire1 707
SD_A_TENOTI1,	//天井落ちる１（ジャリ）//tenoti1 708
SD_A_TENOTI2,	//天井落ちる２（ガラ）//tenoti2 709
SD_A_TENPARA1,	//天井壊れ破片１//tenpara1 710
SD_A_TENPARA2,	//天井壊れ破片２//tenpara2 711
SD_V_SNADMG01,	//NPCスネークダメージ１「うぁっ」//snadmg01 712
SD_V_SNADMG02,	//NPCスネークダメージ２「うっ」//snadmg02 713
SD_V_SNAOUT01,	//NPCスネークアウト「うあーっ…」//snaout01 714
SD_V_SNANEC01,	//NPCスネーク首絞まる「いっ」//snanec01 715
SD_V_SNANED01,	//NPCスネーク首折れアウト「んっ」//snaned01 716
SD_V_SNASLP01,	//NPCスネーク寝息（吸う）「ず～」//snaslp01 717
SD_A_SWORDOPJ,	//刀構える//swordopj 718
SD_A_SWORDCLJ,	//刀収める//swordclj 719
SD_A_SWORDCNJ,	//刀みね打ち切り替え//swordcnj 720
SD_A_SWORDBIJ,	//刀刺しヒット//swordbij 721
SD_A_SWORDCUJ,	//刀振りヒット//swordcuj 722
SD_A_SWORDHIJ,	//刀みね打ちヒット//swordhij 723
SD_A_SWINGBIJ,	//刀刺し風切り//swingbij 724
SD_A_SWINGCUJ,	//刀振り風切り//swingcuj 725
SD_A_SWINGROJ,	//刀回転振り風切り//swingroj 726
SD_A_SWORDRIJ,	//刀銃弾はじき//swordrij 727
SD_A_SWORDHAJ,	//刀と刀の衝突//swordhaj 728
SD_A_SWINGBAJ,	//刀振り返し//swingbaj 729
SD_A_SWORDSEJ,	//刀かまえる//swordsej 730
SD_E_TNGHITPU,	//天狗パンチヒット//tnghitpu 731
SD_E_TNGHITKI,	//天狗キックヒット//tnghitki 732
SD_E_TNGPUNCH,	//天狗パンチ振り風切り//tngpunch 733
SD_E_TNGKICK1,	//天狗キック振り風切り//tngkick 734
SD_E_TNGSLIDE,	//天狗スライディング//tngslide 735
SD_E_TNGJUMP1,	//天狗ジャンプ//tngjump1 736
SD_E_TNGLAND1,	//天狗着地//tngland1 737
SD_E_TNGPFIRE,	//天狗Ｐ９０発砲//tngpfire 738
SD_E_TNGPRELD,	//天狗Ｐ９０リロード//tngpreld 739
SD_V_EMAAFR01,	//エマ脅え１「ひっ！」//emaafr02 740
SD_V_EMAAFR02,	//エマ脅え２「あぁ…ぁぁ…」//emaafr03 741
SD_V_EMABTH01,	//エマ小息継ぎ「はーひー」//emabth01 742
SD_V_EMABTH02,	//エマ大息継ぎ「ぶっはぁひぃ」//emabth03 743
SD_V_EMADMG01,	//エマダメージ１「ヤッ！」//emadmg11 744
SD_V_EMADMG02,	//エマダメージ２「アッ！」//emadmg12 745
SD_V_EMADMW01,	//エマ水中ダメージ１「ブッ！」//emadmw01 746
SD_V_EMADMW02,	//エマ水中ダメージ２「プァッ！」//emadmw02 747
SD_V_EMADWN01,	//エマ転ぶ１「あはっ！」//emadwn01 748
SD_V_EMADWN02,	//エマ転ぶ２「えぁっ！」//emadwn02 749
SD_V_EMAEXP01,	//エマ狙われる予感１「あっ！？」//emaexp01 750
SD_V_EMAEXP02,	//エマ狙われる予感２「はっ！？」//emaexp02 751
SD_V_EMAGIV01,	//エマ気絶うめき１「ううーっ」//emagiv01 752
SD_V_EMAGIV02,	//エマ気絶うめき２「うぅふぅ」//emagiv02 753
SD_V_EMAOUT01,	//エマアウト「キャーッ！」//emaout11 754
SD_V_EMAOUW01,	//エマ水中アウト「アァァァ」//emaouw01 755
SD_V_EMASCR01,	//エマ悲鳴１「キャッ！」//emascr01 756
SD_V_EMASCR02,	//エマ悲鳴２「イヤー！」//emascr02 757
SD_V_EMAHNG01,	//エマ首絞められ「んあっ」//emahng01 758
SD_V_EMAOUH02,	//エマ首絞めアウト//emaouh01 759
SD_V_EMASLP01,	//エマ寝息１吐く「んふ～」//emaslp01 760
SD_V_EMASLP02,	//エマ寝息２吸う「す～」//emaslp02 761
SD_A_EMFOOTL1,	//エマ走り足音（左）//emfootl1 762
SD_A_EMFOOTR1,	//エマ走り足音（右）//emfootr1 763
SD_A_EMFOOTL2,	//エマ歩き足音（左）//emfootl2 764
SD_A_EMFOOTR2,	//エマ歩き足音（右）//emfootr2 765
SD_A_EMSITDW1,	//エマしゃがむ//emsitdw1 766
SD_A_EMSTAND1,	//エマ立つ//emstand1 767
SD_A_MEGAEQU1,	//エマ眼鏡かける//megaequ1 768
SD_A_MEGAOFF1,	//エマ眼鏡外す//megaoff1 769
SD_A_BGWATR01,	//水中環境音（持続）//bgwatr01 770
SD_A_INWDOOR1,	//水中水密ドア開く//inwdoor1 771
SD_P_INWMOVE1,	//プレイヤー水中動作１//inwmove1 772
SD_P_INWMOVE2,	//プレイヤー水中動作２//inwmove2 773
SD_P_INWMOVE3,	//プレイヤー水中動作３//inwmove3 774
SD_P_INWMOVE4,	//プレイヤー水中動作４//inwmove4 775
SD_P_INWSWIM1,	//プレイヤー水中泳ぎ１//inwswim1 776
SD_P_INWSWIM2,	//プレイヤー水中泳ぎ２//inwswim2 777
SD_P_INWTERL1,	//プレイヤー水面へ飛び込む//inwterl1 778
SD_P_INWTERM1,	//プレイヤー水面から水中へ潜る//inwterm1 779
SD_P_IWFOOTL1,	//プレイヤー水入り際足音（左）//iwfootl1 780
SD_P_IWFOOTR1,	//プレイヤー水入り際足音（右）//iwfootr1 781
SD_P_OUTSWIM1,	//プレイヤー水面泳ぎ//outswim1 782
SD_P_OUTWTRS1,	//プレイヤー水面に上がる//outwtrs1 783
SD_V_RAIBRE01,	//プレイヤー危険呼吸「ブァハッ」//raibre01 784
SD_V_RAIBRE02,	//プレイヤー通常呼吸「ハァスー」//raibre02 785
SD_V_RAIDMW01,	//プレイヤー水中ダメージ「ブァ」//raidmw01 786
SD_V_RAIOUW01,	//プレイヤー水中アウト「ブオァ」//raiouw01 787
SD_A_FNACAM01,	//船虫カメラにベチッ//fnacam01 788
SD_A_FNAFOOT1,	//船虫踏み潰す１//fnafoot1 789
SD_A_FNAFOOT2,	//船虫踏み潰す２(２種ランダム)//fnafoot2 790
SD_A_FNALOP01,	//船虫わさわさ遅１//fnaloop0 791
SD_A_FNALOP02,	//船虫わさわさ遅２(交互18fps連)//fnaloop0 792
SD_A_FNALOP11,	//船虫わさわさ中１//fnaloop1 793
SD_A_FNALOP12,	//船虫わさわさ中２(交互18fps連)//fnaloop1 794
SD_A_FNALOP21,	//船虫わさわさ速１//fnaloop2 795
SD_A_FNALOP22,	//船虫わさわさ速２(交互18fps連)//fnaloop2 796
SD_A_FNABOU01,	//船虫弾む１//fnabou01 797
SD_A_FNABOU02,	//船虫弾む２//fnabou01 798
SD_A_FNABOU03,	//船虫弾む３//fnabou02 799
SD_A_FNABOU04,	//船虫弾む４(４種交互に使う)//fnabou02 800
SD_A_WIND_SE1,	//連絡橋ＳＥ風波音//wind_se1 801
SD_A_KMOFLY01,	//海鳥羽ばたき１//kmofly01 802
SD_A_KMOFLY02,	//海鳥羽ばたき２//kmofly02 803
SD_A_KMOFLY03,	//海鳥羽ばたき３//kmofly03 804
SD_A_KMOFLY04,	//海鳥羽ばたき４//kmofly04 805
SD_A_KMOFLY11,	//海鳥羽ばたき５//kmofly11 806
SD_A_KMOFLY12,	//海鳥羽ばたき６//kmofly12 807
SD_A_KMOFLY13,	//海鳥羽ばたき７//kmofly13 808
SD_A_KMOFLY14,	//海鳥羽ばたき８//kmofly14 809
SD_A_KMOSOR01,	//海鳥飛び立ち羽ばたき１//kmosor01 810
SD_A_KMOSOR02,	//海鳥飛び立ち羽ばたき２//kmosor02 811
SD_A_KMOATK01,	//海鳥くちばし攻撃ヒット//kmoatk01 812
SD_A_FUNHIT01,	//海鳥フン、プレイヤーに当たる//funhit01 813
SD_A_FUNOTI01,	//海鳥フン、床に落ちる１//funoti01 814
SD_A_FUNOTI02,	//海鳥フン、床に落ちる２//funoti02 815
SD_A_KMOVOX01,	//海鳥鳴き声１//kmovox01 816
SD_A_KMOVOX02,	//海鳥鳴き声２//kmovox02 817
SD_A_KMOVOX03,	//海鳥鳴き声３//kmovox03 818
SD_A_KMOVOX04,	//海鳥鳴き声４//kmovox04 819
SD_A_KMODMG01,	//海鳥ダメージ鳴き声１//kmodmg01 820
SD_A_KMODMG02,	//海鳥ダメージ鳴き声２//kmodmg02 821
SD_A_KMOOUT01,	//海鳥アウト鳴き声１//kmoout01 822
SD_A_KMOOUT02,	//海鳥アウト鳴き声２//kmoout02 823
SD_A_KMODWN01,	//カモメダウン//kmodwn01 824
SD_A_KMOFOOTL,	//カモメ足音（左）//kmofootl 825
SD_A_KMOFOOTR,	//カモメ足音（右）//kmofootr 826
SD_A_KMOTUTUK,	//カモメ床ついばみ//kmotutuk 827
SD_A_SYONBE01,	//敵兵小便かかる１//syonbe01 828
SD_A_SYONBE02,	//敵兵小便かかる２//syonbe02 829
SD_A_SYONBE03,	//敵兵小便かかる３//syonbe03 830
SD_A_SYONMT01,	//敵兵小便床アタリ１//syonmt01 831
SD_A_SYONMT02,	//敵兵小便床アタリ２//syonmt02 832
SD_A_SYONMT03,	//敵兵小便床アタリ３//syonmt03 833
SD_A_SYONDM01,	//敵兵小便ダンボール１//syondm01 834
SD_A_SYONDM02,	//敵兵小便ダンボール２//syondm02 835
SD_A_SYONDM03,	//敵兵小便ダンボール３//syondm03 836
SD_A_FLOORBR1,	//床壊れる//floorbr1 837
SD_A_FOOTW01L,	//プレイヤー通常歩き（左）//footckl2 838
SD_A_FOOTW01R,	//プレイヤー通常歩き（右）//footckr2 839
SD_A_FOOTW02L,	//プレイヤー鉄板歩き（左）//footmll2 840
SD_A_FOOTW02R,	//プレイヤー鉄板歩き（右）//footmlr2 841
SD_A_FOOTW03L,	//プレイヤー軋み床歩き（左）//footmkl2 842
SD_A_FOOTW03R,	//プレイヤー軋み床歩き（右）//footmkr2 843
SD_S_EYECHK01,	//網膜チェックスタート//eyechk01 844
SD_S_EYESCN01,	//網膜チェック中(8fps連)//eyescn01 845
SD_S_EYENOT01,	//網膜チェック失敗//eyenot01 846
SD_S_EYEOK01,	//網膜チェック成功//eyeok01 847
SD_A_PMHEART1,	//ペースメーカー付心音//pmheart1 848
SD_A_PMHIGH01,	//ペースメーカー自体(12fps連)//pmhigh01 849
SD_A_HOSTMZC1,	//人質もぞもぞ衣擦れ１//hostmzc1 850
SD_A_HOSTMZC2,	//人質もぞもぞ衣擦れ２//hostmzc2 851
SD_A_HOSTMZR1,	//人質もぞもぞロープ軋み１//hostmzr1 852
SD_A_HOSTMZR2,	//人質もぞもぞロープ軋み２//hostmzr2 853
SD_A_TABLEGG1,	//人質もぞもぞ机ゴトゴト１//tablegg1 854
SD_A_TABLEGG2,	//人質もぞもぞ机ゴトゴト２//tablegg2 855
SD_A_HOSTFDMG,	//人質女１ダメージ//hostfdmg 856
SD_A_HOSTFOUT,	//人質女１アウト//hostfout 857
SD_A_HOSTFDM2,	//人質女２ダメージ//hostfdm2 858
SD_A_HOSTFOU2,	//人質女２アウト//hostfou2 859
SD_A_HOSTMDMG,	//人質男１ダメージ//hostmdmg 860
SD_A_HOSTMOUT,	//人質男１アウト//hostmout 861
SD_A_HOSTMDM2,	//人質男２ダメージ//hostmdm2 862
SD_A_HOSTMOU2,	//人質男２アウト//hostmou2 863
SD_A_HOSTFAFR,	//人質女１もがく声//hostfafr 864
SD_A_HOSTFSLP,	//人質女１寝息//hostfslp 865
SD_A_HOSTFAF2,	//人質女２もがく声//hostfaf2 866
SD_A_HOSTFSL2,	//人質女２寝息//hostfsl2 867
SD_A_HOSTMAFR,	//人質男１もがく声//hostmafr 868
SD_A_HOSTMSLP,	//人質男１いびき//hostmslp 869
SD_A_HOSTMAF2,	//人質男２もがく声//hostmaf2 870
SD_A_HOSTMSL2,	//人質男２いびき//hostmsl2 871
SD_A_AHEART01,	//人質心臓音１//aheart01 872
SD_A_AHEART02,	//人質心臓音２//aheart02 873
SD_A_AHEART03,	//人質心臓音３//aheart03 874
SD_A_AHEART04,	//人質心臓音４//aheart04 875
SD_A_RICDMG01,	//エイムズダメージ//ricdmg01 876
SD_A_RICOUT01,	//エイムズアウト//ricout01 877
SD_V_GBSIBKS1,	//人質イベント用スーパーいびき//gbsibks1 878
SD_A_KCEJ_TEL,	//心音まぎらわシリーズ１ダイヤル//kcej_tel 879
SD_A_KCEJ_BSY,	//心音まぎらわシリーズ２話し中//kcej_bsy 880
SD_A_SHINDEN1,	//心音まぎらわシリーズ３心電//shinden1 881
SD_A_SHINDEN2,	//心音まぎらわシリーズ４心電氏//shinden2 882
SD_A_MAILTYPE,	//心音まぎらわシリーズ５メール打//mailtype 883
SD_V_FATDMG01,	//ファットマンダメージ「うっ」//fatdmg01 884
SD_V_FATDMG02,	//ファットマンダメージ「ぐはっ」//fatdmg02 885
SD_V_FATDMG03,	//ファットマンダメージ「うぐっ」//fatdmg03 886
SD_V_FATDMG04,	//ファットマンダメージ「痛て」//fatdmg04 887
SD_V_FATSTN01,	//ファットマンスタン「まぶしい」//fatstn01 888
SD_V_FATSTN02,	//ファットマンスタン「見えない」//fatstn02 889
SD_V_FATHAA01,	//ファットマン息切れ「ハァハァ」//fathaa01 890
SD_V_FATOUT01,	//ファットマンアウト「ぬおーっ」//fatout01 891
SD_V_FATDMB01,	//ファットマン愚痴「邪魔をすな」//fatdmb01 892
SD_V_FATDMB02,	//ファットマン愚痴「何をする！」//fatdmb02 893
SD_V_FATWAH01,	//ファットマン挑発「ぬはは」//fatwah01 894
SD_V_FATWAH02,	//ファットマン挑発「ぬふふ」//fatwah02 895
SD_V_FATKIA01,	//ファットマン気合「ふん！」//fatkia01 896
SD_V_FATKIA02,	//ファットマン気合「うりゃっ！」//fatkia02 897
SD_V_FATKIA03,	//ファットマン気合「くらえ！」//fatkia03 898
SD_V_FATKIA04,	//ファットマン気合「死ね！」//fatkia04 899
SD_V_FATASE01,	//ファットマン焦り「畜生！」//fatase01 900
SD_V_FATASE02,	//ファットマン焦り「やるな！」//fatase02 901
SD_V_FATDOKN5,	//ファットマン秒読み「５」//fatdokn5 902
SD_V_FATDOKN4,	//ファットマン秒読み「４」//fatdokn4 903
SD_V_FATDOKN3,	//ファットマン秒読み「３」//fatdokn3 904
SD_V_FATDOKN2,	//ファットマン秒読み「２」//fatdokn2 905
SD_V_FATDOKN1,	//ファットマン秒読み「１」//fatdokn1 906
SD_V_FATDOKN0,	//ファットマン秒読み「ドカン！」//fatdokn0 907
SD_V_FATWHE01,	//ファットマン見失い「どこだ？」//fatwhe01 908
SD_V_FATWHE02,	//ファットマン見失い「出てこい」//fatwhe02 909
SD_V_FATNOD01,	//ファットマン無効「無駄だ！」//fatnod01 910
SD_V_FATNOD02,	//ファットマン無効「きかんな！」//fatnod02 911
SD_V_FATOTT01,	//ファットマンよろけ「おっとと」//fatott01 912
SD_V_FATCUT01,	//ファットマン踏付け「落ちろ！」//fatcut01 913
SD_V_FATCUT02,	//ファットマン踏付け「ばかめ！」//fatcut02 914
SD_E_C4OUT08,	//Ｃ４爆発（ゲームオーバー）//c4out08 915
SD_E_C4TIME11,	//時限Ｃ４タイマー音１－１//c4time01 916
SD_E_C4TIME12,	//時限Ｃ４タイマー音１－２//c4time02 917
SD_E_C4TIME13,	//時限Ｃ４タイマー音１－３//c4time03 918
SD_E_C4TIME14,	//時限Ｃ４タイマー音１－４//c4time04 919
SD_E_C4TIME21,	//時限Ｃ４タイマー音２－１//c4time01 920
SD_E_C4TIME22,	//時限Ｃ４タイマー音２－２//c4time02 921
SD_E_C4TIME23,	//時限Ｃ４タイマー音２－３//c4time03 922
SD_E_C4TIME24,	//時限Ｃ４タイマー音２－４//c4time04 923
SD_E_C4TIME31,	//時限Ｃ４タイマー音３－１//c4time01 924
SD_E_C4TIME32,	//時限Ｃ４タイマー音３－２//c4time02 925
SD_E_C4TIME33,	//時限Ｃ４タイマー音３－３//c4time03 926
SD_E_C4TIME34,	//時限Ｃ４タイマー音３－４//c4time04 927
SD_E_ELATACK3,	//ブレード指斬り//elatack3 928
SD_A_FAHIKI03,	//ファットマン死体引きずり//fahiki03 929
SD_E_GLOCKF00,	//グロック発砲音//glockf00 930
SD_E_GLOCKH03,	//グロックマガジン外し//glockh03 931
SD_E_GLOCKR04,	//グロックリロード//glockr04 932
SD_E_SUITREV4,	//ボムブラストスーツ跳弾//suitrev4 933
SD_E_ROLLFTL0,	//ローラーブレード通常滑走（左）//rollftl0 934
SD_E_ROLLFTR0,	//ローラーブレード通常滑走（右）//rollftr0 935
SD_E_ROLLST00,	//ローラーブレード通常停止//rollst00 936
SD_E_LOOPCK02,	//ローラー通常持続(8fps連)//loopck02 937
SD_E_ROLLFEL2,	//ローラーブレード鉄床滑走（左）//rollfel2 938
SD_E_ROLLFER2,	//ローラーブレード鉄床滑走（右）//rollfer2 939
SD_E_ROLLSTF0,	//ローラーブレード鉄床停止//rollstf0 940
SD_E_LOOPMT01,	//ローラー鉄床持続(10fps連)//loopmt01 941
SD_E_ROLLFEL3,	//ローラーブレード外鉄滑走（左）//rollfel3 942
SD_E_ROLLFER3,	//ローラーブレード外鉄滑走（右）//rollfer3 943
SD_E_ROLLSTF1,	//ローラーブレード外鉄停止//rollstf1 944
SD_E_LOOPMT02,	//ローラー外鉄持続(10fps連)//loopmt02 945
SD_E_ROLLTUN0,	//ローラーブレードターン//rolltun0 946
SD_E_C4PUT11,	//ファットマンＣ４セット//c4put11 947
SD_E_C4SW11,	//ファットマンＣ４スイッチ//c4sw11 948
SD_V_OTADMG01,	//オタコンダメージ１「あっあ！」//otadmg01 949
SD_V_OTADMG02,	//オタコンダメージ２「ぅあー！」//otadmg02 950
SD_V_OTADMG03,	//オタコンダメージ３「ぬあぁ！」//otadmg03 951
SD_V_OTADMG04,	//オタコンダメージ４「うわっ！」//otadmg04 952
SD_V_OTAOUT11,	//オタコンアウト「うわあぁっ！」//otaout11 953
SD_V_SNADMG11,	//NPCスネークダメージ１「うっ！」//snadmg11 954
SD_V_SNADMG12,	//NPCスネークダメージ２「ううっ」//snadmg12 955
SD_V_SNADMG13,	//NPCスネークダメージ３「おうっ」//snadmg13 956
SD_V_SNADMG14,	//NPCスネークダメージ４「うおっ」//snadmg14 957
SD_V_SNAOUT11,	//NPCスネークアウト「うわあぁっ」//snaout11 958
SD_V_SOLAT101,	//ハリアー汎用攻撃１「食らえ！」//solat101 959
SD_V_SOLAT102,	//ハリアー汎用攻撃２「何処だ？」//solat102 960
SD_V_SOLAT103,	//ハリアー汎用攻撃３「そこか！」//solat103 961
SD_V_SOLAT201,	//ハリアー汎用気合１「でえっ！」//solat201 962
SD_V_SOLAT202,	//ハリアー汎用気合２「でぇや！」//solat202 963
SD_V_SOLAT301,	//ハリアー機銃「蜂の巣にして～」//solat301 964
SD_V_SOLAT401,	//ハリアークラスタ爆「くたばれ」//solat401 965
SD_V_SOLAT501,	//ハリアーロケット弾「よけら～」//solat501 966
SD_V_SOLAT601,	//ハリアーミサイル「あの世に～」//solat601 967
SD_V_SOLAT701,	//ハリアー排気炎「焼け死ねぇ！」//solat701 968
SD_V_SOLDM101,	//ハリアーヘリダメージ１「ぬぅ」//soldm101 969
SD_V_SOLDM102,	//ハリアーヘリダメージ２「糞っ」//soldm102 970
SD_V_SOLDM103,	//ハリアーヘリダメージ３「ちっ」//soldm103 971
SD_V_SOLDM201,	//ハリアー小ダメジ１「どうした」//soldm201 972
SD_V_SOLDM202,	//ハリアー小ダメジ２「効かんな」//soldm202 973
SD_V_SOLDM203,	//ハリアー小ダメジ３「くっ！」//soldm203 974
SD_V_SOLDM301,	//ハリアー中ダメジ１「うおっ！」//soldm301 975
SD_V_SOLDM302,	//ハリアー中ダメジ２「うぅっ！」//soldm302 976
SD_V_SOLDM401,	//ハリアー大ダメジ１「なにぃ！」//soldm401 977
SD_V_SOLDM402,	//ハリアー大ダメジ２「馬鹿な！」//soldm402 978
SD_V_SOLDM403,	//ハリアー大ダメジ３「ぐわぁぁ」//soldm403 979
SD_V_SOLOUT01,	//ハリアーアウト「ぐぁぁぁぁ！」//solout01 980
SD_E_CLMOPN03,	//クラスター爆弾投下開始//clmopn03 981
SD_E_CLMFAL04,	//クラスター爆弾降下//clmfal04 982
SD_E_CLMPAN04,	//クラスター爆弾分裂//clmpan04 983
SD_E_CLMFLY04,	//クラスター爆弾分裂後飛散//clmfly04 984
SD_E_CLMBOM04,	//クラスター爆弾爆発１//clmbom04 985
SD_E_CLMBOM05,	//クラスター爆弾爆発２//clmbom05 986
SD_E_GUNST01,	//ガンポッド機銃開始//gunst01 987
SD_E_GUNLP02,	//ガンポッド機銃(13fps連)//gunlp02 988
SD_E_GUN_WTR1,	//ガンポッド海着水//ricwtr03 989
SD_E_MISOPN01,	//アムラーム発射開始//misopn01 990
SD_E_MISFIRL1,	//アムラーム誘導ミサイル噴射//misfirl1 991
SD_E_MISFIRL5,	//アムラーム誘導ミサイル射出//misfirl5 992
SD_E_AM_FLY01,	//アムラーム飛行音(12fps連)//am_fly01 993
SD_E_AM_SWIN1,	//アムラーム交錯(発射120fps後)//am_swin1 994
SD_E_AM_EXP04,	//アムラーム爆発//am_exp04 995
SD_E_MISFIRS2,	//ロケット弾発射//misfirs2 996
SD_E_M38BOM07,	//ロケット弾爆発1（ランダム呼び）//m38bom07 997
SD_E_M38BOM08,	//ロケット弾爆発2（ランダム呼び）//m38bom08 998
SD_E_NEPPOST1,	//ハリアー熱風開始//neppost1 999
SD_E_NEPPOLP1,	//ハリアー熱風(始25fps後13fps連)//neppolp1 1000
SD_E_H_FLARE1,	//ハリアーフレア発射//h_flare1 1001
SD_E_H_ATACK1,	//ハリアー翼体当たり//h_atack1 1002
SD_E_H_DMGB03,	//ハリアー被弾//h_dmgb03 1003
SD_E_H_SWING1,	//ハリアー交錯（中央）//h_swing1 1004
SD_E_KA_GUN01,	//カサッカ機銃//ka_gun01 1005
SD_E_JETTRB01,	//ハリアージェット(6fps連)//jettrb01 1006
SD_E_HELHOV00,	//ホバリング風切り(12fps連)//helhov00 1007
SD_E_HELLOP00,	//カサッカ羽音(12fps連)//hellop00 1008
SD_V_SOLATK01,	//ソリダス攻撃気合１「は！」//solatk01 1009
SD_V_SOLATK02,	//ソリダス攻撃気合２「ふん！」//solatk02 1010
SD_V_SOLATK03,	//ソリダス攻撃気合３「ふっ！」//solatk03 1011
SD_V_SOLATK04,	//ソリダス攻撃気合４「はっ！」//solatk04 1012
SD_V_SOLATK05,	//ソリダス攻撃気合５「はあっ！」//solatk05 1013
SD_V_SOLATK06,	//ソリダス攻撃気合６「おりぁ！」//solatk06 1014
SD_V_SOLATK07,	//ソリダス攻撃気合７「てーっ！」//solatk07 1015
SD_V_SOLATK08,	//ソリダス攻撃気合８「いやっ！」//solatk08 1016
SD_V_SOLCHO01,	//ソリダス挑発１「ははははは！」//solcho01 1017
SD_V_SOLCHO02,	//ソリダス挑発２「どうした」//solcho02 1018
SD_V_SOLCHO03,	//ソリダス挑発３「そんなものか」//solcho03 1019
SD_V_SOLCHO04,	//ソリダス挑発４「そこまでか～」//solcho04 1020
SD_V_SOLDMG11,	//ソリダスダメージ１「ぶぅぁ！」//soldmg11 1021
SD_V_SOLDMG12,	//ソリダスダメージ２「うぅぁ！」//soldmg12 1022
SD_V_SOLDMG13,	//ソリダスダメージ３「ふぐっ！」//soldmg13 1023
SD_V_SOLDMG14,	//ソリダスダメージ４「おふっ！」//soldmg14 1024
SD_V_SOLELU01,	//ソリダス対エルド１「何をし～」//solelu01 1025
SD_V_SOLELU02,	//ソリダス対エルド２「ふざけ～」//solelu02 1026
SD_V_SOLFRA01,	//ソリダスよろけ１「おっ」//solfra01 1027
SD_V_SOLFRA02,	//ソリダスよろけ２「うぅっ！」//solfra02 1028
SD_V_SOLHAA01,	//ソリダスライフ小「はぁはぁ」//solhaa01 1029
SD_V_SOLKIA11,	//ソリダス攻撃開始１「いくぞ！」//solkia11 1030
SD_V_SOLKIA12,	//ソリダス攻撃開始２「くらえ！」//solkia12 1031
SD_V_SOLKIA13,	//ソリダス攻撃開始３「しねぃ！」//solkia13 1032
SD_V_SOLKIA14,	//ソリダス攻撃開始４「へぇっ！」//solkia14 1033
SD_V_SOLKIA15,	//ソリダス攻撃開始５「はぁっ！」//solkia15 1034
SD_V_SOLMUS01,	//ソリダスマッスル１「てやー！」//solmus01 1035
SD_V_SOLMUS02,	//ソリダスマッスル２「ふんー！」//solmus02 1036
SD_V_SOLOUT11,	//ソリダスアウト「うおぉぉぉ！」//solout11 1037
SD_V_SOLSPE01,	//ソリダス加速装置１「こっちだ」//solspe01 1038
SD_V_SOLSPE02,	//ソリダス加速装置２「むんっ！」//solspe02 1039
SD_V_SOLSPE03,	//ソリダス加速装置３「てぇっ！」//solspe03 1040
SD_V_SOLSTE01,	//ソリダス蛇手捨て「さすがだ～」//solste01 1041
SD_V_SOLWHY01,	//ソリダス攻かわされ１「なに？」//solwhy01 1042
SD_V_SOLWHY02,	//ソリダス攻かわされ２「んっ！」//solwhy02 1043
SD_V_SOLWHY03,	//ソリダス攻かわされ３「ぬぅ！」//solwhy03 1044
SD_E_GURDHEB1,	//ソリダス蛇手ガード//gurdheb1 1045
SD_E_GURDKEN1,	//ソリダス剣ガード//gurdken1 1046
SD_E_HEBATT01,	//蛇手突き//hebatt01 1047
SD_E_HEBFURI1,	//蛇手振り１//hebfuri1 1048
SD_E_HEBFURI2,	//蛇手振り２//hebfuri2 1049
SD_E_HEBHIT01,	//蛇手打撃ヒット//hebhit01 1050
SD_E_HEBOUT01,	//蛇手肩からはずす//hebout01 1051
SD_E_HEBREDY1,	//蛇手攻撃準備//hebredy1 1052
SD_E_HEBSIME1,	//蛇手絞め中（振る毎に１コール）//hebsime1 1053
SD_E_HEBTKAM1,	//蛇手つかみ//hebtkam1 1054
SD_E_KASOKU01,	//加速装置スタート//kasoku01 1055
SD_E_KASOKU02,	//加速装置フォロースルー//kasoku02 1056
SD_E_MUSLBRE1,	//脊髄破壊アウト//muslbre1 1057
SD_E_MUSLON01,	//マッスル装置ため//muslon01 1058
SD_E_TELBSWI1,	//ソリダスエルボー振り//telbswi1 1059
SD_E_TKENBIT1,	//ソリダス剣刺し//tkenbit1 1060
SD_E_TKENCUT1,	//ソリダス剣斬り//tkencut1 1061
SD_E_TKICSWI1,	//ソリダス蹴り振り//tkicswi1 1062
SD_A_TMISBOM1,	//ミサイル着弾爆発//tmisbom1 1063
SD_E_TMISSTA1,	//ミサイル発射//tmissta1 1064
SD_E_TSWING01,	//ソリダス剣振り（右）//tswing01 1065
SD_E_TSWING02,	//ソリダス剣振り（左）//tswing02 1066
SD_E_TSWING11,	//ソリダス剣振り（もろ手）//tswing11 1067
SD_V_PREDMG01,	//大統領ダメージ１「うぁーっ！」//predmg01 1068
SD_V_PREDMG02,	//大統領ダメージ２「ぬぅうっ！」//predmg02 1069
SD_V_PREDMG03,	//大統領ダメージ３「うわっ！」//predmg03 1070
SD_V_PREOUT01,	//大統領アウト「ぬぉぉぉぉ！！」//preout01 1071
SD_V_PRESUP01,	//大統領びっくり１「ほっ！？」//presup01 1072
SD_V_PRESUP02,	//大統領びっくり２「おぉぉ」//presup02 1073
SD_V_PRESUP03,	//大統領びっくり３「ふ！？」//presup03 1074
SD_V_PRESUP04,	//大統領びっくり４「んむぅ」//presup04 1075
SD_V_PRESUP05,	//大統領びっくり５「ほぉっ！？」//presup05 1076
SD_V_RAISIB01,	//ライデン影縛られ１「動けない」//raisib01 1077
SD_V_RAISIB02,	//ライデン影縛られ２「体が！？」//raisib02 1078
SD_V_VMPAKT01,	//ヴァンプ影縛解除悪態１「ちっ」//vmpakt02 1079
SD_V_VMPAKT02,	//ヴァンプ影縛解除悪態２「えぃ」//vmpakt03 1080
SD_V_VMPASE01,	//ヴァンプ焦り１「まだまだ！」//vmpase01 1081
SD_V_VMPASE02,	//ヴァンプ焦り２「やるな！」//vmpase02 1082
SD_V_VMPASE03,	//ヴァンプ焦り３「うっうぅ」//vmpase03 1083
SD_V_VMPASE04,	//ヴァンプ焦り４「えひぃ」//vmpase04 1084
SD_V_VMPCUT01,	//ヴァンプナイフ攻撃強「シャー」//vmpcut01 1085
SD_V_VMPCUT02,	//ヴァンプナイフ攻撃１「シェッ」//vmpcut02 1086
SD_V_VMPCUT03,	//ヴァンプナイフ攻撃２「フンッ」//vmpcut03 1087
SD_V_VMPCUT04,	//ヴァンプナイフ攻撃３「ホッ！」//vmpcut04 1088
SD_V_VMPCUT05,	//ヴァンプナイフ攻撃４「ハッ！」//vmpcut05 1089
SD_V_VMPDMG01,	//ヴァンプダメージ１「ウッ！」//vmpdmg01 1090
SD_V_VMPDMG02,	//ヴァンプダメージ２「イッ！」//vmpdmg03 1091
SD_V_VMPDMG03,	//ヴァンプダメージ３「オォッ！」//vmpdmg05 1092
SD_V_VMPDMG04,	//ヴァンプダメージ４「カァッ！」//vmpdmg06 1093
SD_V_VMPDMH01,	//ヴァンプ強ダメージ１「ウアッ」//vmpdmh01 1094
SD_V_VMPDMH02,	//ヴァンプ強ダメージ２「アァア」//vmpdmh02 1095
SD_V_VMPDMH03,	//ヴァンプ強ダメージ３「ウァア」//vmpdmh04 1096
SD_V_VMPESB01,	//ヴァンプバレエ嘲笑「ハハハッ」//vmpesb01 1097
SD_V_VMPESB02,	//ヴァンプバレエ避１「無駄だ！」//vmpesb02 1098
SD_V_VMPESB03,	//ヴァンプバレエ避２「読めるぞ」//vmpesb03 1099
SD_V_VMPESR01,	//ヴァンプ水面避け１「遅い」//vmpesr02 1100
SD_V_VMPESR02,	//ヴァンプ水面避け２「無駄だ」//vmpesr03 1101
SD_V_VMPGRE01,	//ヴァンプグレネード１「食らえ」//vmpgre01 1102
SD_V_VMPGRE02,	//ヴァンプグレネード２「フッ！」//vmpgre02 1103
SD_V_VMPGRE03,	//ヴァンプグレネード３「ヘァッ」//vmpgre03 1104
SD_V_VMPGRE04,	//ヴァンプグレネード４「テヤッ」//vmpgre04 1105
SD_V_VMPIND01,	//ヴァンプLIFE稀少潜り１「ちぃ」//vmpind01 1106
SD_V_VMPIND02,	//ヴァンプLIFE稀少潜り２「くそ」//vmpind02 1107
SD_V_VMPINW01,	//ヴァンプダメージ潜り１「ふっ」//vmpinw01 1108
SD_V_VMPINW02,	//ヴァンプダメージ潜り２「くっ」//vmpinw02 1109
SD_V_VMPJAW01,	//ヴァンプ上陸１「こっちだ！」//vmpjaw01 1110
SD_V_VMPJAW02,	//ヴァンプ上陸２「ここだ！」//vmpjaw02 1111
SD_V_VMPJAW03,	//ヴァンプ上陸３「どこを見て～」//vmpjaw03 1112
SD_V_VMPJAW04,	//ヴァンプ上陸４「だあぁっ！」//vmpjaw05 1113
SD_V_VMPKIK01,	//ヴァンプキック気合１「フエッ」//vmpkik01 1114
SD_V_VMPKIK02,	//ヴァンプキック気合２「テエッ」//vmpkik02 1115
SD_V_VMPKIK03,	//ヴァンプキック気合３「イヤッ」//vmpkik03 1116
SD_V_VMPKIK04,	//ヴァンプキック気合４「フンッ」//vmpkik04 1117
SD_V_VMPMAN01,	//ヴァンプ手動命中１「くうっ！」//vmpman01 1118
SD_V_VMPMAN02,	//ヴァンプ手動命中２「やるな！」//vmpman02 1119
SD_V_VMPMAN03,	//ヴァンプ手動命中３「ぬえぇい」//vmpman03 1120
SD_V_VMPMAN04,	//ヴァンプ手動命中４「ちぇえい」//vmpman04 1121
SD_V_VMPRUN01,	//ヴァンプ壁走り１「ケッヘヘヘ」//vmprun02 1122
SD_V_VMPRUN02,	//ヴァンプ壁走り２「ィアーーー」//vmprun04 1123
SD_V_VMPSTN01,	//ヴァンプスタン被爆１「目が！」//vmpstn01 1124
SD_V_VMPSTN02,	//ヴァンプスタン被爆２「見えん」//vmpstn02 1125
SD_V_VMPTAM01,	//ヴァンプ気を集中１「ぬぅぅぅ」//vmptam01 1126
SD_V_VMPTAM02,	//ヴァンプ気を集中２「はぁぁぁ」//vmptam02 1127
SD_P_R_MIZIN1,	//ライデン水有エルード飛込//r_mizin1 1128
SD_P_R_MIZOT1,	//ライデン水有エルード飛出//r_mizot1 1129
SD_E_V_FOOTL3,	//ヴァンプ足踏み左//v_footl3 1130
SD_E_V_FOOTR3,	//ヴァンプ足踏み右//v_footr3 1131
SD_E_V_FOOT02,	//ヴァンプ足踏み強打//v_foot02 1132
SD_E_V_FOTML3,	//ヴァンプ足踏み左（鉄棒）//v_fotml3 1133
SD_E_V_FOTMR3,	//ヴァンプ足踏み右（鉄棒）//v_fotmr3 1134
SD_E_V_FOOTM2,	//ヴァンプ足踏み強打（鉄棒）//v_footm2 1135
SD_E_V_JUMP04,	//ヴァンプジャンプ音//v_jump04 1136
SD_E_V_JUMPM4,	//ヴァンプジャンプ音（鉄棒）//v_jumpm4 1137
SD_E_V_TYAKU1,	//ヴァンプ着地//v_tyaku1 1138
SD_E_V_TYAKM1,	//ヴァンプ着地（鉄棒）//v_tyakm1 1139
SD_E_V_HNCL01,	//ヴァンプ手拍子１//v_hncl01 1140
SD_E_V_HNCL02,	//ヴァンプ手拍子２//v_hncl02 1141
SD_E_V_ROOL01,	//ヴァンプ回転ダンス//v_rool01 1142
SD_E_V_KICK01,	//ヴァンプキック音//v_kick01 1143
SD_E_V_NNAGE1,	//ヴァンプナイフ投げ音//v_nnage1 1144
SD_E_MIZU_IN2,	//ヴァンププール飛込み着水(高)//mizu_in2 1145
SD_E_MIZU_IN3,	//ヴァンププール飛込み着水(低)//mizu_in3 1146
SD_E_MIZU_OT1,	//ヴァンププール上がり飛び出し//mizu_ot1 1147
SD_E_V_SWIM01,	//ヴァンプ泳ぎ中（12fps連）//v_swim01 1148
SD_E_V_HEAR01,	//ヴァンプ心臓音（アウト後？）//v_hear01 1149
SD_E_N_YUKA01,	//ナイフ壁、床刺さり1（rdm呼）//n_yuka01 1150
SD_E_N_YUKA02,	//ナイフ壁、床刺さり2（rdm呼）//n_yuka02 1151
SD_E_N_KABE01,	//ナイフ壁、壁刺さり1（rdm呼）//n_kabe01 1152
SD_E_N_KABE02,	//ナイフ壁、壁刺さり2（rdm呼）//n_kabe02 1153
SD_P_N_DMG003,	//ナイフ刺さりダメージ音//n_dmg003 1154
SD_P_N_SHOT02,	//ナイフ打ち落し//n_shot02 1155
SD_E_V_NCUT01,	//ナイフ斬られ//v_ncut01 1156
SD_P_V_NFURI2,	//ナイフ振り//v_nfuri2 1157
SD_E_V_KAGE02,	//影縛りダメージ//v_kage02 1158
SD_A_GUN_WTR2,	//銃弾プール着水//gun_wtr2 1159
SD_E_ARMGUN01,	//ＲＡＹ腕部機銃発砲//armgun01 1160
SD_E_ARMREB01,	//ＲＡＹ機銃跳弾//armreb01 1161
SD_E_ARMSWI01,	//ＲＡＹ機銃構える//armswi01 1162
SD_E_GROWL001,	//ＲＡＹ吠える１（ダメージ１）//growl001 1163
SD_E_GROWL002,	//ＲＡＹ吠える２（近気合）//growl002 1164
SD_E_GROWL003,	//ＲＡＹ吠える３（遠気合）//growl003 1165
SD_E_GROWL004,	//ＲＡＹ吠える４（ダメージ２）//growl004 1166
SD_E_LASCLS01,	//ＲＡＹレーザー口閉まる//lascls01 1167
SD_E_LASER101,	//ＲＡＹレーザー発射開始(12fps待)//laser101 1168
SD_E_LASER201,	//ＲＡＹレーザー発射中(8fps連)//laser201 1169
SD_E_LASOPN01,	//ＲＡＹレーザー口開く//lasopn01 1170
SD_E_LEGMISE1,	//ＲＡＹミサイル飛行１(左足側)//legmise1 1171
SD_E_LEGMISE2,	//ＲＡＹミサイル飛行２(右足側)//legmise2 1172
SD_E_LEGMISF1,	//ＲＡＹミサイル発射//legmisf1 1173
SD_E_LEGSWI01,	//ＲＡＹ踏み付け足上げ//legswi01 1174
SD_E_RBRESS01,	//ＲＡＹ白い息を吐く//rbress01 1175
SD_E_R_FTAT01,	//ＲＡＹ踏み付け足音//r_ftat01 1176
SD_E_R_FTFG01,	//ＲＡＹ遠距離で着地//r_ftfg01 1177
SD_E_R_FTFJ01,	//ＲＡＹ遠距離でジャンプ//r_ftfj01 1178
SD_E_R_FTFL01,	//ＲＡＹ遠距離足音１（左）//r_ftfl01 1179
SD_E_R_FTFR01,	//ＲＡＹ遠距離足音１（右）//r_ftfr01 1180
SD_E_R_FTFL02,	//ＲＡＹ遠距離足音２（左）//r_ftfl01 1181
SD_E_R_FTFR02,	//ＲＡＹ遠距離足音２（右）//r_ftfr01 1182
SD_E_R_FTFL03,	//ＲＡＹ遠距離足音３（左）//r_ftfl01 1183
SD_E_R_FTFR03,	//ＲＡＹ遠距離足音３（右）//r_ftfr01 1184
SD_E_R_FTNG01,	//ＲＡＹ近距離で着地//r_ftng01 1185
SD_E_R_FTNJ01,	//ＲＡＹ近距離でジャンプ//r_ftnj01 1186
SD_E_R_FTNL01,	//ＲＡＹ近距離足音（左）//r_ftnl01 1187
SD_E_R_FTNR01,	//ＲＡＹ近距離足音（右）//r_ftnr01 1188
SD_E_TAILHIT1,	//ＲＡＹ尻尾攻撃ヒット//tailhit1 1189
SD_E_TAILSW01,	//ＲＡＹ尻尾振り音//tailsw01 1190
SD_E_GISIGI01,	//ＲＡＹチャフ受け軋み//gisigi01 1191
SD_A_SAINTWTR,	//小便器への小便(12fps連)//saintwtr 1192
SD_A_DRYERSTR,	//ハンドドライヤー開始(12fps開け)//dryerstr 1193
SD_A_DRYEREND,	//ハンドドライヤー中(12fps連)//dryerend 1194
SD_A_BIGBEN01,	//大便器流す//bigben01 1195
SD_A_SMLBEN01,	//小便器流す//smlben01 1196
SD_A_YUBIBUE2,	//ファットマン指笛//yubibue2 1197
SD_A_FOOT_S1L,	//裸足足音左（呼び分け用）//foot_s1l 1198
SD_A_FOOT_S1R,	//裸足足音右（呼び分け用）//foot_s1r 1199
SD_A_ROLLFIX1,	//ローラーブレード調子直し１//rollfix1 1200
SD_A_ROLLFIX2,	//ローラーブレード調子直し２//rollfix2 1201
SD_A_OMORASI1,	//人質おもらし音//omorasi1 1202
SD_E_H_SWINGR,	//ハリアー交錯（左から右）//h_swingr 1203
SD_E_H_SWINGL,	//ハリアー交錯（右から左）//h_swingl 1204
SD_E_AMALERT1,	//ハリアーアムラーム接近警告音//amalert1 1205
SD_E_PHYFAR09,	//サイファーよたよた飛行音１//phyfar03 1206
SD_E_PHYFAR0A,	//サイファーよたよた飛行音２//phyfar03 1207
SD_E_PHYFAR0B,	//サイファーよたよた飛行音３//phyfar03 1208
SD_E_PHYFAR0C,	//サイファーよたよた飛行音４//phyfar03 1209
SD_E_PHYFAR0D,	//サイファー継ぎ目飛行音１//phyfar04 1210
SD_E_PHYFAR0E,	//サイファー継ぎ目飛行音２//phyfar04 1211
SD_E_PHYFAR0F,	//サイファー継ぎ目飛行音３//phyfar04 1212
SD_E_PHYFAR10,	//サイファー継ぎ目飛行音４//phyfar04 1213
SD_E_CYPHDMG1,	//サイファー弾丸ヒット音//cyphdmg1 1214
SD_E_H_SWIDM1,	//ハリアー飛び去りデモ//h_swidm1 1215
SD_E_H_SWIDM2,	//ハリアーホバリング後飛去りデモ//h_swidm2 1216
SD_E_H_SWIDM3,	//ハリアーホバリング上昇デモ//h_swidm3 1217
SD_E_H_SWIDM4,	//ハリアーカメラかすめデモ//h_swidm4 1218
SD_E_H_HOVUP1,	//ハリアーホバリング上昇音//h_hovup1 1219
SD_E_H_HOVAS1,	//ハリアーホバリング後飛び去り//h_hovas1 1220
SD_A_KA_GRE01,	//スネークＭ４グレネード発射//ka_gre01 1221
SD_A_BRIDGEX1,	//橋壊れ破片音//bridgex1 1222
SD_A_CHUSHA01,	//注射（リキッド腕抑制）//chusha01 1223
SD_A_DM_KINU1,	//シナリオデモ用衣擦れ１遅//dm_kinu1 1224
SD_A_DM_KINU2,	//シナリオデモ用衣擦れ２普//dm_kinu2 1225
SD_A_DM_KINU3,	//シナリオデモ用衣擦れ３速//dm_kinu3 1226
SD_A_G_HOLSTR,	//オセロット銃しまう//g_holstr 1227
SD_A_G_KAMAE1,	//オセロット銃構える//g_kamae1 1228
SD_A_G_PLAY01,	//オセロットガンプレイ回し１//g_play01 1229
SD_A_G_PLAY02,	//オセロットガンプレイ回し２//g_play02 1230
SD_A_G_PLAY03,	//オセロットガンプレイ回転止め//g_play03 1231
SD_A_G_WALK1L,	//オセロット足音左//g_walk1l 1232
SD_A_G_WALK1R,	//オセロット足音右//g_walk1r 1233
SD_A_G_WALK2S,	//オセロット停止足音//g_walk2s 1234
SD_A_O_KAMAE1,	//オルガ銃構える//o_kamae1 1235
SD_A_O_WALK1L,	//オルガ足音左//o_walk1l 1236
SD_A_O_WALK1R,	//オルガ足音右//o_walk1r 1237
SD_A_O_WALK2S,	//オルガ停止足音//o_walk2s 1238
SD_A_S_MANTOB,	//ソリダスマント翻す//s_mantob 1239
SD_A_S_WALK1L,	//ソリダス足音左//s_walk1l 1240
SD_A_S_WALK1R,	//ソリダス足音右//s_walk1r 1241
SD_A_S_WALK2S,	//ソリダス停止足音//s_walk2s 1242
SD_A_O_WALM1L,	//オルガ鉄床足音左//o_walm1l 1243
SD_A_O_WALM1R,	//オルガ鉄床足音右//o_walm1r 1244
SD_A_O_WALM2S,	//オルガ鉄床停止足音//o_walm2s 1245
SD_A_O_HWALL1,	//オルガ壁パンチ//o_hwall1 1246
SD_A_BODYBLOW,	//オルガボディブロウ//bodyblow 1247
SD_A_KASEOPEN,	//拷問台カセ外れる//kaseopen 1248
SD_A_KASESTOP,	//拷問台カセで動けない//kasestop 1249
SD_A_HEBATT02,	//蛇手突き振り//hebatt02 1250
SD_A_HEBFURI3,	//蛇手振り//hebfuri3 1251
SD_A_HEBREDY2,	//蛇手構える//hebredy2 1252
SD_A_HEBSCAN1,	//蛇手スキャニング（12fps連）//hebscan1 1253
SD_A_HEBSIME2,	//蛇手首絞め//hebsime2 1254
SD_A_HEBTKAM2,	//蛇手掴みヒット//hebtkam2 1255
SD_A_DRIPPY13,	//ポンプ室水滴落ち//drippy13 1256
SD_A_SVERY_01,	//ライデン糞スベリ足音１遭遇//svery_01 1257
SD_A_SVERY_02,	//ライデン糞スベリ足音２足掻き//svery_02 1258
SD_A_EV_RMOV1,	//エレベータ屋上柵動く//ev_rmov1 1259
SD_A_EV_RSAK1,	//エレベータ屋上柵開き切る//ev_rsak1 1260
SD_A_EV_RSAK2,	//エレベータ屋上柵閉まり切る//ev_rsak2 1261
SD_E_V_ROOL02,	//ヴァンプ空中回転//v_rool02 1262
SD_E_V_ROOL03,	//ヴァンプ遅い空中回転//v_rool03 1263
SD_E_V_WALKL2,	//ヴァンプ歩行左//v_walkl2 1264
SD_E_V_WALKR2,	//ヴァンプ歩行右//v_walkr2 1265
SD_E_V_RUNL02,	//ヴァンプ走行左//v_runl02 1266
SD_E_V_RUNR02,	//ヴァンプ走行右//v_runr02 1267
SD_A_WPN_WTR1,	//ウエポン着水//wpn_wtr1 1268
SD_A_PC_INST1,	//端末にメディアを挿入//pc_inst1 1269
SD_A_PC_BOOT1,	//端末電源ＯＮ～ファン回転(持続)//pc_boot1 1270
SD_A_PC_HDSEK,	//端末ＨＤシーク(2fps連射で使う)//pc_hdsek 1271
SD_A_PC_START,	//端末起動開始BEEP「ピポッ」//pc_start 1272
SD_A_PC_ALART,	//端末警告BEEP「ピーッ」//pc_alart 1273
SD_A_PC_OKOK1,	//端末了承BEEP「ピロリロッ」//pc_okok1 1274
SD_A_HA_CMPLT,	//HAL5000ロード完了//ha_cmplt 1275
SD_A_HA_SCLWI,	//HAL5000大ラインスクロール//ha_sclwi 1276
SD_A_HA_MOVLI,	//HAL5000小ライン枠移動//ha_movli 1277
SD_A_HA_DISCN,	//HAL5000接続完了文字点滅//ha_discn 1278
SD_A_OTA_IN_1,	//HAL5000オタコン生える//ota_in_1 1279
SD_A_OTA_OUT1,	//HAL5000オタコン沈む//ota_out1 1280
SD_A_OTA_OK_1,	//HAL5000オタコンＯＫ//ota_ok_1 1281
SD_A_OTA_NG_1,	//HAL5000オタコンだめ//ota_ng_1 1282
SD_A_OTA_LAU1,	//HAL5000オタコン笑い//ota_lau1 1283
SD_A_OTA_SHY1,	//HAL5000オタコン恥辱//ota_shy1 1284
SD_A_OTA_SPR1,	//HAL5000オタコン驚き//ota_spr1 1285
SD_A_FLOORON1,	//壊れ床スタート（踏んだ瞬間）//flooron1 1286
SD_A_FLOORVB1,	//壊れ床軋む１（周期合わせて連）//floorvb1 1287
SD_A_FLOORVB2,	//壊れ床軋む２（周期合わせて連）//floorvb1 1288
SD_A_FLOOROU1,	//壊れ床外れる１//floorbr2 1289
SD_A_FLOOROU2,	//壊れ床外れる２//floorbr2 1290
SD_A_FLOORFL1,	//壊れ床床に落ちる//floorfl1 1291
SD_A_BLTCNV1A,	//コンベヤ駆動１－１//bltconv1 1292
SD_A_BLTCNV2A,	//コンベヤ駆動１－２(22fps交互)//bltconv2 1293
SD_A_BLTCNV1B,	//コンベヤ駆動２－１//bltconv1 1294
SD_A_BLTCNV2B,	//コンベヤ駆動２－２(22fps交互)//bltconv2 1295
SD_A_BLTCNV1C,	//コンベヤ駆動３－１//bltconv1 1296
SD_A_BLTCNV2C,	//コンベヤ駆動３－２(22fps交互)//bltconv2 1297
SD_V_RAISNZ01,	//ライデン裸くしゃみ「へっし！」//raisnz01 1298
SD_V_RAIELE01,	//ライデン電気床悲鳴１「あはぁ」//raiele01 1299
SD_V_RAIELE02,	//ライデン電気床悲鳴２「あっは」//raiele02 1300
SD_V_RAIELE03,	//ライデン電気床悲鳴３「あがっ」//raiele03 1301
SD_V_RAIELE04,	//ライデン電気床悲鳴４「うっふ」//raiele04 1302
SD_A_MONINOZ1,	//中央モニターノイズ１//moninoz1 1303
SD_A_MONINOZ2,	//中央モニターノイズ２(6fps連)//moninoz1 1304
SD_A_MONITOR1,	//中央モニター破壊//monitor1 1305
SD_A_TRSPARK1,	//変電室火花スパーク//trspark1 1306
SD_A_TRSPARK2,	//変電室火花粒落ち//trspark2 1307
SD_A_ONARA001,	//人質オナラ１「ブォ」//onara001 1308
SD_A_ONARA002,	//人質オナラ２「ブッ」//onara002 1309
SD_A_ONARA003,	//人質オナラ３「プゥ」//onara003 1310
SD_A_ONARA004,	//人質オナラ４「ピィー」//onara004 1311
SD_A_FLAG_L01,	//旗接触大（撃つ、殴る蹴る）//flag_l01 1312
SD_A_FLAG_S02,	//旗接触小（走って通りぬける）//flag_s02 1313
SD_A_HO_WIND1,	//外部へのハッチ風(w28a)//ho_wind1 1314
SD_A_HO_WIND2,	//外部からのハッチ風(w20a)//ho_wind2 1315
SD_A_CANWTR01,	//水浮き缶飛ぶ//canwtr01 1316
SD_A_CANWALL1,	//水浮き缶壁弾み//canwall1 1317
SD_A_CANWIN01,	//水浮き缶着水//canwin01 1318
SD_A_FALLWTR1,	//滝水１//fallwtr1 1319
SD_A_FALLWTR2,	//滝水２(交互15fps連)//fallwtr2 1320
SD_A_FALLHIT1,	//滝水カブリ１//fallhit1 1321
SD_A_FALLHIT2,	//滝水カブリ２(交互15fps連)//fallhit2 1322
SD_A_INWDOOR2,	//水中水密ドア取っ手回す//inwdoor2 1323
SD_P_HEADNET1,	//イントルード主観柵あたり//headnet1 1324
SD_P_LAMPON01,	//長廊下ランプ点灯//lampon01 1325
SD_P_LAMPOFF1,	//長廊下ランプ消え//lampoff1 1326
SD_A_SLOCKOP2,	//横開ロッカー開かない（ガチ）//slockop2 1327
SD_A_SLOCKOP1,	//横開ロッカー開く（カチャ）//slockop1 1328
SD_A_SLOCKMV1,	//横開ロッカー動作（キィ…）//slockmv1 1329
SD_A_SLOCKCL2,	//横開ロッカー開き切る（カチン）//slockcl2 1330
SD_A_SLOCKCL1,	//横開ロッカー閉リ切る（ガツン）//slockcl1 1331
SD_A_PDOOROP1,	//厨房ドア開かない（ガツ）//pdoorop1 1332
SD_A_PIPEWTR1,	//ＢＣ橋パイプ水１//pipewtr1 1333
SD_A_PIPEWTR2,	//ＢＣ橋パイプ水２(交互15fps連)//pipewtr2 1334
SD_A_DAZZHIT1,	//投光器角度変更跳弾//dazzhit1 1335
SD_A_DAZZLING,	//投光器まぶしい(6fps連)//dazzling 1336
SD_A_BLTSTOP1,	//コンベヤ駆動停止//bltstop1 1337
SD_A_BLTSTRT1,	//コンベヤ駆動開始//bltstrt1 1338
SD_A_LED_BLUE,	//コンベア制御ランプ青点灯//led_blue 1339
SD_A_LED_YELL,	//コンベア制御ランプ黄点灯//led_yell 1340
SD_A_LED_RED1,	//コンベア制御ランプ赤点灯//led_red1 1341
SD_A_ELTSTAR1,	//服装検査作動開始//eltstar1 1342
SD_A_ELTSCAN1,	//服装検査検査中(点滅に合わせ)//eltscan1 1343
SD_A_ELTNOT01,	//服装検査ＮＧ(敵発見まで12fps連)//eltnot01 1344
SD_A_ELTGOOD1,	//服装検査ＯＫ//eltgood1 1345
SD_A_DANB_IN1,	//段ボール到着スタート//danb_in1 1346
SD_A_INWTRG01,	//投擲物投げ入れ//inwtrg01 1347
SD_A_SWIGUARJ,	//刀ガード構える//swiguarj 1348
SD_A_SNAPPK01,	//NPCスネークw11a敵を気絶させる//snappk01 1349
SD_P_INWBRES1,	//水中マスク呼吸//inwbres1 1350
SD_A_BF_ROLL1,	//ファットマン戦前デモ前滑走音//bf_roll1 1351
SD_A_ELFLOOR1,	//電気床感電カット切れ目１//elfloor1 1352
SD_A_ELFLOOR2,	//電気床感電カット切れ目２//elfloor2 1353
SD_A_ELFLOOR3,	//電気床感電カット切れ目３//elfloor3 1354
SD_A_ELFLOOR4,	//電気床感電カット切れ目４//elfloor4 1355
SD_A_ELFSPRK1,	//電気床スパーク//elfsprk1 1356
SD_E_LEGMISE3,	//ＲＡＹミサイル落下１(左足側)//legmise3 1357
SD_E_LEGMISE4,	//ＲＡＹミサイル落下２(右足側)//legmise4 1358
SD_A_OFFSENS1,	//セムテックス赤外線トラップ解除//offsens1 1359
SD_E_DOWNFAT1,	//ファットマンダウン音//downfat1 1360
SD_E_REBFAT01,	//ファットマン反射ボイーン//rebfat01 1361
SD_E_RWATER01,	//ＲＡＹ外環動作水飛沫１//rwater01 1362
SD_A_THUND_P1,	//タイトルフラッシュ//thund_p1 1363
SD_A_OP_NPAP1,	//新聞摩擦（上スクロール）//op_npap1 1364
SD_A_OP_BOOK1,	//本摩擦（右スクロール）//op_book1 1365
SD_A_OP_BOOK2,	//本消え（消失）//op_book2 1366
SD_A_PAGEN001,	//新聞１Ｐめくり//pagen001 1367
SD_A_PAGEB001,	//本１Ｐめくり//pageb001 1368
SD_A_PAGE10N1,	//新聞１０Ｐめくり//page10n1 1369
SD_A_PAGE10B1,	//本１０Ｐめくり//page10b1 1370
SD_A_PAGECLN1,	//新聞とじる//pagecln1 1371
SD_A_PAGECLB1,	//本閉じる//pageclb1 1372
SD_A_C4SURE01,	//Ｃ４引き摺り１（右から中）//c4sure01 1373
SD_A_C4SURE02,	//Ｃ４引き摺り２（中から前）//c4sure02 1374
SD_A_C4SWIT11,	//Ｃ４タイマースイッチＯＮ//c4swit11 1375
SD_A_SPRYGET1,	//冷却スプレー缶持つ//spryget1 1376
SD_A_SPRYPUT1,	//冷却スプレー缶置く//spryput1 1377
SD_A_SENSGET1,	//センサーＡ持つ//sensget1 1378
SD_A_SENSPUT1,	//センサーＡ置く//sensput1 1379
SD_S_JINGLE02,	//緊迫ジングル音２(死体ロッカー)//tan_fnd2 1380
SD_S_JINGLE03,	//緊迫ジングル音３(敵兵発見)//cyp_fnd2 1381
SD_E_RWATER02,	//ＲＡＹ外環動作水飛沫２//rwater02 1382
SD_E_RBLOOD01,	//ＲＡＹ血を流す//rblood01 1383
SD_A_AF_ROLL1,	//ファットマン戦終了デモ前滑走音//af_roll1 1384
SD_A_O_WALK3M,	//オルガ足音フォロー(拷問デモ)//o_walk3m 1385
SD_A_ELFNOZ01,	//電気床駆動音ヴーーン(6fps連)//elfnoz01 1386
SD_V_SOLMUTE1,	//ソリダスＳＥ音声ミュート//dummy 1387
SD_A_INWBOMS1,	//水中爆発水飛沫//inwboms1 1388
SD_A_TOILCLO1,	//トイレ扉ドア閉まる（バタン）//toilclo1 1389
SD_A_TOILMOV1,	//トイレ扉ドア動作（キィ…）//toilmov1 1390
SD_A_TOILOPN1,	//トイレ扉ドア開く（カチャ）//toilopn1 1391
SD_A_TOILHIT1,	//トイレ扉に主観で頭をぶつける。//toilhit1 1392
SD_A_TOILOPN2,	//トイレ扉ドア開かない（ガチン）//toilopn2 1393
SD_A_TOILDWN1,	//トイレ扉ドア外れる//toildwn1 1394
SD_A_TOILDWN2,	//トイレ扉ドア弾む//toildwn2 1395
SD_A_TOILDWN3,	//トイレ扉ドア倒れる//toildwn3 1396
SD_A_TOILDMG1,	//トイレ扉ドア軋みズレ//toildmg1 1397
SD_A_TOILATK1,	//トイレ扉ドア攻撃当たり//toilatk1 1398
SD_A_TEJOHGT1,	//後ろ手に手錠で動けない//tejohgt1 1399
SD_A_FENCEBR1,	//ダクト内フェンスダメージ//FENCEBR1 1400
SD_A_FENCEBR2,	//ダクト内フェンス壊れ//FENCEBR2 1401
SD_W_SOCOMNP1,	//NPCスネーク拳銃ショット//socomnp1 1402
SD_W_FAMASNP1,	//NPCスネーク自動小銃ショット//famasnp1 1403
SD_A_SEALSGNB,	//w14aＣＤ橋でシールズ頑張る//sealsgnb 1404
SD_A_KAIDANGR,	//グラグラ階段グラグラ//kaidangr 1405
SD_A_KAIDANFA,	//グラグラ階段取れる//kaidanfa 1406
SD_A_FLOORTK1,	//壊れ床完全に取れる//floortk1 1407
SD_E_N_NUKI01,	//ヴァンプ、鞘からナイフ抜く//n_nuki01 1408
SD_E_BROODSWJ,	//天狗兵、血プシー！//broodswj 1409
SD_A_HA_LOAD1,	//HAL5000読み込み中//HA_LOAD1 1410
SD_A_FNALPES1,	//船虫スプレー逃げ１//fnalpesc 1411
SD_A_FNALPES2,	//船虫スプレー逃げ２(交互18fps連)//fnalpesc 1412
SD_A_C4COUNTS,	//カウントダウン（ファットマン）//c4counts 1413
SD_E_TKALAND1,	//ソリダスハイジャンプ着地//tkaland1 1414
SD_P_FOOT2S1L,	//プレイヤー裸足歩き足音（左）//foot2s1l 1415
SD_P_FOOT2S1R,	//プレイヤー裸足歩き足音（右）//foot2s1r 1416
SD_P_SENAKA2N,	//裸足で壁に張付く//senaka2n 1417
SD_P_STAND02N,	//裸足でしゃがみ、立ち上がり//stand2n 1418
SD_P_STAND03N,	//裸足で側転着地(ライデン)//stand3n 1419
SD_S_MUTEW11A,	//w11a緊迫ジングル音ミュート//mutew11a 1420
SD_A_WPN_WTR2,	//浮いてる足音着水//wpn_wtr2 1421
SD_S_JINGLE04,	//緊迫ジングル音４(爆弾発見)//bom_fnd1 1422
SD_P_FOOTW01L,	//プレイヤー歩き足音１（左）//footw02l 1423
SD_P_FOOTW01R,	//プレイヤー歩き足音１（右）//footw02r 1424
SD_P_FOOTW02L,	//プレイヤー歩き足音２（左）//footwm2l 1425
SD_P_FOOTW02R,	//プレイヤー歩き足音２（右）//footwm2r 1426
SD_P_FOOTW03L,	//プレイヤー歩き足音３（左）//footwm2l 1427
SD_P_FOOTW03R,	//プレイヤー歩き足音３（右）//footwm2r 1428
SD_P_FOOTW04L,	//プレイヤー歩き足音４（左）//footwm2l 1429
SD_P_FOOTW04R,	//プレイヤー歩き足音４（右）//footwm2r 1430
SD_P_FOOTW05L,	//プレイヤー歩き足音５（左）//footwm2l 1431
SD_P_FOOTW05R,	//プレイヤー歩き足音５（右）//footwm2r 1432
SD_P_FOOTW06L,	//プレイヤー歩き足音６（左）//footwm2l 1433
SD_P_FOOTW06R,	//プレイヤー歩き足音６（右）//footwm2r 1434
SD_P_FOOTW07L,	//プレイヤー歩き足音７（左）//footwm2l 1435
SD_P_FOOTW07R,	//プレイヤー歩き足音７（右）//footwm2r 1436
SD_P_FOOTW08L,	//プレイヤー歩き足音８（左）//footwm2l 1437
SD_P_FOOTW08R,	//プレイヤー歩き足音８（右）//footwm2r 1438
SD_V_SNADKN01,	//NPCスネーク「どこを狙ってる！」//snadkn01 1439
SD_E_V_JMPOUT,	//ヴァンプアウト後ジャンプ//v_jmpout 1440
SD_A_FLAGBT12,	//旗バタ１（1と2をランダムに）//flagbt12 1441
SD_A_FLAGBT13,	//旗バタ２（mtnの周期に合わせて）//flagbt13 1442
SD_A_LADERLP1,	//アンテナ回転きしみ（１回転毎）//laderlp1 1443
SD_A_LADERMT1,	//アンテナ回転駆動（8fps毎連）//ladermt1 1444
SD_A_NETCLSD1,	//鉄ネット柵ドア開かない//netclsd1 1445
SD_P_SLW_IN02,	//水面ハシゴ降り//slw_in02 1446
SD_P_SLWOUT02,	//水面ハシゴ登り//slwout02 1447
SD_P_OUTMOVE1,	//水面動作１（エルード移動）//outmove1 1448
SD_P_OUTMOVE2,	//水面動作２（懸垂上がり）//outmove2 1449
SD_P_OUTMOVE3,	//水面動作３（懸垂下がり）//outmove3 1450
SD_E_INWTERL4,	//敵兵水落ち、連絡橋シナリオ呼び//inwater4 1451
SD_S_SHAFFLE1,	//ローポリ劇場シャッフル音//shaffle1 1452
SD_A_NOIZPTR1,	//ピーターの物音//noizptr1 1453
SD_A_AF_MUTE1,	//ファットマン終了滑走ミュート//af_mute1 1454
SD_A_V_CLASHA,	//標的破壊１//v_clasha1 1455
SD_A_V_HITNG1,	//壊しちゃダメ標的破壊アラーム//v_hitng1 1456
SD_A_V_HITOK1,	//壊していい標的破壊チャイム//v_hitok1 1457
SD_A_V_HOLE03,	//落とし穴作動//v_hole03 1458
SD_A_V_LOCKON,	//ロックオンサークル表示//v_lockon 1459
SD_A_V_LOCKST,	//ロックオン軸線表示//v_lockst 1460
SD_A_V_MATO_G,	//標的出現//v_mato_g 1461
SD_A_V_MATO_I,	//消える標的再出現//v_mato_i 1462
SD_A_V_MATO_O,	//消える標的消失//v_mato_o 1463
SD_A_V_START1,	//開始地形創造１（36fps再生した後）//v_start12 1464
SD_A_V_START2,	//開始地形創造２（これを12fpsで連）//v_start22 1465
SD_A_V_CLASHS,	//標的刀斬り破壊１//v_clashs1 1466
SD_A_V_MTDMGP,	//標的Ｐヒットダメージ//v_mtdmgp 1467
SD_A_V_MTDMGK,	//標的Ｋヒットふっとび//v_mtdmgk 1468
SD_A_V_ROLLIN,	//標的ふっとび後クルクル(１回転毎）//v_rollin 1469
SD_S_TWINKYST,	//「ファイト！」文字点滅//twinky01 1470
SD_A_V_GOALAP,	//ゴール出現//v_goalap 1471
SD_A_V_HOLE13,	//落とし穴作動２//v_hole03 1472
SD_E_GLLFOOTL,	//ゴルルゴン足音（左）//gllfootl 1473
SD_E_GLLFOOTR,	//ゴルルゴン足音（右）//gllfootr 1474
SD_E_GLLGACHA,	//ゴルルゴン歩行衣擦れ//gllgacha 1475
SD_A_V_HOLDUP,	//敵兵ホールドアップ消え//v_holdup5 1476
SD_E_GLLHOWL1,	//ゴルルゴン咆える//gllhowl1 1477
SD_E_GLLBRESS,	//ゴルルゴン唸る//gllbress 1478
SD_E_GLLKINUZ,	//ゴルルゴン上体動衣擦れ//gllkinuz 1479
SD_E_GLLFOOTL2,	//ゴルルゴン足音（左）２//gllfootl 1480
SD_E_GLLFOOTR2,	//ゴルルゴン足音（右）２//gllfootr 1481
SD_E_GLLGACHA2,	//ゴルルゴン歩行衣擦れ２//gllgacha 1482
SD_E_GLLHOWL12,	//ゲノラ咆える//gnlhowl1 1483
SD_E_GLLBRESS2,	//ゲノラ唸る//gnlbress 1484
SD_E_GLLKINUZ2,	//ゴルルゴン上体動衣擦れ２//gllkinuz 1485
SD_E_MGNFOOTL,	//メカゲノラ足音（左）２//mgnfootl 1486
SD_E_MGNFOOTR,	//メカゲノラ足音（右）２//mgnfootr 1487
SD_E_MGNGACHA,	//メカゲノラ歩行衣擦れ２//mgngacha 1488
SD_E_MGNHOWL1,	//メカゲノラ咆える//mgnhowl1 1489
SD_E_MGNBRESS,	//メカゲノラ唸る//mgnbress 1490
SD_E_MGNKINUZ,	//メカゲノラ上体動衣擦れ２//mgnkinuz 1491
SD_A_V_SHITAI,	//敵兵死体消え//v_shitai 1492
SD_A_V_MTHIBI,	//標的被射撃ダメージ//v_mthibi 1493
SD_V_RAIGEGEE,	//ライデン取っ手抜けうろたえ声//raigegee 1494
SD_A_V_CLASHW,	//壁板破壊//v_clashw 1495
SD_I_TIMER101,	//前作スネC4所持タイマー1(15fps連)//timer101 1496
SD_I_TIMER102,	//前作スネC4所持タイマー2(交互に)//timer102 1497
SD_E_STEALTH1,	//ステルス兵実体化//stealth1 1498
SD_E_STEALTH2,	//ステルス兵ステルス化//stealth2 1499
SD_E_H_SWING2,	//ハリアー交錯（下→上）//h_swing2 1500
SD_A_V_SIMATO,	//ダミー標的消え//v_simato 1501
SD_A_V_NOIZE1,	//ＶＲ用ゲームオーバー//v_noize1 1502
SD_S_COUNTDW0,	//タイマーカウントダウンゼロ//countdw3 1503
SD_A_V_END001,	//終了地形崩れ//v_end001 1504
SD_A_V_START3,	//開始地形創造３（ケツに一発）//v_start32 1505
SD_A_V_CLASHB,	//標的破壊２（コンボ）//v_clasha2 1506
SD_A_V_CLASHC,	//標的破壊３（コンボMAX）//v_clasha3 1507
SD_A_V_CLASHT,	//標的刀斬り破壊２（コンボ）//v_clashs2 1508
SD_A_V_CLASHU,	//標的刀斬り破壊３（コンボMAX）//v_clashs3 1509
SD_A_V_GOALEF,	//ゴール静止画化フラッシュ//v_goalef 1510
SD_A_ELEMOV11,	//S1中央棟ＥＶ動開始（テイルズ）//elemov11 1511
SD_A_ELEMOV12,	//S1中央棟ＥＶ動終了（テイルズ）//elemov12 1512
SD_A_GSWIM_01,	//ゴルル泳ぎ１(交互に)//gswim_01 1513
SD_A_GSWIM_02,	//ゴルル泳ぎ２(１周期で１個ずつ)//gswim_02 1514
SD_A_GOVERW01,	//ゴルル浮上//goverw01 1515
SD_A_GOVERW_D,	//ゴルル浮上デモ装飾(20fps遅れて)//goverw_d 1516
SD_A_GROW_SEG,	//ゴルル咆哮(ゲーム時のみ)//grow_seg 1517
SD_A_GROW_SED,	//ゴルル咆哮(デモ時のみ)//grow_sed 1518
SD_A_GGBSDOWN,	//小便兵ダウン声//ggbsdown 1519
SD_A_GJUMPW01,	//ゴルル潜り前ジャンプ//gjumpw01 1520
SD_A_GUNDRW01,	//ゴルル潜る//gundrw01 1521
SD_A_GUNDRW_D,	//ゴルル潜るデモ装飾(20fps遅れて)//gundrw_d 1522
SD_A_GBEFOR_D,	//ゴルル浮上前デモ装飾//gbefor_d 1523
SD_A_LIPPLE01,	//ゴルルリップル(8fps連)//lipple01 1524
SD_A_EYEBRI01,	//ゴルル目キュピン(最初の1回だけ)//eyebri01 1525
SD_A_EYEBON01,	//ゴルルボム発射//eyebon01 1526
SD_A_BRVRBOX1,	//ＶＲ箱壊れ//brvrbox1 1527
SD_A_MG_CBIKU,	//チビメカゲ、びっくり音//mg_cbiku 1528
SD_A_MG_CCHAK,	//チビメカゲ、着地//mg_cchak 1529
SD_A_MG_CFIRE,	//チビメカゲ、ホーミング加速//mg_cfire 1530
SD_A_MG_CJUMP,	//チビメカゲ、ジャンプ//mg_cjump 1531
SD_A_MG_CPARA,	//チビメカゲ、落下傘開く//mg_cpara 1532
SD_A_MG_CSHOT,	//首穴からチビメカゲ発射//mg_cshot 1533
SD_A_MG_INCHA,	//チャフ苦しみ(4,8,12,16四択ﾗﾝﾀﾞﾑ)//mg_incha 1534
SD_A_MG_INFAL,	//チャフ首穴に入る//mg_infal 1535
SD_A_MG_KFALL,	//首落ち//mg_kfall 1536
SD_A_MG_KGOAL,	//首ゴール//mg_kgoal 1537
SD_A_MG_KNEJI,	//首外れクルクル(8fps連)//mg_kneji 1538
SD_A_MG_KSURE,	//首床摩擦(首１回転につき１回)//mg_ksure 1539
SD_A_MG_KTORE,	//首外れ切る～落下開始//mg_ktore 1540
SD_A_MG_KWALL,	//首壁当たり//mg_kwall 1541
SD_A_GLLDRUM1,	//ゴルル胸叩きドラミング//glldrum1 1542
SD_A_GLLHANAD,	//ゴルル鼻血噴射//gllhanad 1543
SD_A_GLLDOWN1,	//ゴルルゴン倒れる//glldown1 1544
SD_A_MGNDOWN1,	//メカゲノラ倒れる//mgndown1 1545
SD_A_GLLDOWN12,	//ゲノラ倒れる//glldown1 1546
SD_A_EVMOTOR2,	//エレベータ動く２(Xbox音切対策)//evmotor1 1547
SD_A_DRYEREN2,	//ハンドドライヤー中２(Xbox対策)//dryerend 1548
SD_A_GLLGIMON,	//ゴルルゴン「？」//gllgimon 1549
SD_A_GLLODROK,	//ゴルルゴン「！」//gllodrok 1550
SD_A_GLLKOFUN,	//ゴルルゴン悦のけぞり//gllkofun 1551
SD_A_GLLSISGI,	//ゴルルゴン悦のけぞり２（停止）//gllsisgi 1552
SD_A_GLLYARAR,	//ゴルルゴン失血死//gllyarar 1553
SD_A_GNLHAT_1,	//ゲノラ、クショミ出そう//gnlhat_1 1554
SD_A_GNLHAT_2,	//ゲノラ、クシャミ出る寸前//gnlhat_2 1555
SD_A_GNLHACTI,	//ゲノラ、ハクション//gnlhacti 1556
SD_A_TARAIOTI,	//たらい、アタマに落ちる//taraioti 1557
SD_A_TARAIKRO,	//たらい、床に落ち転がる//taraikro 1558
SD_A_GNLPUNCH,	//ゲノラ、パンチ振り//gnlpunch 1559
SD_A_GNLP_HIT,	//ゲノラ、パンチヒット//gnlp_hit 1560
SD_A_MG_COUTV,	//チビメカゲ、アウト悲鳴//mg_coutv 1561
SD_A_TARAIREC,	//たらい、被爆発揺れ//tarairec 1562
SD_A_V_START4,	//開始地形創造２－２//v_start22 1563
SD_A_TARAIKAT,	//たらい、落ち始める//taraikat 1564
SD_W_RIFLENP1,	//NPCスネークライフルショット//rifle02 1565
SD_A_DAZZLIN2,	//投光器まぶしい２(交互Xbox対策)//dazzling 1566
} ;
