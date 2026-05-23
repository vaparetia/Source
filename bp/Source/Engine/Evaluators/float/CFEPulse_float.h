#pragma once

//Start:CFEPulse_float_declaration:4831532B:B3B3118D *** Machine generated code - do not edit ***
class CFEPulse_float : public IFloatEvaluator
{
public:
   CFEPulse_float(CEvaluatorAllocator &allocator);
   virtual ~CFEPulse_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xB000B63F;

   enum EProperties
   {
      kP_OnTime                                        = 0x872C3851,
      kP_OnValue                                       = 0x074534E9,
      kP_OffTime                                       = 0x09125C23,
      kP_OffValue                                      = 0xB9C01A9D,
      kP_Loop                                          = 0x016DB2D0,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mOnTime;
   boost::shared_ptr<IFloatEvaluator>              mOnValue;
   boost::shared_ptr<IFloatEvaluator>              mOffTime;
   boost::shared_ptr<IFloatEvaluator>              mOffValue;
   bool                                            mLoop;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CFEPulse_float_declaration:4831532B *** Machine generated code - do not edit *** $End$:CFEPulse_float_declaration:4831532B
};

