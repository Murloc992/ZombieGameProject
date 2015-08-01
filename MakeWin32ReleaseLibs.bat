@echo off
echo Building Release Libs
call LibBuilder.bat "release"
rmdir /s /q build