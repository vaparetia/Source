//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	solmant.c
		��������Υޥ��Ⱦä�

	2001/05/17 K.Kano
	$Id: solmant2.c,v 1.2 2002/12/18 08:53:40 takaki Exp $
*/


#include "solmant.h"

#include "../hair/hair.h"
#include "../rope/rope.h"

#include "../../mode/demo/libdemo.h"


#define UV_FIRESTART	((int)(0.4f*0x1000))
#define UV_LIMIT		((int)(0.42f*0x1000))

#define N_VECSTOCK		256


extern const SAMPLE_HAIR_PARAMETER hair_sample[];

void *NewPointFire( FVECTOR* pos );

void *NewSolidusTestPoint2( void );
void SolidusGetFirePoint( FVECTOR *new_pos, int num );

void *NewSolidusMantFire( void );
void SolidusMantGetFirePoint( FVECTOR *new_pos, int num );

#ifdef PSX2
/*
	�Уӣ��ǥ롼���
*/
/* ĺ����˥��Хåե����ڤ��ؤ� */
void EvmVertexAnimeSwitchBuffer(EVM_VERTEXANIM *anim)
{
	anim->evm->verts_ptr=anim->vertex[anim->buffer_flag];
	anim->evm->uvs0_ptr=anim->uvs0[anim->buffer_flag];
}

/* ĺ����˥��Хåե��ν����� */
void InitEvmVertexAnime(DG_EVMOBJ *evm,EVM_VERTEXANIM *anim)
{
	DG_EVMPACK *pack=evm->packs;
	int n_steps=0;
	int i;

	anim->evm=evm;
	anim->buffer_flag=0;
	anim->vertex_org=(short *)(evm->verts_ptr);
	anim->normal_org=(short *)(evm->norms_ptr);
	anim->uvs0_org=(short *)(evm->uvs0_ptr);
	anim->uvs1_org=(short *)(evm->uvs1_ptr);
	anim->uvs2_org=(short *)(evm->uvs2_ptr);

	/* ĺ����󤬾��񤹤����������׻� */
	for(i=evm->n_packet;i>0;i--,pack++){
		n_steps+=pack->n_step;	/* 1��1qword�ʤΤ�n_step*2ĺ��ʬ�ˤʤ��Τ����� */
	}

	if((anim->vertex[0]=GV_Malloc(n_steps*16*4+
								  N_VECSTOCK*(sizeof(FVECTOR)+
											  sizeof(short *)+
											  sizeof(unsigned char)*8*3)))==NULL){

#ifdef DEBUG_MODE
		printf("EvmVertexAnime : Memory Overflow\n");
#endif
		ASSERT(0);
	}
	anim->vertex[1]=anim->vertex[0]+n_steps*16/sizeof(short);
	anim->uvs0[0]=anim->vertex[1]+n_steps*16/sizeof(short);
	anim->uvs0[1]=anim->uvs0[0]+n_steps*16/sizeof(short);

	anim->stkindex=0;
	anim->stkvec=(FVECTOR *)(anim->uvs0[1]+n_steps*16/sizeof(short));
	anim->mdlvec=(short **)(anim->stkvec+N_VECSTOCK);
	anim->mdlweight=(unsigned char *)(anim->mdlvec+N_VECSTOCK);

	anim->buffer_size=n_steps*16/sizeof(short [4]);

	for(i=0;i<n_steps;i++){
		/* �����ꥳ�ԡ��Ϥ����������®���Τ����� */
		fpu_CopyVector((FVECTOR *)(anim->vertex[0])+i,(FVECTOR *)(anim->vertex_org)+i);
		fpu_CopyVector((FVECTOR *)(anim->vertex[1])+i,(FVECTOR *)(anim->vertex_org)+i);
		fpu_CopyVector((FVECTOR *)(anim->uvs0[0])+i,(FVECTOR *)(anim->uvs0_org)+i);
		fpu_CopyVector((FVECTOR *)(anim->uvs0[1])+i,(FVECTOR *)(anim->uvs0_org)+i);
		anim->uvs0[1][i*8+0]=0x1000-anim->uvs0[1][i*8+0];
		anim->uvs0[1][i*8+4]=0x1000-anim->uvs0[1][i*8+4];
	}
	for(i=0;i<N_VECSTOCK;i++){
		*(anim->mdlvec+i)=NULL;
	}

	EvmVertexAnimeSwitchBuffer(anim);
}
/* ĺ����˥ᳫ�� */
void ExitEvmVertexAnime(EVM_VERTEXANIM *anim)
{
	anim->evm->verts_ptr=anim->vertex_org;
	anim->evm->norms_ptr=anim->normal_org;
	anim->evm->uvs0_ptr=anim->uvs0_org;
	anim->evm->uvs1_ptr=anim->uvs1_org;
	anim->evm->uvs2_ptr=anim->uvs2_org;

	if(anim->vertex[0]!=NULL){
		GV_Free(anim->vertex[0]);
	}
}

