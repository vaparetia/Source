#ifndef __EVERTEXDATATYPE_H__
#define __EVERTEXDATATYPE_H__

enum EVertexDataType
{
   kVDT_Float2,         // 2D float expanded to (value, value, 0, 1)
   kVDT_Float3,         // 3D float expanded to (value, value, value, 1)
   kVDT_Float4,         // 4D float

   kVDT_UByte4N,        // Each of 4 bytes is normalized by dividing to 255.0
   kVDT_UByte4,         // 4D unsigned byte

   kVDT_Half2,          // Two 16-bit floating point values, expanded to (value, value, 0, 1)
   kVDT_Half4,          // Four 16-bit floating point values

   kVDT_Short2N,        // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
   kVDT_Short4N,        // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)

   kVDT_Packed3N,       // 3d signed normalized (11:11:10 on PS3/X360, 10:10:10 on PC D3D) expanded to (value, value, value, 1)

   kVDT_Int32,

   kVDT_Short2,         // 2D signed short expanded to (value, value, 0., 1.)
   kVDT_Short4,         // 4D signed short

   kVDT_Count,

   kVDT_Invalid = -1
};


#endif /* __EVERTEXDATATYPE_H__ */