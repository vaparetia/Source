/*
   Overlap.c

   頂点や法線の重複を除く

   by K.Kano , 1/19/2001

   $Id: Overlap.c,v 1.4 2001/07/18 06:04:21 usr01363 Exp $
   
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


/*-----------------------------------------------------------------*/

static int Bincmp(void *a,void *b,int size)
{
	unsigned char *aa=(unsigned char *)a;
	unsigned char *bb=(unsigned char *)b;

	while(size>0){
		if(*aa!=*bb) return 0;
		aa++,bb++,size--;
	}
	return 1;
}

static int Cmp_KmxVertex(KMX_VERTS *v1,KMX_VERTS *v2)
{
	if(v1->v.vx!=v2->v.vx) return 0;
	if(v1->v.vy!=v2->v.vy) return 0;
	if(v1->v.vz!=v2->v.vz) return 0;

	if(v1->env.val[0]!=v2->env.val[0]) return 0;
	if(v1->env.val[1]!=v2->env.val[1]) return 0;
	if(v1->env.objid!=v2->env.objid) return 0;
	if(v1->env.parentid!=v2->env.parentid) return 0;
	if(v1->env.id!=v2->env.id) return 0;

	if(!Bincmp(&(v1->vert_usrdata),&(v2->vert_usrdata),sizeof(VERTEX_USERDATA))) return 0;

	return 1;
}

static int Cmp_KmxNormal(KMX_NORMS *n1,KMX_NORMS *n2)
{
	if(n1->n.vx!=n2->n.vx) return 0;
	if(n1->n.vy!=n2->n.vy) return 0;
	if(n1->n.vz!=n2->n.vz) return 0;

	if(!Bincmp(&(n1->norm_usrdata),&(n2->norm_usrdata),sizeof(NORMAL_USERDATA))) return 0;

	return 1;
}

static int Cmp_EvfVertex(EVF_VERTS *v1,EVF_VERTS *v2)
{
	int i;

	if(v1->v.vx!=v2->v.vx) return 0;
	if(v1->v.vy!=v2->v.vy) return 0;
	if(v1->v.vz!=v2->v.vz) return 0;

	if(v1->n_envs!=v2->n_envs) return 0;

	for(i=0;i<v1->n_envs;i++){
		if(v1->envs[i].objid==v2->envs[i].objid) return 0;
		if(v1->envs[i].env==v2->envs[i].env) return 0;
	}

	if(!Bincmp(&(v1->vert_usrdata),&(v2->vert_usrdata),sizeof(VERTEX_USERDATA))) return 0;

	return 1;
}

static int Cmp_EvfNormal(EVF_NORMS *n1,EVF_NORMS *n2)
{
	if(n1->n.vx!=n2->n.vx) return 0;
	if(n1->n.vy!=n2->n.vy) return 0;
	if(n1->n.vz!=n2->n.vz) return 0;

	if(!Bincmp(&(n1->norm_usrdata),&(n2->norm_usrdata),sizeof(NORMAL_USERDATA))) return 0;

	return 1;
}

/*-----------------------------------------------------------------*/

static void KmxMdl_RemoveOverlap(KMX_MDL *mdl,int overlap_flag)
{
	int n_verts;
	int *vert_indexes;
	int n_norms;
	int *norm_indexes;

	KMX_VERTS *verts;
	KMX_NORMS *norms;

	int i,j;


	if((vert_indexes=(int *)MDU_Alloc(sizeof(int)*mdl->n_verts))==NULL) return;
	if((norm_indexes=(int *)MDU_Alloc(sizeof(int)*mdl->n_norms))==NULL){
		MDU_Free(vert_indexes);
		return;
	}

	memset(vert_indexes,0x00,sizeof(int));
	memset(norm_indexes,0x00,sizeof(int));

	n_verts=0;
	n_norms=0;

	for(i=0;i<mdl->n_verts;i++){
		for(j=0;j<i;j++){
			if(Cmp_KmxVertex(mdl->verts+i,mdl->verts+j)) break;
		}

		if(i==j){
			*(vert_indexes+i)=n_verts;
			n_verts++;
		}		
		else{
			*(vert_indexes+i)=*(vert_indexes+j);
		}
	}

	for(i=0;i<mdl->n_norms;i++){
		for(j=0;j<i;j++){
			if(Cmp_KmxNormal(mdl->norms+i,mdl->norms+j)) break;
		}

		if(i==j){
			*(norm_indexes+i)=n_norms;
			n_norms++;
		}		
		else{
			*(norm_indexes+i)=*(norm_indexes+j);
		}
	}


#if 1
	if(overlap_flag){
		if(n_verts!=mdl->n_verts){
			if((verts=Kmx_Verts_Alloc(n_verts))!=NULL){
				printf("    n_verts : %d -> %d\n",mdl->n_verts,n_verts);

				for(i=0;i<mdl->n_verts;i++){
					memcpy(verts+*(vert_indexes+i),mdl->verts+i,sizeof(KMX_VERTS));
				}

				for(i=0;i<mdl->n_prims;i++){
					KMX_PRIMS *prim=mdl->prims+i;

					for(j=0;j<prim->n_id;j++){
						int id=*(prim->vid+j);
						*(prim->vid+j)=*(vert_indexes+id);
					}
				}

				Kmx_Verts_Free(mdl->verts);

				mdl->n_verts=n_verts;
				mdl->verts=verts;
			}
		}
	}
#endif


	if(n_norms!=mdl->n_norms){
		if((norms=Kmx_Norms_Alloc(n_norms))!=NULL){
			printf("    n_norms : %d -> %d\n",mdl->n_norms,n_norms);

			for(i=0;i<mdl->n_norms;i++){
				memcpy(norms+*(norm_indexes+i),mdl->norms+i,sizeof(KMX_NORMS));
			}

			for(i=0;i<mdl->n_prims;i++){
				KMX_PRIMS *prim=mdl->prims+i;

				for(j=0;j<prim->n_id;j++){
					int id=*(prim->nid+j);
					*(prim->nid+j)=*(norm_indexes+id);
				}
			}

			Kmx_Norms_Free(mdl->norms);

			mdl->n_norms=n_norms;
			mdl->norms=norms;
		}
	}

	MDU_Free(vert_indexes);
	MDU_Free(norm_indexes);
}

