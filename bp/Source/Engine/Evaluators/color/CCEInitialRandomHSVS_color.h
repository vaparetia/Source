#pragma once

//Start:CCEInitialRandomHSVS_color_declaration:090400E4:8B1283A9 *** Machine generated code - do not edit ***
class CCEInitialRandomHSVS_color : public IColorEvaluator
{
public:
   CCEInitialRandomHSVS_color(CEvaluatorAllocator &allocator);
   virtual ~CCEInitialRandomHSVS_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x809A4ADC;

   enum EProperties
   {
      kP_HueCenter                                     = 0x48E306B6,
      kP_HueRandom                                     = 0x1E283747,
      kP_SatMinRandom                                  = 0xC56D34B1,
      kP_SatMaxRandom                                  = 0x128686F0,
      kP_ValueMinRandom                                = 0xD351FC03,
      kP_ValueMaxRandom                                = 0x04BA4E42,
      kP_AlphaMinRandom                                = 0xF299831A,
      kP_AlphaMaxRandom                                = 0x2572315B,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mHueCenter;
   float                                           mHueRandom;
   float                                           mSatMinRandom;
   float                                           mSatMaxRandom;
   float                                           mValueMinRandom;
   float                                           mValueMaxRandom;
   float                                           mAlphaMinRandom;
   float                                           mAlphaMaxRandom;
//End:CCEInitialRandomHSVS_color_declaration:090400E4 *** Machine generated code - do not edit *** $End$:CCEInitialRandomHSVS_color_declaration:090400E4
   CEvalColorfData                                 mInitialValue;
};

