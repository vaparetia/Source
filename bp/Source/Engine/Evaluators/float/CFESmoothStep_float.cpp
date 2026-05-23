#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFESmoothStep_float(gfloatEvaluatorFactoriesEngine, CFESmoothStep_float::kType_0xF4DB3BA2, CFESmoothStep_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFESmoothStep_float::CFESmoothStep_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFESmoothStep_float_ctor:7365FE5B:15D54739 *** Machine generated code - do not edit ***
{
//End:CFESmoothStep_float_ctor:7365FE5B *** Machine generated code - do not edit *** $End$:CFESmoothStep_float_ctor:7365FE5B
}

CFESmoothStep_float::~CFESmoothStep_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFESmoothStep_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const input = MathUtils::ClampMinMax(mInput->GetValue(updateData), 0.0f, 1.0f);
   real32 const scale = mScale->GetValue(updateData);
   real32 const output = MathUtils::SmoothStep(input) * scale;

   return output;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFESmoothStep_float_body:AF8CD5EC:9D0308C9 *** Machine generated code - do not edit ***
uint32 const CFESmoothStep_float::kType_0xF4DB3BA2 = 0xF4DB3BA2;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFESmoothStep_float::GetType() const
{
   return kType_0xF4DB3BA2;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFESmoothStep_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFESmoothStep_float * pEval = new CFESmoothStep_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFESmoothStep_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Input:   // 0x19E91DD3
         {
            if (!applyProperties.UpdateOnly())
            {
               mInput.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mInput.get(), applyProperties, size, allocator);
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFESmoothStep_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFESmoothStep_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
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

void CFESmoothStep_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
   if (mScale) mScale->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFESmoothStep_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
   if (mScale.get()) mScale->Initialize(updateData);
//End:CFESmoothStep_float_body:AF8CD5EC *** Machine generated code - do not edit *** $End$:CFESmoothStep_float_body:AF8CD5EC
}
#endif //NO_EVALUATOR_INITIALIZER


