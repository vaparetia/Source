#pragma once

//Start:CVERandom_vector_declaration:5033D95C:E6809A78 *** Machine generated code - do not edit ***
class CVERandom_vector : public IVectorEvaluator
{
public:
   CVERandom_vector(CEvaluatorAllocator &allocator);
   virtual ~CVERandom_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xB8FE7652;

   enum EProperties
   {
      kP_Magnitude                                     = 0xB776FFA2,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mMagnitude;
//End:CVERandom_vector_declaration:5033D95C *** Machine generated code - do not edit *** $End$:CVERandom_vector_declaration:5033D95C

};

