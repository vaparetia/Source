/*
	libdg.x
	ディスプレイジェネレータライブラリプロトタイプ宣言

	1999/07/07 K.Takabe
	$Id: libdg.x,v 1.1.1.3 2002/11/19 11:42:10 Yoshizawa1 Exp $

*/



	/*	bound.c	*/
extern	void	DG_BoundChanl( DG_CHANL *, int ) ;
extern	int	DG_BoundObj( DG_OBJ * ) ;

	/*	chanl.c	*/
extern	void DG_InitChanlSystem( int ) ;
extern	void DG_ResetChanlSystem( int ) ;
extern	void DG_DrawChanlSystem( int ) ;
extern	void DG_ClearChanlSystem( int ) ;
extern	void DG_UnDrawChanlSystem( int which );
extern	void DG_SortChanlSystem( int ) ;
//extern	void DG_SetDrawEnv( int, DRAWENV * ) ;
extern	void DG_SetDrawEnv( DG_CHANL *cp, int x, int y, int w, int h );
extern	void DG_ChangeDrawLimit( DG_CHANL *cp, int x1, int y1, int x2, int y2 );
extern	int	DG_QueueObjs( DG_OBJS * ) ;
extern	void DG_DequeueObjs( DG_OBJS * ) ;
extern	DG_OBJS* DG_SearchQueueObjs( DG_OBJS *objs );
extern	int DG_QueuePrim( DG_PRIM * ) ;
extern	void DG_DequeuePrim( DG_PRIM * ) ;
extern	void DG_DequeuePrim2( DG_PRIM2 * ) ;
extern	int	DG_QueuePrim2( DG_PRIM2 * ) ;
extern int DG_QueueSpotObjs( DG_SPOT *objs );
extern void DG_DequeueSpotObjs( DG_SPOT *objs );
extern int DG_QueueEvmObj( DG_EVMOBJ *evmobj );
extern void DG_DequeueEvmObj( DG_EVMOBJ *evmobj );
//extern void DG_SetDefDrawEnv( DRAWENV *env, long x, long y, long w, long h );
extern void *DG_PopDefaultDrawEnv( DG_CHANL *cp, void *addr );
extern DG_OBJ_BUFFER *DG_MakeUserObjectBuffer( int max, int id );
extern void DG_FreeUserObjectBuffer( DG_OBJ_BUFFER *obj_buff );
extern int DG_QueueUserObject( DG_OBJ_BUFFER *obj_buff, void *objs );
extern void DG_DequeueUserObject( DG_OBJ_BUFFER *obj_buff, void *objs );
extern void	DG_AddCurrentGroup( int id );
extern void	DG_DeleteCurrentGroup( int id );
extern void	DG_AddCurrentGroup2( int chanl, int id );
extern void	DG_DeleteCurrentGroup2( int chanl, int id );
extern int DG_SetPrivilegeMode( int mode );
extern int DG_SetReverseDrawOrderMode( int mode );

#ifdef DEBUG
extern void DG_SetBackGroundColor( long r, long g, long b );
#endif

	/*	dgd.c	*/
extern void	DG_ResetSystem() ;
extern void	DG_StartDaemon() ;
extern void	DG_ResetTexture( void );
extern void	DG_VramClear();
extern void DG_SetDisplayMode( int which );
extern void	DG_SetDisplayOffset( int x, int y, int flag );

	/*	frame.c	*/
extern	void DG_SetFogColor( int r, int g, int b );
extern	void DG_SetFogParam( float near, float far );
extern	void DG_SetClipParam( float near, float far );
extern	void	DG_SetDispEnv( int, int, int, int, int ) ;
extern	void	DG_InitFrameSystem() ;
extern	void	DG_StartFrame() ;
extern	void	DG_EndFrame() ;
extern	void	DG_MakeCameraMatrix( FMATRIX *mat, FVECTOR *from, FVECTOR *to );
extern	void	DG_SetCamera( DG_CHANL *chanl, FMATRIX *mat, float screen );
extern void		DG_SetCamera2( DG_CHANL *chanl, FVECTOR *from, FVECTOR *to, float screen );
//extern	void	DG_SetWindow( RECT *, int ) ;
//extern	void	DG_SetScreen( MATRIX *, MATRIX * ) ;
extern	void	DG_FrameChanl( DG_CHANL *cp, int which );

extern	void	DG_AddCurrentGroup( int ) ;
extern	void	DG_DeleteCurrentGroup( int ) ;

	/*	objs.c	*/
