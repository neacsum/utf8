@echo off
rem
rem  Create symbolic links for included projects and output directories.
rem  Environment variable DEV_ROOT points to the root of development tree.
rem
rem  This script should be run as administrator
rem

if not defined DEV_ROOT (
  echo Environment variable DEV_ROOT is not set!
  echo Cannot create symlinks.
goto :EOF    
)

if not exist lib\nul mklink /d lib %DEV_ROOT%\lib

pushd "%~dp0include"

if not exist utpp\nul (
  pushd %DEV_ROOT%
  if not exist utpp\nul git clone https://github.com/neacsum/utpp
  popd
  mklink /d utpp %DEV_ROOT%\utpp\include\utpp
)
popd