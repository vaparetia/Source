/*
   main.c : mdl2kc起動モジュール for linux

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 11/13/99

   $Id: cvdcut.c,v 1.1 1999/11/26 02:36:26 usr01363 Exp $

   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#define STRICT

#include <stdlib.h>
#include <string.h>
#include <SFDLINUX.h>
#include <METALGEAR.h>

#include "fmt_kms.h"
#include "fmt_km4.h"

#include "MDU_util.h"
#include "MDU_mdl.h"      
#include "MDU_tex.h"      

#include "option.h"


int  cv2out_flag;
char in[1024];
char out[1024];
int filename_count;


static void read_option(void)
{
    static char *opt[]={
	"-v",
	"-n",
	"-u",
	"-vu",
	"-nu",
    };
    union {
	char token[256];
	int i;
    } tmpd;

    cv2out_flag=0;
    filename_count=0;

    while(1){
	int tmp=option(opt,sizeof(opt)/sizeof(opt[0]),tmpd.token);

	if(tmp==-1) break;

	switch(tmp){
	case 0:
	    cv2out_flag|=COMMON_VERTS;
	    break;
	case 1:
	    cv2out_flag|=COMMON_NORMS;
	    break;
	case 2:
	    cv2out_flag|=COMMON_UVS;
	    break;
	case 3:
	    cv2out_flag|=COMMON_VERT_USRDATA;
	    break;
	case 4:
	    cv2out_flag|=COMMON_NORM_USRDATA;
	    break;
	default:
	    {
		char *str;
		int i;

		switch(filename_count){
		case 0:
		    str=in;
		    break;
		case 1:
		    str=out;
		    break;
		default:
		    continue;
		}

		for(i=0;tmpd.token[i]!='\0';i++) str[i]=tmpd.token[i];
		str[i]='\0';
		filename_count++;
	    }
	    break;
	}
    }
}


static	void	Usage( void )
{
    printf( "cvdcut : cvd data remover for linux\n" ) ;
    printf( "\t Usage : cvdcut [options] inputfile(*.cv2) [outputfile(*.cv2)]\n" ) ;
    printf( "\t\t -v  : remain vertexes\n" ) ;
    printf( "\t\t -n  : remain normals\n" ) ;
    printf( "\t\t -u  : remain uvs\n" ) ;
    printf( "\t\t -vu : remain vertex userdata\n" ) ;
    printf( "\t\t -nu : remain normal userdata\n" ) ;
    exit( -1 ) ;
}

int		main( argc, argv )
int		argc ;
char		**argv ;
{
    CV2_OBJS *cobjs ;
    CV2_OBJ *cobj;
    int i;

    opt_init(argc,argv);
    read_option();

    if(filename_count==0){
	Usage();
    }

    printf("Reading %s.\n",in);

    cobjs = MDU_LoadCv2( in ) ;

    if(cobjs==NULL) exit(0);

    for(i=0,cobj=cobjs->objs;i<cobjs->n_objs;i++,cobj++){
	if(!(cv2out_flag & COMMON_VERTS) && !(cv2out_flag & COMMON_VERT_USRDATA)){
	    printf("Cut vertexes.\n");
	    cobjs->flag &=~COMMON_VERTS;
	    cobj->verts = NULL ;
	    cobj->verts_index = NULL ;
	}
	if(!(cv2out_flag & COMMON_NORMS) && !(cv2out_flag & COMMON_NORM_USRDATA)){
	    printf("Cut normals.\n");
	    cobjs->flag &=~COMMON_NORMS;
	    cobj->norms = NULL ;
	    cobj->norms_index = NULL ;
	}
	if(!(cv2out_flag & COMMON_UVS)){
	    printf("Cut uvs.\n");
	    cobjs->flag &=~COMMON_UVS;
	    cobj->uvs = NULL ;
	    cobj->uvs_index = NULL ;
	}
	if(!(cv2out_flag & COMMON_VERT_USRDATA)){
	    printf("Cut vertex userdata.\n");
	    cobjs->flag &=~COMMON_VERT_USRDATA;
	    cobj->vert_usrdata = NULL ;
	}
	if(!(cv2out_flag & COMMON_NORM_USRDATA)){
	    printf("Cut normal userdata.\n");
	    cobjs->flag &=~COMMON_NORM_USRDATA;
	    cobj->norm_usrdata = NULL ;
	}
    }

    if(filename_count==1){
	printf("Writing %s.\n",in);
	MDU_SaveCv2( in, cobjs ) ;
    }
    else{
	printf("Writing %s.\n",out);
	MDU_SaveCv2( out, cobjs ) ;
    }

    exit(0);
}
