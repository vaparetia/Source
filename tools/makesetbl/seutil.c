/*
	seutil.c
	2000/03/08 Y.Korekado
	$Id: seutil.c,v 1.2 2000/03/15 04:19:19 usr01749 Exp $
	
	se_table 作成プログラムユーティリティー
*/

#include 	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/types.h>

#define	MAX_CONST	256
#define	MAX_STAGE	2048
#define	MAX_SET_STAGE	256
#define	MAX_NAME	64

#define	MAX_LINE	128

#define	WINVer	(1)

#ifdef WINVer
#include "win_util.h"
#else
#include "MDU_util.h"
#endif


typedef struct {
	char	name[ MAX_NAME ] ;	/* ＳＥネーム */
#if 1
	char	lst[ MAX_LINE ] ;	/* パス名 */
	char	comment[ MAX_LINE ] ;	/* コメント */
#else
	char	pri ;				/* 優先 */
	char	fix ;				/* パンステレオ固定 */
	char	chan ;				/* チャンネル */
	char	padd ;
	char	path[ MAX_NAME ] ;	/* パス名 */
#endif
} SE_SEL;

static	SE_SEL	ConstSeBuff[ MAX_CONST ] ;
static	SE_SEL	StageSeBuff[ MAX_STAGE ] ;
static	SE_SEL	SetStageBuff[ MAX_SET_STAGE ] ;

static	char	stage_table[ MAX_STAGE ] ;	/* 出力 */
static	char	line_buff[ MAX_LINE ] ;

#define	FILL_NUM	(0xff)

/* --------------------------------------------------- */
/* EUC漢字の1バイト目 0xa1～0xfc (0xfe ?) */
/* EUC漢字の2バイト目 0xa1～0xfe */
/* EUC半角カナの1バイト目 0x8e、2バイト目 0xa1～0xdf(SJISの半角カナと同じ) */

/* SJIS漢字の1バイト目 0x81～0x9f,0xe0～0xfc */
/* SJIS漢字の2バイト目 0x40～0x7e,0x80～0xfc */

int ConvSJIS2EUC( char *sjis, char *euc )
{
	unsigned char	first, second ;
	int				kcode, code1, code2 ;

	while ( *sjis != '\0' ){
		first = (unsigned char)sjis[0] ;
		second = (unsigned char)sjis[1] ;

		if ( ( first >= 0x81 && first <= 0x9f )
			|| ( first >= 0xe0 && first <= 0xfc ) ){
			/* 漢字コードの可能性あり */

			/* 文字列終了チェック */
			if ( second == '\0' ) break ;

			/* １バイト目中間コード生成 */
			if ( first < 0xe0 )	code1 = first - 0x81 ;
			else				code1 = first - 0xe0 + ( 0xa0 - 0x81 ) ;

			/* ２バイト目チェック */
			if ( ( second >= 0x40 && second <= 0x7e )
				|| ( second >= 0x80 && second <= 0xfc ) ){
				/* 変換可能漢字コード */

				/* ２バイト目中間コード生成 */
				if ( second < 0x80 )	code2 = second - 0x40 ;
				else					code2 = second - 0x80 + ( 0x7f - 0x40 ) ;

				/* 中間コードから漢字コードを求めてコード変換の開始 */
				kcode = code1 * ( ( 0x7f - 0x40 ) + ( 0xfd - 0x80 ) );
				kcode += code2 ;
				*euc++ = kcode / ( 0xff - 0xa1 ) + 0xa1 ;
				*euc++ = kcode % ( 0xff - 0xa1 ) + 0xa1 ;
				sjis += 2 ;
			} else {
				/* 不正なコード */
				*euc++ = *sjis++;
			}
		} else if ( first >= 0xa1 && first <= 0xdf ){
			/* 半角かな */
			*euc++ = 0x8e ;
			*euc++ = *sjis++ ;
		} else {
			/* 英数・その他 */
			*euc++ = *sjis++;
		}
	}
	*euc = '\0' ;
	return ( 0 );
}
/* --------------------------------------------------- */
static void InitBuff( void ) 
{
	int i ;
	
	for( i=0; i<MAX_STAGE; i++ ) {
		stage_table[ i ] = FILL_NUM ;
	}
}

static char *FGetLine( FILE *fp )
{
	if ( fgets( &line_buff[0], sizeof(char)*MAX_LINE, fp ) == NULL )	return NULL ;

#ifdef PRI_DEB
	printf("%s", line_buff ) ;
#endif
	return &line_buff[0] ;
}

