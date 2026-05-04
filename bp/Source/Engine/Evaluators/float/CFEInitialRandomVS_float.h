#pragma once

//Start:CFEInitialRandomVS_float_declaration:4304D5B5:0997B652 *** Machine generated code - do not edit ***
class CFEInitialRandomVS_float : public IFloatEvaluator
{
public:
   CFEInitialRandomVS_float(CEvaluatorAllocator &allocator);
   virtual ~CFEInitialRandomVS_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x060AAB0D;

   enum EProperties
   {
      kP_CenterValue                                   = 0x2AA88BCF,
      kP_RandomVariation                               = 0xDE09F15B,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mCenterValue;
   float                                           mRandomVariation;
//End:CFEInitialRandomVS_float_declaration:4304D5B5 *** Machine generated code - do not edit *** $End$:CFEInitialRandomVS_float_declaration:4304D5B5
   CEvalFloatData                                  mInitialValue;
};

