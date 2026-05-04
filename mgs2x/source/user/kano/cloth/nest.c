/*
	nest.c
		布シミュレート
		布の各頂点で、固定点からの距離を演算

	1999/08/16 K.Kano
	$Id: nest.c,v 1.1.1.3 2002/11/19 11:43:08 Yoshizawa1 Exp $
*/


#include "cloth.h"

static void set_fnest_sub(int index,VERTEX_PARAMETER *verp,LINE_PARAMETER *linep,int num_lines)
{
    int i;

    for(i=0;i<num_lines;i++,linep++){
	int new_index=0;

	if(linep->index[0]==index){
	    new_index=linep->index[1];
	}
	else if(linep->index[1]==index){
	    new_index=linep->index[0];
	}
	else continue;

	{
	    float fnest;

	    vu0_Ldv0(&((verp+new_index)->lbase));
	    vu0_Ldv1(&((verp+index)->lbase));

	    vu0_Subv0v1();

	    fnest=vu0_VectorLength2v0();
	    fnest=(verp+index)->fnest+fpu_Sqrt(fnest);
	    if((verp+new_index)->fnest<=fnest) continue;

	    (verp+new_index)->fnest=fnest;
	    set_fnest_sub(new_index,verp,linep,num_lines);
	}
    }
}

static void set_fnest(Work *work)
{
    VERTEX_PARAMETER *verp=work->ver_params;
    int i;

    verp=work->ver_params;

    for(i=0;i<work->num_vers;i++,verp++){
	verp->fnest=FLT_MAX;
	if(verp->type==VERTEX_TYPE_STATIC) verp->fnest=0.0f;
    }

    verp=work->ver_params;

    for(i=0;i<work->num_vers;i++,verp++){
	if(verp->type==VERTEX_TYPE_STATIC){
	    set_fnest_sub(i,work->ver_params,work->line_params,work->num_lines);
	}
    }
}
