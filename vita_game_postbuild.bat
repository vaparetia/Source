rem Add anything you want to run post-build to this.
rem For example:
psp2ctrl fsroot %BPE_REPOSITORY%
rem psp2ctrl fscase ignorecase
rem create savegame directories where the elf is
if not exist %MGS_ROOT%\mgs2x\source\main\VTA_Develop\savedata mkdir %MGS_ROOT%\mgs2x\source\main\VTA_Develop\savedata
if not exist %MGS_ROOT%\mgs2x\source\main\VTA_Debug\savedata mkdir %MGS_ROOT%\mgs2x\source\main\VTA_Debug\savedata