/*
   Evf2Evm.c

   構造体用メモリ管理関数群

   by K.Kano , 3/6/2000

   $Id: Evf2Evm.c,v 1.21 2002/08/26 11:22:09 usr01363 Exp $
   
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
#include "fmt_kms.h"
#include "fmt_km4.h"

#include "MDU_util.h"
#include "MDU_mdl.h"

#include "block.h"

#include "ToStrip.h"

#include "Evf.h"

extern	int	MDU_M2K_N_ObjFlags ;
extern	char	MDU_M2K_ObjNames[][ 256 ] ;
extern	int	MDU_M2K_ObjFlags[] ;


#define N_MAX_POINTS	32
#define NOLIMIT_N_MAX_POINTS	0xffff	// unsigned short 限界


/*-----------------------------------------------------------------*/

static EVM_PACK *ConvertToEvm(EVF_DEF *def,STRIP **strips,int n_strips,int fix_flag)
{
    int i,j;
    EVM_PACK *packet;
    EVF_MESH *mesh=&(def->mesh);
    int mdl_type;

    /* 頂点の確認 */
    for(i=0;i<mesh->n_verts;i++){
		if((mesh->verts+i)->v.vx<-2048.0f || (mesh->verts+i)->v.vx>=2048.0f ||
		   (mesh->verts+i)->v.vy<-2048.0f || (mesh->verts+i)->v.vy>=2048.0f ||
		   (mesh->verts+i)->v.vz<-2048.0f || (mesh->verts+i)->v.vz>=2048.0f){

			fix_flag=1;
			break;
		}
    }

    if(n_strips!=0){
		if((packet=Evm_Pack_Alloc(n_strips,NULL))==NULL) return NULL;
    }
    else{
		packet=NULL;
    }

    /* 頂点の範囲の設定 */
    if(fix_flag) def->type|=1; /* -32768 - 32767 */
    else def->type&=~1; /* -2048.0 - 2047.9375 */


#if 0
    mdl_type=def->type;
#else
	mdl_type=0;
	for(i=0;i<MDU_M2K_N_ObjFlags;i++){
		mdl_type|=MDU_M2K_ObjFlags[i];
	}
#endif

    for(i=0;i<n_strips;i++){
		STRIP *s=*(strips+i);
		int n_verts;
		int n_tid;
		int flag=0;
		int tid[N_EVM_MULTI_TEXTURES];
		int uv_index[N_EVM_MULTI_TEXTURES];

		/* テクスチャの数のチェック */
		n_tid=s->n_tid;
		if(n_tid>N_EVM_MULTI_TEXTURES) n_tid=N_EVM_MULTI_TEXTURES;

		/* テクスチャの属性チェック */
		for(j=0;j<n_tid;j++){
			tid[j]=MDU_GetStrCode(def->texs[*(s->tid+j)].name);
			uv_index[j]=j;
		}

		for(j=0;j<n_tid; ){
			int eff;

			eff=IsEffectTexture(def->texs[*(s->tid+uv_index[j])].name);

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
#if 1
				if(j==0){
					int tmp;
					tmp=uv_index[0];
					uv_index[0]=uv_index[1];
					uv_index[1]=tmp;
				}
#else
				{
					int k,tmp;

					tmp=uv_index[j];
					for(k=j+1;k<n_tid;k++){
						uv_index[k-1]=uv_index[k];
					}
					uv_index[2]=tmp;
				}
#endif
				flag|=eff;
#if 0
				n_tid--;
				continue;
#else
				break;
#endif
			}

			j++;
		}

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
#if 0
		if(mdl_type & DG_TYPE_SINGLESIDE){
			flag|=DG_PACKFLAG_CULLON;
		}
		else if(mdl_type & DG_TYPE_AUTOSIDE){
			flag|=DG_PACKFLAG_CULLAUTO;
		}
#else
		if(mdl_type & DG_TYPE_SINGLESIDE){
			flag|=DG_PACKFLAG_CULLON;
		}
		else if(mdl_type & DG_TYPE_AUTOSIDE){
			if(n_tid>0 && IsOverlayTexture(def->texs[*(s->tid+uv_index[0])].name)){
				/* 半透明の時にON */
				flag|=DG_PACKFLAG_CULLON;
			}
		}
#endif

		(packet+i)->flag=flag;


		/* 頂点数の決定 */
		n_verts=(packet+i)->n_verts=s->n_points;


		if(((packet+i)->verts=Evm_Verts_Alloc(n_verts,NULL))==NULL){
			Evm_Pack_Free(packet,n_strips);
			return NULL;
		}
		for(j=0;j<n_verts;j++){
			int vid=s->p[j].vid;

			if(fix_flag){
				/* 頂点の展開 (1.15.0)の場合 */
				*((short *)((packet+i)->verts)+j*4+0)=(short)((mesh->verts+vid)->v.vx);
				*((short *)((packet+i)->verts)+j*4+1)=(short)((mesh->verts+vid)->v.vy);
				*((short *)((packet+i)->verts)+j*4+2)=(short)((mesh->verts+vid)->v.vz);
			}
			else{
				/* 頂点の展開 (1.11.4)の場合 */
				*((short *)((packet+i)->verts)+j*4+0)=(short)((mesh->verts+vid)->v.vx*16.0f);
				*((short *)((packet+i)->verts)+j*4+1)=(short)((mesh->verts+vid)->v.vy*16.0f);
				*((short *)((packet+i)->verts)+j*4+2)=(short)((mesh->verts+vid)->v.vz*16.0f);
			}

			/* 描画フラグ */
			*((short *)((packet+i)->verts)+j*4+3)=s->p[j].draw_flag;

#if 1
			if(!(flag & DG_PACKFLAG_CULLON)){
				/* 両面ポリゴン */
				*((short *)((packet+i)->verts)+j*4+3)|=0x0fff;
			}
#else
			*((short *)((packet+i)->verts)+j*4+3)|=0x0fff;
#endif
		}


		if(((packet+i)->norms=Evm_Norms_Alloc(n_verts,NULL))==NULL){
			Evm_Pack_Free(packet,n_strips);
			return NULL;
		}
		for(j=0;j<n_verts;j++){
			int nid=s->p[j].nid;

			/* 法線の展開 */
			*((short *)((packet+i)->norms)+j*4+0)=(short)((mesh->norms+nid)->n.vx*4096.0f);
			*((short *)((packet+i)->norms)+j*4+1)=(short)((mesh->norms+nid)->n.vy*4096.0f);
			*((short *)((packet+i)->norms)+j*4+2)=(short)((mesh->norms+nid)->n.vz*4096.0f);
			*((short *)((packet+i)->norms)+j*4+3)=0;
		}


		if(flag & DG_PACKFLAG_TEX0){
			(packet+i)->tex_id[0]=tid[uv_index[0]];
		}
		else{
			(packet+i)->tex_id[0]=0;
		}
		if(flag & DG_PACKFLAG_UV0){
			if(((packet+i)->uvs[0]=Evm_Uvs_Alloc(n_verts,NULL))==NULL){
				Evm_Pack_Free(packet,n_strips);
				return NULL;
			}

			for(j=0;j<n_verts;j++){
				/* ＵＶの展開 */
				*((short *)((packet+i)->uvs[0])+j*4+0)
					=(short)((s->p[j].uvs+uv_index[0])->u*4096.0f);
				*((short *)((packet+i)->uvs[0])+j*4+1)
					=(short)((s->p[j].uvs+uv_index[0])->v*4096.0f);
				*((short *)((packet+i)->uvs[0])+j*4+2)
					=(short)((s->p[j].uvs+uv_index[0])->w*4096.0f);
				*((short *)((packet+i)->uvs[0])+j*4+3)
					=0;
			}
		}


		if(flag & DG_PACKFLAG_TEX1){
			(packet+i)->tex_id[1]=tid[uv_index[1]];
		}
		else{
			(packet+i)->tex_id[1]=0;
		}
		if(flag & DG_PACKFLAG_UV1){
			if(((packet+i)->uvs[1]=Evm_Uvs_Alloc(n_verts,NULL))==NULL){
				Evm_Pack_Free(packet,n_strips);
				return NULL;
			}

			for(j=0;j<n_verts;j++){
				/* ＵＶの展開 */
				*((short *)((packet+i)->uvs[1])+j*4+0)
					=(short)((s->p[j].uvs+uv_index[1])->u*4096.0f);
				*((short *)((packet+i)->uvs[1])+j*4+1)
					=(short)((s->p[j].uvs+uv_index[1])->v*4096.0f);
				*((short *)((packet+i)->uvs[1])+j*4+2)
					=(short)((s->p[j].uvs+uv_index[1])->w*4096.0f);
				*((short *)((packet+i)->uvs[1])+j*4+3)
					=0;
			}
		}


		if(flag & DG_PACKFLAG_TEX2){
			(packet+i)->tex_id[2]=tid[uv_index[2]];
		}
		else{
			(packet+i)->tex_id[2]=0;
		}

		if(flag & DG_PACKFLAG_BMAP){
			if(((packet+i)->uvs[2]=Evm_Uvs_Alloc(n_verts,NULL))==NULL){
				Evm_Pack_Free(packet,n_strips);
				return NULL;
			}

			{
				FVECTOR *p0,*p1,*p2;
				/* int calc_flag=0; */

				p0=p1=p2=NULL;

				/* バンプマッピングの際に用いるベクトルを
				   計算し、UV2に入れる */
				for(j=0;j<n_verts;j++){
#if 0
					int vid=s->p[j].vid;

					p0=p1; p1=p2;
					p2=&((mesh->verts+vid)->v);
					calc_flag=(calc_flag<<1) | 1;

					if(!(s->p[j].draw_flag & 0x8000)){
						/* 描画フラグが立った場合 */
						FVECTOR l0,l1;
						TVECTOR d0,d1;

						/* Ｕ方向に当たるベクトルを算出し、それをUV値として与える */
						l0.vx=p1->vx-p0->vx;
						l0.vy=p1->vy-p0->vy;
						l0.vz=p1->vz-p0->vz;
						l1.vx=p2->vx-p0->vx;
						l1.vy=p2->vy-p0->vy;
						l1.vz=p2->vz-p0->vz;

						d0.u=(s->p[j-1].uvs+uv_index[1])->u-(s->p[j-2].uvs+uv_index[1])->u;
						d0.v=(s->p[j-1].uvs+uv_index[1])->v-(s->p[j-2].uvs+uv_index[1])->v;
						d1.u=(s->p[j].uvs+uv_index[1])->u-(s->p[j-2].uvs+uv_index[1])->u;
						d1.v=(s->p[j].uvs+uv_index[1])->v-(s->p[j-2].uvs+uv_index[1])->v;

						/* Ｖ成分を消すことで、Ｕ方向のみのベクトルを算出
						   ( d0.u d0.v ) = l0
						   ( d1.u d1.v ) = l1

						   ( d0.u                     d0.v )             = l0
						 - ( d1.u                     d1.v ) * d0.v/d1.v = l1 * d0.v/d1.v
						 ----------------------------------------------------------------------
						   ( d0.u - d1.u * d0.v/d1.v  0    )             = l0 - l1 * d0.v/d1.v    */


						/* d1.v==0.0fの場合は、l1がそのままＵ方向となるのでそのまま素通し */
						if(d1.v!=0.0f){
							d1.u=d0.u-d1.u*d0.v/d1.v;

							l1.vx=l0.vx-l1.vx*d0.v/d1.v;
							l1.vy=l0.vy-l1.vy*d0.v/d1.v;
							l1.vz=l0.vz-l1.vz*d0.v/d1.v;
						}

						if(d1.u!=0.0f){
							float ilen=l1.vx*l1.vx+l1.vy*l1.vy+l1.vz*l1.vz;
							ilen=1.0/sqrt(ilen);
							l1.vx*=ilen;
							l1.vy*=ilen;
							l1.vz*=ilen;
						}
						else{
							l1.vx=1.0f;
							l1.vy=0.0f;
							l1.vz=0.0f;
						}

						if(calc_flag & 1){
							*((short *)((packet+i)->uvs[2])+j*4+0)
								=(short)(l1.vx*4096.0f);
							*((short *)((packet+i)->uvs[2])+j*4+1)
								=(short)(l1.vy*4096.0f);
							*((short *)((packet+i)->uvs[2])+j*4+2)
								=(short)(l1.vz*4096.0f);
							*((short *)((packet+i)->uvs[2])+j*4+3)
								=0;
						}
						if(calc_flag & 2){
							*((short *)((packet+i)->uvs[2])+(j-1)*4+0)
								=(short)(l1.vx*4096.0f);
							*((short *)((packet+i)->uvs[2])+(j-1)*4+1)
								=(short)(l1.vy*4096.0f);
							*((short *)((packet+i)->uvs[2])+(j-1)*4+2)
								=(short)(l1.vz*4096.0f);
							*((short *)((packet+i)->uvs[2])+(j-1)*4+3)
								=0;
						}
						if(calc_flag & 4){
							*((short *)((packet+i)->uvs[2])+(j-2)*4+0)
								=(short)(l1.vx*4096.0f);
							*((short *)((packet+i)->uvs[2])+(j-2)*4+1)
								=(short)(l1.vy*4096.0f);
							*((short *)((packet+i)->uvs[2])+(j-2)*4+2)
								=(short)(l1.vz*4096.0f);
							*((short *)((packet+i)->uvs[2])+(j-2)*4+3)
								=0;
						}
						calc_flag=0;
					}
#else
					FVECTOR l0={ 0.0f,-1.0f,0.0f,0.0f };
					FVECTOR l1;
					int nid=s->p[j].nid;
					float ilen;

					p0=&((mesh->norms+nid)->n);

					l1.vx=l0.vy*p0->vz-l0.vz*p0->vy;
					l1.vy=l0.vz*p0->vx-l0.vx*p0->vz;
					l1.vz=l0.vx*p0->vy-l0.vy*p0->vx;

					ilen=l1.vx*l1.vx+l1.vy*l1.vy+l1.vz*l1.vz;
					if(ilen>0.0f){
						ilen=1.0/sqrt(ilen);
						l1.vx*=ilen;
						l1.vy*=ilen;
						l1.vz*=ilen;
					}
					else{
						l1.vx=1.0f;
						l1.vy=0.0f;
						l1.vz=0.0f;
					}

					*((short *)((packet+i)->uvs[2])+j*4+0)
						=(short)(l1.vx*4096.0f);
					*((short *)((packet+i)->uvs[2])+j*4+1)
						=(short)(l1.vy*4096.0f);
					*((short *)((packet+i)->uvs[2])+j*4+2)
						=(short)(l1.vz*4096.0f);
					*((short *)((packet+i)->uvs[2])+j*4+3)
						=0;
#endif
				}
			}
		}
		else if(flag & DG_PACKFLAG_UV2){
			if(((packet+i)->uvs[2]=Evm_Uvs_Alloc(n_verts,NULL))==NULL){
				Evm_Pack_Free(packet,n_strips);
				return NULL;
			}

			for(j=0;j<n_verts;j++){
				/* ＵＶの展開 */
				*((short *)((packet+i)->uvs[2])+j*4+0)
					=(short)((s->p[j].uvs+uv_index[2])->u*4096.0f);
				*((short *)((packet+i)->uvs[2])+j*4+1)
					=(short)((s->p[j].uvs+uv_index[2])->v*4096.0f);
				*((short *)((packet+i)->uvs[2])+j*4+2)
					=(short)((s->p[j].uvs+uv_index[2])->w*4096.0f);
				*((short *)((packet+i)->uvs[2])+j*4+3)
					=0;
			}
		}


		/* 使用オブジェＩＤの格納、ついでにソート */
#if 0  /* TYPE_VER1 */
		(packet+i)->n_mats=s->n_objids;
#endif

#if 1  /* TYPE_VER2 */
		(packet+i)->n_mats=s->max_objids;
#endif


		/* 使用オブジェＩＤの初期化 */
		for(j=0;j<8;j++) (packet+i)->mat_id[j]=0xff;

		for(j=0;j<s->n_objids;j++){
			int k,l,objid;

			objid=s->objids[j];
			for(k=0;k<j;k++){
				if(objid<(packet+i)->mat_id[k]) break;
			}
			for(l=j;l>k;l--) (packet+i)->mat_id[l]=(packet+i)->mat_id[l-1];
			(packet+i)->mat_id[k]=objid;
		}


		if(((packet+i)->weight=Evm_Weight_Alloc(n_verts,NULL))==NULL){
			Evm_Pack_Free(packet,n_strips);
			return NULL;
		}
		for(j=0;j<n_verts;j++){
			int sum,max,max_index;
			int vid=(*(strips+i))->p[j].vid;
			int k;

			/* 重み値の展開 */
			for(k=0;k<8;k++){
				*((unsigned char *)((packet+i)->weight)+j*8+k)=0;
			}

			max=0;
			sum=0;
			max_index=-1;

#if 0  /* TYPE_VER1 */
			for(k=0;k<(mesh->verts+vid)->n_envs;k++){
				int objid=((mesh->verts+vid)->envs+k)->objid;
				int l;

				for(l=0;l<(packet+i)->n_mats;l++){
					if(objid==(packet+i)->mat_id[l]){
						*((unsigned char *)((packet+i)->weight)+j*8+l)
							=(unsigned char)(((mesh->verts+vid)->envs+k)->env*128.0f);

						sum+=*((unsigned char *)((packet+i)->weight)+j*8+l);
						if(max<*((unsigned char *)((packet+i)->weight)+j*8+l)){
							max=*((unsigned char *)((packet+i)->weight)+j*8+l);
							max_index=l;
						}
						break;
					}
				}
			}
#endif

#if 1  /* TYPE_VER2 */
			{
				int idx=0;

				for(k=0;k<s->n_objids;k++){
					int objid=(packet+i)->mat_id[k];
					int l;

					for(l=0;l<(mesh->verts+vid)->n_envs;l++){
						if(objid==((mesh->verts+vid)->envs+l)->objid){
							*((unsigned char *)((packet+i)->weight)+j*8+idx)
								=(unsigned char)(((mesh->verts+vid)->envs+l)->env*128.0f);
							*((unsigned char *)((packet+i)->weight)+j*8+idx+4)=k*4;

							sum+=*((unsigned char *)((packet+i)->weight)+j*8+idx);
							if(max<*((unsigned char *)((packet+i)->weight)+j*8+idx)){
								max=*((unsigned char *)((packet+i)->weight)+j*8+idx);
								max_index=idx;
							}

							idx++;

							break;
						}
					}
				}
			}
#endif

			if(max_index!=-1){
				/* 合計がちょうど128になるように補正 */
				sum-=128;
				*((unsigned char *)((packet+i)->weight)+j*8+max_index)=max-sum;
			}
		}
    }

    return packet;
}


