/*
   MDU_mdlKm32KmsUtil.c

   Ｋｍ３ → Ｋｍｓコンバート関数内ユーティリティ

   by M.Sonoyama 1999.Sep.～ 
   $Id: MDU_mdlKm32KmsUtil.c,v 1.3 2000/01/14 05:20:13 usr02011 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#ifndef NOW_NO_USE
static	DG_MDL *Calloc_DG_MDL(n)
long n;
{
    DG_MDL *p;
#if 0
work.addr_size += n*sizeof(DG_MDL);
fprintf(stderr,"DG_MDL size %ld, addr_size %ld\n",n*sizeof(DG_MDL),work.addr_size);
#endif
    p = (DG_MDL *)MDU_Calloc(n,sizeof(DG_MDL));
    if(p == NULL)
	ERR_PRINT(CALLOC);

    return p;
}

static 	KM2_MDL *Calloc_KM2_MDL(n)
long n;
{
    KM2_MDL *p;
#if 0
work.addr_size += n*sizeof(KM2_MDL);
fprintf(stderr,"KM_MDL size %ld, addr_size %ld\n",n*sizeof(KM2_MDL),work.addr_size);
#endif

    p = (KM2_MDL *)MDU_Calloc(n,sizeof(KM2_MDL));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}
#endif

static	SVECTOR *Calloc_SVECTOR(n)
long n;
{
    SVECTOR *p;

#if 0
work.addr_size += n*sizeof(SVECTOR);
fprintf(stderr,"SVECTOR size %ld, addr_size %ld\n",n*sizeof(SVECTOR),work.addr_size);
#endif

    p = (SVECTOR *)MDU_Calloc(n,sizeof(SVECTOR));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}

#ifndef NOW_NO_USE
static	TVECTOR *Calloc_TVECTOR(n)
long n;
{
    TVECTOR *p;

#if 0
work.addr_size += n*sizeof(TVECTOR);
fprintf(stderr,"TVECTOR size %ld, addr_size %ld\n",n*sizeof(TVECTOR),work.addr_size);
#endif

    p = (TVECTOR *)MDU_Calloc(n,sizeof(TVECTOR));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}
#endif

static	u_short *Calloc_USHORT(n)
long n;
{
    u_short *p;
#if 0
work.addr_size += n*sizeof(u_short);
fprintf(stderr,"USHORT size %ld, addr_size %ld\n",n*sizeof(u_short),work.addr_size);
#endif

    p = (u_short *)MDU_Calloc(n,sizeof(u_short));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}

#ifndef NOW_NO_USE
static	NEW_OBJECT *Calloc_NEW_OBJECT(n)
long n;
{
    NEW_OBJECT *p;

#if 0
work.addr_size += n*sizeof(NEW_OBJECT);
fprintf(stderr,"NEW_OBJECT size %ld, addr_size %ld\n",n*sizeof(NEW_OBJECT),work.addr_size);
#endif

    p = (NEW_OBJECT *)MDU_Calloc(n,sizeof(NEW_OBJECT));
    if(p == NULL)
      ERR_PRINT(CALLOC);
    return p;
}
#endif

static	PRIM  *Calloc_PRIM(n)
long n;
{
  PRIM *p;
#if 0
work.addr_size += n*sizeof(PRIM);
fprintf(stderr,"PRIM size %ld, addr_size %ld\n",n*sizeof(PRIM),work.addr_size);
#endif

  p = (PRIM *)MDU_Calloc(n,sizeof(PRIM));
  if(p == NULL)
    ERR_PRINT(CALLOC);
  return p;
}

static	DIV_PRIM  *Calloc_DIV_PRIM(n)
long n;
{
    DIV_PRIM *p;

#if 0
work.addr_size += n*sizeof(DIV_PRIM);
fprintf(stderr,"DIV_PRIM size %ld, addr_size %ld\n",n*sizeof(DIV_PRIM),work.addr_size);
#endif

    p = (DIV_PRIM *)MDU_Calloc(n,sizeof(DIV_PRIM));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}

static	STRIP  *Calloc_STRIP(n)
long n;
{
    STRIP  *p;

#if 0
work.addr_size += n*sizeof(STRIP);
fprintf(stderr,"STRIP size %ld, addr_size %ld\n",n*sizeof(STRIP),work.addr_size);
#endif

    p = (STRIP *)MDU_Calloc(n,sizeof(STRIP));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}

static	POINT2  *Calloc_POINT(n)
long n;
{
    POINT2  *p;

#if 0
work.addr_size += n*sizeof(POINT);
fprintf(stderr,"POINT size %ld, addr_size %ld\n",n*sizeof(POINT),work.addr_size);
#endif

    p = (POINT2 *)MDU_Calloc(n,sizeof(POINT2));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}

static	NEW_PACKET *Calloc_NEW_PACKET(n)
long n;
{
    NEW_PACKET *p;

#if 0
work.addr_size += n*sizeof(NEW_PACKET);
fprintf(stderr,"NEW_PACKET size %ld, addr_size %ld\n",n*sizeof(NEW_PACKET),work.addr_size);
#endif

    p = (NEW_PACKET *)MDU_Calloc(n,sizeof(NEW_PACKET));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}

static	NEW_TVECTOR *Calloc_NEW_TVECTOR(n)
long n;
{
    NEW_TVECTOR *p;

#if 0
work.addr_size += n*sizeof(NEW_TVECTOR);
fprintf(stderr,"NEW_TVECTOR size %ld, addr_size %ld\n",n*sizeof(NEW_TVECTOR),work.addr_size);
#endif

    p = (NEW_TVECTOR *)MDU_Calloc(n,sizeof(NEW_TVECTOR));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}

#ifndef NOW_NO_USE
static	KMS_OBJECT *Calloc_KMS_OBJECT(n)
long n;
{
    KMS_OBJECT *p;

#if 0
work.addr_size += n*sizeof(KMS_OBJECT);
fprintf(stderr,"KMS_OBJECT size %ld, addr_size %ld\n",n*sizeof(KMS_OBJECT),work.addr_size);
#endif

    p = (KMS_OBJECT *)MDU_Calloc(n,sizeof(KMS_OBJECT));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}

static	KMS_PACKET *Calloc_KMS_PACKET(n)
long n;
{
    KMS_PACKET *p;

#if 0
work.addr_size += n*sizeof(KMS_PACKET);
fprintf(stderr,"KMS_PACKET size %ld, addr_size %ld\n",n*sizeof(KMS_PACKET),work.addr_size);
#endif

    p = (KMS_PACKET *)MDU_Calloc(n,sizeof(KMS_PACKET));
    if(p == NULL)
	ERR_PRINT(CALLOC);
    return p;
}
#endif

static	int  P_Equal(p_a,p_b)
POINT2 *p_a;
POINT2 *p_b;
{

    if(p_a->nid != p_b->nid) return 0;
    if(p_a->vid != p_b->vid) return 0;
    if(p_a->uvs.u != p_b->uvs.u) return 0;
    if(p_a->uvs.v != p_b->uvs.v) return 0;

    return 1;
}

static	void ERR_PRINT(mode)
int mode;
{
    switch(mode)
    {
        case STRCMP : 
	    fprintf(stderr,"This input file is not a file of km2\n");
	    break;

	case FILE_OPERATION :
	    fprintf(stderr,"Can't open a INPUT FILE\n");
	    break;

	case FREAD :
	    fprintf(stderr,"Can't read a INPUT FILE\n");
	    break ;
	case CALLOC :
	    fprintf(stderr,"Can't secure any memory area\n");
	    break;
	case FWRITE :
	    fprintf(stderr,"Can't write a OUTPUT FILE\n");
	    break;
	case FSEEK :
	    fprintf(stderr,"Can't seek a INPUT FILE\n");
	    break;
	case FTELL :
	    fprintf(stderr,"Can't ftell a OUTPUT FILE\n");
	    break;
	default :
    }
    exit(1);
}

/*------------------------------------------------------------------*/

