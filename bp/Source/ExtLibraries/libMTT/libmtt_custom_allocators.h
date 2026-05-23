#ifndef __LIBMTT_CUSTOM_ALLOCATORS_H__
#define __LIBMTT_CUSTOM_ALLOCATORS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//Associates a name with an allocator
// id: The unique ID of the allocator, often the base memory address.
// name: The c-string name that will be associated with the allocator
void mttRegisterCustomPoolAllocator (uint64_t id, const char* name);
void mttRegisterCustomStackAllocator (uint64_t id, const char* name);
void mttRegisterCustomHeapAllocator (uint64_t id, const char* name);

//Specifies the size of blocks handed out by the pool allocator.
//Allows custom block sizes in the Dataset Viewer.
// id: The unique ID of the allocator, often the base memory address.
// block_size: The size in bytes of blocks allocated by the pool.
void mttCustomPoolSetBlockSize (uint64_t id, uint64_t block_size);

//Specifies the address range of memory belonging to the allocator.
//Allows range checking & custom display ranges in the Dataset Viewer.
// id: The unique ID of the allocator, often the base memory address.
// start: The minimum address of any block returned by the allocator.
// end: The maximum address plus size of any block returned by the allocator.
void mttCustomAllocatorSetAddressRange (uint64_t id, uint64_t start, uint64_t end);

//Registers a pool allocation event.  Pool allocators allocate memory in fixed size blocks.
// id: The unique ID of the allocator, often the base memory address.
// ptr: A pointer to the allocated block.
// size: The size of the allocated block.
// internal_fragmentation: The overhead for the allocation, including alignment wastage.
// tag: The tag associated with the allocation.  0 is reserved as the invalid value.
void mttCustomPoolAllocation (uint64_t id, const void* ptr, uint64_t size, uint64_t internal_fragmentation, uint64_t tag);

//Registers a stack allocation event.  Stack allocators free memory in reverse allocation order.
// id: The unique ID of the allocator, often the base memory address.
// ptr: A pointer to the allocated block.
// size: The size of the allocated block.
// internal_fragmentation: The overhead for the allocation, including alignment wastage.
// tag: The tag associated with the allocation. 0 is reserved as the invalid value.
void mttCustomStackAllocation (uint64_t id, const void* ptr, uint64_t size, uint64_t internal_fragmentation, uint64_t tag);

//Registers a heap allocation event.  Heap allocators behave like malloc, allowing arbitrarily sized blocks of memory freed in any order.
// id: The unique ID of the allocator, often the base memory address.
// ptr: A pointer to the allocated block.
// size: The size of the allocated block.
// internal_fragmentation: The overhead for the allocation, including alignment wastage.
// tag: The tag associated with the allocation. 0 is reserved as the invalid value.
void mttCustomHeapAllocation (uint64_t id, const void* ptr, uint64_t size, uint64_t internal_fragmentation, uint64_t tag);

//Registers a pool free event.  Pool allocators allocate memory in fixed size blocks.
// id: The unique ID of the allocator, often the base memory address.
// ptr: A pointer to the freed block.
void mttCustomPoolFree (uint64_t id, const void* ptr);

//Registers a stack free event. Stack allocators free memory in reverse allocation order.
// id: The unique ID of the allocator, often the base memory address.
// ptr: A pointer to the freed block.
void mttCustomStackFree (uint64_t id, const void* ptr);

//Registers a heap free event. Heap allocators behave like malloc, allowing arbitrarily sized blocks of memory freed in any order.
// id: The unique ID of the allocator, often the base memory address.
// ptr: A pointer to the freed block.
void mttCustomHeapFree (uint64_t id, const void* ptr);

//Registers a heap reallocation event.
// id: The unique ID of the allocator, often the base memory address.
// old_ptr: A pointer to the block of memory being reallocated.
// ptr: A pointer to the new block of memory.  May be the same as old_ptr.
// size: The size of the new block of memory.
// internal_fragmentation: The overhead for the allocation, including alignment wastage.
void mttCustomHeapRealloc (uint64_t id, const void* old_ptr, const void* ptr, uint64_t size, uint64_t internal_fragmentation);

//Frees all outstanding allocations of an allocator at once.  Useful when the allocator itself is freed.
// id: The unique ID of the allocator, often the base memory address.
void mttCustomAllocatorFreeAll (uint64_t id);

//Sets the parent id of an allocator.  Useful if you are using sub-allocators.
// id: The unique ID of the allocator, often the base memory address.
// parent_id: The unique ID of the super-allocator.
void mttCustomAllocatorSetParent (uint64_t id, uint64_t parent_id);

//Adds a tag to an allocation.
// ptr: A pointer to the block to be tagged.
// tag: The tag id to be associated with the allocation. 0 is reserved as the invalid value.
void mttTagAllocation (const void* ptr, uint64_t tag);

//Associates a string with a tag id.
// tag: The tag id.
// name: The string to be associated with the tag id.
void mttSetTagName (uint64_t tag, const char* name);

#ifdef __cplusplus
}
#endif

#endif
