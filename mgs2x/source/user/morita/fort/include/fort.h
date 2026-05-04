/*
   fort.h
   フォーチュン/ヘッダファイル

   1999/12/18 T.Morita
   $Id: fort.h,v 1.1.1.3 2002/11/19 11:46:16 Yoshizawa1 Exp $			
*/
#ifndef __FRT_H__
#define __FRT_H__

#include "../../include/util.h"

/* モーション用 */
#include "fort.mh"
/* SE用 */
#include "fort_se.h"

/*狩野さんライブラリ*/
#include "../../../kano/attachment/attachments.h"   /* 装備品用 */

/*柴田さんライブラリ*/
#include "../../../shibata/t_headmark/headmark.h"

/* プレイヤー情報用 */
#include "../../../sonoyama/raiden/pl_work.h"
#define	Work_Ply	PlayerWork

#include "fort_flag.h"

#if DEBUG_MODE
/* フォーチュン プリント消去 */
//#define printf FRT_DummyFunc
#endif

#define FRT_NAME         GV_StrCode( "フォーチュン" )
#define FRT_S_MODEL_NAME GV_StrCode( "for_def_sh_mt" )
#define FRT_M_MODEL_NAME GV_StrCode( "for_def_mh_mt" )
#define FRT_MOTION_NAME  GV_StrCode( "fort" )


#define FRT_MAX_FLOAT 60000000000.0f


/* 思考時間 や その他（思考に関するもの） */
#define FRT_STILL_THINKTIME 4    /* 攻撃のワンクール（この回数だけ攻撃して移動する） */

/*ダメージ関係*/
#define FRT_DAMAGE_DFLT     32.0f
#define FRT_MAX_VITALITY    25

#define FRT_ATTACK_RAD      1400.0f
#define FRT_WATCH_RAD       1500.0f 

/* 壁当たりチェック半径 や 隠れ関係 */
#define FRT_CHECK_SPHERE   500
#define FRT_NORMAL_SPHERE  450

/*移動完了距離 また 移行距離 */
#define FRT_WALK_DIST    ( 100.0f* 100.0f)
#define FRT_STOP_DIST    1000.0f 

/* セリフの間の秒数 */
#define FRT_VOX_INTREVAL   8

/* ミスショットする 範囲 */
#define FRT_MISS_SHOT_RAD  500.0f
/* マガジンなどの音がする 範囲 */
#define FRT_HEARD_RAD     (4000.0f*4000.0f)
/*  範囲 */
#define FRT_DEAD_ZONE_Z -3220.0f

/* ターゲット関連 */
#define FRT_HIT_TRG_FLG  (TARGET_OFFENSE|TARGET_SEEK|TARGET_ONLINE|TARGET_ONLINE_MIN|TARGET_CHILD)
#define FRT_MAIN_TRG_FLG (TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|TARGET_ONLINE)

#define FRT_N_STREAM      6 /* ストリーミング */
 

/* オブジェクト部位フラグ */
#define FRT_BODY_UPPER     ( (1<<HUMAN21_ONAKA)| (1<<HUMAN21_MUNE)| \
			      (1<<HUMAN21_KUBI)| (1<<HUMAN21_ATAMA)| \
			      (1<<HUMAN21_MIGI_KATA  )|(1<<HUMAN21_MIGI_UDE1  )| \
			      (1<<HUMAN21_MIGI_UDE2  )|(1<<HUMAN21_MIGI_TE    )| \
			      (1<<HUMAN21_HIDARI_KATA)|(1<<HUMAN21_HIDARI_UDE1)| \
			      (1<<HUMAN21_HIDARI_UDE2)|(1<<HUMAN21_HIDARI_TE  ) )
#define FRT_BODY_RIGHT_ARM ( (1<<HUMAN21_MIGI_KATA  )|(1<<HUMAN21_MIGI_UDE1  )| \
			      (1<<HUMAN21_MIGI_UDE2  )|(1<<HUMAN21_MIGI_TE    ) )