int ActAnimDeletePoly(int frame,int frame_max,int uv_speed,EVM_VERTEXANIM *anim, DG_EVMOBJ* evm)
{
	short *vert;
	short *uv;
	unsigned char *weight;
	int i;
	int flag=anim->evm->def->type & 1;
	FMATRIX *mat=anim->evm->matrix[anim->evm->use_buffer];
	FVECTOR *p;
	short **mv;
	unsigned char *wp;
	DG_EVMPACK *packs;
	int verts_count;
	unsigned char *mat_index;

	if(frame>frame_max+30)
      return 0;

   evm->BP_SpecialModelFlag = 1;
   evm->BP_SpecialModelValue = 0.9f - (frame / 30.0f);

	p=anim->stkvec;
	mv=anim->mdlvec;
	wp=anim->mdlweight+8*2;

	for(i=N_VECSTOCK;i>0;i--,p++,mv++,wp+=8*3){
		if(*mv!=NULL){
			FVECTOR pos;
			int j;

			pos.vx=(*mv)[0-4*2];
			pos.vy=(*mv)[1-4*2];
			pos.vz=(*mv)[2-4*2];
			pos.vw=1.0f;

			vu0_Clrv2();
			vu0_Ldv0(&pos);
			vu0_Setv2w1();

			if(!flag){
				vu0_Mulv0a(1.0f/16.0f);
			}

			for(j=0;j<4;j++){
				vu0_Ldm0(mat+wp[j+4-8*2]);
				vu0_Mulv1m0v0();
				vu0_Mulv1a((float)(wp[j-8*2])/(128.0f*3.0f));

				vu0_Addv2v1();
			}

#if 1
			pos.vx=(*mv)[0-4*1];
			pos.vy=(*mv)[1-4*1];
			pos.vz=(*mv)[2-4*1];
			pos.vw=1.0f;

			vu0_Ldv0(&pos);

			if(!flag){
				vu0_Mulv0a(1.0f/16.0f);
			}

			for(j=0;j<4;j++){
				vu0_Ldm0(mat+wp[j+4-8*1]);
				vu0_Mulv1m0v0();
				vu0_Mulv1a((float)(wp[j-8*1])/(128.0f*3.0f));
				vu0_Addv2v1();
			}

			pos.vx=(*mv)[0-4*0];
			pos.vy=(*mv)[1-4*0];
			pos.vz=(*mv)[2-4*0];
			pos.vw=1.0f;

			vu0_Ldv0(&pos);

			if(!flag){
				vu0_Mulv0a(1.0f/16.0f);
			}

			for(j=0;j<4;j++){
				vu0_Ldm0(mat+wp[j+4-8*0]);
				vu0_Mulv1m0v0();
				vu0_Mulv1a((float)(wp[j-8*0])/(128.0f*3.0f));
				vu0_Addv2v1();
			}
#endif

			vu0_Stv2(p);
		}
	}


	vert=anim->vertex[anim->buffer_flag];
	uv=anim->uvs0[anim->buffer_flag];
	weight=anim->evm->weight_ptr;

	packs=anim->evm->packs;
	verts_count=packs->n_verts;
	mat_index=(unsigned char *)&(packs->matrix_list);

	for(i=anim->buffer_size;i>0;i--,vert+=4,uv+=4,weight+=8){
		if(verts_count==0){
			int flag=(packs->n_verts & 1);

			packs++;
			verts_count=packs->n_verts;
			mat_index=(unsigned char *)&(packs->matrix_list);

			if(flag) continue;
		}

		verts_count--;

		uv[1]+=uv_speed;
		if(uv[1]>0x1000) uv[1]=0x1000;

		if((unsigned short)(vert[3])!=0x8fff){
			if(uv[1-4*0]>=UV_FIRESTART &&
			   uv[1-4*0]-uv_speed<=UV_FIRESTART){

				FVECTOR pos;
				int j;

				pos.vx=vert[0-4*2];
				pos.vy=vert[1-4*2];
				pos.vz=vert[2-4*2];
				pos.vw=1.0f;

				vu0_Clrv2();
				vu0_Ldv0(&pos);
				vu0_Setv2w1();

				if(!flag){
					vu0_Mulv0a(1.0f/16.0f);
				}

				*((long64 *)(anim->mdlweight+anim->stkindex*8*3+8*0))=*(long64 *)&(weight[-8*2]);

				for(j=0;j<4;j++){
					int index=weight[j+4-8*2]/4;

					index=*(mat_index+index);
					*(anim->mdlweight+anim->stkindex*8*3+8*0+4+j)=index;

					vu0_Ldm0(mat+index);
					vu0_Mulv1m0v0();
					vu0_Mulv1a((float)(weight[j-8*2])/(128.0f*3.0f));

					vu0_Addv2v1();
				}

#if 1
				pos.vx=vert[0-4*1];
				pos.vy=vert[1-4*1];
				pos.vz=vert[2-4*1];
				pos.vw=1.0f;

				vu0_Ldv0(&pos);

				if(!flag){
					vu0_Mulv0a(1.0f/16.0f);
				}

				*((long64 *)(anim->mdlweight+anim->stkindex*8*3+8*1))=*(long64 *)&(weight[-8*1]);

				for(j=0;j<4;j++){
					int index=weight[j+4-8*1]/4;

					index=*(mat_index+index);
					*(anim->mdlweight+anim->stkindex*8*3+8*1+4+j)=index;

					vu0_Ldm0(mat+index);
					vu0_Mulv1m0v0();
					vu0_Mulv1a((float)(weight[j-8*1])/(128.0f*3.0f));
					vu0_Addv2v1();
				}

				pos.vx=vert[0-4*0];
				pos.vy=vert[1-4*0];
				pos.vz=vert[2-4*0];
				pos.vw=1.0f;

				vu0_Ldv0(&pos);

				if(!flag){
					vu0_Mulv0a(1.0f/16.0f);
				}

				*((long64 *)(anim->mdlweight+anim->stkindex*8*3+8*2))=*(long64 *)&(weight[-8*0]);

				for(j=0;j<4;j++){
					int index=weight[j+4-8*0]/4;

					index=*(mat_index+index);
					*(anim->mdlweight+anim->stkindex*8*3+8*2+4+j)=index;

					vu0_Ldm0(mat+weight[j+4-8*0]);
					vu0_Mulv1m0v0();
					vu0_Mulv1a((float)(weight[j-8*0])/(128.0f*3.0f));
					vu0_Addv2v1();
				}
#endif

#if 0
				vu0_Stv2(&pos);

				NewPointFire(&pos);
#else
				vu0_Stv2(anim->stkvec+anim->stkindex);
				*(anim->mdlvec+anim->stkindex)=vert;

				// SolidusGetFirePoint(anim->stkvec+anim->stkindex,1);
				SolidusMantGetFirePoint(anim->stkvec+anim->stkindex,1);

				anim->stkindex++;
				if(anim->stkindex>=N_VECSTOCK) anim->stkindex=0;
#endif

			}

			if(uv[1-4*0]>=UV_LIMIT &&
			   uv[1-4*1]>=UV_LIMIT &&
			   uv[1-4*2]>=UV_LIMIT){

				vert[3]=0x8fff;
			}
		}
	}

	return 1;
}
#else
/*
	�أ£ϣ��ǥ롼���
*/
#undef UV_FIRESTART
#undef UV_LIMIT
#define UV_FIRESTART	((int)(0.8f*32767))
#define UV_LIMIT		((int)(0.84f*32767))
#define EFFECT_TIME		(30)
/* ĺ����˥��Хåե����ڤ��ؤ� */
void EvmVertexAnimeSwitchBuffer(EVM_VERTEXANIM *anim)
{
	anim->evm->vbuff = anim->verts[anim->buffer_flag];
#ifdef KP_WINDOWS
	DG_AssignEvmObjVertexBuffer(anim->evm) ;
#endif
}

