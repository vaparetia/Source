/*

	premove.c
	デザイナープレビュー環境用：物体移動
	2000/02/11 K.Kano
	$Id: premove.c,v 1.1.1.3 2002/11/19 11:43:30 Yoshizawa1 Exp $

*/


#include "preview_def.h"


int PreviewVMoveXY(FVECTOR *pos,float v)
{
    FVECTOR dx,dy;
    int flag=0;

    fpu_MulVectorScaler(&dx,(FVECTOR *)&(DG_Chanl(0)->eye.m[0][0]),v);
    fpu_MulVectorScaler(&dy,(FVECTOR *)&(DG_Chanl(0)->eye.m[1][0]),v);

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	fpu_SubVectors(pos,pos,&dx);
	flag=1;
	break;
    case PAD_R:
	fpu_AddVectors(pos,pos,&dx);
	flag=1;
	break;
    }
    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
	fpu_SubVectors(pos,pos,&dy);
	flag=1;
	break;
    case PAD_D:
	fpu_AddVectors(pos,pos,&dy);
	flag=1;
	break;
    }
    return flag;
}

int PreviewVMoveZ(FVECTOR *pos,float v)
{
    FVECTOR dz;
    int flag=0;

    fpu_MulVectorScaler(&dz,(FVECTOR *)&(DG_Chanl(0)->eye.m[2][0]),v);

    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
	fpu_AddVectors(pos,pos,&dz);
	flag=1;
	break;
    case PAD_D:
	fpu_SubVectors(pos,pos,&dz);
	flag=1;
	break;
    }
    return flag;
}

int PreviewVMoveXYZ(FVECTOR *pos,float v)
{
    FVECTOR dx,dy,dz;
    int flag=0;

    fpu_MulVectorScaler(&dx,(FVECTOR *)&(DG_Chanl(0)->eye.m[0][0]),v);
    fpu_MulVectorScaler(&dy,(FVECTOR *)&(DG_Chanl(0)->eye.m[1][0]),v);
    fpu_MulVectorScaler(&dz,(FVECTOR *)&(DG_Chanl(0)->eye.m[2][0]),v);

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	fpu_SubVectors(pos,pos,&dx);
	flag=1;
	break;
    case PAD_R:
	fpu_AddVectors(pos,pos,&dx);
	flag=1;
	break;
    }
    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
	fpu_SubVectors(pos,pos,&dy);
	flag=1;
	break;
    case PAD_D:
	fpu_AddVectors(pos,pos,&dy);
	flag=1;
	break;
    }
    switch(PreviewKey.auto_status & (PAD_X|PAD_Y)){
    case PAD_X:
	fpu_AddVectors(pos,pos,&dz);
	flag=1;
	break;
    case PAD_Y:
	fpu_SubVectors(pos,pos,&dz);
	flag=1;
	break;
    }
    return flag;
}

int PreviewMoveX(FVECTOR *pos,float v)
{
    int flag=0;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	pos->vx-=v;
	flag=1;
	break;
    case PAD_R:
	pos->vx+=v;
	flag=1;
	break;
    }
    return flag;
}

int PreviewMoveY(FVECTOR *pos,float v)
{
    int flag=0;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	pos->vy-=v;
	flag=1;
	break;
    case PAD_R:
	pos->vy+=v;
	flag=1;
	break;
    }
    return flag;
}

int PreviewMoveZ(FVECTOR *pos,float v)
{
    int flag=0;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	pos->vz-=v;
	flag=1;
	break;
    case PAD_R:
	pos->vz+=v;
	flag=1;
	break;
    }
    return flag;
}

static void EularYXZ2Rot(SVECTOR *rot,FMATRIX *m)
{
    rot->vx=(short)((atan2f(m->m[2][1],fpu_Sqrt(1.0f-m->m[2][1]*m->m[2][1]))
		     *2048.0f)/M_PI) & 0xfff;
    rot->vy=(short)((atan2f(m->m[2][0],m->m[2][2])*2048.0f)/M_PI) & 0xfff;
    rot->vz=(short)((atan2f(m->m[0][1],m->m[1][1])*2048.0f)/M_PI) & 0xfff;
}

