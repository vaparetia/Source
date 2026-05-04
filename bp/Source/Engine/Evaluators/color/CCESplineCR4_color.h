#pragma once

//Start:CCESplineCR4_color_declaration:1E884822:496F0E80 *** Machine generated code - do not edit ***
class CCESplineCR4_color : public IColorEvaluator
{
public:
   CCESplineCR4_color(CEvaluatorAllocator &allocator);
   virtual ~CCESplineCR4_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x471A3631;

   enum EProperties
   {
      kP_Time1                                         = 0x44C4E134,
      kP_Color1                                        = 0x5DC0E615,
      kP_Time2                                         = 0xDDCDB08E,
      kP_Color2                                        = 0xC4C9B7AF,
      kP_Time3                                         = 0xAACA8018,
      kP_Color3                                        = 0xB3CE8739,
      kP_Time4                                         = 0x34AE15BB,
      kP_Color4                                        = 0x2DAA129A,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mTime1;
   boost::shared_ptr<IColorEvaluator>              mColor1;
   boost::shared_ptr<IFloatEvaluator>              mTime2;
   boost::shared_ptr<IColorEvaluator>              mColor2;
   boost::shared_ptr<IFloatEvaluator>              mTime3;
   boost::shared_ptr<IColorEvaluator>              mColor3;
   boost::shared_ptr<IFloatEvaluator>              mTime4;
   boost::shared_ptr<IColorEvaluator>              mColor4;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CCESplineCR4_color_declaration:1E884822 *** Machine generated code - do not edit *** $End$:CCESplineCR4_color_declaration:1E884822
};

