/* HZD -> HZX 関数変更草稿 */

typedef unsigned int HZX_GROUP_ID;

/* ---------------------------------------------------------- */
/*
	トラップ関連
*/

// trap.c

/* トラップ進入判定＆イベント発生 */
void		HZD_EnterTrap( hzd, map, ev )
HZD_HDL		*hzd ;
int		map ;
HZD_EVT		*ev ;

	void HZX_EnterTrap( HZX_GROUP_ID id, HZX_EVT *ev );

/* トラップフラッシュ */
void		HZD_FlashTrap( ctrl )
CONTROL		*ctrl ;

	void HZX_FlashTrap( HZX_GROUP_ID id, HZX_EVT *ev );

/* ビハインドモード */
int		HZD_CheckBehind( HZD_BEHIND **bhs,
				 HZD_HDL *hzd, int map, FVECTOR *pos )

	int HZX_CheckBehind( HZX_GROUP_ID id, HZX_BEHIND **bhds, FVECTOR *pos );

// bind.c

int		HZD_ExecEvent( ev, map, mode )
HZD_EVT		*ev ;
u_int		map ;
u_int		mode ;

	int HZX_ExecEvent( HZX_GROUP_ID id, HZX_EVT *ev, u_int mode );

void		HZD_SetEvent( ev, chara, mov, rot )
HZD_EVT		*ev ;
int		chara ;
FVECTOR		*mov ;
SVECTOR		*rot ;

**	void HZX_SetEvent( HZD_EVT *ev, int chara, FVECTOR *mov, SVECTOR *rot );

void		HZD_SetBind( bnd )
HZD_BND		*bnd ;

**	void HZX_SetBind( HZD_BND *bnd );

void	HZD_InitTrapBind( void )

**	void HZX_InitTrapBind( void );

/* ---------------------------------------------------------- */
/*
	動的壁、床設定
*/

// dynamic.c

/* 動的壁移動 */
void		HZD_MoveDynamicSegment( seg, p1, p2 )
HZD_D_SEGMENT	*seg ;
IVECTOR		*p1, *p2 ;

**	void HZX_MoveDynamicSegment( HZX_D_SEGMENT *seg, IVECTOR *p1, IVECTOR *p2 );

/* 動的壁登録 */
HZD_D_SEGMENT	*HZD_AddDynamicSegment( map, p1, p2, atr )
int		map ;
IVECTOR		*p1, *p2 ;
u_int		atr ;

	void HZX_AddDynamicSegment( HZX_GROUP_ID id, IVECTOR *p1, IVECCTOR *p2, u_int atr );

/* 動的壁削除 */
void		HZD_RemoveDynamicSegment( seg ) 
HZD_D_SEGMENT	*seg ;

**	void HZX_RemoveDynamicSegment( HZX_D_SEGMENT *seg );

/* 動的床移動 */
void		HZD_MoveDynamicFloor( flr, p1, p2, p3, p4 ) 
HZD_D_FLOOR	*flr ;
IVECTOR		*p1, *p2, *p3, *p4 ;

**	void HZX_MoveDynamicFloor( HZX_D_FLOOR	*flr, IVECTOR *p1, IVECTOR *p2
								, IVECTOR *p3, IVECTOR *p4 );

/* 動的床登録 */
HZD_D_FLOOR	*HZD_AddDynamicFloor( map, p1, p2, p3, p4, n, atr )
int		map ;
IVECTOR		*p1, *p2, *p3, *p4 ;
int		n ;
u_int		atr ;

	HZX_D_FLOOR *HZX_AddDynamicFloor( HZX_GROUP_ID id, IVECTOR *p1, IVECTOR *p2
									, IVECTOR *p3, IVECTOR *p4, int n, u_int atr );

/* 動的床削除 */
void		HZD_RemoveDynamicFloor( flr ) 
HZD_D_FLOOR	*flr ;

**	HZX_RemoveDynamicFloor( HZX_D_FLOOR *flr );

/* ---------------------------------------------------------- */
/*
	near
*/

// nearx.c

