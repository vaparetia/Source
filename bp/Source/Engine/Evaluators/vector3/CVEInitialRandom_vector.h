#pragma once

//Start:CVEInitialRandom_vector_declaration:651F8D12:69C7A0BE *** Machine generated code - do not edit ***
class CVEInitialRandom_vector : public IVectorEvaluator
{
public:
   CVEInitialRandom_vector(CEvaluatorAllocator &allocator);
   virtual ~CVEInitialRandom_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x3B495042;

   enum EProperties
   {
      kP_Magnitude                                     = 0xB776FFA2,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mMagnitude;
//End:CVEInitialRandom_vector_declaration:651F8D12 *** Machine generated code - do not edit *** $End$:CVEInitialRandom_vector_declaration:651F8D12

   CEvalVector3Data                                mVector;
};

