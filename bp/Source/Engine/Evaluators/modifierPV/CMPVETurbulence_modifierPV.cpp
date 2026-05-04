#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gmodifierPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCMPVETurbulence_modifierPV(gmodifierPVEvaluatorFactoriesEngine, CMPVETurbulence_modifierPV::kType_0xF2204526, CMPVETurbulence_modifierPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CMPVETurbulence_modifierPV::CMPVETurbulence_modifierPV(CEvaluatorAllocator &allocator)
: IModifierPVEvaluator()
//Start:CMPVETurbulence_modifierPV_ctor:BFE28324:15D54739 *** Machine generated code - do not edit ***
{
//End:CMPVETurbulence_modifierPV_ctor:BFE28324 *** Machine generated code - do not edit *** $End$:CMPVETurbulence_modifierPV_ctor:BFE28324
}

CMPVETurbulence_modifierPV::~CMPVETurbulence_modifierPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

uint32 CMPVETurbulence_modifierPV::ApplyModifier(CEvaluatorUpdateData const &updateData, void *pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const
{
   (pTempData);
   // Need to apply pow function to deltaTime
   real32 const strength = mStrength->GetValue(updateData) * deltaTime;
   real32 const frequency = mFrequency->GetValue(updateData);
   real32 const time = mTime->GetValue(updateData);

   CVector3 turbulence = CPerlinNoise::PGNoise4(position * frequency, time) * strength;

   //CVector3 const newVelocity(CVector3::Lerp(velocity, turbulence, lerpAmount));
   velocity += turbulence;

   return kMF_None;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CMPVETurbulence_modifierPV_body:630BA893:94EA68A9 *** Machine generated code - do not edit ***
uint32 const CMPVETurbulence_modifierPV::kType_0xF2204526 = 0xF2204526;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CMPVETurbulence_modifierPV::GetType() const
{
   return kType_0xF2204526;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CMPVETurbulence_modifierPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CMPVETurbulence_modifierPV * pEval = new CMPVETurbulence_modifierPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CMPVETurbulence_modifierPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Strength:   // 0x2FCF838E
         {
            if (!applyProperties.UpdateOnly())
            {
               mStrength.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mStrength.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Frequency:   // 0xA48E3AB0
         {
            if (!applyProperties.UpdateOnly())
            {
               mFrequency.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mFrequency.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Time:   // 0xCFA6377B
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CMPVETurbulence_modifierPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CMPVETurbulence_modifierPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mStrength)
   {
      uint32 const evalPtr = mStrength->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mStrength) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mFrequency)
   {
      uint32 const evalPtr = mFrequency->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mFrequency) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mTime)
   {
      uint32 const evalPtr = mTime->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CMPVETurbulence_modifierPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mStrength) mStrength->PostLoadUpdate();
   if (mFrequency) mFrequency->PostLoadUpdate();
   if (mTime) mTime->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CMPVETurbulence_modifierPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mStrength.get()) mStrength->Initialize(updateData);
   if (mFrequency.get()) mFrequency->Initialize(updateData);
   if (mTime.get()) mTime->Initialize(updateData);
//End:CMPVETurbulence_modifierPV_body:630BA893 *** Machine generated code - do not edit *** $End$:CMPVETurbulence_modifierPV_body:630BA893
}

#endif //NO_EVALUATOR_INITIALIZER

