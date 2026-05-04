/*
   Kmx2Kms.c

   Ｋｍｘ → Ｋｍｓ( For X-BOX )コンバート関数

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 3/3/2000
   Modified by K.Kano , 6/4/2002

   $Id: Kmx2Kmsx.c,v 1.8 2002/08/26 11:22:09 usr01363 Exp $
   
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
#include <sys/time.h>

#include <unistd.h>
#include <SFDLINUX.h>

#include "METALGEAR.h"
#include "fmt_km4.h"

#include "MDU_util.h"
#include "MDU_mdl.h"


#include "ToStrip.h"

#include "Kmx.h"


#define SINGLE_N_MAX_POINTS	64
#define MULTI_N_MAX_POINTS	32
#define NOLIMIT_N_MAX_POINTS	0xffff	// unsigned short 限界


/* -------------------------------------------------------------------------- */

static int MakeKmsxVertex(KMX_MDL *mdl);

static int ConvertToKmsx(KMX_DEF *def,KMX_MDL *mdl,KMS2_MDL *obj,STRIP **strips,int n_strips,
						 int multitex_flag,int multi_uv_flag)
{
    int i,j;
    int mdl_type;
	int multi_tex=0;


	for(i=0;i<mdl->n_prims;i++){
		(mdl->prims+i)->flag=0;
	}


    if(n_strips!=0){
		if((obj->packs=Kms2_MdlPack_Alloc(n_strips,NULL))==NULL) return 0;
    }
    else{
		obj->packs=NULL;
    }

    obj->n_packs=n_strips;
    mdl_type=obj->type=mdl->type;
#if 0
	if(mdl->n_prims>0){
		obj->lx=mdl->lx;
		obj->ly=mdl->ly;
		obj->lz=mdl->lz;
		obj->ux=mdl->ux;
		obj->uy=mdl->uy;
		obj->uz=mdl->uz;
	}
	else{
		obj->lx=obj->ly=obj->lz=0.0f;
		obj->ux=obj->uy=obj->uz=0.0f;
	}
#else
	obj->lx=mdl->lx;
	obj->ly=mdl->ly;
	obj->lz=mdl->lz;
	obj->ux=mdl->ux;
	obj->uy=mdl->uy;
	obj->uz=mdl->uz;
#endif
    obj->tx=mdl->tx;
    obj->ty=mdl->ty;
    obj->tz=mdl->tz;
    obj->parent=mdl->parent;

    for(i=0;i<n_strips;i++){
		int n_verts;
		int n_tid;
		int flag=0;
		int tid[N_KMS_MULTI_TEXTURES];
		int uv_index[N_KMS_MULTI_TEXTURES];

		/* テクスチャの数のチェック */
		n_tid=(*(strips+i))->n_tid;
		if(n_tid>N_KMS_MULTI_TEXTURES) n_tid=N_KMS_MULTI_TEXTURES;

		/* テクスチャの属性チェック */
		for(j=0;j<N_KMS_MULTI_TEXTURES;j++) uv_index[j]=j;

		for(j=0;j<n_tid; ){
			int eff;

			eff=IsEffectTexture(def->texs[*((*(strips+i))->tid+uv_index[j])].name);

			switch(eff){
			case DG_PACKFLAG_EMAP:
			case DG_PACKFLAG_SMAP:
				{
					int k,tmp;

					tmp=uv_index[j];
					for(k=j+1;k<n_tid;k++){
						uv_index[k-1]=uv_index[k];
					}
					uv_index[k-1]=tmp;
				}
				flag|=DG_PACKFLAG_TEX2;
				flag|=eff;
				n_tid--;
				continue;
			case DG_PACKFLAG_BMAP:
				if(j==0){
					int tmp;
					tmp=uv_index[0];
					uv_index[0]=uv_index[1];
					uv_index[1]=tmp;
				}
				flag|=eff;
				n_tid--;
				continue;
			}

			j++;
		}


		/* 頂点数の決定 */
		n_verts=(obj->packs+i)->n_indices=(*(strips+i))->n_points;


		/* Texture ID , UV値の順序変更 */
		{
			u_int tmp_tid[N_KMS_MULTI_TEXTURES];
			TVECTOR tmp_uv[N_KMS_MULTI_TEXTURES];

			for(j=0;j<(*(strips+i))->n_tid;j++){
				tmp_tid[j]=*((*(strips+i))->tid+uv_index[j]);
			}
			for(j=0;j<(*(strips+i))->n_tid;j++){
				*((*(strips+i))->tid+j)=tmp_tid[j];
			}

			for(j=0;j<n_verts;j++){
				KMX_PRIMS *prim=(*(strips+i))->p[j].prim;
				int k,l;

				if(prim->flag) continue;
				prim->flag=1;

				for(l=0;l<n_tid;l++){
					tmp_tid[l]=*(prim->tid+uv_index[l]);
				}
				for(l=0;l<n_tid;l++){
					*(prim->tid+l)=tmp_tid[l];
				}

				for(k=0;k<prim->n_id;k++){
					for(l=0;l<n_tid;l++){
						memcpy(&(tmp_uv[l]),prim->uvs+uv_index[l]*prim->n_id+k,sizeof(TVECTOR));
					}
					for(l=0;l<n_tid;l++){
						memcpy(prim->uvs+l*prim->n_id+k,&(tmp_uv[l]),sizeof(TVECTOR));
					}
				}
			}
		}


		for(j=0;j<(*(strips+i))->n_tid;j++){
			tid[j]=MDU_GetStrCode(def->texs[*((*(strips+i))->tid+j)].name);

			// printf("0x%08x\n",tid[j]);
#if 0
			/* テスト */
			if(tid[j]==4710569) tid[j]=0;
#endif
		}

		// printf("%d\n",n_tid);

#if 1
		if(multitex_flag==1){
			/* Single Texture */
			if(n_tid>1) n_tid=1;
			flag&=~(DG_PACKFLAG_EMAP|DG_PACKFLAG_SMAP|DG_PACKFLAG_BMAP);
		}
#else
		if(n_tid>1) n_tid=1;
#endif

		if(n_tid>1) multi_tex=1;

		/* パケットのフラグの決定 */
		if((*(strips+i))->have_env) flag|=DG_PACKFLAG_ENVELOPE;

		switch(n_tid){
		case 3:
			flag|=DG_PACKFLAG_TEX2;
			flag|=DG_PACKFLAG_UV2;
		case 2:
			flag|=DG_PACKFLAG_TEX1;
			flag|=DG_PACKFLAG_UV1;
		case 1:
			flag|=DG_PACKFLAG_TEX0;
			flag|=DG_PACKFLAG_UV0;
			break;
		}
		if(mdl_type & DG_TYPE_SINGLESIDE){
			flag|=DG_PACKFLAG_CULLON;
		}
		else if(mdl_type & DG_TYPE_AUTOSIDE){
			flag|=DG_PACKFLAG_CULLAUTO;
		}

		(obj->packs+i)->flag=flag;


		if(((obj->packs+i)->index=Kms2_Index_Alloc(n_verts,NULL))==NULL){
			Kms2_MdlPack_Free(obj->packs,obj->n_packs);
			Kms2_VBuff_Free(obj->vbuff);
			obj->vbuff=NULL;
			obj->packs=NULL;
			return 0;
		}
		for(j=0;j<n_verts;j++){
			KMX_PRIMS *prim=(*(strips+i))->p[j].prim;
			int v_order=(*(strips+i))->p[j].v_order;
			int vid;

			/* 頂点の展開 */
			vid=*(prim->vid+v_order);
			*((obj->packs+i)->index+j)=vid;

			/* Texture ID , UV値の順序変更を行なったので、それを反映 */
			{
				KMX_XVERTS *xvert=mdl->xverts+vid;
				int k;

				for(k=0;k<n_tid;k++){
					xvert->uv.tid[k]=*(prim->tid+k);
					memcpy(&(xvert->uv.uvs[k]),prim->uvs+k*prim->n_id+v_order,sizeof(TVECTOR));
				}
			}
		}

		if(flag & DG_PACKFLAG_TEX0){
			(obj->packs+i)->tex_id[0]=tid[0];

			// printf("0x%08x\n",tid[0]);
		}
		else{
			(obj->packs+i)->tex_id[0]=0;
		}

		if(flag & DG_PACKFLAG_TEX1){
			(obj->packs+i)->tex_id[1]=tid[1];

			// printf("0x%08x\n",tid[1]);
		}
		else{
			(obj->packs+i)->tex_id[1]=0;
		}

		if(flag & DG_PACKFLAG_TEX2){
			if(n_tid<3) (obj->packs+i)->tex_id[2]=tid[n_tid];
			else (obj->packs+i)->tex_id[2]=tid[2];

			// printf("0x%08x\n",tid[2]);
		}
		else{
			(obj->packs+i)->tex_id[2]=0;
		}

		// printf("%d\n",n_tid);
    }


	if(multitex_flag==2){
		/* 強制マルチテクスチャモード */
		multi_tex=1;
	}

	if((obj->vbuff=Kms2_VBuff_Alloc(mdl->n_xverts,multi_tex,NULL))==NULL){
		Kms2_MdlPack_Free(obj->packs,obj->n_packs);
		return 0;
	}

    if(multi_tex) obj->stride=sizeof(KMSM_VERTEX);
    else obj->stride=sizeof(KMSS_VERTEX);
	obj->n_verts=mdl->n_xverts;

	for(i=0;i<mdl->n_xverts;i++){
		KMX_XVERTS *v=mdl->xverts+i;

		if(multi_tex){
			KMSM_VERTEX *dv=(KMSM_VERTEX *)((int)(obj->vbuff)+obj->stride*i);

			dv->vx=(short)(v->vertex.v.vx);
			dv->vy=(short)(v->vertex.v.vy);
			dv->vz=(short)(v->vertex.v.vz);
			dv->wt=(short)(v->vertex.env.val[0]*32767.0f);
			dv->nx=(short)(v->normal.n.vx*32767.0f);
			dv->ny=(short)(v->normal.n.vy*32767.0f);
			dv->nz=(short)(v->normal.n.vz*32767.0f);
			dv->f=0;
			dv->u0=(short)(v->uv.uvs[0].u*32767.0f);
			dv->v0=(short)(v->uv.uvs[0].v*32767.0f);
			if(multi_uv_flag){
				dv->u1=(short)(v->uv.uvs[1].u*32767.0f);
				dv->v1=(short)(v->uv.uvs[1].v*32767.0f);
				dv->u2=(short)(v->uv.uvs[2].u*32767.0f);
				dv->v2=(short)(v->uv.uvs[2].v*32767.0f);
			}
			else{
				dv->u1=(short)(v->uv.uvs[0].u*32767.0f);
				dv->v1=(short)(v->uv.uvs[0].v*32767.0f);
				dv->u2=(short)(v->uv.uvs[0].u*32767.0f);
				dv->v2=(short)(v->uv.uvs[0].v*32767.0f);
			}

#if 0
			printf("%d %d %d : %d %d %d : %d %d : %d %d : %d %d\n",
				   dv->vx,dv->vy,dv->vz,dv->nx,dv->ny,dv->nz,
				   dv->u0,dv->v0,dv->u1,dv->v1,dv->u2,dv->v2);
#endif

		}
		else{
			KMSS_VERTEX *dv=(KMSS_VERTEX *)((int)(obj->vbuff)+obj->stride*i);

			dv->vx=(short)(v->vertex.v.vx);
			dv->vy=(short)(v->vertex.v.vy);
			dv->vz=(short)(v->vertex.v.vz);
			dv->wt=(short)(v->vertex.env.val[0]*32767.0f);
			dv->nx=(short)(v->normal.n.vx*32767.0f);
			dv->ny=(short)(v->normal.n.vy*32767.0f);
			dv->nz=(short)(v->normal.n.vz*32767.0f);
			dv->f=0;
			dv->u0=(short)(v->uv.uvs[0].u*32767.0f);
			dv->v0=(short)(v->uv.uvs[0].v*32767.0f);

#if 0
			printf("%d %d %d : %d %d %d : %d %d\n",
				   dv->vx,dv->vy,dv->vz,dv->nx,dv->ny,dv->nz,dv->u0,dv->v0);
#endif

		}
	}

    return 1;
}