/* ĺ����˥��Хåե��ν����� */
void InitEvmVertexAnime(DG_EVMOBJ *evm,EVM_VERTEXANIM *anim)
{
	DG_EVMPACK *pack=evm->packs;
	int		v_size, size ;
	int i, j;

//printf("make common verts list\n");
	{/* ��ͭĺ��������� */
		V_TAG			*top_vtag ;
		DG_VERTEX_EVM	*verts, *v ;
		EVM_PACK		*pack ;
		u_short			*index ;
		int				hash, count ;
		V_TAG			**vtag_table, *vtag;

		/* �ꥹ�ȥơ��֥��ν����� */
		vtag_table = (void*)SCRPAD_ADDR ;
		top_vtag = (void*)( (char*)SCRPAD_ADDR + 4 * 256 ) ;	/* �����ꥹ�ȵ��������� */
		for ( i = 0 ; i < 256 ; i++ ) vtag_table[ i ] = NULL ;
		count = 0 ;

		/* ���ǥ��Υȥ饤���󥰥륹�ȥ��åפ�ĺ���é�äƤ�����ĺ�����Ͽ���Ƥ��� */
		verts = evm->vbuff ;
		pack = evm->def->packet ;
		for ( i = 0 ; i < evm->n_packet ; i++, pack++ ){
//printf("%d\n", i );
			index = pack->index ;
			for ( j = 0 ; j < pack->n_indices ; j++, index++ ){
				/* ĺ��ǡ������� */
				v = &verts[ *index ] ;
				/* ��ɸ�����ϥå����ͤ��� */
				hash = v->vx + ( v->vy << 1 ) + ( v->vz << 2 );
				hash = ( hash ^ ( hash >> 8 ) ) & 0x00ff ;
//printf("hash = %d\n", hash );
				/* ����Ʊ����ɸ��ĺ�����Ͽ�����Ƥʤ���������å� */
				vtag = vtag_table[ hash ] ;
				while ( vtag != NULL ){
					if ( ( vtag->vert->vx == v->vx ) && ( vtag->vert->vy == v->vy ) && ( vtag->vert->vz == v->vz ) ){
						break ;
					}
					vtag = vtag->next ;
				}
				/* �ޤ���Ͽ�����Ƥʤ�ĺ��ξ�����Ͽ */
				if ( vtag == NULL ){
//printf("regist verts %p\n", top_vtag );
					top_vtag->vert = v ;
					top_vtag->next = vtag_table[ hash ] ;
					/* �����륤���ǥå����򥰥��Х륤���ǥå������ѹ� */
					top_vtag->index[0] = pack->mat_id[ v->index[ 0 ] / 4 ];
					top_vtag->index[1] = pack->mat_id[ v->index[ 1 ] / 4 ];
					top_vtag->index[2] = pack->mat_id[ v->index[ 2 ] / 4 ];
					top_vtag->index[3] = pack->mat_id[ v->index[ 3 ] / 4 ];
					vtag_table[ hash ] = top_vtag ;
					top_vtag++ ;
					count++ ;
				}
			}
		}
		/* ��ͭĺ�������Ͽ */
		anim->n_common_verts = count ;
	}
//printf("total common verts = %d\n", anim->n_common_verts );

	anim->evm=evm;
	anim->buffer_flag=0;
	anim->verts_org = evm->vbuff ;

	/* ĺ����󤬾��񤹤����������׻� */
	v_size = sizeof(DG_VERTEX_EVM) * evm->def->n_verts ;
	v_size = ( v_size + 15 ) & ~15 ;	/* ĺ��������ϣ����Х��ȥ��饤�����Ȥˤ����� */
	/* ���ݥ����ꥵ������������ *//* ��Ⱦ��ʬ�λ���������������� */
	//size = v_size * 2 + N_VECSTOCK * ( sizeof(FVECTOR) + sizeof(short *) + sizeof(unsigned char) * 8 * 3 );
	//size = v_size * 2 + sizeof(C_VERTS) * anim->n_common_verts ;

	anim->verts[0] = DG_AllocLocalVideoMemory( v_size * 2 );
	if ( anim->verts[0] == NULL ){
#ifdef DEBUG_MODE
		printf("EvmVertexAnime : Memory Overflow\n");
#endif
		ASSERT(0);
	}
	anim->verts[1] = (void*)( (char*)anim->verts[0] + v_size ) ;
	//anim->common_verts = (void*)( (char*)anim->verts[1] + v_size ) ;
	anim->common_verts = GV_Malloc( sizeof(C_VERTS) * anim->n_common_verts );

	anim->buffer_size = evm->def->n_verts ;

	/* ���ꥸ�ʥ���ĺ��ǡ����򥢥˥᡼�������Ѥ˥��ԡ� */
#ifndef KP_WINDOWS
	memcpy( anim->verts[0], evm->def->vbuff, v_size );
	memcpy( anim->verts[1], evm->def->vbuff, v_size );
#else
	memcpy( anim->verts[0], evm->vbuff, v_size );
	memcpy( anim->verts[1], evm->vbuff, v_size );
#endif

	{/* �����ΥХåե��Σպ�ɸ��ȿž���뤳�Ȥǥե졼�ऴ�Ȥ˥ƥ�����㥢�˥����̤���� */
		DG_VERTEX_EVM	*verts ;
		verts = anim->verts[1] ;
		for ( i = 0 ; i < anim->buffer_size ; i++, verts++ ){
			verts->u0 = 32767 - verts->u0 ;
			verts->u1 = verts->u0 ;
			verts->u2 = verts->u0 ;
		}
	}

	{/* ��ͭĺ��������� */
		V_TAG			*vtag ;
		C_VERTS	*verts ;
		DG_VERTEX_EVM	*evm_verts ;
		vtag = (void*)( (char*)SCRPAD_ADDR + 4 * 256 ) ;	/* �����ꥹ�ȵ��������� */
#ifndef KP_WINDOWS
		evm_verts = evm->def->vbuff ;
#else
		evm_verts = evm->vbuff ;
#endif
		verts = anim->common_verts ;
		for ( i = 0 ; i < anim->n_common_verts ; i++, vtag++, verts++ ){
			evm_verts = vtag->vert ;
			verts->vert.vx = evm_verts->vx * (1.0f/16.0f) ;
			verts->vert.vy = evm_verts->vy * (1.0f/16.0f) ;
			verts->vert.vz = evm_verts->vz * (1.0f/16.0f) ;
			verts->vert.vw = 1.0f ;
			verts->weight[0] = evm_verts->weight[0] ;
			verts->weight[1] = evm_verts->weight[1] ;
			verts->weight[2] = evm_verts->weight[2] ;
			verts->weight[3] = evm_verts->weight[3] ;
			verts->index[0] = vtag->index[0] ;
			verts->index[1] = vtag->index[1] ;
			verts->index[2] = vtag->index[2] ;
			verts->index[3] = vtag->index[3] ;
			verts->tex_v = evm_verts->v0 ;
			verts->flag = -1 ;
		}
	}

	EvmVertexAnimeSwitchBuffer(anim);
}
/* ĺ����˥ᳫ�� */
void ExitEvmVertexAnime(EVM_VERTEXANIM *anim)
{
	/* ĺ����ɥ쥹���� */
	anim->evm->vbuff = anim->verts_org ;

	if ( anim->verts[0] != NULL ){
		DG_DelayedFreeLocalVideoMemory( anim->verts[0] );
	}
	if ( anim->common_verts != NULL ) GV_Free( anim->common_verts );
}

