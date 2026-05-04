/*
   Kmx2Kms.c

   Ｋｍｘ → Ｋｍｓコンバート関数

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 3/3/2000

   $Id: Kmx2Kms2.c,v 1.23 2002/08/26 11:22:09 usr01363 Exp $
   
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


/*------------------------------------------------------------------*/

/* 三角形分割 */
KMX_DEF *KmxDividePrimTriangle(KMX_DEF *olddef)
{
    KMX_DEF *def;
    int i,j;

    if((def=Kmx_Alloc(olddef->n_models,olddef->n_x_models))==NULL) return NULL;

    memcpy(def,olddef,sizeof(KMX_DEF)+sizeof(KMX_MDL)*olddef->n_x_models);

    for(i=0;i<def->n_x_models;i++){
		def->models[i].verts=NULL;
		def->models[i].norms=NULL;
		def->models[i].prims=NULL;
    }

    if((def->texs=Kmx_Texs_Alloc(def->n_texs))==NULL){
		Kmx_Free(def);
		return NULL;
    }

    for(i=0;i<def->n_texs;i++){
		if(((def->texs+i)->name=Kmx_Tex_Set((olddef->texs+i)->name))==NULL){
			Kmx_Free(def);
			return NULL;
		}
		(def->texs+i)->alpha=(olddef->texs+i)->alpha;
    }

    for(i=0;i<def->n_x_models;i++){
		KMX_MDL *mdl=def->models+i;
		KMX_MDL *oldmdl=olddef->models+i;
		KMX_PRIMS *prim,*oldprim;

		if(!Kmx_MdlData_Alloc(mdl,mdl->n_verts,mdl->n_norms,mdl->n_prims*2)){
			Kmx_Free(def);
			return NULL;
		}
		memcpy(mdl->verts,oldmdl->verts,sizeof(KMX_VERTS)*mdl->n_verts);
		memcpy(mdl->norms,oldmdl->norms,sizeof(KMX_NORMS)*mdl->n_norms);

		prim=mdl->prims;
		oldprim=oldmdl->prims;
		mdl->n_prims=0;
		for(j=0;j<oldmdl->n_prims;j++,oldprim++){
			int k;

			switch(oldprim->n_id){
			case 3:
				if(!Kmx_PrimData_Alloc(prim,3,oldprim->n_tid)){
					Kmx_Free(def);
					return NULL;
				}

				*(prim->vid+0)=*(oldprim->vid+0);
				*(prim->vid+1)=*(oldprim->vid+1);
				*(prim->vid+2)=*(oldprim->vid+2);

				*(prim->nid+0)=*(oldprim->nid+0);
				*(prim->nid+1)=*(oldprim->nid+1);
				*(prim->nid+2)=*(oldprim->nid+2);

				for(k=0;k<oldprim->n_tid;k++){
					*(prim->tid+k)=*(oldprim->tid+k);
					*(prim->uvs+3*k+0)=*(oldprim->uvs+3*k+0);
					*(prim->uvs+3*k+1)=*(oldprim->uvs+3*k+1);
					*(prim->uvs+3*k+2)=*(oldprim->uvs+3*k+2);
				}

				memcpy(&(prim->prim_usrdata),&(oldprim->prim_usrdata),
					   sizeof(PRIMITIVE_USERDATA));

				prim++;
				mdl->n_prims++;
				break;

			case 4:
				if(!Kmx_PrimData_Alloc(prim+0,3,oldprim->n_tid)){
					Kmx_Free(def);
					return NULL;
				}
				if(!Kmx_PrimData_Alloc(prim+1,3,oldprim->n_tid)){
					Kmx_Free(def);
					return NULL;
				}

				*((prim+0)->vid+0)=*(oldprim->vid+0);
				*((prim+0)->vid+1)=*(oldprim->vid+1);
				*((prim+0)->vid+2)=*(oldprim->vid+3);

				*((prim+1)->vid+0)=*(oldprim->vid+3);
				*((prim+1)->vid+1)=*(oldprim->vid+1);
				*((prim+1)->vid+2)=*(oldprim->vid+2);

				*((prim+0)->nid+0)=*(oldprim->nid+0);
				*((prim+0)->nid+1)=*(oldprim->nid+1);
				*((prim+0)->nid+2)=*(oldprim->nid+3);

				*((prim+1)->nid+0)=*(oldprim->nid+3);
				*((prim+1)->nid+1)=*(oldprim->nid+1);
				*((prim+1)->nid+2)=*(oldprim->nid+2);

				for(k=0;k<oldprim->n_tid;k++){
					*((prim+0)->tid+k)=*(oldprim->tid+k);
					*((prim+0)->uvs+3*k+0)=*(oldprim->uvs+4*k+0);
					*((prim+0)->uvs+3*k+1)=*(oldprim->uvs+4*k+1);
					*((prim+0)->uvs+3*k+2)=*(oldprim->uvs+4*k+3);

					*((prim+1)->tid+k)=*(oldprim->tid+k);
					*((prim+1)->uvs+3*k+0)=*(oldprim->uvs+4*k+3);
					*((prim+1)->uvs+3*k+1)=*(oldprim->uvs+4*k+1);
					*((prim+1)->uvs+3*k+2)=*(oldprim->uvs+4*k+2);
				}

				memcpy(&((prim+0)->prim_usrdata),&(oldprim->prim_usrdata),
					   sizeof(PRIMITIVE_USERDATA));
				memcpy(&((prim+1)->prim_usrdata),&(oldprim->prim_usrdata),
					   sizeof(PRIMITIVE_USERDATA));

				prim+=2;
				mdl->n_prims+=2;
				break;

			default:
				printf("Error : Strange Polygon ( N Vertex = %d )\n",oldprim->n_id);
				break;
			}
		}
    }

    return def;
}

