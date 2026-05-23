#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEScaleRGBA_color(gcolorEvaluatorFactoriesEngine, CCEScaleRGBA_color::kType_0xB44FB946, CCEScaleRGBA_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEScaleRGBA_color::CCEScaleRGBA_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
//Start:CCEScaleRGBA_color_ctor:926C8631:15D54739 *** Machine generated code - do not edit ***
{
//End:CCEScaleRGBA_color_ctor:926C8631 *** Machine generated code - do not edit *** $End$:CCEScaleRGBA_color_ctor:926C8631
}

CCEScaleRGBA_color::~CCEScaleRGBA_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEScaleRGBA_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CColorf const color(mInputColor->GetValue(updateData));
   real32 const scaleRGBA = mScale->GetValue(updateData);

   // No gamma correction, this is incorrect. Modify later to ensure scale is in linear space, not gamma space.
   return CColorf(color.GetR() * scaleRGBA, color.GetG() * scaleRGBA, color.GetB() * scaleRGBA, color.GetA() * scaleRGBA);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEScaleRGBA_color_body:4E85AD86:D194F90E *** Machine generated code - do not edit ***
uint32 const CCEScaleRGBA_color::kType_0xB44FB946 = 0xB44FB946;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEScaleRGBA_color::GetType() const
{
   return kType_0xB44FB946;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEScaleRGBA_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEScaleRGBA_color * pEval = new CCEScaleRGBA_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEScaleRGBA_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
         case kP_Scale:   // 0x2D870A80
         {
            if (!applyProperties.UpdateOnly())
            {
               mScale.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mScale.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEScaleRGBA_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEScaleRGBA_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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
   if (mScale)
   {
      uint32 const evalPtr = mScale->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mScale) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CCEScaleRGBA_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInputColor) mInputColor->PostLoadUpdate();
   if (mScale) mScale->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEScaleRGBA_color::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInputColor.get()) mInputColor->Initialize(updateData);
   if (mScale.get()) mScale->Initialize(updateData);
//End:CCEScaleRGBA_color_body:4E85AD86 *** Machine generated code - do not edit *** $End$:CCEScaleRGBA_color_body:4E85AD86
}
#endif //NO_EVALUATOR_INITIALIZER


