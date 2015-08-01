@echo off
echo Building Debug Libs
call LibBuilder.bat "debug"
rmdir /s /q build