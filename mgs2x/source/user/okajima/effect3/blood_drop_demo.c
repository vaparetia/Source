//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_drop_demo.c
	落下血：デモバージョン

	2001/06/15 S.Okajima
	$Id: blood_drop_demo.c,v 1.4 2002/11/23 12:16:42 Yoshizawa1 Exp $
*/
///asm

#ifdef PSX2	///
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif	///
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include	"libutl.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"
#include	"../effect/blood.h"

#define	COL_R	(12)
#define	COL_G	(0)
#define	COL_B	(0)

#ifdef ENGLISH
#define	COL_A	(128)
#else
#define	COL_A	(BLOOD_ALPHA_1)
#endif

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_VEC		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(16)
#define N_PRIMS		(0x1000 / 16 / N_VERTS)


#define	TOTAL_VERTS	(N_PRIMS*N_VERTS)
#define N_MUL		(8)
#define CYCLE_MAX	(TOTAL_VERTS/N_MUL)


#define	N_VERTS_STRIP	(64)
#define	N_PRIMS_STRIP	(N_PRIMS/2)
#define	N_STRIP			(N_PRIMS_STRIP*N_VERTS_STRIP/2-1)


/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;
	int			put_flag;
	int			cnt_count;
	int			cnt_count_max;

	int			cycle;
	int			size;
	float		intense;

	FMATRIX		*world;
	FMATRIX		before_world;

	FVECTOR		shift;
	FVECTOR		vec[TOTAL_VERTS];
	DG_PRIM2	*prim ;

} Work ;

static ALIGN16_PRE float ALIGN16_POST OK_s_sin[N_MUL];

/* ---------------------------------------------------------------- */
enum {
	REQ_PUT=0,
	REQ_CONT,
	REQ_KILL,
	REQ_NO
};

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_PUT:
			work->put_flag = 1;
			work->cycle = 0;
			break;
		  case REQ_CONT:
			work->cnt_count     = 0;
			work->cnt_count_max = DIRECT_TICK( msg->message[1] );
			break;
		  case REQ_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			  break;
		}
		msg--;
	}
}


static inline void SetConst( void )
{

#ifdef BP_PSX2_ASM	///
	FVECTOR	fvtemp = {0.5f, 0.5f, 0.5f, 1.0f};
	asm volatile(
"lqc2		vf8,0x00(%0)": : "r"(&fvtemp) :"memory"
 );
#endif
}




