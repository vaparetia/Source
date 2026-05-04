#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gmodifierPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCMPVEGravity_modifierPV(gmodifierPVEvaluatorFactoriesEngine, CMPVEGravity_modifierPV::kType_0xF121806A, CMPVEGravity_modifierPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CMPVEGravity_modifierPV::CMPVEGravity_modifierPV(CEvaluatorAllocator &allocator)
: IModifierPVEvaluator()
//Start:CMPVEGravity_modifierPV_ctor:8A9C8614:15D54739 *** Machine generated code - do not edit ***
{
//End:CMPVEGravity_modifierPV_ctor:8A9C8614 *** Machine generated code - do not edit *** $End$:CMPVEGravity_modifierPV_ctor:8A9C8614
}

CMPVEGravity_modifierPV::~CMPVEGravity_modifierPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

uint32 CMPVEGravity_modifierPV::ApplyModifier(CEvaluatorUpdateData const &updateData, void *pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const
{
   (pTempData);
   (position);
   // Need to apply pow function to deltaTime
   CVector3 gravity = mGravity->GetValue(updateData) * deltaTime;

   velocity += gravity;

   return kMF_None;}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CMPVEGravity_modifierPV_body:5675ADA3:24A31503 *** Machine generated code - do not edit ***
uint32 const CMPVEGravity_modifierPV::kType_0xF121806A = 0xF121806A;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CMPVEGravity_modifierPV::GetType() const
{
   return kType_0xF121806A;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CMPVEGravity_modifierPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CMPVEGravity_modifierPV * pEval = new CMPVEGravity_modifierPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CMPVEGravity_modifierPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Gravity:   // 0x95A01C3B
         {
            if (!applyProperties.UpdateOnly())
            {
               mGravity.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mGravity.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CMPVEGravity_modifierPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CMPVEGravity_modifierPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mGravity)
   {
      uint32 const evalPtr = mGravity->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mGravity) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CMPVEGravity_modifierPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mGravity) mGravity->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CMPVEGravity_modifierPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mGravity.get()) mGravity->Initialize(updateData);
//End:CMPVEGravity_modifierPV_body:5675ADA3 *** Machine generated code - do not edit *** $End$:CMPVEGravity_modifierPV_body:5675ADA3
}

#endif //NO_EVALUATOR_INITIALIZER