static int is_expr_num( char *buf, int *value )
{
	char *p;
	int v;
	int minus_flag;

	v = 0;
	if( buf[ 0 ] == '-' ){
		minus_flag = 1;
		buf ++;
	} else {
		minus_flag = 0;
	}
	if( buf[ 0 ] == '0' ){
		if( ( buf[ 1 ] | 0x20 ) == 'x' ){
			for( p = buf + 2; isxdigit(*p) ; p++ ){
				int c;

				c = *p | 0x20;
				if( c >= '0' && c <= '9' ){
					v = ( v * 16 ) + ( *p - '0' );
				} else if( c >= 'a' && c <= 'f' ){
					v = ( v * 16 ) + ( c - 'a' + 10 );
				} else {
					printf( "不正な16進数値です\n" );
					return 0;
				}
			}
			goto END;
		} else if( ( buf[ 1 ] | 0x20 ) == 'b' ){
			for( p = buf + 2; (*p==0 || *p==1); p++ ){
				int c;

				c = *p;
				if( c == '0' || c == '1' ){
					v = ( v << 1 ) | ( c - '0' );
				} else {
					printf( "不正な2進数値です\n" );
					return 0;
				}
			}
			goto END;
		}
	}
	for( p = buf; isdigit(*p); p++ ){
		if( *p >= '0' && *p <= '9' ){
			v = ( v * 10 ) + ( *p - '0' );
		} else {
			return 0;
		}
	}
END:
	if( minus_flag ) v = -v;

	*value = v;
	return 1;
}

static char *SkipSpace( char *p )
{
	for(;;){
		if( *p == '\0' )return NULL;
		if( !isspace( *p ) ) break;
		p++ ;
	}
	return p ;
}

static char *SkipNextSpace( char *p )
{
	for(;;){
		if( *p == '\0' )return NULL;
		if( isspace( *p ) ) break;
		p++ ;
	}
	return p ;
}

static char *SkipComment( char *p, FILE *fp )
{
	for(;;){
		if( *p == '*' && *(p+1) == '/' ) {
			p += 2 ;
			break;
		}
		if( *p == '\0' ) {
			if ( (p = FGetLine( fp )) == NULL ) return NULL ; 
			p = &line_buff[0] ;
			continue ;
		}
		p++ ;
	}
	return p ;
}

static char *SearchChara( char *p, FILE *fp )
{
	for(;;) {
		if ( (p = SkipSpace( p )) == NULL ) {
			if ( (p = FGetLine( fp )) == NULL ) return NULL ; 
			continue ;
		}

		if ( *p == '/' ) {
			if ( *(p+1) == '/' ) {
				if ( (p = FGetLine( fp )) == NULL ) return NULL ; 
				continue ;
			}
			if ( *(p+1) == '*' ) {
				p +=2 ;
				if ( (p = SkipComment( p, fp )) == NULL ) return NULL ;
				continue ;
			}
		}

		/* ここで先頭ポインタには文字が入っている */
		break ;
	}

	return p ;
}
static	char *GetCharString( char *p, char *buff )
{
	int i ;		

	for( i=0; i<MAX_NAME-1; i++,p++) {
		if ( isspace(*p) ) break ;
		if ( *p == '\0' || *p == '/' ) break ;
		*(buff + i) = *p ;
	}
	*(buff + i) = '\0' ;	/* 最後に入れておく */

	return p ;
}

static int iscomment( char *p )
{
	if ( *p == '/' ) {
		p++ ;
		if ( *p == '/' || *p == '*' ) return 1 ;
	}
	return 0 ;
}

static	char *GetChar( char *p, char *buff )
{
	int i ;

	for( i=0; i<MAX_NAME-1; i++,p++) {
		if ( *p == '\0' || iscomment(p)) break ;
		*(buff + i) = *p ;
	}
	while ( isspace ( *(buff+(i-1) ) ) ){
		i-- ;
	}
	*(buff + i) = '\0' ;	/* 最後に入れておく */

	return p ;
}

static	char *GetCharLine( char *p, char *buff )
{
	int i ;

	for( i=0; i<MAX_NAME-1; i++,p++) {
		if ( *p == '\0'|| *p == '\n' ) break ;
		*(buff + i) = *p ;
	}
	*(buff + i) = '\0' ;	/* 最後に入れておく */

	return p ;
}

static	char *GetDigitString( char *p, char *buff )
{
	int num ;

	if ( is_expr_num( p, &num ) == 0 ) return NULL ;
	*buff = (char)num ;

	return SkipNextSpace( p ) ;
}

static char *SkipToLineEnd( char *p )
{
	for(;;) {
		if ( *p == '\0' ) break ;
	}
	return p ;
}

