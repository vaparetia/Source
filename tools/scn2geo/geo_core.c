/*
   geo_core.c

   GEOコンバート用コア プログラム
   
   2002/11/5 T.Morita

   $Id: geo_core.c,v 1.17 2002/12/11 05:36:22 usr04098 Exp $

   COREの中身は,かなりぐちゃぐちゃになっちゃいました。申し訳ない。

   */

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<math.h>
#include	<sys/types.h> 

#include	"scn2geo.h"



int g_unit_size[] = {
    0,
    sizeof(int)*2,
    sizeof(int)*2,
    0,

    0,
    sizeof(int)*2,
    0,
    0,
} ;
int g_unit_n_types = sizeof(g_unit_size)/sizeof(int) ;
int g_type_size[] = { /*int GEO_PrimTypeSize[] と同じです */

    0, /* PRIM_NONE */
    sizeof(GEO_DOT ),
    sizeof(GEO_LINE),
    sizeof(GEO_POLY),
    sizeof(GEO_BOX ),
    sizeof(GEO_FIELD ),
} ;


#define OFFSET(_head,_ptr) ((void *)((u_int)(_ptr) - (u_int)(_head)))
#define ADDR(_head,_ptr)   ((void *)((u_int)(_ptr) + (u_int)(_head)))




int vvv_real_vert = 0 ;
int vvv_regist = 0 ;
int vvv_n_geom = 0 ;
int vvvv_poly = 0 ;
int vvvv_block = 0 ;
int vvvv_block_blank = 0;


#define BIT_LEN		24

int core_strcode( char *string )
{
	unsigned char c;
	unsigned char *p;
	unsigned int id, mask;

	p = ( unsigned char * )string;
	id = 0;
	mask = ( 1 << BIT_LEN ) - 1;

	while( ( c = *( p++ ) ) != '\0' ){
		id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
		id += c;
		id &= mask;
	}
	if( id == 0 ) id = 1;

	return id ;
}

/*---------------------------------------------------------------------------


  Primitive 関連


  ---------------------------------------------------------------------------*/

void core_prim_calc_normal( FVECTOR *n,	FVECTOR *p0, FVECTOR *p1, FVECTOR *p2 )
{
    FVECTOR a0, a1, v ;

    SUB_FVECTOR( &a0, p1, p0 ) ;
    SUB_FVECTOR( &a1, p2, p1 ) ;
    OUTER_PRODUCT( &v, &a0, &a1 ) ;
    NORMALIZE( (FVECTOR *)n, &v ) ;
}

void core_prim_calc_inner_center( SVECTOR *center, FVECTOR *base,
								  FVECTOR *p0, FVECTOR *p1, FVECTOR *p2 )
{
    FVECTOR pos[3] ;
    FVECTOR ab, bc, ca ;
    FVECTOR cen ;
    float a, b, c, d ;
    float s, r/* 内心半径 */ ;

    SUB_FVECTOR( &pos[0], p0, base ) ;
    SUB_FVECTOR( &pos[1], p1, base ) ;
    SUB_FVECTOR( &pos[2], p2, base ) ;

    /* 3辺の距離を求める */
    SUB_FVECTOR( &ab, &pos[1], &pos[0] ) ;
    SUB_FVECTOR( &bc, &pos[2], &pos[1] ) ;
    SUB_FVECTOR( &ca, &pos[0], &pos[2] ) ;
    a = sqrtf( INNER_PRODUCT( &bc, &bc ) ) ;
    b = sqrtf( INNER_PRODUCT( &ca, &ca ) ) ;
    c = sqrtf( INNER_PRODUCT( &ab, &ab ) ) ;

    /* 面積の等式より s*r == ヘロンの公式(SQR(s*(s-a)*(s-b)*(s-c)))  */
    s = 0.5f*(a + b + c) ;
    r = sqrtf( (s-a)*(s-b)*(s-c)/s ) ;

	/* 内心点cen は 内角２等分線の r(内心半径) の大きさのところに位置
	   する。Bの内角２等分線 と ACの交点をEとする。
		   B	   s = (a+b+c)/2
		  /|\	   |AE| = (s-a)/b なので
	   a / | \ c	AE = AC*(s-a)/b
		/  |  \		BE = AE-AB
	   C---E---A	BI = BE*(|BE|-r)/|BE|
	   b I = BI+B */
    SCALE_FVECTOR( &cen, &ca, (a - s) / b ) ;  /* AE = -CA*(s-a)/b */
    SUB_FVECTOR( &cen, &cen, &ab ) ;           /* BE = AE-AB      */
    d = sqrtf( INNER_PRODUCT( &cen, &cen ) ) ; /* d = |BE|        */
    SCALE_FVECTOR( &cen, &cen, (d-r)/d ) ;     /* BI = BE*(d-r)/d */
    ADD_FVECTOR( center, &cen, &pos[1] ) ;     /* I  = BI+B       */

    /* 内心半径も更新する */
    center->pad = r ;
}

