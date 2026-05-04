//----------------------------------------------------------------------------
// IResourcePool.cpp
// Bluepoint
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "IResourcePool.h"

//----------------------------------------------------------------------------
// Global for loading property resources via PostLoadUpdate();

IResourcePool *     gpResources = NULL;

//----------------------------------------------------------------------------

IResourcePool::~IResourcePool()
{
}