#pragma once

//Start:CFEFollowValue_float_declaration:49505967:442B9EAE *** Machine generated code - do not edit ***
class CFEFollowValue_float : public IFloatEvaluator
{
public:
   CFEFollowValue_float(CEvaluatorAllocator &allocator);
   virtual ~CFEFollowValue_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x879CB96A;

   enum EProperties
   {
      kP_Input                                         = 0x19E91DD3,
      kP_Speed                                         = 0xCEE7D1F2,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mInput;
   boost::shared_ptr<IFloatEvaluator>              mSpeed;
//End:CFEFollowValue_float_declaration:49505967 *** Machine generated code - do not edit *** $End$:CFEFollowValue_float_declaration:49505967
   CEvalFloatData                                  mCurrentValue;
   CEvalFloatData                                  mLastTime;
};