int	HZD_NearHazardCheckZ( map, from, sphere, chk_flag, seg_flag, r_sphere )
int		map ;
FVECTOR		*from ;
int		sphere ;
int		chk_flag, seg_flag ;
int		r_sphere ;

	int HZX_NearHazardChech( HZX_GROUP_ID id, FVECTOR *from, int sphere
								, int chk_flag, int seg_flag, int r_sphere );

/* 結果を取得 */
void	HZD_GetNearHazard( segs, atrs ) 
HZD_SEG		*segs ;
int		*atrs ;

**	void HZX_GetNearHazard( HZX_SEG *segs, int *atrs );

/* 結果のポインタを取得 */
void		HZD_GetNearHazardPtr( segs, blk )
SVECTOR		**segs ;
HZD_BLOCK	**blk ;

**	void HZX_GetNearHazardPtr( HZX_SEG **segs, HZD_BLOCK **blk );

/* 結果のポインタからＨＺＤ＿ＳＥＧを生成 */
void		HZD_MakeNearHazard( segs, v, blk )
HZD_SEG		*segs ;
SVECTOR		**v ;
HZD_BLOCK	**blk ;

**	void HZX_MakeNearHazard( HZX_SEG *segs, SVECTOR **v, HZX_BLOCK **blk );

/* 近接壁へのベクトルを取得 */
void	HZD_GetNearVector( vect_ptr )
FVECTOR		*vect_ptr ;

**	void HZX_GetNearVector( FVECTOR *vect_ptr );

/* 近接壁への最近点がエッジであるかどうか */
void	HZD_GetIsEdge( ie )
signed char		*ie ;

**	void HZX_GetIsEdge( signed char *ie );

/* 反発ベクトルを取得 */
void	HZD_GetReactVector( react )
FVECTOR	*react ;

**	void HZX_GetReactVector( FVECTOR *react );

/* 結果のアトリビュートを取得 */
void	HZD_GetNearHazardAtr( atrs )
int	*atrs ;

**	void HZX_GetNearHazardAtr( int *atrs );

/* 以前のバージョン用 */
void	HZD_GetNearHazardF( segs )
HZD_SEG	**segs ;

	削除？

/*
	レーダー用
*/
int	*HZD_GetNearBlockID( HZD_GRP *grp, FVECTOR *from, int sphere, int *n_blocks )

	int *HZX_GetNearBlockID( HZX_GROUP_ID id, FVECTOR *from, int sphere, int *n_blocks );

/* ---------------------------------------------------------- */
/*
	online
*/

// online.c

/* オンラインチェック */
int 	HZD_OnlineHazardCheckZ( map, from, to, chk_flag, seg_flag, flr_flag )
int		map ;
FVECTOR		*from, *to ;
int		chk_flag, seg_flag, flr_flag ;

	int 	HZX_OnlineHazardCheck( HZX_GROUP_ID id, FVECTOR *from, FVECTOR *to
				, int check_flag, int seg_flag, int flr_flag );

/* 結果を取得 */
void	HZD_GetOnlineHazard( seg, atr )
HZD_FLR		*seg ;
int		*atr ;

**	void HZX_GetOnlineHazard( HZX_FLR *seg, int *atr );

/* 結果のポインタを取得 */
void	HZD_GetOnlineHazardPtr( v, blk )
SVECTOR		*v ;
HZD_BLOCK	*blk ;

**	void HZX_GetOnlineHazardPtr( SVECTOR *v, HZX_BLOCK *block );

/* ＦＲＯＭ－交差点ベクトルを取得 */
void	HZD_GetOnlineVector( vect_ptr )
FVECTOR		*vect_ptr ;

** void HZX_GetOnlineVector( FVECTOR *vect_ptr );

/* 結果の交差点座標を取得 */
void	HZD_GetOnlinePoint( ptp_ptr )
FVECTOR		*ptp_ptr ;

** void HZX_GetOnlinePoint( FVECTOR *ptp_ptr );

/* 結果のアトリビュート取得 */
int	HZD_GetOnlineHazardAtr( void )

** int HZX_GetOnlineHazardAtr( void );

/* 当たったのが壁か床か */
int	HZD_GetOnlineHazardType( void )

**	int HZD_GetOnlineHazardType( void )

