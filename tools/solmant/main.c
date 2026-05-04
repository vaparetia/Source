#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <SFDLINUX.h>
#include <METALGEAR.h>

#include "MDU_util.h"
#include "MDU_mdl.h"      
#include "MDU_tex.h"      

#include "Kmx.h"
#include "Evf.h"
#include "block.h"


typedef struct {
	unsigned int ptr;
	unsigned char weight[3][8];
} ANIM_POLYPTR;

typedef struct {
	unsigned int n_verts;
	ANIM_POLYPTR ptr[0];
} ANIM_DELETEPOLY;

typedef struct {
	int flags;
	int n_frames;
	ANIM_DELETEPOLY *frame_data[0];
} ANIM_DELETEPOLY_HEADER;


typedef struct {
	float y;
	int ptr;
	unsigned char weight[3][8];
} VSORT;


static void EvmCalcAddr(EVM_DEF *evm)
{
	int i;

	evm->packet=(EVM_PACK *)((int)evm+(int)(evm->packet));

	for(i=0;i<evm->n_packs;i++){
		EVM_PACK *pack=evm->packet+i;

		if(pack->verts!=NULL) pack->verts=(void *)((int)evm+(int)(pack->verts));
		if(pack->norms!=NULL) pack->norms=(void *)((int)evm+(int)(pack->norms));
		if(pack->uvs[0]!=NULL) pack->uvs[0]=(void *)((int)evm+(int)(pack->uvs[0]));
		if(pack->uvs[1]!=NULL) pack->uvs[1]=(void *)((int)evm+(int)(pack->uvs[1]));
		if(pack->uvs[2]!=NULL) pack->uvs[2]=(void *)((int)evm+(int)(pack->uvs[2]));
		if(pack->weight!=NULL) pack->weight=(void *)((int)evm+(int)(pack->weight));
		if(pack->rgbs!=NULL) pack->rgbs=(void *)((int)evm+(int)(pack->rgbs));
	}
}

static EVM_DEF *EvmLoad(char *filename)
{
	FILE *fp;
	void *buf;
	int size;

	if((fp=fopen(filename,"rb"))==NULL) return NULL;

	fseek(fp,0,SEEK_END);
	size=ftell(fp);
	fseek(fp,0,SEEK_SET);

	if((buf=(void *)malloc(size))==NULL){
		fclose(fp);
		return NULL;
	}

	if(fread(buf,1,size,fp)!=size){
		free(buf);
		fclose(fp);
		return NULL;
	}
	
	fclose(fp);

	EvmCalcAddr((EVM_DEF *)buf);

	return buf;
}

static int BinSave(char *filename,void *buf,int size)
{
	FILE *fp;

	if((fp=fopen(filename,"wb"))==NULL) return 0;

	if(fwrite(buf,1,size,fp)!=size){
		fclose(fp);
		return 0;
	}

	fclose(fp);

	return 1;
}

static int compare_vsort(const VSORT *a,const VSORT *b)
{
	if(a->y==b->y) return 0;
	else if(a->y>b->y) return 1;
	else return -1;
}

