/*
   MDU_mdlKm42Kms.c

   Ｋｍ４ → Ｋｍｓコンバート関数

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 11/11/1999

   $Id: MDU_mdlKm42Kms.c,v 1.17 2000/02/15 07:26:40 usr02011 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <SFDLINUX.h>

#include "METALGEAR.h"
#include "fmt_km4.h"

#include "MDU_util.h"
#include "MDU_mdl.h"

#include "MDU_mdlKm42Kms.h"

static	WORK	work ;

#include "MDU_mdlKm32KmsUtil.c"

#define N_MAX_POINTS 64

/*------------------------------------------------------------------*/

/* Ｋｍ４ → Ｋｍｓ変換 */

/* KM4_DEF2 の開放 */
void		MDU_mdlFreeKm4Def2( def2 )
KM4_DEF2	*def2 ;
{
    int		i ;
    KM4_MDL	*mdl ;

    mdl = def2->def->models ;
    for ( i = 0; i < def2->def->n_x_models; i ++, mdl ++ ) {
	if ( mdl->verts != NULL ) free( mdl->verts ) ;
	if ( mdl->norms != NULL ) free( mdl->norms ) ;
	if ( mdl->envs != NULL ) free( mdl->envs ) ;
	if ( mdl->vid != NULL ) free( mdl->vid ) ;
	if ( mdl->nid != NULL ) free( mdl->nid ) ;
	if ( mdl->tid != NULL ) free( mdl->tid ) ;
	if ( mdl->uvs != NULL ) free( mdl->uvs ) ;
	if ( mdl->vert_usrdata != NULL ) free( mdl->vert_usrdata ) ;
	if ( mdl->norm_usrdata != NULL ) free( mdl->norm_usrdata ) ;
	if ( mdl->prim_usrdata != NULL ) free( mdl->prim_usrdata ) ;
    }
    free( def2->def ) ;
    free( def2 ) ;
}

/* 三角形分割 */
/* ついでに、テクスチャＩＤをＳＴＲＣＯＤＥ変換 */
static	KM4_DEF2	*Km4DividePrimTriangle( olddef2 )
KM4_DEF2		*olddef2 ;
{
    KM4_DEF2	*def2 ;
    KM4_DEF	*olddef, *def ;
    KM4_MDL	*oldmdl, *mdl ;
    int		size, n_x_models, i, j ;
    long      	n_tris;           /*三角形の数*/
    u_short   	*oldvid, *vid ;    
    u_short   	*oldnid, *nid ;
    TVECTOR   	*olduvs, *uvs ;
    u_int   	*oldtid, *tid ;
    long      	oldnum, num ;      /*頂点番号、法線番号、座標ポインタの位置*/
    long      	tri_d ;            /*座標テクスチャポインタの位置*/

    /* 追加 */
    PRIMITIVE_USERDATA *prim_usrdata, *old_prim_usrdata ;


    def2 = ( KM4_DEF2 * )malloc( sizeof( KM4_DEF2 ) ) ;
    if ( def2 == NULL ) return NULL ;
    olddef = olddef2->def ;
    n_x_models = olddef->n_x_models ;
    size = sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * n_x_models ;
    def = ( KM4_DEF * )malloc( size ) ;
    if ( def == NULL ) return NULL ;
    memcpy( def2, olddef2, sizeof( KM4_DEF2 ) ) ;
    memcpy( def, olddef, size ) ;

    def2->def = def ;

    oldmdl = olddef->models ;
    mdl = def->models ;
    for ( i = 0; i < n_x_models; i ++ ) {
	oldvid = oldmdl->vid ;
	oldnid = oldmdl->nid ;
	olduvs = oldmdl->uvs ;
	oldtid = oldmdl->tid ;

	/* 追加 */
	old_prim_usrdata = oldmdl->prim_usrdata ;

	n_tris = 0 ;
	oldnum = 0 ;
	/*   分割後のn_tris の値を求める */
	for( j = 0; j < oldmdl->n_prims ; j ++, oldnum += 4 ) {
	    if( oldvid[ oldnum + 2 ] == oldvid[ oldnum + 3 ] ) n_tris ++ ;
	    else n_tris += 2 ;
	}	
	/*  n_tris分の領域を確保*/
	vid = ( u_short * )malloc( sizeof( u_short ) * n_tris * 4 ) ;
	if ( vid == NULL ) return NULL ;
	nid = ( u_short * )malloc( sizeof( u_short ) * n_tris * 4 ) ;
	if ( nid == NULL ) return NULL ;
	uvs = ( TVECTOR * )malloc( sizeof( TVECTOR ) * n_tris * 4 ) ;
	if ( uvs == NULL ) return NULL ;
	tid = ( u_int * )malloc( sizeof( u_int ) * n_tris ) ;
	if ( tid == NULL ) return NULL ;
	mdl->vid = vid ;
	mdl->nid = nid ;
	mdl->uvs = uvs ;
	mdl->tid = tid ;

	/* 追加 */
	prim_usrdata = ( PRIMITIVE_USERDATA * )malloc( sizeof( PRIMITIVE_USERDATA ) * n_tris ) ;
	if ( prim_usrdata == NULL ) return NULL ;
	mdl->prim_usrdata = prim_usrdata;

	mdl->verts = ( FVECTOR * )malloc( sizeof( FVECTOR ) * oldmdl->n_verts ) ;
	if ( mdl->verts == NULL ) return NULL ;
	memcpy( mdl->verts, oldmdl->verts, sizeof( FVECTOR ) * oldmdl->n_verts ) ;
	mdl->envs = ( FVECTOR * )malloc( sizeof( FVECTOR ) * oldmdl->n_verts ) ;
	if ( mdl->envs == NULL ) return NULL ;
	memcpy( mdl->envs, oldmdl->envs, sizeof( FVECTOR ) * oldmdl->n_verts ) ;
	mdl->norms = ( FVECTOR * )malloc( sizeof( FVECTOR ) * oldmdl->n_norms ) ;
	if ( mdl->norms == NULL ) return NULL ;
	memcpy( mdl->norms, oldmdl->norms, sizeof( FVECTOR ) * oldmdl->n_norms ) ;

	/* 追加 */
	mdl->vert_usrdata
	    = ( VERTEX_USERDATA * )malloc( sizeof( VERTEX_USERDATA ) * oldmdl->n_verts ) ;
	if ( mdl->vert_usrdata == NULL ) return NULL ;
	memcpy( mdl->vert_usrdata, oldmdl->vert_usrdata,
		sizeof( VERTEX_USERDATA ) * oldmdl->n_verts ) ;
	mdl->norm_usrdata
	    = ( NORMAL_USERDATA * )malloc( sizeof( NORMAL_USERDATA ) * oldmdl->n_norms ) ;
	if ( mdl->norm_usrdata == NULL ) return NULL ;
	memcpy( mdl->norm_usrdata, oldmdl->norm_usrdata,
		sizeof( NORMAL_USERDATA ) * oldmdl->n_norms ) ;

	tri_d = 0 ;
	num = 0 ;
	oldnum = 0 ;
	for( j = 0; j < oldmdl->n_prims ; j ++, oldnum += 4 ) {
	/* 頂点番号列の３番目と４番目を比較し、三角形かどうかを判断する */
	    if( oldvid[ oldnum + 2 ] == oldvid[ oldnum + 3 ] ) {
		/*三角形の処理 */
		/* ポリゴンデータを移す */
		tid[ tri_d ] = oldtid[ j ] ;
		vid[ num ] = oldvid[ oldnum ] ; 
		nid[ num ] = oldnid[ oldnum ] ;
		uvs[ num ] = olduvs[ oldnum ] ;
		vid[ num + 1 ] = oldvid[ oldnum + 1 ] ; 
		nid[ num + 1 ] = oldnid[ oldnum + 1 ] ;
		uvs[ num + 1 ] = olduvs[ oldnum + 1 ] ;
		vid[ num + 2 ] = oldvid[ oldnum + 2 ] ; 
		nid[ num + 2 ] = oldnid[ oldnum + 2 ] ;
		uvs[ num + 2 ] = olduvs[ oldnum + 2 ] ;
		vid[ num + 3 ] = oldvid[ oldnum + 3 ] ; 
		nid[ num + 3 ] = oldnid[ oldnum + 3 ] ;
		uvs[ num + 3 ] = olduvs[ oldnum + 3 ] ;

		/* 追加 */
		prim_usrdata[ tri_d ] = old_prim_usrdata[ j ] ;

		tri_d ++ ;
		num += 4 ;
	    } else {
		tid[ tri_d ] = oldtid[ j ] ;
		vid[ num ] = oldvid[ oldnum ] ; 
		nid[ num ] = oldnid[ oldnum ] ;
		uvs[ num ] = olduvs[ oldnum ] ;
		vid[ num + 1 ] = oldvid[ oldnum + 1 ] ; 
		nid[ num + 1 ] = oldnid[ oldnum + 1 ] ;
		uvs[ num + 1 ] = olduvs[ oldnum + 1 ] ;
		vid[ num + 2 ] = oldvid[ oldnum + 2 ] ; 
		nid[ num + 2 ] = oldnid[ oldnum + 2 ] ;
		uvs[ num + 2 ] = olduvs[ oldnum + 2 ] ;
		vid[ num + 3 ] = oldvid[ oldnum + 2 ] ; 
		nid[ num + 3 ] = oldnid[ oldnum + 2 ] ;
		uvs[ num + 3 ] = olduvs[ oldnum + 2 ] ;

		/* 追加 */
		prim_usrdata[ tri_d ] = old_prim_usrdata[ j ] ;

		tri_d ++ ;
		num += 4 ;
		tid[ tri_d ] = oldtid[ j ] ;
		vid[ num ] = oldvid[ oldnum ] ; 
		nid[ num ] = oldnid[ oldnum ] ;
		uvs[ num ] = olduvs[ oldnum ] ;
		vid[ num + 1 ] = oldvid[ oldnum + 2 ] ; 
		nid[ num + 1 ] = oldnid[ oldnum + 2 ] ;
		uvs[ num + 1 ] = olduvs[ oldnum + 2 ] ;
		vid[ num + 2 ] = oldvid[ oldnum + 3 ] ; 
		nid[ num + 2 ] = oldnid[ oldnum + 3 ] ;
		uvs[ num + 2 ] = olduvs[ oldnum + 3 ] ;
		vid[ num + 3 ] = oldvid[ oldnum + 3 ] ; 
		nid[ num + 3 ] = oldnid[ oldnum + 3 ] ;
		uvs[ num + 3 ] = olduvs[ oldnum + 3 ] ;

		/* 追加 */
		prim_usrdata[ tri_d ] = old_prim_usrdata[ j ] ;

		tri_d ++ ;
		num += 4 ;		
	    }
	}
	mdl->n_prims = n_tris ;
#if 1
	for ( j = 0; j < n_tris; j ++ ) {
//	    printf( "%s : ", olddef2->texs[ mdl->tid[ j ] ] ) ;
	    mdl->tid[ j ] = MDU_GetStrCode( olddef2->texs[ mdl->tid[ j ] ] ) ;
//	    printf( "%d\n", mdl->tid[ j ] ) ;
	}
#endif
	mdl ++ ; oldmdl ++ ;
    }
    return def2 ;
}


