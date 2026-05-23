#pragma once

//Start:CVEConst_vector_declaration:757B64A7:2AF8A217 *** Machine generated code - do not edit ***
class ENGINE_API CVEConst_vector : public IVectorEvaluator
{
public:
   CVEConst_vector(CEvaluatorAllocator &allocator);
   virtual ~CVEConst_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xAE2FE58E;

   enum EProperties
   {
      kP_Vector                                        = 0x1CC24D6D,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   CVector3                                        mVector;
//End:CVEConst_vector_declaration:757B64A7 *** Machine generated code - do not edit *** $End$:CVEConst_vector_declaration:757B64A7
};