/* ĺ����˥��ᥤ�� */
int ActAnimDeletePoly(int frame,int frame_max,int uv_speed,EVM_VERTEXANIM *anim)
{
	DG_VERTEX_EVM	*verts ;
	int i;
	int flag = anim->evm->def->type & 1;	/* ���������⡼�� */
	FMATRIX *mat = anim->evm->matrix[anim->evm->use_buffer];
	FVECTOR *p;
	short **mv;
	unsigned char *wp;
	DG_EVMPACK *packs;
	int verts_count;
	unsigned char *mat_index;

	if(frame>frame_max+30) return 0;

	/* ���˥᡼�������Хåե����ڤ��ؤ� */

	anim->buffer_flag = 1 - anim->buffer_flag ;

	/* �񤭴���ĺ��ǡ����Υݥ��󥿤����� */
	verts = anim->verts[ anim->buffer_flag ];

	uv_speed = uv_speed * 32767 / 4096 ;	/* �Уӣ����फ���أ£ϣش������Ѵ� */
	for ( i = anim->buffer_size ; i > 0 ; i--, verts++ ){
		{/* �ֺ�ɸ�Υ������륢�˥᡼���������� */
			int		v ;
			v = verts->v0 + uv_speed ;
			//if ( v > 32767 ) v = 32767 ;
			/*  �ƥ�����㤬��������������ä��Ȥ��Σ��ԥ�����ʬ���;͵���������� */
			if ( v > 32640 ) v = 32640 ;	/* 32640 = 32767 - 128 */
			/* �ؤǤϣգ��ͤ������Τ��Τ����äƤ����Τǣգ��ͤϣ��ĤȤ�Ʊ���ͤˤʤ��Ϥ� */
			verts->v0 = verts->v1 = verts->v2 = v ;
		}
	}

	{/* �������뤷���գ֤˽��äƱꥨ�ե����Ȥ�ȯ�����Ѻ�ɸ�ι������� */
		C_VERTS		*verts ;
		int			v ;

		verts = anim->common_verts ;
		for ( i = 0 ; i < anim->n_common_verts ; i++, verts++ ){
			/* ���ե����ȵ�ư����å� */
			if ( verts->flag == -1 ){
				v = verts->tex_v + uv_speed ;
				if ( v > UV_FIRESTART ){
					verts->flag = EFFECT_TIME ;
				}
				verts->tex_v = v ;
			}
			/* ���ե����ȵ�ưĺ����Ф��ƻ����Ѻ�ɸ�򹹿����� */
			if ( verts->flag > 0 ){
				FVECTOR		pos, tmp ;
				float		w, s = 1.0f / 32767.0f ;
				/* ĺ���ɸ�λ��� */
				_sceVu0ApplyMatrix( &tmp, mat + verts->index[0], &verts->vert );
				_sceVu0ScaleVector( &pos, &tmp, verts->weight[0] * s );
				_sceVu0ApplyMatrix( &tmp, mat + verts->index[1], &verts->vert );
				_sceVu0ScaleVector( &tmp, &tmp, verts->weight[1] * s );
				_sceVu0AddVector( &pos, &pos, &tmp );
				_sceVu0ApplyMatrix( &tmp, mat + verts->index[2], &verts->vert );
				_sceVu0ScaleVector( &tmp, &tmp, verts->weight[2] * s );
				_sceVu0AddVector( &pos, &pos, &tmp );
				_sceVu0ApplyMatrix( &tmp, mat + verts->index[3], &verts->vert );
				_sceVu0ScaleVector( &tmp, &tmp, verts->weight[3] * s );
				_sceVu0AddVector( &pos, &pos, &tmp );
				pos.vw = 1.0f ;
				verts->pos = pos ;
				/* �գ֤���ã����ľ���ξ����ˤϥ��ե�����ȯ� */
				if ( verts->flag == EFFECT_TIME ){
					SolidusMantGetFirePoint( &verts->pos, 1 );
				}
				verts->flag-- ;
			}
		}
	}

	return 1;
}
#endif


