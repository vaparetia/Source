//----------------------------------------------------------------------------
// DataTypes.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/Graphics/CColorf.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector4.h"
#include "Engine/Math/CQuaternion.h"
#include "Engine/System/CGuid.h"

#include "vector"

//----------------------------------------------------------------------------

using namespace System;

//----------------------------------------------------------------------------

namespace Open3d
{
   // to use all these types in a managed c++ application you need to include the header "Tools/Open3d/STLSupport.h"
   public class TIntVector : public std::vector<int> {};
   public class TFloatVector : public std::vector<float> {};
   public class TVector2Vector : public std::vector<CVector2> {};
   public class TVector3Vector : public std::vector<CVector3> {};
   public class TVector4Vector : public std::vector<CVector4> {};
   public class TQuat4Vector : public std::vector<CQuaternion> {};
   public class TColorVector : public std::vector<CColor> {};
   public class TColorFVector : public std::vector<CColorf> {};
   public class TGuidVector : public std::vector<CGuid> {};

   public value struct Vector2
   {
   public:
      float mX, mY;
   };

   public value struct Vector3
   {
   public:
      Vector3( float x, float y, float z )
         : mX( x ), mY( y ), mZ( z )
      {
      }

      float mX, mY, mZ;
   };

   public value struct Vector4
   {
   public:
      float mX, mY, mZ, mW;
   };

   public value struct Quat3
   {
   public:
      float mX, mY, mZ;
   };

   public value struct Quat4
   {
   public:
      float mX, mY, mZ, mW;
   };

   public value struct Color
   {
   public:
      unsigned int mARGB;
   };

   public value struct ColorF
   {
   public:
      float mR, mG, mB, mA;
   };
}




