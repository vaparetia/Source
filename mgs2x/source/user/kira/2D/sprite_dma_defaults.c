#ifndef _sprite_dma_defaults_c_
#define _sprite_dma_defaults_c_

#include "sprite_dma.h"

/* #define SIZEOF_QWORD(_v)  (sizeof(_v) / sizeof(u_long128)) */
#if 0 //BP_PS2 def PSX2

#if 0 //BP_GCC
static spr_DrawPoint def_draw_point = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_point)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_point), 0)
    }
  }, /* dmatag */

  gif:
  {
    alpha:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(1, 0, 0, 0, 0, 1),
	regs: GS_REGS_AD
      },
      alpha:
      {
	reg: SCE_GS_ALPHA_1,
	data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
      }
    },
      
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 2),
      regs: 0x51
    }, /* giftag */
    data:
    {
      rgbq0: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      xyz0:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
    } /* data */
  } /* gif */
};

static spr_DrawLine def_draw_line = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_line)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_line), 0)
    }
  }, /* dmatag */

  gif:
  {
    alpha:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(1, 0, 0, 0, 0, 1),
	regs: GS_REGS_AD
      },
      alpha:
      {
	reg: SCE_GS_ALPHA_1,
	data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
      }
    },
      
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 4),
      regs: 0x5151
    }, /* giftag */
    data:
    {
      rgbq0: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      xyz0:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
      rgbq1: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      xyz1:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
    } /* data */
  } /* gif */
};

static spr_DrawLineStrip def_draw_lstrip = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_lstrip)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_lstrip), 0)
    }
  }, /* dmatag */

  gif:
  {
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 0, 1,
			   SCE_GS_SET_PRIM(SCE_GS_PRIM_LINESTRIP,
					   1, 0, 0, 1, 0, 1, 0, 0), 0, 1),
      regs: GS_REGS_AD
    }, /* giftag */
    data:
    {
      alpha:
      {
	reg: SCE_GS_ALPHA_1,
	data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
      }
    },
    giftag_ver:
    {
      tag: SCE_GIF_SET_TAG(0, 1, 0, 0, 1, 2),
      regs: 0x51
    }
  }
};

static spr_DrawBox def_draw_box = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_box)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_box), 0)
    }
  }, /* dmatag */

  gif:
  {
    alpha:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(1, 0, 0, 0, 0, 1),
	regs: GS_REGS_AD
      },
      alpha:
      {
	reg: SCE_GS_ALPHA_1,
	data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
      }
    },
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 7),
      regs: 0xf5555d1
    }, /* giftag */
    data:
    {
      rgbq: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      xyz0: SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
      xyz1: SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
      xyz2: SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
      xyz3: SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
      xyz4: SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
      dummy: 0
    } /* data */
  } /* gif */
};

static spr_DrawSprite def_draw_sprite = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_sprite)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_sprite), 0)
    }
  }, /* dmatag */

  gif:
  {
    attrib:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
			     SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
					     0,  /* IIP  */
					     1,  /* TME  */
					     0,  /* FGE  */
					     0,  /* ABE  */
					     0,  /* AA1  */
					     1,  /* FST  */
					     0,  /* CTXT */
					     0), /* FIX  */
			     0, 1),
	regs: GS_REGS_AD
      },
      data:
      {
	alpha:
	{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
	},
	pabe:
	{
	  reg: SCE_GS_PABE,
	  data: SCE_GS_SET_PABE(0)
	},
	tex2:
	{
	  reg: SCE_GS_TEX2_1,
	  data: 0
	},
#ifndef NO_TEX1
	tex1:
	{
	  reg: SCE_GS_TEX1_1,
	  data: 0
	}
#endif /* NO_TEX1 */
      }
    },
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 8),
      regs: 0x53153168
    }, /* giftag */
    data:
    {
      clamp: SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
      tex0:  0,

      rgbq0: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv0:   SCE_GS_SET_UV(0, 0),
      xyz0:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq1: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv1:   SCE_GS_SET_UV(SPR_FIXED(255), SPR_FIXED(255)),
      xyz1:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
    } /* data */
  } /* gif */
};

