/*

  壊れポテトのメッセージ関数

 */

/*  自分の足がポテトに反応するようにする */
extern void BRK_BOX_RegistFoot( int name, OBJECT *body ) ;

/*  ポテトの箱の寿命をポインタで取得する */
extern void BRK_BOX_GetBoxVitality( int name, int **v ) ;

/*  ポテトの数をポインタで取得する */
extern void BRK_BOX_GetPotatoNum( int name, int **v ) ;

/*  n番目のポテトの位置を FVECTORポインタで取得する */
extern void BRK_BOX_GetPotatoPos( int name, FVECTOR **v, int n ) ;

/*  ポテトの箱の位置をポインタで取得する */
extern void BRK_BOX_GetBoxPos( int name, FVECTOR **v ) ;

/*
  コード例

  FVECTOR *pos ;

  BRK_BOX_GetBoxVitality( GV_StrCode( "壊れポテト" ), &work->box_vitatlity, 3 ) ;
  BRK_BOX_GetPotatoPos( GV_StrCode( "壊れポテト" ), &pos, 3 ) ;

*/
