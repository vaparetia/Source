#pragma once

//Start:CFEClampMinMax_float_declaration:4E2FD838:3012884C *** Machine generated code - do not edit ***
class CFEClampMinMax_float : public IFloatEvaluator
{
public:
   CFEClampMinMax_float(CEvaluatorAllocator &allocator);
   virtual ~CFEClampMinMax_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x77796F8C;

   enum EProperties
   {
      kP_Input                                         = 0x19E91DD3,
      kP_Min                                           = 0x99365B30,
      kP_Max                                           = 0xA53B6469,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mInput;
   boost::shared_ptr<IFloatEvaluator>              mMin;
   boost::shared_ptr<IFloatEvaluator>              mMax;
//End:CFEClampMinMax_float_declaration:4E2FD838 *** Machine generated code - do not edit *** $End$:CFEClampMinMax_float_declaration:4E2FD838
};

