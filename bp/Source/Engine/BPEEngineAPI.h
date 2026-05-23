//------------------------------------------------------------------------------------------
// BPEEngineAPI.h
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

// Dynamically link to boost signals inside engine.
#ifndef BOOST_SIGNALS_DYN_LINK
#define BOOST_SIGNALS_DYN_LINK
#endif

#else

// Placeholder for now
#ifdef ENGINE_EXPORTS
#define ENGINE_API 
#else
#define ENGINE_API
#endif

// Boost signals are statically linked

#endif
