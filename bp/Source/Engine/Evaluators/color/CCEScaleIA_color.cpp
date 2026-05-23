#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEScaleIA_color(gcolorEvaluatorFactoriesEngine, CCEScaleIA_color::kType_0x218F68F5, CCEScaleIA_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEScaleIA_color::CCEScaleIA_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
//Start:CCEScaleIA_color_ctor:02E7501B:15D54739 *** Machine generated code - do not edit ***
{
//End:CCEScaleIA_color_ctor:02E7501B *** Machine generated code - do not edit *** $End$:CCEScaleIA_color_ctor:02E7501B
}

CCEScaleIA_color::~CCEScaleIA_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEScaleIA_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CColorf const color(mInputColor->GetValue(updateData));
   real32 const scaleRGB = mScaleRGB->GetValue(updateData);
   real32 const scaleAlpha = mScaleAlpha->GetValue(updateData);

   // No gamma correction, this is incorrect. Modify later to ensure scale is in linear space, not gamma space.
   return CColorf(color.GetR() * scaleRGB, color.GetG() * scaleRGB, color.GetB() * scaleRGB, color.GetA() * scaleAlpha);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEScaleIA_color_body:DE0E7BAC:8DC8D21B *** Machine generated code - do not edit ***
uint32 const CCEScaleIA_color::kType_0x218F68F5 = 0x218F68F5;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEScaleIA_color::GetType() const
{
   return kType_0x218F68F5;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEScaleIA_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEScaleIA_color * pEval = new CCEScaleIA_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEScaleIA_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_InputColor:   // 0xF682AF11
         {
            if (!applyProperties.UpdateOnly())
            {
               mInputColor.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mInputColor.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_ScaleRGB:   // 0xDF953F16
         {
            if (!applyProperties.UpdateOnly())
            {
               mScaleRGB.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mScaleRGB.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_ScaleAlpha:   // 0x0B2A1172
         {
            if (!applyProperties.UpdateOnly())
            {
               mScaleAlpha.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mScaleAlpha.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEScaleIA_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEScaleIA_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mInputColor)
   {
      uint32 const evalPtr = mInputColor->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInputColor) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mScaleRGB)
   {
      uint32 const evalPtr = mScaleRGB->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mScaleRGB) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mScaleAlpha)
   {
      uint32 const evalPtr = mScaleAlpha->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mScaleAlpha) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CCEScaleIA_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInputColor) mInputColor->PostLoadUpdate();
   if (mScaleRGB) mScaleRGB->PostLoadUpdate();
   if (mScaleAlpha) mScaleAlpha->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEScaleIA_color::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInputColor.get()) mInputColor->Initialize(updateData);
   if (mScaleRGB.get()) mScaleRGB->Initialize(updateData);
   if (mScaleAlpha.get()) mScaleAlpha->Initialize(updateData);
//End:CCEScaleIA_color_body:DE0E7BAC *** Machine generated code - do not edit *** $End$:CCEScaleIA_color_body:DE0E7BAC
}

#endif //NO_EVALUATOR_INITIALIZER

