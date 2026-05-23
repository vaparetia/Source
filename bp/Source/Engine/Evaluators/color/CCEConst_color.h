#pragma once

//Start:CCEConst_color_declaration:D875911B:EC45745A *** Machine generated code - do not edit ***
class CCEConst_color : public IColorEvaluator
{
public:
   CCEConst_color(CEvaluatorAllocator &allocator);
   virtual ~CCEConst_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xF734AC54;

   enum EProperties
   {
      kP_Value                                         = 0xDCB67730,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   CColorf                                         mValue;
//End:CCEConst_color_declaration:D875911B *** Machine generated code - do not edit *** $End$:CCEConst_color_declaration:D875911B
};

