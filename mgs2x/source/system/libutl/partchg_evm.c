//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	partchg_evm.c
		人間のモデルの一部すり替えのマルチウェイト版

	2000/03/28 K.Kano
	$Id: partchg_evm.c,v 1.1.1.3 2002/11/19 11:42:58 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "partchg.h"

#include "BP_EndianSupport.h"

/*
  モデルの一部をマルチウェイトアニメーションさせるために、
  マルチウェイトモデルに切替える。
  */


EVMPARTCHANGE_WORK *InitEvmPartChange(DG_OBJS *human,int index,EVM_DEF *evm)
{
	EVMPARTCHANGE_WORK *work;

	if((work=GV_Malloc(sizeof(EVMPARTCHANGE_WORK)))==NULL) return NULL;

	if((work->evmobj=DG_MakeEvmObj(evm,0,0))==NULL){
		GV_Free(work);
		return NULL;
	}
	DG_QueueEvmObj(work->evmobj);
	work->evmobj->light=human->light;

	work->human=human;
	work->index=index;
	work->disp_enable=1;
	work->mtn_enable=0;

	return work ;
}

void ExitEvmPartChange(EVMPARTCHANGE_WORK *work)
{
	int index=work->index;

	work->human->objs[index].flag&=~DG_FLAG_INVISIBLE;
	DG_QueueEvmObj(work->evmobj);
	DG_FreeEvmObj(work->evmobj);
	GV_Free(work);
}



/* EVMオブジェクトにMTNファイルの内容を展開 */
static void EvmActMotion(DG_EVMOBJ *evmobj,int n_joints,FVECTOR *skel_trans,FVECTOR *skel_rot)
{
    int	i ;
    FMATRIX *skel_mats ;
    FVECTOR vec ;
    FMATRIX mat ;
    EVM_SKEL *skel ;

    skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

    /* オブジェクトにマトリクスを設定する */
    skel = &evmobj->def->skeleton[0] ;
    vec.vw = 1.0F ;

    /* モデル情報から親子関係を取得して求める */
    for ( i = 0 ; i < n_joints ; i++ ){
		FMATRIX	*parent ;

		MT_QuatToMat( &mat, skel_rot );

		mat.m[3][0] = skel->rt_tx + skel_trans->vx ;
		mat.m[3][1] = skel->rt_ty + skel_trans->vy ;
		mat.m[3][2] = skel->rt_tz + skel_trans->vz ;

		if(skel->parent==-1) parent=&(evmobj->world);
		else parent = &skel_mats[ skel->parent ] ;

		_sceVu0MulMatrix( &mat, parent, &mat ) ;
		vec.vx = -skel->rt_tx ;
		vec.vy = -skel->rt_ty ;
		vec.vz = -skel->rt_tz ;
		_sceVu0ApplyMatrix( &(mat.m[3][0]), &mat, &vec );
		skel_mats[i] = mat ;

		skel_rot++ ;
		skel_trans++ ;
		skel++ ;
    }
}

static void EvmActMotionDummy(DG_EVMOBJ *evmobj,FMATRIX *mat)
{
    EVM_SKEL *skel ;
    FMATRIX *skel_mats ;
    int i;

    skel=evmobj->def->skeleton;
    skel_mats=evmobj->matrix[evmobj->use_buffer];

    for(i=0;i<evmobj->def->n_x_models;i++,skel++,skel_mats++){
		FVECTOR vec;
		int parent=skel->parent;

		if(parent==-1){
			fpu_CopyMatrix(mat+i,&(evmobj->world));
		}
		else{
			fpu_CopyMatrix(mat+i,mat+parent);
		}
		fpu_CopyMatrix(skel_mats,mat+i);

		vec.vx=skel->tx;
		vec.vy=skel->ty;
		vec.vz=skel->tz;
		vec.vw=1.0f;
		_sceVu0ApplyMatrix(&vec,mat+i,&vec);
		(mat+i)->m[3][0]=vec.vx;
		(mat+i)->m[3][1]=vec.vy;
		(mat+i)->m[3][2]=vec.vz;

		vec.vx=-skel->rt_tx;
		vec.vy=-skel->rt_ty;
		vec.vz=-skel->rt_tz;
		vec.vw=1.0f;
		_sceVu0ApplyMatrix(&vec,mat+i,&vec);
		skel_mats->m[3][0]=vec.vx;
		skel_mats->m[3][1]=vec.vy;
		skel_mats->m[3][2]=vec.vz;
    }
}


void MoveEvmPartChange(EVMPARTCHANGE_WORK *work)
{
	int index=work->index;

    /* 表示／非表示設定 */
    if(work->disp_enable){
		work->evmobj->flag&=~DG_EVMOBJ_INVISIBLE;
		work->human->objs[index].flag|=DG_FLAG_INVISIBLE;
    }
    else{
		work->evmobj->flag|=DG_EVMOBJ_INVISIBLE;
		work->human->objs[index].flag&=~DG_FLAG_INVISIBLE;
    }

    /* 親側の表示／非表示を反映 */
    if(work->human->flag & DG_FLAG_INVISIBLE){
		work->evmobj->flag|=DG_EVMOBJ_INVISIBLE;
    }

	fpu_CopyMatrix(&(work->evmobj->world),&(work->human->objs[index].world));

	if(work->mtn_enable){
		FVECTOR *trans,*rots;
		int i;

		trans=(FVECTOR *)SCRPAD_ADDR;
		rots=trans+work->mtn.n_joints;

		for(i=0;i<work->mtn.n_joints;i++){
			int j=(work->mtn.length+1)*i+work->frame+1;
			fpu_CopyVector(rots+i,work->mtn.rots+j);
			fpu_CopyVector(trans+i,work->mtn.trans+j);
		}

		EvmActMotion(work->evmobj,work->mtn.n_joints,trans,rots);
		work->frame++;
		if(work->frame>=work->mtn.length) work->frame=work->mtn.length-1;
	}
	else{
		FMATRIX *mat;

		mat=(FMATRIX *)SCRPAD_ADDR;
		EvmActMotionDummy(work->evmobj,mat);
	}
}

/* ---------------------------------------------------------------- */
/* ＭＴＮデータのセットアップ */

void SetMtnForEvmPartChange(EVMPARTCHANGE_WORK *work,int name_id)
{
	RMT_DATA *mtn_data=&(work->mtn);
    RMT_FILE_HEADER	*mtn_file_header ;
    int	size ;

    mtn_file_header=GV_GetCache(GV_CacheID(name_id,'r'));

    if(mtn_file_header!=NULL){
      RMT_VerifyEndianSwap( mtn_file_header );

		mtn_data->length = mtn_file_header->motion_length ;
		mtn_data->n_joints = mtn_file_header->motion_joints ;
		size = 0 ;
		mtn_data->move = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size = mtn_file_header->move_size ;
		mtn_data->rots = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size += mtn_file_header->rots_size ;
		mtn_data->trans = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size += mtn_file_header->trans_size ;

		work->frame=0;
		work->mtn_enable=1;
    }
}
