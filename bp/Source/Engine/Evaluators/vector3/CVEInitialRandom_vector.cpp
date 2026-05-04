#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVEInitialRandom_vector(gvector3EvaluatorFactoriesEngine, CVEInitialRandom_vector::kType_0x3B495042, CVEInitialRandom_vector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVEInitialRandom_vector::CVEInitialRandom_vector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
, mVector(allocator.AllocVector3())
//Start:CVEInitialRandom_vector_ctor:BF603E0E:15D54739 *** Machine generated code - do not edit ***
{
//End:CVEInitialRandom_vector_ctor:BF603E0E *** Machine generated code - do not edit *** $End$:CVEInitialRandom_vector_ctor:BF603E0E

}

CVEInitialRandom_vector::~CVEInitialRandom_vector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVEInitialRandom_vector::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return updateData.mDataBlock.GetValue(mVector);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVEInitialRandom_vector_body:638915B9:EF21892A *** Machine generated code - do not edit ***
uint32 const CVEInitialRandom_vector::kType_0x3B495042 = 0x3B495042;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVEInitialRandom_vector::GetType() const
{
   return kType_0x3B495042;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVEInitialRandom_vector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVEInitialRandom_vector * pEval = new CVEInitialRandom_vector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVEInitialRandom_vector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVEInitialRandom_vector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVEInitialRandom_vector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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

void CVEInitialRandom_vector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mMagnitude) mMagnitude->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVEInitialRandom_vector::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mMagnitude.get()) mMagnitude->Initialize(updateData);
//End:CVEInitialRandom_vector_body:638915B9 *** Machine generated code - do not edit *** $End$:CVEInitialRandom_vector_body:638915B9


   real32 const magnitude = mMagnitude->GetValue(updateData);
   CVector3 const vector(updateData.mpRandom->RandomVector3(magnitude));

   updateData.mDataBlock.SetValue(mVector, vector);

   return;
}

#endif //NO_EVALUATOR_INITIALIZER

