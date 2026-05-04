#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVEBuildVector(gvector3EvaluatorFactoriesEngine, CVEBuildVector::kType_0x50BE592D, CVEBuildVector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVEBuildVector::CVEBuildVector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
//Start:CVEBuildVector_ctor:4B1C84F3:15D54739 *** Machine generated code - do not edit ***
{
//End:CVEBuildVector_ctor:4B1C84F3 *** Machine generated code - do not edit *** $End$:CVEBuildVector_ctor:4B1C84F3

}

CVEBuildVector::~CVEBuildVector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVEBuildVector::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return CVector3(mX->GetValue(updateData), 
                   mY->GetValue(updateData), 
                   mZ->GetValue(updateData));
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVEBuildVector_body:97F5AF44:21006566 *** Machine generated code - do not edit ***
uint32 const CVEBuildVector::kType_0x50BE592D = 0x50BE592D;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVEBuildVector::GetType() const
{
   return kType_0x50BE592D;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVEBuildVector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVEBuildVector * pEval = new CVEBuildVector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVEBuildVector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_X:   // 0xB7B2364B
         {
            if (!applyProperties.UpdateOnly())
            {
               mX.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mX.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Y:   // 0xC0B506DD
         {
            if (!applyProperties.UpdateOnly())
            {
               mY.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mY.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Z:   // 0x59BC5767
         {
            if (!applyProperties.UpdateOnly())
            {
               mZ.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mZ.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVEBuildVector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVEBuildVector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mX)
   {
      uint32 const evalPtr = mX->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mX) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mY)
   {
      uint32 const evalPtr = mY->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mY) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mZ)
   {
      uint32 const evalPtr = mZ->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mZ) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CVEBuildVector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mX) mX->PostLoadUpdate();
   if (mY) mY->PostLoadUpdate();
   if (mZ) mZ->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVEBuildVector::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mX.get()) mX->Initialize(updateData);
   if (mY.get()) mY->Initialize(updateData);
   if (mZ.get()) mZ->Initialize(updateData);
//End:CVEBuildVector_body:97F5AF44 *** Machine generated code - do not edit *** $End$:CVEBuildVector_body:97F5AF44

}

#endif //NO_EVALUATOR_INITIALIZER

