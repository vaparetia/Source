/*
	モーション関連ヘッダ
*/


#ifndef __MOTION__
#define __MOTION__

#ifdef __cplusplus
extern "C" {
#endif

/* ===== ＭＴＮ関連 ===== */

/* 新無圧縮モーションデータ */
/*
	補足：モーションデータに入っている実際のモーション数は
	　　　移動量を算出するため、１フレーム分多くデータが入っている。
	　　　motion_lengthには有効フレーム数が入っているので注意。
	　　　オフセットが０の場合はそのデータがないことを示す。
	　　　また、各オフセットは１６バイト境界になっている必要がある。
*/
/* セーブロード用ヘッダ */
typedef struct {
	int			flags ;			/* 各種フラグ */
	int			motion_length ;	/* モーションの有効フレーム数 */
	int			motion_joints ;	/* モーションの関節数 */
	int			motion_tick ;	/* ベースクロック値（１／３００単位） */
	int			move_size ;		/* 移動量のサイズ((motion_length+1)*16) */
	int			rots_size ;		/* 関節回転データのサイズ(((motion_length+1)*motion_joints)*16) */
	int			trans_size ;	/* 関節移動量データへのサイズ(((motion_length+1)*motion_joints)*16) */
	int			minfo_size ;	/* 移動キー情報のサイズ((motion_length+1)*4) */
	int			jinfo_size ;	/* 関節キー情報のサイズ((motion_length+1)*motion_joints)*16) */
	int			sound_size ;	/* サウンド情報のサイズ（将来の拡張用） */
	int			anime_size ;	/* アニメーションのサイズ（将来の拡張用） */
	int			pad[4] ;
	int			error_angle ;	/* 圧縮時のエラー許容角度（ＰＳ固定小数点角度） */
} MTN_FILE_HEADER ;


typedef struct {
	unsigned int	key:1 ;
	unsigned int	fix:1 ;
	unsigned int	pad:30 ;
} KEY_INFO ;

typedef struct {
	MTN_FILE_HEADER	header ;	/* ヘッダー */
	FVECTOR		*move ;			/* 移動量へのポインタ(motion_length+1) */
	FVECTOR		*rots ;			/* 関節回転データへのポインタ((motion_length+1)*motion_joints) */
	FVECTOR		*trans ;		/* 関節移動量データへのポインタ((motion_length+1)*motion_joints) */
	KEY_INFO	*minfo ;		/* 移動キー情報へのポインタ(motion_length+1) */
	KEY_INFO	*jinfo ;		/* 関節キー情報へのポインタ((motion_length+1)*motion_joints) */
	void		*sound ;		/* サウンド情報へのポインタ（将来の拡張用） */
	void		*anime ;		/* アニメーションへのポインタ（将来の拡張用） */
	int			pad ;
} MTN_MOTION ;

enum {
	MTN_FLAG_LOOP = 0x0001,		/* ループモーションフラグ */
	MTN_FLAG_TURN = 0x0002,		/* ターンモーションフラグ */
	MTN_FLAG_ROOT = 0x0004,		/* 移動軌跡有効フラグ */
	MTN_FLAG_FIX  = 0x0008,		/* 関節固定有効フラグ */
	MTN_FLAG_TYPE_MASK = 0x00ff0000,		/* タイプ識別マスク */
	MTN_FLAG_TYPE_01   = 0x00010000,		/* 上半身下半身分離モーションフラグ */
};




/* ===== ＭＴ３関連 ===== */

typedef struct {
	long	name_id ;			/* ファイル名ＩＤ */
	long	flag ;				/* 各種フラグ情報 */
	long	motion_base_tick ;	/* モーションのフレームベースカウント（１／３００秒単位） */
	long	motion_length ;		/* モーションフレーム数 */
	short	*archives_data ;	/* アーカイブデータへのポイント */
	long	archives_size ;		/* アーカイブデータのサイズ（ハーフワード単位） */
	long	rot_units ;			/* 可動関節ユニット数 */
	long	root_offset ;		/* リザーブ */
	long	move_offset ;		/* 移動量データのオフセット */
	long	turn_offset ;		/* 方向転換情報のオフセット */
	long	fix_offset ;		/* 関節固定情報のオフセット */
	long	rots_offset[0] ;	/* 関節回転情報のオフセット */
} MT3_FILE_HEADER ;

#define MT3_FLAG_MOVE_PART	(0x00000001)
#define MT3_FLAG_ROT_PART	(0x00000002)
#define MT3_FLAG_FIX_PART	(0x00000004)
#define MT3_FLAG_TURN_PART	(0x00000008)
#define MT3_FLAG_TURN_FLAG	(0x00000008)
#define MT3_FLAG_ROOT_PART	(0x00000010)
#define MT3_FLAG_TYPE_MASK	(0x00ff0000)	/* タイプのマスク */
#define MT3_FLAG_TYPE_01	(0x00010000)	/* 上半身下半身分離型モーションデータ */


/* ===== 圧縮ツールプレビュー用関連 ===== */

typedef struct {
	int			now_frame ;
	MTN_MOTION	*Motion ;
	MTN_MOTION	*EditMotion ;
} MOTION_INFO ;



/* ===== プロトタイプ宣言 ===== */

extern MTN_MOTION* MT_LoadMt1( char *filename );
extern MTN_MOTION* MTN_DuplicateMotion( MTN_MOTION *org_motion );
extern void MTN_CopyMotion( MTN_MOTION *dest, MTN_MOTION *org_motion );
extern MTN_MOTION* MTN_AllocMotion( int n_joints, int frame );
extern void MTN_FreeMotion( MTN_MOTION *motion );
extern MTN_MOTION* MTN_OpenMTNFile( char *filename );
extern void MTN_SaveMTNFile( char *filename, MTN_MOTION *motion );
extern void MTN_GetMotionData( MTN_MOTION *motion, int frame, FVECTOR *mov, FVECTOR *trans, FVECTOR *rots );
extern void MTN_SetMotionData( MTN_MOTION *motion, int frame, FVECTOR *mov, FVECTOR *trans, FVECTOR *rots );
//extern void MTN_SetMotionData( FVECTOR *pos, FVECTOR *rots, MTN_MOTION *motion, int frame );
//extern void MTN_SetMotionData2( FVECTOR *pos, FVECTOR *rots, MTN_MOTION *motion, void *_mdl, int *merge_flag, int frame );
/* 絶対回転クォータニオンでの誤差を求める */
extern void MTN_GetMotionError( float *errors, MTN_MOTION *in_motion, MTN_MOTION *out_motion, void *_mdl, int *_merge_flag );
/* モーションデータを最終的な相対クォータニオンとトランスレーションに変換する */
void MTN_GetMotionFromMtn( FMATRIX *world, FVECTOR *trans, FVECTOR *qrots, void *object_prt, MTN_MOTION *motion, int frame, int *merge_flag );


extern void MTN_Mt3Encode( MTN_MOTION *out_motion, MTN_MOTION *in_motion, int flag, float error_angle );
extern void MT3_Mt3Save( MTN_MOTION *motion, char *filename, int flag );

extern void MTN_SetMotionDataEx( FMATRIX *mats_buffer, void *model, MTN_MOTION *motion, int frame, int *merge_flag );

/* モーションデータをＹ軸回転で補正する */
extern void MTN_Tool_TurnMotion( MTN_MOTION *motion, float agnle );


#ifdef __cplusplus
}
#endif


#endif