/* 当たった壁のグループ */
int	HZD_GetOnlineHazardGroup( void )

**	int HZD_GetOnlineHazardGroup( void )

/* 以前のバージョン用 */
HZD_SEG		*HZD_GetOnlineHazardF( void )

	削除？

/* ---------------------------------------------------------- */
/*
	床
*/

// levelx.c

/* 床／天井チェック */
int		HZD_LevelHazardCheckZ( map, from, chk_flag, flr_flag )
int		map ;
FVECTOR		*from ;
int		chk_flag, flr_flag ;

	int HZX_LevelHazardCheck( HZX_GROUP_ID id, FVECTOR *from, int chk_flag, int flr_flag );

/* 結果を取得 */
void	HZD_GetLevelHazard( flr, atr )
HZD_FLR		*flr ;
int		*atr ;

**	void HZX_GetLevelHazard( HZD_FLR *flr );

/* 結果のポインタを取得 */
void	HZD_GetLevelHazardPtr( flr, blk )
SVECTOR		**flr ;
HZD_BLOCK	**blk ;

**	void HZX_GetLevelHazardPtr( SVECTOR **flr, HZX_BLOCK **blk );

/* 結果の形状を取得（３ ＯＲ ４） */
void	HZD_GetLevelHazardType( type )
int	*type ;

**	void HZX_GetLevelHazardType( int *type );

/* 結果の高さのみ取得 */
void	HZD_GetLevelHeight( lvl_ptr )
float	*lvl_ptr ;

**	void HZX_GetLevelHeight( float *lvl_ptr );

/* 結果の床高さを取得 */
float	HZD_GetFloorLevel( void )

**	float HZX_GetFloorLevel( void );

/* 結果の床アトリビュートを取得 */
int	HZD_GetLevelAtr( void )

**	int HZX_GetLevelAtr( void )

/* 以前のバージョン用 */
void	HZD_GetLevelHazardF( flrs )
HZD_FLR		**flrs ;

	削除？

/* 傾斜床の高さを調べる２ */
void	HZD_SlopeFloorLevel( h, mov, flr )
float		*h ;
FVECTOR		*mov ;
HZD_FLR		*flr ;

**	HZX_SlopeFloorLevel( float *h, FVECTOR *mov, HZX_FLR *flr );

/*
   現在のブロック
*/
HZD_BLOCK	*HZD_GetInsideBlock( hzd, map, mov )
HZD_HDL	*hzd ;
int	map ;
FVECTOR	*mov ;

**	HZX_BLOCK *HZX_GetInsideBlocK( HZX_GROUP_ID id, FVECTOR *mov );

/* ---------------------------------------------------------- */
/*
	NAVIGATE, ROUTE
*/

// GM_CurrentMapはなるべくつかわない

// inline

static	inline	int	HZD_Address( zone1, zone2 )
int			zone1, zone2 ;

static	inline	int	HZD_Address_M( mapbit, zone1, zone2 )
int		mapbit, zone1, zone2 ;

	extern inline int HZX_Address( HZX_GROUP_ID id, int zone1, int zone2 );

static	inline	int	HZD_ZoneMapBit( mapid )
int			mapid ;

	extern inlne int HZX_ZoneMapBit( map_no )

static	inline	int	HZD_Zone1( address )
int			address ;

**	extern inline int HZX_Zone1( int address );

static	inline	int	HZD_Zone2( address )
int			address ;

**	extern inline int HZX_Zone2( int address );

static	inline	HZX_ZON	*HZD_GetZone( hzd, zone )
HZD_HDL			*hzd ;
int			zone ;

static	inline	HZX_ZON	*HZD_GetZone_M( hzd, zone, mapbit )
HZD_HDL			*hzd ;
int			zone ;
int			mapbit ;

	extern inline HZX_ZON *HZX_GetZone( HZX_GROUP_ID id, int zone );

static	inline HZD_GRP	*HZD_GetGroup( hzd, mapbit )
HZD_HDL		*hzd ;
int	mapbit ;

	extern inline HZX_GRP *HZX_GetGroup( HZX_GROUP_ID id );

static	inline int	HZD_GetLinkGroupID( link_zone )
HZD_ZON	*link_zone ;

	extern inline int HZX_GetLinkGroupID( HZX_ZON *link_zone )