static	void Strip(void)
{
    long        i;
    DG_MDL      *mdl;
    NEW_OBJECT  *obj;
    KMS_DEF	*sdef ;
    CV2_OBJS	*cdef ;
    CV2_OBJ	*cobj ;
    long     n_x_models;
    int ans=1;

    n_x_models = work.dg_def.n_x_models;
    mdl = work.dg_mdl;

    sdef = work.sdef = ( KMS_DEF * )malloc( sizeof( KMS_DEF ) +
					    sizeof( KMS_OBJ ) * n_x_models ) ;
    if ( sdef == NULL ) return ;

    /* 追加 */
    cdef = work.cdef = ( CV2_OBJS * )malloc( sizeof( CV2_OBJS ) +
					     sizeof( CV2_OBJ ) * n_x_models ) ;
    if ( cdef == NULL ) return ;
    memset( cdef, 0x00, sizeof(CV2_OBJS) + sizeof(CV2_OBJ) * n_x_models );

    work.new_obj = sdef->objs ;
    cobj = cdef->objs ;

    obj  = work.new_obj ;

    for(i=0;i<n_x_models;i++){
	printf("Obj %d = %d Prims\n",(int)i,(mdl+i)->n_prims);
    }

    fprintf(stderr,"%4ld /",n_x_models);
    for(i = 0; i < n_x_models; i++,mdl++,obj++,cobj++)
    {
	if(!Make_Strip(mdl,obj,cobj)) ans=0;
	fprintf(stderr,"%4ld,",i);
    }
    fprintf(stderr,"\n");
	
    sdef->n_models = work.dg_def.n_models;
    sdef->n_x_models = work.dg_def.n_x_models;
    sdef->lx       = (float) work.dg_def.lx;
    sdef->ly       = (float) work.dg_def.ly;
    sdef->lz       = (float) work.dg_def.lz;
    sdef->ux       = (float) work.dg_def.ux;
    sdef->uy       = (float) work.dg_def.uy;
    sdef->uz       = (float) work.dg_def.uz;

    cdef->n_objs   = n_x_models ;
    cdef->flag     = COMMON_VERTS|COMMON_NORMS|COMMON_UVS|COMMON_VERT_USRDATA|COMMON_NORM_USRDATA;

    if(!ans){
	printf("Error : Not converted mdl to cv2.\n");
	FreeCV2OBJS(cdef);
	work.cdef=NULL;
    }

//    memcpy( &( work.new_def ), sdef, sizeof( KMS_DEF ) ) ;

#if 0
    /* 領域開放 */
    for(i = 0; i < n_x_models; i++)
    {
	free(work.dg_mdl[i].verts);
	free(work.dg_mdl[i].norms);
    }
    free(work.dg_mdl);
#endif
    
}


/* ------------------------------------------------------------------------------ */
/* 追加                                                                           */
/* ------------------------------------------------------------------------------ */

static void FreeCV2OBJS(cobjs)
CV2_OBJS *cobjs;
{
    if(cobjs!=NULL){
	int i;
	for(i=0;i<cobjs->n_objs;i++){
	    if(cobjs->objs[i].verts!=NULL) free(cobjs->objs[i].verts);
	    if(cobjs->objs[i].verts_index!=NULL) free(cobjs->objs[i].verts_index);
	    if(cobjs->objs[i].norms!=NULL) free(cobjs->objs[i].norms);
	    if(cobjs->objs[i].norms_index!=NULL) free(cobjs->objs[i].norms_index);
	    if(cobjs->objs[i].uvs!=NULL) free(cobjs->objs[i].uvs);
	    if(cobjs->objs[i].uvs_index!=NULL) free(cobjs->objs[i].uvs_index);

	    if(cobjs->objs[i].vert_usrdata!=NULL) free(cobjs->objs[i].vert_usrdata);
	    if(cobjs->objs[i].norm_usrdata!=NULL) free(cobjs->objs[i].norm_usrdata);
	}

	free(cobjs);
    }
}

#if 0

