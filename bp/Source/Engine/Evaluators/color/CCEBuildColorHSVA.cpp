#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEBuildColorHSVA(gcolorEvaluatorFactoriesEngine, CCEBuildColorHSVA::kType_0xE49CBDEB, CCEBuildColorHSVA::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEBuildColorHSVA::CCEBuildColorHSVA(CEvaluatorAllocator &allocator)
: IColorEvaluator()
//Start:CCEBuildColorHSVA_ctor:34F90EF8:15D54739 *** Machine generated code - do not edit ***
{
//End:CCEBuildColorHSVA_ctor:34F90EF8 *** Machine generated code - do not edit *** $End$:CCEBuildColorHSVA_ctor:34F90EF8
}

CCEBuildColorHSVA::~CCEBuildColorHSVA()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEBuildColorHSVA::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const s = mSaturation->GetValue(updateData);
   real32 const v = mValue->GetValue(updateData);
   real32 const a = mAlpha->GetValue(updateData);
   if (s < gkEpsilon32)
   {
      // Completely unsaturated
      return CColorf(v, v, v, a);
   }

   real32 const h = mHue->GetValue(updateData);

   return CColorf::FromHSV(h, s, v, a);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEBuildColorHSVA_body:E810254F:D02C1119 *** Machine generated code - do not edit ***
uint32 const CCEBuildColorHSVA::kType_0xE49CBDEB = 0xE49CBDEB;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEBuildColorHSVA::GetType() const
{
   return kType_0xE49CBDEB;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEBuildColorHSVA::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEBuildColorHSVA * pEval = new CCEBuildColorHSVA(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEBuildColorHSVA::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Hue:   // 0xEE581D0E
         {
            if (!applyProperties.UpdateOnly())
            {
               mHue.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mHue.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Saturation:   // 0x6577AE58
         {
            if (!applyProperties.UpdateOnly())
            {
               mSaturation.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mSaturation.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Value:   // 0xDCB67730
         {
            if (!applyProperties.UpdateOnly())
            {
               mValue.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mValue.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Alpha:   // 0x1121166E
         {
            if (!applyProperties.UpdateOnly())
            {
               mAlpha.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mAlpha.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEBuildColorHSVA.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEBuildColorHSVA::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mHue)
   {
      uint32 const evalPtr = mHue->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mHue) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mSaturation)
   {
      uint32 const evalPtr = mSaturation->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mSaturation) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mValue)
   {
      uint32 const evalPtr = mValue->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mValue) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mAlpha)
   {
      uint32 const evalPtr = mAlpha->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mAlpha) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CCEBuildColorHSVA::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mHue) mHue->PostLoadUpdate();
   if (mSaturation) mSaturation->PostLoadUpdate();
   if (mValue) mValue->PostLoadUpdate();
   if (mAlpha) mAlpha->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEBuildColorHSVA::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mHue.get()) mHue->Initialize(updateData);
   if (mSaturation.get()) mSaturation->Initialize(updateData);
   if (mValue.get()) mValue->Initialize(updateData);
   if (mAlpha.get()) mAlpha->Initialize(updateData);
//End:CCEBuildColorHSVA_body:E810254F *** Machine generated code - do not edit *** $End$:CCEBuildColorHSVA_body:E810254F
}

#endif //NO_EVALUATOR_INITIALIZER