static int ConvertToCvx(KMX_MDL *mdl,CV2_OBJ *obj,STRIP **strips,int n_strips)
{
    int v_cnt,n_cnt,uv_cnt;
    int i;

    v_cnt=(mdl->n_xverts+1) & ~1;
	n_cnt=(mdl->n_xverts+1) & ~1;
	uv_cnt=(mdl->n_xverts+3) & ~3;

    /* 頂点の展開 */
    obj->n_verts=mdl->n_xverts;
    if(obj->n_verts!=0){
		if((obj->verts=Cv2_Verts_Alloc(obj->n_verts,NULL))==NULL) return 0;
		if((obj->vert_usrdata=Cv2_VertUsrdata_Alloc(obj->n_verts,NULL))
		   ==NULL) return 0;

		for(i=0;i<obj->n_verts;i++){
			memcpy(obj->verts+i,&((mdl->xverts+i)->vertex.v),sizeof(FVECTOR));
			(obj->verts+i)->vw=(mdl->xverts+i)->vertex.env.val[0];

			memcpy(obj->vert_usrdata+i,&((mdl->xverts+i)->vertex.vert_usrdata),sizeof(VERTEX_USERDATA));
		}
    }
    else{
		obj->verts=NULL;
		obj->vert_usrdata=NULL;
    }

    /* 頂点インデックスの展開 */
    obj->n_verts_index=v_cnt;
    if(obj->n_verts_index!=0){
		if((obj->verts_index=Cv2_VertsIndex_Alloc(obj->n_verts_index,NULL))==NULL) return 0;

		memset(obj->verts_index,0x00,sizeof(short)*obj->n_verts_index);

		for(i=0;i<mdl->n_xverts;i++){
			*(obj->verts_index+i)=i;
		}
    }
    else{
		obj->verts_index=NULL;
    }

    /* 法線の展開 */
	obj->n_norms=mdl->n_xverts;
	if(obj->n_norms!=0){
		if((obj->norms=Cv2_Norms_Alloc(obj->n_norms,NULL))==NULL) return 0;
		if((obj->norm_usrdata=Cv2_NormUsrdata_Alloc(obj->n_norms,NULL))
		   ==NULL) return 0;

		for(i=0;i<obj->n_norms;i++){
			memcpy(obj->norms+i,&((mdl->xverts+i)->normal.n),sizeof(FVECTOR));
			(obj->norms+i)->vw=(mdl->xverts+i)->vertex.env.val[0];

			memcpy(obj->norm_usrdata+i,&((mdl->xverts+i)->normal.norm_usrdata),
				   sizeof(NORMAL_USERDATA));
		}
	}
	else{
		obj->norms=NULL;
		obj->norm_usrdata=NULL;
    }

    /* 法線インデックスの展開 */
    obj->n_norms_index=n_cnt;
    if(obj->n_norms_index!=0){
		if((obj->norms_index=Cv2_NormsIndex_Alloc(obj->n_norms_index,NULL))==NULL) return 0;

		memset(obj->norms_index,0x00,sizeof(short)*obj->n_norms_index);

		for(i=0;i<mdl->n_xverts;i++){
			*(obj->norms_index+i)=i;
		}
    }
    else{
		obj->norms_index=NULL;
    }

    /* ＵＶの展開 */
    obj->n_uvs=mdl->n_xverts;
    if(obj->n_uvs!=0){
		if((obj->uvs=Cv2_Uvs_Alloc(obj->n_uvs,NULL))==NULL) return 0;

		for(i=0;i<obj->n_uvs;i++){
			(obj->uvs+i)->vx=(mdl->xverts+i)->uv.uvs[0].u;
			(obj->uvs+i)->vy=(mdl->xverts+i)->uv.uvs[0].v;
			(obj->uvs+i)->vz=0.0f;
			(obj->uvs+i)->vw=0.0f;
		}
    }
    else{
		obj->uvs=NULL;
    }

    /* ＵＶインデックスの展開 */
    obj->n_uvs_index=uv_cnt;
    if(obj->n_uvs_index!=0){
		if((obj->uvs_index=Cv2_UvsIndex_Alloc(obj->n_uvs_index,NULL))==NULL) return 0;

		memset(obj->uvs_index,0x00,sizeof(short)*obj->n_uvs_index);

		for(i=0;i<mdl->n_xverts;i++){
			*(obj->uvs_index+i)=i;
		}
    }
    else{
		obj->uvs_index=NULL;
    }

    return 1;
}


