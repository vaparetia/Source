/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#if !defined(EDGEGEOM_ATTRIBUTES_H)
#define EDGEGEOM_ATTRIBUTES_H

// This is the master list of vertex attribute IDs used throughout the
// Edge tools and runtime.  Studios are encouraged to extend this list
// locally with their own custom attributes.  Here are the rules:
//
// - The first 16 attribute IDs are reserved by Edge, and should not be
//   modified.
// - The maximum legal attribute ID is 255.  Don't define more than that!
enum EdgeGeomAttributeId
	{
		EDGE_GEOM_ATTRIBUTE_ID_UNKNOWN = 0x00,

		EDGE_GEOM_ATTRIBUTE_ID_POSITION = 0x01,
		EDGE_GEOM_ATTRIBUTE_ID_NORMAL   = 0x02,
		EDGE_GEOM_ATTRIBUTE_ID_TANGENT  = 0x03,
		EDGE_GEOM_ATTRIBUTE_ID_BINORMAL = 0x04,
		
		
		EDGE_GEOM_ATTRIBUTE_ID_UV0      = 0x05,
		EDGE_GEOM_ATTRIBUTE_ID_UV1      = 0x06,
		EDGE_GEOM_ATTRIBUTE_ID_UV2      = 0x07,
		EDGE_GEOM_ATTRIBUTE_ID_UV3      = 0x08,
		EDGE_GEOM_ATTRIBUTE_ID_COLOR    = 0x09,
		
		EDGE_GEOM_ATTRIBUTE_ID_INSTANCED_COLOR = 0x10,
	};


#endif // EDGEGEOM_ATTRIBUTES_H
