
call ..\..\cp4_environment.bat

SETLOCAL
IF "%1"=="" GOTO ERROR1

for /F %%i in (%1\bp_gcl_process_list.txt) do (
gclconv.exe @_bp_gclc_args_%1.txt @_bp_gclc_args_shared.txt -o %1\%%i.gco _bp\%%i.gcl
gclk.exe @_bp_gclk_args.txt -o %1\%%i.gcx %1\%%i.gco
)
GOTO END

:ERROR1
echo Region '%1' is invalid

GOTO END

:END