/* -------------------------------------------------------------- */
/* -------------------------------------------------------------- */


typedef struct solmant2_Work {
	GV_ACT_EX actor;
	HAIR_WORK hair;
	EVM_VERTEXANIM vanim;
	int frame;
	int name;
	int uv_speed;
	int frame_max;
#ifndef PSX2
	int	first_flag ;
#endif
} Work;


static void Act(Work *_work)
{
    HZX_GROUP_ID hzx_id=HZX_CurrentGroupID;
	HAIR_WORK *work=&(_work->hair);
	int calc_flag=work->calc_flag;
	int calc_init_flag=0;

    GV_MSG *msg;
    int n_msg ;


	if(calc_flag==2){
		calc_init_flag=1;
		calc_flag=0;
		work->calc_flag=1;
	}

    n_msg=GV_ReceiveMessage(_work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);

		switch(command){
		case 0:
			/* ɽ������ɽ�� */
			{
				int disp_sw;

				disp_sw=arg;

				switch(disp_sw){
				case 0:
				case 1:
                    work->mesg_disp = disp_sw ;
					break;
				case -1:
                    work->mesg_disp ^= 1 ;
					break;
				}
			}
			break;
		case 1:
			/* �ѥ��᡼�����ѹ� */
			{
				int d;

				d=arg;

				work->m=hair_sample[d].m;
				work->inv_m=1.0f/work->m;
				work->pa=hair_sample[d].pa;
				work->k=-hair_sample[d].k;
				work->deg_param=hair_sample[d].deg_param;
				work->we=-hair_sample[d].we;
				work->wl=hair_sample[d].wl;
			}
			break;
		case 2:
			/* �饤�ȥե饰���ѹ� */
			work->light_flag=arg;
			break;
		case 3:
			/* ȱ���ӷ׻����Ԥ����Ԥ��ʤ����Υե饰 */
			switch(arg){
			case 0:
			case 1:
				work->calc_flag=*(msg->message+1);
				break;
			case -1:
				work->calc_flag^=1;
				break;
			case -2:
				calc_init_flag=1;
				break;
			}
			calc_init_flag|=(!calc_flag && work->calc_flag);
			break;
		case 4:
			if(_work->frame<0) _work->frame=0;
			break;
		case 5:
			ExitEvmVertexAnime(&(_work->vanim));
			InitEvmVertexAnime(work->evm,&(_work->vanim));
			_work->frame=-1;
			break;
		}

		msg++;
		n_msg--;
    }



    if(work->target!=NULL){
		if(work->visible_flag){
			if(work->mesg_disp) work->evm->flag&=~work->invisible_flags;
			else work->evm->flag|=work->invisible_flags;
		}
		else if(work->target->evmobj!=NULL){
			if(work->target->evmobj->flag & work->invisible_flags){
				work->evm->flag|=work->invisible_flags;
			}
			else{
				if(work->mesg_disp) work->evm->flag&=~work->invisible_flags;
				else work->evm->flag|=work->invisible_flags;
			}
		}
		else if(work->target->objs->flag & DG_FLAG_INVISIBLE){
			work->evm->flag|=work->invisible_flags;
		}
		else if(work->target->objs->objs[work->tobjnum[work->n_parents-1]].flag & DG_FLAG_INVISIBLE){
			work->evm->flag|=work->invisible_flags;
		}
		else{
			if(work->mesg_disp) work->evm->flag&=~work->invisible_flags;
			else work->evm->flag|=work->invisible_flags;
		}

		fpu_CopyMatrix(&(work->light[0]),&(work->target->objs->light[0]));
		fpu_CopyMatrix(&(work->light[1]),&(work->target->objs->light[1]));
    }
	else{
		if(work->mesg_disp) work->evm->flag&=~DG_EVMOBJ_INVISIBLE;
		else work->evm->flag|=DG_EVMOBJ_INVISIBLE;

		DG_GetLightMatrix((FVECTOR *)&(work->root.m[3][0]),work->light);
	}

	if(work->light_flag){
		extern const float p_array[][2];
		float pp,np;
		int i;

		pp=p_array[work->light_flag][0];
		np=p_array[work->light_flag][1];

		for(i=0;i<3;i++){
			float c=0.0f;

			c+=work->light[1].m[0][i]*pp;
			work->light[1].m[0][i]*=np;
			c+=work->light[1].m[1][i]*pp;
			work->light[1].m[1][i]*=np;
			c+=work->light[1].m[2][i]*pp;
			work->light[1].m[2][i]*=np;

			work->light[1].m[3][i]+=c;
			if(work->light[1].m[3][i]>255.0f) work->light[1].m[3][i]=255.0f;
		}
	}

	if(GM_CheckGameStatus(STATE_DEMO) && (DM_EffectInitFlag & DM_EFFECT_INIT)){
		CalcHairCoordinate(work);
	}
	else if(calc_init_flag){
		CalcHairCoordinate(work);
	}

	if(work->calc_flag) MoveHairEvm(work,hzx_id);

	if(_work->frame>=0)
   {
		_work->vanim.buffer_flag=1-_work->vanim.buffer_flag;

		if(ActAnimDeletePoly(_work->frame,_work->frame_max,_work->uv_speed,&(_work->vanim), _work->hair.evm) ){
			EvmVertexAnimeSwitchBuffer(&(_work->vanim));
			_work->frame++;
#ifndef PSX2
			if ( _work->first_flag ){
				_work->uv_speed *= 2 ;
				_work->first_flag = 0 ;
			}
#endif
		}
		else
      {
			// ExitEvmVertexAnime(&(_work->vanim));
			// InitEvmVertexAnime(work->evm,&(_work->vanim));
			// _work->frame=-1;
		}
	}

