#pragma once

//Start:CVEBuildFromFloat_vector_declaration:04B665BF:A1A757AF *** Machine generated code - do not edit ***
class CVEBuildFromFloat_vector : public IVectorEvaluator
{
public:
   CVEBuildFromFloat_vector(CEvaluatorAllocator &allocator);
   virtual ~CVEBuildFromFloat_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x029582BF;

   enum EProperties
   {
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CVEBuildFromFloat_vector_declaration:04B665BF *** Machine generated code - do not edit *** $End$:CVEBuildFromFloat_vector_declaration:04B665BF
};

