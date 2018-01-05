@echo off

set std=/std:c++latest
set output=.\bin\athi.exe
set olvl=-Ox
set wlvl=/W3 /wd4530 /wd4005 /wd4996 /wd4244 /MP
set options=%std% %wlvl% %olvl% %std%
set includes=/I.\dep\Windows /I.\dep\Universal
set src=.\src\*.cpp
set glew=.\dep\Windows\glew32s.lib
set glfw=.\dep\Windows\glfw3dll.lib
set opencl=.\dep\Windows\OpenCL.lib

set libs=%glew% %glfw% %opencl% opengl32.lib user32.lib gdi32.lib kernel32.lib

del %output%

cl /EHsc -Fe%output% %options% %src% %includes% %libs%
xcopy /y .\dep\Windows\glfw3.dll .\bin

del *.obj
