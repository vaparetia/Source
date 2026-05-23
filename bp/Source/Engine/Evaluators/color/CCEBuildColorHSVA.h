#pragma once

//Start:CCEBuildColorHSVA_declaration:8085F107:F997A72F *** Machine generated code - do not edit ***
class CCEBuildColorHSVA : public IColorEvaluator
{
public:
   CCEBuildColorHSVA(CEvaluatorAllocator &allocator);
   virtual ~CCEBuildColorHSVA();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xE49CBDEB;

   enum EProperties
   {
      kP_Hue                                           = 0xEE581D0E,
      kP_Saturation                                    = 0x6577AE58,
      kP_Value                                         = 0xDCB67730,
      kP_Alpha                                         = 0x1121166E,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mHue;
   boost::shared_ptr<IFloatEvaluator>              mSaturation;
   boost::shared_ptr<IFloatEvaluator>              mValue;
   boost::shared_ptr<IFloatEvaluator>              mAlpha;
//End:CCEBuildColorHSVA_declaration:8085F107 *** Machine generated code - do not edit *** $End$:CCEBuildColorHSVA_declaration:8085F107
};

