//------------------------------------------------------------------------------------------
// EvaluatorUtils.cpp
// Helper functions for evaluators
// Bluepoint
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "Engine/Evaluators/EvaluatorUtils.h"

//------------------------------------------------------------------------------------------

namespace EvaluatorUtils
{

//------------------------------------------------------------------------------------------

static int const skMaxUserDataBranchChildren = 4;

typedef bpe::reserved_vector<IUserDataEvaluator const *, 4> TUserDataBranchChildren;

//------------------------------------------------------------------------------------------

static void GetBranchChildren(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, TUserDataBranchChildren &children_Out)
{
   BPE_VERIFY(pEvaluator->GetUserDataType() == IUserDataEvaluator::kDT_Branch, false, "Not a branch.");
   
   struct
   {
      uint32                     mCount;
      IUserDataEvaluator const * mpChildEvaluators[skMaxUserDataBranchChildren];
   } branchChildren;
   
   pEvaluator->GetUserData(updateData, &branchChildren);
   for (int loop = 0; loop < branchChildren.mCount; loop++)
   {
      children_Out.push_back(branchChildren.mpChildEvaluators[loop]);
   }   
}

//------------------------------------------------------------------------------------------

static bool GetFloatFromUserData_Internal(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier, real32 * pFloat)
{
   if (pEvaluator->GetUserDataType() == IUserDataEvaluator::kDT_Float)
   {
      if (strcmp(pEvaluator->GetIdentifier(), pIdentifier) == 0)
      {
         pEvaluator->GetUserData(updateData, pFloat);
         return true;      
      }
   }
   if (pEvaluator->GetUserDataType() == IUserDataEvaluator::kDT_Branch)
   {
      TUserDataBranchChildren children;
      GetBranchChildren(updateData, pEvaluator, children);
      foreach(IUserDataEvaluator const * pChildEvaluator, children)
      {
         bool const bHasValue = GetFloatFromUserData_Internal(updateData, pChildEvaluator, pIdentifier, pFloat);
         if (bHasValue)
         {
            // We have a match from the child
            return true;
         }
      }
   }
   // This evaluator tree had no matches for identifier and data type.
   return false;
}

//------------------------------------------------------------------------------------------

boost::optional<real32> GetFloatFromUserData(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier)
{
   real32 floatVal = gkReal32Max;
   bool const bGotValue = GetFloatFromUserData_Internal(updateData, pEvaluator, pIdentifier, &floatVal);
   if (bGotValue)
   {
      return floatVal;
   }
   return boost::optional<real32>();
}

//------------------------------------------------------------------------------------------

real32 GetFloatFromUserData_Always(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier)
{
   real32 floatVal = 0.0f;
   bool const bGotValue = GetFloatFromUserData_Internal(updateData, pEvaluator, pIdentifier, &floatVal);
   if (bGotValue)
   {
      return floatVal;
   }
   // Couldn't find a match, just return default zero.
   return 0.0f;
}

//------------------------------------------------------------------------------------------

char const * GetStringFromUserData(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier)
{
   char * pString = NULL;
   if (pEvaluator->GetUserDataType() == IUserDataEvaluator::kDT_String)
   {
      if (strcmp(pEvaluator->GetIdentifier(), pIdentifier) == 0)
      {
         pEvaluator->GetUserData(updateData, &pString);
         return pString;      
      }
   }
   if (pEvaluator->GetUserDataType() == IUserDataEvaluator::kDT_Branch)
   {
      TUserDataBranchChildren children;
      GetBranchChildren(updateData, pEvaluator, children);
      foreach(IUserDataEvaluator const * pChildEvaluator, children)
      {
         char const * pString = GetStringFromUserData(updateData, pChildEvaluator, pIdentifier);
         if (pString)
         {
            // We have a match from the child
            return pString;
         }
      }
   }
   // This evaluator tree had no matches for identifier and data type.   
   return NULL;
}

//------------------------------------------------------------------------------------------

char const * GetStringFromUserData_Always(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier)
{
   char const * pString = GetStringFromUserData(updateData, pEvaluator, pIdentifier);
   if (pString)
   {
      return pString;
   }
   // This evaluator tree had no matches for identifier and data type.
   // Just return an empty string
   static char const * sEmptyString = "";
   return sEmptyString;   
}   

//------------------------------------------------------------------------------------------

} // namespace EvaluatorUtils

