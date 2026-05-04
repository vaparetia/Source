#ifndef _VR_H_
#define _VR_H_


#define WAVE_TEX 4962
#define WAVE_BRK_TEX  42568/*fog*/
/*48773/*lqd*/




enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;
/*
typedef struct mdl_data_t
{
    short vtx[COLUMN*COLUMN*2*4] ;
    u_long64 :0 ;
    short nrm[(COLUMN*COLUMN+COLUMN*COLUMN*2/60+2)*2*4] ;
    u_long64 :0 ;
    short uvs[COLUMN*COLUMN*2*4] ;
    u_long64 :0 ;
    short rand[COLUMN][COLUMN] ;
    short pad[23] ;
} MDL ;

typedef struct scrach_pad_t
{
    MDL   mdl[WAVE_SHAPE] ;

    short map[2][COLUMN][COLUMN] ;

    short pad[6] ;
} ScrPad ;
*/

typedef	struct work_t
{
    GV_ACT actor;

    DG_PRIM *prim[2] ;
    FVECTOR pos[2] ;
    float   size[2][XY] ;
    float   vel[2] ;
    int     alph[2] ;
} Work ;



#ifdef _MAIN_FILE_

#else

#endif /*_MAIN_FILE_*/

#endif  /*_VR_H_*/

