@echo off
:: gcc api.c %*
echo %1 | findstr /r "dll" >nul 2>&1 && (
	gcc -shared *.c -o %*
) || (
	gcc *.c %*
)
