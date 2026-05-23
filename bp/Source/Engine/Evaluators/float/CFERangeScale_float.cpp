#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFERangeScale_float(gfloatEvaluatorFactoriesEngine, CFERangeScale_float::kType_0xE0B17143, CFERangeScale_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFERangeScale_float::CFERangeScale_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFERangeScale_float_ctor:F99631CE:B55CBB2D *** Machine generated code - do not edit ***
{
   mClampOutputRange = true;
//End:CFERangeScale_float_ctor:F99631CE *** Machine generated code - do not edit *** $End$:CFERangeScale_float_ctor:F99631CE
}

CFERangeScale_float::~CFERangeScale_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFERangeScale_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const input = mInput->GetValue(updateData);
   real32 const inputMin = mInputMin->GetValue(updateData);
   real32 const inputMax = mInputMax->GetValue(updateData);
   real32 const outputMin = mOutputMin->GetValue(updateData);
   real32 const outputMax = mOutputMax->GetValue(updateData);

   real32 scaledInput = (input - inputMin) / (inputMax - inputMin);
   if (mClampOutputRange)
   {
      scaledInput = bpe::max_val(0.0f, scaledInput);
      scaledInput = bpe::min_val(scaledInput, 1.0f);
   }
   real32 const output = outputMin + scaledInput * (outputMax - outputMin);
   return output;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFERangeScale_float_body:257F1A79:21873775 *** Machine generated code - do not edit ***
uint32 const CFERangeScale_float::kType_0xE0B17143 = 0xE0B17143;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFERangeScale_float::GetType() const
{
   return kType_0xE0B17143;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFERangeScale_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFERangeScale_float * pEval = new CFERangeScale_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFERangeScale_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
         case kP_InputMin:   // 0x3043E680
         {
            if (!applyProperties.UpdateOnly())
            {
               mInputMin.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mInputMin.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_InputMax:   // 0x0C4ED9D9
         {
            if (!applyProperties.UpdateOnly())
            {
               mInputMax.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mInputMax.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_OutputMin:   // 0xA38F6423
         {
            if (!applyProperties.UpdateOnly())
            {
               mOutputMin.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mOutputMin.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_OutputMax:   // 0x9F825B7A
         {
            if (!applyProperties.UpdateOnly())
            {
               mOutputMax.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mOutputMax.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_ClampOutputRange:   // 0x7EC92266
         {
            mClampOutputRange = applyProperties.mStream.ReadBool();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFERangeScale_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFERangeScale_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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
   if (mInputMin)
   {
      uint32 const evalPtr = mInputMin->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInputMin) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mInputMax)
   {
      uint32 const evalPtr = mInputMax->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInputMax) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mOutputMin)
   {
      uint32 const evalPtr = mOutputMin->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mOutputMin) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mOutputMax)
   {
      uint32 const evalPtr = mOutputMax->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mOutputMax) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFERangeScale_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
   if (mInputMin) mInputMin->PostLoadUpdate();
   if (mInputMax) mInputMax->PostLoadUpdate();
   if (mOutputMin) mOutputMin->PostLoadUpdate();
   if (mOutputMax) mOutputMax->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFERangeScale_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
   if (mInputMin.get()) mInputMin->Initialize(updateData);
   if (mInputMax.get()) mInputMax->Initialize(updateData);
   if (mOutputMin.get()) mOutputMin->Initialize(updateData);
   if (mOutputMax.get()) mOutputMax->Initialize(updateData);
//End:CFERangeScale_float_body:257F1A79 *** Machine generated code - do not edit *** $End$:CFERangeScale_float_body:257F1A79
}
#endif //NO_EVALUATOR_INITIALIZER