int core_prim_set_dot( int flag, int attribute, char *name,
					   FVECTOR *size, FVECTOR *pos )
{
    GEO_DOT *p ;
    GEOM *g ;

    g = core_geom_getone( GEO_F_PRIM_DOT|GEO_F_STATIC|flag, 0 ) ;
    if ( g == NULL ) return 0 ;
    p = (void *)(g + 1) ;

    if ( size ) COPY_FVECTOR( &p->size, size ) ;
    if ( pos ) 	COPY_FVECTOR( &p->pos , pos  ) ;

    p->size.attribute = attribute ;

	g->name = core_strcode( name ) ;

    return 1 ;
}

int core_prim_set_line( int flag, int attribute, char *name,
						FVECTOR *size, FVECTOR *from, FVECTOR *to )
{
    GEO_LINE *p ;
    GEOM *g ;

    g = core_geom_getone( GEO_F_PRIM_LINE|GEO_F_STATIC|flag, 0 ) ;
    if ( g == NULL ) return 0 ;
    p = (void *)(g + 1) ;

    if ( size ) COPY_FVECTOR( &p->size, size ) ;
    if ( from ) COPY_FVECTOR( &p->from, from ) ;
    if ( to )   COPY_FVECTOR( &p->to  , to   ) ;

    p->size.attribute = attribute ;

	g->name = core_strcode( name ) ;

    return 1 ;
}


int core_prim_set_poly_index( GEO_POLY *p, void *ref, int index )
{
    u_int offset ;

    offset  = (u_int)ref - (u_int)p ;
    offset /= sizeof(SVECTOR) ;
    if ( offset > 0x3ff ) {
		printf( "prim_poly exceed offset. ofst%x ref%x prim%x\n", offset, ref, p ) ;
    }

    if ( index==0 ){
		SVECTOR *sv = (void*)p + sizeof(SVECTOR)*offset ;
    }

    switch( index ) {
    case 0:
		p->index.low  |= (offset & 0x3ff)<<0 ;
		break ;
    case 1:
		p->index.low  |= (offset & 0x3ff)<<10 ;
		break ;
    case 2:
		p->index.low  |= (offset & 0x3ff)<<20 ;
		break ;
    case 3:
		p->index.low  |= (offset & 0x003)<<30 ;
		p->index.high |= (offset & 0x3ff)>>2  ;
		break ;
    case 4:
		p->index.high |= (offset & 0x3ff)<< 8 ;
		break ;
    case 5:		       
		p->index.high |= (offset & 0x3ff)<<18 ;
		break ;
    }
}

int core_prim_set_poly( int flag, int attribute, char *name,
						FVECTOR *base,
						FVECTOR *p0, FVECTOR *p1, FVECTOR *p2, FVECTOR *p3 )
{
    GEO_POLY *p ;
    GEOM     *g ;
    FVECTOR   n ;

    core_prim_calc_normal( &n, p0, p1, p2 ) ;

    g = core_geom_getone( GEO_F_PRIM_POLY|GEO_F_STATIC|flag, 0 ) ;
    if ( g == NULL ) return 0 ;
    p = (void *)(g + 1) ;
    bzero( p, sizeof(GEO_POLY) ) ;

	/* 共有頂点化するため登録する */
    core_vertex_base( base, p, 5 ) ;
    if ( p0 ) core_vertex_point( p0, p, 1 ) ;
    if ( p1 ) core_vertex_point( p1, p, 0 ) ;
    if ( p2 ) core_vertex_point( p2, p, 2 ) ;
    if ( p3 ) core_vertex_point( p3, p, 3 ) ;
    core_vertex_normal( &n, p, 4 ) ;

	/* 内心点の計算 */
    if ( p0 && p1 && p2 ) {
		core_prim_calc_inner_center( &p->cen1, base, p0, p1, p2 ) ;
	}
    if ( p1 && p2 && p3 ) {
		core_prim_calc_inner_center( &p->cen2, base, p0, p2, p3 ) ;
	}

    p->attribute = attribute ;

	g->name = core_strcode( name ) ;

	vvvv_poly++ ;

    return 1 ;
}

int core_prim_set_box( int flag, int attribute, char *name,
					   FVECTOR *size, FVECTOR *pos )
{
    GEO_BOX *p ;
    GEOM *g ;

    g = core_geom_getone( GEO_F_PRIM_BOX|GEO_F_STATIC|flag, 0 ) ;
    if ( g == NULL ) return 0 ;
    p = (void *)(g + 1) ;
    bzero( p, sizeof(GEO_BOX) ) ;

    if ( size )	COPY_FVECTOR( &p->size, size ) ;
    if ( pos )  COPY_FVECTOR( &p->pos , pos  ) ;

	printf( "setbox p(%.1f %.1f %.1f) sz(%.1f %.1f %.1f)\n", pos->vx, pos->vy, pos->vz, size->vx, size->vy, size->vz ) ;

    p->size.attribute = attribute ;

	g->name = core_strcode( name ) ;

    return 1 ;
}

