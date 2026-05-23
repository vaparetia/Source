#pragma once

//Start:CFEInitialRandomV_float_declaration:173726E3:29534821 *** Machine generated code - do not edit ***
class CFEInitialRandomV_float : public IFloatEvaluator
{
public:
   CFEInitialRandomV_float(CEvaluatorAllocator &allocator);
   virtual ~CFEInitialRandomV_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xF622EFC3;

   enum EProperties
   {
      kP_CenterValue                                   = 0x2AA88BCF,
      kP_RandomVariation                               = 0xDE09F15B,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mCenterValue;
   boost::shared_ptr<IFloatEvaluator>              mRandomVariation;
//End:CFEInitialRandomV_float_declaration:173726E3 *** Machine generated code - do not edit *** $End$:CFEInitialRandomV_float_declaration:173726E3
    CEvalFloatData                                  mInitialValue;
};

