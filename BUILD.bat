@echo off
rem
rem  Master build script. Execute this script to create all output libraries
rem  Environment variable DEV_ROOT points to the root of development tree.
rem

call mklinks.bat
call setup.bat
if "%~1"=="" (msbuild build.proj) else (msbuild -target:%1 build.proj)