static	inline int	HZD_GetLinkZone( link_zone )
HZD_ZON	*link_zone ;

	extern inline int HZX_GetLinkZone( HZX_ZON *link_zone );

// navigate.c

/* 場所と前フレームのゾーンアドレスから高速に現在のゾーンアドレスを計算する */
/* 1:ゾーンに含まれていれば必ず z1==z2 になるように修正 */
int		HZD_GetAddress( hzd, pos, address )
HZD_HDL		*hzd ;
FVECTOR		*pos ;
int			address ;

int		HZD_GetAddress_M( hzd, pos, address, mapbit )
HZD_HDL		*hzd ;
FVECTOR		*pos ;
int			address ;
int			mapbit ;

	HZX_GetAddress( HZX_GROUP_ID id, FVECTOR *pos, int address );

int		HZD_ReachTo( hzd, addr1, addr2 )
HZD_HDL		*hzd ;
int		addr1 ;
int		addr2 ;

int		HZD_ReachTo_M( hzd, addr1, addr2 )
HZD_HDL		*hzd ;
int		addr1 ;
int		addr2 ;

	HZX_ReachTo( HZX_GROUP_ID id, int addr1, int addr2 );

/* 同じ距離なら先に検索されたゾーンを選択 */
int		HZD_Navigate( hzd, addr1, addr2, pos )
HZD_HDL		*hzd ;
int		addr1 ;
int		addr2 ;
FVECTOR		*pos ;

/* 同じ距離なら後に検索されたゾーンを選択 */
int		HZD_Navigate2( hzd, addr1, addr2, pos )
HZD_HDL		*hzd ;
int		addr1 ;
int		addr2 ;
FVECTOR		*pos ;

int		HZD_Navigate_M( hzd, addr1, addr2, pos )
HZD_HDL		*hzd ;
int		addr1 ;
int		addr2 ;
FVECTOR		*pos ;

	int	HZX_Navigate( HZD_GROUP_ID id, int addr1, int addr2, FVECTOR *pos );

int		HZD_NavigateMapToMap( hzd, addr1, addr2 )
HZD_HDL		*hzd ;
int		addr1 ;
int		addr2 ;

	int HZX_NavigateMapToMap( HZX_GROUP_ID id, int zone1, int zone2 );

int	HZD_LinkZoneAddress ( hzd, zone, mapbit )
HZD_HDL		*hzd ;
int			zone ;
int			mapbit ;

	int HZX_LinkZoneAddress( HZX_GROUP_ID id, int zone );
	
/* 同マップ内で２つのゾーンがどれだけ離れているかを計算する */
/* ちょっと重い */
long HZD_ZoneDistance( HZD_HDL *hzd, long zone_from, long zone_to, int mapbit )

	int HZX_ZoneDistance( HZX_GROUP_ID id, int zone_from, int zone_to );

/* ルート内で開始ゾーンから指定距離を超えたゾーン番号を返す */
/* ちょっと重い */
long HZD_OverDistanceZone( HZD_HDL *hzd, long zone_from, long zone_to,int dis_th, int mapbit )

	int HZX_ZoneDistance( HZX_GROUP_ID id, int zone_from, int zone_to, int dis_th );

/* ルート内で開始ゾーンから画面外の最初のゾーン番号を返す */
/* ちょっと重い */
long HZD_BoundOutZone( HZD_HDL *hzd, long zone_from, long zone_to, int dis_th, int mapbit )

	int HZX_BoundOutZone( HZX_GROUP_ID id, int zone_from, int zone_to, int dis_th );

/* 座標がゾーンに含まれているかチェック */
int	HZD_InsideZone( hzd, pos, zone )
HZD_HDL		*hzd ;
FVECTOR		*pos ;
int		zone ;

int	HZD_InsideZone_M( hzd, pos, zone, mapbit )
HZD_HDL		*hzd ;
FVECTOR		*pos ;
int		zone ;
int		mapbit ;

	int HZX_InsideZone( HZX_GROUP_ID id, FVECTOR *pos, int zone );