#ifdef DEBUG_MODE
	if(work->debug_flag){
		void *NewEvmSkeletonTest(DG_EVMOBJ *obj,int color);
		NewEvmSkeletonTest(work->evm,0x00ffff00);
	}
#endif

}

static void InitAct(Work *_work)
{
	HairEvmCalcFirst(&(_work->hair));
	CalcHairCoordinate(&(_work->hair));
	GV_ChangeActFunc(&(_work->actor),Act);
}

static void Die(Work *work)
{
	ExitEvmVertexAnime(&(work->vanim));
	ExitHairEvm(&(work->hair));
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void GetOptionValue_called(Work *_work,int sample_num,
								  OBJECT *target,unsigned char *objnum,int n_objnum,
								  FVECTOR *px,SVECTOR *pr,
								  float oval_param,int collision_flag,unsigned char *collision_objs,
								  int visible_flag,int light_flag,int boundmodel_name)
{
	HAIR_WORK *work=&(_work->hair);
    FVECTOR x;
    SVECTOR rot;
	int i;

    work->m=0.001f;
    work->inv_m=1.0f/work->m;
    work->pa=0.001f;
    work->k=-0.0001f;
    work->we=-1.0f;
    work->wl=(int)CVC2N(10);
	work->deg_param=0.1f;
    work->param_oval=1.2f;
	work->mov_rate=0.0f;
	work->ymov_rate=0.0f;

    work->collision_flag=0;
    work->visible_flag=0;
	work->light_flag=0;
	work->mesg_disp=1;
	work->calc_flag=1;

	work->calc_angree_param=0.0f;
	work->calc_angree_frame=0;
	work->calc_angree_fade_frame=0;

    work->tmat[0]=NULL;
    fpu_CopyUnitMatrix(&(work->root));
    fpu_ClearVector(&x);
    rot=DG_ZeroSVector;

    work->target=NULL;
    work->tobjnum[0]=0;
	work->n_parents=1;

    work->boundmodel=NULL;

#ifdef DEBUG_MODE
	work->debug_flag=0;
#endif

    {
		int d=sample_num;

		work->m=hair_sample[d].m;
		work->inv_m=1.0f/work->m;
		work->pa=hair_sample[d].pa;
		work->k=-hair_sample[d].k;
		work->deg_param=hair_sample[d].deg_param;
		work->we=-hair_sample[d].we;
		work->wl=hair_sample[d].wl;
		work->mov_rate=hair_sample[d].mov_rate;
		work->ymov_rate=hair_sample[d].ymov_rate;
    }

    /* �������� */
	if(px!=NULL) fpu_CopyVector(&x,px);
    x.vw=1.0f;

    /* ����chara������obj���դ��Ƥ��� */
	work->target=target;
	work->n_parents=n_objnum;
	for(i=0;i<work->n_parents;i++){
		work->tobjnum[i]=objnum[i];
	}

    if(work->target!=NULL){
		int i=work->n_parents;

		while(i>0){
			i--;
			work->tmat[i]=&(work->target->objs->objs[work->tobjnum[i]].world);
		}

		// DG_SetLightMatrix(work->evm,work->target->objs->light);
		// work->evm->light=work->target->objs->light;

		fpu_CopyMatrix(&(work->light[0]),&(work->target->objs->light[0]));
		fpu_CopyMatrix(&(work->light[1]),&(work->target->objs->light[1]));
    }
    else{
		DG_GetLightMatrix(&x,work->light);
    }
	// DG_SetLightMatrix(work->evm,work->light);
	work->evm->light=work->light;

   work->evm->BP_SpecialModelFlag = 0;
   work->evm->BP_SpecialModelValue = 1.0f;

    /* �ʱߵ�����Υѥ��᡼�� */
	work->param_oval=oval_param;

    /* ���Ƚ���ե饰 */
	work->collision_flag=collision_flag;

	/* �����Ƚ���򤹤륪�֥������Ȥ��ֹ������� */
	if(collision_flag>0 && collision_objs!=NULL){
		int nobjs=collision_flag;

		// printf("N OBJS = %d\n",nobjs);

		for(i=0;i<nobjs;i++){
			int n=collision_objs[i];
			work->collision_objs[i]=n;

			// printf("OBJNUM = %d\n",n);
		}
		if(work->target!=NULL && work->collision_flag){
			work->collision_flag=nobjs;
		}
	}

    if(boundmodel_name!=0){
		work->boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(boundmodel_name,'k'));
    }

    /* ���ɽ���ե饰 */
	work->visible_flag=visible_flag;

    /* �饤�ȥե饰 */
	work->light_flag=light_flag;

	if(pr!=NULL) rot=*pr;

    DG_SetPos(&DG_UnitMatrix);
    DG_RotatePosZYX(&rot);
    DG_GetPos(&(work->root));

    fpu_CopyVector((FVECTOR *)&(work->root.m[3][0]),&x);

	InitEvmVertexAnime(work->evm,&(_work->vanim));
}

