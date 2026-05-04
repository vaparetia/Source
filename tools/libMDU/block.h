/*
   block.h

   メモリ管理

   by K.Kano 2/29/2000
   $Id: block.h,v 1.5 2002/08/26 11:22:09 usr01363 Exp $

   Konami Computer Entertainment Japan West   
*/

#ifndef _block_h_
#define _block_h_


/* block.c */

/*----------------------------------------------------------------*/

FVECTOR *Cv2_Verts_Alloc(int n_verts,long *size);
FVECTOR *Cv2_Norms_Alloc(int n_norms,long *size);
FVECTOR *Cv2_Uvs_Alloc(int n_uvs,long *size);

short *Cv2_VertsIndex_Alloc(int n_verts_index,long *size);
short *Cv2_NormsIndex_Alloc(int n_norms_index,long *size);
short *Cv2_UvsIndex_Alloc(int n_uvs_index,long *size);

VERTEX_USERDATA *Cv2_VertUsrdata_Alloc(int n_verts,long *size);
NORMAL_USERDATA *Cv2_NormUsrdata_Alloc(int n_norms,long *size);

CV2_OBJS *Cv2_Alloc(int n_models,long *size);

void Cv2_Free(CV2_OBJS *def);

void Cv2Obj_Move(CV2_OBJ *r,CV2_OBJ *a);

/*----------------------------------------------------------------*/

KMS2_DEF *Kms2_Alloc(int n_models,int n_x_models,long *size);
KMS2_MDLPACK *Kms2_MdlPack_Alloc(int n_packs,long *size);
short *Kms2_Verts_Alloc(int n_verts,long *size);
short *Kms2_Norms_Alloc(int n_verts,long *size);
short *Kms2_Uvs_Alloc(int n_verts,long *size);
void *Kms2_VBuff_Alloc(int n_xverts,int multi_flag,long *size);
unsigned short *Kms2_Index_Alloc(int n_verts,long *size);

void Kms2_Verts_Free(short *verts);
void Kms2_Norms_Free(short *norms);
void Kms2_Uvs_Free(short *uvs);
void Kms2_VBuff_Free(void *vbuff);
void Kms2_Index_Free( unsigned short* p );

void Kms2_MdlPack_Free(KMS2_MDLPACK *pack,int n_packs);

void Kms2_Free(KMS2_DEF *def);

void Kms2_Mdl_Move(KMS2_MDL *r,KMS2_MDL *a,int n_models);
void Kms2_MdlPack_Move(KMS2_MDLPACK *r,KMS2_MDLPACK *a,int n_packs);

/*-----------------------------------------------------------------*/

EVM_DEF *Evm_Alloc(int n_models,int n_x_models,long *size);
EVM_PACK *Evm_Pack_Alloc(int n_packs,long *size);
void *Evm_Verts_Alloc(int n_verts,long *size);
void *Evm_Norms_Alloc(int n_verts,long *size);
void *Evm_Uvs_Alloc(int n_verts,long *size);
void *Evm_Weight_Alloc(int n_verts,long *size);

void Evm_Free(EVM_DEF *def);
void Evm_Pack_Free(EVM_PACK *packs,int n_packs);
void Evm_Verts_Free(void *verts);
void Evm_Norms_Free(void *norms);
void Evm_Uvs_Free(void *uvs);
void Evm_Weight_Free(void *weight);

/*-----------------------------------------------------------------*/

KMX_VERTS *Kmx_Verts_Alloc(int n_verts);
KMX_NORMS *Kmx_Norms_Alloc(int n_norms);
KMX_PRIMS *Kmx_Prims_Alloc(int n_prims);
int Kmx_PrimData_Alloc(KMX_PRIMS *prim,int n_id,int n_tid);
KMX_XVERTS *Kmx_XVerts_Alloc(int n_verts);
TEXINFO *Kmx_Texs_Alloc(int n_texs);
char *Kmx_Tex_Set(char *s);

int Kmx_MdlData_Alloc(KMX_MDL *mdl,int n_verts,int n_norms,int n_prims);
KMX_DEF *Kmx_Alloc(int n_models,int n_x_models);

void Kmx_Verts_Free(KMX_VERTS *verts);
void Kmx_Norms_Free(KMX_NORMS *norms);
void Kmx_Prims_Free(KMX_PRIMS *prims,int n_prims);
void Kmx_XVerts_Free(KMX_XVERTS *xverts);
void Kmx_MdlData_Free(KMX_MDL *mdl);
void Kmx_Free(KMX_DEF *def);

void Kmx_MdlData_Move(KMX_MDL *r,KMX_MDL *a,int n_models);
void Kmx_Prims_Move(KMX_PRIMS *r,KMX_PRIMS *a,int n_prims);

#define Kmx_Prim_Move(r,a)		Kmx_Prims_Move(r,a,1)
#define Kmx_MdlData1_Move(r,a)		Kmx_MdlData_Move(r,a,1)

/*-----------------------------------------------------------------*/

EVF_VERTS *Evf_Verts_Alloc(int n_verts);
EVF_ENV *Evf_Envs_Alloc(int n_envs);
#define Evf_Norms_Alloc(n_norms)	(EVF_NORMS *)Kmx_Norms_Alloc(n_norms)
#define Evf_Prims_Alloc(n_prims)	(EVF_PRIMS *)Kmx_Prims_Alloc(n_prims)
#define Evf_PrimData_Alloc(prim,n_id,n_tid)	Kmx_PrimData_Alloc((KMX_PRIMS *)(prim),n_id,n_tid)
#define Evf_Texs_Alloc(n_texs)		Kmx_Texs_Alloc(n_texs)
#define Evf_Tex_Set(s)			Kmx_Tex_Set(s)

int Evf_MeshData_Alloc(EVF_MESH *mdl,int n_verts,int n_norms,int n_prims);
EVF_DEF *Evf_Alloc(int n_models,int n_x_models);

void Evf_Verts_Free(EVF_VERTS *verts,int n_verts);
#define Evf_Norms_Free(norms)		Kmx_Norms_Free((KMX_NORMS *)(norms))
#define Evf_Prims_Free(prims,n_prims)	Kmx_Prims_Free((KMX_PRIMS *)(prims),n_prims)
void Evf_MeshData_Free(EVF_MESH *mdl);
void Evf_Free(EVF_DEF *def);

#define Evf_Prims_Move(r,a,n_prims)	Kmx_Prims_Move((KMX_PRIMS *)(r),(KMX_PRIMS *)(a),n_prims)
void Evf_MeshData_Move(EVF_MESH *r,EVF_MESH *a);

#define Evf_Prim_Move(r,a)		Evf_Prims_Move(r,a,1)

/*-----------------------------------------------------------------*/


#endif