extern	DG_OBJS	*DG_MakeObjs( DG_DEF *, int, int ) ;
extern DG_OBJS *DG_MakeObjs2( DG_DEF *def, int flag, int chanl, DG_DEFMDLPAIR *defmdl_list, int n_list );
extern void DG_ChangeModelObj( DG_OBJS* changeObjs, DG_OBJ *obj, DG_DEF* mdlDef, DG_MDL *mdl );
extern	void	DG_FreeObjs( DG_OBJS * ) ;
extern void DG_ConnectObjs( DG_OBJS *parent, DG_OBJS *child );
extern void DG_DisconnectObjs( DG_OBJS *parent, DG_OBJS *child );
extern void DG_SetFogParamObjs( DG_OBJS *objs, float near, float far );
extern void DG_SetLowObjs( DG_OBJS *objs, DG_OBJS *low_objs );
extern void DG_WriteMdlPaketUV( int, DG_MDLPACK *pack );

	/*	screen.c	*/
extern	void	DG_ScreenChanl( DG_CHANL *, int ) ;
extern void DG_StartBoundingCheckSupport( void );

	/*	chain.c	*/
extern	void	DG_ChainChanl( DG_CHANL *, int ) ;
extern   void  DG_ChainChanlLast( DG_CHANL *, int ) ;
extern	void	DG_SortChainChanl( DG_CHANL *, int ) ;
extern	void	DG_SortChainFirstChanl( DG_CHANL *, int ) ;

	/*	chain2.c	*/
extern void DG_Chain2Chanl( DG_CHANL *cp, int which );
extern void DG_Chain2ChanlLatter( DG_CHANL *cp, int which );
extern void DG_Chain2ChanlLast( DG_CHANL *cp, int which );

	/*	dma_ctrl.c	*/
extern	void DG_DmaReset( void );
extern	void DG_DmaClear( int which );
extern	void DG_DmaStart( int which );
extern	void DG_DmaSkip( int which );
extern	int DG_DmaCheckEnd( void );
#ifndef DEBUG_MODE
extern	void *DG_OpenDmaTask( int type, void *addr, int size );
#else
extern	void *_DG_OpenDmaTask( int type, void *addr, int size, char *fname );
#define DG_OpenDmaTask( _t, _a, _s )	_DG_OpenDmaTask( _t, _a, _s, __FILE__ )
#endif
extern	void DG_CloseDmaTask( void );
extern	void *DG_ResizePacketMemory( int size );
extern void *DG_OpenDmaPacketArea( void );
extern void DG_CloseDmaPacketArea( void *addr );

	/*	opack.c	*/
extern	int DG_WriteObjsPacketInit( void *tag_addr, DG_CHANL *cp, int flag );
extern	int DG_WriteObjsPacketInit2( void *tag_addr, DG_CHANL *cp );
extern	int DG_WriteObjsPacketInit_MakeShadow( void *tag_addr, DG_CHANL *cp );
extern	int DG_WriteObjsPacketInit_MakeSpot( void *tag_addr, DG_CHANL *cp, int color, void *img_addr, int type );
extern	int DG_WriteObjsPacketInit_WriteShadow( void *tag_addr, DG_CHANL *cp, float fog_param1, float fog_param2 );
extern	int DG_WriteObjsPacketInit_MakeShadowMask( void *tag_addr, DG_CHANL *cp, int color );
extern	int DG_WriteObjsPacketEnd( void *tag_addr );
extern	int DG_WriteTextureChangePacks( void *tag_addr, void *tex_packet );
extern	int DG_WriteTextureChangePacks2( void *tag_addr, DG_TEXTURE_LIST *tex_list, int which );
extern	int DG_WriteFogChangePacks( void *tag_addr, int fogcol );
extern	int DG_WriteRasterMaskPacks( void *tag_addr, int mask );
extern	int DG_WriteMipmapSettingPacks( void *tag_addr, DG_MIPMAP *mipmap );

	/*	pos.c	*/
extern	void	DG_GetPos( FMATRIX *world );
extern	void	DG_SetPos( FMATRIX * ) ;
extern	void	DG_SetPos2( FVECTOR *, SVECTOR * ) ;
extern	void	DG_MovePos( FVECTOR * ) ;
extern	void	DG_RotatePos( SVECTOR * ) ;
extern	void	DG_RotatePosZYX( SVECTOR *rot );
extern	void	DG_ScalePos( FVECTOR *scale );
extern	void	DG_PutObjs( DG_OBJS * ) ;
//extern	void	DG_PutPrim( DG_PRIM * ) ;
extern	void	DG_PutVector( FVECTOR *, FVECTOR *, int ) ;
extern	void	DG_RotVector( FVECTOR *, FVECTOR *, int ) ;
extern	void	DG_ReflectMatrix( FVECTOR *, FMATRIX *, FMATRIX * ) ;
extern	void	DG_ReflectVector( FVECTOR *, FVECTOR *, FVECTOR * ) ;
//extern	void	DG_MatrixRotZYX( FMATRIX *, SVECTOR * ) ;