static int isinclude( char *p, char *name )
{
	int i ;
	char inc_name[] = "include" ;
	
	if( *p != '#' ) return 0 ;
	
	p++;
	for( i=0; i<7; i++, p++ ) {
		if( *p != inc_name[i] ) return 0 ;
	}

	while( *p != '<' && *p != '"' ) {
		if ( *p =='\0' ) return 0 ;
		p++;
	}
	p++;
	while( *p != '>' && *p != '"' ) {
		if ( *p =='\0' ) return 0 ;
		*name = *p ;
		p++;
		name++ ;
	}
	*name = '\0' ;

	return 1 ;
}

static int ReadSel( FILE *fp, SE_SEL *sel )
{
	char	*p, filename[MAX_NAME] ;
	int		i, now_num ;

	now_num = 0 ;
	if ( (p = FGetLine( fp )) == NULL ) return -1 ; 

	for(;;) {
		if ( (p = SearchChara( p, fp )) == NULL ) break ;

		if ( isinclude( p, &filename[0] ) ) {
			FILE	*ifp ;
			printf(" include file [%s]\n",&filename[0] ) ;
			if ( ( ifp = fopen( &filename[0], "rt" ) ) == NULL ){
				printf(" not find include file!!\n") ;
				return (-1) ;
			}
			now_num += ReadSel( ifp, &sel[now_num] );
			fclose( ifp );

			if ( (p = FGetLine( fp )) == NULL ) break  ; 
			continue ;
		}


		now_num++ ;
#ifdef PRI_DEB
printf("num [%d]\n",now_num ) ;
#endif
		p = GetCharString( p, &sel[now_num-1].name[0] ) ;

#if 1
		sel[now_num-1].lst[0]='\0';
		if ( (p=SkipSpace( p ) ) != NULL ) {
			p = GetChar( p, &sel[now_num-1].lst[0] ) ;
		}
		sel[now_num-1].comment[0]='\0';
		if ( iscomment(p) ) {
			p = GetCharLine( p, &sel[now_num-1].comment[0] ) ;
		}
		if ( (p = FGetLine( fp )) == NULL ) break  ; 
#else
		sel[now_num-1].pri=0;
		sel[now_num-1].fix=0;
		sel[now_num-1].chan=0;
		sel[now_num-1].path[0]='\0';

		/* 優先度読み込み */
		if ( (p=SkipSpace( p ) ) == NULL ) goto NextLine;
		if ( !(isdigit( *p )) && !((*p) == '-') ) goto NextLine ;
		if ( (p = GetDigitString( p, &sel[now_num-1].pri )) == NULL ) goto NextLine ;

		/* パン固定モード読み込み */
		if ( (p=SkipSpace( p ) ) == NULL ) goto NextLine ;
		if ( !(isdigit( *p )) && !((*p) == '-') ) goto NextLine ;
		if ( (p = GetDigitString( p, &sel[now_num-1].fix )) == NULL ) goto NextLine ;

		/* チャンネル読み込み */
		if ( (p=SkipSpace( p ) ) == NULL ) goto NextLine ;
		if ( !(isdigit( *p )) && !((*p) == '-') ) goto NextLine ;
		if ( (p = GetDigitString( p, &sel[now_num-1].chan )) == NULL ) goto NextLine ;

		/* パス読み込み */
		if ( (p=SkipSpace( p ) ) == NULL ) goto NextLine ;
		if ( (*p=='/' && *(p+1)=='/') || (*p=='/' && *(p+1)=='*') ) continue ;
		p = GetCharString( p, &sel[now_num-1].path[0] ) ;

NextLine:

		if ( (p = FGetLine( fp )) == NULL ) break  ; 
#endif
	}

#ifdef PRI_DEB
	for( i=0; i<now_num; i++ ) {
		printf("[%d] %s",i, sel[i].name ) ;
		printf(" %s\n",sel[i].lst ) ;
	}
#endif	
	return now_num ;
}

static	void	Usage( void )
{
    printf( "makecm2 : texture archiver for MGS2\n" ) ;
    printf( "\tusage : makecm2 -out archive_file_root \\\n" ) ;
    printf( "\t\t\t -in file1 [file2 ...]\n" ) ;
    exit( -1 ) ;
}

static int SearchSe( char *name, SE_SEL *sel, int num )
{
	int i ;
	
	for( i=0; i<num; i++ ) {
		if ( strcmp( name, &sel[i].name[0]) == 0 ) return i ;
	}
	
	return FILL_NUM ;
}