int core_prim_set_field( int flag, int attribute, char *name,
						 FVECTOR *size, FVECTOR *pos )
{
    GEO_FIELD *p ;
    GEOM *g ;

    g = core_geom_getone( GEO_F_PRIM_FIELD|GEO_F_STATIC|flag, 0 ) ;
    if ( g == NULL ) return 0 ;
    p = (void *)(g + 1) ;

    if ( size )	COPY_FVECTOR( &p->size, size ) ;
    if ( pos )  COPY_FVECTOR( &p->pos , pos  ) ;

	printf( "setfield p(%.1f %.1f %.1f) sz(%.1f %.1f %.1f)\n", pos->vx, pos->vy, pos->vz, size->vx, size->vy, size->vz ) ;

#if 1
    p->size.attribute = attribute ;
#else
    p->attribute = attribute ;
#endif

	g->name = core_strcode( name ) ;

    return 1 ;
}




/*---------------------------------------------------------------------------


  Block 関連


  ---------------------------------------------------------------------------*/
static GEO_BLK_WORK blk_work ALIGN16 ;

static inline int core_block_is_fitblock( GEO_BLK_POOL *b, int size, int extra )
{
    return GEO_BLOCK_SIZE - b->extra_size - b->block.size > size + extra ;
}

/* サイズに合うGEO_BLOCKを探す */
GEO_BLK_POOL *core_block_getblock( int size, int extra )
{
    GEO_BLK_POOL *b = blk_work.pool ;

    while( 1 ){	
		if ( core_block_is_fitblock( b, size, extra ) ) {
			if ( b->block.geom == NULL ) {
				b->block.geom = mem_malloc( GEO_BLOCK_SIZE ) ;
			}
			return b ;
		}
		if ( b->block.flag & GEO_BLK_END ) break ; /* 終端フラグ */
		b++ ;
    }
    printf( "Cannot find BLOCK for size %d\n", size ) ;
    return NULL ;
}

/* サイズに適したGEO_BLOCKを探し メモリを確保する */
void *core_block_malloc( GEO_BLK_POOL *b, int size, int align_bit )
{
    int mask = (1 << align_bit) - 1 ;
    void *addr ;

    if ( b ) {
		b->block.size = (b->block.size + mask) & ~mask ;
		addr = (void *)((u_int)b->block.geom + b->block.size) ;
		bzero( addr, size ) ;
		b->block.size += size ;
    } else {
		printf( "geo_geom.c : Cannot malloc from GEO_BLOCK\n" ) ;
		addr = NULL ;
    }

	printf( "geo_geom.c : blockmalloc head%x size%d extsiz%d\n", b->block.geom, b->block.size, b->extra_size ) ;

    return addr ;
}

/* サイズに適したGEO_BLOCKを探し メモリを確保する */
void *core_block_extramalloc( GEO_BLK_POOL *b, int size )
{
    void *addr ;

    if ( b ) {
		b->extra_size += size ;
		addr = (void *)((u_int)b->block.geom + GEO_BLOCK_SIZE - b->extra_size) ;
		bzero( addr, size ) ;
    } else {
		printf( "geo_geom.c : Cannot extramalloc from GEO_BLOCK\n" ) ;
		addr = NULL ;
    }
    return addr ;
}

void core_block_set_align( int align )
{
    blk_work.reset_align = align ;
}

/* poolを初期化する */
void core_block_start( int type, float x, float y, float z )
{
    int i ;
    GEO_BLK_POOL *b = blk_work.pool ;
    void *addr ;

#if 0
    addr = mem_malloc( GEO_BLOCK_SIZE*GEO_BLOCK_MAX ) ;
    if ( addr == NULL ) {
		printf( "no memroy for block_start\n" ) ;
		return ;
    }
#endif

    for ( i=GEO_BLOCK_MAX ; --i>=0 ; ){
		b->block.flag   = GEO_PLG_TYPESET(type) ;
		b->block.size   =  0 ;
		b->block.tail   = -1 ;
		b->block.geom   = NULL ; /* 後でchankに付け直すので mallocで確保する */
		b->block.n_geom = 0 ;
		b->extra_size   = 0 ;

		addr += GEO_BLOCK_SIZE ;
		/*reset_alignのメンバは core_group_addの時に16に設定される */

		b++ ;
    }
    blk_work.pool[GEO_BLOCK_MAX-1].block.flag |= GEO_BLK_END ;

    /* 頂点プールの初期化 */
    core_vertex_start( x, y, z ) ;
}

/* poolからひと続きのメモリに展開し直す */
GEO_BLOCK *core_block_end( void )
{
    GEO_BLK_POOL *b ;
    GEO_BLOCK    *block, *new ;
    int           b_size, g_size ;

    /* GEOMを各ブロックに割り当てる */
    core_geom_reshape() ;

	/* ブロックには何も溜っていない */
	if ( blk_work.pool[0].block.size <= 0 ) return NULL ;

    /* サイズの取得 */
    b_size = 0 ;
	b = blk_work.pool ;
    for ( ; ; ) {
		b_size += sizeof(GEO_BLOCK) ;
		vvvv_block++ ;
		if ( b->block.flag & GEO_BLK_END ) break ; /* 終端フラグ */
		if ( (b+1)->block.size <= 0 ) break ;/* 次のブロックは空だから */
		b++ ;
    }
	b->block.flag |= GEO_BLK_END ; /* 終端フラグを入れておく */

    /* ブロックの一発めだけALIGNされるように こうなっている */
    block = new = mem_alloc_align( b_size, blk_work.reset_align ) ;
    if ( block == NULL ) {
		printf( "no memory for blockhead\n" ) ;
		return NULL ;
	}

	/* GEO_BLOCKの部分だけチャンクメモリに入れていく */
	for ( b=blk_work.pool ; ; b++ ) {
		if ( b->extra_size > 0 ) {
			int align_size ;

			align_size = (b->block.size + 0xf) & 0xfffffff0 ;

			/* サイズを詰める extraは後から埋まるので空きが出てしまう */
			core_vertex_movedest( (void*)b->block.geom + align_size,
								  (void*)b->block.geom + GEO_BLOCK_SIZE - b->extra_size,
								  b->extra_size ) ;
			b->block.size = align_size + b->extra_size ;
		}
		memcpy( new++, &b->block, sizeof(GEO_BLOCK) ) ;

		if ( b->block.flag & GEO_BLK_END ) break ; /* 終端フラグ */
	}
	/* これ以降のブロック割り当てはアラインメントなしにする */
	blk_work.reset_align = 0 ;

    /* 頂点プールの終了 */
    core_vertex_end() ;
	printf( "block end %d %d\n", block->size, b_size ) ;

    return block ;
}