/*-----------------------------------------------------------------*/

/* ストリップ内に含まれるウェイト値が、限界値を越えていないかどうか
   チェックし、もし越えていれば、ストリップを分割する。*/
static STRIP **DivideStripByWeight(STRIP **strips,int start,int *n_strips,EVF_VERTS *verts)
{
    int i,j;

    for(i=start;i<*n_strips;i++){
		STRIP *s=*(strips+i);
		PRIM_POINT *p=s->p;
		int n_objs=0;

		/* 進行チェック用 */
		printf(".");
		fflush(stdout);

		s->n_objids=0;

		for(j=0;j<s->n_points;j++){
			int vid=(p+j)->vid;
			int k;

			for(k=0;k<(verts+vid)->n_envs;k++){
				int objid=((verts+vid)->envs+k)->objid;
				int l;

				for(l=0;l<n_objs;l++){
					if(objid==s->objids[l]) break;
				}
				if(l==n_objs){
					if(n_objs>=MAX_OBJIDS){
						STRIP **new_strips;
						STRIP *x,*y;
						int xsize,ysize;


						/* 進行チェック用 */
						printf("!");
						fflush(stdout);


						/* 有効ウェイト数が最大値を越えたので、ストリップをここで分割 */

						if(j<3){
							/* 有効ウェイト数の最大値を越えている三角形が存在する場合 */
							printf("Error : Over Weights Max\n");
#if 1
							Strips_Free(strips,*n_strips);
							return NULL;
#else
							/* とにかく表示できるように変換だけは先に進める。*/
							goto error;
#endif
						}

						j--;
						while((p+j)->draw_flag & 0x8000) j--;

						xsize=j+1;

						j++;
						while((p+j)->draw_flag & 0x8000) j++;

						ysize=s->n_points-j+2;

						if((new_strips=Strips_Alloc(*n_strips+1))==NULL){
							Strips_Free(strips,*n_strips);
							return NULL;
						}
						if((x=Strip_Alloc(0,xsize))==NULL){
							Strips_Free(strips,*n_strips);
							MDU_Free(new_strips);
							return NULL;
						}
						if((y=Strip_Alloc(s->n_tid,ysize))==NULL){
							MDU_Free(x);
							Strips_Free(strips,*n_strips);
							MDU_Free(new_strips);
							return NULL;
						}

						memcpy(new_strips,strips,sizeof(STRIP *)*(i+1));
						if((*n_strips)-(i+1)>0){
							memcpy(new_strips+i+2,strips+i+1,sizeof(STRIP *)*((*n_strips)-(i+1)));
						}

						memcpy(x,s,sizeof(STRIP)+sizeof(PRIM_POINT)*xsize);
						{
							u_int *tid=y->tid;
							memcpy(y,s,sizeof(STRIP));
							y->tid=tid;
						}
						memcpy(y->p,s->p+s->n_points-ysize,sizeof(PRIM_POINT)*ysize);

						if(y->n_tid>0){
							memcpy(y->tid,s->tid,sizeof(u_int)*y->n_tid);

							if(s->n_points+1<xsize+ysize){
								y->p[0].uvs=NULL;
								y->p[1].uvs=NULL;
								if(!Strip_Uvs_Alloc(y,0)){
									MDU_Free(y->tid);
									MDU_Free(y);
									MDU_Free(x);
									Strips_Free(strips,*n_strips);
									MDU_Free(new_strips);
									return NULL;
								}
								if(!Strip_Uvs_Alloc(y,1)){
									MDU_Free(y->p[0].uvs);
									MDU_Free(y->tid);
									MDU_Free(y);
									MDU_Free(x);
									Strips_Free(strips,*n_strips);
									MDU_Free(new_strips);
									return NULL;
								}
								memcpy(y->p[0].uvs,s->p[xsize-2].uvs,
									   sizeof(TVECTOR)*y->n_tid);
								memcpy(y->p[1].uvs,s->p[xsize-1].uvs,
									   sizeof(TVECTOR)*y->n_tid);
							}
							else if(s->n_points<xsize+ysize){
								y->p[0].uvs=NULL;
								if(!Strip_Uvs_Alloc(y,0)){
									MDU_Free(y->tid);
									MDU_Free(y);
									MDU_Free(x);
									Strips_Free(strips,*n_strips);
									MDU_Free(new_strips);
									return NULL;
								}
								memcpy(y->p[0].uvs,s->p[xsize-1].uvs,
									   sizeof(TVECTOR)*y->n_tid);
							}
						}
						s->n_tid=0;
						s->tid=NULL;
						{
							int n;
							for(n=0;n<s->n_points;n++) s->p[n].uvs=NULL;
						}

						/* 描画しない指定を行なう */
						y->p[0].draw_flag=0x8000;
						y->p[1].draw_flag=0x8000;

						x->n_points=xsize;
						y->n_points=ysize;

						Strip_Free(s);

						*(new_strips+i)=x;
						*(new_strips+i+1)=y;
						(*n_strips)++;

						MDU_Free(strips);
						strips=new_strips;

						goto forbreak;
					}
					else{
						s->objids[n_objs]=objid;
						n_objs++;
					}
				}
			}
error:
			if(!((p+j)->draw_flag & 0x8000)) s->n_objids=n_objs;
		}

forbreak:
		;
    }

    printf("\n");

    return strips;
}