//extern	void	DG_PersVector( SVECTOR *, DVECTOR *, int ) ;
//void DG_PointCheck( SVECTOR *vec, long n_vec );
extern int DG_PointCheckOne( FVECTOR *vec, int flag );
extern int DG_BoundCheck( FMATRIX *world, FVECTOR *bound_max, FVECTOR *bound_min );
extern int DG_BoundCheckFlag( FMATRIX *world, FVECTOR *bound_max, FVECTOR *bound_min, int flag );
extern int 	DG_ObjsBoundCheckChanl( DG_OBJS *objs, int chanl ) ;
extern int	DG_ObjBoundCheckChanl( DG_OBJS *objs, int joint, int chanl ) ;
extern void	DG_TransPersOneChanl( FVECTOR *res, FVECTOR *pos, int chanl ) ;
extern void	DG_TransPersOne( FVECTOR *res, FVECTOR *pos );

	/*	light.c	*/
extern void	DG_InitLightSystem() ;
extern void	DG_SetAmbient( int, int, int ) ;
extern void	DG_SetMainLightDir( int, int, int ) ;
extern void	DG_SetMainLightCol( int, int, int ) ;
extern void DG_ResetFixedLight( void );
extern void DG_SetFixedLightMap( LIT_DEF *light, int map_id );
extern LIT_DEF* DG_GetFixedLight( int map_id );
extern void DG_LightFlagClear( LIT_DEF *lit_def );
extern void DG_ClearTmpLight() ;
extern int DG_SetTmpLight( FVECTOR *, float, float ) ;
extern int DG_SetTmpLight2( FVECTOR *point, float r_range, float e_range, int color, int flag );
extern int DG_SetTmpSpotLight( FVECTOR *point, FVECTOR *dir, float range, float umbra, float penumbra, int color, int flag );
extern int DG_GetLightMatrix( FVECTOR *, FMATRIX * ) ;
extern int DG_GetLightMatrixFix( FVECTOR *pos, FMATRIX *light );
extern int DG_GetShadowLight( FVECTOR *lit_pos, FVECTOR *lit_dir, float *lit_power, FVECTOR *pos, int fix_flag );
extern void	DG_SetLightMatrix( DG_OBJS *, FMATRIX * ) ;
extern void DG_SwitchLightSphere( FVECTOR *pos, float r_range, int flag ) ;
extern void DG_SwitchLightBound( FVECTOR *max, FVECTOR *min, int flag ) ;
extern void DG_DestroyLightSphere( FVECTOR *pos, float r_range ) ;
extern void DG_DestroyLightBound( FVECTOR *max, FVECTOR *min ) ;
extern void DG_LightReshadeChanl( DG_CHANL *cp, int which );

	/* pshade.c */
extern int DG_MakePreshade( DG_OBJS *objs, LIT_DEF *lit_def );
extern int DG_MakePreshadeParts( DG_OBJS *objs, LIT_DEF *lit_def );
extern void DG_FreePreshade( DG_OBJS *objs );
extern void DG_TempPreshadeRGB( DG_OBJ *obj, LIT_DEF *lit_def, int which, int start_flag );
extern void DG_TmpLightPreshadeChanl( DG_CHANL *cp, int which );
extern void DG_RefreshVAnimeChanl( DG_CHANL *cp, int which );

	/*	loader.c	*/
extern	int	DG_LoadInitKms( void *buf, int id );
extern	int	DG_LoadInitMt3( void *buf, int id );
extern	int	DG_LoadInitLt2( void *buf, int id );
extern	int	DG_LoadInitCvd( void *buf, int id );
extern	int	DG_LoadInitCv2( void *buf, int id );
extern	int	DG_LoadInitEvm( void *buf, int id );
extern	int	DG_LoadInitZar( void *buf, int id );



	/*	text.c	*/
