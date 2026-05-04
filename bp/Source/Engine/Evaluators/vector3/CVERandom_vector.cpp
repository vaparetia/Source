#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVERandom_vector(gvector3EvaluatorFactoriesEngine, CVERandom_vector::kType_0xB8FE7652, CVERandom_vector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVERandom_vector::CVERandom_vector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
//Start:CVERandom_vector_ctor:4EDA7B3F:15D54739 *** Machine generated code - do not edit ***
{
//End:CVERandom_vector_ctor:4EDA7B3F *** Machine generated code - do not edit *** $End$:CVERandom_vector_ctor:4EDA7B3F

}

CVERandom_vector::~CVERandom_vector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVERandom_vector::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const magnitude = mMagnitude->GetValue(updateData);
   return updateData.mpRandom->RandomVector3(magnitude);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVERandom_vector_body:92335088:92B7F863 *** Machine generated code - do not edit ***
uint32 const CVERandom_vector::kType_0xB8FE7652 = 0xB8FE7652;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVERandom_vector::GetType() const
{
   return kType_0xB8FE7652;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVERandom_vector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVERandom_vector * pEval = new CVERandom_vector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVERandom_vector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Magnitude:   // 0xB776FFA2
         {
            if (!applyProperties.UpdateOnly())
            {
               mMagnitude.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mMagnitude.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVERandom_vector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVERandom_vector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mMagnitude)
   {
      uint32 const evalPtr = mMagnitude->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mMagnitude) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CVERandom_vector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mMagnitude) mMagnitude->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVERandom_vector::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mMagnitude.get()) mMagnitude->Initialize(updateData);
//End:CVERandom_vector_body:92335088 *** Machine generated code - do not edit *** $End$:CVERandom_vector_body:92335088

}

#endif //NO_EVALUATOR_INITIALIZER

