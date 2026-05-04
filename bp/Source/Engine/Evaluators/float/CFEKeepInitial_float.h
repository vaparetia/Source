#pragma once

//Start:CFEKeepInitial_float_declaration:39D4AAB4:6CF91EB8 *** Machine generated code - do not edit ***
class CFEKeepInitial_float : public IFloatEvaluator
{
public:
   CFEKeepInitial_float(CEvaluatorAllocator &allocator);
   virtual ~CFEKeepInitial_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x84659304;

   enum EProperties
   {
      kP_Value                                         = 0xDCB67730,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mValue;
//End:CFEKeepInitial_float_declaration:39D4AAB4 *** Machine generated code - do not edit *** $End$:CFEKeepInitial_float_declaration:39D4AAB4
   CEvalFloatData                                  mInitialValue;
};