#define FRT_BODY_BOTH_ARMS ( (1<<HUMAN21_MIGI_KATA  )|(1<<HUMAN21_MIGI_UDE1  )| \
			      (1<<HUMAN21_MIGI_UDE2  )|(1<<HUMAN21_MIGI_TE    )| \
			      (1<<HUMAN21_HIDARI_KATA)|(1<<HUMAN21_HIDARI_UDE1)| \
			      (1<<HUMAN21_HIDARI_UDE2)|(1<<HUMAN21_HIDARI_TE  ) )
#define FRT_BODY_LOWER     ( (1<<HUMAN21_KOSHI)| \
			      (1<<HUMAN21_MIGI_ASHI1   )|(1<<HUMAN21_MIGI_ASHI2    )| \
			      (1<<HUMAN21_MIGI_KAKATO  )|(1<<HUMAN21_MIGI_TSUMASAKI)| \
			      (1<<HUMAN21_HIDARI_ASHI1 )|(1<<HUMAN21_HIDARI_ASHI2  )| \
			      (1<<HUMAN21_HIDARI_KAKATO)|(1<<HUMAN21_HIDARI_TSUMASAKI) )
#define FRT_BODY_ALL         (FRT_BODY_UPPER | FRT_BODY_LOWER)


/* LNR弾数 */
#define FRT_WEAPON_BULLET  30


/* プレイヤー情報 */
#define FRT_PLY_VITALITY   (GM_Vitality        )
#define FRT_PLY_CAMERAPOS  (GM_PlayerSubjectCamera[0]->position)
#define FRT_PLY_EQUATESHOT() (work->ply_amo_m9 = GM_WeaponNum(WP_m92  ), \
			      work->ply_amo_us = GM_WeaponNum(WP_Usp  ), \
			      work->ply_amo_so = GM_WeaponNum(WP_Socom), \
			      work->ply_amo_ps = GM_WeaponNum(WP_Psg1) , \
			      work->ply_amo_fa = GM_WeaponNum(WP_Famas))
#define FRT_PLY_SHOOT()     (GM_WeaponNum(WP_m92  ) < work->ply_amo_m9 || \
			     GM_WeaponNum(WP_Usp  ) < work->ply_amo_us || \
			     GM_WeaponNum(WP_Socom) < work->ply_amo_so || \
			     GM_WeaponNum(WP_Psg1 ) < work->ply_amo_ps || \
			     GM_WeaponNum(WP_Famas) < work->ply_amo_fa)
#define FRT_PLY_PLAYERPOS  (GM_PlayerControl->mov)

#define FRT_RESET_PLY_POSITION  -1
#define FRT_GET_PLY_CONTROL     21
#define FRT_GET_PLY_OFFSET      22
#define FRT_PLY_MAX_ENTRIES     23

/* マクロ関数 */
#define BODYWORLD(a,b) ((a)->objs->objs[(b)].world)
#define BODYPOS(a,b)   ((FVECTOR*)&((a)->objs->objs[(b)].world.m[W]))
#define N_MOTION(l) (body.m_ctrl->mt3_ctrl[(l)].motion_num)
#define ChangeFlag(_work,_set,_reset) ((_work)->flag &= ~(_reset),(_work)->flag |= (_set))


/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


/* 型定義 */
typedef	struct work_t Work ;
typedef struct part_t PART ;
typedef enum   flag_t FLAG ;
typedef union  args_t ARGS ;

struct part_t
{
    FMATRIX *world   ; /* 間接のワールド（DG_OBJのworld） */

    Work    *work    ; /* コールバック用のワークへのポインタ */
    int      part_id ; /* 間接番号(BODY21_?????) */
} ;

struct work_t
{
    GV_ACT	 actor     ;

    /* システム用データ */
    CONTROL	 control   ;	/* 移動制御 */
    OBJECT	 body      ;	/* モデル   */
    FMATRIX	 lights[2] ;	/* ライト   */
    RADAR_CTRL   radar     ;    /* レーダー */
    GM_GageSet	 gage      ;    /* ライフゲージ */