static int SearchXVertex(KMX_PRIMS *prim,int index,KMX_XVERTS *xverts,int size)
{
	int i;
	int ans=-1;

	for(i=0;i<size;i++){
		int j;

		if(*(prim->vid+index)!=(xverts+i)->vid) continue;
		if(*(prim->nid+index)!=(xverts+i)->nid) continue;

		if(prim->n_tid!=(xverts+i)->uv.n_tid) continue;

		for(j=0;j<prim->n_tid;j++){
			int u0,v0;
			int u1,v1;

			if(prim->tid[j]!=(xverts+i)->uv.tid[j]) goto next;

			u0=(int)((prim->uvs+j*prim->n_id+index)->u*32767.0f);
			v0=(int)((prim->uvs+j*prim->n_id+index)->v*32767.0f);
			u1=(int)((xverts+i)->uv.uvs[j].u*32767.0f);
			v1=(int)((xverts+i)->uv.uvs[j].v*32767.0f);

			if(u0!=u1 || v0!=v1) goto next;
		}

		ans=i;
		break;

next:;
	}

	return ans;
}

static void CpXVertex(KMX_MDL *mdl,KMX_PRIMS *prim,int index,KMX_XVERTS *xverts,int xindex)
{
	int j;
	int vid,nid;

	vid=(xverts+xindex)->vid=*(prim->vid+index);
	nid=(xverts+xindex)->nid=*(prim->nid+index);

	memcpy(&((xverts+xindex)->vertex),mdl->verts+vid,sizeof(KMX_VERTS));
	memcpy(&((xverts+xindex)->normal),mdl->norms+nid,sizeof(KMX_NORMS));

#if 0
	printf("v = %d %f %f %f\n",xindex,(xverts+xindex)->vertex.v.vx,
		   (xverts+xindex)->vertex.v.vy,(xverts+xindex)->vertex.v.vz);
	printf("n = %d %f %f %f\n",xindex,(xverts+xindex)->normal.n.vx,
		   (xverts+xindex)->normal.n.vy,(xverts+xindex)->normal.n.vz);
#endif

	(xverts+xindex)->uv.n_tid=prim->n_tid;

	for(j=0;j<prim->n_tid;j++){
		(xverts+xindex)->uv.tid[j]=prim->tid[j];

		(xverts+xindex)->uv.uvs[j].u=(prim->uvs+j*prim->n_id+index)->u;
		(xverts+xindex)->uv.uvs[j].v=(prim->uvs+j*prim->n_id+index)->v;
	}
}


