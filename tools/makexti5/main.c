//------------------------------
//
//  MGS2X  DataConverter Tool
//
//		makexti5
//
//		2001/04/25	
//		NISINO Motoaki (X)
//
//------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my.h"

#include "MDU_util.h"
#include "MDU_tex.h"

#include "func.h"
#include "makexti.h"


#include "bmp.h"
#include "bmppos.h"
#include "file.h"

#include "dds.h"



static MDU_Tex *Texs[1024];
static int     N_Texs, Size;
static int dds_f=0;			//..ddsファイルを吐き出すかどうか
static int header_f=0;		//..ヘッダ部用ファイル吐き出し実験
static int compress_f=0;	//..DXTn圧縮をするかどうか


extern long bmp_max_width;
extern long bmp_max_height;

/* bmppos.c で設定されるテクスチャ配置情報 */
extern RECT_BMP_POS rect_bmp_pos[];
extern RECT_BMP_POS rect_arrange_pos[];


#if 0
typedef	struct	{
    u_int	dx, dy ;	
    u_int	w, h ;
    void	*data ;
} MDU_TexRect ;

typedef	struct	{
    u_char		name[ 32 ] ;
    u_int		id ;
    u_int		flag ;
    u_int		width, height ;
    u_int		n_colors ;
    MDU_TexRect		data ;
    MDU_TexRect		clut ;
} MDU_Tex ;
#endif


//..テクスチャファイルの形式
enum {
	TRI_FORMAT_BITMAP	= 0,
	TRI_FORMAT_DDS		= 1
};



/*
 * XBOXで使用しているヘッダ構造体(PS2と同じ)
 * 実機とあわせておく必要がある
 */
typedef struct _tri_fileheader {
	unsigned int tex_offset;    /* テクスチャデータ開始オフセット（ブロック単位） */
	unsigned int tex_size;      /* テクスチャデータサイズ（ブロック単位） */
	unsigned int clut_offset;   /* ＣＬＵＴデータ開始オフセット（ブロック単位） */
	unsigned int clut_size;     /* ＣＬＵＴデータサイズ（ブロック単位） */
	unsigned int n_textures;    /* テクスチャデータ数 */
	unsigned int compress_flag; /* 圧縮フラグ */
	unsigned int texel_addr;    /* テクセルデータへのポインタ（現在未使用） */
	unsigned int clut_addr;     /* ＣＬＵＴデータへのポインタ（現在未使用） */
} TRI_FILEHEADER;

/* テクスチャパラメータ(PS2とはサイズが違うので注意) */
typedef struct _dg_tex {
	float               u_offset;   /* 使用テクスチャの開始Ｕ座標 */
	float               v_offset;   /* 使用テクスチャの開始Ｖ座標 */
	float               u_scale;    /* 使用テクスチャのスケール補正値 */
	float               v_scale;    /* 使用テクスチャのスケール補正値 */
	unsigned int        tex_id;     /* テクスチャＩＤ */
	unsigned int        tri_id;     /* テクスチャアーカイバＩＤ */
	int					flag;       /* テクスチャフラグ(XBOXでは未使用) */
	int                 pad[1];     /* pad */
	int                 tex_trans[4*7];    /* DG_TEX_TRANS の大きさ */
} DG_TEX ;

typedef struct tagRGBA {
	unsigned char r, g, b, a;
} RGBA;




int Usage(void){

	printf("Usage:\n\txti -o [outputfile] -i [inputfile...] [-d] [-c DXT1|DXT2..]\n");
	exit(1);
	//return (0);
}

/* テクスチャ読み込み */
MDU_Tex *LoadTex(char *name)
{
    MDU_Tex *tex;
    char    *ext;
    u_long     size;
	
    size = MDU_GetFileSize(name);
    if (size < 0){
    	printf("ERROR: filesize=%d\n",size);
		return NULL;
	}
    tex = NULL;
    ext = MDU_GetExtension(name);


    if (!strcmp(ext, "bmp")) {
        tex = MDU_LoadBmp(name);  
    }
#if 0
     else if (!strcmp(ext, "tim")) {
        tex = MDU_LoadTim(name);
    } else if (!strcmp(ext, "pcx")) {
        tex = MDU_LoadPcx(name);
    } 
#endif
    else if (!strcmp(ext, "pic")) {
        tex = MDU_LoadPic(name);
        dprintx(tex);
    }else{
        printf("unknown extension : %s\n", ext);
        exit(1);
    }

    if (tex != NULL) {
		//printf("%s id %d\n", name, tex->id);
        //if ((tex->flag & MDU_TEX_FLAG_COLOR256) == Mode) {
		Size += size;
		Texs[N_Texs] = tex;
		N_Texs ++;
		//}
    }
    return tex;
}