    HOMING_TRG   homing    ;              /* ホーミング部位ターゲット(現在 BODY21_ATAMA) */
    TARGET       target    ; /* 防御 部位ターゲット                      */
    POWER_TARGET power     ;
    TARGET       seeker    ; /* 当たっている稼働かの検知 */
    //PART         parts [FRT_N_PARTS  ] ;/* 部位情報(ダメージ量やモーション番号など)*/
    int          name      ;              /* メッセージ受取用                         */
    int          procs [FRT_N_PROCS ] ;   /* プロックリスト                           */
    int          str_id[FRT_N_STREAM] ;   /* ファイルストリーム用                     */
    int          str_hdl   ;              /* ストリームハンドラー  */
    int          vib_time  ;              /* 振動センサー用 GV_Time */

    FVECTOR      pos_adj   ;              /* 位置のアジャスト値(フレーム毎加算)       */

    FVECTOR      head      ; /* 間接のアジャスト値(Eular角/ラジアン) */
    FVECTOR      arm_l     ; /* 間接のアジャスト値(Eular角/ラジアン) */
    FVECTOR      arm_r     ; /* 間接のアジャスト値(Eular角/ラジアン) */
    FVECTOR      breast    ; /* 間接のアジャスト値(Eular角/ラジアン) */

    /* フォーチュン 専用 パラメータ */
    FLAG         flag      ;    /* フォーチュンのステータスフラグ */
    u_short      act_flg   ;    /* 特別行動フラグ */
    u_int        act_phase ;                 /* ゲームフェーズ */
    void      (**act)( Work * ) ;            /* 現在の行動   */
    u_char       act_stop  ;                 /* 全てアクトを通らない残りフレーム数（全機能停止） */

    char         stll_mtn  ;  	/* 静止モーション */
    char         stll_tim  ;  	/* 静止モーション 思考時間 */
    short        wait_sec  ;  	/* 静止タイム用 */
    short        vitality  ;    /* 命 */

    short        weap_blt  ;  	/* 銃(LNR)の残弾  */
    short        weap_amo  ;    /* LNRのマガジン  */
    FVECTOR      weap_lnr  ;  	/* 銃    (LNR)の引金  */
    FVECTOR     *trgt_eye  ;    /* 視線先 */
    FVECTOR     *trgt_aim  ;    /* 狙い先 */
    FVECTOR      trgt_aim_pos ; /* 狙い先コントロール用 */
    FVECTOR      trgt_mis_pos ; /* 狙い先コントロール用(MissShot用) */
    int          trgt_typ  ;
    FVECTOR      trgt_hid  ;    /* 隠れているハイド */
    FVECTOR      trgt_frc  ;    /* 移動 強制移動先 */
    FVECTOR      trgt_nse  ;    /* 音の発振源 */
    FVECTOR     *trgt_pos  ;    /* 移動先 */
    short        shot_cnt  ;    /* フォーチュンがフェーズで撃った弾数 */
    short        shot_tim  ;    /* フォーチュンが狙いを止めるまでの時間 */
    short        lgt_on    ;    /* 撃った後にライトがオンになるフレーム数 */
    short        voice     ;    /* 声のSE番号 */
    short        voice_cnt ;    /* 声のSEカウント */
    short        voice_vox ;    /* 声のSTREAM番号 */
    short        voice_tim ;    /* 声が出るまでのフレーム時間 */
    short        drum_tim  ;
    short        head_time ;    /* ヘッドマークを出すタイミングを計るため */
    int          head_mark ;    /* ヘッドマークを出す */