static spr_DrawZoom def_draw_zoom = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_sprite)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_sprite), 0)
    }
  }, /* dmatag */

  gif:
  {
    attrib:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
			     SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
					     0,  /* IIP  */
					     1,  /* TME  */
					     0,  /* FGE  */
					     0,  /* ABE  */
					     0,  /* AA1  */
					     1,  /* FST  */
					     0,  /* CTXT */
					     0), /* FIX  */
			     0, 1),
	regs: GS_REGS_AD
      },
      data:
      {
	alpha:
	{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
	},
	pabe:
	{
	  reg: SCE_GS_PABE,
	  data: SCE_GS_SET_PABE(0)
	},
	tex2:
	{
	  reg: SCE_GS_TEX2_1,
	  data: 0
	},
#ifndef NO_TEX1
	tex1:
	{
	  reg: SCE_GS_TEX1_1,
	  data: 0
	}
#endif /* NO_TEX1 */
      }
    },
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 8),
      regs: 0x53153168
    }, /* giftag */
    data:
    {
      clamp: SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
      tex0:  0,

      rgbq0: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv0:   SCE_GS_SET_UV(0, 0),
      xyz0:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq1: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv1:   SCE_GS_SET_UV(SPR_FIXED(255), SPR_FIXED(255)),
      xyz1:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
    } /* data */
  } /* gif */
};

static spr_DrawPoly def_draw_spin = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_spin)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_spin), 0)
    }
  }, /* dmatag */

  gif:
  {
    attrib:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
			     SCE_GS_SET_PRIM(SCE_GS_PRIM_TRIFAN,
					     0, 0, 0, 1, 0, 1, 0, 0), 0, 1),
	regs: GS_REGS_AD
      },
      data:
      {
	alpha:
	{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
	},
	pabe:
	{
	  reg: SCE_GS_PABE,
	  data: SCE_GS_SET_PABE(0)
	},
	tex2:
	{
	  reg: SCE_GS_TEX2_1,
	  data: 0
	},
#ifndef NO_TEX1
	tex1:
	{
	  reg: SCE_GS_TEX1_1,
	  data: 0
	}
#endif /* NO_TEX1 */
      }
    },
    
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 14),
      regs: 0x53153153153168
    }, /* giftag */
    data:
    {
      clamp: SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
      tex0:  0,

      rgbq0: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv0:   SCE_GS_SET_UV(0, 0),
      xyz0:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq1: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv1:   SCE_GS_SET_UV(0, 0),
      xyz1:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq2: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv2:   SCE_GS_SET_UV(0, 0),
      xyz2:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq3: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv3:   SCE_GS_SET_UV(0, 0),
      xyz3:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
    } /* data */
  } /* gif */
};

static spr_DrawPoly def_draw_poly = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_poly)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_poly), 0)
    }
  }, /* dmatag */

  gif:
  {
    attrib:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
			     SCE_GS_SET_PRIM(SCE_GS_PRIM_TRIFAN,
					     1, 0, 0, 1, 0, 1, 0, 0), 0, 1),
	regs: GS_REGS_AD
      },
      data:
      {
	alpha:
	{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
	},
	pabe:
	{
	  reg: SCE_GS_PABE,
	  data: SCE_GS_SET_PABE(0)
	},
	tex2:
	{
	  reg: SCE_GS_TEX2_1,
	  data: 0
	},
#ifndef NO_TEX1
	tex1:
	{
	  reg: SCE_GS_TEX1_1,
	  data: 0
	}
#endif /* NO_TEX1 */
      }
    },
    
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 14),
      regs: 0x53153153153168
    }, /* giftag */
    data:
    {
      clamp: SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
      tex0:  0,

      rgbq0: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv0:   SCE_GS_SET_UV(0, 0),
      xyz0:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq1: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv1:   SCE_GS_SET_UV(0, 0),
      xyz1:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq2: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv2:   SCE_GS_SET_UV(0, 0),
      xyz2:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq3: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv3:   SCE_GS_SET_UV(0, 0),
      xyz3:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
    } /* data */
  } /* gif */
};