static void *MakeAnim(EVM_DEF *evm,int frames,int *size)
{
	EVM_PACK *pack;
	short *verts;
	int i,j,k;
	int n_polys;
	VSORT *vsort;
	unsigned char *weight;

	void *file;
	ANIM_DELETEPOLY_HEADER *header;
	ANIM_DELETEPOLY *framedata;


	pack=evm->packet;
	n_polys=0;

	for(i=0;i<evm->n_packs;i++,pack++){
		verts=pack->verts;

		for(j=0;j<pack->n_verts;j++,verts+=4){
			if(!(*(verts+3) & 0x8000)){
				n_polys++;
			}
		}
	}

	if((vsort=(VSORT *)malloc(sizeof(VSORT)*n_polys))==NULL) return NULL;

	pack=evm->packet;
	k=0;

	for(i=0;i<evm->n_packs;i++,pack++){
		verts=pack->verts;
		weight=pack->weight;

		for(j=0;j<pack->n_verts;j++,verts+=4,weight+=8){

			// printf("0x%04x\n",*(verts+3));

			if(!(*(verts+3) & 0x8000)){
				float y1,y2,y3;
				int ii;

				y1=(float)*(verts+1-4*2);
				y2=(float)*(verts+1-4*1);
				y3=(float)*(verts+1-4*0);

				if(!(evm->type & 1)){
					/* -2048.0 ... 2047.9375 */
					y1/=16.0f;
					y2/=16.0f;
					y3/=16.0f;
				}

#if 0
				vsort[k].y=(y1+y2+y3)/3;
#else
				if(y1>y2){
					if(y1>y3){
						vsort[k].y=y1;
					}
					else{
						vsort[k].y=y3;
					}
				}
				else{
					if(y2>y3){
						vsort[k].y=y2;
					}
					else{
						vsort[k].y=y3;
					}
				}
#endif
				vsort[k].ptr=(int)verts-(int)(evm->packet->verts);
				for(ii=0;ii<4;ii++){
					vsort[k].weight[0][ii]=weight[ii-8*2];
					vsort[k].weight[0][ii+4]=pack->mat_id[weight[ii+4-8*2]>>2];
					vsort[k].weight[1][ii]=weight[ii-8*1];
					vsort[k].weight[1][ii+4]=pack->mat_id[weight[ii+4-8*1]>>2];
					vsort[k].weight[2][ii]=weight[ii-8*0];
					vsort[k].weight[2][ii+4]=pack->mat_id[weight[ii+4-8*0]>>2];
				}

				// printf("%d\n",vsort[k].ptr);

				k++;
			}
		}
	}

	// printf("-----\n");

	qsort(vsort,n_polys,sizeof(VSORT),compare_vsort);

	*size=sizeof(ANIM_DELETEPOLY_HEADER)+frames*sizeof(ANIM_DELETEPOLY *)+
		sizeof(ANIM_DELETEPOLY)*frames+sizeof(ANIM_POLYPTR)*n_polys;

	if((file=(void *)malloc(*size))==NULL){
		free(vsort);
		return NULL;
	}

	header=(ANIM_DELETEPOLY_HEADER *)file;
	framedata=(ANIM_DELETEPOLY *)((int)header+sizeof(ANIM_DELETEPOLY_HEADER)+
								  frames*sizeof(ANIM_DELETEPOLY *));

	header->flags=0;
	header->n_frames=frames;

	{
		int pre_index,index;
		float pre_cmpy,cmpy;
		float ymin,ymax;

		pre_index=0;
		index=0;
		ymin=vsort[0].y;
		ymax=vsort[n_polys-1].y;
		ymax+=(ymax-ymin)*0.01f;
		pre_cmpy=0;

		for(i=0;i<frames;i++){
			ANIM_POLYPTR *ptr;

			cmpy=ymin+(ymax-ymin)*(float)(i+1)/(float)frames;

			header->frame_data[i]=(ANIM_DELETEPOLY *)((int)framedata-(int)file);

			ptr=framedata->ptr;

			while(vsort[index].y<cmpy){
				if(index>=n_polys) break;

				ptr->ptr=vsort[index].ptr;
				memcpy(ptr->weight,vsort[index].weight,sizeof(ptr->weight));

				index++;
				ptr++;
			}

			framedata->n_verts=index-pre_index;

			framedata=(ANIM_DELETEPOLY *)ptr;

			pre_index=index;
		}
	}

	free(vsort);

	return file;
}

void main(int argc,char *argv[])
{
	EVM_DEF *evm;
	int frames;
	int size;
	void *file;

	if(argc!=4){
		printf("usage : solmant <evm-file> <anim-file> <n_frames>\n");
		exit(0);
	}

	frames=atoi(argv[3]);

	if((evm=EvmLoad(argv[1]))==NULL){
		printf("Error : %s : file not found\n",argv[1]);

		exit(-1);
	}

	if((file=MakeAnim(evm,frames,&size))==NULL){
		printf("Error : file-image not made\n");

		free(evm);

		exit(-1);
	}

	if(!BinSave(argv[2],file,size)){
		printf("Error : %s : file not saved\n",argv[2]);

		free(evm);
		free(file);

		exit(-1);
	}

	free(evm);
	free(file);

	exit(0);
}

