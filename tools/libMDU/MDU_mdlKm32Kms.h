/*
   MDU_mdlKm32Kms.h

   Ｋｍ３ → Ｋｍｓコンバート関数ヘッダ

   by M.Sonoyama 1999.Sep.～ 
   $Id: MDU_mdlKm32Kms.h,v 1.3 2002/04/10 02:21:43 usr04098 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

typedef struct
{
    u_short  vid;
    u_short  nid;
    TVECTOR  uvs;
} POINT2 ;

typedef struct
{
    POINT2   point[4];
} PRIM;

typedef struct
{
    long      n_prims;
    u_int     tid;
    PRIM      *prims;
    u_short   *flag;
    u_short   have_env;
} DIV_PRIM;

typedef struct
{
    long     n_points;
    POINT2   *point;
    SVECTOR  *verts;
    SVECTOR  *norms;
    u_int    tid;
    u_short  have_env;
} STRIP;

typedef struct
{
    KMS_DEF	*sdef ;

    KM3_DEF      dg_def;        
    KM3_MDL      *dg_mdl;
    
    long        n_div_prims;
    DIV_PRIM    *div_prim;
    
    long        n_strips;
    STRIP       *strip;
    
    KMS_DEF     new_def;
    KMS_OBJ  	*new_obj;
    
    u_long      addr_size;
} WORK;

typedef	KM3_DEF		DG_DEF ; 
typedef	KM3_MDL		DG_MDL ;
typedef	KMS_DEF		NEW_DEF ;
typedef	KMS_OBJ		NEW_OBJECT ;
typedef	KMS_PKT		NEW_PACKET ;
typedef	TVECTOR_S	NEW_TVECTOR ;

typedef struct
{
   long type;
   long n_prims;
   long lx, ly, lz, ux, uy, uz;
   long tx, ty, tz;
   long parent;
   long extend;
   long n_verts;
   long verts_addr;
   long vertsid_addr;
   long n_norms;
   long norms_addr;
   long normsid_addr;
   long uvs;
   long tid;
   long	envs ;
   long	pad ;
   long zero;
} KM2_MDL;

typedef struct
{
    long   type;
    long   n_packs;
    float  lx,ly,lz;
    float  ux,uy,uz;
    float  tx,ty,tz;
    long   parent;
    long   packs;
    long   pad0;
    long   pad1;
    long   pad2;
} KMS_OBJECT;

typedef struct
{
    long flag;
    long n_verts;
    long tid;
    long pad;
    long verts;
    long norms;
    long uvs;
    long rgbs;
} KMS_PACKET;

#undef FREAD
#undef FWRITE
enum {STRCMP,
FILE_OPERATION,
FREAD,
CALLOC,
FSEEK,
FWRITE,
FTELL,
FILED};

#define RIGHT 1
#define LEFT -1
#define   LONG_BYTE  16  /* バイト数 */

/*------------------------------------------------------------------------*/

#define	NOW_NO_USE

/************************************/
/*           strip.c                */
/************************************/
static	void Strip(void);
static	void Make_Strip(DG_MDL *mdl,NEW_OBJECT *obj);
static	void Divide_PRIM(DG_MDL *mdl);
static	void Divide_By_ENV(DG_MDL *mdl);
static	void Divide_By_TEX(PRIM *prim,DG_MDL *mdl);
static	PRIM *Make_PRIM(DG_MDL *mdl);
static	void Check_STRIP_Len(long start_num);
static	void Union_STRIP(long start_num);

static	void Strip_Algo_1(DIV_PRIM  *div_prim,DG_MDL *mdl);
static	STRIP Set_STRIP_1(long prim_num,long verts_num,DIV_PRIM *div_prim,u_short *dummy_flag,int dir);

/********************************************/
/*            util.c                        */
/********************************************/
/* 領域確保   */
#ifndef NOW_NO_USE
static	DG_MDL      *Calloc_DG_MDL(long n);
static	KM2_MDL     *Calloc_KM2_MDL(long n);
static	TVECTOR     *Calloc_TVECTOR(long n);
static	KMS_OBJECT  *Calloc_KMS_OBJECT(long n);
static	KMS_PACKET  *Calloc_KMS_PACKET(long n);
static	NEW_OBJECT  *Calloc_NEW_OBJECT(long n);
#endif
static	SVECTOR     *Calloc_SVECTOR(long n);
static	u_short     *Calloc_USHORT(long n);
static	PRIM        *Calloc_PRIM(long n);
static	DIV_PRIM    *Calloc_DIV_PRIM(long n);
static	STRIP       *Calloc_STRIP(long n);
static	POINT2       *Calloc_POINT(long n);
static	NEW_PACKET  *Calloc_NEW_PACKET(long n);
static	NEW_TVECTOR *Calloc_NEW_TVECTOR(long n);
static	int  P_Equal(POINT2 *p_a,POINT2 *p_b);
/* エラー出力 */
static	void ERR_PRINT(int mode);

/********************************************/
/*          print.c ( デバッグ用 )          */
/********************************************/
#ifndef NOW_NO_USE
static	void Print(void);
static	void Print_VERTS(FILE *fpw,DG_MDL *mdl);
static	void Print_NORMS(FILE *fpw,DG_MDL *mdl);
static	void Print_PRIM(FILE *fpw,DG_MDL *mdl);
#endif