/* チャンクに加えている */
void core_block_reshape( GEO_BLOCK *block )
{
    GEO_BLOCK *b ;

    printf( "reshape block start\n" ) ;

	/* 全てのブロックに対してGEOMの本体をチャンクメモリーに入れていく */
	if ( !(block->size<=0 && block->flag & GEO_BLK_END) ){
		for ( b=block ; b->size>0 ; b++ ) {
			void *old = b->geom ;

			/* GEOM本体のメモリ確保 */
			b->geom = mem_alloc( b->size ) ;
			if ( b->geom == NULL ) {
				printf( "block no memory!!!\n" ) ;
				return ;
			}
			/* GEOM本体のコピー */
			memcpy( b->geom, old, b->size ) ;    

			/* 昔のは解放する */
			mem_free( old ) ; /* core_block_start で確保されていたもの */

			if ( b->flag & GEO_BLK_END ) break ; /* 終端フラグ */
		}
	}

    printf( "reshape block done\n" ) ;
}

void core_block_addr_to_offset( GEO_BLOCK *block, u_int head )
{
    while( 1 ) {
		block->geom = (void *)((u_int)block->geom - head) ;
		if ( block->flag & GEO_BLK_END ) break ; /* 終端フラグ */
		block++ ;
    }
}
void core_block_offset_to_addr( GEO_BLOCK *block, u_int head )
{
    while( 1 ) {
		block->geom = (void *)((u_int)block->geom + head) ;
		if ( block->flag & GEO_BLK_END ) break ; /* 終端フラグ */
		block++ ;
    }
}




/*---------------------------------------------------------------------------


  Vertex Pool 関連
  
  頂点プール


  ---------------------------------------------------------------------------*/
GEO_VTX_WORK vtx_work ;

void core_vertex_start( float x, float y, float z )
{
    GEO_VTX_WORK *work = &vtx_work ;

    /* 全部クリアする */
    bzero( work, sizeof(GEO_VTX_WORK) );
}

void core_vertex_end( void )
{
    int           i ;
    GEO_VTX_WORK *work = &vtx_work ;
    GEO_VTX_POOL *p ;

    p = work->pool ;
    for ( i=work->n_pool ; --i>=0 ; p++ ) {
		if ( p->dest ) {
			core_prim_set_poly_index( p->ref, p->dest, p->ref_data ) ;
		} else {
			printf( "Must not be happened.p[%x] ref%x dest%x ref_dat%x %d\n", p, p->ref, p->dest, p->ref_data, p->fv_sv_flag ) ;
		}
    }    
}

#define ISSAME_VECTOR( _a, _b ) \
( fabs((float)((_a)->vx - (_b)->vx)) < 0.8f &&\
  fabs((float)((_a)->vy - (_b)->vy)) < 0.8f &&\
  fabs((float)((_a)->vz - (_b)->vz)) < 0.8f )

void core_vertex_set_dest( SVECTOR *dest, int max )
{
    GEO_VTX_WORK *work = &vtx_work ;

    work->dest   = dest ;
    work->n_dest = 0    ;
    work->n_dest_max = max ;
}


/* refに当たるものを全て取ってくる 同じものは,統合する */
int vvvv_hit = 0 ;
int core_vertex_fetch_to_dest( void *ref )
{
    int           i, j, k, l ;
    int           count = 0 ;
    GEO_VTX_WORK *work = &vtx_work ;
    GEO_VTX_POOL *p ;

    p = work->pool ;
    for ( i=work->n_pool ; --i>=0 ; p++ ) {
		if ( ref == p->ref ) {

			if ( work->n_dest > work->n_dest_max ) {
				printf( "more than n_dest_max\n" ) ;
				return -1 ;
			}
			if ( work->dest == NULL ){
				printf( "work->dest == NULL !!\n" ) ;
				return -1 ;
			}

			/* 同じベクトルが登録されているかどうかを見てる */
			for( k=0 ; k<work->n_dest ; k++ ) {
				if ( p->fv_sv_flag ?
					 ISSAME_VECTOR( &p->v, (FVECTOR*)(work->dest+k) ) : 
					 ISSAME_VECTOR( &p->v, (SVECTOR*)(work->dest+k) ) ) {
					/* HITしたので そのインデックスを記憶 */
					p->dest = work->dest + k ;
					vvvv_hit += 1+p->fv_sv_flag ;
					goto next_time ;
				}
			}

			/* 新規は destでは後ろから埋まっていく */
			work->dest   -= 1 + p->fv_sv_flag ;
			work->n_dest += 1 + p->fv_sv_flag ;
			count        += 1 + p->fv_sv_flag ;
			p->dest = work->dest ;
			if ( p->fv_sv_flag ) {
				COPY_FVECTOR( (FVECTOR *)work->dest, &p->v ) ;
			} else {
				COPY_FVECTOR( work->dest, &p->v ) ;
			}
		}
    next_time:
    }

    return count ;
}

