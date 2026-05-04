static char option_c_label[] = "@(#)option.c Ver1.00" ;

#include <stdio.h>
#include <stdlib.h>

#ifdef MSDOS_GCC

#include <dir.h>

#endif

int t_argc,t_argv;
int g_argv;
char **g_argc;

#ifdef MSDOS_GCC
int f_wild;
struct ffblk fblock;
#endif

static int ptnmatch(char *a,char *b)
{
    int i=0;

    while(*a==*b && *b!='\0'){
	a++;b++;i++;
    }
    if(*b=='\0'){
	switch(*a){
	case '\0':
	case '\n':
	case ' ':
	case '\t':
	case '&':
	    return(i);
	default:
	    return(0);
	}
    }
    else{
	switch(*a){
	case '&':
	case '%':
	    return(i);
	default:
	    return(0);
	}
    }
}

static int getarg(void)
{
    if(t_argv<g_argv){
	int i= *(g_argc[t_argv]+t_argc);
	if(i){
	    t_argc++;
	    return(i);
	}
	else{
	    t_argv++;
	    t_argc=0;
	    return(' ');
	}
    }
    else return('\0');
}

static void parsent(char *opt,char *arg,void *value)
{
    int ans=0;

    switch(*opt){
    case '&':
    case '%':
	opt++;
	break;
    default:
	return;
    }
    while(*opt==' ' || *opt=='\n' || *opt=='\t') opt++;
    switch(*opt){
    case 's':
    case 'S':
	{
	    int i=0;
	    while(*arg!='\0'){
		*((char *)value+i) = *arg;
		i++;arg++;
	    }
	    *((char *)value+i)='\0';
	}
	break;
    case 'd':
    case 'D':
	{
	    int i=0;
	    int sign=1;
	    if(*arg=='-') sign=-1;
	    else if(*arg=='+') arg++;
	    while(*arg>='0' && *arg<='9'){
		i=i*10+*arg-'0';
		arg++;
	    }
	    *((int *)value)=i*sign;
	}
	break;
    case 'x':
    case 'X':
	{
	    int i=0;
	    while(1){
		if(*arg>='0' && *arg<='9'){
		    i=i*16+*arg-'0';
		}
		else if(*arg>='A' && *arg<='F'){
		    i=i*16+*arg-'A'+10;
		}
		else if(*arg>='a' && *arg<='f'){
		    i=i*16+*arg-'a'+10;
		}
		else break;
		arg++;
	    }
	    *((int *)value)=i;
	}
	break;
    }
}

void opt_init(int argv,char *argc[])
{
    g_argc=argc;
    g_argv=argv;
    t_argv=1;
    t_argc=0;
#ifdef MSDOS_GCC
    f_wild=0;
#endif
}

int option(char *opt[],int size,void *value)
{
    int i,c;
    char tmpline[256];

#ifdef MSDOS_GCC
    if(f_wild){
	if(findnext(&fblock)){
	    int x=0;
	    while(fblock.ff_name[x]){
		*((char *)value+x)=fblock.ff_name[x];
		x++;
	    }
	    *((char *)value+x)='\0';
	    return(size);
	}
	else f_wild=0;
    }
#endif

    c=getarg();
    while(c=='\n' || c==' ' || c=='\t') c=getarg();
    i=0;
    if(c=='\0') return(-1);
    while(c!='\n' && c!=' ' && c!='\t' && c!='\0'){
	tmpline[i++]=c;
	c=getarg();
    }
    tmpline[i]='\0';
    for(i=0;i<size;i++){
	int k;
	k=ptnmatch(opt[i],tmpline);
	if(k){
	    if(*(opt[i]+k)==' ' || *(opt[i]+k)=='\n' || *(opt[i]+k)=='\t'){
		int j=0;
		while(c=='\n' || c==' ' || c=='\t') c=getarg();
		while(c!='\n' && c!=' ' && c!='\t' && c!='\0'){
		    tmpline[j++]=c;
		    c=getarg();
		}
		tmpline[j]='\0';
		while(*(opt[i]+k)=='\n' || *(opt[i]+k)==' ' || *(opt[i]+k)=='\t') k++;
		parsent(opt[i]+k,tmpline,value);
		break;
	    }
	    else{
		parsent(opt[i]+k,tmpline+k,value);
		break;
	    }
	}
    }
#ifdef MSDOS_GCC
    if(i==size){
	int x=0;
	while(tmpline[x]){
	    int i=tmpline[x];
	    if(i=='*' || i=='?'){
		if(findfirst(tmpline,&fblock,0xff)){
		    x=0;
		    f_wild=1;
		    while(fblock.ff_name[x]){
			*((char *)value+x)=fblock.ff_name[x];
			x++;
		    }
		    *((char *)value+x)='\0';
		    return(size);
		}
		else return(option(opt,size,value));
	    }
	    *((char *)value+x)=i;
	    x++;
	}
	*((char *)value+x)='\0';
    }
#else
    if(i==size){
	int x=0;
	while(tmpline[x]){
	    *((char *)value+x)=tmpline[x];
	    x++;
	}
	*((char *)value+x)='\0';
    }
#endif
    return(i);
}
