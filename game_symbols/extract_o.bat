for %%i in (*.a) do (
chdir %%~ni
ar -x ..\%%~ni.a
cd ..
)