/* -------------------------------------------------------------------------- */

static int ConvertToKms2(KMX_DEF *def,KMX_MDL *mdl,KMS2_MDL *obj,STRIP **strips,int n_strips,
						 int multitex_flag)
{
    int i,j;
    int mdl_type;

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
		for(j=0;j<n_tid;j++){
			tid[j]=MDU_GetStrCode(def->texs[*((*(strips+i))->tid+j)].name);

#if 0
			/* テスト */
			if(tid[j]==4710569) tid[j]=0;
#endif
		}

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
					uv_index[2]=tmp;
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

#if 1
		if(multitex_flag==1){
			/* Single Texture */
			if(n_tid>1) n_tid=1;
			flag&=~(DG_PACKFLAG_EMAP|DG_PACKFLAG_SMAP|DG_PACKFLAG_BMAP);
		}
#else
		if(n_tid>1) n_tid=1;
#endif

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


		/* 頂点数の決定 */
		n_verts=(obj->packs+i)->n_verts=(*(strips+i))->n_points;


		if(((obj->packs+i)->verts=Kms2_Verts_Alloc(n_verts,NULL))==NULL){
			Kms2_MdlPack_Free(obj->packs,obj->n_packs);
			return 0;
		}
		for(j=0;j<n_verts;j++){
			int vid=(*(strips+i))->p[j].vid;

			/* 頂点の展開 */
			*((obj->packs+i)->verts+j*4+0)=(short)((mdl->verts+vid)->v.vx);
			*((obj->packs+i)->verts+j*4+1)=(short)((mdl->verts+vid)->v.vy);
			*((obj->packs+i)->verts+j*4+2)=(short)((mdl->verts+vid)->v.vz);

			/* シングルエンベロープの展開 */
			*((obj->packs+i)->verts+j*4+3)
				=(short)((mdl->verts+vid)->env.val[0]*4096.0f);
		}


		if(((obj->packs+i)->norms=Kms2_Norms_Alloc(n_verts,NULL))==NULL){
			Kms2_MdlPack_Free(obj->packs,obj->n_packs);
			return 0;
		}
		for(j=0;j<n_verts;j++){
			int nid=(*(strips+i))->p[j].nid;

			/* 法線の展開 */
			*((obj->packs+i)->norms+j*4+0)=(short)((mdl->norms+nid)->n.vx*4096.0f);
			*((obj->packs+i)->norms+j*4+1)=(short)((mdl->norms+nid)->n.vy*4096.0f);
			*((obj->packs+i)->norms+j*4+2)=(short)((mdl->norms+nid)->n.vz*4096.0f);

			/* 描画フラグ */
#if 1
			*((obj->packs+i)->norms+j*4+3)=(*(strips+i))->p[j].draw_flag;
#elif 0
			*((obj->packs+i)->norms+j*4+3)=(*(strips+i))->p[j].draw_flag;
			if(mdl->type & (DG_TYPE_TRANS | DG_TYPE_OVERLAY0 |
							DG_TYPE_OVERLAY1 | DG_TYPE_OVERLAY2)){

				*((obj->packs+i)->norms+j*4+3)=0x8000;
			}
#else
			if(n_tid==0){
				*((obj->packs+i)->norms+j*4+3)=0x8000; /* 表示しない */
			}
			else{
				*((obj->packs+i)->norms+j*4+3)=(*(strips+i))->p[j].draw_flag;
			}
#endif

			if(!(mdl_type & (DG_TYPE_SINGLESIDE | DG_TYPE_AUTOSIDE))){
				/* 両面ポリゴン */
				*((obj->packs+i)->norms+j*4+3)|=0x0fff;
			}
		}

		if(flag & DG_PACKFLAG_TEX0){
			(obj->packs+i)->tex_id[0]=tid[uv_index[0]];
		}
		else{
			(obj->packs+i)->tex_id[0]=0;
		}
		if(flag & DG_PACKFLAG_UV0){
			if(((obj->packs+i)->uvs[0]=Kms2_Uvs_Alloc(n_verts,NULL))==NULL){
				Kms2_MdlPack_Free(obj->packs,obj->n_packs);
				return 0;
			}

			for(j=0;j<n_verts;j++){
				/* ＵＶの展開 */
				*((obj->packs+i)->uvs[0]+j*2+0)
					=(short)(((*(strips+i))->p[j].uvs+uv_index[0])->u*4096.0f);
				*((obj->packs+i)->uvs[0]+j*2+1)
					=(short)(((*(strips+i))->p[j].uvs+uv_index[0])->v*4096.0f);
			}
		}


		if(flag & DG_PACKFLAG_TEX1){
			(obj->packs+i)->tex_id[1]=tid[uv_index[1]];
		}
		else{
			(obj->packs+i)->tex_id[1]=0;
		}
		if(flag & DG_PACKFLAG_UV1){
			if(((obj->packs+i)->uvs[1]=Kms2_Uvs_Alloc(n_verts,NULL))==NULL){
				Kms2_MdlPack_Free(obj->packs,obj->n_packs);
				return 0;
			}

			for(j=0;j<n_verts;j++){
				/* ＵＶの展開 */
#if 1
				*((obj->packs+i)->uvs[1]+j*2+0)
					=(short)(((*(strips+i))->p[j].uvs+uv_index[1])->u*4096.0f);
				*((obj->packs+i)->uvs[1]+j*2+1)
					=(short)(((*(strips+i))->p[j].uvs+uv_index[1])->v*4096.0f);
#else
				*((obj->packs+i)->uvs[1]+j*2+0)
					=(short)(((*(strips+i))->p[j].uvs+uv_index[0])->u*4096.0f);
				*((obj->packs+i)->uvs[1]+j*2+1)
					=(short)(((*(strips+i))->p[j].uvs+uv_index[0])->v*4096.0f);
#endif
			}
		}


		if(flag & DG_PACKFLAG_TEX2){
			(obj->packs+i)->tex_id[2]=tid[uv_index[2]];
		}
		else{
			(obj->packs+i)->tex_id[2]=0;
		}
		if(flag & DG_PACKFLAG_UV2){
			if(((obj->packs+i)->uvs[2]=Kms2_Uvs_Alloc(n_verts,NULL))==NULL){
				Kms2_MdlPack_Free(obj->packs,obj->n_packs);
				return 0;
			}

			for(j=0;j<n_verts;j++){
				/* ＵＶの展開 */
				*((obj->packs+i)->uvs[2]+j*2+0)
					=(short)(((*(strips+i))->p[j].uvs+uv_index[2])->u*4096.0f);
				*((obj->packs+i)->uvs[2]+j*2+1)
					=(short)(((*(strips+i))->p[j].uvs+uv_index[2])->v*4096.0f);
			}
		}
    }

    return 1;
}

