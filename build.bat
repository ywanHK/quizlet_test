@echo off
if "%1"=="-s" (
	del *.o /s /q
)
mingw32-make
