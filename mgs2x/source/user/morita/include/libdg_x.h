#ifndef __MORITA_LIBDG_H__
#define __MORITA_LIBDG_H__


/*prototypes FOR DG LIB*/

/*comdl.c*/
extern void      DG_AddPluginComdl( void );
extern void      DG_DeletePluginComdl( void );
extern int       DG_QueueComdlObjs( DG_COMDL *comdl );
extern void      DG_DequeueComdlObjs( DG_COMDL *comdl );
extern DG_COMDL* DG_MakeComdl( DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl );
extern void      DG_FreeComdl( DG_COMDL *comdl );

/*chanl.c*/
extern void DG_DequeueEvmObj( DG_EVMOBJ *evmobj ) ;
extern int DG_QueueEvmObj( DG_EVMOBJ *evmobj ) ;


/*objs.c*/
//extern DG_OBJS	*DG_MakeObjs( DG_DEF *def, int flag, int chanl ) ;
//extern DG_OBJS	*DG_MakeObjs2( DG_DEF *def, int flag, int chanl, DG_MDL **mdl_list, int n_list ) ;
extern void DG_ChangeModelObj( DG_OBJS* changeObjs, DG_OBJ *obj, DG_DEF* mdlDef, DG_MDL *mdl );
extern void DG_FreeObjs( DG_OBJS *objs ) ;
extern void DG_ConnectObjs( DG_OBJS *parent, DG_OBJS *child );
extern void DG_DisconnectObjs( DG_OBJS *parent, DG_OBJS *child );
extern void DG_WriteMdlPaketUV( int tri_code, DG_MDLPACK *pack );

/*evmobjs.c*/
extern DG_EVMOBJ *DG_MakeEvmObj( EVM_DEF *def, int flag, int chanl ) ;
extern void       DG_FreeEvmObjs( DG_EVMOBJ *objs ) ;

/*vanime.c*/
extern void DG_MakeAnimVertsBuffer( DG_VERTS_ANIME *v_anime, DG_OBJ *obj, int flag ) ;
extern void DG_RestartAnimVerts( DG_VERTS_ANIME *v_anime );
extern void DG_StopAnimVerts( DG_VERTS_ANIME *v_anime );
extern void DG_FreeAnimVertsBuffer( DG_VERTS_ANIME *v_anime );
extern void DG_RegistCommonVertex( FVECTOR *verts, int n_verts );
extern void DG_RefineStripVertex( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonVertex2( FVECTOR *verts, int n_verts );
extern void DG_RefineStripVertex2( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonNormal( FVECTOR *verts, int n_verts );
extern void DG_RefineStripNormal( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RegistCommonUV( FVECTOR *verts, int n_verts );
extern void DG_RefineStripUV( DG_VERTS_ANIME *v_anime, short *index );
extern void DG_RefineStripUV2( DG_VERTS_ANIME *v_anime, short *index );

/*pshade.c*/
extern void DG_FreePreshade( DG_OBJS *objs );
extern int  DG_MakePreshade( DG_OBJS *objs, LIT_DEF *lit_def );
extern int  DG_MakePreshadeParts( DG_OBJS *objs, LIT_DEF *lit_def ) ;
extern void DG_TempPreshadeRGB( DG_OBJ *obj, LIT_DEF *lit_def, int which, int start_flag );
extern void DG_TmpLightPreshadeChanl( DG_CHANL *cp, int which ) ;
extern void DG_RefreshVAnimeChanl( DG_CHANL *cp, int which );

/*prim2.c*/
//extern int       DG_WritePrimPacks( DG_PRIM2_PACKET *packet, void *addr, int which );
extern void      DG_Prim2Chanl( DG_CHANL *cp, int which );
//extern DG_PRIM2 *DG_MakePrim2( int flag, int n_prims, int n_verts, int chanl );
extern void      DG_FreePrim2( DG_PRIM2 *prim );
extern void      DG_ConfigPrim2Tex( DG_PRIM2 *prim, DG_TEX *tex );

/*light.c*/
extern void	DG_InitLightSystem() ;
extern void	DG_SetAmbient( int, int, int ) ;
extern void	DG_SetMainLightDir( int, int, int ) ;
extern void	DG_SetMainLightCol( int, int, int ) ;
extern void DG_SetFixedLightMap( LIT_DEF *light, int map_id );
extern LIT_DEF* DG_GetFixedLight( int map_id );
extern void DG_ClearTmpLight() ;
extern int DG_SetTmpLight( FVECTOR *, float, float ) ;
extern int DG_SetTmpLight2( FVECTOR *point, float r_range, float e_range, int color, int flag );
extern int DG_SetTmpSpotLight( FVECTOR *point, FVECTOR *dir, float range, float umbra, float penumbra, int color, int flag );
extern int DG_GetLightMatrix( FVECTOR *, FMATRIX * ) ;
extern int DG_GetLightMatrixFix( FVECTOR *, FMATRIX * ) ;
extern void	DG_SetLightMatrix( DG_OBJS *, FMATRIX * ) ;
extern void DG_SwitchLightSphere( FVECTOR *pos, float r_range, int flag ) ;
extern void DG_SwitchLightBound( FVECTOR *max, FVECTOR *min, int flag ) ;
extern void DG_DestroyLightSphere( FVECTOR *pos, float r_range ) ;
extern void DG_DestroyLightBound( FVECTOR *max, FVECTOR *min ) ;

/*text.c*/
extern	void DG_InitTextureCache( void );
extern	int	DG_LoadInitTri( void *buf, int id, int cache_mode );
extern	void *DG_FreeTextureCache( void *buf );
extern	void DG_SetTextureTmpClut( int which );
extern	void DG_ResetTextureTmpClut( void );
extern	DG_TEX	*DG_GetTexture( unsigned int ) ;
extern	DG_TEX* DG_GetTexture2( unsigned int tri_code, unsigned int code );
extern DG_TEX_PACKET* DG_GetTexturePacket( unsigned int code );
extern DG_TEXTURE_LIST* DG_GetTextureList( unsigned int code );
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

/*pos.c*/
extern void DG_GetPos( FMATRIX *world );
extern void DG_SetPos( FMATRIX *world );
extern void DG_SetPos2( FVECTOR *mov, SVECTOR *rot );
extern void DG_MovePos( FVECTOR *mov );
extern void DG_RotatePos( SVECTOR *rot );
extern void DG_RotatePosZYX( SVECTOR *rot );
extern void DG_PutObjs( DG_OBJS *objs );
extern void DG_PutVector( FVECTOR *from, FVECTOR *to, int n );
extern void DG_RotVector( FVECTOR *from, FVECTOR *to, int n );
extern void DG_ReflectMatrix( FVECTOR *pole, FMATRIX *m1, FMATRIX *m2 );
extern void DG_ReflectVector( FVECTOR *pole, FVECTOR *vec1, FVECTOR *vec2 );
extern int  DG_PointCheckOne( FVECTOR *vec, int flag );
extern int  DG_BoundCheckFlag( FMATRIX *world, FVECTOR *bound_max, FVECTOR *bound_min, int flag );
extern int  DG_BoundCheck( FMATRIX *world, FVECTOR *bound_max, FVECTOR *bound_min );
extern int  DG_ObjsBoundCheckChanl( DG_OBJS	*objs, int chanl );

#endif