static int ConvertToCv2(KMX_MDL *mdl,CV2_OBJ *obj,STRIP **strips,int n_strips)
{
    int v_cnt,n_cnt,uv_cnt;
    int i,j;

    v_cnt=n_cnt=uv_cnt=0;

    for(i=0;i<n_strips;i++){
		int n_points=(*(strips+i))->n_points;

		v_cnt+=(n_points+1) & ~1;
		n_cnt+=(n_points+1) & ~1;
		uv_cnt+=(n_points+3) & ~3;
    }

    /* 頂点の展開 */
    obj->n_verts=mdl->n_verts;
    if(obj->n_verts!=0){
		if((obj->verts=Cv2_Verts_Alloc(obj->n_verts,NULL))==NULL) return 0;
		if((obj->vert_usrdata=Cv2_VertUsrdata_Alloc(obj->n_verts,NULL))
		   ==NULL) return 0;

		for(i=0;i<obj->n_verts;i++){
			memcpy(obj->verts+i,&((mdl->verts+i)->v),sizeof(FVECTOR));
			(obj->verts+i)->vw=(mdl->verts+i)->env.val[0];

			memcpy(obj->vert_usrdata+i,&((mdl->verts+i)->vert_usrdata),sizeof(VERTEX_USERDATA));
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

		for(i=0,v_cnt=0;i<n_strips;i++){
			for(j=0;j<(*(strips+i))->n_points;j++){
				*(obj->verts_index+v_cnt)=(*(strips+i))->p[j].vid;
				v_cnt++;
			}
			if(v_cnt & 1){
				*(obj->verts_index+v_cnt)=0;
				v_cnt++;
			}
		}
    }
    else{
		obj->verts_index=NULL;
    }

    /* 法線の展開、頂点と同数にすることを試みるため、
       法線のチェックも行なう */
    {
		int *check_n;

		obj->n_norms=mdl->n_verts;
		if(obj->n_norms!=0){
			if((obj->norms=Cv2_Norms_Alloc(obj->n_norms,NULL))==NULL) return 0;
			if((obj->norm_usrdata=Cv2_NormUsrdata_Alloc(obj->n_norms,NULL))
			   ==NULL) return 0;

			if((check_n=(int *)MDU_Alloc(sizeof(int)*obj->n_norms))==NULL) return 0;
			for(i=0;i<obj->n_norms;i++) *(check_n+i)=0;

			for(i=0;i<n_strips;i++){
				for(j=0;j<(*(strips+i))->n_points;j++){
					int vid=(*(strips+i))->p[j].vid;
					int nid=(*(strips+i))->p[j].nid;

					if(*(check_n+vid)){
						int vid2=vid;

						if((obj->norms+vid)->vx!=(mdl->norms+nid)->n.vx ||
						   (obj->norms+vid)->vy!=(mdl->norms+nid)->n.vy ||
						   (obj->norms+vid)->vz!=(mdl->norms+nid)->n.vz){

							FVECTOR *norms2;
							NORMAL_USERDATA *norm_usrdata2;

							if((norms2=Cv2_Norms_Alloc(obj->n_norms+1,NULL))==NULL){
								MDU_Free(check_n);
								return 0;
							}

							if((norm_usrdata2=Cv2_NormUsrdata_Alloc(obj->n_norms+1,NULL))
							   ==NULL){

								MDU_Free(check_n);
								return 0;
							}

							memcpy(norms2,obj->norms,sizeof(FVECTOR)*obj->n_norms);
							MDU_Free(obj->norms);
							obj->norms=norms2;

							memcpy(norm_usrdata2,obj->norm_usrdata,
								   sizeof(NORMAL_USERDATA)*obj->n_norms);
							MDU_Free(obj->norm_usrdata);
							obj->norm_usrdata=norm_usrdata2;

							vid2=obj->n_norms;
							obj->n_norms++;
						}

						memcpy(obj->norms+vid2,&((mdl->norms+nid)->n),sizeof(FVECTOR));
						(obj->norms+vid2)->vw=(mdl->verts+vid)->env.val[0];

						memcpy(obj->norm_usrdata+vid2,&((mdl->norms+nid)->norm_usrdata),
							   sizeof(NORMAL_USERDATA));

						(*(strips+i))->p[j].nid=vid2;
					}
					else{
						memcpy(obj->norms+vid,&((mdl->norms+nid)->n),sizeof(FVECTOR));
						(obj->norms+vid)->vw=(mdl->verts+vid)->env.val[0];

						(*(strips+i))->p[j].nid=vid;
						*(check_n+vid)=1;
					}
				}
			}

			MDU_Free(check_n);
		}
		else{
			obj->norms=NULL;
			obj->norm_usrdata=NULL;
		}
    }

    /* 法線インデックスの展開 */
    obj->n_norms_index=n_cnt;
    if(obj->n_norms_index!=0){
		if((obj->norms_index=Cv2_NormsIndex_Alloc(obj->n_norms_index,NULL))==NULL) return 0;

		for(i=0,n_cnt=0;i<n_strips;i++){
			for(j=0;j<(*(strips+i))->n_points;j++){
				*(obj->norms_index+n_cnt)=(*(strips+i))->p[j].nid;
				n_cnt++;
			}
			if(n_cnt & 1){
				*(obj->norms_index+n_cnt)=0;
				n_cnt++;
			}
		}
    }
    else{
		obj->norms_index=NULL;
    }

    /* ＵＶの展開、元々そのようなデータは存在しないので、新たにチェック作成する */
    {
		u_int *tid;
		int *check_uv;

		obj->n_uvs=obj->n_verts;
		obj->n_uvs_index=uv_cnt;

		if(obj->n_uvs==0 || obj->n_uvs_index==0){
			obj->n_uvs=obj->n_uvs_index=0;
			obj->uvs=NULL;
			obj->uvs_index=NULL;
			return 1;
		}

		if((obj->uvs=Cv2_Uvs_Alloc(obj->n_uvs,NULL))==NULL) return 0;
		if((obj->uvs_index=Cv2_UvsIndex_Alloc(obj->n_uvs_index,NULL))==NULL) return 0;

		if((tid=(u_int *)MDU_Alloc(sizeof(u_int)*obj->n_uvs))==NULL) return 0;

		if((check_uv=(int *)MDU_Alloc(sizeof(int)*obj->n_uvs))==NULL){
			MDU_Free(tid);
			return 0;
		}
		for(i=0;i<obj->n_uvs;i++) *(check_uv+i)=0;

		for(i=0,uv_cnt=0;i<n_strips;i++){

			if((*(strips+i))->n_tid>0){

				for(j=0;j<(*(strips+i))->n_points;j++){
					int vid=(*(strips+i))->p[j].vid;

					if(*(check_uv+vid)){
						int vid2=vid;

						if((obj->uvs+vid)->vx!=((*(strips+i))->p[j].uvs+0)->u ||
						   (obj->uvs+vid)->vy!=((*(strips+i))->p[j].uvs+0)->v ||
						   *(tid+vid)!=*((*(strips+i))->tid+0)){

							FVECTOR *uvs2;
							u_int *tid2;

							if((uvs2=Cv2_Uvs_Alloc(obj->n_uvs+1,NULL))==NULL){
								MDU_Free(tid);
								MDU_Free(check_uv);
								return 0;
							}

							if((tid2=(u_int *)MDU_Alloc(sizeof(u_int)*(obj->n_uvs+1)))==NULL){
								MDU_Free(uvs2);
								MDU_Free(tid);
								MDU_Free(check_uv);
								return 0;
							}

							memcpy(uvs2,obj->uvs,sizeof(FVECTOR)*obj->n_uvs);
							memcpy(tid2,tid,sizeof(u_int)*obj->n_uvs);
							MDU_Free(obj->uvs);
							MDU_Free(tid);
							obj->uvs=uvs2;
							tid=tid2;

							vid2=obj->n_uvs;
							obj->n_uvs++;
						}
						(obj->uvs+vid2)->vx=((*(strips+i))->p[j].uvs+0)->u;
						(obj->uvs+vid2)->vy=((*(strips+i))->p[j].uvs+0)->v;
						(obj->uvs+vid2)->vz=0.0f;
						(obj->uvs+vid2)->vw=0.0f;

						*(tid+vid2)=*((*(strips+i))->tid+0);

						*(obj->uvs_index+uv_cnt)=vid2;
					}
					else{
						(obj->uvs+vid)->vx=((*(strips+i))->p[j].uvs+0)->u;
						(obj->uvs+vid)->vy=((*(strips+i))->p[j].uvs+0)->v;
						(obj->uvs+vid)->vz=0.0f;
						(obj->uvs+vid)->vw=0.0f;

						*(tid+vid)=*((*(strips+i))->tid+0);

						*(obj->uvs_index+uv_cnt)=vid;

						*(check_uv+vid)=1;
					}

					uv_cnt++;
				}
			}
			while(uv_cnt & 3){
				*(obj->uvs_index+uv_cnt)=0;
				uv_cnt++;
			}
		}

		MDU_Free(tid);
		MDU_Free(check_uv);
    }

    return 1;
}

static int Make_Strip(KMX_DEF *def,KMX_MDL *mdl,KMS2_MDL *obj,CV2_OBJ *cobj,
					  int fast_flag,int n_max_points,int multitex_flag)
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

		if(fast_flag){
			/* 高速のストリップ生成部 */
			strips=EasyStrip(&(div_prims->div_prims[i]),strips,&n_strips,n_max_points);
		}
		else{
			int pre_n_strips=n_strips;
			int tmp_n_strips;

			/* 低速高性能のストリップ生成部 */
			strips=Strip_Algo_1(&(div_prims->div_prims[i]),strips,&n_strips,n_max_points);

			tmp_n_strips=n_strips-pre_n_strips;
			tmp_n_strips=Union_STRIP(strips+pre_n_strips,tmp_n_strips,n_max_points);
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

		/* KMS2 */
		if(!(ans=ConvertToKms2(def,mdl,obj,strips,n_strips,multitex_flag))) goto end_proc;

		// printf("Check 4\n");

		/* CV2 */
		ans=ConvertToCv2(mdl,cobj,strips,n_strips);

	end_proc:

		// printf("Check 5\n");

		Strips_Free(strips,n_strips);

		// printf("Check 6\n");

		return ans;
    }
}

