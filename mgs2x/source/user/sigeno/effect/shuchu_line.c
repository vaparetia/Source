//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shuchu_line.c
	集中線
	2002/04/23 K.Sigeno
	$Id: shuchu_line.c,v 1.1.1.3 2002/11/19 11:49:49 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"libutl.h"
#include	"camera.h"


#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

//テクスチャ
#define	HOM_TAG01		(15045893)	/*lockon_tag1_add_alp*/
#define	HOM_NAME_OCT	(11899535)	/*lockon_octa_add_alp*/
//(A-B)*C>>7+D
#define UWAGAKI (SCE_GS_SET_ALPHA(2,2,0,0,0))
#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))
#define KASAN2 (SCE_GS_SET_ALPHA(0,2,2,1,64))
#define ALPHA_HALF (SCE_GS_SET_ALPHA(0,1,0,1,0x80))
#define GENSAN (SCE_GS_SET_ALPHA(2,0,0,1,0))
#define NEGA	(SCE_GS_SET_ALPHA(0,1,0,2,0))



//#define PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_TEX)
#define PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA)
#define POLY_FLAG	(DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_TEX)
//#define POLY_FLAG	(DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_TEX|DG_PRIM_SORTONLY)


#define CYCLE (8)
//#define ALPHA01	(32)
//#define ALPHA01	(16)
//#define ALPHA01	(24)
//#define ALPHA01	(18)
#define ALPHA01	(24)


//#define LINE_NUM  (16)
#define LINE_NUM  (8)
//#define LINE_NUM  (4)

#define N_VERTS		(2)

typedef	struct	{
	GV_ACT_EX	actor ;
	DG_PRIM2	*prim ;
	FVECTOR	pos ;

	int hankei ;
	int			cnt;

	float	height ;
	int		time ;
	int		line_num ;
	short		tag_tex_w;
	short		tag_tex_h ;

} Work ;

static float GetCameraDis(FVECTOR *pos)
{
	GM_CameraSet	*cam ;
	int	dis, i ;
	FVECTOR vec ;
	cam = GM_GetCurrentCamera( 0 ) ;
    _sceVu0SubVector( &vec, &cam->position, pos ) ;
	return GV_VecLen3F( &vec ) ;
}
static void GetCameraDirVec(FVECTOR *pos,FVECTOR *res)
{
	GM_CameraSet	*cam ;
	int	dis, i ;
	FVECTOR vec ;
	cam = GM_GetCurrentCamera( 0 ) ;
    _sceVu0SubVector( res, &cam->position, pos ) ;
}

static void InitPacket( Work *work){
	int		i,j,k;
	FVECTOR		*pos;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_TEX		*line_tex ;

	float	len1,len2,y_len1,y_len2;
	prim = work->prim ;
//	prim->world = *work->world ;


	/* テクスチャ取得 */
	line_tex = DG_GetTexture( 8617368 );
//	line_tex = DG_GetTexture( 9231374 ); //bonbori
	DG_ConfigPrim2Tex( work->prim, line_tex );

	DG_InvisiblePrim2(work->prim) ;
	DG_SetPrim2Alpha( prim, KASAN );
//	DG_SetPrim2Alpha( prim, GENSAN ) ;
//	DG_SetPrim2Alpha( prim, NEGA ) ;
	for(i= 0 ;i<2;i++){
		for (k= 0 ; k<work->line_num ;k++){
			uvrgb = (DG_PRIM2_UVRGB *)prim->uvrgb[i] ;
			uvrgb += k*N_VERTS ;
			pos = prim->pos[i] ;
			pos += k*N_VERTS ;
			for (j= 0 ; j<N_VERTS ;j++){
				/*線分は１頂点ごとに描画*/
				{
					if ( (j&1) == 0 ){
						uvrgb->f = 0x8fff ;	/* 描画キックしない */
#if 0
						uvrgb->r  = 0;
						uvrgb->g  = 0;
						uvrgb->b  = 127;
						uvrgb->a  = 0;
#else
						uvrgb->r  = 100;
						uvrgb->g  = 100;
						uvrgb->b  = 127;
						uvrgb->a  = ALPHA01;
#endif
					} else {
						uvrgb->f = 0x0fff ;	/* 描画キックを行う */
#if 0
						uvrgb->r  = 100;
						uvrgb->g  = 100;
						uvrgb->b  = 127;
						uvrgb->a  = ALPHA01;
#else
						uvrgb->r  = 0;
						uvrgb->g  = 0;
						uvrgb->b  = 127;
						uvrgb->a  = 0;
#endif
					}

					uvrgb->q = 4096 ;
					/*UV値*/
					uvrgb->u = FTOI12( 0.0F * line_tex->u_scale + line_tex->u_offset ) ;
					uvrgb->v = FTOI12( 0.0F * line_tex->v_scale + line_tex->v_offset ) ;
				}
				uvrgb++ ;
			}
			for (j= 0 ; j<(N_VERTS) ;j++){
				*pos = DG_ZeroVector ;
				pos->vw = 1.0f ;
				pos++;
			}
		}
	}
}

