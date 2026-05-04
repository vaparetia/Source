/*
	pretex.c
		プレビュー用テクスチャ管理

	2000/2/12 K.Kano
	$Id: pretex.c,v 1.1.1.3 2002/11/19 11:43:31 Yoshizawa1 Exp $
*/


#include "preview_def.h"


/* テクスチャ情報の取得 */
static DG_TEX* OK_GetTexture( int num1, int num2 )
{
    DG_TEX	*tex ;

    tex = DG_TextureList[num1]->textures ;
    tex += num2;

    return tex;
}


#define _DG_PRIM2_XYZ(_xyz,_x,_y,_z) \
do{ \
	(_xyz)->vx=_x; \
	(_xyz)->vy=_y; \
	(_xyz)->vz=_z; \
}while(0)

#define _DG_PRIM2_RGBA(_uvrgb,_r,_g,_b,_a) \
do{ \
	(_uvrgb)->r=(_r); \
	(_uvrgb)->g=(_g); \
	(_uvrgb)->b=(_b); \
	(_uvrgb)->a=(_a); \
}while(0)

#define _DG_PRIM2_STQ0(_uvrgb,_s,_t,_q) \
do{ \
	(_uvrgb)->u0=(_s); \
	(_uvrgb)->v0=(_t); \
	(_uvrgb)->q0=(_q); \
}while(0)

#define _DG_PRIM2_STQ1(_uvrgb,_s,_t,_q) \
do{ \
	(_uvrgb)->u1=(_s); \
	(_uvrgb)->v1=(_t); \
	(_uvrgb)->q1=(_q); \
}while(0)

#define _DG_PRIM2_WH(_uvrgb,_w,_h) \
do{ \
	(_uvrgb)->w=(_w); \
	(_uvrgb)->h=(_h); \
}while(0)

void Pretex_InitPacket( int which )
{
	DG_PRIM2_PARAM *param;
    DG_PRIM2_PACKET *packet;
	FVECTOR *pos;
	DG_PRIM2_UVRGBWH *uvrgb;

    packet=PreviewTex.prim->packet[which];
	param=&(PreviewTex.prim->prim_param[which]);
	pos=packet->pos_addr;
	uvrgb=packet->uvrgb_addr;

    packet->sort_z = -100000 + 1;

    /* 半透明属性付加 */
    // param->giftag_normal.PRIM |= SCE_GS_PRIM_ABE ;

    /* プリミティブ描画設定初期化処理 */
    // DG_ConfigPrimInitTex( &packet->prim_init, PreviewTex.tex );
	DG_ConfigPrim2Tex(PreviewTex.prim,PreviewTex.tex);
    //	*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,2,0,1,0 ) ;

    /* プリミティブデータ部の初期化処理 */

#if 0
    DG_SET_RGBA1(uvrgb,128,128,128,64);
    DG_SET_STQ1(uvrgb,PreviewTex.tex->u_offset,PreviewTex.tex->v_offset,1.0F);
    DG_SET_STQ2(uvrgb,1.0F*PreviewTex.tex->u_scale+PreviewTex.tex->u_offset,
				1.0F*PreviewTex.tex->v_scale+PreviewTex.tex->v_offset,1.0F);
#endif

    _DG_PRIM2_RGBA(uvrgb,128,128,128,64);
    _DG_PRIM2_STQ0(uvrgb,PreviewTex.tex->u_offset,PreviewTex.tex->v_offset,1.0F);
    _DG_PRIM2_STQ1(uvrgb,1.0F*PreviewTex.tex->u_scale+PreviewTex.tex->u_offset,
				   1.0F*PreviewTex.tex->v_scale+PreviewTex.tex->v_offset,1.0F);


    //if(GV_Time%60==0)printf("%f\n",(float)(2 << ((PreviewTex.tex->tex0_base >> 26) & 15)) * PreviewTex.tex->u_scale);


#if 1
    PreviewTex.tex_width
		= (int)((float)(2 << ((PreviewTex.tex->tex_trans.tex0.data >> 26) & 15))
				* PreviewTex.tex->u_scale + 0.0f) ;
    PreviewTex.tex_height
		= (int)(((float)(2 << ((PreviewTex.tex->tex_trans.tex0.data >> 30) & 15))
				 * PreviewTex.tex->v_scale + 0.0f)/2.0f) ;
#else
//	PreviewTex.tex_width
//		= (int)((float)(2 << ((PreviewTex.tex->tex0_base >> 26) & 15)) * PreviewTex.tex->u_scale + 0.0f) ;
//	PreviewTex.tex_height
//		= (int)(((float)(2 << ((PreviewTex.tex->tex0_base >> 30) & 15)) * PreviewTex.tex->v_scale + 0.0f)/2.0f) ;
#endif

	_DG_PRIM2_XYZ(pos,(512-PreviewTex.tex_width/2)*16,(448-PreviewTex.tex_height/2)*16,0xfffffff);
	_DG_PRIM2_WH(uvrgb,PreviewTex.tex_width,PreviewTex.tex_height);

#if 0
    DG_SET_XYZF1(pos,(2048-PreviewTex.tex_width/2)*16,
				 (2048-PreviewTex.tex_height/2)*16,0xffffffff,255*16);
#endif

}

