/*
   MDU_darFile.h
   darfile関連ヘッダ

   by M.Sonoyama 1999 Aug.～
   $Id: MDU_darFile.h,v 1.3 1999/10/13 10:46:28 usr02011 Exp $

   Konami Computer Entertainment Japan West   
*/

extern	void	MDU_DarInitFileAll( void ) ;
extern	void	MDU_DarChangeMode( int, int ) ;
extern	int	MDU_DarCreateArchive( char *, int, char ** ) ;
extern	int	MDU_DarAppendArchive( char *, int, char ** ) ;
extern	int	MDU_DarExtractArchive( char *, char * ) ;
extern	int	MDU_DarExtractArchive2( char *, char *, int, char ** ) ;
extern	int	MDU_DarDeleteArchive( char *, int, char ** ) ;

