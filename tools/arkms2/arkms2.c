/*
   arkms.c : kmsファイル内から必要なOBJのみ抽出する

   Programmed by K.Kano , 12/8/99

   $Id: arkms2.c,v 1.3 2002/04/17 11:46:38 usr04098 Exp $

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

static int read_option(KMS2_DEF **def)
{
    KMS2_DEF *tdef;
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
	    objnum1=tdef->n_x_models-1;
	    break;
	case 2:
	    if(flag==0){
		printf("Error : not get input filename yet.\n");
		continue;
	    }
	    flag=1;
	    objnum0=tmpd.i;
	    objnum1=tdef->n_x_models-1;
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
		if((tdef=MDU_LoadKms2(in))==NULL){
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

	if(objnum0>objnum1 && tdef->n_x_models<=objnum1){
	    printf("Error : KMS2 does not have this(these) object(s).\n");
	    continue;
	}

	/* オブジェクトのコピー */
	switch(mov_flag){
	case 0:
	    if(*def==NULL){
		if((*def=malloc(sizeof(KMS2_DEF)+sizeof(KMS2_MDL)))==NULL){
		    printf("Memory Overflow\n");
		    return 0;
		}
#ifdef _XBOX
		(*def)->data_format=MGS_MODEL_NORM|MGS_MODEL_FLAG_INDEX ;
#else
		(*def)->data_format=MGS_MODEL_NORM;
#endif
		(*def)->n_models=(*def)->n_x_models=0;
		(*def)->texture=0;
	    }
	    else{
		KMS2_DEF *sdef;
		if((sdef=realloc(*def,sizeof(KMS2_DEF)
				 +sizeof(KMS2_MDL)*((*def)->n_models+1)))==NULL){

		    printf("Memory Overflow\n");
		    return 0;
		}
		*def=sdef;
	    }
	    memcpy(&((*def)->models[(*def)->n_models]),&(tdef->models[objnum0]),sizeof(KMS2_MDL));
	    (*def)->n_models++;
	    (*def)->n_x_models++;
	    break;
	case 1:
	case 2:
	case 3:
	case 4:
	    {
		int sz;

		sz=objnum1-objnum0+1;

		if(*def==NULL){
		    if((*def=malloc(sizeof(KMS2_DEF)+sizeof(KMS2_MDL)*sz))==NULL){
			printf("Memory Overflow\n");
			return 0;
		    }
#ifdef _XBOX
		    (*def)->data_format=MGS_MODEL_NORM|MGS_MODEL_FLAG_INDEX ;
#else
		    (*def)->data_format=MGS_MODEL_NORM;
#endif
		    (*def)->n_models=(*def)->n_x_models=0;
		    (*def)->texture=0;
		}
		else{
		    KMS2_DEF *sdef;
		    if((sdef=realloc(*def,sizeof(KMS2_DEF)
				     +sizeof(KMS2_MDL)*((*def)->n_models+sz)))==NULL){
			
			printf("Memory Overflow\n");
			return 0;
		    }
		    *def=sdef;
		}
		memcpy(&((*def)->models[(*def)->n_models]),&(tdef->models[objnum0]),
		       sizeof(KMS2_MDL)*sz);
		(*def)->n_models+=sz;
		(*def)->n_x_models+=sz;
	    }
	    break;
	}
    }

    return 1;
}

static	void	Usage( void )
{
    printf( "arkms2 : Get objects you need from KMSs for linux\n" ) ;
    printf( "\t Usage : arkms2 outputfile(*.kms) [inputfile0(*.kms) -n objnum0 -n objnum1 ...] [inputfile1(*.kms) -n objnum0 -n objnum1 ...] ... \n" ) ;
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
    KMS2_DEF nuldef;
    KMS2_DEF *def=NULL;

    opt_init(argc,argv);
    if(!get_outfilename()){
	Usage();
    }

    if(!read_option(&def)) exit(-1);

    if(def==NULL){
	def=&nuldef;
#ifdef _XBOX
	def->data_format=MGS_MODEL_NORM|MGS_MODEL_FLAG_INDEX ;
#else
	def->data_format=MGS_MODEL_NORM;
#endif
	def->n_models=def->n_x_models=0;
	def->texture=0;
    }
    def->lx=0;
    def->ly=0;
    def->lz=0;
    def->ux=0;
    def->uy=0;
    def->uz=0;

    MDU_SaveKms2(out,def,0);

    exit(0);
}