void Tex_DebugPrint(void)
{
    int	x,y;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( "TEXTURE MENU\n" );

    x=LOCATE_X;
    y=LOCATE_Y+COLUMN_HEIGHT+PreviewTex.mode*COLUMN_HEIGHT;
    DEBUG_Locate( x,   y, 0 );
    DEBUG_Printf( ">\n" );

    x=LOCATE_X+FONT_WIDTH;
    y=LOCATE_Y+COLUMN_HEIGHT;
    DEBUG_Locate( x , y, 0 );
    DEBUG_Printf( "TEX NUM : %4d    :%d x %d\n",
				  PreviewTex.tex->tex_id,PreviewTex.tex_width,PreviewTex.tex_height );
    DEBUG_Printf( "GROUP   : %4d[%4d]:%s\n",
				  PreviewTex.tex_list_num1,
				  DG_MaxTextures-1,
				  PreviewFile.tex_filenames[PreviewTex.tex_list_num1] );
    DEBUG_Printf( "NUMBER  : %4d[%4d]\n",
				  PreviewTex.tex_list_num2,
				  DG_TextureList[PreviewTex.tex_list_num1]->header->n_textures-1 );
}

void Tex_DebugCursole(void)
{
    if ( PreviewKey.auto_status & PAD_U ) PreviewTex.mode--;
    if ( PreviewKey.auto_status & PAD_D ) PreviewTex.mode++;
    if ( PreviewTex.mode >= 3 ) PreviewTex.mode = 0;
    if ( PreviewTex.mode <  0 ) PreviewTex.mode = 2;

    switch(PreviewTex.mode){
    case 0:
		if( PreviewKey.auto_status & PAD_L ){
			PreviewTex.tex_list_num2--;
		}
		else if( PreviewKey.auto_status & PAD_R ){
			PreviewTex.tex_list_num2++;
		}
		if( PreviewTex.tex_list_num2
			>= (int)DG_TextureList[PreviewTex.tex_list_num1]->header->n_textures ){

			PreviewTex.tex_list_num1++;
			if( PreviewTex.tex_list_num1 >= DG_MaxTextures ) PreviewTex.tex_list_num1=0;
			PreviewTex.tex_list_num2=0;
		}
		if( PreviewTex.tex_list_num2 < 0 ){
			PreviewTex.tex_list_num1--;
			if( PreviewTex.tex_list_num1 < 0 ) PreviewTex.tex_list_num1=DG_MaxTextures-1;
			PreviewTex.tex_list_num2
				=(int)DG_TextureList[PreviewTex.tex_list_num1]->header->n_textures-1;
		}
		break;
    case 1:
		if( PreviewKey.auto_status & PAD_L ){
			PreviewTex.tex_list_num1--;
			PreviewTex.tex_list_num2 = 0;
		}
		else if( PreviewKey.auto_status & PAD_R ){
			PreviewTex.tex_list_num1++;
			PreviewTex.tex_list_num2 = 0;
		}
		if( PreviewTex.tex_list_num1 >= DG_MaxTextures ) PreviewTex.tex_list_num1=0;
		if( PreviewTex.tex_list_num1 < 0 ) PreviewTex.tex_list_num1=DG_MaxTextures-1;
		break;
    case 2:
		if( PreviewKey.auto_status & PAD_L ){
			PreviewTex.tex_list_num2--;
		}
		else if( PreviewKey.auto_status & PAD_R ){
			PreviewTex.tex_list_num2++;
		}
		if( PreviewTex.tex_list_num2
			>= (int)DG_TextureList[PreviewTex.tex_list_num1]->header->n_textures ){

			PreviewTex.tex_list_num2=0;
		}
		if( PreviewTex.tex_list_num2 < 0 ){
			PreviewTex.tex_list_num2
				=(int)DG_TextureList[PreviewTex.tex_list_num1]->header->n_textures-1;
		}
		break;
    }

    if( PreviewKey.press & PAD_SEL ){
		PreviewTex.tex_list_num1=0;
		PreviewTex.tex_list_num2=0;
    }

    PreviewTex.tex=OK_GetTexture( PreviewTex.tex_list_num1, PreviewTex.tex_list_num2 );

    Pretex_InitPacket( DG_Clock );

}

void Preview_SetTexDefaultData(void)
{
    PreviewTex.tex_list_num1=0;
    PreviewTex.tex_list_num2=0;
    PreviewTex.tex = OK_GetTexture( PreviewTex.tex_list_num1, PreviewTex.tex_list_num2 );
}


int Preview_InitTex(void)
{
	DG_PRIM2 *prim;
	int i;

	Preview_SetTexDefaultData();

    /* ＴＥＸ用プリミティブ */
    prim=PreviewTex.prim=GM_MakePrim2(DG_PRIM2_SPRT|DG_PRIM2_ON_CAMERA|DG_PRIM2_TEX,1,1);

    if(prim==NULL){
		printf("null prim\n");
		return 0;
    }
    // prim->near_z = -100000;
    for ( i = 0 ; i < 2 ; i++ ){
		Pretex_InitPacket( i );
    }
    DG_InvisiblePrim2( PreviewTex.prim ) ;

	return 1;
}

void Preview_ExitTex(void)
{
	if(PreviewTex.prim!=NULL) GM_FreePrim2(PreviewTex.prim);
}
