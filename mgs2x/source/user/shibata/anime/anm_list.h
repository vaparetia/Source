/*
   anm_list.h
   2Dアニメ リスト 
   1999/09/07/   H.Tanaka
/*
   アニメーション関数リスト
*/

/* 
   ブラスト系 (オカジ)    blastsgl.c
*/
extern   void  *AN_Blast_Single( FVECTOR *pos ) ;
extern   void  *AN_Blast_Loop( FVECTOR *pos ) ;

/* 
   血 (オカジ)         blood.c
*/
extern   void  *AN_Blood_Mist( FVECTOR *pos, FVECTOR *vec) ;

/* 
   武器エフェクト (オカジ)   bullet.c
*/
extern   void  *AN_GunFlash( FMATRIX *world ) ;

/*
   マーク (オカジ)  headmark.c
*/
extern   void  *AN_HeadMark( FMATRIX *world, int mark ) ;
extern   void  *AN_Zzz(  FMATRIX *world ) ;

/*
   目？  (オカジ)    testeye.c
*/
extern   void	AN_Test_Eye( FVECTOR *mov ) ;
extern   void	AN_Test_Eye2( FVECTOR *mov, int size ) ;
extern   void	AN_Test_Eye3( FVECTOR *mov ) ;

/*-------------------------------------------------------*/
/*
   武器エフェクト (オタコン ヒュージ )  bullet_e.c
*/

extern   void    AN_BulletCase( FMATRIX *world );
extern   void    AN_CartridgeUSP(FMATRIX *world);
extern   void    AN_BUlletUSP( FMATRIX *world, int silence );
