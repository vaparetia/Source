//----------------------------------------------------------------------------
// BP_Font.h
//----------------------------------------------------------------------------

#ifndef __BP_FONT
#define __BP_FONT

// BP_Renderer forward declarations.
struct _SBP_BufferedTexture;

//----------------------------------------------------------------------------

#define BP_FONT_SIZE_W BP_font_size_W()
#define BP_FONT_SIZE_H BP_font_size_H()

#define BP_RUBI_SIZE_W (BP_FONT_SIZE_W / 2)
#define BP_RUBI_SIZE_H (BP_FONT_SIZE_H / 2)

#define BP_DOWN_MARGINE	(BP_font_max_offset() + 1)

#define BP_FONT_BUFFER_HEIGHT( _line, _lskip ) ( (_line)*(BP_FONT_SIZE_H+(_lskip)) + BP_DOWN_MARGINE )
#define BP_FONT_BUFFER_WIDTH( _col, _cskip ) ( (_col)*(BP_FONT_SIZE_W+(_cskip) ) )

#define BP_FONT_CONVERT_ORIGINAL_X(x) ((x) * BP_FONT_SIZE_W / 24)
#define BP_FONT_CONVERT_ORIGINAL_Y(y) ((y) * BP_FONT_SIZE_H / 24)

#define BP_FONT_CONVERT_NEW_X(x) ((x) * 24 / BP_FONT_SIZE_W)
#define BP_FONT_CONVERT_NEW_Y(y) ((y) * 24 / BP_FONT_SIZE_H)

//----------------------------------------------------------------------------

#if BP_USE_NEW_FONT_SYSTEM()
   #define BP_GCL_LOOKUP_NEW_FONT_STRING(x) GCL_LookupBPStringByKPString(x)
#else
   #define BP_GCL_LOOKUP_NEW_FONT_STRING(x) x
#endif

//----------------------------------------------------------------------------

typedef struct _SBPFont
{
   unsigned int id;
   unsigned int fontSizeX;
   unsigned int fontSizeY;
   unsigned int maxCharacterDescent; //Determines the maximum offset that a character can go below it's pixel size bounding box.
   unsigned int characterTableOffset;
   unsigned int characterCount;
} SBPFont;

typedef struct BP_vraminfo BP_FONT_VRAMINFO;
typedef struct BP_drawinfo BP_FONT_DRAWINFO;
	 
typedef int ( *BP_font_draw_func )( BP_FONT_VRAMINFO *out, int x, int y, int code, int flag );
typedef int ( *BP_rubi_draw_func )( BP_FONT_DRAWINFO *out, int x, int y, unsigned char const *rubi );

struct BP_vraminfo
{
	unsigned char c_width;
	unsigned char c_height;
	unsigned char c_skip;
	unsigned char l_skip;

	unsigned int colorRGBA;
	unsigned char flag;
	unsigned char rubi_display_flag;
	unsigned char y_step;

	short width;
   short pitch;
	short height;
	short max_width;

	unsigned char *vramtop;

   unsigned int bp_texture;

   int max_lines;

   struct _SBP_BufferedTexture* pBufferedTexture;
};

struct BP_drawinfo 
{
	BP_FONT_VRAMINFO *outinfo;
	int xtop;
	int ytop;
	unsigned int colorRGBA;

	int put_Tdot_flag;
	int rubi_left_x;
	int rubi_left_y;
	int rubi_orikaeshi_xmax;

	int block_flag;
	int rubi_flag;
	int cr_flag;

	char *(*callback)( BP_FONT_DRAWINFO *dr, char *now, int no );
	short now_x;
	short now_y;
	int yc;

	char *prev_char;
};

#define FONT_NO_ORIKAESHI		0x01
#define FONT_NO_KINSOKU			0x02
#define FONT_NO_META_CHARA		0x04
#define FONT_WORD_WRAP			0x08
#define FONT_NO_NEW_LINE_SPACE	0x10

#define FONT_RESULT_OVER		0x01
#define FONT_RESULT_ORIKAESHI	0x02

#define FONT_RGB( r, g, b )		(((r)<<0)|((g)<<8)|((b)<<16)|0x80000000)

#define BP_FONT_OUTLINED_DRAW_COUNT 25

#define BP_FONT_PLATFORM_PS3     0
#define BP_FONT_PLATFORM_PS3_JP  1
#define BP_FONT_PLATFORM_X360    2
#define BP_FONT_PLATFORM_VITA    3

//----------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C"
{
#endif

//----------------------------------------------------------------------------

void BP_font_init_resource(int reverseOkCancel);

void BP_font_resident_load_set( char *fonttop );

int BP_font_size_W();
int BP_font_size_H();
int BP_font_max_offset();

void BP_font_set_vraminfo_direct(BP_FONT_VRAMINFO *vinfo, void *vramtop, int width, int pitch, int height, int c_skip, int l_skip, int flag);
void BP_font_set_vraminfo_texture(BP_FONT_VRAMINFO *vinfo, unsigned int bp_texture0, int c_skip, int l_skip, int flag);
void BP_font_set_vraminfo_texture_buffered(BP_FONT_VRAMINFO *vinfo, struct _SBP_BufferedTexture* pBT, int c_skip, int l_skip, int flag);

void BP_font_begin_render_texture(BP_FONT_VRAMINFO *vinfo, int doClear);
void BP_font_begin_render_texture_internal(BP_FONT_VRAMINFO *vinfo, int doClear);
void BP_font_begin_render_texture_buffered_internal(BP_FONT_VRAMINFO *vinfo, int doClear);
void BP_font_end_render_texture(BP_FONT_VRAMINFO *vinfo);
void BP_font_end_render_texture_internal(BP_FONT_VRAMINFO *vinfo);
void BP_font_end_render_texture_buffered_internal(BP_FONT_VRAMINFO *vinfo);
void BP_font_clear_texture(BP_FONT_VRAMINFO *vinfo);
void BP_font_clear_texture_range(BP_FONT_VRAMINFO *vinfo, int y0, int y1);

void BP_font_set_clut4( unsigned int *clut_buf, int no, int fg, int bg );

int BP_font_draw_string( BP_FONT_DRAWINFO *work, char const *string );
void BP_font_open_drawinfo( BP_FONT_DRAWINFO *draw, BP_FONT_VRAMINFO *vram );
void BP_font_set_locate( BP_FONT_DRAWINFO *draw, int x, int y );
void BP_font_set_color( BP_FONT_DRAWINFO *draw, int r, int g, int b, int a);
void BP_font_set_refnum( int refnum );

void BP_font_set_platform(int platform );

unsigned char const * BP_font_decode_utf8_character( int *code, unsigned char const *m );

static inline int BP_font_get_draw_width( BP_FONT_DRAWINFO *draw )
{
	return draw->outinfo->max_width;
}

static inline void BP_font_set_draw_callback( BP_FONT_DRAWINFO *draw, char *( *callback )( BP_FONT_DRAWINFO *dr, char *now, int no ) )
{
   draw->callback = callback;
}

static inline unsigned int BP_MakeFontColor(int r, int g, int b, int a)
{
   return a << 24 | r << 16 | g << 8 | b;
}

int BP_GetOutlineOffsets(int renderPass, int* x, int* y);

//----------------------------------------------------------------------------

#if defined(__cplusplus)
};
#endif

//----------------------------------------------------------------------------

#endif //__BP_FONT