static void InitPos( Work *work){
	DG_PRIM2_UVRGB	*uvrgb;
	int		i,k,j;
	FVECTOR		*pos;
	int			hankei,alpha ;
	DG_PRIM2	*prim ;
	FVECTOR	tmp ;
	float len1 ,len2 , y_len1, y_len2 , line_dis01;
	int rot ;
	DG_SetPos2( &work->pos, &DG_ZeroSVector) ;
	DG_GetPos(&work->prim->as_world ) ;


	prim = work->prim ;
	if(work->cnt>=(work->time/2)){
		alpha = ALPHA01*(work->time - work->cnt)/(work->time/2) ;
	}else {
		alpha = ALPHA01*(work->cnt)/(work->time/2) ;
	}
//	for(i= 0 ;i<2;i++){
	i= work->prim->buffer_clock ; {
		for (k= 0 ; k<work->line_num ;k++){
			pos = prim->pos[i] ;
			pos += k*N_VERTS ;
			uvrgb = (DG_PRIM2_UVRGB *)prim->uvrgb[i] ;
			uvrgb += k*N_VERTS ;

//			for (j= 0 ; j<N_VERTS ;j++){
			j = 0 ;
			{
				rot = (irnd()>>8)&4095 ;
				hankei = (irnd()>>8)% work->hankei ;
//				((float)M_PI*2.0f)*(float)rot/4095.0f;

//放射線
				pos->vx = tmp.vx = (hankei)*cosf(((float)M_PI*2.0f)*(float)rot/4095.0f) ;
				pos->vy = 0.0f ;
				pos->vz = tmp.vz =(hankei)*sinf(((float)M_PI*2.0f)*(float)rot/4095.0f) ;


				pos++;

				pos->vx = tmp.vx * 4.0f ;
				pos->vy = 0.0f;
				pos->vz = tmp.vz * 4.0f;


				if ( (j&1) == 0 ){
//					uvrgb->a  = ALPHA01*work->cnt / work->time ;
					uvrgb->a  = alpha ;
				} else {
				}
				uvrgb++ ;
			}
		}
	}
}
static void MovePos( Work *work,float speed){
	int		i,k,j;
	FVECTOR		*pos;
	int			hankei ;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb;

	FVECTOR	tmp ;
	float len1 ,len2 , y_len1, y_len2 , line_dis01;
	int rot ;
	DG_SetPos2( &work->pos, &DG_ZeroSVector) ;
	DG_GetPos(&work->prim->as_world ) ;


	prim = work->prim ;

//	for(i= 0 ;i<2;i++){
	i= work->prim->buffer_clock ; {
		for (k= 0 ; k<work->line_num ;k++){
			pos = prim->pos[i] ;
			pos += k*N_VERTS ;

			uvrgb = (DG_PRIM2_UVRGB *)prim->uvrgb[i] ;
			uvrgb += k*N_VERTS ;
			for (j= 0 ; j<N_VERTS ;j++){
				/*線分は１頂点ごとに描画*/
				pos->vy += speed ;
				if ( (j&1) == 0 ){
				} else {
					uvrgb->a  = ALPHA01*work->cnt / work->time ;
				}
				pos++ ;
				uvrgb++ ;
			}
		}
	}
}

static	void	Act( work )
Work		*work ;
{
	FVECTOR cam ,tmp;
	SVECTOR	rot ;

	DG_VisiblePrim2(work->prim) ;
//	MovePos(work,16.0f) ;
	InitPos(work) ;

	

	GetCameraDirVec(&work->pos,&cam ) ;
	rot.vx = 1024+GV_VecDir2X( &cam ) ;
	rot.vy = GV_VecDir2( &cam ) ;
	rot.vz = 0 ;

	tmp.vx = cam.vx*0.8f + work->pos.vx ;
	tmp.vy = cam.vy*0.8f + work->pos.vy ;
	tmp.vz = cam.vz*0.8f + work->pos.vz ;

	DG_SetPos2( &tmp, &rot) ;
	DG_GetPos(&work->prim->as_world ) ;

	work->cnt--;
	if(work->cnt<=0){
		GV_DestroyActor( work ) ;
		return ;
	}

}

static	void	Die( work )
Work		*work ;
{
	DG_DequeuePrim2( work->prim );
	DG_FreePrim2( work->prim );
}




/*----------------------------------------------------------------*/
static	int	GetResources( Work	*work ){


//	if(work->line_num <=0) work->line_num = 1;

//	work->prim = GM_MakePrim2( PRIM_FLAG, 1, N_VERTS );
	work->prim = GM_MakePrim2( PRIM_FLAG, work->line_num, N_VERTS );
   work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;

	InitPacket( work );
	InitPos(work) ;

	return 0 ;
}
void	*NewShuChu_Line( int l_num,FVECTOR *pos,int hankei ,float height,int time)
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->line_num = l_num ;
		work->pos = *pos ;
		work->hankei = hankei ;
//		work->height = height ;
		work->cnt = work->time = DIRECT_TICK(time) ;
		if ( GetResources( work) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