/* Clut付きで読み込まれたファイルを32bitカラーに展開する */
void MakeFullColorTex(MDU_Tex *tex)
{
	int i, j;
	RGBA *pixel, *clut;

	if (tex->clut.w <= 0) { /* CLUT無し */
		printf("clut not found ... skip\n");
		return;
	}
	
#if 0
	/* CLUT表示 */
	clut = (RGBA *)tex->clut.data;
	for (i = 0; i < tex->n_colors; i ++) {
		PrintRGBA(clut);
		clut ++;
	}
#endif
	
	clut = (RGBA *)tex->clut.data;
	pixel = (RGBA *)tex->data.data;
	for (i = 0; i < tex->height; i ++) {
		for (j = 0; j < tex->width; j ++, pixel ++) {
			if (*(unsigned int *)pixel > 256) {
				printf("wrong pixel %08x\n", *(unsigned int *)pixel);
			} else {
				//printf("pixel %08x\n", *(unsigned int *)pixel);
				*pixel = clut[*(unsigned int *)pixel];
				/* アルファ値の補正 */
				if ( pixel->a > 127 ){
					pixel->a = 255 ;
				} else {
					pixel->a = pixel->a << 1 ;
				}
			}
		}
	}
}

/* bmp セーブ関係 */
static u_char *MemWrite32(void *buf, unsigned int val)
{
	unsigned char *src = (unsigned char *)&val;
	unsigned char *dst = (u_char*)buf;
	
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];

	return (u_char*)buf + 4;
}

static u_char *MemWrite16(void *buf, unsigned short val)
{
	unsigned char *src = (unsigned char *)&val;
	unsigned char *dst = (u_char*)buf;
	dst[0] = src[0];
	dst[1] = src[1];

	return (u_char*)buf + 2;
}

static u_char *MemWrite08(void *buf, unsigned char val)
{
	*(unsigned char *)buf = val;
	return (u_char*)buf + 1;
}

/* フルカラー用BMPヘッダを書き込む */
static u_char *SetBmpHeader(u_char *buf, int w, int h, int depth)
{
	u_char *p = buf;
	int file_size;
	int image_size;
	
	image_size = w * h * depth / 8;
	file_size = image_size + 54;//sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// BITMAPFILEHEADER
	p = MemWrite08(p, 'B');      // 固定
	p = MemWrite08(p, 'M');      // 固定
	p = MemWrite32(p, file_size); // ファイルサイズ
	p = MemWrite32(p, 0);        // 固定
	// ファイル先頭からイメージデータへのオフセットサイズ
	p = MemWrite32(p, 54/*sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)*/);

	// BITMAPINFOHEADER
	p = MemWrite32(p, 40/*sizeof(BITMAPINFOHEADER)*/); // 構造体サイズ
	p = MemWrite32(p, w);          // 幅
	p = MemWrite32(p, h);          // 高さ
	p = MemWrite16(p, 1);          // 固定
	p = MemWrite16(p, depth);      // 色深度(bit)
	p = MemWrite32(p, 0);          // 圧縮の種類
	p = MemWrite32(p, image_size); // イメージサイズ
	p = MemWrite32(p, 2834);       // 水平解像度(pixel/meter)
	p = MemWrite32(p, 2834);       // 垂直解像度(pixel/meter)
	p = MemWrite32(p, 0);          // カラーインデックス数
	p = MemWrite32(p, 0);          // 重要なカラーインデックス数

	return p;
}

static int GetBmpSize(int w, int h, int depth)
{
	return w * h * depth / 8 + 54;
}

/* フルカラーに変換された tex の内容を、depth に従ってBMP形式に変換する
 * ※BMPのパディングに未対応なので注意!
 */
