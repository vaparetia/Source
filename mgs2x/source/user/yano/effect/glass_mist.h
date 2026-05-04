typedef	struct	{
    GV_ACT_EX	actor ;

    FVECTOR		center ;         /*中心*/
    float 		height , width ; 
    FVECTOR 	perpen ;         /*鏡の法線*/
    FVECTOR 	NozzlePoint ;
    FVECTOR		jet_vec ;		/*噴射方向ベクトル*/
    FVECTOR		glass_point ;   /*鏡との交点*/
    int 	    child_call ;      /*ポリゴンが稼働中かどうかのフラグ*/

    int	        map ;
} MainWork ;