extern	void DG_InitTextureCache( void );
extern	int	DG_LoadInitTri( void *buf, int id, int cache_mode );
extern int DG_LoadInitTriBuf( void *buf, int code, int cache_mode, DG_TEXTURE_LIST *list );
extern	void *DG_FreeTextureCache( void *buf );
extern	void DG_SetTextureTmpClut( int which );
extern	void DG_ResetTextureTmpClut( void );
extern	DG_TEX	*DG_GetTexture( unsigned int ) ;
extern	DG_TEX* DG_GetTexture2( unsigned int tri_code, unsigned int code );
extern DG_TEX_PACKET* DG_GetTexturePacket( unsigned int code );
extern DG_TEXTURE_LIST* DG_GetTextureList( unsigned int code );
extern int	DG_SearchTriFromKms( DG_DEF *def );
extern int	DG_SearchTriFromTex( DG_TEX *org );
extern void *DG_MakeLoadImagePacket( void *tag_addr, int fmt, int width, int height, int vram_addr, int vram_width, void *tex_addr );
extern void *DG_MakeMoveImagePacket( void *tag_addr,
							 int dst_addr, int dst_width, int dst_fmt, int dst_x, int dst_y,
							 int src_addr, int src_width, int src_fmt, int src_x, int src_y,
							 int width, int height, int dir );
extern void DG_GetTexelInfo( int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex );

extern DG_TEX_MOVEREPLACE *DG_MakeMoveReplacePacket( int tri_code, int dst_tex_id, int src_tex_id );
extern void DG_FreeMoveReplacePacket( DG_TEX_MOVEREPLACE *tex_replace );
extern DG_TEX_LOADREPLACE *DG_MakeLoadReplacePacket( int tri_code, int dst_tex_id, void *texel, void *clut );
extern void DG_FreeLoadReplacePacket( DG_TEX_LOADREPLACE *tex_replace );
extern void DG_SetMoveReplaceTexture( DG_TEX_MOVEREPLACE *tex_replace );
extern void DG_ResetMoveReplaceTexture( DG_TEX_MOVEREPLACE *tex_replace );
extern void DG_SetLoadReplaceTexture( DG_TEX_LOADREPLACE *tex_replace );
extern void DG_ResetLoadReplaceTexture( DG_TEX_LOADREPLACE *tex_replace );
extern void DG_MakeMipmap( DG_MIPMAP *mipmap );
extern void DG_SetMipmapLodParam( DG_MIPMAP *mipmap, float screen );


	/*	prim.c	*/
//extern void DG_PrimChanl( DG_CHANL *cp, int which );
//extern DG_PRIM *DG_MakePrim( int type, int n_packet, int n_prims, int chanl, FVECTOR *pos, FVECTOR *vec );
//extern void DG_FreePrim( DG_PRIM *prim );
//extern void DG_ConfigPrimInitTex( DG_PRIM_INIT *prim_init, DG_TEX *tex );
//extern void DG_ConfigPrimTex( DG_PRIM *prim, DG_TEX *tex );

	/*	sort.c	*/
extern	void	DG_SortChanl( DG_CHANL *, int ) ;

	/* shdwchin.c */
extern void DG_ShadowChainChanl( DG_CHANL *cp, int which );
extern void DG_SpotChainChanl( DG_CHANL *cp, int which );
extern void DG_NewShadowChainSetup( );

	/* vanime.c */
