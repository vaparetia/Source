
call ..\..\cp4_environment.bat

SETLOCAL
IF "%1"=="" GOTO ERROR1

for /F %%i in (bp_cdc_process_list.txt) do (
call bp_cdcconv_region.bat %1 %%i
)

GOTO END

:ERROR1
echo Region '%1' is invalid

GOTO END

:END