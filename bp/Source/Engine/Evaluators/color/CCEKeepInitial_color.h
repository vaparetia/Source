#pragma once

//Start:CCEKeepInitial_color_declaration:ABC9597B:E22AE50F *** Machine generated code - do not edit ***
class CCEKeepInitial_color : public IColorEvaluator
{
public:
   CCEKeepInitial_color(CEvaluatorAllocator &allocator);
   virtual ~CCEKeepInitial_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x2B968578;

   enum EProperties
   {
      kP_Value                                         = 0xDCB67730,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IColorEvaluator>              mValue;
//End:CCEKeepInitial_color_declaration:ABC9597B *** Machine generated code - do not edit *** $End$:CCEKeepInitial_color_declaration:ABC9597B
   CEvalColorfData                                 mInitialValue;
};

