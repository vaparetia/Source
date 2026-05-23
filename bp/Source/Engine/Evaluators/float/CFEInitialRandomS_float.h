#pragma once

//Start:CFEInitialRandomS_float_declaration:29BDAB59:BA07E85C *** Machine generated code - do not edit ***
class CFEInitialRandomS_float : public IFloatEvaluator
{
public:
   CFEInitialRandomS_float(CEvaluatorAllocator &allocator);
   virtual ~CFEInitialRandomS_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xA41AC064;

   enum EProperties
   {
      kP_Min                                           = 0x99365B30,
      kP_Max                                           = 0xA53B6469,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mMin;
   float                                           mMax;
//End:CFEInitialRandomS_float_declaration:29BDAB59 *** Machine generated code - do not edit *** $End$:CFEInitialRandomS_float_declaration:29BDAB59
   CEvalFloatData                                  mInitialValue;
};

