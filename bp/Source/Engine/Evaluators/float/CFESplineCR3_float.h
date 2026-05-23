#pragma once

//Start:CFESplineCR3_float_declaration:C7BAC0C6:E6403B9E *** Machine generated code - do not edit ***
class CFESplineCR3_float : public IFloatEvaluator
{
public:
   CFESplineCR3_float(CEvaluatorAllocator &allocator);
   virtual ~CFESplineCR3_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x2D4E1EC3;

   enum EProperties
   {
      kP_Time1                                         = 0x44C4E134,
      kP_Float1                                        = 0x04DC2814,
      kP_Time2                                         = 0xDDCDB08E,
      kP_Float2                                        = 0x9DD579AE,
      kP_Time3                                         = 0xAACA8018,
      kP_Float3                                        = 0xEAD24938,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mTime1;
   boost::shared_ptr<IFloatEvaluator>              mFloat1;
   boost::shared_ptr<IFloatEvaluator>              mTime2;
   boost::shared_ptr<IFloatEvaluator>              mFloat2;
   boost::shared_ptr<IFloatEvaluator>              mTime3;
   boost::shared_ptr<IFloatEvaluator>              mFloat3;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CFESplineCR3_float_declaration:C7BAC0C6 *** Machine generated code - do not edit *** $End$:CFESplineCR3_float_declaration:C7BAC0C6
};