u_char *MakeFullColorBmp(MDU_Tex *tex, int depth)
{
	u_char *top; u_char *p;  /* BMP作成用バッファ */
	int size = GetBmpSize(tex->width, tex->height, depth);
	int width, height, pad;
	
	top = (u_char *)malloc(size);
	if (top == NULL) {
		printf("%s, %d : malloc failed\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	memset(top, 0, size);

	width = tex->width;
	height = tex->height;

	/* 先頭にヘッダを書き込む */
	p = SetBmpHeader((u_char *)top, width, height, depth);

	switch (depth) {
	case 24:
	{
		int i, j;
		RGBA *pixel;

		/* パディング計算/未対応 */
		if ((width * 3) % 4) {
			pad = 4 - ((width * 3) % 4);
		} else {
			pad = 0;
		}

		for (i = 0; i < height; i ++) {
			pixel = (RGBA *)((u_char*)tex->data.data + sizeof(RGBA) * width * (height - i - 1));
			for (j = 0; j < width; j ++, pixel ++) {
				*(unsigned char *)p = pixel->r; p ++;
				*(unsigned char *)p = pixel->g; p ++;
				*(unsigned char *)p = pixel->b;	p ++;
			}
		}
	}
	break;	
	
	case 32:
	{
		int i;
		RGBA *pixel;
		for (i = 0; i < height; i ++) {
			pixel = (RGBA*)((u_char*)tex->data.data + sizeof(RGBA) * width * (height - i - 1));
			memcpy(p, pixel, width * sizeof(RGBA));
			p += width * sizeof(RGBA);
		}
	}
	break;

	default:
		/* 24bit, 32bit 以外には対応しない */
		printf("wrong depth %d\n", depth);
		exit(EXIT_FAILURE);
	}
	return top; /* BMPデータ先頭のアドレスを返す */
}


enum{
    D3DFMT_A8R8G8B8,
    D3DFMT_A1R5G5B5,
    D3DFMT_A4R4G4B4,
    D3DFMT_R8G8B8,
    D3DFMT_R5G6B5,
    D3DFMT_DXT1                 = MAKEFOURCC('D', 'X', 'T', '1'),
    D3DFMT_DXT2                 = MAKEFOURCC('D', 'X', 'T', '2'),
    D3DFMT_DXT3                 = MAKEFOURCC('D', 'X', 'T', '3'),
    D3DFMT_DXT4                 = MAKEFOURCC('D', 'X', 'T', '4'),
    D3DFMT_DXT5                 = MAKEFOURCC('D', 'X', 'T', '5'),
};


static void SetDDSHeader(char *buf,int w,int h,int fmt)
{
	DWORD dwMagic;
	DDS_HEADER *ddsh;
	int dwPitch;
	
	dwMagic = MAKEFOURCC('D','D','S',' ');
	memcpy(buf,&dwMagic,4);
	
	buf+=4;
	
	ddsh=(DDS_HEADER*)buf;
	
    memset(ddsh,0, sizeof(DDS_HEADER));
    ddsh->dwSize = sizeof(DDS_HEADER);	//..まあヘッダのサイズですか
    ddsh->dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE;
    ddsh->dwWidth = w;	//..幅
    ddsh->dwHeight = h;	//..高さ
    ddsh->dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;
	
	//..ピッチとは？値は32bitのもので決めうち
	dwPitch=w*4;
	
    switch (fmt)
    {
#if 1
    case D3DFMT_DXT1:
        ddsh->ddspf = DDSPF_DXT1;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 8 ;
        break;
    case D3DFMT_DXT2:
        ddsh->ddspf = DDSPF_DXT2;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 4 ;
        break;
    case D3DFMT_DXT3:
        ddsh->ddspf = DDSPF_DXT3;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 4 ;
        break;
    case D3DFMT_DXT4:
        ddsh->ddspf = DDSPF_DXT4;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 4 ;
        break;
    case D3DFMT_DXT5:
        ddsh->ddspf = DDSPF_DXT5;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 4 ;
        break;
#endif
    case D3DFMT_A8R8G8B8:
        ddsh->ddspf = DDSPF_A8R8G8B8;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    case D3DFMT_A1R5G5B5:
        ddsh->ddspf = DDSPF_A1R5G5B5;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    case D3DFMT_A4R4G4B4:
        ddsh->ddspf = DDSPF_A4R4G4B4;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    case D3DFMT_R8G8B8:
        ddsh->ddspf = DDSPF_R8G8B8;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    case D3DFMT_R5G6B5:
        ddsh->ddspf = DDSPF_R5G6B5;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    default:
    	printf("ERROR : Invalid Format\n");
    	exit(1);
    }
}



int main(int argc,char **argv)
{
	char *out, *in, *dxtn=NULL;
	char outfile[ 256 ];
	void *pimg, *pcmpimg;
	int		compress_size ;
	
	//system("test_sys.exe");
	
	dds_f=0;
	
	MDU_ParseOption( argc, argv );


    if (MDU_GetOption('d') != NULL) {
		dds_f=TRUE;
		printf("DDS\n");
	}

    if (MDU_GetOption('h') != NULL) {
		header_f=TRUE;
		printf("header\n");
	}

    if (MDU_GetOption('o') == NULL) {
		Usage();
	}
	out = MDU_GetNextValue();
	
	strcpy(outfile,out);
	printf("%s\n",outfile);

    if (MDU_GetOption('c') != NULL) {
		compress_f=TRUE;
		printf("compress\n");
		dxtn = MDU_GetNextValue();
		printf("dxtn=%s\n",dxtn);
		
		if(dxtn==NULL){
			printf("圧縮形式も入力してください。\n");
			return(1);
		}
	}
	
	if (MDU_GetOption('i') == NULL) {
		Usage();
	}


	//..配置用ワーク初期化
	init_rect_arrange();
	
	//..入力ファイル読込み
	while ((in = MDU_GetNextValue()) != NULL) {
        MDU_Tex *tex;
		//void *p;
		printf("input:\t%s\n",in);
		
        tex = LoadTex(in);
		if (tex == NULL) {
			printf("file \"%s\" : LoadTex failed(main.cpp)\n", in);
			continue;
		}
		//..ALPHA
	    MDU_SetTransTexture( tex, in ) ;

		/* CLUT付きで読み込まれたテクスチャを32bitに変換する */
		MakeFullColorTex(tex);

		/* 32bit/または24bitのBMPに変換する */
		//p = MakeFullColorBmp(tex, 24);

		/* BMP結合ルーチン */
		{
			int mozi_code;
			char name[128];
			char *p;
			//..文字コード
			strcpy(name, in);
			p = strrchr(name, '.');
			*p = '\0';
			mozi_code=MDU_GetStrCode(name);
			input_bmppos(tex->width,tex->height, in, mozi_code,tex->flag);
		}
	}
	
	{
		//..テクスチャ配置・作成
		printf("*** MAKE ONE BMP ***\n");
		N_Texs=0;//..よくない処理だが・・
		main_rect_arrange();
		strcat(outfile,"_all.~bmp~");
		pimg=make_one_bmp32(outfile);
		CompressImageDXT3( pimg, bmp_max_width, bmp_max_height, &pcmpimg, &compress_size );
	}

	/* TRI情報書き出し。 */
	{
		int i;
		int w, h; /* 結合されたbmpの幅/高さ */
		FILE *fw;
		TRI_FILEHEADER *head;
		DG_TEX *tex;
		
		/* 結合されたBMPの幅/高さ */
#if 1
		//..2の乗数の場合
		w=bmp_max_width;
		h=bmp_max_height;
#else
		w = rect_arrange_pos[0].w;
		h = rect_arrange_pos[0].h;
#endif
		
		strcpy(outfile,out);
		strcat(outfile, ".xti");
		
		/* 書き出す xti ファイルをオープン */
		fw = fopen(outfile, "wb");

		if (fw == NULL) {
			printf("%s open failed..\n", outfile);
			return EXIT_FAILURE;
		}

		/* ヘッダ + DG_TEX * テクスチャ数 分のメモリを確保 */
		head = (TRI_FILEHEADER *)malloc(sizeof(TRI_FILEHEADER) + N_Texs * sizeof(DG_TEX));
		if (head == NULL) {
			printf("malloc failed\n");
			return EXIT_FAILURE;
		}
		memset(head, 0, sizeof(TRI_FILEHEADER) + N_Texs * sizeof(DG_TEX));

		/* ヘッダに情報設定 */
		head->tex_offset = sizeof(TRI_FILEHEADER) + N_Texs * sizeof(DG_TEX);
		
		//..テクスチャのサイズ設定、テクスチャファイル形式のフラグ設定
		if(dds_f==TRUE){
			head->tex_size = w*h*4+sizeof(DWORD)+sizeof(DDS_HEADER);//size;
			head->texel_addr = TRI_FORMAT_DDS;
		}else{
			head->tex_size = w*h*4+54;//size;
			head->texel_addr = TRI_FORMAT_BITMAP;
		}
		
		head->n_textures = N_Texs;
		head->compress_flag = 0; /* テクスチャフォーマットが変わったらこれを変更する */
		
		
		
		
		
		tex = (DG_TEX *)&head[1];

		/* outfile (出力されるxtiファイル名) から拡張子を抜いた文字列を作る */
		for (i = 0; i < N_Texs; i ++, tex ++) {
			/* ここにGV_StrCodeで XTI の名前を変換した値を入れる */
			tex->tri_id = MDU_GetStrCode(out);
			/* 各テクスチャのオフセット/幅/高さを計算する */
			tex->u_offset = ((float)rect_bmp_pos[i].x + 0.5f) / w;
			tex->v_offset = ((float)rect_bmp_pos[i].y + 0.5f) / h;
			tex->u_scale = ((float)rect_bmp_pos[i].org_w - 1.0f) / w;
			tex->v_scale = ((float)rect_bmp_pos[i].org_h - 1.0f) / h;
			/* テクスチャIDを入れておく */
			tex->tex_id = rect_bmp_pos[i].tex_id;
			
			//..alpha
			tex->flag=rect_bmp_pos[i].tex_flag;
			
		}
		

		if(dds_f==TRUE){
			char dds_file[64];
			char dds_header[sizeof(DWORD)+sizeof(DDS_HEADER)];
			//..ddsファイルを出力
			printf("Putout DDS.\n");
#if 0
			SetDDSHeader(dds_header,w,h,D3DFMT_A8R8G8B8);
			
			{
				//..A8R8G8B8/DDSファイル出力
				FILE *dfp;
				strcpy(dds_file,out);
				strcat(dds_file, ".dds");
				dfp = KsFileOpen(dds_file, "wb");
				//..DDSのヘッダは128byte
				fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),dfp);
				fwrite(pimg, 1, w*h*4, dfp);
				fclose(dfp);
			}
#else
			SetDDSHeader( dds_header, w, h, D3DFMT_DXT3 );
			
			{
				//..DX3/DDSファイル出力
				FILE *dfp;
				strcpy(dds_file,out);
				strcat(dds_file, ".dds");
				dfp = KsFileOpen(dds_file, "wb");
				//..DDSのヘッダは128byte
				fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),dfp);
				fwrite(pcmpimg, 1, compress_size, dfp);
				fclose(dfp);
			}
#endif

			
			//..圧縮 外部コマンド呼び出し
			if(compress_f){
#if 0
				char buffer[256];
				char compress_name[64];
				char h_file[64];
				long t_size;
				FILE *tmpfp;	FILE *hfp;
				
				
				sprintf(compress_name,"%s%s.dds",out,dxtn);
				sprintf(buffer,"xdxtex %s %s %s",dds_file,dxtn,compress_name);
				dprints(buffer);
				//..xdxtex 呼び出し
				system(buffer);
				
				tmpfp=KsFileOpen(compress_name, "rb");
				t_size=KsFileSize(tmpfp);
				
				dprint(t_size);
				
				//..xti　テクスチャのサイズ書き換え
				head->tex_size = t_size;
				
				//..ヘッダのみ出力
				{
				strcpy(h_file,out);
				strcat(h_file, ".hd");
				hfp = KsFileOpen(h_file, "wb");
				fwrite(head, 1, head->tex_offset, hfp);//..xtiヘッダ
				fclose(hfp);
				}
				
				//..合体 ここも外部コマンド呼び出し・・・。
				sprintf(buffer,"combine %s %s %s",
					h_file, compress_name,outfile);
				dprints(buffer);
				system(buffer);
#else
				//..DDS xtiファイル出力
				head->tex_size = compress_size+sizeof(DWORD)+sizeof(DDS_HEADER);//size;
				fwrite(head, 1, head->tex_offset, fw);		//..xtiヘッダ
				SetDDSHeader( dds_header, w, h, D3DFMT_DXT3 );
				fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),fw);//..DDSヘッダ
				fwrite(pcmpimg, 1, compress_size, fw);
				fclose(fw);
#endif


			}else{
				//..DDS xtiファイル出力
				fwrite(head, 1, head->tex_offset, fw);		//..xtiヘッダ
				fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),fw);//..DDSヘッダ
				fwrite(pimg, 1, w*h*4, fw);					//..イメージ
				fclose(fw);
			}
			
		}else{
			{	
				//..BMP XTIファイル出力
				u_char bmp_header[54];
				memset(bmp_header,0,54);
				//..xtiヘッダ
				fwrite(head, 1, head->tex_offset, fw);
				
				SetBmpHeader(bmp_header, w, h,32);
				fwrite(bmp_header,1,54,fw);
				fwrite(pimg, 1, w*h*4, fw);
				fclose(fw);
			}
		}
		
	}
	return EXIT_SUCCESS;


}

