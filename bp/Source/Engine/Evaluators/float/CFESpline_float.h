#pragma once

#include "boost/optional.hpp"
#include "Engine/Math/CSpline.h"

//Start:CFESpline_float_declaration:056FBCBF:B3194B2D *** Machine generated code - do not edit ***
class CFESpline_float : public IFloatEvaluator
{
public:
   CFESpline_float(CEvaluatorAllocator &allocator);
   virtual ~CFESpline_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x1BB90D00;

   enum EProperties
   {
      kP_SplineData                                    = 0x97C93F7C,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   CSpline                                         mSplineData;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CFESpline_float_declaration:056FBCBF *** Machine generated code - do not edit *** $End$:CFESpline_float_declaration:056FBCBF

   virtual void   GetInfo(STypeInfo &typeInfo) const;
};

