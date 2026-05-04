/* オプション解釈ユーティリティ */

static	char	**Argv ;
static	char	**ArgPtr ;
static	int	Argc, Argn ;

/* 文字列をパーサーにかけることを明示 */
void	MDU_ParseOption( argc, argv )
int	argc ;
char	**argv ;
{
    Argc = argc ;
    Argn = 0 ;
    ArgPtr = Argv = argv ;
}

/* オプション開始ポインタを返す */
char	**MDU_GetOption( opt ) 
char	opt ;
{
    int		i ;
    char	**ptr, *p ;

    ptr = Argv ;
    Argn = 0 ;
    for ( i = 0; i < Argc; i ++ ) {
	p = *ptr ;
	if ( p[ 0 ] == '-' ) {
	    if ( p[ 1 ] == opt ) {
		ArgPtr = ptr ;
		Argn = i ;
		return ptr ;
	    }
	} 
	ptr ++ ;
    }
    return NULL ;
}

/* 現在のポインタの値を取得 */
char	*MDU_GetNextValue( void )
{
    char	*ptr ;

    ArgPtr ++ ;
    Argn ++ ;
    ptr = *ArgPtr ;
    if ( Argn == Argc || 
	 ( ptr[ 0 ] == '-' && strlen( ptr ) != 1 ) ) return NULL ;
    return ptr ;
}

/* 現在のポインタの値を整数にして取得 */
int	MDU_GetNextInt( void )
{
    char	*ptr ;

    ptr = MDU_GetNextValue() ;
    if ( ptr == NULL ) return -1 ;
    return atoi( ptr ) ;
}

/*----------------------------------------------------------------*/

void		MDU_ChangeExtension( name, ext )
char		*name, *ext ;
{
    char	c, *cp ;
#if 0
    cp = name ;
    while ( ( c = *( cp ++ ) ) != '\0' ) {
	if ( c == '.' ) break ;
    }
#endif
    cp = strrchr( name, '.' ) ;
    if ( cp == NULL ) return ;
    cp ++ ;
    strcpy( cp, ext ) ;
}

