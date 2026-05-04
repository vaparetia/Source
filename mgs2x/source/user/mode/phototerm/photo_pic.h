#ifndef _photo_pic_h_
#define _photo_pic_h_

typedef ALIGN16_DECL(struct) TexTrans {
  DG_DMATAG   dmatag0;
  DG_GIFTAG   giftag0;
  DG_GSREG    gsregs[4];
  DG_GIFTAG   giftag1;
  DG_DMATAG   dmatag1;
} TexTrans ;

/*
 * 写真画像の DMA 転送用 tag
 */
typedef ALIGN16_DECL(struct) PictDMA{
  TexTrans     trans;     /* 画像をテクスチャとして転送 */

  DG_DMATAG   dmatag_x;
  /* 転送されたテクスチャを元に描画する部分 */
  DG_DMATAG   dmatag;
  struct _pict_gif {
    DG_GIFTAG   giftag0;
    DG_GSREG    texflush;
    DG_GSREG    alpha;

    DG_GIFTAG   giftag1;

    struct _pict_data {
      u_long64 tex0;

      u_long64 prim;
      u_long64 rgbq;

      u_long64 uv0;
      u_long64 xyz0;

      u_long64 uv1;
      u_long64 xyz1;
    } data;
  } gif;
} PictDMA ;


typedef struct PictDraw {
  PictDMA          packet[2];  /* 転送用パケット         */
  unsigned short * pict;       /* 実際の画像ビットマップ */
} PictDraw;

typedef void PIC_PTR;

PIC_PTR      * PIC_GetPicture(int num);
unsigned int * PIC_DecodePicture(unsigned int * buf, PIC_PTR * pic);
PictDraw     * PIC_SetPictDraw(PictDraw * draw,
			       unsigned short * shrink, unsigned int * buf);

#endif /* _photo_pic_h_ */
