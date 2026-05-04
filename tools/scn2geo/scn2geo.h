/*

  SCN2GEO

  T.Morita


 */

#ifndef __SCN2GEO_H__
#define __SCN2GEO_H__



/*  これらは,P3Dを扱うためのものです  */
#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

/* 型定義 */

#define ALIGN16 __attribute__((aligned (16)))
#define ALIGN64 __attribute__((aligned (64)))
#define TO_ALIGN16(_a) (void *)(((_a)+0xf) & 0xfffffff0)

typedef struct { unsigned char r, g, b, cd ; } CVECTOR ;
typedef struct { short vx, vy, vz, pad ; } SVECTOR ;
typedef struct { int vx, vy, vz, vw ; } IVECTOR ALIGN16;
typedef struct { float vx, vy, vz, vw ; } FVECTOR ALIGN16;
typedef struct { float m[4][4] ; } FMATRIX ALIGN16 ;

typedef struct {
    FVECTOR bmin ;
    FVECTOR bmax ;
    FVECTOR cen ;
    FVECTOR max ;
    FVECTOR div ;
    FVECTOR pos ;
} POLYDATA ;

typedef int (*POLYFUNC)( HP3DOBJECT obj, void *data ) ;


/* マクロ定義 */
#define COPY_FVECTOR(_d,_s) ((_d)->vx=(_s)->vx,\
			     (_d)->vy=(_s)->vy,\
			     (_d)->vz=(_s)->vz)

#define SUB_FVECTOR(_r,_a,_b) ((_r)->vx=(_a)->vx-(_b)->vx,\
			       (_r)->vy=(_a)->vy-(_b)->vy,\
			       (_r)->vz=(_a)->vz-(_b)->vz)

#define ADD_FVECTOR(_r,_a,_b) ((_r)->vx=(_a)->vx+(_b)->vx,\
			       (_r)->vy=(_a)->vy+(_b)->vy,\
			       (_r)->vz=(_a)->vz+(_b)->vz)

#define SCALE_FVECTOR(_r,_a,_b) ((_r)->vx=(_a)->vx*(_b),\
				 (_r)->vy=(_a)->vy*(_b),\
				 (_r)->vz=(_a)->vz*(_b))

#define MUL_FVECTOR(_r,_a,_b) ((_r)->vx=(_a)->vx*(_b)->vx,\
			       (_r)->vy=(_a)->vy*(_b)->vy,\
			       (_r)->vz=(_a)->vz*(_b)->vz)

#define INNER_PRODUCT(_a,_b) ((_a)->vx*(_b)->vx+\
			      (_a)->vy*(_b)->vy+\
			      (_a)->vz*(_b)->vz)

#define OUTER_PRODUCT(_r,_a,_b) ((_r)->vx=(_a)->vy*(_b)->vz-(_a)->vz*(_b)->vy,\
				 (_r)->vy=(_a)->vz*(_b)->vx-(_a)->vx*(_b)->vz,\
				 (_r)->vz=(_a)->vx*(_b)->vy-(_a)->vy*(_b)->vx)

#define NORMALIZE(_r,_a)  ((_r)->vw=sqrtf(INNER_PRODUCT(_a,_a)),\
			   SCALE_FVECTOR(_r,_a,1.0f/(_r)->vw))

#define APPLY_MATRIX(_r,_m,_a) ((_r)->vx=INNER_PRODUCT((FVECTOR*)(_m)->m[0],_a),\
				(_r)->vy=INNER_PRODUCT((FVECTOR*)(_m)->m[1],_a),\
				(_r)->vz=INNER_PRODUCT((FVECTOR*)(_m)->m[2],_a),\
				(_r)->vw=INNER_PRODUCT((FVECTOR*)(_m)->m[3],_a))


#define TOOLS
/* システムインクルード */
#include "libgeo.h"


/*

  ツール内部用の型

*/
typedef struct {
    GEO_BLOCK  block ;
    int        extra_size ;
} GEO_BLK_POOL ;

typedef struct {
    GEO_BLK_POOL  pool[GEO_BLOCK_MAX+1] ;
    int           reset_align ;
} GEO_BLK_WORK ;


#define  MAX_VERTEX_REFS 10
#define  MAX_VERTEX_POOL 20000
typedef struct {
    FVECTOR v  ;
    u_char  fv_sv_flag ; /* 補間先でFV(1)であるかSV(0)であるか。 */
    void   *ref  ;
    int     ref_data ;
    void   *dest ;
} GEO_VTX_POOL ;

typedef struct {
    FVECTOR      base ; /* ベースポジション */
    GEO_VTX_POOL pool[MAX_VERTEX_POOL] ;
    int          n_pool ;

    SVECTOR *dest ;
    int      n_dest ;
    int      n_dest_max ;

} GEO_VTX_WORK ;


/*  グローバル変数 geo_main.c */
extern FVECTOR g_division ;


/*  グローバル変数 geo_core.c */
extern int g_unit_size[] ;
extern int g_unit_n_types ;
extern int g_type_size[]  ;

