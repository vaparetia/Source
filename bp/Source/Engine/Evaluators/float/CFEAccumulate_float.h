#pragma once

//Start:CFEAccumulate_float_declaration:29B2D968:D0B64F17 *** Machine generated code - do not edit ***
class CFEAccumulate_float : public IFloatEvaluator
{
public:
   CFEAccumulate_float(CEvaluatorAllocator &allocator);
   virtual ~CFEAccumulate_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xD2096B0D;

   enum EProperties
   {
      kP_Input                                         = 0x19E91DD3,
      kP_Scale                                         = 0x2D870A80,
      kP_Wrap                                          = 0x0C087FE3,
      kP_Offset                                        = 0x5EA6CFE6,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mInput;
   float                                           mScale;
   float                                           mWrap;
   float                                           mOffset;
//End:CFEAccumulate_float_declaration:29B2D968 *** Machine generated code - do not edit *** $End$:CFEAccumulate_float_declaration:29B2D968
   CEvalFloatData                                  mAccumulatedValue;
   CEvalFloatData                                  mLastTime;
};

