@echo off

call cp4_environment
START "DummyTitle" "%VS90COMNTOOLS%..\IDE\devenv.exe" "%BPE_ROOT%\Source\Everything.sln"