static int GetCV2OBJ(cobj,mdl,strip)
CV2_OBJ *cobj;
DG_MDL  *mdl;
STRIP   *strip;
{
    int i,j,k;
    int vert_sum,uv_sum;

    struct _check_uv {
	int flag;
	u_short vid;
	u_int   tid;
	TVECTOR uvs;
    } *check_uv;
    int n_check_uv;

    vert_sum=0;
    uv_sum=0;
    for(i = 0; i < work.n_strips; i ++){
	vert_sum+=(strip[i].n_points+1) & ~1;
	uv_sum+=(strip[i].n_points+3) & ~3;
    }

    if((cobj->verts=(FVECTOR *)malloc(sizeof(FVECTOR)*mdl->n_verts))==NULL) return 0;
    cobj->n_verts=mdl->n_verts;
    if((cobj->norms=(FVECTOR *)malloc(sizeof(FVECTOR)*mdl->n_norms))==NULL) return 0;
    cobj->n_norms=mdl->n_norms;
    if((cobj->verts_index=(short *)malloc(sizeof(short)*vert_sum))==NULL) return 0;
    cobj->n_verts_index=vert_sum;
    if((cobj->norms_index=(short *)malloc(sizeof(short)*vert_sum))==NULL) return 0;
    cobj->n_norms_index=vert_sum;

    if((cobj->vert_usrdata
	=(VERTEX_USERDATA *)malloc(sizeof(VERTEX_USERDATA)*mdl->n_verts))==NULL) return 0;
    if((cobj->norm_usrdata
	=(NORMAL_USERDATA *)malloc(sizeof(NORMAL_USERDATA)*mdl->n_norms))==NULL) return 0;

    if((check_uv=(struct _check_uv *)malloc(sizeof(struct _check_uv)*mdl->n_verts))==NULL) return 0;
    n_check_uv=mdl->n_verts;

    for(i=0;i<cobj->n_verts;i++){
	*(cobj->verts+i)=*(mdl->verts+i);
	*(cobj->vert_usrdata+i)=*(mdl->vert_usrdata+i);
	{
	    ENVDATA *ed ;
	    ed = ( ENVDATA * )( &( mdl->verts[i].vw ) ) ;
	    if ( ed->parent != 255 ) {
		(cobj->verts+i)->vw=(mdl->envs+i)->vx;
	    }
	    else{
		(cobj->verts+i)->vw=1.0f;
	    }
	}
	(check_uv+i)->flag=0;


#if 0
	if((cobj->vert_usrdata+i)->dWeight[0]<0 ||
	   (cobj->vert_usrdata+i)->dWeight[0]>=4096){

	    printf("Strange0 I = %d\n",(cobj->vert_usrdata+i)->dWeight[0]);
	}
	if((cobj->vert_usrdata+i)->dWeight[1]<0 ||
	   (cobj->vert_usrdata+i)->dWeight[1]>=4096){

	    printf("Strange1 I = %d\n",(cobj->vert_usrdata+i)->dWeight[1]);
	}
	if((cobj->vert_usrdata+i)->dWeight[2]<0 ||
	   (cobj->vert_usrdata+i)->dWeight[2]>=4096){

	    printf("Strange2 I = %d\n",(cobj->vert_usrdata+i)->dWeight[2]);
	}
	if((cobj->vert_usrdata+i)->dWeight[3]<0 ||
	   (cobj->vert_usrdata+i)->dWeight[3]>=4096){

	    printf("Strange3 I = %d\n",(cobj->vert_usrdata+i)->dWeight[3]);
	}
#endif

    }
    for(i=0;i<cobj->n_norms;i++){
	*(cobj->norms+i)=*(mdl->norms+i);
	*(cobj->norm_usrdata+i)=*(mdl->norm_usrdata+i);
    }
    for(i=0;i<mdl->n_prims;i++){
	for(j=0;j<4;j++){
	    int vid,nid;

	    vid=mdl->vid[i*4+j];
	    nid=mdl->nid[i*4+j];

	    {
		ENVDATA *ed ;
		ed = ( ENVDATA * )( &( mdl->verts[vid].vw ) ) ;
		if ( ed->parent != 255 ) {
		    (cobj->norms+nid)->vw=(mdl->envs+vid)->vx;
		}
		else{
		    (cobj->norms+nid)->vw=1.0f;
		}
	    }
	}
    }

    k=0;
    for(j=0;j<work.n_strips;j++){
	for(i=0;i<strip[j].n_points;i++,k++){
	    *(cobj->verts_index+k)=strip[j].point[i].vid;
	    *(cobj->norms_index+k)=strip[j].point[i].nid;
	}
	for( ;(k & 1)!=0;k++){
	    *(cobj->verts_index+k)=0;
	    *(cobj->norms_index+k)=0;
	}
    }
    for(j=0;j<mdl->n_prims;j++){
	for(i=0;i<4;i++){
	    int vid;

	    vid=mdl->vid[j*4+i];

	    if((check_uv+vid)->flag==0){
		(check_uv+vid)->vid=vid;
		(check_uv+vid)->tid=mdl->tid[j];
		(check_uv+vid)->uvs=mdl->uvs[j*4+i];

		(check_uv+vid)->flag=1;
	    }
	    else if((check_uv+vid)->vid!=vid ||
		    (check_uv+vid)->tid!=mdl->tid[j] ||
		    (check_uv+vid)->uvs.u!=mdl->uvs[j*4+i].u ||
		    (check_uv+vid)->uvs.v!=mdl->uvs[j*4+i].v){

		for(k=mdl->n_verts;k<n_check_uv;k++){
		    if((check_uv+k)->vid==vid &&
		       (check_uv+k)->tid==mdl->tid[j] &&
		       (check_uv+k)->uvs.u==mdl->uvs[j*4+i].u &&
		       (check_uv+k)->uvs.v==mdl->uvs[j*4+i].v) break;
		}

		if(k==n_check_uv){
		    struct _check_uv *check_uv2;
		    if((check_uv2
			=(struct _check_uv *)realloc(check_uv,
						     sizeof(struct _check_uv)*(n_check_uv+1)))
		       ==NULL){

			free(check_uv);
			return 0;
		    }

		    check_uv=check_uv2;

		    (check_uv+n_check_uv)->vid=vid;
		    (check_uv+n_check_uv)->tid=mdl->tid[j];
		    (check_uv+n_check_uv)->uvs=mdl->uvs[j*4+i];

		    (check_uv+n_check_uv)->flag=1;

		    n_check_uv++;
		}
	    }
	}
    }

    if((cobj->uvs=(FVECTOR *)malloc(sizeof(FVECTOR)*n_check_uv))==NULL){
	free(check_uv);
	return 0;
    }
    cobj->n_uvs=n_check_uv;
    if((cobj->uvs_index=(short *)malloc(sizeof(short)*uv_sum))==NULL){
	free(check_uv);
	return 0;
    }
    cobj->n_uvs_index=uv_sum;

    for(i=0;i<n_check_uv;i++){
	(cobj->uvs+i)->vx=(check_uv+i)->uvs.u;
	(cobj->uvs+i)->vy=(check_uv+i)->uvs.v;
	(cobj->uvs+i)->vz=0.0;
	(cobj->uvs+i)->vw=0.0;
    }

    k=0;
    for(j=0;j<work.n_strips;j++){
	for(i=0;i<strip[j].n_points;i++,k++){
	    int vid=strip[j].point[i].vid;

	    if((check_uv+vid)->tid==strip[j].tid &&
	       (check_uv+vid)->uvs.u==strip[j].point[i].uvs.u &&
	       (check_uv+vid)->uvs.v==strip[j].point[i].uvs.v){

		*(cobj->uvs_index+k)=vid;
		continue;
	    }

	    for(vid=mdl->n_verts;vid<n_check_uv;vid++){
		if((check_uv+vid)->vid==strip[j].point[i].vid &&
		   (check_uv+vid)->tid==strip[j].tid &&
		   (check_uv+vid)->uvs.u==strip[j].point[i].uvs.u &&
		   (check_uv+vid)->uvs.v==strip[j].point[i].uvs.v){

		    *(cobj->uvs_index+k)=vid;
		    break;
		}
	    }
	}
	for( ;(k & 3)!=0;k++){
	    *(cobj->uvs_index+k)=0;
	}
    }

    free(check_uv);

    return 1;
}

#else

