#pragma once

//Start:CVESampleAndHold_vector_declaration:10BEF6AF:0FD0DF8A *** Machine generated code - do not edit ***
class CVESampleAndHold_vector : public IVectorEvaluator
{
public:
   CVESampleAndHold_vector(CEvaluatorAllocator &allocator);
   virtual ~CVESampleAndHold_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x887537E1;

   enum EProperties
   {
      kP_Sample                                        = 0xF6A773F5,
      kP_Hold                                          = 0xBFF8A239,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IVectorEvaluator>             mSample;
   boost::shared_ptr<IFloatEvaluator>              mHold;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CVESampleAndHold_vector_declaration:10BEF6AF *** Machine generated code - do not edit *** $End$:CVESampleAndHold_vector_declaration:10BEF6AF
   CEvalVector3Data                                mLastSample;
   CEvalFloatData                                  mLastSampleTime;
};