void core_vertex_moveref( void *from, void *to )
{
    int           i ;
    GEO_VTX_POOL *p ;
    GEO_VTX_WORK *work = &vtx_work ;

    p = work->pool ;
    for ( i=work->n_pool ; --i>=0 ; p++ ) {
		if ( p->ref == from ) {
			p->ref = to ;
		}
    }
}

void core_vertex_movedest( void *to, void *from, int size )
{
    int           i, j ;
    GEO_VTX_POOL *p ;
    GEO_VTX_WORK *work = &vtx_work ;

    to = (void *)(((u_int)to + 0xf) & 0xfffffff0) ;
    memcpy( to, from, size ) ;

    size /= sizeof(SVECTOR) ;
    for ( j=size ; --j>=0 ; ) {
		p = work->pool ;
		for ( i=work->n_pool ; --i>=0 ; p++ ) {
			if ( p->dest == from ) {
				p->dest = to ;
			}
		}
		from = (void *)from + sizeof(SVECTOR) ;
		to   = (void *)to   + sizeof(SVECTOR) ;
    }
}

int core_vertex_count_newsize( void *ref )
{
    int           i, j, k ;
    int           count = 0 ;
    GEO_VTX_POOL *p ;
    GEO_VTX_WORK *work = &vtx_work ;

    p = work->pool ;
    for ( i=work->n_pool ; --i>=0 ; p++ ) {
		if ( ref == p->ref ) {

			if ( p->fv_sv_flag ) {
				for( k=0 ; k<work->n_dest ; k++ ) {
					if ( ((u_int)(work->dest+k) & 8) == 0 ) {
						if ( ISSAME_VECTOR( &p->v, (FVECTOR*)(work->dest+k) ) ) {
							goto next_time ;
						}
					}
				}
			} else {
				for( k=0 ; k<work->n_dest ; k++ ) {
					if ( ISSAME_VECTOR( &p->v, work->dest+k ) ) {
						goto next_time ;
					}
				}
			}

			count += 1 + p->fv_sv_flag ;
			vvv_real_vert++ ;
		}
    next_time:
    }

    return count * sizeof(SVECTOR) ;
}

int core_vertex_regist( GEO_VTX_WORK *work, FVECTOR *v,
						void *ref, int ref_data, int fv_sv_flag )
{
    GEO_VTX_POOL *p ;

    vvv_regist++ ;

    if ( work->n_pool >= MAX_VERTEX_POOL ) {
		printf( "vertex-regist exceed refs %d\n", work->n_pool ) ;
		return -1 ;
    }

    p = work->pool + work->n_pool ;
    /* 各種情報を覚える */
    p->ref        = ref        ; /* 検索のキーになっている */
    p->ref_data   = ref_data   ; /* 参照データの引数 */
    p->fv_sv_flag = fv_sv_flag ; /* 格納方法がSVかFVか */
    p->dest       = NULL       ; /* 格納時のインデックス 初期化 */

    /* ベクトルコピー */
    COPY_FVECTOR( &p->v, v ) ;

    return work->n_pool++ ;
}

/* ベースの登録(pointより先にしなければならない) */
int core_vertex_base( FVECTOR *b, void *ref, int ref_data )
{
    GEO_VTX_WORK *work = &vtx_work ;

    COPY_FVECTOR( &work->base, b ) ;
    return core_vertex_regist( work, b, ref, ref_data, 1 ) ;
}

/* 頂点の登録(baseより後にしなければならない) */
int core_vertex_point( FVECTOR *v, void *ref, int ref_data )
{
    FVECTOR       p ;
    GEO_VTX_WORK *work = &vtx_work ;

    SUB_FVECTOR( &p, v, &work->base ) ;
    return core_vertex_regist( work, &p, ref, ref_data, 0 ) ;
}

/* 法線の登録 */
int core_vertex_normal( FVECTOR *n, void *ref, int ref_data )
{
    FVECTOR       p ;
    GEO_VTX_WORK *work = &vtx_work ;

    SCALE_FVECTOR( &p, n, 32000 ) ;
    return core_vertex_regist( work, &p, ref, ref_data, 0 ) ;
}


/*---------------------------------------------------------------------------

  Geom 関連

  GEOMはGEO_BLOCKからしか取れないようになる。

  ---------------------------------------------------------------------------*/