/* 基本的に頂点数と法線数が一緒になるようにしたバージョン */
static int GetCV2OBJ(cobj,mdl,strip)
CV2_OBJ *cobj;
DG_MDL  *mdl;
STRIP   *strip;
{
    int i,j,k;
    int vert_sum,uv_sum;

    struct _check_uv {
	int flag;
	u_short vid;
	u_int   tid;
	TVECTOR uvs;
    } *check_uv;
    int n_check_uv;

    int *check_norm;

    //printf("GetCV2OBJ Start\n");

    vert_sum=0;
    uv_sum=0;
    for(i = 0; i < work.n_strips; i ++){
	vert_sum+=(strip[i].n_points+1) & ~1;
	uv_sum+=(strip[i].n_points+3) & ~3;
    }

    if((cobj->verts=(FVECTOR *)malloc(sizeof(FVECTOR)*mdl->n_verts))==NULL) return 0;
    cobj->n_verts=mdl->n_verts;
    if((cobj->norms=(FVECTOR *)malloc(sizeof(FVECTOR)*mdl->n_verts))==NULL) return 0;
    cobj->n_norms=mdl->n_verts;
    if((cobj->verts_index=(short *)malloc(sizeof(short)*vert_sum))==NULL) return 0;
    cobj->n_verts_index=vert_sum;
    if((cobj->norms_index=(short *)malloc(sizeof(short)*vert_sum))==NULL) return 0;
    cobj->n_norms_index=vert_sum;

#if 0
    printf("N verts       = %d\n",mdl->n_verts);
    printf("N verts index = %d\n",vert_sum);
    printf("N check uv    = %d\n",uv_sum);
#endif

    if((cobj->vert_usrdata
	=(VERTEX_USERDATA *)malloc(sizeof(VERTEX_USERDATA)*mdl->n_verts))==NULL) return 0;
    if((cobj->norm_usrdata
	=(NORMAL_USERDATA *)malloc(sizeof(NORMAL_USERDATA)*mdl->n_verts))==NULL) return 0;

    if((check_uv=(struct _check_uv *)malloc(sizeof(struct _check_uv)*mdl->n_verts))==NULL) return 0;
    n_check_uv=mdl->n_verts;

    if((check_norm=(int *)malloc(sizeof(int)*mdl->n_verts))==NULL){
	free(check_uv);
	return 0;
    }

    //printf("Check 1\n");

    for(i=0;i<mdl->n_verts;i++) *(check_norm+i)=0;

    //printf("Check 2\n");

    for(i=0;i<cobj->n_verts;i++){
	*(cobj->verts+i)=*(mdl->verts+i);
	*(cobj->vert_usrdata+i)=*(mdl->vert_usrdata+i);
	{
	    ENVDATA *ed ;
	    ed = ( ENVDATA * )( &( mdl->verts[i].vw ) ) ;
	    if ( ed->parent != 255 ) {
		(cobj->verts+i)->vw=(mdl->envs+i)->vx;
	    }
	    else{
		(cobj->verts+i)->vw=1.0f;
	    }
	}
	(check_uv+i)->flag=0;


#if 0
	if((cobj->vert_usrdata+i)->dWeight[0]<0 ||
	   (cobj->vert_usrdata+i)->dWeight[0]>=4096){

	    printf("Strange0 I = %d\n",(cobj->vert_usrdata+i)->dWeight[0]);
	}
	if((cobj->vert_usrdata+i)->dWeight[1]<0 ||
	   (cobj->vert_usrdata+i)->dWeight[1]>=4096){

	    printf("Strange1 I = %d\n",(cobj->vert_usrdata+i)->dWeight[1]);
	}
	if((cobj->vert_usrdata+i)->dWeight[2]<0 ||
	   (cobj->vert_usrdata+i)->dWeight[2]>=4096){

	    printf("Strange2 I = %d\n",(cobj->vert_usrdata+i)->dWeight[2]);
	}
	if((cobj->vert_usrdata+i)->dWeight[3]<0 ||
	   (cobj->vert_usrdata+i)->dWeight[3]>=4096){

	    printf("Strange3 I = %d\n",(cobj->vert_usrdata+i)->dWeight[3]);
	}
#endif

    }

    //printf("Check 3\n");

    //printf("Check 4\n");

    k=0;
    for(j=0;j<work.n_strips;j++){
	for(i=0;i<strip[j].n_points;i++,k++){
	    int vid=strip[j].point[i].vid;
	    int nid=strip[j].point[i].nid;

	    *(cobj->verts_index+k)=*(cobj->norms_index+k)=vid;

	    if(*(check_norm+vid)){
		if((cobj->norms+vid)->vx!=(mdl->norms+nid)->vx ||
		   (cobj->norms+vid)->vy!=(mdl->norms+nid)->vy ||
		   (cobj->norms+vid)->vz!=(mdl->norms+nid)->vz){

		    FVECTOR *tmp_norm;
		    NORMAL_USERDATA *tmp_norm_usrdata;

		    //printf("Check 4.1\n");

#if 0
		    if((cobj->norms
			=(FVECTOR *)realloc(cobj->norms,
					    sizeof(FVECTOR)*(cobj->n_norms+1)))==NULL){

			free(check_uv);
			free(check_norm);
			return 0;
		    }
#else
		    if((tmp_norm
			=(FVECTOR *)malloc(sizeof(FVECTOR)*(cobj->n_norms+1)))==NULL){

			free(check_uv);
			free(check_norm);
			return 0;
		    }
		    memcpy(tmp_norm,cobj->norms,sizeof(FVECTOR)*cobj->n_norms);
		    free(cobj->norms);
		    cobj->norms=tmp_norm;
#endif			

		    //printf("Check 4.2\n");

#if 0
		    if((cobj->norm_usrdata
			=(NORMAL_USERDATA *)realloc(cobj->norm_usrdata,
						    sizeof(NORMAL_USERDATA)*(cobj->n_norms+1)))
		       ==NULL){

			free(check_uv);
			free(check_norm);
			return 0;
		    }
#else
		    if((tmp_norm_usrdata
			=(NORMAL_USERDATA *)malloc(sizeof(NORMAL_USERDATA)*(cobj->n_norms+1)))
		       ==NULL){

			free(check_uv);
			free(check_norm);
			return 0;
		    }
		    memcpy(tmp_norm_usrdata,cobj->norm_usrdata,
			   sizeof(NORMAL_USERDATA)*cobj->n_norms);
		    free(cobj->norm_usrdata);
		    cobj->norm_usrdata=tmp_norm_usrdata;
#endif

		    *(cobj->norms_index+k)=cobj->n_norms;

		    //printf("Check 4.3\n");

		    *(cobj->norms+cobj->n_norms)=*(mdl->norms+nid);
		    *(cobj->norm_usrdata+cobj->n_norms)=*(mdl->norm_usrdata+nid);

		    {
			ENVDATA *ed ;
			ed = ( ENVDATA * )( &( mdl->verts[vid].vw ) ) ;
			if ( ed->parent != 255 ) {
			    (cobj->norms+cobj->n_norms)->vw=(mdl->envs+vid)->vx;
			}
			else{
			    (cobj->norms+cobj->n_norms)->vw=1.0f;
			}
		    }

		    cobj->n_norms++;

		    //printf("Check 4.4\n");
		}
	    }
	    else{
		*(cobj->norms+vid)=*(mdl->norms+nid);
		*(cobj->norm_usrdata+vid)=*(mdl->norm_usrdata+nid);
		*(check_norm+vid)=1;

		{
		    ENVDATA *ed ;
		    ed = ( ENVDATA * )( &( mdl->verts[vid].vw ) ) ;
		    if ( ed->parent != 255 ) {
			(cobj->norms+vid)->vw=(mdl->envs+vid)->vx;
		    }
		    else{
			(cobj->norms+vid)->vw=1.0f;
		    }
		}
	    }
	}
	for( ;(k & 1)!=0;k++){
	    *(cobj->verts_index+k)=0;
	    *(cobj->norms_index+k)=0;
	}
    }

    if(cobj->n_verts!=cobj->n_norms){
	printf("  N Verts and N Norms are deferent.\n");
    }

    //printf("Check 5\n");

    for(j=0;j<mdl->n_prims;j++){
	for(i=0;i<4;i++){
	    int vid;

	    vid=mdl->vid[j*4+i];

	    if((check_uv+vid)->flag==0){
		(check_uv+vid)->vid=vid;
		(check_uv+vid)->tid=mdl->tid[j];
		(check_uv+vid)->uvs=mdl->uvs[j*4+i];

		(check_uv+vid)->flag=1;
	    }
	    else if((check_uv+vid)->vid!=vid ||
		    (check_uv+vid)->tid!=mdl->tid[j] ||
		    (check_uv+vid)->uvs.u!=mdl->uvs[j*4+i].u ||
		    (check_uv+vid)->uvs.v!=mdl->uvs[j*4+i].v){

		for(k=mdl->n_verts;k<n_check_uv;k++){
		    if((check_uv+k)->vid==vid &&
		       (check_uv+k)->tid==mdl->tid[j] &&
		       (check_uv+k)->uvs.u==mdl->uvs[j*4+i].u &&
		       (check_uv+k)->uvs.v==mdl->uvs[j*4+i].v) break;
		}

		if(k==n_check_uv){
		    struct _check_uv *check_uv2;

#if 0
		    if((check_uv2
			=(struct _check_uv *)realloc(check_uv,
						     sizeof(struct _check_uv)*(n_check_uv+1)))
		       ==NULL){

			free(check_uv);
			free(check_norm);
			return 0;
		    }
#else
		    if((check_uv2
			=(struct _check_uv *)malloc(sizeof(struct _check_uv)*(n_check_uv+1)))
		       ==NULL){

			free(check_uv);
			free(check_norm);
			return 0;
		    }
		    memcpy(check_uv2,check_uv,sizeof(struct _check_uv)*n_check_uv);
		    free(check_uv);
#endif
		    check_uv=check_uv2;

		    (check_uv+n_check_uv)->vid=vid;
		    (check_uv+n_check_uv)->tid=mdl->tid[j];
		    (check_uv+n_check_uv)->uvs=mdl->uvs[j*4+i];

		    (check_uv+n_check_uv)->flag=1;

		    n_check_uv++;
		}
	    }
	}
    }

    //printf("Check 6\n");

    if((cobj->uvs=(FVECTOR *)malloc(sizeof(FVECTOR)*n_check_uv))==NULL){
	free(check_uv);
	free(check_norm);
	return 0;
    }
    cobj->n_uvs=n_check_uv;
    if((cobj->uvs_index=(short *)malloc(sizeof(short)*uv_sum))==NULL){
	free(check_uv);
	free(check_norm);
	return 0;
    }
    cobj->n_uvs_index=uv_sum;

    for(i=0;i<n_check_uv;i++){
	(cobj->uvs+i)->vx=(check_uv+i)->uvs.u;
	(cobj->uvs+i)->vy=(check_uv+i)->uvs.v;
	(cobj->uvs+i)->vz=0.0;
	(cobj->uvs+i)->vw=0.0;
    }

    //printf("Check 7\n");

    k=0;
    for(j=0;j<work.n_strips;j++){
	for(i=0;i<strip[j].n_points;i++,k++){
	    int vid=strip[j].point[i].vid;

	    if((check_uv+vid)->tid==strip[j].tid &&
	       (check_uv+vid)->uvs.u==strip[j].point[i].uvs.u &&
	       (check_uv+vid)->uvs.v==strip[j].point[i].uvs.v){

		*(cobj->uvs_index+k)=vid;
		continue;
	    }

	    for(vid=mdl->n_verts;vid<n_check_uv;vid++){
		if((check_uv+vid)->vid==strip[j].point[i].vid &&
		   (check_uv+vid)->tid==strip[j].tid &&
		   (check_uv+vid)->uvs.u==strip[j].point[i].uvs.u &&
		   (check_uv+vid)->uvs.v==strip[j].point[i].uvs.v){

		    *(cobj->uvs_index+k)=vid;
		    break;
		}
	    }
	}
	for( ;(k & 3)!=0;k++){
	    *(cobj->uvs_index+k)=0;
	}
    }

    free(check_uv);
    free(check_norm);

    //printf("GetCV2OBJ End\n");

    return 1;
}

