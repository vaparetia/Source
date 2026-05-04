

/*******/
/*sigeno 監視カメラ系 視界情報*/
typedef	struct{
	FVECTOR *eyepos ;	/*開始点*/
	FVECTOR *trgpos ;	/*目標点*/
	SVECTOR rot ;		/*視界方向*/
	SVECTOR range ;		/*視界範囲*/
	float length ; 		/*視界距離*/
	short	alert_time;	//警戒時間	
	short	discv_time;	//発見するまでの時間
	/*白びっくり発見用*/
	short	alert2 ;
	short	discv2 ;
	int		accident_uniq_id ;
	FVECTOR	accident_pos ;
	HZX_GROUP_ID	accident_hzx_id ;
/*首振りＳＥ用*/
	int		se_tick;

#ifdef DEBUG_MODE
	int debug_st;	/*デバッグ用*/
#endif
} EYEPARAM ;