/* 双方向リストの尻に繋げる */
void core_geom_append( GEOM *geom, GEOM *tail )
{
    if ( tail ) {
		geom->prev = GEO_LINKADDR( geom, tail ) ;
		tail->next = GEO_LINKADDR( tail, geom ) ;
    }
}

/* 双方向リストの頭に繋げる */
void core_geom_connect( GEOM *geom, GEOM *head )
{
    geom->next = GEO_LINKADDR( geom, head ) ;
    if ( head ) {
		geom->prev = head->prev ;
		head->prev = GEO_LINKADDR( head, geom ) ;
    } else {
		geom->prev = 0 ;
	}
}

#if 0

/* 親子関係 */
void core_geom_connect_parent( GEOM *geom, GEOM *parent )
{
    core_geom_connect( geom, GEO_LINKCHILD( parent ) ) ;
    parent->child = GEO_LINKADDR( parent, geom ) ;
}
#endif

int core_geom_get_primsize( u_int flag )
{
    int size ;

    size = g_type_size[GEO_GetPrimType( flag )] ;
    size *= GEO_GET_NPRIM_FROMFLAG( flag ) ;
    return size ;
}

int core_geom_get_size( u_int flag )
{
    int size ;

    size = core_geom_get_primsize( flag ) ;
    size += sizeof(GEOM) ;
    if ( flag & GEO_F_HAS_CALLBACK )
		size += sizeof(GEO_CALLUNIT) ;
	size += g_unit_size[GEO_GetType( flag )] ;

    return size ;
}

/* 同じPRIMを集めるためのGEOMリスト mem_mallocで確保される */
static GEOM *core_geom_head = NULL ;

void core_geom_reshape( void )
{
    GEOM      *g, *cur, *nxt ;
    GEO_BLK_POOL *b_cur, *b_prv, *tmp ;
    int size, extra_size, prim_size ;

    b_prv = NULL ;
    for ( cur=core_geom_head ; cur ; cur=nxt ){
		nxt = GEO_LINKNEXT(cur) ;

		prim_size  = core_geom_get_primsize( cur->flag ) ;
		extra_size = core_vertex_count_newsize( cur+1 ) ;
		b_cur = core_block_getblock( prim_size+sizeof(GEOM), extra_size ) ;

		if ( b_cur != b_prv ) {
			/* 新しいブロック */
			core_vertex_set_dest( (void*)b_cur->block.geom + GEO_BLOCK_SIZE, GEO_BLOCK_SIZE/sizeof(SVECTOR)/2 ) ;
			b_prv = b_cur ;
		}

		/* ブロックに登録 */
		if ( b_cur ) {
			void *addr ;
			GEOM *geom ;

			/* 同じ PRIM を MERGE している */
			geom = core_block_malloc( b_cur, sizeof(GEOM), 4 ) ;
			memcpy( geom, cur, sizeof(GEOM) ) ;
			GEO_CLR_NPRIM( geom ) ;
			for ( g=cur ; g ; g=nxt ){
				if ( (cur->flag & ~GEO_F_HAS_N_PRIM) !=
					 (g  ->flag & ~GEO_F_HAS_N_PRIM) ) {
					printf( "stop!!!!!! because flag is incorrect.\n" ) ;
					break ;
				}
				if ( (cur->name & 0x00ffffff) != (g  ->name & 0x00ffffff) ) {
					printf( "stop!!!!!! because name is incorrect.\n" ) ;
					break ;
				}
				extra_size = core_vertex_count_newsize( g+1 ) ;
				if ( !core_block_is_fitblock( b_cur, prim_size, extra_size) ) {
					printf( "stop!!!!!! because there is no memory in the block.\n" ) ;
					break ;
				}
				if ( GEO_GET_NPRIM( geom ) >= 255 ){
					printf( "stop!!!!!! because prim overfull.\n" ) ;
					break ;
				}
				geom->flag += GEO_F_HAS_N_ONEPRIM ;

				/* 共有頂点のコピー */
				core_block_extramalloc( b_cur, extra_size ) ;
				core_vertex_fetch_to_dest( g+1 ) ;

				/* PRIMのコピー */
				addr = core_block_malloc( b_cur, prim_size, 0 ) ;
				memcpy( addr, g+1, prim_size ) ;
				core_vertex_moveref( g+1, addr ) ;

				nxt = GEO_LINKNEXT(g) ;

				/* メモリを解放。これはgetoneで取られていた細々したGEOM */
				mem_free( g ) ;
			}

			if ( GEO_GET_NPRIM( geom ) == 0 ){
				printf( "NPRIM is 0!!!!!!!!!!!!!!!!!!!!!!!!!!!!g=%x\n", g ) ;
			}

			geom->next = geom->prev = 0 ;/* クリアしておく */
			core_geom_append( geom, GEO_LINKTAIL( b_cur->block.geom,
												  b_cur->block.tail ) ) ;
			b_cur->block.tail = GEO_LINKADDR( b_cur->block.geom, geom ) ;
			b_cur->block.n_geom++ ;
			vvv_n_geom++ ;
		}
    }

    /* 次回のために初期化しておく */
    core_geom_head = NULL ;
}

