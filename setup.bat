set path=%path%;C:\qt\6.4.1\msvc2019_64\bin\
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cd ..
