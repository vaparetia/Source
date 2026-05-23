Linking command line (change paths as necessary):

-Wl,--whole-archive C:\workspace\MemoryTrackingTool\libmtt\libmtt\PS3_PPU_Debug\libmtt.lib -Wl,--no-whole-archive -Wl,--keep=C:\workspace\MemoryTrackingTool\libmtt\libmtt\libmtt_keep_list.txt


The binary patcher will patch calls to malloc, free, etc.

To instrument your own custom allocators, please see libmtt_custom_allocators.h
