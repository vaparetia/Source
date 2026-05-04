//------------------------------------------------------------------------------------------
// OpcodeConversion.h
// Bluepoint
// Helper functions to convert projectx math types to opcode math types.
// All inline to prevent link issues.
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

#ifndef OPCODECONVERSION_H
#define OPCODECONVERSION_H

//------------------------------------------------------------------------------------------

#include "Engine/Math/CMatrix4.h"
#include "Engine/Math/CMatrix34.h"
#include "Engine/Math/CMatrix3.h"
#include "ExtLibraries/Opcode/Opcode.h"

//------------------------------------------------------------------------------------------
// forward declaration

class CMatrix3;
class CMatrix4;
class CQuaternion;

//------------------------------------------------------------------------------------------
namespace OpcodeConversion
{
   inline CVector3      PointToCVector3(Point const &in)
   {
      return CVector3(in.x, in.y, in.z);
   }

   inline Point         PointFromCVector3(CVector3 const &in)
   {
      return Point(in.mX, in.mY, in.mZ);
   }
  
   inline CVector4      HPointToCVector4(HPoint const &in)
   {
      return CVector4(in.x, in.y, in.z, in.w);
   }

   inline HPoint        HPointFromCVector4(CVector4 const &in)
   {
      return HPoint(in.mX, in.mY, in.mZ, in.mW);
   }
   
   inline HPoint        HPointFromCVector3(CVector3 const &in, real32 const w)
   {
      return HPoint(in.mX, in.mY, in.mZ, w);
   }

   inline CVector3      HPointToCVector3(HPoint const &in)
   {
      // Drop the 'w'
      return CVector3(in.x, in.y, in.z);
   }

   inline CMatrix4      Matrix4x4ToCMatrix4(Matrix4x4 const &in)
   {
      CMatrix4 out(  HPointToCVector4(in.GetRow(0)),
                     HPointToCVector4(in.GetRow(1)),
                     HPointToCVector4(in.GetRow(2)),
                     HPointToCVector4(in.GetRow(3)));
      return out;
   }
   
   inline Matrix4x4     Matrix4x4FromCMatrix4(CMatrix4 const &in)
   {
      Matrix4x4 out;
      out.SetRow(0, HPointFromCVector4(in.GetRow(0)));
      out.SetRow(1, HPointFromCVector4(in.GetRow(1)));
      out.SetRow(2, HPointFromCVector4(in.GetRow(2)));
      out.SetRow(3, HPointFromCVector4(in.GetRow(3)));
      return out;
   }

   inline CMatrix34     Matrix4x4ToCMatrix34(Matrix4x4 const &in)
   {
      CMatrix34 out( HPointToCVector3(in.GetRow(0)),
                     HPointToCVector3(in.GetRow(1)),
                     HPointToCVector3(in.GetRow(2)),
                     HPointToCVector3(in.GetRow(3)));
      return out;
   }
   
   inline Matrix4x4     Matrix4x4FromCMatrix34(CMatrix34 const &in)
   {
      Matrix4x4 out;
      out.SetRow(0, PointFromCVector3(in.GetRow(0)));
      out.SetRow(1, PointFromCVector3(in.GetRow(1)));
      out.SetRow(2, PointFromCVector3(in.GetRow(2)));
      out.SetRow(3, PointFromCVector3(in.GetRow(3)));
      return out;
   }
}

#endif

//------------------------------------------------------------------------------------------

