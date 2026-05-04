//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cotton.c
	ワタ

	2000/01/30 M.Matsuzaki
	$Id: cotton.c,v 1.1.1.3 2002/11/19 11:47:04 Yoshizawa1 Exp $

*/

/* -------------------------------------------------------------- */
/* include */
#define MATSUZAKI_SOURCE
#include "../../skoba/weapon_old/matsu.h"

/*----------------------------------------------------------------*/
/* define */

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

#define DELETE_TIME		(90)

#define M_PI2				((float)M_PI*2.0f)

/* DUSTワタ（落下） */
#define DUST_ON
#define DUST_WH				(30)
#define N_DUST_KIND			(9)
#define N_DUST_USEKIND		(9)
#define N_DUST_VERTS		(8)
#define N_DUST_ALLVERTS		(N_DUST_VERTS*N_DUST_USEKIND)
static int strcode_d[N_DUST_KIND]={
	4596910,5645486,6694062,7742638,4597678,5646254,6694830,7743406,8791982
};

#define N_ALLVERTS			(N_DUST_ALLVERTS)

#define	DUST_GRAVITY	(2.0f)
#define	RAISE_SHIFT		(110.0f)

/*----------------------------------------------------------------*/
/* struct */

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;
	int			name;
	
	DG_PRIM2	*prim_d[N_DUST_USEKIND];
	FVECTOR		speed_d[N_DUST_ALLVERTS];
	
	int			count ;
	
	FVECTOR		center;
	FVECTOR		direction;
} Work ;