#endif


static void EasyStrip(DIV_PRIM *div_prim,DG_MDL *mdl)
{
    int strips_p0=work.n_strips;
    int strips_p=work.n_strips;
    int point_p=0;
    int i;

    if(div_prim->n_prims==0) return;

    work.strip[strips_p].point=(POINT2 *)Calloc_POINT(64);
    work.strip[strips_p].verts=(SVECTOR *)Calloc_SVECTOR(64);
    work.strip[strips_p].norms=(SVECTOR *)Calloc_SVECTOR(64);
    work.strip[strips_p].n_points=0;

    for(i=0;i<div_prim->n_prims;i++){
	if(point_p>60){
	    strips_p++;
	    point_p=0;

	    work.strip[strips_p].point=(POINT2 *)Calloc_POINT(64);
	    work.strip[strips_p].verts=(SVECTOR *)Calloc_SVECTOR(64);
	    work.strip[strips_p].norms=(SVECTOR *)Calloc_SVECTOR(64);
	    work.strip[strips_p].n_points=0;
	}

	if(point_p==0){
	    work.strip[strips_p].point[point_p+0]=div_prim->prims[i].point[0];
	    work.strip[strips_p].point[point_p+1]=div_prim->prims[i].point[1];
	    work.strip[strips_p].point[point_p+2]=div_prim->prims[i].point[3];
	    work.strip[strips_p].norms[point_p+0].vw=0x8fff; /* 描画しない */
	    work.strip[strips_p].norms[point_p+1].vw=0x8fff; /* 描画しない */
	    work.strip[strips_p].norms[point_p+2].vw=0x0fff; /* 描画する   */

	    if(div_prim->prims[i].point[2].vid!=div_prim->prims[i].point[3].vid){
		work.strip[strips_p].point[point_p+3]=div_prim->prims[i].point[2];
		work.strip[strips_p].norms[point_p+3].vw=0x0fff; /* 描画する   */
		point_p+=4;
	    }
	    else{
		point_p+=3;
	    }
	}
	else if(work.strip[strips_p].point[point_p-2].vid
		==div_prim->prims[i].point[0].vid &&
		work.strip[strips_p].point[point_p-2].nid
		==div_prim->prims[i].point[0].nid &&
		work.strip[strips_p].point[point_p-2].uvs.u
		==div_prim->prims[i].point[0].uvs.u &&
		work.strip[strips_p].point[point_p-2].uvs.v
		==div_prim->prims[i].point[0].uvs.v &&
		work.strip[strips_p].point[point_p-1].vid
		==div_prim->prims[i].point[1].vid &&
		work.strip[strips_p].point[point_p-1].nid
		==div_prim->prims[i].point[1].nid &&
		work.strip[strips_p].point[point_p-1].uvs.u
		==div_prim->prims[i].point[1].uvs.u &&
		work.strip[strips_p].point[point_p-1].uvs.v
		==div_prim->prims[i].point[1].uvs.v){

	    work.strip[strips_p].point[point_p+0]=div_prim->prims[i].point[3];
	    work.strip[strips_p].norms[point_p+0].vw=0x0fff; /* 描画する   */

	    if(div_prim->prims[i].point[2].vid!=div_prim->prims[i].point[3].vid){
		work.strip[strips_p].point[point_p+1]=div_prim->prims[i].point[2];
		work.strip[strips_p].norms[point_p+1].vw=0x0fff; /* 描画する   */
		point_p+=2;
	    }
	    else{
		point_p+=1;
	    }
	}
	else if(work.strip[strips_p].point[point_p-1].vid
		==div_prim->prims[i].point[0].vid &&
		work.strip[strips_p].point[point_p-1].nid
		==div_prim->prims[i].point[0].nid &&
		work.strip[strips_p].point[point_p-1].uvs.u
		==div_prim->prims[i].point[0].uvs.u &&
		work.strip[strips_p].point[point_p-1].uvs.v
		==div_prim->prims[i].point[0].uvs.v){

	    work.strip[strips_p].point[point_p+0]=div_prim->prims[i].point[1];
	    work.strip[strips_p].point[point_p+1]=div_prim->prims[i].point[3];
	    work.strip[strips_p].norms[point_p+0].vw=0x8fff; /* 描画しない */
	    work.strip[strips_p].norms[point_p+1].vw=0x0fff; /* 描画する   */

	    if(div_prim->prims[i].point[2].vid!=div_prim->prims[i].point[3].vid){
		work.strip[strips_p].point[point_p+2]=div_prim->prims[i].point[2];
		work.strip[strips_p].norms[point_p+2].vw=0x0fff; /* 描画する   */
		point_p+=3;
	    }
	    else{
		point_p+=2;
	    }
	}
	else{
	    work.strip[strips_p].point[point_p+0]=div_prim->prims[i].point[0];
	    work.strip[strips_p].point[point_p+1]=div_prim->prims[i].point[1];
	    work.strip[strips_p].point[point_p+2]=div_prim->prims[i].point[3];
	    work.strip[strips_p].norms[point_p+0].vw=0x8fff; /* 描画しない */
	    work.strip[strips_p].norms[point_p+1].vw=0x8fff; /* 描画しない */
	    work.strip[strips_p].norms[point_p+2].vw=0x0fff; /* 描画する   */

	    if(div_prim->prims[i].point[2].vid!=div_prim->prims[i].point[3].vid){
		work.strip[strips_p].point[point_p+3]=div_prim->prims[i].point[2];
		work.strip[strips_p].norms[point_p+3].vw=0x0fff; /* 描画する   */
		point_p+=4;
	    }
	    else{
		point_p+=3;
	    }
	}

	work.strip[strips_p].n_points=point_p;

    }
    strips_p++;

    for(i=strips_p0;i<strips_p;i++){
	int j;

	work.strip[i].tid = div_prim->tid;
	work.strip[i].have_env = div_prim->have_env;

	for(j=0;j<work.strip[i].n_points;j++){
	    int vid = work.strip[i].point[j].vid;
	    int nid = work.strip[i].point[j].nid;


//	    work.strip[i].verts[j] = mdl->verts[vid];
	    work.strip[i].verts[j].vx = mdl->verts[vid].vx;
	    work.strip[i].verts[j].vy = mdl->verts[vid].vy;
	    work.strip[i].verts[j].vz = mdl->verts[vid].vz;

	    {
		ENVDATA	*ed ;

		ed = ( ENVDATA * )( &( mdl->verts[vid].vw ) ) ;
		if ( ed->parent != 255 ) {
#if 1
		    work.strip[i].verts[j].vw 
			= ( u_short )( mdl->envs[vid].vx * 4096 ) ;
#else
		    work.strip[i].verts[j].vw = 0 ;
#endif
		} else {
		    work.strip[i].verts[j].vw = 4096 ;
		}
	    }

//	    work.strip[i].norms[j] = mdl->norms[nid];
	    work.strip[i].norms[j].vx = mdl->norms[nid].vx * 4096.0F ;
	    work.strip[i].norms[j].vy = mdl->norms[nid].vy * 4096.0F ;
	    work.strip[i].norms[j].vz = mdl->norms[nid].vz * 4096.0F ;
	}
    }
    free(div_prim->prims);

    work.n_strips=strips_p;
}

/* ------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------ */


