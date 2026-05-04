#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEPulse_float(gfloatEvaluatorFactoriesEngine, CFEPulse_float::kType_0xB000B63F, CFEPulse_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEPulse_float::CFEPulse_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFEPulse_float_ctor:603D6D91:D98915DB *** Machine generated code - do not edit ***
{
   mLoop = true;
//End:CFEPulse_float_ctor:603D6D91 *** Machine generated code - do not edit *** $End$:CFEPulse_float_ctor:603D6D91
}

CFEPulse_float::~CFEPulse_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEPulse_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 input = mInput->GetValue(updateData);
   real32 const onTime = mOnTime->GetValue(updateData);
   if (mLoop)
   {
      real32 const offTime = mOffTime->GetValue(updateData);
      input = (real32)fmodf(input, (onTime + offTime));
   }
   if (input > onTime) return mOffValue->GetValue(updateData);
   return mOnValue->GetValue(updateData);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEPulse_float_body:BCD44626:CA56DB93 *** Machine generated code - do not edit ***
uint32 const CFEPulse_float::kType_0xB000B63F = 0xB000B63F;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEPulse_float::GetType() const
{
   return kType_0xB000B63F;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEPulse_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEPulse_float * pEval = new CFEPulse_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEPulse_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_OnTime:   // 0x872C3851
         {
            if (!applyProperties.UpdateOnly())
            {
               mOnTime.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mOnTime.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_OnValue:   // 0x074534E9
         {
            if (!applyProperties.UpdateOnly())
            {
               mOnValue.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mOnValue.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_OffTime:   // 0x09125C23
         {
            if (!applyProperties.UpdateOnly())
            {
               mOffTime.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mOffTime.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_OffValue:   // 0xB9C01A9D
         {
            if (!applyProperties.UpdateOnly())
            {
               mOffValue.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mOffValue.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Loop:   // 0x016DB2D0
         {
            mLoop = applyProperties.mStream.ReadBool();
            break;
         }
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
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEPulse_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEPulse_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mOnTime)
   {
      uint32 const evalPtr = mOnTime->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mOnTime) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mOnValue)
   {
      uint32 const evalPtr = mOnValue->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mOnValue) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mOffTime)
   {
      uint32 const evalPtr = mOffTime->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mOffTime) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mOffValue)
   {
      uint32 const evalPtr = mOffValue->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mOffValue) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEPulse_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mOnTime) mOnTime->PostLoadUpdate();
   if (mOnValue) mOnValue->PostLoadUpdate();
   if (mOffTime) mOffTime->PostLoadUpdate();
   if (mOffValue) mOffValue->PostLoadUpdate();
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEPulse_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mOnTime.get()) mOnTime->Initialize(updateData);
   if (mOnValue.get()) mOnValue->Initialize(updateData);
   if (mOffTime.get()) mOffTime->Initialize(updateData);
   if (mOffValue.get()) mOffValue->Initialize(updateData);
   if (mInput.get()) mInput->Initialize(updateData);
//End:CFEPulse_float_body:BCD44626 *** Machine generated code - do not edit *** $End$:CFEPulse_float_body:BCD44626
}

#endif //NO_EVALUATOR_INITIALIZER

