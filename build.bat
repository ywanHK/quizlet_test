@echo off
:: gcc api.c %*
echo %1 | findstr /r "dll" >nul 2>&1 && (
	gcc -shared *.c -o %* && echo BUILD SUCCESSFUL || echo BUILD NOT SUCCESSFUL
) || (
	gcc *.c %* && echo BUILD SUCCESSFUL || echo BUILD NOT SUCCESSFUL
)