int PreviewVRotXY(FMATRIX *world,SVECTOR *rot,int v)
{
    FMATRIX m,inv,r;
    SVECTOR trot;
    int flag=0;

    trot=DG_ZeroSVector;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	trot.vx=-v;
	flag=1;
	break;
    case PAD_R:
	trot.vx=v;
	flag=1;
	break;
    }
    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
	trot.vy=-v;
	flag=1;
	break;
    case PAD_D:
	trot.vy=v;
	flag=1;
	break;
    }

    DG_SetPos2(&DG_ZeroVector,rot);
    DG_GetPos(&m);
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&m);
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)world,*(sceVu0FMATRIX *)&inv);
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&m);

    DG_SetPos2(&DG_ZeroVector,&trot);
    DG_GetPos(&r);
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&(DG_Chanl(0)->eye));
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&r);

    EularYXZ2Rot(rot,&m);

    return flag;
}

int PreviewVRotZ(FMATRIX *world,SVECTOR *rot,int v)
{
    FMATRIX m,inv,r;
    SVECTOR trot;
    int flag=0;

    trot=DG_ZeroSVector;

    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
	trot.vz=-v;
	flag=1;
	break;
    case PAD_D:
	trot.vz=v;
	flag=1;
	break;
    }

    DG_SetPos2(&DG_ZeroVector,rot);
    DG_GetPos(&m);
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&m);
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)world,*(sceVu0FMATRIX *)&inv);
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&m);

    DG_SetPos2(&DG_ZeroVector,&trot);
    DG_GetPos(&r);
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&(DG_Chanl(0)->eye));
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&r);

    EularYXZ2Rot(rot,&m);

    return flag;
}

int PreviewVRotXYZ(FMATRIX *world,SVECTOR *rot,int v)
{
    FMATRIX m,inv,r;
    SVECTOR trot;
    int flag=0;

    trot=DG_ZeroSVector;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	trot.vx=-v;
	flag=1;
	break;
    case PAD_R:
	trot.vx=v;
	flag=1;
	break;
    }
    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
	trot.vy=-v;
	flag=1;
	break;
    case PAD_D:
	trot.vy=v;
	flag=1;
	break;
    }
    switch(PreviewKey.auto_status & (PAD_X|PAD_Y)){
    case PAD_X:
	trot.vz=-v;
	flag=1;
	break;
    case PAD_Y:
	trot.vz=v;
	flag=1;
	break;
    }

    DG_SetPos2(&DG_ZeroVector,rot);
    DG_GetPos(&m);
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&m);
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)world,*(sceVu0FMATRIX *)&inv);

    sceVu0InversMatrix(*(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&m);

    DG_SetPos2(&DG_ZeroVector,&trot);
    DG_GetPos(&r);
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)&inv,*(sceVu0FMATRIX *)&(DG_Chanl(0)->eye));
    sceVu0MulMatrix(*(sceVu0FMATRIX *)&m,
		    *(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&r);

    EularYXZ2Rot(rot,&m);

    return flag;
}

int PreviewRotX(SVECTOR *rot,int v)
{
    int flag=0;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	rot->vx-=v;
	flag=1;
	break;
    case PAD_R:
	rot->vx+=v;
	flag=1;
	break;
    }
    return flag;
}

int PreviewRotY(SVECTOR *rot,int v)
{
    int flag=0;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	rot->vy-=v;
	flag=1;
	break;
    case PAD_R:
	rot->vy+=v;
	flag=1;
	break;
    }
    return flag;
}

int PreviewRotZ(SVECTOR *rot,int v)
{
    int flag=0;

    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_L:
	rot->vz-=v;
	flag=1;
	break;
    case PAD_R:
	rot->vz+=v;
	flag=1;
	break;
    }
    return flag;
}
