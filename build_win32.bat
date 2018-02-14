@echo off

set std=/std:c++latest
set output=.\bin\athi.exe

set olvl=
set wlvl=/W3 /wd4530 /wd4005 /wd4996 /wd4244 /wd4101 /MP
set options=%std% %wlvl% %olvl% %std%

set includes=/I.\dep\Windows /I.\dep\Universal
set src=.\src\*.cpp .\dep\Universal\*.cpp .\src\Renderer\*.cpp .\src\Utility\*.cpp

set glew=.\dep\Windows\glew32s.lib
set glfw=.\dep\Windows\glfw3dll.lib
set opencl=.\dep\Windows\OpenCL.lib
set freetype=.\dep\Windows\freetype271.lib

set libs=%glew% %glfw% %opencl% %freetype% opengl32.lib user32.lib gdi32.lib kernel32.lib

del %output%

call cl /EHsc /DNDEBUG -Fe%output% %options% %src% %includes% %libs%
xcopy /y .\dep\Windows\glfw3.dll .\bin
xcopy /y .\dep\Windows\freetype271.dll .\bin

del *.obj