/* 処理列 */
GEOM *core_geom_getone( u_int flag, int extra_size )
{
    GEOM *g, *one ;
    void *addr ;

    /* ブロックのタイプと同じ */
    flag |= GEO_F_HAS_N_ONEPRIM ;
    if ( g_unit_size[GEO_GetType( flag )] )
		flag |= GEO_F_HAS_UNITDATA ;

    /* 新規に確保 */
    if ( (one = mem_malloc( core_geom_get_size( flag ) )) == NULL ){
		return NULL ;
    }
    one->flag = flag ;

    /* 自分と同じPRIMの入っているGEOMを探す */
    for ( g=core_geom_head ; g ; g=GEO_LINKNEXT(g) ){
		/* かならず同じフラグの先頭に繋がっていく */
		if ( (g->flag & ~GEO_F_HAS_N_PRIM)==(flag & ~GEO_F_HAS_N_PRIM) ) {
			GEOM *nxt = GEO_LINKNEXT(g) ;

			if ( nxt ) nxt->prev = GEO_LINKADDR( nxt, one ) ;
			g->next   = GEO_LINKADDR( g  , one ) ;
			one->next = GEO_LINKADDR( one, nxt ) ;
			one->prev = GEO_LINKADDR( one, g   ) ;

			return one ;
		}
    }

    core_geom_connect( one, core_geom_head ) ;
    core_geom_head = one ;

    return one ;
}




/*---------------------------------------------------------------------------


  Geogroup 関連


  ---------------------------------------------------------------------------*/
int core_group_radix_get_size( GEO_GROUP *g )
{
    return sizeof(GEO_RADIX) +
		sizeof(char) * ((g_unit_n_types * GEO_BLOCK_MAXBITS + 7)/ 8) ;
}

int core_group_radix_get_offset( GEO_RADIX *r, int type )
{
	return GEO_GetRadixOffset( r, type ) ;
}

/*
  RADIXにぶら下がっているBLOCKメモリーをリニアスペースに取り直す
  もっと細かいことをいうとGEOMだけ今はリニアな場所にないのでGEOMを移す。
  geom_core.cで説明したようにGEOMプールは最後なので この処理が最後に入る
 */
void core_group_radix_reshape( GEO_GROUP *g )
{
    int i, j ;

    for ( i=0 ; i<g->max.vx*g->max.vy*g->max.vz ; i++ ){
		GEO_RADIX *radix ;

		radix = (GEO_RADIX *)((u_int)g->radix + g->radix_size * i) ;
		if ( radix->offset == GEO_RADIX_NONE ) continue ;

		for ( j=g_unit_n_types ; --j>=0 ; ){
			int offset ;

			offset = core_group_radix_get_offset( radix, j ) ;
			if ( offset || !j ) {
				core_block_reshape( g->block + radix->offset + offset ) ;
			}
		}
    }
}

/* BLOCKをRADIXに登録 */
int core_group_radix_add_block( GEO_GROUP *g, GEO_BLOCK *b,
								int x, int y, int z )
{
    int        idx, type ;
    GEO_RADIX *radix ;

	/* 登録するブロックはない */
    if ( b==NULL ) return -1 ;

    /* RADIXのアドレスを計算 */
    idx   = x + g->max.vx*z + g->max.vx*g->max.vz*y ;
    radix = (void *)((u_int)g->radix + g->radix_size * idx) ;
    type  = GEO_GetType( b->flag ) ;

    /* メインオフセットがなければ設定する */
    if ( radix->offset == GEO_RADIX_NONE ){
		radix->offset = ((u_int)b - (u_int)g->block) / sizeof(GEO_BLOCK) ;
		printf( "radix%x idx %d block%x bofst%d bsize%d\n", radix, idx, b, radix->offset, b->size ) ;
    }

    /* それぞれのブロックタイプについてオフセットを設定する */
    if ( type ) {
		int offset ;
		int bit = (type%2) * 4 ;

		if ( radix->types[type/2] & (0xf << bit) ){
			printf( "geo_core.c : Type %d is already registed in radix(%d,%d,%d)!!!\n", type, x,y,z ) ;
			return -1 ;
		}

		/* メインオフセットからの差分になる */
		offset = ((u_int)b - (u_int)g->block) / sizeof(GEO_BLOCK) ;
		offset -= radix->offset ;
		radix->types[type/2] |= (offset & 0xf) << bit ;

		printf( "radix%x idx%d blok%x type%dofst%d bofst%d types[%d]<<bit%d\n", radix, idx, b, type, offset, radix->offset, type/2,bit ) ;
    }

    return 0 ;
}

/* グループのメモリを確保する core_group_addでここに追加していく */
GEO_GROUP *core_group_make_groups( int n_groups )
{
    GEO_GROUP *grps ;

    /* グループ分のメモリを確保する(チャンクメモリ) */
    if ( (grps = mem_alloc( sizeof(GEO_GROUP) * n_groups )) ) {
		grps[n_groups-1].flag |= GEO_GRP_END ;
    }
    return grps ;
}

