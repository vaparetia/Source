#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVEBuildFromFloat_vector(gvector3EvaluatorFactoriesEngine, CVEBuildFromFloat_vector::kType_0x029582BF, CVEBuildFromFloat_vector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVEBuildFromFloat_vector::CVEBuildFromFloat_vector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
//Start:CVEBuildFromFloat_vector_ctor:EFBEC492:15D54739 *** Machine generated code - do not edit ***
{
//End:CVEBuildFromFloat_vector_ctor:EFBEC492 *** Machine generated code - do not edit *** $End$:CVEBuildFromFloat_vector_ctor:EFBEC492
}

CVEBuildFromFloat_vector::~CVEBuildFromFloat_vector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVEBuildFromFloat_vector::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const value = mInput->GetValue(updateData);
   return CVector3(value, value, value);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVEBuildFromFloat_vector_body:3357EF25:D9E5FA6B *** Machine generated code - do not edit ***
uint32 const CVEBuildFromFloat_vector::kType_0x029582BF = 0x029582BF;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVEBuildFromFloat_vector::GetType() const
{
   return kType_0x029582BF;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVEBuildFromFloat_vector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVEBuildFromFloat_vector * pEval = new CVEBuildFromFloat_vector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVEBuildFromFloat_vector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVEBuildFromFloat_vector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVEBuildFromFloat_vector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CVEBuildFromFloat_vector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVEBuildFromFloat_vector::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
//End:CVEBuildFromFloat_vector_body:3357EF25 *** Machine generated code - do not edit *** $End$:CVEBuildFromFloat_vector_body:3357EF25
}
#endif //NO_EVALUATOR_INITIALIZER