#ifdef BP_PSX2_ASM	///
static inline void InterPos( FVECTOR *pos0, FVECTOR *pos1, FVECTOR *pos2 )
{
	asm volatile("
		lqc2		vf10,0x00(%0)
		lqc2		vf11,0x00(%1)
		lqc2		vf12,0x00(%2)
		vadd.xyz	vf11, vf11, vf12	
		vmul.xyz	vf11, vf11, vf8
		vadd.xyz	vf10, vf10, vf11
		vmul.xyz	vf10, vf10, vf8
		sqc2		vf10,0x00(%0)
		": : "r"(pos0),"r"(pos1),"r"(pos2) :"memory" );
}
#else
static inline void InterPos( FVECTOR *pos0, FVECTOR *pos1, FVECTOR *pos2 )
{
	FVECTOR	pos10,pos11,pos12;
	FVECTOR	fvtemp = {0.5f, 0.5f, 0.5f, 1.0f};
	pos10 = *pos0 ;						//lqc2		vf10,0x00(%0)
	pos11 = *pos1 ;						//lqc2		vf11,0x00(%1)
	pos12 = *pos2 ;						//lqc2		vf12,0x00(%2)
	_sceVu0AddVector(&pos11,&pos11,&pos12) ;	//vadd.xyz	vf11, vf11, vf12
	_sceVu0MulVector(&pos11,&pos11,&fvtemp) ;	//vmul.xyz	vf11, vf11, vf8
	_sceVu0AddVector(&pos10,&pos10,&pos11) ;	//vadd.xyz	vf10, vf10, vf11
	_sceVu0MulVector(&pos10,&pos10,&fvtemp) ;	//vmul.xyz	vf10, vf10, vf8
	*pos0 = pos11 ;						//sqc2		vf10,0x00(%0)
}
#endif

/*----------------------------------------------------------------*/
static void Act( Work *work )
{
	FVECTOR	*sc_pos;
	FVECTOR	*sc_pos1;
	FVECTOR	*sc_pos2;
	FVECTOR	*sc_vec;
	FVECTOR	center;
	FVECTOR	n_center;
	FVECTOR	diff;
	FVECTOR	force;
	FMATRIX	world;
	int	clock;
	int	num;
	int	i;
	int	alpha;
	float	*s_sin;
	DG_PRIM2			*prim ;
	FVECTOR		cam;
	SVECTOR		rot;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */

	CheckMesgParam( work );

	DG_COPY_MAT( &world, work->world );
	DG_SetPos( &world );
	DG_PutVector( &work->shift, (FVECTOR *)world.m[3], 1 );


	force.vx = 0.0f;
	force.vy = 0.0f;
	force.vz = work->intense / (float)N_MUL;
	DG_SetPos( &world );
	DG_RotVector( &force, &force, 1 );

	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );

	if( work->cnt_count_max > 0 ){
		work->cnt_count--;
		if( work->cnt_count <= 0){
			work->cnt_count = work->cnt_count_max;
			work->put_flag = 1;
			work->cycle = 0;
		}
	}


	DG_COPY_VEC( &center, (FVECTOR *)work->before_world.m[3] );
	DG_COPY_VEC( &n_center, (FVECTOR *)world.m[3] );
	_sceVu0SubVector( &diff, &n_center, &center );
	_sceVu0ScaleVector( &diff, &diff, 1.0f / (float)N_MUL );



//printf("%f %f %f \n",diff.vx,diff.vy,diff.vz);

	prim = work->prim;

	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim ) ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	OK_Mem_Scr( SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
	OK_Mem_Scr( SCR_VEC, work->vec,  sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;



	num = work->cycle*N_MUL;


	uvrgbwh0 = prim->uvrgb[  clock];
	uvrgbwh1 = prim->uvrgb[1-clock];
	sc_vec  = SCR_VEC;
	sc_pos  = SCR_POS;
	for ( i = 0 ; i < N_VERTS*N_PRIMS; i++ ){
		uvrgbwh0->a = uvrgbwh1->a;
		sc_vec->vy+= P_GRAVITY;
		sc_pos->vx+= sc_vec->vx;
		sc_pos->vy+= sc_vec->vy;
		sc_pos->vz+= sc_vec->vz;
		sc_pos++;
		sc_vec++;
		uvrgbwh0++;
		uvrgbwh1++;
	}

//printf("%d\n",work->put_flag);

//	if( work->put_flag ){
//	if( GV_Time%300>150 ){
	if( GV_PadData[ 0 ].status & PAD_X ){
		alpha = COL_A;
	}else{
		alpha = 0;
	}
	uvrgbwh0 = prim->uvrgb[  clock];
	uvrgbwh0+= num;
	sc_pos  = SCR_POS;
	sc_vec  = SCR_VEC;
	sc_pos += num;
	sc_vec += num;
	rot.vz = 0;
	s_sin = OK_s_sin;
	for ( i = 0 ; i < N_MUL  ; i++ ){
		uvrgbwh0->a = alpha;
		DG_COPY_VEC( sc_pos, &center );
		_sceVu0AddVector( &center, &center, &diff );

		rot.vx = ((irnd()>>8)&2047)-1024;
		rot.vy = (irnd()>>8)&4095;

		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_RotVector( &diff, sc_vec, 1 );
		_sceVu0ScaleVector( sc_vec, sc_vec, (*s_sin++) );
		_sceVu0AddVector( sc_vec, sc_vec, &diff );
		_sceVu0AddVector( sc_vec, sc_vec, &force );

		uvrgbwh0++;
		sc_pos++;
		sc_vec++;
	}



	SetConst();
	if( work->cycle < CYCLE_MAX-1 ){
		sc_pos  = SCR_POS;
		sc_pos += num+N_MUL;
		sc_pos1 = sc_pos;
		sc_pos2 = sc_pos;
		sc_pos += 1;
		sc_pos2+= 2;
		for ( i=num+N_MUL; i<N_VERTS*N_PRIMS-2; i++ ){
			InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
		}
		sc_pos2 = SCR_POS;
		InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
		sc_pos  = SCR_POS;
		InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
		sc_pos1 = SCR_POS;
		InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
	}else{
		sc_pos  = SCR_POS;
		sc_pos1 = sc_pos;
		sc_pos2 = sc_pos;
		sc_pos += 1;
		sc_pos2+= 2;
	}
	for ( i=0; i<num+N_MUL-3; i++ ){
		InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
	}

	OK_Scr_Mem( prim->pos[clock], SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( work->vec,        SCR_VEC, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;


	work->cycle++;
	if( work->cycle >= CYCLE_MAX ) work->cycle = 0;

	work->put_flag = 0;
	DG_COPY_MAT( &work->before_world, &world );
}

/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	work->prim       = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*center;
	FVECTOR				*pos;
	FVECTOR				*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i;

	center = SCR_POS;
	DG_COPY_VEC( center, (FVECTOR *)work->world->m[3] );
	_sceVu0AddVector( center, center, &work->shift );

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


	pos     = SCR_POS;
	vec     = SCR_VEC;
	uvrgbwh = SCR_UVR;
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		DG_COPY_VEC( pos, center );
//@		DG_COPY_VEC( vec, &DG_ZeroVector );
		vec->x = DG_ZeroVector.x;
		vec->y = DG_ZeroVector.y;
		vec->z = DG_ZeroVector.z;
		vec->w = DG_ZeroVector.w;
		uvrgbwh->w  = 
		uvrgbwh->h  = work->size;
		uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		uvrgbwh->r  = COL_R ;
		uvrgbwh->g  = COL_G ;
		uvrgbwh->b  = COL_B ;
		uvrgbwh->a  = COL_A ;

		vec++;
		pos++;
		uvrgbwh++ ;
	}

	OK_Scr_Mem( work->vec,      SCR_VEC, sizeof(FVECTOR),          N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( prim->pos[0],   SCR_POS, sizeof(FVECTOR),          N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( prim->pos[1],   SCR_POS, sizeof(FVECTOR),          N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[0], SCR_UVR, sizeof(DG_PRIM2_UVRGBWH), N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[1], SCR_UVR, sizeof(DG_PRIM2_UVRGBWH), N_VERTS*N_PRIMS ) ;

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			i;

	for( i=0; i<N_MUL; i++ ){
		OK_s_sin[i] = sinf( PI*(float)(i)/(float)(N_MUL) );
	}

//	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	work->put_flag = 0;
	work->cnt_count_max = 0;
	return 0 ;
}


void *NewBloodDropDemo( int name, FMATRIX *world, FVECTOR *shift, float intense, int size )
{
	Work		*work ;

	OPERATOR() ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->world = world;
		DG_COPY_MAT( &work->before_world, world );
		DG_COPY_VEC( &work->shift, shift );
		work->intense = intense;
		work->size = size;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