static int Make_Strip(mdl,obj,cobj)
DG_MDL     *mdl;
NEW_OBJECT *obj;
CV2_OBJ    *cobj;
{
    DIV_PRIM   *div_prim;
//    NEW_PACKET *pack;
    STRIP      *strip;
    NEW_TVECTOR *new_uvs;
    POINT2       *point;
    long      n_div_prims;
    long      i,j;
    long     n_pre_strips;
    int ans=1;

    /* テクスチャを条件によって分割する*/
    Divide_PRIM(mdl);
    div_prim = work.div_prim;
    n_div_prims = work.n_div_prims;
    work.n_strips = 0;
    work.strip = Calloc_STRIP(mdl->n_prims);

    n_pre_strips = 0;
    for(i = 0; i < n_div_prims; i++)
    {
#if 1

	/* 変更後のルーチン */
	if(work.fast_flag){
	    /* 高速ストリップ作成 */
	    EasyStrip(&div_prim[i],mdl);
	}
	else{
	    n_pre_strips = work.n_strips;

	    Strip_Algo_1(&div_prim[i],mdl);
#if 1
	    /*頂点数が６４以下かどうかをチェックする*/
	    Check_STRIP_Len(n_pre_strips);
#endif
#if 1
	    /* シングルサイド指定の時は以下の処理を
	       行わない */
	    if ( !( mdl->type & DG_TYPE_SINGLESIDE ) ) {
		/*できるだけ頂点をまとめる*/
		Union_STRIP(n_pre_strips);
	    }
#endif
	}

#else

	/* 元々のルーチン */
	n_pre_strips = work.n_strips;
	Strip_Algo_1(&div_prim[i],mdl);
#if 1
	/*頂点数が６４以下かどうかをチェックする*/
	Check_STRIP_Len(n_pre_strips);
#endif
#if 1
	/* シングルサイド指定の時は以下の処理を
	   行わない */
	if ( !( mdl->type & DG_TYPE_SINGLESIDE ) ) {
	    /*できるだけ頂点をまとめる*/
	    Union_STRIP(n_pre_strips);
	}
#endif

#endif
    }

    /*オブジェクトのデータを格納する */
    obj->type    = mdl->type ;
    obj->n_packs = work.n_strips;
    obj->lx      = (float) mdl->lx;
    obj->ly      = (float) mdl->ly;
    obj->lz      = (float) mdl->lz;
    obj->ux      = (float) mdl->ux;
    obj->uy      = (float) mdl->uy;
    obj->uz      = (float) mdl->uz;
    obj->tx      = (float) mdl->tx;
    obj->ty      = (float) mdl->ty;
    obj->tz      = (float) mdl->tz;
    obj->parent  = mdl->parent;
//    obj->extend  = mdl->extend ;

    /*パケットのデータを格納する*/
    obj->pack    = Calloc_NEW_PACKET(work.n_strips);
    strip = work.strip;

    /* 追加、CV2を生成する */
    if(!GetCV2OBJ(cobj,mdl,strip)) ans=0;

    for(i = 0; i < work.n_strips; i ++)
    {
	if ( strip[i].have_env ) obj->pack[ i ].flag = DG_PACKET_ENVELOPE ;
//	obj->pack[i].flag    = strip[i].have_env;
	obj->pack[i].n_verts = strip[i].n_points;
	obj->pack[i].tid     = (u_int)strip[i].tid;
	obj->pack[i].verts   = strip[i].verts;
	obj->pack[i].norms   = strip[i].norms;
	obj->pack[i].uvs     = Calloc_NEW_TVECTOR(strip[i].n_points);
	new_uvs = obj->pack[i].uvs;
	point   = strip[i].point;
	for(j = 0; j < strip[i].n_points; j ++)
	{
//	    new_uvs[j].u = (short) point[j].uvs.u * 4096 / 255;
//	    new_uvs[j].v = (short) point[j].uvs.v * 4096 / 255;
	    new_uvs[j].u = (short)( point[j].uvs.u * 4096 ) ;
	    new_uvs[j].v = (short)( point[j].uvs.v * 4096 ) ;
	}
	
	free(strip[i].point);
    }
    /* 領域開放 */
    free(strip);

    return ans;
}


/* ストリップをできるだけまとめる */
static	void Union_STRIP(start_num)
long start_num;
{
    long    end_num;
    STRIP   *strip;
    u_short *flag;
    long    max;
    long    n_tmp_verts;
    long    n_rest_verts;
    long    strip_num;
    long    n_strips;
    long    i,j;
    POINT2   *point;
    SVECTOR *verts;
    SVECTOR *norms;

    /*初期化*/
    flag = Calloc_USHORT(work.n_strips);
    n_strips = start_num;
    end_num = work.n_strips;
    strip = work.strip;
    for(i = start_num; i < work.n_strips; i++)
    {
	if(flag[i]) continue;

	/* 初期化 */
	n_tmp_verts = 0;
	n_rest_verts = N_MAX_POINTS;
	point = Calloc_POINT(N_MAX_POINTS);
	verts = Calloc_SVECTOR(N_MAX_POINTS);
	norms = Calloc_SVECTOR(N_MAX_POINTS);
	
	/* 最初のデータを移す */

	/* test */
	strip[i].norms[0].vw = 0x8fff;
	strip[i].norms[1].vw = 0x8fff;
	for(j = 0; j < strip[i].n_points; j++)
	{
	    point[n_tmp_verts] = strip[i].point[j];
	    verts[n_tmp_verts] = strip[i].verts[j];
	    norms[n_tmp_verts] = strip[i].norms[j];
	    n_tmp_verts ++;
	}
	flag[i] = 1;

	free(strip[i].point);
	free(strip[i].verts);
	free(strip[i].norms);

	while(n_tmp_verts < (N_MAX_POINTS - 2))
	{
	    /* 初期化 */
	    n_rest_verts = N_MAX_POINTS - n_tmp_verts;
	    max = 0;
	    strip_num   = -1;
	    /* つなげられるストリップの中で最大のものを選ぶ */
	    for(j = i + 1; j < work.n_strips; j++)
	    {
		if(flag[j]) continue;
		if((strip[j].n_points > max) && (strip[j].n_points <= n_rest_verts))
		{
		    max = strip[j].n_points;
		    strip_num = j;
		}
	    }
	    
	    if(strip_num < 0) break;
	    /* 上で選んだストリップをくっつける */
	    /* くっつける前に非描画領域をセットする */
	    strip[strip_num].norms[0].vw = 0x8fff;
	    strip[strip_num].norms[1].vw = 0x8fff;
	    for(j = 0; j < max; j++)
	    {
		point[n_tmp_verts] = strip[strip_num].point[j];
		verts[n_tmp_verts] = strip[strip_num].verts[j];
		norms[n_tmp_verts] = strip[strip_num].norms[j];
		n_tmp_verts ++;
	    }
	    flag[strip_num] = 1;
	    free(strip[strip_num].point);
	    free(strip[strip_num].verts);
	    free(strip[strip_num].norms);
		
	}
	/* 結果代入*/
	strip[n_strips].point = point;
	strip[n_strips].verts = verts;
	strip[n_strips].norms = norms;
	strip[n_strips].n_points = n_tmp_verts;
	n_strips ++;
    }

    free(flag);
    work.n_strips = n_strips;
	    
}
	
/* ストリップの長さをチェックする */
static	void Check_STRIP_Len(start_num)
long start_num;
{
    long     end_num;
    long     n_rest_points;
    POINT2    *point;
    SVECTOR  *verts;
    SVECTOR  *norms;
    long     p_num;
    long     i,j;
    STRIP     *strip;
    char     check;
  
    end_num = work.n_strips;
    strip = work.strip;
    for(i = start_num; i < work.n_strips; i++)
    {
	if(strip[i].n_points <= N_MAX_POINTS) 
	    continue;
	/*初期化*/
	n_rest_points = strip[i].n_points;
	point = strip[i].point;
	verts = strip[i].verts;
	norms = strip[i].norms;
	p_num = 0;

	/*ストリップiの値を新たに作る*/
	strip[i].point = Calloc_POINT(N_MAX_POINTS);
	strip[i].verts = Calloc_SVECTOR(N_MAX_POINTS);
	strip[i].norms = Calloc_SVECTOR(N_MAX_POINTS);
	strip[i].n_points = N_MAX_POINTS;
	for(j = 0; j < N_MAX_POINTS; j++,p_num++)
	{
	    strip[i].point[j] = point[p_num];
	    strip[i].verts[j] = verts[p_num];
	    strip[i].norms[j] = norms[p_num];
	}
	strip[i].n_points = j;
	/* 62,63は次の0,1に対応するので２で調整する */
	n_rest_points -= N_MAX_POINTS;
	if(n_rest_points == 0)
	    continue;
      
	p_num -= 2;
	n_rest_points += 2;
	

	/* 残りのデータを移していく*/
	check = 0;
	while(n_rest_points > N_MAX_POINTS)
	{
	    /*初期化*/
	    strip[end_num].tid = strip[i].tid;
	    strip[end_num].have_env = strip[i].have_env;
	    strip[end_num].point = Calloc_POINT(N_MAX_POINTS);
	    strip[end_num].verts = Calloc_SVECTOR(N_MAX_POINTS);
	    strip[end_num].norms = Calloc_SVECTOR(N_MAX_POINTS);
	    strip[end_num].n_points = N_MAX_POINTS;
	    for(j = 0; j < N_MAX_POINTS; j++,p_num++)
	    {
		strip[end_num].point[j] = point[p_num];
		strip[end_num].verts[j] = verts[p_num];
		strip[end_num].norms[j] = norms[p_num];
	    }
	    /* 62,63は次の0,1に対応するので２で調整する */
	    n_rest_points -= N_MAX_POINTS;
	    strip[end_num].n_points = j;
	    end_num++;
	    if(n_rest_points == 0)
	    {
		check = 1;
		break;
	    }

	    p_num -= 2;
	    n_rest_points += 2;
	    
	}

	/*初期化*/
	if(check == 0)
	{
	    strip[end_num].tid = strip[i].tid;
	    strip[end_num].have_env = strip[i].have_env;
	    strip[end_num].point = Calloc_POINT(n_rest_points);
	    strip[end_num].verts = Calloc_SVECTOR(n_rest_points);
	    strip[end_num].norms = Calloc_SVECTOR(n_rest_points);
#if 0
	strip[end_num].point = Calloc_POINT(N_MAX_POINTS);
	strip[end_num].verts = Calloc_SVECTOR(N_MAX_POINTS);
	strip[end_num].norms = Calloc_SVECTOR(N_MAX_POINTS);
#endif
	    strip[end_num].n_points = n_rest_points;
	    for(j = 0; j < n_rest_points; j++,p_num++)
	    {
		strip[end_num].point[j] = point[p_num];
		strip[end_num].verts[j] = verts[p_num];
		strip[end_num].norms[j] = norms[p_num];
	    }
	    strip[end_num].n_points = j;
	    end_num++;
	}
	free(point);
	free(verts);
	free(norms);
	
    }

    work.n_strips = end_num;
}