/* Ｋｍｘ → Ｋｍｓ変換 */
static int Strip(KMX_DEF *kmxdef,KMS2_DEF **def,CV2_OBJS **cdef,int fast_flag,int multitex_flag, int etc_flag)
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

		if(!Make_Strip(kmxdef,mdl,obj,cobj,fast_flag,n_max_points,multitex_flag)){
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

static int CreateIndex( KMS2_DEF* pdef, int etc_flag )
{
	int i, j, k;
	KMS2_MDL* pmdl = pdef->models;

	if( etc_flag & MDU_FLAG_INDEX ) {
		// インデックスをつける
		pdef->data_format |= MGS_MODEL_FLAG_INDEX;

		for( i = pdef->n_x_models ; i > 0 ; --i , pmdl++ ) {
			KMS2_MDLPACK* ppack = pmdl->packs;
			unsigned int count = 0;
			for ( j = pmdl->n_packs ; j > 0 ; --j, ppack++ ) {
				// 必要メモリを数える
				int size = 2;	// 最初２点は書き込む
				short* pn = (short*)ppack->norms + 4 * 2;
				unsigned short* pIndex;

				if( !( ppack->flag & (DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ) ) {
					// カリングなし
					for( k = 2; k < ppack->n_verts ; k++ ) { 
						if( pn[3] & 0x8000 ) size++;	// NO_KICK エミュレートのため頂点が増える
						size++;
						pn += 4;
					}

					if( (pIndex = ppack->index = MDU_Alloc( size * sizeof( unsigned short ) ) ) == NULL ) {
						return 0;
					}
				
					// 実際に書き込む
					ppack->n_indices = size;

					pn = (short*)ppack->norms + 4 * 2;

					*pIndex++ = (unsigned short)count++;
					*pIndex++ = (unsigned short)count++;	// 最初二点
				
					for( k = 2; k < ppack->n_verts ; k++ ) { 
						if( pn[3] & 0x8000 ) *pIndex++ = (unsigned short)count - 1;
						*pIndex++ = (unsigned short)count++;
						pn += 4;
					}
					if( count > 0xffff ) {	// index が short -> 1 model に 65536 頂点まで
						printf("Too many vertexes in 1 model\n");
						return 0;
					}
				} else {
					// カリングあり
					int		cull_flag = 0x00 ;
					for( k = 2; k < ppack->n_verts ; k++ ) { 
						if( pn[3] & 0x8000 ) {
							cull_flag = 0x20 - cull_flag;
							size++;	// NO_KICK エミュレートのため頂点が増える
						} else if ( ( pn[3] & 0xff ) == cull_flag ) {
							cull_flag = 0x20 - cull_flag; 
							size++;	// 回転方向を合わせるために頂点が増える
						}
						size++;
						cull_flag = 0x20 - cull_flag ;
						pn += 4;
					}

					if( (pIndex = ppack->index = MDU_Alloc( size * sizeof( unsigned short ) ) ) == NULL ) {
						return 0;
					}
				
					// 実際に書き込む
					ppack->n_indices = size;
					cull_flag = 0;

					pn = (short*)ppack->norms + 4 * 2;

					*pIndex++ = (unsigned short)count++;
					*pIndex++ = (unsigned short)count++;	// 最初二点
				
					for( k = 2; k < ppack->n_verts ; k++ ) { 
						if( pn[3] & 0x8000 ) {
							*pIndex++ = (unsigned short)count - 1;	
							cull_flag = 0x20 - cull_flag;
						} else if ( ( pn[3] & 0xff ) == cull_flag ) {
							*(pIndex - 1) = (unsigned short)count - 2;
							*pIndex++ = (unsigned short)count - 1;
							cull_flag = 0x20 - cull_flag;
						}
						*pIndex++ = (unsigned short)count++;
						cull_flag = 0x20 - cull_flag ;
						pn += 4;
					}
					if( count > 0xffff ) {	// index が short -> 1 model に 65536 頂点まで
						printf("Too many vertexes in 1 model\n");
						return 0;
					}
				}
			}
		}
	} else {
		// インデックス関係のメンバを一応リセット
		for( i = pdef->n_x_models ; i > 0 ; --i , pmdl++ ) {
			KMS2_MDLPACK* ppack = pmdl->packs;
			for ( j = pmdl->n_packs ; j > 0 ; --j, ppack++ ) {
				ppack->n_indices = 0;
				ppack->index = NULL;
			}
		}
	}
	return 1;
}
/* -------------------------------------------------------------------------- */

/* KMSと同時にCVD2も作る */  
int MDU_Kmx2Kms2(KMX_DEF *kmxdef,KMS2_DEF **def,CV2_OBJS **cdef,int fast_flag,
				 int multitex_flag,int overlap_flag, int etc_flag )
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
    ans=Strip(kmxdef2,def,cdef,fast_flag,multitex_flag, etc_flag );

    Kmx_Free(kmxdef2);

	/* インデックスをつける */
	if( ans ) {
		ans = CreateIndex( *def, etc_flag );
	}

    return ans;
}