extern void DG_MakeAnimVertsBuffer( DG_VERTS_ANIME *v_anime, DG_OBJ *obj, int flag );
extern void DG_RestartAnimVerts( DG_VERTS_ANIME *v_anime );
extern void DG_StopAnimVerts( DG_VERTS_ANIME *v_anime );
extern void DG_FreeAnimVertsBuffer( DG_VERTS_ANIME *v_anime );
extern void DG_RegistCommonVertex( FVECTOR *verts, int n_verts );
extern void DG_RefineStripVertex( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonVertex2( FVECTOR *verts, int n_verts );
extern void DG_RefineStripVertex2( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonNormal( FVECTOR *verts, int n_verts );
extern void DG_RegistCommonNormal2( FVECTOR *verts, int n_verts );
extern void DG_RefineStripNormal( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonUV( FVECTOR *verts, int n_verts );
extern void DG_RefineStripUV( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RefineStripUV2( DG_VERTS_ANIME *v_anime, short *index );

	/* comdl.c */
extern void DG_AddPluginComdl( void );
extern void DG_DeletePluginComdl( void );
//extern void DG_ComdlChanl( DG_CHANL *cp, int which );
extern DG_COMDL* DG_MakeComdl( DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl );
extern void DG_FreeComdl( DG_COMDL *comdl );
extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );

	/*	prim2.c	*/
//extern int DG_WritePrimPacks( DG_CHANL *cp, DG_PRIM2_PACKET *packet, void *addr, int which );
extern void DG_BP_WritePrimInitPacket( DG_CHANL *cp );
extern void DG_BP_WritePrimLocalParam( DG_PRIM2 *prim );
extern int DG_BP_WritePrimPacksAndPendRender( DG_CHANL *viewport, DG_PRIM2_PACKET *packet, int clone );
extern void DG_BP_WritePrimPacksFinalize();
extern void DG_Prim2Chanl( DG_CHANL *cp, int which );
extern void DG_Prim2FirstChanl( DG_CHANL *cp, int which );
extern DG_PRIM2 *DG_MakePrim2( int type, int n_prims, int n_verts, int chanl );
extern void DG_FreePrim2( DG_PRIM2 *prim );
extern void DG_ConfigPrim2Tex( DG_PRIM2 *prim, DG_TEX *tex );
extern void DG_SetPrim2Buffer( DG_PRIM2 *prim2, FVECTOR *pos0, FVECTOR *pos1, void *uvrgb0, void *uvrgb1 );

	/* evmobj.c */
extern void DG_ChainEvmChanl( DG_CHANL *cp, int which );
extern void DG_ChainEvmChanlLatter( DG_CHANL *cp, int which );
extern DG_EVMOBJ *DG_MakeEvmObj( EVM_DEF *def, int flag, int chanl );
extern void DG_FreeEvmObj( DG_EVMOBJ *evmobj );
extern void DG_WriteEvmMdlPaketUV( int tri_code, EVM_PACK *pack );
extern int  DG_QueueEvmObj( DG_EVMOBJ *evmobj ) ;
extern void DG_DequeueEvmObj( DG_EVMOBJ *evmobj ) ;

	/* plugin.c */
extern void DG_InitPluginSystem( void );
extern DG_PLUGIN *DG_SearchPlugin( int object_id );
void DG_MakePlugin( DG_PLUGIN *plugin, int id, int flag, int phase, int prio, void *func, int n_queue );
extern void DG_FreePlugin( DG_PLUGIN *plugin );
extern void DG_AddPlugin( DG_PLUGIN *add_plugin );
extern void DG_DeletePlugin( DG_PLUGIN *del_plugin );
extern void DG_PluginStartChanl( DG_CHANL *cp, int which );
extern void DG_PluginEndChanl( DG_CHANL *cp, int which );
extern void DG_PluginChanl( DG_CHANL *cp, int which );

	/* store.c */
extern void DG_StoreChanl( DG_CHANL *cp, int which );
extern void DG_SetFrameStore( void *store_addr, int mode );
extern void DG_RetryStoreChanl( void );

	/* packet.c */
extern void DG_InitSystemPacket( void );
extern void *DG_WritePacket_VIF1Init( void *addr );
extern void *DG_WritePacket_Vu1ModelProg( void *addr, int param );
extern void *DG_WritePacket_Vu1ModelProgEx( void *addr, int param );
extern void *DG_WritePacket_Vu1ComdlProg( void *addr, int param, int **func_list );
extern void *DG_WritePacket_Vu1ShadowProg( void *addr, int param );
extern void *DG_WritePacket_DefaultModelGifInit( void *addr );
extern void *DG_WritePacket_DefaultSemiTransModelGifInit( void *addr );
extern void *DG_WritePacket_MakeShadowModelGifInit( void *addr );
extern void *DG_WritePacket_WriteShadowModelGifInit( void *addr );
extern void *DG_WritePacket_MakeShadowMaskGifInit( void *addr );
extern void *DG_WritePacket_MakeSpotModelGifInit( void *addr );
extern void *DG_WritePacket_MakeSpotShadowModelGifInit( void *addr );
extern void *DG_WritePacket_Circle( void *addr, int color );
extern void *DG_WritePacket_MakeSoftTexture( void *addr );
extern void *DG_WritePacket_MakeSoftTexture2( void *addr );
extern void *DG_WritePacket_TransSpotImage( void *addr, void *data, int color );
extern void *DG_WritePacket_SoftImageTurnOut( void *addr, int which );

	/* opt_cmf.c */
extern void DG_AddPluginOptcmf( void );
extern void DG_AddPluginOptcmf( void );

	/* m_weight.c */
extern void DG_StartMultiWeightSupport( void );
extern void DG_StoreSkeletonMatrix( FMATRIX *mats, int num );
extern void DG_TransMultiWeightVertex( FVECTOR *verts, FVECTOR *weight, char *mat_id, int n_verts );
extern void DG_TransMultiWeightNormal( FVECTOR *norms, FVECTOR *weight, char *mati_d, int n_verts );
extern void DG_TransMultiWeightVertexNormal( FVECTOR *res_verts, FVECTOR *res_norms, FVECTOR *verts, FVECTOR *norms, FVECTOR *weight, char *mat_id, int n_verts );

extern void DG_CoverShadowChainChanl( DG_CHANL *cp, int which );
