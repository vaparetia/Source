#pragma once

//Start:CFEGaussianRandom_float_declaration:EA0EF166:17918C1C *** Machine generated code - do not edit ***
class CFEGaussianRandom_float : public IFloatEvaluator
{
public:
   CFEGaussianRandom_float(CEvaluatorAllocator &allocator);
   virtual ~CFEGaussianRandom_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xC4D0EF7A;

   enum EProperties
   {
      kP_Mean                                          = 0xD0CA408E,
      kP_Variance                                      = 0xBBB90070,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mMean;
   boost::shared_ptr<IFloatEvaluator>              mVariance;
//End:CFEGaussianRandom_float_declaration:EA0EF166 *** Machine generated code - do not edit *** $End$:CFEGaussianRandom_float_declaration:EA0EF166
};

