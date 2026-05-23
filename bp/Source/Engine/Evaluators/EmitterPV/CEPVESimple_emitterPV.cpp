#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gemitterPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCEPVESimple_emitterPV(gemitterPVEvaluatorFactoriesEngine, CEPVESimple_emitterPV::kType_0x409A437A, CEPVESimple_emitterPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CEPVESimple_emitterPV::CEPVESimple_emitterPV(CEvaluatorAllocator &allocator)
: IEmitterPVEvaluator()
//Start:CEPVESimple_emitterPV_ctor:6C262167:15D54739 *** Machine generated code - do not edit ***
{
//End:CEPVESimple_emitterPV_ctor:6C262167 *** Machine generated code - do not edit *** $End$:CEPVESimple_emitterPV_ctor:6C262167
}

CEPVESimple_emitterPV::~CEPVESimple_emitterPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

void CEPVESimple_emitterPV::GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const
{
   if (mPosition)
   {
      position = mPosition->GetValue(updateData);
   }
   else
   {
      position = CVector3::Zero();
   }

   if (mVelocity)
   {
      velocity = mVelocity->GetValue(updateData);
   }
   else
   {
      velocity = CVector3::Zero();
   }
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CEPVESimple_emitterPV_body:B0CF0AD0:9D68C03D *** Machine generated code - do not edit ***
uint32 const CEPVESimple_emitterPV::kType_0x409A437A = 0x409A437A;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CEPVESimple_emitterPV::GetType() const
{
   return kType_0x409A437A;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CEPVESimple_emitterPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CEPVESimple_emitterPV * pEval = new CEPVESimple_emitterPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CEPVESimple_emitterPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Position:   // 0xBF5A86A3
         {
            if (!applyProperties.UpdateOnly())
            {
               mPosition.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mPosition.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Velocity:   // 0x47AEB4FF
         {
            if (!applyProperties.UpdateOnly())
            {
               mVelocity.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mVelocity.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CEPVESimple_emitterPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CEPVESimple_emitterPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mPosition)
   {
      uint32 const evalPtr = mPosition->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mPosition) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mVelocity)
   {
      uint32 const evalPtr = mVelocity->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mVelocity) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CEPVESimple_emitterPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mPosition) mPosition->PostLoadUpdate();
   if (mVelocity) mVelocity->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CEPVESimple_emitterPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mPosition.get()) mPosition->Initialize(updateData);
   if (mVelocity.get()) mVelocity->Initialize(updateData);
//End:CEPVESimple_emitterPV_body:B0CF0AD0 *** Machine generated code - do not edit *** $End$:CEPVESimple_emitterPV_body:B0CF0AD0
}

#endif //NO_EVALUATOR_INITIALIZER