    /* プレイヤー情報 */
    short        ply_time   ;    /* プレイ時間( one sec ) */
    short        ply_tic    ;    /* プレイ時間( less than one sec ) */
    u_short      ply_hide   ;    /* 隠れている (1[flg]:1[flg]:2[expose]:6[place]:6[time]) */
    u_short      ply_stop   ;    /* プレイヤーが止まっている   */
    short        ply_aim_cnt;    /* プレイヤーがフォーチュンに撃った弾数 */
    short        ply_amo_m9 ;    /* 武器現在のM9   残弾 */
    short        ply_amo_us ;    /* 武器現在のUSP  残弾 */
    short        ply_amo_so ;    /* 武器現在のSocom残弾 */
    short        ply_amo_ps ;    /* 武器現在のPSG1 残弾 */
    short        ply_amo_fa ;    /* 武器現在のFamas残弾 */
    FVECTOR      ply_prev   ;    /* ５frm前の位置                    */
    FVECTOR      ply_shoot  ;    /* プレイヤーが撃った武器（銃系  ） */
    FVECTOR      ply_throw  ;    /* プレイヤーが投げた武器（ボム系） */
    GM_BOMB     *ply_grenade;    /* プレイヤーのいる大まかな場所 */

    /* その他の情報 */
#if DEBUG_MODE
    /* DEBUG用 リスタートの初期化用 */
    FVECTOR      init_pos ;
    SVECTOR      init_rot ;
#endif

} ;


/*inline functions*/
 static inline float VectorSquare( FVECTOR *a, FVECTOR *b )
{
    return (a->vx-b->vx)*(a->vx-b->vx) + (a->vz-b->vz)*(a->vz-b->vz) ;
}
static inline float BackFaceCullingXZ( FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    return (v1->vx - v2->vx)*(v3->vz - v2->vz) - (v1->vz - v2->vz)*(v3->vx - v2->vx) ;
}

/* external variable */
extern CONTROL  *GM_PlayerControl  ;
extern OBJECT   *GM_PlayerBody     ;
extern OBJECT   *GM_PlayerArmBody  ;
extern FVECTOR   GM_PlayerPosition ;
extern Work_Ply *GM_PlayerWork     ;

extern GM_CameraSet *GM_PlayerSubjectCamera[ 4 ] ;	/* プレイヤー主観カメラ */


/* Prototypes */
/* fort_ini.c */
extern int  FRT_InitControl( Work *work, int name, int where ) ;
extern int  FRT_InitObject( Work *work ) ;
extern int  FRT_InitParam( Work *work, int name ) ;
extern int  FRT_GetOptionValue( Work *work, int name, int where ) ;
extern void FRT_InitTarget( Work *work, int where ) ;
extern void FRT_InitVitalityGage( Work *work ) ;
extern void FRT_InitMessages( Work *work ) ;
extern void FRT_InitHomingTarget( Work *work ) ;
extern void FRT_InitRadar( Work *work ) ;
extern void FRT_LaunchExternalChara( Work *work, int name, int where ) ;

/* fort_act.c */
extern void FRT_PreAction( Work *work ) ;
extern void FRT_PostAction( Work *work ) ;
extern void FRT_Action( Work *work ) ;

extern void FRT_ActUnrecogFireP( Work *work ) ;
extern void FRT_ActFireP( Work *work ) ;

extern void FRT_ActStillStart( Work *work ) ;
extern void FRT_ActThink( Work *work ) ;


/* fort_lst.h */
extern void (*FRT_ActionDefault0[])( Work * ) ;
extern void (*FRT_ActionDefault1[])( Work * ) ;
extern void (*FRT_ActionAbort[])( Work * ) ;
extern void (**FRT_ActListWait[])( Work * ) ;
extern void (*FRT_ActionReset[])( Work * ) ;
extern void (*FRT_ActionStopStill[])( Work * ) ;


/* fort_clb.c */
extern void FRT_HitTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void FRT_MainTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;


/* fort_dsp.c */
extern void FRT_Display( Work *work ) ;


/* fort_rcg.c */
extern void     FRT_RecogStartStream( Work *work ) ;
extern void     FRT_RecogStopStream( Work *work ) ;
extern FVECTOR *FRT_RecogAimPos( Work *work ) ;
extern FVECTOR *FRT_MiscPlayerPos( int joint ) ;
extern FVECTOR *FRT_MiscPlayerPosNoHide( int joint ) ;
extern void     FRT_Recognition( Work *work ) ;

