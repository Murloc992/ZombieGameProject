@echo off
set buildconfig=%1
rmdir /s /q "build"
rmdir /s /q "libs/win32%buildconfig%"
mkdir "build"
mkdir "libs/win32%buildconfig%"
cd "build"
call vcvars32.bat
cmake ../ -DCMAKE_BUILD_TYPE=%buildconfig% -G "NMake Makefiles"
nmake
cd "../libs/boost/"
start /wait b2 -j8 --without-python --build-dir="../../build"  toolset=msvc cxxflags="-fPIC" link=static threading=multi %buildconfig%
cd "../../build/"
for /r %%f in (*.lib) do @xcopy /yq "%%f" "../libs/win32%buildconfig%"
cd "../"
exit