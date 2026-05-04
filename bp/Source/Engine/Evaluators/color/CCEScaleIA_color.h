#pragma once

//Start:CCEScaleIA_color_declaration:8ED9AA94:5FF287E2 *** Machine generated code - do not edit ***
class CCEScaleIA_color : public IColorEvaluator
{
public:
   CCEScaleIA_color(CEvaluatorAllocator &allocator);
   virtual ~CCEScaleIA_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x218F68F5;

   enum EProperties
   {
      kP_InputColor                                    = 0xF682AF11,
      kP_ScaleRGB                                      = 0xDF953F16,
      kP_ScaleAlpha                                    = 0x0B2A1172,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IColorEvaluator>              mInputColor;
   boost::shared_ptr<IFloatEvaluator>              mScaleRGB;
   boost::shared_ptr<IFloatEvaluator>              mScaleAlpha;
//End:CCEScaleIA_color_declaration:8ED9AA94 *** Machine generated code - do not edit *** $End$:CCEScaleIA_color_declaration:8ED9AA94
};