#ifndef NOW_NO_USE
static	void Print(void)
{
    FILE   *fpw;
    DG_DEF def;
    DG_MDL *mdl;
    long   i;

    fpw = fopen("debug_data","wt");
    def = work.dg_def;
    fprintf(fpw, "\n----------------\n MODEL \n-----------------\n");
    fprintf(fpw,"n_models : %d\n", def.n_models) ;
    fprintf(fpw,"n_x_models : %d\n",def.n_x_models) ;
    fprintf(fpw,"[%f,%f,%f]",def.lx,def.ly,def.lz) ;
    fprintf(fpw,"～ [%f,%f,%f]\n",def.ux,def.uy,def.uz) ;

    mdl = work.dg_mdl;
    fprintf( fpw, "\n----------------\n MODEL UNIT \n----------------\n");
    for(i = 0; i < def.n_x_models; i++, mdl++)
    {
	fprintf(fpw,"\nMODEL_UNIT [%ld] { \n", i);
	fprintf(fpw,"type : %x,",mdl->type) ;
	fprintf(fpw,"n_prims : %d\n",mdl->n_prims) ;
	
	fprintf(fpw,"lx : %f, ly : %f, lz : %f\n",mdl->lx,mdl->ly,mdl->lz);
	fprintf(fpw,"ux : %f, uy : %f, uz : %f\n",mdl->ux,mdl->uy,mdl->uz);
	fprintf(fpw,"tx : %f, ty : %f, tz : %f\n",mdl->tx,mdl->ty,mdl->tz);
	fprintf(fpw,"parent : %d, extend : %d\n",mdl->parent,mdl->extend) ;
	fprintf(fpw,"n_verts : %d,",mdl->n_verts) ;
	fprintf(fpw,"n_norms : %d,\n",mdl->n_norms );
    }

    mdl = work.dg_mdl;
    for( i = 0 ; i < def.n_x_models ; i ++, mdl ++) {
	fprintf(fpw,"\n\nMODEL_VERT(%ld) = {\n",i) ;
	Print_VERTS(fpw, mdl) ;     /* 頂点データを表示する */
	fprintf(fpw,"\n\nMDL_NORM(%ld) = {\n",i) ;
	Print_NORMS(fpw, mdl) ;     /* 法線データを表示する */
	fprintf( fpw, "\n\nMDL_PRIM(%ld) = {\n",i) ;
	Print_PRIM(fpw, mdl) ;       /* ポリゴンデータを表示する */
    } 
   
    fclose(fpw);
}

