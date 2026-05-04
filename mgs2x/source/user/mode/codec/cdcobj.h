#ifndef _cdcobj_h_
#define _cdcobj_h_

void CDC_InitObject( OBJECT *object, int model, int flag, int chanl );
void CDC_FreeObject( OBJECT *object );
void CDC_ConfigObjectEvm( OBJECT *object, int model, int flag, int chanl );
void CDC_ConfigObjectMotion(OBJECT * object,
			    int n_layer, int motion, int flag);

#endif /* _cdcobj_h_ */