/* ---------------------------------------------------------------- */
/* スクラッチパッドからメインメモリへ転送 */
static void Mz_MemCopy( void *dst, void *src, int size, int num ){
	UTL_StartSprToMem( dst, src, size * num / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}

/* メインメモリからスクラッチパッドへ転送 */
#if 0
static void Mz_MemCopy2( void *dst, void *src, int size, int num ){
	UTL_StartMemToSpr( dst, src, size * num / sizeof(u_long128) );
	UTL_EndMemToSpr() ;
}
#endif

/* ---------------------------------------------------------------- */
static void MZ_rand(int *p,int num){
	int i,j,k;
	for(i=0;i<num;i++)p[i]=i;
	for(i=0;i<num;i++){
		j=irnd()%num;
		k=p[i];p[i]=p[j];p[j]=k;
	}
}

/* ---------------------------------------------------------------- */
static short wh_sc[]={0,1,0,-1,0};

static void Act( Work *work ){
	FVECTOR				*pos ,*pos_b, *speed;
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	//FVECTOR			raise_shift;
	int					i,j,k,l;
	//float				*sinseed;
	
	short				count2;
	u_short				alpha;//,wh[4];
	
#ifdef DUST_ON
	
	speed = work->speed_d;
	i=0;
	
	/* a */
	if(work->count < 8)
		alpha = work->count*8;
	else if(work->count > (DELETE_TIME-15))
		alpha = (DELETE_TIME-work->count)*4;
	else
		alpha = 64;
	
	for(j=0;j<N_DUST_USEKIND;j++){
		prim = work->prim_d[j];
		GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
		DG_SwitchBuffPrim2( prim );
		
		pos = prim->pos[prim->buffer_clock];
		uvrgbwh = prim->uvrgb[prim->buffer_clock];
		pos_b = prim->pos[1-prim->buffer_clock];
		
		count2=work->count*2;
		for(k=0;k<N_DUST_VERTS;k++){
			l=i%4;
			speed->vx*=0.9f;
			speed->vz*=0.9f;
			if(speed->vy>0.0f){
				speed->vy-=DUST_GRAVITY;
			}else{
				speed->vy=-2.0f+(float)(l-2)*0.5f;
			}
			
			_sceVu0AddVector(pos,pos_b,speed);
			
			/* WH */
			if(work->count<16){
				uvrgbwh->w = wh_sc[l+1]*count2;
				//(short)(work->count*2)*cosf(((float)(l))*(float)M_PI/2.0f);
				uvrgbwh->h = wh_sc[l]*count2;
				//(short)(work->count*2)*sinf(((float)(l))*(float)M_PI/2.0f);
			}
			
			uvrgbwh->a=alpha;
			
			pos++;
			uvrgbwh++;
			pos_b++;
			speed++;
			
			i++;
		}
	}
	
#endif
	
	if(work->count++ >= DELETE_TIME){
		GV_DestroyActor( work ) ;
		//printf("** exit **\n");
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work ){
	int i;
	for(i=0;i<N_DUST_USEKIND;i++)GM_FreePrim2(work->prim_d[i]);
	//for(i=0;i<N_SMOKE_USEKIND;i++)GM_FreePrim2(work->prim_s[i]);
}

/* ---------------------------------------------------------------- */
static int GetResources(Work *work,int name,int where,FVECTOR *p0,FVECTOR *p1){
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	FVECTOR				*pos ,*speed,tmp;
	int					j,k,c,rnd_buf[16] ;
	//FMATRIX				tmpmat;
    SVECTOR     	    local_rot ;
	float				f;
	
	work->map = where ;
	work->name = name ;
	work->center = *p0;
	work->direction = *p1;
	work->direction.vw=0.0f;
	_sceVu0Normalize(&work->direction,&work->direction);
	
	_sceVu0ScaleVector(&tmp,&work->direction,30.0f);
	_sceVu0AddVector(&work->center,&work->center,&tmp);
	
	work->count = 0;
	
#ifdef DUST_ON
	
	speed = work->speed_d;
	MZ_rand(rnd_buf,N_DUST_KIND);
	
#if 0
	for(j=0;j<N_DUST_KIND;j++){
		printf("* %d *\n",rnd_buf[j]);
	}
#endif
	
	for(j=0;j<N_DUST_USEKIND;j++){
		prim = GM_MakePrim2(DG_PRIM2_RSPRT|DG_PRIM2_TEX|//DG_PRIM2_FOG|
							DG_PRIM2_ALPHA, 1, N_DUST_VERTS );
		work->prim_d[j] = prim;
		tex = DG_GetTexture( strcode_d[rnd_buf[j]] );
		if(tex==NULL)return -1;
		DG_ConfigPrim2Tex( prim, tex );
		//prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		pos = MEM_ADDR1 ;
		uvrgbwh = MEM_ADDR2 ;
		
		for(k=0;k<N_DUST_VERTS;k++){
			_sceVu0CopyVector(pos,&work->center);
			_sceVu0CopyVector(speed,&work->direction);
			f=10.0f*rnd();f*=f;
			speed->vx*=f;speed->vy*=f;speed->vz*=f;
			local_rot.vz=irnd() % 768-384 ;
			local_rot.vx=irnd() % 768-384 ;
			local_rot.vy=irnd() % 768-384 ;
			DG_SetPos(&DG_UnitMatrix) ;
			DG_RotatePos(&local_rot) ;
			DG_RotVector(speed,speed,1) ;
			speed->vy*=0.5f;
			uvrgbwh->u0 = FTOI12( 0.0F*tex->u_scale+tex->u_offset );
			uvrgbwh->v0 = FTOI12( 0.0F*tex->v_scale+tex->v_offset );
			uvrgbwh->u1 = FTOI12( 1.0F*tex->u_scale+tex->u_offset );
			uvrgbwh->v1 = FTOI12( 1.0F*tex->v_scale+tex->v_offset );
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			c=irnd()%64;
 			uvrgbwh->r = 32 +c;
			uvrgbwh->g = 32 +c;
			uvrgbwh->b = 32 +c;
			uvrgbwh->a = 0 ;
			
			pos++;
			uvrgbwh++;
			speed++;
		}
		Mz_MemCopy(prim->pos[0],MEM_ADDR1,sizeof(FVECTOR),N_DUST_VERTS);
		Mz_MemCopy(prim->uvrgb[0],MEM_ADDR2,
				   sizeof(DG_PRIM2_UVRGBWH),N_DUST_VERTS);
		Mz_MemCopy(prim->pos[1],MEM_ADDR1,sizeof(FVECTOR),N_DUST_VERTS);
		Mz_MemCopy(prim->uvrgb[1],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),
				   N_DUST_VERTS);
	}
#endif
	
	GM_SeSetMode( SD_A_RICSOF_C, &work->center, GM_SEMODE_REAL ) ;
//printf("&&& \n");
	
	return (0);
}


/* ---------------------------------------------------------------- */
void *NewMzCotton( int name, int where ,FVECTOR *pos,FVECTOR *pdir){
	Work		*work ;
	
	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where,pos,pdir ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	
	return (void *)work ;
}