#define XVERTS_UNIT		64

static int MakeKmsxVertex(KMX_MDL *mdl)
{
	KMX_XVERTS *xverts;
	int allocated_size,size;
	KMX_PRIMS *prim;
	int j,k;

	prim=mdl->prims;

	xverts=NULL;
	allocated_size=0;
	size=0;

	for(j=0;j<mdl->n_prims;j++,prim++){
		for(k=0;k<prim->n_id;k++){
			int index;

			// printf("Check 1.1.1\n");

			index=SearchXVertex(prim,k,xverts,size);

			// printf("Check 1.1.2\n");

			if(index<0){
				if(size+1>=allocated_size){
					KMX_XVERTS *new_xverts;

					// printf("Check 1.1.3\n");

					if((new_xverts=Kmx_XVerts_Alloc(allocated_size+XVERTS_UNIT))==NULL){
						if(xverts!=NULL) Kmx_XVerts_Free(xverts);
						return 0;
					}

					// printf("Check 1.1.4\n");

					if(xverts!=NULL){
						memcpy(new_xverts,xverts,sizeof(KMX_XVERTS)*size);
						Kmx_XVerts_Free(xverts);
					}

					// printf("Check 1.1.5\n");

					xverts=new_xverts;
					allocated_size+=XVERTS_UNIT;
				}

				index=size;
				size++;

				// printf("Check 1.1.6\n");

				CpXVertex(mdl,prim,k,xverts,index);
			}

			// printf("Check 1.2.1  %d\n",index);

			*(prim->vid+k)=index;

			// printf("Check 1.3.1\n");
		}
	}

	mdl->n_xverts=size;
	mdl->xverts=xverts;

	return 1;
}

