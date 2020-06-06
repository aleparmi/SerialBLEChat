@echo off
REM generated from catkin/cmake/templates/env.bat.in

if "%1"=="" (
  echo "Usage: env.bat COMMANDS"
  echo "Calling env.bat without arguments is not supported anymore. Instead spawn a subshell and source a setup file manually."
  exit 1
) else (
  call "C:/Users/Andrea/Google Drive/Hochschule/NGN/Project/Project_local/serial/catkin_generated/setup_cached.bat"
  %*
)
