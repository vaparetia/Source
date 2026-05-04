/*
   brk_utlh
   壊れ用 プロトタイプ定義ファイル

   2000/09/14 T. Morita
   $Id: brk_utl.x,v 1.1.1.3 2002/11/19 11:45:50 Yoshizawa1 Exp $
*/




/* brk_utl_lgt.c */
extern void  BRK_UTL_GetLightMatrix( FVECTOR *pos, FMATRIX *lights, float intense, int where ) ;
extern void  BRK_UTL_PosColor( FVECTOR *pos, FVECTOR *dir,
			       IVECTOR *col, float intense, int where ) ;
extern void  BRK_UTL_ComdlColor( DG_COMDL_POS *p, float intense, int where ) ;
extern void  BRK_UTL_ComdlColor2( DG_COMDL_POS *p, int axis ) ;




/* brk_utl_aim.c */
extern float BRK_UTL_DistPointToLine( FVECTOR *p, FVECTOR *lp, FVECTOR *ld ) ;
extern int   BRK_UTL_CheckLineInFloatPolygon( FVECTOR *lpos, FVECTOR *ldir,
									FVECTOR *ppos
									) ;
extern int   BRK_UTL_CheckLineInPolygon( FVECTOR *lpos, FVECTOR *ldir,
#ifdef PSX2
							    SVECTOR *vtx
#else
							    DG_VERTEX_KMSS *p0,
							    DG_VERTEX_KMSS *p1,
							    DG_VERTEX_KMSS *p2
#endif
							   ) ;
extern void  BRK_UTL_LocalForceAndPoint( FVECTOR *lpos, FVECTOR *ldir,
					 FVECTOR *pos , FVECTOR *dir,
					 FMATRIX *world ) ;
extern int   BRK_UTL_CheckModelLine( FVECTOR *offs,
				     FVECTOR *hit, FVECTOR *force,
				     DG_MDL *model, FMATRIX *world ) ;

/* brk_utl_act.c */
extern void  BRK_UTL_SizeOfMDL( DG_MDL *mdl, FMATRIX *m, FVECTOR *size ) ;
extern void  BRK_UTL_SizeOfBound( DG_OBJS *objs, FMATRIX *m, FVECTOR *size ) ;
extern int   BRK_UTL_PutOnHazard( FMATRIX *mat, FVECTOR *o_pos,
				  float width, int rx_msk, int ry_msk,
				  void *hzd, FVECTOR *size ) ;
extern int   BRK_UTL_PutCenterHazard( void *hzd, FVECTOR *o_pos, FVECTOR *o_vel,
				      float length, float speed ) ;
extern void *BRK_UTL_GetCenterHazard( void *hzd, FVECTOR *o_pos, FVECTOR *o_vel,
				      float length, float speed ) ;


/* brk_utl_obj.c */
extern void BRK_UTL_CallOffenceWhenThrough( TARGET *off, TARGET *def ) ;
extern void BRK_UTL_PK_Vibrate( FVECTOR *pos, TARGET *off ) ;
extern FVECTOR *BRK_UTL_BlowBlastCheck( FVECTOR *pos, TARGET *off ) ;



/* brk_utl_ini.c */
extern void      BRK_UTL_InitSPR( DG_PRIM2 *p, int i, int rgba, DG_TEX *t, float size ) ;
extern DG_PRIM2* BRK_UTL_MakeSPRT( int n_prim, int tex_id, u_long64 alpha ) ;
extern DG_PRIM2* BRK_UTL_MakeSPRTWH( int n_prim, int tex_id, u_long64 alpha, float size, int rgba ) ;
extern void      BRK_UTL_InitPOLY( DG_PRIM2 *p, int i, int rgba, DG_TEX *t ) ;
extern DG_PRIM2* BRK_UTL_MakePOLY( int n_prim, int tex_id, u_long64 alpha, int rgba ) ;
extern DG_PRIM2* BRK_UTL_MakePOLYFOG( int n_prim, int tex_id, u_long64 alpha, int rgba ) ;
extern DG_PRIM2* BRK_UTL_MakeRSPRTWH( int n_prim, int tex_id, u_long64 alpha, float size, int rgba ) ;
extern DG_PRIM2* BRK_UTL_MakeLINE2( int n_prim, int n_verts,
				    int tex_id, u_long64 alpha,
				    int rgba0, int rgba1 ) ;
extern DG_PRIM2* BRK_UTL_MakeLINE( int n_prim, int tex_id, u_long64 alpha, int rgba0, int rgba1 ) ;




/* brk_utl_mtx.c */
extern FMATRIX BRK_UTL_MatrixTable[16][16] ;
extern int     BRK_UTL_InitMatrixTable( void ) ;



/* brk_utl_glass.c */
extern void BRK_UTL_PutGlassWeb( TARGET *t, DG_TEX *tex,
				 FVECTOR *hit_pos, float offset, float size,
				 FVECTOR        *v0, FVECTOR        *v1,
				 DG_PRIM2_UVRGB *u0, DG_PRIM2_UVRGB *u1  ) ;