/* �񸻤����� */
static int GetResources_called(Work *work,int model_name,int sample_num,
							   OBJECT *target,unsigned char *objnum,int n_objnum,
							   FVECTOR *x,SVECTOR *r,
							   float oval_param,int collision_flag,unsigned char *collision_objs,
							   int visible_flag,int light_flag,int boundmodel_name)
{
	if(!InitHairEvm(&(work->hair),model_name,0)) return 0;
    GetOptionValue_called(work,sample_num,
						  target,objnum,n_objnum,x,r,
						  oval_param,collision_flag,collision_objs,
						  visible_flag,light_flag,boundmodel_name);

    return 1;
}


void *NewSolMant_called(int name,int model_name,int sample_num,
						OBJECT *target,FVECTOR *x,SVECTOR *r,
						float oval_param,int visible_flag,int light_flag,int boundmodel_name,
						int frame_max)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		int sample_num2;
		unsigned char *objnum;
		int n_objnum;
		unsigned char *coliobjs;
		int n_coliobjs;

        work->name=name;

        GV_SetActor(&(work->actor),InitAct,Die) ;
        GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		sample_num2=19;

		objnum=(unsigned char *)(hair_root[sample_num]);
		n_objnum=n_hair_root[sample_num];
		coliobjs=(unsigned char *)(hair_colcheck[sample_num]);
		if((n_coliobjs=n_hair_colcheck[sample_num])==0xff){
			n_coliobjs=-1;
		}

        if(!GetResources_called(work,model_name,sample_num2,
								target,objnum,n_objnum,x,r,
								oval_param,n_coliobjs,coliobjs,
								visible_flag,light_flag,boundmodel_name)){
            GV_DestroyActor(work) ;
            return NULL ;
        }

		work->frame=-1;
		work->name=name;
		work->uv_speed=2*0x1000/frame_max;
		work->frame_max=frame_max;
#ifndef PSX2
		work->uv_speed /= 2 ;
		work->first_flag = 1 ;
#endif

		// GV_SetActorChild(work,NewSolidusTestPoint2());
		GV_SetActorChild(work,NewSolidusMantFire());
    }
    return (void *)work ;
}
