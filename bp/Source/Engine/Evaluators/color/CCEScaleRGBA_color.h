#pragma once

//Start:CCEScaleRGBA_color_declaration:5A055353:B5F577E5 *** Machine generated code - do not edit ***
class CCEScaleRGBA_color : public IColorEvaluator
{
public:
   CCEScaleRGBA_color(CEvaluatorAllocator &allocator);
   virtual ~CCEScaleRGBA_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xB44FB946;

   enum EProperties
   {
      kP_InputColor                                    = 0xF682AF11,
      kP_Scale                                         = 0x2D870A80,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IColorEvaluator>              mInputColor;
   boost::shared_ptr<IFloatEvaluator>              mScale;
//End:CCEScaleRGBA_color_declaration:5A055353 *** Machine generated code - do not edit *** $End$:CCEScaleRGBA_color_declaration:5A055353
};

