#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gmodifierPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCMPVEWind_modifierPV(gmodifierPVEvaluatorFactoriesEngine, CMPVEWind_modifierPV::kType_0x103ABDFE, CMPVEWind_modifierPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CMPVEWind_modifierPV::CMPVEWind_modifierPV(CEvaluatorAllocator &allocator)
: IModifierPVEvaluator()
//Start:CMPVEWind_modifierPV_ctor:395CB907:15D54739 *** Machine generated code - do not edit ***
{
//End:CMPVEWind_modifierPV_ctor:395CB907 *** Machine generated code - do not edit *** $End$:CMPVEWind_modifierPV_ctor:395CB907
}

CMPVEWind_modifierPV::~CMPVEWind_modifierPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

uint32 CMPVEWind_modifierPV::ApplyModifier(CEvaluatorUpdateData const &updateData, void *pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const
{
   (pTempData);
   // Need to apply pow function to deltaTime
   real32 const lerpAmount = mWindDrag->GetValue(updateData) * deltaTime;

   CVector3 wind(mWindSpeed->GetValue(updateData));

   CVector3 const newVelocity(CVector3::Lerp(velocity, wind, lerpAmount));
   velocity = newVelocity;

   return kMF_None;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CMPVEWind_modifierPV_body:E5B592B0:BF81DDF3 *** Machine generated code - do not edit ***
uint32 const CMPVEWind_modifierPV::kType_0x103ABDFE = 0x103ABDFE;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CMPVEWind_modifierPV::GetType() const
{
   return kType_0x103ABDFE;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CMPVEWind_modifierPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CMPVEWind_modifierPV * pEval = new CMPVEWind_modifierPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CMPVEWind_modifierPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_WindSpeed:   // 0x7FB7DF9A
         {
            if (!applyProperties.UpdateOnly())
            {
               mWindSpeed.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mWindSpeed.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_WindDrag:   // 0x96904E6E
         {
            if (!applyProperties.UpdateOnly())
            {
               mWindDrag.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mWindDrag.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CMPVEWind_modifierPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CMPVEWind_modifierPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mWindSpeed)
   {
      uint32 const evalPtr = mWindSpeed->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mWindSpeed) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mWindDrag)
   {
      uint32 const evalPtr = mWindDrag->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mWindDrag) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CMPVEWind_modifierPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mWindSpeed) mWindSpeed->PostLoadUpdate();
   if (mWindDrag) mWindDrag->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CMPVEWind_modifierPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mWindSpeed.get()) mWindSpeed->Initialize(updateData);
   if (mWindDrag.get()) mWindDrag->Initialize(updateData);
//End:CMPVEWind_modifierPV_body:E5B592B0 *** Machine generated code - do not edit *** $End$:CMPVEWind_modifierPV_body:E5B592B0
}

#endif //NO_EVALUATOR_INITIALIZER