/* グループを追加する */
int core_group_add( GEO_GROUP *g, FVECTOR *div, FVECTOR *max, FVECTOR *base )
{
	int i    ;
    int size ;

    g->base.vx = base->vx ;
    g->base.vy = base->vy ;
    g->base.vz = base->vz ;
    g->div.vx  = div->vx ;
    g->div.vy  = div->vy ;
    g->div.vz  = div->vz ;
    g->max.vx  = max->vx ;
    g->max.vy  = max->vy ;
    g->max.vz  = max->vz ;
    g->n_types = g_unit_n_types ;
    g->radix_size = core_group_radix_get_size( g )  ;

    /* RADIX分のメモリを確保する(チャンクメモリ) */
    size = g->radix_size * max->vx * max->vy * max->vz ;
    if ( !(g->radix = mem_alloc( size )) ){
		return -1 ;
    }
	for ( i=0 ; i<max->vx*max->vy*max->vz ; i++ ) {
		GEO_RADIX *radix ;
		radix = (void*)((u_int)g->radix + g->radix_size*i) ;
		radix->offset = GEO_RADIX_NONE ;
	}

    /* 最初のブロックだけALIGN16になる様に設定する */
    /* ブロックアドレスの計算  ブロックが貯められる最初の場所 */
    g->block = TO_ALIGN16( (u_int)g->radix + size ) ;
    core_block_set_align( 4 ) ;

    return 0 ;
}


/* メモリをオフセットに変える */
int core_group_addr_to_offset( GEO_GROUP *g, u_int head )
{
    while( 1 ) {
		int i, j ;

		for ( i=0 ; i<g->max.vx*g->max.vy*g->max.vz ; i++ ){
			GEO_RADIX *radix ;

			radix = (GEO_RADIX *)((u_int)g->radix + g->radix_size * i) ;
			if ( radix->offset == GEO_RADIX_NONE ) continue ;

			for ( j=g->n_types ; --j>=0 ; ){
				int offset ;

				offset = core_group_radix_get_offset( radix, j ) ;
				if ( offset || !j ) {
					core_block_addr_to_offset( g->block + radix->offset + offset, head ) ;
				}
			}
		}

		/* アドレスからオフセットに変えている */
		g->radix = (void *)((u_int)g->radix - head) ;
		g->block = (void *)((u_int)g->block - head) ;

		if ( g->flag & GEO_GRP_END ) break ;
		g++ ;
    }
}

int core_group_offset_to_addr( GEO_GROUP *g, u_int head )
{
    while( 1 ) {
		int i, j ;

		/* オフセットからアドレスに変えている */
		g->radix = (void *)(head + (u_int)g->radix) ;
		g->block = (void *)(head + (u_int)g->block) ;

		for ( i=0 ; i<g->max.vx*g->max.vy*g->max.vz ; i++ ){
			GEO_RADIX *radix ;

			radix = (GEO_RADIX *)((u_int)g->radix + g->radix_size * i) ;
			if ( radix->offset == GEO_RADIX_NONE ) continue ;
			for ( j=g->n_types ; --j>=0 ; ){
				int offset ;

				offset = core_group_radix_get_offset( radix, j ) ;
				if ( offset || !j ) {
					core_block_offset_to_addr( g->block + radix->offset + offset, head ) ;
				}
			}
		}

		if ( g->flag & GEO_GRP_END ) break ;
		g++ ;
    }
}

/*---------------------------------------------------------------------------


  Geodef 関連


  ---------------------------------------------------------------------------*/

static GEO_CHANK *chank_tmp = NULL ;
int core_geodef_chank_start( GEO_CHANK *chank, int type )
{
    chank_tmp = chank ;
    chank_tmp->size = mem_tell() ;
    chank_tmp->data = mem_get_mem() ;

    return 0 ;
}

int core_geodef_chank_end( void )
{
    if ( chank_tmp == NULL ) {
		printf( "geo_core.c : Illeagal chank end!!No chank-start has found\n" ) ;
		return -1 ;
    }
    chank_tmp->size = mem_tell() - chank_tmp->size ;
    printf( "chank size %d\n", chank_tmp->size  ) ;
    chank_tmp = NULL ;

	printf( "\nstatistics\nvvvv_hit/poly=%d/%d real_pnts/regist%d/%d\n",
			vvvv_hit, vvvv_poly,
			vvv_real_vert, vvv_regist ) ;

	printf( "vvv_geoms=%d  blocks(empty)=%d(%d)\n", vvv_n_geom,
			vvvv_block+ vvvv_block_blank,vvvv_block_blank ) ;


    return 0 ;
}

/* オフセットをメモリに変える */
int core_geodef_offset_to_addr( GEO_DEF *def )
{
    GEO_CHANK *c = def->chanks ;
    int        i ;

    for( i=0 ; i<def->n_chanks ; i++, c++ ) {
		c->data = (void *)((u_int)c->data + (u_int)def ) ;
		if ( i== 0 ) {
			core_group_offset_to_addr( c->data, (u_int)def ) ;
		}
    }
    return 0 ;
}

int core_geodef_addr_to_offset( GEO_DEF *def )
{
    GEO_CHANK *c = def->chanks ;
    int        i ;

    for( i=0 ; i<def->n_chanks ; i++, c++ ) {
		if ( i== 0 ) {
			core_group_addr_to_offset( c->data, (u_int)def ) ;
		}
		c->data = (void *)((u_int)c->data - (u_int)def) ;
    }
    return 0 ;
}
