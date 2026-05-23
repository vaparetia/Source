#pragma once

//Start:CVEMathOperator_vector_declaration:B9AAFDDE:EE77E904 *** Machine generated code - do not edit ***
class CVEMathOperator_vector : public IVectorEvaluator
{
public:
   CVEMathOperator_vector(CEvaluatorAllocator &allocator);
   virtual ~CVEMathOperator_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x268B891A;

   enum EProperties
   {
      kP_Operator                                      = 0x2ED0C5D7,
      kP_A                                             = 0xD3D99E8B,
      kP_B                                             = 0x4AD0CF31,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   enum EOperatorType
   {
      kOP_Add,
      kOP_Subtract,
      kOP_Multiply,
      kOP_Divide,
      kOP_Max,
      kOP_Min,

      EOperatorType_Count
   };

   // Regular component properties
   EOperatorType                                   mOperator;
   boost::shared_ptr<IVectorEvaluator>             mA;
   boost::shared_ptr<IVectorEvaluator>             mB;
//End:CVEMathOperator_vector_declaration:B9AAFDDE *** Machine generated code - do not edit *** $End$:CVEMathOperator_vector_declaration:B9AAFDDE
};