/* アルゴリズム１*/
static	void Strip_Algo_1(div_prim,mdl)
DIV_PRIM *div_prim;
DG_MDL   *mdl;
{
    long    n_prims;    /* div_prims内のprimの数 */
    long    n_strips;   /* stripの数 */
    u_short *dummy_flag;
    long    i,j,k;
    STRIP   tmp_strip;
    PRIM    *tmp_prim,*p_tmp_prim;
    long    num;
    u_short vid,nid;
 
    n_prims = div_prim->n_prims;
    n_strips = work.n_strips;
    
    num = n_prims;
    while(num > 0)
    {

        n_prims = num;
	/* フラッグを用意*/
	div_prim->flag = Calloc_USHORT(n_prims);
	/* ダミーのフラッグを用意 */
	dummy_flag = Calloc_USHORT(n_prims);

	work.strip[n_strips].n_points = 0;
	for(i = 0; i < n_prims; i++)
	{
	    for(j = 0; j < 3; j++)
	    {
		tmp_strip = Set_STRIP_1(i,j,div_prim,dummy_flag,RIGHT);
		if(tmp_strip.n_points > work.strip[n_strips].n_points)
		{
		    work.strip[n_strips] = tmp_strip;
		    for(k = 0; k < n_prims; k++)
			div_prim->flag[k] = dummy_flag[k];
		}
		else
		{
		    free(tmp_strip.point);
		}
	    }
	    for(j = 0; j < 3; j++)
	    {
		tmp_strip = Set_STRIP_1(i,j,div_prim,dummy_flag,LEFT);
		if(tmp_strip.n_points > work.strip[n_strips].n_points)
		{
		    work.strip[n_strips] = tmp_strip;
		    for(k = 0; k < n_prims; k++)
			div_prim->flag[k] = dummy_flag[k];
		}
		else
		{
		    free(tmp_strip.point);
		}
	    }
	}
	/* 分割プリミティブから ストリップへの値の受け渡し */
	work.strip[n_strips].tid = div_prim->tid;
	work.strip[n_strips].have_env = div_prim->have_env;
	work.strip[n_strips].verts = Calloc_SVECTOR(work.strip[n_strips].n_points);
	work.strip[n_strips].norms = Calloc_SVECTOR(work.strip[n_strips].n_points);
	for(i = 0; i < work.strip[n_strips].n_points; i++)
	{
	    vid = work.strip[n_strips].point[i].vid;
	    nid = work.strip[n_strips].point[i].nid;
//	    work.strip[n_strips].verts[i] = mdl->verts[vid];
	    work.strip[n_strips].verts[i].vx = mdl->verts[vid].vx;
	    work.strip[n_strips].verts[i].vy = mdl->verts[vid].vy;
	    work.strip[n_strips].verts[i].vz = mdl->verts[vid].vz;
	    {
		ENVDATA	*ed ;

		ed = ( ENVDATA * )( &( mdl->verts[vid].vw ) ) ;
		if ( ed->parent != 255 ) {
#if 1
		    work.strip[n_strips].verts[i].vw 
			= ( u_short )( mdl->envs[vid].vx * 4096 ) ;
#else
		    work.strip[n_strips].verts[i].vw = 0 ;
#endif
		} else {
		    work.strip[n_strips].verts[i].vw = 4096 ;
		}
	    }
	    
#if 0 
	    if(work.strip[n_strips].verts[i].vw != -1)
	    {
/********* エンベローブの一時しのぎ ***********************/
		parent_vid = work.strip[n_strips].verts[i].vw;
		work.strip[n_strips].verts[i].vx = work.dg_mdl[mdl->parent].verts[parent_vid].vx - mdl->tx;
		work.strip[n_strips].verts[i].vy = work.dg_mdl[mdl->parent].verts[parent_vid].vy - mdl->ty;
		work.strip[n_strips].verts[i].vz = work.dg_mdl[mdl->parent].verts[parent_vid].vz - mdl->tz;
/**********************************************************/
		work.strip[n_strips].verts[i].vw = 0;
	    }
	    else
		work.strip[n_strips].verts[i].vw = 4096;
#endif
//	    work.strip[n_strips].norms[i] = mdl->norms[nid];
	    work.strip[n_strips].norms[i].vx = mdl->norms[nid].vx * 4096.0F ;
	    work.strip[n_strips].norms[i].vy = mdl->norms[nid].vy * 4096.0F ;
	    work.strip[n_strips].norms[i].vz = mdl->norms[nid].vz * 4096.0F ;
	    work.strip[n_strips].norms[i].vw = 0x0fff;  /* 描画領域に初期化 */
	}
	/* 値を更新 */	
	num -= (work.strip[n_strips].n_points - 2); 
	p_tmp_prim = Calloc_PRIM(num);
        tmp_prim = p_tmp_prim;
	for(i = 0; i < n_prims; i++)
	{
	    if(div_prim->flag[i] == 0)
	    {
		*tmp_prim = div_prim->prims[i];
		tmp_prim ++;
	    }
	}
	n_strips ++;
      	free(div_prim->flag);
	free(div_prim->prims);
	free(dummy_flag);

	div_prim->prims = p_tmp_prim;
	div_prim->n_prims = num;
    }
    work.n_strips = n_strips;

    free(div_prim->prims);
}

/* ストリップを作る */

static	STRIP Set_STRIP_1(prim_num,verts_num,div_prim,dummy_flag,dir)
long      prim_num;
long      verts_num;
DIV_PRIM  *div_prim;
u_short   *dummy_flag;
int       dir;
{
    long    i,j,d;
    STRIP   tmp_strip;
    u_short *tmp_flag;
    long    n_prims;
    long    n_points;
    POINT2   *strip_point;
    POINT2   *prim_point;
    PRIM    *prim;
    POINT2   line[2];
    int     check;
    int     tmp_num;

    n_prims = div_prim->n_prims;

    tmp_flag = dummy_flag;
    /* 初期化 */
    for(i = 0; i < n_prims; i++,tmp_flag++) *tmp_flag = 0;
    dummy_flag[prim_num] = 1;
    tmp_strip.point = Calloc_POINT(n_prims + 2);
    strip_point = tmp_strip.point;
    prim  = div_prim->prims;

    /* 第１頂点を格納 */
    prim_point = prim[prim_num].point;
    *strip_point = prim_point[verts_num];
    strip_point ++;

    /* 第２頂点を格納 */
    /* strip_point[1] */
    tmp_num = (verts_num + dir + 3) % 3;
    *strip_point = prim_point[tmp_num];
    line[0]  = *strip_point;
    strip_point ++;   
               
    /* 第３頂点を格納 */
    /* strip_point[2] */
    tmp_num = (tmp_num + dir + 3) % 3;
    *strip_point = prim_point[tmp_num];
    line[1] = *strip_point;
    strip_point ++;

    n_points = 3;
   
    while(1)
    { 
	check = 0;
	d = n_points % 2;
	tmp_flag = dummy_flag;
	prim = div_prim->prims;
	for(i = 0; i < n_prims; i++,tmp_flag++,prim++)
	{
	    if(*tmp_flag) continue;
	    prim_point = prim->point;
	    for(j = 0; j < 3; j++)
	    {
		/* 条件を満たせば、くっつける */
		tmp_num = (j + dir + 3) % 3;
		if((P_Equal(&line[d],&prim_point[j]) )
		   && (P_Equal(&line[(d+1) % 2],&prim_point[tmp_num])))
		{
		    *tmp_flag = 1;
		    line[0] = line[1];
		    tmp_num = (tmp_num + dir + 3) % 3;
		    /*strip_point[n_points]*/
		    *strip_point = prim_point[tmp_num];
		    line[1] = *strip_point;
		    strip_point ++;
		
		    n_points ++;
		    check = 1;
		    goto label1;
		}
	    }
	}
	
    label1:
	if(check == 0) break;
    }

    tmp_strip.n_points = n_points;
#if 0
    strip_point = tmp_strip.point;
    tmp_strip.point = (POINT2 *)realloc(strip_point,n_points * sizeof(POINT2));
    if(tmp_strip.point == NULL)
	ERR_PRINT(CALLOC);
#else
    strip_point = tmp_strip.point;
    if((tmp_strip.point=(POINT2 *)malloc(n_points*sizeof(POINT2)))==NULL){
	tmp_strip.point=strip_point;
    }
    else{
	memcpy(tmp_strip.point,strip_point,n_points*sizeof(POINT2));
	free(strip_point);
    }
#endif

    return tmp_strip;
}    

