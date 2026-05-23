#pragma once

//Start:CCESpline_color_declaration:6F8334AC:D15A61F3 *** Machine generated code - do not edit ***
class CCESpline_color : public IColorEvaluator
{
public:
   CCESpline_color(CEvaluatorAllocator &allocator);
   virtual ~CCESpline_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xB44A1B7C;

   enum EProperties
   {
      kP_R                                             = 0x5767DF55,
      kP_G                                             = 0x3ABA3BBE,
      kP_B                                             = 0x4AD0CF31,
      kP_A                                             = 0xD3D99E8B,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   CSpline                                         mR;
   CSpline                                         mG;
   CSpline                                         mB;
   CSpline                                         mA;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CCESpline_color_declaration:6F8334AC *** Machine generated code - do not edit *** $End$:CCESpline_color_declaration:6F8334AC
};

