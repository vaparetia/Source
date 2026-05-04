/*
   linux_p.c : linux にない関数を定義

   by M.Sonoyama 1999.Sep.～

   $Id: linux_p.c,v 1.2 2002/02/22 04:41:47 usr04098 Exp $

   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>

long tell( int handle )
{
    long	p ;
    p = lseek( handle, 0, SEEK_CUR ) ;
    return p ;
}
#if 0
size_t mallocblksize( void *block )
{
    size_t	size ;
    size = malloc_usable_size(block) ;
    return ( size ) ;
}
#endif