/*  Block 関連 geo_core.c */
extern GEO_BLK_POOL *core_block_getblock( int size, int extra_size ) ;
extern GEO_BLK_POOL *core_block_findblock( void *ptr ) ;
extern void *core_block_malloc( GEO_BLK_POOL *b, int size, int align_bit ) ;
extern void *core_block_extramalloc( GEO_BLK_POOL *b, int size ) ;
extern void core_block_set_align( int align ) ;
extern void core_block_start( int type, float x, float y, float z ) ;
extern GEO_BLOCK *core_block_end( void ) ;
extern void core_block_reshape( GEO_BLOCK *block ) ;
extern void core_block_addr_to_offset( GEO_BLOCK *block, u_int head ) ;
extern void core_block_offset_to_addr( GEO_BLOCK *block, u_int head ) ;



/*  Vertex登録 関連 geo_core.c */
extern void core_vertex_set_current( GEO_VTX_WORK *current ) ;
extern void core_vertex_start( float x, float y, float z ) ;
extern void core_vertex_end( void ) ;
extern int core_vertex_regist( GEO_VTX_WORK *work, FVECTOR *v, void *ref, int ref_data, int sv_fv_flag ) ;
extern int core_vertex_point( FVECTOR *v, void *ref, int ref_data ) ;
extern int core_vertex_normal( FVECTOR *n, void *ref, int ref_data ) ;
extern int core_vertex_base( FVECTOR *n, void *ref, int ref_data ) ;
extern void *core_vertex_reshape( GEO_VTX_WORK *work ) ;
extern void core_vertex_movedest( void *from, void *to, int size ) ;

/*  Geom 関連 geo_core.c */
extern void  core_geom_append( GEOM *geom, GEOM *tail ) ;
extern void  core_geom_connect( GEOM *geom, GEOM *head ) ;
extern void  core_geom_connect_parent( GEOM *geom, GEOM *parent ) ;
extern int   core_geom_get_size( u_int flag ) ;
extern GEOM *core_geom_getone( u_int flag, int extra_size ) ;
extern void  core_geom_reshape( void ) ;

/* Geogroup 関連 geo_core.c */
extern GEO_GROUP *core_group_make_groups( int n_groups ) ;
extern int        core_group_radix_get_offset( GEO_RADIX *r, int type ) ;
extern int        core_group_radix_add_geom( GEO_GROUP *g, GEOM *geom ) ;
extern void       core_group_radix_reshape( GEO_GROUP *g ) ;
extern int        core_group_add( GEO_GROUP *g,
				  FVECTOR *div, FVECTOR *max, FVECTOR *base ) ;

/* Geodef 関連 geo_core.c */
extern int      core_geodef_count_size( GEO_DEF *def ) ;
extern GEO_DEF *core_geodef_reshape( GEO_DEF *old ) ;
extern int      core_geodef_addr_to_offset( GEO_DEF *def ) ;
extern int      core_geodef_offset_to_addr( GEO_DEF *def ) ;
extern int      core_geodef_filesize( FILE *fp ) ;
extern void    *core_geodef_chank_malloc_align( int size, int align_bit ) ;
extern void    *core_geodef_chank_malloc( int size ) ;
extern int      core_geodef_chank_start( GEO_CHANK *chank, int type ) ;
extern int      core_geodef_chank_end( void ) ;

/* Prim 関連 geo_core.c */
extern int  core_prim_set_dot( int flag, int attribute, char *name,
							   FVECTOR *size, FVECTOR *pos ) ;
extern int  core_prim_set_line( int flag, int attribute, char *name,
								FVECTOR *size, FVECTOR *from, FVECTOR *to ) ;
extern int  core_prim_set_poly( int flag, int attribute, char *name,
								FVECTOR *base,
								FVECTOR *p0, FVECTOR *p1,
								FVECTOR *p2, FVECTOR *p3 );
extern int  core_prim_set_box( int flag, int attribute, char *name,
							   FVECTOR *size, FVECTOR *pos ) ;
extern int  core_prim_set_field( int flag, int attribute, char *name,
								 FVECTOR *size, FVECTOR *pos ) ;


/*  geo_memory.c */
extern void *mem_alloc_align( int size, int align_bit ) ;
extern void *mem_alloc( int size ) ;
extern void *mem_get_mem( void ) ;
extern void *mem_get_mem_align( int align_bit ) ;
extern int   mem_tell( void ) ;
extern int   mem_tell_align( int align_bit ) ;
extern void  mem_startup( void ) ;
extern void  mem_shutdown( void ) ;

extern void *mem_malloc_align( int size, int align_bit ) ;
extern void *mem_malloc( int size ) ;
extern void  mem_free( void *ptr ) ;


/* コンバート関連 geo_convert.c */
extern GEO_DEF *convert_geodef( HP3DMODEL model ) ;


/* POLYFUNC集 geo_poly.c */
extern int poly_bound( HP3DOBJECT obj, void *data ) ;
extern int poly_floor( HP3DOBJECT obj, void *data ) ;
extern int poly_hazard( HP3DOBJECT obj, void *data ) ;
extern int poly_trap( HP3DOBJECT obj, void *data ) ;
extern int poly_dump( HP3DOBJECT obj, void *data ) ;


/* ファイル入出力関連 geo_file.c */
extern int file_geo_save( GEO_DEF *def, char *file_name ) ;
extern GEO_DEF *file_geo_load( char *file_name ) ;
extern HP3DMODEL file_scn_load( char *file_name ) ;
extern void file_dump_radix(GEO_DEF *def ) ;


#endif /*__SCN2GEO_H__*/
