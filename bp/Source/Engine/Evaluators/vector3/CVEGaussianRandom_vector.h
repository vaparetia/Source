#pragma once

//Start:CVEGaussianRandom_vector_declaration:F8C5E06E:9A5023A6 *** Machine generated code - do not edit ***
class CVEGaussianRandom_vector : public IVectorEvaluator
{
public:
   CVEGaussianRandom_vector(CEvaluatorAllocator &allocator);
   virtual ~CVEGaussianRandom_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x2BD6C203;

   enum EProperties
   {
      kP_Mean                                          = 0xD0CA408E,
      kP_Variance                                      = 0xBBB90070,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IVectorEvaluator>             mMean;
   boost::shared_ptr<IVectorEvaluator>             mVariance;
//End:CVEGaussianRandom_vector_declaration:F8C5E06E *** Machine generated code - do not edit *** $End$:CVEGaussianRandom_vector_declaration:F8C5E06E
};

