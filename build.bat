@echo off

set cpp_standard  = /std:c++latest
set output        = .\bin\athi.exe

set compile_flags       = /MP /DNDEBUG
set optimization_lvl    = -Ox
set warning_lvl         = /W3
set warning_flags       = /wd4530 /wd4005 /wd4996 /wd4244 /wd4101

set includes_windows        = /I.\dep\Windows
set includes_dependencies   = /I.\dep\Universal

set src_application  = .\src\*.cpp .\src\Renderer\*.cpp .\src\Utility\*.cpp
set src_dependencies = .\dep\Universal\*.cpp

set glew     =.\dep\Windows\glew32s.lib
set glfw     =.\dep\Windows\glfw3dll.lib
set opencl   =.\dep\Windows\OpenCL.lib
set freetype =.\dep\Windows\freetype271.lib

set includes    = %includes_windows% %includes_dependencies%
set src         = %src_application% %src_dependencies%
set options     = %compile_flags% %std% %warning_lvl% %warning_flags% %optimization_lvl% %cpp_standard%
set libs        = %glew% %glfw% %opencl% %freetype% opengl32.lib user32.lib gdi32.lib kernel32.lib

del %output%

call cl %options% %src% %includes% %libs% -Fe%output%
xcopy /y .\dep\Windows\glfw3.dll .\bin
xcopy /y .\dep\Windows\freetype271.dll .\bin

del *.obj
