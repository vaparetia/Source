//------------------------------------------------------------------------------------------
// BPEAssert.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEConstants.h"
#include "Engine/Basics/BPEDebugFuncs.h"

//------------------------------------------------------------------------------------------

#define BPE_SET_ASSERT_LOG_FUNC( fptr )   bpe_set_assert_log_func( fptr )
#define BPE_SET_ASSERT_POPUP_FUNC( fptr ) bpe_set_assert_popup_func( fptr )
#define BPE_RESET_ASSERT_FUNCS( dummy )   bpe_reset_assert_funcs()

//------------------------------------------------------------------------------------------
// runtime assert

#ifndef BPE_DEBUG

#define BPE_ASSERT( test, msg )  ((void)0)
#define BPE_ASSERT_NO_MSG( test )  ((void)0)
#define BPE_ASSERTA( msg )       ((void)0)

#else

// Assert with a message
#define BPE_ASSERT( test, msg )\
{\
   const bool TestResults(test);\
   if( !(TestResults) )\
   {\
      bpe_log_assert_failure( __FILE__, __LINE__, "Assertion", #test, msg );\
      if( bpe_query_to_halt_program( __FILE__, __LINE__, "Assertion", #test, msg ) )\
      {\
         BPE_SET_BREAKPOINT;\
      }\
   }\
}                                                                        

// Assert without a message
#define BPE_ASSERT_NO_MSG( test )\
{\
   const bool TestResults(test);\
   if( !(TestResults) )\
   {\
      bpe_log_assert_failure( __FILE__, __LINE__, "Assertion", #test, "No Msg" );\
      if( bpe_query_to_halt_program( __FILE__, __LINE__, "Assertion", #test, "No Msg" ) )\
      {\
         BPE_SET_BREAKPOINT;\
      }\
   }\
}                                                                        

// Always assert
#define BPE_ASSERTA( msg )\
{\
   {\
      bpe_log_assert_failure( __FILE__, __LINE__, "Assertion", "Always", msg );\
      if( bpe_query_to_halt_program( __FILE__, __LINE__, "Assertion", "Always", msg ) )\
      {\
         BPE_SET_BREAKPOINT;\
      }\
   }\
}                                                                        


#endif // BPE_DEBUG

//------------------------------------------------------------------------------------------
// compile time assert

#define BPE_CTASSERT(cond)    int BPE_CTASSERT( int compile_time_assertion_failed[ ((cond) ? 1 : -1) ] )

//------------------------------------------------------------------------------------------
// will stop program if condition is not met
// Note that we no longer support exceptions.

#ifdef GOLD_VERSION
#  define BPE_VERIFY( test, exception, msg ) {}
#  define BPE_VERIFYA( exception, msg ) {}
#  define BPE_CHECK_SCE(x) (x)
#else
static inline int bpe_sce_check( char const *str, int var )
{
   if ( var < 0 )
   {
      bpe_log_sce_fail( str, var );
   }

   return var;
}
#  define BPE_CHECK_SCE(x) bpe_sce_check( #x, (x) )
#define BPE_VERIFY( test, exception, msg )\
{\
   bool TestResults(test);\
   if (!(TestResults))\
   {\
      bpe_log_assert_failure(__FILE__,__LINE__,"Verify",#test,msg);\
      bpe_query_to_halt_program(__FILE__,__LINE__,"Verify",#test,msg);\
      BPE_SET_BREAKPOINT_ALWAYS;\
      (exception);\
   }\
}

// Always verify
// Note that we no longer support exceptions.
#define BPE_VERIFYA( exception, msg )\
{\
   {\
      bpe_log_assert_failure(__FILE__,__LINE__,"Verify", "Always", msg);\
      bpe_query_to_halt_program(__FILE__,__LINE__,"Verify", "Always", msg);\
      BPE_SET_BREAKPOINT_ALWAYS;\
      (exception); \
   }\
}
#endif

//------------------------------------------------------------------------------------------
// Deal with boost exceptions not linking when using DLL's.
// inline doesn't work correctly in this case using GCC/PS3
#if BPE_TARGET != BPE_TARGET_X360
namespace std
{
   class exception;
}

namespace boost
{
   ENGINE_API void throw_exception(std::exception const & e);
}

// This define shuts up warning:
// warning C4273: 'boost::throw_exception' : inconsistent dll linkage
// If you get other warnings, check the include order, BPEAssert.h should be before boost.
#define BOOST_THROW_EXCEPTION_HPP_INCLUDED
#endif

//------------------------------------------------------------------------------------------

#ifndef BPE_DEBUG
#  define BPE_BREAKPOINT(msg) ((void)0)
#else
#  define BPE_BREAKPOINT(msg) { BPE_SET_BREAKPOINT; }
#endif

//------------------------------------------------------------------------------------------