static spr_DrawTriangle def_draw_triangle = {
  dmatag:
  {
    qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_triangle)),
    vifcode:
    {
      SCE_VIF1_SET_NOP(0),
      SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_triangle), 0)
    }
  }, /* dmatag */

  gif:
  {
    attrib:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
			     SCE_GS_SET_PRIM(SCE_GS_PRIM_TRI,
					     1, 0, 0, 1, 0, 1, 0, 0), 0, 1),
	regs: GS_REGS_AD
      },
      data:
      {
	alpha:
	{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(0, 2, 0, 1, 0)
	},
	pabe:
	{
	  reg: SCE_GS_PABE,
	  data: SCE_GS_SET_PABE(0)
	},
	tex2:
	{
	  reg: SCE_GS_TEX2_1,
	  data: 0
	},
#ifndef NO_TEX1
	tex1:
	{
	  reg: SCE_GS_TEX1_1,
	  data: 0
	}
#endif /* NO_TEX1 */
      }
    },
    
    giftag: 
    {
      tag: SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 11),
      regs: 0x53153153168
    }, /* giftag */
    data:
    {
      clamp: SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
      tex0:  0,

      rgbq0: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv0:   SCE_GS_SET_UV(0, 0),
      xyz0:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq1: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv1:   SCE_GS_SET_UV(0, 0),
      xyz1:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

      rgbq2: SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
      uv2:   SCE_GS_SET_UV(0, 0),
      xyz2:  SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

    } /* data */
  } /* gif */
};

#endif //BP_GCC

#else

/* XBOX・p・_・~・[ */
static spr_DrawPoint def_draw_point = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_point)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_point), 0)
		}
	},
	{/* gif */
		{/* alpha */
			{ SCE_GIF_SET_TAG(1, 0, 0, 0, 0, 1),GS_REGS_AD },
			{ SCE_GS_SET_ALPHA(0, 2, 0, 1, 0), SCE_GS_ALPHA_1 }
		},
		{/* giftag */
			SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 2),
			0x51
		},
		{/* data */
			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
		}
	}
};

static spr_DrawLine def_draw_line = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_line)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_line), 0)
		}
	},
	{/* gif */
		{/* alpha */
			{ SCE_GIF_SET_TAG(1, 0, 0, 0, 0, 1),GS_REGS_AD },
			{ SCE_GS_SET_ALPHA(0, 2, 0, 1, 0), SCE_GS_ALPHA_1 }
		},
		{/* giftag */
			SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 4),
			0x5151
		},
		{/* data */
			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
		}
	}
};

static spr_DrawLineStrip def_draw_lstrip = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_lstrip)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_lstrip), 0)
		}
	},
	{/* gif */
		{/* giftag */
			SCE_GIF_SET_TAG(1, 0, 1,
							SCE_GS_SET_PRIM(SCE_GS_PRIM_LINESTRIP,
											1, 0, 0, 1, 0, 1, 0, 0), 0, 1),
			GS_REGS_AD
		},
		{/* data */
			{ SCE_GS_SET_ALPHA(0, 2, 0, 1, 0), SCE_GS_ALPHA_1 }
		},
		{/* giftag_ver */
			SCE_GIF_SET_TAG(0, 1, 0, 0, 1, 2),
			0x51
		}
	}
};

static spr_DrawBox def_draw_box = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_box)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_box), 0)
		}
	},
	{/* gif */
		{/* alpha */
			{ SCE_GIF_SET_TAG(1, 0, 0, 0, 0, 1),GS_REGS_AD },
			{ SCE_GS_SET_ALPHA(0, 2, 0, 1, 0), SCE_GS_ALPHA_1 }
		},
		{/* giftag */
			SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 7),
			0xf5555d1
		},
		{/* data */
			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
			0
		}
		
	}
};

static spr_DrawSprite def_draw_sprite = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_sprite)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_sprite), 0)
		}
	},
	{/* gif */
		{/* attrib */
			{/* giftag */
				SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
								SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
												0,  /* IIP  */
												1,  /* TME  */
												0,  /* FGE  */
												0,  /* ABE  */
												0,  /* AA1  */
												1,  /* FST  */
												0,  /* CTXT */
												0), /* FIX  */
								0, 1),
				GS_REGS_AD
			},
			{/* data */
				{SCE_GS_SET_ALPHA(0, 2, 0, 1, 0),SCE_GS_ALPHA_1},
				{SCE_GS_SET_PABE(0),SCE_GS_PABE},
				{0,SCE_GS_TEX2_1},
#ifndef NO_TEX1
				{0,SCE_GS_TEX1_1}
#endif /* NO_TEX1 */
			}
		},
		{/* giftag */
			SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 8),
			0x53153168
		},
		{/* data */
			SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
			0,

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(SPR_FIXED(255), SPR_FIXED(255)),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
		}
	}

};

