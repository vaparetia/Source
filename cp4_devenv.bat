@echo off

call cp4_environment
START "DummyTitle" "%VS90COMNTOOLS%..\IDE\devenv.exe" "%MGS_ROOT%\mgs2x\source\MGS2.sln"