static	void Print_VERTS(fpw, mdl)
FILE *fpw;
DG_MDL *mdl;
{
    long i;
    FVECTOR *vert;

    vert = mdl->verts;
    for(i = 0; i < mdl->n_verts; i++, vert++)
    {
	fprintf(fpw,"[%5f,%5f,",vert->vx,vert->vy ) ;
	fprintf( fpw,"%5f,%3f],",vert->vz,vert->vw ) ;
	if( ( i % 3 ) == 2 ) fprintf( fpw,"\n") ;
    }
    fprintf(fpw,"\n}\n");
}

static	void Print_NORMS(fpw, mdl)
FILE     *fpw ;
DG_MDL   *mdl ;
{
    long i ;
    FVECTOR *norm ;
    
    norm = mdl->norms ;
    for( i = 0 ; i < mdl->n_norms ; i ++, norm ++ ) {
	fprintf( fpw,"[%5f,%5f,",norm->vx,norm->vy ) ;
	fprintf( fpw,"%5f,%5f]",norm->vz,norm->vw ) ;
	if( ( i % 3 ) == 2 ) fprintf( fpw,"\n") ;
    }
    fprintf(fpw,"\n}\n") ;
}

static	void Print_PRIM(fpw,mdl)
FILE    *fpw;
DG_MDL  *mdl;
{
    long      i,j;
    u_short   *vid;
    u_short   *nid;
    TVECTOR   *uvs;
    u_int   *tid;

    vid = mdl->vid;
    nid = mdl->nid;
    uvs = mdl->uvs;
    tid = mdl->tid;

    for(i = 0; i < mdl->n_prims; i++)
    {
	fprintf(fpw,"( %d,",*tid);
	tid++;
	for(j = 0; j < 4; j++)
	{
	    fprintf( fpw,"[%2d,",*vid) ;
	    fprintf( fpw,"%2d,",*nid ) ;
	    fprintf( fpw,"%3f,%3f], ",uvs->u, uvs->v ); 
	    vid++; nid++; uvs++;
	}
	fprintf( fpw,"\n") ;
    }
    fprintf( fpw,"\n}\n") ;

}
#endif


