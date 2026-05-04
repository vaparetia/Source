/*
   arcvd.c : cvdファイル内から必要なOBJのみ抽出する

   Programmed by K.Kano , 12/8/99

   $Id: arcvd.c,v 1.1 1999/12/08 11:32:42 usr01363 Exp $

   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

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


char in[1024];
char out[1024];


static char *opt[]={
    "-n %d",
    "-a",
    "-u %d",
    "-l %d",
    "-b %d",
};

static int get_outfilename(void)
{
    int tmp=option(opt,sizeof(opt)/sizeof(opt[0]),out);

    if(tmp==-1) return 0;

    if(tmp<sizeof(opt)/sizeof(opt[0])) return 0;

    return 1;
}

static int read_option(CV2_OBJS **def)
{
    CV2_OBJS *tdef;
    int flag=0;
    union {
	char token[256];
	int i;
    } tmpd;

    while(1){
	int objnum0,objnum1;
	int mov_flag=option(opt,sizeof(opt)/sizeof(opt[0]),tmpd.token);

	if(mov_flag==-1) break;

	switch(mov_flag){
	case 0:
	    if(flag==0){
		printf("Error : not get input filename yet.\n");
		continue;
	    }
	    flag=1;
	    objnum0=tmpd.i;
	    break;
	case 1:
	    if(flag==0){
		printf("Error : not get input filename yet.\n");
		continue;
	    }
	    flag=1;
	    objnum0=0;
	    objnum1=tdef->n_objs-1;
	    break;
	case 2:
	    if(flag==0){
		printf("Error : not get input filename yet.\n");
		continue;
	    }
	    flag=1;
	    objnum0=tmpd.i;
	    objnum1=tdef->n_objs-1;
	    break;
	case 3:
	    if(flag==0){
		printf("Error : not get input filename yet.\n");
		continue;
	    }
	    flag=1;
	    objnum0=0;
	    objnum1=tmpd.i;
	    break;
	case 4:
	    if(flag==0){
		printf("Error : not get input filename yet.\n");
		continue;
	    }
	    flag=2;
	    objnum0=tmpd.i;
	    continue;
	default:
	    if(flag==2){
		objnum1=atoi(tmpd.token);
		mov_flag=4;
		flag=1;
	    }
	    else{
		int i;
		for(i=0;tmpd.token[i]!='\0';i++) in[i]=tmpd.token[i];
		in[i]='\0';

		printf("Reading %s.\n",in);
		if((tdef=MDU_LoadCv2(in))==NULL){
		    printf("Error : Failed to read %s.\n",in);
		    flag=0;
		}
		else{
		    flag=1;
		}
		continue;
	    }
	    break;
	}

	if(objnum0>objnum1 && tdef->n_objs<=objnum1){
	    printf("Error : CV2 does not have this(these) object(s).\n");
	    continue;
	}

	/* オブジェクトのコピー */
	switch(mov_flag){
	case 0:
	    if(*def==NULL){
		if((*def=malloc(sizeof(CV2_OBJS)+sizeof(CV2_OBJ)))==NULL){
		    printf("Memory Overflow\n");
		    return 0;
		}
		(*def)->n_objs=0;
		(*def)->flag=tdef->flag;
	    }
	    else{
		CV2_OBJS *sdef;
		if((sdef=realloc(*def,sizeof(CV2_OBJS)
				 +sizeof(CV2_OBJ)*((*def)->n_objs+1)))==NULL){

		    printf("Memory Overflow\n");
		    return 0;
		}
		*def=sdef;
		(*def)->flag&=tdef->flag;
	    }
	    memcpy(&((*def)->objs[(*def)->n_objs]),&(tdef->objs[objnum0]),sizeof(CV2_OBJ));
	    (*def)->n_objs++;
	    break;
	case 1:
	case 2:
	case 3:
	case 4:
	    {
		int sz;

		sz=objnum1-objnum0+1;

		if(*def==NULL){
		    if((*def=malloc(sizeof(CV2_OBJS)+sizeof(CV2_OBJ)*sz))==NULL){
			printf("Memory Overflow\n");
			return 0;
		    }
		    (*def)->n_objs=0;
		    (*def)->flag=tdef->flag;
		}
		else{
		    CV2_OBJS *sdef;
		    if((sdef=realloc(*def,sizeof(CV2_OBJS)
				     +sizeof(CV2_OBJ)*((*def)->n_objs+sz)))==NULL){
			
			printf("Memory Overflow\n");
			return 0;
		    }
		    *def=sdef;
		    (*def)->flag&=tdef->flag;
		}
		memcpy(&((*def)->objs[(*def)->n_objs]),&(tdef->objs[objnum0]),sizeof(CV2_OBJ)*sz);
		(*def)->n_objs+=sz;
	    }
	    break;
	}
    }

    return 1;
}

static	void	Usage( void )
{
    printf( "arcvd : Get CV2 objects you need from CV2s for linux\n" ) ;
    printf( "\t Usage : arcvd outputfile(*.cv2) [inputfile0(*.cv2) -n objnum0 -n objnum1 ...] [inputfile1(*.cv2) -n objnum0 -n objnum1 ...] ... \n" ) ;
    printf( "\t\t -n objnum          : Get one object\n" ) ;
    printf( "\t\t -a                 : Get all objects\n" ) ;
    printf( "\t\t -u objnum          : Get objects from objnum to end\n" ) ;
    printf( "\t\t -l objnum          : Get objects from 0 to objnum\n" ) ;
    printf( "\t\t -b objnum0 objnum1 : Get objects from objnum0 to objnum1\n" ) ;
    exit( -1 ) ;
}

int		main( argc, argv )
int		argc ;
char		**argv ;
{
    CV2_OBJS nuldef;
    CV2_OBJS *def=NULL;

    opt_init(argc,argv);
    if(!get_outfilename()){
	Usage();
    }

    if(!read_option(&def)) exit(-1);

    if(def==NULL){
	def=&nuldef;
	def->n_objs=0;
	def->flag=0;
    }
    def->id=705644; /* StrCode("Common Vertex Data ver1.1") */

    MDU_SaveCv2(out,def);

    exit(0);
}