static void CountMaxWeights(STRIP **strips,int n_strips,EVF_VERTS *verts)
{
	int i,j;
    for(i=0;i<n_strips;i++){
		STRIP *s=*(strips+i);
		PRIM_POINT *p=s->p;

		s->max_objids=0;
		for(j=0;j<s->n_points;j++){
			int vid=(p+j)->vid;
			if(s->max_objids<(verts+vid)->n_envs) s->max_objids=(verts+vid)->n_envs;
		}
	}
}


/*-----------------------------------------------------------------*/

static int Make_Strip(EVF_DEF *evf,EVM_DEF *def,int fast_flag,int n_max_points,int fix_flag)
{
    DIV_PRIMS *div_prims;
    STRIP **strips;
    EVF_VERTS *verts=evf->mesh.verts;
    int n_strips;
    int i;
    int midpoint;

    if((div_prims=Evf2DivPrims(&(evf->mesh),evf->n_x_models))==NULL) return 0;

    strips=NULL;
    n_strips=0;

#if 0
    if((strips=MDU_Alloc(sizeof(STRIP *)*32768))==NULL){
		Div_Prims_Free(div_prims);
		return 0;
    }
#endif

    /* 進行チェック用 */
#if 1
    printf("Make Strip Single Weight & No Trans : ");
#else
    printf("Make Strip Single Weight : ");
#endif
    fflush(stdout);

    /* シングルウェイト、不透明テクスチャの処理 */
    for(i=div_prims->start_single;i<div_prims->n_div_prims;i++){

#if 1
		if(div_prims->div_prims[i].n_tid>0){
			if(IsOverlayTexture(evf->texs[div_prims->div_prims[i].tid[0]].name)) continue;
		}
#endif

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

    /* 進行チェック用 */
#if 1
    printf("\nMake Strip Multi Weight & No Trans : ");
#else
    printf("\nMake Strip Multi Weight : ");
#endif
    fflush(stdout);

    /* マルチウェイト、不透明テクスチャの処理 */
    for(i=0;i<div_prims->start_single;i++){

#if 1
		if(div_prims->div_prims[i].n_tid>0){
			if(IsOverlayTexture(evf->texs[div_prims->div_prims[i].tid[0]].name)) continue;
		}
#endif

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


    if(n_strips!=0){
		if(strips==NULL) return 0;
    }


    /* 進行チェック用 */
    printf("\nDivide Strip By Weight : ");

    /* 有効ウェイト値の判定 */
    strips=DivideStripByWeight(strips,0,&n_strips,verts);
    midpoint=n_strips;


#if 1
    /* 進行チェック用 */
    printf("Make Strip Single Weight & Trans : ");
    fflush(stdout);

    /* シングルウェイト、半透明テクスチャの処理 */
    for(i=div_prims->start_single;i<div_prims->n_div_prims;i++){

		if(div_prims->div_prims[i].n_tid>0){
			if(!IsOverlayTexture(evf->texs[div_prims->div_prims[i].tid[0]].name)) continue;
		}
		else continue;

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

    /* 進行チェック用 */
    printf("\nMake Strip Multi Weight & Trans : ");
    fflush(stdout);

    /* マルチウェイト、半透明テクスチャの処理 */
    for(i=0;i<div_prims->start_single;i++){

		if(div_prims->div_prims[i].n_tid>0){
			if(!IsOverlayTexture(evf->texs[div_prims->div_prims[i].tid[0]].name)) continue;
		}
		else continue;

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

    if(n_strips!=0){
		if(strips==NULL) return 0;

		/* 2002/8/26 Ken Kano 追加
		   半透明表示の際に、よく優先順位の逆転が起こっていたので、
		   それを防ぐためにストリップの順序を、テクスチャのα値の平均の濃い順にして、
		   半透明は必ず後で描画されるようにする。*/
		SortStrip(evf->texs,strips,n_strips);
    }


    /* 進行チェック用 */
    printf("\nDivide Strip By Weight : ");

    /* 有効ウェイト値の判定 */
    strips=DivideStripByWeight(strips,midpoint,&n_strips,verts);
#endif


	/* 頂点内の最大ウェイト個数をストリップごとに記録 */
	CountMaxWeights(strips,n_strips,verts);


    Div_Prims_Free(div_prims);

    /* モデルフォーマットに変換 */
    {
		int ans=1;

		/* EVM */
		if((def->packet=ConvertToEvm(evf,strips,n_strips,fix_flag))==NULL) ans=0;
		else def->n_packs=n_strips;

		Strips_Free(strips,n_strips);

		return ans;
    }
}

static int CreateIndex( EVM_DEF* pdef, int etc_flag )
{
	int i, j;
	EVM_PACK* ppack = pdef->packet;

	unsigned int count = 0;

	if( etc_flag & MDU_FLAG_INDEX ) {
		// インデックスをつける
		pdef->type |= EVM_TYPE_INDEX;
		
		for( i = pdef->n_packs ; i > 0 ; --i , ppack++ ) {
			
			// 必要メモリを数える
			int size = 2;	// 最初２点は書き込む
			short* pv = (short*)ppack->verts + 4 * 2;
			unsigned short* pIndex;

			if( !( ppack->flag & (DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ) ) {
				// カリングなし
				for( j = 2; j < ppack->n_verts ; j++ ) { 
					if( pv[3] & 0x8000 ) size++;	// NO_KICK エミュレートのため頂点が増える
					size++;
					pv += 4;
				}

				if( (pIndex = ppack->index = MDU_Alloc( size * sizeof( unsigned short ) ) ) == NULL ) {
					return 0;
				}
				
				// 実際に書き込む
				ppack->n_indices = size;

				pv = (short*)ppack->verts + 4 * 2;

				*pIndex++ = (unsigned short)count++;
				*pIndex++ = (unsigned short)count++;	// 最初二点
				
				for( j = 2; j < ppack->n_verts ; j++ ) { 
					if( pv[3] & 0x8000 ) *pIndex++ = (unsigned short)count - 1;
					*pIndex++ = (unsigned short)count++;
					pv += 4;
				}
				if( count > 0xffff ) {	// index が short -> 1 obj に 65536 頂点まで
					printf("Too many vertexes in 1 obj\n");
					return 0;
				}
			} else {
				// カリングあり
				int		cull_flag = 0x00 ;
				for( j = 2; j < ppack->n_verts ; j++ ) { 
					if( pv[3] & 0x8000 ) {
						cull_flag = 0x20 - cull_flag; 
						size++;	// NO_KICK エミュレートのため頂点が増える
					} else if ( ( pv[3] & 0xff ) == cull_flag ) {
						cull_flag = 0x20 - cull_flag; 
						size++;	// 回転方向を合わせるために頂点が増える
					}
					cull_flag = 0x20 - cull_flag; 
					size++;
					pv += 4;
				}

				if( (pIndex = ppack->index = MDU_Alloc( size * sizeof( unsigned short ) ) ) == NULL ) {
					return 0;
				}
				
				// 実際に書き込む
				ppack->n_indices = size;
				cull_flag = 0;

				pv = (short*)ppack->verts + 4 * 2;

				*pIndex++ = (unsigned short)count++;
				*pIndex++ = (unsigned short)count++;	// 最初二点
				
				for( j = 2; j < ppack->n_verts ; j++ ) { 
					if( pv[3] & 0x8000 ) {
						*pIndex++ = (unsigned short)count - 1;
						cull_flag = 0x20 - cull_flag;
					} else if ( ( pv[3] & 0xff ) == cull_flag ) {
						*(pIndex - 1) = count - 2;
						*pIndex++ = (unsigned short)count - 1;
						cull_flag = 0x20 - cull_flag;
					}
					cull_flag = 0x20 - cull_flag; 
					*pIndex++ = count++;
					pv += 4;
				}
				if( count > 0xffff ) {	// index が short -> 1 obj に 65536 頂点まで
					printf("Too many vertexes in 1 obj\n");
					return 0;
				}
			}
		}
	} else {
		// インデックス関係のメンバを一応リセット
		for( i = pdef->n_packs ; i > 0 ; --i , ppack++ ) {
			ppack->n_indices = 0;
			ppack->index = NULL;
		}
	}
	return 1;
}

/*-----------------------------------------------------------------*/

EVM_DEF *MDU_Evf2Evm(EVF_DEF *evfdef,int fast_flag,int fix_flag, int etc_flag )
{
    EVF_MESH *mesh=&(evfdef->mesh);
    EVF_SKEL *mdl0;
    EVM_DEF *def;
    EVM_SKEL *mdl1;
    int i;
	int	n_max_points;

	/* 重複する頂点や法線をまとめる */
	Evf_RemoveOverlap(evfdef);

    printf("Evf -> Evm Conversion Start.\n");

    def=Evm_Alloc(evfdef->n_models,evfdef->n_x_models,NULL);

    /* 進行チェック用 */
    printf("Mesh = %d Prims\n",mesh->n_prims);

    /* 進行チェック用 */
    if(fast_flag) printf("Easy Strip start\n");
    else printf("Strip Algo start\n");

	/* 最大頂点数決定 */
	if( etc_flag & MDU_FLAG_NO_LIMIT ) {
		n_max_points = NOLIMIT_N_MAX_POINTS;
	} else {
		n_max_points = N_MAX_POINTS;
	}

    /* ストリップの作成 */
    if(!Make_Strip(evfdef,def,fast_flag,n_max_points,fix_flag)){
		Evm_Free(def);
		return NULL;
    }

    /* 進行チェック用 */
    printf(" O.K.\n");

    /* EVMの設定 */
#if 0  /* TYPE_VER1 */
    def->type=evfdef->type;
#endif

#if 1  /* TYPE_VER2 */
    def->type=(evfdef->type & ~EVM_TYPE_VERMASK) | EVM_TYPE_VER2;
#endif
    def->lx=evfdef->lx;
    def->ly=evfdef->ly;
    def->lz=evfdef->lz;
    def->ux=evfdef->ux;
    def->uy=evfdef->uy;
    def->uz=evfdef->uz;

    mdl0=evfdef->skeletons;
    mdl1=def->skeleton;
    for(i=0;i<evfdef->n_x_models;i++,mdl0++,mdl1++){
		mdl1->flag=mdl0->flag;
		mdl1->parent=mdl0->parent;

		mdl1->tx=mdl0->tx;
		mdl1->ty=mdl0->ty;
		mdl1->tz=mdl0->tz;
		mdl1->rt_tx=mdl0->rt_tx;
		mdl1->rt_ty=mdl0->rt_ty;
		mdl1->rt_tz=mdl0->rt_tz;
    }

	/* インデックスデータ生成 */
	if( !CreateIndex( def, etc_flag ) ) {
		Evm_Free(def);
		return NULL;
	}

    return def;
}

/*-----------------------------------------------------------------*/
