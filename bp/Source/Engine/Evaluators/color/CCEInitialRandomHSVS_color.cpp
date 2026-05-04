#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEInitialRandomHSVS_color(gcolorEvaluatorFactoriesEngine, CCEInitialRandomHSVS_color::kType_0x809A4ADC, CCEInitialRandomHSVS_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEInitialRandomHSVS_color::CCEInitialRandomHSVS_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
, mInitialValue(allocator.AllocCColorf())
//Start:CCEInitialRandomHSVS_color_ctor:C23E53E9:E8BB41BB *** Machine generated code - do not edit ***
{
   mHueCenter = 180.0f;
   mHueRandom = 180.0f;
   mSatMinRandom = 0.0f;
   mSatMaxRandom = 1.0f;
   mValueMinRandom = 0.0f;
   mValueMaxRandom = 1.0f;
   mAlphaMinRandom = 0.0f;
   mAlphaMaxRandom = 1.0f;
//End:CCEInitialRandomHSVS_color_ctor:C23E53E9 *** Machine generated code - do not edit *** $End$:CCEInitialRandomHSVS_color_ctor:C23E53E9
}

CCEInitialRandomHSVS_color::~CCEInitialRandomHSVS_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEInitialRandomHSVS_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CColorf const &color = updateData.mDataBlock.GetValue(mInitialValue);
   return color;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEInitialRandomHSVS_color_body:1ED7785E:03742D9E *** Machine generated code - do not edit ***
uint32 const CCEInitialRandomHSVS_color::kType_0x809A4ADC = 0x809A4ADC;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEInitialRandomHSVS_color::GetType() const
{
   return kType_0x809A4ADC;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEInitialRandomHSVS_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEInitialRandomHSVS_color * pEval = new CCEInitialRandomHSVS_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEInitialRandomHSVS_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_HueCenter:   // 0x48E306B6
         {
            mHueCenter = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_HueRandom:   // 0x1E283747
         {
            mHueRandom = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_SatMinRandom:   // 0xC56D34B1
         {
            mSatMinRandom = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_SatMaxRandom:   // 0x128686F0
         {
            mSatMaxRandom = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_ValueMinRandom:   // 0xD351FC03
         {
            mValueMinRandom = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_ValueMaxRandom:   // 0x04BA4E42
         {
            mValueMaxRandom = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_AlphaMinRandom:   // 0xF299831A
         {
            mAlphaMinRandom = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_AlphaMaxRandom:   // 0x2572315B
         {
            mAlphaMaxRandom = applyProperties.mStream.ReadReal32();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEInitialRandomHSVS_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEInitialRandomHSVS_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CCEInitialRandomHSVS_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEInitialRandomHSVS_color::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CCEInitialRandomHSVS_color_body:1ED7785E *** Machine generated code - do not edit *** $End$:CCEInitialRandomHSVS_color_body:1ED7785E

   // HSV chosen as there is a HSV color wheel in maya.

   CRandom &rand = *updateData.mpRandom;
   real32 const s = rand.RangeFloat(mSatMinRandom, mSatMaxRandom);
   real32 const v = rand.RangeFloat(mValueMinRandom, mValueMaxRandom);
   real32 const a = rand.RangeFloat(mAlphaMinRandom, mAlphaMaxRandom);
   if (s < gkEpsilon32)
   {
      // Completely unsaturated
      updateData.mDataBlock.SetValue(mInitialValue, CColorf(v, v, v, a));
      return;
   }

   real32 h = rand.RangeFloat(mHueCenter - mHueRandom, mHueCenter + mHueRandom);
   // Deal with wrap around
   h = (real32) fmodf(h, 360.0f);
   while (h < 0.0f) h += 360.0f;

   CColorf const color(CColorf::FromHSV(h, s, v, a));
   updateData.mDataBlock.SetValue(mInitialValue, color);
}

#endif //NO_EVALUATOR_INITIALIZER