extern int      FRT_SearchHidePlace( Work *work, int stll_mtn ) ;
extern void     FRT_RecogExecProc( Work *work, int id ) ;


/* fort_msg.c */
extern void FRT_Message( Work *work ) ;

extern void FRT_SendLightOffMessage( Work *work ) ;
extern void FRT_SendLightOnMessage( Work *work ) ;

extern void FRT_SendHoloMessage( int msg ) ;               /* misc/fort_hol.c へのメッセージ */
extern void FRT_SendHoloGetFlagAddrMessage( int **flag ) ; /* misc/fort_hol.c へのメッセージ */
extern void FRT_SendGetNearBoxMessage( FVECTOR *from, FVECTOR *to, FVECTOR *r ) ;
extern void FRT_SendWallScarRandomSEMessage() ;            /* morita/wall_scar/ へのメッセージ */
extern void FRT_SendSpotLightGetFlagAddrMessage( int **flag ) ; /* misc/fort_lgt.c へのメッセージ */
extern void FRT_SendDebugDumpMessage() ; /* デバッグ状態を表示する */

/* fort_aim.c */
extern void FRT_AIM_InitAimSpot() ;
extern int  FRT_AIM_GetNumberOfPool() ;
extern int  FRT_AIM_GetAimSpot( FVECTOR *aim_spot,
				u_int phase, int type,
				FVECTOR *src, FVECTOR *dst ) ;
extern int  FRT_AIM_GetAimmingObjects( u_int phase, int type, FVECTOR *src ) ;
extern int  FRT_AIM_VerifyAimSpot( void *aim_id, FVECTOR *pos ) ;
extern void FRT_AIM_CheckAimSpot( Work *work ) ;
extern void FRT_AIM_DebugPrint() ;


extern int  FRT_OBJ_ActCheckHideFromToByID( u_int id, FVECTOR *from, FVECTOR *to ) ;


/* fort_com.c */
extern void FRT_InitCommandWork( void *ptr ) ;
extern void FRT_FreeCommandWork() ;


#ifdef DEBUG_MODE
/* fort_dbg.c */
extern void FRT_InitDebugWork( void *ptr ) ;
extern void FRT_FreeDebugWork() ;
extern void FRT_Debug( Work *work ) ;
extern void FRT_DummyFunc( char *fmt, ... ) ;
extern void FRT_InitDbgConfiguration() ;
extern int  FRT_DbgOffense ;
#endif


/* external CHARA */
extern void *NewFortWeaponLNR( int where, DG_OBJS *body,
			       FVECTOR *trig, short *bul, int damage ) ;  /* weapon/fort_lnr.c */
extern void *NewFortWeaponLNRAMO( FMATRIX *r, FMATRIX *l, short *flag ) ; /* weapon/fort_amo.c */
extern void *NewFortWeaponBarriar( int mode, FVECTOR *fort ) ;            /* weapon/fort_bar.c */

extern void *AN_HeadMark( FMATRIX *world, int mode ) ;                            /* tanaka/effect/  */
extern void *NewPiyori( FMATRIX *world ) ;                                       /* tanaka/piyori/  */
extern void *AN_Test_Eye2( FVECTOR *mov, int size ) ;                             /* for test                   */
extern void *NewBodyShadow( DG_OBJS *o ) ;                                       /* okajima/effect/body_sdw.c  */
extern void *NewFootSplash( OBJECT *body, CONTROL *c ) ;                         /* okajima/effect/ft_splash.c */
extern void *NewBodySplash2( DG_OBJS *o, CONTROL *c, int model_id, int *flag ) ; /* okajima/effect/body_sph.c  */