/* テクスチャを条件によって分割する*/
static	void Divide_PRIM(mdl)
DG_MDL    *mdl;
{
    PRIM      *prim;
  
    /* プリミティブを作成する */
    prim = Make_PRIM(mdl);
    /* テクスチャの種類によって分割 */
    Divide_By_TEX(prim,mdl);
    /* 領域開放(プリミティブ) */
    free(prim);
    /* エンベロープによる分割 */
    Divide_By_ENV(mdl);
    
}

/* エンベロープの有無による分割 */
static	void Divide_By_ENV(mdl)
DG_MDL *mdl;
{
    /* 新しいプリミティブ */
    DIV_PRIM *new_div_prim;
    long     n_new_div_prims;
    long     n_new_prims;
   
    FVECTOR   *verts;
    PRIM      *prim;
    u_short   *flag;
    DIV_PRIM  *div_prim;
    u_short     vid;
    long      n_div_prims;
    long      n_prims;
    long      i,j,k,d;
    u_short      check;
    u_short      check_env;
    ENVDATA	*ed ;

    /* 初期化 */
    verts = mdl->verts;	
    div_prim = work.div_prim;
    n_div_prims     = work.n_div_prims;
    n_new_div_prims = work.n_div_prims;

    /* 頂点数を調べる */
    for(i = 0; i < n_div_prims; i ++)
    {
	/*設定*/
	prim    = div_prim[i].prims;
	n_prims = div_prim[i].n_prims;
	div_prim[i].flag = Calloc_USHORT(n_prims);
	flag    = div_prim[i].flag;
	check_env = 0;
	for(j = 0; j < n_prims; j++)
	{
	    for(k = 0; k < 3; k++)
	    {
		vid = prim[j].point[k].vid;
		/* エンベロープかどうかを調べる */
		ed = ( ENVDATA * )( &( verts[ vid ].vw ) ) ;
		if ( ed->parent != 255 ) {
		    flag[ j ] = 1 ;
		    check_env = 1 ;
		    goto label_env ;
		}	
#if 0
		if(verts[vid].vw >= 0)
		{  
		    flag[j] = 1;
		    check_env = 1;
		    goto label_env;
		}
#endif
		/* エンベローブがなければ */
		flag[j] = 0;
	    }
	label_env: 
	}
	if(check_env)
	    n_new_div_prims ++;
    }

    /* 分割プリミティブ数が増えていなければそのまま */
    if(n_new_div_prims == n_div_prims)
	return;

    /* 分割する */
    /* 初期化 */
    new_div_prim = Calloc_DIV_PRIM(n_new_div_prims);
    d = 0;

    for(i = 0; i < n_div_prims; i++)
    {
	/* 初期化 */
	prim = div_prim[i].prims;
	flag = div_prim[i].flag;
	n_prims = div_prim[i].n_prims;

	/* 一回目 */
	new_div_prim[d].prims = Calloc_PRIM(n_prims);
	check = 0;
	n_new_prims = 0;
	for(j = 0; j < n_prims; j++)
	{
	    if(flag[j] == 0)
	    {
		new_div_prim[d].prims[n_new_prims] = prim[j];
		n_new_prims ++;
	    }
	    else
		check = 1;
	}
	
	/* 値を代入 */
	new_div_prim[d].n_prims = n_new_prims;
	new_div_prim[d].tid     = div_prim[i].tid;
	new_div_prim[d].have_env = 0;
	d++;

	/* ２回目 */
	if(check)
	{
	    new_div_prim[d].prims = Calloc_PRIM(n_prims);
	    n_new_prims = 0;
	    for(j = 0; j < n_prims; j++)
	    {
		if(flag[j])
		{
		    new_div_prim[d].prims[n_new_prims] = prim[j];
		    n_new_prims ++;
		}
	    }
	    /* 値を代入 */
	    new_div_prim[d].n_prims = n_new_prims;
	    new_div_prim[d].tid     = div_prim[i].tid;
	    new_div_prim[d].have_env = 1;
	    d++;
	}

    }	

    /* 余分な領域を開放 */

    for(i = 0; i < work.n_div_prims; i++)
    {
	free(div_prim[i].prims);
	free(div_prim[i].flag);
    }
    free(div_prim);
    /* workの情報を更新 */
    work.n_div_prims = n_new_div_prims;
    work.div_prim    = new_div_prim;
}
    
/* テクスチャの種類によって分割 */
static	void Divide_By_TEX(prim,mdl)
PRIM    *prim;
DG_MDL  *mdl;
{
    u_short   *flag;
    long      n_div_prims;
    long      i,j;
    long      n;                 /* div_primの数 */
    u_int	   *tid;
    DIV_PRIM  *div_prim;
    
    /* テクスチャ数を調べる */
    flag = Calloc_USHORT(mdl->n_prims);
    tid  = mdl->tid;
    n_div_prims = mdl->n_prims;
    for(i = 0; i < mdl->n_prims; i++)
    {
	if(flag[i]) continue;
	for(j = i + 1; j < mdl->n_prims; j++)
	{
	    if(flag[j]) continue;
	    if(tid[i] == tid[j])
	    {
		flag[j] = 1;
		n_div_prims --;
	    }
	}
    }

    work.n_div_prims = n_div_prims;
    work.div_prim  = Calloc_DIV_PRIM(n_div_prims);
    div_prim = work.div_prim;
    /* フラッグ初期化 */
    for(i = 0; i < mdl->n_prims; i++)
	flag[i] = 0;
    for(i = 0; i < n_div_prims; i++,div_prim++)
    {
	div_prim->prims = Calloc_PRIM(mdl->n_prims);
	n = 0;
	for(j = 0; j < mdl->n_prims; j++)
	{
	    if(flag[j]) continue;
	    if(n == 0)
	    {
		flag[j] = 1;
		div_prim->tid = tid[j];
		div_prim->prims[n] = prim[j];
		n++;
		continue;
	    }
	    if(div_prim->tid == tid[j])
	    {
		flag[j] = 1;
		div_prim->prims[n] = prim[j];
		n++;
		continue;
	    }
	}
	div_prim->n_prims = n;
    } 
    /* 領域開放 */
    free(flag);
}

/* プリミティブを作成する */
static	PRIM *Make_PRIM(mdl)
DG_MDL  *mdl;
{
    PRIM     *prim,*p_prim;
    long     i,j;
    u_short  *vid;
    u_short  *nid;
    TVECTOR  *uvs;

    p_prim = Calloc_PRIM(mdl->n_prims);
    prim   = p_prim;
    vid    = mdl->vid;
    nid    = mdl->nid;
    uvs    = mdl->uvs;

    for(i = 0; i < mdl->n_prims; i++,prim++)
    {
	for(j = 0; j < 4; j++)
	{
	    prim->point[j].vid = *vid;
	    prim->point[j].nid = *nid;
	    prim->point[j].uvs = *uvs;
	    vid ++; nid++; uvs++;
	}
    }
    return p_prim;
}

/* KMSと同時にCVD2も作る */  
KMS_DEF		*MDU_Km42Kms( km4def2, cdef, fast_flag )
KM4_DEF2	*km4def2 ;
CV2_OBJS	**cdef ;
int		fast_flag ;
{
    KM4_DEF2	*def2 ;
    KMS_DEF	*sdef ;

    printf( ".km4 -> .kms conversion start\n" ) ;
    /* 三角形分割 */
    def2 = Km4DividePrimTriangle( km4def2 ) ;
    work.dg_def = *( def2->def ) ;
    work.dg_mdl = def2->def->models ;
    work.fast_flag = fast_flag;
    Strip() ;
    MDU_mdlFreeKm4Def2( def2 ) ;
    sdef = work.sdef ;
    *cdef = work.cdef ;
    return sdef ;
}