static spr_DrawZoom def_draw_zoom = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_sprite)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_sprite), 0)
		}
	},
	{/* gif */
		{/* attrib */
			{/* giftag */
				SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
								SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
												0,  /* IIP  */
												1,  /* TME  */
												0,  /* FGE  */
												0,  /* ABE  */
												0,  /* AA1  */
												1,  /* FST  */
												0,  /* CTXT */
												0), /* FIX  */
								0, 1),
				GS_REGS_AD
			},
			{/* data */
				{SCE_GS_SET_ALPHA(0, 2, 0, 1, 0),SCE_GS_ALPHA_1},
				{SCE_GS_SET_PABE(0),SCE_GS_PABE},
				{0,SCE_GS_TEX2_1},
#ifndef NO_TEX1
				{0,SCE_GS_TEX1_1}
#endif /* NO_TEX1 */
			}
		},
		{/* giftag */
			SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 8),
			0x53153168
		},
		{/* data */
			SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
			0,

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(SPR_FIXED(255), SPR_FIXED(255)),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
		}
	}
};

static spr_DrawPoly def_draw_spin = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_spin)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_spin), 0)
		}
	},
	{/* gif */
		{/* attrib */
			{/* giftag */
				SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
			     SCE_GS_SET_PRIM(SCE_GS_PRIM_TRIFAN,
					     0, 0, 0, 1, 0, 1, 0, 0), 0, 1),
				GS_REGS_AD
			},
			{/* data */
				{SCE_GS_SET_ALPHA(0, 2, 0, 1, 0),SCE_GS_ALPHA_1},
				{SCE_GS_SET_PABE(0),SCE_GS_PABE},
				{0,SCE_GS_TEX2_1},
#ifndef NO_TEX1
				{0,SCE_GS_TEX1_1}
#endif /* NO_TEX1 */
			}
		},
		{/* giftag */
			SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 14),
			0x53153153153168
		},
		{/* data */
			SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
			0,

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
		}
	}

};

static spr_DrawPoly def_draw_poly = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_poly)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_poly), 0)
		}
	},
	{/* gif */
		{/* attrib */
			{/* giftag */
				SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
			     SCE_GS_SET_PRIM(SCE_GS_PRIM_TRIFAN,
					     1, 0, 0, 1, 0, 1, 0, 0), 0, 1),
				GS_REGS_AD
			},
			{/* data */
				{SCE_GS_SET_ALPHA(0, 2, 0, 1, 0),SCE_GS_ALPHA_1},
				{SCE_GS_SET_PABE(0),SCE_GS_PABE},
				{0,SCE_GS_TEX2_1},
#ifndef NO_TEX1
				{0,SCE_GS_TEX1_1}
#endif /* NO_TEX1 */
			}
		},
		{/* giftag */
			SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 14),
			0x53153153153168
		},
		{/* data */
			SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
			0,

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR)
		}
	}

};

static spr_DrawTriangle def_draw_triangle = {
	{/* DMATAG */
		DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_triangle)),
		NULL,
		{
			SCE_VIF1_SET_NOP(0),
			SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_triangle), 0)
		}
	},
	{/* gif */
		{/* attrib */
			{/* giftag */
				SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _txat_data), 0, 1,
			     SCE_GS_SET_PRIM(SCE_GS_PRIM_TRI,
					     1, 0, 0, 1, 0, 1, 0, 0), 0, 1),
				GS_REGS_AD
			},
			{/* data */
				{SCE_GS_SET_ALPHA(0, 2, 0, 1, 0),SCE_GS_ALPHA_1},
				{SCE_GS_SET_PABE(0),SCE_GS_PABE},
				{0,SCE_GS_TEX2_1},
#ifndef NO_TEX1
				{0,SCE_GS_TEX1_1}
#endif /* NO_TEX1 */
			}
		},
		{/* giftag */
			SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 11),
			0x53153153168
		},
		{/* data */
			SCE_GS_SET_CLAMP(1, 1, 0, 0, 0, 0),
			0,

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),

			SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0),
			SCE_GS_SET_UV(0, 0),
			SCE_GS_SET_XYZ(0, 0, SPR_MOST_NEAR),
		}
	}
};

#endif
#endif /* _sprite_dma_defaults_c_ */
