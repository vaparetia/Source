#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFERandomV_float(gfloatEvaluatorFactoriesEngine, CFERandomV_float::kType_0x7595C9D3, CFERandomV_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFERandomV_float::CFERandomV_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFERandomV_float_ctor:5D5D9A36:15D54739 *** Machine generated code - do not edit ***
{
//End:CFERandomV_float_ctor:5D5D9A36 *** Machine generated code - do not edit *** $End$:CFERandomV_float_ctor:5D5D9A36
}

CFERandomV_float::~CFERandomV_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFERandomV_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const centerValue = mCenterValue->GetValue(updateData);
   real32 const randomVariation = mRandomVariation->GetValue(updateData);
   real32 const value = centerValue + ((updateData.mpRandom->RandomFloat() - 0.5f) * 2.0f * randomVariation);

   return value;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFERandomV_float_body:81B4B181:83FB541E *** Machine generated code - do not edit ***
uint32 const CFERandomV_float::kType_0x7595C9D3 = 0x7595C9D3;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFERandomV_float::GetType() const
{
   return kType_0x7595C9D3;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFERandomV_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFERandomV_float * pEval = new CFERandomV_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFERandomV_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_CenterValue:   // 0x2AA88BCF
         {
            if (!applyProperties.UpdateOnly())
            {
               mCenterValue.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mCenterValue.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_RandomVariation:   // 0xDE09F15B
         {
            if (!applyProperties.UpdateOnly())
            {
               mRandomVariation.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mRandomVariation.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFERandomV_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFERandomV_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mCenterValue)
   {
      uint32 const evalPtr = mCenterValue->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mCenterValue) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mRandomVariation)
   {
      uint32 const evalPtr = mRandomVariation->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mRandomVariation) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFERandomV_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mCenterValue) mCenterValue->PostLoadUpdate();
   if (mRandomVariation) mRandomVariation->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFERandomV_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mCenterValue.get()) mCenterValue->Initialize(updateData);
   if (mRandomVariation.get()) mRandomVariation->Initialize(updateData);
//End:CFERandomV_float_body:81B4B181 *** Machine generated code - do not edit *** $End$:CFERandomV_float_body:81B4B181
}

#endif //NO_EVALUATOR_INITIALIZER