extern void *NewFortEquip( int name, OBJECT *body, int flag, int wpname ) ;
/* morita/demo_fort/fort_equip.c  */
extern void *NewShadow( DG_OBJ *lfoot, DG_OBJ *rfoot, CONTROL *c, FMATRIX *lgt, int *flag );
/* morita/shadow/foot_shadow.c  */
extern void *NewDropShadow( OBJECT *body, CONTROL *c, FMATRIX *lgt, int *flag ) ;
/* morita/shadow/drop_shadow.c  */
extern void *NewFitActionDoll( DG_OBJS *objs, int model_id, int flag, int param ) ;
/* takabe/effect1/fit_doll.c*/

extern void *NewSplashParts_Demo( FVECTOR *center, SVECTOR *rot, float intense ) ;
/* okajima/demo_effect/d_splash_parts.c*/
extern void  *NewControl_Headmark2( FMATRIX *world, int *type, TARGET *trg, CONTROL *cntrl ) ;
/* shibata/t_headmark/control_headmark2.c */

extern void *NewPadVibration( char *scr, int type ) ;

/* sonoyama/etc/gaged.c prototype-set */
extern void GM_AppendGageSet( GM_GageSet *gs ) ;
extern void GM_RemoveGageSet( GM_GageSet *gs ) ;
extern void GM_SetGageColor( GM_GageSet *gs,
			     u_char r1, u_char g1, u_char b1,
			     u_char r2, u_char g2, u_char b2,
			     u_char r3, u_char g3, u_char b3,
			     u_char r4, u_char g4, u_char b4 ) ;
extern void GM_VisibleGage( GM_GageSet *gs ) ;
extern void GM_InvisibleGage( GM_GageSet *gs ) ;
extern void GM_InitGageSet( GM_GageSet *gs, char *name,
			    int x, int w, int h, int value,
			    int max, int min, int delay, int level ) ;

/* kano/attachment/attachments.c */
extern void *NewAttachments_called( OBJECT *target,
				    const ATTACHMENT_ARGUMENT  *args1, int size1,
				    const ATTACHMENT_ARGUMENT2 *args2, int size2,
				    const ATTACHMENT_ARGUMENT3 *args3, int size3 ) ;
/* okajima/demo_effect/d_fog_set.c */
extern void *NewFogSet_Demo( int col_r, int col_g, int col_b,
			     float near, float far,
			     int time ) ;

#if DEBUG_MODE
//for debug
#include "../../include/util.h"
#endif



/*Reference Global Variable*/

#ifdef __MAIN_FILE__

FVECTOR FRT_Phase_00Pos = { 0.0f, -43000.0f, -7000.0f, 0.0f } ;
FVECTOR FRT_Phase_0cPos = { 0.0f, -43000.0f, -5000.0f, 0.0f } ;

FVECTOR FRT_LeftPos   = { -1000.0f, -43000.0f, -7000.0f, 0.0f } ;
FVECTOR FRT_RightPos  = {  1000.0f, -43000.0f, -7000.0f, 0.0f } ;
FVECTOR FRT_HidingPos = {     0.0f, -43000.0f, -5000.0f, 0.0f } ;

#else   /* __MAIN_FILE__ */

extern FVECTOR FRT_Phase_00Pos ;
extern FVECTOR FRT_Phase_0cPos ;

extern FVECTOR FRT_LeftPos   ;
extern FVECTOR FRT_RightPos  ;
extern FVECTOR FRT_HidingPos ;
extern int     FRT_AimID        ;
extern int     FRT_SmokeClouded ;  /* fort_var.c */
extern CVECTOR FRT_FogColor ;      /* fort_var.c */
extern float   FRT_FogNear  ;      /* fort_var.c */
extern float   FRT_FogFar   ;      /* fort_var.c */
extern int     FRT_BulletEnable ;  /* fort_var.c */
extern int     FRT_LastCieling ;   /* fort_var.c */

extern int FRT_GasExplode ;        /* fort_var.c */
extern int FRT_GasFire    ;        /* fort_var.c */

extern CVECTOR OK_FogColorBase;
extern float   OK_FogNear;
extern float   OK_FogFar;

#endif  /* __MAIN_FILE__ */

#endif  /* __FRT_H__ */


