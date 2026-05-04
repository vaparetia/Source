//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	nest.c
		布シミュレート
		布の各頂点で、固定点からの距離を演算

	1999/08/16 K.Kano
	$Id: nest.c,v 1.1.1.3 2002/11/19 11:43:44 Yoshizawa1 Exp $
*/


#include "waving_cloth.h"

static void set_fnest_sub(FVECTOR *verts,int index,VERTEX_PARAMETER *verp,
						  LINE_PARAMETER *linep,int num_lines)
{
    LINE_PARAMETER *linep2=linep;
    int i;

    for(i=num_lines;i>0;i--,linep2++){
		int new_index=0;

		if(linep2->index[0]==index){
			new_index=linep2->index[1];
		}
		else if(linep2->index[1]==index){
			new_index=linep2->index[0];
		}
		else continue;

		// printf("index , new index = %d , %d\n",index,new_index);

		{
			float fnest;

			fnest=(verp+index)->fnest+linep2->l0;
			if((verp+new_index)->fnest<=fnest) continue;

			(verp+new_index)->fnest=fnest;
			set_fnest_sub(verts,new_index,verp,linep,num_lines);
		}
    }
}

static void set_fnest(Work *work)
{
    CV2_MDL *cvd=(*(work->cvd_def+0))->models;
    VERTEX_PARAMETER *verp=work->ver_params;
    int i;

    verp=work->ver_params;

    for(i=0;i<cvd->n_verts;i++,verp++){
		verp->fnest=FLT_MAX;
		if((cvd->vert_usrdata+i)->nVertexSwing==0) verp->fnest=0.0f;
    }

    verp=work->ver_params;

    for(i=0;i<work->num_vers;i++,verp++){
		if((cvd->vert_usrdata+i)->nVertexSwing==0){
			set_fnest_sub(cvd->verts,i,work->ver_params,work->line_params,work->num_lines);
		}
    }
}