/* 隣接したゾーン */
int	HZD_NearZones( hzd, zone, near )
HZD_HDL		*hzd ;
int			zone ;
int			*near ;

int	HZD_NearZones_M( hzd, zone, near, mapbit )
HZD_HDL		*hzd ;
int			zone ;
int			*near ;
int			mapbit ;

	int HZX_NearZones( HZX_GROUP_ID id, int zone, int *near );

	/*
		目標のゾーンから遠くなるために
		次に行くべきゾーンを計算する
	*/
int	HZD_FarZoneNavigate( hzd, zone1, zone2, rout, mapbit )
HZD_HDL		*hzd ;		/* 当たりハンドラ	*/
int		zone1 ;		/* 現在のゾーン		*/
int		zone2 ;		/* 目標のゾーン		*/
int		*rout ;		/* ルート数	*/
int		mapbit ;

	int HZX_FarZoneNavigate( HZX_GROUP_ID id, int zone1, int zone2, int *root );

	/*
		目標のゾーンまでのルート数を返す
	*/
int	HZD_GetRoute( hzd, fromadd, toadd )
HZD_HDL		*hzd ;		/* 当たりハンドラ	*/
int			fromadd ;	/* 現在のゾーン		*/
int			toadd ;		/* 目標のゾーン		*/

int	HZD_GetRoute_M( hzd, zone1, zone2, mapbit )
HZD_HDL		*hzd ;	/* 当たりハンドラ	*/
int		zone1 ;		/* 現在のゾーン		*/
int		zone2 ;		/* 目標のゾーン		*/
int		mapbit ;

	int HZX_GetRoute( HZX_GROUP_ID id, int zone1, int zone2 );

/* 現在の座標がどのマップに属するかを
   ゾーンから検索する */
int	HZD_GetMapIDbyZone( pos, map_id, zone_num )
FVECTOR		*pos ;
int		map_id ;
int		*zone_num ;

	int HZX_GetMapIDbyZone( HZX_GROUP_ID id, FVECTOR *pos, int *zone_num );

/* 位置からグループ番号とゾーン番号を返す */
void	HZD_Pos2Zone( pos, g, z )
FVECTOR		*pos ;
int		*g, *z ;

	void HZX_Pos2Zone( FVECTOR *pos, int *group, int *zone );

/* キャラクタのグループ変更関数 */
void	HZD_CheckChangeGroup( ctrl, addr )
CONTROL		*ctrl ;
int		*addr ;

	void HZX_CheckChangeGroup( CONTROL *ctrl, int *addr );			// 要検討（->GM?)

// route.c

void		HZD_MakeRoute( grp, routes )
HZD_GRP		*grp ;
u_char		*routes ;

**	void HZX_MakeRoute( HZX_GRP *grp, u_char *routes );

/* ---------------------------------------------------------- */
/*
	SETUP
*/

// hzdd.c

void HZD_ResetSystem( void )
void HZD_ResetMemory( void )
void	HZD_Initialize( void ) 
void HZD_StartDaemon( void )

/* カレントのＨＺＤを取得 */
inline	HZD_HDL	*HZD_GetCurrentHzd( map_id )
/* 被検索グループリセット */
inline	void	HZD_ResetCurrentGroup( void )
/* 被検索グループに追加 */
inline	void	HZD_AddCurrentGroup( group )
int		group ;
/* 被検索グループから削除 */
inline	void	HZD_DeleteCurrentGroup( group )
int		group ;

// loadhzd.c

HZD_HDL		*HZD_GetHandler( id )
int		id ;

/* 当たりハンドラーの廃棄 */
void		HZD_FreeHandler( hdl )
HZD_HDL		*hdl ;

/* 初期化 */
void	HZD_InitHzdCache( void )

/* グループセッティング */
void		HZD_SetupGroup( hzd )
HZD_HDL		*hzd ;

/* ローダ */
void		HZD_LoadHzd( hzx, id )
HZD_DEF		*hzx ;
int		id ;

/* ---------------------------------------------------------- */
/*
	UTIL
*/

// vector.c

int		HZD_HazardReaction( vects, n_vects, range, react )
FVECTOR		*vects ;
int		n_vects ;
int		range ;
FVECTOR		*react ;
