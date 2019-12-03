@echo off
rem
rem  Create symbolic links for included projects and output directories.
rem  Environment variable DEV_ROOT points to the root of development tree.
rem
rem  This script should be run as administrator
rem
if defined DEV_ROOT goto MAKELINKS
echo Environment variable DEV_ROOT is not set!
ECHO Cannot create symlinks.
goto :EOF

:MAKELINKS
if not exist lib\nul mklink /d lib %DEV_ROOT%\lib

pushd "%~dp0include"

if not exist utpp\nul    mklink /d utpp %DEV_ROOT%\utpp\include\utpp
popd