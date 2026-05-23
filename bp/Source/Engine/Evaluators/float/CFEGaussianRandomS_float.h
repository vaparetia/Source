#pragma once

//Start:CFEGaussianRandomS_float_declaration:DE2B9BCD:9EDFB566 *** Machine generated code - do not edit ***
class CFEGaussianRandomS_float : public IFloatEvaluator
{
public:
   CFEGaussianRandomS_float(CEvaluatorAllocator &allocator);
   virtual ~CFEGaussianRandomS_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xB4855225;

   enum EProperties
   {
      kP_Mean                                          = 0xD0CA408E,
      kP_Variance                                      = 0xBBB90070,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mMean;
   float                                           mVariance;
//End:CFEGaussianRandomS_float_declaration:DE2B9BCD *** Machine generated code - do not edit *** $End$:CFEGaussianRandomS_float_declaration:DE2B9BCD
};

