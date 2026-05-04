/* -------------------------------------------------------------------*/
/*                    マズルフラッシュ                                */
/* -------------------------------------------------------------------*/
/* m92 */
extern void MazzleM92( FMATRIX *world, int silence, int light_mode ) ;
/* usp */
extern void MazzleUSP(FMATRIX *world,int silence, int light_mode) ;
extern void AN_MazzleUSP(FMATRIX *world, int  silence) ;
/* famas */
extern void MazzleFAMAS( FMATRIX *world, int silence, int light_mode ) ;
/* mkr */
extern void MazzleMKR( FMATRIX *world, int silence, int light_mode ) ;
extern void AN_MazzleMKR(FMATRIX *world,int silence,int mode) ;
/* aks */
extern void MazzleAKS( FMATRIX *world, int silence, int light_mode ) ;
extern void AN_MazzleAKS(FMATRIX *world,int silence,int mode) ;
// p90
extern void *MazzleFlash2( int , FMATRIX *, FVECTOR *, float *, u_char *, float );

extern void *MazzleFlash(int n_verts,FMATRIX *world,FVECTOR *shift, float *size, u_char *bright) ;
extern void *SubMazzleFlash(FMATRIX  *world) ;
extern void *SubMazzleFlash2(FMATRIX  *world) ;
extern void *NewGunSmoke(FVECTOR *start,FVECTOR *speed, float size,u_char num) ;
extern void *NewLineSmokeMng( FMATRIX *pWorld , FVECTOR *pShift , int life );
extern void *NewSmokeNormal( int , FMATRIX *, FVECTOR *, FVECTOR * , int , float , FVECTOR * , float );
extern void *NewSmokeDemo( int , FMATRIX *, FVECTOR *, FVECTOR * , int , float , FVECTOR * , float );
extern void *NewGunSmoke2( FMATRIX * , FVECTOR * , float , int , float );
extern void *NewSubMazzleFlushMngNormal( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , float , int );

/* -------------------------------------------------------------------*/
/*                    空薬莢                                            */
/* -------------------------------------------------------------------*/
/* m92 */

/* usp */
extern void   CartridgeUSP(FMATRIX *world, OBJECT *weapon, CONTROL *control) ;
extern void   AN_CartridgeUSP_E(FMATRIX *world,OBJECT   *weapon,CONTROL  *control);
extern void   AN_CartridgeUSP(FMATRIX *world,OBJECT   *weapon,CONTROL  *control) ;

/* famas */
/* mkr */
extern void   CartridgeMKR(FMATRIX *world, OBJECT   *weapon, CONTROL  *control) ;
extern void   AN_CartridgeMKR_E(FMATRIX *world, OBJECT   *weapon, CONTROL  *control) ;
/* aks */

//extern void *NewGunSmoke_Repeater() ;
extern void  *NewSpark1(int n_packets, FVECTOR *center, float min_speed, 
                       float speed_wide, float gravity, SVECTOR *rot, 
		       SVECTOR *rot_wide, FVECTOR *color, float length, int count );

/* 薬莢 */
extern void *MdlCartridg_USP( 
FVECTOR  *pos,                       
FVECTOR  *speed,              /* スピード */
SVECTOR  *rot,
OBJECT   *weapon,
CONTROL  *control              /* 人体コントロール */
) ;

extern void *MdlCartridg_AKS( FVECTOR *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL *control ) ;
extern void *MdlCartridg_P90( FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL *control ) ;
extern void *MdlCartridg_GLK( FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL *control ) ;
extern void *MdlCartridg_M4A1( FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL *control ) ;
extern void *MdlCartridg_ABK( FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL *control ) ;
extern void *MdlCartridg_M4_demo(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control );
extern void *MdlCartridg_M4_demo_gun(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control );
extern void *MdlCartridg_SOCOM(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control );
extern void *MdlCartridg_SPS(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control );
extern void *MdlCartridg_PSG(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control );
extern void *MdlCartridg_MECA(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control );

/* 火花 */
extern void *NewSpark2(
int          n_packets,  FMATRIX      *matrix,float        min_speed,
float        speed_wide,float        gravity,SVECTOR      *rot,
SVECTOR      *rot_wide,FVECTOR      *color,float        length,	
int          count 
);

/* テンプライト */
extern void  Big_TmpLight2(FVECTOR *pos,float r_range,float e_range,int color,int flag) ;