static int Make_Strip(KMX_DEF *def,KMX_MDL *mdl,KMS2_MDL *obj,CV2_OBJ *cobj,
					  int fast_flag,int n_max_points,int multitex_flag,int multi_uv_flag)
{
    DIV_PRIMS *div_prims;
    STRIP **strips;
    int n_strips;
    int i;

    if((div_prims=Kmx2DivPrims(mdl))==NULL) return 0;

    strips=NULL;
    n_strips=0;

    // printf("Check 1\n");

#if 0
    if((strips=MDU_Alloc(sizeof(STRIP *)*32768))==NULL){
		Div_Prims_Free(div_prims);
		return 0;
    }
#endif

    for(i=0;i<div_prims->n_div_prims;i++){

		// printf("div_prims = %d\n",i);

		// printf("FastFlag = %d\n",fast_flag);

#if 0
		if(fast_flag){
			/* 高速のストリップ生成部 */
			strips=EasyStrip(&(div_prims->div_prims[i]),strips,&n_strips,n_max_points);
		}
		else
#endif

		{
			int pre_n_strips=n_strips;
			int tmp_n_strips;

			/* 低速高性能のストリップ生成部 */
			strips=Strip_Algo_1_X(&(div_prims->div_prims[i]),strips,&n_strips,n_max_points);

			tmp_n_strips=n_strips-pre_n_strips;
			tmp_n_strips=Union_STRIP_X(strips+pre_n_strips,tmp_n_strips,n_max_points);
			n_strips=pre_n_strips+tmp_n_strips;
		}
    }

    // printf("Check 2\n");

    Div_Prims_Free(div_prims);
    if(n_strips!=0){
		if(strips==NULL) return 0;

		/* 2002/8/26 Ken Kano 追加
		   半透明表示の際に、よく優先順位の逆転が起こっていたので、
		   それを防ぐためにストリップの順序を、テクスチャのα値の平均の濃い順にして、
		   半透明は必ず後で描画されるようにする。*/
		SortStrip(def->texs,strips,n_strips);
    }

    // printf("Check 3\n");

    /* 各モデルフォーマットに変換 */
    {
		int ans=1;

		/* 頂点情報作成 */
		if(!(ans=MakeKmsxVertex(mdl))) goto end_proc;

		// printf("Check 3.1\n");

		/* KMSX */
		if(!(ans=ConvertToKmsx(def,mdl,obj,strips,n_strips,multitex_flag,multi_uv_flag))) goto end_proc;

		// printf("Check 4\n");

		/* CV2 */
		ans=ConvertToCvx(mdl,cobj,strips,n_strips);

	end_proc:

		// printf("Check 5\n");

		Strips_Free(strips,n_strips);

		// printf("Check 6\n");

		return ans;
    }
}

