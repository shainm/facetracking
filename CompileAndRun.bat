.\FirstRun.bat
cd build
cmake --build . --config Release
cd ..
move haarcascade_frontalface_default.xml .\build\Release
.\Run.bat