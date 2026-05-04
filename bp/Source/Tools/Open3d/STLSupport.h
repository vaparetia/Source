//------------------------------------------------------------------------------------------
// STLSupport.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Graphics/CColorf.h"
#include "Engine/Math/CVector4.h"
#include "Engine/Math/CQuaternion.h"
#include "Engine/System/CGuid.h"

#include "vector"

//------------------------------------------------------------------------------------------

using namespace System;

//------------------------------------------------------------------------------------------

namespace Open3d
{
   public class TIntVector : public std::vector<int> {};
   public class TFloatVector : public std::vector<float> {};
   public class TVector2Vector : public std::vector<CVector2> {};
   public class TVector3Vector : public std::vector<CVector3> {};
   public class TVector4Vector : public std::vector<CVector4> {};
   public class TQuat4Vector : public std::vector<CQuaternion> {};
   public class TColorVector : public std::vector<CColor> {};
   public class TColorFVector : public std::vector<CColorf> {};
   public class TGuidVector : public std::vector<CGuid> {};
}