void Kmx_RemoveOverlap(KMX_DEF *kmx,int overlap_flag)
{
	int i;

	/* 頂点は頂点アニメの問題があり増減できないのと、
	   問題が起きたのが法線の方なので、
	   法線だけ削る事にする。*/

#if 1
	printf("Remove Overlap Vertexes & Normals\n");
#else
	printf("Remove Overlap Normals\n");
#endif

	for(i=0;i<kmx->n_x_models;i++){
		printf("  Obj %d : Check Overlap\n",i);

		KmxMdl_RemoveOverlap(kmx->models+i,overlap_flag);
	}
}

/*-----------------------------------------------------------------*/

void Evf_RemoveOverlap(EVF_DEF *evf)
{
	int n_verts;
	int *vert_indexes;
	int n_norms;
	int *norm_indexes;

	EVF_VERTS *verts;
	EVF_NORMS *norms;
	EVF_MESH *mesh;

	int i,j;


	printf("Remove Overlap Vertexes & Normals\n");

	mesh=&(evf->mesh);

	if((vert_indexes=(int *)MDU_Alloc(sizeof(int)*mesh->n_verts))==NULL) return;
	if((norm_indexes=(int *)MDU_Alloc(sizeof(int)*mesh->n_norms))==NULL){
		MDU_Free(vert_indexes);
		return;
	}

	memset(vert_indexes,0x00,sizeof(int));
	memset(norm_indexes,0x00,sizeof(int));

	n_verts=0;
	n_norms=0;

	for(i=0;i<mesh->n_verts;i++){
		for(j=0;j<i;j++){
			if(Cmp_EvfVertex(mesh->verts+i,mesh->verts+j)) break;
		}

		if(i==j){
			*(vert_indexes+i)=n_verts;
			n_verts++;
		}		
		else{
			*(vert_indexes+i)=*(vert_indexes+j);
		}
	}

	for(i=0;i<mesh->n_norms;i++){
		for(j=0;j<i;j++){
			if(Cmp_EvfNormal(mesh->norms+i,mesh->norms+j)) break;
		}

		if(i==j){
			*(norm_indexes+i)=n_norms;
			n_norms++;
		}		
		else{
			*(norm_indexes+i)=*(norm_indexes+j);
		}
	}

	if(n_verts!=mesh->n_verts){
		if((verts=Evf_Verts_Alloc(n_verts))!=NULL){
			printf("  n_verts : %d -> %d\n",mesh->n_verts,n_verts);

			for(i=0;i<mesh->n_verts;i++){
				memcpy(verts+*(vert_indexes+i),mesh->verts+i,sizeof(EVF_VERTS));
				if(((verts+*(vert_indexes+i))->envs=Evf_Envs_Alloc((mesh->verts+i)->n_envs))==NULL){
					Evf_Verts_Free(verts,n_verts);
					MDU_Free(vert_indexes);
					MDU_Free(norm_indexes);
					return;
				}
				memcpy((verts+*(vert_indexes+i))->envs,(mesh->verts+i)->envs,
					   sizeof(EVF_ENV)*(mesh->verts+i)->n_envs);
				(verts+*(vert_indexes+i))->n_envs=(mesh->verts+i)->n_envs;
			}

			for(i=0;i<mesh->n_prims;i++){
				EVF_PRIMS *prim=mesh->prims+i;

				for(j=0;j<prim->n_id;j++){
					int id=*(prim->vid+j);
					*(prim->vid+j)=*(vert_indexes+id);
				}
			}

			Evf_Verts_Free(mesh->verts,mesh->n_verts);

			mesh->n_verts=n_verts;
			mesh->verts=verts;
		}
	}
	if(n_norms!=mesh->n_norms){
		if((norms=Evf_Norms_Alloc(n_norms))!=NULL){
			printf("  n_norms : %d -> %d\n",mesh->n_norms,n_norms);

			for(i=0;i<mesh->n_norms;i++){
				memcpy(norms+*(norm_indexes+i),mesh->norms+i,sizeof(EVF_NORMS));
			}

			for(i=0;i<mesh->n_prims;i++){
				EVF_PRIMS *prim=mesh->prims+i;

				for(j=0;j<prim->n_id;j++){
					int id=*(prim->nid+j);
					*(prim->nid+j)=*(norm_indexes+id);
				}
			}

			Evf_Norms_Free(mesh->norms);

			mesh->n_norms=n_norms;
			mesh->norms=norms;
		}
	}

	MDU_Free(vert_indexes);
	MDU_Free(norm_indexes);
}