/* Ｋｍｘ → Ｋｍｓ変換 */
static int Strip(KMX_DEF *kmxdef,KMS2_DEF **def,CV2_OBJS **cdef,int fast_flag,int multitex_flag, int etc_flag, int multi_uv_flag )
{
    KMX_MDL *mdl;
    KMS2_MDL *obj;
    CV2_OBJ *cobj;
    int n_x_models=kmxdef->n_x_models;
    int i;
    int multi_tex=0;
    int n_max_points;

    *def=Kms2_Alloc(kmxdef->n_models,kmxdef->n_x_models,NULL);
    *cdef=Cv2_Alloc(kmxdef->n_x_models,NULL);

    /* シングルテクスチャかマルチテクスチャか確認 */
    for(i=0;i<kmxdef->n_x_models;i++){
		KMX_PRIMS *prim=kmxdef->models[i].prims;
		int n_prims=kmxdef->models[i].n_prims;
		int j;

		for(j=0;j<n_prims;j++,prim++){
			if(prim->n_tid>1){
				multi_tex=1;
				goto forbreak;
			}
		}
    }

forbreak:
    switch(multitex_flag){
    case 0: /* Auto */
		break;
    case 1: /* Single Tex */
		multi_tex=0;
		break;
    case 2: /* Multi Tex */
		multi_tex=1;
		break;
    }

    if(multi_tex){
		(*def)->data_format=MGS_MODEL_MULTITEX;
		n_max_points=MULTI_N_MAX_POINTS;
    }
    else{
		(*def)->data_format=MGS_MODEL_NORM;
		n_max_points=SINGLE_N_MAX_POINTS;
    }

	if( etc_flag & MDU_FLAG_NO_LIMIT ) {
		n_max_points = NOLIMIT_N_MAX_POINTS;
	}
	if( etc_flag & MDU_FLAG_INDEX ) {
		(*def)->data_format|=MGS_MODEL_FLAG_INDEX;
	}
    (*def)->lx=kmxdef->lx;
    (*def)->ly=kmxdef->ly;
    (*def)->lz=kmxdef->lz;
    (*def)->ux=kmxdef->ux;
    (*def)->uy=kmxdef->uy;
    (*def)->uz=kmxdef->uz;
    (*def)->tx=kmxdef->tx;
    (*def)->ty=kmxdef->ty;
    (*def)->tz=kmxdef->tz;

    (*cdef)->flag
		= COMMON_VERTS|COMMON_NORMS|COMMON_UVS|COMMON_VERT_USRDATA|COMMON_NORM_USRDATA;

    mdl=kmxdef->models;
    obj=(*def)->models;
    cobj=(*cdef)->objs;

    /* 進行チェック用 */
    for(i=0;i<n_x_models;i++){
		printf("Obj %d = %d Prims\n",(int)i,(mdl+i)->n_prims);
    }

    /* 進行チェック用 */
    if(fast_flag) printf("Easy Strip start\n");
    else printf("Strip Algo start\n");


    for(i=0;i<n_x_models;i++,mdl++,obj++,cobj++){
		struct timeval pre_t,t;
		long sub_usec,sub_sec;
		float sum_t;

		// pre_t=time(NULL);
		gettimeofday(&pre_t,NULL);

		/* 進行チェック用 */
		printf("Make Strip Obj. %d : ",i);

		if(!Make_Strip(kmxdef,mdl,obj,cobj,fast_flag,n_max_points,multitex_flag,multi_uv_flag)){
			Kms2_Free(*def);
			Cv2_Free(*cdef);

			*def=NULL;
			*cdef=NULL;

			return 0;
		}

		/* 進行チェック用 */
		printf(" O.K.\n");

		// t=time(NULL);
		gettimeofday(&t,NULL);

		sub_usec=t.tv_usec-pre_t.tv_usec;
		sub_sec=t.tv_sec-pre_t.tv_sec;
		sum_t=(float)sub_sec*1000000.0f+(float)sub_usec;

		printf("Prim , Time , Strip , Time/Prim , Time/Strip = %d %f %d , %f %f\n",
			   mdl->n_prims,sum_t,obj->n_packs,
			   sum_t/(float)(mdl->n_prims),
			   sum_t/(float)(obj->n_packs));
    }

    return 1;
}

/* -------------------------------------------------------------------------- */

/* KMSと同時にCVD2も作る */  
int MDU_Kmx2Kmsx(KMX_DEF *kmxdef,KMS2_DEF **def,CV2_OBJS **cdef,int fast_flag,
				 int multitex_flag,int overlap_flag, int etc_flag, int multi_uv_flag )
{
    KMX_DEF	*kmxdef2 ;
    int ans;

    /* 進行チェック用 */
    printf( ".kmx -> .kms2 conversion start\n" ) ;

    /* 三角形分割 */
    if((kmxdef2=KmxDividePrimTriangle(kmxdef))==NULL) return 0;

	/* 重複する法線をまとめる */
	Kmx_RemoveOverlap(kmxdef2,overlap_flag);

    /* ストリップ化とデータコンバート */
    ans=Strip(kmxdef2,def,cdef,fast_flag,multitex_flag, etc_flag, multi_uv_flag );

    Kmx_Free(kmxdef2);

    return ans;
}
