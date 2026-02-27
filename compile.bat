@echo off
set QT_PATH=D:\Qt\5.15.2\mingw81_64
set MINGW_PATH=D:\Qt\Tools\mingw810_64
set PATH=%MINGW_PATH%\bin;%QT_PATH%\bin;%PATH%

cd /d "%~dp0"
echo Running qmake...
%QT_PATH%\bin\qmake.exe PonyWork.pro
echo Running make...
%MINGW_PATH%\bin\mingw32-make.exe release
echo Compilation completed!
pause