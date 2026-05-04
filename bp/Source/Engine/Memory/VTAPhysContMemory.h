#pragma once
#ifndef __BP_VTAPHYSCONTMEMORY__H
#define __BP_VTAPHYSCONTMEMORY__H

namespace NPhysContMem
{ 
   typedef void (*FORCE_UNLOCK_CALLBACK)(void*);

   /// Aligned always used when allocating any physically continuous memory.
   int const kPhysContMemAlignment = 1024 * 1024;

   /// Size of the allocation when the physically continuous memory is allocated. Since we want all available physically
   /// continuous memory available as a scratch-pad 16MB is allocated which seems to be the upper bound.
   int const kPhysContMemAllocSize = 16 * 1024 * 1024;

   /// Enumerates the different systems that can lock/unlock the physically continuous memory. If a new system needs to 
   /// access physically continuous memory a new enumeration should be added here so the policies can be updated 
   /// accordingly.
   enum EMemOwnerType
   {
      // Texture decompression - The policy is such that texture compression always has precedence. If the memory is 
      // locked and a texture requires decompression, the notify unlocked callback is invoked and the memory is 
      // unlocked. Therefore systems must handle the case where memory could be hijacked by texture decompression.
      OWNER_TYPE_TEXTURE_DECOMPRESSION,

      // Used for internal movie playback and buffering.
      OWNER_TYPE_MOVIE_PLAYBACK,
   };

   void InitPhysContMem();

   /// Returns a pointer to kPhysContMemAllocSize bytes of memory to use as a scratch pad. Texture decompression is the
   /// only system that can override and forcibly unlock the memory. Any other system that tries to lock when already 
   /// locked will result in a failure.
   /// @note The allocation policy is as follows:
   ///    * Texture decompression allocates all available physically continuous memory for use during decompression.
   ///    * Movie playback frees the memory if allocated but does not reallocate any memory. The movie player needs to do 
   ///      its own allocations. The lock flag is still set in this case to still explicit policy enforcement.
   void* LockPhysContMem(EMemOwnerType ownerType, FORCE_UNLOCK_CALLBACK pUnlockCallback, void* pUserData);

   /// Indicates the scratch pad memory is available for other systems to use.
   void UnlockPhysContMem();

   void ShutdownPhysContMem();
}

#endif
